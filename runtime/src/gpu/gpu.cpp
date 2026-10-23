#include "gpu.h"

#include <windows.h>
#include <mmsystem.h>

#include <algorithm>
#include <atomic>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <set>
#include <string>

#include "config.h"
#include "guest_memory.h"
#include "guest_thread.h"
#include "import_log.h"
#include "pm4.h"
#include "ucode_info.h"
#include "fb_decode.h"
namespace rt::gpu::d3d { extern bool g_gpuTiming; }
namespace rt::gpu { extern int g_syncReason; }   // M60 (d3d12_backend.cpp)
#include "gpu_names.h"
#include "image_io.h"
#include "backend.h"
#include "perf.h"
#include "xenia/gpu/xenos.h"

#pragma comment(lib, "winmm.lib")

namespace rt::gpu {

Config& Cfg() { static Config c; return c; }
std::string FramesDir() {
    std::string d = Cfg().framesDir.empty() ? GetPaths().userdataDir + "\\frames" : Cfg().framesDir;
    CreateDirectoryA(d.c_str(), nullptr);
    return d;
}

namespace backend {
static Backend* g_b = nullptr;
Backend* Get() { return g_b; }
void Set(Backend* b) { g_b = b; }
} // namespace backend

namespace {
void TraceSwapState(const char* tag, uint64_t n, std::atomic<uint64_t>& ctr);   // M60 (definida con VsyncMain)

constexpr uint32_t kGuestRegWords = 0x4000;       // 64 KB / 4
constexpr uint32_t REG_CP_RB_WPTR = 0x01C5;        // graphics_system.cc WriteRegister
constexpr uint32_t REG_D1GRPH_PRIMARY_SURFACE = 0x1844;

uint32_t g_regs[kRegCount];
uint8_t g_shadow[kGuestRegWords * 4];              // ultima copia vista de la pagina guest (big-endian)
std::mutex g_regMu;

std::atomic<uint32_t> g_wptr{0xBAADF00D};
std::atomic<uint32_t> g_rptr{0};
std::atomic<uint32_t> g_ringPhys{0}, g_ringBytes{0};
std::atomic<uint32_t> g_wbPhys{0}, g_wbBlock{0};
std::atomic<uint32_t> g_cb{0}, g_cbData{0};
std::atomic<uint64_t> g_frames{0}, g_vblank{0}, g_counter{0}, g_swapCalls{0}, g_scanouts{0}, g_batches{0};
std::atomic<bool> g_running{false};
HANDLE g_cpThread = nullptr, g_vsThread = nullptr;

FILE* g_gpuLog = nullptr;
std::mutex g_logMu;

uint8_t* GuestRegPage() { return GuestMemory::Base() + kGpuRegBase; }

void GpuLogLine(const char* s) {
    std::lock_guard<std::mutex> g(g_logMu);
    if (!g_gpuLog) g_gpuLog = std::fopen(Cfg().logPath.c_str(), "w");
    if (g_gpuLog) { std::fputs(s, g_gpuLog); std::fputc('\n', g_gpuLog); std::fflush(g_gpuLog); }
}

void SetGuestReg(uint32_t idx, uint32_t v) { // pagina guest + copia + fichero interno
    uint8_t* p = GuestRegPage() + idx * 4;
    uint32_t be = __builtin_bswap32(v);
    std::memcpy(p, &be, 4);
    std::memcpy(g_shadow + idx * 4, &be, 4);
    g_regs[idx] = v;
}

// Escrituras del guest en la pagina de registros -> fichero interno (el equivalente de WriteRegister de GraphicsSystem).
void SyncFromGuest() {
    std::lock_guard<std::mutex> g(g_regMu);
    const uint8_t* page = GuestRegPage();
    for (uint32_t blk = 0; blk < kGuestRegWords * 4; blk += 64) {
        if (!std::memcmp(page + blk, g_shadow + blk, 64)) continue;
        for (uint32_t off = blk; off < blk + 64; off += 4) {
            uint32_t be;
            std::memcpy(&be, page + off, 4);
            uint32_t oldbe;
            std::memcpy(&oldbe, g_shadow + off, 4);
            if (be == oldbe) continue;
            std::memcpy(g_shadow + off, &be, 4);
            uint32_t idx = off / 4, v = __builtin_bswap32(be);
            g_regs[idx] = v;
            if (Cfg().regDump) ImportLog::Get().Printf("GPUREG W idx=0x%X val=0x%08X", idx, v);
            if (idx == REG_CP_RB_WPTR) g_wptr = v;
            else if (idx == REG_D1GRPH_PRIMARY_SURFACE) {
                uint64_t n = ++g_scanouts;
                if (n <= 5 || n % 100 == 0) ImportLog::Get().Printf("SCANOUT %llu D1GRPH_PRIMARY_SURFACE_ADDRESS=0x%08X", (unsigned long long)n, v);
            }
        }
    }
}

thread_local GuestThread* t_ctx = nullptr;

void DispatchInterrupt(uint32_t source, uint32_t cpu) {
    uint32_t cb = g_cb;
    if (!cb) return;
    if (!t_ctx) {
        t_ctx = GuestThread::CreateAttached(source == 0 ? "gpu-vsync" : "gpu-cp");
        if (!t_ctx) return;
        t_ctx->Attach();
    }
    t_ctx->SetCpu(cpu == 0xFFFFFFFF ? 2 : cpu); // Xenia: cpu por defecto 2
    if (source == 1) M60Event('I');
    if (source == 1 && Cfg().swapInterval) {
        // M60 --swap-interval N: el manejador de interrupcion del CP del juego (sub_82F58198, fuente 1) llama a [blk+16] con r3 = [blk+20],
        // blk = [dev+11028], dev = userData del callback. Si [blk+16] es el envio de volteo (sub_82F5C258), r3 = frontbuffer | intervalo<<8 |
        // umbral (sub_82F5C258: r29 = (r3>>8)&0xF, r27 = r3&0xFF, r30 = r3&0xFFFFF000). Se sustituye el intervalo antes de la llamada.
        uint8_t* b = GuestMemory::Base();
        auto R = [&](uint32_t a) { return __builtin_bswap32(*reinterpret_cast<uint32_t*>(b + a)); };
        uint32_t dev = g_cbData;
        if (dev >= 0x40000000u && dev < 0xA0000000u) {
            uint32_t blk = R(dev + 11028);
            if (blk >= 0x40000000u && blk < 0xE0000000u && R(blk + 16) == 0x82F5C258u) {
                uint32_t v = R(blk + 20);
                uint32_t nv = (v & ~0xF00u) | (Cfg().swapInterval << 8);
                if (nv != v) GuestMemory::Write32(blk + 20, nv);
                if (rt::gpu::d3d::g_gpuTiming) {   // M60 diagnostico: volteo tardio = el vencimiento ideal (ultimo + intervalo) ya paso
                    static uint32_t tot = 0, late = 0; static uint64_t lastV = 0;
                    uint32_t lastDue = R(dev + 16960), cur = R(dev + 16952);
                    ++tot;
                    if (lastDue + Cfg().swapInterval <= cur) { ++late; static int nd = 0; if (nd < 30 && g_frames.load() > 1600) { ++nd; M60DumpEvents("volteo tardio"); } }
                    uint64_t vb = g_vblank.load();
                    if (vb - lastV >= 60) { ImportLog::Get().Printf("M60 volteos tardios=%u de %u", late, tot); tot = late = 0; lastV = vb; }
                }
                static uint64_t n = 0;
                if (++n <= 3) ImportLog::Get().Printf("M60 swap-interval: argumento de volteo %08X -> %08X", v, nv);
            }
        }
    }
    t_ctx->CallGuest(cb, source, g_cbData);
}

// ---- H0: estado observable por draw, histogramas, bitmap de registros sucios ----
struct ActiveShader { uint64_t hash = 0; std::shared_ptr<ShaderInfo> info; uint32_t addr = 0; };

std::map<std::string, uint64_t> g_hist;           // clave "TEXFMT ..." -> recuento (se vuelca al final)
std::mutex g_histMu;
uint32_t g_dirty[(kRegCount + 31) / 32];
uint64_t g_drawsSeen = 0, g_dirtyTotal = 0, g_dirtyMax = 0, g_drawsNoVs = 0, g_drawsNoPs = 0;
uint64_t g_shaderParseFail = 0;
std::unordered_map<uint64_t, std::shared_ptr<ShaderInfo>> g_shaderStore;
std::set<uint32_t> g_fbPngFrames;

void Hist(const std::string& k) { std::lock_guard<std::mutex> g(g_histMu); ++g_hist[k]; }

struct CpHost : Pm4Host {
    ActiveShader active[2];
    uint64_t pollSpins = 0;
    void WriteReg(uint32_t idx, uint32_t v) override {
        if (idx >= kRegCount) return;
        std::lock_guard<std::mutex> g(g_regMu);
        if (g_regs[idx] != v) g_dirty[idx >> 5] |= 1u << (idx & 31);
        if (idx < kGuestRegWords) SetGuestReg(idx, v); else g_regs[idx] = v;
        if (idx >= 0x0578 && idx <= 0x057F) { // SCRATCH_REG0..7: escritura a memoria si UMSK lo habilita
            uint32_t s = idx - 0x0578;
            if ((1u << s) & g_regs[0x01DC]) {
                uint32_t addr = g_regs[0x01DD] + s * 4;
                if (uint8_t* p = PhysHost(addr, 4)) { uint32_t be = __builtin_bswap32(v); std::memcpy(p, &be, 4); }
            }
        }
    }
    uint32_t ReadReg(uint32_t idx) override { return idx < kRegCount ? g_regs[idx] : 0; }
    uint8_t* Phys(uint32_t phys, uint32_t bytes) override { return PhysHost(phys, bytes ? bytes : 1); }
    void Interrupt(uint32_t mask) override {
        for (int n = 0; n < 6; ++n) if (mask & (1u << n)) DispatchInterrupt(1, n);
    }
    void Swap(uint32_t fb, uint32_t w, uint32_t h) override {
        uint64_t n = ++g_frames;
        ++g_counter;
        perf::OnFrame(n);
        ImportLog::Get().Printf("FRAME %llu fb=%08X %ux%u", (unsigned long long)n, fb, w, h);
        if (Cfg().traceSwap) { static std::atomic<uint64_t> ctrSw{0}; TraceSwapState("SWAP", n, ctrSw); }
        M60Event('S');
        if (Cfg().dumpFb) DumpFrontbuffer(n, fb, w, h);
        if (backend::Get()) {
            backend::SwapInfo si;
            si.frame = n; si.fbPhys = fb; si.w = w; si.h = h;
            for (int i = 0; i < 6; ++i) si.fetch[i] = g_regs[0x4800 + i];
            backend::Get()->OnSwap(si);
        }
    }
    void DumpFrontbuffer(uint64_t n, uint32_t fb, uint32_t w, uint32_t h) {
        FbDesc d;
        for (int i = 0; i < 6; ++i) d.fetch[i] = g_regs[0x4800 + i];
        d.phys = fb; d.w = w; d.h = h;
        std::vector<uint8_t> rgba;
        std::string info;
        char b[512];
        if (!DecodeFrontbuffer(d, rgba, &info)) {
            std::snprintf(b, sizeof(b), "FBDUMP frame=%llu fb=%08X %ux%u decode=FAIL %s", (unsigned long long)n, fb, w, h, info.c_str());
            GpuLogLine(b);
            return;
        }
        ImageStats s = ComputeStats(w, h, rgba.data(), w * 4);
        std::snprintf(b, sizeof(b), "FBDUMP frame=%llu fb=%08X %ux%u %s nonblack_px=%llu hash=%016llX",
                      (unsigned long long)n, fb, w, h, info.c_str(), (unsigned long long)s.nonblack, (unsigned long long)s.hash);
        GpuLogLine(b);
        if (g_fbPngFrames.count((uint32_t)n)) {
            char name[64];
            std::snprintf(name, sizeof(name), "\\fb_%05llu.png", (unsigned long long)n);
            std::string path = rt::gpu::FramesDir() + name;
            bool ok = WritePngRGBA(path, w, h, rgba.data(), w * 4);
            ImportLog::Get().Printf("FBDUMP png %s -> %s nonblack_px=%llu", path.c_str(), ok ? "OK" : "FALLO", (unsigned long long)s.nonblack);
        }
    }
    uint64_t Counter() override { return g_counter; }
    bool Poll() override {
        SyncFromGuest();
        if (!g_running) return false;
        if ((++pollSpins & 0xFF) == 0) Sleep(1); else SwitchToThread();
        return true;
    }
    void OnShader(uint32_t type, const uint8_t* bytes, uint32_t dwords, uint32_t addr, bool mapped) override {
        if (!mapped || !bytes || !dwords || type > 1) return;
        uint64_t h = HashUcode(bytes, dwords, type);
        auto it = g_shaderStore.find(h);
        if (it == g_shaderStore.end()) {
            auto info = std::make_shared<ShaderInfo>();
            if (!AnalyzeUcode(type == 0, bytes, dwords, *info)) {
                ++g_shaderParseFail;
                ImportLog::Get().Printf("SHADER-ANALYZE FALLO type=%s dwords=%u: %s", type == 0 ? "vs" : "ps", dwords, info->error.c_str());
            }
            it = g_shaderStore.emplace(h, info).first;
            std::string dir = GetPaths().userdataDir + "\\shaders";
            CreateDirectoryA(GetPaths().userdataDir.c_str(), nullptr);
            CreateDirectoryA(dir.c_str(), nullptr);
            char name[64];
            std::snprintf(name, sizeof(name), "\\ucode_%016llX_%s.bin", (unsigned long long)h, type == 0 ? "vs" : "ps");
            FILE* f = std::fopen((dir + name).c_str(), "wb");
            if (f) { std::fwrite(bytes, 1, dwords * 4, f); std::fclose(f); }
            ImportLog::Get().Printf("SHADER-DUMP ucode %s dwords=%u -> %s", type == 0 ? "vs" : "ps", dwords, name + 1);
        }
        active[type].hash = h; active[type].info = it->second; active[type].addr = addr;
    }
    void OnDraw(const DrawEvent& ev) override {
        ++g_drawsSeen;
        uint64_t nd = 0;
        {
            std::lock_guard<std::mutex> g(g_regMu);
            for (uint32_t i = 0; i < sizeof(g_dirty) / 4; ++i) { nd += __builtin_popcount(g_dirty[i]); g_dirty[i] = 0; }
        }
        g_dirtyTotal += nd; if (nd > g_dirtyMax) g_dirtyMax = nd;
        uint32_t surf = g_regs[0x2000], color = g_regs[0x2001], depth = g_regs[0x2002], mode = g_regs[0x2208], copy = g_regs[0x2318];
        const uint64_t frame = g_frames.load() + 1;
        const bool stateLogThis = Cfg().stateLog && (!Cfg().stateLogFrame || frame == Cfg().stateLogFrame);
        if (stateLogThis) {
            char b[512];
            std::snprintf(b, sizeof(b), "DRAWSTATE frame=%llu draw=%llu vs=%016llX ps=%016llX surf=%08X color=%08X depth=%08X mode=%u copy=%08X prim=%u n=%u src=%u dirty=%llu cm=%08X zc=%08X blend=%08X,%08X,%08X,%08X",
                          (unsigned long long)frame, (unsigned long long)g_drawsSeen,
                          (unsigned long long)active[0].hash, (unsigned long long)active[1].hash,
                          surf, color, depth, mode & 7, copy, ev.prim, ev.nIdx, ev.src, (unsigned long long)nd,
                          g_regs[0x2104], g_regs[0x2200], g_regs[0x2201], g_regs[0x2209], g_regs[0x220A], g_regs[0x220B]);
            GpuLogLine(b);
            for (int stage = 0; stage < 2; ++stage) {
                if (!active[stage].info) continue;
                const ShaderInfo& si = *active[stage].info;
                const char* stageName = stage == 0 ? "VS" : "PS";
                std::set<uint32_t> seenVfetch;
                for (const VFetchUse& v : si.vfetch) {
                    if (!seenVfetch.insert(v.constIdx).second) continue;
                    uint32_t grp = v.constIdx / 3, slot = v.constIdx % 3;
                    char fetch[192];
                    std::snprintf(fetch, sizeof(fetch), "DRAWFETCH frame=%llu draw=%llu stage=%s kind=vertex idx=%u words=%08X,%08X fmt=%u stride=%u",
                                  (unsigned long long)frame, (unsigned long long)g_drawsSeen, stageName, v.constIdx,
                                  g_regs[0x4800 + grp * 6 + slot * 2], g_regs[0x4800 + grp * 6 + slot * 2 + 1], v.format, v.stride);
                    GpuLogLine(fetch);
                }
                for (uint32_t i = 0; i < 32; ++i) {
                    if (!(si.tfetchMask & (1u << i))) continue;
                    char fetch[256];
                    const uint32_t r = 0x4800 + i * 6;
                    std::snprintf(fetch, sizeof(fetch), "DRAWFETCH frame=%llu draw=%llu stage=%s kind=texture idx=%u words=%08X,%08X,%08X,%08X,%08X,%08X",
                                  (unsigned long long)frame, (unsigned long long)g_drawsSeen, stageName, i,
                                  g_regs[r], g_regs[r + 1], g_regs[r + 2], g_regs[r + 3], g_regs[r + 4], g_regs[r + 5]);
                    GpuLogLine(fetch);
                }
            }
        }
        if (!active[0].info) ++g_drawsNoVs;
        if (!active[1].info) ++g_drawsNoPs;
        if (!(Cfg().hist || stateLogThis)) {   // M8: diagnostico H0 bajo demanda (snprintf + std::map por draw)
            if (backend::Get()) backend::Get()->OnDraw(ev, active[0].info.get(), active[1].info.get());
            return;
        }
        char k[256];
        std::snprintf(k, sizeof(k), "EDRAMMODE mode=%u(%s)", mode & 7, EdramModeName(mode & 7)); Hist(k);
        std::snprintf(k, sizeof(k), "MSAA %s", MsaaName((surf >> 16) & 3)); Hist(k);
        std::snprintf(k, sizeof(k), "PRIM %u(%s) src=%u", ev.prim, PrimName(ev.prim), ev.src); Hist(k);
        std::snprintf(k, sizeof(k), "RTFMT color0=%s depth=%s mode=%u", ColorRtName((color >> 16) & 15), DepthRtName((depth >> 16) & 1), mode & 7); Hist(k);
        std::snprintf(k, sizeof(k), "SQCONST vs=%08X ps=%08X", g_regs[0x2307], g_regs[0x2308]); Hist(k);
        // solo las constantes de fetch referenciadas por el microcodigo activo
        for (int t = 0; t < 2; ++t) {
            if (!active[t].info) continue;
            const ShaderInfo& si = *active[t].info;
            const char* tn = t == 0 ? "VS" : "PS";
            std::set<uint64_t> seen;
            for (const VFetchUse& v : si.vfetch) {
                if (!seen.insert(((uint64_t)v.constIdx << 32) | v.format).second) continue;
                uint32_t grp = v.constIdx / 3, slot = v.constIdx % 3;
                uint32_t d0 = g_regs[0x4800 + grp * 6 + slot * 2], d1 = g_regs[0x4800 + grp * 6 + slot * 2 + 1];
                std::snprintf(k, sizeof(k), "VTXFMT %s fmt=%s endian=%u fetchtype=%u stride=%u", tn, VtxFmtName(v.format), d1 & 3, d0 & 3, v.stride);
                Hist(k);
            }
            for (uint32_t i = 0; i < 32; ++i) {
                if (!(si.tfetchMask & (1u << i))) continue;
                xe::gpu::xenos::xe_gpu_texture_fetch_t tf;
                std::memcpy(&tf, &g_regs[0x4800 + i * 6], sizeof(tf));
                if ((uint32_t)tf.type != 2) { // la ranura contiene otro tipo de constante (p. ej. de vertice): no es una textura valida
                    std::snprintf(k, sizeof(k), "TEXFMT %s INVALIDA fetchtype=%u (constante de fetch sin textura valida)", tn, (unsigned)tf.type);
                    Hist(k);
                    continue;
                }
                std::snprintf(k, sizeof(k), "TEXFMT %s fmt=%s dim=%s tiled=%u endian=%u fetchtype=%u sign=%u%u%u%u packed_mips=%u",
                              tn, TexFmtName((uint32_t)tf.format), DimName((uint32_t)tf.dimension), (unsigned)tf.tiled, (unsigned)tf.endianness,
                              (unsigned)tf.type, (unsigned)tf.sign_x, (unsigned)tf.sign_y, (unsigned)tf.sign_z, (unsigned)tf.sign_w, (unsigned)tf.packed_mips);
                Hist(k);
            }
            if (si.memexportAddr) { std::snprintf(k, sizeof(k), "MEMEXPORT %s con memexport (eM mask=%02X)", tn, si.memexportData); Hist(k); }
        }
        if (backend::Get()) backend::Get()->OnDraw(ev, active[0].info.get(), active[1].info.get());
    }
    void Log(const char* line) override { GpuLogLine(line); }
    void MemWritten(uint32_t a, uint32_t n) override { if (backend::Get()) backend::Get()->MemWritten(a, n); }
    bool WriteOrdered(uint32_t a, const void* d, uint32_t n) override { return backend::Get() && backend::Get()->WriteOrdered(a, d, n); }   // M60
    void ReadBarrier(uint32_t a, uint32_t n) override { if (backend::Get()) backend::Get()->ReadBarrier(a, n); }                         // M60
    void SyncPoint() override { if (backend::Get()) backend::Get()->SyncPoint(); }
    void FrameDone(const char* s) override { GpuLogLine(s); }
};

// Ejecucion protegida: una violacion de acceso dentro del analizador detiene el lote, no el proceso.
bool SafeExecute(Pm4Parser* p, const uint8_t* ring, uint32_t cap, uint32_t rd, uint32_t wr) {
    __try {
        return p->ExecuteRing(ring, cap, rd, wr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ImportLog::Get().Printf("EXCEPCION en el procesador de comandos rd=%u wr=%u", rd, wr);
        return false;
    }
}

DWORD WINAPI CpMain(void*) {
    perf::NameCurrentThread("gpu-cp");
    CpHost host;
    Pm4Parser parser(&host);
    parser.SetLogLimit(Cfg().logPerFrame);
    // M60 (--gpu-timing): tiempo del CP sin trabajo (anillo vacio) y en lotes, por segundo
    LARGE_INTEGER qf, tPrint, t0, t1; QueryPerformanceFrequency(&qf); QueryPerformanceCounter(&tPrint);
    double idleS = 0, batchS = 0; uint64_t idleN = 0;
    auto statTick = [&]() {
        if (!rt::gpu::d3d::g_gpuTiming) return;
        LARGE_INTEGER n; QueryPerformanceCounter(&n);
        double el = (double)(n.QuadPart - tPrint.QuadPart) / (double)qf.QuadPart;
        if (el < 1.0) return;
        ImportLog::Get().Printf("M60 CP anillo_vacio_ms=%.1f (esperas=%llu, media_us=%.0f) lotes_ms=%.1f en %.2f s", idleS * 1000, (unsigned long long)idleN,
                                idleN ? idleS * 1e6 / (double)idleN : 0.0, batchS * 1000, el);
        idleS = batchS = 0; idleN = 0; tPrint = n;
    };
    while (g_running) {
        SyncFromGuest();
        uint32_t w = g_wptr, ringBytes = g_ringBytes, ringPhys = g_ringPhys;
        if (!ringBytes || w == 0xBAADF00D || w == g_rptr) {
            QueryPerformanceCounter(&t0); Sleep(1); QueryPerformanceCounter(&t1);
            idleS += (double)(t1.QuadPart - t0.QuadPart) / (double)qf.QuadPart; ++idleN; statTick();
            continue;
        }
        QueryPerformanceCounter(&t0);
        M60Event('L');
        uint8_t* ring = PhysHost(ringPhys, ringBytes);
        if (!ring) { ImportLog::Get().Printf("RB: anillo %08X+%X no asignado", ringPhys, ringBytes); Sleep(100); continue; }
        uint32_t capD = ringBytes / 4;
        if (w >= capD) { ImportLog::Get().Printf("RB: wptr=0x%X fuera del anillo (%u dwords), se usa modulo", w, capD); }
        uint32_t rd = g_rptr;
        SafeExecute(&parser, ring, ringBytes, rd, w);
        g_syncReason = 9;   // M60 diagnostico
        if (backend::Get()) backend::Get()->SyncPoint();   // M8: antes de publicar el rptr (la CPU puede observar el fin del lote)
        uint32_t newR = w % capD;
        g_rptr = newR;
        uint32_t wb = g_wbPhys;
        if (wb) {
            if (uint8_t* p = PhysHost(wb, 4)) { uint32_t be = __builtin_bswap32(newR); std::memcpy(p, &be, 4); }
        }
        QueryPerformanceCounter(&t1); batchS += (double)(t1.QuadPart - t0.QuadPart) / (double)qf.QuadPart; statTick();
        uint64_t b = ++g_batches;
        if (b <= 40 || b % 500 == 0) ImportLog::Get().Printf("RB wptr=%u rptr=%u (lote %llu, desde rptr=%u)", w, newR, (unsigned long long)b, rd);
    }
    parser.FlushPartial();
    return 0;
}

// M60: estado de la cola de volteo del juego (dispositivo D3D guest). Direcciones de ppc_gen/sub_82F5C258, sub_82F5C128.
// dev = *(*(0x8200088C)) (rama r11=2188). Solo diagnostico (lectura de memoria guest desde el hilo de VBLANK).
void TraceSwapState(const char* tag, uint64_t n, std::atomic<uint64_t>& ctr) {
    uint64_t c = ++ctr;
    if (c > 400 && (c % 50) != 0) return;
    uint8_t* b = GuestMemory::Base();
    auto R = [&](uint32_t a) { return __builtin_bswap32(*reinterpret_cast<uint32_t*>(b + a)); };
    // El manejador VBLANK recibe como r3 el userData del callback (VdSetGraphicsInterruptCallback): ese valor es el dispositivo.
    uint32_t dev = g_cbData;
    if (dev < 0x40000000u || dev >= 0xA0000000u) { ImportLog::Get().Printf("SWT %s n=%llu dev=%08X (no valido)", tag, (unsigned long long)n, dev); return; }
    ImportLog::Get().Printf("SWT %s n=%llu vbg=%u gst(16952)=%u last_due(16960)=%u sub(16968)=%u sub2(16972)=%u flip(16976)=%u rd(17108)=%u wr(17112)=%u q0=(%u,%u) q1=(%u,%u) q2=(%u,%u)",
        tag, (unsigned long long)n, (unsigned)g_vblank.load(), R(dev + 16952), R(dev + 16960), R(dev + 16968), R(dev + 16972), R(dev + 16976),
        R(dev + 17108), R(dev + 17112), R(dev + 16980), R(dev + 16984), R(dev + 16988), R(dev + 16992), R(dev + 16996), R(dev + 17000));
}


DWORD WINAPI VsyncMain(void*) {
    perf::NameCurrentThread("gpu-vsync");
    LARGE_INTEGER f, last, now;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&last);
    const LONGLONG period = f.QuadPart / (Cfg().vblankHz ? Cfg().vblankHz : 60);
    // M60: espera hasta el siguiente vencimiento con temporizador de alta resolucion (antes Sleep(1): intervalos medidos 15,2-18,0 ms)
    HANDLE vtm = Cfg().legacyVsync ? nullptr : CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    while (g_running) {
        QueryPerformanceCounter(&now);
        if (vtm && now.QuadPart - last.QuadPart < period) {
            LONGLONG rem = period - (now.QuadPart - last.QuadPart);            // en ticks de QPC
            LARGE_INTEGER due; due.QuadPart = -(rem * 10000000LL / f.QuadPart);  // 100 ns, relativo
            if (due.QuadPart < 0 && SetWaitableTimer(vtm, &due, 0, nullptr, nullptr, FALSE)) WaitForSingleObject(vtm, 100);
            QueryPerformanceCounter(&now);
        }
        if (now.QuadPart - last.QuadPart >= period) {
            last.QuadPart += period;
            if (now.QuadPart - last.QuadPart > period * 4) last = now; // evita rafagas tras una pausa
            ++g_vblank;
            ++g_counter;               // Xenia: command_processor_->increment_counter()
            if (rt::gpu::d3d::g_gpuTiming) {   // M60 diagnostico: regularidad del VBLANK (intervalo real entre interrupciones)
                static LARGE_INTEGER prev{}; static double mn = 1e9, mx = 0; static uint32_t cnt = 0, early = 0, late = 0;
                if (prev.QuadPart) {
                    double ms = (double)(now.QuadPart - prev.QuadPart) * 1000.0 / (double)f.QuadPart;
                    mn = std::min(mn, ms); mx = std::max(mx, ms); ++cnt;
                    if (ms < 0.5 * 1000.0 / (Cfg().vblankHz ? Cfg().vblankHz : 60)) ++early;
                    if (ms > 1.5 * 1000.0 / (Cfg().vblankHz ? Cfg().vblankHz : 60)) ++late;
                    if (cnt >= (Cfg().vblankHz ? Cfg().vblankHz : 60)) {
                        ImportLog::Get().Printf("M60 VBLANK intervalo_ms min=%.2f max=%.2f tempranos=%u tardios=%u (de %u) esperas_marca=%llu (%.2f ms)", mn, mx, early, late, cnt,
                                                (unsigned long long)g_vbWaits.exchange(0), (double)g_vbWaitTicks.exchange(0) * 1000.0 / (double)f.QuadPart);
                        mn = 1e9; mx = 0; cnt = early = late = 0;
                    }
                }
                prev = now;
            }
            if (Cfg().traceSwap) { static std::atomic<uint64_t> ctrVb{0}; TraceSwapState("VB", g_vblank, ctrVb); }
            // M8: el barrido de contenedores de shaders (fase 4a, diagnostico) bloqueaba este hilo ~1 s a los 10 s y 30 s y, con el,
            // las interrupciones de VBLANK que el juego usa para voltear: ahora solo con --shader-scan.
            if (Cfg().shaderScan && (g_vblank == 600 || g_vblank == 1800)) ScanShaderContainers(g_vblank == 600 ? "10 s" : "30 s");
            M60Event('B', (uint32_t)g_vblank.load());
            if (g_swapMarkDone.load() < g_swapMarkQueued.load()) {   // M60: fotograma aun no completo en memoria guest
                LARGE_INTEGER w0, w1; QueryPerformanceCounter(&w0);
                while (g_running && g_swapMarkDone.load() < g_swapMarkQueued.load()) {
                    QueryPerformanceCounter(&w1);
                    if (w1.QuadPart - w0.QuadPart > f.QuadPart / 5) { static int nw = 0; if (nw++ < 5) ImportLog::Get().Printf("M60 VBLANK: espera de marca de XE_SWAP > 200 ms; se continua"); break; }
                    SwitchToThread();
                }
                QueryPerformanceCounter(&w1);
                g_vbWaitTicks += (uint64_t)(w1.QuadPart - w0.QuadPart); ++g_vbWaits;
                M60Event('b');
            }
            DispatchInterrupt(0, 2);   // Xenia: DispatchInterruptCallback(0, 2)
        }
        if (!vtm) Sleep(1);
    }
    if (vtm) CloseHandle(vtm);
    return 0;
}

} // namespace


// ---- Prueba de extremo a extremo del dibujo (H3/H4): rectangulo de limpieza + resolve + swap con shaders reales del juego ----
namespace {
struct PacketBuilder {
    std::vector<uint8_t> bytes;   // dwords big-endian, como en el anillo del guest
    void Dw(uint32_t v) { uint32_t be = __builtin_bswap32(v); const uint8_t* p = reinterpret_cast<const uint8_t*>(&be); bytes.insert(bytes.end(), p, p + 4); }
    void Raw(const std::vector<uint8_t>& b) { bytes.insert(bytes.end(), b.begin(), b.end()); }
    void Regs(uint32_t base, std::initializer_list<uint32_t> v) { Dw(((uint32_t)(v.size() - 1) << 16) | base); for (uint32_t x : v) Dw(x); }
    void Reg(uint32_t idx, uint32_t v) { Regs(idx, {v}); }
    void Type3(uint32_t op, uint32_t count) { Dw((3u << 30) | ((count - 1) << 16) | (op << 8)); }
};
bool LoadFile(const std::string& p, std::vector<uint8_t>& out) {
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return false;
    std::fseek(f, 0, SEEK_END); long n = std::ftell(f); std::fseek(f, 0, SEEK_SET);
    out.resize(n > 0 ? n : 0);
    bool ok = n > 0 && std::fread(out.data(), 1, n, f) == (size_t)n;
    std::fclose(f);
    return ok;
}
void PutBeFloat(uint8_t* p, float f) { uint32_t v; std::memcpy(&v, &f, 4); v = __builtin_bswap32(v); std::memcpy(p, &v, 4); }
} // namespace

int SelfTestRender(const std::string& shaderDir, uint32_t cr, uint32_t cg, uint32_t cb) {
    std::vector<uint8_t> vsCode, psCode;
    if (!LoadFile(shaderDir + "/ucode_F839F5C6ABF9A216_vs.bin", vsCode) || !LoadFile(shaderDir + "/ucode_7D64CA4C3C5FF6CC_ps.bin", psCode)) {
        std::printf("selftest-gpu: faltan los ucode del directorio %s\n", shaderDir.c_str());
        return 2;
    }
    // memoria fisica del guest: vertices del rectangulo (VB1), vertices del resolve (VB2) y frontbuffer
    constexpr uint32_t kVb1 = 0x01000000, kVb2 = 0x01001000, kFb = 0x02000000;
    uint8_t* vb1 = PhysHost(kVb1, 256);
    uint8_t* vb2 = PhysHost(kVb2, 256);
    uint8_t* fbm = PhysHost(kFb, 1280 * 736 * 4);
    if (!vb1 || !vb2 || !fbm) { std::printf("selftest-gpu: memoria fisica no disponible\n"); return 2; }
    std::memset(fbm, 0, 1280 * 736 * 4);
    float r = (float)cr / 255.0f, g = (float)cg / 255.0f, b = (float)cb / 255.0f;
    const float verts[3][3] = {{0, 0, 0}, {1280, 0, 0}, {0, 720, 0}};
    for (int i = 0; i < 3; ++i) {
        float* dummy = nullptr; (void)dummy;
        uint8_t* p = vb1 + i * 28;
        PutBeFloat(p, verts[i][0]); PutBeFloat(p + 4, verts[i][1]); PutBeFloat(p + 8, verts[i][2]);
        PutBeFloat(p + 12, r); PutBeFloat(p + 16, g); PutBeFloat(p + 20, b); PutBeFloat(p + 24, 1.0f);
    }
    const float rv[6] = {0, 0, 1280, 0, 0, 720};
    for (int i = 0; i < 6; ++i) PutBeFloat(vb2 + i * 4, rv[i]);

    PacketBuilder pb;
    // estado comun (equivalente a lo que fija el controlador D3D9 del guest)
    pb.Reg(0x2000, 1280);                                   // RB_SURFACE_INFO: pitch 1280, 1X
    pb.Reg(0x2001, 0);                                      // RB_COLOR_INFO: base 0, k_8_8_8_8
    pb.Reg(0x2002, 0);                                      // RB_DEPTH_INFO
    pb.Reg(0x2104, 0xF);                                    // RB_COLOR_MASK
    pb.Reg(0x2201, 0x00010001);                             // RB_BLENDCONTROL0: sin mezcla (src=1, dst=0)
    pb.Reg(0x2200, 0);                                      // RB_DEPTHCONTROL
    pb.Reg(0x2202, 0);                                      // RB_COLORCONTROL
    pb.Reg(0x2204, 1u << 16);                               // PA_CL_CLIP_CNTL: clip_disable
    pb.Reg(0x2205, 0);                                      // PA_SU_SC_MODE_CNTL
    pb.Reg(0x2206, 0);                                      // PA_CL_VTE_CNTL
    pb.Reg(0x200E, 0); pb.Reg(0x200F, (8192u << 16) | 8192u);   // PA_SC_SCREEN_SCISSOR
    pb.Reg(0x2080, 0);                                      // PA_SC_WINDOW_OFFSET
    pb.Reg(0x2081, 1u << 31); pb.Reg(0x2082, (720u << 16) | 1280u);   // PA_SC_WINDOW_SCISSOR (window_offset_disable)
    pb.Reg(0x2307, 0x000FF000); pb.Reg(0x2308, 0x000FF100); // SQ_VS_CONST / SQ_PS_CONST
    // constante de fetch de vertice 0 -> VB1 (21 dwords, endian 8in32)
    pb.Regs(0x4800, {(kVb1 & ~3u) | 3u, (21u << 2) | 2u});
    for (int t = 0; t < 2; ++t) {   // shaders
        const std::vector<uint8_t>& code = t == 0 ? vsCode : psCode;
        pb.Type3(0x2B, 2 + (uint32_t)(code.size() / 4));
        pb.Dw((uint32_t)t); pb.Dw((uint32_t)(code.size() / 4));
        pb.Raw(code);
    }
    pb.Reg(0x2208, 4);                                      // RB_MODECONTROL: kColorDepth
    pb.Type3(0x36, 1); pb.Dw(8u | (2u << 6) | (3u << 16));  // DRAW_INDX_2: lista de rectangulos, 3 indices, autoindice
    // resolve a frontbuffer: region (0,0)-(1280,720) tomada de los vertices de vf0
    pb.Regs(0x4800, {(kVb2 & ~3u) | 3u, (6u << 2) | 2u});
    pb.Reg(0x2208, 6);                                      // RB_MODECONTROL: kCopy
    pb.Reg(0x2318, 1u << 20);                               // RB_COPY_CONTROL: copy_command = kConvert, fuente = color 0
    pb.Reg(0x2319, kFb);                                    // RB_COPY_DEST_BASE
    pb.Reg(0x231A, 1280u | (720u << 16));                   // RB_COPY_DEST_PITCH
    pb.Reg(0x231B, 2u | (6u << 7) | (1u << 24));            // RB_COPY_DEST_INFO: endian 8in32, k_8_8_8_8, swap R/B
    pb.Type3(0x36, 1); pb.Dw(8u | (2u << 6) | (3u << 16));
    // constante de fetch 0 = textura del frontbuffer (como VdSwap): k_8_8_8_8 tiled, 1280x720, endian 8in32, swizzle A0A
    xe::gpu::xenos::xe_gpu_texture_fetch_t tf;
    std::memset(&tf, 0, sizeof(tf));
    tf.type = xe::gpu::xenos::FetchConstantType::kTexture;
    tf.pitch = 1280 / 32; tf.tiled = 1;
    tf.format = xe::gpu::xenos::TextureFormat::k_8_8_8_8;
    tf.endianness = xe::gpu::xenos::Endian::k8in32;
    tf.base_address = kFb >> 12;
    tf.size_2d.width = 1279; tf.size_2d.height = 719;
    tf.swizzle = 0xA0A;
    pb.Regs(0x4800, {tf.dword_0, tf.dword_1, tf.dword_2, tf.dword_3, tf.dword_4, tf.dword_5});
    pb.Type3(0x64, 4); pb.Dw(0x53574150); pb.Dw(kFb); pb.Dw(1280); pb.Dw(720);

    CpHost host;
    Pm4Parser parser(&host);
    parser.SetLogLimit(Cfg().logPerFrame);
    uint32_t used = (uint32_t)pb.bytes.size();
    pb.bytes.resize(used * 2, 0);   // anillo con holgura: wr debe ser menor que la capacidad (los dwords a cero son paquetes nulos)
    bool ok = SafeExecute(&parser, pb.bytes.data(), (uint32_t)pb.bytes.size(), 0, used / 4);
    parser.FlushPartial();
    std::printf("selftest-gpu: color=(%u,%u,%u) PM4 ejecutado=%d dwords=%zu frames=%llu\n", cr, cg, cb, (int)ok, pb.bytes.size() / 4, (unsigned long long)g_frames.load());
    return ok && g_frames.load() == 1 ? 0 : 1;
}

void Start() {
    if (g_running) return;
    std::memset(g_regs, 0, sizeof(g_regs));
    std::memset(g_shadow, 0, sizeof(g_shadow));
    // Valores de lectura de GraphicsSystem::ReadRegister (Xenia).
    SetGuestReg(0x0F00, 0x08100748);  // RB_EDRAM_TIMING
    SetGuestReg(0x0F01, 0x0000200E);  // RB_BC_CONTROL
    SetGuestReg(0x194C, 0x000002D0);  // R500_D1MODE_V_COUNTER
    SetGuestReg(0x1951, 1);           // estado de interrupcion (vblank)
    SetGuestReg(0x1961, 0x050002D0);  // AVIVO_D1MODE_VIEWPORT_SIZE (1280x720)
    {
        const char* s = Cfg().fbFrames.c_str();
        while (*s) { char* e; unsigned long v = std::strtoul(s, &e, 10); if (e == s) break; g_fbPngFrames.insert((uint32_t)v); s = *e ? e + 1 : e; }
    }
    if (Cfg().noThreads) return;
    timeBeginPeriod(1);
    g_running = true;
    g_cpThread = CreateThread(nullptr, 0, CpMain, nullptr, 0, nullptr);
    g_vsThread = CreateThread(nullptr, 0, VsyncMain, nullptr, 0, nullptr);
    ImportLog::Get().Printf("GPU: hilos CP y vsync iniciados (registros en %08X-%08X)", kGpuRegBase, kGpuRegBase + kGpuRegSize - 1);
}

void Stop() {
    g_running = false;
    if (g_cpThread) { WaitForSingleObject(g_cpThread, 2000); g_cpThread = nullptr; }
    if (g_vsThread) { WaitForSingleObject(g_vsThread, 2000); g_vsThread = nullptr; }
    if (backend::Get()) backend::Get()->Shutdown();
}

void GpuLog(const char* line) { GpuLogLine(line); }

void SetInterruptCallback(uint32_t cb, uint32_t data) {
    g_cbData = data;
    g_cb = cb;
    ImportLog::Get().Printf("GPU: SetInterruptCallback(%08X, %08X)", cb, data);
}

void InitializeRingBuffer(uint32_t phys, uint32_t log2) {
    g_rptr = 0;
    g_ringPhys = phys & 0x1FFFFFFF;
    g_ringBytes = 1u << (log2 + 3); // Xenia: primary_buffer_size_ = 1 << (size_log2 + 3)
    ImportLog::Get().Printf("GPU: InitializeRingBuffer phys=%08X bytes=0x%X", phys, g_ringBytes.load());
}

void EnableReadPointerWriteBack(uint32_t phys, uint32_t blockLog2) {
    g_wbPhys = phys & 0x1FFFFFFF;
    g_wbBlock = blockLog2;
    ImportLog::Get().Printf("GPU: EnableReadPointerWriteBack phys=%08X block_log2=%u", phys, blockLog2);
}

void RecordSwapCall() { ++g_swapCalls; M60Event('V'); }
std::atomic<uint64_t> g_swapMarkQueued{0}, g_swapMarkDone{0};
// M60 diagnostico: V = VdSwap del juego, B = VBLANK (a = numero), S = XE_SWAP en el CP, I = interrupcion de volteo,
// F/f = inicio/fin de FlushResolves con espera, K/k = FlushAndWait, L = lote del anillo
namespace { struct M60Ev { int64_t t; char c; uint32_t a; }; M60Ev g_m60ev[512]; std::atomic<uint32_t> g_m60n{0}; std::mutex g_m60mu; }
void M60Event(char c, uint32_t a) {
    if (!rt::gpu::d3d::g_gpuTiming) return;
    LARGE_INTEGER t; QueryPerformanceCounter(&t);
    std::lock_guard<std::mutex> g(g_m60mu);
    uint32_t i = g_m60n++ & 511; g_m60ev[i] = {t.QuadPart, c, a};
}
std::atomic<uint64_t> g_vbWaitTicks{0}, g_vbWaits{0};
void M60DumpEvents(const char* why) {
    LARGE_INTEGER f, now; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&now);
    std::string s = std::string("M60 eventos (") + why + "), ms antes de ahora:";
    std::lock_guard<std::mutex> g(g_m60mu);
    uint32_t n = g_m60n.load();
    for (uint32_t k = (n > 60 ? n - 60 : 0); k < n; ++k) {
        const M60Ev& e = g_m60ev[k & 511];
        char b[40]; std::snprintf(b, sizeof(b), " %c%s%.1f", e.c, e.c == 'B' ? "" : "@", (double)(now.QuadPart - e.t) * 1000.0 / (double)f.QuadPart);
        s += b;
    }
    ImportLog::Get().Printf("%s", s.c_str());
}

uint64_t FrameCount() { return g_frames; }
uint64_t VblankCount() { return g_vblank; }
uint32_t ReadPointer() { return g_rptr; }
uint32_t WritePointer() { return g_wptr; }
uint64_t SwapCalls() { return g_swapCalls; }

uint8_t* PhysHost(uint32_t phys, uint32_t bytes) {
    uint32_t p = phys & 0x1FFFFFFF;
    uint8_t* base = GuestMemory::Base();
    uint8_t* h = base + 0xA0000000u + p;
    uint64_t end = (uint64_t)p + bytes;
    if (end > 0x20000000ull) return nullptr;
    // M8: con las vistas A0/C0 sobre la seccion de 512 MB toda la memoria fisica esta confirmada (la seccion nunca se
    // libera: Decommit solo pone a cero), por lo que VirtualQuery siempre devolveria MEM_COMMIT: se omite (llamada al sistema).
    if (GuestMemory::PhysicalViewsMapped()) return h;
    // Comprobacion de memoria confirmada (por regiones).
    uint8_t* cur = h;
    uint8_t* stop = h + bytes;
    while (cur < stop) {
        MEMORY_BASIC_INFORMATION mi;
        if (!VirtualQuery(cur, &mi, sizeof(mi)) || mi.State != MEM_COMMIT || (mi.Protect & (PAGE_NOACCESS | PAGE_GUARD))) return nullptr;
        cur = (uint8_t*)mi.BaseAddress + mi.RegionSize;
    }
    return h;
}

void Summary() {
    ImportLog::Get().Printf("GPU resumen: frames(XE_SWAP)=%llu VdSwap=%llu vblank=%llu lotes_RB=%llu scanout_writes=%llu wptr=%u rptr=%u",
                            (unsigned long long)g_frames, (unsigned long long)g_swapCalls, (unsigned long long)g_vblank,
                            (unsigned long long)g_batches, (unsigned long long)g_scanouts, g_wptr.load(), g_rptr.load());
    ImportLog::Get().Printf("H0 draws=%llu sin_vs=%llu sin_ps=%llu shaders_unicos=%zu analisis_fallidos=%llu dirty_total=%llu dirty_medio=%.2f dirty_max=%llu",
                            (unsigned long long)g_drawsSeen, (unsigned long long)g_drawsNoVs, (unsigned long long)g_drawsNoPs, g_shaderStore.size(),
                            (unsigned long long)g_shaderParseFail, (unsigned long long)g_dirtyTotal,
                            g_drawsSeen ? (double)g_dirtyTotal / (double)g_drawsSeen : 0.0, (unsigned long long)g_dirtyMax);
    uint64_t vs = 0, ps = 0, vsMem = 0, ps_kill = 0, vsLoop = 0, vsJmp = 0, psLoop = 0, psJmp = 0, calls = 0;
    for (auto& kv : g_shaderStore) {
        const ShaderInfo& s = *kv.second;
        if (s.vertex) { ++vs; if (s.memexportAddr) ++vsMem; if (s.loopCount) ++vsLoop; if (s.jmpCount) ++vsJmp; }
        else { ++ps; if (s.kills) ++ps_kill; if (s.loopCount) ++psLoop; if (s.jmpCount) ++psJmp; }
        calls += s.callCount;
    }
    ImportLog::Get().Printf("H0 shaders vs=%llu ps=%llu vs_con_memexport=%llu ps_con_kill=%llu loops(vs/ps)=%llu/%llu saltos(vs/ps)=%llu/%llu llamadas=%llu",
                            (unsigned long long)vs, (unsigned long long)ps, (unsigned long long)vsMem, (unsigned long long)ps_kill,
                            (unsigned long long)vsLoop, (unsigned long long)psLoop, (unsigned long long)vsJmp, (unsigned long long)psJmp, (unsigned long long)calls);
    std::lock_guard<std::mutex> g(g_histMu);
    for (auto& kv : g_hist) ImportLog::Get().Printf("HIST %s n=%llu", kv.first.c_str(), (unsigned long long)kv.second);
}

uint32_t VirtToPhys(uint32_t v) { return v >= 0xA0000000u ? (v & 0x1FFFFFFF) : v; }

const uint32_t* g_regFile = g_regs;
static_assert(kRegCount == 0x5003, "RegRead en linea (gpu.h) supone kRegCount = 0x5003");
void RegWriteCpu(uint32_t idx, uint32_t v) {
    if (idx >= kRegCount) return;
    std::lock_guard<std::mutex> g(g_regMu);
    if (idx < kGuestRegWords) SetGuestReg(idx, v); else g_regs[idx] = v;
}

} // namespace rt::gpu
