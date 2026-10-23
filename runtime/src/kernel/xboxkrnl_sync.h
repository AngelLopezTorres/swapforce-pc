#pragma once

#include <cstdint>

namespace rt {

// Diagnostico opcional de un handle concreto de sincronizacion.
void SetSyncTraceHandle(uint32_t handle);
void DumpSyncTrace();

} // namespace rt
