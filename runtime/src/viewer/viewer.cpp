#include "viewer/viewer.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "import_log.h"
#include "util/image_io.h"

#pragma comment(lib, "ws2_32.lib")

namespace rt::viewer {

std::atomic<bool> g_active{false};

namespace {

// Mascara XINPUT_GAMEPAD (xam_input.cpp)
constexpr uint16_t kUp = 0x0001, kDown = 0x0002, kLeft = 0x0004, kRight = 0x0008;
constexpr uint16_t kStart = 0x0010, kBack = 0x0020, kA = 0x1000, kB = 0x2000, kXb = 0x4000, kYb = 0x8000;
constexpr int64_t kKeyTimeoutMs = 500;     // sin latido en 500 ms -> botones a 0
constexpr int64_t kDemandMs = 3000;        // sin peticiones en 3 s -> no se codifica
constexpr size_t kMaxHeader = 4096;
constexpr uint32_t kMaxClients = 12;   // el navegador deja conexiones colgadas: margen amplio (solo loopback)
constexpr size_t kMaxJpeg = 4u << 20;      // 4 MiB

Options g_opt;
std::mutex g_mu;                 // raw + jpeg + estado de hilos
std::condition_variable g_cv;    // nuevo JPEG disponible / parada
std::vector<uint8_t> g_raw;      // RGBA tight (cw*ch*4), escrito por CP
uint32_t g_rawW = 0, g_rawH = 0;
uint64_t g_rawFrame = 0;
bool g_rawFresh = false;
std::vector<uint8_t> g_jpeg;     // ultimo JPEG
uint64_t g_jpegSeq = 0;
uint64_t g_jpegFrame = 0;
uint32_t g_jpegW = 0, g_jpegH = 0;
bool g_stop = false;
std::thread g_acceptThread, g_encThread;
std::vector<std::thread> g_clientThreads;
std::atomic<uint32_t> g_clients{0};
SOCKET g_listen = INVALID_SOCKET;
std::mutex g_sockMu;
std::vector<SOCKET> g_sockets;   // conexiones activas (para cerrarlas en Stop)
std::atomic<int64_t> g_lastDemand{0};
std::atomic<int64_t> g_lastCopy{0};
std::atomic<uint16_t> g_buttons{0};
std::atomic<int64_t> g_buttonStamp{0};
bool g_wsaStarted = false;

int64_t NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool Demand() { return NowMs() - g_lastDemand.load() < kDemandMs; }

// ---------- HTTP ----------
struct Request {
    std::string method, path, query, host, origin;
    bool ok = false;
};

bool ReadHeader(SOCKET s, std::string& head) {
    head.clear();
    char buf[1024];
    while (head.find("\r\n\r\n") == std::string::npos) {
        if (head.size() >= kMaxHeader) return false;
        int n = recv(s, buf, sizeof(buf), 0);
        if (n <= 0) return false;
        head.append(buf, (size_t)n);
    }
    return true;
}

std::string HeaderValue(const std::string& head, const char* name) {
    size_t nl = 0;
    size_t nlen = std::strlen(name);
    while (nl != std::string::npos) {
        size_t start = (nl == 0) ? 0 : nl + 2;
        size_t end = head.find("\r\n", start);
        std::string line = head.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (line.size() > nlen && _strnicmp(line.c_str(), name, nlen) == 0 && line[nlen] == ':') {
            size_t v = nlen + 1;
            while (v < line.size() && (line[v] == ' ' || line[v] == '\t')) ++v;
            return line.substr(v);
        }
        nl = end;
    }
    return std::string();
}

bool SendAll(SOCKET s, const void* data, size_t n) {
    const char* p = static_cast<const char*>(data);
    while (n > 0) {
        int k = send(s, p, (int)std::min<size_t>(n, 65536), 0);
        if (k <= 0) return false;
        p += k; n -= (size_t)k;
    }
    return true;
}

bool SendSimple(SOCKET s, int code, const char* text, const char* extra = "") {
    char b[512];
    int n = std::snprintf(b, sizeof(b), "HTTP/1.1 %d %s\r\n%sContent-Length: 0\r\nConnection: close\r\n\r\n", code, text, extra);
    return SendAll(s, b, (size_t)n);
}

std::string Hostport(uint16_t port, const char* host) {
    return std::string(host) + ":" + std::to_string(port);
}

uint16_t g_port = 0;

// Botones desde /key?down=a,b,...
bool ParseKeys(const std::string& q, uint16_t& mask) {
    mask = 0;
    if (q.compare(0, 5, "down=") != 0) return false;
    std::string list = q.substr(5);
    size_t pos = 0;
    while (pos <= list.size()) {
        size_t comma = list.find(',', pos);
        std::string t = list.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
        if (!t.empty()) {
            if (t == "up") mask |= kUp;
            else if (t == "down") mask |= kDown;
            else if (t == "left") mask |= kLeft;
            else if (t == "right") mask |= kRight;
            else if (t == "enter") mask |= kStart;
            else if (t == "esc") mask |= kBack;
            else if (t == "z") mask |= kA;
            else if (t == "x") mask |= kB;
            else if (t == "y") mask |= kYb;
            else if (t == "xb") mask |= kXb;
            else return false;
        }
        if (comma == std::string::npos) break;
        pos = comma + 1;
    }
    return true;
}

void ServeStream(SOCKET s) {
    const char* hdr = "HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n";
    if (!SendAll(s, hdr, std::strlen(hdr))) return;
    uint64_t last = 0;
    std::vector<uint8_t> copy;
    for (;;) {
        {
            std::unique_lock<std::mutex> l(g_mu);
            g_cv.wait_for(l, std::chrono::milliseconds(1000), [&] { return g_stop || g_jpegSeq != last; });
            if (g_stop) return;
            if (g_jpegSeq == last) {
                g_lastDemand = NowMs();
                // el cliente puede haberse ido sin que se envie nada: detectarlo para liberar el hueco
                char probe; u_long avail = 0; (void)avail;
                fd_set rf; FD_ZERO(&rf); FD_SET(s, &rf); timeval tv{0, 0};
                if (select(0, &rf, nullptr, nullptr, &tv) > 0 && recv(s, &probe, 1, MSG_PEEK) <= 0) return;
                continue;
            }
            last = g_jpegSeq;
            copy = g_jpeg;
        }
        g_lastDemand = NowMs();
        char part[160];
        int n = std::snprintf(part, sizeof(part), "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n", copy.size());
        if (!SendAll(s, part, (size_t)n) || !SendAll(s, copy.data(), copy.size()) || !SendAll(s, "\r\n", 2)) return;
    }
}

void HandleClient(SOCKET s) {
    DWORD tmo = 5000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tmo, sizeof(tmo));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tmo, sizeof(tmo));
    std::string head;
    if (!ReadHeader(s, head)) { SendSimple(s, 431, "Request Header Fields Too Large"); return; }
    size_t sp1 = head.find(' '), sp2 = head.find(' ', sp1 + 1), le = head.find("\r\n");
    if (sp1 == std::string::npos || sp2 == std::string::npos || le == std::string::npos || sp2 > le) { SendSimple(s, 400, "Bad Request"); return; }
    std::string method = head.substr(0, sp1);
    std::string target = head.substr(sp1 + 1, sp2 - sp1 - 1);
    std::string path = target, query;
    size_t qm = target.find('?');
    if (qm != std::string::npos) { path = target.substr(0, qm); query = target.substr(qm + 1); }
    std::string host = HeaderValue(head, "Host");
    std::string origin = HeaderValue(head, "Origin");
    if (host != Hostport(g_port, "127.0.0.1") && host != Hostport(g_port, "localhost")) { SendSimple(s, 403, "Forbidden"); return; }
    if (method != "GET") { SendSimple(s, 405, "Method Not Allowed", "Allow: GET\r\n"); return; }
    g_lastDemand = NowMs();
    if (path == "/") {
        static const char* kPage =
            "<!doctype html><html lang=es><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'>"
            "<title>SwapForce visor</title><body style='margin:0;background:#000;display:flex;justify-content:center'>"
            "<img id=v src='/stream' style='max-width:100%;height:auto'>"
            "<script>"
            "const K={ArrowUp:'up',ArrowDown:'down',ArrowLeft:'left',ArrowRight:'right',Enter:'enter',Escape:'esc',KeyZ:'z',Space:'z',KeyX:'x',KeyC:'y',KeyY:'y',KeyV:'xb',Backspace:'x'};"
            "const act=new Set();"
            "function send(){const q=[...act].join(',');fetch('/key?down='+q,{method:'GET',cache:'no-store'}).catch(()=>{});}"
            "const P=new Set(['y','xb']);"
            "addEventListener('keydown',e=>{const k=K[e.code];if(k){e.preventDefault();if(P.has(k)){if(e.repeat||act.has(k))return;act.add(k);send();setTimeout(()=>{act.delete(k);send();},150);return;}act.add(k);send();}});"
            "addEventListener('keyup',e=>{const k=K[e.code];if(k&&!P.has(k)){act.delete(k);send();}});"
            "setInterval(send,200);"
            "</script></body></html>";
        char b[256];
        int n = std::snprintf(b, sizeof(b), "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nCache-Control: no-store\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n", std::strlen(kPage));
        if (SendAll(s, b, (size_t)n)) SendAll(s, kPage, std::strlen(kPage));
        return;
    }
    if (path == "/frame.jpg") {
        std::vector<uint8_t> copy;
        uint64_t frameNo = 0;
        {
            std::unique_lock<std::mutex> l(g_mu);
            g_cv.wait_for(l, std::chrono::milliseconds(2000), [] { return g_stop || g_jpegSeq != 0; });
            copy = g_jpeg;
            frameNo = g_jpegFrame;
        }
        if (copy.empty()) { SendSimple(s, 503, "Service Unavailable", "Retry-After: 1\r\n"); return; }
        char b[256];
        int n = std::snprintf(b, sizeof(b), "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nCache-Control: no-store\r\nX-Frame: %llu\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                              (unsigned long long)frameNo, copy.size());
        if (SendAll(s, b, (size_t)n)) SendAll(s, copy.data(), copy.size());
        return;
    }
    if (path == "/stream") { ServeStream(s); return; }
    if (path == "/key") {
        if (!g_opt.input) { SendSimple(s, 404, "Not Found"); return; }
        if (!origin.empty() && origin != "http://" + Hostport(g_port, "127.0.0.1") && origin != "http://" + Hostport(g_port, "localhost")) {
            SendSimple(s, 403, "Forbidden"); return;
        }
        uint16_t mask = 0;
        if (!ParseKeys(query, mask)) { SendSimple(s, 400, "Bad Request"); return; }
        g_buttons = mask;
        g_buttonStamp = NowMs();
        SendSimple(s, 204, "No Content");
        return;
    }
    SendSimple(s, 404, "Not Found");
}

void ClientMain(SOCKET s) {
    HandleClient(s);
    shutdown(s, SD_BOTH);
    closesocket(s);
    {
        std::lock_guard<std::mutex> l(g_sockMu);
        g_sockets.erase(std::remove(g_sockets.begin(), g_sockets.end(), s), g_sockets.end());
    }
    g_clients--;
}

void AcceptMain() {
    while (true) {
        // select con tiempo: en Windows cerrar el socket desde otro hilo no garantiza despertar a accept().
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(g_listen, &rfds);
        timeval tv{0, 200000};
        int r = select(0, &rfds, nullptr, nullptr, &tv);
        {
            std::lock_guard<std::mutex> l(g_mu);
            if (g_stop) break;
        }
        if (r <= 0) continue;
        SOCKET c = accept(g_listen, nullptr, nullptr);
        if (c == INVALID_SOCKET) continue;
        if (g_clients.load() >= kMaxClients) {
            DWORD tmo = 1000;
            setsockopt(c, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tmo, sizeof(tmo));
            SendSimple(c, 503, "Service Unavailable", "Retry-After: 1\r\n");
            shutdown(c, SD_BOTH);
            closesocket(c);
            continue;
        }
        g_clients++;
        {
            std::lock_guard<std::mutex> l(g_sockMu);
            g_sockets.push_back(c);
        }
        std::lock_guard<std::mutex> l(g_mu);
        g_clientThreads.emplace_back(ClientMain, c);
    }
}

// ---------- codificacion ----------
void Downscale(const std::vector<uint8_t>& src, uint32_t w, uint32_t h, uint32_t f, std::vector<uint8_t>& dst, uint32_t& ow, uint32_t& oh) {
    ow = w / f; oh = h / f;
    dst.resize((size_t)ow * oh * 4);
    for (uint32_t y = 0; y < oh; ++y)
        for (uint32_t x = 0; x < ow; ++x) {
            uint32_t acc[4] = {0, 0, 0, 0};
            for (uint32_t dy = 0; dy < f; ++dy)
                for (uint32_t dx = 0; dx < f; ++dx) {
                    const uint8_t* p = &src[((size_t)(y * f + dy) * w + (x * f + dx)) * 4];
                    for (int c = 0; c < 4; ++c) acc[c] += p[c];
                }
            uint32_t n = f * f;
            uint8_t* d = &dst[((size_t)y * ow + x) * 4];
            for (int c = 0; c < 4; ++c) d[c] = (uint8_t)(acc[c] / n);
        }
}

void EncMain() {
    std::vector<uint8_t> work, scaled, jpg;
    uint64_t synthSeq = 0;
    const auto period = std::chrono::milliseconds(1000 / std::max<uint32_t>(1, g_opt.fps));
    std::unique_lock<std::mutex> l(g_mu);
    while (!g_stop) {
        g_cv.wait_for(l, period, [] { return g_stop; });
        if (g_stop) break;
        if (!Demand()) continue;
        uint32_t w = 0, h = 0;
        uint64_t frame = 0;
        if (g_opt.synthetic) {
            w = 640; h = 360; frame = ++synthSeq;
            work.resize((size_t)w * h * 4);
            for (uint32_t y = 0; y < h; ++y)
                for (uint32_t x = 0; x < w; ++x) {
                    uint8_t* p = &work[((size_t)y * w + x) * 4];
                    bool bar = ((x + frame * 4) / 40) % 2 == 0;
                    p[0] = (uint8_t)(x * 255 / w); p[1] = (uint8_t)(y * 255 / h); p[2] = bar ? 200 : 40; p[3] = 255;
                }
        } else {
            if (!g_rawFresh) continue;
            g_rawFresh = false;
            work.swap(g_raw);
            w = g_rawW; h = g_rawH; frame = g_rawFrame;
        }
        l.unlock();
        const std::vector<uint8_t>* src = &work;
        uint32_t ow = w, oh = h;
        if (g_opt.scale > 1 && w >= g_opt.scale && h >= g_opt.scale) {
            Downscale(work, w, h, g_opt.scale, scaled, ow, oh);
            src = &scaled;
        }
        jpg.clear();
        bool ok = rt::EncodeJpegRGBA(ow, oh, src->data(), ow * 4, g_opt.quality, jpg) && jpg.size() <= kMaxJpeg;
        l.lock();
        if (ok) {
            g_jpeg.swap(jpg);
            g_jpegSeq++;
            g_jpegFrame = frame;
            g_jpegW = ow; g_jpegH = oh;
            g_cv.notify_all();
        }
    }
}

} // namespace

Options& Cfg() { return g_opt; }

void OnFrame(const uint8_t* rgba, uint32_t stride, uint32_t cw, uint32_t ch, uint64_t frame) {
    if (!g_active.load(std::memory_order_relaxed)) return;
    if (!Demand()) return;
    int64_t now = NowMs();
    if (now - g_lastCopy.load() < (int64_t)(1000 / std::max<uint32_t>(1, g_opt.fps))) return;
    std::unique_lock<std::mutex> l(g_mu, std::try_to_lock);
    if (!l.owns_lock()) return;   // el hilo de codificacion lo tiene: se descarta el fotograma
    g_raw.resize((size_t)cw * ch * 4);
    for (uint32_t y = 0; y < ch; ++y) std::memcpy(&g_raw[(size_t)y * cw * 4], rgba + (size_t)y * stride, (size_t)cw * 4);
    g_rawW = cw; g_rawH = ch; g_rawFrame = frame; g_rawFresh = true;
    g_lastCopy = now;
}

bool WantsFrame() {
    if (!g_active.load(std::memory_order_relaxed) || !Demand()) return false;
    return NowMs() - g_lastCopy.load() >= (int64_t)(1000 / std::max<uint32_t>(1, g_opt.fps));
}

uint16_t Buttons() {
    if (!g_opt.input) return 0;
    if (NowMs() - g_buttonStamp.load() > kKeyTimeoutMs) return 0;
    return g_buttons.load();
}

bool ParseArg(int argc, char** argv, int& i) {
    auto num = [&](uint64_t lo, uint64_t hi, uint64_t def) -> uint64_t {
        if (i + 1 >= argc || argv[i + 1][0] == '-') return def;
        uint64_t v = std::strtoull(argv[++i], nullptr, 10);
        if (v < lo || v > hi) { ImportLog::Get().Printf("viewer: valor fuera de rango %llu (%llu-%llu); se usa %llu", v, lo, hi, def); return def; }
        return v;
    };
    if (!std::strcmp(argv[i], "--viewer")) {
        g_opt.enabled = true;
        g_opt.port = (uint16_t)num(1024, 65535, 8765);
        return true;
    }
    if (!std::strcmp(argv[i], "--viewer-fps")) { g_opt.fps = (uint32_t)num(1, 30, 10); return true; }
    if (!std::strcmp(argv[i], "--viewer-quality")) { g_opt.quality = (uint32_t)num(10, 95, 70); return true; }
    if (!std::strcmp(argv[i], "--viewer-scale")) {
        uint32_t s = (uint32_t)num(1, 4, 1);
        if (s != 1 && s != 2 && s != 4) { ImportLog::Get().Printf("viewer: escala %u no admitida (1, 2 o 4); se usa 1", s); s = 1; }
        g_opt.scale = s;
        return true;
    }
    if (!std::strcmp(argv[i], "--viewer-input")) { g_opt.input = true; return true; }
    if (!std::strcmp(argv[i], "--viewer-synthetic")) { g_opt.synthetic = true; return true; }
    return false;
}

bool Start(const Options& o) {
    g_opt = o;
    if (!o.enabled) return false;
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { ImportLog::Get().Printf("viewer: WSAStartup fallo"); return false; }
    g_wsaStarted = true;
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) { ImportLog::Get().Printf("viewer: socket fallo"); WSACleanup(); g_wsaStarted = false; return false; }
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(o.port);
    inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);
    if (bind(s, (sockaddr*)&a, sizeof(a)) != 0 || listen(s, 4) != 0) {
        ImportLog::Get().Printf("viewer: bind/listen en 127.0.0.1:%u fallo; se continua sin visor", (unsigned)o.port);
        closesocket(s); WSACleanup(); g_wsaStarted = false;
        return false;
    }
    g_port = o.port;
    g_listen = s;
    {
        std::lock_guard<std::mutex> l(g_mu);
        g_stop = false;
    }
    g_active = true;
    g_acceptThread = std::thread(AcceptMain);
    g_encThread = std::thread(EncMain);
    ImportLog::Get().Printf("viewer: escuchando en 127.0.0.1:%u fps=%u calidad=%u", (unsigned)o.port, o.fps, o.quality);
    return true;
}

void Stop() {
    if (!g_active.exchange(false)) return;
    {
        std::lock_guard<std::mutex> l(g_mu);
        g_stop = true;
    }
    g_cv.notify_all();
    if (g_acceptThread.joinable()) g_acceptThread.join();   // sale en <= 200 ms (select)
    closesocket(g_listen);
    g_listen = INVALID_SOCKET;
    {
        std::lock_guard<std::mutex> l(g_sockMu);
        for (SOCKET c : g_sockets) { shutdown(c, SD_BOTH); }
    }
    std::vector<std::thread> cl;
    {
        std::lock_guard<std::mutex> l(g_mu);
        cl.swap(g_clientThreads);
    }
    for (auto& t : cl) if (t.joinable()) t.join();
    if (g_encThread.joinable()) g_encThread.join();
    if (g_wsaStarted) { WSACleanup(); g_wsaStarted = false; }
    ImportLog::Get().Printf("viewer: detenido");
}

} // namespace rt::viewer
