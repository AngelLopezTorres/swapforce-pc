#include "xma_ffmpeg.h"

#include <windows.h>

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <mutex>

#ifdef SWAPFORCE_FFMPEG_HEADERS
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/log.h>
#include <libavutil/mem.h>
}
#endif

namespace rt::audio {

#ifndef SWAPFORCE_FFMPEG_HEADERS

bool FfAvailable(std::string* info) {
    if (info) *info = "compilado sin cabeceras de FFmpeg (runtime\\third_party_bin\\ffmpeg\\include ausente)";
    return false;
}
const std::string& FfVersionInfo() { static std::string s = "compilado sin cabeceras de FFmpeg"; return s; }
FfXmaDecoder::FfXmaDecoder() {}
FfXmaDecoder::~FfXmaDecoder() {}
bool FfXmaDecoder::Open(int, int) { return false; }
void FfXmaDecoder::Reset() {}
int FfXmaDecoder::DecodePacket(const uint8_t*, std::vector<int16_t>&, int*) { return -1; }

#else

namespace {

struct Api {
    HMODULE hAvcodec = nullptr, hAvutil = nullptr, hSwr = nullptr;
    unsigned (*avcodec_version_)() = nullptr;
    const AVCodec* (*avcodec_find_decoder_)(enum AVCodecID) = nullptr;
    AVCodecContext* (*avcodec_alloc_context3_)(const AVCodec*) = nullptr;
    int (*avcodec_open2_)(AVCodecContext*, const AVCodec*, AVDictionary**) = nullptr;
    void (*avcodec_free_context_)(AVCodecContext**) = nullptr;
    int (*avcodec_send_packet_)(AVCodecContext*, const AVPacket*) = nullptr;
    int (*avcodec_receive_frame_)(AVCodecContext*, AVFrame*) = nullptr;
    void (*avcodec_flush_buffers_)(AVCodecContext*) = nullptr;
    AVPacket* (*av_packet_alloc_)() = nullptr;
    void (*av_packet_free_)(AVPacket**) = nullptr;
    int (*av_new_packet_)(AVPacket*, int) = nullptr;
    void (*av_packet_unref_)(AVPacket*) = nullptr;
    AVFrame* (*av_frame_alloc_)() = nullptr;
    void (*av_frame_free_)(AVFrame**) = nullptr;
    void (*av_frame_unref_)(AVFrame*) = nullptr;
    void* (*av_mallocz_)(size_t) = nullptr;
    void (*av_channel_layout_default_)(AVChannelLayout*, int) = nullptr;
    void (*av_log_set_level_)(int) = nullptr;
};

Api g_api;
std::string g_info;
bool g_loaded = false, g_ok = false;
std::once_flag g_once;
AVPacket* g_pkt = nullptr;   // reutilizado (solo bajo g_decodeMu)
std::mutex g_decodeMu;

std::wstring ExeDir() {
    wchar_t p[MAX_PATH * 2] = {}; GetModuleFileNameW(nullptr, p, MAX_PATH * 2);
    std::wstring s = p; size_t k = s.find_last_of(L"\\/"); return k == std::wstring::npos ? L"." : s.substr(0, k);
}

std::wstring FindDll(const std::wstring& dir, const wchar_t* prefix) {
    WIN32_FIND_DATAW fd; std::wstring pat = dir + L"\\" + prefix + L"-*.dll";
    HANDLE h = FindFirstFileW(pat.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return L"";
    std::wstring r = dir + L"\\" + fd.cFileName; FindClose(h); return r;
}

template <class T> bool Sym(HMODULE m, const char* n, T& out) { out = reinterpret_cast<T>(GetProcAddress(m, n)); return out != nullptr; }

void LoadAll() {
    g_loaded = true;
    std::vector<std::wstring> dirs;
    wchar_t env[MAX_PATH] = {};
    if (GetEnvironmentVariableW(L"SWAPFORCE_FFMPEG_DIR", env, MAX_PATH)) dirs.push_back(env);
    std::wstring e = ExeDir();
    dirs.push_back(e); dirs.push_back(e + L"\\ffmpeg"); dirs.push_back(e + L"\\..\\..\\third_party_bin\\ffmpeg"); dirs.push_back(e + L"\\..\\third_party_bin\\ffmpeg");
    std::wstring dAvcodec, dir;
    for (auto& d : dirs) { dAvcodec = FindDll(d, L"avcodec"); if (!dAvcodec.empty()) { dir = d; break; } }
    if (dAvcodec.empty()) { g_info = "no se encontro avcodec-NN.dll (exe, exe\\ffmpeg, runtime\\third_party_bin\\ffmpeg, SWAPFORCE_FFMPEG_DIR)"; return; }
    std::wstring dUtil = FindDll(dir, L"avutil"), dSwr = FindDll(dir, L"swresample");
    if (dUtil.empty()) { g_info = "falta avutil-NN.dll junto a avcodec"; return; }
    // Dependencias primero, por ruta completa (avcodec las resuelve por nombre ya cargado).
    if (!dSwr.empty()) g_api.hSwr = LoadLibraryExW(dSwr.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    g_api.hAvutil = LoadLibraryExW(dUtil.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!g_api.hAvutil) { g_info = "LoadLibrary(avutil) fallo, GetLastError=" + std::to_string(GetLastError()); return; }
    g_api.hAvcodec = LoadLibraryExW(dAvcodec.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!g_api.hAvcodec) { g_info = "LoadLibrary(avcodec) fallo, GetLastError=" + std::to_string(GetLastError()); return; }
    HMODULE c = g_api.hAvcodec, u = g_api.hAvutil;
    bool ok = Sym(c, "avcodec_version", g_api.avcodec_version_) && Sym(c, "avcodec_find_decoder", g_api.avcodec_find_decoder_) &&
              Sym(c, "avcodec_alloc_context3", g_api.avcodec_alloc_context3_) && Sym(c, "avcodec_open2", g_api.avcodec_open2_) &&
              Sym(c, "avcodec_free_context", g_api.avcodec_free_context_) && Sym(c, "avcodec_send_packet", g_api.avcodec_send_packet_) &&
              Sym(c, "avcodec_receive_frame", g_api.avcodec_receive_frame_) && Sym(c, "avcodec_flush_buffers", g_api.avcodec_flush_buffers_) &&
              Sym(c, "av_packet_alloc", g_api.av_packet_alloc_) && Sym(c, "av_packet_free", g_api.av_packet_free_) &&
              Sym(c, "av_new_packet", g_api.av_new_packet_) && Sym(c, "av_packet_unref", g_api.av_packet_unref_) &&
              Sym(u, "av_frame_alloc", g_api.av_frame_alloc_) && Sym(u, "av_frame_free", g_api.av_frame_free_) &&
              Sym(u, "av_frame_unref", g_api.av_frame_unref_) && Sym(u, "av_mallocz", g_api.av_mallocz_) &&
              Sym(u, "av_channel_layout_default", g_api.av_channel_layout_default_) && Sym(u, "av_log_set_level", g_api.av_log_set_level_);
    if (!ok) { g_info = "faltan simbolos en las DLL de FFmpeg"; return; }
    unsigned v = g_api.avcodec_version_();
    char b[256];
    std::snprintf(b, sizeof b, "avcodec %u.%u.%u", v >> 16, (v >> 8) & 255, v & 255);
    if ((v >> 16) != LIBAVCODEC_VERSION_MAJOR) {
        std::snprintf(b, sizeof b, "version mayor de avcodec %u distinta de la de las cabeceras (%d): no se usa", v >> 16, LIBAVCODEC_VERSION_MAJOR);
        g_info = b; return;
    }
    if (!g_api.avcodec_find_decoder_(AV_CODEC_ID_XMA2)) { g_info = std::string(b) + ": sin decodificador xma2"; return; }
    g_api.av_log_set_level_(AV_LOG_ERROR);
    char dirU8[MAX_PATH * 2] = {}; WideCharToMultiByte(CP_UTF8, 0, dir.c_str(), -1, dirU8, sizeof dirU8 - 1, nullptr, nullptr);
    g_info = std::string(b) + " (" + dirU8 + ")";
    g_pkt = g_api.av_packet_alloc_();
    g_ok = g_pkt != nullptr;
}

} // namespace

bool FfAvailable(std::string* info) {
    std::call_once(g_once, LoadAll);
    if (info) *info = g_info;
    return g_ok;
}
const std::string& FfVersionInfo() { FfAvailable(); return g_info; }

FfXmaDecoder::FfXmaDecoder() {}
FfXmaDecoder::~FfXmaDecoder() {
    if (!g_ok) return;
    if (frame_) { AVFrame* f = (AVFrame*)frame_; g_api.av_frame_free_(&f); frame_ = nullptr; }
    if (ctx_) { AVCodecContext* c = (AVCodecContext*)ctx_; g_api.avcodec_free_context_(&c); ctx_ = nullptr; }
}

bool FfXmaDecoder::Open(int channels, int rate) {
    if (!FfAvailable() || (channels != 1 && channels != 2)) return false;
    if (ctx_) { AVCodecContext* c = (AVCodecContext*)ctx_; g_api.avcodec_free_context_(&c); ctx_ = nullptr; }
    const AVCodec* codec = g_api.avcodec_find_decoder_(AV_CODEC_ID_XMA2);
    AVCodecContext* c = g_api.avcodec_alloc_context3_(codec);
    if (!c) return false;
    c->sample_rate = rate;
    g_api.av_channel_layout_default_(&c->ch_layout, channels);
    // XMA2WAVEFORMATEX (34 bytes): NumStreams(u16), ChannelMask(u32), SamplesEncoded(u32), BytesPerBlock(u32), PlayBegin, PlayLength,
    // LoopBegin, LoopLength (u32 cada uno), LoopCount(u8), EncoderVersion(u8), BlockCount(u16).
    uint8_t* ex = (uint8_t*)g_api.av_mallocz_(34 + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!ex) { g_api.avcodec_free_context_(&c); return false; }
    ex[0] = 1; ex[1] = 0;                                  // 1 stream
    ex[2] = channels == 2 ? 3 : 4;                         // mascara: FL|FR, o FC
    ex[10] = 0; ex[11] = 0x08; ex[12] = 0; ex[13] = 0;     // BytesPerBlock = 2048 (informativo)
    ex[31] = 4;                                            // EncoderVersion 4 (XMA2)
    c->extradata = ex; c->extradata_size = 34;
    int r = g_api.avcodec_open2_(c, codec, nullptr);
    if (r < 0) { lastErr_ = r; g_api.avcodec_free_context_(&c); return false; }
    ctx_ = c; channels_ = channels; rate_ = rate;
    if (!frame_) frame_ = g_api.av_frame_alloc_();
    return frame_ != nullptr;
}

void FfXmaDecoder::Reset() {
    if (ctx_) g_api.avcodec_flush_buffers_((AVCodecContext*)ctx_);
}

int FfXmaDecoder::DecodePacket(const uint8_t* pk, std::vector<int16_t>& out, int* spf) {
    if (!ctx_ || !frame_) return -1;
    std::lock_guard<std::mutex> lk(g_decodeMu);   // g_pkt compartido
    AVCodecContext* c = (AVCodecContext*)ctx_;
    AVFrame* f = (AVFrame*)frame_;
    if (g_api.av_new_packet_(g_pkt, 2048) < 0) return -1;
    std::memcpy(g_pkt->data, pk, 2048);
    int r = g_api.avcodec_send_packet_(c, g_pkt);
    g_api.av_packet_unref_(g_pkt);
    if (r < 0 && r != AVERROR(EAGAIN)) { lastErr_ = r; ++errors_; return -1; }
    int frames = 0;
    for (;;) {
        r = g_api.avcodec_receive_frame_(c, f);
        if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) break;
        if (r < 0) { lastErr_ = r; ++errors_; return frames ? frames : -1; }
        int n = f->nb_samples, ch = f->ch_layout.nb_channels;
        if (spf) *spf = n;
        size_t base = out.size();
        out.resize(base + (size_t)n * channels_);
        for (int i = 0; i < n; ++i)
            for (int k = 0; k < channels_; ++k) {
                float v;
                if (f->format == AV_SAMPLE_FMT_FLTP) v = ((const float*)f->extended_data[k < ch ? k : 0])[i];
                else if (f->format == AV_SAMPLE_FMT_FLT) v = ((const float*)f->extended_data[0])[(size_t)i * ch + (k < ch ? k : 0)];
                else if (f->format == AV_SAMPLE_FMT_S16P) v = ((const int16_t*)f->extended_data[k < ch ? k : 0])[i] / 32768.0f;
                else v = 0.f;
                float s = v * 32768.0f;
                if (s > 32767.f) s = 32767.f; else if (s < -32768.f) s = -32768.f;
                out[base + (size_t)i * channels_ + k] = (int16_t)std::lrintf(s);
            }
        ++frames;
        g_api.av_frame_unref_(f);
    }
    return frames;
}

#endif

} // namespace rt::audio
