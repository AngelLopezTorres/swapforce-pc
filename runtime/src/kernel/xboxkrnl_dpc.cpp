// DPC: KeInitializeDpc / KeInsertQueueDpc / KeRemoveQueueDpc con un hilo DPC.
// Layout de XDPC y de KeInitializeDpc: Xenia xboxkrnl_threading.cc (copia en runtime\ref\); Xenia NO despacha DPC
// ("DPC does not dispatch yet; going to hang!"), el despacho es propio. Firma de la rutina diferida:
// VOID KDEFERRED_ROUTINE(PKDPC Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2) (Microsoft Learn, wdm.h).
#include <windows.h>

#include <atomic>
#include <deque>
#include <mutex>
#include <set>

#include "guest_abi.h"
#include "guest_thread.h"
#include "import_log.h"

using namespace rt;

namespace {

std::mutex g_mu;
std::deque<uint32_t> g_queue;
std::set<uint32_t> g_queued;
HANDLE g_event = nullptr, g_thread = nullptr;
std::atomic<uint64_t> g_executed{0};

DWORD WINAPI DpcMain(void*) {
    GuestThread* t = GuestThread::CreateAttached("dpc");
    if (!t) return 1;
    t->Attach();
    t->SetCpu(1);
    uint8_t* base = GuestMemory::Base();
    for (;;) {
        WaitForSingleObject(g_event, INFINITE);
        for (;;) {
            uint32_t dpc;
            {
                std::lock_guard<std::mutex> g(g_mu);
                if (g_queue.empty()) break;
                dpc = g_queue.front();
                g_queue.pop_front();
                g_queued.erase(dpc);
            }
            uint32_t routine = G32(base, dpc + 0x0C), context = G32(base, dpc + 0x10);
            uint32_t a1 = G32(base, dpc + 0x14), a2 = G32(base, dpc + 0x18);
            uint64_t n = ++g_executed;
            if (n <= 10) ImportLog::Get().Printf("DPC %08X routine=%08X ctx=%08X a1=%08X a2=%08X", dpc, routine, context, a1, a2);
            if (routine) t->CallGuest(routine, dpc, context, nullptr, a1, a2);
        }
    }
}

} // namespace

GUEST_IMPORT_IMPL(KeInitializeDpc) {
    ImportScope sc("KeInitializeDpc", ctx, base);
    uint32_t dpc = ctx.r3.u32;
    if (!dpc) return;
    P32(base, dpc + 0x00, (19u << 24)); // type = DpcObject, importance 0, number 0 (Xenia)
    P32(base, dpc + 0x04, 0);
    P32(base, dpc + 0x08, 0);
    P32(base, dpc + 0x0C, ctx.r4.u32);  // routine
    P32(base, dpc + 0x10, ctx.r5.u32);  // context
    P32(base, dpc + 0x14, 0);
    P32(base, dpc + 0x18, 0);
}

GUEST_IMPORT_IMPL(KeInsertQueueDpc) {
    ImportScope sc("KeInsertQueueDpc", ctx, base);
    uint32_t dpc = ctx.r3.u32;
    {
        std::lock_guard<std::mutex> g(g_mu);
        if (!g_event) {
            g_event = CreateEventA(nullptr, FALSE, FALSE, nullptr);
            g_thread = CreateThread(nullptr, 0, DpcMain, nullptr, 0, nullptr);
        }
        if (!g_queued.insert(dpc).second) { SetRet(ctx, 0); return; }
        P32(base, dpc + 0x14, ctx.r4.u32);
        P32(base, dpc + 0x18, ctx.r5.u32);
        g_queue.push_back(dpc);
    }
    SetEvent(g_event);
    SetRet(ctx, 1);
}

GUEST_IMPORT_IMPL(KeRemoveQueueDpc) {
    ImportScope sc("KeRemoveQueueDpc", ctx, base);
    uint32_t dpc = ctx.r3.u32;
    std::lock_guard<std::mutex> g(g_mu);
    bool r = g_queued.erase(dpc) != 0;
    if (r) for (auto it = g_queue.begin(); it != g_queue.end(); ++it) if (*it == dpc) { g_queue.erase(it); break; }
    SetRet(ctx, r ? 1 : 0);
}
