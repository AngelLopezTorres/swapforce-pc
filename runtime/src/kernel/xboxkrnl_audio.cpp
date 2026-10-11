// Audio XAudio minimo (M2). Referencias (copias en runtime\ref\): Xenia xboxkrnl_audio.cc, audio_system.cc/.h,
// xaudio2_audio_driver.cc/.h, conversion.h. Comportamiento de Xenia reproducido:
//  - 8 clientes como maximo; XAudioRegisterRenderDriverClient lee callback=[r3+0], arg=[r3+4], devuelve *r4 = 0x41550000|indice;
//  - cada cliente tiene un semaforo con 64 de cuenta maxima y 64 iniciales; el hilo de audio espera cualquier semaforo, decrementa
//    uno y llama callback(wrapped_arg) en contexto guest, donde wrapped_arg = puntero guest a un bloque de 4 bytes que contiene arg
//    (AudioSystem::RegisterClient: SystemHeapAlloc(4) + store_and_swap(arg));
//  - XAudioSubmitRenderDriverFrame encola una trama (6 canales secuenciales x 256 muestras float big-endian = 6144 bytes);
//    el semaforo se libera cuando el backend termina de reproducir la trama (IXAudio2VoiceCallback::OnBufferEnd).
// Sumidero: nulo con reloj de 48 kHz (cada trama dura 256/48000 s = 5,333 ms; las tramas se "reproducen" en serie).
// No se emite sonido (WASAPI/XAudio2 no implementado: pendiente).
#include <windows.h>
#include <timeapi.h>

#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <mutex>

#include "guest_abi.h"
#include "guest_heap.h"
#include "guest_thread.h"
#include "import_log.h"

#pragma comment(lib, "winmm.lib")

namespace rt { extern bool g_noAudio; }
using namespace rt;

namespace {

constexpr int kMaxClients = 8;
constexpr int kMaxQueued = 64;
constexpr uint32_t kChannelSamples = 256, kChannels = 6;

struct Client { bool inUse = false; uint32_t cb = 0, arg = 0, wrapped = 0; int sem = 0; };
struct Pending { int client; LONGLONG done; };

std::mutex g_mu;
Client g_cl[kMaxClients];
std::deque<Pending> g_play;
LONGLONG g_playEnd = 0;
HANDLE g_thread = nullptr;
std::atomic<bool> g_run{false};
std::atomic<uint64_t> g_submits{0}, g_callbacks{0}, g_registers{0}, g_unregisters{0}, g_nonSilent{0};
std::atomic<uint32_t> g_peakBits{0};
LONGLONG g_freq = 0;

LONGLONG Now() { LARGE_INTEGER c; QueryPerformanceCounter(&c); return c.QuadPart; }

DWORD WINAPI AudioMain(void*) {
    timeBeginPeriod(1);
    GuestThread* gt = GuestThread::CreateAttached("Audio Worker");
    if (!gt) { ImportLog::Get().Printf("AUDIO: no se pudo crear el contexto guest"); return 1; }
    gt->Attach();
    LONGLONG lastStat = Now();
    uint64_t lastSub = 0, lastCb = 0, sec = 0;
    while (g_run) {
        LONGLONG now = Now();
        {
            std::lock_guard<std::mutex> g(g_mu);
            while (!g_play.empty() && g_play.front().done <= now) {   // OnBufferEnd
                Client& c = g_cl[g_play.front().client];
                if (c.inUse && c.sem < kMaxQueued) ++c.sem;
                g_play.pop_front();
            }
        }
        bool pumped = false;
        for (int i = 0; i < kMaxClients; ++i) {
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
            ImportLog::Get().Printf("AUDIOSTAT t=%llus submits/s=%llu callbacks/s=%llu total_submits=%llu total_callbacks=%llu tramas_no_silenciosas=%llu pico=%g", (unsigned long long)sec,
                                    (unsigned long long)(s - lastSub), (unsigned long long)(cbn - lastCb), (unsigned long long)s, (unsigned long long)cbn, (unsigned long long)g_nonSilent.load(), pk);
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
    ImportLog::Get().Printf("AUDIO: hilo de audio iniciado (sumidero nulo, 48 kHz, trama=%u muestras x %u canales)", kChannelSamples, kChannels);
}

} // namespace

namespace rt {
bool g_noAudio = false;   // --no-audio: comportamiento anterior (stubs sin efecto), para la medicion de referencia de M0
void AudioSummary() {
    float pk; uint32_t pb = g_peakBits; std::memcpy(&pk, &pb, 4);
    ImportLog::Get().Printf("AUDIO resumen: register=%llu unregister=%llu submit=%llu callbacks=%llu tramas_no_silenciosas=%llu pico=%g", (unsigned long long)g_registers.load(),
                            (unsigned long long)g_unregisters.load(), (unsigned long long)g_submits.load(), (unsigned long long)g_callbacks.load(), (unsigned long long)g_nonSilent.load(), pk);
}
void AudioStop() { g_run = false; }
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
    if (samples && samples < 0x7FFFE000u) {
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
    LONGLONG period = g_freq * kChannelSamples / 48000;
    std::lock_guard<std::mutex> g(g_mu);
    if (!g_cl[idx].inUse) return;
    LONGLONG now = Now();
    LONGLONG start = g_playEnd > now ? g_playEnd : now;
    g_playEnd = start + period;
    g_play.push_back({idx, g_playEnd});
    ++g_submits;
}
