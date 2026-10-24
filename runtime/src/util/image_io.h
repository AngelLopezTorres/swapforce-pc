#pragma once
// Escritura de PNG con WIC (windowscodecs, componente de Windows; sin dependencias externas).
#include <cstdint>
#include <string>
#include <vector>

namespace rt {

// rgba: w*h pixeles de 4 bytes en orden R,G,B,A (stride en bytes). Crea el directorio padre si no existe.
bool WritePngRGBA(const std::string& path, uint32_t w, uint32_t h, const uint8_t* rgba, uint32_t strideBytes);

// JPEG en memoria (WIC, propiedad ImageQuality 0-1 = quality/100). rgba: R,G,B,A; out se rellena con el fichero JPEG.
bool EncodeJpegRGBA(uint32_t w, uint32_t h, const uint8_t* rgba, uint32_t strideBytes, uint32_t quality, std::vector<uint8_t>& out);

// Decodifica el primer fotograma WIC a RGBA8. Devuelve false si no se puede abrir o convertir.
bool ReadImageRGBA(const std::string& path, uint32_t& w, uint32_t& h, std::vector<uint8_t>& rgba);

// Utilidades de comprobacion sobre RGBA.
struct ImageStats { uint64_t nonblack = 0; uint64_t hash = 0; uint32_t minR = 255, maxR = 0, minG = 255, maxG = 0, minB = 255, maxB = 0; };
ImageStats ComputeStats(uint32_t w, uint32_t h, const uint8_t* rgba, uint32_t strideBytes);

} // namespace rt
