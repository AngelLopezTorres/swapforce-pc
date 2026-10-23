#pragma once
// Implementacion interna del backend D3D12 (H1-H5). Dividida en: d3d12_backend.cpp (dispositivo, presentacion, anillos),
// d3d12_draw.cpp (estado, PSO, draw), d3d12_rt.cpp (render targets y resolve), d3d12_tex.cpp (texturas).
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <atomic>
#include <functional>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "backend.h"
#include "d3d12_backend.h"
#include "gpu.h"
#include "import_log.h"
#include "ucode_hlsl.h"
#include "ucode_info.h"
#include "xenia/gpu/registers.h"
#include "xenia/gpu/xenos.h"

namespace rt::gpu { extern int g_syncReason; extern uint32_t g_syncAddr; }   // M60 (pm4.cpp)
namespace rt::gpu::d3d {

extern unsigned g_kickDraws, g_kickResolveDraws;
extern bool g_asyncResolveWriter;
extern bool g_resolveSync, g_noResolveShortcut, g_resolveNoDse, g_resolveSlowPath, g_resolveVerify, g_verifyShortcut;
extern bool g_depthResolveSync, g_depthResolveVerify, g_resolveStaging, g_gpuTiming, g_depthResolveCs, g_lazyReadback, g_strictResolveSync, g_presentVerify, g_orderedWrites, g_eagerIssue, g_swapMarks;   // M60: --depth-resolve-sync, --depth-resolve-verify
void SetDumpHlslHash(uint64_t hash);

using Microsoft::WRL::ComPtr;
namespace reg = xe::gpu::reg;
namespace xenos = xe::gpu::xenos;

constexpr uint32_t kW = 1280, kH = 720;           // tamano de la imagen presentada
constexpr uint32_t kFramesInFlight = 3;
constexpr uint64_t kSharedMemBytes = 0x20000000;  // memoria fisica del guest (512 MB)
constexpr uint32_t kUploadSectionBytes = 48u << 20;   // por fotograma en vuelo
constexpr uint32_t kCbvSrvHeapSize = 98304 * 8;   // descriptores visibles al shader (96 por draw); M60: x8, en 4 segmentos rotatorios
constexpr uint32_t kSrvSegments = 4;
constexpr uint32_t kSamplerHeapSize = 2048;

// Lectura de un registro como la union de registers.h (Get<T>() de Xenia RegisterFile).
template <class T> inline T R() { T t; t.value = rt::gpu::RegRead((uint32_t)T::register_index); return t; }
inline uint32_t RV(uint32_t idx) { return rt::gpu::RegRead(idx); }
inline float RF(uint32_t idx) { uint32_t v = rt::gpu::RegRead(idx); float f; std::memcpy(&f, &v, 4); return f; }

// ---- ventana ----
struct UiThread {
    HWND hwnd = nullptr;
    HANDLE thread = nullptr, ready = nullptr;
    std::atomic<bool> stop{false};
    bool Start();
    void Stop();
};

struct UploadAlloc { uint8_t* cpu = nullptr; D3D12_GPU_VIRTUAL_ADDRESS gpu = 0; uint64_t offset = 0; };

// ---- render targets del host ----
struct HostRT {
    ComPtr<ID3D12Resource> res;
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_RENDER_TARGET;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    uint32_t w = 0, h = 0;
    uint32_t viewIdx = 0;       // indice en el heap RTV o DSV
    bool depth = false;
    uint32_t xenosFormat = 0;   // ColorRenderTargetFormat o DepthRenderTargetFormat
    uint64_t lastUseDraw = 0;
    uint32_t writes = 0;
};

struct ResolveImage {            // resultado del ultimo resolve a una direccion (para presentar sin destilar)
    uint32_t destBase = 0, w = 0, h = 0, pitch = 0;
    std::vector<uint8_t> rgba;   // RGBA8 de la region resuelta (x0,y0)=(0,0) de la region
    uint32_t x0 = 0, y0 = 0;
    uint64_t frame = 0;
    uint32_t memHash = 0;
};

// ---- texturas del guest ----
struct HostTexture {
    ComPtr<ID3D12Resource> res;
    uint32_t cpuSrv = 0;         // indice en el heap CPU de SRV
    uint32_t dim = 0;            // 0 = 2D, 1 = 3D, 2 = cubo
    uint64_t dataHash = 0;
    uint64_t lastUse = 0;
    uint32_t w = 0, h = 0;
};

class D3D12Backend : public backend::Backend {
public:
    Options opt;
    UiThread ui;
    ComPtr<IDXGIFactory4> factory;
    ComPtr<ID3D12Device> dev;
    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent = nullptr;
    uint64_t fenceValue = 0;
    uint64_t allocFence[kFramesInFlight] = {};   // ultima valla que usa la seccion del anillo de subida de cada fotograma
    // M8: grupo de asignadores (una lista puede enviarse varias veces por fotograma: Kick)
    struct AllocSlot { ComPtr<ID3D12CommandAllocator> a; uint64_t fence = 0; };
    std::vector<AllocSlot> allocPool;
    int curAlloc = -1;
    uint32_t drawsSinceKick = 0;
    uint64_t kicks = 0;
    int AcquireAllocator();
    void Kick();                                // M8: envia lo grabado sin esperar y reabre la lista
    ComPtr<ID3D12GraphicsCommandList> cl;
    bool clOpen = false;
    uint32_t frameSlot = 0;
    ComPtr<IDXGISwapChain3> swap;
    ComPtr<ID3D12Resource> backbuf[kFramesInFlight];
    ComPtr<ID3D12Resource> frameRT;
    D3D12_RESOURCE_STATES frameRTState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    ComPtr<ID3D12Resource> readback;           // fotograma completo (volcado/estadisticas)
    ComPtr<ID3D12Resource> resolveReadback;    // resolve (region de un RT)
    uint64_t resolveReadbackBytes = 0;
    ComPtr<ID3D12Resource> depthReadback;      // plano de profundidad completo (copia D3D12)
    uint64_t depthReadbackBytes = 0;
    std::vector<uint8_t> depthResolveScratch;
    std::set<uint32_t> dumpSet;
    std::string adapterName;
    bool usingWarp = false;
    D3D12_RESOURCE_BINDING_TIER bindingTier = D3D12_RESOURCE_BINDING_TIER_1;

    // descriptores
    ComPtr<ID3D12DescriptorHeap> rtvHeap, dsvHeap, cpuSrvHeap, gpuSrvHeap, gpuSmpHeap;
    uint32_t rtvInc = 0, dsvInc = 0, srvInc = 0, smpInc = 0;
    uint32_t rtvUsed = 0, dsvUsed = 0, cpuSrvUsed = 0;
    uint32_t gpuSrvNext = 0, gpuSmpNext = 0;
    uint64_t srvSegFence[kSrvSegments] = {};   // M60: ultima valla que usa descriptores de cada segmento
    uint32_t nullSrv2D = 0, nullSrv3D = 0, nullSrvCube = 0;
    uint32_t frameRtvIdx = 0;

    // memoria compartida y anillo de subida
    ComPtr<ID3D12Resource> sharedMem;
    D3D12_RESOURCE_STATES sharedState = D3D12_RESOURCE_STATE_COPY_DEST;
    std::vector<uint32_t> pageHash;            // hash por pagina de 4 KB ya subida (0 = nunca)
    ComPtr<ID3D12Resource> uploadBuf;
    uint8_t* uploadCpu = nullptr;
    uint32_t uploadSection = 0, uploadOffset = 0;
    uint64_t uploadBytesFrame = 0, uploadBytesTotal = 0, pagesUploaded = 0, pagesSkipped = 0;

    // pipeline
    ComPtr<ID3D12RootSignature> rootSig;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> psoCache;
    struct DxilBlob { std::string data; bool ok = false; std::string err; };
    std::unordered_map<uint64_t, DxilBlob> dxilCache;
    DxilBlob gsRect;
    bool gsRectTried = false;
    std::unordered_map<std::string, uint32_t> samplerTables;    // clave de 32 muestreadores -> indice en el heap de muestreadores
    struct SmpFast { std::string desc; uint32_t base; };
    std::unordered_multimap<uint64_t, SmpFast> samplerFast;     // M8: indice por hash (se vacia junto con samplerTables)
    std::unordered_multimap<uint64_t, std::pair<std::string, ID3D12PipelineState*>> psoFast;   // M8: indice por hash de psoCache

    // render targets
    std::map<uint64_t, HostRT> colorRTs, depthRTs;
    std::vector<ResolveImage> resolveImages;
    uint64_t drawSerial = 0;
    uint32_t debugDumpTextureBase = 0;
    uint64_t debugDumpTextureFrame = 0, debugDumpTextureDraw = 0;
    uint64_t lastResolveNonzero = 0, lastResolvePixels = 0;
    uint32_t resolveDumps = 0, textureDumps = 0;

    // texturas
    std::unordered_map<uint64_t, HostTexture> textures;
    uint64_t texSerial = 0, texUploads = 0;

    // contadores
    uint64_t drawsOk = 0, drawsSkip = 0, shaderFail = 0, psoFail = 0, resolves = 0, hrFail = 0, removed = 0, texFail = 0, texCreated = 0, texHits = 0;
    uint64_t fDrawsOk = 0, fDrawsSkip = 0, fShaderFail = 0, fPsoFail = 0, fResolve = 0, fHrFail = 0, fTexFail = 0;
    uint64_t framesPresented = 0, presentCalls = 0;
    uint32_t firstHrLogs = 0;
    std::map<std::string, uint64_t> notes;          // SIMPLIFY y causas de omision (recuentos)
    std::set<std::string> notesLogged;
    std::unordered_map<const char*, uint64_t> notesLit;   // M60: recuentos por puntero de literal (sin std::string por llamada)

    // M60: --gpu-timing (marcas de tiempo de la cola al principio y al final de cada lista enviada; diagnostico)
    static constexpr uint32_t kTsSlots = 1u << 17;
    std::vector<uint8_t> tsCat;
    std::vector<uint64_t> tsTag;
    uint64_t tsListFirst = 0, tsReadbackBytes = 0, tsFlushWaits = 0;
    std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>> tsResolveBytes;   // destino|w|h|prof -> (bytes, n)
    double tsCatTicks[9] = {};
    std::unordered_map<uint64_t, double> tsTagTicks;
    void TsMark(uint8_t cat, uint64_t tag);
    ComPtr<ID3D12QueryHeap> tsHeap;
    ComPtr<ID3D12Resource> tsReadback;
    const uint64_t* tsCpu = nullptr;
    uint64_t tsFreq = 0, tsNext = 0, tsHarvest = 0;
    std::vector<uint64_t> tsFence;
    bool tsOpen = false;
    double tsBusyTicks = 0;
    uint64_t tsLists = 0, tsLastStart = 0, tsLastEnd = 0;
    LARGE_INTEGER tsPrint{};
    void TsBegin();
    void TsEnd();
    void TsHarvest();

    // ---- d3d12_backend.cpp ----
    bool Init();
    bool Hr(HRESULT hr, const char* what);
    void CheckRemoved();
    void WaitFence(uint64_t v);
    uint64_t Submit();
    void BeginList(bool newSection = true);
    void FlushAndWait();                        // cierra, ejecuta y espera; reabre la lista
    void Transition(ID3D12Resource* r, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
    void SetFrameRTState(D3D12_RESOURCE_STATES s);
    UploadAlloc Upload(uint32_t bytes, uint32_t align);
    void SetSharedState(D3D12_RESOURCE_STATES s);
    void EnsureMemory(uint32_t phys, uint32_t bytes);
    void Note(const std::string& key);          // cuenta y registra una vez (SIMPLIFY ...)
    void Note(const char* lit);                 // M60: literal (camino rapido; se funde con `notes` al cerrar)
    void OnSwap(const backend::SwapInfo& s) override;
    void OnDraw(const DrawEvent& ev, const ShaderInfo* vs, const ShaderInfo* ps) override;
    void Shutdown() override;
    bool PresentFromRgba(const uint8_t* rgba, uint32_t w, uint32_t h);

    // ---- d3d12_draw.cpp ----
    bool CreateRootSignature();
    const DxilBlob* GetDxil(const ShaderInfo& si, int paramGenPos);
    const DxilBlob* GetRectGs();
    ID3D12PipelineState* GetPso(const ShaderInfo& vs, const ShaderInfo* ps, uint32_t topoClass, bool rectGs);
    bool DoDraw(const DrawEvent& ev, const ShaderInfo* vs, const ShaderInfo* ps);
    uint32_t AllocGpuSrv(uint32_t n);

    // ---- d3d12_rt.cpp ----
    HostRT* GetColorRT(uint32_t base, uint32_t pitch, uint32_t fmt, uint32_t needH);
    HostRT* GetDepthRT(uint32_t base, uint32_t pitch, uint32_t fmt, uint32_t needH);
    bool DoResolve(const DrawEvent& ev);
    // M8: resolve diferido y atajo GPU
    struct PendingResolve {
        uint64_t frame = 0;
        uint32_t bpp = 4, bppLog2 = 2, rowPitch = 0, w = 0, h = 0;
        int32_t x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        uint32_t destBase = 0, endian = 0, dpAligned = 0, sfmt = 0, dfmt = 0;
        bool swapRB = false, ok32 = false;
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        int64_t lo = 0, hi = 0;
        uint8_t* mem = nullptr;
        uint64_t poolOffset = 0;
        uint64_t fence = 0;     // valla tras la que la copia de lectura esta completa
        bool dead = false;      // cubierto por un resolve posterior (no se escribe)
        // M60: resolve de profundidad diferido. El bufer de lectura contiene el plano 0 completo (float32, fila srcPitch);
        // la conversion a 20e4 (<<8) se hace al escribir (DepthResolveSource), no en el hilo del CP.
        bool depth = false;
        uint32_t srcPitch = 0;
        // M60: lectura perezosa. La region ya esta en resolveStage (VRAM) en poolOffset; la copia al bufer de lectura se
        // emite en FlushResolves solo si el resolve sigue vivo (no cubierto por otro posterior). fence = kNotReady hasta entonces.
        bool lazyCopy = false;
        uint64_t copyBytes = 0;
        // M60: escritura ordenada del CP (EVENT_WRITE_SHD/EXT, MEM_WRITE, REG_TO_MEM, COND_WRITE): [lo, lo + wbytes) <- wdata
        bool isWrite = false;
        uint32_t wbytes = 0;
        uint8_t wdata[16] = {};
        uint64_t markSeq = 0;   // M60: marca de XE_SWAP (isWrite con wbytes = 0)
        bool predDead = false;  // M60: prediccion (por el fotograma anterior) de que otro resolve posterior lo cubrira
    };
    std::unordered_map<uint64_t, uint32_t> predPrev, predCur;   // M60: resolves por destino y rectangulo en el fotograma anterior / actual
    void PredictDead(PendingResolve& pr);
    static constexpr uint64_t kNotReady = ~0ull;
    const uint8_t* DepthResolveSource(const PendingResolve& pr, const uint8_t* src, std::vector<uint8_t>& scratch, bool parallel);   // M60
    std::vector<uint8_t> depthWrScratch;   // M60: usado solo por el hilo escritor o por FlushResolves sincronico
    struct ResolvedTex {
        HostTexture ht;
        D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST;
        uint32_t srvMap = 0xFFFFFFFF;
        bool valid = false, swapRB = false;
        uint32_t destBase = 0, w = 0, h = 0, dfmt = 0, endian = 0, dpAligned = 0, lo = 0, hi = 0;
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        uint64_t serial = 0;      // incrementa con cada copia del resolve
        uint64_t srcSerial = 0;   // derivada: serial de la copia de origen ya copiada
        bool derived = false;
    };
    uint64_t resolveCopySerial = 0, resolveSubCopies = 0;
    std::vector<PendingResolve> pendingResolves;   // modo sincronico
    // M8: hilo escritor (modo asincrono)
    std::deque<PendingResolve> wrQueue;
    std::mutex wrMu;
    std::condition_variable wrCv, wrDone;
    std::thread wrThread;
    bool wrBusy = false, wrStop = false;
    uint64_t wrMaxFence = 0;
    const uint8_t* resolvePoolCpu = nullptr;
    void EnqueueResolve(PendingResolve pr);
    ComPtr<ID3D12Resource> resolvePool;
    ComPtr<ID3D12Resource> resolveStage;                 // M60: copia intermedia en VRAM (--resolve-staging)
    D3D12_RESOURCE_STATES resolveStageState = D3D12_RESOURCE_STATE_COPY_DEST;
    // M60: resolve de profundidad por sombreador de calculo (solo la region; conversion float32 -> 20e4<<8 en la GPU)
    ComPtr<ID3D12RootSignature> csRootSig;
    ComPtr<ID3D12PipelineState> csDepthPso;
    bool csDepthTried = false;
    bool EnsureDepthCs();
    bool DepthRegionToStage(HostRT* src, PendingResolve& pr);
    // M60: presentacion desde la copia GPU del resolve del frontbuffer (sin esperar la escritura en memoria guest)
    ComPtr<ID3D12RootSignature> csBlitRootSig;
    ComPtr<ID3D12PipelineState> csBlitPso;
    bool csBlitTried = false;
    bool EnsureBlitCs();
    bool PresentFromResolvedTex(const backend::SwapInfo& s);
    ComPtr<ID3D12Resource> viewerReadback;
    uint64_t viewerFence = 0, viewerFrame = 0;
    uint32_t viewerW = 0, viewerH = 0;
    bool viewerPending = false;
    void ViewerHarvest();
    uint64_t presentGpu = 0, presentCpu = 0, orderedWrites = 0;
    std::atomic<uint64_t> wrBusyTicks{0}, wrCount{0}, wrBytes{0};   // M60 diagnostico del hilo escritor
    void CopyToResolvePool(const D3D12_TEXTURE_COPY_LOCATION& src, const D3D12_BOX* box, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& fp, uint64_t bytes, PendingResolve& pr);
    bool LazyReadback() const;
    uint64_t resolvePoolBytes = 0, resolvePoolOffset = 0;
    std::unordered_map<uint64_t, ResolvedTex> resolvedTex;
    std::atomic<uint64_t> resolveDeadSkipped{0}, resolveWritten{0};
    uint64_t resolveFlushByRead = 0, resolveShortcutHits = 0;
    void WriteResolve(const PendingResolve& pr, const uint8_t* srcp);
    bool EnsureResolvePool(uint64_t need);
    void FlushResolves();
    bool FlushResolvesIfOverlap(uint32_t lo, uint32_t hi);
    std::unordered_map<uint64_t, uint64_t> tsTexFlush;
    uint64_t tsSyncN[16] = {}; double tsSyncMs[16] = {}, tsSyncGpuMs = 0;   // M60: vaciados con resolves pendientes por causa   // M60 diagnostico: base|fmt|w|h de texturas que fuerzan vaciado
    void InvalidateResolvedTex(uint32_t lo, uint32_t hi);
    void RecordResolvedTex(const PendingResolve& pr, const D3D12_TEXTURE_COPY_LOCATION& srcLoc, const D3D12_BOX& box);
    HostTexture* LookupResolvedTex(uint32_t base, uint32_t width, uint32_t height, uint32_t fmt, uint32_t endian, uint32_t pitchAligned, uint32_t swizzle);
    // M60: los puntos del CP que no son de fin de canalizacion ni de coherencia (INTERRUPT=1, XE_SWAP=2, fin de lote=9, WAIT_REG_MEM
    // sobre memoria=15 sin solape con resolves pendientes) no esperan a los resolves. Siguen vaciando: EVENT_WRITE_SHD/EXT, MEM_WRITE,
    // REG_TO_MEM, COND_WRITE, WAIT_REG_MEM sobre registro (COHER_STATUS_HOST: coherencia pedida por la CPU) y las lecturas del CP
    // (texturas, memoria, frontbuffer) por solape. --strict-resolve-sync restaura el criterio de M8 (vaciado en todos).
    // M60 (2): las escrituras del CP (4 REG_TO_MEM, 5 MEM_WRITE, 6 COND_WRITE, 7 EVENT_WRITE_SHD, 8 EVENT_WRITE_EXT) no esperan: se emiten
    // las copias pendientes y la escritura se encola detras de los resolves (WriteOrdered), como una escritura de fin de canalizacion.
    // COHER_STATUS_HOST (3): vaciado solo si el rango pedido (COHER_BASE_HOST, COHER_SIZE_HOST) solapa algo pendiente.
    void SyncPoint() override {
        const int r = rt::gpu::g_syncReason;
        if (!g_strictResolveSync && g_asyncResolveWriter) {
            if (r == 2) { SwapMark(); rt::gpu::g_syncReason = 0; return; }
            if (r == 1 || r == 9) { rt::gpu::g_syncReason = 0; return; }
            if (r == 15) { FlushResolvesIfOverlap(rt::gpu::g_syncAddr, rt::gpu::g_syncAddr + 4); rt::gpu::g_syncReason = 0; return; }
            if (r >= 4 && r <= 8 && g_orderedWrites) { IssuePending(); rt::gpu::g_syncReason = 0; return; }
            if (r == 3 && rt::gpu::g_syncAddr == 0x0A31) {
                // Con el bit 31 a 0 (sin peticion) se sigue vaciando: omitirlo provoco DXGI_ERROR_DEVICE_HUNG (ejecuciones c13 y c16, esta
                // ultima con marcas de XE_SWAP). El juego usa esta espera como barrera de "GPU inactiva" (PROGRESS.md, 60 FPS).
                uint32_t base = RV(0x0A30) & 0x1FFFFFFF, size = RV(0x0A2F);
                if (size && (uint64_t)base + size <= 0x20000000ull) { FlushResolvesIfOverlap(base, base + size); rt::gpu::g_syncReason = 0; return; }
            }
        }
        if (r == 3 && g_gpuTiming) {   // M60 diagnostico
            static int n = 0;
            if (n < 12) { ++n; ImportLog::Get().Printf("M60 WAIT_REG_MEM registro=%04X COHER base=%08X size=%08X status=%08X", rt::gpu::g_syncAddr, RV(0x0A30), RV(0x0A2F), RV(0x0A31)); }
        }
        FlushResolves();
        rt::gpu::g_syncReason = 0;
    }
    bool WriteOrdered(uint32_t phys, const void* data, uint32_t bytes) override;
    void ReadBarrier(uint32_t phys, uint32_t bytes) override { FlushResolvesIfOverlap(phys, phys + bytes); }
    void IssuePending();
    void SwapMark();
    void IssueCopies(bool keepLast);
    bool shortcutVerifyActive = false;
    void VerifyShortcut(HostTexture* gpuTex, HostTexture* cpuTex, uint32_t base, uint32_t w, uint32_t h, uint32_t fmt);
    // reparto de filas entre hilos auxiliares (escrituras disjuntas); devuelve false si algun tramo fallo
    bool ParallelRows(uint32_t rows, const std::function<bool(uint32_t, uint32_t)>& fn);
    void ClearRT(HostRT* rt, const float* color);

    // ---- d3d12_tex.cpp ----
    HostTexture* GetTexture(uint32_t fetchIdx, bool vertexStage);
    std::vector<uint8_t> texScratch;            // M8: bufer de destilado reutilizado
    std::vector<uint8_t> resolveRow;
    std::vector<uint8_t> fbScratch;            // M8: fila convertida del resolve
    struct FrameResolve { uint32_t lo, hi, destBase, w, h, fmt, endian, x0, y0; };
    std::vector<FrameResolve> frameResolves;
    // M8: memo del hash de datos de textura dentro de un fotograma (clave de textura -> rango y hash). Se vacia en cada XE_SWAP
    // y se invalidan las entradas que solapan cualquier resolve (escritura de la GPU en memoria guest).
    struct TexMemo { uint32_t lo, hi; uint64_t hash; };
    std::unordered_map<uint64_t, TexMemo> texMemo;
    uint64_t texMemoHits = 0, texMemoMiss = 0;
    void MemWritten(uint32_t a, uint32_t n) override {
        uint32_t lo = a & 0x1FFFFFFF;
        InvalidateTexMemo(lo, lo + n);
        InvalidateResolvedTex(lo, lo + n);
    }
    void InvalidateTexMemo(uint32_t lo, uint32_t hi) {
        for (auto it = texMemo.begin(); it != texMemo.end();) { if (it->second.lo < hi && it->second.hi > lo) it = texMemo.erase(it); else ++it; }
    }
    bool SeqLog() const { static long f = std::getenv("M8_SEQ_FRAME") ? std::atol(std::getenv("M8_SEQ_FRAME")) : -1; return f >= 0 && (long)rt::gpu::FrameCount() + 1 == f; }    // M8 (diagnostico): resolves del fotograma en curso
    uint64_t fTexOverlap = 0, fTexExact = 0, fTexCalls = 0, fResolveDistinctAfter = 0;
    D3D12_SAMPLER_DESC SamplerFromFetch(uint32_t fetchIdx);
};

DXGI_FORMAT ColorFormatToDxgi(uint32_t xenosFmt, bool* approx);

} // namespace rt::gpu::d3d
