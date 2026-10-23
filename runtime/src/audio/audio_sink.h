// Sumidero de audio del XAudio guest (A1). Recibe tramas de 6 canales float big-endian (256 muestras por canal, canales
// secuenciales: L, R, C, LFE, SL, SR), las mezcla a estereo (ITU-R BS.775: C y SL/SR a -3 dB, LFE descartado) y las entrega a:
//   - WASAPI, modo compartido, flujo dirigido por eventos (Microsoft Learn: IAudioClient / IAudioRenderClient), o
//   - un sumidero nulo con reloj de 48 kHz (comportamiento anterior).
// El reloj que libera los semaforos del cliente guest es el del dispositivo (muestras consumidas), no un reloj virtual.
// Opcionalmente vuelca lo entregado por el guest a un WAV de 16 bit / 48 kHz / estereo (--dump-audio).
#pragma once
#include <cstdint>
#include <string>

namespace rt::audio {

enum class SinkMode { Auto, Null, Wasapi };

constexpr uint32_t kSinkFrameSamples = 256;       // muestras por canal y trama guest
constexpr uint32_t kSinkRate = 48000;

struct SinkStats {
    bool deviceActive = false;           // true: WASAPI abierto y en marcha
    uint64_t framesSubmitted = 0;        // tramas guest recibidas
    uint64_t samplesConsumed = 0;        // muestras estereo consumidas por el dispositivo (o el reloj nulo)
    uint64_t framesDropped = 0;          // tramas descartadas por cola llena (overrun)
    uint64_t underrunEvents = 0;         // veces que el dispositivo pidio datos y la cola estaba vacia (tras el primer dato)
    uint64_t underrunSamples = 0;        // muestras de silencio insertadas por underrun
    uint64_t clipped = 0;                // muestras recortadas al mezclar
    uint64_t wasapiWrites = 0;           // llamadas GetBuffer/ReleaseBuffer correctas
    uint64_t wasapiFramesWritten = 0;    // muestras estereo escritas al dispositivo
    uint32_t wasapiErrors = 0;           // HRESULT fallidos tras abrir
    uint32_t lastHr = 0;
    uint32_t queuedSamples = 0;          // cola actual (muestras estereo)
};

void SinkConfigure(SinkMode mode, const std::string& dumpWavPath);
bool SinkStart();                              // idempotente; true si hay salida WASAPI real
void SinkStop();                               // cierra WASAPI y finaliza el WAV
void SinkSubmitFrame(const uint8_t* guestFrame);   // 6 x 256 float big-endian (6144 bytes), puntero HOST
uint32_t SinkQueuedSamples();
uint64_t SinkConsumedSamples();
bool SinkDeviceDriven();                       // el reloj es el del dispositivo real
SinkStats SinkGetStats();
// RMS (dBFS) y pico de lo entregado desde la ultima llamada (ventana de estadistica); reinicia la ventana.
void SinkTakeLevel(double& rmsDb, float& peak, uint64_t& samples);
// Resumen (una linea de registro) para el cierre.
void SinkSummary();
// Utilidad pura (probada en selftest): mezcla 6 canales planares float (host) a estereo.
// in: 6 x n floats (planar, orden L R C LFE SL SR); out: n pares L,R.
void DownmixStereo(const float* in6, uint32_t n, float* outLR, uint64_t* clipped);

} // namespace rt::audio
