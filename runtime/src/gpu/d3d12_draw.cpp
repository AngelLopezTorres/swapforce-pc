#include "fast_hash.h"
#include "perf.h"
// H3: draws. Estado de D3D12 derivado de los registros del Xenos (port reducido de pipeline_cache.cc y draw_util.cc de Xenia,
// copias en runtime\ref\). Simplificaciones registradas con Note("SIMPLIFY ...").
#include <algorithm>
#include <cmath>

#include "config.h"
#include "d3d12_impl.h"
#include "fb_decode.h"

namespace rt::gpu::d3d {

static std::atomic<uint64_t> g_dumpHlslHash{0};
static std::atomic<uint64_t> g_debugStopFrame{0};

void SetDumpHlslHash(uint64_t hash) { g_dumpHlslHash.store(hash); }

static float HalfToFloat(uint16_t h) {
    const uint32_t sign = (uint32_t)(h & 0x8000u) << 16;
    uint32_t exp = (h >> 10) & 0x1Fu, mant = h & 0x03FFu, bits;
    if (!exp) {
        if (!mant) bits = sign;
        else {
            int shift = 0;
            while (!(mant & 0x0400u)) { mant <<= 1; ++shift; }
            mant &= 0x03FFu;
            bits = sign | ((uint32_t)(127 - 14 - shift) << 23) | (mant << 13);
        }
    } else if (exp == 0x1Fu) bits = sign | 0x7F800000u | (mant << 13);
    else bits = sign | ((exp + 112u) << 23) | (mant << 13);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

// ---- constantes del shader (debe coincidir con el cbuffer XeConsts de ucode_hlsl.cpp) ----
struct XeConsts {
    float vsFloat[256][4];
    float psFloat[256][4];
    uint32_t fetch[48][4];
    uint32_t boolC[2][4];
    uint32_t loop[8][4];
    uint32_t draw0[4], draw1[4], draw2[4];
    float ndcScale[4], ndcOffset[4];
};
static_assert(sizeof(XeConsts) == 9200, "XeConsts debe medir 9200 bytes (4096*2 + 768 + 32 + 128 + 16*5)");

bool D3D12Backend::CreateRootSignature() {
    D3D12_DESCRIPTOR_RANGE srv[3] = {};
    for (int i = 0; i < 3; ++i) {
        srv[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; srv[i].NumDescriptors = 32; srv[i].BaseShaderRegister = 0;
        srv[i].RegisterSpace = 1 + i; srv[i].OffsetInDescriptorsFromTableStart = i * 32;
    }
    D3D12_DESCRIPTOR_RANGE smp = {};
    smp.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER; smp.NumDescriptors = 32; smp.BaseShaderRegister = 0; smp.RegisterSpace = 0;
    smp.OffsetInDescriptorsFromTableStart = 0;
    D3D12_ROOT_PARAMETER p[4] = {};
    p[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; p[0].Descriptor = {0, 0}; p[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    p[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV; p[1].Descriptor = {0, 0}; p[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    p[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; p[2].DescriptorTable = {3, srv}; p[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    p[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; p[3].DescriptorTable = {1, &smp}; p[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    D3D12_ROOT_SIGNATURE_DESC rs = {};
    rs.NumParameters = 4; rs.pParameters = p; rs.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    ComPtr<ID3DBlob> blob, err;
    HRESULT hr = D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err);
    if (FAILED(hr)) {
        ImportLog::Get().Printf("D3D12SerializeRootSignature fallo: %s", err ? (const char*)err->GetBufferPointer() : "?");
        return false;
    }
    return Hr(dev->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSig)), "CreateRootSignature");
}

// M60: anillo de kSrvSegments segmentos. Al entrar en un segmento se espera a la ultima valla que lo uso (normalmente ya cumplida),
// en lugar de vaciar toda la GPU (FlushAndWait) al llenarse el heap.
uint32_t D3D12Backend::AllocGpuSrv(uint32_t n) {
    constexpr uint32_t seg = kCbvSrvHeapSize / kSrvSegments;
    uint32_t s0 = gpuSrvNext / seg;
    uint32_t next = gpuSrvNext;
    if (next + n > kCbvSrvHeapSize) next = 0;
    else if ((next + n - 1) / seg != s0) next = (next + n - 1) / seg * seg;
    uint32_t s1 = next / seg;
    if (s1 != s0 || next != gpuSrvNext) {
        if (srvSegFence[s1] > fenceValue) Kick();       // el segmento lo usa la lista abierta (no deberia ocurrir con 4 segmentos)
        WaitFence(srvSegFence[s1]);
    }
    gpuSrvNext = next + n;
    srvSegFence[s1] = fenceValue + 1;   // la lista abierta se senaliza con el siguiente Submit
    return next;
}

// ---- shaders -> DXIL (cache en memoria y en disco por hash) ----
static bool ReadAll(const std::string& p, std::string& out) {
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return false;
    std::fseek(f, 0, SEEK_END); long n = std::ftell(f); std::fseek(f, 0, SEEK_SET);
    out.resize(n > 0 ? n : 0);
    bool ok = n > 0 && std::fread(&out[0], 1, n, f) == (size_t)n;
    std::fclose(f);
    return ok;
}
static void WriteAll(const std::string& p, const std::string& d) {
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fwrite(d.data(), 1, d.size(), f); std::fclose(f); }
}

const D3D12Backend::DxilBlob* D3D12Backend::GetDxil(const ShaderInfo& si, int paramGenPos) {
    uint64_t requestedHash = si.hash;
    const bool dumpHlsl = requestedHash != 0 && g_dumpHlslHash.compare_exchange_strong(requestedHash, 0);
    HlslResult dumpedHlsl;
    bool translatedForDump = false;
    if (dumpHlsl) {
        HlslOptions dumpOptions; dumpOptions.paramGenPos = paramGenPos;
        dumpedHlsl = TranslateToHlsl(si, dumpOptions);
        translatedForDump = true;
        if (dumpedHlsl.ok) {
            const std::string hlslDir = GetPaths().userdataDir + "\\shaders\\hlsl";
            CreateDirectoryA(GetPaths().userdataDir.c_str(), nullptr);
            CreateDirectoryA((GetPaths().userdataDir + "\\shaders").c_str(), nullptr);
            CreateDirectoryA(hlslDir.c_str(), nullptr);
            char hlslName[128];
            std::snprintf(hlslName, sizeof(hlslName), "\\%016llX_pg%d_%s.hlsl",
                          (unsigned long long)si.hash, paramGenPos, si.vertex ? "vs" : "ps");
            const std::string hlslPath = hlslDir + hlslName;
            WriteAll(hlslPath, dumpedHlsl.hlsl);
            ImportLog::Get().Printf("D3D12 HLSL_DUMP %016llX %s ruta=%s bytes=%u",
                                    (unsigned long long)si.hash, si.vertex ? "vs" : "ps", hlslPath.c_str(),
                                    (unsigned)dumpedHlsl.hlsl.size());
        } else {
            ImportLog::Get().Printf("D3D12 HLSL_DUMP_FAIL %016llX %s traduccion: %s",
                                    (unsigned long long)si.hash, si.vertex ? "vs" : "ps", dumpedHlsl.error.c_str());
        }
    }
    uint64_t key = si.hash ^ ((uint64_t)(paramGenPos + 1) * 0x9E3779B97F4A7C15ull);
    auto it = dxilCache.find(key);
    if (it != dxilCache.end()) return it->second.ok ? &it->second : nullptr;
    DxilBlob& b = dxilCache[key];
    std::string dir = GetPaths().userdataDir + "\\shaders\\dxil";
    CreateDirectoryA(GetPaths().userdataDir.c_str(), nullptr);
    CreateDirectoryA((GetPaths().userdataDir + "\\shaders").c_str(), nullptr);
    CreateDirectoryA(dir.c_str(), nullptr);
    char name[160];
    std::snprintf(name, sizeof(name), "\\%016llX_pg%d_%s_%s.dxil", (unsigned long long)si.hash, paramGenPos, kTranslatorVersion, si.vertex ? "vs" : "ps");
    std::string path = dir + name;
    if (ReadAll(path, b.data)) { b.ok = true; return &b; }
    HlslOptions ho; ho.paramGenPos = paramGenPos;
    HlslResult hr = translatedForDump ? std::move(dumpedHlsl) : TranslateToHlsl(si, ho);
    if (!hr.ok) { b.err = hr.error; ImportLog::Get().Printf("D3D12 SHADER_FAIL %016llX %s traduccion: %s", (unsigned long long)si.hash, si.vertex ? "vs" : "ps", hr.error.c_str()); return nullptr; }
    std::string err;
    if (!CompileHlslToDxil(hr.hlsl, si.vertex, b.data, err)) {
        b.err = err;
        ImportLog::Get().Printf("D3D12 SHADER_FAIL %016llX %s DXC: %s", (unsigned long long)si.hash, si.vertex ? "vs" : "ps", err.substr(0, 600).c_str());
        return nullptr;
    }
    WriteAll(path, b.data);
    b.ok = true;
    return &b;
}

const D3D12Backend::DxilBlob* D3D12Backend::GetRectGs() {
    if (!gsRectTried) {
        gsRectTried = true;
        std::string err;
        gsRect.ok = CompileHlslProfile(RectListGeometryShaderHlsl(), L"gs_6_0", gsRect.data, err);
        if (!gsRect.ok) ImportLog::Get().Printf("D3D12 GS de lista de rectangulos: DXC fallo: %s", err.substr(0, 600).c_str());
    }
    return gsRect.ok ? &gsRect : nullptr;
}

// ---- PSO ----
static D3D12_BLEND MapBlend(uint32_t f, bool alpha) {
    switch (f) {
        case 0: return D3D12_BLEND_ZERO;
        case 1: return D3D12_BLEND_ONE;
        case 4: return alpha ? D3D12_BLEND_SRC_ALPHA : D3D12_BLEND_SRC_COLOR;
        case 5: return alpha ? D3D12_BLEND_INV_SRC_ALPHA : D3D12_BLEND_INV_SRC_COLOR;
        case 6: return D3D12_BLEND_SRC_ALPHA;
        case 7: return D3D12_BLEND_INV_SRC_ALPHA;
        case 8: return alpha ? D3D12_BLEND_DEST_ALPHA : D3D12_BLEND_DEST_COLOR;
        case 9: return alpha ? D3D12_BLEND_INV_DEST_ALPHA : D3D12_BLEND_INV_DEST_COLOR;
        case 10: return D3D12_BLEND_DEST_ALPHA;
        case 11: return D3D12_BLEND_INV_DEST_ALPHA;
        case 12: case 14: return D3D12_BLEND_BLEND_FACTOR;
        case 13: case 15: return D3D12_BLEND_INV_BLEND_FACTOR;
        case 16: return D3D12_BLEND_SRC_ALPHA_SAT;
        default: return D3D12_BLEND_ZERO;
    }
}
static D3D12_BLEND_OP MapBlendOp(uint32_t o) {
    switch (o) { case 0: return D3D12_BLEND_OP_ADD; case 1: return D3D12_BLEND_OP_SUBTRACT; case 2: return D3D12_BLEND_OP_MIN;
                 case 3: return D3D12_BLEND_OP_MAX; case 4: return D3D12_BLEND_OP_REV_SUBTRACT; default: return D3D12_BLEND_OP_ADD; }
}

uint32_t g_psoRtFmt[4], g_psoDsvFmt, g_psoNrt;

struct PsoState {
    uint64_t vs, ps; int pg;
    uint32_t topo; bool rectGs;
    uint32_t nrt; uint32_t rtFmt[4]; uint32_t dsvFmt;
    uint32_t blend[4]; uint32_t colorMask;
    uint32_t depthControl; uint32_t stencilRead, stencilWrite;
    uint32_t modeCntl; uint32_t clipDisable;
    uint32_t pad;
};

ID3D12PipelineState* D3D12Backend::GetPso(const ShaderInfo& vsI, const ShaderInfo* psI, uint32_t topoClass, bool rectGs) {
    // el llamador deja el estado en `cur`; se reconstruye aqui a partir de los registros (mismos valores)
    auto edramMode = R<reg::RB_MODECONTROL>().edram_mode;
    bool colorMode = edramMode == xenos::EdramMode::kColorDepth;
    PsoState st;
    std::memset(&st, 0, sizeof(st));
    auto sq = R<reg::SQ_PROGRAM_CNTL>();
    auto misc = R<reg::SQ_CONTEXT_MISC>();
    int pg = (psI && sq.param_gen) ? (int)misc.param_gen_pos : -1;
    st.vs = vsI.hash; st.ps = psI ? psI->hash : 0; st.pg = pg; st.topo = topoClass; st.rectGs = rectGs;
    // render targets: formatos de los RT vinculados (los fija DoDraw en colorFmtCur/depthFmtCur)
    st.nrt = g_psoNrt; for (int i = 0; i < 4; ++i) st.rtFmt[i] = g_psoRtFmt[i]; st.dsvFmt = g_psoDsvFmt;
    auto depth = R<reg::RB_DEPTHCONTROL>();
    st.depthControl = depth.value;
    for (int i = 0; i < 4; ++i) st.blend[i] = RV(0x2201 + (i == 0 ? 0 : 7 + i));   // RB_BLENDCONTROL0 = 0x2201, 1..3 = 0x2209..0x220B
    st.colorMask = RV(0x2104);
    auto sr = R<reg::RB_STENCILREFMASK>();
    st.stencilRead = sr.stencilmask; st.stencilWrite = sr.stencilwritemask;
    st.modeCntl = R<reg::PA_SU_SC_MODE_CNTL>().value & 0x7; st.clipDisable = R<reg::PA_CL_CLIP_CNTL>().clip_disable;
    uint64_t hkey = FastHash(&st, sizeof(st));
    {
        auto range = psoFast.equal_range(hkey);
        for (auto r = range.first; r != range.second; ++r)
            if (r->second.first.size() == sizeof(st) && !std::memcmp(r->second.first.data(), &st, sizeof(st))) return r->second.second;
    }
    std::string key((const char*)&st, sizeof(st));
    auto it = psoCache.find(key);
    if (it != psoCache.end()) { psoFast.emplace(hkey, std::make_pair(key, it->second.Get())); return it->second.Get(); }
    PERF_ZONE(kZonePsoCreate);
    rt::gpu::M60Event('P');

    const DxilBlob* vsB = GetDxil(vsI, -1);
    const DxilBlob* psB = psI ? GetDxil(*psI, pg) : nullptr;
    if (!vsB || (psI && !psB)) { ++shaderFail; ++fShaderFail; return nullptr; }
    D3D12_GRAPHICS_PIPELINE_STATE_DESC d = {};
    d.pRootSignature = rootSig.Get();
    d.VS = {vsB->data.data(), vsB->data.size()};
    if (psB) d.PS = {psB->data.data(), psB->data.size()};
    if (rectGs) {
        const DxilBlob* g = GetRectGs();
        if (!g) { ++shaderFail; ++fShaderFail; return nullptr; }
        d.GS = {g->data.data(), g->data.size()};
    }
    d.SampleMask = UINT_MAX;
    d.SampleDesc.Count = 1;
    d.PrimitiveTopologyType = topoClass == 0 ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT : topoClass == 1 ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // rasterizador
    auto mc = R<reg::PA_SU_SC_MODE_CNTL>();
    d.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    d.RasterizerState.CullMode = mc.cull_front ? D3D12_CULL_MODE_FRONT : mc.cull_back ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
    d.RasterizerState.FrontCounterClockwise = mc.face == 0;
    d.RasterizerState.DepthClipEnable = R<reg::PA_CL_CLIP_CNTL>().clip_disable ? FALSE : TRUE;
    // profundidad / stencil
    bool depthOn = depth.z_enable && !(depth.zfunc == xenos::CompareFunction::kAlways && !depth.z_write_enable);
    bool stencilOn = depth.stencil_enable != 0;
    if (!colorMode && edramMode != xenos::EdramMode::kDepthOnly) { depthOn = false; stencilOn = false; }
    if (st.dsvFmt == 0) { depthOn = false; stencilOn = false; }
    if (depthOn) {
        d.DepthStencilState.DepthEnable = TRUE;
        d.DepthStencilState.DepthWriteMask = depth.z_write_enable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        d.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)((uint32_t)D3D12_COMPARISON_FUNC_NEVER + (uint32_t)depth.zfunc);
        // En esta ruta de viewport/EDRAM, invertir la comparacion Z reproduce
        // la profundidad del titulo; la causa general aun no esta verificada.
        auto& f = d.DepthStencilState.DepthFunc;
        if (f == D3D12_COMPARISON_FUNC_GREATER_EQUAL) f = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        else if (f == D3D12_COMPARISON_FUNC_GREATER) f = D3D12_COMPARISON_FUNC_LESS;
        else if (f == D3D12_COMPARISON_FUNC_LESS_EQUAL) f = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        else if (f == D3D12_COMPARISON_FUNC_LESS) f = D3D12_COMPARISON_FUNC_GREATER;
    }
    if (stencilOn) {
        d.DepthStencilState.StencilEnable = TRUE;
        d.DepthStencilState.StencilReadMask = (UINT8)sr.stencilmask;
        d.DepthStencilState.StencilWriteMask = (UINT8)sr.stencilwritemask;
        auto Op = [](xenos::StencilOp o) { return (D3D12_STENCIL_OP)((uint32_t)D3D12_STENCIL_OP_KEEP + (uint32_t)o); };
        auto Fn = [](xenos::CompareFunction f) { return (D3D12_COMPARISON_FUNC)((uint32_t)D3D12_COMPARISON_FUNC_NEVER + (uint32_t)f); };
        d.DepthStencilState.FrontFace = {Op(depth.stencilfail), Op(depth.stencilzfail), Op(depth.stencilzpass), Fn(depth.stencilfunc)};
        if (depth.backface_enable) d.DepthStencilState.BackFace = {Op(depth.stencilfail_bf), Op(depth.stencilzfail_bf), Op(depth.stencilzpass_bf), Fn(depth.stencilfunc_bf)};
        else d.DepthStencilState.BackFace = d.DepthStencilState.FrontFace;
    }
    if (depthOn || stencilOn) d.DSVFormat = (DXGI_FORMAT)st.dsvFmt;
    // color y mezcla
    d.BlendState.IndependentBlendEnable = TRUE;
    d.NumRenderTargets = colorMode ? st.nrt : 0;
    for (uint32_t i = 0; i < d.NumRenderTargets; ++i) {
        d.RTVFormats[i] = (DXGI_FORMAT)st.rtFmt[i];
        auto bc = R<reg::RB_BLENDCONTROL>(); bc.value = st.blend[i];
        D3D12_RENDER_TARGET_BLEND_DESC& b = d.BlendState.RenderTarget[i];
        uint32_t wm = (st.colorMask >> (4 * i)) & 0xF;
        b.RenderTargetWriteMask = (UINT8)wm;
        bool trivial = (uint32_t)bc.color_srcblend == 1 && (uint32_t)bc.color_destblend == 0 && (uint32_t)bc.color_comb_fcn == 0 &&
                       (uint32_t)bc.alpha_srcblend == 1 && (uint32_t)bc.alpha_destblend == 0 && (uint32_t)bc.alpha_comb_fcn == 0;
        if (wm && !trivial) {
            b.BlendEnable = TRUE;
            b.SrcBlend = MapBlend((uint32_t)bc.color_srcblend, false); b.DestBlend = MapBlend((uint32_t)bc.color_destblend, false); b.BlendOp = MapBlendOp((uint32_t)bc.color_comb_fcn);
            b.SrcBlendAlpha = MapBlend((uint32_t)bc.alpha_srcblend, true); b.DestBlendAlpha = MapBlend((uint32_t)bc.alpha_destblend, true); b.BlendOpAlpha = MapBlendOp((uint32_t)bc.alpha_comb_fcn);
        }
    }
    ComPtr<ID3D12PipelineState> pso;
    HRESULT hr = dev->CreateGraphicsPipelineState(&d, IID_PPV_ARGS(&pso));
    if (FAILED(hr)) {
        ++psoFail; ++fPsoFail;
        ImportLog::Get().Printf("D3D12 PSO_FAIL hr=0x%08X vs=%016llX ps=%016llX nrt=%u rt0=%u dsv=%u topo=%u rectgs=%d", (unsigned)hr, (unsigned long long)st.vs,
                                (unsigned long long)st.ps, d.NumRenderTargets, st.rtFmt[0], st.dsvFmt, topoClass, (int)rectGs);
        psoCache[key] = nullptr;
        return nullptr;
    }
    ID3D12PipelineState* raw = pso.Get();
    psoCache[key] = pso;
    return raw;
}

// ---- viewport (port de draw_util::GetHostViewportInfo con escala de resolucion 1, sin float24) ----
struct ViewportInfo { uint32_t xy_offset[2], xy_extent[2]; float z_min, z_max, ndc_scale[3], ndc_offset[3]; };

static void GetViewport(uint32_t xyMax, ViewportInfo& v, bool pixelShaderWritesDepth) {
    auto clip = R<reg::PA_CL_CLIP_CNTL>();
    auto vte = R<reg::PA_CL_VTE_CNTL>();
    auto mc = R<reg::PA_SU_SC_MODE_CNTL>();
    auto vtx = R<reg::PA_SU_VTX_CNTL>();
    float scale_xy[2] = {vte.vport_x_scale_ena ? RF(0x210F) : 1.0f, vte.vport_y_scale_ena ? RF(0x2111) : 1.0f};
    float scale_z = vte.vport_z_scale_ena ? RF(0x2113) : 1.0f;
    float off_xy[2] = {vte.vport_x_offset_ena ? RF(0x2110) : 0.0f, vte.vport_y_offset_ena ? RF(0x2112) : 0.0f};
    float off_z = vte.vport_z_offset_ena ? RF(0x2114) : 0.0f;
    float add_xy[2] = {0, 0};
    if (mc.vtx_window_offset_enable) {
        auto wo = R<reg::PA_SC_WINDOW_OFFSET>();
        add_xy[0] += (float)wo.window_x_offset; add_xy[1] += (float)wo.window_y_offset;
    }
    if (vtx.pix_center == xenos::PixelCenter::kD3DZero) { add_xy[0] += 0.5f; add_xy[1] += 0.5f; }   // half_pixel_offset (cvar por defecto de Xenia)
    float z_min, z_max, ns[3], no[3];
    if (clip.clip_disable) {
        for (int i = 0; i < 2; ++i) {
            v.xy_offset[i] = 0;
            uint32_t ext = std::min(8192u, xyMax);
            v.xy_extent[i] = ext;
            float e = (float)ext, p2n = 2.0f / e;
            ns[i] = scale_xy[i] * p2n;
            no[i] = (off_xy[i] - e * 0.5f + add_xy[i]) * p2n;
        }
        z_min = 0.0f; z_max = 1.0f; ns[2] = scale_z; no[2] = off_z;
    } else {
        for (int i = 0; i < 2; ++i) {
            float off = off_xy[i] + add_xy[i], sc = scale_xy[i], sa = std::fabs(sc), mx = (float)xyMax;
            uint32_t a0 = (uint32_t)std::min(std::max(off - sa, 0.0f), mx), a1 = (uint32_t)std::min(std::max(off + sa, 0.0f), mx);
            uint32_t ext = a1 - a0;
            v.xy_offset[i] = a0; v.xy_extent[i] = ext;
            if (ext) { ns[i] = sc * 2.0f / (float)ext; no[i] = (off - ((float)a0 + (float)ext * 0.5f)) * 2.0f / (float)ext; }
            else { ns[i] = 1.0f; no[i] = 0.0f; }
        }
        float hco, hcs;
        if (clip.dx_clip_space_def) { hco = off_z; hcs = scale_z; ns[2] = 1.0f; no[2] = 0.0f; }
        else { hco = off_z - scale_z; hcs = scale_z * 2.0f; ns[2] = 0.5f; no[2] = 0.5f; }
        if (pixelShaderWritesDepth) { z_min = 0.0f; z_max = 1.0f; }
        else {
            z_min = std::min(std::max(hco, 0.0f), 1.0f); z_max = std::min(std::max(hco + hcs, 0.0f), 1.0f);
            if (z_min > z_max) { std::swap(z_min, z_max); ns[2] = -ns[2]; no[2] = 1.0f - no[2]; }
        }
    }
    v.z_min = z_min; v.z_max = z_max;
    ns[1] = -ns[1]; no[1] = -no[1];    // origin_bottom_left = true en Direct3D 12
    for (int i = 0; i < 3; ++i) { v.ndc_scale[i] = ns[i]; v.ndc_offset[i] = no[i]; }
}

static void GetScissor(int32_t out[4]) {   // x0, y0, x1, y1
    auto tl = R<reg::PA_SC_WINDOW_SCISSOR_TL>(); auto br = R<reg::PA_SC_WINDOW_SCISSOR_BR>();
    int32_t tlx = (int32_t)tl.tl_x, tly = (int32_t)tl.tl_y, brx = (int32_t)br.br_x, bry = (int32_t)br.br_y;
    if (!tl.window_offset_disable) {
        auto wo = R<reg::PA_SC_WINDOW_OFFSET>();
        tlx += wo.window_x_offset; tly += wo.window_y_offset; brx += wo.window_x_offset; bry += wo.window_y_offset;
    }
    auto stl = R<reg::PA_SC_SCREEN_SCISSOR_TL>(); auto sbr = R<reg::PA_SC_SCREEN_SCISSOR_BR>();
    tlx = std::max(tlx, (int32_t)stl.tl_x); tly = std::max(tly, (int32_t)stl.tl_y);
    brx = std::min(brx, (int32_t)sbr.br_x); bry = std::min(bry, (int32_t)sbr.br_y);
    uint32_t pitch = R<reg::RB_SURFACE_INFO>().surface_pitch;
    tlx = std::min(tlx, (int32_t)pitch); brx = std::min(brx, (int32_t)pitch);
    tlx = std::max(tlx, 0); tly = std::max(tly, 0);
    brx = std::max(brx, tlx); bry = std::max(bry, tly);
    out[0] = tlx; out[1] = tly; out[2] = brx; out[3] = bry;
}

bool D3D12Backend::DoDraw(const DrawEvent& ev, const ShaderInfo* vs, const ShaderInfo* ps) {
    auto mode = R<reg::RB_MODECONTROL>().edram_mode;
    if (mode == xenos::EdramMode::kCopy) {
        bool ok = DoResolve(ev);
        if (ok) { ++resolves; ++fResolve; } else Note("OMISION resolve: fallo (ver notas previas)");
        return ok;
    }
    if (g_debugStopFrame.load(std::memory_order_relaxed) == rt::gpu::FrameCount() + 1) return true;
    if (mode != xenos::EdramMode::kColorDepth && mode != xenos::EdramMode::kDepthOnly) { Note("OMISION draw con edram_mode=kNoOperation"); return false; }
    if (!vs) { Note("OMISION draw sin vertex shader cargado"); return false; }
    bool colorMode = mode == xenos::EdramMode::kColorDepth;
    if (colorMode && !ps) { Note("OMISION draw en kColorDepth sin pixel shader"); return false; }
    auto surf = R<reg::RB_SURFACE_INFO>();
    if (!surf.surface_pitch) { Note("OMISION draw con surface_pitch=0"); return false; }
    auto mc = R<reg::PA_SU_SC_MODE_CNTL>();
    uint32_t prim = ev.prim;
    bool polygonal = prim == 4 || prim == 5 || prim == 6 || prim == 7 || prim == 13 || prim == 14 || prim == 15 || prim == 8;
    if (polygonal && mc.cull_front && mc.cull_back) return true;      // ambas caras descartadas: no hay rasterizacion
    if (ev.majorMode != 0 || prim >= 0x10) { Note("OMISION draw con modo mayor explicito/primitiva >= 0x10 (teselacion o copia 2D)"); return false; }
    // ---- primitiva ----
    uint32_t n = ev.nIdx, count = n, topoClass = 2;
    D3D12_PRIMITIVE_TOPOLOGY topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    bool rectGs = false;
    switch (prim) {
        case 1: topoClass = 0; topo = D3D_PRIMITIVE_TOPOLOGY_POINTLIST; break;
        case 2: topoClass = 1; topo = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
        case 3: topoClass = 1; topo = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP; break;
        case 4: topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; count = n / 3 * 3; break;
        case 5: case 15: topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; count = n >= 3 ? (n - 2) * 3 : 0; break;
        case 6: topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
        case 8: topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; rectGs = true; count = n / 3 * 3; break;
        case 13: topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; count = n / 4 * 6; break;
        default: Note("OMISION primitiva no soportada tipo=" + std::to_string(prim)); return false;
    }
    if (prim == 1 && vs->writesPointSize) Note("SIMPLIFY lista de puntos con tamano de punto exportado: se dibuja 1 pixel (sin expansion a sprite)");
    if (!count) return true;
    // ---- render targets ----
    int32_t sc[4];
    GetScissor(sc);
    uint32_t needH = std::min(((uint32_t)std::max(sc[3], 1) + 31u) & ~31u, 4096u);
    HostRT* cRT[4] = {nullptr, nullptr, nullptr, nullptr};
    uint32_t nrt = 0;
    if (colorMode) {
        for (uint32_t i = 0; i < 4; ++i) {
            if (!(ps->writesColors & (1u << i))) continue;
            auto ci = R<reg::RB_COLOR_INFO>(); ci.value = RV(i == 0 ? 0x2001 : 0x2003 + (i - 1));
            uint32_t mask = (RV(0x2104) >> (4 * i)) & 0xF;
            if (!mask) continue;
            cRT[i] = GetColorRT(ci.color_base | (ci.color_base_bit_11 << 11), surf.surface_pitch, (uint32_t)ci.color_format, needH);
            if (!cRT[i]) { Note("OMISION draw: no se pudo crear el render target de color"); return false; }
            nrt = i + 1;
        }
        // huecos (RT intermedios sin escribir): se rellenan con el RT correspondiente si existe, si no no se soporta
        for (uint32_t i = 0; i < nrt; ++i)
            if (!cRT[i]) { Note("OMISION draw: RT de color con huecos"); return false; }
    }
    auto dc = R<reg::RB_DEPTHCONTROL>();
    bool depthUsed = (dc.z_enable && !(dc.zfunc == xenos::CompareFunction::kAlways && !dc.z_write_enable)) || dc.stencil_enable;
    HostRT* dRT = nullptr;
    if (depthUsed) {
        auto di = R<reg::RB_DEPTH_INFO>();
        dRT = GetDepthRT(di.depth_base | (di.depth_base_bit_11 << 11), surf.surface_pitch, (uint32_t)di.depth_format, needH);
        if (!dRT) { Note("OMISION draw: no se pudo crear el render target de profundidad"); return false; }
    }
    if (colorMode && !nrt && !dRT) return true;   // nada que escribir
    for (uint32_t i = 0; i < 4; ++i) g_psoRtFmt[i] = (i < nrt && cRT[i]) ? (uint32_t)cRT[i]->format : 0;
    g_psoNrt = nrt; g_psoDsvFmt = dRT ? (uint32_t)dRT->format : 0;
    // ---- PSO ----
    ID3D12PipelineState* pso = GetPso(*vs, colorMode ? ps : nullptr, topoClass, rectGs);
    if (!pso) { Note("OMISION draw: sin PSO (shader_fail/pso_fail)"); return false; }
    // ---- memoria del guest para los fetch de vertice e indices ----
    for (uint32_t i = 0; i < 96; ++i) {
        if (!(vs->vfetchMask[i / 32] & (1u << (i % 32)))) continue;
        uint32_t d0 = RV(0x4800 + (i / 3) * 6 + (i % 3) * 2), d1 = RV(0x4800 + (i / 3) * 6 + (i % 3) * 2 + 1);
        if ((d0 & 3) != 3) continue;     // no es una constante de fetch de vertice valida
        uint32_t bytes = std::min<uint32_t>(((d1 >> 2) & 0xFFFFFF) * 4, 64u << 20);
        EnsureMemory(d0 & ~3u, bytes);
    }
    uint32_t idxBytes = ev.indexSize ? 4 : 2;
    if (ev.src == 0) {
        uint32_t lastIdx = n;   // cota superior de indices leidos (logicos): n
        EnsureMemory(ev.dmaBase & 0x1FFFFFFF, std::min<uint32_t>(lastIdx * idxBytes + 8, 64u << 20));
    }
    SetSharedState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    // ---- constantes ----
    UploadAlloc cb = Upload(sizeof(XeConsts), 256);
    if (!cb.cpu) { Note("OMISION draw: constantes demasiado grandes"); return false; }
    XeConsts* c = reinterpret_cast<XeConsts*>(cb.cpu);
    // M8: copia directa del fichero de registros (antes ~2300 llamadas a RegRead por draw)
    std::memcpy(c->vsFloat, rt::gpu::g_regFile + 0x4000, 1024 * 4);
    std::memcpy(c->psFloat, rt::gpu::g_regFile + 0x4400, 1024 * 4);
    std::memcpy(c->fetch, rt::gpu::g_regFile + 0x4800, 192 * 4);
    std::memcpy(c->boolC, rt::gpu::g_regFile + 0x4900, 8 * 4);
    std::memcpy(c->loop, rt::gpu::g_regFile + 0x4908, 32 * 4);
    c->draw0[0] = prim; c->draw0[1] = ev.src; c->draw0[2] = ev.indexSize; c->draw0[3] = ev.dmaSize >> 30;
    c->draw1[0] = ev.dmaBase & 0x1FFFFFFF; c->draw1[1] = n; c->draw1[2] = RV(0x2102) & 0xFFFFFF; c->draw1[3] = 0;
    {
        auto cc = R<reg::RB_COLORCONTROL>();
        c->draw2[0] = cc.alpha_test_enable ? ((uint32_t)cc.alpha_func | 8u) : 0u;
        uint32_t ar = RV(0x210E); c->draw2[1] = ar;
        auto vte = R<reg::PA_CL_VTE_CNTL>();
        c->draw2[2] = (vte.vtx_xy_fmt ? 1u : 0u) | (vte.vtx_z_fmt ? 2u : 0u) | (vte.vtx_w0_fmt ? 4u : 0u);
        c->draw2[3] = 0;
    }
    ViewportInfo vp;
    GetViewport(D3D12_VIEWPORT_BOUNDS_MAX, vp, ps && ps->writesDepth);
    for (int i = 0; i < 3; ++i) { c->ndcScale[i] = vp.ndc_scale[i]; c->ndcOffset[i] = vp.ndc_offset[i]; }
    c->ndcScale[3] = 1.0f; c->ndcOffset[3] = 0.0f;
    if (const char* dbg = std::getenv("XE_DBG_PS")) {   // depuracion: constantes y primeros bytes de textura para un PS (hash hexadecimal)
        static int dbgCount = 0;
        const uint64_t debugFrame = rt::gpu::Cfg().stateLogFrame;
        const bool debugFrameMatch = !debugFrame || rt::gpu::FrameCount() + 1 == debugFrame;
        if (ps && ps->hash == std::strtoull(dbg, nullptr, 16) && debugFrameMatch && dbgCount < 3) {
            ++dbgCount;
            ImportLog::Get().Printf("DBG PS %016llX draw=%llu c253=(%g,%g,%g,%g) c254=(%g,%g,%g,%g) c255=(%g,%g,%g,%g)", (unsigned long long)ps->hash, (unsigned long long)drawSerial,
                                    RF(0x4400 + 253 * 4), RF(0x4400 + 253 * 4 + 1), RF(0x4400 + 253 * 4 + 2), RF(0x4400 + 253 * 4 + 3),
                                    RF(0x4400 + 254 * 4), RF(0x4400 + 254 * 4 + 1), RF(0x4400 + 254 * 4 + 2), RF(0x4400 + 254 * 4 + 3),
                                    RF(0x4400 + 255 * 4), RF(0x4400 + 255 * 4 + 1), RF(0x4400 + 255 * 4 + 2), RF(0x4400 + 255 * 4 + 3));
            for (uint32_t t = 0; t < 3; ++t) {
                uint32_t w1 = RV(0x4800 + t * 6 + 1), w3 = RV(0x4800 + t * 6 + 3);
                const uint8_t* m = PhysHost(w1 & 0xFFFFF000, 64);
                char hex[200] = {};
                if (m) for (int i = 0; i < 32; ++i) std::snprintf(hex + i * 3, 4, "%02X ", m[i]);
                ImportLog::Get().Printf("DBG tex%u dw0=%08X dw1=%08X dw2=%08X dw3=%08X dw4=%08X dw5=%08X mem[0..31]=%s", t, RV(0x4800 + t * 6), w1, RV(0x4800 + t * 6 + 2), w3,
                                        RV(0x4800 + t * 6 + 4), RV(0x4800 + t * 6 + 5), hex);
            }
            std::vector<uint32_t> vertexIndices;
            if (ev.src == 0 && ev.nIdx && ev.nIdx <= 32) {
                std::string indexList;
                const uint32_t indexBytes = ev.indexSize ? 4u : 2u;
                const uint32_t indexEndian = ev.dmaSize >> 30;
                for (uint32_t i = 0; i < ev.nIdx; ++i) {
                    const uint32_t a = (ev.dmaBase + i * indexBytes) & 0x1FFFFFFFu;
                    const uint8_t* p = PhysHost(a & 0x1FFFFFFCu, 4);
                    if (!p) break;
                    uint32_t word; std::memcpy(&word, p, sizeof(word));
                    word = GpuSwap32(word, indexEndian);
                    const uint32_t index = ev.indexSize ? word : ((a & 2u) ? (word >> 16) : (word & 0xFFFFu));
                    vertexIndices.push_back(index);
                    char b[24]; std::snprintf(b, sizeof(b), "%s%u", i ? "," : "", index); indexList += b;
                }
                ImportLog::Get().Printf("DBG IDX %016llX draw=%llu src=%u n=%u indexSize=%u dmaBase=%08X dmaSize=%08X indexes=%s",
                                        (unsigned long long)ps->hash, (unsigned long long)drawSerial, ev.src, ev.nIdx,
                                        ev.indexSize, ev.dmaBase, ev.dmaSize, indexList.c_str());
            }
            if (vs && !vertexIndices.empty()) {
                for (const VFetchUse& vf : vs->vfetch) {
                    if (vf.constIdx != 95) continue;
                    const uint32_t d0 = RV(0x4800 + (vf.constIdx / 3) * 6 + (vf.constIdx % 3) * 2);
                    const uint32_t d1 = RV(0x4800 + (vf.constIdx / 3) * 6 + (vf.constIdx % 3) * 2 + 1);
                    const uint32_t strideBytes = vf.stride * 4;
                    std::set<uint32_t> seen;
                    for (uint32_t index : vertexIndices) {
                        if (!seen.insert(index).second || seen.size() > 4) continue;
                        const uint32_t address = ((d0 & 0xFFFFFFFCu) + index * strideBytes) & 0x1FFFFFFFu;
                        const uint8_t* p = PhysHost(address, strideBytes);
                        if (!p) continue;
                        std::string raw;
                        for (uint32_t i = 0; i < strideBytes; ++i) {
                            char b[4]; std::snprintf(b, sizeof(b), "%s%02X", i ? " " : "", p[i]); raw += b;
                        }
                        float pos[3]{};
                        for (uint32_t c = 0; c < 3; ++c) {
                            uint32_t word; std::memcpy(&word, p + c * 4, sizeof(word));
                            word = GpuSwap32(word, d1 & 3u); std::memcpy(&pos[c], &word, sizeof(word));
                        }
                        uint32_t uvWord; std::memcpy(&uvWord, p + 16, sizeof(uvWord)); uvWord = GpuSwap32(uvWord, d1 & 3u);
                        const float u = HalfToFloat((uint16_t)(uvWord >> 16)), v = HalfToFloat((uint16_t)uvWord);
                        ImportLog::Get().Printf("DBG VTX %016llX draw=%llu fetch=95 index=%u address=%08X endian=%u stride=%u pos=(%g,%g,%g) uv=(%g,%g) raw=%s",
                                                (unsigned long long)ps->hash, (unsigned long long)drawSerial, index, address,
                                                d1 & 3u, vf.stride, pos[0], pos[1], pos[2], u, v, raw.c_str());
                    }
                    break;
                }
            }
        }
    }
    if (rt::gpu::Cfg().binkTrace && ps && (ps->hash >> 32) == 0x7D955510ull) {   // M0: hash de los planos Y/U/V del video Bink (tiled k_8, mosaicos de 32x32)
        static std::set<uint64_t> sy, su, sv;
        static uint64_t n = 0;
        uint64_t hh[3] = {0, 0, 0}; uint32_t ww[3] = {}, hs[3] = {}; uint32_t b0[3] = {}; bool okp = true;
        for (uint32_t t = 0; t < 3; ++t) {
            uint32_t d1 = RV(0x4800 + t * 6 + 1), d2 = RV(0x4800 + t * 6 + 2);
            ww[t] = (d2 & 0x1FFF) + 1; hs[t] = ((d2 >> 13) & 0x1FFF) + 1;
            uint32_t bytes = ((ww[t] + 31) & ~31u) * ((hs[t] + 31) & ~31u);
            const uint8_t* m = PhysHost(d1 & 0xFFFFF000, bytes);
            if (!m) { okp = false; continue; }
            uint64_t h = 1469598103934665603ull;
            for (uint32_t i = 0; i < bytes; ++i) { h ^= m[i]; h *= 1099511628211ull; }
            hh[t] = h; b0[t] = m[0];
        }
        ++n;
        sy.insert(hh[0]); su.insert(hh[1]); sv.insert(hh[2]);
        if (n <= 400 || n % 50 == 0)
            ImportLog::Get().Printf("BINKPLANE frame=%llu draw=%llu hashY=%016llX hashU=%016llX hashV=%016llX dim=%ux%u,%ux%u,%ux%u mem=%d y0=%02X u0=%02X v0=%02X distinct(Y,U,V)=%zu,%zu,%zu n=%llu",
                (unsigned long long)rt::gpu::FrameCount(), (unsigned long long)drawSerial, (unsigned long long)hh[0], (unsigned long long)hh[1], (unsigned long long)hh[2],
                ww[0], hs[0], ww[1], hs[1], ww[2], hs[2], (int)okp, b0[0], b0[1], b0[2], sy.size(), su.size(), sv.size(), (unsigned long long)n);
    }
    // ---- descriptores: texturas y muestreadores ----
    // M8: las texturas se obtienen antes de reservar descriptores: GetTexture puede vaciar la lista (FlushAndWait reinicia gpuSrvNext)
    uint32_t texMask = vs->tfetchMask | (ps ? ps->tfetchMask : 0);
    HostTexture* texs[32] = {};
    debugDumpTextureBase = 0; debugDumpTextureFrame = 0; debugDumpTextureDraw = 0;
    if (const char* dbgPs = std::getenv("XE_DBG_PS")) {
        if (const char* dbgBase = std::getenv("XE_DBG_TEXTURE_BASE")) {
            const uint64_t debugFrame = rt::gpu::Cfg().stateLogFrame;
            if (ps && ps->hash == std::strtoull(dbgPs, nullptr, 16) && (!debugFrame || rt::gpu::FrameCount() + 1 == debugFrame)) {
                debugDumpTextureBase = (uint32_t)std::strtoul(dbgBase, nullptr, 16);
                debugDumpTextureFrame = rt::gpu::FrameCount() + 1;
                debugDumpTextureDraw = drawSerial;
            }
        }
    }
    for (uint32_t i = 0; i < 32; ++i) {
        if (texMask & (1u << i)) texs[i] = GetTexture(i, (vs->tfetchMask & (1u << i)) != 0 && !(ps && (ps->tfetchMask & (1u << i))));
    }
    debugDumpTextureBase = 0; debugDumpTextureFrame = 0; debugDumpTextureDraw = 0;
    uint32_t base = AllocGpuSrv(96);
    D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = cpuSrvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE dstStart = gpuSrvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = gpuSrvHeap->GetGPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE src[96];
    for (uint32_t i = 0; i < 32; ++i) {
        for (uint32_t d = 0; d < 3; ++d) {
            uint32_t idx = d == 0 ? nullSrv2D : d == 1 ? nullSrv3D : nullSrvCube;
            if (texs[i] && texs[i]->dim == d) idx = texs[i]->cpuSrv;
            src[d * 32 + i] = {cpuStart.ptr + (SIZE_T)idx * srvInc};
        }
    }
    {
        D3D12_CPU_DESCRIPTOR_HANDLE dst = {dstStart.ptr + (SIZE_T)base * srvInc};
        UINT dstCount = 96, srcCounts[96];
        for (int i = 0; i < 96; ++i) srcCounts[i] = 1;
        dev->CopyDescriptors(1, &dst, &dstCount, 96, src, srcCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
    // muestreadores (tabla de 32, con cache por contenido)
    D3D12_SAMPLER_DESC smp[32];
    for (uint32_t i = 0; i < 32; ++i) smp[i] = SamplerFromFetch(i);
    uint32_t smpBase = 0;
    uint64_t shash = FastHash(smp, sizeof(smp));
    bool smpFound = false;
    {
        auto range = samplerFast.equal_range(shash);
        for (auto r = range.first; r != range.second; ++r)
            if (!std::memcmp(r->second.desc.data(), smp, sizeof(smp))) { smpBase = r->second.base; smpFound = true; break; }
    }
    std::string skey;
    if (!smpFound) {
        skey.assign((const char*)smp, sizeof(smp));
        auto sit = samplerTables.find(skey);
        if (sit != samplerTables.end()) { smpBase = sit->second; smpFound = true; samplerFast.emplace(shash, SmpFast{skey, smpBase}); }
    }
    if (!smpFound) {
        if (gpuSmpNext + 32 > kSamplerHeapSize) { FlushAndWait(); }
        smpBase = gpuSmpNext; gpuSmpNext += 32;
        D3D12_CPU_DESCRIPTOR_HANDLE h = gpuSmpHeap->GetCPUDescriptorHandleForHeapStart();
        for (uint32_t i = 0; i < 32; ++i) { D3D12_CPU_DESCRIPTOR_HANDLE hh = {h.ptr + (SIZE_T)(smpBase + i) * smpInc}; dev->CreateSampler(&smp[i], hh); }
        samplerTables[skey] = smpBase;
        samplerFast.emplace(shash, SmpFast{skey, smpBase});
    }
    // ---- estado y draw ----
    for (uint32_t i = 0; i < nrt; ++i) {
        Transition(cRT[i]->res.Get(), cRT[i]->state, D3D12_RESOURCE_STATE_RENDER_TARGET); cRT[i]->state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        cRT[i]->writes++;
    }
    if (dRT) { Transition(dRT->res.Get(), dRT->state, D3D12_RESOURCE_STATE_DEPTH_WRITE); dRT->state = D3D12_RESOURCE_STATE_DEPTH_WRITE; dRT->writes++; }
    D3D12_CPU_DESCRIPTOR_HANDLE rtv[4] = {};
    for (uint32_t i = 0; i < nrt; ++i) { rtv[i] = rtvHeap->GetCPUDescriptorHandleForHeapStart(); rtv[i].ptr += (SIZE_T)cRT[i]->viewIdx * rtvInc; }
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
    if (dRT) { dsv = dsvHeap->GetCPUDescriptorHandleForHeapStart(); dsv.ptr += (SIZE_T)dRT->viewIdx * dsvInc; }
    cl->OMSetRenderTargets(nrt, nrt ? rtv : nullptr, FALSE, dRT ? &dsv : nullptr);
    cl->SetPipelineState(pso);
    cl->SetGraphicsRootSignature(rootSig.Get());
    cl->SetGraphicsRootConstantBufferView(0, cb.gpu);
    cl->SetGraphicsRootShaderResourceView(1, sharedMem->GetGPUVirtualAddress());
    D3D12_GPU_DESCRIPTOR_HANDLE g1 = {gpuStart.ptr + (UINT64)base * srvInc};
    cl->SetGraphicsRootDescriptorTable(2, g1);
    D3D12_GPU_DESCRIPTOR_HANDLE g2 = gpuSmpHeap->GetGPUDescriptorHandleForHeapStart();
    g2.ptr += (UINT64)smpBase * smpInc;
    cl->SetGraphicsRootDescriptorTable(3, g2);
    cl->IASetPrimitiveTopology(topo);
    D3D12_VIEWPORT v = {(float)vp.xy_offset[0], (float)vp.xy_offset[1], (float)vp.xy_extent[0], (float)vp.xy_extent[1], vp.z_min, vp.z_max};
    cl->RSSetViewports(1, &v);
    uint32_t rtw = cRT[0] ? cRT[0]->w : (dRT ? dRT->w : surf.surface_pitch), rth = cRT[0] ? cRT[0]->h : (dRT ? dRT->h : 1024);
    D3D12_RECT r = {sc[0], sc[1], std::min<int32_t>(sc[2], (int32_t)rtw), std::min<int32_t>(sc[3], (int32_t)rth)};
    if (r.right <= r.left || r.bottom <= r.top) return true;     // scissor vacio
    const bool rasterLog = rt::gpu::Cfg().stateLog;
    const uint64_t stateFrame = rasterLog ? rt::gpu::FrameCount() + 1 : 0;
    if (rasterLog && (!rt::gpu::Cfg().stateLogFrame || stateFrame == rt::gpu::Cfg().stateLogFrame)) {
        char b[1024];
        const HostRT* c0 = cRT[0];
        std::snprintf(b, sizeof(b),
                      "DRAWRASTER frame=%llu draw=%llu raw(surface=%08X color=%08X depth=%08X depthctl=%08X clip=%08X scmode=%08X vte=%08X "
                      "win=(%08X,%08X,%08X) screen=(%08X,%08X) vreg=(%08X,%08X,%08X,%08X,%08X,%08X)) "
                      "scissor=(%d,%d)-(%d,%d) host=(%ld,%ld)-(%ld,%ld) viewport=(%u,%u %ux%u z=%.6g..%.6g) "
                      "rt=%08X pitch=%u fmt=%u %ux%u msaa=%u dsv=%08X fmt=%u pitch=%u %ux%u",
                      (unsigned long long)stateFrame, (unsigned long long)drawSerial,
                      RV(0x2000), RV(0x2001), RV(0x2002), RV(0x2200), RV(0x2204), RV(0x2205), RV(0x2206),
                      RV(0x2080), RV(0x2081), RV(0x2082), RV(0x200E), RV(0x200F),
                      RV(0x210F), RV(0x2110), RV(0x2111), RV(0x2112), RV(0x2113), RV(0x2114),
                      sc[0], sc[1], sc[2], sc[3], r.left, r.top, r.right, r.bottom,
                      vp.xy_offset[0], vp.xy_offset[1], vp.xy_extent[0], vp.xy_extent[1], vp.z_min, vp.z_max,
                      c0 ? (RV(0x2001) & 0xFFFu) : 0u, surf.surface_pitch, c0 ? c0->xenosFormat : 0u,
                      c0 ? c0->w : 0u, c0 ? c0->h : 0u, (uint32_t)surf.msaa_samples,
                      RV(0x2002), dRT ? dRT->xenosFormat : 0u, surf.surface_pitch, dRT ? dRT->w : 0u, dRT ? dRT->h : 0u);
        ImportLog::Get().Printf("D3D12 %s", b);
    }
    cl->RSSetScissorRects(1, &r);
    float bf[4] = {RF(0x2105), RF(0x2106), RF(0x2107), RF(0x2108)};
    cl->OMSetBlendFactor(bf);
    cl->OMSetStencilRef(R<reg::RB_STENCILREFMASK>().stencilref);
    cl->DrawInstanced(count, 1, 0, 0);
    if (const char* stopDbg = std::getenv("XE_DBG_STOP_AFTER_PS")) {
        const uint64_t debugFrame = rt::gpu::Cfg().stateLogFrame;
        if (ps && ps->hash == std::strtoull(stopDbg, nullptr, 16) && (!debugFrame || rt::gpu::FrameCount() + 1 == debugFrame)) {
            g_debugStopFrame.store(rt::gpu::FrameCount() + 1, std::memory_order_relaxed);
            ImportLog::Get().Printf("DBG STOP_AFTER_PS hash=%016llX draw=%llu frame=%llu",
                                    (unsigned long long)ps->hash, (unsigned long long)drawSerial,
                                    (unsigned long long)(rt::gpu::FrameCount() + 1));
        }
    }
    return true;
}

} // namespace rt::gpu::d3d
