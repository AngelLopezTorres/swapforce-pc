#pragma once
// Destilado por CPU del frontbuffer (textura 2D tiled/lineal de 32 bpp) a RGBA8.
// Direccionamiento: xenia/gpu/texture_address.h (Tiled2D); endian y swizzle: xe_gpu_texture_fetch_t (xenos.h).
#include <cstdint>
#include <string>
#include <vector>

namespace rt::gpu {

struct FbDesc {
    uint32_t fetch[6] = {};  // constante de fetch (orden del host) escrita por VdSwap en SHADER_CONSTANT_FETCH_00_0
    uint32_t phys = 0;
    uint32_t w = 0, h = 0;
};

// Devuelve false si el formato no esta soportado (solo k_8_8_8_8 y k_2_10_10_10[_AS_16_16_16_16] de 32 bpp).
bool DecodeFrontbuffer(const FbDesc& d, std::vector<uint8_t>& rgba, std::string* info);

uint32_t GpuSwap32(uint32_t v, uint32_t endian);

} // namespace rt::gpu
