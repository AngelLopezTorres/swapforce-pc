#pragma once
// Visor en vivo en navegador (PLAN_VIEW.md). Servidor HTTP en 127.0.0.1 (Winsock), sin autenticacion.
// Captura en el hilo del CP (OnSwap) con filtro de ritmo y try_lock; codificacion JPEG (WIC) en hilo propio.
#include <atomic>
#include <cstdint>
#include <string>

namespace rt::viewer {

struct Options {
    bool enabled = false;      // --viewer [puerto]
    uint16_t port = 8765;      // 1024-65535
    uint32_t fps = 10;         // --viewer-fps 1-30
    uint32_t quality = 70;     // --viewer-quality 10-95
    uint32_t scale = 1;        // --viewer-scale 1|2|4
    bool input = false;        // --viewer-input (acepta /key)
    bool synthetic = false;    // --viewer-synthetic (diagnostico: patron animado en lugar de fotogramas del juego)
};

Options& Cfg();

// Analiza --viewer [puerto], --viewer-fps N, --viewer-quality N, --viewer-scale 1|2|4, --viewer-input, --viewer-synthetic.
// Devuelve true si consume argv[i] (y, si procede, el valor siguiente). Valores fuera de rango: se registra y se usa el defecto.
bool ParseArg(int argc, char** argv, int& i);

// Arranca el servidor. Devuelve false si no pudo enlazar (se registra en el log; el juego continua).
bool Start(const Options& o);
void Stop();

// Iniciado (atomico): comprobacion barata en el hilo del CP.
extern std::atomic<bool> g_active;

// Llamado por el hilo del CP con el RGBA ya destilado (stride en bytes; cw x ch = region util).
void OnFrame(const uint8_t* rgba, uint32_t stride, uint32_t cw, uint32_t ch, uint64_t frame);

// Botones del navegador (mascara XINPUT_GAMEPAD); 0 si caducados (>500 ms sin latido) o sin --viewer-input.
uint16_t Buttons();

// Autoprueba V0: JPEG de patron sintetico (FFD8/FFD9, decodificacion WIC, PSNR).
int RunJpegSelfTest();

} // namespace rt::viewer
