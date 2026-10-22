// Diagnostico M0/Bink: contadores de entrada a funciones guest sin modificar ppc_gen.
// Las definiciones fuertes sustituyen a los alias debiles `sub_X` del codigo recompilado; delegan en `__imp__sub_X`.
//  - sub_82FC13C8: contiene la llamada a XAudioRegisterRenderDriverClient (lr=82FC1434).
//  - Todas las funciones de las secciones BINK y RADCODE (incluida sub_83088A88): bink_probe_gen.inc,
//    generado por runtime/tools/gen_bink_probe.py. Por funcion: entradas, tid guest y lr de la ultima entrada,
//    y hasta 4 llamantes distintos externos a BINK/RADCODE (puntos de entrada del API llamados por el juego).
#include <atomic>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "guest_thread.h"
#include "import_log.h"
#include "ppc_config.h"
#include "ppc_context.h"

extern "C" PPC_FUNC(__imp__sub_82FC13C8);

namespace {
std::atomic<uint64_t> g_n82FC13C8{0};

struct ProbeSlot {
    std::atomic<uint64_t> n{0};
    std::atomic<uint32_t> lastLr{0}, lastTid{0};
    std::atomic<uint32_t> ext[4]{};
};

constexpr uint32_t kBinkLo = 0x8307E000, kBinkHi = 0x830A9EF4;
ProbeSlot* Slots();

inline void BinkHit(int i, PPCContext& ctx) {
    ProbeSlot& s = Slots()[i];
    s.n.fetch_add(1, std::memory_order_relaxed);
    uint32_t lr = (uint32_t)ctx.lr;
    s.lastLr.store(lr, std::memory_order_relaxed);
    rt::GuestThread* t = rt::GuestThread::Current();
    s.lastTid.store(t ? t->id : 0, std::memory_order_relaxed);
    if (lr < kBinkLo || lr >= kBinkHi) {
        for (auto& e : s.ext) {
            uint32_t v = e.load(std::memory_order_relaxed);
            if (v == lr) break;
            if (v == 0) { uint32_t z = 0; if (e.compare_exchange_strong(z, lr) || z == lr) break; }
        }
    }
}
} // namespace

// bink_probe_gen.inc deriva del binario del juego (direcciones de funciones): no se publica (como import_stubs.gen.cpp).
// Sin el, solo quedan las sondas de sub_82FC13C8 y sub_830804C8 (BINKWAIT).
#if __has_include("bink_probe_gen.inc")
#include "bink_probe_gen.inc"
#else
#define BINK_PROBE_N 1
static const uint32_t kBinkProbeAddr[BINK_PROBE_N] = {0x830804C8};
#define BINK_PROBE_IDX_830804C8 0
#endif
extern "C" PPC_FUNC(__imp__sub_830804C8);

namespace {
ProbeSlot* Slots() {
    static ProbeSlot s[BINK_PROBE_N];
    return s;
}
} // namespace

PPC_FUNC(sub_82FC13C8) { ++g_n82FC13C8; __imp__sub_82FC13C8(ctx, base); }

// sub_830804C8: llamada por el bucle de reproduccion del juego (sub_825A80E0) antes de decodificar; si devuelve
// distinto de 0 el juego no avanza el fotograma (estructura de BinkWait [NV]). Se registran los campos del objeto
// (r3) que la funcion lee (desplazamientos decimales tomados del propio codigo) y el valor devuelto.
PPC_FUNC(sub_830804C8) {
    BinkHit(BINK_PROBE_IDX_830804C8, ctx);
    uint32_t b = ctx.r3.u32;
    __imp__sub_830804C8(ctx, base);
    static std::atomic<uint32_t> n{0};
    uint32_t k = n.fetch_add(1);
    if (b && (k < 8 || (k % 100) == 0)) {
        auto g = [&](uint32_t off) { return __builtin_bswap32(*reinterpret_cast<uint32_t*>(base + b + off)); };
        uint32_t p340 = g(340);
        uint32_t f60 = p340 ? __builtin_bswap32(*reinterpret_cast<uint32_t*>(base + p340 + 60)) : 0;
        uint32_t f76 = p340 ? __builtin_bswap32(*reinterpret_cast<uint32_t*>(base + p340 + 76)) : 0;
        rt::ImportLog::Get().Printf("BINKWAIT #%u ret=%u obj=%08X [20]=%u [24]=%u [28]=%u [32]=%08X [64]=%u [72]=%u [280]=%d [296]=%u [380]=%u "
                                "[1120]=%u [1136]=%d [1140]=%u [1144]=%u [1148]=%u [1176]=%u p340=%08X p340[60]=%08X p340[76]=%u",
                                k, ctx.r3.u32, b, g(20), g(24), g(28), g(32), g(64), g(72), (int)g(280), g(296), g(380), g(1120),
                                (int)g(1136), g(1140), g(1144), g(1148), g(1176), p340, f60, f76);
    }
}

namespace rt {
uint64_t BinkProbeCount(uint32_t addr) {
    for (int i = 0; i < BINK_PROBE_N; ++i) if (kBinkProbeAddr[i] == addr) return Slots()[i].n.load();
    return 0;
}

void BinkProbeReport() {
    auto& log = ImportLog::Get();
    uint64_t called = 0;
    for (int i = 0; i < BINK_PROBE_N; ++i) if (Slots()[i].n.load()) ++called;
    log.Printf("BINKPROBE entradas sub_83088A88=%llu sub_82FC13C8=%llu funciones_BINK=%d llamadas_alguna_vez=%llu",
               (unsigned long long)BinkProbeCount(0x83088A88), (unsigned long long)g_n82FC13C8.load(), BINK_PROBE_N,
               (unsigned long long)called);
    for (int i = 0; i < BINK_PROBE_N; ++i) {
        ProbeSlot& s = Slots()[i];
        uint64_t n = s.n.load();
        if (!n) continue;
        log.Printf("BINKFN %08X n=%llu ultimo_tid=%u ultimo_lr=%08X externos=%08X,%08X,%08X,%08X", kBinkProbeAddr[i],
                   (unsigned long long)n, s.lastTid.load(), s.lastLr.load(), s.ext[0].load(), s.ext[1].load(), s.ext[2].load(),
                   s.ext[3].load());
    }
}
} // namespace rt
