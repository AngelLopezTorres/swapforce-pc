#include "image_io.h"

#include <windows.h>
#include <wincodec.h>

#include <vector>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

namespace rt {

static void MakeDirs(const std::string& path) {
    for (size_t i = 0; i < path.size(); ++i)
        if (path[i] == '\\' || path[i] == '/') {
            std::string d = path.substr(0, i);
            if (d.size() > 2) CreateDirectoryA(d.c_str(), nullptr);
        }
}

template <class T> static void Rel(T*& p) { if (p) { p->Release(); p = nullptr; } }

bool WritePngRGBA(const std::string& path, uint32_t w, uint32_t h, const uint8_t* rgba, uint32_t stride) {
    MakeDirs(path);
    HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool coInit = SUCCEEDED(co);
    IWICImagingFactory* f = nullptr;
    IWICBitmapEncoder* enc = nullptr;
    IWICBitmapFrameEncode* fr = nullptr;
    IWICStream* st = nullptr;
    IPropertyBag2* pb = nullptr;
    bool ok = false;
    std::vector<uint8_t> bgra((size_t)w * h * 4);
    for (uint32_t y = 0; y < h; ++y)
        for (uint32_t x = 0; x < w; ++x) {
            const uint8_t* s = rgba + (size_t)y * stride + x * 4;
            uint8_t* d = &bgra[((size_t)y * w + x) * 4];
            d[0] = s[2]; d[1] = s[1]; d[2] = s[0]; d[3] = s[3];
        }
    WCHAR wp[MAX_PATH * 2];
    MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, wp, MAX_PATH * 2);
    do {
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f)))) break;
        if (FAILED(f->CreateStream(&st))) break;
        if (FAILED(st->InitializeFromFilename(wp, GENERIC_WRITE))) break;
        if (FAILED(f->CreateEncoder(GUID_ContainerFormatPng, nullptr, &enc))) break;
        if (FAILED(enc->Initialize(st, WICBitmapEncoderNoCache))) break;
        if (FAILED(enc->CreateNewFrame(&fr, &pb))) break;
        if (FAILED(fr->Initialize(pb))) break;
        if (FAILED(fr->SetSize(w, h))) break;
        WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
        if (FAILED(fr->SetPixelFormat(&fmt))) break;
        if (FAILED(fr->WritePixels(h, w * 4, (UINT)bgra.size(), bgra.data()))) break;
        if (FAILED(fr->Commit())) break;
        if (FAILED(enc->Commit())) break;
        ok = true;
    } while (false);
    Rel(pb); Rel(fr); Rel(enc); Rel(st); Rel(f);
    if (coInit) CoUninitialize();
    return ok;
}

ImageStats ComputeStats(uint32_t w, uint32_t h, const uint8_t* rgba, uint32_t stride) {
    ImageStats s;
    uint64_t hh = 1469598103934665603ull;
    for (uint32_t y = 0; y < h; ++y) {
        const uint8_t* r = rgba + (size_t)y * stride;
        for (uint32_t x = 0; x < w; ++x) {
            const uint8_t* p = r + x * 4;
            if (p[0] | p[1] | p[2]) ++s.nonblack;
            if (p[0] < s.minR) s.minR = p[0]; if (p[0] > s.maxR) s.maxR = p[0];
            if (p[1] < s.minG) s.minG = p[1]; if (p[1] > s.maxG) s.maxG = p[1];
            if (p[2] < s.minB) s.minB = p[2]; if (p[2] > s.maxB) s.maxB = p[2];
            hh ^= *(const uint32_t*)p; hh *= 1099511628211ull;
        }
    }
    s.hash = hh;
    return s;
}

} // namespace rt
