// Envoltorio de libavcodec (FFmpeg, LGPL) cargado DINAMICAMENTE con LoadLibrary para decodificar XMA2 (decodificador "xma2" de
// libavcodec/wmaprodec.c, AV_CODEC_ID_XMA2). No hay dependencia de enlace: si las DLL (avcodec-NN.dll, avutil-NN.dll,
// swresample-NN.dll) no estan, FfAvailable() devuelve false y el runtime usa el sumidero silencioso de XMA.
// Las cabeceras (solo en tiempo de compilacion) se toman de runtime\third_party_bin\ffmpeg\include si existen
// (SWAPFORCE_FFMPEG_HEADERS); esa carpeta esta ignorada por git.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace rt::audio {

// Carga las DLL una vez. Devuelve true si estan cargadas y la version mayor de avcodec coincide con la de las cabeceras.
bool FfAvailable(std::string* info = nullptr);
const std::string& FfVersionInfo();   // "avcodec 62.28.103 (carpeta)" o motivo del fallo

class FfXmaDecoder {
public:
    FfXmaDecoder();
    ~FfXmaDecoder();
    FfXmaDecoder(const FfXmaDecoder&) = delete;
    FfXmaDecoder& operator=(const FfXmaDecoder&) = delete;
    bool Open(int channels, int sampleRate);   // 1 o 2 canales
    void Reset();                              // avcodec_flush_buffers
    bool IsOpen() const { return ctx_ != nullptr; }
    int channels() const { return channels_; }
    int sampleRate() const { return rate_; }
    // Envia un paquete XMA2 de 2048 bytes (con cabecera de 32 bits) y anade el PCM decodificado (int16 entrelazado,
    // endian nativo) a out. Devuelve el numero de tramas (512 muestras por canal) obtenidas, o -1 si hay error.
    int DecodePacket(const uint8_t* packet2048, std::vector<int16_t>& out, int* samplesPerFrame = nullptr);
    uint64_t errors() const { return errors_; }
    int lastError() const { return lastErr_; }

private:
    void* ctx_ = nullptr;     // AVCodecContext*
    void* frame_ = nullptr;   // AVFrame*
    int channels_ = 0, rate_ = 0, lastErr_ = 0;
    uint64_t errors_ = 0;
};

} // namespace rt::audio
