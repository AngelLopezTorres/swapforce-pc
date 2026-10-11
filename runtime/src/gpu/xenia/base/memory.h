#pragma once
#include <cstdint>
#include <cstring>
namespace xe { namespace memory {
template <typename To, typename From> inline To Reinterpret(const From& v) { static_assert(sizeof(To) == sizeof(From), ""); To t; std::memcpy(&t, &v, sizeof(To)); return t; }
} }
