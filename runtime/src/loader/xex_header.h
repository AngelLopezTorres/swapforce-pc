#pragma once
#include <cstdint>
#include <string>

namespace rt {

constexpr uint32_t kXexHeaderGuest = 0x85000000; // direccion guest donde se copia la cabecera XEX

struct XexHeaderInfo {
    uint32_t size = 0;        // bytes copiados (data_offset, be32 en +8)
    uint32_t moduleFlags = 0; // be32 en +4
    uint32_t guestAddr = 0;
};

// Lee la cabecera de default.xex ("XEX2", data_offset en +8) y la copia a memoria guest.
bool LoadXexHeader(const std::string& xexPath, XexHeaderInfo& out, std::string* err);

struct XexTlsInfo {
    uint32_t slotCount = 0, rawDataAddress = 0, dataSize = 0, rawDataSize = 0;
    bool present = false;
};

// Busca una cabecera opcional (clave) en la copia guest. Devuelve el valor (inline u offset) y, si es
// una estructura, su direccion guest.
bool XexFindOptHeader(uint32_t key, uint32_t* value, uint32_t* guestStruct);
XexTlsInfo XexGetTlsInfo(); // clave 0x00020104

} // namespace rt
