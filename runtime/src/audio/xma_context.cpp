#include "xma_context.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstring>
#include <mutex>

#include "guest_abi.h"
#include "guest_heap.h"
#include "guest_memory.h"
#include "import_log.h"
#include "xma_ffmpeg.h"

#include <cstdio>
#include <memory>
#include <vector>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

namespace rt::audio {
namespace {

constexpr uint32_t kPhysicalMask = 0x1FFFFFFF;
constexpr uint32_t kXmaRegisterBase = 0x7FEA0000;
constexpr uint32_t kXmaRegisterSize = 0x10000;

std::mutex g_mu;
uint32_t g_contextArray = 0;
std::array<bool, kXmaContextCount> g_allocated{};
std::array<bool, kXmaContextCount> g_consumedInput{};
uint64_t g_decodeRequests = 0;
uint64_t g_simulatedPackets = 0;
std::atomic<bool> g_workerRunning{false};
HANDLE g_worker = nullptr;

// Estado de decodificacion por contexto (A2).
struct CtxState {
    std::unique_ptr<rt::audio::FfXmaDecoder> dec;
    int ch = 0, rate = 0, spf = 512;
    std::vector<int16_t> pend;      // PCM decodificado pendiente de escribir (int16 entrelazado, endian host)
    size_t pendPos = 0;
    uint32_t pktIdx = 0;            // siguiente paquete del buffer actual que se enviara al decodificador
    uint32_t expectOff = 0xFFFFFFFFu;   // ultimo desplazamiento de lectura escrito por el motor
    bool consumed = false;          // se consumio entrada desde la ultima vez que se cerro output_buffer_valid
    FILE* cap = nullptr;
    uint32_t capSeq = 0;
    void Reset() { dec.reset(); ch = rate = 0; spf = 512; pend.clear(); pendPos = 0; pktIdx = 0; expectOff = 0xFFFFFFFFu; consumed = false; if (cap) { std::fclose(cap); cap = nullptr; } }
};
std::array<CtxState, kXmaContextCount> g_cs;
XmaStats g_stats;
std::string g_capDir;
uint64_t g_dbgLines = 0;
bool g_trace = false;
bool g_forceSilence = false;   // --xma=silence: sumidero silencioso aunque haya libavcodec (medicion de referencia)   // --xma-trace: registra cada cambio de estado del contexto (primeras 700 lineas)
uint64_t g_traceLines = 0;

uint32_t ContextAddress(uint32_t index) {
    return g_contextArray + index * kXmaContextBytes;
}

bool InitializeLocked() {
    if (g_contextArray) return true;
    uint8_t* base = GuestMemory::Base();
    if (!base) return false;

    uint32_t address = PhysicalHeap().Alloc(kXmaContextCount * kXmaContextBytes, 256);
    if (!address) return false;

    g_contextArray = address;
    P32(base, kXmaContextArrayAddressRegister, address & kPhysicalMask);
    P32(base, kXmaCurrentContextIndexRegister, 0);
    P32(base, kXmaNextContextIndexRegister, 1);
    return true;
}

bool FindContextLocked(uint32_t address, uint32_t& index) {
    if (!g_contextArray || address < g_contextArray) return false;
    uint32_t delta = address - g_contextArray;
    if ((delta % kXmaContextBytes) != 0) return false;
    index = delta / kXmaContextBytes;
    return index < kXmaContextCount && g_allocated[index];
}

} // namespace

uint32_t CreateXmaContext() {
    std::lock_guard<std::mutex> lock(g_mu);
    if (!InitializeLocked()) return 0;
    uint8_t* base = GuestMemory::Base();
    for (uint32_t i = 0; i < kXmaContextCount; ++i) {
        if (g_allocated[i]) continue;
        g_allocated[i] = true;
        g_consumedInput[i] = false;
        g_cs[i].Reset();
        std::memset(base + ContextAddress(i), 0, kXmaContextBytes);
        return ContextAddress(i);
    }
    return 0;
}

bool ReleaseXmaContext(uint32_t guestAddress) {
    std::lock_guard<std::mutex> lock(g_mu);
    uint32_t index = 0;
    if (!FindContextLocked(guestAddress, index)) return false;
    std::memset(GuestMemory::Base() + guestAddress, 0, kXmaContextBytes);
    g_allocated[index] = false;
    g_consumedInput[index] = false;
    g_cs[index].Reset();
    return true;
}

uint32_t XmaContextArrayGuestAddress() {
    std::lock_guard<std::mutex> lock(g_mu);
    return InitializeLocked() ? g_contextArray : 0;
}

bool WriteXmaRegister(uint32_t guestAddress, uint32_t value) {
    std::lock_guard<std::mutex> lock(g_mu);
    if (!GuestMemory::Base() || guestAddress < kXmaRegisterBase ||
        guestAddress + sizeof(uint32_t) > kXmaRegisterBase + kXmaRegisterSize ||
        (guestAddress & 3)) return false;

    P32(GuestMemory::Base(), guestAddress, value);
    if (guestAddress >= kXmaContext0KickRegister &&
        guestAddress < kXmaContext0KickRegister + kXmaContextKickRegisterCount * 4) {
        uint32_t group = (guestAddress - kXmaContext0KickRegister) / 4;
        uint32_t bits = value;
        while (bits) {
            uint32_t bit = 0;
            uint32_t lowest = bits & (~bits + 1);
            while ((1u << bit) != lowest) ++bit;
            uint32_t index = group * 32 + bit;
            if (index < kXmaContextCount && g_allocated[index]) ++g_decodeRequests;
            bits &= bits - 1;
        }
    }
    return true;
}

uint32_t ReadXmaRegister(uint32_t guestAddress) {
    std::lock_guard<std::mutex> lock(g_mu);
    if (!GuestMemory::Base() || guestAddress < kXmaRegisterBase ||
        guestAddress + sizeof(uint32_t) > kXmaRegisterBase + kXmaRegisterSize ||
        (guestAddress & 3)) return 0;
    return G32(GuestMemory::Base(), guestAddress);
}

XmaDecodeBoundaryResult SubmitXmaDecode(uint32_t contextGuestAddress) {
    std::lock_guard<std::mutex> lock(g_mu);
    uint32_t index = 0;
    if (!FindContextLocked(contextGuestAddress, index)) return XmaDecodeBoundaryResult::InvalidContext;
    ++g_decodeRequests;
    return FfAvailable() ? XmaDecodeBoundaryResult::Queued : XmaDecodeBoundaryResult::BackendUnavailable;
}

uint64_t XmaDecodeRequestCount() {
    std::lock_guard<std::mutex> lock(g_mu);
    return g_decodeRequests;
}

bool XmaDecoderBackendAvailable() { return FfAvailable(); }

uint32_t SimulateXmaStep() {
    std::lock_guard<std::mutex> lock(g_mu);
    if (!g_contextArray || !GuestMemory::Base()) return 0;
    uint8_t* base = GuestMemory::Base();
    uint32_t consumed = 0;
    for (uint32_t i = 0; i < kXmaContextCount; ++i) {
        if (!g_allocated[i]) continue;
        uint32_t address = ContextAddress(i);
        uint32_t w0 = G32(base, address);
        uint32_t w1 = G32(base, address + 4);
        if (!(w1 & 0x80000000u)) continue; // output_buffer_valid
        uint32_t valid = (w0 >> 20) & 3;
        if (!valid) {
            if (g_consumedInput[i]) {
                P32(base, address + 4, w1 & ~0x80000000u);
                g_consumedInput[i] = false;
            }
            continue;
        }
        uint32_t blocks = (w0 >> 22) & 31;
        if (!blocks) continue;
        uint32_t w4 = G32(base, address + 16);
        uint32_t current = w4 >> 31;
        if (!(valid & (1u << current))) current ^= 1;
        uint32_t packets = current ? (w1 & 0xFFF) : (w0 & 0xFFF);
        if (!packets) continue;

        uint32_t outputPhysical = G32(base, address + 28) & kPhysicalMask;
        uint32_t capacity = blocks * 256;
        if (!outputPhysical || outputPhysical > 0x20000000u - capacity) continue;
        uint32_t bytes = std::min(capacity, (w1 & (1u << 29)) ? 2048u : 1024u);
        uint32_t write = (w0 >> 27) & 31;
        if (write >= blocks) write = 0;
        uint32_t offset = write * 256;
        uint32_t first = std::min(bytes, capacity - offset);
        uint8_t* output = base + 0xA0000000u + outputPhysical;
        std::memset(output + offset, 0, first);
        if (first < bytes) std::memset(output, 0, bytes - first);
        uint32_t nextWrite = (write + bytes / 256) % blocks;
        w0 = (w0 & ~(0x1Fu << 27) & ~(1u << (20 + current))) | (nextWrite << 27);
        P32(base, address, w0);
        P32(base, address + 8, packets * kXmaPacketBytes * 8);
        P32(base, address + 16, (w4 & 0x7FFFFFFFu) | ((current ^ 1) << 31));
        g_consumedInput[i] = true;
        ++g_simulatedPackets;
        ++consumed;
        if (g_simulatedPackets <= 16 || (g_simulatedPackets & 1023) == 0)
            ImportLog::Get().Printf("XMA SILENCE ctx=%u packets=%u bytes=%u write=%u valid=%u total=%llu",
                                    i, packets, bytes, nextWrite, (w0 >> 20) & 3,
                                    static_cast<unsigned long long>(g_simulatedPackets));
    }
    return consumed;
}

uint64_t XmaSimulatedPacketCount() {
    std::lock_guard<std::mutex> lock(g_mu);
    return g_simulatedPackets;
}

// ---- Decodificacion real (A2) -------------------------------------------------------------------------------------------
// Semantica (Xenia xma_context.h/xma_decoder.cc; xma_context.cc no esta en runtime\ref, [NV] el detalle fino):
//  - contexto de 64 B en big-endian: dword0 = paquetes buffer0 (12 b) | loop_count (8) | in0_valid (b20) | in1_valid (b21) |
//    bloques de salida (5, b22) | desplazamiento de escritura de salida en bloques de 256 B (5, b27); dword1 = paquetes buffer1 |
//    ... | sample_rate (2, b27) | is_stereo (b29) | output_buffer_valid (b31); dword2 = desplazamiento de lectura de entrada (26 b, en bits);
//    dword4 b31 = buffer actual; dword5/6 = direcciones FISICAS de los buffers de entrada; dword7 = direccion fisica de salida;
//    dword9 b0-4 = desplazamiento de lectura de salida (bloques de 256 B).
//  - cada paquete XMA2 mide 2048 B; el decodificador devuelve tramas de 512 muestras por canal en PCM 16 bit big-endian.
//  - al agotar un buffer de entrada: se limpia su bit de validez, se alterna current_buffer y el desplazamiento de lectura pasa a 32
//    (Xenia SwapInputBuffer) [NV: fuente de memoria; coincide con el valor 0x20 observado en el juego].
//  - output_buffer_valid se cierra cuando no queda entrada ni PCM pendiente tras haber consumido entrada (igual que el sumidero previo).
namespace {

inline void Rmw(uint8_t* base, uint32_t a, uint32_t clearMask, uint32_t setBits) {
    volatile LONG* p = reinterpret_cast<volatile LONG*>(base + a);
    for (;;) {
        LONG old = *p;
        uint32_t v = _byteswap_ulong(static_cast<uint32_t>(old));
        v = (v & ~clearMask) | setBits;
        LONG nw = static_cast<LONG>(_byteswap_ulong(v));
        if (InterlockedCompareExchange(p, nw, old) == old) return;
    }
}

bool PhysOk(uint32_t phys, uint32_t bytes) { return phys && static_cast<uint64_t>(phys) + bytes <= 0x20000000ull; }

// Una pasada sobre el contexto `i` (g_mu retenido). Devuelve true si hubo progreso.
bool DecodeCtxLocked(uint32_t i, uint8_t* base) {
    CtxState& st = g_cs[i];
    const uint32_t a = ContextAddress(i);
    uint32_t w0 = G32(base, a), w1 = G32(base, a + 4);
    if (g_trace && g_traceLines < 700) {
        static uint32_t last[4] = {};
        uint32_t cw[4] = {w0, w1, G32(base, a + 8), G32(base, a + 16)};
        if (std::memcmp(cw, last, sizeof cw) != 0) {
            std::memcpy(last, cw, sizeof cw); ++g_traceLines;
            ImportLog::Get().Printf("XMA TRACE ctx=%u w0=%08X w1=%08X w2=%08X w4=%08X w9=%08X p5=%08X p6=%08X pktIdx=%u expect=%d pend=%zu", i, cw[0], cw[1], cw[2], cw[3], G32(base, a + 36), G32(base, a + 20), G32(base, a + 24),
                                    st.pktIdx, (int)st.expectOff, st.pend.size() - st.pendPos);
        }
    }
    if (!(w1 >> 31)) return false;                        // output_buffer_valid = 0: el decodificador no escribe
    bool valid[2] = {((w0 >> 20) & 1) != 0, ((w0 >> 21) & 1) != 0};
    if (!valid[0] && !valid[1] && st.pend.size() == st.pendPos) {
        if (st.consumed) { Rmw(base, a + 4, 0x80000000u, 0); st.consumed = false; return true; }
        return false;
    }
    const int channels = ((w1 >> 29) & 1) ? 2 : 1;
    static const int kRates[4] = {24000, 32000, 44100, 48000};
    const int rate = kRates[(w1 >> 27) & 3];
    if (!st.dec || st.ch != channels || st.rate != rate) {
        st.dec = std::make_unique<FfXmaDecoder>();
        if (!st.dec->Open(channels, rate)) {
            ImportLog::Get().Printf("XMA: no se pudo abrir el decodificador ctx=%u ch=%d rate=%d err=%d", i, channels, rate, st.dec->lastError());
            st.dec.reset(); ++g_stats.decodeErrors; return false;
        }
        st.ch = channels; st.rate = rate; st.pend.clear(); st.pendPos = 0; st.pktIdx = 0; st.expectOff = 0xFFFFFFFFu;
        if (g_dbgLines++ < 60) ImportLog::Get().Printf("XMA: decodificador abierto ctx=%u canales=%d frecuencia=%d", i, channels, rate);
    }
    const uint32_t blocks = (w0 >> 22) & 31;
    const uint32_t outPhys = G32(base, a + 28) & kPhysicalMask;
    const uint32_t cap = blocks * 256;
    if (!blocks || !PhysOk(outPhys, cap)) return false;
    uint8_t* ring = base + 0xA0000000u + outPhys;
    bool progress = false;
    for (int iter = 0; iter < 256; ++iter) {
        // 1) vaciar PCM pendiente en bloques de 256 B (128 muestras int16); nunca se llena el anillo del todo
        //    (write == read significaria "vacio") salvo que el anillo mida un solo bloque.
        constexpr size_t kBlkSamples = 128;
        while (st.pend.size() - st.pendPos >= kBlkSamples) {
            w0 = G32(base, a);
            uint32_t write = (w0 >> 27) & 31, read = G32(base, a + 36) & 31;
            uint32_t wOff = write * 256, rOff = read * 256;
            uint32_t used = wOff >= rOff ? wOff - rOff : cap - (rOff - wOff);
            uint32_t freeB = cap - used;
            uint32_t maxBlk = freeB >= 512 ? (freeB - 256) / 256 : ((cap == 256 && freeB == 256) ? 1 : 0);
            if (!maxBlk) { ++g_stats.ringBlocked; return progress; }
            uint32_t n = std::min<uint32_t>(maxBlk, static_cast<uint32_t>((st.pend.size() - st.pendPos) / kBlkSamples));
            uint32_t bytes = n * 256;
            int peak = 0;
            const int16_t* src = st.pend.data() + st.pendPos;
            uint32_t first = std::min(bytes, cap - wOff);
            auto put = [&](uint8_t* d, const int16_t* s2, uint32_t nb) {
                for (uint32_t k = 0; k < nb / 2; ++k) {
                    int16_t v = s2[k]; int av = v < 0 ? -v : v; if (av > peak) peak = av;
                    uint16_t u = _byteswap_ushort(static_cast<uint16_t>(v)); std::memcpy(d + k * 2, &u, 2);
                }
            };
            put(ring + wOff, src, first);
            if (first < bytes) put(ring, src + first / 2, bytes - first);
            Rmw(base, a, 0x1Fu << 27, ((write + n) % blocks) << 27);
            st.pendPos += static_cast<size_t>(n) * kBlkSamples;
            g_stats.blocksWritten += n; if (peak) g_stats.nonSilentBlocks += n; if (peak > g_stats.peak) g_stats.peak = peak;
            progress = true;
        }
        if (st.pendPos && st.pendPos == st.pend.size()) { st.pend.clear(); st.pendPos = 0; }
        else if (st.pendPos > 65536) { st.pend.erase(st.pend.begin(), st.pend.begin() + st.pendPos); st.pendPos = 0; }
        // 2) hace falta mas PCM: siguiente paquete del buffer de entrada actual
        w0 = G32(base, a); w1 = G32(base, a + 4);
        valid[0] = (w0 >> 20) & 1; valid[1] = (w0 >> 21) & 1;
        uint32_t cur = G32(base, a + 16) >> 31;
        if (!valid[cur]) { if (valid[cur ^ 1]) cur ^= 1; else break; }
        const uint32_t npk = cur ? (w1 & 0xFFF) : (w0 & 0xFFF);
        const uint32_t inPhys = G32(base, a + (cur ? 24 : 20)) & kPhysicalMask;
        uint32_t off = G32(base, a + 8) & 0x3FFFFFFu;
        if (st.expectOff != 0xFFFFFFFFu && off != st.expectOff && off != 32) {   // 32 = el guest rearma un buffer (continuidad)
            uint32_t p = off / (kXmaPacketBytes * 8);
            if (p >= npk) {   // desplazamiento al final (o mas alla) del buffer: el guest lo da por consumido; no se reenvia ningun paquete
                st.pktIdx = npk;
            } else {          // otro valor: reposicion real
                ++g_stats.repositions;
                st.pktIdx = p;
                st.dec->Reset(); st.pend.clear(); st.pendPos = 0;
                if (g_dbgLines++ < 60) ImportLog::Get().Printf("XMA: reposicion ctx=%u offset=%u (esperado %u) paquete=%u/%u", i, off, st.expectOff, st.pktIdx, npk);
            }
        }
        if (!npk || !PhysOk(inPhys, npk * kXmaPacketBytes)) { Rmw(base, a, 1u << (20 + cur), 0); Rmw(base, a + 16, 1u << 31, (cur ^ 1) << 31); progress = true; continue; }
        if (st.pktIdx >= npk) {   // buffer agotado
            Rmw(base, a, 1u << (20 + cur), 0);
            Rmw(base, a + 16, 1u << 31, (cur ^ 1u) << 31);
            Rmw(base, a + 8, 0x3FFFFFFu, 32);
            st.pktIdx = 0; st.expectOff = 32; st.consumed = true; ++g_stats.buffersConsumed; progress = true;
            continue;
        }
        const uint8_t* pk = base + 0xA0000000u + inPhys + st.pktIdx * kXmaPacketBytes;
        if (!g_capDir.empty()) {
            if (!st.cap) { char nm[512]; std::snprintf(nm, sizeof nm, "%s\\ctx%u_%u_ch%d_r%d.xma", g_capDir.c_str(), i, st.capSeq++, channels, rate); st.cap = std::fopen(nm, "wb"); }
            if (st.cap) std::fwrite(pk, 1, kXmaPacketBytes, st.cap);
        }
        int spf = 0;
        const size_t before = st.pend.size();
        int n = st.dec->DecodePacket(pk, st.pend, &spf);
        if (n < 0) { ++g_stats.decodeErrors; if (g_stats.decodeErrors <= 10) ImportLog::Get().Printf("XMA: error de decodificacion ctx=%u paquete=%u/%u err=%d", i, st.pktIdx, npk, st.dec->lastError()); }
        else if (n > 0) {
            g_stats.framesDecoded += n;
            g_stats.samplesDecoded += (st.pend.size() - before) / channels;
        }
        ++g_stats.packetsFed; ++g_stats.passes;
        if (g_dbgLines < 40) { ++g_dbgLines; ImportLog::Get().Printf("XMA DECODE ctx=%u buf=%u paq=%u/%u off=%u tramas=%d spf=%d pcm_pend=%zu out=[w=%u r=%u blk=%u]", i, cur, st.pktIdx, npk, off, n, spf, st.pend.size() - st.pendPos, (G32(base, a) >> 27) & 31, G32(base, a + 36) & 31, blocks); }
        ++st.pktIdx;
        uint32_t newOff = st.pktIdx < npk ? st.pktIdx * kXmaPacketBytes * 8 + 32 : npk * kXmaPacketBytes * 8;
        Rmw(base, a + 8, 0x3FFFFFFu, newOff);
        st.expectOff = newOff; st.consumed = true; progress = true;
    }
    return progress;
}

} // namespace

uint32_t XmaDecodeStep() {
    if (g_forceSilence || !FfAvailable()) return SimulateXmaStep();
    std::lock_guard<std::mutex> lock(g_mu);
    if (!g_contextArray || !GuestMemory::Base()) return 0;
    uint8_t* base = GuestMemory::Base();
    uint32_t n = 0;
    for (uint32_t i = 0; i < kXmaContextCount; ++i)
        if (g_allocated[i] && DecodeCtxLocked(i, base)) ++n;
    return n;
}

XmaStats XmaGetStats() { std::lock_guard<std::mutex> lock(g_mu); XmaStats s = g_stats; s.backend = FfAvailable(); return s; }
void XmaSetTrace(bool on) { g_trace = on; }
void XmaForceSilence(bool on) { g_forceSilence = on; }
void XmaSetCaptureDir(const std::string& dir) { std::lock_guard<std::mutex> lock(g_mu); g_capDir = dir; }

static DWORD WINAPI XmaWorkerMain(void*) {
    timeBeginPeriod(1);
    while (g_workerRunning.load()) {
        uint32_t n = XmaDecodeStep();
        Sleep(n ? 1 : 2);
    }
    return 0;
}

void StartXmaWorker() {
    if (g_workerRunning.exchange(true)) return;
    std::string why;
    bool ff = FfAvailable(&why);
    ImportLog::Get().Printf("XMA: motor de decodificacion: %s (%s)", ff ? "libavcodec xma2" : "SIN decodificador (salida silenciosa)", why.c_str());
    g_worker = CreateThread(nullptr, 0, XmaWorkerMain, nullptr, 0, nullptr);
    if (!g_worker) g_workerRunning = false;
}

void StopXmaWorker() {
    g_workerRunning = false;
    if (g_worker) {
        WaitForSingleObject(g_worker, 2000);
        CloseHandle(g_worker);
        g_worker = nullptr;
    }
}

} // namespace rt::audio
