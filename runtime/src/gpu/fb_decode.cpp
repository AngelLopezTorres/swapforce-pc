#include "fb_decode.h"

#include <cstdio>
#include <cstring>

#include "gpu.h"
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
    out.assign((size_t)w * h * 4, 0);
    uint32_t sw[4] = {t.swizzle & 7u, (t.swizzle >> 3) & 7u, (t.swizzle >> 6) & 7u, (t.swizzle >> 9) & 7u};
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
    return true;
}

} // namespace rt::gpu
