// Hilos y TLS: ExCreateThread, NtResumeThread, Ke*Thread, KeTls*, ExTerminateThread.
#include <atomic>
#include <memory>
#include <mutex>

#include "guest_abi.h"
#include "guest_heap.h"
#include "guest_thread.h"
#include "import_log.h"
#include "kobject.h"
#include "xex_header.h"

using namespace rt;

namespace {

std::mutex g_tlsMu;
bool g_tlsUsed[0x40]; // 0x40 slots (cabecera TLS del XEX)

std::shared_ptr<KObject> ThreadFromPtr(uint32_t p) {
    return HandleTable::Get().FromNative(p);
}

} // namespace

GUEST_IMPORT_IMPL(ExCreateThread) {
    ImportScope sc("ExCreateThread", ctx, base);
    // (HANDLE* out, StackSize, ThreadId*, XapiThreadStartup, StartAddress, StartContext, CreationFlags)
    uint32_t hOut = ctx.r3.u32, stack = ctx.r4.u32, idOut = ctx.r5.u32, xapi = ctx.r6.u32, start = ctx.r7.u32,
             startCtx = ctx.r8.u32, flags = ctx.r9.u32;
    GuestThreadParams p;
    p.stackSize = stack;
    p.suspended = (flags & 1) != 0; // CREATE_SUSPENDED
    p.name = "ExCreateThread";
    uint32_t r3 = startCtx, r4 = 0;
    if (xapi) { p.entry = xapi; r3 = start; r4 = startCtx; } // XapiThreadStartup(StartAddress, StartContext)
    else p.entry = start;
    p.arg = r3;
    std::string err;
    GuestThread* t = GuestThread::Create(p, &err);
    if (!t) { SetRet(ctx, 0xC0000017); return; }
    t->Ctx().r4.u64 = r4;
    auto o = std::make_shared<KObject>(KType::Thread);
    o->thread = t;
    o->ownsHost = false;
    if (!t->Start()) { SetRet(ctx, 0xC0000017); return; }
    o->host = t->HostHandle();
    HandleTable::Get().SetNativePtr(o, t->kthread);
    if (!p.suspended) t->Resume();
    ImportLog::Get().Printf("  ExCreateThread -> tid=%u entry=%08X arg=%08X flags=%X", t->id, p.entry, p.arg, flags);
    if (idOut) P32(base, idOut, t->id);
    uint32_t th = HandleTable::Get().Add(o);
    ImportLog::Get().Printf("  ExCreateThread handle=%08X", th);
    if (hOut) P32(base, hOut, th);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtResumeThread) {
    ImportScope sc("NtResumeThread", ctx, base);
    auto o = HandleTable::Get().Lookup(ctx.r3.u32);
    if (!o || o->type != KType::Thread || !o->thread) { SetRet(ctx, ST_INVALID_HANDLE); return; }
    uint32_t prev = o->thread->ResumeThread();
    if (ctx.r4.u32) P32(base, ctx.r4.u32, prev);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(ExTerminateThread) {
    ImportScope sc("ExTerminateThread", ctx, base);
    ImportLog::Get().Printf("  ExTerminateThread codigo=%08X", ctx.r3.u32);
    ExitThread(ctx.r3.u32);
}

GUEST_IMPORT_IMPL(KeSetBasePriorityThread) { ImportScope sc("KeSetBasePriorityThread", ctx, base); SetRet(ctx, 0); }
GUEST_IMPORT_IMPL(KeQueryBasePriorityThread) { ImportScope sc("KeQueryBasePriorityThread", ctx, base); SetRet(ctx, 0); }
GUEST_IMPORT_IMPL(KeSetAffinityThread) { ImportScope sc("KeSetAffinityThread", ctx, base); SetRet(ctx, 0); }

GUEST_IMPORT_IMPL(KeGetCurrentProcessType) {
    ImportScope sc("KeGetCurrentProcessType", ctx, base);
    SetRet(ctx, 1); // X_PROCTYPE_USER (Xenia)
}

GUEST_IMPORT_IMPL(XexCheckExecutablePrivilege) {
    ImportScope sc("XexCheckExecutablePrivilege", ctx, base);
    uint32_t flags = GuestMemory::Read32(kXexHeaderGuest + 4);
    uint32_t bit = ctx.r3.u32;
    SetRet(ctx, bit < 32 && (flags & (1u << bit)) ? 1 : 0);
}

// ---- TLS dinamico (KeTls*) sobre los slots guest del hilo actual ----
GUEST_IMPORT_IMPL(KeTlsAlloc) {
    ImportScope sc("KeTlsAlloc", ctx, base);
    std::lock_guard<std::mutex> g(g_tlsMu);
    for (uint32_t i = 0; i < 0x40; ++i)
        if (!g_tlsUsed[i]) { g_tlsUsed[i] = true; SetRet(ctx, i); return; }
    SetRet(ctx, 0xFFFFFFFF);
}

GUEST_IMPORT_IMPL(KeTlsFree) {
    ImportScope sc("KeTlsFree", ctx, base);
    uint32_t i = ctx.r3.u32;
    std::lock_guard<std::mutex> g(g_tlsMu);
    if (i < 0x40 && g_tlsUsed[i]) { g_tlsUsed[i] = false; SetRet(ctx, 1); }
    else SetRet(ctx, 0);
}

GUEST_IMPORT_IMPL(KeTlsGetValue) {
    ImportScope sc("KeTlsGetValue", ctx, base);
    GuestThread* t = GuestThread::Current();
    uint32_t i = ctx.r3.u32;
    SetRet(ctx, (t && i < 0x40) ? G32(base, t->tlsDynamic + i * 4) : 0);
}

GUEST_IMPORT_IMPL(KeTlsSetValue) {
    ImportScope sc("KeTlsSetValue", ctx, base);
    GuestThread* t = GuestThread::Current();
    uint32_t i = ctx.r3.u32;
    if (t && i < 0x40) { P32(base, t->tlsDynamic + i * 4, ctx.r4.u32); SetRet(ctx, 1); }
    else SetRet(ctx, 0);
}
