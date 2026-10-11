#pragma once
#include <cstdint>
namespace xe {
inline uint16_t byte_swap(uint16_t v) { return __builtin_bswap16(v); }
inline uint32_t byte_swap(uint32_t v) { return __builtin_bswap32(v); }
inline uint64_t byte_swap(uint64_t v) { return __builtin_bswap64(v); }
inline int32_t byte_swap(int32_t v) { return (int32_t)__builtin_bswap32((uint32_t)v); }
}
namespace xe { template <typename T> using le = T; }
