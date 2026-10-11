#include "guest_thread.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <vector>
#include <cstdio>
#include <cstring>
#include <new>
#include <string>

#include "guest_abi.h"
#include "guest_heap.h"
#include "guest_memory.h"
#include "import_log.h"
#include "ppc_config.h"
#include "xex_header.h"
#include "trace_hooks.h"
#include "ppc_context.h"

namespace rt {

thread_local TraceRing g_traceRing;
static thread_local GuestThread* g_current = nullptr;
static std::atomic<uint32_t> g_nextId{1};

GuestThread* GuestThread::Current() { return g_current; }

static std::mutex g_regMu;
static std::vector<GuestThread*> g_registry;

GuestThread* GuestThread::ById(uint32_t id) {
    std::lock_guard<std::mutex> g(g_regMu);
    for (GuestThread* t : g_registry) if (t->id == id) return t;
    return nullptr;
}

uint32_t XexDefaultStackSize() {
    uint32_t v;
    if (XexFindOptHeader(0x00020200, &v, nullptr) && v >= 0x4000) return v; // tamano de pila por defecto del XEX
    return 0x40000;
}

GuestThread* GuestThread::Create(const GuestThreadParams& p, std::string* err) {
    auto fail = [&](const char* m) -> GuestThread* { if (err) *err = m; return nullptr; };
    GuestThread* t = new GuestThread();
    t->params_ = p;
    t->id = g_nextId.fetch_add(1);
    { std::lock_guard<std::mutex> g(g_regMu); g_registry.push_back(t); }
    uint8_t* base = GuestMemory::Base();

    uint32_t stackSize = p.stackSize ? p.stackSize : XexDefaultStackSize();
    stackSize = (stackSize + 0xFFFF) & ~0xFFFFu;
    t->stackLow = StackHeap().Alloc(stackSize, 0x10000);
    if (!t->stackLow) return fail("pila guest");
    t->stackHigh = t->stackLow + stackSize;

    XexTlsInfo tls = XexGetTlsInfo();
    uint32_t slots = tls.present && tls.slotCount ? tls.slotCount : 0x40;
    uint32_t ext = tls.present ? (tls.dataSize > tls.rawDataSize ? tls.dataSize : tls.rawDataSize) : 0;
    t->tlsSize = slots * 4 + ext;
    t->tlsDynamic = 0;
    t->tls = SystemHeap().Alloc(t->tlsSize);
    t->pcr = SystemHeap().Alloc(0x1000);
    t->kthread = SystemHeap().Alloc(0xAB0);
    if (!t->tls || !t->pcr || !t->kthread) return fail("PCR/TLS/KTHREAD guest");
    t->tlsDynamic = t->tls + ext;
    if (tls.present && tls.rawDataSize)
        std::memcpy(base + t->tls, base + tls.rawDataAddress, tls.rawDataSize);

    // PCR (offsets X_KPCR de Xenia, xthread.h)
    P32(base, t->pcr + 0x00, t->tls);
    P32(base, t->pcr + 0x30, t->pcr);
    P32(base, t->pcr + 0x70, t->stackHigh);
    P32(base, t->pcr + 0x74, t->stackLow);
    P32(base, t->pcr + 0x100, t->kthread);
    base[t->pcr + 0x10C] = 0;
    P32(base, t->pcr + 0x150, 0);
    // KTHREAD (offsets Xenia): 0x5C stack_base, 0x60 stack_limit, 0x68 tls, 0x14C id
    P32(base, t->kthread + 0x5C, t->stackHigh);
    P32(base, t->kthread + 0x60, t->stackLow);
    P32(base, t->kthread + 0x68, t->tls);
    P32(base, t->kthread + 0x14C, t->id);

    void* mem = _aligned_malloc(sizeof(PPCContext), 0x40);
    if (!mem) return fail("memoria del contexto");
    t->ctx_ = new (mem) PPCContext();
    std::memset((void*)t->ctx_, 0, sizeof(PPCContext));
    t->ctx_->msr = 0x200A000;
    t->ctx_->r1.u64 = t->stackHigh - 0x100;
    t->ctx_->r13.u64 = t->pcr;
    t->ctx_->r3.u64 = p.arg;
    t->ctx_->lr = 0;
    t->ctx_->fpscr.csr = 0x1F80; // MXCSR por defecto del host (mascaras de excepcion)
    t->suspendedFlag_ = p.suspended;
    t->resumeEvent_ = CreateEventA(nullptr, TRUE, !p.suspended, nullptr);
    return t;
}

static LONG LogException(EXCEPTION_POINTERS* ep, GuestThread* t) {
    auto& log = ImportLog::Get();
    EXCEPTION_RECORD* er = ep->ExceptionRecord;
    uint8_t* base = GuestMemory::Base();
    uint64_t fa = er->NumberParameters >= 2 ? er->ExceptionInformation[1] : 0;
    bool inGuest = fa >= (uint64_t)base && fa < (uint64_t)base + 0x100000000ull;
    log.Printf("EXCEPCION guest tid=%u codigo=0x%08lX hostRIP=%p acceso=%s dirHost=0x%llX dirGuest=%s%08llX",
               t->id, er->ExceptionCode, er->ExceptionAddress,
               er->NumberParameters >= 1 ? (er->ExceptionInformation[0] == 1 ? "escritura" : er->ExceptionInformation[0] == 8 ? "DEP" : "lectura") : "-",
               (unsigned long long)fa, inGuest ? "" : "(fuera de guest) ", (unsigned long long)(inGuest ? fa - (uint64_t)base : 0));
    if (inGuest) {
        uint32_t g = (uint32_t)(fa - (uint64_t)base);
        const char* cls = g >= 0xE0000000 ? "vista fisica 0xE0000000 (4K)" : g >= 0xC0000000 ? "vista fisica 0xC0000000 (16M)"
                        : g >= 0xA0000000 ? "vista fisica 0xA0000000 (64K) sin asignar" : "fuera de A0-FF";
        log.Printf("  clase de acceso (Fase 1.3): %s", cls);
    }
    PPCContext& c = t->Ctx();
    log.Printf("  guest: lr=%08X r1=%08X r3=%08X r4=%08X r5=%08X r11=%08X r12=%08X ultimo import=%s",
               (uint32_t)c.lr, c.r1.u32, c.r3.u32, c.r4.u32, c.r5.u32, c.r11.u32, c.r12.u32, ImportLog::LastImport());
    return EXCEPTION_EXECUTE_HANDLER;
}

static int RunGuestSeh(PPCFunc* fn, GuestThread* t, uint8_t* base) {
    __try {
        fn(t->Ctx(), base);
        return 0;
    } __except (LogException(GetExceptionInformation(), t)) {
        return 1;
    }
}

GuestThread* GuestThread::CreateAttached(const char* name, std::string* err) {
    GuestThreadParams p;
    p.name = name;
    return Create(p, err);
}

void GuestThread::Attach() {
    g_current = this;
    traceRing = &g_traceRing;
}

void GuestThread::SetCpu(uint32_t cpu) { GuestMemory::Base()[pcr + 0x10C] = (uint8_t)cpu; }

bool GuestThread::CallGuest(uint32_t addr, uint32_t r3, uint32_t r4, uint32_t* ret, uint32_t r5, uint32_t r6) {
    uint8_t* base = GuestMemory::Base();
    PPCFunc* fn = PPC_LOOKUP_FUNC(base, addr);
    if (!fn) { ImportLog::Get().Printf("ERROR: CallGuest %08X sin funcion en la tabla", addr); return false; }
    ctx_->r1.u64 = stackHigh - 0x100;
    ctx_->r3.u64 = r3;
    ctx_->r4.u64 = r4;
    ctx_->r5.u64 = r5;
    ctx_->r6.u64 = r6;
    ctx_->lr = 0;
    int rc = RunGuestSeh(fn, this, base);
    if (ret) *ret = ctx_->r3.u32;
    return rc == 0;
}

// Diagnostico: SWAPFORCE_WATCH_F30=<tid> coloca un punto de interrupcion hardware de escritura sobre ctx.f30 de ese hilo
// y registra la funcion guest (por RIP host) que lo modifica.
static std::string GuestFuncFor(uintptr_t host);
static char g_wring[16][160];
static std::atomic<uint32_t> g_wn{0};
static LONG CALLBACK WatchVeh(EXCEPTION_POINTERS* ep) {
    if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP) return EXCEPTION_CONTINUE_SEARCH;
    GuestThread* t = GuestThread::Current();
    if (!t) return EXCEPTION_CONTINUE_SEARCH;
    static bool frozen = false;
    if (ep->ContextRecord->Dr6 & 2) { // DR1: memoria guest
        uint32_t a = (uint32_t)std::strtoul(std::getenv("SWAPFORCE_WATCH_MEM"), nullptr, 16);
        uint64_t v = __builtin_bswap64(*(uint64_t*)(GuestMemory::Base() + a));
        double d; std::memcpy(&d, &v, 8);
        static int mn = 0;
        if (mn++ < 60) ImportLog::Get().Printf("WATCHMEM %08X tid=%u guest=%s valor=%g (%016llX) r1=%08X lr=%08X", a, t->id, GuestFuncFor((uintptr_t)ep->ContextRecord->Rip).c_str(), d, (unsigned long long)v, t->Ctx().r1.u32, (uint32_t)t->Ctx().lr);
        ep->ContextRecord->Dr6 = 0; ep->ContextRecord->EFlags |= 0x10000; return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (frozen) { ep->ContextRecord->EFlags |= 0x10000; return EXCEPTION_CONTINUE_EXECUTION; }
    if (std::getenv("SWAPFORCE_WATCH_VERBOSE")) ImportLog::Get().Printf("W %u %s %g r1=%08X lr=%08X", g_wn.load(), GuestFuncFor((uintptr_t)ep->ContextRecord->Rip).c_str(), t->Ctx().f30.f64, t->Ctx().r1.u32, (uint32_t)t->Ctx().lr);
    uint32_t i = g_wn++ & 15;
    std::snprintf(g_wring[i], sizeof(g_wring[i]), "WATCH f30 #%u tid=%u guest=%s f30(nuevo)=%g r1=%08X lr=%08X", g_wn.load(), t->id,
                  GuestFuncFor((uintptr_t)ep->ContextRecord->Rip).c_str(), t->Ctx().f30.f64, t->Ctx().r1.u32, (uint32_t)t->Ctx().lr);
    ep->ContextRecord->EFlags |= 0x10000; // RF
    return EXCEPTION_CONTINUE_EXECUTION;
}
void DumpWatchRing() {
    uint32_t n = g_wn;
    for (uint32_t k = 0; k < 16 && k < n; ++k) ImportLog::Get().Printf("%s", g_wring[(n - 1 - k) & 15]);
}

DWORD WINAPI GuestThreadProc(void* p) {
    GuestThread* t = static_cast<GuestThread*>(p);
    t->Run();
    return 0;
}

void GuestThread::Run() {
    g_current = this;
    traceRing = &g_traceRing;
    if (const char* w = std::getenv("SWAPFORCE_WATCH_F30")) {
        if ((uint32_t)std::strtoul(w, nullptr, 0) == id) {
            static bool veh = false;
            if (!veh) { AddVectoredExceptionHandler(1, WatchVeh); veh = true; }
            CONTEXT c = {};
            c.ContextFlags = CONTEXT_DEBUG_REGISTERS;
            c.Dr0 = (DWORD64)&ctx_->f30;
            c.Dr7 = 1 | (1ull << 16) | (3ull << 18) | (1ull << 8); // L0, escritura, 8 bytes
            c.Dr7 &= ~0ull;
            if (const char* wm = std::getenv("SWAPFORCE_WATCH_MEM")) {
                c.Dr1 = (DWORD64)(GuestMemory::Base() + std::strtoul(wm, nullptr, 16));
                c.Dr7 |= (1ull << 2) | (1ull << 20) | (3ull << 22); // L1, escritura, 8 bytes
            }
            SetThreadContext(GetCurrentThread(), &c);
        }
    }
    WaitForSingleObject(resumeEvent_, INFINITE);
    auto& log = ImportLog::Get();
    uint8_t* base = GuestMemory::Base();
    PPCFunc* fn = PPC_LOOKUP_FUNC(base, params_.entry);
    log.Printf("start %s tid=%u entry=%08X host_fn=%p pcr=%08X stack=%08X-%08X tls=%08X",
               params_.name, id, params_.entry, (void*)fn, pcr, stackLow, stackHigh, tls);
    if (!fn) { log.Printf("ERROR: entrada guest %08X sin funcion en la tabla", params_.entry); exitCode_ = 2; finished_ = true; return; }
    exitCode_ = RunGuestSeh(fn, this, base);
    log.Printf("end %s tid=%u %s r3=%08X", params_.name, id, exitCode_ ? "por excepcion" : "retorno normal", ctx_->r3.u32);
    finished_ = true;
}

bool GuestThread::Start() {
    host_ = CreateThread(nullptr, 512u * 1024 * 1024, GuestThreadProc, this, STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr);
    return host_ != nullptr;
}

void GuestThread::Resume() { suspendedFlag_ = false; SetEvent(resumeEvent_); }

uint32_t GuestThread::ResumeThread() {
    uint32_t prev = suspendedFlag_ ? 1 : 0;
    if (prev) Resume();
    return prev;
}

bool GuestThread::Join(uint32_t ms) { return host_ && WaitForSingleObject(host_, ms) == WAIT_OBJECT_0; }

} // namespace rt

namespace rt {

static bool SafeRead(const uintptr_t* p, uintptr_t* out) {
    SIZE_T n = 0;
    return ReadProcessMemory(GetCurrentProcess(), p, out, sizeof(*out), &n) && n == sizeof(*out);
}

static std::vector<std::pair<uintptr_t, uint32_t>>& FuncVec() {
    static std::vector<std::pair<uintptr_t, uint32_t>> v;
    return v;
}

static std::string GuestFuncFor(uintptr_t host) {
    auto& v = FuncVec();
    if (v.empty()) {
        for (const PPCFuncMapping* m = PPCFuncMappings; m->guest != 0; ++m) v.emplace_back((uintptr_t)m->host, (uint32_t)m->guest);
        std::sort(v.begin(), v.end());
    }
    auto it = std::upper_bound(v.begin(), v.end(), std::make_pair(host, 0xFFFFFFFFu));
    if (it == v.begin()) return "";
    --it;
    if (host - it->first > 0x40000) return "";
    char b[32];
    std::snprintf(b, sizeof(b), "%08X+0x%zX", it->second, (size_t)(host - it->first));
    return b;
}

static bool GuestFuncNoAlloc(uintptr_t host, uint32_t* guest, size_t* off) {
    auto& v = FuncVec();
    auto it = std::upper_bound(v.begin(), v.end(), std::make_pair(host, 0xFFFFFFFFu));
    if (it == v.begin()) return false;
    --it;
    if (host - it->first > 0x40000) return false;
    *guest = it->second; *off = host - it->first;
    return true;
}

void GuestThread::DumpAllThreads() {
    GuestFuncFor(0); // construye la tabla antes de suspender hilos (evita bloqueos del asignador)
    auto& log = ImportLog::Get();
    HMODULE mod = GetModuleHandleA(nullptr);
    auto* dos = (IMAGE_DOS_HEADER*)mod;
    auto* nt = (IMAGE_NT_HEADERS*)((uint8_t*)mod + dos->e_lfanew);
    uintptr_t lo = (uintptr_t)mod, hi = lo + nt->OptionalHeader.SizeOfImage;
    std::vector<GuestThread*> ts;
    { std::lock_guard<std::mutex> g(g_regMu); ts = g_registry; }
    log.Printf("DumpAllThreads: %zu hilos registrados", ts.size());
    for (GuestThread* t : ts) {
        if (t->finished_ || !t->host_) continue;
        char head[384] = "";
        char frames[1024] = "";
        size_t fl = 0;
        bool have = false;
        if (SuspendThread(t->host_) == (DWORD)-1) continue;
        CONTEXT c;
        c.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
        if (GetThreadContext(t->host_, &c)) {
            have = true;
            uint32_t g = 0; size_t o = 0;
            bool r = GuestFuncNoAlloc((uintptr_t)c.Rip, &g, &o);
            std::snprintf(head, sizeof(head), "hilo guest tid=%u %s name=\"%s\" wait=%u:%08X:type%u:n%u susp=%d RIP=%08X+0x%zX(%d) guest_lr=%08X r1=%08X last_import=%s r3=%08X r4=%08X r5=%08X r6=%08X", t->id,
                          t->params_.name, t->dynName, t->waitKind, t->waitHandle, t->waitType, t->waitCount, (int)t->suspendedFlag_, g, o, (int)r, (uint32_t)t->ctx_->lr, t->ctx_->r1.u32,
                          t->lastImport ? t->lastImport : "", t->ctx_->r3.u32, t->ctx_->r4.u32, t->ctx_->r5.u32, t->ctx_->r6.u32);
            // Desenrollado exacto de la pila host (RtlVirtualUnwind) y traduccion a funciones guest.
            CONTEXT uc = c;
            uc.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
            for (int i = 0; i < 64 && fl + 24 < sizeof(frames); ++i) {
                DWORD64 imgBase = 0;
                RUNTIME_FUNCTION* rf = RtlLookupFunctionEntry(uc.Rip, &imgBase, nullptr);
                if (!rf) break;
                if (uc.Rip >= lo && uc.Rip < hi && GuestFuncNoAlloc((uintptr_t)uc.Rip, &g, &o))
                    fl += std::snprintf(frames + fl, sizeof(frames) - fl, " %08X", g);
                void* hd = nullptr; DWORD64 ef = 0;
                RtlVirtualUnwind(UNW_FLAG_NHANDLER, imgBase, uc.Rip, rf, &uc, &hd, &ef, nullptr);
                if (!uc.Rip) break;
            }
        }
        ::ResumeThread(t->host_);
        char tr[1024] = "";
        if (have && t->traceRing) {
            const TraceRing* r = (const TraceRing*)t->traceRing;
            size_t tl = 0;
            uint32_t n = r->n;
            for (uint32_t k = 0; k < 24 && k < n && tl + 22 < sizeof(tr); ++k) {
                uint32_t i = (n - 1 - k) & 1023;
                tl += std::snprintf(tr + tl, sizeof(tr) - tl, " %08X<%08X", r->target[i], r->lr[i]);
            }
        }
        if (have) {
            // Traza guest por cadena de marcos de pila (back chain en [sp]; LR guardado en [caller_sp-8]).
            uint8_t* base = GuestMemory::Base();
            char bt[1024];
            size_t bl = 0;
            uint32_t sp = t->ctx_->r1.u32;
            for (int d = 0; d < 40; ++d) {
                if (sp < t->stackLow || sp >= t->stackHigh) break;
                uint32_t next = G32(base, sp);
                if (next <= sp || next >= t->stackHigh) break;
                uint32_t lrs = G32(base, next - 8);
                bl += std::snprintf(bt + bl, sizeof(bt) - bl, " %08X", lrs);
                if (bl + 12 >= sizeof(bt)) break;
                sp = next;
            }
            bt[bl] = 0;
            log.Printf("  traza guest (LR por marco):%s", bt);
            log.Printf("  ultimas llamadas indirectas (destino<lr, mas reciente primero):%s", tr);
        }
        if (have) { log.Printf("%s", head); log.Printf("  marcos candidatos:%s", frames); }
        if (have && t->waitKind == 4) {   // espera en RtlEnterCriticalSection: propietario (X_RTL_CRITICAL_SECTION +0x18 = KTHREAD guest)
            uint8_t* gb = GuestMemory::Base();
            uint32_t cs = t->waitHandle, owner = G32(gb, cs + 0x18);
            GuestThread* ot = nullptr;
            { std::lock_guard<std::mutex> g(g_regMu); for (GuestThread* x : g_registry) if (x->kthread == owner) ot = x; }
            log.Printf("  CS %08X owner_kthread=%08X owner_tid=%u owner_name=\"%s\" owner_terminado=%d recursion=%u", cs, owner, ot ? ot->id : 0, ot ? ot->dynName : "", ot ? (int)ot->finished_ : -1, G32(gb, cs + 0x14));
        }
    }
}

} // namespace rt

namespace rt {
uint64_t GuestTimebase() {
    static LARGE_INTEGER f = [] { LARGE_INTEGER x; QueryPerformanceFrequency(&x); return x; }();
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    uint64_t q = (uint64_t)c.QuadPart, fr = (uint64_t)f.QuadPart;
    return (q / fr) * 50000000ull + (q % fr) * 50000000ull / fr;
}
} // namespace rt
