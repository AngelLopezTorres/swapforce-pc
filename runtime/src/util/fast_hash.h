#pragma once
// M8: hash rapido de bloques de memoria para deteccion de cambios (texturas, paginas de memoria guest).
// Esquema tipo XXH3 (acumulacion por carriles con multiplicacion 32x32->64; AVX2 si la CPU lo admite, si no escalar).
// Solo se usa para detectar cambios de contenido: no es criptografico. Mismo resultado en ambas rutas.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <immintrin.h>
#include <intrin.h>

namespace rt {

namespace fh_detail {
constexpr uint64_t kSecret[4] = {0xBE4BA423396CFEB8ull, 0x1CAD21F72C81017Cull, 0xDB979083E96DD4DEull, 0x1F67B3B7A4A44072ull};
constexpr uint64_t kPrime = 0x9E3779B185EBCA87ull;

inline uint64_t Avalanche(uint64_t h) {
    h ^= h >> 37; h *= 0x165667919E3779F9ull; h ^= h >> 32;
    return h;
}

inline void AccScalar(uint64_t acc[4], const uint8_t* p) {
    for (int i = 0; i < 4; ++i) {
        uint64_t d; std::memcpy(&d, p + i * 8, 8);
        uint64_t k = d ^ kSecret[i];
        acc[i] += (uint64_t)(uint32_t)k * (uint64_t)(uint32_t)(k >> 32);
        acc[i ^ 1] += d;
    }
}

__attribute__((target("avx2"))) inline void AccAvx2(uint64_t acc4[4], const uint8_t* p, size_t blocks) {
    __m256i acc = _mm256_loadu_si256((const __m256i*)acc4);
    const __m256i key = _mm256_loadu_si256((const __m256i*)kSecret);
    for (size_t b = 0; b < blocks; ++b) {
        __m256i d = _mm256_loadu_si256((const __m256i*)(p + b * 32));
        __m256i k = _mm256_xor_si256(d, key);
        __m256i hi = _mm256_srli_epi64(k, 32);
        __m256i prod = _mm256_mul_epu32(k, hi);
        __m256i sw = _mm256_shuffle_epi32(d, _MM_SHUFFLE(1, 0, 3, 2));   // intercambia los carriles 64 bits (i ^ 1) dentro de cada 128
        acc = _mm256_add_epi64(acc, _mm256_add_epi64(prod, sw));
    }
    _mm256_storeu_si256((__m256i*)acc4, acc);
}

inline bool DetectAvx2() {   // CPUID.7.0:EBX[5] y soporte del SO para estado YMM (OSXSAVE + XCR0[2:1])
    int r[4];
    __cpuid(r, 0);
    if (r[0] < 7) return false;
    __cpuid(r, 1);
    if (!(r[2] & (1 << 27)) || !(r[2] & (1 << 28))) return false;
    if ((_xgetbv(0) & 6) != 6) return false;
    __cpuidex(r, 7, 0);
    return (r[1] & (1 << 5)) != 0;
}
inline bool HasAvx2() {
    static const bool v = DetectAvx2();
    return v;
}
} // namespace fh_detail

inline uint64_t FastHash(const void* data, size_t n, uint64_t seed = 0) {
    using namespace fh_detail;
    const uint8_t* p = static_cast<const uint8_t*>(data);
    uint64_t acc[4] = {seed ^ kPrime, seed + 0x85EBCA77C2B2AE63ull, seed ^ 0x27D4EB2F165667C5ull, seed + 0x94D049BB133111EBull};
    size_t blocks = n / 32;
    if (blocks) {
        if (HasAvx2()) AccAvx2(acc, p, blocks);
        else for (size_t b = 0; b < blocks; ++b) AccScalar(acc, p + b * 32);
    }
    uint64_t h = (uint64_t)n * kPrime;
    for (int i = 0; i < 4; ++i) h = (h ^ Avalanche(acc[i] + i)) * kPrime;
    for (size_t i = blocks * 32; i < n; ++i) h = (h ^ p[i]) * 0x100000001B3ull;
    return Avalanche(h);
}

} // namespace rt
