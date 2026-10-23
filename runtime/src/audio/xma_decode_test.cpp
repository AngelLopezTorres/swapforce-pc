// Prueba de decodificacion XMA2 (--selftest-xma con --xma-file). Entrada: fichero de paquetes de 2048 B (un flujo XMA2 real, p. ej.
// capturado con --xma-capture; no se incluye en el repositorio). Comprueba:
//  1) decodificacion directa con libavcodec: sin errores, 512 muestras por trama, PCM no silencioso, resultado determinista;
//  2) a traves de un contexto guest simulado (buffers de entrada 0/1 de 4 paquetes, anillo de salida de 16 y de 31 bloques con
//     consumo por parte de un "guest"): la salida del anillo coincide BIT A BIT con (1) y los contadores/flags quedan coherentes.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "guest_abi.h"
#include "guest_heap.h"
#include "xma_context.h"
#include "xma_ffmpeg.h"

namespace rt::audio {
namespace {

bool Rd(const std::string& path, std::vector<uint8_t>& out) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    std::fseek(f, 0, SEEK_END); long n = std::ftell(f); std::fseek(f, 0, SEEK_SET);
    out.resize(n > 0 ? (size_t)n : 0);
    size_t r = out.empty() ? 0 : std::fread(out.data(), 1, out.size(), f);
    std::fclose(f);
    return r == out.size();
}

uint64_t Fnv(const std::vector<int16_t>& v) {
    uint64_t h = 1469598103934665603ull;
    for (int16_t s : v) { h ^= (uint16_t)s; h *= 1099511628211ull; }
    return h;
}

void Rmw(uint8_t* base, uint32_t a, uint32_t clr, uint32_t set) { P32(base, a, (G32(base, a) & ~clr) | set); }

// Simula al guest con un anillo de `blocks` bloques de 256 B. Devuelve el PCM leido (int16 host).
bool ViaContext(uint8_t* base, const std::vector<uint8_t>& data, int ch, int rate, uint32_t blocks, uint32_t pktPerBuf, std::vector<int16_t>& got, std::string& why) {
    uint32_t ctxAddr = CreateXmaContext();
    if (!ctxAddr) { why = "CreateXmaContext fallo"; return false; }
    const uint32_t npk = (uint32_t)(data.size() / kXmaPacketBytes);
    uint32_t bufAddr[2] = {rt::PhysicalHeap().Alloc(pktPerBuf * kXmaPacketBytes, 256), rt::PhysicalHeap().Alloc(pktPerBuf * kXmaPacketBytes, 256)};
    uint32_t outAddr = rt::PhysicalHeap().Alloc(blocks * 256, 256);
    if (!bufAddr[0] || !bufAddr[1] || !outAddr) { why = "sin memoria fisica"; ReleaseXmaContext(ctxAddr); return false; }
    const uint32_t mask = 0x1FFFFFFF;
    static const int rates[4] = {24000, 32000, 44100, 48000};
    uint32_t rateId = 3; for (uint32_t k = 0; k < 4; ++k) if (rates[k] == rate) rateId = k;
    // contexto inicial: ambos buffers vacios; salida valida
    P32(base, ctxAddr + 0, blocks << 22);
    P32(base, ctxAddr + 4, 0x80000000u | (ch == 2 ? 1u << 29 : 0) | (rateId << 27));
    P32(base, ctxAddr + 8, 32);
    P32(base, ctxAddr + 16, 0);
    P32(base, ctxAddr + 20, bufAddr[0] & mask);
    P32(base, ctxAddr + 24, bufAddr[1] & mask);
    P32(base, ctxAddr + 28, outAddr & mask);
    P32(base, ctxAddr + 36, 0);
    uint32_t nextFill = 0, filled = 0, readBlk = 0;
    const uint32_t cap = blocks * 256;
    const uint8_t* ring = base + 0xA0000000u + (outAddr & mask);
    int idle = 0;
    for (int step = 0; step < 200000 && idle < 50; ++step) {
        // guest: rellenar el siguiente buffer (en orden) si esta libre
        uint32_t w0 = G32(base, ctxAddr);
        if (filled < npk && !((w0 >> (20 + nextFill)) & 1)) {
            uint32_t n = std::min(pktPerBuf, npk - filled);
            std::memcpy(base + 0xA0000000u + (bufAddr[nextFill] & mask), data.data() + (size_t)filled * kXmaPacketBytes, (size_t)n * kXmaPacketBytes);
            if (nextFill == 0) Rmw(base, ctxAddr, 0xFFFu | (1u << 20), n | (1u << 20));
            else { Rmw(base, ctxAddr, 1u << 21, 1u << 21); Rmw(base, ctxAddr + 4, 0xFFFu, n); }
            filled += n; nextFill ^= 1;
        }
        // guest: mantener output_buffer_valid mientras haya entrada pendiente
        if (filled < npk || ((G32(base, ctxAddr) >> 20) & 3)) Rmw(base, ctxAddr + 4, 0, 0x80000000u);
        uint32_t did = XmaDecodeStep();
        // guest: consumir salida
        uint32_t wr = ((G32(base, ctxAddr) >> 27) & 31) * 256, rd = readBlk * 256;
        uint32_t avail = wr >= rd ? wr - rd : cap - (rd - wr);
        if (avail) {
            size_t base0 = got.size(); got.resize(base0 + avail / 2);
            for (uint32_t k = 0; k < avail; k += 2) {
                uint16_t u; std::memcpy(&u, ring + (rd + k) % cap, 2); got[base0 + k / 2] = (int16_t)_byteswap_ushort(u);
            }
            readBlk = wr / 256;
            Rmw(base, ctxAddr + 36, 0x1F, readBlk);
            did = 1;
        }
        idle = did ? 0 : idle + 1;
    }
    uint32_t w0 = G32(base, ctxAddr), w2 = G32(base, ctxAddr + 8) & 0x3FFFFFF;
    bool ok = filled == npk && ((w0 >> 20) & 3) == 0 && w2 == 32;
    if (!ok) { char b[200]; std::snprintf(b, sizeof b, "estado final incoherente: rellenados=%u/%u valid=%u offset=%u", filled, npk, (w0 >> 20) & 3, w2); why = b; }
    ReleaseXmaContext(ctxAddr);
    rt::PhysicalHeap().Free(bufAddr[0]); rt::PhysicalHeap().Free(bufAddr[1]); rt::PhysicalHeap().Free(outAddr);
    return ok;
}

} // namespace

bool XmaDecodeSelfTest(uint8_t* base, const std::string& file, int ch, int rate) {
    std::string why;
    if (!FfAvailable(&why)) { std::printf("xma-decode: libavcodec NO disponible: %s -> FALLO\n", why.c_str()); return false; }
    std::printf("xma-decode: %s\n", why.c_str());
    std::vector<uint8_t> data;
    if (!Rd(file, data) || data.size() < kXmaPacketBytes) { std::printf("xma-decode: no se pudo leer %s -> FALLO\n", file.c_str()); return false; }
    uint32_t npk = (uint32_t)(data.size() / kXmaPacketBytes);
    std::printf("xma-decode: fichero=%s paquetes=%u canales=%d frecuencia=%d\n", file.c_str(), npk, ch, rate);
    auto direct = [&](std::vector<int16_t>& pcm, int& frames, int& spf, int& errs) {
        FfXmaDecoder d; frames = 0; spf = 0; errs = 0;
        if (!d.Open(ch, rate)) { errs = -1; return; }
        for (uint32_t p = 0; p < npk; ++p) {
            int n = d.DecodePacket(data.data() + (size_t)p * kXmaPacketBytes, pcm, &spf);
            if (n < 0) ++errs; else frames += n;
        }
    };
    std::vector<int16_t> ref, ref2; int fr = 0, spf = 0, er = 0, fr2 = 0, spf2 = 0, er2 = 0;
    direct(ref, fr, spf, er); direct(ref2, fr2, spf2, er2);
    if (const char* dumpPath = std::getenv("SWAPFORCE_XMA_DUMP")) {   // volcado opcional del PCM directo (int16 LE) para comparar con herramientas externas
        if (FILE* df = std::fopen(dumpPath, "wb")) { std::fwrite(ref.data(), sizeof(int16_t), ref.size(), df); std::fclose(df); }
    }
    double ss = 0; int pk = 0; for (int16_t s : ref) { ss += (double)s * s; pk = std::max(pk, std::abs((int)s)); }
    double rmsDb = ref.empty() ? -200 : 20 * std::log10(std::sqrt(ss / ref.size()) / 32768.0 + 1e-12);
    bool direct_ok = er == 0 && fr > 0 && spf > 0 && (spf % 64) == 0 && !ref.empty() && (ref.size() % (size_t)ch) == 0 && pk > 0 && rmsDb > -80 && ref == ref2;
    std::printf("xma-decode directo: AVFrame=%d muestras_por_AVFrame(ultimo)=%d errores=%d pcm_muestras=%zu pico=%d rms=%.1f dBFS fnv=%016llx determinista=%s -> %s\n",
                fr, spf, er, ref.size(), pk, rmsDb, (unsigned long long)Fnv(ref), ref == ref2 ? "si" : "NO", direct_ok ? "OK" : "FALLO");
    bool all = direct_ok;
    StopXmaWorker();
    const uint32_t cfg[3][2] = {{16, 4}, {31, 4}, {16, 1}};   // bloques de salida, paquetes por buffer de entrada
    for (auto& c : cfg) {
        std::vector<int16_t> got; std::string w;
        bool ok = ViaContext(base, data, ch, rate, c[0], c[1], got, w);
        // El anillo se escribe en bloques de 256 B (128 muestras int16): el resto parcial final de la referencia no se escribe.
        bool same = got.size() <= ref.size() && ref.size() - got.size() < 128 && std::equal(got.begin(), got.end(), ref.begin());
        std::printf("xma-decode contexto: salida=%u bloques, %u paquetes/buffer: pcm_muestras=%zu (ref %zu) fnv=%016llx prefijo_identico_a_directo=%s %s -> %s\n",
                    c[0], c[1], got.size(), ref.size(), (unsigned long long)Fnv(got), same ? "si" : "NO", w.c_str(), (ok && same) ? "OK" : "FALLO");
        all = all && ok && same;
    }
    XmaStats s = XmaGetStats();
    std::printf("xma-decode contadores: paquetes=%llu muestras_decodificadas=%llu bloques256_escritos=%llu bloques_no_silenciosos=%llu buffers=%llu bloqueos_salida=%llu errores=%llu pico=%d\n",
                (unsigned long long)s.packetsFed, (unsigned long long)s.samplesDecoded, (unsigned long long)s.blocksWritten, (unsigned long long)s.nonSilentBlocks,
                (unsigned long long)s.buffersConsumed, (unsigned long long)s.ringBlocked, (unsigned long long)s.decodeErrors, s.peak);
    std::printf("xma-decode %s\n", all ? "OK" : "FALLO");
    return all;
}

} // namespace rt::audio
