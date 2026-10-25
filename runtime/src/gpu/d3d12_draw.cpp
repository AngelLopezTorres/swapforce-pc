#include "fast_hash.h"
#include "perf.h"
// H3: draws. Estado de D3D12 derivado de los registros del Xenos (port reducido de pipeline_cache.cc y draw_util.cc de Xenia,
// copias en runtime\ref\). Simplificaciones registradas con Note("SIMPLIFY ...").
#include <algorithm>
#include <cmath>
#include <limits>

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
};

static D3D12_COMPARISON_FUNC DepthFuncToHost(uint32_t guestFunc) {
    // Xenia mapea CompareFunction directamente a D3D12; el viewport conserva
    // los valores Z del invitado aunque el NDC se invierta para ordenar el rango.
    return (D3D12_COMPARISON_FUNC)((uint32_t)D3D12_COMPARISON_FUNC_NEVER + guestFunc);
}

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
    static const bool dbgNoDepth = std::getenv("XE_DBG_NODEPTH") != nullptr, dbgNoCull = std::getenv("XE_DBG_NOCULL") != nullptr;   // experimentos de diagnostico
    if (dbgNoDepth) { depthOn = false; stencilOn = false; }
    if (dbgNoCull) d.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    if (depthOn) {
        d.DepthStencilState.DepthEnable = TRUE;
        d.DepthStencilState.DepthWriteMask = depth.z_write_enable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        d.DepthStencilState.DepthFunc = DepthFuncToHost((uint32_t)depth.zfunc);
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
    // Cache local, versionada por traductor y esquema del pipeline. El controlador
    // valida el blob; cualquier incompatibilidad vuelve a la creacion normal.
    // Experimental hasta completar la aceptacion del juego; activar con
    // XE_PSO_DISK_CACHE=1. Incrementar v1 al cambiar la raiz o la generacion del PSO.
    static const bool diskPsoCache = [] { const char* v = std::getenv("XE_PSO_DISK_CACHE"); return v && std::strcmp(v, "1") == 0; }();
    const std::string psoDir = GetPaths().userdataDir + "\\shaders\\pso";
    if (diskPsoCache) CreateDirectoryA(psoDir.c_str(), nullptr);
    char psoName[192];
    std::snprintf(psoName, sizeof(psoName), "\\%s_v1_%016llX.pso", kTranslatorVersion, (unsigned long long)hkey);
    const std::string psoPath = psoDir + psoName;
    std::string cachedPso;
    const bool cacheRead = diskPsoCache && ReadAll(psoPath, cachedPso) && cachedPso.size() > key.size() &&
                           !std::memcmp(cachedPso.data(), key.data(), key.size());
    if (cacheRead) d.CachedPSO = {cachedPso.data() + key.size(), cachedPso.size() - key.size()};
    const uint64_t psoStart = perf::g_enabled ? perf::Now() : 0;
    HRESULT hr = dev->CreateGraphicsPipelineState(&d, IID_PPV_ARGS(&pso));
    bool cacheHit = cacheRead && SUCCEEDED(hr);
    if (FAILED(hr) && cacheRead) {
        ImportLog::Get().Printf("D3D12 PSO_CACHE_REJECT key=%016llX hr=%08X", (unsigned long long)hkey, (unsigned)hr);
        d.CachedPSO = {};
        pso.Reset();
        hr = dev->CreateGraphicsPipelineState(&d, IID_PPV_ARGS(&pso));
    }
    if (perf::g_enabled) {
        LARGE_INTEGER f; QueryPerformanceFrequency(&f);
        ImportLog::Get().Printf("PERF PSO_CREATE key=%016llX cached=%d ok=%d ms=%.3f", (unsigned long long)hkey,
                               cacheHit ? 1 : 0, SUCCEEDED(hr) ? 1 : 0, (perf::Now() - psoStart) * 1000.0 / f.QuadPart);
    }
    if (FAILED(hr)) {
        ++psoFail; ++fPsoFail;
        ImportLog::Get().Printf("D3D12 PSO_FAIL hr=0x%08X vs=%016llX ps=%016llX nrt=%u rt0=%u dsv=%u topo=%u rectgs=%d", (unsigned)hr, (unsigned long long)st.vs,
                                (unsigned long long)st.ps, d.NumRenderTargets, st.rtFmt[0], st.dsvFmt, topoClass, (int)rectGs);
        psoCache[key] = nullptr;
        return nullptr;
    }
    if (diskPsoCache && !cacheHit) {
        ComPtr<ID3DBlob> blob;
        if (SUCCEEDED(pso->GetCachedBlob(&blob)))
            WriteAll(psoPath, key + std::string((const char*)blob->GetBufferPointer(), blob->GetBufferSize()));
    }
    ID3D12PipelineState* raw = pso.Get();
    psoCache[key] = pso;
    return raw;
}

// ---- viewport (port de draw_util::GetHostViewportInfo con escala de resolucion 1, sin float24) ----
struct ViewportInfo { uint32_t xy_offset[2], xy_extent[2]; float z_min, z_max, ndc_scale[3], ndc_offset[3]; };

static void GetViewport(uint32_t xyMaxX, uint32_t xyMaxY, ViewportInfo& v, bool pixelShaderWritesDepth) {
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
            uint32_t xyMax = i ? xyMaxY : xyMaxX;
            uint32_t ext = std::min(8192u, std::max(1u, xyMax));
            v.xy_extent[i] = ext;
            float e = (float)ext, p2n = 2.0f / e;
            ns[i] = scale_xy[i] * p2n;
            no[i] = (off_xy[i] - e * 0.5f + add_xy[i]) * p2n;
        }
        z_min = 0.0f; z_max = 1.0f; ns[2] = scale_z; no[2] = off_z;
    } else {
        for (int i = 0; i < 2; ++i) {
            float off = off_xy[i] + add_xy[i], sc = scale_xy[i], sa = std::fabs(sc);
            float mx = (float)(i ? xyMaxY : xyMaxX);
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

static bool DumpDepthSnapshot(D3D12Backend& backend, HostRT& depth, uint32_t base, uint32_t pitch,
                              uint64_t frame, uint64_t draw, uint32_t zfunc, const char* stage) {
    const D3D12_RESOURCE_DESC desc = depth.res->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp = {};
    UINT rows = 0;
    UINT64 rowBytes = 0, totalBytes = 0;
    backend.dev->GetCopyableFootprints(&desc, 0, 1, 0, &fp, &rows, &rowBytes, &totalBytes);
    const DXGI_FORMAT format = fp.Footprint.Format;
    const bool f32 = format == DXGI_FORMAT_R32_TYPELESS || format == DXGI_FORMAT_R32_FLOAT;
    const bool d24 = format == DXGI_FORMAT_R24G8_TYPELESS || format == DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    if ((!f32 && !d24) || rows != desc.Height || totalBytes == 0 || totalBytes > (128ull << 20)) {
        ImportLog::Get().Printf("D3D12 DEPTH_SNAPSHOT frame=%llu draw=%llu stage=%s base=%u pitch=%u fmt=%u footprint=%u rows=%u bytes=%llu -> NO SOPORTADO",
                                (unsigned long long)frame, (unsigned long long)draw, stage, base, pitch,
                                depth.xenosFormat, (unsigned)format, rows, (unsigned long long)totalBytes);
        return false;
    }
    if (!backend.depthReadback || backend.depthReadbackBytes < totalBytes) {
        D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_READBACK};
        D3D12_RESOURCE_DESC bd = {};
        bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; bd.Width = totalBytes;
        bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
        bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        backend.depthReadback.Reset();
        backend.depthReadbackBytes = totalBytes;
        if (!backend.Hr(backend.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST,
                                                              nullptr, IID_PPV_ARGS(&backend.depthReadback)),
                        "CreateCommittedResource(snapshot profundidad)")) return false;
    }

    const D3D12_RESOURCE_STATES oldState = depth.state;
    backend.Transition(depth.res.Get(), oldState, D3D12_RESOURCE_STATE_COPY_SOURCE);
    depth.state = D3D12_RESOURCE_STATE_COPY_SOURCE;
    D3D12_TEXTURE_COPY_LOCATION from = {}, to = {};
    from.pResource = depth.res.Get(); from.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; from.SubresourceIndex = 0;
    to.pResource = backend.depthReadback.Get(); to.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT; to.PlacedFootprint = fp;
    backend.cl->CopyTextureRegion(&to, 0, 0, 0, &from, nullptr);
    backend.Transition(depth.res.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, oldState);
    depth.state = oldState;
    backend.FlushAndWait();

    void* mapped = nullptr;
    D3D12_RANGE readRange = {0, (SIZE_T)totalBytes};
    if (!backend.Hr(backend.depthReadback->Map(0, &readRange, &mapped), "Map(snapshot profundidad)")) return false;

    char name[160];
    std::snprintf(name, sizeof(name), "\\depth_f%05llu_draw%llu_base%03X_%s.f32",
                  (unsigned long long)frame, (unsigned long long)draw, base, stage);
    const std::string path = rt::gpu::FramesDir() + name;
    FILE* dump = std::fopen(path.c_str(), "wb");
    std::vector<float> row(desc.Width);
    double sum = 0.0;
    float minValue = std::numeric_limits<float>::infinity();
    float maxValue = -std::numeric_limits<float>::infinity();
    uint64_t finite = 0, nonFinite = 0, zero = 0, one = 0, belowPoint2 = 0, atPoint2 = 0;
    uint64_t bins[8] = {};
    for (uint32_t y = 0; y < desc.Height; ++y) {
        const uint8_t* src = static_cast<const uint8_t*>(mapped) + (size_t)y * fp.Footprint.RowPitch;
        for (uint32_t x = 0; x < desc.Width; ++x) {
            float value;
            if (f32) std::memcpy(&value, src + (size_t)x * 4, sizeof(value));
            else {
                uint32_t bits;
                std::memcpy(&bits, src + (size_t)x * 4, sizeof(bits));
                value = (float)(bits & 0x00FFFFFFu) * (1.0f / 16777215.0f);
            }
            row[x] = value;
            if (!std::isfinite(value)) { ++nonFinite; continue; }
            ++finite; sum += value;
            minValue = std::min(minValue, value); maxValue = std::max(maxValue, value);
            if (value <= 1.0e-7f) ++zero;
            if (value >= 0.999999f) ++one;
            if (value < 0.2f) ++belowPoint2;
            if (std::fabs(value - 0.2f) < 1.0e-5f) ++atPoint2;
            const int bin = value < 0.0f ? 0 : value >= 1.0f ? 7 : std::min(6, (int)(value * 7.0f));
            ++bins[bin];
        }
        if (dump && std::fwrite(row.data(), sizeof(float), row.size(), dump) != row.size()) {
            std::fclose(dump); dump = nullptr;
        }
    }
    if (dump) std::fclose(dump);
    D3D12_RANGE writeRange = {0, 0};
    backend.depthReadback->Unmap(0, &writeRange);
    ImportLog::Get().Printf("D3D12 DEPTH_SNAPSHOT frame=%llu draw=%llu stage=%s base=%u pitch=%u fmt=%u zfunc=%u host=%ux%u footprint=%u row=%u valores=%llu no_finitos=%llu min=%.9g max=%.9g media=%.9g cero=%llu uno=%llu lt02=%llu eq02=%llu bins=%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu dump=%s",
                            (unsigned long long)frame, (unsigned long long)draw, stage,
                            base, pitch, depth.xenosFormat, zfunc,
                            (uint32_t)desc.Width, (uint32_t)desc.Height, (unsigned)format, fp.Footprint.RowPitch,
                            (unsigned long long)finite, (unsigned long long)nonFinite,
                            finite ? minValue : 0.0f, finite ? maxValue : 0.0f, finite ? sum / finite : 0.0,
                            (unsigned long long)zero, (unsigned long long)one, (unsigned long long)belowPoint2,
                            (unsigned long long)atPoint2,
                            (unsigned long long)bins[0], (unsigned long long)bins[1], (unsigned long long)bins[2], (unsigned long long)bins[3],
                            (unsigned long long)bins[4], (unsigned long long)bins[5], (unsigned long long)bins[6], (unsigned long long)bins[7],
                            dump ? path.c_str() : "FALLO");
    return dump != nullptr;
}

void D3D12Backend::FinishDepthQueries(uint64_t frame) {
    if (depthQueryFrame != frame || depthQueryRecords.empty() || !depthQueryHeap || !depthQueryReadback) return;
    const uint32_t count = (uint32_t)depthQueryRecords.size();
    if (!clOpen) BeginList();
    if (!clOpen) return;
    cl->ResolveQueryData(depthQueryHeap.Get(), D3D12_QUERY_TYPE_OCCLUSION, 0, count,
                         depthQueryReadback.Get(), 0);
    FlushAndWait();

    void* mapped = nullptr;
    D3D12_RANGE readRange = {0, (SIZE_T)count * sizeof(uint64_t)};
    if (!Hr(depthQueryReadback->Map(0, &readRange, &mapped), "Map(occlusion por draw)")) {
        ImportLog::Get().Printf("D3D12 OCCLUSION_SUMMARY frame=%llu draws=%u map=FALLO",
                                (unsigned long long)frame, count);
        depthQueryRecords.clear(); depthQueryFrame = 0;
        return;
    }
    const uint64_t* samples = static_cast<const uint64_t*>(mapped);
    uint64_t zero = 0, nonzero = 0, total = 0;
    for (uint32_t i = 0; i < count; ++i) {
        const DepthQueryRecord& q = depthQueryRecords[i];
        const uint64_t n = samples[i];
        if (!n) ++zero; else ++nonzero;
        total += n;
        const double ratio = q.scissorBound ? (double)n / (double)q.scissorBound : 0.0;
        ImportLog::Get().Printf("D3D12 OCCLUSION frame=%llu draw=%llu src=%llu prim=%u zfunc=%u host_zfunc=%u zen=%u zwr=%u dsv_fmt=%u dsv_dxgi=%u dsv=%p gen=%llu dsv_dims=%ux%u rtv=%p rtv_dims=%ux%u targets_compatible=%u viewport=(%u,%u %ux%u) viewport_in_dsv=%u scissor=(%d,%d)-(%d,%d) scissor_in_dsv=%u viewport_z=%.7g..%.7g ndc_z=(%.7g,%.7g) depthclip=%u ndc_reversed=%u psdepth=%u indices=%u issued=%u samples=%llu scissor_bound=%llu samples_per_bound=%.9g prior_clear=%llu clear_at=%llu/%llu prior_write_draws=%llu write_at=%llu/%llu",
                                (unsigned long long)q.frame, (unsigned long long)q.draw,
                                (unsigned long long)q.source,
                                q.primitive, q.zfunc, q.hostZfunc, q.zEnable, q.zWrite, q.dsvFormat,
                                q.dsvDxgiFormat,
                                (void*)(uintptr_t)q.dsvPtr, (unsigned long long)q.dsvGeneration,
                                q.dsvWidth, q.dsvHeight, (void*)(uintptr_t)q.rtvPtr,
                                q.rtvWidth, q.rtvHeight, q.targetsCompatible,
                                q.viewportX, q.viewportY, q.viewportWidth, q.viewportHeight,
                                q.viewportInDsv, q.scissor[0], q.scissor[1], q.scissor[2], q.scissor[3],
                                q.scissorInDsv, q.zMin, q.zMax, q.ndcScaleZ, q.ndcOffsetZ,
                                q.depthClip, q.ndcZReversed, q.writesDepth, q.indices, q.issuedIndices,
                                (unsigned long long)n, (unsigned long long)q.scissorBound, ratio,
                                (unsigned long long)q.priorClearCount,
                                (unsigned long long)q.lastClearFrame, (unsigned long long)q.lastClearDraw,
                                (unsigned long long)q.priorDepthWriteCount,
                                (unsigned long long)q.lastDepthWriteFrame, (unsigned long long)q.lastDepthWriteDraw);
    }
    D3D12_RANGE writeRange = {0, 0};
    depthQueryReadback->Unmap(0, &writeRange);
    ImportLog::Get().Printf("D3D12 OCCLUSION_SUMMARY frame=%llu draws=%u zero=%llu nonzero=%llu samples=%llu",
                            (unsigned long long)frame, count, (unsigned long long)zero,
                            (unsigned long long)nonzero, (unsigned long long)total);
    depthQueryRecords.clear();
    depthQueryFrame = 0;
}

static uint32_t VertexFormatBytes(uint32_t format) {
    switch ((xenos::VertexFormat)format) {
        case xenos::VertexFormat::k_8_8_8_8:
        case xenos::VertexFormat::k_2_10_10_10:
        case xenos::VertexFormat::k_10_11_11:
        case xenos::VertexFormat::k_11_11_10:
        case xenos::VertexFormat::k_16_16:
        case xenos::VertexFormat::k_16_16_FLOAT:
        case xenos::VertexFormat::k_32:
        case xenos::VertexFormat::k_32_FLOAT:
            return 4;
        case xenos::VertexFormat::k_16_16_16_16:
        case xenos::VertexFormat::k_16_16_16_16_FLOAT:
        case xenos::VertexFormat::k_32_32:
        case xenos::VertexFormat::k_32_32_FLOAT:
            return 8;
        case xenos::VertexFormat::k_32_32_32_FLOAT:
            return 12;
        case xenos::VertexFormat::k_32_32_32_32:
        case xenos::VertexFormat::k_32_32_32_32_FLOAT:
            return 16;
        default:
            return 0;
    }
}

static void DumpGuestVertexInputs(const ShaderInfo& vs, uint64_t frame, uint64_t draw,
                                 uint32_t vte, uint32_t drawSource, uint32_t indexOffset,
                                 uint32_t vertexCount) {
    ImportLog::Get().Printf("D3D12 GUEST_VS_INPUT frame=%llu draw=%llu shader=%016llX dwords=%u vte=%08X draw_src=%u vgt_index_offset=%u vfetches=%u float_dynamic=%u",
                            (unsigned long long)frame, (unsigned long long)draw,
                            (unsigned long long)vs.hash, vs.dwords, vte, drawSource, indexOffset,
                            (uint32_t)vs.vfetch.size(), vs.floatDynamic ? 1u : 0u);
    for (const VFetchUse& vf : vs.vfetch) {
        const uint32_t reg = 0x4800 + (vf.constIdx / 3) * 6 + (vf.constIdx % 3) * 2;
        const uint32_t d0 = RegRead(reg), d1 = RegRead(reg + 1);
        const uint32_t base = d0 & 0xFFFFFFFCu;
        const uint32_t sizeBytes = ((d1 >> 2) & 0xFFFFFFu) * 4u;
        char comp = "xyzw"[vf.srcSwizzle & 3u];
        ImportLog::Get().Printf("D3D12 GUEST_VFETCH frame=%llu draw=%llu shader=%016llX const=%u mini=%u src=r%u.%c src_rel=%u rounded=%u dest=r%u swizzle=%03X fmt=%u signed=%u normalized=%u offset_dw=%d stride_dw=%u descriptor=(%08X,%08X) base=%08X size=%u endian=%u",
                                (unsigned long long)frame, (unsigned long long)draw,
                                (unsigned long long)vs.hash, vf.constIdx, vf.mini ? 1u : 0u,
                                vf.src, comp, vf.srcRelative ? 1u : 0u, vf.indexRounded ? 1u : 0u,
                                vf.dest, vf.destSwizzle, vf.format, vf.isSigned ? 1u : 0u,
                                vf.normalized ? 1u : 0u, vf.offset, vf.stride, d0, d1,
                                base, sizeBytes, d1 & 3u);
        const uint32_t readBytes = VertexFormatBytes(vf.format);
        for (uint32_t vertexId = 0; vertexId < vertexCount; ++vertexId) {
            const uint32_t fetchIndex = vertexId + indexOffset;
            const int64_t relBytes = ((int64_t)fetchIndex * vf.stride + vf.offset) * 4;
            const int64_t guestAddress64 = (int64_t)base + relBytes;
            if (!readBytes || guestAddress64 < 0 || guestAddress64 > 0x1FFFFFFFu || relBytes < 0 ||
                (sizeBytes && (uint64_t)relBytes + readBytes > sizeBytes)) {
                ImportLog::Get().Printf("D3D12 GUEST_VTX frame=%llu draw=%llu const=%u vertex_id=%u fetch_index=%u draw_src=%u address=OUT_OF_DESCRIPTOR rel_bytes=%lld read_bytes=%u",
                                        (unsigned long long)frame, (unsigned long long)draw,
                                        vf.constIdx, vertexId, fetchIndex, drawSource,
                                        (long long)relBytes, readBytes);
                continue;
            }
            const uint32_t address = (uint32_t)guestAddress64 & 0x1FFFFFFFu;
            const uint8_t* mem = PhysHost(address, readBytes);
            if (!mem) {
                ImportLog::Get().Printf("D3D12 GUEST_VTX frame=%llu draw=%llu const=%u vertex_id=%u fetch_index=%u draw_src=%u address=%08X read_bytes=%u PhysHost=NULL",
                                        (unsigned long long)frame, (unsigned long long)draw,
                                        vf.constIdx, vertexId, fetchIndex, drawSource, address, readBytes);
                continue;
            }
            std::string raw, swapped;
            float pos[3] = {};
            for (uint32_t b = 0; b < readBytes; ++b) {
                char token[4]; std::snprintf(token, sizeof(token), "%s%02X", b ? " " : "", mem[b]); raw += token;
            }
            for (uint32_t wordIndex = 0; wordIndex < readBytes / 4; ++wordIndex) {
                uint32_t word; std::memcpy(&word, mem + wordIndex * 4, 4);
                word = GpuSwap32(word, d1 & 3u);
                char token[20]; std::snprintf(token, sizeof(token), "%s%08X", wordIndex ? " " : "", word); swapped += token;
                if (wordIndex < 3) std::memcpy(&pos[wordIndex], &word, 4);
            }
            const bool xyzFloat = vf.format == (uint32_t)xenos::VertexFormat::k_32_32_32_FLOAT;
            ImportLog::Get().Printf("D3D12 GUEST_VTX frame=%llu draw=%llu shader=%016llX const=%u vertex_id=%u fetch_index=%u draw_src=%u address=%08X endian=%u bytes=%u raw=[%s] swapped_words=[%s]%s",
                                    (unsigned long long)frame, (unsigned long long)draw,
                                    (unsigned long long)vs.hash, vf.constIdx, vertexId, fetchIndex, drawSource, address,
                                    d1 & 3u, readBytes, raw.c_str(), swapped.c_str(),
                                    xyzFloat ? "" : " position_decode=not_xyz_float32");
            if (xyzFloat)
                ImportLog::Get().Printf("D3D12 GUEST_VTX_POS frame=%llu draw=%llu const=%u vertex_id=%u fetch_index=%u x=%.9g y=%.9g z=%.9g",
                                        (unsigned long long)frame, (unsigned long long)draw,
                                        vf.constIdx, vertexId, fetchIndex, pos[0], pos[1], pos[2]);
        }
    }
    for (uint32_t c = 0; c < 256; ++c) {
        if (!(vs.floatBitmap[c / 64] & (1ull << (c % 64)))) continue;
        uint32_t bits[4]; float value[4];
        for (uint32_t lane = 0; lane < 4; ++lane) {
            bits[lane] = RegRead(0x4000 + c * 4 + lane);
            std::memcpy(&value[lane], &bits[lane], sizeof(float));
        }
        ImportLog::Get().Printf("D3D12 GUEST_VS_CONST frame=%llu draw=%llu shader=%016llX c%u bits=(%08X,%08X,%08X,%08X) float=(%.9g,%.9g,%.9g,%.9g)",
                                (unsigned long long)frame, (unsigned long long)draw,
                                (unsigned long long)vs.hash, c, bits[0], bits[1], bits[2], bits[3],
                                value[0], value[1], value[2], value[3]);
    }
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
    // Alias de EDRAM: el juego limpia superficies con RectList en modo MSAA (p.ej. 4X con pitch/2) y luego dibuja a 1X sobre
    // los mismos bytes de EDRAM (520x304 a 4X = 1040x608 a 1X). Sin EDRAM compartida, esa limpieza se redirige al RT 1X equivalente.
    const bool clipOff = R<reg::PA_CL_CLIP_CNTL>().clip_disable;
    const bool aliasClear = prim == 8 && clipOff && surf.msaa_samples != xenos::MsaaSamples::k1X && !std::getenv("XE_DBG_NOALIAS");
    const uint32_t aliasX = aliasClear && surf.msaa_samples == xenos::MsaaSamples::k4X ? 2u : 1u;
    const uint32_t aliasY = aliasClear ? 2u : 1u;
    const uint32_t guestNeedH = needH;
    const uint32_t surfPitch = surf.surface_pitch * aliasX;
    if (aliasClear) {
        needH = std::min(needH * aliasY, 4096u);
        sc[0] *= (int32_t)aliasX; sc[2] *= (int32_t)aliasX; sc[1] *= (int32_t)aliasY; sc[3] *= (int32_t)aliasY;
    }
    HostRT* cRT[4] = {nullptr, nullptr, nullptr, nullptr};
    uint32_t nrt = 0;
    if (colorMode) {
        for (uint32_t i = 0; i < 4; ++i) {
            if (!(ps->writesColors & (1u << i))) continue;
            auto ci = R<reg::RB_COLOR_INFO>(); ci.value = RV(i == 0 ? 0x2001 : 0x2003 + (i - 1));
            uint32_t mask = (RV(0x2104) >> (4 * i)) & 0xF;
            if (!mask) continue;
            cRT[i] = GetColorRT(ci.color_base | (ci.color_base_bit_11 << 11), surfPitch, (uint32_t)ci.color_format, needH);
            if (!cRT[i]) { Note("OMISION draw: no se pudo crear el render target de color"); return false; }
            nrt = i + 1;
        }
        // huecos (RT intermedios sin escribir): se rellenan con el RT correspondiente si existe, si no no se soporta
        for (uint32_t i = 0; i < nrt; ++i)
            if (!cRT[i]) {
                if (rt::gpu::Cfg().stateLogFrame == rt::gpu::FrameCount() + 1)
                    ImportLog::Get().Printf("RT_GAP frame=%llu draw=%llu ps=%016llX writes=%X mask=%X slot=%u",
                        (unsigned long long)(rt::gpu::FrameCount() + 1), (unsigned long long)drawSerial,
                        (unsigned long long)ps->hash, ps->writesColors, RV(0x2104), i);
                Note("OMISION draw: RT de color con huecos"); return false;
            }
    }
    auto dc = R<reg::RB_DEPTHCONTROL>();
    bool depthUsed = (dc.z_enable && !(dc.zfunc == xenos::CompareFunction::kAlways && !dc.z_write_enable)) || dc.stencil_enable;
    HostRT* dRT = nullptr;
    uint32_t depthBase = 0, depthFormat = 0;
    if (depthUsed) {
        auto di = R<reg::RB_DEPTH_INFO>();
        depthBase = di.depth_base | (di.depth_base_bit_11 << 11);
        depthFormat = (uint32_t)di.depth_format;
        dRT = GetDepthRT(depthBase, surfPitch, depthFormat, needH);
        if (!dRT) { Note("OMISION draw: no se pudo crear el render target de profundidad"); return false; }
    }
    if (dRT && rt::gpu::Cfg().stateLog && rt::gpu::Cfg().stateLogFrame &&
        rt::gpu::FrameCount() + 1 == rt::gpu::Cfg().stateLogFrame) {
        static uint64_t depthDumpFrame = 0;
        static std::set<uint64_t> depthStartDumped, depthCompareDumped;
        const uint64_t frame = rt::gpu::FrameCount() + 1;
        if (depthDumpFrame != frame) {
            depthDumpFrame = frame;
            depthStartDumped.clear();
            depthCompareDumped.clear();
        }
        const uint64_t key = (uint64_t)depthBase | ((uint64_t)surf.surface_pitch << 12) | ((uint64_t)depthFormat << 32);
        const bool relational = dc.z_enable && dc.zfunc != xenos::CompareFunction::kAlways;
        if (!depthStartDumped.count(key)) {
            DumpDepthSnapshot(*this, *dRT, depthBase, surf.surface_pitch, frame, drawSerial, (uint32_t)dc.zfunc,
                              relational ? "start_pre_compare" : "start_pre_draw");
            depthStartDumped.insert(key);
            if (relational) depthCompareDumped.insert(key);
        } else if (relational && !depthCompareDumped.count(key)) {
            DumpDepthSnapshot(*this, *dRT, depthBase, surf.surface_pitch, frame, drawSerial, (uint32_t)dc.zfunc,
                              "post_clear_pre_compare");
            depthCompareDumped.insert(key);
        }
    }
    if (colorMode && !nrt && !dRT) return true;   // nada que escribir
    for (uint32_t i = 0; i < 4; ++i) g_psoRtFmt[i] = (i < nrt && cRT[i]) ? (uint32_t)cRT[i]->format : 0;
    g_psoNrt = nrt; g_psoDsvFmt = dRT ? (uint32_t)dRT->format : 0;
    ViewportInfo vp;
    // clip_disable usa un viewport host que cubre el destino guest activo.
    // No se debe usar el limite global D3D12 como maximo guest de ambos ejes.
    GetViewport(surf.surface_pitch, aliasClear ? guestNeedH / aliasY : needH, vp, ps && ps->writesDepth);
    if (aliasClear) { vp.xy_extent[0] = surfPitch; vp.xy_extent[1] = needH; }   // NDC en coordenadas guest (4X), viewport host sobre el RT 1X
    // Solo diagnóstico: indica si el shader transforma Z con signo negativo.
    // La función de comparación Xenos se mapea directamente a D3D12.
    const bool ndcZReversed = !(ps && ps->writesDepth) && vp.ndc_scale[2] < 0.0f;
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
    if (rt::gpu::Cfg().stateLog && rt::gpu::Cfg().stateLogFrame && rt::gpu::FrameCount() + 1 == rt::gpu::Cfg().stateLogFrame) {
        auto zc = R<reg::RB_DEPTHCONTROL>();
        ImportLog::Get().Printf("D3D12 ZSTATE frame=%llu draw=%llu serial=%llu psdepth=%u pshash=%016llX ndc_reversed=%u zfunc=%u host_zfunc=%u zen=%u zwr=%u zmin=%.7g zmax=%.7g ndc_z=(%.7g,%.7g) vte_z=(%.7g,%.7g) depthclip=%u clipdis=%u dxclip=%u dsv_fmt=%u dsv_dxgi=%u dsv=%p gen=%llu clear=%llu clear_at=%llu/%llu write_draws=%llu write_at=%llu/%llu vte=%08X",
                                (unsigned long long)(rt::gpu::FrameCount() + 1),
                                (unsigned long long)ev.src, (unsigned long long)drawSerial,
                                (unsigned)(ps && ps->writesDepth), (unsigned long long)(ps ? ps->hash : 0),
                                (unsigned)ndcZReversed, (unsigned)zc.zfunc,
                                (unsigned)DepthFuncToHost((uint32_t)zc.zfunc),
                                (unsigned)zc.z_enable, (unsigned)zc.z_write_enable,
                                vp.z_min, vp.z_max, vp.ndc_scale[2], vp.ndc_offset[2],
                                RF(0x2113), RF(0x2114),
                                R<reg::PA_CL_CLIP_CNTL>().clip_disable ? 0u : 1u,
                                (unsigned)R<reg::PA_CL_CLIP_CNTL>().clip_disable,
                                (unsigned)R<reg::PA_CL_CLIP_CNTL>().dx_clip_space_def,
                                dRT ? dRT->xenosFormat : 99u, dRT ? (uint32_t)dRT->format : 0u,
                                dRT ? (void*)dRT->res.Get() : nullptr,
                                (unsigned long long)(dRT ? dRT->resourceGeneration : 0u),
                                (unsigned long long)(dRT ? dRT->clearCount : 0u),
                                (unsigned long long)(dRT ? dRT->lastClearFrame : 0u),
                                (unsigned long long)(dRT ? dRT->lastClearDraw : 0u),
                                (unsigned long long)(dRT ? dRT->depthWriteCount : 0u),
                                (unsigned long long)(dRT ? dRT->lastDepthWriteFrame : 0u),
                                (unsigned long long)(dRT ? dRT->lastDepthWriteDraw : 0u), RV(0x2206));
    }
    cl->RSSetViewports(1, &v);
    uint32_t rtw = cRT[0] ? cRT[0]->w : (dRT ? dRT->w : surf.surface_pitch), rth = cRT[0] ? cRT[0]->h : (dRT ? dRT->h : 1024);
    D3D12_RECT r = {sc[0], sc[1], std::min<int32_t>(sc[2], (int32_t)rtw), std::min<int32_t>(sc[3], (int32_t)rth)};
    if (r.right <= r.left || r.bottom <= r.top) return true;     // scissor vacio
    const bool viewportInDsv = !dRT ||
        ((uint64_t)vp.xy_offset[0] + vp.xy_extent[0] <= dRT->w &&
         (uint64_t)vp.xy_offset[1] + vp.xy_extent[1] <= dRT->h);
    const bool scissorInDsv = !dRT ||
        (r.left >= 0 && r.top >= 0 && r.right <= (int32_t)dRT->w && r.bottom <= (int32_t)dRT->h);
    bool targetsCompatible = true;
    if (dRT && nrt) {
        for (uint32_t i = 0; i < nrt; ++i)
            if (!cRT[i] || cRT[i]->w != dRT->w || cRT[i]->h != dRT->h) targetsCompatible = false;
    }
    const bool rasterLog = rt::gpu::Cfg().stateLog;
    const uint64_t stateFrame = rasterLog ? rt::gpu::FrameCount() + 1 : 0;
    if (rasterLog && (!rt::gpu::Cfg().stateLogFrame || stateFrame == rt::gpu::Cfg().stateLogFrame)) {
        char b[1536];
        const HostRT* c0 = cRT[0];
        std::snprintf(b, sizeof(b),
                      "DRAWRASTER frame=%llu draw=%llu vs=%016llX ps=%016llX prim=%u indices=%u count=%u rectgs=%u raw(surface=%08X color=%08X depth=%08X depthctl=%08X clip=%08X scmode=%08X vte=%08X "
                      "win=(%08X,%08X,%08X) screen=(%08X,%08X) vreg=(%08X,%08X,%08X,%08X,%08X,%08X)) "
                      "scissor=(%d,%d)-(%d,%d) host=(%ld,%ld)-(%ld,%ld) viewport=(%u,%u %ux%u z=%.6g..%.6g) "
                      "rt=%08X pitch=%u fmt=%u %ux%u msaa=%u rtv0=%p dsv=%08X fmt=%u dxgi=%u resource=%p gen=%llu pitch=%u %ux%u "
                      "vp_in_dsv=%u scissor_in_dsv=%u targets_compatible=%u depthclip=%u ndc_xy=(%.7g,%.7g;%.7g,%.7g) ndc_z=(%.7g,%.7g)",
                      (unsigned long long)stateFrame, (unsigned long long)drawSerial,
                      (unsigned long long)(vs ? vs->hash : 0), (unsigned long long)(ps ? ps->hash : 0),
                      prim, ev.nIdx, count, rectGs ? 1u : 0u,
                      RV(0x2000), RV(0x2001), RV(0x2002), RV(0x2200), RV(0x2204), RV(0x2205), RV(0x2206),
                      RV(0x2080), RV(0x2081), RV(0x2082), RV(0x200E), RV(0x200F),
                      RV(0x210F), RV(0x2110), RV(0x2111), RV(0x2112), RV(0x2113), RV(0x2114),
                      sc[0], sc[1], sc[2], sc[3], r.left, r.top, r.right, r.bottom,
                      vp.xy_offset[0], vp.xy_offset[1], vp.xy_extent[0], vp.xy_extent[1], vp.z_min, vp.z_max,
                      c0 ? (RV(0x2001) & 0xFFFu) : 0u, surf.surface_pitch, c0 ? c0->xenosFormat : 0u,
                      c0 ? c0->w : 0u, c0 ? c0->h : 0u, (uint32_t)surf.msaa_samples,
                      c0 ? (void*)c0->res.Get() : nullptr, RV(0x2002), dRT ? dRT->xenosFormat : 0u,
                      dRT ? (uint32_t)dRT->format : 0u, dRT ? (void*)dRT->res.Get() : nullptr,
                      (unsigned long long)(dRT ? dRT->resourceGeneration : 0u), surf.surface_pitch,
                      dRT ? dRT->w : 0u, dRT ? dRT->h : 0u,
                      viewportInDsv, scissorInDsv, targetsCompatible,
                      R<reg::PA_CL_CLIP_CNTL>().clip_disable ? 0u : 1u,
                      vp.ndc_scale[0], vp.ndc_offset[0], vp.ndc_scale[1], vp.ndc_offset[1], vp.ndc_scale[2], vp.ndc_offset[2]);
        ImportLog::Get().Printf("D3D12 %s", b);
    }
    cl->RSSetScissorRects(1, &r);
    float bf[4] = {RF(0x2105), RF(0x2106), RF(0x2107), RF(0x2108)};
    cl->OMSetBlendFactor(bf);
    cl->OMSetStencilRef(R<reg::RB_STENCILREFMASK>().stencilref);
    const uint64_t frame = rt::gpu::FrameCount() + 1;
    bool queryDraw = rt::gpu::Cfg().stateLog && rt::gpu::Cfg().stateLogFrame == frame;
    if (queryDraw && depthQueryFrame != frame) {
        if (!depthQueryRecords.empty()) {
            ImportLog::Get().Printf("D3D12 OCCLUSION abandoned_frame=%llu next_frame=%llu draws=%zu",
                                    (unsigned long long)depthQueryFrame, (unsigned long long)frame,
                                    depthQueryRecords.size());
            depthQueryRecords.clear();
        }
        depthQueryFrame = frame;
    }
    if (queryDraw && !depthQueryHeap) {
        D3D12_QUERY_HEAP_DESC qd = {};
        qd.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        qd.Count = 16384;
        if (Hr(dev->CreateQueryHeap(&qd, IID_PPV_ARGS(&depthQueryHeap)), "CreateQueryHeap(occlusion por draw)")) {
            depthQueryCapacity = qd.Count;
            D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_READBACK};
            D3D12_RESOURCE_DESC bd = {};
            bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bd.Width = (UINT64)depthQueryCapacity * sizeof(uint64_t);
            bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
            bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            if (!Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &bd,
                                                  D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                  IID_PPV_ARGS(&depthQueryReadback)),
                    "CreateCommittedResource(readback de occlusion)")) {
                depthQueryHeap.Reset(); depthQueryCapacity = 0;
            }
        }
    }
    const uint32_t queryIndex = (uint32_t)depthQueryRecords.size();
    if (queryDraw && (!depthQueryHeap || !depthQueryReadback || queryIndex >= depthQueryCapacity)) {
        static uint64_t overflowFrame = 0;
        if (overflowFrame != frame) {
            overflowFrame = frame;
            ImportLog::Get().Printf("D3D12 OCCLUSION unavailable frame=%llu heap=%u readback=%u index=%u capacity=%u",
                                    (unsigned long long)frame, depthQueryHeap ? 1u : 0u,
                                    depthQueryReadback ? 1u : 0u, queryIndex, depthQueryCapacity);
        }
        queryDraw = false;
    }
    if (queryDraw) cl->BeginQuery(depthQueryHeap.Get(), D3D12_QUERY_TYPE_OCCLUSION, queryIndex);
    cl->DrawInstanced(count, 1, 0, 0);
    if (queryDraw) {
        cl->EndQuery(depthQueryHeap.Get(), D3D12_QUERY_TYPE_OCCLUSION, queryIndex);
        DepthQueryRecord q;
        q.frame = frame; q.draw = drawSerial; q.source = ev.src; q.primitive = prim;
        q.dsvPtr = (uint64_t)(uintptr_t)(dRT ? dRT->res.Get() : nullptr);
        q.dsvGeneration = dRT ? dRT->resourceGeneration : 0;
        q.priorClearCount = dRT ? dRT->clearCount : 0;
        q.priorDepthWriteCount = dRT ? dRT->depthWriteCount : 0;
        q.lastClearFrame = dRT ? dRT->lastClearFrame : 0;
        q.lastClearDraw = dRT ? dRT->lastClearDraw : 0;
        q.lastDepthWriteFrame = dRT ? dRT->lastDepthWriteFrame : 0;
        q.lastDepthWriteDraw = dRT ? dRT->lastDepthWriteDraw : 0;
        q.rtvPtr = (uint64_t)(uintptr_t)(cRT[0] ? cRT[0]->res.Get() : nullptr);
        q.indices = ev.nIdx; q.issuedIndices = count;
        q.zfunc = (uint32_t)dc.zfunc;
        q.hostZfunc = (uint32_t)DepthFuncToHost(q.zfunc);
        q.zEnable = dc.z_enable ? 1u : 0u; q.zWrite = dc.z_write_enable ? 1u : 0u;
        q.dsvFormat = dRT ? dRT->xenosFormat : 0;
        q.dsvDxgiFormat = dRT ? (uint32_t)dRT->format : 0;
        q.dsvWidth = dRT ? dRT->w : 0; q.dsvHeight = dRT ? dRT->h : 0;
        q.viewportX = vp.xy_offset[0]; q.viewportY = vp.xy_offset[1];
        q.viewportWidth = vp.xy_extent[0]; q.viewportHeight = vp.xy_extent[1];
        q.rtvWidth = cRT[0] ? cRT[0]->w : 0; q.rtvHeight = cRT[0] ? cRT[0]->h : 0;
        q.viewportInDsv = viewportInDsv ? 1u : 0u;
        q.scissorInDsv = scissorInDsv ? 1u : 0u;
        q.targetsCompatible = targetsCompatible ? 1u : 0u;
        q.depthClip = R<reg::PA_CL_CLIP_CNTL>().clip_disable ? 0u : 1u;
        q.writesDepth = ps && ps->writesDepth ? 1u : 0u;
        q.ndcZReversed = ndcZReversed ? 1u : 0u;
        q.zMin = vp.z_min; q.zMax = vp.z_max;
        q.ndcScaleZ = vp.ndc_scale[2]; q.ndcOffsetZ = vp.ndc_offset[2];
        q.scissor[0] = r.left; q.scissor[1] = r.top; q.scissor[2] = r.right; q.scissor[3] = r.bottom;
        q.scissorBound = (uint64_t)(r.right - r.left) * (uint64_t)(r.bottom - r.top);
        depthQueryRecords.push_back(q);
    }

    // Diagnóstico por lectura directa de PhysHost: no repite el VS ni crea PSO.
    static uint64_t guestInputDumpFrame = 0;
    static uint32_t rectListSeen = 0;
    if (guestInputDumpFrame != frame) {
        guestInputDumpFrame = frame;
        rectListSeen = 0;
    }
    const uint32_t vteValue = RV(0x2206);
    const bool rectList3 = prim == 8 && ev.nIdx == 3 && vteValue == 0x300;
    if (rt::gpu::Cfg().stateLog && rt::gpu::Cfg().stateLogFrame == frame && rectList3) {
        const bool firstThree = rectListSeen < 3;
        ++rectListSeen;
        const bool alwaysClear = dc.z_enable && dc.z_write_enable && dc.zfunc == xenos::CompareFunction::kAlways;
        if (firstThree || alwaysClear || (colorMode && nrt && clipOff && ps && !ps->tfetchMask))
            DumpGuestVertexInputs(*vs, frame, drawSerial, vteValue, ev.src,
                                  RV(0x2102) & 0xFFFFFFu, 3);
    }
    if (dRT && prim == 8 && n == 3 && dc.z_enable && dc.z_write_enable &&
        dc.zfunc == xenos::CompareFunction::kAlways && rt::gpu::Cfg().stateLog &&
        rt::gpu::Cfg().stateLogFrame == frame) {
        static uint64_t depthClearSnapshotFrame = 0;
        static std::set<uint64_t> depthClearSnapshotBases;
        if (depthClearSnapshotFrame != frame) { depthClearSnapshotFrame = frame; depthClearSnapshotBases.clear(); }
        if (depthClearSnapshotBases.insert(depthBase).second)
            DumpDepthSnapshot(*this, *dRT, depthBase, surf.surface_pitch, frame, drawSerial,
                              (uint32_t)dc.zfunc, "post_always_clear");
    }
    if (dRT && dc.z_enable && dc.z_write_enable) {
        ++dRT->depthWriteCount;
        dRT->lastDepthWriteFrame = frame;
        dRT->lastDepthWriteDraw = drawSerial;
    }
    // Capture the first relational draw that can update this DSV. Together
    // with the pre-compare snapshot, this distinguishes a failed depth test
    // from a viewport/depth-write problem without dumping every draw.
    if (dRT && dc.z_enable && dc.z_write_enable && dc.zfunc != xenos::CompareFunction::kAlways &&
        rt::gpu::Cfg().stateLog && rt::gpu::Cfg().stateLogFrame &&
        rt::gpu::FrameCount() + 1 == rt::gpu::Cfg().stateLogFrame) {
        static uint64_t depthPostDumpFrame = 0;
        static std::set<uint64_t> depthPostDumped;
        const uint64_t frame = rt::gpu::FrameCount() + 1;
        if (depthPostDumpFrame != frame) {
            depthPostDumpFrame = frame;
            depthPostDumped.clear();
        }
        const uint64_t key = (uint64_t)depthBase | ((uint64_t)surf.surface_pitch << 12) | ((uint64_t)depthFormat << 32);
        if (!depthPostDumped.count(key)) {
            depthPostDumped.insert(key);
            DumpDepthSnapshot(*this, *dRT, depthBase, surf.surface_pitch, frame, drawSerial,
                              (uint32_t)dc.zfunc, "post_first_relational_write");
        }
    }
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
