#pragma once
// Convencion de llamada guest (Xbox 360): argumentos 1-8 en r3-r10; 9.o en r1+0x54, siguientes cada 8 bytes
// (plan: PLAN_RUNTIME.md, datos verificados). Retorno en r3.
#include <cstdint>
#include <cstring>

#include "guest_memory.h"
#include "ppc_context.h"

namespace rt {

inline uint32_t G32(uint8_t* base, uint32_t a) { return __builtin_bswap32(*reinterpret_cast<uint32_t*>(base + a)); }
inline uint64_t G64(uint8_t* base, uint32_t a) { return __builtin_bswap64(*reinterpret_cast<uint64_t*>(base + a)); }
inline uint16_t G16(uint8_t* base, uint32_t a) { return __builtin_bswap16(*reinterpret_cast<uint16_t*>(base + a)); }
inline void P32(uint8_t* base, uint32_t a, uint32_t v) { *reinterpret_cast<uint32_t*>(base + a) = __builtin_bswap32(v); }
inline void P64(uint8_t* base, uint32_t a, uint64_t v) { *reinterpret_cast<uint64_t*>(base + a) = __builtin_bswap64(v); }
inline void P16(uint8_t* base, uint32_t a, uint16_t v) { *reinterpret_cast<uint16_t*>(base + a) = __builtin_bswap16(v); }

// Argumento i (0-based) como 32 bits.
inline uint32_t Arg32(PPCContext& ctx, uint8_t* base, int i) {
    switch (i) {
        case 0: return ctx.r3.u32;
        case 1: return ctx.r4.u32;
        case 2: return ctx.r5.u32;
        case 3: return ctx.r6.u32;
        case 4: return ctx.r7.u32;
        case 5: return ctx.r8.u32;
        case 6: return ctx.r9.u32;
        case 7: return ctx.r10.u32;
        default: return G32(base, ctx.r1.u32 + 0x54 + (uint32_t)(i - 8) * 8);
    }
}

inline void SetRet(PPCContext& ctx, uint32_t v) { ctx.r3.u64 = v; }

} // namespace rt
