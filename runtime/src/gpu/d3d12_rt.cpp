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

// Escritura de un pixel de 32 bits en la memoria del guest con el endian del resolve (Endian128: solo 0-3 para 32 bpp).
static inline uint32_t SwapE(uint32_t v, uint32_t e) { return GpuSwap32(v, e & 3); }

bool D3D12Backend::DoResolve(const DrawEvent& ev) {
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
            uint32_t bpp = BytesPerPixel(src->format);
            uint32_t rowPitch = (w * bpp + 255) & ~255u;
            uint64_t need = (uint64_t)rowPitch * h;
            if (!resolveReadback || resolveReadbackBytes < need) {
                D3D12_HEAP_PROPERTIES rp = {D3D12_HEAP_TYPE_READBACK};
                D3D12_RESOURCE_DESC bd = {};
                resolveReadbackBytes = std::max<uint64_t>(need, 8u << 20);
                bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; bd.Width = resolveReadbackBytes; bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
                bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                resolveReadback.Reset();
                if (!Hr(dev->CreateCommittedResource(&rp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resolveReadback)), "readback del resolve")) return false;
            }
            Transition(src->res.Get(), src->state, D3D12_RESOURCE_STATE_COPY_SOURCE); src->state = D3D12_RESOURCE_STATE_COPY_SOURCE;
            D3D12_TEXTURE_COPY_LOCATION dst = {}, sl = {};
            dst.pResource = resolveReadback.Get(); dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            dst.PlacedFootprint.Footprint = {src->format, w, h, 1, rowPitch};
            sl.pResource = src->res.Get(); sl.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; sl.SubresourceIndex = 0;
            D3D12_BOX box = {(UINT)x0, (UINT)y0, 0, (UINT)x1, (UINT)y1, 1};
            cl->CopyTextureRegion(&dst, 0, 0, 0, &sl, &box);
            FlushAndWait();
            void* p = nullptr;
            D3D12_RANGE rr = {0, (SIZE_T)need};
            if (!Hr(resolveReadback->Map(0, &rr, &p), "Map(readback del resolve)")) return false;
            // direccionamiento del destino (texture_address.h): base + Tiled2D(x, y) con las mismas coordenadas que el RT
            uint32_t bppLog2 = bpp == 8 ? 3 : 2;
            uint32_t dpAligned = (dpitch.copy_dest_pitch + 31) & ~31u;
            if (!dpAligned) dpAligned = (surf.surface_pitch + 31) & ~31u;
            uint32_t ax0 = (uint32_t)x0 & ~31u, ay0 = (uint32_t)y0 & ~31u, ax1 = ((uint32_t)x1 + 31) & ~31u, ay1 = ((uint32_t)y1 + 31) & ~31u;
            int64_t lo = (int64_t)destBase + xe::gpu::texture_address::Tiled2D((int32_t)ax0, (int32_t)ay0, dpAligned, bppLog2);
            int64_t hi = (int64_t)destBase + xe::gpu::texture_address::Tiled2D((int32_t)ax1 - 32, (int32_t)ay1 - 32, dpAligned, bppLog2) + 32 * 32 * (1 << bppLog2);
            uint8_t* mem = nullptr;
            if (lo >= 0 && hi > lo && hi <= 0x20000000) mem = PhysHost((uint32_t)lo, (uint32_t)(hi - lo));
            if (!mem) {
                Note("OMISION resolve: memoria de destino no asignada o fuera de rango");
            } else {
                uint32_t endian = (uint32_t)dinfo.copy_dest_endian;
                bool swapRB = dinfo.copy_dest_swap != 0;
                const uint8_t* srcp = (const uint8_t*)p;
                for (uint32_t y = 0; y < h; ++y)
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
                wrote = true;
            }
            if (bpp == 4) {   // estadisticas del contenido resuelto (RGB != 0) y volcado opcional
                uint64_t nz = 0;
                for (uint32_t y = 0; y < h; ++y) {
                    const uint32_t* row = reinterpret_cast<const uint32_t*>((const uint8_t*)p + (size_t)y * rowPitch);
                    for (uint32_t x = 0; x < w; ++x) if (row[x] & 0x00FFFFFFu) ++nz;
                }
                lastResolveNonzero = nz; lastResolvePixels = (uint64_t)w * h;
                if (resolveDumps < opt.dumpResolves && (sfmt == 0 || sfmt == 1)) {
                    std::vector<uint8_t> img((size_t)w * h * 4);
                    for (uint32_t y = 0; y < h; ++y) std::memcpy(&img[(size_t)y * w * 4], (const uint8_t*)p + (size_t)y * rowPitch, (size_t)w * 4);
                    char nm[96];
                    std::snprintf(nm, sizeof(nm), "\\userdata\\frames\\resolve_%03u_%08X_%ux%u.png", resolveDumps, destBase, w, h);
                    std::string path = GetPaths().runtimeDir + nm;
                    bool okp = WritePngRGBA(path, w, h, img.data(), w * 4);
                    ImportLog::Get().Printf("D3D12 volcado de resolve -> %s %s nonzero=%llu/%llu", path.c_str(), okp ? "OK" : "FALLO", (unsigned long long)nz, (unsigned long long)((uint64_t)w * h));
                    ++resolveDumps;
                }
            }
            D3D12_RANGE wr = {0, 0};
            resolveReadback->Unmap(0, &wr);
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

} // namespace rt::gpu::d3d
