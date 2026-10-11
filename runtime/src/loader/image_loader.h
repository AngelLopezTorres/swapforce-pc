#pragma once
#include <cstdint>
#include <string>

namespace rt {

struct ImageLoadResult {
    uint32_t size = kImageSizeConst();
    uint64_t fileSize = 0;
    bool mz = false;
    size_t funcTableEntries = 0; // entradas escritas en la tabla
    static constexpr uint32_t kImageSizeConst() { return 0x14B0000; }
};

// Confirma imagen y tabla de funciones, copia image.bin y rellena la tabla desde PPCFuncMappings.
bool LoadGameImage(const std::string& imagePath, ImageLoadResult& out, std::string* err);

} // namespace rt
