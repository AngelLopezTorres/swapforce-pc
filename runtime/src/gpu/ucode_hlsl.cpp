#include "ucode_hlsl.h"

#include <windows.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "xenia/gpu/ucode.h"

namespace rt::gpu {
using namespace xe::gpu::ucode;
namespace xs = xe::gpu::xenos;

namespace {

// ---------------------------------------------------------------------------------------------------------------
// Prologo HLSL comun. Interfaz (fijada por la raiz de firmas del backend D3D12, ver d3d12_backend.cpp):
//   b0  XeConsts: constantes float de VS/PS (SHADER_CONSTANT_000.. y 256..), constantes de fetch (192 dwords de
//       SHADER_CONSTANT_FETCH_00_0), booleanas (8 dwords), de bucle (32 dwords) y parametros del draw.
//   t0  gMem: memoria fisica del guest (512 MB) como ByteAddressBuffer (datos big-endian tal como estan en memoria).
//   t0..t31 en space1/2/3: texturas 2D / 3D / cubo indexadas por constante de fetch; s0..s31: muestreadores.
// ---------------------------------------------------------------------------------------------------------------
const char* kPrelude = R"HLSL(
#define XE_FLT_MAX 3.402823466e+38f
cbuffer XeConsts : register(b0)
{
    float4 gVsFloat[256];
    float4 gPsFloat[256];
    uint4  gFetch[48];
    uint4  gBool[2];
    uint4  gLoop[8];
    uint4  gDraw0;    // x = tipo de primitiva (VGT_DRAW_INITIATOR), y = fuente de indices (0 DMA, 1 inmediato, 2 auto), z = tamano de indice (0 = 16 bit, 1 = 32 bit), w = endian de VGT_DMA_SIZE
    uint4  gDraw1;    // x = direccion fisica (bytes) del buffer de indices, y = numero de indices, z = VGT_INDX_OFFSET, w = reservado
    uint4  gDraw2;    // x = prueba alfa (funcion | habilitada << 3), y = asuint(RB_ALPHA_REF), z = formato de vertice de PA_CL_VTE_CNTL (bit0 xy_fmt, bit1 z_fmt, bit2 w0_fmt), w = reservado
    float4 gNdcScale;  // transformacion NDC del guest -> NDC del host (draw_util::GetHostViewportInfo): xyz = escala
    float4 gNdcOffset; // xyz = desplazamiento (se multiplica por w)
};
ByteAddressBuffer gMem : register(t0, space0);
Texture2D<float4> gTex2D[32] : register(t0, space1);
Texture3D<float4> gTex3D[32] : register(t0, space2);
TextureCube<float4> gTexCube[32] : register(t0, space3);
SamplerState gSmp[32] : register(s0);

uint xe_dw(uint i) { return gFetch[i >> 2][i & 3]; }
bool xe_bool(uint i) { return ((gBool[i >> 7][(i >> 5) & 3] >> (i & 31)) & 1u) != 0u; }
uint xe_loopc(uint i) { return gLoop[i >> 2][i & 3]; }
uint xe_swap(uint v, uint e)
{
    if (e == 1u) return ((v & 0x00FF00FFu) << 8) | ((v >> 8) & 0x00FF00FFu);
    if (e == 2u) return ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) << 8) | ((v >> 8) & 0x0000FF00u) | (v >> 24);
    if (e == 3u) return (v >> 16) | (v << 16);
    return v;
}
// Multiplicacion "legacy" de Direct3D 9 (ucode.h): +-0 * cualquier cosa = +0.
float xe_mul(float a, float b) { return (a == 0.0f || b == 0.0f) ? 0.0f : a * b; }
float4 xe_mul(float4 a, float4 b) { return select((a == 0.0f) | (b == 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), a * b); }
float xe_max(float a, float b) { return a >= b ? a : b; }
float4 xe_max(float4 a, float4 b) { return select(a >= b, a, b); }
float xe_min(float a, float b) { return a < b ? a : b; }
float4 xe_min(float4 a, float4 b) { return select(a < b, a, b); }
float xe_trunc(float a) { return a >= 0.0f ? floor(a) : -floor(-a); }
float4 xe_trunc(float4 a) { return select(a >= 0.0f, floor(a), -floor(-a)); }
float4 xe_set(bool4 c) { return select(c, float4(1.0f, 1.0f, 1.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)); }
float xe_set1(bool c) { return c ? 1.0f : 0.0f; }
float xe_rcp(float a) { return a == 1.0f ? 1.0f : 1.0f / a; }
float xe_rcpc(float a) { float t = xe_rcp(a); return isinf(t) ? (t > 0.0f ? XE_FLT_MAX : -XE_FLT_MAX) : t; }
float xe_rcpf(float a) { float t = xe_rcp(a); return isinf(t) ? (t > 0.0f ? 0.0f : -0.0f) : t; }
float xe_rsq(float a) { return a == 1.0f ? 1.0f : 1.0f / sqrt(a); }
float xe_rsqc(float a) { float t = xe_rsq(a); return isinf(t) ? (t > 0.0f ? XE_FLT_MAX : -XE_FLT_MAX) : t; }
float xe_rsqf(float a) { float t = xe_rsq(a); return isinf(t) ? (t > 0.0f ? 0.0f : -0.0f) : t; }
float xe_log(float a) { return a == 1.0f ? 0.0f : log2(a); }
float xe_logc(float a) { float t = xe_log(a); return (isinf(t) && t < 0.0f) ? -XE_FLT_MAX : t; }
float xe_mulsprev2(float a, float b, float p)
{
    return (p == -XE_FLT_MAX || !isfinite(p) || !isfinite(b) || b <= 0.0f) ? -XE_FLT_MAX : xe_mul(a, p);
}
// Cube (ucode.h): devuelve (tc, sc, 2 * eje mayor, id de cara) a partir del vector de direccion.
float4 xe_cube(float3 v)
{
    float3 a = abs(v);
    float tc, sc, ma, id;
    if (a.z >= a.x && a.z >= a.y) { tc = -v.y; sc = v.z < 0.0f ? -v.x : v.x; ma = 2.0f * v.z; id = v.z < 0.0f ? 5.0f : 4.0f; }
    else if (a.y >= a.x) { tc = v.y < 0.0f ? -v.z : v.z; sc = v.x; ma = 2.0f * v.y; id = v.y < 0.0f ? 3.0f : 2.0f; }
    else { tc = -v.y; sc = v.x < 0.0f ? v.z : -v.z; ma = 2.0f * v.x; id = v.x < 0.0f ? 1.0f : 0.0f; }
    return float4(tc, sc, ma, id);
}
// Direccion de muestreo a partir de (s, t, cara) con s,t en [1, 2) (convencion de cubemaps de D3D9, ucode.h kCube).
float3 xe_cubedir(float3 c)
{
    float s = (c.x - 1.5f) * 2.0f, t = (c.y - 1.5f) * 2.0f;
    uint f = (uint)c.z;
    if (f == 0u) return float3(1.0f, -t, -s);
    if (f == 1u) return float3(-1.0f, -t, s);
    if (f == 2u) return float3(s, 1.0f, t);
    if (f == 3u) return float3(s, -1.0f, -t);
    if (f == 4u) return float3(s, -t, 1.0f);
    return float3(-s, -t, -1.0f);
}
float xe_unorm(uint v, uint bits) { return float(v) / float((1u << bits) - 1u); }
int xe_sext(uint v, uint bits) { return ((int)(v << (32u - bits))) >> (32u - bits); }
float xe_snorm(int v, uint bits, uint rf)
{
    float m = float((1u << (bits - 1u)) - 1u);
    return rf == 0u ? max(float(v) / m, -1.0f) : (2.0f * float(v) + 1.0f) / (2.0f * m + 1.0f);
}
// Mapeo de vertice logico -> indice de primitiva segun el tipo (listas, abanico, lista de quads).
uint xe_logical(uint vid)
{
    uint p = gDraw0.x;
    if (p == 5u || p == 15u) { uint tri = vid / 3u, k = vid % 3u; return k == 0u ? 0u : tri + k; }
    if (p == 13u) { uint q = vid / 6u, k = vid % 6u; uint m[6] = { 0u, 1u, 2u, 0u, 2u, 3u }; return q * 4u + m[k]; }
    return vid;
}
uint xe_vertex_index(uint vid)
{
    uint l = xe_logical(vid);
    uint idx = l;
    if (gDraw0.y == 0u)
    {
        uint e = gDraw0.w;
        if (gDraw0.z != 0u) idx = xe_swap(gMem.Load((gDraw1.x + 4u * l) & 0x1FFFFFFFu), e);
        else
        {
            uint a = gDraw1.x + 2u * l;
            uint w = xe_swap(gMem.Load(a & 0x1FFFFFFCu), e);
            idx = (a & 2u) != 0u ? (w >> 16) : (w & 0xFFFFu);
        }
    }
    return (uint)((int)(idx & 0xFFFFFFu) + (int)gDraw1.z);
}
)HLSL";

struct Emitter {
    const ShaderInfo& si;
    const HlslOptions& opt;
    bool vs;
    std::string out;
    int ind = 1;
    uint32_t numRegs = 1;
    uint32_t unsupported = 0;
    std::string firstError;
    VertexFetchInstruction prevFull;
    bool havePrevFull = false;

    Emitter(const ShaderInfo& s, const HlslOptions& o) : si(s), opt(o), vs(s.vertex) { std::memset(&prevFull, 0, sizeof(prevFull)); }

    void L(const char* fmt, ...) {
        char b[2048];
        va_list ap; va_start(ap, fmt); std::vsnprintf(b, sizeof(b), fmt, ap); va_end(ap);
        for (int i = 0; i < ind; ++i) out += "    ";
        out += b; out += '\n';
    }
    void Unsupported(const std::string& what) {
        ++unsupported;
        if (firstError.empty()) firstError = what;
        L("/* NO SOPORTADO: %s */", what.c_str());
    }

    static const char* Comp(uint32_t c) { static const char* n[4] = {"x", "y", "z", "w"}; return n[c & 3]; }
    static std::string Mask(uint32_t m) { std::string s; for (int i = 0; i < 4; ++i) if (m & (1u << i)) s += "xyzw"[i]; return s; }

    std::string Reg(uint32_t idx, bool rel) {
        char b[64];
        if (rel) std::snprintf(b, sizeof(b), "r[(%u + aL) & 127]", idx);
        else std::snprintf(b, sizeof(b), "r[%u]", idx);
        return b;
    }
    std::string Const(uint32_t idx, bool addressed, bool a0rel) {
        char b[96];
        const char* arr = vs ? "gVsFloat" : "gPsFloat";
        if (!addressed) std::snprintf(b, sizeof(b), "%s[%u]", arr, idx & 255);
        else std::snprintf(b, sizeof(b), "%s[clamp((int)%u + %s, 0, 255)]", arr, idx, a0rel ? "a0" : "aL");
        return b;
    }

    // Operando de ALU i (1..3) con swizzle completo de 4 componentes (relativo al componente, ucode.h).
    std::string AluSrc4(const AluInstruction& a, int i) {
        uint32_t reg = a.src_reg(i), swz = a.src_swizzle(i);
        std::string base; bool abs = false;
        if (a.src_is_temp(i)) {
            base = Reg(AluInstruction::src_temp_reg(reg), AluInstruction::is_src_temp_relative(reg));
            abs = AluInstruction::is_src_temp_value_absolute(reg);
        } else {
            base = Const(reg, a.src_const_is_addressed(i), a.is_const_address_register_relative());
            abs = a.abs_constants();
        }
        std::string s;
        for (int j = 0; j < 4; ++j) s += Comp(AluInstruction::GetSwizzledComponentIndex(swz, j));
        std::string e = base + "." + s;
        if (abs) e = "abs(" + e + ")";
        if (a.src_negate(i)) e = "(-" + e + ")";
        return e;
    }
    // Operando escalar de la fuente 3 (a = W relativo; b = X relativo).
    std::string ScalarA(const AluInstruction& a) { return ScalarComp(a, 3); }
    std::string ScalarB(const AluInstruction& a) { return ScalarComp(a, 0); }
    std::string ScalarComp(const AluInstruction& a, int comp) {
        uint32_t reg = a.src_reg(3), swz = a.src_swizzle(3);
        std::string base; bool abs = false;
        if (a.src_is_temp(3)) {
            base = Reg(AluInstruction::src_temp_reg(reg), AluInstruction::is_src_temp_relative(reg));
            abs = AluInstruction::is_src_temp_value_absolute(reg);
        } else {
            base = Const(reg, a.src_const_is_addressed(3), a.is_const_address_register_relative());
            abs = a.abs_constants();
        }
        std::string e = base + "." + Comp(AluInstruction::GetSwizzledComponentIndex(swz, comp));
        if (abs) e = "abs(" + e + ")";
        if (a.src_negate(3)) e = "(-" + e + ")";
        return e;
    }
    // mulsc/addsc/subsc: constante c# (operando 3, W) y registro temporal fijo (X).
    std::string ScalarConstA(const AluInstruction& a) {
        uint32_t reg = a.src_reg(3), swz = a.src_swizzle(3);
        std::string e = Const(reg, a.src_const_is_addressed(3), a.is_const_address_register_relative()) + "." +
                        Comp(AluInstruction::GetSwizzledComponentIndex(swz, 3));
        if (a.abs_constants()) e = "abs(" + e + ")";
        if (a.src_negate(3)) e = "(-" + e + ")";
        return e;
    }
    std::string ScalarConstB(const AluInstruction& a) {
        uint32_t swz = a.src_swizzle(3);
        std::string e = Reg(a.scalar_const_reg_op_src_temp_reg(), false) + "." + Comp(AluInstruction::GetSwizzledComponentIndex(swz, 0));
        if (a.abs_constants()) e = "abs(" + e + ")";
        if (a.src_negate(3)) e = "(-" + e + ")";
        return e;
    }

    std::string ExportTarget(uint32_t d) {
        char b[64];
        if (vs) {
            if (d < 16) { std::snprintf(b, sizeof(b), "oI[%u]", d); return b; }
            if (d == 62) return "oPos";
            if (d == 63) return "oPts";
        } else {
            if (d < 4) { std::snprintf(b, sizeof(b), "oC[%u]", d); return b; }
            if (d == 61) return "oDepthV";
        }
        return "";  // memexport (eA / eM#) u otros: sin efecto
    }

    void EmitAlu(const AluInstruction& a) {
        const AluVectorOpcodeInfo& vi = GetAluVectorOpcodeInfo(a.vector_opcode());
        const AluScalarOpcodeInfo& sci = GetAluScalarOpcodeInfo(a.scalar_opcode());
        AluVectorOpcode vop = a.vector_opcode();
        AluScalarOpcode sop = a.scalar_opcode();
        uint32_t vmask = a.GetVectorOpResultWriteMask(), smask = a.GetScalarOpResultWriteMask();
        bool isExport = a.is_export();
        bool scalarRetain = sop == AluScalarOpcode::kRetainPrev;
        // Instruccion sin ningun efecto (max r0, r0 sin mascara + retain_prev): se omite.
        bool vectorNop = !vmask && !(vi.changed_state);
        bool scalarNop = scalarRetain && !smask;
        if (vectorNop && scalarNop && !(isExport && a.GetConstant0WriteMask() | a.GetConstant1WriteMask())) return;

        bool pred = a.is_predicated();
        if (pred) { L("if (p0 == %s)", a.predicate_condition() ? "true" : "false"); L("{"); ++ind; }
        L("{");
        ++ind;
        uint32_t nv = vi.GetOperandCount();
        // Lectura de operandos antes de cualquier escritura (el hardware lee todas las fuentes primero).
        for (uint32_t i = 1; i <= nv; ++i) L("float4 v%u = %s;", i - 1, AluSrc4(a, (int)i).c_str());
        for (uint32_t i = nv + 1; i <= 3; ++i) L("float4 v%u = float4(0.0f, 0.0f, 0.0f, 0.0f);", i - 1);
        if (sci.operand_count == 1) {
            L("float sa = %s;", ScalarA(a).c_str());
            if (sci.single_operand_is_two_component) L("float sb = %s;", ScalarB(a).c_str()); else L("float sb = 0.0f;");
        } else if (sci.operand_count == 2) {
            L("float sa = %s;", ScalarConstA(a).c_str());
            L("float sb = %s;", ScalarConstB(a).c_str());
        } else { L("float sa = 0.0f;"); L("float sb = 0.0f;"); }
        L("float4 vr = float4(0.0f, 0.0f, 0.0f, 0.0f);");
        L("float sr = ps;");
        // ---- operacion vectorial ----
        auto SetP = [&](const char* cmp) {
            // setp_*_push: p0 = (src0.w == 0 && src1.w <cmp> 0); dest = (src0.x == 0 && src1.x <cmp> 0) ? 0 : src0.x + 1
            L("p0 = (v0.w == 0.0f && v1.w %s 0.0f);", cmp);
            L("vr = ((v0.x == 0.0f && v1.x %s 0.0f) ? 0.0f : v0.x + 1.0f).xxxx;", cmp);
        };
        switch (vop) {
            case AluVectorOpcode::kAdd: L("vr = v0 + v1;"); break;
            case AluVectorOpcode::kMul: L("vr = xe_mul(v0, v1);"); break;
            case AluVectorOpcode::kMax: L("vr = xe_max(v0, v1);"); break;
            case AluVectorOpcode::kMin: L("vr = xe_min(v0, v1);"); break;
            case AluVectorOpcode::kSeq: L("vr = xe_set(v0 == v1);"); break;
            case AluVectorOpcode::kSgt: L("vr = xe_set(v0 > v1);"); break;
            case AluVectorOpcode::kSge: L("vr = xe_set(v0 >= v1);"); break;
            case AluVectorOpcode::kSne: L("vr = xe_set(v0 != v1);"); break;
            case AluVectorOpcode::kFrc: L("vr = v0 - floor(v0);"); break;
            case AluVectorOpcode::kTrunc: L("vr = xe_trunc(v0);"); break;
            case AluVectorOpcode::kFloor: L("vr = floor(v0);"); break;
            case AluVectorOpcode::kMad: L("vr = xe_mul(v0, v1) + v2;"); break;
            case AluVectorOpcode::kCndEq: L("vr = select(v0 == 0.0f, v1, v2);"); break;
            case AluVectorOpcode::kCndGe: L("vr = select(v0 >= 0.0f, v1, v2);"); break;
            case AluVectorOpcode::kCndGt: L("vr = select(v0 > 0.0f, v1, v2);"); break;
            case AluVectorOpcode::kDp4: L("vr = (xe_mul(v0.x, v1.x) + xe_mul(v0.y, v1.y) + xe_mul(v0.z, v1.z) + xe_mul(v0.w, v1.w)).xxxx;"); break;
            case AluVectorOpcode::kDp3: L("vr = (xe_mul(v0.x, v1.x) + xe_mul(v0.y, v1.y) + xe_mul(v0.z, v1.z)).xxxx;"); break;
            case AluVectorOpcode::kDp2Add: L("vr = (xe_mul(v0.x, v1.x) + xe_mul(v0.y, v1.y) + v2.x).xxxx;"); break;
            case AluVectorOpcode::kCube: L("vr = xe_cube(float3(v0.z, v0.w, v0.x));"); break;   // src0 = zzxy: x = src0.z, y = src0.w, z = src0.x
            case AluVectorOpcode::kMax4: L("vr = max(max(v0.x, v0.y), max(v0.z, v0.w)).xxxx;"); break;
            case AluVectorOpcode::kSetpEqPush: SetP("=="); break;
            case AluVectorOpcode::kSetpNePush: SetP("!="); break;
            case AluVectorOpcode::kSetpGtPush: SetP(">"); break;
            case AluVectorOpcode::kSetpGePush: SetP(">="); break;
            case AluVectorOpcode::kKillEq: L("vr = xe_set1(any(v0 == v1)).xxxx;"); if (!vs) L("if (any(v0 == v1)) discard;"); break;
            case AluVectorOpcode::kKillGt: L("vr = xe_set1(any(v0 > v1)).xxxx;"); if (!vs) L("if (any(v0 > v1)) discard;"); break;
            case AluVectorOpcode::kKillGe: L("vr = xe_set1(any(v0 >= v1)).xxxx;"); if (!vs) L("if (any(v0 >= v1)) discard;"); break;
            case AluVectorOpcode::kKillNe: L("vr = xe_set1(any(v0 != v1)).xxxx;"); if (!vs) L("if (any(v0 != v1)) discard;"); break;
            case AluVectorOpcode::kDst: L("vr = float4(1.0f, xe_mul(v0.y, v1.y), v0.z, v1.w);"); break;
            case AluVectorOpcode::kMaxA:
                L("a0 = (int)clamp(floor(v0.w + 0.5f), -256.0f, 255.0f);");
                L("vr = xe_max(v0, v1);");
                break;
            default: Unsupported("opcode vectorial " + std::to_string((uint32_t)vop)); break;
        }
        if (a.vector_clamp()) L("vr = saturate(vr);");
        // ---- operacion escalar ----
        bool scalarWrites = !scalarRetain;
        auto SetpS = [&](const char* cond, const char* val_true, const char* val_false) {
            L("if (%s) { sr = %s; p0 = true; } else { sr = %s; p0 = false; }", cond, val_true, val_false);
        };
        auto KillS = [&](const char* cond) {
            L("if (%s) { sr = 1.0f; %s } else { sr = 0.0f; }", cond, vs ? "" : "discard;");
        };
        switch (sop) {
            case AluScalarOpcode::kAdds: L("sr = sa + sb;"); break;
            case AluScalarOpcode::kAddsPrev: L("sr = sa + ps;"); break;
            case AluScalarOpcode::kMuls: L("sr = xe_mul(sa, sb);"); break;
            case AluScalarOpcode::kMulsPrev: L("sr = xe_mul(sa, ps);"); break;
            case AluScalarOpcode::kMulsPrev2: L("sr = xe_mulsprev2(sa, sb, ps);"); break;
            case AluScalarOpcode::kMaxs: L("sr = xe_max(sa, sb);"); break;
            case AluScalarOpcode::kMins: L("sr = xe_min(sa, sb);"); break;
            case AluScalarOpcode::kSeqs: L("sr = xe_set1(sa == 0.0f);"); break;
            case AluScalarOpcode::kSgts: L("sr = xe_set1(sa > 0.0f);"); break;
            case AluScalarOpcode::kSges: L("sr = xe_set1(sa >= 0.0f);"); break;
            case AluScalarOpcode::kSnes: L("sr = xe_set1(sa != 0.0f);"); break;
            case AluScalarOpcode::kFrcs: L("sr = sa - floor(sa);"); break;
            case AluScalarOpcode::kTruncs: L("sr = xe_trunc(sa);"); break;
            case AluScalarOpcode::kFloors: L("sr = floor(sa);"); break;
            case AluScalarOpcode::kExp: L("sr = exp2(sa);"); break;
            case AluScalarOpcode::kLogc: L("sr = xe_logc(sa);"); break;
            case AluScalarOpcode::kLog: L("sr = xe_log(sa);"); break;
            case AluScalarOpcode::kRcpc: L("sr = xe_rcpc(sa);"); break;
            case AluScalarOpcode::kRcpf: L("sr = xe_rcpf(sa);"); break;
            case AluScalarOpcode::kRcp: L("sr = xe_rcp(sa);"); break;
            case AluScalarOpcode::kRsqc: L("sr = xe_rsqc(sa);"); break;
            case AluScalarOpcode::kRsqf: L("sr = xe_rsqf(sa);"); break;
            case AluScalarOpcode::kRsq: L("sr = xe_rsq(sa);"); break;
            case AluScalarOpcode::kMaxAs: L("a0 = (int)clamp(floor(sa + 0.5f), -256.0f, 255.0f);"); L("sr = xe_max(sa, sb);"); break;
            case AluScalarOpcode::kMaxAsf: L("a0 = (int)clamp(floor(sa), -256.0f, 255.0f);"); L("sr = xe_max(sa, sb);"); break;
            case AluScalarOpcode::kSubs: L("sr = sa - sb;"); break;
            case AluScalarOpcode::kSubsPrev: L("sr = sa - ps;"); break;
            case AluScalarOpcode::kSetpEq: SetpS("sa == 0.0f", "0.0f", "1.0f"); break;
            case AluScalarOpcode::kSetpNe: SetpS("sa != 0.0f", "0.0f", "1.0f"); break;
            case AluScalarOpcode::kSetpGt: SetpS("sa > 0.0f", "0.0f", "1.0f"); break;
            case AluScalarOpcode::kSetpGe: SetpS("sa >= 0.0f", "0.0f", "1.0f"); break;
            case AluScalarOpcode::kSetpInv: SetpS("sa == 1.0f", "0.0f", "(sa == 0.0f ? 1.0f : sa)"); break;
            case AluScalarOpcode::kSetpPop: SetpS("sa - 1.0f <= 0.0f", "0.0f", "sa - 1.0f"); break;
            case AluScalarOpcode::kSetpClr: L("sr = XE_FLT_MAX; p0 = false;"); break;
            case AluScalarOpcode::kSetpRstr: SetpS("sa == 0.0f", "0.0f", "sa"); break;
            case AluScalarOpcode::kKillsEq: KillS("sa == 0.0f"); break;
            case AluScalarOpcode::kKillsGt: KillS("sa > 0.0f"); break;
            case AluScalarOpcode::kKillsGe: KillS("sa >= 0.0f"); break;
            case AluScalarOpcode::kKillsNe: KillS("sa != 0.0f"); break;
            case AluScalarOpcode::kKillsOne: KillS("sa == 1.0f"); break;
            case AluScalarOpcode::kSqrt: L("sr = sqrt(sa);"); break;
            case AluScalarOpcode::kMulsc0: case AluScalarOpcode::kMulsc1: L("sr = xe_mul(sa, sb);"); break;
            case AluScalarOpcode::kAddsc0: case AluScalarOpcode::kAddsc1: L("sr = sa + sb;"); break;
            case AluScalarOpcode::kSubsc0: case AluScalarOpcode::kSubsc1: L("sr = sa - sb;"); break;
            case AluScalarOpcode::kSin: L("sr = sin(sa);"); break;
            case AluScalarOpcode::kCos: L("sr = cos(sa);"); break;
            case AluScalarOpcode::kRetainPrev: break;
            default: Unsupported("opcode escalar " + std::to_string((uint32_t)sop)); break;
        }
        if (scalarWrites && a.scalar_clamp()) L("sr = saturate(sr);");
        // ---- escritura de resultados ----
        if (isExport) {
            std::string tgt = ExportTarget(a.vector_dest());
            if (tgt.empty()) {
                L("/* export a eA/eM# (memexport) o registro no usado: sin efecto */");
            } else {
                if (vmask) L("%s.%s = vr.%s;", tgt.c_str(), Mask(vmask).c_str(), Mask(vmask).c_str());
                if (smask) L("%s.%s = sr.xxxx.%s;", tgt.c_str(), Mask(smask).c_str(), Mask(smask).c_str());
                uint32_t c0 = a.GetConstant0WriteMask(), c1 = a.GetConstant1WriteMask();
                if (c0) L("%s.%s = 0.0f;", tgt.c_str(), Mask(c0).c_str());
                if (c1) L("%s.%s = 1.0f;", tgt.c_str(), Mask(c1).c_str());
            }
        } else {
            if (vmask) L("%s.%s = vr.%s;", Reg(a.vector_dest(), a.is_vector_dest_relative()).c_str(), Mask(vmask).c_str(), Mask(vmask).c_str());
            if (smask) L("%s.%s = sr.xxxx.%s;", Reg(a.scalar_dest(), a.is_scalar_dest_relative()).c_str(), Mask(smask).c_str(), Mask(smask).c_str());
        }
        if (scalarWrites) L("ps = sr;");
        --ind;
        L("}");
        if (pred) { --ind; L("}"); }
    }

    // Escritura del resultado de un fetch en el registro destino segun el swizzle de destino (3 bits por componente).
    void FetchWrite(uint32_t dst, bool rel, uint32_t dswz, const char* val) {
        std::string r = Reg(dst, rel);
        for (int i = 0; i < 4; ++i) {
            uint32_t s = (dswz >> (3 * i)) & 7;
            if (s == 7) continue;
            if (s < 4) L("%s.%s = %s.%s;", r.c_str(), Comp(i), val, Comp(s));
            else if (s == 4) L("%s.%s = 0.0f;", r.c_str(), Comp(i));
            else if (s == 5) L("%s.%s = 1.0f;", r.c_str(), Comp(i));
            else L("%s.%s = 0.0f;", r.c_str(), Comp(i));
        }
    }

    void EmitVertexFetch(const VertexFetchInstruction& v) {
        const VertexFetchInstruction& full = v.is_mini_fetch() ? prevFull : v;
        if (v.is_mini_fetch() && !havePrevFull) { Unsupported("vfetch_mini sin vfetch_full previo"); return; }
        bool pred = v.is_predicated();
        if (pred) { L("if (p0 == %s)", v.predicate_condition() ? "true" : "false"); L("{"); ++ind; }
        L("{");
        ++ind;
        uint32_t ci = full.fetch_constant_index();
        uint32_t dw = (ci / 3) * 6 + (ci % 3) * 2;
        L("uint fc0 = xe_dw(%u), fc1 = xe_dw(%u);", dw, dw + 1);
        L("uint en = fc1 & 3u;");
        if (!v.is_mini_fetch()) {
            L("uint vidx = (uint)max(floor(%s.%s%s), 0.0f);", Reg(full.src(), full.is_src_relative()).c_str(), Comp(full.src_swizzle() & 3),
              full.is_index_rounded() ? " + 0.5f" : "");
            L("xe_vbase = fc0 & 0xFFFFFFFCu; xe_vidx = vidx;");
        }
        L("uint ad = (xe_vbase + (uint)((int)(xe_vidx * %uu) + (int)(%d)) * 4u) & 0x1FFFFFFFu;", full.stride(), v.offset());
        xs::VertexFormat fmt = v.data_format();
        bool sgn = v.is_signed(), norm = v.is_normalized();
        uint32_t rf = (uint32_t)v.signed_rf_mode();
        auto W = [&](int k) { L("uint w%d = xe_swap(gMem.Load((ad + %uu) & 0x1FFFFFFFu), en);", k, k * 4); };
        L("float4 t = float4(0.0f, 0.0f, 0.0f, 1.0f);");
        auto Unpack8 = [&](int comp, int shift) {
            if (sgn) {
                if (norm) L("t.%s = xe_snorm(xe_sext((w0 >> %d) & 0xFFu, 8u), 8u, %uu);", Comp(comp), shift, rf);
                else L("t.%s = (float)xe_sext((w0 >> %d) & 0xFFu, 8u);", Comp(comp), shift);
            } else {
                if (norm) L("t.%s = (float)((w0 >> %d) & 0xFFu) / 255.0f;", Comp(comp), shift);
                else L("t.%s = (float)((w0 >> %d) & 0xFFu);", Comp(comp), shift);
            }
        };
        auto Unpack16 = [&](int comp, int wi, int shift) {
            if (sgn) {
                if (norm) L("t.%s = xe_snorm(xe_sext((w%d >> %d) & 0xFFFFu, 16u), 16u, %uu);", Comp(comp), wi, shift, rf);
                else L("t.%s = (float)xe_sext((w%d >> %d) & 0xFFFFu, 16u);", Comp(comp), wi, shift);
            } else {
                if (norm) L("t.%s = (float)((w%d >> %d) & 0xFFFFu) / 65535.0f;", Comp(comp), wi, shift);
                else L("t.%s = (float)((w%d >> %d) & 0xFFFFu);", Comp(comp), wi, shift);
            }
        };
        auto UnpackBits = [&](int comp, int shift, int bits) {
            if (sgn) {
                if (norm) L("t.%s = xe_snorm(xe_sext((w0 >> %d) & %uu, %uu), %uu, %uu);", Comp(comp), shift, (1u << bits) - 1, bits, bits, rf);
                else L("t.%s = (float)xe_sext((w0 >> %d) & %uu, %uu);", Comp(comp), shift, (1u << bits) - 1, bits);
            } else {
                if (norm) L("t.%s = (float)((w0 >> %d) & %uu) / %u.0f;", Comp(comp), shift, (1u << bits) - 1, (1u << bits) - 1);
                else L("t.%s = (float)((w0 >> %d) & %uu);", Comp(comp), shift, (1u << bits) - 1);
            }
        };
        auto Unpack32 = [&](int comp, int wi, bool isFloat) {
            if (isFloat) L("t.%s = asfloat(w%d);", Comp(comp), wi);
            else if (sgn) L("t.%s = %s;", Comp(comp), norm ? ("max((float)asint(w" + std::to_string(wi) + ") / 2147483647.0f, -1.0f)").c_str() : ("(float)asint(w" + std::to_string(wi) + ")").c_str());
            else L("t.%s = %s;", Comp(comp), norm ? ("(float)w" + std::to_string(wi) + " / 4294967295.0f").c_str() : ("(float)w" + std::to_string(wi)).c_str());
        };
        switch (fmt) {
            case xs::VertexFormat::k_8_8_8_8:
                W(0); for (int i = 0; i < 4; ++i) Unpack8(i, 8 * i);
                break;
            case xs::VertexFormat::k_2_10_10_10:
                W(0); UnpackBits(0, 0, 10); UnpackBits(1, 10, 10); UnpackBits(2, 20, 10); UnpackBits(3, 30, 2);
                break;
            case xs::VertexFormat::k_10_11_11:   // [NV] disposicion de bits: x en los 10 bits bajos, luego y (11) y z (11)
                W(0); UnpackBits(0, 0, 10); UnpackBits(1, 10, 11); UnpackBits(2, 21, 11); L("t.w = 1.0f;");
                break;
            case xs::VertexFormat::k_11_11_10:   // [NV] x en los 11 bits bajos, luego y (11) y z (10)
                W(0); UnpackBits(0, 0, 11); UnpackBits(1, 11, 11); UnpackBits(2, 22, 10); L("t.w = 1.0f;");
                break;
            case xs::VertexFormat::k_16_16:
                W(0); Unpack16(0, 0, 0); Unpack16(1, 0, 16); L("t.z = 0.0f; t.w = 1.0f;");
                break;
            case xs::VertexFormat::k_16_16_16_16:
                W(0); W(1); Unpack16(0, 0, 0); Unpack16(1, 0, 16); Unpack16(2, 1, 0); Unpack16(3, 1, 16);
                break;
            case xs::VertexFormat::k_16_16_FLOAT:
                W(0); L("t.x = f16tof32(w0 & 0xFFFFu); t.y = f16tof32(w0 >> 16); t.z = 0.0f; t.w = 1.0f;");
                break;
            case xs::VertexFormat::k_16_16_16_16_FLOAT:
                W(0); W(1); L("t = float4(f16tof32(w0 & 0xFFFFu), f16tof32(w0 >> 16), f16tof32(w1 & 0xFFFFu), f16tof32(w1 >> 16));");
                break;
            case xs::VertexFormat::k_32: W(0); Unpack32(0, 0, false); L("t.yz = 0.0f; t.w = 1.0f;"); break;
            case xs::VertexFormat::k_32_32: W(0); W(1); Unpack32(0, 0, false); Unpack32(1, 1, false); L("t.z = 0.0f; t.w = 1.0f;"); break;
            case xs::VertexFormat::k_32_32_32_32: W(0); W(1); W(2); W(3); for (int i = 0; i < 4; ++i) Unpack32(i, i, false); break;
            case xs::VertexFormat::k_32_FLOAT: W(0); Unpack32(0, 0, true); L("t.yz = 0.0f; t.w = 1.0f;"); break;
            case xs::VertexFormat::k_32_32_FLOAT: W(0); W(1); Unpack32(0, 0, true); Unpack32(1, 1, true); L("t.z = 0.0f; t.w = 1.0f;"); break;
            case xs::VertexFormat::k_32_32_32_FLOAT: W(0); W(1); W(2); for (int i = 0; i < 3; ++i) Unpack32(i, i, true); L("t.w = 1.0f;"); break;
            case xs::VertexFormat::k_32_32_32_32_FLOAT: W(0); W(1); W(2); W(3); for (int i = 0; i < 4; ++i) Unpack32(i, i, true); break;
            default: Unsupported("formato de vertice " + std::to_string((uint32_t)fmt)); break;
        }
        if (v.exp_adjust() != 0) L("t *= exp2(%d.0f);", v.exp_adjust());
        FetchWrite(v.dest(), v.is_dest_relative(), v.dest_swizzle(), "t");
        --ind;
        L("}");
        if (pred) { --ind; L("}"); }
        if (!v.is_mini_fetch()) { prevFull = v; havePrevFull = true; }
    }

    void EmitTextureFetch(const TextureFetchInstruction& t) {
        FetchOpcode op = t.opcode();
        bool pred = t.is_predicated();
        uint32_t ci = t.fetch_constant_index();
        uint32_t src = t.src(); bool srel = t.is_src_relative();
        std::string sreg = Reg(src, srel);
        auto SrcN = [&](int n) {
            std::string s;
            for (int j = 0; j < n; ++j) s += Comp((t.src_swizzle() >> (2 * j)) & 3);
            return sreg + "." + s;
        };
        if (pred) { L("if (p0 == %s)", t.predicate_condition() ? "true" : "false"); L("{"); ++ind; }
        switch (op) {
            case FetchOpcode::kSetTextureLod: L("xe_lod = %s;", SrcN(1).c_str()); break;
            case FetchOpcode::kSetTextureGradientsHorz: L("xe_gradH = %s;", SrcN(3).c_str()); break;
            case FetchOpcode::kSetTextureGradientsVert: L("xe_gradV = %s;", SrcN(3).c_str()); break;
            case FetchOpcode::kTextureFetch: {
                L("{");
                ++ind;
                xs::FetchOpDimension dim = t.dimension();
                const char* smp = vs ? "SampleLevel" : "Sample";
                std::string lodArg = ", 0.0f";
                if (t.use_register_lod()) { smp = "SampleLevel"; lodArg = ", xe_lod"; }
                else if (vs) { smp = "SampleLevel"; lodArg = ", 0.0f"; }
                else lodArg = "";
                bool grads = t.use_register_gradients();
                bool unnorm = t.unnormalized_coordinates();
                float bias = t.lod_bias();
                if (dim == xs::FetchOpDimension::k2D) {
                    L("float2 uv = %s;", SrcN(2).c_str());
                    if (unnorm || t.offset_x() != 0.0f || t.offset_y() != 0.0f) {
                        L("uint tw, th; gTex2D[%u].GetDimensions(tw, th);", ci);
                        if (unnorm) L("uv /= float2(tw, th);");
                        if (t.offset_x() != 0.0f || t.offset_y() != 0.0f) L("uv += float2(%.1ff, %.1ff) / float2(tw, th);", t.offset_x(), t.offset_y());
                    }
                    if (grads) L("float4 t = gTex2D[%u].SampleGrad(gSmp[%u], uv, xe_gradH.xy, xe_gradV.xy);", ci, ci);
                    else if (!vs && bias != 0.0f && !t.use_register_lod()) L("float4 t = gTex2D[%u].SampleBias(gSmp[%u], uv, %.4ff);", ci, ci, bias);
                    else L("float4 t = gTex2D[%u].%s(gSmp[%u], uv%s);", ci, smp, ci, lodArg.c_str());
                } else if (dim == xs::FetchOpDimension::k3DOrStacked) {
                    L("float3 uv = %s;", SrcN(3).c_str());
                    if (unnorm) { L("uint tw, th, td; gTex3D[%u].GetDimensions(tw, th, td);", ci); L("uv /= float3(tw, th, td);"); }
                    if (grads) L("float4 t = gTex3D[%u].SampleGrad(gSmp[%u], uv, xe_gradH, xe_gradV);", ci, ci);
                    else L("float4 t = gTex3D[%u].%s(gSmp[%u], uv%s);", ci, smp, ci, lodArg.c_str());
                } else if (dim == xs::FetchOpDimension::kCube) {
                    L("float3 dr = xe_cubedir(%s);", SrcN(3).c_str());
                    if (grads) L("float4 t = gTexCube[%u].SampleGrad(gSmp[%u], dr, xe_gradH, xe_gradV);", ci, ci);
                    else L("float4 t = gTexCube[%u].%s(gSmp[%u], dr%s);", ci, smp, ci, lodArg.c_str());
                } else {  // 1D: se trata como 2D de altura 1 (la textura 1D del guest se sube como 2D, H5)
                    L("float2 uv = float2(%s, 0.5f);", SrcN(1).c_str());
                    L("float4 t = gTex2D[%u].%s(gSmp[%u], uv%s);", ci, smp, ci, lodArg.c_str());
                }
                FetchWrite(t.dest(), t.is_dest_relative(), t.dest_swizzle(), "t");
                --ind;
                L("}");
                break;
            }
            case FetchOpcode::kGetTextureComputedLod:
                if (vs) { Unsupported("getCompTexLOD en VS"); break; }
                L("{ float4 t = float4(0, 0, 0, 0);");
                if (t.dimension() == xs::FetchOpDimension::k2D) L("  t.x = gTex2D[%u].CalculateLevelOfDetail(gSmp[%u], %s);", ci, ci, SrcN(2).c_str());
                else Unsupported("getCompTexLOD no 2D");
                FetchWrite(t.dest(), t.is_dest_relative(), t.dest_swizzle(), "t");
                L("}");
                break;
            case FetchOpcode::kGetTextureGradients:
                // ucode.h: la fuente es de 2 componentes; XZ = ddx(fuente.xy), YW = ddy(fuente.xy) (derivadas de quad, probablemente gruesas)
                if (vs) { Unsupported("getGradients en VS"); break; }
                L("{ float2 gs = %s; float4 t = float4(ddx_coarse(gs.x), ddy_coarse(gs.x), ddx_coarse(gs.y), ddy_coarse(gs.y));", SrcN(2).c_str());
                FetchWrite(t.dest(), t.is_dest_relative(), t.dest_swizzle(), "t");
                L("}");
                break;
            case FetchOpcode::kGetTextureWeights:
                // ucode.h: X = factor horizontal, Y = vertical, Z = profundidad, W = mip del filtro bilineal. Aproximacion para 2D (los
                // factores son las fracciones de la posicion en texels del nivel de LOD; [NV] frente al hardware).
                if (vs || t.dimension() != xs::FetchOpDimension::k2D) { Unsupported("getWeights (solo 2D en PS)"); break; }
                L("{ float2 uv = %s; uint tw, th, nl; gTex2D[%u].GetDimensions(0, tw, th, nl);", SrcN(2).c_str(), ci);
                if (t.unnormalized_coordinates()) L("  uv /= float2(tw, th);");
                L("  float lod = max(gTex2D[%u].CalculateLevelOfDetail(gSmp[%u], uv), 0.0f); float2 dm = float2(tw, th) * exp2(-floor(lod));", ci, ci);
                L("  float2 f = frac(uv * dm - 0.5f); float4 t = float4(f.x, f.y, 0.0f, frac(lod));");
                FetchWrite(t.dest(), t.is_dest_relative(), t.dest_swizzle(), "t");
                L("}");
                break;
            case FetchOpcode::kGetTextureBorderColorFrac:
                Unsupported("fetch opcode " + std::to_string((uint32_t)op));
                break;
            default: Unsupported("fetch opcode " + std::to_string((uint32_t)op)); break;
        }
        if (pred) { --ind; L("}"); }
    }

    void EmitExecBody(uint32_t addr, uint32_t count, uint32_t seq) {
        for (uint32_t k = 0; k < count; ++k, seq >>= 2) {
            const uint32_t* p = si.ucode.data() + (addr + k) * 3;
            if ((addr + k + 1) * 3 > si.ucode.size()) { Unsupported("exec fuera de rango"); return; }
            if (seq & 1) {
                const FetchInstruction& f = *reinterpret_cast<const FetchInstruction*>(p);
                if (f.opcode() == FetchOpcode::kVertexFetch) EmitVertexFetch(f.vertex_fetch());
                else EmitTextureFetch(f.texture_fetch());
            } else {
                EmitAlu(*reinterpret_cast<const AluInstruction*>(p));
            }
        }
    }

    bool Run(HlslResult& res) {
        // ---- decodificacion de flujo de control ----
        std::vector<ControlFlowInstruction> cf(si.cfPairBound * 2);
        for (uint32_t i = 0; i < si.cfPairBound; ++i) UnpackControlFlowInstructions(si.ucode.data() + i * 3, &cf[i * 2]);
        bool machine = false;
        for (auto& c : cf) {
            if (c.opcode() == ControlFlowOpcode::kCondJmp) machine = true;
            if (c.opcode() == ControlFlowOpcode::kCondCall || c.opcode() == ControlFlowOpcode::kReturn) { res.error = "kCondCall/kReturn no soportados"; return false; }
        }
        if (machine && si.loopCount) { res.error = "bucles junto a saltos condicionales no soportados"; return false; }
        numRegs = si.dynamicRegs ? 128 : std::max<uint32_t>(si.regBound, 1);
        if (!vs) numRegs = std::max<uint32_t>(numRegs, 16);
        // ---- cabecera ----
        out += kPrelude;
        out += "\nstatic float4 r[" + std::to_string(numRegs) + "];\n";
        out += "static float4 oPos, oI[16], oC[4], oPts;\nstatic float oDepthV;\nstatic int a0, aL;\nstatic bool p0;\nstatic float ps, xe_lod;\n";
        out += "static float3 xe_gradH, xe_gradV;\nstatic uint xe_vbase, xe_vidx;\n\nvoid xe_body()\n{\n";
        // ---- cuerpo ----
        std::vector<uint32_t> loopStack;
        if (machine) {
            L("uint pc = 0u;");
            L("while (true)");
            L("{");
            ++ind;
            L("switch (pc)");
            L("{");
            ++ind;
        }
        for (uint32_t i = 0; i < cf.size(); ++i) {
            const ControlFlowInstruction& c = cf[i];
            if (machine) L("case %uu:", i);
            uint32_t addr = 0, count = 0, seq = 0; bool end = false; int kind = 0; // 0 uncond, 1 bool, 2 pred
            bool cond = false; uint32_t boolIdx = 0;
            switch (c.opcode()) {
                case ControlFlowOpcode::kNop: case ControlFlowOpcode::kAlloc: case ControlFlowOpcode::kMarkVsFetchDone: continue;
                case ControlFlowOpcode::kExec: case ControlFlowOpcode::kExecEnd:
                    addr = c.exec.address(); count = c.exec.count(); seq = c.exec.sequence(); end = c.opcode() == ControlFlowOpcode::kExecEnd; break;
                case ControlFlowOpcode::kCondExec: case ControlFlowOpcode::kCondExecEnd:
                case ControlFlowOpcode::kCondExecPredClean: case ControlFlowOpcode::kCondExecPredCleanEnd:
                    addr = c.cond_exec.address(); count = c.cond_exec.count(); seq = c.cond_exec.sequence();
                    end = c.opcode() == ControlFlowOpcode::kCondExecEnd || c.opcode() == ControlFlowOpcode::kCondExecPredCleanEnd;
                    kind = 1; cond = c.cond_exec.condition(); boolIdx = c.cond_exec.bool_address(); break;
                case ControlFlowOpcode::kCondExecPred: case ControlFlowOpcode::kCondExecPredEnd:
                    addr = c.cond_exec_pred.address(); count = c.cond_exec_pred.count(); seq = c.cond_exec_pred.sequence();
                    end = c.opcode() == ControlFlowOpcode::kCondExecPredEnd; kind = 2; cond = c.cond_exec_pred.condition(); break;
                case ControlFlowOpcode::kLoopStart: {
                    uint32_t id = c.loop_start.loop_id();
                    L("{");
                    ++ind;
                    L("int aLsave%u = aL; uint lc%u = xe_loopc(%uu); uint cnt%u = lc%u & 0xFFu; int st%u = (int)((lc%u >> 8) & 0xFFu); int sp%u = ((int)(lc%u << 8)) >> 24;", i, i, id, i, i, i, i, i, i);
                    L("for (uint it%u = 0u; it%u < cnt%u; ++it%u)", i, i, i, i);
                    L("{");
                    ++ind;
                    if (c.loop_start.is_repeat()) L("aL = aLsave%u + (int)it%u * sp%u;", i, i, i);
                    else L("aL = st%u + (int)it%u * sp%u;", i, i, i);
                    loopStack.push_back(i);
                    continue;
                }
                case ControlFlowOpcode::kLoopEnd: {
                    if (loopStack.empty()) { res.error = "kLoopEnd sin kLoopStart"; return false; }
                    uint32_t s = loopStack.back(); loopStack.pop_back();
                    if (c.loop_end.is_predicated_break()) L("if (p0 == %s) break;", c.loop_end.condition() ? "true" : "false");
                    --ind;
                    L("}");
                    L("aL = aLsave%u;", s);
                    --ind;
                    L("}");
                    continue;
                }
                case ControlFlowOpcode::kCondJmp: {
                    const auto& j = c.cond_jmp;
                    if (j.is_unconditional()) L("pc = %uu; continue;", j.address());
                    else if (j.is_predicated()) L("if (p0 == %s) { pc = %uu; continue; }", j.condition() ? "true" : "false", j.address());
                    else L("if (xe_bool(%uu) == %s) { pc = %uu; continue; }", j.bool_address(), j.condition() ? "true" : "false", j.address());
                    continue;
                }
                default: Unsupported("instruccion de flujo de control " + std::to_string((uint32_t)c.opcode())); continue;
            }
            // exec
            bool wrap = kind != 0;
            if (kind == 1) L("if (xe_bool(%uu) == %s)", boolIdx, cond ? "true" : "false");
            else if (kind == 2) L("if (p0 == %s)", cond ? "true" : "false");
            L("{");
            ++ind;
            EmitExecBody(addr, count, seq);
            if (end) L("return;");
            --ind;
            L("}");
            (void)wrap;
        }
        if (!loopStack.empty()) { res.error = "bucle sin cerrar"; return false; }
        if (machine) {
            L("default: break;");
            --ind; L("}");
            L("break;");
            --ind; L("}");
        }
        out += "}\n\n";
        EmitEntry();
        res.hlsl = out;
        res.unsupportedCount = unsupported;
        if (unsupported) { res.error = "instrucciones no soportadas (" + std::to_string(unsupported) + "): " + firstError; return false; }
        res.ok = true;
        return true;
    }

    void EmitEntry() {
        char b[512];
        if (vs) {
            out += "struct VSOut\n{\n    float4 pos : SV_Position;\n";
            for (int i = 0; i < 16; ++i) { std::snprintf(b, sizeof(b), "    float4 i%d : TEXCOORD%d;\n", i, i); out += b; }
            out += "};\n\nVSOut main(uint vid : SV_VertexID)\n{\n";
            out += "    for (uint k = 0; k < " + std::to_string(numRegs) + "u; ++k) r[k] = float4(0.0f, 0.0f, 0.0f, 0.0f);\n";
            out += "    oPos = float4(0.0f, 0.0f, 0.0f, 1.0f); oPts = float4(0.0f, 0.0f, 0.0f, 0.0f); a0 = 0; aL = 0; p0 = false; ps = 0.0f; xe_lod = 0.0f;\n";
            out += "    xe_gradH = float3(0, 0, 0); xe_gradV = float3(0, 0, 0); xe_vbase = 0; xe_vidx = 0;\n";
            out += "    for (uint q = 0; q < 16u; ++q) oI[q] = float4(0.0f, 0.0f, 0.0f, 0.0f);\n";
            out += "    r[0] = float4((float)xe_vertex_index(vid), 0.0f, 0.0f, 0.0f);\n";
            out += "    xe_body();\n    VSOut o;\n";
            out += "    float4 pp = oPos;\n";
            out += "    if ((gDraw2.z & 4u) != 0u) pp.w = 1.0f / pp.w;\n";
            out += "    if ((gDraw2.z & 1u) != 0u) pp.xy *= pp.w;\n";
            out += "    if ((gDraw2.z & 2u) != 0u) pp.z *= pp.w;\n";
            out += "    pp.xyz = pp.xyz * gNdcScale.xyz + gNdcOffset.xyz * pp.w;\n";
            out += "    o.pos = pp;\n";
            for (int i = 0; i < 16; ++i) { std::snprintf(b, sizeof(b), "    o.i%d = oI[%d];\n", i, i); out += b; }
            out += "    return o;\n}\n";
        } else {
            out += "struct PSIn\n{\n    float4 pos : SV_Position;\n";
            for (int i = 0; i < 16; ++i) { std::snprintf(b, sizeof(b), "    float4 i%d : TEXCOORD%d;\n", i, i); out += b; }
            out += "    bool front : SV_IsFrontFace;\n};\n\nstruct PSOut\n{\n";
            for (int i = 0; i < 4; ++i)
                if (si.writesColors & (1u << i)) { std::snprintf(b, sizeof(b), "    float4 c%d : SV_Target%d;\n", i, i); out += b; }
            if (si.writesDepth) out += "    float depth : SV_Depth;\n";
            if (!si.writesColors && !si.writesDepth) out += "    float4 c0 : SV_Target0;\n";
            out += "};\n\nPSOut main(PSIn In)\n{\n";
            out += "    for (uint k = 0; k < " + std::to_string(numRegs) + "u; ++k) r[k] = float4(0.0f, 0.0f, 0.0f, 0.0f);\n";
            out += "    for (uint q = 0; q < 4u; ++q) oC[q] = float4(0.0f, 0.0f, 0.0f, 0.0f);\n";
            out += "    oDepthV = 0.0f; a0 = 0; aL = 0; p0 = false; ps = 0.0f; xe_lod = 0.0f; xe_gradH = float3(0, 0, 0); xe_gradV = float3(0, 0, 0);\n";
            for (int i = 0; i < 16; ++i) {
                if ((uint32_t)i >= numRegs) break;
                std::snprintf(b, sizeof(b), "    r[%d] = In.i%d;\n", i, i); out += b;
            }
            if (opt.paramGenPos >= 0 && (uint32_t)opt.paramGenPos < numRegs) {
                // SQ_CONTEXT_MISC.param_gen_pos: XY = posicion en pantalla entera, signo de X = cara trasera (ucode: registers.h)
                std::snprintf(b, sizeof(b), "    r[%d] = float4(asfloat(asuint(floor(In.pos.x)) | (In.front ? 0u : 0x80000000u)), floor(In.pos.y), 0.0f, 0.0f);\n", opt.paramGenPos);
                out += b;
            }
            out += "    xe_body();\n";
            out += "    if ((gDraw2.x & 8u) != 0u)\n    {\n        float ar = asfloat(gDraw2.y), av = oC[0].w; uint f = gDraw2.x & 7u; bool pass;\n";
            out += "        if (f == 0u) pass = false; else if (f == 1u) pass = av < ar; else if (f == 2u) pass = av == ar; else if (f == 3u) pass = av <= ar;\n";
            out += "        else if (f == 4u) pass = av > ar; else if (f == 5u) pass = av != ar; else if (f == 6u) pass = av >= ar; else pass = true;\n";
            out += "        if (!pass) discard;\n    }\n";
            out += "    PSOut o;\n";
            for (int i = 0; i < 4; ++i)
                if (si.writesColors & (1u << i)) { std::snprintf(b, sizeof(b), "    o.c%d = oC[%d];\n", i, i); out += b; }
            if (si.writesDepth) out += "    o.depth = oDepthV;\n";
            if (!si.writesColors && !si.writesDepth) out += "    o.c0 = float4(0.0f, 0.0f, 0.0f, 0.0f);\n";
            out += "    return o;\n}\n";
        }
    }
};

} // namespace

HlslResult TranslateToHlsl(const ShaderInfo& si, const HlslOptions& opt) {
    HlslResult res;
    Emitter e(si, opt);
    e.Run(res);
    if (res.hlsl.empty()) res.hlsl = e.out;
    return res;
}

} // namespace rt::gpu
