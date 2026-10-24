#include "image_io.h"

#include <windows.h>
#include <wincodec.h>

#include <algorithm>
#include <limits>
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

bool EncodeJpegRGBA(uint32_t w, uint32_t h, const uint8_t* rgba, uint32_t stride, uint32_t quality, std::vector<uint8_t>& out) {
    out.clear();
    if (!w || !h || !rgba) return false;
    HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool coInit = SUCCEEDED(co);
    IWICImagingFactory* f = nullptr;
    IWICBitmapEncoder* enc = nullptr;
    IWICBitmapFrameEncode* fr = nullptr;
    IWICStream* st = nullptr;
    IPropertyBag2* pb = nullptr;
    IStream* ms = nullptr;
    HGLOBAL hg = nullptr;
    bool ok = false;
    std::vector<uint8_t> bgr24((size_t)w * h * 3);
    for (uint32_t y = 0; y < h; ++y)
        for (uint32_t x = 0; x < w; ++x) {
            const uint8_t* s = rgba + (size_t)y * stride + x * 4;
            uint8_t* d = &bgr24[((size_t)y * w + x) * 3];
            d[0] = s[2]; d[1] = s[1]; d[2] = s[0];
        }
    do {
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f)))) break;
        if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &ms))) break;
        if (FAILED(f->CreateStream(&st))) break;
        if (FAILED(st->InitializeFromIStream(ms))) break;
        if (FAILED(f->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, &enc))) break;
        if (FAILED(enc->Initialize(st, WICBitmapEncoderNoCache))) break;
        if (FAILED(enc->CreateNewFrame(&fr, &pb))) break;
        // "ImageQuality" (0.0-1.0), Microsoft Learn: JPEG Format-Specific Encoder Options.
        PROPBAG2 opt = {};
        opt.pstrName = const_cast<LPOLESTR>(L"ImageQuality");
        VARIANT var;
        VariantInit(&var);
        var.vt = VT_R4;
        var.fltVal = (float)std::min<uint32_t>(100u, std::max<uint32_t>(1u, quality)) / 100.0f;
        if (FAILED(pb->Write(1, &opt, &var))) break;
        if (FAILED(fr->Initialize(pb))) break;
        if (FAILED(fr->SetSize(w, h))) break;
        // El codificador JPEG de WIC admite 24bppBGR; SetPixelFormat puede cambiar el formato solicitado,
        // por eso se comprueba y los datos se preparan para el formato devuelto.
        WICPixelFormatGUID fmt = GUID_WICPixelFormat24bppBGR;
        if (FAILED(fr->SetPixelFormat(&fmt))) break;
        if (fmt != GUID_WICPixelFormat24bppBGR) break;
        if (FAILED(fr->WritePixels(h, w * 3, (UINT)bgr24.size(), bgr24.data()))) break;
        if (FAILED(fr->Commit())) break;
        if (FAILED(enc->Commit())) break;
        LARGE_INTEGER zero = {};
        ULARGE_INTEGER pos = {};
        if (FAILED(ms->Seek(zero, STREAM_SEEK_CUR, &pos))) break;
        if (FAILED(GetHGlobalFromStream(ms, &hg))) break;
        void* p = GlobalLock(hg);
        if (!p) break;
        out.assign(static_cast<const uint8_t*>(p), static_cast<const uint8_t*>(p) + (size_t)pos.QuadPart);
        GlobalUnlock(hg);
        ok = !out.empty();
    } while (false);
    Rel(pb); Rel(fr); Rel(enc); Rel(st);
    if (ms) ms->Release();
    if (f) f->Release();
    if (coInit) CoUninitialize();
    if (!ok) out.clear();
    return ok;
}

bool ReadImageRGBA(const std::string& path, uint32_t& w, uint32_t& h, std::vector<uint8_t>& rgba) {
    w = h = 0;
    rgba.clear();
    int chars = MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, nullptr, 0);
    if (!chars) return false;
    std::vector<WCHAR> widePath((size_t)chars);
    if (!MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, widePath.data(), chars)) return false;

    HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool coInit = SUCCEEDED(co);
    if (FAILED(co) && co != RPC_E_CHANGED_MODE) return false;
    IWICImagingFactory* f = nullptr;
    IWICBitmapDecoder* dec = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* cv = nullptr;
    bool ok = false;
    do {
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f)))) break;
        // WIC decoder/converter: Microsoft Learn, https://learn.microsoft.com/en-us/windows/win32/wic/-wic-decoder-howto-createusingfilename
        if (FAILED(f->CreateDecoderFromFilename(widePath.data(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &dec))) break;
        if (FAILED(dec->GetFrame(0, &frame)) || FAILED(f->CreateFormatConverter(&cv))) break;
        if (FAILED(cv->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) break;
        UINT iw = 0, ih = 0;
        if (FAILED(cv->GetSize(&iw, &ih)) || !iw || !ih || iw > std::numeric_limits<UINT>::max() / 4u) break;
        uint64_t bytes = (uint64_t)iw * ih * 4u;
        if (bytes > std::numeric_limits<UINT>::max() || bytes > (uint64_t)std::numeric_limits<size_t>::max()) break;
        rgba.resize((size_t)bytes);
        if (FAILED(cv->CopyPixels(nullptr, iw * 4u, (UINT)bytes, rgba.data()))) { rgba.clear(); break; }
        w = iw; h = ih; ok = true;
    } while (false);
    if (cv) cv->Release();
    if (frame) frame->Release();
    if (dec) dec->Release();
    if (f) f->Release();
    if (coInit) CoUninitialize();
    if (!ok) { w = h = 0; rgba.clear(); }
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
