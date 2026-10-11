// Banco de pruebas de CPU (--selftest-cpu): compara el codigo producido por XenonRecomp con un modelo de referencia
// escrito a partir de ALTIVECPEM Rev.3 (NXP), Power ISA v2.07B (OpenPOWER) y, para VMX128, el codigo de Xenia.
#pragma once
#include <cstddef>
#include <cstdint>

#define PPC_CONFIG_H_INCLUDED
struct PPCContext;
void cputest_indirect(PPCContext& ctx, uint8_t* base, uint32_t target);   // sustituto de PPC_CALL_INDIRECT_FUNC en las pruebas
#define PPC_CALL_INDIRECT_FUNC(x) cputest_indirect(ctx, base, (x))
#include <ppc_context.h>

struct CpuTest {
    const char* kind;   // familia (selecciona el modelo de referencia)
    const char* text;   // ensamblador de la prueba
    uint32_t word;      // primera palabra de instruccion
    uint32_t addr;      // direccion de la funcion en el ELF sintetico
    PPCFunc* fn;
    int nwords;
};
extern const CpuTest g_cpu_tests[];
extern const size_t g_cpu_tests_count;

// Devuelve 0 si fail=0. n = casos aleatorios por prueba; seed = semilla.
int RunCpuSelfTest(uint64_t n, uint64_t seed);
