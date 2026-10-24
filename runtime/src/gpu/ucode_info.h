#pragma once
// Analisis estatico del microcodigo Xenos (H0/H2). Estructuras de bits: xenia/gpu/ucode.h (Xenia, BSD).
// Semantica de la busqueda del limite del flujo de control y de la recogida de fetch: shader_translator.cc
// (Shader::AnalyzeUcode / GatherExecInformation), copia en runtime\ref\.
#include <cstdint>
#include <string>
#include <vector>

namespace rt::gpu {

struct VFetchUse {
    uint32_t constIdx = 0, format = 0, stride = 0;
    bool mini = false;
    uint32_t src = 0, srcSwizzle = 0, dest = 0, destSwizzle = 0;
    int32_t offset = 0;
    bool srcRelative = false, destRelative = false, indexRounded = false;
    bool isSigned = false, normalized = false;
};
struct TFetchUse { uint32_t constIdx; uint32_t dim; uint32_t opcode; };

struct ShaderInfo {
    bool vertex = false;
    uint64_t hash = 0;                    // FNV-1a de los bytes big-endian + tipo (igual que el nombre de volcado)
    uint32_t dwords = 0;
    std::vector<uint32_t> ucode;          // palabras en orden del host (ya intercambiadas)
    uint32_t cfPairBound = 0;             // pares de instrucciones de flujo de control (3 dwords cada uno)
    std::vector<VFetchUse> vfetch;
    std::vector<TFetchUse> tfetch;
    uint32_t vfetchMask[3] = {0, 0, 0};   // constantes de fetch de vertice referenciadas (0-95)
    uint32_t tfetchMask = 0;              // constantes de textura referenciadas (0-31)
    uint32_t regBound = 0;                // cota estatica de registros temporales
    bool dynamicRegs = false;
    uint32_t writesInterp = 0, writesColors = 0;
    bool writesDepth = false, writesPos = false, writesPointSize = false;
    bool memexportAddr = false; uint32_t memexportData = 0;
    bool kills = false;
    uint32_t aluCount = 0, fetchCount = 0, execCount = 0, loopCount = 0, callCount = 0, jmpCount = 0, condCount = 0;
    bool floatDynamic = false;
    uint64_t floatBitmap[4] = {0, 0, 0, 0};
    uint32_t boolBitmap[8] = {0};
    uint32_t loopBitmap = 0;
    std::string error;
};

// beBytes: microcodigo tal como esta en la memoria del guest (big-endian). Devuelve false si esta malformado.
bool AnalyzeUcode(bool vertex, const uint8_t* beBytes, uint32_t dwords, ShaderInfo& out);

uint64_t HashUcode(const uint8_t* beBytes, uint32_t dwords, uint32_t type);

} // namespace rt::gpu
