#pragma once
#include <atomic>
// Subsistema GPU minimo (Fases 1-4): registros en 0x7FC80000, hilo del procesador de comandos (anillo PM4,
// puntero de lectura con write-back), hilo de vsync a 60 Hz con callback de interrupcion y analizador PM4.
// Referencias: Xenia graphics_system.cc, command_processor.cc, xboxkrnl_video.cc (copias en runtime\ref\).
#include <cstdint>
#include <string>

namespace rt::gpu {

struct Config {
    bool regDump = false;          // --gpu-regdump: registra cada indice que cambia (GPUREG W idx=...)
    bool noThreads = false;        // pruebas: no crear hilos
    uint32_t logPerFrame = 48;     // lineas de paquetes por fotograma en gpu.log (los DRAWSTATE y resumenes no tienen limite)
    std::string logPath = "gpu.log";
    bool stateLog = false;         // --gpu-statelog: una linea DRAWSTATE por draw en gpu.log
    uint64_t stateLogFrame = 0;     // --gpu-statelog-frame N: estado detallado solo en el fotograma N
    bool shaderScan = false;       // M8: --shader-scan (barrido de contenedores de shaders a 10 s, 30 s y al final)
    std::string framesDir;         // M8: --frames-dir
    bool hist = false;             // M8: --gpu-hist (o --gpu-statelog): histogramas H0 por draw (TEXFMT/VTXFMT/...); antes siempre activos
    bool dumpFb = false;           // --dump-fb: destila el frontbuffer por CPU en cada XE_SWAP (nonblack_px, hash)
    std::string fbFrames;          // --fb-frames N[,M..]: fotogramas del frontbuffer que se escriben como PNG
    bool noD3d = false;            // --no-d3d: sin backend de dibujo (solo CP/analisis)
    bool headless = false;         // --headless: D3D12 sin ventana
    bool warp = false;             // --warp: adaptador WARP
    bool clearTest = false;        // --d3d-clear-test: color fijo por fotograma (prueba H1)
    bool d3dStats = false;         // --d3d-stats: nonblack_px/hash en cada fotograma
    uint32_t dumpResolves = 0;     // --dump-resolves K
    uint64_t dumpResolveFrame = 0; // --dump-resolves-frame N: resolves de color del fotograma N
    uint32_t dumpTextures = 0;     // --dump-textures K
    bool binkTrace = false;        // --bink-trace: BINKPLANE por draw del PS de composicion Bink (M0)
    std::string dumpFrames;        // --dump-frames N[,M..]: PNG del fotograma renderizado por el backend
    bool traceSwap = false;        // --trace-swap-interval: estado de la cola de volteo guest (VB/SWAP) en gpu.log
    uint32_t swapInterval = 1;     // --swap-interval N (1-15): M60, intervalo de presentacion (vblancos por volteo) que sustituye al que pide el juego (2).
                                   // Defecto 1 (60 Hz; ver PROGRESS.md 60 FPS: la logica medida es por tiempo real). 0 = el del juego (30 fps).
    bool legacyVsync = false;      // M60: --legacy-sleep (VBLANK con Sleep(1), como antes)
    uint32_t vblankHz = 60;        // --vblank-hz N: frecuencia del hilo de VBLANK (experimento M60; por defecto 60)
};
Config& Cfg();
std::string FramesDir();   // M8: --frames-dir DIR (por defecto runtime/userdata/frames)

void Start();                      // valores por defecto de registros + hilos CP y vsync
void Stop();

// Interfaz de los imports Vd* (guest).
void SetInterruptCallback(uint32_t callback, uint32_t userData);
void InitializeRingBuffer(uint32_t phys, uint32_t sizeLog2);
void EnableReadPointerWriteBack(uint32_t phys, uint32_t blockSizeLog2);
void RecordSwapCall();
void M60Event(char c, uint32_t a = 0);
// M60: marcas de XE_SWAP en la cola del escritor de resolves. El VBLANK (que puede voltear) espera a que el escritor haya aplicado
// todo lo anterior al ultimo XE_SWAP procesado: un fotograma no se muestra (ni el juego recicla su memoria) antes de estar completo.
extern std::atomic<uint64_t> g_swapMarkQueued, g_swapMarkDone, g_vbWaitTicks, g_vbWaits;   // M60 diagnostico (--gpu-timing): anillo de eventos con marca QPC
void M60DumpEvents(const char* why);

// Estado observable (pruebas y diagnostico).
uint64_t FrameCount();
uint64_t VblankCount();
uint32_t ReadPointer();
uint32_t WritePointer();
uint64_t SwapCalls();

// Acceso a la memoria fisica (vista 0xA0000000; Xenia memory.cc: TranslatePhysical = base + (addr & 0x1FFFFFFF)).
uint8_t* PhysHost(uint32_t phys, uint32_t bytes);   // nullptr si algun byte no esta confirmado
uint32_t VirtToPhys(uint32_t virt);                  // Xenia: direcciones >= 0xA0000000 -> & 0x1FFFFFFF

// Fichero de registros interno (orden del host).
extern const uint32_t* g_regFile;   // M8: fichero de registros (kRegCount = 0x5003 entradas)
inline uint32_t RegRead(uint32_t idx) { return idx < 0x5003u ? g_regFile[idx] : 0; }
void RegWriteCpu(uint32_t idx, uint32_t v);          // escritura desde el CP: refleja en la pagina guest si idx < 0x4000

void GpuLog(const char* line);                       // linea en gpu.log
int SelfTestRender(const std::string& shaderDir, uint32_t r, uint32_t g, uint32_t b);   // prueba de dibujo de extremo a extremo
void ScanShaderContainers(const char* tag);          // Fase 4a
void Summary();                                      // una linea en runtime.log

} // namespace rt::gpu
