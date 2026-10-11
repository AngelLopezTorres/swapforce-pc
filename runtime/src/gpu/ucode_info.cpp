#include "ucode_info.h"

#include <algorithm>
#include <cstring>

#include "xenia/gpu/ucode.h"

namespace rt::gpu {
using namespace xe::gpu::ucode;

uint64_t HashUcode(const uint8_t* b, uint32_t dwords, uint32_t type) {
    uint64_t h = 1469598103934665603ull;
    for (uint32_t i = 0; i < dwords * 4; ++i) { h ^= b[i]; h *= 1099511628211ull; }
    h ^= type;
    return h;
}

bool AnalyzeUcode(bool vertex, const uint8_t* be, uint32_t dwords, ShaderInfo& o) {
    o = ShaderInfo();
    o.vertex = vertex;
    o.dwords = dwords;
    o.hash = HashUcode(be, dwords, vertex ? 0 : 1);
    o.ucode.resize(dwords);
    for (uint32_t i = 0; i < dwords; ++i) {
        uint32_t w; std::memcpy(&w, be + i * 4, 4);
        o.ucode[i] = __builtin_bswap32(w);
    }
    if (dwords < 3) { o.error = "microcodigo demasiado corto"; return false; }
    const uint32_t* u = o.ucode.data();
    uint32_t bound = dwords / 3;
    for (uint32_t i = 0; i < bound; ++i) {
        ControlFlowInstruction ab[2];
        UnpackControlFlowInstructions(u + i * 3, ab);
        for (int j = 0; j < 2; ++j)
            if (IsControlFlowOpcodeExec(ab[j].opcode())) bound = std::min<uint32_t>(bound, ab[j].exec.address());
    }
    o.cfPairBound = bound;
    VertexFetchInstruction prevFull;
    std::memset(&prevFull, 0, sizeof(prevFull));
    auto regUse = [&](uint32_t reg, bool rel) { if (rel) o.dynamicRegs = true; else o.regBound = std::max(o.regBound, reg + 1); };
    for (uint32_t i = 0; i < bound; ++i) {
        ControlFlowInstruction ab[2];
        UnpackControlFlowInstructions(u + i * 3, ab);
        for (int j = 0; j < 2; ++j) {
            const ControlFlowInstruction& cf = ab[j];
            uint32_t addr = 0, count = 0, seq = 0;
            bool isExec = true;
            switch (cf.opcode()) {
                case ControlFlowOpcode::kExec: case ControlFlowOpcode::kExecEnd:
                    addr = cf.exec.address(); count = cf.exec.count(); seq = cf.exec.sequence(); break;
                case ControlFlowOpcode::kCondExec: case ControlFlowOpcode::kCondExecEnd:
                case ControlFlowOpcode::kCondExecPredClean: case ControlFlowOpcode::kCondExecPredCleanEnd:
                    addr = cf.cond_exec.address(); count = cf.cond_exec.count(); seq = cf.cond_exec.sequence();
                    { uint32_t b = cf.cond_exec.bool_address(); o.boolBitmap[(b / 32) & 7] |= 1u << (b % 32); ++o.condCount; }
                    break;
                case ControlFlowOpcode::kCondExecPred: case ControlFlowOpcode::kCondExecPredEnd:
                    addr = cf.cond_exec_pred.address(); count = cf.cond_exec_pred.count(); seq = cf.cond_exec_pred.sequence(); ++o.condCount; break;
                case ControlFlowOpcode::kLoopStart: ++o.loopCount; o.loopBitmap |= 1u << cf.loop_start.loop_id(); isExec = false; break;
                case ControlFlowOpcode::kLoopEnd: o.loopBitmap |= 1u << cf.loop_end.loop_id(); isExec = false; break;
                case ControlFlowOpcode::kCondCall: ++o.callCount; isExec = false;
                    if (!cf.cond_call.is_unconditional() && !cf.cond_call.is_predicated()) { uint32_t b = cf.cond_call.bool_address(); o.boolBitmap[(b / 32) & 7] |= 1u << (b % 32); }
                    break;
                case ControlFlowOpcode::kCondJmp: ++o.jmpCount; isExec = false;
                    if (!cf.cond_jmp.is_unconditional() && !cf.cond_jmp.is_predicated()) { uint32_t b = cf.cond_jmp.bool_address(); o.boolBitmap[(b / 32) & 7] |= 1u << (b % 32); }
                    break;
                default: isExec = false; break;
            }
            if (!isExec) continue;
            ++o.execCount;
            for (uint32_t k = 0; k < count; ++k, seq >>= 2) {
                uint32_t off = addr + k;
                if ((off + 1) * 3 > dwords) { o.error = "exec fuera del microcodigo"; return false; }
                const uint32_t* p = u + off * 3;
                if (seq & 1) {
                    ++o.fetchCount;
                    const FetchInstruction& f = *reinterpret_cast<const FetchInstruction*>(p);
                    if (f.opcode() == FetchOpcode::kVertexFetch) {
                        const VertexFetchInstruction& v = f.vertex_fetch();
                        const VertexFetchInstruction& full = v.is_mini_fetch() ? prevFull : v;
                        if (!v.is_mini_fetch()) prevFull = v;
                        uint32_t ci = full.fetch_constant_index();
                        o.vfetchMask[ci / 32] |= 1u << (ci % 32);
                        o.vfetch.push_back({ci, (uint32_t)v.data_format(), full.stride(), v.is_mini_fetch()});
                        regUse(v.dest(), v.is_dest_relative());
                        if (!v.is_mini_fetch()) regUse(full.src(), full.is_src_relative());
                    } else {
                        const TextureFetchInstruction& t = f.texture_fetch();
                        uint32_t op = (uint32_t)t.opcode();
                        if (op != 24 && op != 25 && op != 26) { o.tfetchMask |= 1u << t.fetch_constant_index(); }
                        o.tfetch.push_back({t.fetch_constant_index(), (uint32_t)t.dimension(), op});
                        regUse(t.dest(), t.is_dest_relative());
                        regUse(t.src(), t.is_src_relative());
                    }
                } else {
                    ++o.aluCount;
                    const AluInstruction& a = *reinterpret_cast<const AluInstruction*>(p);
                    const auto& vi = GetAluVectorOpcodeInfo(a.vector_opcode());
                    const auto& si = GetAluScalarOpcodeInfo(a.scalar_opcode());
                    if ((vi.changed_state & kAluOpChangedStatePixelKill) || (si.changed_state & kAluOpChangedStatePixelKill)) o.kills = true;
                    uint32_t vm = a.GetVectorOpResultWriteMask(), sm = a.GetScalarOpResultWriteMask();
                    if (a.is_export()) {
                        uint32_t d = a.vector_dest();
                        if (d == (uint32_t)ExportRegister::kExportAddress) o.memexportAddr = true;
                        else if (d >= (uint32_t)ExportRegister::kExportData0 && d <= (uint32_t)ExportRegister::kExportData4) o.memexportData |= 1u << (d - 33);
                        else if (vertex) {
                            if (d < 16) o.writesInterp |= 1u << d;
                            else if (d == 62) o.writesPos = true;
                            else if (d == 63) o.writesPointSize = true;
                        } else {
                            if (d < 4) o.writesColors |= 1u << d;
                            else if (d == 61) o.writesDepth = true;
                        }
                    } else {
                        if (vm) regUse(a.vector_dest(), a.is_vector_dest_relative());
                        if (sm) regUse(a.scalar_dest(), a.is_scalar_dest_relative());
                    }
                    uint32_t nv = vi.GetOperandCount();
                    auto operand = [&](uint32_t i) {
                        uint32_t reg = a.src_reg(i);
                        if (a.src_is_temp(i)) regUse(AluInstruction::src_temp_reg(reg), AluInstruction::is_src_temp_relative(reg));
                        else {
                            if (a.src_const_is_addressed(i)) o.floatDynamic = true;
                            else o.floatBitmap[(reg >> 6) & 3] |= 1ull << (reg & 63);
                        }
                    };
                    for (uint32_t i = 1; i <= nv; ++i) operand(i);
                    if (si.operand_count == 1) operand(3);
                    else if (si.operand_count == 2) {
                        if (a.src_const_is_addressed(3)) o.floatDynamic = true;
                        else o.floatBitmap[(a.src_reg(3) >> 6) & 3] |= 1ull << (a.src_reg(3) & 63);
                        regUse(a.scalar_const_reg_op_src_temp_reg(), false);
                    }
                }
            }
        }
    }
    return true;
}

} // namespace rt::gpu
