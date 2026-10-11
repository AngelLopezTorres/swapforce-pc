#pragma once
// Escritura de PNG con WIC (windowscodecs, componente de Windows; sin dependencias externas).
#include <cstdint>
#include <string>

namespace rt {

// rgba: w*h pixeles de 4 bytes en orden R,G,B,A (stride en bytes). Crea el directorio padre si no existe.
bool WritePngRGBA(const std::string& path, uint32_t w, uint32_t h, const uint8_t* rgba, uint32_t strideBytes);

// Utilidades de comprobacion sobre RGBA.
struct ImageStats { uint64_t nonblack = 0; uint64_t hash = 0; uint32_t minR = 255, maxR = 0, minG = 255, maxG = 0, minB = 255, maxB = 0; };
ImageStats ComputeStats(uint32_t w, uint32_t h, const uint8_t* rgba, uint32_t strideBytes);

} // namespace rt
