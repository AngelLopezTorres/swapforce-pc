#pragma once
// Traductor de microcodigo Xenos -> HLSL (H2). Semantica de cada operacion: comentarios de xenia/gpu/ucode.h
// (Xenia, BSD; copia en runtime\ref\ucode.h). La emision de ALU toma como referencia el recompilador XenosRecomp (MIT),
// pero con flujo de control, predicacion y fetch propios (XenosRecomp requiere el contenedor de shader del XDK).
#include <cstdint>
#include <string>

#include "ucode_info.h"

namespace rt::gpu {

struct HlslOptions {
    int paramGenPos = -1;      // PS: registro que recibe XY de pantalla/cara (SQ_PROGRAM_CNTL.param_gen + SQ_CONTEXT_MISC.param_gen_pos), -1 = ninguno
};

struct HlslResult {
    std::string hlsl;
    std::string error;         // no vacio: instruccion/estructura no soportada
    bool ok = false;
    uint32_t unsupportedCount = 0;
};

HlslResult TranslateToHlsl(const ShaderInfo& si, const HlslOptions& opt);

// Compilacion HLSL -> DXIL con DXC (dxcompiler.dll + dxil.dll junto al ejecutable). Devuelve false si falla (log en `err`).
bool CompileHlslToDxil(const std::string& hlsl, bool vertex, std::string& dxil, std::string& err);
// Variante con perfil explicito ("vs_6_0", "ps_6_0", "gs_6_0").
bool CompileHlslProfile(const std::string& hlsl, const wchar_t* profile, std::string& dxil, std::string& err);

// Geometry shader que expande una lista de rectangulos (3 vertices por rectangulo) a una tira de 4 vertices
// (cuarto vertice por reflexion respecto a la arista mas larga, como pipeline_cache.cc de Xenia).
std::string RectListGeometryShaderHlsl();

// Version del traductor (parte de la clave de la cache de DXIL en disco).
constexpr const char* kTranslatorVersion = "t2";

// Prueba H2: traduce y compila todos los ucode_*.bin de `dir`. Devuelve 0 si todos compilan.
int RunShaderTest(const std::string& dir);

} // namespace rt::gpu
