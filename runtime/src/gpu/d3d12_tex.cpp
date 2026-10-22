#include "perf.h"
#include "fast_hash.h"
// H5: texturas del guest -> texturas D3D12. Destilado (detiling) con xenia/gpu/texture_address.h, endian y formato segun la
// constante de fetch (xe_gpu_texture_fetch_t de xenos.h). Solo nivel base (sin mips) en esta version.
#include <algorithm>
#include <cmath>

#include "config.h"
#include "d3d12_impl.h"
#include "fb_decode.h"
#include "gpu_names.h"
#include "image_io.h"
#include "xenia/gpu/texture_address.h"

namespace rt::gpu::d3d {

bool g_texSlowPath = false;   // M8: --tex-slow-path (camino por bloque original, para comparar)
bool g_texVerify = false;
bool g_texMemoOff = false;    // M8: --no-tex-memo
bool g_verifyShortcut = false;   // M8: --verify-shortcut

// M8: lee de la GPU ambas texturas y compara: byte k de la textura del camino CPU == byte perm[k] de la copia del RT.
void D3D12Backend::VerifyShortcut(HostTexture* gpuTex, HostTexture* cpuTex, uint32_t base, uint32_t w, uint32_t h, uint32_t fmt) {
    static uint64_t ok = 0, bad = 0, skipped = 0;
    ResolvedTex* rtp = nullptr;
    for (auto& kv : resolvedTex) if (&kv.second.ht == gpuTex) rtp = &kv.second;
    if (!cpuTex || !rtp || !cpuTex->res) { ++skipped; return; }
    uint32_t rowPitch = (w * 4 + 255) & ~255u;
    uint64_t bytes = (uint64_t)rowPitch * h;
    D3D12_HEAP_PROPERTIES rp = {D3D12_HEAP_TYPE_READBACK};
    D3D12_RESOURCE_DESC bd = {};
    bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; bd.Width = bytes * 2; bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
    bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ComPtr<ID3D12Resource> rb;
    if (!Hr(dev->CreateCommittedResource(&rp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&rb)), "verify readback")) return;
    const D3D12_RESOURCE_STATES srv = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    ID3D12Resource* rs[2] = {cpuTex->res.Get(), gpuTex->res.Get()};
    DXGI_FORMAT fm[2] = {cpuTex->res->GetDesc().Format, gpuTex->res->GetDesc().Format};
    for (int i = 0; i < 2; ++i) {
        Transition(rs[i], srv, D3D12_RESOURCE_STATE_COPY_SOURCE);
        D3D12_TEXTURE_COPY_LOCATION d = {}, s = {};
        d.pResource = rb.Get(); d.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        d.PlacedFootprint.Offset = (uint64_t)i * bytes;
        d.PlacedFootprint.Footprint = {fm[i], w, h, 1, rowPitch};
        s.pResource = rs[i]; s.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; s.SubresourceIndex = 0;
        cl->CopyTextureRegion(&d, 0, 0, 0, &s, nullptr);
        Transition(rs[i], D3D12_RESOURCE_STATE_COPY_SOURCE, srv);
    }
    FlushAndWait();
    void* p = nullptr;
    D3D12_RANGE r = {0, (SIZE_T)(bytes * 2)};
    if (!Hr(rb->Map(0, &r, &p), "verify map")) return;
    const uint8_t* a = (const uint8_t*)p;
    const uint8_t* g = a + bytes;
    static const uint32_t permId[4] = {0, 1, 2, 3}, permSwap[4] = {2, 1, 0, 3};
    const uint32_t* perm = (fmt == 6 && rtp->swapRB) ? permSwap : permId;
    uint64_t diff = 0;
    for (uint32_t y = 0; y < h; ++y)
        for (uint32_t x = 0; x < w; ++x) {
            const uint8_t* pa = a + (size_t)y * rowPitch + x * 4;
            const uint8_t* pg = g + (size_t)y * rowPitch + x * 4;
            for (int k = 0; k < 4; ++k) if (pa[k] != pg[perm[k]]) { ++diff; break; }
        }
    D3D12_RANGE wz = {0, 0};
    rb->Unmap(0, &wz);
    ++(diff ? bad : ok);
    if (diff || ok <= 5 || ok % 200 == 0)
        ImportLog::Get().Printf("M8 verify-shortcut base=%08X %ux%u fmt=%u swapRB=%u -> %s (pixeles distintos=%llu; ok=%llu mal=%llu omitidas=%llu)", base, w, h, fmt,
                                (unsigned)rtp->swapRB, diff ? "DISTINTO" : "IGUAL", (unsigned long long)diff, (unsigned long long)ok, (unsigned long long)bad, (unsigned long long)skipped);
}     // M8: --tex-verify (ejecuta ambos caminos y compara byte a byte)
static uint64_t texVerifyOk = 0, texVerifyBad = 0;
extern uint64_t g_memoryEpoch;
uint64_t g_memoryEpoch = 0;

static uint64_t Fnv(const uint8_t* p, size_t n, uint64_t h = 1469598103934665603ull) {
    return FastHash(p, n, h);   // M8: hash vectorial (antes FNV-1a de 64 bits en cadena)
}

D3D12_SAMPLER_DESC D3D12Backend::SamplerFromFetch(uint32_t idx) {
    xenos::xe_gpu_texture_fetch_t tf;
    uint32_t w[6];
    for (int i = 0; i < 6; ++i) w[i] = RV(0x4800 + idx * 6 + i);
    std::memcpy(&tf, w, sizeof(tf));
    D3D12_SAMPLER_DESC s = {};
    bool valid = (uint32_t)tf.type == 2;
    auto addr = [&](xenos::ClampMode m) {
        switch (m) {
            case xenos::ClampMode::kRepeat: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case xenos::ClampMode::kMirroredRepeat: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case xenos::ClampMode::kClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case xenos::ClampMode::kMirrorClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            case xenos::ClampMode::kClampToHalfway: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case xenos::ClampMode::kMirrorClampToHalfway: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            case xenos::ClampMode::kClampToBorder: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            default: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        }
    };
    s.AddressU = valid ? addr(tf.clamp_x) : D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    s.AddressV = valid ? addr(tf.clamp_y) : D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    s.AddressW = valid ? addr(tf.clamp_z) : D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    bool magLin = !valid || tf.mag_filter != xenos::TextureFilter::kPoint;
    bool minLin = !valid || tf.min_filter != xenos::TextureFilter::kPoint;
    bool mipLin = valid && tf.mip_filter == xenos::TextureFilter::kLinear;
    if (valid && tf.aniso_filter != xenos::AnisoFilter::kDisabled && tf.aniso_filter != xenos::AnisoFilter::kMax_1_1 && tf.aniso_filter != xenos::AnisoFilter::kUseFetchConst) {
        s.Filter = D3D12_FILTER_ANISOTROPIC;
        uint32_t a = (uint32_t)tf.aniso_filter;
        s.MaxAnisotropy = a >= 5 ? 16 : (1u << a);
    } else {
        s.Filter = (D3D12_FILTER)(((minLin ? 1 : 0) << 4) | ((magLin ? 1 : 0) << 2) | (mipLin ? 1 : 0));
        s.MaxAnisotropy = 1;
    }
    s.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    if (valid && tf.border_color == xenos::BorderColor::k_ABGR_White) { s.BorderColor[0] = s.BorderColor[1] = s.BorderColor[2] = s.BorderColor[3] = 1.0f; }
    s.MinLOD = 0.0f; s.MaxLOD = D3D12_FLOAT32_MAX;
    return s;
}

struct FmtInfo { DXGI_FORMAT dxgi; DXGI_FORMAT srgb; uint32_t bpb; uint32_t bw, bh; int expand; };   // expand: 0 directo, 1 = 5_6_5, 2 = 1_5_5_5, 3 = 4_4_4_4, 4 = 24_8, 5 = 24_8_FLOAT (a RGBA8 / R32F)

static bool GetFmt(uint32_t f, FmtInfo& o) {
    using D = DXGI_FORMAT;
    switch (f) {
        case 2: o = {D::DXGI_FORMAT_R8_UNORM, D::DXGI_FORMAT_R8_UNORM, 1, 1, 1, 0}; return true;
        case 10: o = {D::DXGI_FORMAT_R8G8_UNORM, D::DXGI_FORMAT_R8G8_UNORM, 2, 1, 1, 0}; return true;
        case 6: case 14: o = {D::DXGI_FORMAT_R8G8B8A8_UNORM, D::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 4, 1, 1, 0}; return true;
        case 7: o = {D::DXGI_FORMAT_R10G10B10A2_UNORM, D::DXGI_FORMAT_R10G10B10A2_UNORM, 4, 1, 1, 0}; return true;
        case 18: o = {D::DXGI_FORMAT_BC1_UNORM, D::DXGI_FORMAT_BC1_UNORM_SRGB, 8, 4, 4, 0}; return true;
        case 19: o = {D::DXGI_FORMAT_BC2_UNORM, D::DXGI_FORMAT_BC2_UNORM_SRGB, 16, 4, 4, 0}; return true;
        case 20: o = {D::DXGI_FORMAT_BC3_UNORM, D::DXGI_FORMAT_BC3_UNORM_SRGB, 16, 4, 4, 0}; return true;
        case 49: o = {D::DXGI_FORMAT_BC5_UNORM, D::DXGI_FORMAT_BC5_UNORM, 16, 4, 4, 0}; return true;
        case 24: o = {D::DXGI_FORMAT_R16_UNORM, D::DXGI_FORMAT_R16_UNORM, 2, 1, 1, 0}; return true;
        case 25: o = {D::DXGI_FORMAT_R16G16_UNORM, D::DXGI_FORMAT_R16G16_UNORM, 4, 1, 1, 0}; return true;
        case 26: o = {D::DXGI_FORMAT_R16G16B16A16_UNORM, D::DXGI_FORMAT_R16G16B16A16_UNORM, 8, 1, 1, 0}; return true;
        case 30: o = {D::DXGI_FORMAT_R16_FLOAT, D::DXGI_FORMAT_R16_FLOAT, 2, 1, 1, 0}; return true;
        case 31: o = {D::DXGI_FORMAT_R16G16_FLOAT, D::DXGI_FORMAT_R16G16_FLOAT, 4, 1, 1, 0}; return true;
        case 32: o = {D::DXGI_FORMAT_R16G16B16A16_FLOAT, D::DXGI_FORMAT_R16G16B16A16_FLOAT, 8, 1, 1, 0}; return true;
        case 36: o = {D::DXGI_FORMAT_R32_FLOAT, D::DXGI_FORMAT_R32_FLOAT, 4, 1, 1, 0}; return true;
        case 37: o = {D::DXGI_FORMAT_R32G32_FLOAT, D::DXGI_FORMAT_R32G32_FLOAT, 8, 1, 1, 0}; return true;
        case 38: o = {D::DXGI_FORMAT_R32G32B32A32_FLOAT, D::DXGI_FORMAT_R32G32B32A32_FLOAT, 16, 1, 1, 0}; return true;
        case 4: o = {D::DXGI_FORMAT_R8G8B8A8_UNORM, D::DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, 1, 1}; return true;
        case 3: o = {D::DXGI_FORMAT_R8G8B8A8_UNORM, D::DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, 1, 2}; return true;
        case 15: o = {D::DXGI_FORMAT_R8G8B8A8_UNORM, D::DXGI_FORMAT_R8G8B8A8_UNORM, 2, 1, 1, 3}; return true;
        case 22: o = {D::DXGI_FORMAT_R32_FLOAT, D::DXGI_FORMAT_R32_FLOAT, 4, 1, 1, 4}; return true;
        case 23: o = {D::DXGI_FORMAT_R32_FLOAT, D::DXGI_FORMAT_R32_FLOAT, 4, 1, 1, 5}; return true;
        default: return false;
    }
}

static uint32_t Log2u(uint32_t v) { uint32_t l = 0; while ((1u << (l + 1)) <= v) ++l; return l; }

// M8: comprobacion (una vez por bpbLog2) de que Tiled2D coloca contiguos los bloques de cada grupo alineado de 16 bytes
// (16 >> bpbLog2 bloques consecutivos en x a direcciones consecutivas). Si se cumple, el destilado copia tramos de 16 bytes;
// si no, se usa el camino bloque a bloque original. Comprobado sobre 128x96 bloques con 3 pitch distintos.
static bool TiledRunsContiguous(uint32_t bpbLog2) {
    static int cache[5] = {-1, -1, -1, -1, -1};
    if (bpbLog2 > 4) return false;
    if (cache[bpbLog2] >= 0) return cache[bpbLog2] != 0;
    uint32_t run = std::max(1u, 16u >> bpbLog2), bpb = 1u << bpbLog2;
    bool ok = true;
    const uint32_t pitches[3] = {32, 128, 1280};
    for (uint32_t pi = 0; pi < 3 && ok; ++pi)
        for (int32_t y = 0; y < 96 && ok; ++y)
            for (int32_t x = 0; x < 128 && ok; x += (int32_t)run) {
                int32_t o0 = xe::gpu::texture_address::Tiled2D(x, y, pitches[pi], bpbLog2);
                for (uint32_t i = 1; i < run; ++i)
                    if (xe::gpu::texture_address::Tiled2D(x + (int32_t)i, y, pitches[pi], bpbLog2) != o0 + (int32_t)(i * bpb)) { ok = false; break; }
            }
    cache[bpbLog2] = ok ? 1 : 0;
    ImportLog::Get().Printf("M8 destilado: Tiled2D bpbLog2=%u tramos contiguos de %u bloques -> %s", bpbLog2, run, ok ? "SI (camino rapido)" : "NO (camino por bloque)");
    return ok;
}

// Copia las filas de bloques de una cara (destilado) a dst (rowPitch bytes por fila), bloques en bruto de bpb bytes.
static void DetileRaw(uint8_t* dst, uint32_t rowPitch, const uint8_t* lm, uint32_t wBlocks, uint32_t hBlocks, bool tiled,
                      uint32_t pitchAligned, uint32_t bpbLog2, uint32_t linPitchBytes) {
    uint32_t bpb = 1u << bpbLog2;
    if (!tiled) {
        for (uint32_t by = 0; by < hBlocks; ++by) std::memcpy(dst + (size_t)by * rowPitch, lm + (size_t)by * linPitchBytes, (size_t)wBlocks * bpb);
        return;
    }
    uint32_t run = std::max(1u, 16u >> bpbLog2);
    bool fast = TiledRunsContiguous(bpbLog2);
    for (uint32_t by = 0; by < hBlocks; ++by) {
        uint8_t* d = dst + (size_t)by * rowPitch;
        if (fast) {
            for (uint32_t bx = 0; bx < wBlocks; bx += run) {
                uint32_t n = std::min(run, wBlocks - bx);
                size_t off = (size_t)xe::gpu::texture_address::Tiled2D((int32_t)bx, (int32_t)by, pitchAligned, bpbLog2);
                if (n == run && run * bpb == 16) std::memcpy(d + (size_t)bx * bpb, lm + off, 16);
                else std::memcpy(d + (size_t)bx * bpb, lm + off, (size_t)n * bpb);
            }
        } else {
            for (uint32_t bx = 0; bx < wBlocks; ++bx) {
                size_t off = (size_t)xe::gpu::texture_address::Tiled2D((int32_t)bx, (int32_t)by, pitchAligned, bpbLog2);
                std::memcpy(d + (size_t)bx * bpb, lm + off, bpb);
            }
        }
    }
}

// Endian por palabras en una fila (misma semantica que el camino por bloque: bpb>=4 -> GpuSwap32 por palabra de 32 bits;
// bpb==2 -> intercambio de los 2 bytes del texel con endian 1 o 2; bpb==1 -> nada).
static void SwapRow(uint8_t* d, size_t bytes, uint32_t bpb, uint32_t endian) {
    if (!endian) return;
    if (bpb >= 4) {
        size_t n = bytes / 4;
        uint32_t* w = reinterpret_cast<uint32_t*>(d);
        switch (endian) {
            case 1: for (size_t i = 0; i < n; ++i) { uint32_t v = w[i]; w[i] = ((v << 8) & 0xFF00FF00u) | ((v >> 8) & 0x00FF00FFu); } break;
            case 2: for (size_t i = 0; i < n; ++i) w[i] = __builtin_bswap32(w[i]); break;
            case 3: for (size_t i = 0; i < n; ++i) { uint32_t v = w[i]; w[i] = (v >> 16) | (v << 16); } break;
        }
    } else if (bpb == 2 && (endian == 1 || endian == 2)) {
        size_t n = bytes / 2;
        uint16_t* h = reinterpret_cast<uint16_t*>(d);
        for (size_t i = 0; i < n; ++i) h[i] = __builtin_bswap16(h[i]);
    }
}

// ---- volcado de depuracion (solo para --dump-textures): descompresion BC1/BC2/BC3 en CPU ----
static void Bc1Block(const uint8_t* b, uint8_t out[16][4], bool opaqueOnly) {
    uint16_t c0 = b[0] | (b[1] << 8), c1 = b[2] | (b[3] << 8);
    auto expand = [](uint16_t c, uint8_t* o) { o[0] = (uint8_t)(((c >> 11) & 31) * 255 / 31); o[1] = (uint8_t)(((c >> 5) & 63) * 255 / 63); o[2] = (uint8_t)((c & 31) * 255 / 31); o[3] = 255; };
    uint8_t col[4][4];
    expand(c0, col[0]); expand(c1, col[1]);
    if (c0 > c1 || opaqueOnly) {
        for (int k = 0; k < 3; ++k) { col[2][k] = (uint8_t)((2 * col[0][k] + col[1][k]) / 3); col[3][k] = (uint8_t)((col[0][k] + 2 * col[1][k]) / 3); }
        col[2][3] = col[3][3] = 255;
    } else {
        for (int k = 0; k < 3; ++k) { col[2][k] = (uint8_t)((col[0][k] + col[1][k]) / 2); col[3][k] = 0; }
        col[2][3] = 255; col[3][3] = 0;
    }
    uint32_t idx = b[4] | (b[5] << 8) | (b[6] << 16) | ((uint32_t)b[7] << 24);
    for (int i = 0; i < 16; ++i) std::memcpy(out[i], col[(idx >> (2 * i)) & 3], 4);
}
static void DecodeBcToRgba(uint32_t fmt, const uint8_t* lin, uint32_t rowPitch, uint32_t w, uint32_t h, std::vector<uint8_t>& rgba) {
    rgba.assign((size_t)w * h * 4, 0);
    uint32_t bx = (w + 3) / 4, by = (h + 3) / 4;
    for (uint32_t y = 0; y < by; ++y)
        for (uint32_t x = 0; x < bx; ++x) {
            const uint8_t* b = lin + (size_t)y * rowPitch + (size_t)x * (fmt == 18 ? 8 : 16);
            uint8_t px[16][4];
            if (fmt == 18) Bc1Block(b, px, false);
            else {
                Bc1Block(b + 8, px, true);
                if (fmt == 19) { for (int i = 0; i < 16; ++i) { uint8_t a = (b[i / 2] >> (4 * (i & 1))) & 15; px[i][3] = (uint8_t)(a * 17); } }
                else {
                    uint8_t a0 = b[0], a1 = b[1], al[8];
                    al[0] = a0; al[1] = a1;
                    if (a0 > a1) for (int k = 1; k < 7; ++k) al[k + 1] = (uint8_t)(((7 - k) * a0 + k * a1) / 7);
                    else { for (int k = 1; k < 5; ++k) al[k + 1] = (uint8_t)(((5 - k) * a0 + k * a1) / 5); al[6] = 0; al[7] = 255; }
                    uint64_t bits = 0; for (int k = 0; k < 6; ++k) bits |= (uint64_t)b[2 + k] << (8 * k);
                    for (int i = 0; i < 16; ++i) px[i][3] = al[(bits >> (3 * i)) & 7];
                }
            }
            for (uint32_t j = 0; j < 4; ++j) for (uint32_t i = 0; i < 4; ++i) {
                uint32_t X = x * 4 + i, Y = y * 4 + j;
                if (X < w && Y < h) std::memcpy(&rgba[((size_t)Y * w + X) * 4], px[j * 4 + i], 4);
            }
        }
}

HostTexture* D3D12Backend::GetTexture(uint32_t idx, bool vertexStage) {
    PERF_ZONE(kZoneTexUpload);
    (void)vertexStage;
    uint32_t w6[6];
    for (int i = 0; i < 6; ++i) w6[i] = RV(0x4800 + idx * 6 + i);
    xenos::xe_gpu_texture_fetch_t tf;
    std::memcpy(&tf, w6, sizeof(tf));
    if ((uint32_t)tf.type != 2) return nullptr;     // constante sin textura valida: se enlaza el SRV nulo (no es un fallo del backend)
    uint32_t fmtId = (uint32_t)tf.format;
    FmtInfo fi;
    if (!GetFmt(fmtId, fi)) { ++texFail; ++fTexFail; Note(std::string("OMISION textura: formato no soportado ") + TexFmtName(fmtId)); return nullptr; }
    uint32_t dimension = (uint32_t)tf.dimension;      // 0 = 1D, 1 = 2D / apilada, 2 = 3D, 3 = cubo
    uint32_t width, height, depth = 1, layers = 1;
    if (dimension == 0) { width = tf.size_1d.width + 1; height = 1; }
    else if (dimension == 2) { width = tf.size_3d.width + 1; height = tf.size_3d.height + 1; depth = tf.size_3d.depth + 1; }
    else { width = tf.size_2d.width + 1; height = tf.size_2d.height + 1; if (dimension == 3) layers = 6; else if (tf.stacked) depth = tf.size_2d.stack_depth + 1; }
    if (dimension == 2 || depth > 1) { ++texFail; ++fTexFail; Note("OMISION textura 3D/apilada no soportada"); return nullptr; }
    if (width > 8192 || height > 8192 || !width || !height) { ++texFail; ++fTexFail; Note("OMISION textura con dimensiones invalidas"); return nullptr; }
    uint32_t baseAddr = (uint32_t)tf.base_address << 12;
    uint32_t bw = fi.bw, bh = fi.bh;
    uint32_t wBlocks = (width + bw - 1) / bw, hBlocks = (height + bh - 1) / bh;
    uint32_t pitchPx = (uint32_t)tf.pitch * 32;
    if (!pitchPx) pitchPx = (width + 31) & ~31u;
    uint32_t pitchBlocks = (pitchPx + bw - 1) / bw;
    bool tiled = tf.tiled != 0;
    uint32_t endian = (uint32_t)tf.endianness;
    uint32_t bpbLog2 = Log2u(fi.bpb);
    uint32_t pitchAligned = (std::max(pitchBlocks, wBlocks) + 31) & ~31u, hAligned = (hBlocks + 31) & ~31u;
    uint32_t cube = dimension == 3 ? 1 : 0;
    if (cube && !tiled) { ++texFail; ++fTexFail; Note("OMISION cubemap lineal"); return nullptr; }
    // rango de memoria necesario
    uint64_t faceBytes = tiled ? (uint64_t)pitchAligned * hAligned * fi.bpb : (uint64_t)pitchPx / bw * fi.bpb * hBlocks;
    uint64_t totalBytes = faceBytes * layers;
    if (baseAddr + totalBytes > 0x20000000ull) { ++texFail; ++fTexFail; Note("OMISION textura fuera de la memoria fisica"); return nullptr; }
    // la ultima fila de bloques puede estar justo al final de la asignacion (alineado a 32x32 solo si hace falta)
    uint64_t needBytes = totalBytes;
    if (tiled) {
        int64_t maxOff = 0;
        for (uint32_t bx = 0; bx < wBlocks; bx += std::max(1u, wBlocks / 64))
            maxOff = std::max<int64_t>(maxOff, xe::gpu::texture_address::Tiled2D((int32_t)bx, (int32_t)hBlocks - 1, pitchAligned, bpbLog2));
        maxOff = std::max<int64_t>(maxOff, xe::gpu::texture_address::Tiled2D((int32_t)wBlocks - 1, (int32_t)hBlocks - 1, pitchAligned, bpbLog2));
        needBytes = (uint64_t)maxOff + fi.bpb + (layers > 1 ? faceBytes * (layers - 1) : 0);
    }
    // M8: lectura de un resolve del mismo fotograma con coincidencia exacta -> copia GPU del RT (sin memoria guest)
    if (dimension == 1 && layers == 1 && depth == 1 && tiled && !((uint32_t)tf.sign_x == 3 && fi.dxgi != fi.srgb))
        if (!shortcutVerifyActive)
            if (HostTexture* rtx = LookupResolvedTex(baseAddr, width, height, fmtId, endian, pitchAligned, (uint32_t)tf.swizzle)) {
                if (g_verifyShortcut) {   // M8: --verify-shortcut
                    shortcutVerifyActive = true;
                    HostTexture* cpuTex = GetTexture(idx, vertexStage);
                    shortcutVerifyActive = false;
                    VerifyShortcut(rtx, cpuTex, baseAddr, width, height, fmtId);
                }
                return rtx;
            }
    // si la memoria a leer tiene resolves pendientes de escribir, se completan antes (exactitud)
    FlushResolvesIfOverlap(baseAddr, (uint32_t)std::min<uint64_t>((uint64_t)baseAddr + needBytes, 0x20000000ull));
    const uint8_t* mem = PhysHost(baseAddr, (uint32_t)needBytes);
    ++fTexCalls;
    if (SeqLog()) ImportLog::Get().Printf("M8SEQ draw=%llu TEX fetch=%u base=%08X bytes=%llu %ux%u fmt=%s tiled=%u endian=%u", (unsigned long long)drawSerial, idx, baseAddr, (unsigned long long)needBytes, width, height, TexFmtName(fmtId), (unsigned)tiled, endian);
    for (auto& fr : frameResolves)
        if (baseAddr < fr.hi && baseAddr + needBytes > fr.lo) {
            ++fTexOverlap;
            if (fr.destBase == baseAddr && fr.fmt == fmtId && fr.endian == endian && fr.x0 == 0 && fr.y0 == 0 && width <= fr.w && height <= fr.h && tiled) ++fTexExact;
            break;
        }
    if (!mem) { ++texFail; ++fTexFail; Note("OMISION textura en memoria no asignada"); return nullptr; }
    uint64_t key = Fnv((const uint8_t*)w6, 16) ^ ((uint64_t)w6[4] << 7) ^ ((uint64_t)w6[5] << 31);   // direccion y descriptor (sin la dir. de mip)
    key ^= (uint64_t)baseAddr * 0x9E3779B97F4A7C15ull;
    uint64_t dataHash;
    uint64_t memoKey = key ^ (needBytes * 0xC2B2AE3D27D4EB4Full);
    auto mit = g_texMemoOff ? texMemo.end() : texMemo.find(memoKey);
    if (mit != texMemo.end()) { dataHash = mit->second.hash; ++texMemoHits; }
    else {
        dataHash = Fnv(mem, (size_t)needBytes);
        ++texMemoMiss;
        if (!g_texMemoOff) texMemo[memoKey] = {baseAddr, (uint32_t)std::min<uint64_t>((uint64_t)baseAddr + needBytes, 0xFFFFFFFFull), dataHash};
    }
    auto it = textures.find(key);
    if (it != textures.end() && it->second.dataHash == dataHash && it->second.res) { it->second.lastUse = ++texSerial; ++texHits; return &it->second; }
    // ---- conversion a datos lineales del host ----
    bool srgb = (uint32_t)tf.sign_x == 3 && fi.dxgi != fi.srgb;
    DXGI_FORMAT dfmt = srgb ? fi.srgb : fi.dxgi;
    uint32_t hostBpb = fi.bpb;
    if (fi.expand >= 1 && fi.expand <= 3) hostBpb = 4;
    uint32_t rowBytes = wBlocks * hostBpb;
    uint32_t rowPitch = (rowBytes + 255) & ~255u;
    uint32_t layerPitchRows = hBlocks;
    uint64_t m8t0 = perf::Now();
    std::vector<uint8_t>& lin = texScratch;   // M8: bufer reutilizado (antes un vector nuevo por subida)
    lin.resize((size_t)rowPitch * hBlocks * layers);
    bool fastPath = (fi.expand == 0 || ((fi.expand == 4 || fi.expand == 5) && fi.bpb == 4)) && !g_texSlowPath;
    if (fastPath) {   // M8: destilado por tramos + endian por fila (resultado identico al camino por bloque)
        for (uint32_t L = 0; L < layers; ++L) {
            uint8_t* dl = &lin[(size_t)L * layerPitchRows * rowPitch];
            DetileRaw(dl, rowPitch, mem + faceBytes * L, wBlocks, hBlocks, tiled, pitchAligned, bpbLog2, pitchPx / bw * fi.bpb);
            if (endian) for (uint32_t by = 0; by < hBlocks; ++by) SwapRow(dl + (size_t)by * rowPitch, (size_t)wBlocks * fi.bpb, fi.bpb, endian);
            if (fi.expand == 4 || fi.expand == 5)   // M8: conversion de profundidad por fila (mismas expresiones que el camino por bloque)
                for (uint32_t by = 0; by < hBlocks; ++by) {
                    uint32_t* row = reinterpret_cast<uint32_t*>(dl + (size_t)by * rowPitch);
                    if (fi.expand == 4) for (uint32_t bx = 0; bx < wBlocks; ++bx) { float f = (float)(row[bx] >> 8) / 16777215.0f; std::memcpy(&row[bx], &f, 4); }
                    else for (uint32_t bx = 0; bx < wBlocks; ++bx) { float f = xenos::Float20e4To32(row[bx] >> 8); std::memcpy(&row[bx], &f, 4); }
                }
        }
    }
    std::vector<uint8_t> fastCopy;
    if (fastPath && g_texVerify) { fastCopy = lin; fastPath = false; }   // M8: --tex-verify compara con el camino por bloque
    for (uint32_t L = 0; L < layers && !fastPath; ++L) {
        const uint8_t* lm = mem + faceBytes * L;
        for (uint32_t by = 0; by < hBlocks; ++by) {
            uint8_t* dst = &lin[((size_t)L * layerPitchRows + by) * rowPitch];
            for (uint32_t bx = 0; bx < wBlocks; ++bx) {
                size_t off = tiled ? (size_t)xe::gpu::texture_address::Tiled2D((int32_t)bx, (int32_t)by, pitchAligned, bpbLog2)
                                   : ((size_t)by * (pitchPx / bw) + bx) * fi.bpb;
                const uint8_t* s = lm + off;
                uint8_t* d = dst + (size_t)bx * hostBpb;
                // endian por palabras: 8in16 (1), 8in32 (2), 16in32 (3) sobre unidades de hasta 4 bytes
                uint8_t tmp[16];
                std::memcpy(tmp, s, fi.bpb);
                if (endian) {
                    if (fi.bpb >= 4) { for (uint32_t k = 0; k + 4 <= fi.bpb; k += 4) { uint32_t v; std::memcpy(&v, tmp + k, 4); v = GpuSwap32(v, endian); std::memcpy(tmp + k, &v, 4); } }
                    else if (fi.bpb == 2) { if (endian == 1 || endian == 2) std::swap(tmp[0], tmp[1]); }
                }
                switch (fi.expand) {
                    case 0: std::memcpy(d, tmp, fi.bpb); break;
                    case 1: { uint16_t v; std::memcpy(&v, tmp, 2);   // [NV] 5_6_5: R en los 5 bits bajos
                        d[0] = (uint8_t)(((v & 31) * 255 + 15) / 31); d[1] = (uint8_t)((((v >> 5) & 63) * 255 + 31) / 63); d[2] = (uint8_t)((((v >> 11) & 31) * 255 + 15) / 31); d[3] = 255; } break;
                    case 2: { uint16_t v; std::memcpy(&v, tmp, 2);   // [NV] 1_5_5_5
                        d[0] = (uint8_t)(((v & 31) * 255 + 15) / 31); d[1] = (uint8_t)((((v >> 5) & 31) * 255 + 15) / 31); d[2] = (uint8_t)((((v >> 10) & 31) * 255 + 15) / 31); d[3] = (v >> 15) ? 255 : 0; } break;
                    case 3: { uint16_t v; std::memcpy(&v, tmp, 2);   // [NV] 4_4_4_4
                        d[0] = (uint8_t)((v & 15) * 17); d[1] = (uint8_t)(((v >> 4) & 15) * 17); d[2] = (uint8_t)(((v >> 8) & 15) * 17); d[3] = (uint8_t)(((v >> 12) & 15) * 17); } break;
                    case 4: { uint32_t v; std::memcpy(&v, tmp, 4); float f = (float)(v >> 8) / 16777215.0f; std::memcpy(d, &f, 4); } break;
                    case 5: { uint32_t v; std::memcpy(&v, tmp, 4); float f = xenos::Float20e4To32(v >> 8); std::memcpy(d, &f, 4); } break;
                }
            }
        }
    }
    if (!fastCopy.empty()) {
        bool same = true;
        for (uint32_t L = 0; L < layers && same; ++L)
            for (uint32_t by = 0; by < hBlocks && same; ++by) {
                size_t o = ((size_t)L * layerPitchRows + by) * rowPitch;
                if (std::memcmp(&fastCopy[o], &lin[o], (size_t)wBlocks * fi.bpb)) same = false;
            }
        ++(same ? texVerifyOk : texVerifyBad);
        if (!same || texVerifyOk <= 3 || (texVerifyOk % 2000) == 0) ImportLog::Get().Printf("M8 tex-verify fmt=%s %ux%u tiled=%u endian=%u -> %s (ok=%llu mal=%llu)", TexFmtName(fmtId), width, height,
                                                               (unsigned)tiled, endian, same ? "IGUAL" : "DISTINTO", (unsigned long long)texVerifyOk, (unsigned long long)texVerifyBad);
    }
    // ---- recurso ----
    HostTexture& ht = textures[key];
    bool recreate = !ht.res || ht.w != width || ht.h != height || ht.dim != (cube ? 2u : 0u);
    if (recreate) {
        PERF_ZONE(kZoneTexCreate);
        D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; rd.Width = (width + bw - 1) / bw * bw; rd.Height = (height + bh - 1) / bh * bh;
        rd.DepthOrArraySize = (UINT16)layers; rd.MipLevels = 1; rd.Format = dfmt; rd.SampleDesc.Count = 1;
        ComPtr<ID3D12Resource> res;
        if (!Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&res)), "CreateCommittedResource(textura)")) {
            textures.erase(key); ++texFail; ++fTexFail; return nullptr;
        }
        bool fresh = ht.cpuSrv == 0 && !ht.res;
        ht.res = res; ht.w = width; ht.h = height; ht.dim = cube ? 2 : 0;
        if (fresh) {
            if (cpuSrvUsed >= 8192) { Note("OMISION demasiados SRV de textura"); textures.erase(key); ++texFail; ++fTexFail; return nullptr; }
            ht.cpuSrv = cpuSrvUsed++;
        }
        D3D12_SHADER_RESOURCE_VIEW_DESC sv = {};
        sv.Format = dfmt;
        sv.ViewDimension = cube ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
        // swizzle de la constante de fetch (3 bits por componente: X, Y, Z, W, 0, 1)
        uint32_t sw[4] = {tf.swizzle & 7u, (tf.swizzle >> 3) & 7u, (tf.swizzle >> 6) & 7u, (tf.swizzle >> 9) & 7u};
        uint32_t map = 0;
        for (int i = 0; i < 4; ++i) {
            uint32_t c = sw[i] <= 3 ? sw[i] : (sw[i] == 5 ? 5u : (sw[i] == 4 ? 4u : 0u));
            map |= c << (3 * i);
        }
        map |= (1u << 12);   // bit "siempre 1" de D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING
        sv.Shader4ComponentMapping = map;
        if (cube) sv.TextureCube.MipLevels = 1; else sv.Texture2D.MipLevels = 1;
        D3D12_CPU_DESCRIPTOR_HANDLE h = cpuSrvHeap->GetCPUDescriptorHandleForHeapStart();
        h.ptr += (SIZE_T)ht.cpuSrv * srvInc;
        dev->CreateShaderResourceView(ht.res.Get(), &sv, h);
        ++texCreated;
        if (texCreated <= 40)
            ImportLog::Get().Printf("D3D12 textura nueva #%llu fetch=%u fmt=%s %ux%u%s tiled=%u endian=%u base=%08X pitchpx=%u swz=%03X dxgi=%u", (unsigned long long)texCreated, idx,
                                    TexFmtName(fmtId), width, height, cube ? " cubo" : "", (unsigned)tiled, endian, baseAddr, pitchPx, (unsigned)tf.swizzle, (uint32_t)dfmt);
    }
    // ---- subida ----
    Transition(ht.res.Get(), recreate ? D3D12_RESOURCE_STATE_COPY_DEST : (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE), D3D12_RESOURCE_STATE_COPY_DEST);
    UploadAlloc u = Upload((uint32_t)lin.size(), 512);
    if (!u.cpu) { ++texFail; ++fTexFail; Note("OMISION textura demasiado grande para el anillo de subida"); return nullptr; }
    std::memcpy(u.cpu, lin.data(), lin.size());
    for (uint32_t L = 0; L < layers; ++L) {
        D3D12_TEXTURE_COPY_LOCATION dst = {}, src = {};
        dst.pResource = ht.res.Get(); dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; dst.SubresourceIndex = L;
        src.pResource = uploadBuf.Get(); src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint.Offset = u.offset + (uint64_t)L * layerPitchRows * rowPitch;
        src.PlacedFootprint.Footprint = {dfmt, (width + bw - 1) / bw * bw, (height + bh - 1) / bh * bh, 1, rowPitch};
        cl->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
    }
    Transition(ht.res.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    ht.dataHash = dataHash; ht.lastUse = ++texSerial;
    if (SeqLog()) { LARGE_INTEGER f; QueryPerformanceFrequency(&f); ImportLog::Get().Printf("M8SEQ UPLOAD fetch=%u base=%08X %ux%u fmt=%s expand=%d bytes=%zu us=%.0f", idx, baseAddr, width, height, TexFmtName(fmtId), fi.expand, lin.size(), (perf::Now() - m8t0) * 1e6 / f.QuadPart); }
    ++texUploads;
    if (textureDumps < opt.dumpTextures) {
        std::vector<uint8_t> img;
        uint32_t sw4[4] = {tf.swizzle & 7u, (tf.swizzle >> 3) & 7u, (tf.swizzle >> 6) & 7u, (tf.swizzle >> 9) & 7u};
        bool ok = true;
        if (fmtId == 18 || fmtId == 19 || fmtId == 20) DecodeBcToRgba(fmtId, lin.data(), rowPitch, width, height, img);
        else if (fmtId == 6 || fmtId == 14) { img.resize((size_t)width * height * 4); for (uint32_t y = 0; y < height; ++y) std::memcpy(&img[(size_t)y * width * 4], &lin[(size_t)y * rowPitch], (size_t)width * 4); }
        else if (fmtId == 2) { img.resize((size_t)width * height * 4); for (uint32_t y = 0; y < height; ++y) for (uint32_t x = 0; x < width; ++x) { uint8_t v = lin[(size_t)y * rowPitch + x]; uint8_t* o = &img[((size_t)y * width + x) * 4]; o[0] = o[1] = o[2] = v; o[3] = 255; } }
        else if (fmtId == 10) { img.resize((size_t)width * height * 4); for (uint32_t y = 0; y < height; ++y) for (uint32_t x = 0; x < width; ++x) { const uint8_t* s = &lin[(size_t)y * rowPitch + x * 2]; uint8_t* o = &img[((size_t)y * width + x) * 4]; o[0] = s[0]; o[1] = s[1]; o[2] = 0; o[3] = 255; } }
        else ok = false;
        if (ok) {
            // el volcado aplica el swizzle de la constante de fetch solo a los formatos de 4 canales; el resto se muestra en bruto
            if (fmtId == 6 || fmtId == 14 || fmtId == 18 || fmtId == 19 || fmtId == 20)
                for (size_t i = 0; i < img.size(); i += 4) { uint8_t c[4] = {img[i], img[i + 1], img[i + 2], img[i + 3]}; for (int k = 0; k < 4; ++k) img[i + k] = sw4[k] <= 3 ? c[sw4[k]] : (sw4[k] == 5 ? 255 : 0); }
            for (size_t i = 3; i < img.size(); i += 4) img[i] = 255;   // sin alfa para poder verlo
            char nm[128];
            std::snprintf(nm, sizeof(nm), "\\tex_%03u_%s_%ux%u_%08X.png", textureDumps, TexFmtName(fmtId), width, height, baseAddr);
            std::string path = rt::gpu::FramesDir() + nm;
            bool okp = WritePngRGBA(path, width, height, img.data(), width * 4);
            ImportLog::Get().Printf("D3D12 volcado de textura -> %s %s", path.c_str(), okp ? "OK" : "FALLO");
        }
        ++textureDumps;
    }
    static uint32_t k8logged = 0;
    if (texUploads <= 80 || (texUploads % 500) == 0 || (fmtId == 2 && k8logged++ < 150)) {
        uint64_t nz = 0;
        for (uint8_t b : lin) if (b) ++nz;
        ImportLog::Get().Printf("D3D12 subida de textura #%llu fetch=%u fmt=%s %ux%u base=%08X bytes_no_nulos=%llu/%zu hash=%016llX", (unsigned long long)texUploads, idx,
                                TexFmtName(fmtId), width, height, baseAddr, (unsigned long long)nz, lin.size(), (unsigned long long)dataHash);
    }
    return &ht;
}

} // namespace rt::gpu::d3d
