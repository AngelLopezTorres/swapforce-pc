#pragma once

#include <cstdint>
#include <string>

namespace rt::audio {

constexpr uint32_t kXmaContextCount = 320;
constexpr uint32_t kXmaContextBytes = 64;
constexpr uint32_t kXmaPacketBytes = 2048;
constexpr uint32_t kXmaSamplesPerFrame = 512;
constexpr uint32_t kXmaSamplesPerSubframe = 128;

constexpr uint32_t kXmaContextArrayAddressRegister = 0x7FEA1800;
constexpr uint32_t kXmaCurrentContextIndexRegister = 0x7FEA1818;
constexpr uint32_t kXmaNextContextIndexRegister = 0x7FEA181C;
constexpr uint32_t kXmaContext0KickRegister = 0x7FEA1940;
constexpr uint32_t kXmaContextKickRegisterCount = 10;

// Los dwords permanecen en big-endian guest. Se evita exponer bitfields C++,
// cuyo orden de bits y bytes depende del compilador anfitrion.
struct XmaContextData {
    uint32_t dwords[16];
};
static_assert(sizeof(XmaContextData) == kXmaContextBytes, "XMA context must be 64 bytes");

enum class XmaContextWord : uint8_t {
    Input0CountAndFlags = 0,
    Input1CountAndFlags = 1,
    InputReadOffset = 2,
    LoopStart = 3,
    LoopEndAndMetadata = 4,
    Input0PhysicalAddress = 5,
    Input1PhysicalAddress = 6,
    OutputPhysicalAddress = 7,
    WorkPhysicalAddress = 8,
    OutputReadOffsetAndFlags = 9,
    Reserved10 = 10,
};

enum class XmaDecodeBoundaryResult {
    InvalidContext,
    BackendUnavailable,
    Queued,            // hay decodificador libavcodec: el trabajador procesara el contexto
};

// Contadores del motor de decodificacion (A2).
struct XmaStats {
    bool backend = false;               // libavcodec cargado
    uint64_t passes = 0;                // pasadas por contexto con trabajo
    uint64_t packetsFed = 0;            // paquetes de 2048 B enviados al decodificador
    uint64_t framesDecoded = 0;         // AVFrame obtenidos de libavcodec (cada uno agrupa varias tramas XMA de 512 muestras)
    uint64_t samplesDecoded = 0;        // muestras por canal decodificadas
    uint64_t blocksWritten = 0;         // bloques de 256 B escritos en el buffer de salida guest
    uint64_t nonSilentBlocks = 0;       // bloques escritos con alguna muestra distinta de cero
    uint64_t buffersConsumed = 0;       // buffers de entrada liberados
    uint64_t repositions = 0;           // desplazamiento de lectura modificado por el guest
    uint64_t decodeErrors = 0;
    uint64_t ringBlocked = 0;           // pasadas detenidas por buffer de salida lleno
    int peak = 0;                       // pico absoluto int16 escrito
};

uint32_t CreateXmaContext();
bool ReleaseXmaContext(uint32_t guestAddress);
uint32_t XmaContextArrayGuestAddress();
bool WriteXmaRegister(uint32_t guestAddress, uint32_t value);
uint32_t ReadXmaRegister(uint32_t guestAddress);
XmaDecodeBoundaryResult SubmitXmaDecode(uint32_t contextGuestAddress);
uint64_t XmaDecodeRequestCount();
bool XmaDecoderBackendAvailable();
uint32_t SimulateXmaStep();           // respaldo sin decodificador: salida silenciosa
uint32_t XmaDecodeStep();             // una pasada del motor (libavcodec si esta disponible; si no, SimulateXmaStep)
XmaStats XmaGetStats();
void XmaForceSilence(bool on);                   // --xma=silence
void XmaSetTrace(bool on);                       // --xma-trace
void XmaSetCaptureDir(const std::string& dir);   // --xma-capture: vuelca los paquetes enviados por contexto (diagnostico)
// Prueba de decodificacion: dir -> bloques reales (fichero de paquetes de 2048 B) decodificados directamente y a traves del
// contexto guest; compara ambos bit a bit. Devuelve true si todo coincide. Escribe el detalle con printf.
bool XmaDecodeSelfTest(uint8_t* guestBase, const std::string& packetFile, int channels, int sampleRate);
uint64_t XmaSimulatedPacketCount();
void StartXmaWorker();
void StopXmaWorker();

} // namespace rt::audio
