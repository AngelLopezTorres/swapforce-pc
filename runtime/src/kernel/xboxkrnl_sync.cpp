// Sincronizacion: eventos, semaforos, mutantes, esperas, secciones criticas, spinlocks, tiempo.
// Semantica: Windows (NT) documentada en Microsoft Learn; layout guest de Xbox 360 segun Xenia.
#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

#include "guest_abi.h"
#include "guest_heap.h"
#include "guest_thread.h"
#include "import_log.h"
#include "kobject.h"

using namespace rt;

namespace {
// Marca RAII de espera del hilo guest actual (volcado de hilos, M0).
struct WaitMark {
    GuestThread* t;
    WaitMark(uint32_t kind, uint32_t h, uint32_t type, uint32_t n) : t(GuestThread::Current()) {
        if (t) { t->waitHandle = h; t->waitType = type; t->waitCount = n; t->waitKind = kind; }
    }
    ~WaitMark() { if (t) { t->waitKind = 0; } }
};

constexpr uint32_t ST_TIMEOUT = 0x102;
constexpr uint32_t ST_INVALID_PARAMETER = 0xC000000D;
constexpr uint32_t ST_ABANDONED = 0x80;

std::shared_ptr<KObject> ObjFromHandle(uint32_t h) {
    if (h == kCurrentThreadPseudoHandle) {
        GuestThread* t = GuestThread::Current();
        if (!t) return nullptr;
        auto o = std::make_shared<KObject>(KType::Thread);
        o->host = t->HostHandle();
        o->ownsHost = false;
        o->thread = t;
        return o;
    }
    return HandleTable::Get().Lookup(h);
}

// Convierte un puntero a LARGE_INTEGER guest (100 ns; negativo = relativo) en milisegundos host.
DWORD TimeoutMs(uint8_t* base, uint32_t ptr) {
    if (!ptr) return INFINITE;
    int64_t v = (int64_t)G64(base, ptr);
    if (v < 0) {
        uint64_t ms = ((uint64_t)(-v) + 9999) / 10000;
        return ms >= INFINITE ? INFINITE - 1 : (DWORD)ms;
    }
    if (v == 0) return 0;
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    int64_t now = ((int64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    if (v <= now) return 0;
    return (DWORD)(((uint64_t)(v - now)) / 10000);
}

uint32_t WaitResult(DWORD r, uint32_t count) {
    if (r == WAIT_TIMEOUT) return ST_TIMEOUT;
    if (r >= WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + count) return r - WAIT_OBJECT_0;
    if (r >= WAIT_ABANDONED_0 && r < WAIT_ABANDONED_0 + count) return ST_ABANDONED + (r - WAIT_ABANDONED_0);
    return ST_INVALID_PARAMETER;
}

uint32_t WaitOne(const std::shared_ptr<KObject>& o, DWORD ms) {
    if (!o || !o->host) return 0xC0000008;
    return WaitResult(WaitForSingleObject(o->host, ms), 1);
}

uint32_t WaitMany(std::vector<std::shared_ptr<KObject>>& objs, bool all, DWORD ms) {
    std::vector<HANDLE> hs;
    for (auto& o : objs) {
        if (!o || !o->host) return 0xC0000008;
        hs.push_back(o->host);
    }
    if (hs.size() > MAXIMUM_WAIT_OBJECTS) return ST_INVALID_PARAMETER;
    return WaitResult(WaitForMultipleObjects((DWORD)hs.size(), hs.data(), all, ms), (uint32_t)hs.size());
}

std::shared_ptr<KObject> NewEvent(bool manual, bool initial) {
    auto o = std::make_shared<KObject>(KType::Event);
    o->manualReset = manual;
    o->host = CreateEventA(nullptr, manual, initial, nullptr);
    return o;
}

} // namespace

// ---------------- Eventos ----------------
GUEST_IMPORT_IMPL(NtCreateEvent) {
    ImportScope sc("NtCreateEvent", ctx, base);
    // (HANDLE* out, OBJECT_ATTRIBUTES*, EventType (0 = notificacion/manual, 1 = sincronizacion/auto), InitialState)
    auto o = NewEvent(ctx.r5.u32 == 0, ctx.r6.u32 != 0);
    if (!o->host) { SetRet(ctx, 0xC0000001); return; }
    uint32_t h = HandleTable::Get().Add(o);
    ImportLog::Get().Printf("  NtCreateEvent handle=%08X manual=%d initial=%u", h, (int)o->manualReset, ctx.r6.u32);
    if (ctx.r3.u32) P32(base, ctx.r3.u32, h);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtSetEvent) {
    ImportScope sc("NtSetEvent", ctx, base);
    auto o = ObjFromHandle(ctx.r3.u32);
    if (!o || o->type != KType::Event) { SetRet(ctx, ST_INVALID_HANDLE); return; }
    uint32_t prev = WaitForSingleObject(o->host, 0) == WAIT_OBJECT_0 ? 1 : 0;
    if (prev && !o->manualReset) SetEvent(o->host); // la sonda consumio la senal de un evento auto
    SetEvent(o->host);
    if (ctx.r4.u32) P32(base, ctx.r4.u32, prev);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtClearEvent) {
    ImportScope sc("NtClearEvent", ctx, base);
    auto o = ObjFromHandle(ctx.r3.u32);
    if (!o || o->type != KType::Event) { SetRet(ctx, ST_INVALID_HANDLE); return; }
    ResetEvent(o->host);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtPulseEvent) {
    ImportScope sc("NtPulseEvent", ctx, base);
    auto o = ObjFromHandle(ctx.r3.u32);
    if (!o || o->type != KType::Event) { SetRet(ctx, ST_INVALID_HANDLE); return; }
    PulseEvent(o->host);
    if (ctx.r4.u32) P32(base, ctx.r4.u32, 0);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(KeSetEvent) {
    ImportScope sc("KeSetEvent", ctx, base);
    auto o = HandleTable::Get().FromNative(ctx.r3.u32);
    if (!o || o->type != KType::Event) { SetRet(ctx, 0); return; }
    uint32_t prev = WaitForSingleObject(o->host, 0) == WAIT_OBJECT_0 ? 1 : 0;
    SetEvent(o->host);
    SetRet(ctx, prev);
}

GUEST_IMPORT_IMPL(KeResetEvent) {
    ImportScope sc("KeResetEvent", ctx, base);
    auto o = HandleTable::Get().FromNative(ctx.r3.u32);
    if (!o || o->type != KType::Event) { SetRet(ctx, 0); return; }
    uint32_t prev = o->manualReset && WaitForSingleObject(o->host, 0) == WAIT_OBJECT_0 ? 1 : 0;
    ResetEvent(o->host);
    SetRet(ctx, prev);
}

// ---------------- Semaforos y mutantes ----------------
GUEST_IMPORT_IMPL(NtCreateSemaphore) {
    ImportScope sc("NtCreateSemaphore", ctx, base);
    // (HANDLE* out, OBJECT_ATTRIBUTES*, InitialCount, MaximumCount)
    auto o = std::make_shared<KObject>(KType::Semaphore);
    o->host = CreateSemaphoreA(nullptr, (LONG)ctx.r5.u32, (LONG)ctx.r6.u32, nullptr);
    if (!o->host) { SetRet(ctx, ST_INVALID_PARAMETER); return; }
    uint32_t h = HandleTable::Get().Add(o);
    ImportLog::Get().Printf("  NtCreateSemaphore handle=%08X initial=%u max=%u", h, ctx.r5.u32, ctx.r6.u32);
    if (ctx.r3.u32) P32(base, ctx.r3.u32, h);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtReleaseSemaphore) {
    ImportScope sc("NtReleaseSemaphore", ctx, base);
    auto o = ObjFromHandle(ctx.r3.u32);
    if (!o || o->type != KType::Semaphore) { SetRet(ctx, ST_INVALID_HANDLE); return; }
    LONG prev = 0;
    if (!ReleaseSemaphore(o->host, (LONG)ctx.r4.u32, &prev)) { SetRet(ctx, 0xC0000047); return; } // STATUS_SEMAPHORE_LIMIT_EXCEEDED
    if (ctx.r5.u32) P32(base, ctx.r5.u32, (uint32_t)prev);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtCreateMutant) {
    ImportScope sc("NtCreateMutant", ctx, base);
    // (HANDLE* out, OBJECT_ATTRIBUTES*, InitialOwner)
    auto o = std::make_shared<KObject>(KType::Mutant);
    o->host = CreateMutexA(nullptr, ctx.r5.u32 != 0, nullptr);
    if (!o->host) { SetRet(ctx, 0xC0000001); return; }
    uint32_t h = HandleTable::Get().Add(o);
    ImportLog::Get().Printf("  NtCreateMutant handle=%08X", h);
    if (ctx.r3.u32) P32(base, ctx.r3.u32, h);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtReleaseMutant) {
    ImportScope sc("NtReleaseMutant", ctx, base);
    auto o = ObjFromHandle(ctx.r3.u32);
    if (!o || o->type != KType::Mutant) { SetRet(ctx, ST_INVALID_HANDLE); return; }
    if (!ReleaseMutex(o->host)) { SetRet(ctx, 0xC0000046); return; } // STATUS_MUTANT_NOT_OWNED
    if (ctx.r4.u32) P32(base, ctx.r4.u32, 0);
    SetRet(ctx, ST_SUCCESS);
}

// ---------------- Esperas ----------------
GUEST_IMPORT_IMPL(NtWaitForSingleObjectEx) {
    ImportScope sc("NtWaitForSingleObjectEx", ctx, base);
    // (handle, WaitMode, Alertable, Timeout*)
    auto o = ObjFromHandle(ctx.r3.u32);
    WaitMark wm(1, ctx.r3.u32, o ? (uint32_t)o->type : 99, 1);
    SetRet(ctx, WaitOne(o, TimeoutMs(base, ctx.r6.u32)));
}

GUEST_IMPORT_IMPL(KeWaitForSingleObject) {
    ImportScope sc("KeWaitForSingleObject", ctx, base);
    // (objeto guest, WaitReason, WaitMode, Alertable, Timeout*)
    auto o = HandleTable::Get().FromNative(ctx.r3.u32);
    WaitMark wm(2, ctx.r3.u32, o ? (uint32_t)o->type : 99, 1);
    SetRet(ctx, WaitOne(o, TimeoutMs(base, ctx.r7.u32)));
}

GUEST_IMPORT_IMPL(KeWaitForMultipleObjects) {
    ImportScope sc("KeWaitForMultipleObjects", ctx, base);
    // (Count, Objects**, WaitType (0 = todos, 1 = cualquiera), WaitReason, WaitMode, Alertable, Timeout*, WaitBlockArray)
    uint32_t count = ctx.r3.u32, arr = ctx.r4.u32;
    bool all = ctx.r5.u32 == 0;
    std::vector<std::shared_ptr<KObject>> objs;
    for (uint32_t i = 0; i < count; ++i) objs.push_back(HandleTable::Get().FromNative(G32(base, arr + i * 4)));
    WaitMark wm(3, count ? G32(base, arr) : 0, (count && objs[0]) ? (uint32_t)objs[0]->type : 99, count);
    SetRet(ctx, WaitMany(objs, all, TimeoutMs(base, Arg32(ctx, base, 6))));
}

GUEST_IMPORT_IMPL(KeDelayExecutionThread) {
    ImportScope sc("KeDelayExecutionThread", ctx, base);
    // (WaitMode, Alertable, Interval*)
    DWORD ms = TimeoutMs(base, ctx.r5.u32);
    if (ms > 2000) ImportLog::Get().Printf("  KeDelayExecutionThread tid=%u lr=%08X intervalo=%016llX (%u ms)", GuestThread::Current() ? GuestThread::Current()->id : 0, (uint32_t)ctx.lr, (unsigned long long)G64(base, ctx.r5.u32), (unsigned)ms);
    Sleep(ms == INFINITE ? 0xFFFFFFF : ms);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtYieldExecution) {
    ImportScope sc("NtYieldExecution", ctx, base);
    SwitchToThread();
    SetRet(ctx, ST_SUCCESS);
}

// ---------------- Handles / objetos ----------------
GUEST_IMPORT_IMPL(NtClose) {
    ImportScope sc("NtClose", ctx, base);
    SetRet(ctx, HandleTable::Get().Close(ctx.r3.u32) ? ST_SUCCESS : ST_INVALID_HANDLE);
}

GUEST_IMPORT_IMPL(NtDuplicateObject) {
    ImportScope sc("NtDuplicateObject", ctx, base);
    // (handle, HANDLE* new, Options)
    uint32_t h = ctx.r3.u32;
    uint32_t n = 0;
    if (h == kCurrentThreadPseudoHandle) {
        auto o = ObjFromHandle(h);
        if (o) { o->ownsHost = false; n = HandleTable::Get().Add(o); }
    } else {
        n = HandleTable::Get().Duplicate(h);
    }
    if (!n) { SetRet(ctx, ST_INVALID_HANDLE); return; }
    if (ctx.r4.u32) P32(base, ctx.r4.u32, n);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(ObReferenceObjectByHandle) {
    ImportScope sc("ObReferenceObjectByHandle", ctx, base);
    // (handle, ObjectType*, void** out)
    auto o = ObjFromHandle(ctx.r3.u32);
    if (!o) { SetRet(ctx, ST_INVALID_HANDLE); return; }
    uint32_t p = HandleTable::Get().NativePtr(o);
    if (!p) { SetRet(ctx, 0xC0000017); return; }
    if (ctx.r5.u32) P32(base, ctx.r5.u32, p);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(ObDereferenceObject) { ImportScope sc("ObDereferenceObject", ctx, base); }
GUEST_IMPORT_IMPL(ObReferenceObject) { ImportScope sc("ObReferenceObject", ctx, base); }

// ---------------- Secciones criticas (Rtl*) ----------------
namespace {

struct CritSec {
    std::recursive_mutex m;
    int recursion = 0;
};

std::mutex g_csMu;
std::vector<CritSec*> g_cs; // el indice (+1) se guarda en +0x04 de la X_RTL_CRITICAL_SECTION guest

CritSec* GetCs(uint8_t* base, uint32_t p, bool create) {
    uint32_t idx = G32(base, p + 4);
    if (idx && idx <= g_cs.size() && G32(base, p + 8) == 0x43530001) return g_cs[idx - 1];
    if (!create) return nullptr;
    std::lock_guard<std::mutex> g(g_csMu);
    idx = G32(base, p + 4);
    if (idx && idx <= g_cs.size() && G32(base, p + 8) == 0x43530001) return g_cs[idx - 1];
    g_cs.push_back(new CritSec());
    P32(base, p + 4, (uint32_t)g_cs.size());
    P32(base, p + 8, 0x43530001);
    P32(base, p + 0x10, 0xFFFFFFFF); // lock_count = -1
    P32(base, p + 0x14, 0);          // recursion_count
    P32(base, p + 0x18, 0);          // owning_thread
    return g_cs.back();
}

void InitCs(PPCContext& ctx, uint8_t* base) {
    uint32_t p = ctx.r3.u32;
    std::memset(base + p, 0, 0x1C);
    GetCs(base, p, true);
    SetRet(ctx, ST_SUCCESS);
}

} // namespace

GUEST_IMPORT_IMPL(RtlInitializeCriticalSection) {
    ImportScope sc("RtlInitializeCriticalSection", ctx, base);
    InitCs(ctx, base);
}

GUEST_IMPORT_IMPL(RtlInitializeCriticalSectionAndSpinCount) {
    ImportScope sc("RtlInitializeCriticalSectionAndSpinCount", ctx, base);
    InitCs(ctx, base);
}

GUEST_IMPORT_IMPL(RtlEnterCriticalSection) {
    ImportScope sc("RtlEnterCriticalSection", ctx, base);
    uint32_t p = ctx.r3.u32;
    CritSec* c = GetCs(base, p, true);
    if (!c->m.try_lock()) { WaitMark wm(4, p, 98, 1); c->m.lock(); }   // 4 = seccion critica (M0: el volcado muestra el propietario)
    ++c->recursion;
    P32(base, p + 0x14, (uint32_t)c->recursion);
    P32(base, p + 0x10, (uint32_t)(c->recursion - 1));
    P32(base, p + 0x18, GuestThread::Current() ? GuestThread::Current()->kthread : 0);
}

GUEST_IMPORT_IMPL(RtlLeaveCriticalSection) {
    ImportScope sc("RtlLeaveCriticalSection", ctx, base);
    uint32_t p = ctx.r3.u32;
    CritSec* c = GetCs(base, p, false);
    if (!c) return;
    --c->recursion;
    P32(base, p + 0x14, (uint32_t)c->recursion);
    P32(base, p + 0x10, (uint32_t)(c->recursion - 1));
    if (c->recursion == 0) P32(base, p + 0x18, 0);
    c->m.unlock();
}

GUEST_IMPORT_IMPL(RtlTryEnterCriticalSection) {
    ImportScope sc("RtlTryEnterCriticalSection", ctx, base);
    uint32_t p = ctx.r3.u32;
    CritSec* c = GetCs(base, p, true);
    if (!c->m.try_lock()) { SetRet(ctx, 0); return; }
    ++c->recursion;
    P32(base, p + 0x14, (uint32_t)c->recursion);
    SetRet(ctx, 1);
}

// ---------------- Spinlocks y regiones criticas ----------------
GUEST_IMPORT_IMPL(KfAcquireSpinLock) {
    ImportScope sc("KfAcquireSpinLock", ctx, base);
    auto* lock = reinterpret_cast<volatile LONG*>(base + ctx.r3.u32);
    while (InterlockedCompareExchange(lock, 1, 0) != 0) YieldProcessor();
    SetRet(ctx, 0); // IRQL anterior
}
GUEST_IMPORT_IMPL(KfReleaseSpinLock) {
    ImportScope sc("KfReleaseSpinLock", ctx, base);
    InterlockedExchange(reinterpret_cast<volatile LONG*>(base + ctx.r3.u32), 0);
}
GUEST_IMPORT_IMPL(KeAcquireSpinLockAtRaisedIrql) {
    ImportScope sc("KeAcquireSpinLockAtRaisedIrql", ctx, base);
    auto* lock = reinterpret_cast<volatile LONG*>(base + ctx.r3.u32);
    while (InterlockedCompareExchange(lock, 1, 0) != 0) YieldProcessor();
}
GUEST_IMPORT_IMPL(KeReleaseSpinLockFromRaisedIrql) {
    ImportScope sc("KeReleaseSpinLockFromRaisedIrql", ctx, base);
    InterlockedExchange(reinterpret_cast<volatile LONG*>(base + ctx.r3.u32), 0);
}
GUEST_IMPORT_IMPL(KeEnterCriticalRegion) { ImportScope sc("KeEnterCriticalRegion", ctx, base); }
GUEST_IMPORT_IMPL(KeLeaveCriticalRegion) { ImportScope sc("KeLeaveCriticalRegion", ctx, base); }

// ---------------- Tiempo ----------------
GUEST_IMPORT_IMPL(KeQuerySystemTime) {
    ImportScope sc("KeQuerySystemTime", ctx, base);
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    if (ctx.r3.u32) P64(base, ctx.r3.u32, ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime);
}

GUEST_IMPORT_IMPL(KeQueryPerformanceFrequency) {
    ImportScope sc("KeQueryPerformanceFrequency", ctx, base);
    ctx.r3.u64 = 50000000; // 50 MHz (valor de Xenia; frecuencia del timebase del 360)
}

