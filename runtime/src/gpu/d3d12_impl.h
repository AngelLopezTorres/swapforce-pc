#pragma once
// Implementacion interna del backend D3D12 (H1-H5). Dividida en: d3d12_backend.cpp (dispositivo, presentacion, anillos),
// d3d12_draw.cpp (estado, PSO, draw), d3d12_rt.cpp (render targets y resolve), d3d12_tex.cpp (texturas).
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <atomic>
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

namespace rt::gpu::d3d {

using Microsoft::WRL::ComPtr;
namespace reg = xe::gpu::reg;
namespace xenos = xe::gpu::xenos;

constexpr uint32_t kW = 1280, kH = 720;           // tamano de la imagen presentada
constexpr uint32_t kFramesInFlight = 3;
constexpr uint64_t kSharedMemBytes = 0x20000000;  // memoria fisica del guest (512 MB)
constexpr uint32_t kUploadSectionBytes = 48u << 20;   // por fotograma en vuelo
constexpr uint32_t kCbvSrvHeapSize = 98304;       // descriptores visibles al shader (96 por draw)
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
    ComPtr<ID3D12CommandAllocator> alloc[kFramesInFlight];
    uint64_t allocFence[kFramesInFlight] = {};
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
    std::set<uint32_t> dumpSet;
    std::string adapterName;
    bool usingWarp = false;
    D3D12_RESOURCE_BINDING_TIER bindingTier = D3D12_RESOURCE_BINDING_TIER_1;

    // descriptores
    ComPtr<ID3D12DescriptorHeap> rtvHeap, dsvHeap, cpuSrvHeap, gpuSrvHeap, gpuSmpHeap;
    uint32_t rtvInc = 0, dsvInc = 0, srvInc = 0, smpInc = 0;
    uint32_t rtvUsed = 0, dsvUsed = 0, cpuSrvUsed = 0;
    uint32_t gpuSrvNext = 0, gpuSmpNext = 0;
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

    // render targets
    std::map<uint64_t, HostRT> colorRTs, depthRTs;
    std::vector<ResolveImage> resolveImages;
    uint64_t drawSerial = 0;
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

    // ---- d3d12_backend.cpp ----
    bool Init();
    bool Hr(HRESULT hr, const char* what);
    void CheckRemoved();
    void WaitFence(uint64_t v);
    uint64_t Submit();
    void BeginList();
    void FlushAndWait();                        // cierra, ejecuta y espera; reabre la lista
    void Transition(ID3D12Resource* r, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
    void SetFrameRTState(D3D12_RESOURCE_STATES s);
    UploadAlloc Upload(uint32_t bytes, uint32_t align);
    void SetSharedState(D3D12_RESOURCE_STATES s);
    void EnsureMemory(uint32_t phys, uint32_t bytes);
    void Note(const std::string& key);          // cuenta y registra una vez (SIMPLIFY ...)
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
    void ClearRT(HostRT* rt, const float* color);

    // ---- d3d12_tex.cpp ----
    HostTexture* GetTexture(uint32_t fetchIdx, bool vertexStage);
    D3D12_SAMPLER_DESC SamplerFromFetch(uint32_t fetchIdx);
};

DXGI_FORMAT ColorFormatToDxgi(uint32_t xenosFmt, bool* approx);

} // namespace rt::gpu::d3d
