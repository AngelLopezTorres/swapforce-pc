#include "guest_heap.h"

#include "guest_memory.h"

namespace rt {

GuestHeap::GuestHeap(uint32_t b, uint32_t e, uint32_t g) : begin_(b), end_(e), gran_(g) {}

static uint32_t AlignUp(uint32_t v, uint32_t a) { return (uint32_t)(((uint64_t)v + a - 1) & ~(uint64_t)(a - 1)); }

uint32_t GuestHeap::Alloc(uint32_t size, uint32_t align, bool commit) {
    if (!size) return 0;
    if (align < gran_) align = gran_;
    uint32_t sz = AlignUp(size, 0x1000);
    std::lock_guard<std::mutex> g(mu_);
    uint64_t cur = AlignUp(begin_, align);
    for (auto it = blocks_.begin();; ++it) {
        uint64_t limit = (it == blocks_.end()) ? (uint64_t)end_ : it->first;
        if (cur + sz <= limit) {
            if (commit && !GuestMemory::Commit((uint32_t)cur, sz)) return 0;
            blocks_[(uint32_t)cur] = sz;
            return (uint32_t)cur;
        }
        if (it == blocks_.end()) return 0;
        cur = AlignUp(it->first + it->second, align);
    }
}

uint32_t GuestHeap::AllocAt(uint32_t addr, uint32_t size, bool commit) {
    uint32_t sz = AlignUp(size, 0x1000);
    if (addr < begin_ || (uint64_t)addr + sz > end_ || (addr & 0xFFF)) return 0;
    std::lock_guard<std::mutex> g(mu_);
    auto it = blocks_.lower_bound(addr);
    if (it != blocks_.end() && it->first < (uint64_t)addr + sz) return 0;
    if (it != blocks_.begin()) {
        auto p = std::prev(it);
        if ((uint64_t)p->first + p->second > addr) return 0;
    }
    if (commit && !GuestMemory::Commit(addr, sz)) return 0;
    blocks_[addr] = sz;
    return addr;
}

bool GuestHeap::Free(uint32_t addr, uint32_t* sizeOut) {
    std::lock_guard<std::mutex> g(mu_);
    auto it = blocks_.find(addr);
    if (it == blocks_.end()) return false;
    if (sizeOut) *sizeOut = it->second;
    GuestMemory::Decommit(addr, it->second);
    blocks_.erase(it);
    return true;
}

uint32_t GuestHeap::BlockSize(uint32_t addr) {
    std::lock_guard<std::mutex> g(mu_);
    auto it = blocks_.find(addr);
    return it == blocks_.end() ? 0 : it->second;
}

bool GuestHeap::Find(uint32_t addr, uint32_t* start, uint32_t* size) {
    std::lock_guard<std::mutex> g(mu_);
    auto it = blocks_.upper_bound(addr);
    if (it == blocks_.begin()) return false;
    --it;
    if ((uint64_t)it->first + it->second <= addr) return false;
    if (start) *start = it->first;
    if (size) *size = it->second;
    return true;
}

uint32_t GuestHeap::BytesInUse() {
    std::lock_guard<std::mutex> g(mu_);
    uint64_t t = 0;
    for (auto& kv : blocks_) t += kv.second;
    return (uint32_t)t;
}

GuestHeap& SystemHeap() { static GuestHeap h(0x86000000, 0x90000000, 0x1000); return h; }
GuestHeap& StackHeap() { static GuestHeap h(0x70000000, 0x7F000000, 0x10000); return h; }
GuestHeap& VirtualHeap() { static GuestHeap h(0x40000000, 0x70000000, 0x10000); return h; }
GuestHeap& PhysicalHeap() { static GuestHeap h(0xA0000000, 0xC0000000, 0x10000); return h; }

} // namespace rt
