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

namespace rt::gpu { int Pm4SelfTest(); }
int RunCpuSelfTest(uint64_t n, uint64_t seed);   // selftest\cpu_selftest.cpp
namespace rt { extern bool g_noAudio; void AudioSummary(); void BinkProbeReport(); }
PPC_EXTERN_FUNC(__imp__XamInputGetState);
PPC_EXTERN_FUNC(__imp__XamInputGetCapabilities);

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

static bool LoadAll(std::string& err, rt::ImageLoadResult& img, rt::XexHeaderInfo& xex, int& vars) {
    if (!rt::LoadPaths(const_cast<rt::Paths&>(rt::GetPaths()), &err)) return false;
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
    bool selftest = false, iotest = false, inputSelfTest = false;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--max-imports") && i + 1 < argc) log.Config().maxImports = std::strtoull(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--log") && i + 1 < argc) logPath = argv[++i];
        else if (!std::strcmp(argv[i], "--log-all")) log.Config().logAll = true;
        else if (!std::strcmp(argv[i], "--selftest-load")) selftest = true;
        else if (!std::strcmp(argv[i], "--io-selftest")) iotest = true;
        else if (!std::strcmp(argv[i], "--selftest-input")) inputSelfTest = true;
        else if (!std::strcmp(argv[i], "--gpu-regdump")) rt::gpu::Cfg().regDump = true;
        else if (!std::strcmp(argv[i], "--gpu-log") && i + 1 < argc) rt::gpu::Cfg().logPath = argv[++i];
        else if (!std::strcmp(argv[i], "--gpu-statelog")) rt::gpu::Cfg().stateLog = true;
        else if (!std::strcmp(argv[i], "--dump-fb")) rt::gpu::Cfg().dumpFb = true;
        else if (!std::strcmp(argv[i], "--no-d3d")) rt::gpu::Cfg().noD3d = true;
        else if (!std::strcmp(argv[i], "--no-audio")) rt::g_noAudio = true;
        else if (!std::strcmp(argv[i], "--bink-trace")) rt::gpu::Cfg().binkTrace = true;
        else if (!std::strcmp(argv[i], "--headless")) rt::gpu::Cfg().headless = true;
        else if (!std::strcmp(argv[i], "--warp")) rt::gpu::Cfg().warp = true;
        else if (!std::strcmp(argv[i], "--d3d-clear-test")) rt::gpu::Cfg().clearTest = true;
        else if (!std::strcmp(argv[i], "--d3d-stats")) rt::gpu::Cfg().d3dStats = true;
        else if (!std::strcmp(argv[i], "--dump-textures") && i + 1 < argc) rt::gpu::Cfg().dumpTextures = (uint32_t)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--dump-resolves") && i + 1 < argc) rt::gpu::Cfg().dumpResolves = (uint32_t)std::strtoul(argv[++i], nullptr, 0);
        else if (!std::strcmp(argv[i], "--dump-frames") && i + 1 < argc) rt::gpu::Cfg().dumpFrames = argv[++i];
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

    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--selftest-gpu") && i + 1 < argc) {
            std::string e;
            if (!rt::LoadPaths(const_cast<rt::Paths&>(rt::GetPaths()), &e)) { std::printf("ERROR: %s\n", e.c_str()); return 2; }
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
    log.Printf("start _xstart 0x82F422E8");
    th->Start();
    th->Resume();
    if (!th->Join(timeoutSec * 1000)) {
        log.Printf("TIMEOUT %us: el hilo principal sigue en ejecucion; ultimo import=%s", timeoutSec, rt::ImportLog::LastImport());
        rt::GuestThread::DumpAllThreads();
        rt::BinkProbeReport();
        rt::AudioSummary();
        rt::gpu::ScanShaderContainers("fin (timeout)");
        rt::gpu::Stop();
        rt::gpu::Summary();
        log.Summary();
        std::fflush(nullptr);
        ExitProcess(4);
    }
    rt::gpu::Stop();
    rt::gpu::Summary();
    log.Summary();
    return th->ExitCode();
}
