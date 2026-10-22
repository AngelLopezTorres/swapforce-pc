// Vuelca, para cada palabra distinta con opcodes primarios 4, 5 y 6 (VMX y VMX128) de las secciones ejecutables de analysis\image.bin,
// el nombre que le asigna el desensamblador de XenonRecomp (binutils "cell 64") tras aplicar ppc::FixMisdecodedVsldoi128. Uso: scan_op4 image.bin > op4_xr.txt
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>
#include "disasm.h"
static uint32_t rd32le(const std::vector<uint8_t>& d, size_t o) { return d[o] | d[o+1]<<8 | d[o+2]<<16 | (uint32_t)d[o+3]<<24; }
int main(int argc, char** argv) {
    FILE* f = fopen(argv[1], "rb"); if (!f) return 1;
    std::vector<uint8_t> d; uint8_t b[65536]; size_t n;
    while ((n = fread(b, 1, sizeof b, f)) > 0) d.insert(d.end(), b, b + n);
    fclose(f);
    uint32_t pe = rd32le(d, 0x3c), base = rd32le(d, pe + 24 + 28);
    int ns = d[pe+6] | d[pe+7]<<8, opt = d[pe+20] | d[pe+21]<<8;
    struct W { uint32_t first; uint64_t cnt; };
    std::map<uint32_t, W> words;
    size_t o = pe + 24 + opt;
    for (int i = 0; i < ns; i++, o += 40) {
        uint32_t vs = rd32le(d, o+8), va = rd32le(d, o+12), ch = rd32le(d, o+36);
        if (!(ch & 0x20)) continue;
        char nm[9] = {}; memcpy(nm, &d[o], 8);
        fprintf(stderr, "seccion %s base=%08X size=%X\n", nm, base + va, vs);
        for (uint32_t a = va; a + 4 <= va + vs; a += 4) {
            uint32_t w = (uint32_t)d[a]<<24 | d[a+1]<<16 | d[a+2]<<8 | d[a+3];
            if ((w >> 26) < 4 || (w >> 26) > 6) continue;
            auto& e = words[w]; if (!e.cnt) e.first = base + a; e.cnt++;
        }
    }
    for (auto& [w, e] : words) {
        uint32_t be = __builtin_bswap32(w);
        ppc_insn insn{};
        ppc::Disassemble(&be, e.first, insn);
        { ppc_insn fx; if (ppc::FixMisdecodedVsldoi128(insn, w, fx)) insn = fx; }   // correccion del recompilador (XenonUtils/disasm.h)
        printf("%08X %08X %llu %s %u %u %u %u %u\n", w, e.first, (unsigned long long)e.cnt, insn.opcode ? insn.opcode->name : "NULL",
               insn.operands[0], insn.operands[1], insn.operands[2], insn.operands[3], insn.operands[4]);
    }
    fprintf(stderr, "palabras distintas opcode 4: %zu\n", words.size());
}
