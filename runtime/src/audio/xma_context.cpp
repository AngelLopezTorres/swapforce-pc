#include "xma_context.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstring>
#include <mutex>

#include "guest_abi.h"
#include "guest_heap.h"
#include "guest_memory.h"
#include "import_log.h"

namespace rt::audio {
namespace {

constexpr uint32_t kPhysicalMask = 0x1FFFFFFF;
constexpr uint32_t kXmaRegisterBase = 0x7FEA0000;
constexpr uint32_t kXmaRegisterSize = 0x10000;

std::mutex g_mu;
uint32_t g_contextArray = 0;
std::array<bool, kXmaContextCount> g_allocated{};
std::array<bool, kXmaContextCount> g_consumedInput{};
uint64_t g_decodeRequests = 0;
uint64_t g_simulatedPackets = 0;
std::atomic<bool> g_workerRunning{false};
HANDLE g_worker = nullptr;

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
        g_consumedInput[i] = false;
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
    g_consumedInput[index] = false;
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

uint32_t SimulateXmaStep() {
    std::lock_guard<std::mutex> lock(g_mu);
    if (!g_contextArray || !GuestMemory::Base()) return 0;
    uint8_t* base = GuestMemory::Base();
    uint32_t consumed = 0;
    for (uint32_t i = 0; i < kXmaContextCount; ++i) {
        if (!g_allocated[i]) continue;
        uint32_t address = ContextAddress(i);
        uint32_t w0 = G32(base, address);
        uint32_t w1 = G32(base, address + 4);
        if (!(w1 & 0x80000000u)) continue; // output_buffer_valid
        uint32_t valid = (w0 >> 20) & 3;
        if (!valid) {
            if (g_consumedInput[i]) {
                P32(base, address + 4, w1 & ~0x80000000u);
                g_consumedInput[i] = false;
            }
            continue;
        }
        uint32_t blocks = (w0 >> 22) & 31;
        if (!blocks) continue;
        uint32_t w4 = G32(base, address + 16);
        uint32_t current = w4 >> 31;
        if (!(valid & (1u << current))) current ^= 1;
        uint32_t packets = current ? (w1 & 0xFFF) : (w0 & 0xFFF);
        if (!packets) continue;

        uint32_t outputPhysical = G32(base, address + 28) & kPhysicalMask;
        uint32_t capacity = blocks * 256;
        if (!outputPhysical || outputPhysical > 0x20000000u - capacity) continue;
        uint32_t bytes = std::min(capacity, (w1 & (1u << 29)) ? 2048u : 1024u);
        uint32_t write = (w0 >> 27) & 31;
        if (write >= blocks) write = 0;
        uint32_t offset = write * 256;
        uint32_t first = std::min(bytes, capacity - offset);
        uint8_t* output = base + 0xA0000000u + outputPhysical;
        std::memset(output + offset, 0, first);
        if (first < bytes) std::memset(output, 0, bytes - first);
        uint32_t nextWrite = (write + bytes / 256) % blocks;
        w0 = (w0 & ~(0x1Fu << 27) & ~(1u << (20 + current))) | (nextWrite << 27);
        P32(base, address, w0);
        P32(base, address + 8, packets * kXmaPacketBytes * 8);
        P32(base, address + 16, (w4 & 0x7FFFFFFFu) | ((current ^ 1) << 31));
        g_consumedInput[i] = true;
        ++g_simulatedPackets;
        ++consumed;
        if (g_simulatedPackets <= 16 || (g_simulatedPackets & 1023) == 0)
            ImportLog::Get().Printf("XMA SILENCE ctx=%u packets=%u bytes=%u write=%u valid=%u total=%llu",
                                    i, packets, bytes, nextWrite, (w0 >> 20) & 3,
                                    static_cast<unsigned long long>(g_simulatedPackets));
    }
    return consumed;
}

uint64_t XmaSimulatedPacketCount() {
    std::lock_guard<std::mutex> lock(g_mu);
    return g_simulatedPackets;
}

static DWORD WINAPI XmaWorkerMain(void*) {
    while (g_workerRunning.load()) {
        SimulateXmaStep();
        Sleep(10);
    }
    return 0;
}

void StartXmaWorker() {
    if (g_workerRunning.exchange(true)) return;
    g_worker = CreateThread(nullptr, 0, XmaWorkerMain, nullptr, 0, nullptr);
    if (!g_worker) g_workerRunning = false;
}

void StopXmaWorker() {
    g_workerRunning = false;
    if (g_worker) {
        WaitForSingleObject(g_worker, 2000);
        CloseHandle(g_worker);
        g_worker = nullptr;
    }
}

} // namespace rt::audio
