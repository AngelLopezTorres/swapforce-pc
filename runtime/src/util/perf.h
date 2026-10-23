#pragma once
// M8: perfilador de muestreo propio (--perf) y temporizadores por zona.
//  - Hilo muestreador: cada ~1 ms suspende cada hilo del proceso que haya consumido ciclos (QueryThreadCycleTime),
//    lee RIP/RSP (GetThreadContext), desenrolla la pila con la .pdata del ejecutable (RtlVirtualUnwind) y agrega
//    por RVA (exclusivo e inclusivo). Simbolizacion fuera de linea con SwapForceRT.map (tools\perf_report.py).
//  - Zonas (PerfZone): tiempo acumulado (QPC) y llamadas, solo si --perf.
//  - Linea "PERF t=... swaps=... fps=..." por segundo.
#include <cstdint>

namespace rt::perf {

enum Zone : int {
    kZoneFenceWait = 0,   // espera de valla (resolve/fin de fotograma)
    kZoneFbDecode,        // destilado del frontbuffer por CPU
    kZoneTexUpload,       // destilado + subida de texturas
    kZoneMemUpload,       // subida de paginas de memoria guest
    kZoneResolve,         // resolve completo (incl. readback)
    kZonePresent,         // Present
    kZoneSwapTotal,       // OnSwap completo
    kZoneDraw,            // OnDraw completo
    kZoneResolveFlush,    // M8: escritura diferida de resolves (incluye la espera de la GPU)
    kZonePsoCreate,       // M8: creacion de PSO (incluye carga/compilacion de DXIL)
    kZoneTexCreate,       // M8: creacion de recursos de textura
    kZoneCount
};

extern bool g_enabled;
extern bool g_sample;
extern uint32_t g_tironMs;   // M60

void Start(const char* outPath);   // inicia el muestreador (si no se llama, todo queda inactivo)
void Stop();                       // detiene y escribe el informe
void NameCurrentThread(const char* name);   // SetThreadDescription (tambien sin --perf)
void ZoneAdd(int zone, uint64_t qpcTicks);
void OnFrame(uint64_t frame);       // M8: deteccion de tirones (intervalo entre XE_SWAP > 100 ms)
uint64_t Now();                    // QPC

struct ZoneScope {
    int z; uint64_t t0;
    explicit ZoneScope(int zone) : z(zone), t0(g_enabled ? Now() : 0) {}
    ~ZoneScope() { if (g_enabled) ZoneAdd(z, Now() - t0); }
};

} // namespace rt::perf

#define PERF_ZONE(z) ::rt::perf::ZoneScope _perf_zone_##__LINE__(::rt::perf::z)
