#include "fb_decode.h"

#include <cstdio>
#include <cstring>

#include "gpu.h"
#include "import_log.h"
#include <immintrin.h>
#include "xenia/gpu/texture_address.h"
#include "xenia/gpu/xenos.h"

namespace rt::gpu {

uint32_t GpuSwap32(uint32_t v, uint32_t endian) {
    switch (endian) {
        default: case 0: return v;
        case 1: return ((v << 8) & 0xFF00FF00) | ((v >> 8) & 0x00FF00FF);
        case 2: return __builtin_bswap32(v);
        case 3: return (v >> 16) | (v << 16);
    }
}

bool g_fbSlowPath = false;   // M8: --fb-slow-path (camino por pixel original)
bool g_fbVerify = false;     // M8: --fb-verify (ejecuta ambos y compara)

// M8: Tiled2D(x..x+3, y) contiguos para 4 bytes por pixel (misma comprobacion que en d3d12_tex.cpp).
static bool FbRunsContiguous() {
    static int c = -1;
    if (c >= 0) return c != 0;
    bool ok = true;
    const uint32_t pitches[3] = {32, 128, 1280};
    for (uint32_t pi = 0; pi < 3 && ok; ++pi)
        for (int32_t y = 0; y < 96 && ok; ++y)
            for (int32_t x = 0; x < 128 && ok; x += 4) {
                int32_t o0 = xe::gpu::texture_address::Tiled2D(x, y, pitches[pi], 2);
                for (int32_t i = 1; i < 4; ++i)
                    if (xe::gpu::texture_address::Tiled2D(x + i, y, pitches[pi], 2) != o0 + i * 4) { ok = false; break; }
            }
    c = ok ? 1 : 0;
    return ok;
}

// Camino rapido para k_8_8_8_8: byte de salida i del pixel = byte e(sw[i]) de la palabra cargada (o constante 0/255).
__attribute__((target("ssse3"))) static void FbFast(const uint8_t* src, uint8_t* out, uint32_t w, uint32_t h, bool tiled, uint32_t pitchPx,
                                                   uint32_t pitchAligned, uint32_t endian, const uint32_t sw[4]) {
    auto srcByte = [&](uint32_t j) -> int {   // byte j de v = GpuSwap32(raw, endian) -> indice de byte en raw
        switch (endian) { case 1: return (int)(j ^ 1); case 2: return (int)(3 - j); case 3: return (int)(j ^ 2); default: return (int)j; }
    };
    alignas(16) uint8_t mask[16], orv[16];
    for (int px = 0; px < 4; ++px)
        for (int i = 0; i < 4; ++i) {
            uint32_t s = sw[i];
            if (s <= 3) { mask[px * 4 + i] = (uint8_t)(px * 4 + srcByte(s)); orv[px * 4 + i] = 0; }
            else { mask[px * 4 + i] = 0x80; orv[px * 4 + i] = s == 5 ? 255 : 0; }
        }
    const __m128i m = _mm_load_si128((const __m128i*)mask), o = _mm_load_si128((const __m128i*)orv);
    for (uint32_t y = 0; y < h; ++y) {
        uint8_t* orow = out + (size_t)y * w * 4;
        uint32_t x = 0;
        for (; x + 4 <= w; x += 4) {
            uint32_t off = tiled ? (uint32_t)xe::gpu::texture_address::Tiled2D((int32_t)x, (int32_t)y, pitchAligned, 2) : (y * pitchPx + x) * 4;
            __m128i v = _mm_loadu_si128((const __m128i*)(src + off));
            v = _mm_or_si128(_mm_shuffle_epi8(v, m), o);
            _mm_storeu_si128((__m128i*)(orow + x * 4), v);
        }
        for (; x < w; ++x) {   // resto (w no multiplo de 4)
            uint32_t off = tiled ? (uint32_t)xe::gpu::texture_address::Tiled2D((int32_t)x, (int32_t)y, pitchAligned, 2) : (y * pitchPx + x) * 4;
            for (int i = 0; i < 4; ++i) orow[x * 4 + i] = sw[i] <= 3 ? src[off + srcByte(sw[i])] : (sw[i] == 5 ? 255 : 0);
        }
    }
}

bool DecodeFrontbuffer(const FbDesc& d, std::vector<uint8_t>& out, std::string* info) {
    namespace x = xe::gpu::xenos;
    x::xe_gpu_texture_fetch_t t;
    std::memcpy(&t, d.fetch, sizeof(t));
    uint32_t fmt = (uint32_t)t.format;
    bool is8888 = fmt == (uint32_t)x::TextureFormat::k_8_8_8_8;
    bool is2101010 = fmt == (uint32_t)x::TextureFormat::k_2_10_10_10 || fmt == (uint32_t)x::TextureFormat::k_2_10_10_10_AS_16_16_16_16;
    char b[256];
    std::snprintf(b, sizeof(b), "fmt=%u tiled=%u endian=%u swz=%03X pitch=%u size=%ux%u base=%05X",
                  fmt, (unsigned)t.tiled, (unsigned)t.endianness, (unsigned)t.swizzle, (unsigned)t.pitch * 32,
                  (unsigned)t.size_2d.width + 1, (unsigned)t.size_2d.height + 1, (unsigned)t.base_address);
    if (info) *info = b;
    if (!is8888 && !is2101010) return false;
    uint32_t w = d.w, h = d.h;
    if (!w || !h || w > 8192 || h > 8192) return false;
    uint32_t pitchPx = t.pitch * 32;
    if (pitchPx < w) pitchPx = (w + 31) & ~31u;
    uint32_t pitchAligned = (pitchPx + 31) & ~31u;
    uint32_t hAligned = (h + 31) & ~31u;
    uint32_t bytes = t.tiled ? pitchAligned * hAligned * 4 : pitchPx * h * 4;
    const uint8_t* src = PhysHost(d.phys, bytes);
    if (!src) return false;
    uint32_t sw[4] = {t.swizzle & 7u, (t.swizzle >> 3) & 7u, (t.swizzle >> 6) & 7u, (t.swizzle >> 9) & 7u};
    std::vector<uint8_t> fastOut;
    bool fast = is8888 && !g_fbSlowPath && (!t.tiled || FbRunsContiguous());
    if (fast) {
        out.resize((size_t)w * h * 4);
        FbFast(src, out.data(), w, h, t.tiled != 0, pitchPx, pitchAligned, (uint32_t)t.endianness, sw);
        if (!g_fbVerify) return true;
        fastOut.swap(out);
    }
    out.assign((size_t)w * h * 4, 0);
    for (uint32_t y = 0; y < h; ++y)
        for (uint32_t xx = 0; xx < w; ++xx) {
            uint32_t off = t.tiled ? (uint32_t)xe::gpu::texture_address::Tiled2D((int32_t)xx, (int32_t)y, pitchAligned, 2)
                                   : (y * pitchPx + xx) * 4;
            uint32_t raw;
            std::memcpy(&raw, src + off, 4);          // palabra cargada en orden del host (little-endian)
            uint32_t v = GpuSwap32(raw, (uint32_t)t.endianness);
            uint32_t ch[4];
            if (is8888) { for (int i = 0; i < 4; ++i) ch[i] = (v >> (8 * i)) & 0xFF; }
            else {
                ch[0] = (v & 0x3FF) >> 2; ch[1] = ((v >> 10) & 0x3FF) >> 2; ch[2] = ((v >> 20) & 0x3FF) >> 2; ch[3] = ((v >> 30) & 3) * 85;
            }
            uint8_t* o = &out[((size_t)y * w + xx) * 4];
            for (int i = 0; i < 4; ++i) o[i] = sw[i] <= 3 ? (uint8_t)ch[sw[i]] : (sw[i] == 5 ? 255 : 0);
        }
    if (!fastOut.empty()) {
        static uint64_t ok = 0, bad = 0;
        bool same = fastOut == out;
        ++(same ? ok : bad);
        if (!same || ok <= 3 || ok % 200 == 0)
            ImportLog::Get().Printf("M8 fb-verify %ux%u tiled=%u endian=%u swz=%03X -> %s (ok=%llu mal=%llu)", w, h, (unsigned)t.tiled, (unsigned)t.endianness,
                                    (unsigned)t.swizzle, same ? "IGUAL" : "DISTINTO", (unsigned long long)ok, (unsigned long long)bad);
    }
    return true;
}

} // namespace rt::gpu
