// Audio XAudio minimo (M2). Referencias (copias en runtime\ref\): Xenia xboxkrnl_audio.cc, audio_system.cc/.h,
// xaudio2_audio_driver.cc/.h, conversion.h. Comportamiento de Xenia reproducido:
//  - 8 clientes como maximo; XAudioRegisterRenderDriverClient lee callback=[r3+0], arg=[r3+4], devuelve *r4 = 0x41550000|indice;
//  - cada cliente tiene un semaforo con 64 de cuenta maxima y 64 iniciales; el hilo de audio espera cualquier semaforo, decrementa
//    uno y llama callback(wrapped_arg) en contexto guest, donde wrapped_arg = puntero guest a un bloque de 4 bytes que contiene arg
//    (AudioSystem::RegisterClient: SystemHeapAlloc(4) + store_and_swap(arg));
//  - XAudioSubmitRenderDriverFrame encola una trama (6 canales secuenciales x 256 muestras float big-endian = 6144 bytes);
//    el semaforo se libera cuando el backend termina de reproducir la trama (IXAudio2VoiceCallback::OnBufferEnd).
// Sumidero (A1): audio_sink.* (WASAPI en modo compartido o reloj nulo de 48 kHz; cada trama dura 256/48000 s = 5,333 ms).
// El semaforo de un cliente se libera cuando el sumidero ha CONSUMIDO la trama (reloj del dispositivo), y el hilo de audio solo
// llama al callback guest mientras la cola del sumidero sea menor que kTargetQueued (contrapresion; evita overruns).
#include <windows.h>
#include <timeapi.h>

#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>
#include <cstdlib>

#include "perf.h"
#include "guest_abi.h"
#include "guest_heap.h"
#include "guest_thread.h"
#include "import_log.h"
#include "../audio/xma_context.h"
#include "../audio/audio_sink.h"
#include "../audio/xma_ffmpeg.h"

#pragma comment(lib, "winmm.lib")

namespace rt { extern bool g_noAudio; }
using namespace rt;

namespace {

constexpr int kMaxClients = 8;
constexpr int kMaxQueued = 64;
constexpr uint32_t kChannelSamples = 256, kChannels = 6;

struct Client { bool inUse = false; uint32_t cb = 0, arg = 0, wrapped = 0; int sem = 0; };
struct Pending { int client; uint64_t endSample; };   // endSample: muestras acumuladas entregadas al sumidero al terminar la trama
constexpr uint32_t kTargetQueued = 10 * 256;           // muestras estereo en cola antes de dejar de llamar al callback (53 ms)

std::mutex g_mu;
Client g_cl[kMaxClients];
std::deque<Pending> g_play;
uint64_t g_submittedSamples = 0;
HANDLE g_thread = nullptr;
std::atomic<bool> g_run{false};
std::atomic<uint64_t> g_submits{0}, g_callbacks{0}, g_registers{0}, g_unregisters{0}, g_nonSilent{0};
std::atomic<uint32_t> g_peakBits{0};
LONGLONG g_freq = 0;

// La direccion de la trama la elige el juego (heap 0x83xxxxxx, etc.): se comprueba que el rango host este confirmado y sea legible.
bool Readable(const void* p, size_t n) {
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(p, &mbi, sizeof mbi) || mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))) return false;
    return static_cast<const uint8_t*>(p) + n <= static_cast<const uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
}
std::atomic<uint64_t> g_badFrames{0};

LONGLONG Now() { LARGE_INTEGER c; QueryPerformanceCounter(&c); return c.QuadPart; }

DWORD WINAPI AudioMain(void*) {
    rt::perf::NameCurrentThread("audio-worker");
    timeBeginPeriod(1);
    GuestThread* gt = GuestThread::CreateAttached("Audio Worker");
    if (!gt) { ImportLog::Get().Printf("AUDIO: no se pudo crear el contexto guest"); return 1; }
    gt->Attach();
    LONGLONG lastStat = Now();
    uint64_t lastSub = 0, lastCb = 0, sec = 0;
    while (g_run) {
        LONGLONG now = Now();
        const uint64_t consumed = rt::audio::SinkConsumedSamples();
        {
            std::lock_guard<std::mutex> g(g_mu);
            while (!g_play.empty() && g_play.front().endSample <= consumed) {   // OnBufferEnd
                Client& c = g_cl[g_play.front().client];
                if (c.inUse && c.sem < kMaxQueued) ++c.sem;
                g_play.pop_front();
            }
        }
        bool pumped = false;
        const bool room = rt::audio::SinkQueuedSamples() < kTargetQueued;
        for (int i = 0; room && i < kMaxClients; ++i) {
            uint32_t cb = 0, wr = 0;
            {
                std::lock_guard<std::mutex> g(g_mu);
                Client& c = g_cl[i];
                if (c.inUse && c.sem > 0 && c.cb) { --c.sem; cb = c.cb; wr = c.wrapped; }
            }
            if (cb) {
                gt->SetCpu(5);
                gt->CallGuest(cb, wr, 0);
                ++g_callbacks;
                pumped = true;
            }
        }
        if (now - lastStat >= g_freq) {
            lastStat += g_freq; ++sec;
            uint64_t s = g_submits, cbn = g_callbacks;
            float pk; uint32_t pb = g_peakBits; std::memcpy(&pk, &pb, 4);
            double rmsDb; float lp; uint64_t ln; rt::audio::SinkTakeLevel(rmsDb, lp, ln);
            rt::audio::SinkStats ss = rt::audio::SinkGetStats();
            rt::audio::XmaStats xs = rt::audio::XmaGetStats();
            ImportLog::Get().Printf("AUDIOSTAT t=%llus submits/s=%llu callbacks/s=%llu total_submits=%llu total_callbacks=%llu tramas_no_silenciosas=%llu pico=%g | sumidero=%s entregadas=%llu cola=%u underruns=%llu descartadas=%llu recortes=%llu rms=%.1fdBFS pico_est=%.4f | xma paquetes=%llu bloques256B=%llu no_sil=%llu",
                                    (unsigned long long)sec, (unsigned long long)(s - lastSub), (unsigned long long)(cbn - lastCb), (unsigned long long)s, (unsigned long long)cbn, (unsigned long long)g_nonSilent.load(), pk,
                                    ss.deviceActive ? "wasapi" : "nulo", (unsigned long long)ss.framesSubmitted, ss.queuedSamples, (unsigned long long)ss.underrunEvents, (unsigned long long)ss.framesDropped,
                                    (unsigned long long)ss.clipped, rmsDb, lp, (unsigned long long)xs.packetsFed, (unsigned long long)xs.blocksWritten, (unsigned long long)xs.nonSilentBlocks);
            lastSub = s; lastCb = cbn;
        }
        if (!pumped) Sleep(1);
    }
    return 0;
}

void EnsureThread() {
    if (g_thread) return;
    LARGE_INTEGER f; QueryPerformanceFrequency(&f); g_freq = f.QuadPart;
    g_run = true;
    g_thread = CreateThread(nullptr, 0, AudioMain, nullptr, 0, nullptr);
    bool dev = rt::audio::SinkStart();
    ImportLog::Get().Printf("AUDIO: hilo de audio iniciado (sumidero=%s, 48 kHz, trama=%u muestras x %u canales)", dev ? "wasapi" : "nulo", kChannelSamples, kChannels);
}

} // namespace

namespace rt {
bool g_noAudio = false;   // --no-audio: comportamiento anterior (stubs sin efecto), para la medicion de referencia de M0
void AudioSummary() {
    float pk; uint32_t pb = g_peakBits; std::memcpy(&pk, &pb, 4);
    ImportLog::Get().Printf("AUDIO resumen: register=%llu unregister=%llu submit=%llu callbacks=%llu tramas_no_silenciosas=%llu pico=%g", (unsigned long long)g_registers.load(),
                            (unsigned long long)g_unregisters.load(), (unsigned long long)g_submits.load(), (unsigned long long)g_callbacks.load(), (unsigned long long)g_nonSilent.load(), pk);
}
void AudioStop() { g_run = false; rt::audio::SinkStop(); }
void AudioFinish() {   // cierre: resumen del sumidero y de XMA y finalizacion del WAV
    rt::audio::SinkSummary();
    rt::audio::XmaStats x = rt::audio::XmaGetStats();
    ImportLog::Get().Printf("XMA decodificacion: backend=%s (%s) paquetes=%llu muestras_decodificadas=%llu bloques_escritos=%llu bloques_no_silenciosos=%llu buffers_consumidos=%llu reposiciones=%llu bloqueos_salida=%llu errores=%llu pico=%d",
                            x.backend ? "libavcodec_xma2" : "sin_decodificador", rt::audio::FfVersionInfo().c_str(), (unsigned long long)x.packetsFed, (unsigned long long)x.samplesDecoded,
                            (unsigned long long)x.blocksWritten, (unsigned long long)x.nonSilentBlocks, (unsigned long long)x.buffersConsumed, (unsigned long long)x.repositions,
                            (unsigned long long)x.ringBlocked, (unsigned long long)x.decodeErrors, x.peak);
    g_run = false; rt::audio::SinkStop();
}
// Opciones de linea de ordenes de audio: --audio=auto|null|wasapi, --dump-audio <ruta.wav>, --xma-capture <dir>,
// --xma-file <paquetes.xma> [--xma-channels N] [--xma-rate HZ] (selftest). Devuelve true si consumio el argumento.
static std::string g_dumpPath, g_xmaFile; static int g_xmaCh = 0, g_xmaRate = 0; static rt::audio::SinkMode g_sinkMode = rt::audio::SinkMode::Auto;
bool AudioParseArg(int argc, char** argv, int& i) {
    const char* a = argv[i];
    if (!std::strncmp(a, "--audio=", 8)) {
        const char* v = a + 8;
        if (!std::strcmp(v, "null")) g_sinkMode = rt::audio::SinkMode::Null;
        else if (!std::strcmp(v, "wasapi")) g_sinkMode = rt::audio::SinkMode::Wasapi;
        else if (!std::strcmp(v, "auto")) g_sinkMode = rt::audio::SinkMode::Auto;
        else { std::printf("--audio: valor no valido: %s (null|wasapi|auto)\n", v); std::exit(2); }
        rt::audio::SinkConfigure(g_sinkMode, g_dumpPath); return true;
    }
    if (!std::strcmp(a, "--dump-audio") && i + 1 < argc) { g_dumpPath = argv[++i]; rt::audio::SinkConfigure(g_sinkMode, g_dumpPath); return true; }
    if (!std::strcmp(a, "--xma-capture") && i + 1 < argc) { rt::audio::XmaSetCaptureDir(argv[++i]); return true; }
    if (!std::strcmp(a, "--xma=silence")) { rt::audio::XmaForceSilence(true); return true; }
    if (!std::strcmp(a, "--xma-trace")) { rt::audio::XmaSetTrace(true); return true; }
    if (!std::strcmp(a, "--xma-file") && i + 1 < argc) { g_xmaFile = argv[++i]; return true; }
    if (!std::strcmp(a, "--xma-channels") && i + 1 < argc) { g_xmaCh = std::atoi(argv[++i]); return true; }
    if (!std::strcmp(a, "--xma-rate") && i + 1 < argc) { g_xmaRate = std::atoi(argv[++i]); return true; }
    return false;
}
bool AudioXmaTestFile(std::string& file, int& ch, int& rate) {
    file = g_xmaFile; ch = g_xmaCh; rate = g_xmaRate;
    if (file.empty()) return false;
    if (!ch) { size_t k = file.find("_ch"); ch = k != std::string::npos ? std::atoi(file.c_str() + k + 3) : 2; if (ch != 1 && ch != 2) ch = 2; }
    if (!rate) { size_t k = file.find("_r"); rate = k != std::string::npos ? std::atoi(file.c_str() + k + 2) : 48000; if (rate <= 0) rate = 48000; }
    return true;
}
} // namespace rt

GUEST_IMPORT_IMPL(XAudioGetSpeakerConfig) {
    ImportScope sc("XAudioGetSpeakerConfig", ctx, base);
    if (g_noAudio) { SetRet(ctx, 0); return; }
    if (ctx.r3.u32) P32(base, ctx.r3.u32, 0x00010001);   // Xenia: *config_ptr = 0x00010001
    SetRet(ctx, 0);
}

GUEST_IMPORT_IMPL(XAudioGetVoiceCategoryVolume) {
    ImportScope sc("XAudioGetVoiceCategoryVolume", ctx, base);
    if (g_noAudio) { SetRet(ctx, 0); return; }
    if (ctx.r4.u32) P32(base, ctx.r4.u32, 0x3F800000);   // Xenia: (unk, float* out) -> *out = 1.0f
    SetRet(ctx, 0);
}

GUEST_IMPORT_IMPL(XAudioRegisterRenderDriverClient) {
    ImportScope sc("XAudioRegisterRenderDriverClient", ctx, base);
    if (g_noAudio) { SetRet(ctx, 0); return; }
    uint32_t cbp = ctx.r3.u32, drv = ctx.r4.u32;
    uint32_t cb = G32(base, cbp), arg = G32(base, cbp + 4);
    int idx = -1;
    uint32_t wr = SystemHeap().Alloc(4);
    if (wr) P32(base, wr, arg);
    EnsureThread();
    {
        std::lock_guard<std::mutex> g(g_mu);
        for (int i = 0; i < kMaxClients; ++i) if (!g_cl[i].inUse) { idx = i; break; }
        if (idx >= 0) { g_cl[idx].inUse = true; g_cl[idx].cb = cb; g_cl[idx].arg = arg; g_cl[idx].wrapped = wr; g_cl[idx].sem = kMaxQueued; }
    }
    if (idx < 0) { SetRet(ctx, 0xC0000001); return; }   // Xenia hace assert; codigo propio
    ++g_registers;
    P32(base, drv, 0x41550000u | (uint32_t)idx);
    ImportLog::Get().Printf("AUDIO: RegisterRenderDriverClient cliente=%d callback=%08X arg=%08X wrapped=%08X", idx, cb, arg, wr);
    SetRet(ctx, 0);
}

GUEST_IMPORT_IMPL(XAudioUnregisterRenderDriverClient) {
    ImportScope sc("XAudioUnregisterRenderDriverClient", ctx, base);
    uint32_t drv = ctx.r3.u32;
    int idx = (int)(drv & 0xFFFF);
    if ((drv & 0xFFFF0000) == 0x41550000 && idx < kMaxClients) {
        uint32_t wr = 0;
        {
            std::lock_guard<std::mutex> g(g_mu);
            wr = g_cl[idx].wrapped;
            g_cl[idx] = Client();
            for (auto it = g_play.begin(); it != g_play.end();) it = (it->client == idx) ? g_play.erase(it) : it + 1;
        }
        if (wr) SystemHeap().Free(wr);
        ++g_unregisters;
    }
    SetRet(ctx, 0);
}

GUEST_IMPORT_IMPL(XAudioSubmitRenderDriverFrame) {
    ImportScope sc("XAudioSubmitRenderDriverFrame", ctx, base);
    uint32_t drv = ctx.r3.u32, samples = ctx.r4.u32;
    int idx = (int)(drv & 0xFFFF);
    SetRet(ctx, 0);
    if ((drv & 0xFFFF0000) != 0x41550000 || idx >= kMaxClients) return;
    // Estadistica: pico absoluto de la trama (muestras float big-endian, 6 x 256).
    float peak = 0.f;
    const bool readable = samples && Readable(base + samples, kChannels * kChannelSamples * 4);
    if (!readable && g_badFrames++ < 5) ImportLog::Get().Printf("AUDIO: trama con direccion no legible %08X", samples);
    if (readable) {
        for (uint32_t i = 0; i < kChannels * kChannelSamples; ++i) {
            uint32_t b = G32(base, samples + i * 4); float f; std::memcpy(&f, &b, 4);
            if (f == f) peak = std::fmax(peak, std::fabs(f));
        }
    }
    if (peak > 0.f) {
        ++g_nonSilent;
        uint32_t pb = g_peakBits; float cur; std::memcpy(&cur, &pb, 4);
        if (peak > cur) { uint32_t nb; std::memcpy(&nb, &peak, 4); g_peakBits = nb; }
    }
    if (readable) rt::audio::SinkSubmitFrame(base + samples);
    std::lock_guard<std::mutex> g(g_mu);
    if (!g_cl[idx].inUse) return;
    g_submittedSamples += kChannelSamples;
    g_play.push_back({idx, g_submittedSamples});
    ++g_submits;
}

GUEST_IMPORT_IMPL(XMACreateContext) {
    ImportScope sc("XMACreateContext", ctx, base);
    if (!ctx.r3.u32) { SetRet(ctx, 0xC000000D); return; } // STATUS_INVALID_PARAMETER
    uint32_t context = rt::audio::CreateXmaContext();
    P32(base, ctx.r3.u32, context);
    if (!context) { SetRet(ctx, 0xC0000017); return; } // STATUS_NO_MEMORY
    rt::audio::StartXmaWorker();
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(XMAReleaseContext) {
    ImportScope sc("XMAReleaseContext", ctx, base);
    if (!rt::audio::ReleaseXmaContext(ctx.r3.u32))
        ImportLog::Get().Printf("XMA: ReleaseContext ignorado para contexto invalido %08X", ctx.r3.u32);
    SetRet(ctx, ST_SUCCESS);
}
