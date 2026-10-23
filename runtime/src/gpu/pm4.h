#pragma once
// Analizador PM4 (Fase 3): decodifica paquetes del anillo/IB, actualiza el fichero de registros y registra;
// no dibuja. Semantica tomada de Xenia (command_processor.cc, xenos.h; copia local en runtime\ref\).
#include <cstdint>
#include <string>
#include <vector>

namespace rt::gpu {

constexpr uint32_t kRegCount = 0x5003; // Xenia register_file.h: kRegisterCount

// Opcodes Type-3 (xenos.h, enum Type3Opcode).
enum Pm4Op : uint32_t {
    PM4_NOP = 0x10, PM4_REG_RMW = 0x21, PM4_DRAW_INDX = 0x22, PM4_VIZ_QUERY = 0x23, PM4_SET_STATE = 0x25,
    PM4_WAIT_FOR_IDLE = 0x26, PM4_IM_LOAD = 0x27, PM4_IM_LOAD_IMMEDIATE = 0x2B, PM4_IM_STORE = 0x2C,
    PM4_SET_CONSTANT = 0x2D, PM4_LOAD_CONSTANT_CONTEXT = 0x2E, PM4_LOAD_ALU_CONSTANT = 0x2F,
    PM4_DRAW_INDX_BIN = 0x34, PM4_DRAW_INDX_2_BIN = 0x35, PM4_DRAW_INDX_2 = 0x36, PM4_INDIRECT_BUFFER_PFD = 0x37,
    PM4_INVALIDATE_STATE = 0x3B, PM4_WAIT_REG_MEM = 0x3C, PM4_MEM_WRITE = 0x3D, PM4_REG_TO_MEM = 0x3E,
    PM4_INDIRECT_BUFFER = 0x3F, PM4_COND_EXEC = 0x44, PM4_COND_WRITE = 0x45, PM4_EVENT_WRITE = 0x46,
    PM4_ME_INIT = 0x48, PM4_SET_SHADER_BASES = 0x4A, PM4_SET_BIN_BASE_OFFSET = 0x4B, PM4_MEM_WRITE_CNTR = 0x4F,
    PM4_SET_BIN_MASK = 0x50, PM4_SET_BIN_SELECT = 0x51, PM4_WAIT_REG_EQ = 0x52, PM4_WAIT_REG_GTE = 0x53,
    PM4_INTERRUPT = 0x54, PM4_SET_CONSTANT2 = 0x55, PM4_SET_SHADER_CONSTANTS = 0x56, PM4_EVENT_WRITE_SHD = 0x58,
    PM4_EVENT_WRITE_CFL = 0x59, PM4_EVENT_WRITE_EXT = 0x5A, PM4_EVENT_WRITE_ZPD = 0x5B, PM4_WAIT_UNTIL_READ = 0x5C,
    PM4_WAIT_IB_PFD_COMPLETE = 0x5D, PM4_CONTEXT_UPDATE = 0x5E, PM4_SET_BIN_MASK_LO = 0x60,
    PM4_SET_BIN_MASK_HI = 0x61, PM4_SET_BIN_SELECT_LO = 0x62, PM4_SET_BIN_SELECT_HI = 0x63, PM4_XE_SWAP = 0x64,
};

const char* Pm4OpName(uint32_t op); // nullptr si el opcode no esta en la tabla de xenos.h

// Evento de dibujo (VGT_DRAW_INITIATOR de DRAW_INDX / DRAW_INDX_2).
struct DrawEvent {
    uint32_t opcode = 0, initiator = 0, prim = 0, src = 0, nIdx = 0, indexSize = 0, majorMode = 0;
    uint32_t dmaBase = 0, dmaSize = 0;   // solo src == 0 (DMA)
};

// Operaciones del procesador de comandos que el analizador necesita.
struct Pm4Host {
    virtual ~Pm4Host() {}
    virtual void WriteReg(uint32_t idx, uint32_t v) = 0;
    virtual uint32_t ReadReg(uint32_t idx) = 0;
    virtual uint8_t* Phys(uint32_t phys, uint32_t bytes) = 0;       // puntero host o nullptr si no hay memoria confirmada
    virtual void Interrupt(uint32_t cpuMask) = 0;
    virtual void Swap(uint32_t fbPhys, uint32_t w, uint32_t h) = 0; // XE_SWAP
    virtual uint64_t Counter() = 0;
    virtual bool Poll() = 0;                                        // espera de WAIT_REG_MEM: false = abortar
    virtual void OnShader(uint32_t type, const uint8_t* beBytes, uint32_t dwords, uint32_t addr, bool mapped) = 0;
    virtual void Log(const char* line) = 0;                         // linea de gpu.log
    virtual void FrameDone(const char* summary) = 0;                // fin de fotograma (tras XE_SWAP)
    virtual void OnDraw(const DrawEvent&) {}                        // tras actualizar VGT_DRAW_INITIATOR/DMA
    virtual void MemWritten(uint32_t /*phys*/, uint32_t /*bytes*/) {}   // M8: el CP escribio memoria guest (invalidacion de caches)
    virtual void SyncPoint() {}                                     // M8: punto en que la CPU guest puede observar el trabajo previo
    // M60: escritura del CP en memoria guest ordenada tras los resolves pendientes (semantica de fin de canalizacion). true = encolada
    // por el backend (el CP no debe escribir); false = escribir ahora. ReadBarrier: el CP va a leer [phys, phys+bytes).
    virtual bool WriteOrdered(uint32_t /*phys*/, const void* /*data*/, uint32_t /*bytes*/) { return false; }
    virtual void ReadBarrier(uint32_t /*phys*/, uint32_t /*bytes*/) {}
};

struct Pm4Stats {
    uint64_t packets = 0, type0 = 0, type1 = 0, type2 = 0, type3 = 0;
    uint64_t op[128] = {};
    uint64_t unknownOp = 0;      // opcodes no presentes en xenos.h
    uint64_t unimplemented = 0;  // opcodes conocidos que Xenia descarta o que no se interpretan
    uint64_t draws = 0, drawIndx = 0, drawIndx2 = 0;
    uint64_t imLoad = 0, imLoadUnmapped = 0;
    uint64_t overflow = 0;       // paquetes truncados
    uint64_t frames = 0;
};

class Pm4Parser {
public:
    explicit Pm4Parser(Pm4Host* h) : host_(h) {}
    // Ejecuta [rd, wr) (indices en dwords) del anillo de `cap` bytes en host `ring`. Devuelve false si hubo error.
    bool ExecuteRing(const uint8_t* ring, uint32_t capBytes, uint32_t rdIdx, uint32_t wrIdx);
    Pm4Stats& Stats() { return total_; }
    Pm4Stats& Frame() { return frame_; }
    void SetLogLimit(uint32_t perFrame) { logLimit_ = perFrame; }
    void FlushPartial();   // emite el resumen del fotograma en curso (sin XE_SWAP) al detener el CP

private:
    struct Reader {
        const uint8_t* buf; uint32_t cap; uint32_t rd; uint32_t rem; // rem en bytes
        uint32_t Read() { uint32_t v = __builtin_bswap32(*reinterpret_cast<const uint32_t*>(buf + rd)); rd += 4; if (rd >= cap) rd = 0; rem -= 4; return v; }
        void Skip(uint32_t dwords) { rd = (rd + dwords * 4) % cap; rem -= dwords * 4; }
    };
    bool ExecuteBuffer(Reader& r, int depth);
    bool Packet(Reader& r, int depth);
    bool Type3(Reader& r, uint32_t pkt, int depth);
    bool Wait(uint32_t waitInfo, uint32_t pollAddr, uint32_t ref, uint32_t mask, uint32_t wait);
    bool Compare(uint32_t fn, uint32_t value, uint32_t mask, uint32_t ref);
    void Line(const char* fmt, ...);
    void EndFrame();
    Pm4Host* host_;
    Pm4Stats total_, frame_;
    uint32_t logLimit_ = 48, logged_ = 0;
    uint64_t bins_[2] = {0, 0};
};

} // namespace rt::gpu
