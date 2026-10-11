// --selftest-cpu: ver cpu_selftest.h.
//
// Fuentes del modelo de referencia (todas en runtime\ref\, no se distribuyen):
//  - ALTIVECPEM Rev.3 (NXP/Freescale, 04/2006), cap. 6 "AltiVec Instructions" (pseudocodigo de cada instruccion VMX) y
//    3.2.5.4 "NaN Conversion to Integer" (los NaN convierten a 0 en vctuxs/vctsxs).
//  - Power ISA v2.07B (OpenPOWER, 2018), Libro I, modo de 64 bits (MSR[SF]=1): 3.3.2/3.3.3 (cargas/almacenamientos con actualizacion),
//    3.3.5 (lhbrx), 3.3.8 (CR0 con Rc), 3.3.9 (addc, addme, subfze, mulhdu; CA = acarreo del bit 0), 3.3.13 (eqv), 4.6.2/4.6.3 (lfs/stfs),
//    4.6.6 (frsqrte: error relativo <= 1/32), capitulo de Branch Facility (bc: ctr_ok = BO2 | ((CTR[M:63] != 0) xor BO3), M=0 en 64 bits;
//    bclr/bclrl: NIA <- LR[0:61]||0b00).
//  - VMX128 (vsel128, vsro128, vnor128, vpkswss128, vcfpuxws128, vsldoi128, vupkd3d128, vpkd3d128): no existe especificacion publica;
//    modelo tomado de Xenia (ppc_emit_altivec.cc, x64_seq_vector.cc PACK/UNPACK FLOAT16_2/FLOAT16_4, ppc_decode_data.h). Esta
//    limitacion esta documentada en PROGRESS.md.
// El estado completo (GPR, FPR, 128 VR, CR, XER, CTR, LR y ventana de memoria) se compara tras cada caso.
#include "cpu_selftest.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <immintrin.h>
#include <map>
#include <string>
#include <vector>
#include <malloc.h>

#include "cpu_ctx_map.inc"
#include "cpu_xenia_tables.inc"

namespace {

constexpr uint32_t MEM_LO = 0x40000, MEM_SZ = 0x400, HOST_SZ = 0x50000;

struct VR { uint8_t b[16]; };   // orden PPC: b[0] = byte mas significativo (bits 0-7)

struct St {
    uint64_t g[32], f[32];
    VR v[128];
    uint8_t cr[8];              // por campo: LT=8 GT=4 EQ=2 SO=1
    uint8_t ca, so, ov;
    uint64_t ctr, lr;
    uint8_t mem[MEM_SZ];        // memoria guest [MEM_LO, MEM_LO+MEM_SZ), big-endian
};

// ---------------- RNG ----------------
struct Rng {
    uint64_t s;
    explicit Rng(uint64_t seed) : s(seed) {}
    uint64_t next() { uint64_t z = (s += 0x9E3779B97F4A7C15ull); z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull; z = (z ^ (z >> 27)) * 0x94D049BB133111EBull; return z ^ (z >> 31); }
    uint32_t u32() { return uint32_t(next() >> 32); }
    uint64_t below(uint64_t n) { return next() % n; }
};

// ---------------- limites ----------------
const uint64_t E64[] = { 0, 1, 2, 0x7FFFFFFFull, 0x80000000ull, 0xFFFFFFFFull, 0x100000000ull, 0x100000001ull, 0x7FFFFFFFFFFFFFFFull,
                         0x8000000000000000ull, 0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFF00000000ull, 0x00000000FFFFFFFEull };
const uint32_t E32[] = { 0, 1, 2, 0x7F, 0x80, 0xFF, 0x7FFF, 0x8000, 0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0x80000001, 0xFFFFFFFE, 0xFFFFFFFF,
                         0x00010001, 0xFFFF0000, 0x0000FFFF, 0x7FFF7FFF, 0x80008000, 0x01010101, 0x80808080, 0x7F7F7F7F, 0xFEFEFEFE };
const uint16_t E16[] = { 0, 1, 2, 0x7FFF, 0x8000, 0x8001, 0xFFFF, 0xFFFE, 0x00FF, 0xFF00, 0x1234, 0xFFF0, 0x000F, 0x0010 };
const uint8_t E8[] = { 0, 1, 2, 0x7F, 0x80, 0x81, 0xFF, 0xFE, 0x0F, 0x10, 0xF0, 0x08 };
const uint32_t EF[] = { 0x00000000, 0x80000000, 0x3F800000, 0xBF800000, 0x00000001, 0x80000001, 0x007FFFFF, 0x807FFFFF, 0x00800000, 0x7F7FFFFF,
                        0xFF7FFFFF, 0x7F800000, 0xFF800000, 0x7FC00000, 0xFFC00001, 0x7F800001, 0xFF800001, 0x4F000000, 0x4F800000, 0x4F7FFFFF,
                        0x4EFFFFFF, 0x3F000000, 0xBF000000, 0x3F7FFFFF, 0xCF000000, 0xCF000001, 0x477FE000, 0x477FF000, 0xC77FE000, 0x38800000,
                        0x33800000, 0x33000000, 0x3F800001, 0x4B800000, 0x5F800000, 0x1F800000 };
const uint64_t ED[] = { 0x0000000000000000ull, 0x8000000000000000ull, 0x3FF0000000000000ull, 0xBFF0000000000000ull, 0x4010000000000000ull,
                        0x3FD0000000000000ull, 0x0000000000000001ull, 0x000FFFFFFFFFFFFFull, 0x0010000000000000ull, 0x7FEFFFFFFFFFFFFFull,
                        0x7FF0000000000000ull, 0xFFF0000000000000ull, 0x7FF8000000000000ull, 0x7FF0000000000001ull, 0xFFF8000000000001ull,
                        0x2B00000000000000ull, 0x3B00000000000000ull, 0x5B00000000000000ull, 0x7FE0000000000000ull, 0x0020000000000000ull,
                        0x3E90000000000000ull, 0x4680000000000000ull, 0x1D00000000000000ull };
template <class T, size_t N> constexpr size_t cnt(const T (&)[N]) { return N; }

// ---------------- campos de instruccion ----------------
inline uint32_t fRT(uint32_t w) { return (w >> 21) & 31; }
inline uint32_t fRA(uint32_t w) { return (w >> 16) & 31; }
inline uint32_t fRB(uint32_t w) { return (w >> 11) & 31; }
inline uint32_t fRc(uint32_t w) { return w & 1; }
inline int32_t fD(uint32_t w) { return int16_t(w & 0xFFFF); }
// VX/VA (ALTIVECPEM cap. 6)
inline uint32_t vD(uint32_t w) { return (w >> 21) & 31; }
inline uint32_t vA(uint32_t w) { return (w >> 16) & 31; }
inline uint32_t vB(uint32_t w) { return (w >> 11) & 31; }
inline uint32_t vC(uint32_t w) { return (w >> 6) & 31; }
// VMX128 (Xenia ppc_decode_data.h FormatVX128*)
inline uint32_t xD(uint32_t w) { return ((w >> 21) & 31) | (((w >> 2) & 3) << 5); }
inline uint32_t xA(uint32_t w) { return ((w >> 16) & 31) | (((w >> 5) & 1) << 5) | (((w >> 10) & 1) << 6); }
inline uint32_t xB(uint32_t w) { return ((w >> 11) & 31) | ((w & 3) << 5); }

// ---------------- vectores (orden PPC) ----------------
inline uint32_t W(const VR& v, int i) { return uint32_t(v.b[4 * i]) << 24 | v.b[4 * i + 1] << 16 | v.b[4 * i + 2] << 8 | v.b[4 * i + 3]; }
inline void SW(VR& v, int i, uint32_t x) { v.b[4 * i] = x >> 24; v.b[4 * i + 1] = x >> 16; v.b[4 * i + 2] = x >> 8; v.b[4 * i + 3] = x; }
inline uint16_t H(const VR& v, int i) { return uint16_t(v.b[2 * i] << 8 | v.b[2 * i + 1]); }
inline void SH(VR& v, int i, uint16_t x) { v.b[2 * i] = x >> 8; v.b[2 * i + 1] = x; }

inline int16_t sat16(int64_t x) { return x > 32767 ? 32767 : x < -32768 ? -32768 : int16_t(x); }
inline int32_t sat32(int64_t x) { return x > 2147483647ll ? 2147483647 : x < -2147483648ll ? int32_t(-2147483647 - 1) : int32_t(x); }

// ---------------- memoria ----------------
inline uint32_t mrd(const St& s, uint64_t ea, int n) {
    uint32_t x = 0; for (int i = 0; i < n; i++) x = x << 8 | s.mem[uint32_t(ea) - MEM_LO + i]; return x;
}
inline uint64_t mrd64(const St& s, uint64_t ea) { return uint64_t(mrd(s, ea, 4)) << 32 | mrd(s, ea + 4, 4); }
inline void mwr(St& s, uint64_t ea, int n, uint64_t x) { for (int i = 0; i < n; i++) s.mem[uint32_t(ea) - MEM_LO + i] = uint8_t(x >> (8 * (n - 1 - i))); }

// ---------------- coma flotante ----------------
// lfs (Power ISA 4.6.2): single (32 bits) -> double (64 bits), sin redondeo; denormales normalizados.
uint64_t sp2dp(uint32_t w) {
    uint64_t s = uint64_t(w >> 31) << 63; uint32_t e = (w >> 23) & 0xFF, m = w & 0x7FFFFF;
    if (e > 0 && e < 255) return s | (uint64_t(e) - 127 + 1023) << 52 | uint64_t(m) << 29;
    if (e == 0 && m == 0) return s;
    if (e == 0) { int b = 31 - __builtin_clz(m); return s | (uint64_t(b) + 874) << 52 | ((uint64_t(m) << (52 - b)) & 0xFFFFFFFFFFFFFull); }
    return s | 0x7FFull << 52 | uint64_t(m) << 29;
}
// stfs (Power ISA 4.6.3): double -> single sin redondeo para valores representables (el resto es indefinido segun el ISA).
bool dp2sp(uint64_t d, uint32_t& out) {
    uint32_t exp = (d >> 52) & 0x7FF;
    if (exp > 896 || (d & 0x7FFFFFFFFFFFFFFFull) == 0) { out = uint32_t((d >> 32) & 0xC0000000u) | uint32_t((d >> 29) & 0x3FFFFFFFu); return true; }
    if (exp >= 874) {
        uint32_t sh = 897 - exp; uint64_t frac = (1ull << 52) | (d & 0xFFFFFFFFFFFFFull);
        out = uint32_t(d >> 63) << 31 | uint32_t(((frac >> sh) >> 29) & 0x7FFFFF); return true;
    }
    return false;
}
inline bool isnan64(uint64_t x) { return (x & 0x7FF0000000000000ull) == 0x7FF0000000000000ull && (x & 0xFFFFFFFFFFFFFull); }
inline bool isnan32(uint32_t x) { return (x & 0x7F800000u) == 0x7F800000u && (x & 0x7FFFFFu); }

// ---------------- estado esperado ----------------
struct Exp {
    St s;
    bool lvehx = false; uint32_t lvehx_vd = 0, lvehx_eb = 0;   // solo 2 bytes definidos
    bool stfs_nan_lenient = false; uint32_t stfs_off = 0;      // palabra de memoria en la que un NaN vale cualquier NaN
    bool check_mem = false;
    bool blrl = false; uint32_t blrl_target = 0;
    bool frsqrte = false; uint32_t frt = 0; uint64_t frsqrte_in = 0;
};

void set_cr0(St& s, int64_t t) { s.cr[0] = (t < 0 ? 8 : 0) | (t > 0 ? 4 : 0) | (t == 0 ? 2 : 0) | s.so; }

enum K {
    K_ADDC, K_ADDME, K_SUBFZE, K_EQV, K_MULHDU,
    K_LHZU, K_LHAU, K_STHU, K_LFSU, K_LFDU, K_STFSU, K_STFDU,
    K_LBZUX, K_LHZUX, K_LWZUX, K_LDUX, K_STBUX, K_STHUX, K_STDUX, K_LFSUX, K_STFSUX,
    K_LHBRX, K_LVEHX, K_DCBST, K_FRSQRTE, K_BDZF, K_BDNZT, K_BLRL,
    K_VNOR, K_VANDC, K_VSLH, K_VSRH, K_VSRAH, K_VSRO, K_VMINSW, K_VMINUH, K_VMAXUH, K_VAVGUH, K_VSUBUWM, K_VSUBUBM, K_VSUBSHS, K_VADDSWS,
    K_VPKSWSS, K_VCMPGTSB, K_VCMPGTSW, K_VCMPGTUW, K_VSPLTISH, K_VCTUXS, K_VSEL, K_VSLDOI,
    K_VNOR128, K_VSRO128, K_VPKSWSS128, K_VSEL128, K_VCFPUXWS128, K_VSLDOI128, K_VUPKD3D128_5, K_VPKD3D128_3, K_VPKD3D128_5,
    K_COUNT
};

struct KDef { const char* name; K k; bool rc; };
const KDef KDEFS[] = {
    {"addc", K_ADDC, 0}, {"addme", K_ADDME, 0}, {"subfze", K_SUBFZE, 0}, {"eqv", K_EQV, 0}, {"mulhdu", K_MULHDU, 0},
    {"lhzu", K_LHZU, 0}, {"lhau", K_LHAU, 0}, {"sthu", K_STHU, 0}, {"lfsu", K_LFSU, 0}, {"lfdu", K_LFDU, 0}, {"stfsu", K_STFSU, 0}, {"stfdu", K_STFDU, 0},
    {"lbzux", K_LBZUX, 0}, {"lhzux", K_LHZUX, 0}, {"lwzux", K_LWZUX, 0}, {"ldux", K_LDUX, 0}, {"stbux", K_STBUX, 0}, {"sthux", K_STHUX, 0},
    {"stdux", K_STDUX, 0}, {"lfsux", K_LFSUX, 0}, {"stfsux", K_STFSUX, 0},
    {"lhbrx", K_LHBRX, 0}, {"lvehx", K_LVEHX, 0}, {"dcbst", K_DCBST, 0}, {"frsqrte", K_FRSQRTE, 0}, {"bdzf", K_BDZF, 0}, {"bdnzt", K_BDNZT, 0}, {"blrl", K_BLRL, 0},
    {"vnor", K_VNOR, 0}, {"vandc", K_VANDC, 0}, {"vslh", K_VSLH, 0}, {"vsrh", K_VSRH, 0}, {"vsrah", K_VSRAH, 0}, {"vsro", K_VSRO, 0}, {"vminsw", K_VMINSW, 0},
    {"vminuh", K_VMINUH, 0}, {"vmaxuh", K_VMAXUH, 0}, {"vavguh", K_VAVGUH, 0}, {"vsubuwm", K_VSUBUWM, 0}, {"vsububm", K_VSUBUBM, 0}, {"vsubshs", K_VSUBSHS, 0},
    {"vaddsws", K_VADDSWS, 0}, {"vpkswss", K_VPKSWSS, 0},
    {"vcmpgtsb", K_VCMPGTSB, 0}, {"vcmpgtsb.", K_VCMPGTSB, 1}, {"vcmpgtsw", K_VCMPGTSW, 0}, {"vcmpgtsw.", K_VCMPGTSW, 1},
    {"vcmpgtuw", K_VCMPGTUW, 0}, {"vcmpgtuw.", K_VCMPGTUW, 1},
    {"vspltish", K_VSPLTISH, 0}, {"vctuxs", K_VCTUXS, 0}, {"vsel", K_VSEL, 0}, {"vsldoi", K_VSLDOI, 0},
    {"vnor128", K_VNOR128, 0}, {"vsro128", K_VSRO128, 0}, {"vpkswss128", K_VPKSWSS128, 0}, {"vsel128", K_VSEL128, 0}, {"vcfpuxws128", K_VCFPUXWS128, 0},
    {"vsldoi128", K_VSLDOI128, 0}, {"vupkd3d128_5", K_VUPKD3D128_5, 0}, {"vpkd3d128_3", K_VPKD3D128_3, 0}, {"vpkd3d128_5", K_VPKD3D128_5, 0},
};

inline float f32of(uint32_t b) { float f; std::memcpy(&f, &b, 4); return f; }

// vctuxs (ALTIVECPEM): x = vB * 2^UIMM, redondeo hacia cero, saturacion [0, 2^32-1]; NaN -> 0 (ALTIVECPEM 3.2.5.4).
uint32_t ctuxs(uint32_t fb, uint32_t uimm) {
    if (isnan32(fb)) return 0;
    long double x = (long double)f32of(fb) * std::ldexp(1.0L, int(uimm));
    if (x >= 4294967296.0L) return 0xFFFFFFFFu;
    if (x <= 0) return 0;
    return uint32_t(std::truncl(x));
}

// ---------------- modelo ----------------
void model(K k, bool rc, uint32_t w, uint32_t addr, const St& in, Exp& e) {
    St& o = e.s;
    o = in;
    if (k <= K_MULHDU) rc = fRc(w);          // familias enteras XO/X-form: Rc es el bit 31 de la palabra
    auto cmp64 = [&](uint64_t t) { if (rc) set_cr0(o, int64_t(t)); };
    switch (k) {
    // ---- enteros (Power ISA v2.07B Libro I, modo de 64 bits) ----
    case K_ADDC: { uint64_t a = in.g[fRA(w)], b = in.g[fRB(w)], t = a + b; o.ca = t < a; o.g[fRT(w)] = t; cmp64(t); break; }
    case K_ADDME: {                       // RT <- (RA) + CA - 1 ; CA = acarreo del bit 0 de (RA)+CA+(2^64-1)
        unsigned __int128 s = (unsigned __int128)in.g[fRA(w)] + in.ca + 0xFFFFFFFFFFFFFFFFull;
        uint64_t t = uint64_t(s); o.ca = uint8_t(s >> 64); o.g[fRT(w)] = t; cmp64(t); break; }
    case K_SUBFZE: {                      // RT <- NOT(RA) + CA
        unsigned __int128 s = (unsigned __int128)(~in.g[fRA(w)]) + in.ca;
        uint64_t t = uint64_t(s); o.ca = uint8_t(s >> 64); o.g[fRT(w)] = t; cmp64(t); break; }
    case K_EQV: { uint64_t t = ~(in.g[fRT(w)] ^ in.g[fRB(w)]); o.g[fRA(w)] = t; cmp64(t); break; }   // RA <- NOT(RS xor RB), RS = bits 21-25
    case K_MULHDU: { uint64_t t = uint64_t(((unsigned __int128)in.g[fRA(w)] * in.g[fRB(w)]) >> 64); o.g[fRT(w)] = t; cmp64(t); break; }
    // ---- cargas/almacenamiento con actualizacion (D-form) ----
    case K_LHZU: case K_LHAU: case K_STHU: case K_LFSU: case K_LFDU: case K_STFSU: case K_STFDU: {
        uint64_t ea = in.g[fRA(w)] + int64_t(fD(w)); uint32_t rt = fRT(w);
        if (k == K_LHZU) o.g[rt] = mrd(in, ea, 2);
        else if (k == K_LHAU) o.g[rt] = uint64_t(int64_t(int16_t(mrd(in, ea, 2))));
        else if (k == K_STHU) mwr(o, ea, 2, in.g[rt]);
        else if (k == K_LFSU) o.f[rt] = sp2dp(mrd(in, ea, 4));
        else if (k == K_LFDU) o.f[rt] = mrd64(in, ea);
        else if (k == K_STFSU) { uint32_t sp = 0; dp2sp(in.f[rt], sp); mwr(o, ea, 4, sp); e.stfs_nan_lenient = true; e.stfs_off = uint32_t(ea) - MEM_LO; }
        else mwr(o, ea, 8, in.f[rt]);
        o.g[fRA(w)] = ea; e.check_mem = true; break; }
    // ---- X-form con actualizacion ----
    case K_LBZUX: case K_LHZUX: case K_LWZUX: case K_LDUX: case K_STBUX: case K_STHUX: case K_STDUX: case K_LFSUX: case K_STFSUX: {
        uint64_t ea = in.g[fRA(w)] + in.g[fRB(w)]; uint32_t rt = fRT(w);
        switch (k) {
        case K_LBZUX: o.g[rt] = mrd(in, ea, 1); break;
        case K_LHZUX: o.g[rt] = mrd(in, ea, 2); break;
        case K_LWZUX: o.g[rt] = mrd(in, ea, 4); break;
        case K_LDUX: o.g[rt] = mrd64(in, ea); break;
        case K_STBUX: mwr(o, ea, 1, in.g[rt]); break;
        case K_STHUX: mwr(o, ea, 2, in.g[rt]); break;
        case K_STDUX: mwr(o, ea, 8, in.g[rt]); break;
        case K_LFSUX: o.f[rt] = sp2dp(mrd(in, ea, 4)); break;
        default: { uint32_t sp = 0; dp2sp(in.f[rt], sp); mwr(o, ea, 4, sp); e.stfs_nan_lenient = true; e.stfs_off = uint32_t(ea) - MEM_LO; } break;
        }
        o.g[fRA(w)] = ea; e.check_mem = true; break; }
    case K_LHBRX: {                       // RT <- 48 ceros || media palabra invertida (Power ISA 3.3.5)
        uint64_t ea = (fRA(w) ? in.g[fRA(w)] : 0) + in.g[fRB(w)]; uint32_t h = mrd(in, ea, 2);
        o.g[fRT(w)] = ((h & 0xFF) << 8) | (h >> 8); e.check_mem = true; break; }
    case K_LVEHX: {                       // ALTIVECPEM lvehx: eb = EA[28:30]||0 ; vD[eb*8:eb*8+15] <- MEM(EA&~1, 2); resto indefinido
        uint64_t ea = (fRA(w) ? in.g[fRA(w)] : 0) + in.g[fRB(w)]; uint32_t eb = uint32_t(ea) & 0xE;
        uint32_t h = mrd(in, ea & ~1ull, 2);
        o.v[fRT(w)].b[eb] = h >> 8; o.v[fRT(w)].b[eb + 1] = h & 0xFF;
        e.lvehx = true; e.lvehx_vd = fRT(w); e.lvehx_eb = eb; e.check_mem = true; break; }
    case K_DCBST: e.check_mem = true; break;
    case K_FRSQRTE: e.frsqrte = true; e.frt = fRT(w); e.frsqrte_in = in.f[fRB(w)]; break;
    case K_BDZF: case K_BDNZT: {          // bc (Power ISA 2.4.1): 64 bits, M = 0
        uint32_t BO = fRT(w), BI = fRA(w);
        uint32_t bo0 = (BO >> 4) & 1, bo1 = (BO >> 3) & 1, bo2 = (BO >> 2) & 1, bo3 = (BO >> 1) & 1;
        uint64_t ctr = in.ctr; if (!bo2) ctr--;
        bool ctr_ok = bo2 || ((ctr != 0) ^ bo3);
        uint32_t bit = (in.cr[BI / 4] >> (3 - BI % 4)) & 1;
        bool cond_ok = bo0 || (bit == bo1);
        o.ctr = ctr;
        if (!(ctr_ok && cond_ok)) o.g[3] = in.g[3] + 1;     // la instruccion siguiente (addi r3,r3,1) solo se ejecuta si no se salta
        break; }
    case K_BLRL:                          // bclrl BO=20: NIA <- LR[0:61]||0b00 ; LR <- CIA+4 (Power ISA 2.4.2)
        e.blrl = true; e.blrl_target = uint32_t(in.lr) & ~3u; o.lr = addr + 4; break;

    // ---- VMX (ALTIVECPEM cap. 6) ----
    case K_VNOR: case K_VNOR128: {
        uint32_t d = k == K_VNOR ? vD(w) : xD(w), a = k == K_VNOR ? vA(w) : xA(w), b = k == K_VNOR ? vB(w) : xB(w);
        for (int i = 0; i < 16; i++) o.v[d].b[i] = ~(in.v[a].b[i] | in.v[b].b[i]); break; }
    case K_VANDC: for (int i = 0; i < 16; i++) o.v[vD(w)].b[i] = in.v[vA(w)].b[i] & ~in.v[vB(w)].b[i]; break;
    case K_VSLH: for (int i = 0; i < 8; i++) SH(o.v[vD(w)], i, uint16_t(H(in.v[vA(w)], i) << (H(in.v[vB(w)], i) & 15))); break;
    case K_VSRH: for (int i = 0; i < 8; i++) SH(o.v[vD(w)], i, uint16_t(H(in.v[vA(w)], i) >> (H(in.v[vB(w)], i) & 15))); break;
    case K_VSRAH: for (int i = 0; i < 8; i++) SH(o.v[vD(w)], i, uint16_t(int16_t(H(in.v[vA(w)], i)) >> (H(in.v[vB(w)], i) & 15))); break;
    case K_VSRO: case K_VSRO128: {        // shb <- (vB)[121:124] ; vD <- (vA) >>ui (shb||0b000)
        uint32_t d = k == K_VSRO ? vD(w) : xD(w), a = k == K_VSRO ? vA(w) : xA(w), b = k == K_VSRO ? vB(w) : xB(w);
        int shb = (in.v[b].b[15] >> 3) & 0xF;
        for (int i = 0; i < 16; i++) o.v[d].b[i] = i - shb >= 0 ? in.v[a].b[i - shb] : 0; break; }
    case K_VMINSW: for (int i = 0; i < 4; i++) { int32_t a = int32_t(W(in.v[vA(w)], i)), b = int32_t(W(in.v[vB(w)], i)); SW(o.v[vD(w)], i, uint32_t(a < b ? a : b)); } break;
    case K_VMINUH: for (int i = 0; i < 8; i++) { uint16_t a = H(in.v[vA(w)], i), b = H(in.v[vB(w)], i); SH(o.v[vD(w)], i, a < b ? a : b); } break;
    case K_VMAXUH: for (int i = 0; i < 8; i++) { uint16_t a = H(in.v[vA(w)], i), b = H(in.v[vB(w)], i); SH(o.v[vD(w)], i, a > b ? a : b); } break;
    case K_VAVGUH: for (int i = 0; i < 8; i++) SH(o.v[vD(w)], i, uint16_t((uint32_t(H(in.v[vA(w)], i)) + H(in.v[vB(w)], i) + 1) >> 1)); break;
    case K_VSUBUWM: for (int i = 0; i < 4; i++) SW(o.v[vD(w)], i, W(in.v[vA(w)], i) - W(in.v[vB(w)], i)); break;
    case K_VSUBUBM: for (int i = 0; i < 16; i++) o.v[vD(w)].b[i] = uint8_t(in.v[vA(w)].b[i] - in.v[vB(w)].b[i]); break;
    case K_VSUBSHS: for (int i = 0; i < 8; i++) SH(o.v[vD(w)], i, uint16_t(sat16(int64_t(int16_t(H(in.v[vA(w)], i))) - int16_t(H(in.v[vB(w)], i))))); break;
    case K_VADDSWS: for (int i = 0; i < 4; i++) SW(o.v[vD(w)], i, uint32_t(sat32(int64_t(int32_t(W(in.v[vA(w)], i))) + int32_t(W(in.v[vB(w)], i))))); break;
    case K_VPKSWSS: case K_VPKSWSS128: {
        uint32_t d = k == K_VPKSWSS ? vD(w) : xD(w), a = k == K_VPKSWSS ? vA(w) : xA(w), b = k == K_VPKSWSS ? vB(w) : xB(w);
        for (int i = 0; i < 4; i++) { SH(o.v[d], i, uint16_t(sat16(int32_t(W(in.v[a], i))))); SH(o.v[d], i + 4, uint16_t(sat16(int32_t(W(in.v[b], i))))); } break; }
    case K_VCMPGTSB: case K_VCMPGTSW: case K_VCMPGTUW: {
        VR& r = o.v[vD(w)]; const VR& a = in.v[vA(w)]; const VR& b = in.v[vB(w)]; bool all1 = true, all0 = true;
        if (k == K_VCMPGTSB) for (int i = 0; i < 16; i++) { bool t = int8_t(a.b[i]) > int8_t(b.b[i]); r.b[i] = t ? 0xFF : 0; all1 &= t; all0 &= !t; }
        else for (int i = 0; i < 4; i++) {
            bool t = k == K_VCMPGTSW ? int32_t(W(a, i)) > int32_t(W(b, i)) : W(a, i) > W(b, i);
            SW(r, i, t ? 0xFFFFFFFFu : 0); all1 &= t; all0 &= !t; }
        if (rc) o.cr[6] = (all1 ? 8 : 0) | (all0 ? 2 : 0);       // CR6 = {vD==ones, 0, vD==zeros, 0}
        break; }
    case K_VSPLTISH: { int simm = int((vA(w) ^ 16) - 16); for (int i = 0; i < 8; i++) SH(o.v[vD(w)], i, uint16_t(int16_t(simm))); break; }
    case K_VCTUXS: for (int i = 0; i < 4; i++) SW(o.v[vD(w)], i, ctuxs(W(in.v[vB(w)], i), vA(w))); break;
    case K_VCFPUXWS128: for (int i = 0; i < 4; i++) SW(o.v[xD(w)], i, ctuxs(W(in.v[xB(w)], i), (w >> 16) & 31)); break;
    case K_VSEL: case K_VSEL128: {        // vD <- (vA & ~vC) | (vB & vC); vsel128: vC = vD previo (Xenia InstrEmit_vsel128)
        uint32_t d = k == K_VSEL ? vD(w) : xD(w), a = k == K_VSEL ? vA(w) : xA(w), b = k == K_VSEL ? vB(w) : xB(w), c = k == K_VSEL ? vC(w) : xD(w);
        for (int i = 0; i < 16; i++) o.v[d].b[i] = (in.v[a].b[i] & ~in.v[c].b[i]) | (in.v[b].b[i] & in.v[c].b[i]); break; }
    case K_VSLDOI: case K_VSLDOI128: {    // vD <- ((vA)||(vB)) << (SH*8) [0:127]
        uint32_t d = k == K_VSLDOI ? vD(w) : xD(w), a = k == K_VSLDOI ? vA(w) : xA(w), b = k == K_VSLDOI ? vB(w) : xB(w);
        uint32_t sh = (w >> 6) & 15;
        for (int i = 0; i < 16; i++) o.v[d].b[i] = i + sh < 16 ? in.v[a].b[i + sh] : in.v[b].b[i + sh - 16]; break; }
    case K_VUPKD3D128_5: {                // Xenia UNPACK FLOAT16_4: X,Y,Z,W = medias palabras PPC 4..7 de vB (binary16 -> binary32)
        const VR b = in.v[xB(w)]; VR r;
        for (int i = 0; i < 4; i++) SW(r, i, [&] { float f = _cvtsh_ss(H(b, 4 + i)); uint32_t u; std::memcpy(&u, &f, 4); return u; }());
        o.v[xD(w)] = r; break; }
    case K_VPKD3D128_3: case K_VPKD3D128_5: {   // Xenia PACK FLOAT16_2/FLOAT16_4 (redondeo a cero) + Permute(control[pack][shift], vD, nuevo)
        uint32_t imm = (w >> 16) & 31, pack = imm & 3, shift = (w >> 6) & 3;
        const VR b = in.v[xB(w)];
        auto h = [&](int i) { uint32_t u = W(b, i); float f; std::memcpy(&f, &u, 4); return uint32_t(_cvtss_sh(f, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC)); };
        uint32_t nw[4] = { 0, 0, 0, 0 };
        if (k == K_VPKD3D128_5) { nw[2] = h(0) << 16 | h(1); nw[3] = h(2) << 16 | h(3); }
        else nw[3] = h(0) << 16 | h(1);
        if (pack < 1 || pack > 3) break;
        const int* c = kXeniaPackCtl[pack][shift];
        VR r = in.v[xD(w)];
        for (int p = 0; p < 4; p++) if (c[2 * p] == 1) SW(r, p, nw[c[2 * p + 1]]);
        o.v[xD(w)] = r; break; }
    default: break;
    }
}

// ---------------- generacion de casos ----------------
uint64_t pick64(Rng& r) {
    switch (r.below(8)) {
    case 0: case 1: case 2: return r.next();
    case 3: case 4: return E64[r.below(cnt(E64))];
    case 5: return r.u32();
    case 6: return uint64_t(int64_t(int32_t(r.u32())));
    default: return E32[r.below(cnt(E32))];
    }
}
uint64_t pickd(Rng& r) { return r.below(3) == 0 ? r.next() : ED[r.below(cnt(ED))]; }
uint32_t pickf(Rng& r) { return r.below(2) ? r.u32() : EF[r.below(cnt(EF))]; }
void rvec(Rng& r, VR& v) {
    switch (r.below(7)) {
    case 0: for (auto& x : v.b) x = uint8_t(r.u32()); break;
    case 1: for (int i = 0; i < 4; i++) SW(v, i, r.below(2) ? r.u32() : E32[r.below(cnt(E32))]); break;
    case 2: for (int i = 0; i < 8; i++) SH(v, i, r.below(2) ? uint16_t(r.u32()) : E16[r.below(cnt(E16))]); break;
    case 3: for (auto& x : v.b) x = r.below(2) ? uint8_t(r.u32()) : E8[r.below(cnt(E8))]; break;
    case 4: for (int i = 0; i < 4; i++) SW(v, i, pickf(r)); break;
    case 5: { uint32_t x = r.below(2) ? E32[r.below(cnt(E32))] : r.u32(); for (int i = 0; i < 4; i++) SW(v, i, x); break; }
    default: for (int i = 0; i < 4; i++) SW(v, i, r.below(3) == 0 ? r.u32() : pickf(r)); break;
    }
}
// Vector determinista por indice de caso (limites): devuelve false si i supera la lista.
bool edge_vec_pair(uint64_t i, VR& a, VR& b) {
    const uint64_t n32 = cnt(E32) * cnt(E32), n16 = cnt(E16) * cnt(E16), n8 = cnt(E8) * cnt(E8);
    if (i < n32) { for (int l = 0; l < 4; l++) { SW(a, l, E32[i % cnt(E32)]); SW(b, l, E32[(i / cnt(E32)) % cnt(E32)]); } return true; }
    i -= n32;
    if (i < n16) { for (int l = 0; l < 8; l++) { SH(a, l, E16[i % cnt(E16)]); SH(b, l, E16[(i / cnt(E16)) % cnt(E16)]); } return true; }
    i -= n16;
    if (i < n8) { for (int l = 0; l < 16; l++) { a.b[l] = E8[i % cnt(E8)]; b.b[l] = E8[(i / cnt(E8)) % cnt(E8)]; } return true; }
    i -= n8;
    if (i < 16 * cnt(E8) * 2) {          // un solo byte distinto (lanes 0 y 15 incluidos) sobre fondo 0x00 / 0xFF
        int lane = int(i % 16); uint8_t e8 = E8[(i / 16) % cnt(E8)]; bool inv = (i / (16 * cnt(E8))) & 1;
        std::memset(a.b, inv ? 0xFF : 0x00, 16); a.b[lane] = e8; std::memset(b.b, inv ? 0x00 : 0xFF, 16); b.b[(lane + 5) & 15] = ~e8; return true;
    }
    i -= 16 * cnt(E8) * 2;
    if (i < cnt(EF) * cnt(EF)) { for (int l = 0; l < 4; l++) { SW(a, l, EF[(i + l) % cnt(EF)]); SW(b, l, EF[((i / cnt(EF)) + l * 3) % cnt(EF)]); } return true; }
    return false;
}
constexpr uint64_t NEDGE_VEC = cnt(E32) * cnt(E32) + cnt(E16) * cnt(E16) + cnt(E8) * cnt(E8) + 16 * cnt(E8) * 2 + cnt(EF) * cnt(EF);
constexpr uint64_t NEDGE_INT = cnt(E64) * cnt(E64) * 2;

struct Cfg { const CpuTest* t; K k; bool rc; };

bool is_vec(K k) { return k >= K_VNOR; }
bool is_mem(K k) { return (k >= K_LHZU && k <= K_LVEHX) || k == K_DCBST; }

uint64_t nedge(K k) {
    if (is_vec(k)) return NEDGE_VEC;
    if (k == K_FRSQRTE) return cnt(ED);
    if (k == K_BDZF || k == K_BDNZT) return cnt(E64) * 3;
    if (k == K_BLRL) return cnt(E64);
    if (k <= K_MULHDU) return NEDGE_INT;
    return 0;
}

void gen(const Cfg& c, uint64_t i, Rng& r, const St& bg, St& s) {
    const uint32_t w = c.t->word; const K k = c.k;
    s = bg;
    for (int j = 0; j < 32; j++) s.g[j] = pick64(r);
    for (int j = 0; j < 8; j++) s.cr[j] = uint8_t(r.u32() & 15);
    s.ca = r.u32() & 1; s.so = r.u32() & 1; s.ov = r.u32() & 1;
    s.ctr = pick64(r); s.lr = pick64(r);
    if (k <= K_MULHDU) {
        if (i < NEDGE_INT) {
            uint32_t ra = k == K_EQV ? fRT(w) : fRA(w), rb = fRB(w);
            uint64_t a = E64[i % cnt(E64)], b = E64[(i / cnt(E64)) % cnt(E64)];
            s.g[rb] = b; s.g[ra] = a; s.ca = (i / (cnt(E64) * cnt(E64))) & 1;
        }
        return;
    }
    if (is_mem(k)) {
        for (auto& x : s.mem) x = uint8_t(r.u32());
        const uint32_t ra = fRA(w), rb = fRB(w), rt = fRT(w);
        const bool dform = (w >> 26) != 31;
        const uint64_t ea = MEM_LO + 0x100 + r.below(0x200);
        const int64_t off = int64_t(r.below(0x100)) - 0x80;
        if (dform) s.g[ra] = ea - int64_t(fD(w));                         // EA = (RA) + D
        else if (ra == 0) s.g[rb] = ea;                                   // EA = (RB)
        else { s.g[ra] = ea - uint64_t(off); s.g[rb] = uint64_t(off); }    // EA = (RA) + (RB), (RB) con signo
        if (k == K_STFSU || k == K_STFSUX) {                              // stfs solo esta definido para valores representables en simple (Power ISA 4.6.3)
            uint32_t sp = r.below(3) == 0 ? EF[r.below(cnt(EF))] : r.u32();
            s.f[rt] = sp2dp(sp);
        } else if (k == K_STFDU) s.f[rt] = pickd(r);
        if (k == K_LFSU || k == K_LFSUX) { uint32_t x = pickf(r); for (int j = 0; j < 4; j++) s.mem[ea - MEM_LO + j] = uint8_t(x >> (24 - 8 * j)); }
        if (k == K_LFDU) { uint64_t x = pickd(r); for (int j = 0; j < 8; j++) s.mem[ea - MEM_LO + j] = uint8_t(x >> (56 - 8 * j)); }
        return;
    }
    switch (k) {
    case K_FRSQRTE: s.f[fRB(w)] = i < cnt(ED) ? ED[i] : pickd(r); return;
    case K_BDZF: case K_BDNZT: {
        if (i < cnt(E64) * 3) { s.ctr = E64[i % cnt(E64)]; if (i / cnt(E64) == 1) s.ctr = 1; else if (i / cnt(E64) == 2) s.ctr = 0x100000001ull + (i % 3); }
        if (r.below(3) == 0) s.ctr = r.below(4);
        return; }
    case K_BLRL: if (i < cnt(E64)) s.lr = E64[i]; if (r.below(3) == 0) s.lr = (s.lr & ~3ull) | r.below(4); return;
    default: break;
    }
    // vectoriales: se asignan en orden vD (valor inicial), vC/mascara, vB, vA para que los alias queden coherentes
    uint32_t d, a, b, cc = 0; bool x128 = false;
    switch (k) {
    case K_VNOR128: case K_VSRO128: case K_VPKSWSS128: case K_VSEL128: case K_VSLDOI128: x128 = true; d = xD(w); a = xA(w); b = xB(w); break;
    case K_VCFPUXWS128: case K_VUPKD3D128_5: case K_VPKD3D128_3: case K_VPKD3D128_5: x128 = true; d = xD(w); a = xB(w); b = xB(w); break;
    case K_VSEL: d = vD(w); a = vA(w); b = vB(w); cc = vC(w); break;
    case K_VCTUXS: d = vD(w); a = vB(w); b = vB(w); break;
    case K_VSPLTISH: d = vD(w); a = b = vB(w); break;
    default: d = vD(w); a = vA(w); b = vB(w); break;
    }
    (void)x128;
    rvec(r, s.v[d]); if (k == K_VSEL) rvec(r, s.v[cc]);
    rvec(r, s.v[b]); rvec(r, s.v[a]);
    VR ea, eb;
    if (i < NEDGE_VEC && edge_vec_pair(i, ea, eb)) {
        VR pm = ea; for (int j = 0; j < 16; j++) pm.b[j] ^= uint8_t(0x3C + 7 * j);   // mascara de vsel/vsel128, destino previo de vpkd3d128
        s.v[d] = pm;
        if (k == K_VSEL) { s.v[cc] = eb; for (int j = 0; j < 16; j++) s.v[cc].b[j] ^= 0xA5; }
        s.v[b] = eb; s.v[a] = ea;           // en caso de alias, la ultima asignacion manda (el modelo lee el estado inicial)
    }
}

// ---------------- E/S del contexto ----------------
struct Harness {
    alignas(64) PPCContext ctx;
    PPCRegister* G[32]; PPCRegister* F[32]; PPCVRegister* V[128]; PPCCRRegister* CR[8];
    uint8_t* host = nullptr;
    uint32_t mxcsr0 = 0;
    Harness() { std::memset((void*)&ctx, 0, sizeof ctx); BuildCtxMap(ctx, G, F, V, CR); host = (uint8_t*)_aligned_malloc(HOST_SZ, 64); std::memset(host, 0, HOST_SZ); }
    ~Harness() { _aligned_free(host); }
    void load(const St& s) {
        for (int i = 0; i < 32; i++) { G[i]->u64 = s.g[i]; F[i]->u64 = s.f[i]; }
        for (int i = 0; i < 128; i++) for (int j = 0; j < 16; j++) V[i]->u8[j] = s.v[i].b[15 - j];     // inversion completa del vector
        for (int i = 0; i < 8; i++) { CR[i]->lt = (s.cr[i] >> 3) & 1; CR[i]->gt = (s.cr[i] >> 2) & 1; CR[i]->eq = (s.cr[i] >> 1) & 1; CR[i]->so = s.cr[i] & 1; }
        ctx.xer.ca = s.ca; ctx.xer.so = s.so; ctx.xer.ov = s.ov; ctx.ctr.u64 = s.ctr; ctx.lr = s.lr;
        std::memcpy(host + MEM_LO, s.mem, MEM_SZ);
        _mm_setcsr(mxcsr0); ctx.fpscr.csr = mxcsr0;
    }
    void store(St& s) {
        for (int i = 0; i < 32; i++) { s.g[i] = G[i]->u64; s.f[i] = F[i]->u64; }
        for (int i = 0; i < 128; i++) for (int j = 0; j < 16; j++) s.v[i].b[15 - j] = V[i]->u8[j];
        for (int i = 0; i < 8; i++) s.cr[i] = uint8_t(CR[i]->lt << 3 | CR[i]->gt << 2 | CR[i]->eq << 1 | CR[i]->so);
        s.ca = ctx.xer.ca; s.so = ctx.xer.so; s.ov = ctx.xer.ov; s.ctr = ctx.ctr.u64; s.lr = ctx.lr;
        std::memcpy(s.mem, host + MEM_LO, MEM_SZ);
    }
};

uint32_t g_ind_calls = 0, g_ind_target = 0;

std::string hex128(const VR& v) { char b[40]; for (int i = 0; i < 16; i++) std::snprintf(b + 2 * i, 3, "%02X", v.b[i]); return b; }

// Compara y devuelve descripcion de la primera diferencia (vacia si igual).
std::string compare(const Exp& e, const St& got) {
    char buf[256];
    const St& x = e.s;
    if (e.frsqrte) { /* tratado aparte */ }
    for (int i = 0; i < 32; i++) if (x.g[i] != got.g[i]) { std::snprintf(buf, sizeof buf, "r%d esperado %016llX obtenido %016llX", i, (unsigned long long)x.g[i], (unsigned long long)got.g[i]); return buf; }
    for (int i = 0; i < 32; i++) {
        if (e.frsqrte && i == int(e.frt)) continue;
        if (x.f[i] != got.f[i] && !(isnan64(x.f[i]) && isnan64(got.f[i]))) { std::snprintf(buf, sizeof buf, "f%d esperado %016llX obtenido %016llX", i, (unsigned long long)x.f[i], (unsigned long long)got.f[i]); return buf; }
    }
    for (int i = 0; i < 128; i++) {
        if (e.lvehx && i == int(e.lvehx_vd)) {
            if (x.v[i].b[e.lvehx_eb] != got.v[i].b[e.lvehx_eb] || x.v[i].b[e.lvehx_eb + 1] != got.v[i].b[e.lvehx_eb + 1]) { std::snprintf(buf, sizeof buf, "v%d bytes %u-%u esperado %02X%02X obtenido %02X%02X", i, e.lvehx_eb, e.lvehx_eb + 1, x.v[i].b[e.lvehx_eb], x.v[i].b[e.lvehx_eb + 1], got.v[i].b[e.lvehx_eb], got.v[i].b[e.lvehx_eb + 1]); return buf; }
            continue;
        }
        if (std::memcmp(&x.v[i], &got.v[i], 16)) return "v" + std::to_string(i) + " esperado " + hex128(x.v[i]) + " obtenido " + hex128(got.v[i]);
    }
    for (int i = 0; i < 8; i++) if (x.cr[i] != got.cr[i]) { std::snprintf(buf, sizeof buf, "cr%d esperado %X obtenido %X", i, x.cr[i], got.cr[i]); return buf; }
    if (x.ca != got.ca || x.so != got.so || x.ov != got.ov) { std::snprintf(buf, sizeof buf, "xer ca/so/ov esperado %d%d%d obtenido %d%d%d", x.ca, x.so, x.ov, got.ca, got.so, got.ov); return buf; }
    if (x.ctr != got.ctr) { std::snprintf(buf, sizeof buf, "ctr esperado %016llX obtenido %016llX", (unsigned long long)x.ctr, (unsigned long long)got.ctr); return buf; }
    if (x.lr != got.lr) { std::snprintf(buf, sizeof buf, "lr esperado %016llX obtenido %016llX", (unsigned long long)x.lr, (unsigned long long)got.lr); return buf; }
    if (e.check_mem) {
        for (uint32_t i = 0; i < MEM_SZ; i++) if (x.mem[i] != got.mem[i]) {
            if (e.stfs_nan_lenient && i >= e.stfs_off && i < e.stfs_off + 4) {
                auto wd = [&](const St& s) { return uint32_t(s.mem[e.stfs_off]) << 24 | s.mem[e.stfs_off + 1] << 16 | s.mem[e.stfs_off + 2] << 8 | s.mem[e.stfs_off + 3]; };
                if (isnan32(wd(x)) && isnan32(wd(got))) { i = e.stfs_off + 3; continue; }
            }
            std::snprintf(buf, sizeof buf, "mem[%05X] esperado %02X obtenido %02X", MEM_LO + i, x.mem[i], got.mem[i]); return buf;
        }
    }
    return "";
}

// frsqrte: Power ISA 4.6.6: estimacion de 1/sqrt(frB) con error relativo <= 2^-5; casos especiales exactos.
std::string check_frsqrte(const Exp& e, const St& got) {
    uint64_t in = e.frsqrte_in, out = got.f[e.frt]; char buf[200];
    uint64_t sign = in >> 63, ex = (in >> 52) & 0x7FF, fr = in & 0xFFFFFFFFFFFFFull;
    auto fail = [&](const char* why) { std::snprintf(buf, sizeof buf, "frsqrte(%016llX) = %016llX: %s", (unsigned long long)in, (unsigned long long)out, why); return std::string(buf); };
    if (ex == 0x7FF && fr) return isnan64(out) ? "" : fail("entrada NaN: se esperaba NaN");
    if (ex == 0 && fr == 0) return out == (sign ? 0xFFF0000000000000ull : 0x7FF0000000000000ull) ? "" : fail("+-0: se esperaba +-infinito");
    if (sign) return isnan64(out) ? "" : fail("negativo: se esperaba NaN");
    if (ex == 0x7FF) return out == 0 ? "" : fail("+inf: se esperaba +0");
    double xv; std::memcpy(&xv, &in, 8); double ov; std::memcpy(&ov, &out, 8);
    long double ref = 1.0L / sqrtl((long double)xv);
    if (ref > 1.7976931348623157e308L) return (out == 0x7FF0000000000000ull) ? "" : fail("desbordamiento esperado");
    if (std::isnan(ov) || std::isinf(ov)) return fail("resultado no finito");
    long double rel = fabsl((long double)ov - ref) / ref;
    if (rel > 1.0L / 32.0L) return fail("error relativo > 2^-5");
    return "";
}

struct Stat { uint64_t total = 0, fail = 0; std::string first; };

} // namespace

void cputest_indirect(PPCContext&, uint8_t*, uint32_t target) { g_ind_calls++; g_ind_target = target; }

int RunCpuSelfTest(uint64_t n, uint64_t seed) {
    std::map<std::string, int> kid; std::vector<KDef> defs(std::begin(KDEFS), std::end(KDEFS));
    Harness* H = new Harness();
    H->mxcsr0 = _mm_getcsr() & ~0x8040u;      // sin FTZ/DAZ
    const uint32_t mxsave = _mm_getcsr();
    std::map<std::string, Stat> stats;           // por familia
    std::vector<std::string> order;
    uint64_t total = 0, fail = 0, nan_note = 0;
    int shown = 0;
    St* bg = new St(); St* in = new St(); St* got = new St(); Exp* ex = new Exp();
    std::printf("cpu-selftest: %zu funciones, %llu casos aleatorios por funcion + limites, semilla %llu\n", g_cpu_tests_count, (unsigned long long)n, (unsigned long long)seed);
    for (size_t ti = 0; ti < g_cpu_tests_count; ti++) {
        const CpuTest& t = g_cpu_tests[ti];
        const KDef* kd = nullptr;
        for (auto& d : defs) if (!std::strcmp(d.name, t.kind)) kd = &d;
        if (!kd) { std::printf("cpu-selftest: familia sin modelo: %s\n", t.kind); fail++; continue; }
        Cfg c{ &t, kd->k, kd->rc };
        Rng rng(seed * 0x9E3779B97F4A7C15ull + t.word * 2654435761ull + ti);
        std::memset(bg, 0, sizeof *bg);
        for (auto& x : bg->g) x = rng.next();
        for (auto& x : bg->f) x = rng.next();
        for (auto& v : bg->v) for (auto& b : v.b) b = uint8_t(rng.u32());
        for (auto& b : bg->mem) b = uint8_t(rng.u32());
        Stat& st = stats[t.kind]; if (st.total == 0) order.push_back(t.kind);
        uint64_t ne = nedge(c.k), cases = ne + n;
        uint64_t tfail = 0;
        for (uint64_t i = 0; i < cases; i++) {
            gen(c, i, rng, *bg, *in);
            H->load(*in); g_ind_calls = 0; g_ind_target = 0;
            t.fn(H->ctx, H->host);
            H->store(*got);
            _mm_setcsr(H->mxcsr0);
            model(c.k, c.rc, t.word, t.addr, *in, *ex);
            std::string d = c.k == K_FRSQRTE ? check_frsqrte(*ex, *got) : compare(*ex, *got);
            if (d.empty() && c.k == K_BLRL) {
                if (g_ind_calls != 1 || g_ind_target != ex->blrl_target) { char b[120]; std::snprintf(b, sizeof b, "llamada indirecta: n=%u destino %08X esperado %08X", g_ind_calls, g_ind_target, ex->blrl_target); d = b; }
            } else if (d.empty() && g_ind_calls != 0) d = "llamada indirecta inesperada";
            if (c.k == K_FRSQRTE) { /* GPR/FPR restantes ya comparados mediante compare con frt excluido */
                if (d.empty()) { Exp e2 = *ex; e2.frsqrte = true; std::string d2 = compare(e2, *got); d = d2; }
            }
            st.total++; total++;
            if (!d.empty()) {
                st.fail++; fail++; tfail++;
                if (tfail <= 2 && shown < 400) {
                    shown++;
                    std::printf("FALLO %s [%08X] caso %llu: %s\n", t.text, t.word, (unsigned long long)i, d.c_str());
                    if (is_vec(c.k)) {
                        uint32_t wd = t.word; bool x128 = c.k >= K_VNOR128;
                        uint32_t a = x128 ? xA(wd) : vA(wd), b = x128 ? xB(wd) : vB(wd), dd = x128 ? xD(wd) : vD(wd);
                        std::printf("    entrada vD=%s vA=%s vB=%s\n", hex128(in->v[dd]).c_str(), hex128(in->v[a]).c_str(), hex128(in->v[b]).c_str());
                    }
                }
            }
        }
    }
    _mm_setcsr(mxsave);
    std::printf("%-14s %10s %8s\n", "familia", "casos", "fallos");
    for (auto& k : order) std::printf("%-14s %10llu %8llu\n", k.c_str(), (unsigned long long)stats[k].total, (unsigned long long)stats[k].fail);
    std::printf("cpu-selftest total=%llu fail=%llu (funciones=%zu familias=%zu)%s\n", (unsigned long long)total, (unsigned long long)fail, g_cpu_tests_count, order.size(), fail ? " FALLO" : " OK");
    (void)nan_note;
    delete H; delete bg; delete in; delete got; delete ex;
    return fail ? 1 : 0;
}
