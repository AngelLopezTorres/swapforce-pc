#pragma once
#include <cstdint>
#include <algorithm>
namespace xe {
typedef uint32_t fourcc_t;
constexpr fourcc_t make_fourcc(char a, char b, char c, char d) {
  return fourcc_t(uint8_t(a)) << 24 | fourcc_t(uint8_t(b)) << 16 | fourcc_t(uint8_t(c)) << 8 | fourcc_t(uint8_t(d));
}
constexpr fourcc_t make_fourcc(const char (&s)[5]) { return make_fourcc(s[0], s[1], s[2], s[3]); }
}
namespace xe {
template <typename T> constexpr T saturate(T x) { return x < T(0) ? T(0) : (x > T(1) ? T(1) : x); }
inline uint32_t lzcnt(uint32_t v) { return v ? (uint32_t)__builtin_clz(v) : 32u; }
inline uint32_t countof_dummy() { return 0; }
}
