#pragma once
// Interfaz entre el procesador de comandos (CP) y el backend de dibujo (D3D12).
// El backend se invoca desde el hilo del CP (como en Xenia, donde el procesador de comandos ejecuta el dibujo).
#include <cstdint>

#include "pm4.h"
#include "ucode_info.h"

namespace rt::gpu::backend {

struct SwapInfo {
    uint64_t frame = 0;
    uint32_t fbPhys = 0, w = 0, h = 0;
    uint32_t fetch[6] = {};      // SHADER_CONSTANT_FETCH_00_0..5 en el momento del XE_SWAP
};

class Backend {
public:
    virtual ~Backend() {}
    virtual void OnSwap(const SwapInfo& s) = 0;
    virtual void OnDraw(const DrawEvent& ev, const ShaderInfo* vs, const ShaderInfo* ps) = 0;
    virtual void Shutdown() = 0;
    virtual void MemWritten(uint32_t /*phys*/, uint32_t /*bytes*/) {}   // M8
    virtual void SyncPoint() {}                                         // M8   // libera recursos y escribe el resumen en runtime.log
};

Backend* Get();
void Set(Backend* b);

} // namespace rt::gpu::backend
