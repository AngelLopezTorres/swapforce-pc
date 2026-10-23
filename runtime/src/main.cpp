#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "config.h"
#include "guest_memory.h"
#include "image_loader.h"
#include "import_log.h"
#include "kernel_vars.h"
#include "ppc_config.h"
#include "ppc_context.h"
#include "xex_header.h"
#include "guest_thread.h"
#include "guest_abi.h"
#include "vfs.h"
#include "gpu.h"
#include "d3d12_backend.h"
#include "ucode_hlsl.h"
#include "perf.h"
#include "hid/portal.h"
#include "audio/xma_context.h"
#include "kernel/xboxkrnl_sync.h"

namespace rt::gpu { int Pm4SelfTest(); extern bool g_fbSlowPath, g_fbVerify; }
namespace rt::gpu::d3d { extern bool g_texSlowPath, g_texVerify, g_resolveSlowPath, g_resolveVerify, g_texMemoOff, g_resolveSync, g_noResolveShortcut, g_resolveNoDse, g_verifyShortcut; extern unsigned g_rowThreads, g_kickDraws, g_kickResolveDraws; extern bool g_windowNoActivate; extern bool g_asyncResolveWriter; }
namespace rt::gpu::d3d { void SetDumpHlslHash(uint64_t hash); }
int RunCpuSelfTest(uint64_t n, uint64_t seed);   // selftest\cpu_selftest.cpp
namespace rt { extern bool g_binkProbeOn; extern bool g_importSerialize; extern bool g_noAudio; void AudioSummary(); void BinkProbeReport(); }
namespace rt { void ConfigureInputStartInjection(uint32_t afterCalls, uint32_t durationCalls); }
namespace rt { void ConfigureXamUser0SignedIn(bool enabled, uint32_t afterCalls); }
static std::string g_userdataOverride;
PPC_EXTERN_FUNC(__imp__XamInputGetState);
PPC_EXTERN_FUNC(__imp__XamInputGetCapabilities);
PPC_EXTERN_FUNC(__imp__XexGetModuleHandle);
PPC_EXTERN_FUNC(__imp__XexGetProcedureAddress);
PPC_EXTERN_FUNC(__imp__XexUnloadImage);
PPC_EXTERN_FUNC(__imp__XMACreateContext);
PPC_EXTERN_FUNC(__imp__XMAReleaseContext);

PPC_EXTERN_FUNC(_xstart);
PPC_EXTERN_FUNC(__imp__NtCreateFile);
PPC_EXTERN_FUNC(__imp__NtReadFile);
PPC_EXTERN_FUNC(__imp__NtClose);

// Prueba de E/S sobre las importaciones reales (sin ejecutar codigo del juego).
static int IoSelfTest(rt::ImportLog& log) {
    uint8_t* base = rt::GuestMemory::Base();
    constexpr uint32_t S = 0x85030000;
    rt::GuestMemory::Commit(S, 0x20000);
    auto* ctx = new PPCContext();
    std::memset((void*)ctx, 0, sizeof(*ctx));
    ctx->r1.u64 = S + 0x1F000;
    auto create = [&](const char* path, uint32_t* status, uint32_t* handle) {
        uint32_t n = (uint32_t)std::strlen(path);
        std::memcpy(base + S + 0x100, path, n + 1);
        rt::P16(base, S, (uint16_t)n); rt::P16(base, S + 2, (uint16_t)(n + 1)); rt::P32(base, S + 4, S + 0x100);
        rt::P32(base, S + 0x20, 0); rt::P32(base, S + 0x24, S); rt::P32(base, S + 0x28, 0x40); // OBJECT_ATTRIBUTES
        ctx->r3.u64 = S + 0x40; ctx->r4.u64 = 0x80100080; ctx->r5.u64 = S + 0x20; ctx->r6.u64 = S + 0x50;
        ctx->r7.u64 = 0; ctx->r8.u64 = 0; ctx->r9.u64 = 7; ctx->r10.u64 = 1;
        rt::P32(base, ctx->r1.u32 + 0x54, 0x4020);
        __imp__NtCreateFile(*ctx, base);
        *status = ctx->r3.u32; *handle = rt::G32(base, S + 0x40);
    };
    uint32_t st, h;
    create("game:\\archives\\CollectibleIcons.pak", &st, &h);
    std::printf("NtCreateFile game:\\archives\\CollectibleIcons.pak -> 0x%08X handle=%08X\n", st, h);
    bool ok = st == 0;
    ctx->r3.u64 = h; ctx->r4.u64 = 0; ctx->r5.u64 = 0; ctx->r6.u64 = 0; ctx->r7.u64 = S + 0x60; ctx->r8.u64 = S + 0x1000;
    ctx->r9.u64 = 4096; ctx->r10.u64 = 0; // sin offset: posicion actual
    __imp__NtReadFile(*ctx, base);
    uint32_t rs = ctx->r3.u32, bytes = rt::G32(base, S + 0x64);
    std::printf("NtReadFile -> 0x%08X bytes=%u primeros=%02X%02X%02X%02X\n", rs, bytes, base[S + 0x1000], base[S + 0x1001], base[S + 0x1002], base[S + 0x1003]);
    ok = ok && rs == 0 && bytes == 4096;
    ctx->r3.u64 = h; __imp__NtClose(*ctx, base);
    create("game:\\archives\\ScriptPatch.pak", &st, &h);
    std::printf("NtCreateFile game:\\archives\\ScriptPatch.pak -> 0x%08X handle=%08X\n", st, h);
    ok = ok && st == 0xC0000034;
    create("game:\\archives\\..\\default.xex", &st, &h);
    std::printf("NtCreateFile game:\\archives\\..\\default.xex -> 0x%08X\n", st);
    ok = ok && st == 0xC0000033;
    log.Printf("io-selftest %s", ok ? "OK" : "FALLO");
    return ok ? 0 : 1;
}

static bool XamPortalExportSelfTest(uint8_t* base) {
    constexpr uint32_t scratch = 0x85030000;
    if (!rt::GuestMemory::Commit(scratch, 0x1000)) return false;

    constexpr char moduleName[] = "xam.xex";
    std::memcpy(base + scratch + 0x100, moduleName, sizeof(moduleName));
    PPCContext ctx{};
    ctx.r3.u64 = scratch + 0x100;
    ctx.r4.u64 = scratch + 0x20;
    __imp__XexGetModuleHandle(ctx, base);
    if (ctx.r3.u32 != 0) return false;
    uint32_t handle = rt::G32(base, scratch + 0x20);

    auto resolve = [&](uint32_t ordinal, uint32_t expectedGuest, uint32_t expectedResult) {
        ctx = PPCContext{};
        ctx.r3.u64 = handle;
        ctx.r4.u64 = ordinal;
        ctx.r5.u64 = scratch + 0x24;
        __imp__XexGetProcedureAddress(ctx, base);
        uint32_t address = rt::G32(base, scratch + 0x24);
        if (ctx.r3.u32 != expectedResult || address != expectedGuest) return false;
        if (!expectedGuest) return true;
        PPCFunc* thunk = PPC_LOOKUP_FUNC(base, address);
        if (!thunk) return false;
        ctx = PPCContext{};
        thunk(ctx, base);
        return ctx.r3.u32 == 0xC00000BB;
    };

    bool ok = resolve(0x4A1, 0x830A9EE0, 0) && resolve(0x4A2, 0x830A9EE4, 0) &&
              resolve(0x4A3, 0, 0xC0000225);
    ctx = PPCContext{};
    ctx.r3.u64 = handle;
    __imp__XexUnloadImage(ctx, base);
    return ok;
}

static bool XmaSelfTest(uint8_t* base) {
    constexpr uint32_t scratch = 0x85034000;
    if (!rt::GuestMemory::Commit(scratch, 0x1000)) return false;

    PPCContext ctx{};
    ctx.r3.u64 = scratch;
    __imp__XMACreateContext(ctx, base);
    uint32_t createStatus = ctx.r3.u32;
    uint32_t first = rt::G32(base, scratch);
    if (createStatus != 0 || !first) {
        std::printf("xma-selftest create=%08X context=%08X -> FALLO\n", createStatus, first);
        return false;
    }
    rt::audio::StopXmaWorker(); // El paso sintetico de este selftest se ejecuta de forma determinista.
    uint32_t contextArray = rt::audio::XmaContextArrayGuestAddress();

    bool layout = sizeof(rt::audio::XmaContextData) == 64 && first == contextArray &&
                  rt::G32(base, rt::audio::kXmaContextArrayAddressRegister) == (contextArray & 0x1FFFFFFF) &&
                  rt::G32(base, rt::audio::kXmaCurrentContextIndexRegister) == 0 &&
                  rt::G32(base, rt::audio::kXmaNextContextIndexRegister) == 1;
    const uint32_t input0Word = static_cast<uint32_t>(rt::audio::XmaContextWord::Input0PhysicalAddress);
    const uint32_t outputFlagsWord = static_cast<uint32_t>(rt::audio::XmaContextWord::OutputReadOffsetAndFlags);
    rt::P32(base, first + input0Word * 4, 0x01234567);
    rt::P32(base, first + outputFlagsWord * 4, 0xC0000000);
    layout = layout && rt::G32(base, first + input0Word * 4) == 0x01234567 &&
             rt::G32(base, first + outputFlagsWord * 4) == 0xC0000000;

    bool registers = rt::audio::WriteXmaRegister(rt::audio::kXmaCurrentContextIndexRegister, 7) &&
                     rt::audio::WriteXmaRegister(rt::audio::kXmaNextContextIndexRegister, 8) &&
                     rt::audio::ReadXmaRegister(rt::audio::kXmaCurrentContextIndexRegister) == 7 &&
                     rt::audio::ReadXmaRegister(rt::audio::kXmaNextContextIndexRegister) == 8;

    uint64_t requestsBefore = rt::audio::XmaDecodeRequestCount();
    auto boundary = rt::audio::SubmitXmaDecode(first);
    bool decoderBoundary = boundary == rt::audio::XmaDecodeBoundaryResult::BackendUnavailable &&
                           !rt::audio::XmaDecoderBackendAvailable() &&
                           rt::audio::XmaDecodeRequestCount() == requestsBefore + 1;

    // El sumidero sintetico consume un paquete y hace visible el avance del
    // contexto sin presentar su salida silenciosa como audio decodificado.
    constexpr uint32_t outputPhysical = 0x00100000;
    std::memset(base + 0xA0000000u + outputPhysical, 0x7F, 1024);
    rt::P32(base, first, 1u | (1u << 20) | (4u << 22));
    rt::P32(base, first + 4, 0x80000000u);
    rt::P32(base, first + 16, 0);
    rt::P32(base, first + 28, outputPhysical);
    uint64_t packetsBefore = rt::audio::XmaSimulatedPacketCount();
    bool silence = rt::audio::SimulateXmaStep() == 1 &&
                   rt::audio::XmaSimulatedPacketCount() == packetsBefore + 1 &&
                   (rt::G32(base, first) & (1u << 20)) == 0 &&
                   rt::G32(base, first + 8) == rt::audio::kXmaPacketBytes * 8;
    for (uint32_t j = 0; j < 1024; ++j) silence = silence && base[0xA0000000u + outputPhysical + j] == 0;
    rt::audio::SimulateXmaStep();
    silence = silence && (rt::G32(base, first + 4) & 0x80000000u) == 0;

    ctx = PPCContext{};
    ctx.r3.u64 = scratch + 4;
    __imp__XMACreateContext(ctx, base);
    uint32_t secondStatus = ctx.r3.u32;
    uint32_t second = rt::G32(base, scratch + 4);
    if (secondStatus != 0 || !second) {
        ctx = PPCContext{};
        ctx.r3.u64 = first;
        __imp__XMAReleaseContext(ctx, base);
        return false;
    }
    bool allocation = secondStatus == 0 && second == first + rt::audio::kXmaContextBytes;

    ctx = PPCContext{};
    ctx.r3.u64 = first;
    __imp__XMAReleaseContext(ctx, base);
    bool released = ctx.r3.u32 == 0 && rt::G32(base, first) == 0 &&
                    !rt::audio::ReleaseXmaContext(first);
    ctx = PPCContext{};
    ctx.r3.u64 = second;
    __imp__XMAReleaseContext(ctx, base);
    released = released && ctx.r3.u32 == 0 && rt::G32(base, second) == 0;

    uint32_t allocated[rt::audio::kXmaContextCount]{};
    uint32_t count = 0;
    for (; count < rt::audio::kXmaContextCount; ++count) {
        allocated[count] = rt::audio::CreateXmaContext();
        if (!allocated[count]) break;
    }
    bool exhaustion = count == rt::audio::kXmaContextCount && rt::audio::CreateXmaContext() == 0;
    for (uint32_t i = 0; i < count; ++i) released = rt::audio::ReleaseXmaContext(allocated[i]) && released;
    exhaustion = exhaustion && count == rt::audio::kXmaContextCount;

    bool ok = createStatus == 0 && first != 0 && layout && registers && decoderBoundary && silence && allocation && released && exhaustion;
    std::printf("xma-selftest create=%08X context=%08X array=%08X layout=%s registers=%s decode-boundary=%s silence=%s packets=%llu requests=%llu release=%s capacity=%u/%u -> %s\n",
                createStatus, first, contextArray, layout ? "OK" : "FAIL", registers ? "OK" : "FAIL",
                decoderBoundary ? "BACKEND_UNAVAILABLE" : "FAIL", silence ? "OK" : "FAIL",
                static_cast<unsigned long long>(rt::audio::XmaSimulatedPacketCount()),
                static_cast<unsigned long long>(rt::audio::XmaDecodeRequestCount()),
                released ? "OK" : "FAIL", count, rt::audio::kXmaContextCount, ok ? "OK" : "FALLO");
    return ok;
}

static bool LoadAll(std::string& err, rt::ImageLoadResult& img, rt::XexHeaderInfo& xex, int& vars) {
    if (!rt::LoadPaths(const_cast<rt::Paths&>(rt::GetPaths()), &err)) return false;
    if (!g_userdataOverride.empty()) rt::SetUserdataDir(g_userdataOverride);
    if (!rt::GuestMemory::Init()) { err = "no se puede reservar 4 GB alineados"; return false; }
    if (!rt::LoadGameImage(rt::GetPaths().image, img, &err)) return false;
    if (!rt::LoadXexHeader(rt::GetPaths().xex, xex, &err)) return false;
    rt::Vfs::Get().Init(rt::GetPaths().isoRoot, rt::GetPaths().cacheDir, rt::GetPaths().hddDir);
    vars = rt::InitKernelVars(xex.guestAddr);
    return vars >= 0;
}

static LONG WINAPI CrashFilter(EXCEPTION_POINTERS* ep) {
    FILE* f = std::fopen("crash.txt", "w");
    if (f) {
        std::fprintf(f, "excepcion no controlada codigo=%08lX rip=%p base_exe=%p\n", ep->ExceptionRecord->ExceptionCode,
                     ep->ExceptionRecord->ExceptionAddress, (void*)GetModuleHandleA(nullptr));
        std::fclose(f);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char** argv) {
    SetUnhandledExceptionFilter(CrashFilter);
    auto& log = rt::ImportLog::Get();
    std::string logPath = "runtime.log";
    bool selftest = false, iotest = false, inputSelfTest = false, portalSelfTest = false, xmaSelfTest = false;
    std::string perfOut;
    std::string userdataOverride;
    uint32_t traceKernelHandle = 0;
    uint32_t inputStartAfterCalls = 0, inputStartDurationCalls = 0;
    bool testUser0SignedIn = false;
    uint32_t testUser0SignedInAfterCalls = 0;
    uint64_t dumpHlslHash = 0;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--max-imports") && i + 1 < argc) log.Config().maxImports = std::strtoull(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--log") && i + 1 < argc) logPath = argv[++i];
        else if (!std::strcmp(argv[i], "--userdata-dir") && i + 1 < argc) userdataOverride = argv[++i];
        else if (!std::strcmp(argv[i], "--inject-start-after-calls") && i + 1 < argc) inputStartAfterCalls = (uint32_t)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--inject-start-duration-calls") && i + 1 < argc) inputStartDurationCalls = (uint32_t)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--test-user0-signed-in")) testUser0SignedIn = true;
        else if (!std::strcmp(argv[i], "--test-user0-signed-in-after-calls") && i + 1 < argc) { testUser0SignedIn = true; testUser0SignedInAfterCalls = (uint32_t)std::strtoul(argv[++i], nullptr, 0); }
        else if (!std::strcmp(argv[i], "--log-all")) log.Config().logAll = true;
        else if (!std::strcmp(argv[i], "--selftest-load")) selftest = true;
        else if (!std::strcmp(argv[i], "--io-selftest")) iotest = true;
        else if (!std::strcmp(argv[i], "--selftest-input")) inputSelfTest = true;
        else if (!std::strcmp(argv[i], "--selftest-portal")) portalSelfTest = true;
        else if (!std::strcmp(argv[i], "--selftest-xma")) xmaSelfTest = true;
        else if (!std::strcmp(argv[i], "--gpu-regdump")) rt::gpu::Cfg().regDump = true;
        else if (!std::strcmp(argv[i], "--gpu-log") && i + 1 < argc) rt::gpu::Cfg().logPath = argv[++i];
        else if (!std::strcmp(argv[i], "--gpu-statelog")) rt::gpu::Cfg().stateLog = true;
        else if (!std::strcmp(argv[i], "--gpu-statelog-frame") && i + 1 < argc) { rt::gpu::Cfg().stateLog = true; rt::gpu::Cfg().stateLogFrame = std::strtoull(argv[++i], nullptr, 0); }
        else if (!std::strcmp(argv[i], "--gpu-hist")) rt::gpu::Cfg().hist = true;
        else if (!std::strcmp(argv[i], "--frames-dir") && i + 1 < argc) rt::gpu::Cfg().framesDir = argv[++i];
        else if (!std::strcmp(argv[i], "--shader-scan")) rt::gpu::Cfg().shaderScan = true;
        else if (!std::strcmp(argv[i], "--window-noactivate")) rt::gpu::d3d::g_windowNoActivate = true;
        else if (!std::strcmp(argv[i], "--dump-fb")) rt::gpu::Cfg().dumpFb = true;
        else if (!std::strcmp(argv[i], "--no-d3d")) rt::gpu::Cfg().noD3d = true;
        else if (!std::strcmp(argv[i], "--no-audio")) rt::g_noAudio = true;
        else if (!std::strcmp(argv[i], "--bink-trace")) { rt::gpu::Cfg().binkTrace = true; rt::g_binkProbeOn = true; }
        else if (!std::strcmp(argv[i], "--headless")) rt::gpu::Cfg().headless = true;
        else if (!std::strcmp(argv[i], "--tex-slow-path")) rt::gpu::d3d::g_texSlowPath = true;
        else if (!std::strcmp(argv[i], "--tex-verify")) rt::gpu::d3d::g_texVerify = true;
        else if (!std::strcmp(argv[i], "--resolve-slow-path")) rt::gpu::d3d::g_resolveSlowPath = true;
        else if (!std::strcmp(argv[i], "--resolve-verify")) rt::gpu::d3d::g_resolveVerify = true;
        else if (!std::strcmp(argv[i], "--no-tex-memo")) rt::gpu::d3d::g_texMemoOff = true;
        else if (!std::strcmp(argv[i], "--resolve-sync")) rt::gpu::d3d::g_resolveSync = true;
        else if (!std::strcmp(argv[i], "--fb-slow-path")) rt::gpu::g_fbSlowPath = true;
        else if (!std::strcmp(argv[i], "--sync-resolve-writer")) rt::gpu::d3d::g_asyncResolveWriter = false;
        else if (!std::strcmp(argv[i], "--kick-draws") && i + 1 < argc) rt::gpu::d3d::g_kickDraws = (unsigned)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--kick-resolve-draws") && i + 1 < argc) rt::gpu::d3d::g_kickResolveDraws = (unsigned)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--fb-verify")) rt::gpu::g_fbVerify = true;
        else if (!std::strcmp(argv[i], "--verify-shortcut")) rt::gpu::d3d::g_verifyShortcut = true;
        else if (!std::strcmp(argv[i], "--no-resolve-shortcut")) rt::gpu::d3d::g_noResolveShortcut = true;
        else if (!std::strcmp(argv[i], "--no-resolve-dse")) rt::gpu::d3d::g_resolveNoDse = true;
        else if (!std::strcmp(argv[i], "--gpu-row-threads") && i + 1 < argc) rt::gpu::d3d::g_rowThreads = (unsigned)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--perf-nosample")) rt::perf::g_sample = false;
        else if (!std::strcmp(argv[i], "--import-serialize")) rt::g_importSerialize = true;
        else if (!std::strcmp(argv[i], "--perf")) perfOut = (i + 1 < argc && argv[i + 1][0] != '-') ? argv[++i] : "perf_report.txt";
        else if (!std::strcmp(argv[i], "--warp")) rt::gpu::Cfg().warp = true;
        else if (!std::strcmp(argv[i], "--d3d-clear-test")) rt::gpu::Cfg().clearTest = true;
        else if (!std::strcmp(argv[i], "--d3d-stats")) rt::gpu::Cfg().d3dStats = true;
        else if (!std::strcmp(argv[i], "--dump-textures") && i + 1 < argc) rt::gpu::Cfg().dumpTextures = (uint32_t)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--dump-resolves") && i + 1 < argc) rt::gpu::Cfg().dumpResolves = (uint32_t)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--dump-resolves-frame") && i + 1 < argc) rt::gpu::Cfg().dumpResolveFrame = std::strtoull(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--gpu-dump-hlsl-hash") && i + 1 < argc) dumpHlslHash = std::strtoull(argv[++i], nullptr, 16);
        else if (!std::strcmp(argv[i], "--dump-frames") && i + 1 < argc) rt::gpu::Cfg().dumpFrames = argv[++i];
        else if (!std::strcmp(argv[i], "--trace-kernel-handle") && i + 1 < argc) traceKernelHandle = (uint32_t)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--fb-frames") && i + 1 < argc) rt::gpu::Cfg().fbFrames = argv[++i];
        else if (!std::strcmp(argv[i], "--selftest-pm4")) return rt::gpu::Pm4SelfTest();
        else if (!std::strcmp(argv[i], "--selftest-cpu")) {   // --selftest-cpu [N=10000] [semilla=1]
            uint64_t n = 10000, seed = 1;
            if (i + 1 < argc && argv[i + 1][0] != '-') n = std::strtoull(argv[++i], nullptr, 0);
            if (i + 1 < argc && argv[i + 1][0] != '-') seed = std::strtoull(argv[++i], nullptr, 0);
            return RunCpuSelfTest(n, seed);
        }
        else if (!std::strcmp(argv[i], "--shader-test") && i + 1 < argc) return rt::gpu::RunShaderTest(argv[i + 1]);
        else if (!std::strcmp(argv[i], "--selftest-phys")) {
            if (!rt::GuestMemory::Init()) { std::printf("selftest-phys: Init fallo\n"); return 2; }
            uint8_t* b = rt::GuestMemory::Base();
            bool views = rt::GuestMemory::PhysicalViewsMapped();
            volatile uint32_t* a = reinterpret_cast<volatile uint32_t*>(b + 0xA0001000u);
            *a = 0xCAFEBABE;
            uint32_t c = *reinterpret_cast<volatile uint32_t*>(b + 0xC0001000u);
            *reinterpret_cast<volatile uint32_t*>(b + 0xC0002000u) = 0x12345678;
            uint32_t a2 = *reinterpret_cast<volatile uint32_t*>(b + 0xA0002000u);
            bool ok = views && c == 0xCAFEBABE && a2 == 0x12345678;
            std::printf("selftest-phys: vistas=%d A0:0x1000 escrito CAFEBABE -> C0:0x1000 leido %08X; C0:0x2000 escrito 12345678 -> A0:0x2000 leido %08X; E0 no mapeada (documentado) -> %s\n", (int)views, c, a2, ok ? "OK" : "FALLO");
            return ok ? 0 : 1;
        }
    }
    if (!log.Open(logPath)) { std::fprintf(stderr, "no se puede abrir %s\n", logPath.c_str()); return 2; }
    g_userdataOverride = userdataOverride;
    rt::SetSyncTraceHandle(traceKernelHandle);
    rt::ConfigureInputStartInjection(inputStartAfterCalls, inputStartDurationCalls);
    rt::ConfigureXamUser0SignedIn(testUser0SignedIn, testUser0SignedInAfterCalls);
    rt::gpu::d3d::SetDumpHlslHash(dumpHlslHash);

    if (inputSelfTest) {
        if (!rt::GuestMemory::Init()) { std::printf("input-selftest: GuestMemory::Init fallo\n"); return 2; }
        constexpr uint32_t out = 0x85030000;
        if (!rt::GuestMemory::Commit(out, 0x1000)) { std::printf("input-selftest: Commit fallo\n"); return 2; }
        auto* ctx = new PPCContext{};
        auto* base = rt::GuestMemory::Base();
        ctx->r3.u64=0; ctx->r5.u64=out;
        __imp__XamInputGetState(*ctx,base);
        uint32_t stateRc=ctx->r3.u32, buttons=rt::G16(base,out+4);
        ctx->r3.u64=0; ctx->r4.u64=0; ctx->r5.u64=out+0x100;
        __imp__XamInputGetCapabilities(*ctx,base);
        uint32_t capsRc=ctx->r3.u32;
        bool ok=stateRc==0 && capsRc==0;
        std::printf("input-selftest state=%08X buttons=0x%04X capabilities=%08X %s\n",stateRc,buttons,capsRc,ok?"OK":"FALLO");
        delete ctx;
        return ok?0:1;
    }

    if (portalSelfTest) {
        if (!rt::GuestMemory::Init()) { std::printf("portal-selftest: GuestMemory::Init fallo\n"); return 2; }
        uint8_t* base = rt::GuestMemory::Base();
        bool dispatch = XamPortalExportSelfTest(base);
        rt::hid::PortalProbeResult probe;
        bool probeCall = rt::hid::ProbePortal(probe);
        bool physical = probeCall && rt::hid::PortalProbeAccepted(probe);
        std::printf("portal-selftest dispatch=%s get/set=0x4A1/0x4A2 unknown=0xC0000225; ", dispatch ? "OK" : "FALLO");
        std::printf("device=%s open=%s interface0=%s interrupt-in=%s read=%s bytes=%u endpoint=0x%02X error=%lu detail=%s -> %s\n",
                    probe.found ? "present" : "missing", probe.opened ? "yes" : "no",
                    probe.interfaceZero ? "yes" : "no", probe.interruptInFound ? "yes" : "no",
                    probe.readSucceeded ? "complete" : (probe.readTimedOut ? "timeout" : (probe.readAttempted ? "failed" : "not-attempted")),
                    probe.bytesRead, probe.endpoint, static_cast<unsigned long>(probe.error), probe.detail.c_str(),
                    dispatch && physical ? "OK" : "FALLO");
        log.Printf("portal-selftest dispatch=%s probe=%s endpoint=0x%02X bytes=%u error=%lu",
                   dispatch ? "OK" : "FALLO", physical ? "OK" : "FALLO", probe.endpoint,
                   probe.bytesRead, static_cast<unsigned long>(probe.error));
        return dispatch && physical ? 0 : 1;
    }

    if (xmaSelfTest) {
        if (!rt::GuestMemory::Init()) { std::printf("xma-selftest: GuestMemory::Init fallo\n"); return 2; }
        bool ok = XmaSelfTest(rt::GuestMemory::Base());
        log.Printf("xma-selftest %s", ok ? "OK" : "FALLO");
        return ok ? 0 : 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--selftest-gpu") && i + 1 < argc) {
            std::string e;
            if (!rt::LoadPaths(const_cast<rt::Paths&>(rt::GetPaths()), &e)) { std::printf("ERROR: %s\n", e.c_str()); return 2; }
            if (!userdataOverride.empty()) rt::SetUserdataDir(userdataOverride);
            if (!rt::GuestMemory::Init()) { std::printf("selftest-gpu: GuestMemory::Init fallo\n"); return 2; }
            rt::gpu::Cfg().noThreads = true;
            rt::gpu::Start();
            rt::gpu::d3d::Options o;
            auto& gc = rt::gpu::Cfg();
            o.headless = gc.headless; o.warp = gc.warp; o.stats = true; o.dumpFrames = gc.dumpFrames.empty() ? "1" : gc.dumpFrames;
            if (!rt::gpu::d3d::Start(o)) { std::printf("selftest-gpu: D3D12 no disponible\n"); return 2; }
            int rc = rt::gpu::SelfTestRender(argv[i + 1], 200, 100, 50);
            rt::gpu::Stop();
            rt::gpu::Summary();
            log.Summary();
            return rc;
        }
        if (!std::strcmp(argv[i], "--vfs-test") && i + 1 < argc) {
            std::string e;
            if (!rt::LoadPaths(const_cast<rt::Paths&>(rt::GetPaths()), &e)) { std::printf("ERROR: %s\n", e.c_str()); return 2; }
            rt::Vfs::Get().Init(rt::GetPaths().isoRoot, rt::GetPaths().cacheDir, rt::GetPaths().hddDir);
            rt::VfsResolved r = rt::Vfs::Get().Resolve(argv[i + 1]);
            WIN32_FILE_ATTRIBUTE_DATA d = {};
            bool ok = r.status == 0 && GetFileAttributesExA(r.host.c_str(), GetFileExInfoStandard, &d);
            unsigned long long sz = ((unsigned long long)d.nFileSizeHigh << 32) | d.nFileSizeLow;
            if (ok) std::printf("resolved=%s size=%llu\n", r.host.c_str(), sz);
            else std::printf("no resuelto: status=0x%08X host=%s\n", r.status, r.host.c_str());
            return ok ? 0 : 1;
        }
    }
    std::string err;
    rt::ImageLoadResult img;
    rt::XexHeaderInfo xex;
    int vars = 0;
    if (!LoadAll(err, img, xex, vars)) { log.Printf("ERROR carga: %s", err.c_str()); return 2; }

    if (iotest) return IoSelfTest(log);
    if (selftest) {
        uint8_t* base = rt::GuestMemory::Base();
        log.Printf("base=%p fixed=%d", (void*)base, (int)rt::GuestMemory::ReservedAtFixed());
        log.Printf("image %s size=0x%X file=0x%llX mz=%d", img.mz ? "ok" : "FALLO", rt::kImageSize, (unsigned long long)img.fileSize, (int)img.mz);
        log.Printf("xex header size=0x%X guest=%08X", xex.size, xex.guestAddr);
        size_t n = 0;
        for (const PPCFuncMapping* m = PPCFuncMappings; m->guest != 0; ++m) ++n;
        log.Printf("functable entries=%zu (PPCFuncMappings=%zu)", img.funcTableEntries, n);
        log.Printf("vars=%d", vars);
        bool look = (PPC_LOOKUP_FUNC(base, 0x82F422E8) == _xstart);
        log.Printf("PPC_LOOKUP_FUNC(base,0x82F422E8) == _xstart -> %s", look ? "true" : "false");
        return (img.mz && vars == 13 && look && img.funcTableEntries == n) ? 0 : 1;
    }
    uint32_t timeoutSec = 60;
    for (int i = 1; i < argc; ++i)
        if (!std::strcmp(argv[i], "--timeout") && i + 1 < argc) timeoutSec = (uint32_t)std::strtoul(argv[++i], nullptr, 0);
    rt::GuestThreadParams gp;
    gp.entry = 0x82F422E8;
    gp.name = "_xstart";
    rt::GuestThread* th = rt::GuestThread::Create(gp, &err);
    if (!th) { log.Printf("ERROR hilo: %s", err.c_str()); return 2; }
    if (!rt::gpu::Cfg().noD3d) {
        rt::gpu::d3d::Options o;
        auto& g = rt::gpu::Cfg();
        o.headless = g.headless; o.warp = g.warp; o.clearTest = g.clearTest; o.dumpFrames = g.dumpFrames; o.stats = g.d3dStats; o.dumpResolves = g.dumpResolves; o.dumpTextures = g.dumpTextures;
        if (!rt::gpu::d3d::Start(o)) log.Printf("D3D12 no disponible: se continua sin dibujo");
    }
    rt::gpu::Start();
    if (!perfOut.empty()) rt::perf::Start(perfOut.c_str());
    log.Printf("start _xstart 0x82F422E8");
    th->Start();
    th->Resume();
    if (!th->Join(timeoutSec * 1000)) {
        log.Printf("TIMEOUT %us: el hilo principal sigue en ejecucion; ultimo import=%s", timeoutSec, rt::ImportLog::LastImport());
        rt::perf::Stop();
        rt::GuestThread::DumpAllThreads();
        rt::DumpSyncTrace();
        rt::BinkProbeReport();
        rt::AudioSummary();
        if (rt::gpu::Cfg().shaderScan) rt::gpu::ScanShaderContainers("fin (timeout)");
        log.Printf("XMA resumen: paquetes_sinteticos=%llu backend=sin_decodificador",
                   static_cast<unsigned long long>(rt::audio::XmaSimulatedPacketCount()));
        rt::gpu::Stop();
        rt::gpu::Summary();
        rt::audio::StopXmaWorker();
        log.Summary();
        std::fflush(nullptr);
        ExitProcess(4);
    }
    rt::perf::Stop();
    rt::gpu::Stop();
    rt::audio::StopXmaWorker();
    rt::gpu::Summary();
    rt::DumpSyncTrace();
    log.Summary();
    return th->ExitCode();
}
