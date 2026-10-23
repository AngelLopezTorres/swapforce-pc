#include <immintrin.h>
#include "perf.h"
// H4: render targets del host y resolve (copia de EDRAM a memoria del guest).
// Simplificaciones (registradas como "SIMPLIFY ..." en runtime.log):
//   - un render target de textura por clave (color_base, pitch, formato); sin modelo de EDRAM (sin solapes ni transferencias de propiedad);
//   - MSAA tratado como 1X; tiles predicados sobre el RT completo; RTV/DSV convencionales (sin ROV);
//   - resolve v1: lectura a CPU de la region y escritura en la memoria guest con tiling (texture_address.h) y endian.
#include <algorithm>
#include <cmath>

#include "config.h"
#include "d3d12_impl.h"
#include "fb_decode.h"
#include "image_io.h"
#include "gpu_names.h"
#include "xenia/gpu/texture_address.h"

namespace rt::gpu::d3d {

DXGI_FORMAT ColorFormatToDxgi(uint32_t f, bool* approx) {
    if (approx) *approx = false;
    switch (f) {
        case 0: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case 1: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case 2: case 10: return DXGI_FORMAT_R10G10B10A2_UNORM;
        case 3: case 12: if (approx) *approx = true; return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case 4: if (approx) *approx = true; return DXGI_FORMAT_R16G16_SNORM;
        case 5: if (approx) *approx = true; return DXGI_FORMAT_R16G16B16A16_SNORM;
        case 6: return DXGI_FORMAT_R16G16_FLOAT;
        case 7: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case 14: return DXGI_FORMAT_R32_FLOAT;
        case 15: return DXGI_FORMAT_R32G32_FLOAT;
        default: return DXGI_FORMAT_UNKNOWN;
    }
}

static uint32_t BytesPerPixel(DXGI_FORMAT f) {
    switch (f) {
        case DXGI_FORMAT_R16G16B16A16_FLOAT: case DXGI_FORMAT_R16G16B16A16_SNORM: case DXGI_FORMAT_R32G32_FLOAT: return 8;
        default: return 4;
    }
}

HostRT* D3D12Backend::GetColorRT(uint32_t base, uint32_t pitch, uint32_t fmt, uint32_t needH) {
    uint64_t key = (uint64_t)base | ((uint64_t)pitch << 12) | ((uint64_t)fmt << 32);
    bool approx = false;
    DXGI_FORMAT df = ColorFormatToDxgi(fmt, &approx);
    if (df == DXGI_FORMAT_UNKNOWN) { Note("OMISION formato de render target de color no soportado=" + std::to_string(fmt)); return nullptr; }
    if (approx) {   // M60: el texto se construye solo la primera vez por formato (antes, en cada draw); el recuento sigue siendo por llamada
        static const char* lit[16] = {};
        static std::string txt[16];
        if (fmt < 16) {
            if (!lit[fmt]) { txt[fmt] = "SIMPLIFY formato de render target de color " + std::to_string(fmt) + " aproximado por un formato de PC (" + std::to_string((uint32_t)df) + ")"; lit[fmt] = txt[fmt].c_str(); }
            Note(lit[fmt]);
        } else Note("SIMPLIFY formato de render target de color " + std::to_string(fmt) + " aproximado por un formato de PC (" + std::to_string((uint32_t)df) + ")");
    }
    if (R<reg::RB_SURFACE_INFO>().msaa_samples != xenos::MsaaSamples::k1X) Note("SIMPLIFY MSAA tratado como 1X");
    uint32_t w = std::min(std::max(pitch, 16u), 8192u);
    uint32_t h = std::min(std::max(needH, 1024u), 8192u);
    auto it = colorRTs.find(key);
    if (it != colorRTs.end()) {
        HostRT& rt = it->second;
        if (rt.h >= needH) return &rt;
        h = std::min(((needH + 255) & ~255u), 8192u);
        Note("SIMPLIFY render target de color ampliado en altura (se pierde el contenido previo)");
    }
    HostRT& rt = colorRTs[key];
    D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; rd.Width = w; rd.Height = h; rd.DepthOrArraySize = 1; rd.MipLevels = 1;
    rd.Format = df; rd.SampleDesc.Count = 1; rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE cv = {df, {0, 0, 0, 0}};
    ComPtr<ID3D12Resource> res;
    if (!Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_RENDER_TARGET, &cv, IID_PPV_ARGS(&res)), "CreateCommittedResource(RT color)")) {
        colorRTs.erase(key);
        return nullptr;
    }
    bool fresh = !rt.res;
    if (fresh) {
        if (rtvUsed >= 128) { Note("OMISION demasiados render targets de color"); colorRTs.erase(key); return nullptr; }
        rt.viewIdx = rtvUsed++;
    }
    rt.res = res; rt.state = D3D12_RESOURCE_STATE_RENDER_TARGET; rt.format = df; rt.w = w; rt.h = h; rt.depth = false; rt.xenosFormat = fmt;
    D3D12_CPU_DESCRIPTOR_HANDLE hd = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    hd.ptr += (SIZE_T)rt.viewIdx * rtvInc;
    dev->CreateRenderTargetView(rt.res.Get(), nullptr, hd);
    char b[160];
    std::snprintf(b, sizeof(b), "RT color nuevo base=%u pitch=%u fmt=%s(%u) host=%ux%u dxgi=%u", base, pitch, ColorRtName(fmt), fmt, w, h, (uint32_t)df);
    ImportLog::Get().Printf("D3D12 %s", b);
    return &rt;
}

HostRT* D3D12Backend::GetDepthRT(uint32_t base, uint32_t pitch, uint32_t fmt, uint32_t needH) {
    uint64_t key = (uint64_t)base | ((uint64_t)pitch << 12) | ((uint64_t)fmt << 32);
    DXGI_FORMAT df = fmt == 0 ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    if (fmt != 0) Note("SIMPLIFY profundidad D24FS8 (float de 20e4) aproximada por D32_FLOAT");
    uint32_t w = std::min(std::max(pitch, 16u), 8192u);
    uint32_t h = std::min(std::max(needH, 1024u), 8192u);
    auto it = depthRTs.find(key);
    if (it != depthRTs.end()) {
        HostRT& rt = it->second;
        if (rt.h >= needH) return &rt;
        h = std::min(((needH + 255) & ~255u), 8192u);
    }
    HostRT& rt = depthRTs[key];
    D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; rd.Width = w; rd.Height = h; rd.DepthOrArraySize = 1; rd.MipLevels = 1;
    rd.Format = df; rd.SampleDesc.Count = 1; rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    D3D12_CLEAR_VALUE cv = {};
    cv.Format = df; cv.DepthStencil = {1.0f, 0};
    ComPtr<ID3D12Resource> res;
    if (!Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_DEPTH_WRITE, &cv, IID_PPV_ARGS(&res)), "CreateCommittedResource(RT profundidad)")) {
        depthRTs.erase(key);
        return nullptr;
    }
    bool fresh = !rt.res;
    if (fresh) {
        if (dsvUsed >= 64) { Note("OMISION demasiados render targets de profundidad"); depthRTs.erase(key); return nullptr; }
        rt.viewIdx = dsvUsed++;
    }
    rt.res = res; rt.state = D3D12_RESOURCE_STATE_DEPTH_WRITE; rt.format = df; rt.w = w; rt.h = h; rt.depth = true; rt.xenosFormat = fmt;
    D3D12_CPU_DESCRIPTOR_HANDLE hd = dsvHeap->GetCPUDescriptorHandleForHeapStart();
    hd.ptr += (SIZE_T)rt.viewIdx * dsvInc;
    dev->CreateDepthStencilView(rt.res.Get(), nullptr, hd);
    ImportLog::Get().Printf("D3D12 RT profundidad nuevo base=%u pitch=%u fmt=%u host=%ux%u dxgi=%u", base, pitch, fmt, w, h, (uint32_t)df);
    return &rt;
}

bool g_resolveSlowPath = false, g_resolveVerify = false;
bool g_resolveSync = false;          // M8: --resolve-sync (espera y escritura inmediata por resolve, comportamiento anterior)
bool g_noResolveShortcut = false;   // M8: --no-resolve-shortcut (sin atajo GPU para texturas resueltas)
bool g_resolveNoDse = false;
bool g_depthResolveSync = false;     // M60: --depth-resolve-sync (resolve de profundidad con espera en el hilo del CP, ronda 7)
bool g_depthResolveVerify = false;   // M60: --depth-resolve-verify (diferido y sincronico del mismo resolve: memoria guest identica)

// M60: plano de profundidad float32 (fila srcPitch, plano completo) -> filas de 32 bits 20e4<<8 de la region (fila pr.rowPitch).
// Misma conversion que la ronda 7 (Float32To20e4(depth * 2, redondeo al par) << 8).
const uint8_t* D3D12Backend::DepthResolveSource(const PendingResolve& pr, const uint8_t* src, std::vector<uint8_t>& scratch, bool parallel) {
    scratch.resize((size_t)pr.rowPitch * pr.h);
    uint8_t* dst = scratch.data();
    auto conv = [&](uint32_t yb, uint32_t ye) {
        for (uint32_t y = yb; y < ye; ++y) {
            const uint8_t* row = src + (size_t)(pr.y0 + y) * pr.srcPitch + (size_t)pr.x0 * 4;
            uint32_t* out = (uint32_t*)(dst + (size_t)y * pr.rowPitch);
            for (uint32_t x = 0; x < pr.w; ++x) {
                float depth; std::memcpy(&depth, row + x * 4, 4);
                out[x] = xenos::Float32To20e4(depth * 2.0f, true) << 8;
            }
        }
        return true;
    };
    if (parallel) ParallelRows(pr.h, conv); else conv(0, pr.h);
    return dst;
}

bool g_asyncResolveWriter = true;   // M8: --sync-resolve-writer (escritura en el hilo del CP en el punto de sincronizacion)        // M8: --no-resolve-dse (escribe tambien los resolves cubiertos por otro posterior)   // M8: --resolve-slow-path (escritura por pixel original)

// M8: igual que TiledRunsContiguous de d3d12_tex.cpp (16 >> bppLog2 pixeles consecutivos en x a direcciones consecutivas).
static bool TiledRunsContiguousRt(uint32_t bppLog2) {
    static int cache[5] = {-1, -1, -1, -1, -1};
    if (bppLog2 > 4) return false;
    if (cache[bppLog2] >= 0) return cache[bppLog2] != 0;
    uint32_t run = std::max(1u, 16u >> bppLog2), bpb = 1u << bppLog2;
    bool ok = true;
    const uint32_t pitches[3] = {32, 128, 1280};
    for (uint32_t pi = 0; pi < 3 && ok; ++pi)
        for (int32_t y = 0; y < 96 && ok; ++y)
            for (int32_t x = 0; x < 128 && ok; x += (int32_t)run) {
                int32_t o0 = xe::gpu::texture_address::Tiled2D(x, y, pitches[pi], bppLog2);
                for (uint32_t i = 1; i < run; ++i)
                    if (xe::gpu::texture_address::Tiled2D(x + (int32_t)i, y, pitches[pi], bppLog2) != o0 + (int32_t)(i * bpb)) { ok = false; break; }
            }
    cache[bppLog2] = ok ? 1 : 0;
    ImportLog::Get().Printf("M8 resolve: Tiled2D bppLog2=%u tramos contiguos de %u -> %s", bppLog2, run, ok ? "SI" : "NO");
    return ok;
}

// M8: filas [yBeg, yEnd) de un resolve: tramo de 16 bytes -> pshufb(perm) -> destino Tiled2D. Resto (< 16 bytes): por bytes.
__attribute__((target("ssse3"))) static bool ResolveRowsSse(const uint8_t* srcp, uint32_t rowPitch, uint8_t* mem, int64_t lo, uint32_t destBase,
                                                          uint32_t dpAligned, uint32_t bppLog2, uint32_t bpp, int32_t x0, int32_t y0, uint32_t w,
                                                          uint32_t yBeg, uint32_t yEnd, uint32_t run, const uint8_t* perm) {
    const __m128i m = _mm_load_si128((const __m128i*)perm);
    for (uint32_t y = yBeg; y < yEnd; ++y) {
        const uint8_t* srow = srcp + (size_t)y * rowPitch;
        for (uint32_t x = 0; x < w; x += run) {
            uint32_t n = std::min(run, w - x);
            int64_t off = (int64_t)destBase + xe::gpu::texture_address::Tiled2D((int32_t)(x0 + x), (int32_t)(y0 + y), dpAligned, bppLog2) - lo;
            if (n == run) {
                __m128i v = _mm_loadu_si128((const __m128i*)(srow + (size_t)x * bpp));
                _mm_storeu_si128((__m128i*)(mem + off), _mm_shuffle_epi8(v, m));
            } else {
                uint8_t tmp[16];
                std::memcpy(tmp, srow + (size_t)x * bpp, (size_t)n * bpp);
                uint8_t o[16];
                for (uint32_t k = 0; k < n * bpp; ++k) o[k] = tmp[perm[k]];
                std::memcpy(mem + off, o, (size_t)n * bpp);
            }
        }
    }
    return true;
}

// Escritura de un pixel de 32 bits en la memoria del guest con el endian del resolve (Endian128: solo 0-3 para 32 bpp).
static inline uint32_t SwapE(uint32_t v, uint32_t e) { return GpuSwap32(v, e & 3); }

bool D3D12Backend::DoResolve(const DrawEvent& ev) {
    PERF_ZONE(kZoneResolve);
    (void)ev;
    auto cc = R<reg::RB_COPY_CONTROL>();
    if (cc.copy_command != xenos::CopyCommand::kRaw && cc.copy_command != xenos::CopyCommand::kConvert) {
        Note("OMISION resolve con copy_command=" + std::to_string((uint32_t)cc.copy_command)); return false;
    }
    // Region: vertices del rectangulo en la constante de fetch de vertice 0 (D3D9: siempre vf0, escritos por la CPU).
    uint32_t d0 = RV(0x4800), d1 = RV(0x4801);
    if ((d0 & 3) != 3 || ((d1 >> 2) & 0xFFFFFF) != 6) { Note("OMISION resolve: formato de buffer de vertices del rectangulo no soportado"); return false; }
    const uint8_t* vp = PhysHost(d0 & ~3u & 0x1FFFFFFF, 24);
    if (!vp) { Note("OMISION resolve: vertices del rectangulo en memoria no asignada"); return false; }
    int32_t vf[6];
    auto vtx = R<reg::PA_SU_VTX_CNTL>();
    float half = vtx.pix_center == xenos::PixelCenter::kD3DZero ? 0.5f : 0.0f;
    for (int i = 0; i < 6; ++i) {
        uint32_t raw; std::memcpy(&raw, vp + i * 4, 4);
        uint32_t sw = GpuSwap32(raw, d1 & 3);
        float f; std::memcpy(&f, &sw, 4);
        vf[i] = (int32_t)std::lrintf((f + half) * 256.0f);
    }
    int32_t x0 = std::min(std::min(vf[0], vf[2]), vf[4]), y0 = std::min(std::min(vf[1], vf[3]), vf[5]);
    int32_t x1 = std::max(std::max(vf[0], vf[2]), vf[4]), y1 = std::max(std::max(vf[1], vf[3]), vf[5]);
    x0 = (x0 + 127) >> 8; y0 = (y0 + 127) >> 8; x1 = (x1 + 127) >> 8; y1 = (y1 + 127) >> 8;
    if (R<reg::PA_SU_SC_MODE_CNTL>().vtx_window_offset_enable) {
        auto wo = R<reg::PA_SC_WINDOW_OFFSET>();
        x0 += wo.window_x_offset; y0 += wo.window_y_offset; x1 += wo.window_x_offset; y1 += wo.window_y_offset;
    }
    {   // scissor (sin recorte al pitch todavia)
        auto tl = R<reg::PA_SC_WINDOW_SCISSOR_TL>(); auto br = R<reg::PA_SC_WINDOW_SCISSOR_BR>();
        int32_t tlx = tl.tl_x, tly = tl.tl_y, brx = br.br_x, bry = br.br_y;
        if (!tl.window_offset_disable) { auto wo = R<reg::PA_SC_WINDOW_OFFSET>(); tlx += wo.window_x_offset; tly += wo.window_y_offset; brx += wo.window_x_offset; bry += wo.window_y_offset; }
        auto stl = R<reg::PA_SC_SCREEN_SCISSOR_TL>(); auto sbr = R<reg::PA_SC_SCREEN_SCISSOR_BR>();
        tlx = std::max(tlx, (int32_t)stl.tl_x); tly = std::max(tly, (int32_t)stl.tl_y); brx = std::min(brx, (int32_t)sbr.br_x); bry = std::min(bry, (int32_t)sbr.br_y);
        tlx = std::max(tlx, 0); tly = std::max(tly, 0); brx = std::max(brx, tlx); bry = std::max(bry, tly);
        x0 = std::clamp(x0, tlx, brx); y0 = std::clamp(y0, tly, bry); x1 = std::clamp(x1, tlx, brx); y1 = std::clamp(y1, tly, bry);
    }
    x0 &= ~7; y0 &= ~7; x1 = (x1 + 7) & ~7; y1 = (y1 + 7) & ~7;
    auto surf = R<reg::RB_SURFACE_INFO>();
    int32_t pitchAligned8 = (int32_t)(surf.surface_pitch & ~7u);
    if (x1 > pitchAligned8) { x0 = std::min(x0, pitchAligned8); x1 = std::min(x1, pitchAligned8); }
    if (x0 >= x1 || y0 >= y1) { Note("OMISION resolve: region vacia"); return false; }
    if (surf.msaa_samples != xenos::MsaaSamples::k1X) Note("SIMPLIFY MSAA tratado como 1X (resolve sin promediar muestras)");
    uint32_t w = (uint32_t)(x1 - x0), h = (uint32_t)(y1 - y0);
    uint32_t srcSel = cc.copy_src_select;
    bool isDepth = srcSel >= 4;
    auto dinfo = R<reg::RB_COPY_DEST_INFO>();
    auto dpitch = R<reg::RB_COPY_DEST_PITCH>();
    uint32_t destBase = RV(0x2319);
    HostRT* src = nullptr;
    uint32_t srcBase = 0;
    uint32_t needH = ((uint32_t)y1 + 31u) & ~31u;
    if (!isDepth) {
        auto ci = R<reg::RB_COLOR_INFO>(); ci.value = RV(srcSel == 0 ? 0x2001 : 0x2003 + (srcSel - 1));
        srcBase = ci.color_base | (ci.color_base_bit_11 << 11);
        src = GetColorRT(srcBase, surf.surface_pitch, (uint32_t)ci.color_format, needH);
        if (!src) { Note("OMISION resolve: sin render target de color de origen"); return false; }
    }
    bool wrote = false;
    if (src) {
        uint32_t dfmt = (uint32_t)dinfo.copy_dest_format;
        uint32_t sfmt = src->xenosFormat;
        if (Cfg().dumpResolveFrame && FrameCount() + 1 == Cfg().dumpResolveFrame) {
            uint32_t di = RV(0x2002);
            ImportLog::Get().Printf("D3D12 RESOLVEFRAME frame=%llu draw=%llu srcsel=%u surface=%08X color_info=%08X depth_info=%08X copy_control=%08X copy_dest=%08X dest_pitch=%08X clear_value=%08X src_base=%08X srcfmt=%u dst=%08X dstfmt=%u rect=%d,%d-%d,%d msaa=%u clear=%u/%u",
                                    (unsigned long long)(FrameCount() + 1), (unsigned long long)drawSerial, srcSel, RV(0x2000), RV(srcSel == 0 ? 0x2001 : 0x2003 + (srcSel - 1)), di,
                                    RV(0x2318), RV(0x231A), RV(0x231B), RV(0x231D), srcBase,
                                    sfmt, destBase, dfmt, x0, y0, x1, y1, (RV(0x2000) >> 16) & 3,
                                    (uint32_t)cc.color_clear_enable, (uint32_t)cc.depth_clear_enable);
        }
        // formatos de destino admitidos en la v1 (combinaciones bit a bit equivalentes, ver IsColorResolveFormatBitwiseEquivalent de xenos.h)
        bool ok32 = (sfmt == 0 || sfmt == 1) && (dfmt == 6 || dfmt == 14 || dfmt == 50);
        bool ok1010 = (sfmt == 2 || sfmt == 10) && (dfmt == 7 || dfmt == 54);
        bool okF32 = sfmt == 14 && dfmt == 36;
        bool ok1616f = sfmt == 6 && dfmt == 31;
        bool ok16x4f = sfmt == 7 && dfmt == 32;
        if (!(ok32 || ok1010 || okF32 || ok1616f || ok16x4f)) {
            Note("SIMPLIFY resolve de formato RT=" + std::to_string(sfmt) + " a formato de destino=" + std::to_string(dfmt) + " no implementado: destino sin escribir");
        } else {
            PendingResolve pr;
            pr.frame = FrameCount() + 1;
            pr.bpp = BytesPerPixel(src->format);
            pr.bppLog2 = pr.bpp == 8 ? 3 : 2;
            pr.rowPitch = (w * pr.bpp + 255) & ~255u;
            pr.w = w; pr.h = h; pr.x0 = x0; pr.y0 = y0; pr.x1 = x1; pr.y1 = y1;
            pr.destBase = destBase; pr.endian = (uint32_t)dinfo.copy_dest_endian; pr.swapRB = dinfo.copy_dest_swap != 0;
            pr.ok32 = ok32; pr.sfmt = sfmt; pr.dfmt = dfmt; pr.format = src->format;
            // direccionamiento del destino (texture_address.h): base + Tiled2D(x, y) con las mismas coordenadas que el RT
            pr.dpAligned = (dpitch.copy_dest_pitch + 31) & ~31u;
            if (!pr.dpAligned) pr.dpAligned = (surf.surface_pitch + 31) & ~31u;
            uint32_t ax0 = (uint32_t)x0 & ~31u, ay0 = (uint32_t)y0 & ~31u, ax1 = ((uint32_t)x1 + 31) & ~31u, ay1 = ((uint32_t)y1 + 31) & ~31u;
            pr.lo = (int64_t)destBase + xe::gpu::texture_address::Tiled2D((int32_t)ax0, (int32_t)ay0, pr.dpAligned, pr.bppLog2);
            pr.hi = (int64_t)destBase + xe::gpu::texture_address::Tiled2D((int32_t)ax1 - 32, (int32_t)ay1 - 32, pr.dpAligned, pr.bppLog2) + 32 * 32 * (1 << pr.bppLog2);
            pr.mem = nullptr;
            if (pr.lo >= 0 && pr.hi > pr.lo && pr.hi <= 0x20000000) pr.mem = PhysHost((uint32_t)pr.lo, (uint32_t)(pr.hi - pr.lo));
            if (SeqLog()) ImportLog::Get().Printf("M8SEQ draw=%llu RESOLVE dest=%08X lo=%08X hi=%08X %ux%u at(%d,%d) fmt=%u endian=%u srcfmt=%u", (unsigned long long)drawSerial, destBase, (uint32_t)pr.lo, (uint32_t)pr.hi, w, h, x0, y0, dfmt, pr.endian, sfmt);
            uint32_t ilo = (uint32_t)std::max<int64_t>(pr.lo, 0), ihi = (uint32_t)std::min<int64_t>(pr.hi, 0x20000000);
            InvalidateTexMemo(ilo, ihi);
            InvalidateResolvedTex(ilo, ihi);
            if (pr.mem) frameResolves.push_back({(uint32_t)pr.lo, (uint32_t)pr.hi, destBase, w, h, dfmt, pr.endian, (uint32_t)x0, (uint32_t)y0});
            if (!pr.mem) {
                Note("OMISION resolve: memoria de destino no asignada o fuera de rango");
            } else {
                Transition(src->res.Get(), src->state, D3D12_RESOURCE_STATE_COPY_SOURCE); src->state = D3D12_RESOURCE_STATE_COPY_SOURCE;
                D3D12_TEXTURE_COPY_LOCATION sl = {};
                sl.pResource = src->res.Get(); sl.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; sl.SubresourceIndex = 0;
                D3D12_BOX box = {(UINT)x0, (UINT)y0, 0, (UINT)x1, (UINT)y1, 1};
                // M8 (atajo GPU): copia de la region del RT a una textura del host para las lecturas del mismo fotograma que coinciden
                // exactamente con este resolve (ver LookupResolvedTex); el contenido en bits es el que tendria la textura destilada.
                if (!g_resolveSync && !g_noResolveShortcut && x0 == 0 && y0 == 0 && ((sfmt == 0 && dfmt == 6) || (sfmt == 14 && dfmt == 36)))
                    RecordResolvedTex(pr, sl, box);
                uint64_t need = (uint64_t)pr.rowPitch * h;
                if (g_resolveSync) {   // comportamiento anterior: espera y escritura inmediata
                    if (!resolveReadback || resolveReadbackBytes < need) {
                        D3D12_HEAP_PROPERTIES rp = {D3D12_HEAP_TYPE_READBACK};
                        D3D12_RESOURCE_DESC bd = {};
                        resolveReadbackBytes = std::max<uint64_t>(need, 8u << 20);
                        bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; bd.Width = resolveReadbackBytes; bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
                        bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                        resolveReadback.Reset();
                        if (!Hr(dev->CreateCommittedResource(&rp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resolveReadback)), "readback del resolve")) return false;
                    }
                    D3D12_TEXTURE_COPY_LOCATION dst = {};
                    dst.pResource = resolveReadback.Get(); dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                    dst.PlacedFootprint.Footprint = {src->format, w, h, 1, pr.rowPitch};
                    cl->CopyTextureRegion(&dst, 0, 0, 0, &sl, &box);
                    FlushAndWait();
                    void* p = nullptr;
                    D3D12_RANGE rr = {0, (SIZE_T)need};
                    if (!Hr(resolveReadback->Map(0, &rr, &p), "Map(readback del resolve)")) return false;
                    WriteResolve(pr, (const uint8_t*)p);
                    D3D12_RANGE wr = {0, 0};
                    resolveReadback->Unmap(0, &wr);
                } else {   // M8: diferido hasta el siguiente punto de sincronizacion (FlushResolves)
                    if (!EnsureResolvePool(need)) return false;
                    pr.poolOffset = resolvePoolOffset;
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT pfp = {};
                    pfp.Offset = pr.poolOffset;
                    pfp.Footprint = {src->format, w, h, 1, pr.rowPitch};
                    CopyToResolvePool(sl, &box, pfp, need, pr);
                    if (g_gpuTiming) { auto& e = tsResolveBytes[((uint64_t)destBase << 32) | (w << 16) | h]; e.first += need; ++e.second; }
                    TsMark(2, 0);   // M60
                    resolvePoolOffset = (resolvePoolOffset + need + 511) & ~511ull;
                    EnqueueResolve(pr);
                    if (g_kickDraws && drawsSinceKick >= g_kickResolveDraws) Kick();   // M8: la copia y el trabajo previo empiezan en la GPU
                }
                wrote = true;
            }
        }
    } else {
        auto di = R<reg::RB_DEPTH_INFO>();
        srcBase = di.depth_base | (di.depth_base_bit_11 << 11);
        src = GetDepthRT(srcBase, surf.surface_pitch, (uint32_t)di.depth_format, needH);
        if (src && (uint32_t)di.depth_format == 1 &&
            ((uint32_t)dinfo.copy_dest_format == 6 || (uint32_t)dinfo.copy_dest_format == 23)) {
            // D32_FLOAT_S8X24: plano 0, subrecurso completo. D3D12 no permite copiar
            // una subregion de un recurso de profundidad (CopyTextureRegion).
            D3D12_RESOURCE_DESC desc = src->res->GetDesc();
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp = {};
            UINT rows = 0;
            UINT64 rowBytes = 0, totalBytes = 0;
            dev->GetCopyableFootprints(&desc, 0, 1, 0, &fp, &rows, &rowBytes, &totalBytes);
            if ((fp.Footprint.Format == DXGI_FORMAT_R32_TYPELESS || fp.Footprint.Format == DXGI_FORMAT_R32_FLOAT) &&
                totalBytes && totalBytes <= (64ull << 20)) {
                D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_READBACK};
                D3D12_RESOURCE_DESC bd = {};
                bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; bd.Width = totalBytes;
                bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
                bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                // M60: descriptor comun del resolve de profundidad (antes se construia tras la espera)
                PendingResolve pr;
                pr.frame = FrameCount() + 1; pr.bpp = 4; pr.bppLog2 = 2;
                pr.rowPitch = (w * 4 + 255) & ~255u;
                pr.w = w; pr.h = h; pr.x0 = x0; pr.y0 = y0; pr.x1 = x1; pr.y1 = y1;
                pr.destBase = destBase; pr.endian = (uint32_t)dinfo.copy_dest_endian;
                pr.dfmt = (uint32_t)dinfo.copy_dest_format;
                pr.dpAligned = (dpitch.copy_dest_pitch + 31) & ~31u;
                if (!pr.dpAligned) pr.dpAligned = (surf.surface_pitch + 31) & ~31u;
                pr.depth = true; pr.srcPitch = fp.Footprint.RowPitch;
                {
                    uint32_t ax0 = (uint32_t)x0 & ~31u, ay0 = (uint32_t)y0 & ~31u;
                    uint32_t ax1 = ((uint32_t)x1 + 31) & ~31u, ay1 = ((uint32_t)y1 + 31) & ~31u;
                    pr.lo = (int64_t)destBase + xe::gpu::texture_address::Tiled2D((int32_t)ax0, (int32_t)ay0, pr.dpAligned, 2);
                    pr.hi = (int64_t)destBase + xe::gpu::texture_address::Tiled2D((int32_t)ax1 - 32, (int32_t)ay1 - 32, pr.dpAligned, 2) + 4096;
                }
                if (pr.lo >= 0 && pr.hi > pr.lo && pr.hi <= 0x20000000)
                    pr.mem = PhysHost((uint32_t)pr.lo, (uint32_t)(pr.hi - pr.lo));
                static uint64_t depthWrites = 0;
                const bool deferred = !g_depthResolveSync && !g_resolveSync && g_asyncResolveWriter;
                std::vector<uint8_t> verifySnap;
                if (pr.mem && deferred) {
                    // M60: copia del plano completo al bufer de lectura diferido, sin esperar; conversion y escritura en el
                    // hilo escritor en orden con los demas resolves (mismo punto de la lista de comandos que el camino sincronico).
                    InvalidateTexMemo((uint32_t)pr.lo, (uint32_t)pr.hi);
                    InvalidateResolvedTex((uint32_t)pr.lo, (uint32_t)pr.hi);
                    const uint64_t regionBytes = (uint64_t)pr.rowPitch * h;
                    const bool useCs = g_depthResolveCs && g_resolveStaging;
                    if (EnsureResolvePool(useCs ? std::max(regionBytes, totalBytes) : totalBytes)) {
                        pr.poolOffset = resolvePoolOffset;
                        uint64_t used = totalBytes;
                        if (useCs && DepthRegionToStage(src, pr)) {   // M60: solo la region, ya convertida a 20e4<<8
                            pr.depth = false;
                            used = regionBytes;
                        } else {
                            Transition(src->res.Get(), src->state, D3D12_RESOURCE_STATE_COPY_SOURCE);
                            src->state = D3D12_RESOURCE_STATE_COPY_SOURCE;
                            D3D12_TEXTURE_COPY_LOCATION from = {};
                            from.pResource = src->res.Get(); from.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; from.SubresourceIndex = 0;
                            D3D12_PLACED_SUBRESOURCE_FOOTPRINT pfp = fp;
                            pfp.Offset = pr.poolOffset;
                            CopyToResolvePool(from, nullptr, pfp, totalBytes, pr);
                        }
                        if (g_gpuTiming) { auto& e = tsResolveBytes[((uint64_t)destBase << 32) | (w << 16) | h | 0x80000000u]; e.first += used; ++e.second; }
                        TsMark(3, 0);   // M60
                        resolvePoolOffset = (resolvePoolOffset + used + 511) & ~511ull;
                        EnqueueResolve(pr);
                        wrote = true;
                        if (++depthWrites <= 4 || depthWrites % 500 == 0)
                            ImportLog::Get().Printf("D3D12 DEPTH_RESOLVE diferido dst=%08X %ux%u writes=%llu", destBase, w, h, (unsigned long long)depthWrites);
                        if (g_depthResolveVerify) {   // M60: escribe el diferido, guarda la memoria y repite por el camino sincronico
                            FlushResolves();
                            verifySnap.assign(pr.mem, pr.mem + (pr.hi - pr.lo));
                        }
                    }
                }
                bool ready = pr.mem && (!deferred || !verifySnap.empty());
                if (ready && (!depthReadback || depthReadbackBytes < totalBytes)) {
                    depthReadback.Reset();
                    depthReadbackBytes = totalBytes;
                    ready = Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST,
                                                            nullptr, IID_PPV_ARGS(&depthReadback)), "CreateCommittedResource(resolve profundidad)");
                }
                if (ready) {   // camino sincronico (ronda 7): espera de la GPU en el hilo del CP
                    Transition(src->res.Get(), src->state, D3D12_RESOURCE_STATE_COPY_SOURCE);
                    src->state = D3D12_RESOURCE_STATE_COPY_SOURCE;
                    D3D12_TEXTURE_COPY_LOCATION from = {}, to = {};
                    from.pResource = src->res.Get(); from.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; from.SubresourceIndex = 0;
                    to.pResource = depthReadback.Get(); to.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT; to.PlacedFootprint = fp;
                    cl->CopyTextureRegion(&to, 0, 0, 0, &from, nullptr);
                    FlushAndWait();
                    void* mapped = nullptr;
                    D3D12_RANGE readRange = {0, (SIZE_T)totalBytes};
                    if (Hr(depthReadback->Map(0, &readRange, &mapped), "Map(resolve profundidad)")) {
                        FlushResolvesIfOverlap((uint32_t)pr.lo, (uint32_t)pr.hi);
                        InvalidateTexMemo((uint32_t)pr.lo, (uint32_t)pr.hi);
                        InvalidateResolvedTex((uint32_t)pr.lo, (uint32_t)pr.hi);
                        WriteResolve(pr, DepthResolveSource(pr, (const uint8_t*)mapped, depthResolveScratch, false));
                        wrote = true;
                        if (!verifySnap.empty()) {
                            static uint64_t vOk = 0, vBad = 0;
                            bool same = std::memcmp(verifySnap.data(), pr.mem, verifySnap.size()) == 0;
                            ++(same ? vOk : vBad);
                            if (!same || vOk <= 3 || vOk % 200 == 0)
                                ImportLog::Get().Printf("M60 depth-resolve-verify dst=%08X %ux%u bytes=%zu -> %s (ok=%llu mal=%llu)", destBase, w, h, verifySnap.size(),
                                                        same ? "IGUAL" : "DISTINTO", (unsigned long long)vOk, (unsigned long long)vBad);
                        } else if (++depthWrites <= 4 || depthWrites % 500 == 0)
                            ImportLog::Get().Printf("D3D12 DEPTH_RESOLVE dst=%08X %ux%u writes=%llu", destBase, w, h, (unsigned long long)depthWrites);
                        D3D12_RANGE wr = {0, 0}; depthReadback->Unmap(0, &wr);
                    }
                }
            } else Note("SIMPLIFY resolve de profundidad: formato de plano no soportado=" + std::to_string((uint32_t)fp.Footprint.Format));
        } else Note("SIMPLIFY resolve de profundidad no implementado para este formato");
    }
    // ---- limpiezas del resolve (RB_COPY_CONTROL.color_clear_enable / depth_clear_enable) sobre la region ----
    D3D12_RECT rect = {x0, y0, x1, y1};
    if (cc.color_clear_enable && src) {
        uint32_t cv = RV(0x231E);
        float col[4] = {0, 0, 0, 0};
        switch (src->xenosFormat) {
            case 0: case 1:
                for (int j = 0; j < 4; ++j) col[j] = (float)((cv >> (8 * j)) & 0xFF) / 255.0f;
                if (src->xenosFormat == 1) for (int j = 0; j < 3; ++j) col[j] = xenos::PWLGammaToLinear(col[j]);
                break;
            case 2: case 10:
                for (int j = 0; j < 3; ++j) col[j] = (float)((cv >> (10 * j)) & 0x3FF) / 1023.0f;
                col[3] = (float)((cv >> 30) & 3) / 3.0f;
                break;
            case 14: col[0] = (float)cv; break;
            default: Note("SIMPLIFY limpieza de color para formato RT=" + std::to_string(src->xenosFormat) + " aproximada a negro"); break;
        }
        Transition(src->res.Get(), src->state, D3D12_RESOURCE_STATE_RENDER_TARGET); src->state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        D3D12_CPU_DESCRIPTOR_HANDLE hd = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        hd.ptr += (SIZE_T)src->viewIdx * rtvInc;
        cl->ClearRenderTargetView(hd, col, 1, &rect);
    }
    if (cc.depth_clear_enable) {
        auto di = R<reg::RB_DEPTH_INFO>();
        HostRT* d = GetDepthRT(di.depth_base | (di.depth_base_bit_11 << 11), surf.surface_pitch, (uint32_t)di.depth_format, needH);
        if (d) {
            uint32_t cv = RV(0x231D);
            uint32_t d24 = (cv >> 8) & 0xFFFFFF;
            float dv = d->xenosFormat == 0 ? xenos::UNorm24To32(d24) : xenos::Float20e4To32(d24) * 0.5f;
            Transition(d->res.Get(), d->state, D3D12_RESOURCE_STATE_DEPTH_WRITE); d->state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
            D3D12_CPU_DESCRIPTOR_HANDLE hd = dsvHeap->GetCPUDescriptorHandleForHeapStart();
            hd.ptr += (SIZE_T)d->viewIdx * dsvInc;
            cl->ClearDepthStencilView(hd, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, dv, (UINT8)(cv & 0xFF), 1, &rect);
        }
    }
    static uint64_t logged = 0;
    if (logged < 12 || (logged % 200) == 0)
        ImportLog::Get().Printf("D3D12 RESOLVE #%llu region=(%d,%d)-(%d,%d) src=%s(%u) dest=%08X pitch=%u fmt=%u endian=%u swap=%u escrito=%d nonzero=%llu/%llu clear(color=%u,depth=%u)",
                                (unsigned long long)logged, x0, y0, x1, y1, isDepth ? "profundidad" : "color", srcSel, destBase, dpitch.copy_dest_pitch,
                                (uint32_t)dinfo.copy_dest_format, (uint32_t)dinfo.copy_dest_endian, (uint32_t)dinfo.copy_dest_swap, (int)wrote, (unsigned long long)lastResolveNonzero, (unsigned long long)lastResolvePixels,
                                (uint32_t)cc.color_clear_enable, (uint32_t)cc.depth_clear_enable);
    ++logged;
    return true;
}


// ---- M8: escritura de un resolve en la memoria guest (comun al camino sincrono y al diferido) ----
void D3D12Backend::WriteResolve(const PendingResolve& pr, const uint8_t* srcp) {
    const uint32_t w = pr.w, h = pr.h, bpp = pr.bpp, bppLog2 = pr.bppLog2, rowPitch = pr.rowPitch, endian = pr.endian;
    const int32_t x0 = pr.x0, y0 = pr.y0;
    const int64_t lo = pr.lo, hi = pr.hi;
    const uint32_t destBase = pr.destBase, dpAligned = pr.dpAligned;
    const bool swapRB = pr.swapRB, ok32 = pr.ok32;
    uint8_t* mem = pr.mem;
    // camino rapido: fila convertida (swap R/B + endian) en un bufer y copia por tramos de 16 bytes contiguos de
    // Tiled2D (comprobado en TiledRunsContiguousRt); x0 es multiplo de 8, por lo que los tramos quedan alineados.
    bool fastW = !g_resolveSlowPath && (x0 & 3) == 0 && TiledRunsContiguousRt(bppLog2);
    if (fastW) {
        uint32_t run = 16u >> bppLog2;
        // permutacion de bytes de un tramo de 16 bytes (4 pixeles de 32 bits o 2 de 64): byte de salida j = byte de entrada perm[j]
        // (mismas operaciones que el camino por pixel: swap R/B y despues GpuSwap32/endian 4).
        alignas(16) uint8_t perm[16];
        auto e = [&](uint32_t j) -> uint32_t { switch (endian & 3) { case 1: return j ^ 1; case 2: return 3 - j; case 3: return j ^ 2; default: return j; } };
        for (uint32_t k = 0; k < 16; ++k) {
            uint32_t word = k / 4, j = k % 4;
            if (bpp == 4) {
                uint32_t i = e(j);
                if (ok32 && swapRB) i = i == 0 ? 2 : (i == 2 ? 0 : i);
                perm[k] = (uint8_t)(word * 4 + i);
            } else {
                if (endian == 4) perm[k] = (uint8_t)((word ^ 1) * 4 + (3 - j));
                else perm[k] = (uint8_t)(word * 4 + e(j));
            }
        }
        auto rowsFn = [&](uint32_t yBeg, uint32_t yEnd) { return ResolveRowsSse(srcp, rowPitch, mem, lo, destBase, dpAligned, bppLog2, bpp, x0, y0, w, yBeg, yEnd, run, perm); };
        if (!ParallelRows(h, rowsFn)) fastW = false;
    }
    std::vector<uint8_t> fastSnap;
    if (fastW && g_resolveVerify) { fastSnap.assign(mem, mem + (hi - lo)); fastW = false; }   // M8: --resolve-verify
    for (uint32_t y = 0; y < h && !fastW; ++y)
        for (uint32_t x = 0; x < w; ++x) {
            int64_t off = (int64_t)destBase + xe::gpu::texture_address::Tiled2D((int32_t)(x0 + x), (int32_t)(y0 + y), dpAligned, bppLog2) - lo;
            const uint8_t* s = srcp + (size_t)y * rowPitch + (size_t)x * bpp;
            if (bpp == 4) {
                uint32_t v;
                std::memcpy(&v, s, 4);
                if (ok32 && swapRB) v = (v & 0xFF00FF00u) | ((v & 0xFF) << 16) | ((v >> 16) & 0xFF);
                v = SwapE(v, endian);
                std::memcpy(mem + off, &v, 4);
            } else {
                uint32_t v0, v1; std::memcpy(&v0, s, 4); std::memcpy(&v1, s + 4, 4);
                if (endian == 4) { uint32_t t = __builtin_bswap32(v1); v1 = __builtin_bswap32(v0); v0 = t; }
                else { v0 = SwapE(v0, endian); v1 = SwapE(v1, endian); }
                std::memcpy(mem + off, &v0, 4); std::memcpy(mem + off + 4, &v1, 4);
            }
        }
    if (!fastSnap.empty()) {
        static uint64_t vOk = 0, vBad = 0;
        bool same = std::memcmp(fastSnap.data(), mem, fastSnap.size()) == 0;
        ++(same ? vOk : vBad);
        if (!same || vOk <= 3 || vOk % 500 == 0) ImportLog::Get().Printf("M8 resolve-verify %ux%u bpp=%u endian=%u swap=%u -> %s (ok=%llu mal=%llu)", w, h, bpp, endian, (unsigned)swapRB,
                                                                       same ? "IGUAL" : "DISTINTO", (unsigned long long)vOk, (unsigned long long)vBad);
    }
    static uint64_t statN = 0;
    const bool targetedDump = Cfg().dumpResolveFrame && pr.frame == Cfg().dumpResolveFrame;
    bool wantStat = opt.stats || resolveDumps < opt.dumpResolves || targetedDump || statN < 12 || ((statN + 1) % 200) == 0;   // solo si se registran
    ++statN;
    if (bpp == 4 && wantStat) {   // estadisticas del contenido resuelto (RGB != 0) y volcado opcional
        uint64_t nz = 0;
        for (uint32_t y = 0; y < h; ++y) {
            const uint32_t* row = reinterpret_cast<const uint32_t*>(srcp + (size_t)y * rowPitch);
            for (uint32_t x = 0; x < w; ++x) if (row[x] & 0x00FFFFFFu) ++nz;
        }
        lastResolveNonzero = nz; lastResolvePixels = (uint64_t)w * h;
        if ((targetedDump || (!Cfg().dumpResolveFrame && resolveDumps < opt.dumpResolves)) && (pr.sfmt == 0 || pr.sfmt == 1)) {
            std::vector<uint8_t> img((size_t)w * h * 4);
            for (uint32_t y = 0; y < h; ++y) std::memcpy(&img[(size_t)y * w * 4], srcp + (size_t)y * rowPitch, (size_t)w * 4);
            ImageStats imageStats = ComputeStats(w, h, img.data(), w * 4);
            char nm[96];
            std::snprintf(nm, sizeof(nm), "\\resolve_f%05llu_%03u_%08X_%ux%u.png", (unsigned long long)pr.frame, resolveDumps, destBase, w, h);
            std::string path = rt::gpu::FramesDir() + nm;
            bool okp = WritePngRGBA(path, w, h, img.data(), w * 4);
            ImportLog::Get().Printf("D3D12 volcado de resolve frame=%llu -> %s %s nonzero=%llu/%llu hash=%016llX RGB=%u-%u,%u-%u,%u-%u",
                                    (unsigned long long)pr.frame, path.c_str(), okp ? "OK" : "FALLO", (unsigned long long)nz,
                                    (unsigned long long)((uint64_t)w * h), (unsigned long long)imageStats.hash,
                                    imageStats.minR, imageStats.maxR, imageStats.minG, imageStats.maxG, imageStats.minB, imageStats.maxB);
            ++resolveDumps;
        }
    }
}

bool g_resolveStaging = true;    // M60: RT -> bufer en VRAM -> bufer de lectura (--no-resolve-staging: RT -> lectura directa, como M8)
bool g_diagNoReadback = false;   // M60 DIAGNOSTICO (NO FIEL): --diag-no-readback, no copia los resolves a la memoria guest
unsigned g_diagSyncMask = 0;   // M60 DIAGNOSTICO: bits = causas (g_syncReason) cuyo vaciado se omite
bool g_lazyReadback = true;   // M60: --eager-readback desactiva (copia al bufer de lectura en cada resolve)
bool D3D12Backend::LazyReadback() const { return g_lazyReadback && g_resolveStaging && g_asyncResolveWriter && resolveStage; }
void D3D12Backend::CopyToResolvePool(const D3D12_TEXTURE_COPY_LOCATION& src, const D3D12_BOX* box, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& fp, uint64_t bytes, PendingResolve& pr) {
    if (g_diagNoReadback) return;
    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT; dst.PlacedFootprint = fp;
    if (!g_resolveStaging || !resolveStage) {
        tsReadbackBytes += bytes;
        dst.pResource = resolvePool.Get();
        cl->CopyTextureRegion(&dst, 0, 0, 0, &src, box);
        return;
    }
    Transition(resolveStage.Get(), resolveStageState, D3D12_RESOURCE_STATE_COPY_DEST);
    dst.pResource = resolveStage.Get();
    cl->CopyTextureRegion(&dst, 0, 0, 0, &src, box);
    TsMark(8, 0);   // M60: diagnostico (copia RT -> VRAM)
    resolveStageState = D3D12_RESOURCE_STATE_COPY_DEST;
    if (LazyReadback()) { pr.lazyCopy = true; pr.copyBytes = bytes; return; }
    Transition(resolveStage.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
    resolveStageState = D3D12_RESOURCE_STATE_COPY_SOURCE;
    tsReadbackBytes += bytes;
    cl->CopyBufferRegion(resolvePool.Get(), fp.Offset, resolveStage.Get(), fp.Offset, bytes);
}

static uint64_t ResolvedKey(uint32_t base, uint32_t w, uint32_t h, uint32_t fmt);
bool g_depthResolveCs = true;   // M60: --depth-resolve-copy desactiva (copia del plano completo y conversion en la CPU)
static const char* kDepthCsHlsl = R"(
// M60: resolve de profundidad D32_FLOAT -> 20e4<<8 (misma aritmetica entera que xenos Float32To20e4 de Xenia con
// round_to_nearest_even=true, aplicada a depth*2; el producto por 2 se hace en enteros para no depender de FTZ).
cbuffer C : register(b0) { uint x0; uint y0; uint w; uint h; uint pitchWords; uint dstWord; uint writeTex; };
Texture2D<float> depthTex : register(t0);
RWByteAddressBuffer outBuf : register(u0);
RWTexture2D<float> outTex : register(u1);   // atajo: valor que daria la textura k_24_8_FLOAT destilada (Float20e4To32)
uint From20e4(uint f24) {   // xenos Float20e4To32 (Xenia), en bits
    f24 &= 0xFFFFFFu;
    if (f24 == 0u) return 0u;
    uint mantissa = f24 & 0xFFFFFu;
    uint exponent = f24 >> 20;
    if (exponent == 0u) {
        uint lz = (31u - firstbithigh(mantissa)) - 11u;
        exponent = 1u - lz;   // aritmetica modular; (exponent + 112) es >= 93
        mantissa = (mantissa << lz) & 0xFFFFFu;
    }
    return ((exponent + 112u) << 23) | (mantissa << 3);
}
uint To20e4(uint u) {
    bool pos = (u & 0x80000000u) == 0u && u != 0u && !((u & 0x7F800000u) == 0x7F800000u && (u & 0x7FFFFFu) != 0u);
    if (!pos) return 0u;
    if (u >= 0x3FFFFFF8u) return 0xFFFFFFu;
    if (u < 0x38800000u) {
        uint shift = min(113u - (u >> 23), 24u);
        u = (0x800000u | (u & 0x7FFFFFu)) >> shift;
    } else {
        u += 0xC8000000u;
    }
    u += 3u + ((u >> 3) & 1u);
    return (u >> 3) & 0xFFFFFFu;
}
[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    if (id.x >= w || id.y >= h) return;
    uint u = asuint(depthTex.Load(int3(x0 + id.x, y0 + id.y, 0)));
    uint e = u & 0x7F800000u;
    if (e == 0u) u = (u & 0x80000000u) | ((u & 0x7FFFFFu) << 1);
    else if (e != 0x7F800000u) u += 0x00800000u;
    uint v = To20e4(u);
    outBuf.Store((dstWord + id.y * pitchWords + id.x) * 4u, v << 8);
    if (writeTex != 0u) outTex[id.xy] = asfloat(From20e4(v));
}
)";

bool D3D12Backend::EnsureDepthCs() {
    if (csDepthPso) return true;
    if (csDepthTried) return false;
    csDepthTried = true;
    D3D12_DESCRIPTOR_RANGE r = {};
    r.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; r.NumDescriptors = 1; r.BaseShaderRegister = 0; r.RegisterSpace = 0; r.OffsetInDescriptorsFromTableStart = 0;
    D3D12_DESCRIPTOR_RANGE ru = {};
    ru.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; ru.NumDescriptors = 1; ru.BaseShaderRegister = 1; ru.RegisterSpace = 0; ru.OffsetInDescriptorsFromTableStart = 0;
    D3D12_ROOT_PARAMETER p[4] = {};
    p[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS; p[0].Constants = {0, 0, 7}; p[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    p[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; p[3].DescriptorTable = {1, &ru}; p[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    p[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; p[1].DescriptorTable = {1, &r}; p[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    p[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV; p[2].Descriptor = {0, 0}; p[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    D3D12_ROOT_SIGNATURE_DESC rs = {};
    rs.NumParameters = 4; rs.pParameters = p;
    ComPtr<ID3DBlob> blob, err;
    if (FAILED(D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err))) { ImportLog::Get().Printf("M60 CS profundidad: firma raiz fallida"); return false; }
    if (!Hr(dev->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&csRootSig)), "CreateRootSignature(CS profundidad)")) return false;
    std::string dxil, e;
    if (!CompileHlslProfile(kDepthCsHlsl, L"cs_6_0", dxil, e)) { ImportLog::Get().Printf("M60 CS profundidad: compilacion fallida: %s", e.c_str()); return false; }
    D3D12_COMPUTE_PIPELINE_STATE_DESC d = {};
    d.pRootSignature = csRootSig.Get(); d.CS = {dxil.data(), dxil.size()};
    if (!Hr(dev->CreateComputePipelineState(&d, IID_PPV_ARGS(&csDepthPso)), "CreateComputePipelineState(profundidad)")) { csDepthPso.Reset(); return false; }
    ImportLog::Get().Printf("M60 CS de resolve de profundidad listo (DXIL %zu bytes)", dxil.size());
    return true;
}

// M60: region del plano de profundidad -> bufer intermedio (VRAM) en pr.poolOffset con filas de pr.rowPitch bytes (20e4<<8), y copia de
// esa region al bufer de lectura. Requiere resolveStage. Devuelve false si no se pudo (el llamador usa la copia del plano completo).
bool D3D12Backend::DepthRegionToStage(HostRT* src, PendingResolve& pr) {
    if (!resolveStage || !EnsureDepthCs()) return false;
    // M60: descriptores antes de las transiciones (AllocGpuSrv puede enviar la lista: Kick emite copias que cambian el estado de resolveStage)
    uint32_t slot = AllocGpuSrv(2);
    uint32_t uslot = slot + 1;
    const D3D12_RESOURCE_STATES srvState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_DEPTH_READ;
    Transition(src->res.Get(), src->state, srvState); src->state = srvState;
    Transition(resolveStage.Get(), resolveStageState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS); resolveStageState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    D3D12_CPU_DESCRIPTOR_HANDLE ch = gpuSrvHeap->GetCPUDescriptorHandleForHeapStart(); ch.ptr += (SIZE_T)slot * srvInc;
    D3D12_GPU_DESCRIPTOR_HANDLE gh = gpuSrvHeap->GetGPUDescriptorHandleForHeapStart(); gh.ptr += (UINT64)slot * srvInc;
    D3D12_SHADER_RESOURCE_VIEW_DESC sv = {};
    sv.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS; sv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; sv.Texture2D.MipLevels = 1;
    sv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    dev->CreateShaderResourceView(src->res.Get(), &sv, ch);
    // M60: atajo GPU para la lectura como textura k_24_8_FLOAT del mismo fotograma (como RecordResolvedTex de color)
    ResolvedTex* rtex = nullptr;
    // El resolve de profundidad se copia como k_8_8_8_8 (6) o k_24_8_FLOAT (23); la textura que lo lee como k_24_8_FLOAT
    // obtiene, con el mismo endian, la palabra 20e4<<8: el atajo se registra con formato 23 (LookupResolvedTex exige igualdad).
    if (!g_resolveSync && !g_noResolveShortcut && pr.x0 == 0 && pr.y0 == 0 && (pr.dfmt == 6 || pr.dfmt == 23)) {
        uint64_t key = ResolvedKey(pr.destBase, pr.w, pr.h, 23);
        ResolvedTex& rt = resolvedTex[key];
        if (rt.ht.res && rt.format != DXGI_FORMAT_R32_FLOAT) rt.ht.res.Reset();
        if (!rt.ht.res) {
            D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
            D3D12_RESOURCE_DESC rd = {};
            rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; rd.Width = pr.w; rd.Height = pr.h; rd.DepthOrArraySize = 1; rd.MipLevels = 1;
            rd.Format = DXGI_FORMAT_R32_FLOAT; rd.SampleDesc.Count = 1; rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            bool fresh = rt.ht.cpuSrv == 0 && rt.srvMap == 0xFFFFFFFF && !rt.valid;
            if (!Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&rt.ht.res)), "textura de resolve de profundidad (atajo)")) {
                resolvedTex.erase(key);
            } else {
                if (fresh || rt.ht.cpuSrv == 0) {
                    if (cpuSrvUsed >= 8192) { Note("OMISION atajo de resolve: sin SRV libres"); rt.ht.res.Reset(); }
                    else rt.ht.cpuSrv = cpuSrvUsed++;
                }
                rt.ht.w = pr.w; rt.ht.h = pr.h; rt.ht.dim = 0;
                rt.state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS; rt.srvMap = 0xFFFFFFFF; rt.format = DXGI_FORMAT_R32_FLOAT;
            }
        }
        auto it = resolvedTex.find(key);
        if (it != resolvedTex.end() && it->second.ht.res) rtex = &it->second;
    }
    D3D12_CPU_DESCRIPTOR_HANDLE uch = gpuSrvHeap->GetCPUDescriptorHandleForHeapStart(); uch.ptr += (SIZE_T)uslot * srvInc;
    D3D12_GPU_DESCRIPTOR_HANDLE ugh = gpuSrvHeap->GetGPUDescriptorHandleForHeapStart(); ugh.ptr += (UINT64)uslot * srvInc;
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC ud = {};
        ud.Format = DXGI_FORMAT_R32_FLOAT; ud.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        if (rtex) {
            Transition(rtex->ht.res.Get(), rtex->state, D3D12_RESOURCE_STATE_UNORDERED_ACCESS); rtex->state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            dev->CreateUnorderedAccessView(rtex->ht.res.Get(), nullptr, &ud, uch);
        } else dev->CreateUnorderedAccessView(nullptr, nullptr, &ud, uch);   // descriptor nulo (no se escribe: writeTex = 0)
    }
    uint32_t c[7] = {(uint32_t)pr.x0, (uint32_t)pr.y0, pr.w, pr.h, pr.rowPitch / 4, (uint32_t)(pr.poolOffset / 4), rtex ? 1u : 0u};
    cl->SetComputeRootSignature(csRootSig.Get());
    cl->SetPipelineState(csDepthPso.Get());
    cl->SetComputeRoot32BitConstants(0, 7, c, 0);
    cl->SetComputeRootDescriptorTable(3, ugh);
    cl->SetComputeRootDescriptorTable(1, gh);
    cl->SetComputeRootUnorderedAccessView(2, resolveStage->GetGPUVirtualAddress());
    cl->Dispatch((pr.w + 7) / 8, (pr.h + 7) / 8, 1);
    TsMark(8, 0);
    if (rtex) {
        const D3D12_RESOURCE_STATES srv = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        Transition(rtex->ht.res.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srv); rtex->state = srv;
        rtex->valid = true; rtex->derived = false;
        rtex->serial = ++resolveCopySerial;
        rtex->destBase = pr.destBase; rtex->w = pr.w; rtex->h = pr.h; rtex->dfmt = 23; rtex->endian = pr.endian; rtex->dpAligned = pr.dpAligned;
        rtex->swapRB = false; rtex->lo = (uint32_t)pr.lo; rtex->hi = (uint32_t)pr.hi;
    }
    uint64_t bytes = (uint64_t)pr.rowPitch * pr.h;
    if (LazyReadback()) { pr.lazyCopy = true; pr.copyBytes = bytes; return true; }   // estado UAV: FlushResolves hace la transicion
    Transition(resolveStage.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE); resolveStageState = D3D12_RESOURCE_STATE_COPY_SOURCE;
    if (!g_diagNoReadback) { cl->CopyBufferRegion(resolvePool.Get(), pr.poolOffset, resolveStage.Get(), pr.poolOffset, bytes); tsReadbackBytes += bytes; }
    return true;
}

bool D3D12Backend::EnsureResolvePool(uint64_t need) {
    if (!resolvePool) {
        resolvePoolBytes = std::max<uint64_t>(256ull << 20, (need + 511) & ~511ull);
        D3D12_HEAP_PROPERTIES rp = {D3D12_HEAP_TYPE_READBACK};
        D3D12_RESOURCE_DESC bd = {};
        bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; bd.Width = resolvePoolBytes; bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
        bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        if (!Hr(dev->CreateCommittedResource(&rp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resolvePool)), "readback de resolves diferidos")) return false;
        void* pm = nullptr;
        D3D12_RANGE rr = {0, (SIZE_T)resolvePoolBytes};   // mapeo persistente (permitido en montones de lectura; se lee tras la valla)
        if (!Hr(resolvePool->Map(0, &rr, &pm), "Map(readback diferido)")) return false;
        resolvePoolCpu = (const uint8_t*)pm;
        if (g_resolveStaging) {   // M60
            D3D12_HEAP_PROPERTIES dp = {D3D12_HEAP_TYPE_DEFAULT};
            D3D12_RESOURCE_DESC sd = bd; sd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;   // M60: destino UAV del resolve de profundidad
            if (!Hr(dev->CreateCommittedResource(&dp, D3D12_HEAP_FLAG_NONE, &sd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resolveStage)), "bufer intermedio de resolves")) resolveStage.Reset();
            resolveStageState = D3D12_RESOURCE_STATE_COPY_DEST;
        }
        ImportLog::Get().Printf("M8 resolve diferido: bufer de lectura de %llu MB", (unsigned long long)(resolvePoolBytes >> 20));
    }
    if (!g_asyncResolveWriter) {
        if (resolvePoolOffset + need > resolvePoolBytes) {
            rt::gpu::g_syncReason = 12;
            FlushResolves();
            if (need > resolvePoolBytes) { Note("OMISION resolve: region mayor que el bufer de lectura diferido"); return false; }
        }
        return true;
    }
    // M60: dos mitades; al llenarse la actual se pasa a la otra cuando ya no la usa ninguna entrada pendiente (normalmente ya libre).
    const uint64_t half = resolvePoolBytes / 2;
    if (need > half) { Note("OMISION resolve: region mayor que medio bufer de lectura diferido"); return false; }
    const uint64_t cur = resolvePoolOffset < half ? 0 : half;
    if (resolvePoolOffset + need <= cur + half) return true;
    const uint64_t other = cur ? 0 : half;
    rt::gpu::g_syncReason = 12;
    rt::gpu::M60Event('H');
    IssuePending();
    LARGE_INTEGER q0; QueryPerformanceCounter(&q0);
    {
        std::unique_lock<std::mutex> g(wrMu);
        wrDone.wait(g, [&] {
            for (auto& e : wrQueue) if (!e.isWrite && e.poolOffset >= other && e.poolOffset < other + half) return false;
            return true;
        });
    }
    if (g_gpuTiming) {
        LARGE_INTEGER q1, f; QueryPerformanceCounter(&q1); QueryPerformanceFrequency(&f);
        ++tsSyncN[12]; tsSyncMs[12] += (double)(q1.QuadPart - q0.QuadPart) * 1000.0 / (double)f.QuadPart;
    }
    resolvePoolOffset = other;
    rt::gpu::M60Event('h');
    return true;
}

// M60: escritura ordenada del CP. Si hay entradas en la cola del escritor, la escritura se encola detras (el escritor la aplica cuando
// los resolves anteriores ya estan en memoria). Las copias pendientes ya se emitieron en SyncPoint (IssuePending).
bool g_orderedWrites = true;   // M60: --no-ordered-writes desactiva (vaciado completo antes de cada escritura del CP, como M8)
bool D3D12Backend::WriteOrdered(uint32_t phys, const void* data, uint32_t bytes) {
    if (!g_orderedWrites || g_strictResolveSync || !g_asyncResolveWriter) return false;
    {
        std::lock_guard<std::mutex> g(wrMu);
        if (wrQueue.empty()) return false;
        const uint8_t* s = (const uint8_t*)data;
        for (uint32_t o = 0; o < bytes; o += 16) {
            PendingResolve w;
            w.isWrite = true; w.wbytes = std::min(16u, bytes - o);
            std::memcpy(w.wdata, s + o, w.wbytes);
            w.lo = (int64_t)(phys + o); w.hi = w.lo + w.wbytes;
            w.fence = 0; w.poolOffset = ~0ull;
            wrQueue.push_back(w);
        }
        ++orderedWrites;
    }
    wrCv.notify_one();
    return true;
}

// M60: marca de XE_SWAP: se emiten las copias pendientes y se encola la marca; el escritor la publica en g_swapMarkDone al llegar.
bool g_swapMarks = false;   // M60: --swap-marks (diagnostico): el VBLANK espera a que el escritor complete lo anterior al ultimo XE_SWAP
void D3D12Backend::SwapMark() {
    if (!g_asyncResolveWriter) return;
    IssuePending();   // el escritor avanza mientras el CP espera al siguiente fotograma
    if (!g_swapMarks) return;
    uint64_t seq = rt::gpu::g_swapMarkQueued.load() + 1;
    {
        std::lock_guard<std::mutex> g(wrMu);
        if (!wrQueue.empty()) {
            PendingResolve m;
            m.isWrite = true; m.wbytes = 0; m.markSeq = seq; m.fence = 0; m.poolOffset = ~0ull; m.lo = m.hi = 0;
            wrQueue.push_back(m);
        }
        rt::gpu::g_swapMarkQueued.store(seq);
        if (wrQueue.empty()) rt::gpu::g_swapMarkDone.store(seq);
    }
    wrCv.notify_one();
}

// M60: emite las copias VRAM -> bufer de lectura de los resolves vivos aun no emitidos y envia la lista (sin esperar).
bool g_eagerIssue = true;   // M60: --no-eager-issue (las copias perezosas se emiten solo en los vaciados)
// Emite en la lista abierta las copias VRAM -> lectura de las entradas aun no emitidas (vivas); keepLast: deja sin emitir la ultima
// (puede quedar cubierta por el siguiente resolve). Requiere la lista abierta.
void D3D12Backend::IssueCopies(bool keepLast) {
    std::lock_guard<std::mutex> g(wrMu);
    bool any = false;
    size_t n = wrQueue.size();
    if (keepLast && n && !wrQueue.back().isWrite) --n;
    for (size_t i = 0; i < n; ++i) {
        PendingResolve& pr = wrQueue[i];
        if (pr.fence != kNotReady) continue;
        if (pr.dead) { pr.fence = 0; continue; }
        if (keepLast && pr.predDead) continue;   // M60: prediccion de cubierto: se espera a IssuePending
        if (!any) { Transition(resolveStage.Get(), resolveStageState, D3D12_RESOURCE_STATE_COPY_SOURCE); resolveStageState = D3D12_RESOURCE_STATE_COPY_SOURCE; any = true; }
        cl->CopyBufferRegion(resolvePool.Get(), pr.poolOffset, resolveStage.Get(), pr.poolOffset, pr.copyBytes);
        tsReadbackBytes += pr.copyBytes;
        pr.fence = fenceValue + 1;
        if (pr.fence > wrMaxFence) wrMaxFence = pr.fence;
    }
    if (any) TsMark(2, 1);
}
void D3D12Backend::IssuePending() {
    if (!g_asyncResolveWriter) return;
    bool pending = false;
    { std::lock_guard<std::mutex> g(wrMu); for (auto& pr : wrQueue) if (pr.fence == kNotReady) { pending = true; break; } }
    if (pending) {
        if (!clOpen) BeginList();   // lista nueva normal (seccion del anillo de subida con su espera)
        IssueCopies(false);
    }
    wrCv.notify_all();
    if (clOpen && wrMaxFence > fenceValue) Kick();   // las copias pueden estar en la lista abierta: se envian (sin esperar)
}

// M8: completa los resolves pendientes (una sola espera de la GPU para todos) y los escribe en la memoria guest en orden.
// Se omite un resolve si otro posterior al mismo destino (mismo pitch y bpp) cubre toda su region: la memoria final es identica
// y la CPU guest no puede observar el estado intermedio (no hay punto de sincronizacion entre ambos).
void D3D12Backend::FlushResolves() {
    if (!g_asyncResolveWriter) {   // version sincronica (cambio 5): una espera y escritura en el hilo del CP
        if (pendingResolves.empty()) return;
        PERF_ZONE(kZoneResolveFlush);
        FlushAndWait();
        const uint8_t* p = resolvePoolCpu;
        size_t n = pendingResolves.size();
        for (size_t i = 0; i < n; ++i) {
            const PendingResolve& a = pendingResolves[i];
            bool dead = false;
            if (!g_resolveNoDse)
                for (size_t j = i + 1; j < n && !dead; ++j) {
                    const PendingResolve& b = pendingResolves[j];
                    if (b.destBase == a.destBase && b.dpAligned == a.dpAligned && b.bppLog2 == a.bppLog2 && b.x0 <= a.x0 && b.y0 <= a.y0 && b.x1 >= a.x1 && b.y1 >= a.y1) dead = true;
                }
            if (dead) { ++resolveDeadSkipped; continue; }
            WriteResolve(a, a.depth ? DepthResolveSource(a, p + a.poolOffset, depthWrScratch, true) : p + a.poolOffset);
            ++resolveWritten;
        }
        pendingResolves.clear();
        resolvePoolOffset = 0;
        return;
    }
    {
        std::lock_guard<std::mutex> g(wrMu);
        if (wrQueue.empty()) { resolvePoolOffset = 0; return; }
    }
    PERF_ZONE(kZoneResolveFlush);
    const int reason = rt::gpu::g_syncReason & 15;   // M60: solo estadistica
    if (g_diagSyncMask & (1u << reason)) return;   // M60 DIAGNOSTICO (NO FIEL): --diag-skip-sync MASCARA
    LARGE_INTEGER fq0; QueryPerformanceCounter(&fq0);
    rt::gpu::M60Event('F', (uint32_t)reason);
    IssuePending();   // M60: lectura perezosa (copias solo de los resolves vivos) y envio
    if (g_gpuTiming && fence->GetCompletedValue() < wrMaxFence) {   // M60 diagnostico: tiempo hasta que la GPU termina (sin cambiar el orden)
        static HANDLE ev = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        fence->SetEventOnCompletion(wrMaxFence, ev); WaitForSingleObject(ev, 20000);
        LARGE_INTEGER fq1, ff; QueryPerformanceCounter(&fq1); QueryPerformanceFrequency(&ff);
        tsSyncGpuMs += (double)(fq1.QuadPart - fq0.QuadPart) * 1000.0 / (double)ff.QuadPart;
    }
    std::unique_lock<std::mutex> g(wrMu);
    wrDone.wait(g, [&] { return wrQueue.empty(); });
    resolvePoolOffset = 0;
    rt::gpu::M60Event('f');
    if (g_gpuTiming) {
        LARGE_INTEGER fq1, ff; QueryPerformanceCounter(&fq1); QueryPerformanceFrequency(&ff);
        ++tsSyncN[reason]; tsSyncMs[reason] += (double)(fq1.QuadPart - fq0.QuadPart) * 1000.0 / (double)ff.QuadPart;
    }
}

// M60: si en el fotograma anterior hubo n resolves al mismo destino y rectangulo, los n-1 primeros de este fotograma se predicen
// cubiertos; su copia al bufer de lectura no se emite por adelantado (IssueCopies con keepLast), solo en IssuePending si siguen vivos.
void D3D12Backend::PredictDead(PendingResolve& pr) {
    uint64_t k = ((uint64_t)pr.destBase << 32) ^ ((uint64_t)pr.dpAligned << 20) ^ ((uint64_t)pr.bppLog2 << 60) ^
                 ((uint64_t)(uint32_t)pr.x0 << 2) ^ ((uint64_t)(uint32_t)pr.y0 << 14) ^ ((uint64_t)(uint32_t)pr.x1 << 26) ^ ((uint64_t)(uint32_t)pr.y1 << 38);
    uint32_t n = ++predCur[k];
    auto it = predPrev.find(k);
    pr.predDead = it != predPrev.end() && n < it->second;
}

void D3D12Backend::EnqueueResolve(PendingResolve pr) {
    if (!g_asyncResolveWriter) { pendingResolves.push_back(pr); return; }
    if (pr.lazyCopy) PredictDead(pr);
    pr.fence = pr.lazyCopy ? kNotReady : fenceValue + 1;   // la copia esta en la lista abierta: la senaliza el siguiente Submit (M60: o se emite en FlushResolves)
    {
        std::lock_guard<std::mutex> g(wrMu);
        size_t dseFrom = wrBusy ? 1 : 0;   // M60: la eliminacion no cruza escrituras ordenadas (la CPU puede observar el estado intermedio)
        for (size_t i = wrQueue.size(); i-- > dseFrom;) if (wrQueue[i].isWrite) { dseFrom = i + 1; break; }
        if (!g_resolveNoDse)   // eliminacion de escrituras muertas: solo entradas aun no iniciadas (la primera puede estar en curso)
            for (size_t i = dseFrom; i < wrQueue.size(); ++i) {
                PendingResolve& o = wrQueue[i];
                if (!o.dead && pr.destBase == o.destBase && pr.dpAligned == o.dpAligned && pr.bppLog2 == o.bppLog2 && pr.x0 <= o.x0 && pr.y0 <= o.y0 && pr.x1 >= o.x1 && pr.y1 >= o.y1) o.dead = true;
            }
        wrQueue.push_back(pr);
        if (pr.fence != kNotReady && pr.fence > wrMaxFence) wrMaxFence = pr.fence;
    }
    wrCv.notify_one();
    if (!wrThread.joinable()) {
        wrStop = false;
        wrThread = std::thread([this] {
            rt::perf::NameCurrentThread("gpu-resolve-writer");
            HANDLE ev = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            for (;;) {
                PendingResolve pr;
                {
                    std::unique_lock<std::mutex> g(wrMu);
                    wrCv.wait(g, [&] { return wrStop || (!wrQueue.empty() && wrQueue.front().fence != kNotReady); });   // M60: solo entradas con copia emitida
                    if (wrStop && wrQueue.empty()) break;
                    if (wrQueue.front().fence == kNotReady) { wrQueue.pop_front(); wrDone.notify_all(); continue; }   // M60: cierre sin copia emitida
                    wrBusy = true;
                    pr = wrQueue.front();
                }
                if (fence->GetCompletedValue() < pr.fence) {
                    fence->SetEventOnCompletion(pr.fence, ev);
                    WaitForSingleObject(ev, 20000);
                }
                bool dead;
                { std::lock_guard<std::mutex> g(wrMu); dead = wrQueue.front().dead; }
                if (pr.isWrite) {   // M60
                    if (pr.wbytes) { if (uint8_t* p = PhysHost((uint32_t)pr.lo, pr.wbytes)) std::memcpy(p, pr.wdata, pr.wbytes); }
                    else if (pr.markSeq) rt::gpu::g_swapMarkDone.store(pr.markSeq);
                } else if (dead) ++resolveDeadSkipped;
                else {
                    LARGE_INTEGER w0; QueryPerformanceCounter(&w0);
                    const uint8_t* s = resolvePoolCpu + pr.poolOffset;
                    if (pr.depth) s = DepthResolveSource(pr, s, depthWrScratch, true);   // M60
                    WriteResolve(pr, s);
                    ++resolveWritten;
                    LARGE_INTEGER w1; QueryPerformanceCounter(&w1);
                    wrBusyTicks += (uint64_t)(w1.QuadPart - w0.QuadPart); ++wrCount; wrBytes += (uint64_t)pr.w * pr.h * pr.bpp;   // M60 diagnostico
                }
                {
                    std::lock_guard<std::mutex> g(wrMu);
                    wrQueue.pop_front();
                    wrBusy = false;
                }
                wrDone.notify_all();
            }
            CloseHandle(ev);
        });
    }
}

bool D3D12Backend::FlushResolvesIfOverlap(uint32_t lo, uint32_t hi) {
    bool hit = false;
    if (!g_asyncResolveWriter) {
        for (auto& pr : pendingResolves) if ((int64_t)lo < pr.hi && (int64_t)hi > pr.lo) { hit = true; break; }
    } else {
        std::lock_guard<std::mutex> g(wrMu);
        for (auto& pr : wrQueue) if ((int64_t)lo < pr.hi && (int64_t)hi > pr.lo) { hit = true; break; }
    }
    if (hit) { ++resolveFlushByRead; FlushResolves(); }
    return hit;
}

// ---- M8: atajo GPU para lecturas de textura de un resolve del mismo fotograma ----
void D3D12Backend::InvalidateResolvedTex(uint32_t lo, uint32_t hi) {
    for (auto& kv : resolvedTex) if (!kv.second.derived && kv.second.valid && kv.second.lo < hi && kv.second.hi > lo) kv.second.valid = false;
}

static uint64_t ResolvedKey(uint32_t base, uint32_t w, uint32_t h, uint32_t fmt) {
    return ((uint64_t)base << 32) ^ ((uint64_t)w << 16) ^ (uint64_t)h ^ ((uint64_t)fmt << 58);
}

void D3D12Backend::RecordResolvedTex(const PendingResolve& pr, const D3D12_TEXTURE_COPY_LOCATION& srcLoc, const D3D12_BOX& box) {
    uint64_t key = ResolvedKey(pr.destBase, pr.w, pr.h, pr.dfmt);
    ResolvedTex& rt = resolvedTex[key];
    if (!rt.ht.res) {
        D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; rd.Width = pr.w; rd.Height = pr.h; rd.DepthOrArraySize = 1; rd.MipLevels = 1;
        rd.Format = pr.format; rd.SampleDesc.Count = 1;
        if (!Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&rt.ht.res)), "textura de resolve (atajo)")) {
            resolvedTex.erase(key); return;
        }
        if (cpuSrvUsed >= 8192) { Note("OMISION atajo de resolve: sin SRV libres"); resolvedTex.erase(key); return; }
        rt.ht.cpuSrv = cpuSrvUsed++;
        rt.ht.w = pr.w; rt.ht.h = pr.h; rt.ht.dim = 0;
        rt.state = D3D12_RESOURCE_STATE_COPY_DEST;
        rt.srvMap = 0xFFFFFFFF;
    }
    Transition(rt.ht.res.Get(), rt.state, D3D12_RESOURCE_STATE_COPY_DEST);
    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource = rt.ht.res.Get(); dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; dst.SubresourceIndex = 0;
    cl->CopyTextureRegion(&dst, 0, 0, 0, &srcLoc, &box);
    Transition(rt.ht.res.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    rt.state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    rt.valid = true;
    rt.serial = ++resolveCopySerial;
    rt.destBase = pr.destBase; rt.w = pr.w; rt.h = pr.h; rt.dfmt = pr.dfmt; rt.endian = pr.endian; rt.dpAligned = pr.dpAligned;
    rt.swapRB = pr.swapRB && pr.ok32; rt.lo = (uint32_t)pr.lo; rt.hi = (uint32_t)pr.hi; rt.format = pr.format;
}

HostTexture* D3D12Backend::LookupResolvedTex(uint32_t base, uint32_t width, uint32_t height, uint32_t fmt, uint32_t endian, uint32_t pitchAligned, uint32_t swizzle) {
    if (g_resolveSync || g_noResolveShortcut) return nullptr;
    // origen: copia valida de un resolve a 'base' con el mismo formato, endian y pitch que cubra (0,0)-(width,height)
    ResolvedTex* srcRt = nullptr;
    for (auto& kv : resolvedTex) {
        ResolvedTex& c = kv.second;
        if (g_gpuTiming && c.destBase == base && !c.derived) {   // M60 diagnostico
            static int nlog = 0;
            if (nlog < 20 && fmt == 23 && (c.dfmt != fmt || c.endian != endian || c.dpAligned != pitchAligned || !c.valid || c.w < width || c.h < height)) {
                ++nlog;
                ImportLog::Get().Printf("M60 atajo no coincide base=%08X tex(fmt=%u endian=%u pitch=%u %ux%u) resolve(fmt=%u endian=%u pitch=%u %ux%u valido=%d)", base, fmt, endian, pitchAligned, width, height,
                                        c.dfmt, c.endian, c.dpAligned, c.w, c.h, (int)c.valid);
            }
        }
        if (c.derived || !c.valid || c.destBase != base || c.dfmt != fmt || c.endian != endian || c.dpAligned != pitchAligned) continue;
        if (c.w < width || c.h < height) continue;
        if (!srcRt || c.serial > srcRt->serial) srcRt = &c;   // la mas reciente
    }
    if (!srcRt) return nullptr;
    ResolvedTex* rtp = srcRt;
    if (srcRt->w != width || srcRt->h != height) {   // subregion (0,0)-(width,height): copia GPU a una textura de ese tamano
        uint64_t key = ResolvedKey(base, width, height, fmt) ^ 0x5A5A5A5A00000000ull;
        ResolvedTex& d = resolvedTex[key];
        if (!d.ht.res || d.format != srcRt->format) {
            D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
            D3D12_RESOURCE_DESC rd = {};
            rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; rd.Width = width; rd.Height = height; rd.DepthOrArraySize = 1; rd.MipLevels = 1;
            rd.Format = srcRt->format; rd.SampleDesc.Count = 1;
            if (!Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&d.ht.res)), "textura derivada de resolve")) {
                resolvedTex.erase(key); return nullptr;
            }
            if (cpuSrvUsed >= 8192) { Note("OMISION atajo de resolve: sin SRV libres"); resolvedTex.erase(key); return nullptr; }
            d.ht.cpuSrv = cpuSrvUsed++;
            d.ht.w = width; d.ht.h = height; d.ht.dim = 0;
            d.state = D3D12_RESOURCE_STATE_COPY_DEST; d.srvMap = 0xFFFFFFFF; d.derived = true; d.format = srcRt->format;
            d.srcSerial = 0;
        }
        if (d.srcSerial != srcRt->serial) {
            const D3D12_RESOURCE_STATES srv = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            Transition(srcRt->ht.res.Get(), srcRt->state, D3D12_RESOURCE_STATE_COPY_SOURCE);
            Transition(d.ht.res.Get(), d.state, D3D12_RESOURCE_STATE_COPY_DEST);
            D3D12_TEXTURE_COPY_LOCATION dl = {}, sl = {};
            dl.pResource = d.ht.res.Get(); dl.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            sl.pResource = srcRt->ht.res.Get(); sl.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            D3D12_BOX box = {0, 0, 0, width, height, 1};
            cl->CopyTextureRegion(&dl, 0, 0, 0, &sl, &box);
            Transition(srcRt->ht.res.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, srv);
            Transition(d.ht.res.Get(), D3D12_RESOURCE_STATE_COPY_DEST, srv);
            srcRt->state = srv; d.state = srv;
            d.srcSerial = srcRt->serial;
            ++resolveSubCopies;
        }
        d.swapRB = srcRt->swapRB; d.destBase = base; d.w = width; d.h = height; d.dfmt = fmt; d.endian = endian; d.dpAligned = pitchAligned;
        rtp = &d;
    }
    ResolvedTex& rt = *rtp;
    // mapeo de componentes: el byte k de la textura destilada es el canal perm[k] del RT (intercambio R/B del resolve)
    static const uint32_t permId[4] = {0, 1, 2, 3}, permSwap[4] = {2, 1, 0, 3};
    const uint32_t* perm = rt.swapRB ? permSwap : permId;
    uint32_t sw[4] = {swizzle & 7u, (swizzle >> 3) & 7u, (swizzle >> 6) & 7u, (swizzle >> 9) & 7u};
    uint32_t map = 0;
    for (int i = 0; i < 4; ++i) {
        uint32_t c = sw[i] <= 3 ? perm[sw[i]] : (sw[i] == 5 ? 5u : (sw[i] == 4 ? 4u : 0u));
        map |= c << (3 * i);
    }
    map |= (1u << 12);
    if (map != rt.srvMap) {
        D3D12_SHADER_RESOURCE_VIEW_DESC sv = {};
        sv.Format = rt.format; sv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; sv.Texture2D.MipLevels = 1;
        sv.Shader4ComponentMapping = map;
        D3D12_CPU_DESCRIPTOR_HANDLE h = cpuSrvHeap->GetCPUDescriptorHandleForHeapStart();
        h.ptr += (SIZE_T)rt.ht.cpuSrv * srvInc;
        dev->CreateShaderResourceView(rt.ht.res.Get(), &sv, h);
        rt.srvMap = map;
    }
    ++resolveShortcutHits;
    return &rt.ht;
}

} // namespace rt::gpu::d3d
