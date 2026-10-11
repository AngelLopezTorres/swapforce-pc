#pragma once
#include <cstdint>

namespace rt {

struct KernelVarInfo {
    const char* name;
    uint32_t ordinal;     // ordinal de xboxkrnl.exe (Xenia xboxkrnl_table.inc)
    uint32_t recordAddr;  // direccion guest del registro de import (slot que recibe el puntero)
    uint32_t dataSize;    // bytes de datos de la variable
};

constexpr int kKernelVarCount = 13;
extern const KernelVarInfo kKernelVars[kKernelVarCount];

// Escribe, en cada registro de import de variable, el puntero guest a su dato. Devuelve el numero
// de variables inicializadas (13 si todo va bien). Comprueba que el registro contiene el ordinal esperado.
int InitKernelVars(uint32_t xexHeaderGuest);

uint32_t KernelVarAddress(const char* name); // direccion guest del dato (0 si no existe)

} // namespace rt
