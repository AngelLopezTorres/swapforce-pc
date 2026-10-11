// Diagnostico M0: contadores de entrada a funciones guest concretas sin modificar ppc_gen.
// Las definiciones fuertes sustituyen a los alias debiles `sub_X` del codigo recompilado; delegan en `__imp__sub_X`.
//  - sub_83088A88: funcion de la seccion BINK que contiene las 2 palabras 0x112DD951 / 0x10CCD951 (vsldoi128 mal decodificado).
//  - sub_82FC13C8: contiene la llamada a XAudioRegisterRenderDriverClient (lr=82FC1434).
#include <atomic>
#include <cstdint>

#include "import_log.h"
#include "ppc_config.h"
#include "ppc_context.h"

extern "C" PPC_FUNC(__imp__sub_83088A88);
extern "C" PPC_FUNC(__imp__sub_82FC13C8);

namespace {
std::atomic<uint64_t> g_n83088A88{0}, g_n82FC13C8{0};
}

PPC_FUNC(sub_83088A88) { ++g_n83088A88; __imp__sub_83088A88(ctx, base); }
PPC_FUNC(sub_82FC13C8) { ++g_n82FC13C8; __imp__sub_82FC13C8(ctx, base); }

namespace rt {
void BinkProbeReport() {
    ImportLog::Get().Printf("BINKPROBE entradas sub_83088A88=%llu sub_82FC13C8=%llu", (unsigned long long)g_n83088A88.load(), (unsigned long long)g_n82FC13C8.load());
}
} // namespace rt
