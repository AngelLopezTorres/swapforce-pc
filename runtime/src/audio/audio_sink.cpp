// Sumidero de audio: WASAPI (modo compartido, por eventos) o nulo con reloj de 48 kHz; volcado WAV opcional. Ver audio_sink.h.
// Referencias: Microsoft Learn, "Rendering a Stream" (IAudioClient::Initialize con AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
// IAudioRenderClient::GetBuffer/ReleaseBuffer) y AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | SRC_DEFAULT_QUALITY (el motor de audio
// de Windows convierte el formato pedido -48 kHz, 2 canales, float- al formato de mezcla del dispositivo).
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <timeapi.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>

#include "audio_sink.h"
#include "import_log.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "winmm.lib")

namespace rt::audio {
namespace {

constexpr uint32_t kRingFrames = 96;                              // 96 x 5,333 ms = 512 ms como maximo
constexpr uint32_t kRingSamples = kRingFrames * kSinkFrameSamples;   // muestras estereo
constexpr REFERENCE_TIME kBufferDuration100ns = 400000;           // 40 ms de bufer WASAPI (solicitado)

std::mutex g_mu;
std::vector<float> g_ring(kRingSamples * 2);
uint32_t g_head = 0, g_count = 0;   // en muestras estereo
SinkStats g_st;
bool g_started = false;             // ya llego al menos una trama guest
SinkMode g_mode = SinkMode::Auto;
std::string g_dumpPath;
std::atomic<bool> g_run{false};
HANDLE g_thread = nullptr;
bool g_starting = false, g_initDone = false;

// Ventana de nivel
double g_lvlSumSq = 0; uint64_t g_lvlN = 0; float g_lvlPeak = 0;

// WAV
FILE* g_wav = nullptr;
uint64_t g_wavSamples = 0, g_wavLastPatch = 0;

void WavHeader(FILE* f, uint64_t samples) {
    uint32_t dataBytes = (uint32_t)std::min<uint64_t>(samples * 4, 0xFFFFFFFFull - 36);
    uint8_t h[44];
    auto w32 = [&](int o, uint32_t v) { std::memcpy(h + o, &v, 4); };
    auto w16 = [&](int o, uint16_t v) { std::memcpy(h + o, &v, 2); };
    std::memcpy(h, "RIFF", 4); w32(4, 36 + dataBytes); std::memcpy(h + 8, "WAVEfmt ", 8);
    w32(16, 16); w16(20, 1); w16(22, 2); w32(24, kSinkRate); w32(28, kSinkRate * 4); w16(32, 4); w16(34, 16);
    std::memcpy(h + 36, "data", 4); w32(40, dataBytes);
    long pos = std::ftell(f);
    std::fseek(f, 0, SEEK_SET); std::fwrite(h, 1, 44, f); std::fseek(f, pos, SEEK_SET);
}

int16_t ToI16(float v) {
    float s = v * 32767.0f;
    if (s > 32767.0f) s = 32767.0f; else if (s < -32768.0f) s = -32768.0f;
    return (int16_t)std::lrintf(s);
}

void ConsumeLocked(uint32_t n, float* out /*puede ser null*/) {
    uint32_t first = std::min(n, kRingSamples - g_head);
    if (out) {
        std::memcpy(out, &g_ring[g_head * 2], first * 2 * sizeof(float));
        if (n > first) std::memcpy(out + first * 2, &g_ring[0], (n - first) * 2 * sizeof(float));
    }
    g_head = (g_head + n) % kRingSamples; g_count -= n; g_st.samplesConsumed += n;
}

// ---- reloj nulo ------------------------------------------------------------------------------------------------------
void NullClockLoop(LARGE_INTEGER freq) {
    LARGE_INTEGER last; QueryPerformanceCounter(&last);
    double carry = 0;
    while (g_run) {
        Sleep(1);
        LARGE_INTEGER now; QueryPerformanceCounter(&now);
        double want = (double)(now.QuadPart - last.QuadPart) * kSinkRate / (double)freq.QuadPart + carry;
        last = now;
        std::lock_guard<std::mutex> g(g_mu);
        uint32_t n = (uint32_t)want;
        if (g_count == 0) { carry = 0; continue; }   // dispositivo ocioso: no acumula credito
        uint32_t take = std::min(n, g_count);
        if (take) ConsumeLocked(take, nullptr);
        carry = (take < n) ? 0 : want - n;
    }
}

DWORD WINAPI NullMain(void*) {
    timeBeginPeriod(1);
    LARGE_INTEGER f; QueryPerformanceFrequency(&f);
    NullClockLoop(f);
    return 0;
}

// ---- WASAPI ------------------------------------------------------------------------------------------------------------
template <class T> void Rel(T*& p) { if (p) { p->Release(); p = nullptr; } }
const GUID kSubFloat = {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

struct InitResult { std::atomic<int> state{0}; };   // 0 pendiente, 1 ok, 2 fallo
InitResult g_init;
std::string g_devName;

DWORD WINAPI WasapiMain(void*) {
    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* en = nullptr; IMMDevice* dev = nullptr; IAudioClient* cl = nullptr; IAudioRenderClient* rc = nullptr;
    HANDLE ev = nullptr; UINT32 bufFrames = 0; HRESULT hr = S_OK; const char* stage = "";
    auto fail = [&](const char* s, HRESULT h) {
        rt::ImportLog::Get().Printf("AUDIO: WASAPI no disponible (%s hr=0x%08X): se usa el sumidero nulo", s, (unsigned)h);
        g_st.lastHr = (uint32_t)h; g_init.state = 2;
    };
    do {
        stage = "CoCreateInstance(MMDeviceEnumerator)";
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&en);
        if (FAILED(hr)) break;
        stage = "GetDefaultAudioEndpoint";
        hr = en->GetDefaultAudioEndpoint(eRender, eConsole, &dev);
        if (FAILED(hr)) break;
        {   // nombre amigable (informativo)
            IPropertyStore* ps = nullptr;
            if (SUCCEEDED(dev->OpenPropertyStore(STGM_READ, &ps)) && ps) {
                PROPVARIANT pv; PropVariantInit(&pv);
                if (SUCCEEDED(ps->GetValue(PKEY_Device_FriendlyName, &pv)) && pv.vt == VT_LPWSTR && pv.pwszVal) {
                    char nm[256] = {}; WideCharToMultiByte(CP_UTF8, 0, pv.pwszVal, -1, nm, sizeof nm - 1, nullptr, nullptr); g_devName = nm;
                }
                PropVariantClear(&pv); ps->Release();
            }
        }
        stage = "Activate(IAudioClient)";
        hr = dev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&cl);
        if (FAILED(hr)) break;
        WAVEFORMATEXTENSIBLE wf = {};
        wf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE; wf.Format.nChannels = 2; wf.Format.nSamplesPerSec = kSinkRate;
        wf.Format.wBitsPerSample = 32; wf.Format.nBlockAlign = 8; wf.Format.nAvgBytesPerSec = kSinkRate * 8;
        wf.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        wf.Samples.wValidBitsPerSample = 32; wf.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT; wf.SubFormat = kSubFloat;
        stage = "Initialize";
        hr = cl->Initialize(AUDCLNT_SHAREMODE_SHARED,
                            AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
                            kBufferDuration100ns, 0, &wf.Format, nullptr);
        if (FAILED(hr)) break;
        ev = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        stage = "SetEventHandle"; hr = cl->SetEventHandle(ev); if (FAILED(hr)) break;
        stage = "GetBufferSize"; hr = cl->GetBufferSize(&bufFrames); if (FAILED(hr)) break;
        stage = "GetService(IAudioRenderClient)"; hr = cl->GetService(__uuidof(IAudioRenderClient), (void**)&rc); if (FAILED(hr)) break;
        BYTE* p = nullptr;
        stage = "GetBuffer(prefill)"; hr = rc->GetBuffer(bufFrames, &p); if (FAILED(hr)) break;
        hr = rc->ReleaseBuffer(bufFrames, AUDCLNT_BUFFERFLAGS_SILENT); if (FAILED(hr)) { stage = "ReleaseBuffer(prefill)"; break; }
        stage = "Start"; hr = cl->Start(); if (FAILED(hr)) break;
        stage = nullptr;
    } while (false);
    if (stage) {
        fail(stage, hr);
        Rel(rc); Rel(cl); Rel(dev); Rel(en); if (ev) CloseHandle(ev);
        if (SUCCEEDED(hrCo)) CoUninitialize();
        return 1;
    }
    {
        std::lock_guard<std::mutex> g(g_mu); g_st.deviceActive = true;
    }
    rt::ImportLog::Get().Printf("AUDIO: WASAPI abierto: dispositivo=\"%s\" modo=compartido formato=48000 Hz 2 canales float (AUTOCONVERTPCM) bufer=%u muestras (%.1f ms)",
                                g_devName.c_str(), (unsigned)bufFrames, bufFrames * 1000.0 / kSinkRate);
    g_init.state = 1;
    std::vector<float> tmp;
    bool lost = false;
    while (g_run) {
        DWORD w = WaitForSingleObject(ev, 200);
        if (!g_run) break;
        UINT32 pad = 0;
        hr = cl->GetCurrentPadding(&pad);
        if (FAILED(hr)) { ++g_st.wasapiErrors; g_st.lastHr = (uint32_t)hr; if (hr == AUDCLNT_E_DEVICE_INVALIDATED) { lost = true; break; } continue; }
        UINT32 avail = bufFrames - pad;
        if (!avail) continue;
        BYTE* p = nullptr;
        hr = rc->GetBuffer(avail, &p);
        if (FAILED(hr)) { ++g_st.wasapiErrors; g_st.lastHr = (uint32_t)hr; if (hr == AUDCLNT_E_DEVICE_INVALIDATED) { lost = true; break; } continue; }
        uint32_t got = 0;
        {
            std::lock_guard<std::mutex> g(g_mu);
            got = std::min<uint32_t>(avail, g_count);
            if (got) ConsumeLocked(got, reinterpret_cast<float*>(p));
            if (got < avail) {
                std::memset(p + (size_t)got * 8, 0, (size_t)(avail - got) * 8);
                if (g_started) { ++g_st.underrunEvents; g_st.underrunSamples += avail - got; }
            }
        }
        hr = rc->ReleaseBuffer(avail, 0);
        if (FAILED(hr)) { ++g_st.wasapiErrors; g_st.lastHr = (uint32_t)hr; if (hr == AUDCLNT_E_DEVICE_INVALIDATED) { lost = true; break; } continue; }
        { std::lock_guard<std::mutex> g(g_mu); ++g_st.wasapiWrites; g_st.wasapiFramesWritten += avail; }
        (void)w;
    }
    cl->Stop();
    Rel(rc); Rel(cl); Rel(dev); Rel(en); CloseHandle(ev);
    if (SUCCEEDED(hrCo)) CoUninitialize();
    if (lost && g_run) {
        rt::ImportLog::Get().Printf("AUDIO: dispositivo WASAPI invalidado (hr=0x%08X): reloj nulo desde ahora", (unsigned)g_st.lastHr);
        { std::lock_guard<std::mutex> g(g_mu); g_st.deviceActive = false; }
        LARGE_INTEGER f; QueryPerformanceFrequency(&f);
        timeBeginPeriod(1);
        NullClockLoop(f);
    }
    return 0;
}

// Diagnostico de caidas del proceso (se encadena con el filtro de main): modulo y desplazamiento del RIP y de las direcciones
// de retorno plausibles de la pila, en audio_crash.txt del directorio actual.
LPTOP_LEVEL_EXCEPTION_FILTER g_prevFilter = nullptr;
void DescribeAddr(FILE* f, const char* tag, uint64_t a) {
    HMODULE m = nullptr; char nm[MAX_PATH] = "?";
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(a), &m) && m) GetModuleFileNameA(m, nm, MAX_PATH);
    std::fprintf(f, "%s %016llX modulo=%s +0x%llX\n", tag, (unsigned long long)a, nm, (unsigned long long)(a - reinterpret_cast<uint64_t>(m)));
}
LONG WINAPI AudioCrashFilter(EXCEPTION_POINTERS* ep) {
    if (FILE* f = std::fopen("audio_crash.txt", "w")) {
        std::fprintf(f, "codigo=%08lX hilo=%lu\n", ep->ExceptionRecord->ExceptionCode, GetCurrentThreadId());
        if (ep->ExceptionRecord->NumberParameters >= 2) std::fprintf(f, "acceso tipo=%llu direccion=%016llX\n", (unsigned long long)ep->ExceptionRecord->ExceptionInformation[0], (unsigned long long)ep->ExceptionRecord->ExceptionInformation[1]);
        DescribeAddr(f, "RIP", ep->ContextRecord->Rip);
        const uint64_t* sp = reinterpret_cast<const uint64_t*>(ep->ContextRecord->Rsp);
        int shown = 0;
        for (int i = 0; i < 512 && shown < 24; ++i) {
            uint64_t v;
            if (IsBadReadPtr(sp + i, 8)) break;
            v = sp[i];
            HMODULE m = nullptr;
            if (v > 0x10000 && GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(v), &m) && m) { DescribeAddr(f, "  pila", v); ++shown; }
        }
        std::fclose(f);
    }
    return g_prevFilter ? g_prevFilter(ep) : EXCEPTION_CONTINUE_SEARCH;
}

} // namespace

void DownmixStereo(const float* in, uint32_t n, float* out, uint64_t* clipped) {
    const float k = 0.70710678f;   // -3 dB (ITU-R BS.775-3)
    const float *L = in, *R = in + n, *C = in + 2 * n, *SL = in + 4 * n, *SR = in + 5 * n;   // in + 3n = LFE (descartado)
    for (uint32_t i = 0; i < n; ++i) {
        float l = L[i] + k * C[i] + k * SL[i];
        float r = R[i] + k * C[i] + k * SR[i];
        if (!(l == l)) l = 0; if (!(r == r)) r = 0;
        if (l > 1.f || l < -1.f) { if (clipped) ++*clipped; l = l > 1.f ? 1.f : -1.f; }
        if (r > 1.f || r < -1.f) { if (clipped) ++*clipped; r = r > 1.f ? 1.f : -1.f; }
        out[2 * i] = l; out[2 * i + 1] = r;
    }
}

void SinkConfigure(SinkMode mode, const std::string& dumpWavPath) { g_mode = mode; g_dumpPath = dumpWavPath; }

bool SinkStart() {
    {
        std::lock_guard<std::mutex> g(g_mu);
        if (g_initDone || g_starting) return g_st.deviceActive;
        g_starting = true;
    }
    if (!g_dumpPath.empty()) {
        g_wav = std::fopen(g_dumpPath.c_str(), "wb");
        if (g_wav) { WavHeader(g_wav, 0); std::fseek(g_wav, 44, SEEK_SET); rt::ImportLog::Get().Printf("AUDIO: volcado WAV 48000 Hz 16 bit estereo -> %s", g_dumpPath.c_str()); }
        else rt::ImportLog::Get().Printf("AUDIO: no se pudo crear el WAV %s", g_dumpPath.c_str());
    }
    g_run = true;
    g_prevFilter = SetUnhandledExceptionFilter(AudioCrashFilter);
    bool wasapi = false;
    if (g_mode != SinkMode::Null) {
        g_thread = CreateThread(nullptr, 0, WasapiMain, nullptr, 0, nullptr);
        if (g_thread) {
            for (int i = 0; i < 100 && g_init.state == 0; ++i) Sleep(50);   // hasta 5 s
            if (g_init.state == 1) wasapi = true;
            else {
                if (g_init.state == 0) rt::ImportLog::Get().Printf("AUDIO: WASAPI no respondio en 5 s: se usa el sumidero nulo");
                g_run = false; WaitForSingleObject(g_thread, 2000); CloseHandle(g_thread); g_thread = nullptr; g_run = true;
            }
        }
    }
    if (!wasapi) {
        g_thread = CreateThread(nullptr, 0, NullMain, nullptr, 0, nullptr);
        rt::ImportLog::Get().Printf("AUDIO: sumidero nulo con reloj de 48 kHz%s", g_mode == SinkMode::Wasapi ? " (--audio=wasapi solicitado pero no disponible)" : "");
    }
    std::lock_guard<std::mutex> g(g_mu);
    g_initDone = true; g_starting = false;
    return g_st.deviceActive;
}

void SinkStop() {
    g_run = false;
    if (g_thread) { WaitForSingleObject(g_thread, 2000); CloseHandle(g_thread); g_thread = nullptr; }
    std::lock_guard<std::mutex> g(g_mu);
    if (g_wav) { WavHeader(g_wav, g_wavSamples); std::fclose(g_wav); g_wav = nullptr; }
}

void SinkSubmitFrame(const uint8_t* gf) {
    float planar[6 * kSinkFrameSamples], st[kSinkFrameSamples * 2];
    for (uint32_t i = 0; i < 6 * kSinkFrameSamples; ++i) {
        uint32_t b; std::memcpy(&b, gf + i * 4, 4); b = _byteswap_ulong(b); std::memcpy(&planar[i], &b, 4);
    }
    uint64_t clipped = 0;
    DownmixStereo(planar, kSinkFrameSamples, st, &clipped);
    double ss = 0; float pk = 0;
    for (uint32_t i = 0; i < kSinkFrameSamples * 2; ++i) { ss += (double)st[i] * st[i]; pk = std::max(pk, std::fabs(st[i])); }
    std::lock_guard<std::mutex> g(g_mu);
    g_started = true;
    ++g_st.framesSubmitted; g_st.clipped += clipped;
    g_lvlSumSq += ss; g_lvlN += kSinkFrameSamples * 2; g_lvlPeak = std::max(g_lvlPeak, pk);
    if (g_count + kSinkFrameSamples > kRingSamples) {   // overrun: recorta la trama mas antigua
        g_head = (g_head + kSinkFrameSamples) % kRingSamples; g_count -= kSinkFrameSamples; ++g_st.framesDropped;   // no cuenta como consumida
    }
    uint32_t tail = (g_head + g_count) % kRingSamples;
    uint32_t first = std::min<uint32_t>(kSinkFrameSamples, kRingSamples - tail);
    std::memcpy(&g_ring[tail * 2], st, first * 2 * sizeof(float));
    if (first < kSinkFrameSamples) std::memcpy(&g_ring[0], st + first * 2, (kSinkFrameSamples - first) * 2 * sizeof(float));
    g_count += kSinkFrameSamples;
    if (g_wav) {
        int16_t pcm[kSinkFrameSamples * 2];
        for (uint32_t i = 0; i < kSinkFrameSamples * 2; ++i) pcm[i] = ToI16(st[i]);
        std::fwrite(pcm, sizeof(int16_t), kSinkFrameSamples * 2, g_wav);
        g_wavSamples += kSinkFrameSamples;
        if (g_wavSamples - g_wavLastPatch >= kSinkRate) { g_wavLastPatch = g_wavSamples; WavHeader(g_wav, g_wavSamples); std::fflush(g_wav); }
    }
}

uint32_t SinkQueuedSamples() { std::lock_guard<std::mutex> g(g_mu); return g_count; }
uint64_t SinkConsumedSamples() { std::lock_guard<std::mutex> g(g_mu); return g_st.samplesConsumed; }
bool SinkDeviceDriven() { std::lock_guard<std::mutex> g(g_mu); return g_st.deviceActive; }
SinkStats SinkGetStats() { std::lock_guard<std::mutex> g(g_mu); SinkStats s = g_st; s.queuedSamples = g_count; return s; }

void SinkTakeLevel(double& rmsDb, float& peak, uint64_t& samples) {
    std::lock_guard<std::mutex> g(g_mu);
    double rms = g_lvlN ? std::sqrt(g_lvlSumSq / (double)g_lvlN) : 0.0;
    rmsDb = rms > 0 ? 20.0 * std::log10(rms) : -200.0;
    peak = g_lvlPeak; samples = g_lvlN / 2;
    g_lvlSumSq = 0; g_lvlN = 0; g_lvlPeak = 0;
}

void SinkSummary() {
    SinkStats s = SinkGetStats();
    rt::ImportLog::Get().Printf("AUDIO sumidero: modo=%s tramas=%llu consumidas_muestras=%llu descartadas=%llu underruns=%llu (muestras=%llu) recortes=%llu wasapi_escrituras=%llu wasapi_muestras=%llu errores_hr=%u ultimo_hr=0x%08X wav_muestras=%llu",
                                s.deviceActive ? "wasapi" : "nulo", (unsigned long long)s.framesSubmitted, (unsigned long long)s.samplesConsumed,
                                (unsigned long long)s.framesDropped, (unsigned long long)s.underrunEvents, (unsigned long long)s.underrunSamples,
                                (unsigned long long)s.clipped, (unsigned long long)s.wasapiWrites, (unsigned long long)s.wasapiFramesWritten,
                                s.wasapiErrors, s.lastHr, (unsigned long long)g_wavSamples);
}

} // namespace rt::audio
