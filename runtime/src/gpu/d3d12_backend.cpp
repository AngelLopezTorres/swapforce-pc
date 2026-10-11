#include "d3d12_impl.h"

#include "config.h"
#include "fb_decode.h"
#include "image_io.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "user32.lib")

namespace rt::gpu::d3d {

// ---- Hilo de interfaz: solo crea la ventana y bombea mensajes (nunca bloquea; DXGI puede enviar SendMessage a este hilo).
//      Documentacion: "Multithreading and DXGI" (Microsoft Learn, dxgi-best-practices; copia en runtime\ref\).
static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
        case WM_CLOSE: ImportLog::Get().Printf("UI: ventana cerrada por el usuario, se termina el proceso"); std::fflush(nullptr); ExitProcess(0); return 0;
        case WM_DESTROY: return 0;
    }
    return DefWindowProcW(h, m, w, l);
}
static DWORD WINAPI UiMain(void* p) {
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
    if (self->hwnd) ShowWindow(self->hwnd, SW_SHOWNORMAL);
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
    for (uint32_t i = 0; i < kFramesInFlight; ++i)
        if (!Hr(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc[i])), "CreateCommandAllocator")) return false;
    if (!Hr(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc[0].Get(), nullptr, IID_PPV_ARGS(&cl)), "CreateCommandList")) return false;
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
    rd.Format = DXGI_FORMAT_R8G8B8A8_UNORM; rd.SampleDesc.Count = 1; rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
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
    fence->SetEventOnCompletion(v, fenceEvent);
    WaitForSingleObject(fenceEvent, 20000);
}
uint64_t D3D12Backend::Submit() {
    ID3D12CommandList* l[] = {cl.Get()};
    queue->ExecuteCommandLists(1, l);
    queue->Signal(fence.Get(), ++fenceValue);
    return fenceValue;
}
void D3D12Backend::BeginList() {
    if (clOpen) return;
    WaitFence(allocFence[frameSlot]);
    Hr(alloc[frameSlot]->Reset(), "CommandAllocator::Reset");
    Hr(cl->Reset(alloc[frameSlot].Get(), nullptr), "CommandList::Reset");
    clOpen = true;
    uploadOffset = 0; uploadBytesFrame = 0;
    ID3D12DescriptorHeap* heaps[] = {gpuSrvHeap.Get(), gpuSmpHeap.Get()};
    cl->SetDescriptorHeaps(2, heaps);
}
void D3D12Backend::FlushAndWait() {
    if (!clOpen) return;
    Hr(cl->Close(), "CommandList::Close (flush)");
    clOpen = false;
    uint64_t fv = Submit();
    allocFence[frameSlot] = fv;
    WaitFence(fv);
    gpuSrvNext = 0;   // toda la GPU esta ociosa: se pueden reutilizar los descriptores
    gpuSmpNext = 0; samplerTables.clear();
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
    phys &= 0x1FFFFFFF;
    uint64_t end = (uint64_t)phys + bytes;
    if (end > kSharedMemBytes) end = kSharedMemBytes;
    uint32_t p0 = phys >> 12, p1 = (uint32_t)((end + 4095) >> 12);
    uint32_t runStart = 0; bool inRun = false;
    auto flush = [&](uint32_t from, uint32_t to) {   // paginas [from, to)
        uint32_t n = (to - from) * 4096;
        const uint8_t* src = PhysHost(from << 12, n);
        if (!src) return;
        UploadAlloc u = Upload(n, 512);
        if (!u.cpu) { Note("OMISION memoria: rango de subida mayor que la seccion del anillo"); return; }
        std::memcpy(u.cpu, src, n);
        SetSharedState(D3D12_RESOURCE_STATE_COPY_DEST);
        cl->CopyBufferRegion(sharedMem.Get(), (UINT64)from << 12, uploadBuf.Get(), u.offset, n);
        pagesUploaded += to - from;
    };
    for (uint32_t pg = p0; pg < p1; ++pg) {
        const uint8_t* src = PhysHost(pg << 12, 4096);
        uint32_t h = 0;
        if (src) {
            uint64_t x = 1469598103934665603ull;
            const uint64_t* q = reinterpret_cast<const uint64_t*>(src);
            for (int i = 0; i < 512; ++i) { x ^= q[i]; x *= 1099511628211ull; }
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
    BeginList();
    ++drawSerial;
    if (DoDraw(ev, vs, ps)) { ++drawsOk; ++fDrawsOk; }
    else { ++drawsSkip; ++fDrawsSkip; }
}

void D3D12Backend::OnSwap(const backend::SwapInfo& s) {
    BeginList();
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
        std::vector<uint8_t> rgba;
        if (ri && ri->x0 == 0 && ri->y0 == 0 && ri->w == s.w) {
            source = "resolve";
            PresentFromRgba(ri->rgba.data(), ri->w, std::min(ri->h, s.h));
        } else {
            FbDesc d;
            for (int i = 0; i < 6; ++i) d.fetch[i] = s.fetch[i];
            d.phys = s.fbPhys; d.w = s.w; d.h = s.h;
            std::string info;
            if (DecodeFrontbuffer(d, rgba, &info)) { source = "frontbuffer"; PresentFromRgba(rgba.data(), s.w, s.h); }
            else Note("OMISION presentacion: no se pudo destilar el frontbuffer (" + info + ")");
        }
    }
    bool dump = dumpSet.count((uint32_t)s.frame) != 0;
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
    Hr(cl->Close(), "CommandList::Close");
    clOpen = false;
    uint64_t fv = Submit();
    allocFence[frameSlot] = fv;
    frameSlot = (frameSlot + 1) % kFramesInFlight;
    if (swap) { Hr(swap->Present(1, 0), "Present"); ++presentCalls; }
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
                std::snprintf(name, sizeof(name), "\\userdata\\frames\\d3d_%05llu.png", (unsigned long long)s.frame);
                std::string path = GetPaths().runtimeDir + name;
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
    fDrawsOk = fDrawsSkip = fShaderFail = fPsoFail = fResolve = fHrFail = fTexFail = 0;
}

void D3D12Backend::Shutdown() {
    if (!dev) return;
    if (fence) WaitFence(fenceValue);
    ImportLog::Get().Printf("D3D12 resumen: frames=%llu present=%llu draws_ok=%llu draws_skip=%llu shader_fail=%llu pso_fail=%llu resolve=%llu tex_fail=%llu hr_fail=%llu removed=%llu",
                            (unsigned long long)framesPresented, (unsigned long long)presentCalls, (unsigned long long)drawsOk, (unsigned long long)drawsSkip,
                            (unsigned long long)shaderFail, (unsigned long long)psoFail, (unsigned long long)resolves, (unsigned long long)texFail,
                            (unsigned long long)hrFail, (unsigned long long)removed);
    ImportLog::Get().Printf("D3D12 memoria: paginas_subidas=%llu paginas_sin_cambios=%llu bytes_subidos=%llu texturas_creadas=%llu texturas_reutilizadas=%llu psos=%zu dxil=%zu rt_color=%zu rt_profundidad=%zu",
                            (unsigned long long)pagesUploaded, (unsigned long long)pagesSkipped, (unsigned long long)uploadBytesTotal, (unsigned long long)texCreated,
                            (unsigned long long)texHits, psoCache.size(), dxilCache.size(), colorRTs.size(), depthRTs.size());
    for (auto& kv : notes) ImportLog::Get().Printf("D3D12 NOTA n=%llu %s", (unsigned long long)kv.second, kv.first.c_str());
    backend::Set(nullptr);
    ui.Stop();
}

static D3D12Backend* g_backend = nullptr;

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
