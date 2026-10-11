// Memoria del kernel: Nt*VirtualMemory, Mm*, ExAllocatePool*, XamAlloc/Free.
// Referencia de comportamiento: Xenia xboxkrnl_memory.cc (BSD). Ningun asignador devuelve memoria
// dentro de 0x82000000-0x84E43DE8 (los heaps estan en otras regiones).
#include <cstring>

#include "guest_abi.h"
#include "guest_heap.h"
#include "import_log.h"

namespace rt {

constexpr uint32_t MEM_COMMIT_ = 0x1000, MEM_RESERVE_ = 0x2000, MEM_DECOMMIT_ = 0x4000, MEM_RELEASE_ = 0x8000;
constexpr uint32_t ST_INVALID_PARAMETER = 0xC000000D, ST_NO_MEMORY = 0xC0000017, ST_MEMORY_NOT_ALLOCATED = 0xC00000A0,
                   ST_BUFFER_TOO_SMALL = 0xC0000023;

static GuestHeap* HeapFor(uint32_t addr) {
    GuestHeap* hs[] = {&VirtualHeap(), &PhysicalHeap(), &SystemHeap(), &StackHeap()};
    for (auto* h : hs) if (h->Contains(addr)) return h;
    return nullptr;
}

static uint32_t Align4K(uint32_t v) { return (v + 0xFFF) & ~0xFFFu; }

} // namespace rt

using namespace rt;

GUEST_IMPORT_IMPL(NtAllocateVirtualMemory) {
    ImportScope sc("NtAllocateVirtualMemory", ctx, base);
    uint32_t basePtr = ctx.r3.u32, sizePtr = ctx.r4.u32, type = ctx.r5.u32;
    if (!basePtr || !sizePtr) { SetRet(ctx, ST_INVALID_PARAMETER); return; }
    uint32_t addr = G32(base, basePtr), size = G32(base, sizePtr);
    if (!size || !(type & (MEM_COMMIT_ | MEM_RESERVE_))) { SetRet(ctx, ST_INVALID_PARAMETER); return; }
    bool commit = (type & MEM_COMMIT_) != 0;
    uint32_t rsize = Align4K(size);
    if (addr == 0) {
        uint32_t a = VirtualHeap().Alloc(rsize, 0x10000, commit);
        if (!a) { SetRet(ctx, ST_NO_MEMORY); return; }
        P32(base, basePtr, a);
        P32(base, sizePtr, rsize);
        SetRet(ctx, ST_SUCCESS);
        return;
    }
    GuestHeap* h = HeapFor(addr);
    if (!h) { SetRet(ctx, ST_INVALID_PARAMETER); return; }
    uint32_t start, bsize;
    uint32_t pageAddr = addr & ~0xFFFu;
    if (h->Find(pageAddr, &start, &bsize)) {
        // Subrango de un bloque existente: confirmar paginas si se pide MEM_COMMIT.
        if ((uint64_t)pageAddr + rsize > (uint64_t)start + bsize) { SetRet(ctx, ST_INVALID_PARAMETER); return; }
        if (commit && !GuestMemory::Commit(pageAddr, rsize)) { SetRet(ctx, ST_NO_MEMORY); return; }
    } else if (!h->AllocAt(pageAddr, rsize, commit)) {
        SetRet(ctx, ST_NO_MEMORY); return;
    }
    P32(base, basePtr, pageAddr);
    P32(base, sizePtr, rsize);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtFreeVirtualMemory) {
    ImportScope sc("NtFreeVirtualMemory", ctx, base);
    uint32_t basePtr = ctx.r3.u32, sizePtr = ctx.r4.u32, type = ctx.r5.u32;
    if (!basePtr) { SetRet(ctx, ST_INVALID_PARAMETER); return; }
    uint32_t addr = G32(base, basePtr);
    uint32_t size = sizePtr ? G32(base, sizePtr) : 0;
    GuestHeap* h = HeapFor(addr);
    if (!h) { SetRet(ctx, ST_MEMORY_NOT_ALLOCATED); return; }
    if (type & MEM_DECOMMIT_) {
        if (!size) { SetRet(ctx, ST_INVALID_PARAMETER); return; }
        GuestMemory::Decommit(addr & ~0xFFFu, Align4K(size));
        SetRet(ctx, ST_SUCCESS);
        return;
    }
    uint32_t freed = 0;
    if (!h->Free(addr, &freed)) { SetRet(ctx, ST_MEMORY_NOT_ALLOCATED); return; }
    P32(base, basePtr, addr);
    if (sizePtr) P32(base, sizePtr, freed);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(NtQueryVirtualMemory) {
    ImportScope sc("NtQueryVirtualMemory", ctx, base);
    uint32_t addr = ctx.r3.u32, info = ctx.r4.u32;
    if (!info) { SetRet(ctx, ST_INVALID_PARAMETER); return; }
    uint32_t start = 0, size = 0;
    bool found = false;
    if (addr >= kImageBase && addr < kFuncTableEnd) { start = kImageBase; size = kFuncTableEnd - kImageBase; found = true; }
    else if (GuestHeap* h = HeapFor(addr)) found = h->Find(addr, &start, &size);
    if (!found) { SetRet(ctx, ST_INVALID_PARAMETER); return; }
    P32(base, info + 0, addr & ~0xFFFu); // base_address
    P32(base, info + 4, start);          // allocation_base
    P32(base, info + 8, 4);              // allocation_protect = PAGE_READWRITE
    P32(base, info + 12, start + size - (addr & ~0xFFFu)); // region_size
    P32(base, info + 16, MEM_COMMIT_);   // state
    P32(base, info + 20, 4);             // protect
    P32(base, info + 24, 0x20000);       // type = MEM_PRIVATE
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(MmAllocatePhysicalMemoryEx) {
    ImportScope sc("MmAllocatePhysicalMemoryEx", ctx, base);
    uint32_t size = ctx.r4.u32, align = ctx.r8.u32;
    if (!size) { SetRet(ctx, 0); return; }
    SetRet(ctx, PhysicalHeap().Alloc(Align4K(size), align ? align : 0x10000));
}

GUEST_IMPORT_IMPL(MmFreePhysicalMemory) {
    ImportScope sc("MmFreePhysicalMemory", ctx, base);
    PhysicalHeap().Free(ctx.r4.u32);
}

GUEST_IMPORT_IMPL(MmQueryAllocationSize) {
    ImportScope sc("MmQueryAllocationSize", ctx, base);
    GuestHeap* h = HeapFor(ctx.r3.u32);
    uint32_t start = 0, size = 0;
    SetRet(ctx, (h && h->Find(ctx.r3.u32, &start, &size)) ? size : 0);
}

GUEST_IMPORT_IMPL(MmGetPhysicalAddress) {
    ImportScope sc("MmGetPhysicalAddress", ctx, base);
    uint32_t a = ctx.r3.u32;
    SetRet(ctx, a >= 0xA0000000 ? (a & 0x1FFFFFFF) : a);
}

GUEST_IMPORT_IMPL(MmQueryAddressProtect) {
    ImportScope sc("MmQueryAddressProtect", ctx, base);
    uint32_t a = ctx.r3.u32, s, z;
    bool ok = (a >= kImageBase && a < kFuncTableEnd) || (HeapFor(a) && HeapFor(a)->Find(a, &s, &z));
    SetRet(ctx, ok ? 4 : 0); // PAGE_READWRITE
}

GUEST_IMPORT_IMPL(MmQueryStatistics) {
    ImportScope sc("MmQueryStatistics", ctx, base);
    uint32_t p = ctx.r3.u32;
    if (!p || G32(base, p) != 104) { SetRet(ctx, ST_BUFFER_TOO_SMALL); return; }
    std::memset(base + p + 4, 0, 100);
    P32(base, p + 4, 0x00020000); // total_physical_pages (512 MB / 4 KB), Xenia
    P32(base, p + 8, 0x300);      // kernel_pages, Xenia
    SetRet(ctx, ST_SUCCESS);
}

static void PoolAlloc(PPCContext& ctx, uint32_t size) {
    if (!size) { SetRet(ctx, 0); return; }
    SetRet(ctx, SystemHeap().Alloc(size < 0x1000 ? 0x1000 : size, 0x1000));
}

GUEST_IMPORT_IMPL(ExAllocatePoolTypeWithTag) {
    ImportScope sc("ExAllocatePoolTypeWithTag", ctx, base);
    PoolAlloc(ctx, ctx.r3.u32);
}
GUEST_IMPORT_IMPL(ExAllocatePoolWithTag) {
    ImportScope sc("ExAllocatePoolWithTag", ctx, base);
    PoolAlloc(ctx, ctx.r3.u32);
}
GUEST_IMPORT_IMPL(ExFreePool) {
    ImportScope sc("ExFreePool", ctx, base);
    SystemHeap().Free(ctx.r3.u32);
}
GUEST_IMPORT_IMPL(XamAlloc) {
    ImportScope sc("XamAlloc", ctx, base);
    // [NV] firma tomada de Xenia: XamAlloc(flags, size, out_ptr) -> *out_ptr = bloque
    uint32_t size = ctx.r4.u32, out = ctx.r5.u32;
    uint32_t a = size ? SystemHeap().Alloc(size < 0x1000 ? 0x1000 : size, 0x1000) : 0;
    if (out) P32(base, out, a);
    SetRet(ctx, a ? ST_SUCCESS : ST_NO_MEMORY);
}
GUEST_IMPORT_IMPL(XamFree) {
    ImportScope sc("XamFree", ctx, base);
    SystemHeap().Free(ctx.r3.u32);
    SetRet(ctx, ST_SUCCESS);
}
