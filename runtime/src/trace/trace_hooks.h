#pragma once
// Incluido a la fuerza (/FI) en las unidades recompiladas: registra los destinos de las llamadas
// indirectas (bctrl/bctr) en un anillo por hilo, para diagnostico (opcion SWAPFORCE_TRACE).
#include <cstdint>
#include <intrin.h>

namespace rt {
struct TraceRing {
    uint32_t target[1024];
    uint32_t lr[1024];
    uint32_t n;
};
extern thread_local TraceRing g_traceRing;
inline void TraceIndirect(uint32_t target, uint32_t lr) {
    TraceRing& r = g_traceRing;
    uint32_t i = r.n++ & 1023;
    r.target[i] = target;
    r.lr[i] = lr;
}
} // namespace rt

// mftb: base de tiempo del 360 a 50 MHz (KeQueryPerformanceFrequency); el TSC del host (GHz) la hacia ir ~60 veces mas rapida.
namespace rt { uint64_t GuestTimebase(); }
#define __rdtsc() ::rt::GuestTimebase()

#define PPC_CALL_INDIRECT_FUNC(x) (::rt::TraceIndirect((uint32_t)(x), (uint32_t)ctx.lr), (PPC_LOOKUP_FUNC(base, x))(ctx, base))
