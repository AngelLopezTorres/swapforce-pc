// M8: perfilador de muestreo (--perf). Ver perf.h.
// Reglas de seguridad: mientras un hilo esta suspendido no se reserva memoria ni se toma ningun bloqueo
// (el hilo suspendido podria tener el del montón); solo se copian RIP/RSP y se desenrolla con la .pdata del exe.
#include "perf.h"

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "gpu.h"
#include "import_log.h"
#include "guest_thread.h"

#pragma comment(lib, "psapi.lib")

namespace rt::gpu { void Pm4WaitReport(void (*out)(const char*)); }

namespace rt::perf {

bool g_enabled = false;
bool g_sample = true;   // M8: --perf-nosample (solo fps y zonas, sin muestreo de pilas)

namespace {

constexpr int kMaxDepth = 48;
const char* kZoneNames[kZoneCount] = {"espera_valla", "fb_decode", "subida_texturas", "subida_memoria", "resolve", "present",
                                      "OnSwap_total", "OnDraw_total", "resolve_vaciado", "creacion_pso", "creacion_textura"};
std::atomic<uint64_t> g_zoneTicks[kZoneCount];
std::atomic<uint64_t> g_zoneCalls[kZoneCount];

struct ModRange { uintptr_t lo, hi; std::string name; };

struct ThreadRec {
    DWORD tid = 0;
    HANDLE h = nullptr;
    std::string name;
    uintptr_t stackLo = 0, stackHi = 0;
    uint64_t lastCycles = 0, firstCycles = 0, cyclesAtLast = 0;
    uint64_t busy = 0, idle = 0;
    uint64_t stallCycles0 = 0;
    bool alive = true;
    std::unordered_map<uint64_t, uint32_t> excl;    // clave = rva de inicio de funcion (exe) o (1<<63 | modulo<<32 | 0)
    std::unordered_map<uint32_t, uint32_t> incl;    // inclusivo por funcion (muestras ocupadas)
};

struct State {
    HANDLE thread = nullptr;
    std::atomic<bool> run{false};
    std::string out;
    uintptr_t exeBase = 0, exeEnd = 0, textLo = 0, textHi = 0;
    const RUNTIME_FUNCTION* rf = nullptr;
    uint32_t rfCount = 0;
    std::vector<ModRange> mods;
    std::vector<ThreadRec*> threads;
    std::unordered_map<uint32_t, uint64_t> inclBusy, exclBusy, inclWall;   // rva de inicio de funcion / rva
    std::unordered_map<uint64_t, uint64_t> exclMod;                         // modulo (fuera del exe) -> muestras ocupadas
    uint64_t samplesBusy = 0, samplesIdle = 0, rounds = 0;
    uint64_t t0 = 0, qpf = 1;
    DWORD selfTid = 0, fpsTid = 0;
    HANDLE fpsThread = nullptr;
};
State* S = nullptr;

typedef LONG(NTAPI* NtQIT)(HANDLE, ULONG, PVOID, ULONG, PULONG);
struct TBI { LONG ExitStatus; PVOID TebBaseAddress; struct { HANDLE p, t; } ClientId; ULONG_PTR AffinityMask; LONG Priority; LONG BasePriority; };

const RUNTIME_FUNCTION* LookupRf(uint32_t rva) {
    uint32_t lo = 0, hi = S->rfCount;
    while (lo < hi) {
        uint32_t mid = (lo + hi) / 2;
        const RUNTIME_FUNCTION& r = S->rf[mid];
        if (rva < r.BeginAddress) hi = mid;
        else if (rva >= r.EndAddress) lo = mid + 1;
        else return &r;
    }
    return nullptr;
}

void InitExe() {
    S->exeBase = (uintptr_t)GetModuleHandleA(nullptr);
    auto* dos = (IMAGE_DOS_HEADER*)S->exeBase;
    auto* nt = (IMAGE_NT_HEADERS64*)(S->exeBase + dos->e_lfanew);
    S->exeEnd = S->exeBase + nt->OptionalHeader.SizeOfImage;
    auto& d = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
    S->rf = (const RUNTIME_FUNCTION*)(S->exeBase + d.VirtualAddress);
    S->rfCount = d.Size / sizeof(RUNTIME_FUNCTION);
    auto* sec = IMAGE_FIRST_SECTION(nt);
    for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i)
        if (!std::memcmp(sec[i].Name, ".text", 5)) {
            S->textLo = S->exeBase + sec[i].VirtualAddress;
            S->textHi = S->textLo + sec[i].Misc.VirtualSize;
        }
}

void RefreshModules() {
    HMODULE mods[512];
    DWORD need = 0;
    if (!K32EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &need)) return;
    std::vector<ModRange> v;
    for (DWORD i = 0; i < need / sizeof(HMODULE) && i < 512; ++i) {
        MODULEINFO mi;
        char nm[MAX_PATH];
        if (!K32GetModuleInformation(GetCurrentProcess(), mods[i], &mi, sizeof(mi))) continue;
        if (!K32GetModuleBaseNameA(GetCurrentProcess(), mods[i], nm, sizeof(nm))) std::strcpy(nm, "?");
        v.push_back({(uintptr_t)mi.lpBaseOfDll, (uintptr_t)mi.lpBaseOfDll + mi.SizeOfImage, nm});
    }
    S->mods.swap(v);
}

int ModIndex(uintptr_t a) {
    for (size_t i = 0; i < S->mods.size(); ++i) if (a >= S->mods[i].lo && a < S->mods[i].hi) return (int)i;
    return -1;
}

std::string ThreadName(HANDLE h, DWORD tid) {
    PWSTR w = nullptr;
    std::string r;
    if (SUCCEEDED(GetThreadDescription(h, &w)) && w && *w) {
        char b[256];
        WideCharToMultiByte(CP_UTF8, 0, w, -1, b, sizeof(b), nullptr, nullptr);
        r = b;
    }
    if (w) LocalFree(w);
    if (r.empty()) r = "host-" + std::to_string(tid);
    return r;
}

void RefreshThreads() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) return;
    static NtQIT qit = (NtQIT)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationThread");
    THREADENTRY32 te = {sizeof(te)};
    DWORD pid = GetCurrentProcessId();
    for (auto* t : S->threads) t->alive = false;
    for (BOOL ok = Thread32First(snap, &te); ok; ok = Thread32Next(snap, &te)) {
        if (te.th32OwnerProcessID != pid || te.th32ThreadID == S->selfTid || te.th32ThreadID == S->fpsTid) continue;
        ThreadRec* rec = nullptr;
        for (auto* t : S->threads) if (t->tid == te.th32ThreadID && t->h) { rec = t; break; }
        if (!rec) {
            HANDLE h = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);
            if (!h) continue;
            rec = new ThreadRec();
            rec->tid = te.th32ThreadID;
            rec->h = h;
            TBI bi = {};
            if (qit && qit(h, 0, &bi, sizeof(bi), nullptr) == 0 && bi.TebBaseAddress) {
                auto* tib = (NT_TIB*)bi.TebBaseAddress;
                rec->stackHi = (uintptr_t)tib->StackBase;
                rec->stackLo = rec->stackHi > (64ull << 20) ? 0 : 0;   // limite inferior: se comprueba por VirtualQuery
            }
            QueryThreadCycleTime(h, &rec->firstCycles);
            rec->lastCycles = rec->firstCycles;
            S->threads.push_back(rec);
        }
        rec->alive = true;
        rec->name = ThreadName(rec->h, rec->tid);   // el juego puede renombrar
    }
    CloseHandle(snap);
}

// Desenrolla la pila del contexto c en frames[] (direcciones absolutas). Sin reservas de memoria.
int Unwind(CONTEXT& c, uintptr_t stackHi, uintptr_t* frames) {
    int n = 0;
    __try {
        for (int d = 0; d < kMaxDepth && c.Rip; ++d) {
            uintptr_t rip = c.Rip;
            frames[n++] = rip;
            if (rip >= S->exeBase && rip < S->exeEnd) {
                const RUNTIME_FUNCTION* fe = LookupRf((uint32_t)(rip - S->exeBase));
                if (!fe) {   // funcion hoja sin .pdata
                    if (!c.Rsp || (stackHi && c.Rsp >= stackHi)) break;
                    c.Rip = *(uint64_t*)c.Rsp; c.Rsp += 8;
                } else {
                    void* hd = nullptr; DWORD64 est = 0;
                    RtlVirtualUnwind(UNW_FLAG_NHANDLER, S->exeBase, rip, (PRUNTIME_FUNCTION)fe, &c, &hd, &est, nullptr);
                }
            } else {
                // fuera del exe: busqueda de la primera direccion de retorno al .text del exe en la pila
                if (!stackHi || c.Rsp >= stackHi) break;
                uintptr_t lim = std::min<uintptr_t>(stackHi, c.Rsp + 0x8000);
                bool found = false;
                for (uintptr_t p = c.Rsp; p + 8 <= lim; p += 8) {
                    uint64_t q = *(uint64_t*)p;
                    if (q >= S->textLo + 6 && q < S->textHi) {
                        const uint8_t* ins = (const uint8_t*)q;
                        bool isCall = ins[-5] == 0xE8 || (ins[-6] == 0xFF && ins[-5] == 0x15) || (ins[-2] == 0xFF && (ins[-1] & 0x38) == 0x10) ||
                                      (ins[-3] == 0xFF && (ins[-2] & 0x38) == 0x10);
                        if (!isCall) continue;
                        c.Rip = q; c.Rsp = p + 8; found = true;
                        break;
                    }
                }
                if (!found) break;
            }
            if (stackHi && c.Rsp >= stackHi) break;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return n;
}

void Account(ThreadRec* t, bool busy, const uintptr_t* fr, int n) {
    if (n <= 0) return;
    uintptr_t leaf = fr[0];
    if (busy) {
        ++S->samplesBusy; ++t->busy;
        if (leaf >= S->exeBase && leaf < S->exeEnd) {
            uint32_t rva = (uint32_t)(leaf - S->exeBase);
            ++S->exclBusy[rva];
            const RUNTIME_FUNCTION* fe = LookupRf(rva);
            ++t->excl[fe ? fe->BeginAddress : rva];
        } else {
            int m = ModIndex(leaf);
            uint64_t key = (1ull << 63) | ((uint64_t)(uint32_t)m << 32);
            ++t->excl[key];
            ++S->exclMod[(uint64_t)(uint32_t)m];
        }
    } else {
        ++S->samplesIdle; ++t->idle;
    }
    // inclusivo: funcion (inicio de .pdata) contada una vez por muestra
    uint32_t seen[kMaxDepth]; int ns = 0;
    for (int i = 0; i < n; ++i) {
        uintptr_t a = fr[i];
        if (a < S->exeBase || a >= S->exeEnd) continue;
        uint32_t rva = (uint32_t)(a - S->exeBase - (i ? 1 : 0));
        const RUNTIME_FUNCTION* fe = LookupRf(rva);
        uint32_t key = fe ? fe->BeginAddress : rva;
        bool dup = false;
        for (int k = 0; k < ns; ++k) if (seen[k] == key) { dup = true; break; }
        if (dup) continue;
        seen[ns++] = key;
        if (busy) { ++S->inclBusy[key]; ++t->incl[key]; }
        else ++S->inclWall[key];
    }
}

// M8: volcado de pilas host de todos los hilos a un fichero propio (no usa ImportLog). Direcciones absolutas + base de modulos.
void DumpHostStacks(const char* path) {
    RefreshModules();
    RefreshThreads();
    FILE* f = std::fopen(path, "a");
    if (!f) return;
    std::fprintf(f, "== volcado de pilas host (exe base=%llX)\n", (unsigned long long)S->exeBase);
    for (auto& m : S->mods) std::fprintf(f, "modulo %llX %llX %s\n", (unsigned long long)m.lo, (unsigned long long)m.hi, m.name.c_str());
    static uintptr_t fr[kMaxDepth];
    for (ThreadRec* t : S->threads) {
        if (!t->alive || t->tid == GetCurrentThreadId()) continue;
        int n = 0;
        if (SuspendThread(t->h) == (DWORD)-1) continue;
        CONTEXT c; c.ContextFlags = CONTEXT_FULL;
        if (GetThreadContext(t->h, &c)) n = Unwind(c, t->stackHi, fr);
        ResumeThread(t->h);
        std::fprintf(f, "hilo %lu \"%s\":", t->tid, t->name.c_str());
        for (int i = 0; i < n; ++i) std::fprintf(f, " %llX", (unsigned long long)fr[i]);
        std::fprintf(f, "\n");
    }
    std::fclose(f);
}

// M8: tasa de XE_SWAP/VdSwap por segundo en un hilo ligero (no depende del muestreador)
DWORD WINAPI FpsMain(void*) {
    NameCurrentThread("perf-fps");
    uint64_t t0 = Now(), next = t0 + S->qpf;
    uint64_t lastFrames = gpu::FrameCount(), lastSwaps = gpu::SwapCalls(), lastT = t0;
    uint32_t sec = 0;
    while (S->run) {
        uint64_t now = Now();
        {   // deteccion de bloqueo (antes de cualquier Printf): >3 s sin XE_SWAP -> pilas host a m8_stall_stacks.txt (max 3)
            static uint64_t lf = 0, lft = 0; static int dumps = 0;
            uint64_t fc = gpu::FrameCount();
            if (fc != lf) { lf = fc; lft = now; }
            else if (fc && lft && now - lft > 3 * S->qpf && dumps < 3) { ++dumps; lft = now; DumpHostStacks("m8_stall_stacks.txt"); }
        }
        if (now < next) { Sleep((DWORD)std::min<uint64_t>(50, (next - now) * 1000 / S->qpf + 1)); continue; }
        ++sec;
        uint64_t f = gpu::FrameCount(), sw = gpu::SwapCalls();
        double dt = (double)(now - lastT) / (double)S->qpf;
        ImportLog::Get().Printf("PERF t=%u frames=%llu swaps=%llu fps_xe_swap=%.2f fps_vdswap=%.2f", sec, (unsigned long long)f, (unsigned long long)sw,
                                (double)(f - lastFrames) / dt, (double)(sw - lastSwaps) / dt);
        lastFrames = f; lastSwaps = sw; lastT = now;
        next += S->qpf;
    }
    return 0;
}

DWORD WINAPI SamplerMain(void*) {
    NameCurrentThread("perf-sampler");
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    HANDLE tm = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    if (tm) { LARGE_INTEGER due; due.QuadPart = -10000; SetWaitableTimer(tm, &due, 1, nullptr, nullptr, FALSE); }
    uint64_t lastRefresh = 0, lastSec = Now();
    uint64_t lastFrames = gpu::FrameCount(), lastSwaps = gpu::SwapCalls();
    uint32_t sec = 0;
    static uintptr_t frames[kMaxDepth];
    while (S->run) {
        if (tm) WaitForSingleObject(tm, 100); else Sleep(1);
        uint64_t now = Now();
        if (now - lastRefresh > S->qpf / 4) { RefreshModules(); RefreshThreads(); lastRefresh = now; }
        ++S->rounds;
        for (ThreadRec* t : S->threads) {
            if (!t->alive) continue;
            uint64_t cyc = 0;
            QueryThreadCycleTime(t->h, &cyc);
            bool busy = cyc - t->lastCycles > 50000;   // ~>15 us de CPU desde la ultima muestra
            t->lastCycles = cyc;
            if (SuspendThread(t->h) == (DWORD)-1) { t->alive = false; continue; }
            CONTEXT c;
            c.ContextFlags = CONTEXT_FULL;
            int n = 0;
            if (GetThreadContext(t->h, &c)) n = Unwind(c, t->stackHi, frames);
            ResumeThread(t->h);
            Account(t, busy, frames, n);
        }
        {   // M8: diagnostico de tirones: >400 ms sin XE_SWAP -> volcado de hilos guest y, al reanudarse, hilos con mas ciclos
            static uint64_t lastF = 0, lastFT = 0; static bool inStall = false; static uint64_t stallT0 = 0;
            uint64_t f = gpu::FrameCount();
            if (f != lastF) {
                if (inStall) {
                    std::vector<std::pair<uint64_t, ThreadRec*>> v;
                    for (ThreadRec* t : S->threads) { uint64_t c = 0; QueryThreadCycleTime(t->h, &c); v.push_back({c - t->stallCycles0, t}); }
                    std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return a.first > b.first; });
                    char b[900]; int n = std::snprintf(b, sizeof(b), "M8 TIRON fin: %.0f ms; hilos por ciclos:", (now - stallT0) * 1000.0 / S->qpf);
                    for (size_t i = 0; i < v.size() && i < 8 && n < 820; ++i) n += std::snprintf(b + n, sizeof(b) - n, " [%s %.0fM]", v[i].second->name.c_str(), v[i].first / 1e6);
                    ImportLog::Get().Printf("%s", b);
                    inStall = false;
                }
                lastF = f; lastFT = now;
            } else if (!inStall && lastFT && now - lastFT > S->qpf * 4 / 10 && f > 0) {
                inStall = true; stallT0 = lastFT;
                for (ThreadRec* t : S->threads) { uint64_t c = 0; QueryThreadCycleTime(t->h, &c); t->stallCycles0 = c; }
                ImportLog::Get().Printf("M8 TIRON en curso: %llu ms sin XE_SWAP tras el fotograma %llu", (unsigned long long)((now - lastFT) * 1000 / S->qpf), (unsigned long long)f);
                GuestThread::DumpWaitStates();
            }
        }
        if (false) {
            ++sec;
            uint64_t f = gpu::FrameCount(), sw = gpu::SwapCalls();
            double dt = (double)(now - lastSec) / (double)S->qpf;
            ImportLog::Get().Printf("PERF t=%u frames=%llu swaps=%llu fps_xe_swap=%.2f fps_vdswap=%.2f", sec, (unsigned long long)f,
                                    (unsigned long long)sw, (double)(f - lastFrames) / dt, (double)(sw - lastSwaps) / dt);
            lastFrames = f; lastSwaps = sw; lastSec = now;
        }
    }
    if (tm) CloseHandle(tm);
    return 0;
}

void WriteReport() {
    FILE* f = std::fopen(S->out.c_str(), "w");
    if (!f) return;
    double secs = (double)(Now() - S->t0) / (double)S->qpf;
    std::fprintf(f, "# perf report (rva relativas a la base del exe; simbolizar con tools\\perf_report.py)\n");
    std::fprintf(f, "duracion_s %.3f rondas %llu muestras_ocupadas %llu muestras_inactivas %llu\n", secs, (unsigned long long)S->rounds,
                 (unsigned long long)S->samplesBusy, (unsigned long long)S->samplesIdle);
    for (int z = 0; z < kZoneCount; ++z)
        std::fprintf(f, "zona %s llamadas=%llu total_ms=%.1f media_us=%.1f\n", kZoneNames[z], (unsigned long long)g_zoneCalls[z].load(),
                     g_zoneTicks[z].load() * 1000.0 / S->qpf,
                     g_zoneCalls[z] ? g_zoneTicks[z].load() * 1e6 / S->qpf / g_zoneCalls[z].load() : 0.0);
    for (size_t i = 0; i < S->mods.size(); ++i) std::fprintf(f, "modulo %zu %s\n", i, S->mods[i].name.c_str());
    for (ThreadRec* t : S->threads) {
        uint64_t cyc = 0;
        QueryThreadCycleTime(t->h, &cyc);
        std::fprintf(f, "hilo tid=%lu nombre=\"%s\" ciclos=%llu ocupadas=%llu inactivas=%llu\n", t->tid, t->name.c_str(),
                     (unsigned long long)(cyc - t->firstCycles), (unsigned long long)t->busy, (unsigned long long)t->idle);
        std::vector<std::pair<uint64_t, uint32_t>> v(t->excl.begin(), t->excl.end());
        std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return a.second > b.second; });
        for (size_t i = 0; i < v.size() && i < 40; ++i) {
            if (v[i].first >> 63) std::fprintf(f, "  hexcl mod %u %u\n", (unsigned)((v[i].first >> 32) & 0x7FFFFFFF), v[i].second);
            else std::fprintf(f, "  hexcl rva %08X %u\n", (uint32_t)v[i].first, v[i].second);
        }
        std::vector<std::pair<uint32_t, uint32_t>> vi(t->incl.begin(), t->incl.end());
        std::sort(vi.begin(), vi.end(), [](auto& a, auto& b) { return a.second > b.second; });
        for (size_t i = 0; i < vi.size() && i < 40; ++i) std::fprintf(f, "  hincl rva %08X %u\n", vi[i].first, vi[i].second);
    }
    auto dump = [&](const char* tag, std::unordered_map<uint32_t, uint64_t>& m, size_t lim) {
        std::vector<std::pair<uint32_t, uint64_t>> v(m.begin(), m.end());
        std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return a.second > b.second; });
        for (size_t i = 0; i < v.size() && i < lim; ++i) std::fprintf(f, "%s %08X %llu\n", tag, v[i].first, (unsigned long long)v[i].second);
    };
    dump("excl", S->exclBusy, 3000);
    for (auto& kv : S->exclMod) std::fprintf(f, "exclmod %u %llu\n", (unsigned)kv.first, (unsigned long long)kv.second);
    dump("incl", S->inclBusy, 600);
    dump("inclwall", S->inclWall, 300);
    std::fclose(f);
}

} // namespace

uint64_t Now() { LARGE_INTEGER t; QueryPerformanceCounter(&t); return (uint64_t)t.QuadPart; }

void ZoneAdd(int z, uint64_t t) {
    g_zoneTicks[z].fetch_add(t, std::memory_order_relaxed);
    g_zoneCalls[z].fetch_add(1, std::memory_order_relaxed);
}

uint32_t g_tironMs = 100;   // M60: --tiron-ms N (umbral del registro M8 TIRON entre XE_SWAP)
void OnFrame(uint64_t frame) {
    if (!g_enabled || !S) return;
    static uint64_t last = 0, lastZ[kZoneCount] = {};
    uint64_t now = Now();
    if (last && now - last > S->qpf * g_tironMs / 1000) {
        char b[600]; int n = std::snprintf(b, sizeof(b), "M8 TIRON frame=%llu dt_ms=%.1f zonas_ms:", (unsigned long long)frame, (now - last) * 1000.0 / S->qpf);
        for (int z = 0; z < kZoneCount && n < (int)sizeof(b) - 40; ++z)
            n += std::snprintf(b + n, sizeof(b) - n, " %s=%.1f", kZoneNames[z], (g_zoneTicks[z].load() - lastZ[z]) * 1000.0 / S->qpf);
        ImportLog::Get().Printf("%s", b);
    }
    last = now;
    for (int z = 0; z < kZoneCount; ++z) lastZ[z] = g_zoneTicks[z].load();
    {   // M60: zonas por segundo (ms de cada zona en el ultimo segundo; las zonas pueden anidarse)
        static uint64_t secT = 0, secF = 0, secZ[kZoneCount] = {};
        if (!secT) { secT = now; secF = frame; for (int z = 0; z < kZoneCount; ++z) secZ[z] = g_zoneTicks[z].load(); }
        else if (now - secT >= S->qpf) {
            char b[700]; int n = std::snprintf(b, sizeof(b), "M60 ZONAS %.2f s frames=%llu ms:", (now - secT) / (double)S->qpf, (unsigned long long)(frame - secF));
            for (int z = 0; z < kZoneCount && n < (int)sizeof(b) - 40; ++z) {
                uint64_t v = g_zoneTicks[z].load();
                n += std::snprintf(b + n, sizeof(b) - n, " %s=%.1f", kZoneNames[z], (v - secZ[z]) * 1000.0 / S->qpf);
                secZ[z] = v;
            }
            ImportLog::Get().Printf("%s", b);
            secT = now; secF = frame;
        }
    }
}

void NameCurrentThread(const char* name) {
    wchar_t w[128];
    MultiByteToWideChar(CP_UTF8, 0, name, -1, w, 128);
    SetThreadDescription(GetCurrentThread(), w);
}

void Start(const char* outPath) {
    if (S) return;
    S = new State();
    S->out = outPath;
    LARGE_INTEGER q; QueryPerformanceFrequency(&q); S->qpf = (uint64_t)q.QuadPart;
    S->t0 = Now();
    InitExe();
    g_enabled = true;
    S->run = true;
    S->fpsThread = CreateThread(nullptr, 1 << 16, FpsMain, nullptr, CREATE_SUSPENDED, &S->fpsTid);
    if (g_sample) {
        S->thread = CreateThread(nullptr, 1 << 20, SamplerMain, nullptr, CREATE_SUSPENDED, &S->selfTid);
        ResumeThread(S->thread);
    }
    ResumeThread(S->fpsThread);
    ImportLog::Get().Printf("PERF muestreador iniciado (.pdata=%u entradas) -> %s", S->rfCount, outPath);
}

void Stop() {
    if (!S || !S->run) return;
    S->run = false;
    if (S->thread) WaitForSingleObject(S->thread, 3000);
    WaitForSingleObject(S->fpsThread, 3000);
    WriteReport();
    rt::gpu::Pm4WaitReport([](const char* l) { ImportLog::Get().Printf("%s", l); });
    for (int z = 0; z < kZoneCount; ++z)
        ImportLog::Get().Printf("PERF zona %s llamadas=%llu total_ms=%.1f", kZoneNames[z], (unsigned long long)g_zoneCalls[z].load(), g_zoneTicks[z].load() * 1000.0 / S->qpf);
    ImportLog::Get().Printf("PERF informe escrito en %s", S->out.c_str());
}

} // namespace rt::perf
