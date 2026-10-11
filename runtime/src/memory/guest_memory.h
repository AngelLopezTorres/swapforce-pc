#pragma once
// Espacio de direcciones guest de 4 GB: reserva con VirtualAlloc y confirmacion por regiones.
#include <cstdint>

namespace rt {

// Mapa de memoria del runtime (guest).
constexpr uint32_t kImageBase = 0x82000000;
constexpr uint32_t kImageSize = 0x14B0000;
constexpr uint32_t kCodeBase = 0x823E0000;
constexpr uint32_t kCodeSize = 0xCC9EF4;
constexpr uint32_t kFuncTableBase = kImageBase + kImageSize;      // 0x834B0000
constexpr uint32_t kFuncTableSize = kCodeSize * 2 * 1;            // 0x1993DE8
constexpr uint32_t kFuncTableEnd = kFuncTableBase + kFuncTableSize; // 0x84E43DE8
constexpr uint32_t kKernelArea = 0x85000000;                      // cabecera XEX, variables, estructuras
constexpr uint32_t kKernelAreaSize = 0x00400000;

constexpr uint32_t kGpuRegBase = 0x7FC80000;
constexpr uint32_t kGpuRegSize = 0x10000;
constexpr uint32_t kXmaRegBase = 0x7FEA0000;
constexpr uint32_t kXmaRegSize = 0x10000;

class GuestMemory {
public:
    static bool Init();                         // reserva 4 GB
    static uint8_t* Base();
    static bool Commit(uint32_t guest, uint32_t size); // lectura/escritura
    static bool Decommit(uint32_t guest, uint32_t size);
    static bool IsInProtectedRange(uint32_t guest, uint32_t size); // solapa imagen+tabla
    static void Write32(uint32_t guest, uint32_t v);   // big-endian
    static uint32_t Read32(uint32_t guest);
    static void Write64(uint32_t guest, uint64_t v);
    static uint64_t Read64(uint32_t guest);
    static bool PhysicalViewsMapped();         // vistas A0/C0 de la memoria fisica sobre una seccion
    static bool ReservedAtFixed();             // true si base == 0x100000000
};

} // namespace rt
