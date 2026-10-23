// Autoprueba V0 (--selftest-jpeg): codificacion JPEG en memoria y verificacion con decodificador WIC.
#include <windows.h>
#include <wincodec.h>

#include <cmath>
#include <cstdio>
#include <vector>

#include "util/image_io.h"
#include "viewer/viewer.h"

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

namespace rt::viewer {

int RunJpegSelfTest() {
    const uint32_t w = 1280, h = 720, q = 70;
    std::vector<uint8_t> src((size_t)w * h * 4);
    // Patron: gradientes suaves y bloques de color con bordes (FFD8/FFD9 y PSNR).
    for (uint32_t y = 0; y < h; ++y)
        for (uint32_t x = 0; x < w; ++x) {
            uint8_t* p = &src[((size_t)y * w + x) * 4];
            bool block = ((x / 160) + (y / 90)) % 2 == 0;
            p[0] = block ? (uint8_t)(x * 255 / w) : (uint8_t)(255 - x * 255 / w);
            p[1] = block ? (uint8_t)(y * 255 / h) : 128;
            p[2] = block ? 40 : (uint8_t)(y * 255 / h);
            p[3] = 255;
        }
    std::vector<uint8_t> jpg;
    auto t0 = GetTickCount64();
    bool enc = rt::EncodeJpegRGBA(w, h, src.data(), w * 4, q, jpg);
    auto encMs = GetTickCount64() - t0;
    bool soi = jpg.size() > 4 && jpg[0] == 0xFF && jpg[1] == 0xD8;
    bool eoi = jpg.size() > 4 && jpg[jpg.size() - 2] == 0xFF && jpg[jpg.size() - 1] == 0xD9;

    // Decodificacion con WIC y PSNR (RGB).
    double psnr = 0.0;
    bool dec = false;
    HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IWICImagingFactory* f = nullptr;
    IWICStream* st = nullptr;
    IWICBitmapDecoder* d = nullptr;
    IWICBitmapFrameDecode* fr = nullptr;
    IWICFormatConverter* cv = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f))) && enc) {
        if (SUCCEEDED(f->CreateStream(&st)) && SUCCEEDED(st->InitializeFromMemory(jpg.data(), (DWORD)jpg.size())) &&
            SUCCEEDED(f->CreateDecoderFromStream(st, nullptr, WICDecodeMetadataCacheOnDemand, &d)) &&
            SUCCEEDED(d->GetFrame(0, &fr)) && SUCCEEDED(f->CreateFormatConverter(&cv)) &&
            SUCCEEDED(cv->Initialize(fr, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
            UINT dw = 0, dh = 0;
            cv->GetSize(&dw, &dh);
            if (dw == w && dh == h) {
                std::vector<uint8_t> out((size_t)w * h * 4);
                if (SUCCEEDED(cv->CopyPixels(nullptr, w * 4, (UINT)out.size(), out.data()))) {
                    double se = 0.0;
                    for (size_t i = 0; i < out.size(); i += 4)
                        for (int c = 0; c < 3; ++c) {
                            double df = (double)out[i + c] - (double)src[i + c];
                            se += df * df;
                        }
                    double mse = se / ((double)w * h * 3);
                    psnr = mse > 0 ? 10.0 * std::log10(255.0 * 255.0 / mse) : 99.0;
                    dec = true;
                }
            }
        }
    }
    if (cv) cv->Release();
    if (fr) fr->Release();
    if (d) d->Release();
    if (st) st->Release();
    if (f) f->Release();
    if (SUCCEEDED(co)) CoUninitialize();

    bool ok = enc && soi && eoi && dec && psnr >= 30.0;
    std::printf("selftest-jpeg: %ux%u q=%u bytes=%zu enc_ms=%llu SOI=%d EOI=%d wic_decode=%d PSNR=%.2f dB -> %s\n",
                w, h, q, jpg.size(), (unsigned long long)encMs, (int)soi, (int)eoi, (int)dec, psnr, ok ? "OK" : "FALLO");
    return ok ? 0 : 1;
}

} // namespace rt::viewer
