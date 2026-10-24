#include "perf.h"
#include <algorithm>
#include "fast_hash.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include "d3d12_impl.h"

#include "config.h"
#include "fb_decode.h"
#include "image_io.h"
#include "gpu_names.h"
#include "viewer/viewer.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "user32.lib")

namespace rt::gpu::d3d {

// ---- Hilo de interfaz: solo crea la ventana y bombea mensajes (nunca bloquea; DXGI puede enviar SendMessage a este hilo).
//      Documentacion: "Multithreading and DXGI" (Microsoft Learn, dxgi-best-practices; copia en runtime\ref\).
bool g_windowNoActivate = false;
static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
        case WM_CLOSE: ImportLog::Get().Printf("UI: ventana cerrada por el usuario, se termina el proceso"); std::fflush(nullptr); ExitProcess(0); return 0;
        case WM_DESTROY: return 0;
    }
    return DefWindowProcW(h, m, w, l);
}
static DWORD WINAPI UiMain(void* p) {
    rt::perf::NameCurrentThread("d3d-ui");
    auto* self = static_cast<UiThread*>(p);
    WNDCLASSEXW wc = {sizeof(wc)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.lpszClassName = L"SwapForceRTWindow";
    RegisterClassExW(&wc);
    RECT r = {0, 0, (LONG)kW, (LONG)kH};
    DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    AdjustWindowRect(&r, style, FALSE);
    self->hwnd = CreateWindowExW(0, wc.lpszClassName, L"Skylanders Swap Force (PC port - runtime)", style, CW_USEDEFAULT, CW_USEDEFAULT,
                                 r.right - r.left, r.bottom - r.top, nullptr, nullptr, wc.hInstance, nullptr);
    // M8: --window-noactivate: ventana minimizada sin activar (no roba el foco); la cadena de intercambio y Present son las mismas
    if (self->hwnd) ShowWindow(self->hwnd, g_windowNoActivate ? SW_SHOWMINNOACTIVE : SW_SHOWNORMAL);
    SetEvent(self->ready);
    MSG msg;
    while (!self->stop && GetMessageW(&msg, nullptr, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    return 0;
}
bool UiThread::Start() {
    ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    thread = CreateThread(nullptr, 0, UiMain, this, 0, nullptr);
    WaitForSingleObject(ready, 10000);
    return hwnd != nullptr;
}
void UiThread::Stop() {
    stop = true;
    if (hwnd) PostMessageW(hwnd, WM_NULL, 0, 0);
    if (thread) WaitForSingleObject(thread, 1000);
}

bool D3D12Backend::Hr(HRESULT hr, const char* what) {
    if (SUCCEEDED(hr)) return true;
    ++hrFail; ++fHrFail;
    if (firstHrLogs++ < 30) ImportLog::Get().Printf("D3D12 HRESULT=0x%08X en %s", (unsigned)hr, what);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) CheckRemoved();
    return false;
}
void D3D12Backend::CheckRemoved() {
    if (!dev) return;
    HRESULT r = dev->GetDeviceRemovedReason();
    if (r != S_OK) { ++removed; if (removed < 5) ImportLog::Get().Printf("D3D12: dispositivo perdido, GetDeviceRemovedReason=0x%08X", (unsigned)r); }
}
void D3D12Backend::Note(const std::string& key) {
    ++notes[key];
    if (notesLogged.insert(key).second) ImportLog::Get().Printf("D3D12 %s", key.c_str());
}
void D3D12Backend::Note(const char* lit) {
    auto it = notesLit.find(lit);
    if (it != notesLit.end()) { ++it->second; return; }
    notesLit.emplace(lit, 1);
    if (notesLogged.insert(lit).second) ImportLog::Get().Printf("D3D12 %s", lit);
}

bool D3D12Backend::Init() {
    if (!Hr(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)), "CreateDXGIFactory2")) return false;
    ComPtr<IDXGIAdapter1> ad;
    if (opt.warp) {
        ComPtr<IDXGIAdapter> w;
        if (Hr(factory->EnumWarpAdapter(IID_PPV_ARGS(&w)), "EnumWarpAdapter")) w.As(&ad);
        usingWarp = true;
    } else {
        for (UINT i = 0; factory->EnumAdapters1(i, &ad) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 d; ad->GetDesc1(&d);
            if (d.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { ad.Reset(); continue; }
            if (SUCCEEDED(D3D12CreateDevice(ad.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) break;
            ad.Reset();
        }
        if (!ad) { ImportLog::Get().Printf("D3D12: no hay adaptador de hardware, se usa WARP"); ComPtr<IDXGIAdapter> w; if (Hr(factory->EnumWarpAdapter(IID_PPV_ARGS(&w)), "EnumWarpAdapter")) w.As(&ad); usingWarp = true; }
    }
    if (!ad) return false;
    DXGI_ADAPTER_DESC1 d; ad->GetDesc1(&d);
    char nm[256]; WideCharToMultiByte(CP_ACP, 0, d.Description, -1, nm, sizeof(nm), nullptr, nullptr);
    adapterName = nm;
    if (!Hr(D3D12CreateDevice(ad.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev)), "D3D12CreateDevice")) return false;
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS fo = {};
        if (SUCCEEDED(dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &fo, sizeof(fo)))) bindingTier = fo.ResourceBindingTier;
        D3D12_FEATURE_DATA_SHADER_MODEL sm = {D3D_SHADER_MODEL_6_0};
        HRESULT hrSm = dev->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &sm, sizeof(sm));
        ImportLog::Get().Printf("D3D12: ResourceBindingTier=%d ShaderModel max=0x%X (hr=0x%08X)", (int)bindingTier, (unsigned)sm.HighestShaderModel, (unsigned)hrSm);
    }
    D3D12_COMMAND_QUEUE_DESC qd = {};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (!Hr(dev->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue)), "CreateCommandQueue")) return false;
    if (!Hr(dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "CreateFence")) return false;
    fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    int a0 = AcquireAllocator();
    if (a0 < 0) return false;
    if (!Hr(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocPool[a0].a.Get(), nullptr, IID_PPV_ARGS(&cl)), "CreateCommandList")) return false;
    cl->Close();
    // descriptores
    auto heap = [&](ComPtr<ID3D12DescriptorHeap>& h, D3D12_DESCRIPTOR_HEAP_TYPE t, UINT n, bool visible, const char* what) {
        D3D12_DESCRIPTOR_HEAP_DESC hd = {t, n, visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0};
        return Hr(dev->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&h)), what);
    };
    if (!heap(rtvHeap, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128, false, "heap rtv")) return false;
    if (!heap(dsvHeap, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64, false, "heap dsv")) return false;
    if (!heap(cpuSrvHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 8192, false, "heap srv cpu")) return false;
    if (!heap(gpuSrvHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kCbvSrvHeapSize, true, "heap srv gpu")) return false;
    if (!heap(gpuSmpHeap, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, kSamplerHeapSize, true, "heap sampler gpu")) return false;
    rtvInc = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsvInc = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    srvInc = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    smpInc = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    // textura del fotograma
    D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; rd.Width = kW; rd.Height = kH; rd.DepthOrArraySize = 1; rd.MipLevels = 1;
    rd.Format = DXGI_FORMAT_R8G8B8A8_UNORM; rd.SampleDesc.Count = 1; rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;   // M60: UAV (presentacion por CS)
    D3D12_CLEAR_VALUE cv = {DXGI_FORMAT_R8G8B8A8_UNORM, {0, 0, 0, 1}};
    if (!Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_RENDER_TARGET, &cv, IID_PPV_ARGS(&frameRT)), "frameRT")) return false;
    frameRtvIdx = rtvUsed++;
    {
        D3D12_CPU_DESCRIPTOR_HANDLE h = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        h.ptr += (SIZE_T)frameRtvIdx * rtvInc;
        dev->CreateRenderTargetView(frameRT.Get(), nullptr, h);
    }
    // readback de fotograma
    D3D12_HEAP_PROPERTIES rp = {D3D12_HEAP_TYPE_READBACK};
    D3D12_RESOURCE_DESC bd = {};
    bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; bd.Width = (UINT64)kW * 4 * kH; bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
    bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    if (!Hr(dev->CreateCommittedResource(&rp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback)), "readback")) return false;
    // memoria compartida (512 MB) y anillo de subida
    D3D12_RESOURCE_DESC sd = bd;
    sd.Width = kSharedMemBytes;
    if (!Hr(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &sd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&sharedMem)), "memoria compartida 512 MB")) return false;
    sharedState = D3D12_RESOURCE_STATE_COPY_DEST;
    pageHash.assign(kSharedMemBytes / 4096, 0);
    D3D12_HEAP_PROPERTIES up = {D3D12_HEAP_TYPE_UPLOAD};
    D3D12_RESOURCE_DESC ud = bd;
    ud.Width = (UINT64)kFramesInFlight * kUploadSectionBytes;
    if (!Hr(dev->CreateCommittedResource(&up, D3D12_HEAP_FLAG_NONE, &ud, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuf)), "anillo de subida")) return false;
    D3D12_RANGE none = {0, 0};
    if (!Hr(uploadBuf->Map(0, &none, (void**)&uploadCpu), "Map(anillo)")) return false;
    // SRV nulos (2D, 3D, cubo) en el heap de CPU
    auto nullSrv = [&](D3D12_SRV_DIMENSION dim) {
        D3D12_SHADER_RESOURCE_VIEW_DESC sv = {};
        sv.Format = DXGI_FORMAT_R8G8B8A8_UNORM; sv.ViewDimension = dim; sv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        if (dim == D3D12_SRV_DIMENSION_TEXTURE2D) sv.Texture2D.MipLevels = 1;
        else if (dim == D3D12_SRV_DIMENSION_TEXTURE3D) sv.Texture3D.MipLevels = 1;
        else sv.TextureCube.MipLevels = 1;
        uint32_t idx = cpuSrvUsed++;
        D3D12_CPU_DESCRIPTOR_HANDLE h = cpuSrvHeap->GetCPUDescriptorHandleForHeapStart();
        h.ptr += (SIZE_T)idx * srvInc;
        dev->CreateShaderResourceView(nullptr, &sv, h);
        return idx;
    };
    nullSrv2D = nullSrv(D3D12_SRV_DIMENSION_TEXTURE2D);
    nullSrv3D = nullSrv(D3D12_SRV_DIMENSION_TEXTURE3D);
    nullSrvCube = nullSrv(D3D12_SRV_DIMENSION_TEXTURECUBE);
    if (!CreateRootSignature()) return false;
    if (!opt.headless) {
        if (!ui.Start()) { ImportLog::Get().Printf("D3D12: no se pudo crear la ventana"); return false; }
        DXGI_SWAP_CHAIN_DESC1 scd = {};
        scd.Width = kW; scd.Height = kH; scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM; scd.SampleDesc.Count = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; scd.BufferCount = kFramesInFlight; scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        ComPtr<IDXGISwapChain1> sc1;
        if (!Hr(factory->CreateSwapChainForHwnd(queue.Get(), ui.hwnd, &scd, nullptr, nullptr, &sc1), "CreateSwapChainForHwnd")) return false;
        factory->MakeWindowAssociation(ui.hwnd, DXGI_MWA_NO_ALT_ENTER);
        sc1.As(&swap);
        for (uint32_t i = 0; i < kFramesInFlight; ++i)
            if (!Hr(swap->GetBuffer(i, IID_PPV_ARGS(&backbuf[i])), "GetBuffer")) return false;
    }
    ImportLog::Get().Printf("D3D12: adaptador=\"%s\" warp=%d headless=%d ventana=%p memoria_compartida=512MB", adapterName.c_str(), (int)usingWarp, (int)opt.headless, (void*)ui.hwnd);
    return true;
}

// ---- sincronizacion ----
void D3D12Backend::WaitFence(uint64_t v) {
    if (fence->GetCompletedValue() >= v) return;
    PERF_ZONE(kZoneFenceWait);
    rt::gpu::M60Event('W');
    fence->SetEventOnCompletion(v, fenceEvent);
    WaitForSingleObject(fenceEvent, 20000);
    rt::gpu::M60Event('w');
}
// ---- M60: --gpu-timing ----
// Marcas de tiempo de la cola (EndQuery TIMESTAMP) al abrir cada lista, tras cada operacion marcada (TsMark) y al cerrarla.
// El tiempo entre dos marcas consecutivas de la misma lista se atribuye a la operacion de la segunda marca. Solo diagnostico.
bool g_gpuTiming = false;
static const char* kTsCatName[] = {"inicio", "draw", "resolve_color", "resolve_prof", "subida_mem", "subida_tex", "swap", "fin", "otro"};
void D3D12Backend::TsBegin() {
    if (!g_gpuTiming) return;
    if (!tsHeap) {
        D3D12_QUERY_HEAP_DESC qd = {D3D12_QUERY_HEAP_TYPE_TIMESTAMP, kTsSlots, 0};
        if (!Hr(dev->CreateQueryHeap(&qd, IID_PPV_ARGS(&tsHeap)), "CreateQueryHeap(timestamp)")) { g_gpuTiming = false; return; }
        D3D12_HEAP_PROPERTIES rp = {D3D12_HEAP_TYPE_READBACK};
        D3D12_RESOURCE_DESC bd = {};
        bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; bd.Width = (UINT64)kTsSlots * 8; bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
        bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        if (!Hr(dev->CreateCommittedResource(&rp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&tsReadback)), "readback timestamp")) { g_gpuTiming = false; return; }
        void* p = nullptr; D3D12_RANGE rr = {0, (SIZE_T)kTsSlots * 8};
        tsReadback->Map(0, &rr, &p); tsCpu = (const uint64_t*)p;
        queue->GetTimestampFrequency(&tsFreq);
        tsFence.assign(kTsSlots, ~0ull); tsCat.assign(kTsSlots, 0); tsTag.assign(kTsSlots, 0);
        QueryPerformanceCounter(&tsPrint);
    }
    if (tsNext - tsHarvest + 2 >= kTsSlots) return;   // sin hueco: esta lista no se mide
    tsListFirst = tsNext;
    tsOpen = true;
    TsMark(0, 0);
}
void D3D12Backend::TsMark(uint8_t cat, uint64_t tag) {
    if (!tsOpen) return;
    if (tsNext - tsHarvest + 2 >= kTsSlots) return;
    uint32_t i = (uint32_t)(tsNext % kTsSlots);
    cl->EndQuery(tsHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, i);
    tsCat[i] = cat; tsTag[i] = tag; tsFence[i] = ~0ull;
    ++tsNext;
}
void D3D12Backend::TsEnd() {
    if (!g_gpuTiming || !tsOpen) return;
    TsMark(7, 0);
    uint64_t q = tsListFirst;
    while (q < tsNext) {   // resolucion en tramos contiguos del anillo
        uint32_t i = (uint32_t)(q % kTsSlots);
        uint32_t n = (uint32_t)std::min<uint64_t>(tsNext - q, kTsSlots - i);
        cl->ResolveQueryData(tsHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, i, n, tsReadback.Get(), (UINT64)i * 8);
        q += n;
    }
}
void D3D12Backend::TsHarvest() {
    if (!g_gpuTiming || !tsCpu) return;
    uint64_t done = fence->GetCompletedValue();
    while (tsHarvest < tsNext) {
        uint32_t i = (uint32_t)(tsHarvest % kTsSlots);
        if (tsFence[i] == ~0ull || tsFence[i] > done) break;
        if (tsCat[i] != 0) {
            uint32_t j = (uint32_t)((tsHarvest - 1) % kTsSlots);
            uint64_t s = tsCpu[j], e = tsCpu[i];
            if (e >= s) {
                double d = (double)(e - s);
                tsCatTicks[tsCat[i] < 9 ? tsCat[i] : 8] += d;
                tsBusyTicks += d;
                if (tsCat[i] == 1) tsTagTicks[tsTag[i]] += d;
            }
            if (tsCat[i] == 7) ++tsLists;
        }
        ++tsHarvest;
    }
    LARGE_INTEGER now, f; QueryPerformanceCounter(&now); QueryPerformanceFrequency(&f);
    double el = (double)(now.QuadPart - tsPrint.QuadPart) / (double)f.QuadPart;
    if (el >= 1.0) {
        const double k = 1000.0 / (double)tsFreq;
        char b[600]; int n = std::snprintf(b, sizeof(b), "M60 GPU ocupada_ms=%.1f en %.2f s listas=%llu lectura_MB=%.1f FlushAndWait=%llu |", tsBusyTicks * k, el, (unsigned long long)tsLists, tsReadbackBytes / 1048576.0, (unsigned long long)tsFlushWaits);
        tsReadbackBytes = 0; tsFlushWaits = 0;
        for (int c = 1; c < 9; ++c) if (tsCatTicks[c] > 0 && n < (int)sizeof(b) - 40) n += std::snprintf(b + n, sizeof(b) - n, " %s=%.1f", kTsCatName[c], tsCatTicks[c] * k);
        ImportLog::Get().Printf("%s", b);
        std::vector<std::pair<double, uint64_t>> top;
        for (auto& kv : tsTagTicks) top.push_back({kv.second, kv.first});
        std::sort(top.begin(), top.end(), [](auto& x, auto& y) { return x.first > y.first; });
        for (size_t t = 0; t < top.size() && t < 6; ++t)
            ImportLog::Get().Printf("M60 GPU draw ps/vs=%016llX ms=%.2f", (unsigned long long)top[t].second, top[t].first * k);
        {
            std::vector<std::pair<uint64_t, uint64_t>> rb;
            for (auto& kv : tsResolveBytes) rb.push_back({kv.second.first, kv.first});
            std::sort(rb.begin(), rb.end(), [](auto& x, auto& y) { return x.first > y.first; });
            for (size_t t = 0; t < rb.size() && t < 10; ++t) {
                uint64_t key = rb[t].second;
                ImportLog::Get().Printf("M60 GPU lectura destino=%08X %ux%u %s MB=%.1f n=%llu", (uint32_t)(key >> 32), (uint32_t)((key >> 16) & 0x7FFF), (uint32_t)(key & 0xFFFF),
                                        (key & 0x80000000u) ? "prof" : "color", rb[t].first / 1048576.0, (unsigned long long)tsResolveBytes[key].second);
            }
            tsResolveBytes.clear();
            for (auto& kv : tsTexFlush)
                ImportLog::Get().Printf("M60 vaciado por textura base=%08X fmt=%s %ux%u n=%llu", (uint32_t)(kv.first >> 32), TexFmtName((uint32_t)((kv.first >> 26) & 63)),
                                        (uint32_t)((kv.first >> 13) & 0x1FFF), (uint32_t)(kv.first & 0x1FFF), (unsigned long long)kv.second);
            tsTexFlush.clear();
            static const char* rn[16] = {"otro", "INTERRUPT", "XE_SWAP", "WAIT_REG_MEM", "REG_TO_MEM", "MEM_WRITE", "COND_WRITE", "EVENT_WRITE_SHD", "EVENT_WRITE_EXT", "fin_lote", "textura", "memoria", "bufer_lleno", "OnSwap", "presentacion", "WAIT_MEM"};
            char sb[700]; int sn = std::snprintf(sb, sizeof(sb), "M60 vaciados con resolves pendientes:");
            for (int r = 0; r < 16; ++r) if (tsSyncN[r] && sn < (int)sizeof(sb) - 60) sn += std::snprintf(sb + sn, sizeof(sb) - sn, " %s=%llu/%.1fms", rn[r], (unsigned long long)tsSyncN[r], tsSyncMs[r]);
            if (sn < (int)sizeof(sb) - 40) sn += std::snprintf(sb + sn, sizeof(sb) - sn, " (hasta_fin_GPU=%.1fms)", tsSyncGpuMs);
            {
                LARGE_INTEGER qf; QueryPerformanceFrequency(&qf);
                uint64_t bt = wrBusyTicks.exchange(0), c = wrCount.exchange(0), by = wrBytes.exchange(0);
                if (sn < (int)sizeof(sb) - 80) sn += std::snprintf(sb + sn, sizeof(sb) - sn, " escritor: %.1f ms, %llu resolves, %.1f MB", bt * 1000.0 / (double)qf.QuadPart, (unsigned long long)c, by / 1048576.0);
            }
            ImportLog::Get().Printf("%s", sb);
            for (int r = 0; r < 16; ++r) { tsSyncN[r] = 0; tsSyncMs[r] = 0; }
            tsSyncGpuMs = 0;
        }
        tsBusyTicks = 0; tsLists = 0; tsPrint = now; tsTagTicks.clear();
        for (double& c : tsCatTicks) c = 0;
    }
}

// ---- M60: presentacion desde la copia GPU del resolve del frontbuffer ----
bool g_strictResolveSync = false;   // M60: --strict-resolve-sync (vaciado de resolves tambien en XE_SWAP, INTERRUPT, WAIT_REG_MEM y fin de lote, como M8)
bool g_presentVerify = false;       // M60: --present-verify
uint32_t g_dumpEveryMs = 0, g_dumpFromS = 0, g_dumpForS = 30;   // M60: --dump-every-ms N --dump-from-s T [--dump-for-s D]
static const char* kBlitCsHlsl = R"(
cbuffer C : register(b0) { uint w; uint h; };
Texture2D<float4> src : register(t0);
RWTexture2D<unorm float4> dst : register(u0);
[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    if (id.x >= w || id.y >= h) return;
    dst[id.xy] = src.Load(int3(id.xy, 0));
}
)";
bool D3D12Backend::EnsureBlitCs() {
    if (csBlitPso) return true;
    if (csBlitTried) return false;
    csBlitTried = true;
    D3D12_DESCRIPTOR_RANGE r[2] = {};
    r[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; r[0].NumDescriptors = 1;
    r[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; r[1].NumDescriptors = 1;
    D3D12_ROOT_PARAMETER p[3] = {};
    p[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS; p[0].Constants = {0, 0, 2}; p[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    p[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; p[1].DescriptorTable = {1, &r[0]}; p[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    p[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; p[2].DescriptorTable = {1, &r[1]}; p[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    D3D12_ROOT_SIGNATURE_DESC rs = {}; rs.NumParameters = 3; rs.pParameters = p;
    ComPtr<ID3DBlob> blob, err;
    if (FAILED(D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err))) return false;
    if (!Hr(dev->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&csBlitRootSig)), "CreateRootSignature(blit)")) return false;
    std::string dxil, e;
    if (!CompileHlslProfile(kBlitCsHlsl, L"cs_6_0", dxil, e)) { ImportLog::Get().Printf("M60 CS de presentacion: compilacion fallida: %s", e.c_str()); return false; }
    D3D12_COMPUTE_PIPELINE_STATE_DESC d = {}; d.pRootSignature = csBlitRootSig.Get(); d.CS = {dxil.data(), dxil.size()};
    if (!Hr(dev->CreateComputePipelineState(&d, IID_PPV_ARGS(&csBlitPso)), "CreateComputePipelineState(blit)")) { csBlitPso.Reset(); return false; }
    ImportLog::Get().Printf("M60 CS de presentacion listo (DXIL %zu bytes)", dxil.size());
    return true;
}
// Mismo resultado que DecodeFrontbuffer (k_8_8_8_8 en mosaico): el SRV del atajo aplica el endian (coincidente con el del resolve)
// y el swizzle de la constante de fetch del frontbuffer; la textura del atajo es igual bit a bit a la destilada (--verify-shortcut).
bool D3D12Backend::PresentFromResolvedTex(const backend::SwapInfo& s) {
    xenos::xe_gpu_texture_fetch_t t;
    std::memcpy(&t, s.fetch, sizeof(t));
    if ((uint32_t)t.format != 6 || !t.tiled || !s.w || !s.h) return false;
    uint32_t pitchPx = (uint32_t)t.pitch * 32;
    if (pitchPx < s.w) pitchPx = (s.w + 31) & ~31u;
    uint32_t pitchAligned = (pitchPx + 31) & ~31u;
    HostTexture* ht = LookupResolvedTex(s.fbPhys, s.w, s.h, 6, (uint32_t)t.endianness, pitchAligned, (uint32_t)t.swizzle);
    if (!ht || !EnsureBlitCs()) return false;
    uint32_t base = AllocGpuSrv(2);
    D3D12_CPU_DESCRIPTOR_HANDLE cpu0 = gpuSrvHeap->GetCPUDescriptorHandleForHeapStart(); cpu0.ptr += (SIZE_T)base * srvInc;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu1 = cpu0; cpu1.ptr += srvInc;
    D3D12_GPU_DESCRIPTOR_HANDLE g0 = gpuSrvHeap->GetGPUDescriptorHandleForHeapStart(); g0.ptr += (UINT64)base * srvInc;
    D3D12_GPU_DESCRIPTOR_HANDLE g1 = g0; g1.ptr += srvInc;
    D3D12_CPU_DESCRIPTOR_HANDLE srcSrv = cpuSrvHeap->GetCPUDescriptorHandleForHeapStart(); srcSrv.ptr += (SIZE_T)ht->cpuSrv * srvInc;
    dev->CopyDescriptorsSimple(1, cpu0, srcSrv, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_UNORDERED_ACCESS_VIEW_DESC ud = {}; ud.Format = DXGI_FORMAT_R8G8B8A8_UNORM; ud.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    dev->CreateUnorderedAccessView(frameRT.Get(), nullptr, &ud, cpu1);
    SetFrameRTState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    uint32_t c[2] = {std::min(s.w, kW), std::min(s.h, kH)};
    cl->SetComputeRootSignature(csBlitRootSig.Get());
    cl->SetPipelineState(csBlitPso.Get());
    cl->SetComputeRoot32BitConstants(0, 2, c, 0);
    cl->SetComputeRootDescriptorTable(1, g0);
    cl->SetComputeRootDescriptorTable(2, g1);
    cl->Dispatch((c[0] + 7) / 8, (c[1] + 7) / 8, 1);
    return true;
}
void D3D12Backend::ViewerHarvest() {
    if (!viewerPending || !viewerFence || fence->GetCompletedValue() < viewerFence) return;
    void* p = nullptr; D3D12_RANGE rr = {0, (SIZE_T)kW * 4 * kH};
    if (Hr(viewerReadback->Map(0, &rr, &p), "Map(visor)")) {
        viewer::OnFrame((const uint8_t*)p, kW * 4, viewerW, viewerH, viewerFrame);
        D3D12_RANGE wz = {0, 0}; viewerReadback->Unmap(0, &wz);
    }
    viewerPending = false;
}

uint64_t D3D12Backend::Submit() {
    ID3D12CommandList* l[] = {cl.Get()};
    queue->ExecuteCommandLists(1, l);
    queue->Signal(fence.Get(), ++fenceValue);
    if (tsOpen) { for (uint64_t q = tsListFirst; q < tsNext; ++q) tsFence[q % kTsSlots] = fenceValue; tsOpen = false; }   // M60
    if (curAlloc >= 0) allocPool[curAlloc].fence = fenceValue;
    drawsSinceKick = 0;
    return fenceValue;
}
int D3D12Backend::AcquireAllocator() {
    uint64_t done = fence ? fence->GetCompletedValue() : 0;
    for (size_t i = 0; i < allocPool.size(); ++i)
        if ((int)i != curAlloc && allocPool[i].fence <= done) return (int)i;
    AllocSlot s;
    if (!Hr(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&s.a)), "CreateCommandAllocator")) return -1;
    allocPool.push_back(s);
    return (int)allocPool.size() - 1;
}
// newSection=true: empieza una seccion nueva del anillo de subida (espera a que la GPU haya terminado de usarla).
// newSection=false (Kick): continua en la misma seccion; solo cambia el asignador.
void D3D12Backend::BeginList(bool newSection) {
    if (clOpen) return;
    if (newSection) WaitFence(allocFence[frameSlot]);
    int a = AcquireAllocator();
    if (a < 0) return;
    curAlloc = a;
    Hr(allocPool[a].a->Reset(), "CommandAllocator::Reset");
    Hr(cl->Reset(allocPool[a].a.Get(), nullptr), "CommandList::Reset");
    clOpen = true;
    if (newSection) { uploadOffset = 0; uploadBytesFrame = 0; }
    ID3D12DescriptorHeap* heaps[] = {gpuSrvHeap.Get(), gpuSmpHeap.Get()};
    cl->SetDescriptorHeaps(2, heaps);
    TsBegin();   // M60
}
unsigned g_kickDraws = 64;   // M8: --kick-draws N (0 = sin envios parciales)
unsigned g_kickResolveDraws = 32;   // M8: --kick-resolve-draws N (envio tras un resolve si hay >= N draws sin enviar)
void D3D12Backend::Kick() {
    if (!clOpen) return;
    if (g_eagerIssue && g_asyncResolveWriter && resolveStage) { IssueCopies(true); wrCv.notify_all(); }   // M60: el escritor avanza durante el fotograma
    TsEnd();   // M60
    Hr(cl->Close(), "CommandList::Close (kick)");
    clOpen = false;
    uint64_t fv = Submit();
    allocFence[frameSlot] = fv;
    ++kicks;
    BeginList(false);
}
void D3D12Backend::FlushAndWait() {
    if (!clOpen) return;
    ++tsFlushWaits;   // M60
    rt::gpu::M60Event('K');
    TsEnd();   // M60
    Hr(cl->Close(), "CommandList::Close (flush)");
    clOpen = false;
    uint64_t fv = Submit();
    allocFence[frameSlot] = fv;
    WaitFence(fv);
    rt::gpu::M60Event('k');
    gpuSrvNext = 0;   // toda la GPU esta ociosa: se pueden reutilizar los descriptores
    gpuSmpNext = 0; samplerTables.clear(); samplerFast.clear();
    BeginList();
}
void D3D12Backend::Transition(ID3D12Resource* r, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
    if (from == to) return;
    D3D12_RESOURCE_BARRIER b = {};
    b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    b.Transition.pResource = r; b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    b.Transition.StateBefore = from; b.Transition.StateAfter = to;
    cl->ResourceBarrier(1, &b);
}
void D3D12Backend::SetFrameRTState(D3D12_RESOURCE_STATES s) { Transition(frameRT.Get(), frameRTState, s); frameRTState = s; }
void D3D12Backend::SetSharedState(D3D12_RESOURCE_STATES s) { Transition(sharedMem.Get(), sharedState, s); sharedState = s; }

UploadAlloc D3D12Backend::Upload(uint32_t bytes, uint32_t align) {
    UploadAlloc u;
    if (bytes > kUploadSectionBytes) return u;
    uint32_t aligned = (uploadOffset + align - 1) & ~(align - 1);
    if ((uint64_t)aligned + bytes > kUploadSectionBytes) { FlushAndWait(); aligned = 0; }
    uploadOffset = aligned + bytes;
    uploadBytesFrame += bytes; uploadBytesTotal += bytes;
    u.offset = (uint64_t)frameSlot * kUploadSectionBytes + aligned;
    u.cpu = uploadCpu + u.offset;
    u.gpu = uploadBuf->GetGPUVirtualAddress() + u.offset;
    return u;
}

// Sube a la memoria compartida las paginas de [phys, phys+bytes) cuyo contenido cambio desde la ultima subida.
void D3D12Backend::EnsureMemory(uint32_t phys, uint32_t bytes) {
    if (!bytes) return;
    PERF_ZONE(kZoneMemUpload);
    phys &= 0x1FFFFFFF;
    uint64_t end = (uint64_t)phys + bytes;
    if (end > kSharedMemBytes) end = kSharedMemBytes;
    uint32_t p0 = phys >> 12, p1 = (uint32_t)((end + 4095) >> 12);
    rt::gpu::g_syncReason = 11;
    FlushResolvesIfOverlap(p0 << 12, (uint32_t)std::min<uint64_t>((uint64_t)p1 << 12, kSharedMemBytes));   // M8: exactitud con resolves diferidos
    uint32_t runStart = 0; bool inRun = false;
    const uint8_t* whole = PhysHost(p0 << 12, (p1 - p0) * 4096);   // M8: una sola comprobacion para todo el rango
    auto flush = [&](uint32_t from, uint32_t to) {   // paginas [from, to)
        uint32_t n = (to - from) * 4096;
        const uint8_t* src = PhysHost(from << 12, n);
        if (!src) return;
        UploadAlloc u = Upload(n, 512);
        if (!u.cpu) { Note("OMISION memoria: rango de subida mayor que la seccion del anillo"); return; }
        std::memcpy(u.cpu, src, n);
        SetSharedState(D3D12_RESOURCE_STATE_COPY_DEST);
        cl->CopyBufferRegion(sharedMem.Get(), (UINT64)from << 12, uploadBuf.Get(), u.offset, n);
        TsMark(4, 0);   // M60
        pagesUploaded += to - from;
    };
    for (uint32_t pg = p0; pg < p1; ++pg) {
        const uint8_t* src = whole ? whole + ((size_t)(pg - p0) << 12) : PhysHost(pg << 12, 4096);
        uint32_t h = 0;
        if (src) {
            uint64_t x = FastHash(src, 4096);   // M8: hash vectorial (antes FNV de 64 bits en cadena)
            h = (uint32_t)(x ^ (x >> 32)) | 1u;
        }
        bool changed = src && pageHash[pg] != h;
        if (changed) { pageHash[pg] = h; if (!inRun) { inRun = true; runStart = pg; } }
        else {
            if (!changed && src) ++pagesSkipped;
            if (inRun) { flush(runStart, pg); inRun = false; }
        }
    }
    if (inRun) flush(runStart, p1);
}

bool D3D12Backend::PresentFromRgba(const uint8_t* rgba, uint32_t w, uint32_t h) {
    uint32_t cw = std::min(w, kW), ch = std::min(h, kH);
    uint32_t pitch = (cw * 4 + 255) & ~255u;
    UploadAlloc u = Upload(pitch * ch, 512);
    if (!u.cpu) return false;
    for (uint32_t y = 0; y < ch; ++y) std::memcpy(u.cpu + (size_t)y * pitch, rgba + (size_t)y * w * 4, cw * 4);
    SetFrameRTState(D3D12_RESOURCE_STATE_COPY_DEST);
    D3D12_TEXTURE_COPY_LOCATION dst = {}, src = {};
    dst.pResource = frameRT.Get(); dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; dst.SubresourceIndex = 0;
    src.pResource = uploadBuf.Get(); src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint.Offset = u.offset;
    src.PlacedFootprint.Footprint = {DXGI_FORMAT_R8G8B8A8_UNORM, cw, ch, 1, pitch};
    cl->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
    return true;
}

void D3D12Backend::OnDraw(const DrawEvent& ev, const ShaderInfo* vs, const ShaderInfo* ps) {
    PERF_ZONE(kZoneDraw);
    BeginList();
    ++drawSerial;
    if (DoDraw(ev, vs, ps)) { ++drawsOk; ++fDrawsOk; }
    else { ++drawsSkip; ++fDrawsSkip; }
    TsMark(1, ps ? ps->hash : (vs ? vs->hash : 0));   // M60
    if (g_kickDraws && ++drawsSinceKick >= g_kickDraws) Kick();   // M8: la GPU empieza a trabajar mientras el CP sigue
}

void D3D12Backend::OnSwap(const backend::SwapInfo& s) {
    PERF_ZONE(kZoneSwapTotal);
    FinishDepthQueries(s.frame);
    ViewerHarvest();   // M60: imagen pedida a la GPU en un fotograma anterior
    rt::gpu::g_syncReason = 13;
    if (g_strictResolveSync || !g_asyncResolveWriter) FlushResolves();   // M8 (ya vaciado en XE_SWAP); M60: solo con --strict-resolve-sync
    rt::gpu::g_syncReason = 0;
    BeginList();
    bool gpuPresented = false;
    if (!opt.clearTest && !g_strictResolveSync && g_asyncResolveWriter) {   // M60
        gpuPresented = PresentFromResolvedTex(s);
        if (gpuPresented) ++presentGpu;
    }
    if (g_presentVerify && gpuPresented) {   // M60: compara la imagen de la GPU con DecodeFrontbuffer tras escribir los resolves
        rt::gpu::g_syncReason = 14; FlushResolves();
        SetFrameRTState(D3D12_RESOURCE_STATE_COPY_SOURCE);
        D3D12_TEXTURE_COPY_LOCATION dst = {}, src = {};
        dst.pResource = readback.Get(); dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dst.PlacedFootprint.Footprint = {DXGI_FORMAT_R8G8B8A8_UNORM, kW, kH, 1, kW * 4};
        src.pResource = frameRT.Get(); src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; src.SubresourceIndex = 0;
        cl->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        FlushAndWait();
        FbDesc d; for (int i = 0; i < 6; ++i) d.fetch[i] = s.fetch[i];
        d.phys = s.fbPhys; d.w = s.w; d.h = s.h;
        std::string info;
        if (DecodeFrontbuffer(d, fbScratch, &info)) {
            void* p = nullptr; D3D12_RANGE rr = {0, (SIZE_T)kW * 4 * kH};
            if (Hr(readback->Map(0, &rr, &p), "Map(present-verify)")) {
                uint32_t cw = std::min(s.w, kW), ch = std::min(s.h, kH);
                uint64_t diff = 0;
                for (uint32_t y = 0; y < ch; ++y)
                    for (uint32_t x = 0; x < cw; ++x)
                        diff += std::memcmp((const uint8_t*)p + (size_t)y * kW * 4 + x * 4, fbScratch.data() + ((size_t)y * s.w + x) * 4, 4) != 0;
                D3D12_RANGE wz = {0, 0}; readback->Unmap(0, &wz);
                static uint64_t vOk = 0, vBad = 0;
                ++(diff ? vBad : vOk);
                if (diff || vOk <= 3 || vOk % 200 == 0)
                    ImportLog::Get().Printf("M60 present-verify frame=%llu %ux%u -> %s (pixeles distintos=%llu; ok=%llu mal=%llu)", (unsigned long long)s.frame, cw, ch,
                                            diff ? "DISTINTO" : "IGUAL", (unsigned long long)diff, (unsigned long long)vOk, (unsigned long long)vBad);
            }
        }
    }
    for (auto& kv : resolvedTex) kv.second.valid = false;   // M8: el atajo solo vale dentro del fotograma del resolve
    predPrev.swap(predCur); predCur.clear();   // M60
    // ---- imagen del fotograma ----
    const char* source = "nada";
    if (opt.clearTest) {
        SetFrameRTState(D3D12_RESOURCE_STATE_RENDER_TARGET);
        const float color[4] = {0.2f, 0.4f, 0.6f, 1.0f};   // bytes RGBA = 51,102,153,255
        D3D12_CPU_DESCRIPTOR_HANDLE h = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        h.ptr += (SIZE_T)frameRtvIdx * rtvInc;
        cl->ClearRenderTargetView(h, color, 0, nullptr);
        source = "clear-test";
    } else {
        const ResolveImage* ri = nullptr;
        for (auto& r : resolveImages)
            if (r.destBase == s.fbPhys && r.w >= s.w && r.h >= s.h && !r.rgba.empty()) ri = &r;
        std::vector<uint8_t>& rgba = fbScratch;   // M8: reutilizado (antes 3,6 MB nuevos por fotograma)
        if (gpuPresented) {   // M60
            source = "resolve-gpu";
            if (viewer::WantsFrame() && !viewerPending) {   // copia asincrona para el visor (se entrega en un OnSwap posterior)
                if (!viewerReadback) {
                    D3D12_HEAP_PROPERTIES rp = {D3D12_HEAP_TYPE_READBACK};
                    D3D12_RESOURCE_DESC bd = {};
                    bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; bd.Width = (UINT64)kW * 4 * kH; bd.Height = 1; bd.DepthOrArraySize = 1; bd.MipLevels = 1;
                    bd.SampleDesc.Count = 1; bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                    Hr(dev->CreateCommittedResource(&rp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&viewerReadback)), "readback del visor");
                }
                if (viewerReadback) {
                    SetFrameRTState(D3D12_RESOURCE_STATE_COPY_SOURCE);
                    D3D12_TEXTURE_COPY_LOCATION dst = {}, src = {};
                    dst.pResource = viewerReadback.Get(); dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                    dst.PlacedFootprint.Footprint = {DXGI_FORMAT_R8G8B8A8_UNORM, kW, kH, 1, kW * 4};
                    src.pResource = frameRT.Get(); src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; src.SubresourceIndex = 0;
                    cl->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
                    viewerPending = true; viewerFence = 0; viewerFrame = s.frame;
                    viewerW = std::min(s.w, kW); viewerH = std::min(s.h, kH);
                }
            }
        } else if (ri && ri->x0 == 0 && ri->y0 == 0 && ri->w == s.w) {
            source = "resolve";
            viewer::OnFrame(ri->rgba.data(), ri->w * 4, std::min(ri->w, kW), std::min(std::min(ri->h, s.h), kH), s.frame);
            PresentFromRgba(ri->rgba.data(), ri->w, std::min(ri->h, s.h));
        } else {
            FbDesc d;
            for (int i = 0; i < 6; ++i) d.fetch[i] = s.fetch[i];
            d.phys = s.fbPhys; d.w = s.w; d.h = s.h;
            std::string info;
            bool okfb;
            if (!g_strictResolveSync) { rt::gpu::g_syncReason = 14; FlushResolves(); ++presentCpu; }   // M60: lectura del frontbuffer en memoria guest
            { PERF_ZONE(kZoneFbDecode); okfb = DecodeFrontbuffer(d, rgba, &info); }
            if (okfb) { source = "frontbuffer"; viewer::OnFrame(rgba.data(), s.w * 4, std::min(s.w, kW), std::min(s.h, kH), s.frame); PresentFromRgba(rgba.data(), s.w, s.h); }
            else Note("OMISION presentacion: no se pudo destilar el frontbuffer (" + info + ")");
        }
    }
    TsMark(6, 0);   // M60
    bool dump = dumpSet.count((uint32_t)s.frame) != 0;
    if (g_dumpEveryMs) {   // M60: volcado por reloj (--dump-every-ms N --dump-from-s T), para comparar animaciones entre ritmos
        static LARGE_INTEGER t0{}, nextT{}; LARGE_INTEGER f, now; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&now);
        if (!t0.QuadPart) { t0 = now; nextT.QuadPart = now.QuadPart + (LONGLONG)g_dumpFromS * f.QuadPart; }
        if (now.QuadPart >= nextT.QuadPart && now.QuadPart < t0.QuadPart + (LONGLONG)(g_dumpFromS + g_dumpForS) * f.QuadPart) {
            dump = true; nextT.QuadPart = now.QuadPart + (LONGLONG)g_dumpEveryMs * f.QuadPart / 1000;
        }
    }
    bool wantStats = opt.stats || dump;
    UINT bbIdx = 0;
    if (swap) {
        bbIdx = swap->GetCurrentBackBufferIndex();
        SetFrameRTState(D3D12_RESOURCE_STATE_COPY_SOURCE);
        Transition(backbuf[bbIdx].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
        cl->CopyResource(backbuf[bbIdx].Get(), frameRT.Get());
        Transition(backbuf[bbIdx].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
    }
    if (wantStats) {
        SetFrameRTState(D3D12_RESOURCE_STATE_COPY_SOURCE);
        D3D12_TEXTURE_COPY_LOCATION dst = {}, src = {};
        dst.pResource = readback.Get(); dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dst.PlacedFootprint.Footprint = {DXGI_FORMAT_R8G8B8A8_UNORM, kW, kH, 1, kW * 4};
        src.pResource = frameRT.Get(); src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; src.SubresourceIndex = 0;
        cl->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
    }
    SetFrameRTState(D3D12_RESOURCE_STATE_RENDER_TARGET);
    TsEnd();   // M60
    Hr(cl->Close(), "CommandList::Close");
    clOpen = false;
    uint64_t fv = Submit();
    allocFence[frameSlot] = fv;
    if (viewerPending && !viewerFence) viewerFence = fv;   // M60
    frameSlot = (frameSlot + 1) % kFramesInFlight;
    if (swap) { PERF_ZONE(kZonePresent); Hr(swap->Present(1, 0), "Present"); ++presentCalls; }
    ++framesPresented;
    CheckRemoved();
    long long nonblack = -1; unsigned long long hash = 0;
    if (wantStats) {
        WaitFence(fv);
        void* p = nullptr;
        D3D12_RANGE rr = {0, (SIZE_T)kW * 4 * kH};
        if (Hr(readback->Map(0, &rr, &p), "Map(readback)")) {
            ImageStats st = ComputeStats(kW, kH, (const uint8_t*)p, kW * 4);
            nonblack = (long long)st.nonblack; hash = st.hash;
            if (dump) {
                char name[64];
                std::snprintf(name, sizeof(name), "\\d3d_%05llu.png", (unsigned long long)s.frame);
                std::string path = rt::gpu::FramesDir() + name;
                bool ok = WritePngRGBA(path, kW, kH, (const uint8_t*)p, kW * 4);
                ImportLog::Get().Printf("D3D dump frame=%llu -> %s %s fuente=%s nonblack_px=%lld min/max RGB=%u-%u,%u-%u,%u-%u", (unsigned long long)s.frame, path.c_str(),
                                        ok ? "OK" : "FALLO", source, nonblack, st.minR, st.maxR, st.minG, st.maxG, st.minB, st.maxB);
            }
            D3D12_RANGE wr = {0, 0};
            readback->Unmap(0, &wr);
        }
    }
    char b[360];
    std::snprintf(b, sizeof(b), "D3D frame=%llu draws_ok=%llu draws_skip=%llu shader_fail=%llu pso_fail=%llu resolve=%llu tex_fail=%llu nonblack_px=%lld hash=%016llX hr_fail=%llu removed=%llu fuente=%s",
                  (unsigned long long)s.frame, (unsigned long long)fDrawsOk, (unsigned long long)fDrawsSkip, (unsigned long long)fShaderFail,
                  (unsigned long long)fPsoFail, (unsigned long long)fResolve, (unsigned long long)fTexFail, nonblack, hash, (unsigned long long)fHrFail,
                  (unsigned long long)removed, source);
    GpuLog(b);
    if (perf::g_enabled) {
        static uint64_t lastUp = 0, lastHit = 0, lastMiss = 0, lastSc = 0;
        std::snprintf(b, sizeof(b), "M8 frame=%llu resolves=%zu tex_calls=%llu tex_solapa_resolve=%llu tex_exacta=%llu subidas=%llu memo_hit=%llu memo_miss=%llu atajo=%llu", (unsigned long long)s.frame, frameResolves.size(),
                      (unsigned long long)fTexCalls, (unsigned long long)fTexOverlap, (unsigned long long)fTexExact, (unsigned long long)(texUploads - lastUp),
                      (unsigned long long)(texMemoHits - lastHit), (unsigned long long)(texMemoMiss - lastMiss), (unsigned long long)(resolveShortcutHits - lastSc));
        lastUp = texUploads; lastHit = texMemoHits; lastMiss = texMemoMiss; lastSc = resolveShortcutHits;
        GpuLog(b);
    }
    texMemo.clear();
    frameResolves.clear(); fTexCalls = fTexOverlap = fTexExact = 0;
    fDrawsOk = fDrawsSkip = fShaderFail = fPsoFail = fResolve = fHrFail = fTexFail = 0;
    TsHarvest();   // M60
}

void D3D12Backend::Shutdown() {
    if (!dev) return;
    rt::gpu::g_syncReason = 0;
    FlushResolves();
    ImportLog::Get().Printf("M60 presentacion: desde_GPU=%llu desde_memoria=%llu escrituras_ordenadas=%llu", (unsigned long long)presentGpu, (unsigned long long)presentCpu, (unsigned long long)orderedWrites);
    if (wrThread.joinable()) { { std::lock_guard<std::mutex> g(wrMu); wrStop = true; } wrCv.notify_all(); wrThread.join(); }
    ImportLog::Get().Printf("M8 envios parciales (kick)=%llu asignadores=%zu", (unsigned long long)kicks, allocPool.size());
    ImportLog::Get().Printf("M8 resolves: escritos=%llu omitidos_por_cubiertos=%llu vaciados_por_lectura=%llu atajo_gpu=%llu subcopias=%llu",
                            (unsigned long long)resolveWritten.load(), (unsigned long long)resolveDeadSkipped.load(), (unsigned long long)resolveFlushByRead,
                            (unsigned long long)resolveShortcutHits, (unsigned long long)resolveSubCopies);
    if (fence) WaitFence(fenceValue);
    ImportLog::Get().Printf("D3D12 resumen: frames=%llu present=%llu draws_ok=%llu draws_skip=%llu shader_fail=%llu pso_fail=%llu resolve=%llu tex_fail=%llu hr_fail=%llu removed=%llu",
                            (unsigned long long)framesPresented, (unsigned long long)presentCalls, (unsigned long long)drawsOk, (unsigned long long)drawsSkip,
                            (unsigned long long)shaderFail, (unsigned long long)psoFail, (unsigned long long)resolves, (unsigned long long)texFail,
                            (unsigned long long)hrFail, (unsigned long long)removed);
    ImportLog::Get().Printf("D3D12 memoria: paginas_subidas=%llu paginas_sin_cambios=%llu bytes_subidos=%llu texturas_creadas=%llu texturas_reutilizadas=%llu psos=%zu dxil=%zu rt_color=%zu rt_profundidad=%zu",
                            (unsigned long long)pagesUploaded, (unsigned long long)pagesSkipped, (unsigned long long)uploadBytesTotal, (unsigned long long)texCreated,
                            (unsigned long long)texHits, psoCache.size(), dxilCache.size(), colorRTs.size(), depthRTs.size());
    for (auto& kv : notesLit) notes[kv.first] += kv.second;   // M60
    for (auto& kv : notes) ImportLog::Get().Printf("D3D12 NOTA n=%llu %s", (unsigned long long)kv.second, kv.first.c_str());
    backend::Set(nullptr);
    ui.Stop();
}

static D3D12Backend* g_backend = nullptr;

// ---- M8: grupo de hilos para repartir filas (escrituras disjuntas en memoria guest) ----
namespace {
struct RowPool {
    std::vector<std::thread> th;
    std::mutex mu;
    std::condition_variable cv, done;
    const std::function<bool(uint32_t, uint32_t)>* fn = nullptr;
    uint32_t rows = 0, chunk = 0, nChunks = 0;
    std::atomic<uint32_t> next{0}, finished{0};
    std::atomic<bool> fail{false};
    uint64_t gen = 0;
    bool stop = false;
    void Work() {
        for (;;) {
            uint32_t c = next.fetch_add(1);
            if (c >= nChunks) return;
            uint32_t b = c * chunk, e = std::min(rows, b + chunk);
            if (!(*fn)(b, e)) fail = true;
            if (finished.fetch_add(1) + 1 == nChunks) { std::lock_guard<std::mutex> g(mu); done.notify_all(); }
        }
    }
    void Start(unsigned n) {
        for (unsigned i = 0; i < n; ++i)
            th.emplace_back([this, i] {
                char nm[32]; std::snprintf(nm, sizeof(nm), "gpu-rows-%u", i); rt::perf::NameCurrentThread(nm);
                uint64_t seen = 0;
                for (;;) {
                    { std::unique_lock<std::mutex> g(mu); cv.wait(g, [&] { return stop || gen != seen; }); if (stop) return; seen = gen; }
                    Work();
                }
            });
    }
    bool Run(uint32_t r, const std::function<bool(uint32_t, uint32_t)>& f) {
        chunk = 16; rows = r; nChunks = (r + chunk - 1) / chunk; fn = &f;
        next = 0; finished = 0; fail = false;
        { std::lock_guard<std::mutex> g(mu); ++gen; }
        cv.notify_all();
        Work();
        std::unique_lock<std::mutex> g(mu);
        done.wait(g, [&] { return finished.load() >= nChunks; });
        return !fail;
    }
};
RowPool* g_rowPool = nullptr;
} // namespace

unsigned g_rowThreads = 6;   // M8: --gpu-row-threads N (0 = sin hilos auxiliares)

bool D3D12Backend::ParallelRows(uint32_t rows, const std::function<bool(uint32_t, uint32_t)>& fn) {
    if (!g_rowThreads || rows < 64) return fn(0, rows);
    if (!g_rowPool) { g_rowPool = new RowPool(); g_rowPool->Start(g_rowThreads); }
    return g_rowPool->Run(rows, fn);
}

} // namespace rt::gpu::d3d

namespace rt::gpu::d3d {
bool Start(const Options& o) {
    auto* b = new D3D12Backend();
    b->opt = o;
    const char* s = o.dumpFrames.c_str();
    while (*s) { char* e; unsigned long v = std::strtoul(s, &e, 10); if (e == s) break; b->dumpSet.insert((uint32_t)v); s = *e ? e + 1 : e; }
    if (!b->Init()) { ImportLog::Get().Printf("D3D12: inicializacion FALLIDA"); delete b; return false; }
    g_backend = b;
    backend::Set(b);
    return true;
}
} // namespace rt::gpu::d3d
