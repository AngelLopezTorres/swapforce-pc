#include "xma_context.h"

#include <array>
#include <cstring>
#include <mutex>

#include "guest_abi.h"
#include "guest_heap.h"
#include "guest_memory.h"

namespace rt::audio {
namespace {

constexpr uint32_t kPhysicalMask = 0x1FFFFFFF;
constexpr uint32_t kXmaRegisterBase = 0x7FEA0000;
constexpr uint32_t kXmaRegisterSize = 0x10000;

std::mutex g_mu;
uint32_t g_contextArray = 0;
std::array<bool, kXmaContextCount> g_allocated{};
uint64_t g_decodeRequests = 0;

uint32_t ContextAddress(uint32_t index) {
    return g_contextArray + index * kXmaContextBytes;
}

bool InitializeLocked() {
    if (g_contextArray) return true;
    uint8_t* base = GuestMemory::Base();
    if (!base) return false;

    uint32_t address = PhysicalHeap().Alloc(kXmaContextCount * kXmaContextBytes, 256);
    if (!address) return false;

    g_contextArray = address;
    P32(base, kXmaContextArrayAddressRegister, address & kPhysicalMask);
    P32(base, kXmaCurrentContextIndexRegister, 0);
    P32(base, kXmaNextContextIndexRegister, 1);
    return true;
}

bool FindContextLocked(uint32_t address, uint32_t& index) {
    if (!g_contextArray || address < g_contextArray) return false;
    uint32_t delta = address - g_contextArray;
    if ((delta % kXmaContextBytes) != 0) return false;
    index = delta / kXmaContextBytes;
    return index < kXmaContextCount && g_allocated[index];
}

} // namespace

uint32_t CreateXmaContext() {
    std::lock_guard<std::mutex> lock(g_mu);
    if (!InitializeLocked()) return 0;
    uint8_t* base = GuestMemory::Base();
    for (uint32_t i = 0; i < kXmaContextCount; ++i) {
        if (g_allocated[i]) continue;
        g_allocated[i] = true;
        std::memset(base + ContextAddress(i), 0, kXmaContextBytes);
        return ContextAddress(i);
    }
    return 0;
}

bool ReleaseXmaContext(uint32_t guestAddress) {
    std::lock_guard<std::mutex> lock(g_mu);
    uint32_t index = 0;
    if (!FindContextLocked(guestAddress, index)) return false;
    std::memset(GuestMemory::Base() + guestAddress, 0, kXmaContextBytes);
    g_allocated[index] = false;
    return true;
}

uint32_t XmaContextArrayGuestAddress() {
    std::lock_guard<std::mutex> lock(g_mu);
    return InitializeLocked() ? g_contextArray : 0;
}

bool WriteXmaRegister(uint32_t guestAddress, uint32_t value) {
    std::lock_guard<std::mutex> lock(g_mu);
    if (!GuestMemory::Base() || guestAddress < kXmaRegisterBase ||
        guestAddress + sizeof(uint32_t) > kXmaRegisterBase + kXmaRegisterSize ||
        (guestAddress & 3)) return false;

    P32(GuestMemory::Base(), guestAddress, value);
    if (guestAddress >= kXmaContext0KickRegister &&
        guestAddress < kXmaContext0KickRegister + kXmaContextKickRegisterCount * 4) {
        uint32_t group = (guestAddress - kXmaContext0KickRegister) / 4;
        uint32_t bits = value;
        while (bits) {
            uint32_t bit = 0;
            uint32_t lowest = bits & (~bits + 1);
            while ((1u << bit) != lowest) ++bit;
            uint32_t index = group * 32 + bit;
            if (index < kXmaContextCount && g_allocated[index]) ++g_decodeRequests;
            bits &= bits - 1;
        }
    }
    return true;
}

uint32_t ReadXmaRegister(uint32_t guestAddress) {
    std::lock_guard<std::mutex> lock(g_mu);
    if (!GuestMemory::Base() || guestAddress < kXmaRegisterBase ||
        guestAddress + sizeof(uint32_t) > kXmaRegisterBase + kXmaRegisterSize ||
        (guestAddress & 3)) return 0;
    return G32(GuestMemory::Base(), guestAddress);
}

XmaDecodeBoundaryResult SubmitXmaDecode(uint32_t contextGuestAddress) {
    std::lock_guard<std::mutex> lock(g_mu);
    uint32_t index = 0;
    if (!FindContextLocked(contextGuestAddress, index)) return XmaDecodeBoundaryResult::InvalidContext;
    ++g_decodeRequests;
    return XmaDecodeBoundaryResult::BackendUnavailable;
}

uint64_t XmaDecodeRequestCount() {
    std::lock_guard<std::mutex> lock(g_mu);
    return g_decodeRequests;
}

bool XmaDecoderBackendAvailable() { return false; }

} // namespace rt::audio
