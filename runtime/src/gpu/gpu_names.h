#pragma once
// Nombres de enumeraciones Xenos para los histogramas y registros (valores de xenos.h de Xenia).
#include <cstdint>

namespace rt::gpu {

inline const char* TexFmtName(uint32_t f) {
    static const char* n[64] = {
        "k_1_REVERSE", "k_1", "k_8", "k_1_5_5_5", "k_5_6_5", "k_6_5_5", "k_8_8_8_8", "k_2_10_10_10", "k_8_A", "k_8_B",
        "k_8_8", "k_Cr_Y1_Cb_Y0_REP", "k_Y1_Cr_Y0_Cb_REP", "k_16_16_EDRAM", "k_8_8_8_8_A", "k_4_4_4_4", "k_10_11_11",
        "k_11_11_10", "k_DXT1", "k_DXT2_3", "k_DXT4_5", "k_16_16_16_16_EDRAM", "k_24_8", "k_24_8_FLOAT", "k_16",
        "k_16_16", "k_16_16_16_16", "k_16_EXPAND", "k_16_16_EXPAND", "k_16_16_16_16_EXPAND", "k_16_FLOAT",
        "k_16_16_FLOAT", "k_16_16_16_16_FLOAT", "k_32", "k_32_32", "k_32_32_32_32", "k_32_FLOAT", "k_32_32_FLOAT",
        "k_32_32_32_32_FLOAT", "k_32_AS_8", "k_32_AS_8_8", "k_16_MPEG", "k_16_16_MPEG", "k_8_INTERLACED",
        "k_32_AS_8_INTERLACED", "k_32_AS_8_8_INTERLACED", "k_16_INTERLACED", "k_16_MPEG_INTERLACED",
        "k_16_16_MPEG_INTERLACED", "k_DXN", "k_8_8_8_8_AS_16_16_16_16", "k_DXT1_AS_16_16_16_16",
        "k_DXT2_3_AS_16_16_16_16", "k_DXT4_5_AS_16_16_16_16", "k_2_10_10_10_AS_16_16_16_16",
        "k_10_11_11_AS_16_16_16_16", "k_11_11_10_AS_16_16_16_16", "k_32_32_32_FLOAT", "k_DXT3A", "k_DXT5A", "k_CTX1",
        "k_DXT3A_AS_1_1_1_1", "k_8_8_8_8_GAMMA_EDRAM", "k_2_10_10_10_FLOAT_EDRAM"};
    return f < 64 ? n[f] : "?";
}
// VertexFormat usa los mismos valores que TextureFormat para los formatos admitidos (xenos.h).
inline const char* VtxFmtName(uint32_t f) {
    switch (f) {
        case 0: return "kUndefined";
        case 6: case 7: case 16: case 17: case 25: case 26: case 31: case 32: case 33: case 34: case 35: case 36: case 37: case 38: case 57:
            return TexFmtName(f);
        default: return "?";
    }
}
inline const char* ColorRtName(uint32_t f) {
    switch (f) {
        case 0: return "k_8_8_8_8"; case 1: return "k_8_8_8_8_GAMMA"; case 2: return "k_2_10_10_10";
        case 3: return "k_2_10_10_10_FLOAT"; case 4: return "k_16_16"; case 5: return "k_16_16_16_16";
        case 6: return "k_16_16_FLOAT"; case 7: return "k_16_16_16_16_FLOAT"; case 10: return "k_2_10_10_10_AS_10_10_10_10";
        case 12: return "k_2_10_10_10_FLOAT_AS_16_16_16_16"; case 14: return "k_32_FLOAT"; case 15: return "k_32_32_FLOAT";
        default: return "?";
    }
}
inline const char* DepthRtName(uint32_t f) { return f == 0 ? "kD24S8" : "kD24FS8"; }
inline const char* EdramModeName(uint32_t m) {
    switch (m) { case 0: return "kNoOperation"; case 4: return "kColorDepth"; case 5: return "kDepthOnly"; case 6: return "kCopy"; default: return "?"; }
}
inline const char* MsaaName(uint32_t m) { return m == 0 ? "1X" : m == 1 ? "2X" : m == 2 ? "4X" : "?"; }
inline const char* DimName(uint32_t d) { static const char* n[4] = {"1D", "2D/stacked", "3D", "Cube"}; return n[d & 3]; }
inline const char* PrimName(uint32_t p) {
    switch (p) {
        case 0: return "none"; case 1: return "point_list"; case 2: return "line_list"; case 3: return "line_strip";
        case 4: return "tri_list"; case 5: return "tri_fan"; case 6: return "tri_strip"; case 7: return "tri_wflags";
        case 8: return "rect_list"; case 12: return "line_loop"; case 13: return "quad_list"; case 14: return "quad_strip";
        case 15: return "polygon"; default: return "other";
    }
}

} // namespace rt::gpu
