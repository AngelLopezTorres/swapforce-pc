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
    if (approx) Note("SIMPLIFY formato de render target de color " + std::to_string(fmt) + " aproximado por un formato de PC (" + std::to_string((uint32_t)df) + ")");
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
    uint32_t needH = ((uint32_t)y1 + 31u) & ~31u;
    if (!isDepth) {
        auto ci = R<reg::RB_COLOR_INFO>(); ci.value = RV(srcSel == 0 ? 0x2001 : 0x2003 + (srcSel - 1));
        src = GetColorRT(ci.color_base | (ci.color_base_bit_11 << 11), surf.surface_pitch, (uint32_t)ci.color_format, needH);
        if (!src) { Note("OMISION resolve: sin render target de color de origen"); return false; }
    }
    bool wrote = false;
    if (src) {
        uint32_t dfmt = (uint32_t)dinfo.copy_dest_format;
        uint32_t sfmt = src->xenosFormat;
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
                    D3D12_TEXTURE_COPY_LOCATION dst = {};
                    dst.pResource = resolvePool.Get(); dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                    dst.PlacedFootprint.Offset = pr.poolOffset;
                    dst.PlacedFootprint.Footprint = {src->format, w, h, 1, pr.rowPitch};
                    cl->CopyTextureRegion(&dst, 0, 0, 0, &sl, &box);
                    resolvePoolOffset = (resolvePoolOffset + need + 511) & ~511ull;
                    EnqueueResolve(pr);
                    if (g_kickDraws && drawsSinceKick >= g_kickResolveDraws) Kick();   // M8: la copia y el trabajo previo empiezan en la GPU
                }
                wrote = true;
            }
        }
    } else {
        Note("SIMPLIFY resolve de profundidad no implementado: destino sin escribir");
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
    bool wantStat = opt.stats || resolveDumps < opt.dumpResolves || statN < 12 || ((statN + 1) % 200) == 0;   // solo si se registran
    ++statN;
    if (bpp == 4 && wantStat) {   // estadisticas del contenido resuelto (RGB != 0) y volcado opcional
        uint64_t nz = 0;
        for (uint32_t y = 0; y < h; ++y) {
            const uint32_t* row = reinterpret_cast<const uint32_t*>(srcp + (size_t)y * rowPitch);
            for (uint32_t x = 0; x < w; ++x) if (row[x] & 0x00FFFFFFu) ++nz;
        }
        lastResolveNonzero = nz; lastResolvePixels = (uint64_t)w * h;
        if (resolveDumps < opt.dumpResolves && (pr.sfmt == 0 || pr.sfmt == 1)) {
            std::vector<uint8_t> img((size_t)w * h * 4);
            for (uint32_t y = 0; y < h; ++y) std::memcpy(&img[(size_t)y * w * 4], srcp + (size_t)y * rowPitch, (size_t)w * 4);
            char nm[96];
            std::snprintf(nm, sizeof(nm), "\\resolve_%03u_%08X_%ux%u.png", resolveDumps, destBase, w, h);
            std::string path = rt::gpu::FramesDir() + nm;
            bool okp = WritePngRGBA(path, w, h, img.data(), w * 4);
            ImportLog::Get().Printf("D3D12 volcado de resolve -> %s %s nonzero=%llu/%llu", path.c_str(), okp ? "OK" : "FALLO", (unsigned long long)nz, (unsigned long long)((uint64_t)w * h));
            ++resolveDumps;
        }
    }
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
        ImportLog::Get().Printf("M8 resolve diferido: bufer de lectura de %llu MB", (unsigned long long)(resolvePoolBytes >> 20));
    }
    if (resolvePoolOffset + need > resolvePoolBytes) {
        FlushResolves();
        if (need > resolvePoolBytes) { Note("OMISION resolve: region mayor que el bufer de lectura diferido"); return false; }
    }
    return true;
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
            WriteResolve(a, p + a.poolOffset);
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
    // las copias del ultimo resolve pueden estar aun en la lista abierta: se envian (sin esperar)
    if (clOpen && wrMaxFence > fenceValue) Kick();
    std::unique_lock<std::mutex> g(wrMu);
    wrDone.wait(g, [&] { return wrQueue.empty(); });
    resolvePoolOffset = 0;
}

void D3D12Backend::EnqueueResolve(PendingResolve pr) {
    if (!g_asyncResolveWriter) { pendingResolves.push_back(pr); return; }
    pr.fence = fenceValue + 1;   // la copia esta en la lista abierta: la senaliza el siguiente Submit
    {
        std::lock_guard<std::mutex> g(wrMu);
        if (!g_resolveNoDse)   // eliminacion de escrituras muertas: solo entradas aun no iniciadas (la primera puede estar en curso)
            for (size_t i = wrBusy ? 1 : 0; i < wrQueue.size(); ++i) {
                PendingResolve& o = wrQueue[i];
                if (!o.dead && pr.destBase == o.destBase && pr.dpAligned == o.dpAligned && pr.bppLog2 == o.bppLog2 && pr.x0 <= o.x0 && pr.y0 <= o.y0 && pr.x1 >= o.x1 && pr.y1 >= o.y1) o.dead = true;
            }
        wrQueue.push_back(pr);
        if (pr.fence > wrMaxFence) wrMaxFence = pr.fence;
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
                    wrCv.wait(g, [&] { return wrStop || !wrQueue.empty(); });
                    if (wrStop && wrQueue.empty()) break;
                    wrBusy = true;
                    pr = wrQueue.front();
                }
                if (fence->GetCompletedValue() < pr.fence) {
                    fence->SetEventOnCompletion(pr.fence, ev);
                    WaitForSingleObject(ev, 20000);
                }
                bool dead;
                { std::lock_guard<std::mutex> g(wrMu); dead = wrQueue.front().dead; }
                if (dead) ++resolveDeadSkipped;
                else { WriteResolve(pr, resolvePoolCpu + pr.poolOffset); ++resolveWritten; }
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

void D3D12Backend::FlushResolvesIfOverlap(uint32_t lo, uint32_t hi) {
    bool hit = false;
    if (!g_asyncResolveWriter) {
        for (auto& pr : pendingResolves) if ((int64_t)lo < pr.hi && (int64_t)hi > pr.lo) { hit = true; break; }
    } else {
        std::lock_guard<std::mutex> g(wrMu);
        for (auto& pr : wrQueue) if ((int64_t)lo < pr.hi && (int64_t)hi > pr.lo) { hit = true; break; }
    }
    if (hit) { ++resolveFlushByRead; FlushResolves(); }
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
