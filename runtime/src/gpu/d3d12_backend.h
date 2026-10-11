#pragma once
// Backend D3D12 (H1+): ventana, dispositivo, cola directa, swap chain, volcado de fotogramas a PNG.
#include <string>

namespace rt::gpu::d3d {

struct Options {
    bool headless = false;      // sin ventana ni swap chain: render a textura fuera de pantalla
    bool warp = false;          // adaptador WARP (software)
    bool clearTest = false;     // H1: cada fotograma se rellena con un color fijo (prueba de la cadena de presentacion)
    std::string dumpFrames;     // "N[,M..]": fotogramas del backend que se escriben como PNG en runtime/userdata/frames
    uint32_t dumpTextures = 0;  // escribir como PNG las primeras K subidas de textura
    uint32_t dumpResolves = 0;  // escribir como PNG los primeros K resolves de color de 32 bpp
    bool stats = false;         // calcular nonblack_px/hash en cada fotograma (lectura de la textura)
};

// Crea el backend y lo registra en el procesador de comandos. Devuelve false si no se puede inicializar.
bool Start(const Options& o);

} // namespace rt::gpu::d3d
