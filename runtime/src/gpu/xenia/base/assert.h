#pragma once
// Shim minimo de xenia/base/assert.h (Xenia, BSD): las comprobaciones se eliminan.
#include <cassert>
#define static_assert_size(type, size) static_assert(sizeof(type) == (size), "tamano de " #type)
#define assert_true(...) ((void)0)
#define assert_false(...) ((void)0)
#define assert_zero(...) ((void)0)
#define assert_not_zero(...) ((void)0)
#define assert_null(...) ((void)0)
#define assert_not_null(...) ((void)0)
#define assert_always(...) ((void)0)
#define assert_unhandled_case(...) ((void)0)
#define assert_not_implemented(...) ((void)0)
