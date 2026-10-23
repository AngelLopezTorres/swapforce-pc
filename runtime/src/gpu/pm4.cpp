#include "pm4.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <atomic>
#include <windows.h>
#include "perf.h"

namespace rt::gpu {
int g_syncReason = 0;   // M60: causa del punto de sincronizacion en curso (politica de vaciado de resolves y estadistica)
uint32_t g_syncAddr = 0;   // M60: direccion de la espera WAIT_REG_MEM sobre memoria

struct OpName { uint32_t op; const char* name; };
static const OpName kOps[] = {
    {PM4_NOP, "NOP"}, {PM4_REG_RMW, "REG_RMW"}, {PM4_DRAW_INDX, "DRAW_INDX"}, {PM4_VIZ_QUERY, "VIZ_QUERY"},
    {PM4_SET_STATE, "SET_STATE"}, {PM4_WAIT_FOR_IDLE, "WAIT_FOR_IDLE"}, {PM4_IM_LOAD, "IM_LOAD"},
    {PM4_IM_LOAD_IMMEDIATE, "IM_LOAD_IMMEDIATE"}, {PM4_IM_STORE, "IM_STORE"}, {PM4_SET_CONSTANT, "SET_CONSTANT"},
    {PM4_LOAD_CONSTANT_CONTEXT, "LOAD_CONSTANT_CONTEXT"}, {PM4_LOAD_ALU_CONSTANT, "LOAD_ALU_CONSTANT"},
    {PM4_DRAW_INDX_BIN, "DRAW_INDX_BIN"}, {PM4_DRAW_INDX_2_BIN, "DRAW_INDX_2_BIN"}, {PM4_DRAW_INDX_2, "DRAW_INDX_2"},
    {PM4_INDIRECT_BUFFER_PFD, "INDIRECT_BUFFER_PFD"}, {PM4_INVALIDATE_STATE, "INVALIDATE_STATE"},
    {PM4_WAIT_REG_MEM, "WAIT_REG_MEM"}, {PM4_MEM_WRITE, "MEM_WRITE"}, {PM4_REG_TO_MEM, "REG_TO_MEM"},
    {PM4_INDIRECT_BUFFER, "INDIRECT_BUFFER"}, {PM4_COND_EXEC, "COND_EXEC"}, {PM4_COND_WRITE, "COND_WRITE"},
    {PM4_EVENT_WRITE, "EVENT_WRITE"}, {PM4_ME_INIT, "ME_INIT"}, {PM4_SET_SHADER_BASES, "SET_SHADER_BASES"},
    {PM4_SET_BIN_BASE_OFFSET, "SET_BIN_BASE_OFFSET"}, {PM4_MEM_WRITE_CNTR, "MEM_WRITE_CNTR"},
    {PM4_SET_BIN_MASK, "SET_BIN_MASK"}, {PM4_SET_BIN_SELECT, "SET_BIN_SELECT"}, {PM4_WAIT_REG_EQ, "WAIT_REG_EQ"},
    {PM4_WAIT_REG_GTE, "WAIT_REG_GTE"}, {PM4_INTERRUPT, "INTERRUPT"}, {PM4_SET_CONSTANT2, "SET_CONSTANT2"},
    {PM4_SET_SHADER_CONSTANTS, "SET_SHADER_CONSTANTS"}, {PM4_EVENT_WRITE_SHD, "EVENT_WRITE_SHD"},
    {PM4_EVENT_WRITE_CFL, "EVENT_WRITE_CFL"}, {PM4_EVENT_WRITE_EXT, "EVENT_WRITE_EXT"},
    {PM4_EVENT_WRITE_ZPD, "EVENT_WRITE_ZPD"}, {PM4_WAIT_UNTIL_READ, "WAIT_UNTIL_READ"},
    {PM4_WAIT_IB_PFD_COMPLETE, "WAIT_IB_PFD_COMPLETE"}, {PM4_CONTEXT_UPDATE, "CONTEXT_UPDATE"},
    {PM4_SET_BIN_MASK_LO, "SET_BIN_MASK_LO"}, {PM4_SET_BIN_MASK_HI, "SET_BIN_MASK_HI"},
    {PM4_SET_BIN_SELECT_LO, "SET_BIN_SELECT_LO"}, {PM4_SET_BIN_SELECT_HI, "SET_BIN_SELECT_HI"},
    {PM4_XE_SWAP, "XE_SWAP"},
};

const char* Pm4OpName(uint32_t op) {
    for (auto& o : kOps) if (o.op == op) return o.name;
    return nullptr;
}

// Registros usados (register_table.inc de Xenia).
constexpr uint32_t REG_COHER_STATUS_HOST = 0x0A31;
constexpr uint32_t REG_VGT_EVENT_INITIATOR = 0x21F9;
constexpr uint32_t REG_VGT_DRAW_INITIATOR = 0x21FC, REG_VGT_DMA_BASE = 0x21FA, REG_VGT_DMA_SIZE = 0x21FB;

static uint32_t GpuSwap32(uint32_t v, uint32_t endian) {
    switch (endian) {
        default: case 0: return v;
        case 1: return ((v << 8) & 0xFF00FF00) | ((v >> 8) & 0x00FF00FF);
        case 2: return __builtin_bswap32(v);
        case 3: return (v >> 16) | (v << 16);
    }
}

void Pm4Parser::Line(const char* fmt, ...) {
    if (logged_ >= logLimit_) return;
    ++logged_;
    char b[256];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(b, sizeof(b), fmt, ap);
    va_end(ap);
    host_->Log(b);
}

bool Pm4Parser::ExecuteRing(const uint8_t* ring, uint32_t capBytes, uint32_t rdIdx, uint32_t wrIdx) {
    uint32_t capD = capBytes / 4;
    if (!capD) return false;
    rdIdx %= capD; wrIdx %= capD;
    uint32_t remD = wrIdx >= rdIdx ? wrIdx - rdIdx : capD - rdIdx + wrIdx;
    Reader r{ring, capBytes, rdIdx * 4, remD * 4};
    return ExecuteBuffer(r, 0);
}

bool Pm4Parser::ExecuteBuffer(Reader& r, int depth) {
    while (r.rem) {
        if (!Packet(r, depth)) return false;
    }
    return true;
}

bool Pm4Parser::Packet(Reader& r, int depth) {
    uint32_t pkt = r.Read();
    ++total_.packets; ++frame_.packets;
    if (pkt == 0) { ++total_.type0; ++frame_.type0; return true; } // Xenia: paquete 0 = 1 dword sin efecto
    switch (pkt >> 30) {
        case 0: { // base 14:0, n = 29:16 + 1, bit 15 = una sola direccion
            ++total_.type0; ++frame_.type0;
            uint32_t count = ((pkt >> 16) & 0x3FFF) + 1, base = pkt & 0x7FFF, one = (pkt >> 15) & 1;
            if (r.rem < count * 4) { ++total_.overflow; ++frame_.overflow; Line("T0 overflow count=%u rem=%u", count, r.rem / 4); return false; }
            for (uint32_t m = 0; m < count; ++m) host_->WriteReg(one ? base : base + m, r.Read());
            return true;
        }
        case 1: { // dos registros de 11 bits
            ++total_.type1; ++frame_.type1;
            if (r.rem < 8) { ++total_.overflow; ++frame_.overflow; return false; }
            uint32_t d1 = r.Read(), d2 = r.Read();
            host_->WriteReg(pkt & 0x7FF, d1);
            host_->WriteReg((pkt >> 11) & 0x7FF, d2);
            return true;
        }
        case 2: ++total_.type2; ++frame_.type2; return true;
        default: return Type3(r, pkt, depth);
    }
}

bool Pm4Parser::Compare(uint32_t fn, uint32_t value, uint32_t mask, uint32_t ref) {
    uint32_t v = value & mask;
    switch (fn & 7) {
        case 0: return false;
        case 1: return v < ref;
        case 2: return v <= ref;
        case 3: return v == ref;
        case 4: return v != ref;
        case 5: return v >= ref;
        case 6: return v > ref;
        default: return true;
    }
}

bool Pm4Parser::Type3(Reader& r, uint32_t pkt, int depth) {
    ++total_.type3; ++frame_.type3;
    uint32_t opcode = (pkt >> 8) & 0x7F, count = ((pkt >> 16) & 0x3FFF) + 1;
    ++total_.op[opcode]; ++frame_.op[opcode];
    if (r.rem < count * 4) { ++total_.overflow; ++frame_.overflow; Line("T3 op=0x%02X overflow count=%u rem=%u", opcode, count, r.rem / 4); return false; }
    const uint32_t end = r.rem - count * 4; // bytes restantes al terminar este paquete
    const char* nm = Pm4OpName(opcode);
    if (!nm) { ++total_.unknownOp; ++frame_.unknownOp; Line("T3 UNKNOWN op=0x%02X count=%u", opcode, count); r.Skip(count); return true; }
    if (pkt & 1) { // predicado: comprobacion de "bin" (Xenia); los intercambios predicados se descartan
        bool pass = (bins_[0] & bins_[1]) != 0;
        if (!pass || opcode == PM4_XE_SWAP) { r.Skip(count); return true; }
    }
    bool ok = true, implemented = true;
    switch (opcode) {
        case PM4_ME_INIT: case PM4_NOP: case PM4_WAIT_FOR_IDLE: case PM4_INVALIDATE_STATE: case PM4_CONTEXT_UPDATE:
            r.Skip(count); break;
        case PM4_INTERRUPT: {
            uint32_t mask = r.Read();
            g_syncReason = 1;   // M60 diagnostico
            host_->SyncPoint();
            Line("INTERRUPT cpu_mask=0x%X", mask);
            host_->Interrupt(mask);
            break;
        }
        case PM4_XE_SWAP: {
            uint32_t magic = r.Read(), fb = r.Read(), w = r.Read(), h = r.Read();
            r.Skip(count - 4);
            Line("XE_SWAP magic=%08X fb=%08X %ux%u", magic, fb, w, h);
            g_syncReason = 2;   // M60 diagnostico
            host_->SyncPoint();
            host_->Swap(fb, w, h);
            EndFrame();
            break;
        }
        case PM4_INDIRECT_BUFFER: case PM4_INDIRECT_BUFFER_PFD: {
            uint32_t ptr = r.Read() & 0x1FFFFFFF, len = r.Read() & 0xFFFFF;
            uint8_t* p = host_->Phys(ptr, len * 4);
            Line("INDIRECT_BUFFER addr=%08X dwords=%u mapped=%d", ptr, len, p ? 1 : 0);
            if (!p) break;
            if (depth >= 8) { Line("INDIRECT_BUFFER anidado > 8"); break; }
            Reader ib{p, len * 4, 0, len * 4};
            if (len) ok = ExecuteBuffer(ib, depth + 1);
            break;
        }
        case PM4_WAIT_REG_MEM: {
            uint32_t wi = r.Read(), addr = r.Read(), ref = r.Read(), mask = r.Read(), wait = r.Read();
            Line("WAIT_REG_MEM info=%X addr=%08X ref=%X mask=%X", wi, addr, ref, mask);
            g_syncReason = (wi & 0x10) ? 15 : 3;   // M60: 15 = espera sobre memoria, 3 = sobre registro (p. ej. COHER_STATUS_HOST)
            g_syncAddr = (wi & 0x10) ? (addr & 0x1FFFFFFC) : addr;
            host_->SyncPoint();
            ok = Wait(wi, addr, ref, mask, wait);
            break;
        }
        case PM4_REG_RMW: {
            uint32_t info = r.Read(), andm = r.Read(), orm = r.Read();
            uint32_t v = host_->ReadReg(info & 0x1FFF);
            v &= ((info >> 31) & 1) ? host_->ReadReg(andm & 0x1FFF) : andm;
            v |= ((info >> 30) & 1) ? host_->ReadReg(orm & 0x1FFF) : orm;
            host_->WriteReg(info & 0x1FFF, v);
            break;
        }
        case PM4_REG_TO_MEM: {
            uint32_t reg = r.Read(), mem = r.Read();
            uint32_t v = GpuSwap32(host_->ReadReg(reg), mem & 3);
            g_syncReason = 4;   // M60 diagnostico
            host_->SyncPoint();
            if (uint8_t* p = host_->Phys(mem & ~3u, 4)) { if (!host_->WriteOrdered(mem & ~3u, &v, 4)) std::memcpy(p, &v, 4); host_->MemWritten(mem & ~3u, 4); }
            break;
        }
        case PM4_MEM_WRITE: {
            uint32_t addr = r.Read();
            g_syncReason = 5;   // M60 diagnostico
            host_->SyncPoint();
            if (count > 1) host_->MemWritten(addr & ~3u, (count - 1) * 4);
            for (uint32_t i = 0; i + 1 < count; ++i) {
                uint32_t d = GpuSwap32(r.Read(), addr & 3);
                if (uint8_t* p = host_->Phys(addr & ~3u, 4)) { if (!host_->WriteOrdered(addr & ~3u, &d, 4)) std::memcpy(p, &d, 4); }
                addr += 4;
            }
            break;
        }
        case PM4_COND_WRITE: {
            uint32_t wi = r.Read(), pa = r.Read(), ref = r.Read(), mask = r.Read(), wa = r.Read(), wd = r.Read();
            uint32_t value = 0;
            if (wi & 0x10) {
                host_->ReadBarrier(pa & ~3u, 4);   // M60: escrituras ordenadas pendientes en esa direccion
                uint32_t* p = reinterpret_cast<uint32_t*>(host_->Phys(pa & ~3u, 4));
                if (p) value = GpuSwap32(*p, pa & 3);
            } else value = host_->ReadReg(pa);
            if (Compare(wi, value, mask, ref)) {
                if (wi & 0x100) {
                    uint32_t d = GpuSwap32(wd, wa & 3);
                    g_syncReason = 6;   // M60 diagnostico
                    host_->SyncPoint();
                    if (uint8_t* p = host_->Phys(wa & ~3u, 4)) { if (!host_->WriteOrdered(wa & ~3u, &d, 4)) std::memcpy(p, &d, 4); host_->MemWritten(wa & ~3u, 4); }
                } else host_->WriteReg(wa, wd);
            }
            break;
        }
        case PM4_EVENT_WRITE: {
            uint32_t init = r.Read();
            host_->WriteReg(REG_VGT_EVENT_INITIATOR, init & 0x3F);
            r.Skip(count - 1);
            break;
        }
        case PM4_EVENT_WRITE_SHD: {
            uint32_t init = r.Read(), addr = r.Read(), value = r.Read();
            host_->WriteReg(REG_VGT_EVENT_INITIATOR, init & 0x3F);
            uint32_t d = ((init >> 31) & 1) ? (uint32_t)host_->Counter() : value;
            d = GpuSwap32(d, addr & 3);
            g_syncReason = 7;   // M60 diagnostico
            host_->SyncPoint();
            if (uint8_t* p = host_->Phys(addr & ~3u, 4)) { if (!host_->WriteOrdered(addr & ~3u, &d, 4)) std::memcpy(p, &d, 4); host_->MemWritten(addr & ~3u, 4); }
            break;
        }
        case PM4_EVENT_WRITE_EXT: {
            uint32_t init = r.Read(), addr = r.Read();
            host_->WriteReg(REG_VGT_EVENT_INITIATOR, init & 0x3F);
            // Xenia: extents fijos {0, 8192>>3, 0, 8192>>3, 0, 1} en 16 bits con intercambio de bytes
            // (valor 8192 de kTexture2DCubeMaxWidthHeight: [NV]).
            static const uint16_t ext[6] = {0, 8192 >> 3, 0, 8192 >> 3, 0, 1};
            g_syncReason = 8;   // M60 diagnostico
            host_->SyncPoint();
            if (uint8_t* p = host_->Phys(addr & ~3u, 12)) {
                uint8_t buf[12];
                for (int i = 0; i < 6; ++i) { uint16_t v = __builtin_bswap16(ext[i]); std::memcpy(buf + i * 2, &v, 2); }
                if (!host_->WriteOrdered(addr & ~3u, buf, 12)) std::memcpy(p, buf, 12);
                host_->MemWritten(addr & ~3u, 12);
            }
            r.Skip(count - 2);
            break;
        }
        case PM4_EVENT_WRITE_ZPD: {
            uint32_t init = r.Read();
            host_->WriteReg(REG_VGT_EVENT_INITIATOR, init & 0x3F);
            r.Skip(count - 1);
            break;
        }
        case PM4_DRAW_INDX: case PM4_DRAW_INDX_2: {
            uint32_t rem = count;
            if (opcode == PM4_DRAW_INDX && rem) { r.Read(); --rem; } // token de viz query
            if (!rem) break;
            uint32_t init = r.Read(); --rem;
            host_->WriteReg(REG_VGT_DRAW_INITIATOR, init);
            uint32_t src = (init >> 6) & 3, prim = init & 0x3F, nidx = init >> 16;
            DrawEvent ev;
            ev.opcode = opcode; ev.initiator = init; ev.prim = prim; ev.src = src; ev.nIdx = nidx;
            ev.majorMode = (init >> 8) & 3; ev.indexSize = (init >> 11) & 1;
            if (src == 0 && rem >= 2) { // DMA
                uint32_t base = r.Read(), size = r.Read(); rem -= 2;
                host_->WriteReg(REG_VGT_DMA_BASE, base); host_->WriteReg(REG_VGT_DMA_SIZE, size);
                ev.dmaBase = base; ev.dmaSize = size;
                Line("%s prim=%u n=%u dma_base=%08X dma_size=%08X", nm, prim, nidx, base, size);
            } else Line("%s prim=%u n=%u src=%u", nm, prim, nidx, src);
            r.Skip(rem);
            ++total_.draws; ++frame_.draws;
            host_->OnDraw(ev);
            if (opcode == PM4_DRAW_INDX) { ++total_.drawIndx; ++frame_.drawIndx; } else { ++total_.drawIndx2; ++frame_.drawIndx2; }
            break;
        }
        case PM4_SET_CONSTANT: {
            uint32_t ot = r.Read(), idx = ot & 0x7FF, type = (ot >> 16) & 0xFF;
            static const uint32_t basev[5] = {0x4000, 0x4800, 0x4900, 0x4908, 0x2000};
            if (type > 4) { r.Skip(count - 1); break; }
            idx += basev[type];
            for (uint32_t n = 0; n + 1 < count; ++n, ++idx) host_->WriteReg(idx, r.Read());
            break;
        }
        case PM4_SET_CONSTANT2: case PM4_SET_SHADER_CONSTANTS: {
            uint32_t idx = r.Read() & 0xFFFF;
            for (uint32_t n = 0; n + 1 < count; ++n, ++idx) host_->WriteReg(idx, r.Read());
            break;
        }
        case PM4_LOAD_ALU_CONSTANT: {
            uint32_t addr = r.Read() & 0x3FFFFFFF, ot = r.Read(), sz = r.Read() & 0xFFF;
            uint32_t idx = ot & 0x7FF, type = (ot >> 16) & 0xFF;
            static const uint32_t basev[5] = {0x4000, 0x4800, 0x4900, 0x4908, 0x2000};
            if (type > 4) break;
            idx += basev[type];
            const uint8_t* p = host_->Phys(addr & 0x1FFFFFFF, sz * 4);
            for (uint32_t n = 0; n < sz; ++n, ++idx)
                host_->WriteReg(idx, p ? __builtin_bswap32(*reinterpret_cast<const uint32_t*>(p + n * 4)) : 0);
            break;
        }
        case PM4_IM_LOAD: {
            uint32_t at = r.Read(), ss = r.Read();
            uint32_t type = at & 3, addr = (at & ~3u) & 0x1FFFFFFF, sz = ss & 0xFFFF;
            const uint8_t* p = host_->Phys(addr, sz * 4);
            ++total_.imLoad; ++frame_.imLoad;
            if (!p) { ++total_.imLoadUnmapped; ++frame_.imLoadUnmapped; }
            Line("IM_LOAD type=%s addr=%08X dwords=%u mapped=%d", type == 0 ? "VS" : type == 1 ? "PS" : "?", addr, sz, p ? 1 : 0);
            host_->OnShader(type, p, sz, addr, p != nullptr);
            break;
        }
        case PM4_IM_LOAD_IMMEDIATE: {
            uint32_t type = r.Read(), ss = r.Read(), sz = ss & 0xFFFF;
            std::vector<uint8_t> code;
            for (uint32_t i = 0; i < sz && i + 2 < count; ++i) {
                uint32_t w = __builtin_bswap32(r.Read());
                code.insert(code.end(), (uint8_t*)&w, (uint8_t*)&w + 4);
            }
            ++total_.imLoad; ++frame_.imLoad;
            Line("IM_LOAD_IMMEDIATE type=%s dwords=%u", type == 0 ? "VS" : type == 1 ? "PS" : "?", sz);
            host_->OnShader(type, code.data(), (uint32_t)(code.size() / 4), 0, true);
            break;
        }
        case PM4_VIZ_QUERY: r.Skip(count); break;
        case PM4_SET_BIN_MASK_LO: bins_[1] = (bins_[1] & 0xFFFFFFFF00000000ull) | r.Read(); break;
        case PM4_SET_BIN_MASK_HI: bins_[1] = (bins_[1] & 0xFFFFFFFFull) | ((uint64_t)r.Read() << 32); break;
        case PM4_SET_BIN_SELECT_LO: bins_[0] = (bins_[0] & 0xFFFFFFFF00000000ull) | r.Read(); break;
        case PM4_SET_BIN_SELECT_HI: bins_[0] = (bins_[0] & 0xFFFFFFFFull) | ((uint64_t)r.Read() << 32); break;
        case PM4_SET_BIN_MASK: { uint64_t hi = r.Read(), lo = r.Read(); bins_[1] = (hi << 32) | lo; break; }
        case PM4_SET_BIN_SELECT: { uint64_t hi = r.Read(), lo = r.Read(); bins_[0] = (hi << 32) | lo; break; }
        default: // conocido en xenos.h pero sin tratamiento en Xenia (descartado)
            implemented = false;
            r.Skip(count);
            break;
    }
    if (!implemented) {
        ++total_.unimplemented; ++frame_.unimplemented;
        Line("T3 %s (0x%02X) sin tratamiento (descartado) count=%u", nm, opcode, count);
    }
    if (r.rem > end) r.Skip((r.rem - end) / 4); // el manejador leyo menos que el paquete
    return ok;
}

// M8: tiempo de espera de WAIT_REG_MEM por direccion (solo con --perf)
struct WaitAcc { std::atomic<uint32_t> addr{0}; std::atomic<uint64_t> ticks{0}, n{0}, maxT{0}; };
static WaitAcc g_waitAcc[32];
void Pm4WaitReport(void (*out)(const char*)) {
    LARGE_INTEGER f; QueryPerformanceFrequency(&f);
    for (auto& w : g_waitAcc) {
        if (!w.n) continue;
        char b[200];
        std::snprintf(b, sizeof(b), "PERF cp_wait addr=%08X n=%llu total_ms=%.1f media_us=%.1f max_ms=%.2f", w.addr.load(), (unsigned long long)w.n.load(),
                      w.ticks * 1000.0 / f.QuadPart, w.ticks * 1e6 / f.QuadPart / w.n, w.maxT * 1000.0 / f.QuadPart);
        out(b);
    }
}
bool Pm4Parser::Wait(uint32_t wi, uint32_t pa, uint32_t ref, uint32_t mask, uint32_t wait) {
    struct T { uint32_t pa; uint64_t t0; ~T() {
        if (!perf::g_enabled) return;
        uint64_t d = perf::Now() - t0;
        for (auto& w : g_waitAcc) {
            uint32_t a = w.addr.load();
            if (a == 0) { uint32_t z = 0; if (!w.addr.compare_exchange_strong(z, pa) && z != pa) continue; }
            else if (a != pa) continue;
            w.ticks += d; ++w.n; if (d > w.maxT) w.maxT = d;
            break;
        }
    } } tm{pa, perf::g_enabled ? perf::Now() : 0};
    (void)wait;
    bool mem = (wi & 0x10) != 0;
    for (;;) {
        uint32_t v;
        if (mem) {
            uint32_t* p = reinterpret_cast<uint32_t*>(host_->Phys(pa & ~3u, 4));
            if (!p) { Line("WAIT_REG_MEM memoria no asignada %08X", pa); return true; }
            v = GpuSwap32(*reinterpret_cast<volatile uint32_t*>(p), pa & 3);
        } else {
            if (pa == REG_COHER_STATUS_HOST && (host_->ReadReg(pa) & 0x80000000u)) host_->WriteReg(pa, 0); // MakeCoherent
            v = host_->ReadReg(pa);
        }
        if (Compare(wi, v, mask, ref)) return true;
        if (!host_->Poll()) return false;
    }
}

void Pm4Parser::FlushPartial() {
    if (!frame_.packets) return;
    char b[256];
    std::snprintf(b, sizeof(b), "frame=partial packets=%llu draws=%llu (DRAW_INDX=%llu DRAW_INDX_2=%llu)",
                  (unsigned long long)frame_.packets, (unsigned long long)frame_.draws, (unsigned long long)frame_.drawIndx,
                  (unsigned long long)frame_.drawIndx2);
    host_->FrameDone(b);
    frame_ = Pm4Stats();
}

void Pm4Parser::EndFrame() {
    ++total_.frames; ++frame_.frames;
    char b[1400];
    int n = std::snprintf(b, sizeof(b),
        "frame=%llu packets=%llu t0=%llu t1=%llu t2=%llu t3=%llu draws=%llu (DRAW_INDX=%llu DRAW_INDX_2=%llu) UNKNOWN=%llu sin_tratar=%llu im_load=%llu (sin_memoria=%llu) overflow=%llu ops:",
        (unsigned long long)total_.frames, (unsigned long long)frame_.packets, (unsigned long long)frame_.type0,
        (unsigned long long)frame_.type1, (unsigned long long)frame_.type2, (unsigned long long)frame_.type3,
        (unsigned long long)frame_.draws, (unsigned long long)frame_.drawIndx, (unsigned long long)frame_.drawIndx2,
        (unsigned long long)frame_.unknownOp, (unsigned long long)frame_.unimplemented, (unsigned long long)frame_.imLoad,
        (unsigned long long)frame_.imLoadUnmapped, (unsigned long long)frame_.overflow);
    for (uint32_t i = 0; i < 128 && n < (int)sizeof(b) - 40; ++i)
        if (frame_.op[i])
            n += std::snprintf(b + n, sizeof(b) - n, " %s=%llu", Pm4OpName(i) ? Pm4OpName(i) : "UNKNOWN", (unsigned long long)frame_.op[i]);
    host_->FrameDone(b);
    frame_ = Pm4Stats();
    logged_ = 0;
}

} // namespace rt::gpu
