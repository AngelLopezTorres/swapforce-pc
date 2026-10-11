// Prueba del analizador PM4 sobre un anillo sintetico (sin juego, sin GPU): --selftest-pm4.
#include <cstdio>
#include <cstring>
#include <vector>

#include "pm4.h"

namespace rt::gpu {

namespace {

struct MockHost : Pm4Host {
    uint32_t regs[kRegCount] = {};
    std::vector<uint8_t> mem = std::vector<uint8_t>(0x1000, 0);
    int interrupts = 0, swaps = 0, frames = 0, shaders = 0, lines = 0;
    uint32_t lastMask = 0;
    void WriteReg(uint32_t i, uint32_t v) override { regs[i] = v; }
    uint32_t ReadReg(uint32_t i) override { return regs[i]; }
    uint8_t* Phys(uint32_t p, uint32_t n) override { return (p < 0x1000 && p + n <= 0x1000) ? mem.data() + p : nullptr; }
    void Interrupt(uint32_t m) override { ++interrupts; lastMask = m; }
    void Swap(uint32_t, uint32_t, uint32_t) override { ++swaps; }
    uint64_t Counter() override { return 7; }
    bool Poll() override { return false; }
    void OnShader(uint32_t, const uint8_t*, uint32_t, uint32_t, bool) override { ++shaders; }
    void Log(const char*) override { ++lines; }
    void FrameDone(const char* s) override { ++frames; std::printf("  %s\n", s); }
};

struct Ring {
    std::vector<uint32_t> w; // palabras en orden guest (se guardan big-endian)
    void Put(uint32_t v) { w.push_back(v); }
    uint32_t T0(uint32_t reg, uint32_t n) { return ((n - 1) << 16) | reg; }
    uint32_t T3(uint32_t op, uint32_t n) { return (3u << 30) | ((n - 1) << 16) | (op << 8); }
};

} // namespace

int Pm4SelfTest() {
    MockHost h;
    Pm4Parser p(&h);
    Ring r;
    r.Put(r.T0(0x2000, 2)); r.Put(0x11111111); r.Put(0x22222222);                    // tipo 0, 2 registros
    r.Put((1u << 30) | (0x0100u << 11) | 0x0200u); r.Put(0xAAAA0001); r.Put(0xBBBB0002); // tipo 1
    r.Put(2u << 30);                                                                   // tipo 2
    r.Put(r.T3(PM4_NOP, 3)); r.Put(0); r.Put(0); r.Put(0);                             // NOP de 3 dwords
    r.Put(r.T3(PM4_SET_CONSTANT, 3)); r.Put((4u << 16) | 0x10); r.Put(0xC0DE0001); r.Put(0xC0DE0002); // registros 0x2010,0x2011
    r.Put(r.T3(PM4_IM_LOAD_IMMEDIATE, 4)); r.Put(0); r.Put(2); r.Put(0x12345678); r.Put(0x9ABCDEF0);
    r.Put(r.T3(PM4_DRAW_INDX, 5)); r.Put(0); r.Put((3u << 16) | (0u << 6) | 4); r.Put(0x100); r.Put(0x6); r.Put(0);
    r.Put(r.T3(PM4_DRAW_INDX_2, 2)); r.Put((3u << 16) | (2u << 6) | 4); r.Put(0);
    r.Put(r.T3(PM4_MEM_WRITE, 2)); r.Put(0x200 | 2); r.Put(0xDEADBEEF);               // endian 8in32
    r.Put(r.T3(PM4_EVENT_WRITE_SHD, 3)); r.Put(0x80000000u | 5); r.Put(0x300 | 2); r.Put(0); // contador
    r.Put(r.T3(PM4_INTERRUPT, 1)); r.Put(0x05);
    r.Put(r.T3(0x7E, 2)); r.Put(0); r.Put(0);                                          // opcode desconocido
    r.Put(r.T3(PM4_XE_SWAP, 4)); r.Put(0x53574150); r.Put(0x400); r.Put(1280); r.Put(720);
    // Anillo de 64 dwords con lectura cerca del final para provocar la vuelta.
    const uint32_t cap = 64, start = 64 - 10;
    std::vector<uint32_t> ring(cap, 0);
    for (size_t i = 0; i < r.w.size(); ++i) ring[(start + i) % cap] = __builtin_bswap32(r.w[i]);
    bool wrapped = start + r.w.size() > cap;
    uint32_t wr = (uint32_t)((start + r.w.size()) % cap);
    bool ok = p.ExecuteRing((const uint8_t*)ring.data(), cap * 4, start, wr);
    Pm4Stats& s = p.Stats();
    uint32_t mem300, mem200;
    std::memcpy(&mem300, &h.mem[0x300], 4); std::memcpy(&mem200, &h.mem[0x200], 4);
    std::printf("palabras=%zu vuelta_del_anillo=%d ok=%d packets=%llu t0=%llu t1=%llu t2=%llu t3=%llu draws=%llu DRAW_INDX=%llu DRAW_INDX_2=%llu UNKNOWN=%llu frames=%llu\n",
                r.w.size(), (int)wrapped, (int)ok, (unsigned long long)s.packets, (unsigned long long)s.type0,
                (unsigned long long)s.type1, (unsigned long long)s.type2, (unsigned long long)s.type3, (unsigned long long)s.draws,
                (unsigned long long)s.drawIndx, (unsigned long long)s.drawIndx2, (unsigned long long)s.unknownOp, (unsigned long long)s.frames);
    std::printf("regs 0x2000=%08X 0x2001=%08X 0x0200=%08X 0x0100=%08X 0x2010=%08X 0x2011=%08X; mem[0x200]=%08X (GpuSwap 8in32 de DEADBEEF) mem[0x300]=%08X; interrupts=%d mask=%X swaps=%d shaders=%d\n",
                h.regs[0x2000], h.regs[0x2001], h.regs[0x0200], h.regs[0x0100], h.regs[0x2010], h.regs[0x2011], mem200, mem300,
                h.interrupts, h.lastMask, h.swaps, h.shaders);
    bool pass = ok && wrapped && s.draws == 2 && s.drawIndx == 1 && s.drawIndx2 == 1 && s.unknownOp == 1 && s.frames == 1 &&
                h.regs[0x2000] == 0x11111111 && h.regs[0x2001] == 0x22222222 && h.regs[0x0200] == 0xAAAA0001 && h.regs[0x0100] == 0xBBBB0002 &&
                h.regs[0x2010] == 0xC0DE0001 && h.regs[0x2011] == 0xC0DE0002 && mem200 == 0xEFBEADDEu /* bswap de DEADBEEF almacenado LE = bytes DE AD BE EF */ &&
                h.interrupts == 1 && h.lastMask == 5 && mem300 == 0x07000000u && h.swaps == 1 && h.shaders == 1 && h.frames == 1;
    std::printf("selftest-pm4 %s\n", pass ? "OK" : "FALLO");
    return pass ? 0 : 1;
}

} // namespace rt::gpu
