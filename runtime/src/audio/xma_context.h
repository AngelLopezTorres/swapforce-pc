#pragma once

#include <cstdint>

namespace rt::audio {

constexpr uint32_t kXmaContextCount = 320;
constexpr uint32_t kXmaContextBytes = 64;
constexpr uint32_t kXmaPacketBytes = 2048;
constexpr uint32_t kXmaSamplesPerFrame = 512;
constexpr uint32_t kXmaSamplesPerSubframe = 128;

constexpr uint32_t kXmaContextArrayAddressRegister = 0x7FEA1800;
constexpr uint32_t kXmaCurrentContextIndexRegister = 0x7FEA1818;
constexpr uint32_t kXmaNextContextIndexRegister = 0x7FEA181C;
constexpr uint32_t kXmaContext0KickRegister = 0x7FEA1940;
constexpr uint32_t kXmaContextKickRegisterCount = 10;

// Los dwords permanecen en big-endian guest. Se evita exponer bitfields C++,
// cuyo orden de bits y bytes depende del compilador anfitrion.
struct XmaContextData {
    uint32_t dwords[16];
};
static_assert(sizeof(XmaContextData) == kXmaContextBytes, "XMA context must be 64 bytes");

enum class XmaContextWord : uint8_t {
    Input0CountAndFlags = 0,
    Input1CountAndFlags = 1,
    InputReadOffset = 2,
    LoopStart = 3,
    LoopEndAndMetadata = 4,
    Input0PhysicalAddress = 5,
    Input1PhysicalAddress = 6,
    OutputPhysicalAddress = 7,
    WorkPhysicalAddress = 8,
    OutputReadOffsetAndFlags = 9,
    Reserved10 = 10,
};

enum class XmaDecodeBoundaryResult {
    InvalidContext,
    BackendUnavailable,
};

uint32_t CreateXmaContext();
bool ReleaseXmaContext(uint32_t guestAddress);
uint32_t XmaContextArrayGuestAddress();
bool WriteXmaRegister(uint32_t guestAddress, uint32_t value);
uint32_t ReadXmaRegister(uint32_t guestAddress);
XmaDecodeBoundaryResult SubmitXmaDecode(uint32_t contextGuestAddress);
uint64_t XmaDecodeRequestCount();
bool XmaDecoderBackendAvailable();

} // namespace rt::audio
