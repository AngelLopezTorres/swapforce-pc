#include "portal.h"

#include <windows.h>
#include <setupapi.h>
#include <winusb.h>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <array>
#include <deque>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include "guest_abi.h"
#include "import_log.h"

namespace rt::hid {
namespace {

using KUSB_HANDLE = void*;

using UsbKInitializeFn = BOOL(WINAPI*)(HANDLE, KUSB_HANDLE*);
using UsbKFreeFn = BOOL(WINAPI*)(KUSB_HANDLE);
using UsbKClaimInterfaceFn = BOOL(WINAPI*)(KUSB_HANDLE, UCHAR, BOOL);
using UsbKQueryInterfaceSettingsFn = BOOL(WINAPI*)(KUSB_HANDLE, UCHAR, PUSB_INTERFACE_DESCRIPTOR);
using UsbKQueryPipeFn = BOOL(WINAPI*)(KUSB_HANDLE, UCHAR, UCHAR, PWINUSB_PIPE_INFORMATION);
using UsbKSetPipePolicyFn = BOOL(WINAPI*)(KUSB_HANDLE, UCHAR, UINT, UINT, PVOID);
using UsbKReadPipeFn = BOOL(WINAPI*)(KUSB_HANDLE, UCHAR, PUCHAR, UINT, PUINT, LPOVERLAPPED);
using UsbKWritePipeFn = BOOL(WINAPI*)(KUSB_HANDLE, UCHAR, PUCHAR, UINT, PUINT, LPOVERLAPPED);
using UsbKAbortPipeFn = BOOL(WINAPI*)(KUSB_HANDLE, UCHAR);
// Firma según el encabezado de libusbK (UsbK_ControlTransfer): SetupPacket por valor. [NV] no contrastada con el encabezado en este equipo.
using UsbKControlTransferFn = BOOL(WINAPI*)(KUSB_HANDLE, WINUSB_SETUP_PACKET, PUCHAR, ULONG, PULONG, LPOVERLAPPED);

struct LibUsbKApi {
    HMODULE dll = nullptr;
    UsbKInitializeFn initialize = nullptr;
    UsbKFreeFn free = nullptr;
    UsbKClaimInterfaceFn claimInterface = nullptr;
    UsbKQueryInterfaceSettingsFn queryInterfaceSettings = nullptr;
    UsbKQueryPipeFn queryPipe = nullptr;
    UsbKSetPipePolicyFn setPipePolicy = nullptr;
    UsbKReadPipeFn readPipe = nullptr;
    UsbKWritePipeFn writePipe = nullptr;
    UsbKAbortPipeFn abortPipe = nullptr;
    UsbKControlTransferFn controlTransfer = nullptr;   // opcional: solo modos control

    bool Load() {
        dll = LoadLibraryExW(L"libusbK.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!dll) return false;
        initialize = reinterpret_cast<UsbKInitializeFn>(GetProcAddress(dll, "UsbK_Initialize"));
        free = reinterpret_cast<UsbKFreeFn>(GetProcAddress(dll, "UsbK_Free"));
        claimInterface = reinterpret_cast<UsbKClaimInterfaceFn>(GetProcAddress(dll, "UsbK_ClaimInterface"));
        queryInterfaceSettings = reinterpret_cast<UsbKQueryInterfaceSettingsFn>(GetProcAddress(dll, "UsbK_QueryInterfaceSettings"));
        queryPipe = reinterpret_cast<UsbKQueryPipeFn>(GetProcAddress(dll, "UsbK_QueryPipe"));
        setPipePolicy = reinterpret_cast<UsbKSetPipePolicyFn>(GetProcAddress(dll, "UsbK_SetPipePolicy"));
        readPipe = reinterpret_cast<UsbKReadPipeFn>(GetProcAddress(dll, "UsbK_ReadPipe"));
        writePipe = reinterpret_cast<UsbKWritePipeFn>(GetProcAddress(dll, "UsbK_WritePipe"));
        abortPipe = reinterpret_cast<UsbKAbortPipeFn>(GetProcAddress(dll, "UsbK_AbortPipe"));
        controlTransfer = reinterpret_cast<UsbKControlTransferFn>(GetProcAddress(dll, "UsbK_ControlTransfer"));
        return initialize && free && claimInterface && queryInterfaceSettings && queryPipe && setPipePolicy && readPipe && writePipe && abortPipe;
    }

    ~LibUsbKApi() { if (dll) FreeLibrary(dll); }
};

struct DeviceInfoList {
    HDEVINFO value = INVALID_HANDLE_VALUE;
    ~DeviceInfoList() { if (value != INVALID_HANDLE_VALUE) SetupDiDestroyDeviceInfoList(value); }
};

struct UsbDeviceHandle {
    HANDLE file = INVALID_HANDLE_VALUE;
    KUSB_HANDLE usb = nullptr;
    LibUsbKApi* api = nullptr;
    ~UsbDeviceHandle() {
        if (usb && api && api->free) api->free(usb);
        if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    }
};

constexpr GUID kGuidDevInterfaceUsbDevice =
    {0xA5DCBF10, 0x6530, 0x11D2, {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED}};
constexpr DWORD kPipeTransferTimeout = 3;
constexpr DWORD kTransferTimeoutMs = 250;
constexpr UCHAR kUsbDirectionIn = 0x80;
constexpr USBD_PIPE_TYPE kUsbInterruptPipe = static_cast<USBD_PIPE_TYPE>(3);

bool HasPortalVidPid(const std::wstring& path) {
    constexpr wchar_t token[] = L"VID_1430&PID_0150";
    auto it = std::search(path.begin(), path.end(), std::begin(token), std::end(token) - 1,
                          [](wchar_t a, wchar_t b) { return towupper(a) == towupper(b); });
    return it != path.end();
}

bool FindPortalPath(std::wstring& path, DWORD& error) {
    DeviceInfoList devices;
    devices.value = SetupDiGetClassDevsW(&kGuidDevInterfaceUsbDevice, nullptr, nullptr,
                                          DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devices.value == INVALID_HANDLE_VALUE) { error = GetLastError(); return false; }

    for (DWORD index = 0;; ++index) {
        SP_DEVICE_INTERFACE_DATA interfaceData{};
        interfaceData.cbSize = sizeof(interfaceData);
        if (!SetupDiEnumDeviceInterfaces(devices.value, nullptr, &kGuidDevInterfaceUsbDevice,
                                         index, &interfaceData)) {
            error = GetLastError();
            return false;
        }

        DWORD required = 0;
        SetupDiGetDeviceInterfaceDetailW(devices.value, &interfaceData, nullptr, 0, &required, nullptr);
        DWORD detailError = GetLastError();
        if (detailError != ERROR_INSUFFICIENT_BUFFER || required < sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W)) {
            error = detailError;
            continue;
        }

        std::vector<uint8_t> storage(required);
        auto* detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(storage.data());
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
        if (!SetupDiGetDeviceInterfaceDetailW(devices.value, &interfaceData, detail, required, nullptr, nullptr)) {
            error = GetLastError();
            continue;
        }

        std::wstring candidate(detail->DevicePath);
        if (HasPortalVidPid(candidate)) { path = std::move(candidate); error = ERROR_SUCCESS; return true; }
    }
}

} // namespace

bool ProbePortal(PortalProbeResult& result) {
    result = {};
    std::wstring path;
    DWORD error = ERROR_SUCCESS;
    if (!FindPortalPath(path, error)) {
        result.error = error;
        result.detail = "dispositivo VID_1430&PID_0150 no encontrado";
        return false;
    }
    result.found = true;

    LibUsbKApi api;
    if (!api.Load()) {
        result.error = api.dll ? ERROR_PROC_NOT_FOUND : GetLastError();
        result.detail = "libusbK.dll o sus exportaciones no disponibles en System32";
        return false;
    }

    UsbDeviceHandle device;
    device.api = &api;
    device.file = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
    if (device.file == INVALID_HANDLE_VALUE) {
        result.error = GetLastError();
        result.detail = "CreateFile del Portal fallo";
        return false;
    }
    result.opened = true;

    if (!api.initialize(device.file, &device.usb)) {
        result.error = GetLastError();
        result.detail = "UsbK_Initialize fallo";
        return false;
    }

    USB_INTERFACE_DESCRIPTOR interfaceDescriptor{};
    if (!api.queryInterfaceSettings(device.usb, 0, &interfaceDescriptor)) {
        result.error = GetLastError();
        result.detail = "UsbK_QueryInterfaceSettings fallo";
        return false;
    }
    if (interfaceDescriptor.bInterfaceNumber != 0) {
        if (!api.claimInterface(device.usb, 0, FALSE) ||
            !api.queryInterfaceSettings(device.usb, 0, &interfaceDescriptor)) {
            result.error = GetLastError();
            result.detail = "no se pudo seleccionar la interfaz USB 0";
            return false;
        }
    }
    result.interfaceZero = interfaceDescriptor.bInterfaceNumber == 0;
    if (!result.interfaceZero) {
        result.error = ERROR_NOT_FOUND;
        result.detail = "la primera interfaz no es la interfaz USB 0";
        return false;
    }

    WINUSB_PIPE_INFORMATION inputPipe{};
    bool foundPipe = false;
    for (UCHAR i = 0; i < interfaceDescriptor.bNumEndpoints; ++i) {
        WINUSB_PIPE_INFORMATION candidate{};
        if (!api.queryPipe(device.usb, interfaceDescriptor.bAlternateSetting, i, &candidate)) continue;
        if (candidate.PipeType == kUsbInterruptPipe && (candidate.PipeId & kUsbDirectionIn)) {
            inputPipe = candidate;
            foundPipe = true;
            break;
        }
    }
    if (!foundPipe) {
        result.error = ERROR_NOT_FOUND;
        result.detail = "interfaz USB 0 sin endpoint interrupt IN";
        return false;
    }
    result.interruptInFound = true;
    result.endpoint = inputPipe.PipeId;

    UINT timeout = kTransferTimeoutMs;
    if (!api.setPipePolicy(device.usb, inputPipe.PipeId, kPipeTransferTimeout,
                           sizeof(timeout), &timeout)) {
        result.error = GetLastError();
        result.detail = "UsbK_SetPipePolicy fallo";
        return false;
    }

    std::vector<UCHAR> packet(std::max<USHORT>(inputPipe.MaximumPacketSize, 1));
    UINT transferred = 0;
    result.readAttempted = true;
    if (api.readPipe(device.usb, inputPipe.PipeId, packet.data(), static_cast<UINT>(packet.size()),
                     &transferred, nullptr)) {
        result.readSucceeded = true;
        result.bytesRead = transferred;
        result.detail = "lectura interrupt IN completada";
        return true;
    }

    result.error = GetLastError();
    result.readTimedOut = result.error == ERROR_SEM_TIMEOUT || result.error == ERROR_TIMEOUT;
    result.detail = result.readTimedOut ? "lectura interrupt IN agotada tras 250 ms" : "UsbK_ReadPipe fallo";
    return result.readTimedOut;
}

bool PortalProbeAccepted(const PortalProbeResult& result) {
    return result.found && result.opened && result.interfaceZero && result.interruptInFound &&
           result.readAttempted && (result.readSucceeded || result.readTimedOut);
}

static void LogRawCall(const char* name, PPCContext& ctx, uint8_t* base) {
    static std::atomic<uint64_t> getCalls{0}, setCalls{0};
    uint64_t call = (name[0] == 'G' ? getCalls : setCalls).fetch_add(1) + 1;
    // El juego sondea GetRaw miles de veces por minuto. Conservar las primeras
    // muestras y una por cada 1024 llamadas limita el volumen de la traza.
    if (call > 8 && (call & 1023) != 0) return;
    const uint32_t args[] = {ctx.r3.u32, ctx.r4.u32, ctx.r5.u32, ctx.r6.u32,
                             ctx.r7.u32, ctx.r8.u32, ctx.r9.u32, ctx.r10.u32};
    ImportLog::Get().Printf("PORTAL %s call=%llu r3..r10=%08X,%08X,%08X,%08X,%08X,%08X,%08X,%08X",
                            name, static_cast<unsigned long long>(call), args[0], args[1], args[2], args[3],
                            args[4], args[5], args[6], args[7]);
    for (unsigned i = 0; i < 8; ++i) {
        if (args[i] < 0x10000u || args[i] > 0xFFFFFFF0u) continue;
        const uint8_t* p = base + args[i];
        MEMORY_BASIC_INFORMATION region{};
        if (!VirtualQuery(p, &region, sizeof(region)) || region.State != MEM_COMMIT ||
            (region.Protect & (PAGE_GUARD | PAGE_NOACCESS))) continue;
        uintptr_t available = reinterpret_cast<uintptr_t>(region.BaseAddress) +
                              region.RegionSize - reinterpret_cast<uintptr_t>(p);
        unsigned n = static_cast<unsigned>(std::min<uintptr_t>(available, 16));
        char hex[33]{};
        for (unsigned j = 0; j < n; ++j)
            std::snprintf(hex + j * 2, sizeof(hex) - j * 2, "%02X", p[j]);
        ImportLog::Get().Printf("PORTAL %s r%u=%08X bytes[%u]=%s", name, i + 3,
                                args[i], n, hex);
    }
}

namespace {

PortalWriteMode g_writeModeSetting = PortalWriteMode::Auto;   // --portal-write
bool g_coalesceS = true;   // --portal-queue=coalesce|fifo: descarta informes 'S' antiguos pendientes al llegar otro 'S'
std::string g_replayPath;   // --portal-replay <registro>: Portal VIRTUAL (solo software) que reproduce respuestas grabadas
PortalXlate g_xlate = PortalXlate::StripHdr;                      // --portal-xlate (defecto provisional; ver PROGRESS.md)
enum class PortalStateMode { Raw, Stable };
PortalStateMode g_stateMode = PortalStateMode::Stable;   // --portal-state=raw|stable
uint32_t PortalStateStatusOffset() {
    if (g_xlate == PortalXlate::StripHdr || g_xlate == PortalXlate::StripHdr20) return 3;
    if (g_xlate == PortalXlate::StripHdrLen) return 4;
    return 1;
}
// Valor de Auto para VID_1430&PID_0150 (dispositivo de este equipo). Fijado tras medir (ver PROGRESS.md, Portal escrituras).
constexpr PortalWriteMode kAutoDefault0150 = PortalWriteMode::ControlExact;

// ---- P2: dispositivo Portal con hilo de E/S propio (paso directo, sin comandos propios) ----
// Valores de retorno de Xenia (xam_input.cc): INVALID_PARAMETER, DEVICE_NOT_CONNECTED.
// ERROR_BUSY (0xAA) para cola de escritura llena: valor no contrastado con Xenia [NV].
constexpr uint32_t kPortalOk = 0;
constexpr uint32_t kPortalInvalidParameter = 0x57;
constexpr uint32_t kPortalNotConnected = 0x48F;
constexpr uint32_t kPortalBusy = 0xAA;
constexpr uint32_t kPortalReportSize = 0x20;      // kPortalBufferSize de Xenia (portal.h)
constexpr DWORD kPortalIoTimeoutMs = 100;          // timeout = 100 de Xenia (hardware_portal.h)
constexpr size_t kPortalReportCap = 64;
constexpr size_t kPortalWriteCap = 16;

struct PortalPacket {
    uint8_t type = 0;
    uint32_t len = 0;
    uint8_t data[kPortalReportSize]{};
    uint8_t stateRawStatus = 0;
    bool stateTransition = false, stateArrival = false, stateChangedMasked = false;
};

struct PortalStateEvent {
    bool log = false;
    uint8_t raw = 0, delivered = 0;
    bool present = false, transition = false, masked = false, summary = false;
    uint64_t reportsDelivered = 0, changedDelivered = 0;
    uint64_t arrivalsDelivered = 0;
};

struct PortalQueueMetrics {
    uint64_t reportsReceived = 0, changedReceived = 0;
    uint64_t changedMasked = 0;
    uint64_t coalescedS = 0, coalescedChanged = 0;
    uint64_t droppedS = 0, droppedChanged = 0;
};

uint8_t PortalStateSignature(uint8_t status) {
    // Bits observados como presencia/posición: 0x01, 0x04 y 0x08.
    return static_cast<uint8_t>(status & 0x0Du);
}

bool PortalStatePresent(uint8_t status) {
    return (status & 0x01u) != 0 || (status & 0x0Cu) != 0;
}

bool IsDisconnectError(DWORD e) {
    return e == ERROR_DEVICE_NOT_CONNECTED || e == ERROR_GEN_FAILURE ||
           e == ERROR_NO_SUCH_DEVICE || e == ERROR_FILE_NOT_FOUND;
}

std::string HexOf(const uint8_t* p, uint32_t n) {
    static const char digits[] = "0123456789ABCDEF";
    std::string s;
    for (uint32_t i = 0; i < n; ++i) { s.push_back(digits[p[i] >> 4]); s.push_back(digits[p[i] & 15]); }
    return s;
}

class PortalIo {
public:
    static PortalIo& Get() {
        static PortalIo* io = [] { auto* p = new PortalIo(); p->Start(); return p; }();  // sin destructor: el proceso termina sin unir hilos
        return *io;
    }

    std::atomic<bool> connected{false};
    std::mutex readMu;          // equivalente al lock_ de Portal::Read de Xenia
    uint32_t prevStatus = 0;    // guardado bajo readMu

    bool PopReport(PortalPacket& out) {
        std::lock_guard<std::mutex> lock(queueMu_);
        if (reports_.empty()) return false;
        out = reports_.front();
        reports_.pop_front();
        return true;
    }

    void NormalizeStateReport(PortalPacket& report, PortalStateEvent& event) {
        if (report.type != 0x53) return;
        const uint32_t field = PortalStateStatusOffset();
        if (field >= report.len) return;

        const uint8_t status = report.data[field];
        const uint8_t rawStatus = report.stateRawStatus;
        const bool present = PortalStatePresent(status);
        const bool transition = report.stateTransition;
        const bool changed = (status & 0x02u) != 0;
        ++stateReportsDelivered_;
        if (changed) ++stateChangedDelivered_;
        const uint8_t delivered = status;
        if ((rawStatus & 0x02u) && !report.stateChangedMasked && report.stateArrival && present)
            ++stateArrivalsDelivered_;
        const bool summary = stateReportsDelivered_ == 1 || stateReportsDelivered_ % 1000 == 0;
        if (changed || transition || report.stateChangedMasked || summary) {
            event.log = true;
            event.raw = rawStatus;
            event.delivered = delivered;
            event.present = present;
            event.transition = transition;
            event.masked = report.stateChangedMasked;
            event.summary = summary;
            event.reportsDelivered = stateReportsDelivered_;
            event.changedDelivered = stateChangedDelivered_;
            event.arrivalsDelivered = stateArrivalsDelivered_;
        }
    }

    void ReadQueueMetrics(PortalQueueMetrics& metrics) {
        std::lock_guard<std::mutex> lock(queueMu_);
        metrics.reportsReceived = stateReportsReceived_;
        metrics.changedReceived = stateChangedReceived_;
        metrics.changedMasked = stateChangedMasked_;
        metrics.coalescedS = coalescedS_;
        metrics.coalescedChanged = coalescedChanged_;
        metrics.droppedS = droppedS_;
        metrics.droppedChanged = droppedChanged_;
    }

    bool PushWrite(const uint8_t* data, uint32_t len) {
        std::lock_guard<std::mutex> lock(queueMu_);
        if (writes_.size() >= kPortalWriteCap) return false;
        PortalPacket p;
        p.len = len;
        std::memcpy(p.data, data, len);   // copia antes de devolver el control al juego
        writes_.push_back(p);
        return true;
    }

private:
    void Start() { std::thread([this] { Run(); }).detach(); }

    bool PopWrite(PortalPacket& out) {
        std::lock_guard<std::mutex> lock(queueMu_);
        if (writes_.empty()) return false;
        out = writes_.front();
        writes_.pop_front();
        return true;
    }

    void PushReport(const PortalPacket& p) {
        std::lock_guard<std::mutex> lock(queueMu_);
        PortalPacket queued = p;
        if (queued.type == 0x53) {
            ++stateReportsReceived_;
            const uint32_t field = PortalStateStatusOffset();
            if (field < queued.len) {
                const uint8_t status = queued.data[field];
                const bool changed = (status & 0x02u) != 0;
                const uint8_t signature = PortalStateSignature(status);
                const bool transition = !sourceStateKnown_ || signature != sourceStateSignature_;
                const bool present = PortalStatePresent(status);
                queued.stateRawStatus = status;
                queued.stateTransition = transition;
                queued.stateArrival = present && (!sourceStateKnown_ || !sourceStatePresent_);
                if (changed) ++stateChangedReceived_;
                if (g_stateMode == PortalStateMode::Stable && changed && sourceStateKnown_ && !transition) {
                    queued.data[field] = static_cast<uint8_t>(status & ~0x02u);
                    queued.stateChangedMasked = true;
                    ++stateChangedMasked_;
                }
                sourceStateKnown_ = true;
                sourceStateSignature_ = signature;
                sourceStatePresent_ = present;
            }
        }
        if (g_coalesceS && queued.type == 0x53) {
            // El Portal emite rafagas de informes de estado 'S'; el juego lee uno por llamada (~20/s) y las respuestas
            // (R/A/Q) quedaban ~1-2 s detras en la cola. Se conservan transiciones y candidatos de llegada; el resto
            // de los informes 'S' antiguos se coalesce. Los demás tipos no se tocan.
            for (auto it = reports_.begin(); it != reports_.end();) {
                if (it->type == 0x53) {
                    const uint32_t field = PortalStateStatusOffset();
                    const bool changedCandidate = field < it->len && (it->data[field] & 0x02u);
                    if (it->stateTransition || changedCandidate) { ++it; continue; }
                    if (it->stateRawStatus & 0x02u) ++coalescedChanged_;
                    it = reports_.erase(it);
                    ++coalescedS_;
                } else ++it;
            }
        }
        if (reports_.size() >= kPortalReportCap) {
            auto drop = reports_.begin();
            for (auto it = reports_.begin(); it != reports_.end(); ++it) {
                if (it->type != 0x53) continue;
                const uint32_t field = PortalStateStatusOffset();
                const bool changedCandidate = field < it->len && (it->data[field] & 0x02u);
                if (!it->stateTransition && !changedCandidate) { drop = it; break; }
            }
            const PortalPacket& dropped = *drop;
            if (dropped.type == 0x53) {
                ++droppedS_;
                if (dropped.stateRawStatus & 0x02u) ++droppedChanged_;
            }
            reports_.erase(drop);
            ++droppedReports_;
        }
        reports_.push_back(queued);
    }

    // ---- Portal virtual (--portal-replay): sin USB. Responde a las ordenes R/A/Q del juego con las respuestas que el
    // Portal fisico dio en un registro anterior (lineas "PORTAL USB dir=IN ... data[32]=") y emite estados 'S'
    // (ausente -> llegada -> presente). No escribe nada en ningun dispositivo. ----
    static bool HexLine(const std::string& line, const char* tag, uint8_t* out, uint32_t& n) {
        size_t p = line.find(tag);
        if (p == std::string::npos) return false;
        p += std::strlen(tag);
        n = 0;
        while (p + 1 < line.size() && n < kPortalReportSize && std::isxdigit((unsigned char)line[p]) && std::isxdigit((unsigned char)line[p + 1])) {
            out[n++] = (uint8_t)std::stoul(line.substr(p, 2), nullptr, 16);
            p += 2;
        }
        return n > 0;
    }

    void DeliverBuf(const uint8_t* buf, uint32_t n) {
        PortalPacket p;
        p.len = n; p.type = buf[0];
        std::memcpy(p.data, buf, n);
        if (g_xlate == PortalXlate::StripHdr) {
            uint8_t t[kPortalReportSize]{};
            t[0] = 0x0B; t[1] = 0x14;
            std::memcpy(t + 2, buf, std::min<uint32_t>(n, kPortalReportSize - 2));
            p.len = std::min<uint32_t>(n + 2, kPortalReportSize);
            std::memcpy(p.data, t, kPortalReportSize);
        }
        PushReport(p);
    }

    void RunVirtual() {
        std::map<std::string, std::array<uint8_t, kPortalReportSize>> resp;
        uint8_t sPresent[kPortalReportSize]{};
        bool haveS = false;
        std::ifstream f(g_replayPath);
        std::string line;
        std::string pendKey;   // clave de la ultima orden R/A/Q emitida por el juego, a la espera de su respuesta
        uint8_t pendType = 0;
        while (std::getline(f, line)) {
            const bool isOut = line.find("PORTAL USB dir=OUT") != std::string::npos;
            if (!isOut && line.find("PORTAL USB dir=IN") == std::string::npos) continue;
            uint8_t b[kPortalReportSize]{}; uint32_t n = 0;
            if (!HexLine(line, "data[32]=", b, n) || n < 2) continue;
            if (isOut) {
                if (b[0] == 0x52 || b[0] == 0x41 || b[0] == 0x51) { pendType = b[0]; pendKey.assign((const char*)b, b[0] == 0x51 ? 3 : (b[0] == 0x52 ? 1 : 2)); }
                continue;
            }
            if (b[0] == 0x53) { if (!haveS && (b[1] & 1)) { std::memcpy(sPresent, b, n); haveS = true; } continue; }
            if (pendType && b[0] == pendType && !pendKey.empty()) {      // la respuesta se asocia a la orden pendiente (la de Q puede cambiar el byte de ranura)
                if (!resp.count(pendKey)) { std::array<uint8_t, kPortalReportSize> a{}; std::memcpy(a.data(), b, n); resp[pendKey] = a; }
                pendKey.clear(); pendType = 0;
            }
        }
        ImportLog::Get().Printf("PORTAL VIRTUAL registro=%s respuestas=%zu estado_presente=%s", g_replayPath.c_str(), resp.size(), haveS ? "si" : "no");
        if (!haveS) for (;;) Sleep(1000);
        connected = true;
        const DWORD t0 = GetTickCount();
        uint32_t counter = 0, arrival = 0;
        for (;;) {
            PortalPacket w;
            while (PopWrite(w)) {
                const uint8_t t = w.data[0];
                if (t == 0x41 && w.data[1] <= 1) arrival = 3;      // 'A' (activar): el Portal real responde con 53 03 (presente + cambiado)
                if (t != 0x52 && t != 0x41 && t != 0x51) continue;       // C (color), S y el resto: sin respuesta
                std::string key((const char*)w.data, t == 0x51 ? 3 : (t == 0x52 ? 1 : 2));
                auto it = resp.find(key);
                if (it != resp.end()) DeliverBuf(it->second.data(), kPortalReportSize);
                else ImportLog::Get().Printf("PORTAL VIRTUAL sin respuesta grabada para orden %02X %02X %02X", t, w.data[1], w.data[2]);
            }
            const DWORD age = GetTickCount() - t0;
            uint8_t s[kPortalReportSize]{};
            std::memcpy(s, sPresent, sizeof(s));
            s[5] = (uint8_t)(0x80 + (counter++ & 0x3F));
            if (age < 4000) s[1] = 0x00;                                  // ausente
            else if (age < 4050) s[1] = 0x03;                            // llegada (presente + cambiado)
            else if (arrival) { s[1] = 0x03; --arrival; }                // llegada tras activar
            else s[1] = 0x01;                                            // presente
            DeliverBuf(s, kPortalReportSize);
            Sleep(4);
        }
    }

    void Run() {
        if (!g_replayPath.empty()) { RunVirtual(); return; }
        if (!api_.Load()) {
            ImportLog::Get().Printf("PORTAL USB libusbK.dll o exportaciones no disponibles; Portal desconectado");
            for (;;) Sleep(1000);
        }
        DWORD lastIdleLog = GetTickCount();
        uint32_t idle = 0;
        for (;;) {
            if (!usb_ && !OpenDevice()) { Sleep(500); continue; }
            PortalPacket w;
            while (usb_ && PopWrite(w)) DoWrite(w);
            if (!usb_) continue;
            DoRead(idle);
            DWORD now = GetTickCount();
            if (now - lastIdleLog >= 1000) {
                if (idle) ImportLog::Get().Printf("PORTAL USB IN sin datos (timeout) x%u en ~1 s", idle);
                idle = 0;
                lastIdleLog = now;
            }
        }
    }

    bool OpenDevice() {
        std::wstring path;
        DWORD err = ERROR_SUCCESS;
        if (!FindPortalPath(path, err)) {
            LogOpenFailure(err ? err : 0xFFFFu, "dispositivo VID_1430&PID_0150 no encontrado");
            return false;
        }
        HANDLE f = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                               nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
        if (f == INVALID_HANDLE_VALUE) { LogOpenFailure(GetLastError(), "CreateFile fallo"); return false; }
        KUSB_HANDLE u = nullptr;
        USB_INTERFACE_DESCRIPTOR d{};
        if (!api_.initialize(f, &u) || !api_.queryInterfaceSettings(u, 0, &d)) {
            LogOpenFailure(GetLastError(), "UsbK_Initialize/QueryInterfaceSettings fallo");
            if (u) api_.free(u);
            CloseHandle(f);
            return false;
        }
        if (d.bInterfaceNumber != 0 && (!api_.claimInterface(u, 0, FALSE) || !api_.queryInterfaceSettings(u, 0, &d))) {
            LogOpenFailure(GetLastError(), "no se pudo seleccionar la interfaz USB 0");
            api_.free(u);
            CloseHandle(f);
            return false;
        }
        // Descriptor de interfaz y endpoints tal como están en la configuración cacheada (sin transferencias).
        ImportLog::Get().Printf("PORTAL USB descriptor interfaz=%u clase=%02X subclase=%02X protocolo=%02X bNumEndpoints=%u alt=%u",
                                d.bInterfaceNumber, d.bInterfaceClass, d.bInterfaceSubClass, d.bInterfaceProtocol,
                                d.bNumEndpoints, d.bAlternateSetting);
        WINUSB_PIPE_INFORMATION in{}, out{};
        bool hasIn = false, hasOut = false;
        for (UCHAR i = 0; i < d.bNumEndpoints; ++i) {
            WINUSB_PIPE_INFORMATION c{};
            if (!api_.queryPipe(u, d.bAlternateSetting, i, &c)) continue;
            ImportLog::Get().Printf("PORTAL USB endpoint 0x%02X tipo=%d mps=%u intervalo=%u dir=%s",
                                    c.PipeId, static_cast<int>(c.PipeType), c.MaximumPacketSize, c.Interval,
                                    (c.PipeId & kUsbDirectionIn) ? "IN" : "OUT");
            if (c.PipeType != kUsbInterruptPipe) continue;
            if (c.PipeId & kUsbDirectionIn) { if (!hasIn) { in = c; hasIn = true; } }
            else if (!hasOut) { out = c; hasOut = true; }
        }
        // Modo de escritura: explícito por CLI o automático según VID:PID.
        PortalWriteMode mode = g_writeModeSetting;
        if (mode == PortalWriteMode::Auto)
            mode = HasPortalVidPid(path) ? kAutoDefault0150 : PortalWriteMode::Interrupt;   // insensible a mayusculas (ruta en minusculas)
        const bool needsOut = mode == PortalWriteMode::Interrupt;
        if (!hasIn || (needsOut && !hasOut)) {
            LogOpenFailure(ERROR_NOT_FOUND, "interfaz USB 0 sin endpoint interrupt IN (y OUT en modo interrupt)");
            api_.free(u);
            CloseHandle(f);
            return false;
        }
        if (mode != PortalWriteMode::Interrupt && !api_.controlTransfer) {
            LogOpenFailure(ERROR_PROC_NOT_FOUND, "UsbK_ControlTransfer no exportada por libusbK.dll");
            api_.free(u);
            CloseHandle(f);
            return false;
        }
        UINT timeout = kPortalIoTimeoutMs;
        bool policyOk = api_.setPipePolicy(u, in.PipeId, kPipeTransferTimeout, sizeof(timeout), &timeout);
        if (policyOk && hasOut && mode == PortalWriteMode::Interrupt)
            policyOk = api_.setPipePolicy(u, out.PipeId, kPipeTransferTimeout, sizeof(timeout), &timeout);
        if (policyOk && mode != PortalWriteMode::Interrupt)   // control por defecto: pipe 0 (sin timeout no bloquea indefinidamente)
            policyOk = api_.setPipePolicy(u, 0, kPipeTransferTimeout, sizeof(timeout), &timeout);
        if (!policyOk) {
            LogOpenFailure(GetLastError(), "UsbK_SetPipePolicy fallo");
            api_.free(u);
            CloseHandle(f);
            return false;
        }
        file_ = f;
        usb_ = u;
        inPipe_ = in.PipeId;
        outPipe_ = hasOut ? out.PipeId : 0;
        writeMode_ = mode;
        lastOpenError_ = 0xFFFFFFFFu;
        connected = true;
        const char* modeName = mode == PortalWriteMode::Interrupt ? "interrupt" :
                               mode == PortalWriteMode::ControlExact ? "control" : "control-pad32";
        ImportLog::Get().Printf("PORTAL USB abierto interfaz=0 in=0x%02X(mps=%u) out=0x%02X(%s) modo_escritura=%s timeout_ms=%u",
                                in.PipeId, in.MaximumPacketSize, outPipe_, hasOut ? "interrupt" : "sin endpoint OUT",
                                modeName, kPortalIoTimeoutMs);
        return true;
    }

    void LogOpenFailure(DWORD err, const char* what) {
        if (err == lastOpenError_) return;   // un aviso por cada cambio de error
        lastOpenError_ = err;
        ImportLog::Get().Printf("PORTAL USB apertura fallida: %s (win32=%lu); reintento cada 500 ms", what,
                                static_cast<unsigned long>(err));
    }

    void CloseDevice(const char* why) {
        connected = false;   // primero: el juego ve la desconexion en el siguiente GetRaw/SetRaw
        if (usb_) {
            api_.abortPipe(usb_, inPipe_);
            if (writeMode_ == PortalWriteMode::Interrupt) api_.abortPipe(usb_, outPipe_);
            api_.free(usb_);
            usb_ = nullptr;
        }
        if (file_ != INVALID_HANDLE_VALUE) { CloseHandle(file_); file_ = INVALID_HANDLE_VALUE; }
        size_t dropped = 0;
        {
            std::lock_guard<std::mutex> lock(queueMu_);
            dropped = writes_.size();
            writes_.clear();
        }
        ImportLog::Get().Printf("PORTAL USB cierre motivo=%s escrituras_descartadas=%llu", why,
                                static_cast<unsigned long long>(dropped));
    }

    void DoWrite(const PortalPacket& w) {
        PortalPacket copy = w;
        UINT transferred = 0;
        const DWORD t0 = GetTickCount();
        BOOL ok = FALSE;
        uint32_t txLen = copy.len;
        char via[96];
        if (writeMode_ == PortalWriteMode::Interrupt) {
            ok = api_.writePipe(usb_, outPipe_, copy.data, copy.len, &transferred, nullptr);
            std::snprintf(via, sizeof(via), "via=interrupt ep=0x%02X", outPipe_);
        } else {
            // SET_REPORT (USB HID 1.11, 7.2.2): informe de salida, id 0, interfaz 0.
            if (writeMode_ == PortalWriteMode::ControlPad32 && txLen < kPortalReportSize) {
                std::memset(copy.data + txLen, 0, kPortalReportSize - txLen);   // relleno a ceros
                txLen = kPortalReportSize;
            }
            WINUSB_SETUP_PACKET setup{};
            setup.RequestType = 0x21;
            setup.Request = 0x09;
            setup.Value = 0x0200;
            setup.Index = 0;
            setup.Length = static_cast<USHORT>(txLen);
            ULONG ctlTransferred = 0;
            ok = api_.controlTransfer(usb_, setup, copy.data, txLen, &ctlTransferred, nullptr);
            transferred = ctlTransferred;
            std::snprintf(via, sizeof(via), "via=control setup=%02X/%02X/%04X/%04X/%04X",
                          setup.RequestType, setup.Request, setup.Value, setup.Index, setup.Length);
        }
        const DWORD err = ok ? ERROR_SUCCESS : GetLastError();
        const uint32_t shown = txLen < kPortalReportSize ? txLen : kPortalReportSize;
        if (ok) {
            ImportLog::Get().Printf("PORTAL USB dir=OUT %s len=%u tx=%u data[%u]=%s res=OK bytes=%u t_ms=%lu",
                                    via, copy.len, txLen, shown, HexOf(copy.data, shown).c_str(), transferred,
                                    static_cast<unsigned long>(GetTickCount() - t0));
        } else {
            ImportLog::Get().Printf("PORTAL USB dir=OUT %s len=%u tx=%u data[%u]=%s res=ERR(win32=%lu) t_ms=%lu",
                                    via, copy.len, txLen, shown, HexOf(copy.data, shown).c_str(),
                                    static_cast<unsigned long>(err), static_cast<unsigned long>(GetTickCount() - t0));
            if (IsDisconnectError(err)) CloseDevice("escritura");
        }
    }

    void DoRead(uint32_t& idle) {
        uint8_t buf[kPortalReportSize]{};
        UINT n = 0;
        BOOL ok = api_.readPipe(usb_, inPipe_, buf, sizeof(buf), &n, nullptr);
        if (ok) {
            if (n == 0) { ++idle; return; }
            PortalPacket p;
            p.len = n;
            p.type = buf[0];
            std::memcpy(p.data, buf, n);
            const bool translated = g_xlate == PortalXlate::StripHdr || g_xlate == PortalXlate::StripHdrLen || g_xlate == PortalXlate::StripHdr20;
            if (translated) {
                uint8_t t[kPortalReportSize]{};
                t[0] = 0x0B; t[1] = 0x14;
                uint32_t off = 2;
                if (g_xlate == PortalXlate::StripHdrLen) { t[2] = 0x1E; off = 3; }
                const uint32_t copyN = std::min<uint32_t>(n, kPortalReportSize - off);
                std::memcpy(t + off, buf, copyN);
                p.len = std::min<uint32_t>(n + off, kPortalReportSize);
                if (g_xlate == PortalXlate::StripHdr20) { p.len = 20; std::memset(t + 20, 0, kPortalReportSize - 20); }   // 0x14 = 20 B totales [NV]
                std::memcpy(p.data, t, kPortalReportSize);
            }
            PushReport(p);
            if (translated)
                ImportLog::Get().Printf("PORTAL USB dir=IN ep=0x%02X len=%u data[%u]=%s res=OK xlate=hdr entregado[%u]=%s", inPipe_, n, n,
                                        HexOf(buf, n).c_str(), p.len, HexOf(p.data, p.len).c_str());
            else
                ImportLog::Get().Printf("PORTAL USB dir=IN ep=0x%02X len=%u data[%u]=%s res=OK xlate=no", inPipe_, n, n,
                                        HexOf(buf, n).c_str());
            return;
        }
        const DWORD err = GetLastError();
        if (err == ERROR_SEM_TIMEOUT || err == ERROR_TIMEOUT) { ++idle; return; }
        ImportLog::Get().Printf("PORTAL USB dir=IN ep=0x%02X res=ERR(win32=%lu)", inPipe_, static_cast<unsigned long>(err));
        if (IsDisconnectError(err)) CloseDevice("lectura");
    }

    LibUsbKApi api_;
    HANDLE file_ = INVALID_HANDLE_VALUE;
    KUSB_HANDLE usb_ = nullptr;
    UCHAR inPipe_ = 0, outPipe_ = 0;
    PortalWriteMode writeMode_ = PortalWriteMode::Interrupt;   // resuelto al abrir
    DWORD lastOpenError_ = 0xFFFFFFFFu;
    std::mutex queueMu_;
    std::deque<PortalPacket> reports_, writes_;
    uint64_t droppedReports_ = 0;
    uint64_t coalescedS_ = 0;
    uint64_t stateReportsReceived_ = 0, stateChangedReceived_ = 0;
    uint64_t stateChangedMasked_ = 0;
    uint64_t coalescedChanged_ = 0, droppedS_ = 0, droppedChanged_ = 0;
    bool sourceStateKnown_ = false, sourceStatePresent_ = false;
    uint8_t sourceStateSignature_ = 0;
    uint64_t stateReportsDelivered_ = 0, stateChangedDelivered_ = 0;
    uint64_t stateArrivalsDelivered_ = 0;   // protegido por readMu
};

} // namespace

// Lectura guest validada (no desreferencia direcciones fuera del bloque comprometido).
static bool GuestReadable(uint8_t* base, uint32_t a, uint32_t n) {
    if (a < 0x10000u || a > 0xFFFFFFF0u) return false;
    MEMORY_BASIC_INFORMATION region{};
    if (!VirtualQuery(base + a, &region, sizeof(region)) || region.State != MEM_COMMIT ||
        (region.Protect & (PAGE_GUARD | PAGE_NOACCESS))) return false;
    uintptr_t available = reinterpret_cast<uintptr_t>(region.BaseAddress) + region.RegionSize -
                          reinterpret_cast<uintptr_t>(base + a);
    return available >= n;
}

// P1: registra el resultado observado tras la llamada (sin cambiar el valor devuelto).
static void LogRawOutcome(const char* name, uint64_t call, uint32_t ret, uint8_t* base,
                          uint32_t lenPtr, uint32_t statePtr, uint32_t bufPtr,
                          uint32_t lenIn, uint32_t stateIn) {
    const uint32_t lenOut = GuestReadable(base, lenPtr, 4) ? G32(base, lenPtr) : 0xFFFFFFFFu;
    const uint32_t stateOut = GuestReadable(base, statePtr, 2) ? G16(base, statePtr) : 0xFFFFu;
    char hex[65]{};
    unsigned n = 0;
    if (GuestReadable(base, bufPtr, 32)) {
        for (; n < 32; ++n) std::snprintf(hex + n * 2, sizeof(hex) - n * 2, "%02X", base[bufPtr + n]);
    }
    ImportLog::Get().Printf("PORTAL %s OUT call=%llu ret=%08X len=%08X->%08X state=%04X->%04X buf[%u]=%s",
                            name, static_cast<unsigned long long>(call), ret, lenIn, lenOut,
                            stateIn, stateOut, n, hex);
}

bool SetPortalQueue(const char* text) {
    if (!text) return false;
    if (!std::strcmp(text, "coalesce")) g_coalesceS = true;
    else if (!std::strcmp(text, "fifo")) g_coalesceS = false;
    else return false;
    return true;
}

bool SetPortalReplay(const char* path) {
    if (!path || !*path) return false;
    g_replayPath = path;
    return true;
}

bool SetPortalState(const char* text) {
    if (!text) return false;
    if (!std::strcmp(text, "raw")) g_stateMode = PortalStateMode::Raw;
    else if (!std::strcmp(text, "stable")) g_stateMode = PortalStateMode::Stable;
    else return false;
    return true;
}

bool SetPortalXlate(const char* text) {
    if (!text) return false;
    if (!std::strcmp(text, "none")) g_xlate = PortalXlate::None;
    else if (!std::strcmp(text, "strip")) g_xlate = PortalXlate::Strip;
    else if (!std::strcmp(text, "strip-hdr")) g_xlate = PortalXlate::StripHdr;
    else if (!std::strcmp(text, "strip-hdrlen")) g_xlate = PortalXlate::StripHdrLen;
    else if (!std::strcmp(text, "strip-hdr20")) g_xlate = PortalXlate::StripHdr20;
    else return false;
    return true;
}

bool SetPortalWriteMode(const char* text) {
    if (!text) return false;
    if (!std::strcmp(text, "auto")) g_writeModeSetting = PortalWriteMode::Auto;
    else if (!std::strcmp(text, "interrupt")) g_writeModeSetting = PortalWriteMode::Interrupt;
    else if (!std::strcmp(text, "control")) g_writeModeSetting = PortalWriteMode::ControlExact;
    else if (!std::strcmp(text, "control-pad32")) g_writeModeSetting = PortalWriteMode::ControlPad32;
    else return false;
    return true;
}

static uint64_t PortalCallCount(bool get) {
    static std::atomic<uint64_t> getCalls{0}, setCalls{0};
    return (get ? getCalls : setCalls).fetch_add(1) + 1;
}

void XamInputNonControllerGetRaw(PPCContext& ctx, uint8_t* base) {
    const uint64_t call = PortalCallCount(true);
    LogRawCall("GetRaw", ctx, base);
    const uint32_t statePtr = ctx.r3.u32, lenPtr = ctx.r4.u32, bufPtr = ctx.r5.u32;
    const bool sampled = call <= 8 || (call & 1023) == 0;
    PortalIo& io = PortalIo::Get();
    uint32_t ret = kPortalOk;
    uint32_t lenIn = 0xFFFFFFFFu, stateIn = 0xFFFFu;
    // Orden de validaciones como en Xenia (XamInputNonControllerGetRawEx_entry).
    if (!statePtr || !lenPtr || !bufPtr || !GuestReadable(base, lenPtr, 4) || !GuestReadable(base, statePtr, 2)) {
        ret = kPortalInvalidParameter;
    } else {
        const uint32_t cap = G32(base, lenPtr);
        lenIn = cap;
        stateIn = G16(base, statePtr);
        if (cap == 0 || cap > kPortalReportSize) {
            ret = kPortalInvalidParameter;
        } else if (!io.connected) {
            ret = kPortalNotConnected;   // Xenia: sin portal no escribe salidas
        } else if (!GuestReadable(base, bufPtr, cap)) {
            ret = kPortalInvalidParameter;
        } else {
            std::lock_guard<std::mutex> lock(io.readMu);
            PortalPacket r;
            uint32_t n = 0;
            if (io.PopReport(r)) {
                PortalStateEvent stateEvent;
                io.NormalizeStateReport(r, stateEvent);
                n = r.len < cap ? r.len : cap;
                std::memcpy(base + bufPtr, r.data, n);
                if (stateEvent.log) {
                    PortalQueueMetrics queueMetrics;
                    io.ReadQueueMetrics(queueMetrics);
                    const char* action = stateEvent.summary && !stateEvent.masked && !stateEvent.transition && !(stateEvent.raw & 0x02u)
                                             ? "resumen"
                                             : (stateEvent.masked ? "changed_enmascarado" : "entregado");
                    const char* mode = g_stateMode == PortalStateMode::Stable ? "stable" : "raw";
                    ImportLog::Get().Printf("PORTAL STATE mode=%s call=%llu raw=%02X entregado=%02X presencia=%s transicion=%u accion=%s s_usb=%llu bit_cambiado_usb=%llu s_juego=%llu bit_cambiado_juego=%llu llegadas_cambiado_entregadas=%llu cambiado_enmascarado=%llu s_coalesced=%llu bit_cambiado_coalesced=%llu s_descartados=%llu bit_cambiado_descartado=%llu",
                                            mode, static_cast<unsigned long long>(call), stateEvent.raw, stateEvent.delivered,
                                            stateEvent.present ? "si" : "no", (unsigned)stateEvent.transition, action,
                                            static_cast<unsigned long long>(queueMetrics.reportsReceived),
                                            static_cast<unsigned long long>(queueMetrics.changedReceived),
                                            static_cast<unsigned long long>(stateEvent.reportsDelivered),
                                            static_cast<unsigned long long>(stateEvent.changedDelivered),
                                            static_cast<unsigned long long>(stateEvent.arrivalsDelivered),
                                            static_cast<unsigned long long>(queueMetrics.changedMasked),
                                            static_cast<unsigned long long>(queueMetrics.coalescedS),
                                            static_cast<unsigned long long>(queueMetrics.coalescedChanged),
                                            static_cast<unsigned long long>(queueMetrics.droppedS),
                                            static_cast<unsigned long long>(queueMetrics.droppedChanged));
                }
                // Registro de lo que consume el juego (se omiten los informes de estado 'S' por volumen).
                const bool hdr = g_xlate == PortalXlate::StripHdr || g_xlate == PortalXlate::StripHdrLen || g_xlate == PortalXlate::StripHdr20;
                const uint8_t type = hdr ? r.data[(g_xlate == PortalXlate::StripHdrLen) ? 3 : 2] : r.data[0];
                if (type != 0x53)
                    ImportLog::Get().Printf("PORTAL XLATE dir=IN entregado_al_juego call=%llu cap=%u len=%u data[%u]=%s",
                                            static_cast<unsigned long long>(call), cap, n, n, HexOf(r.data, n).c_str());
            }
            // Regla de Xenia: state = (estado anterior == estado actual), ambos OK.
            const uint16_t state = io.prevStatus == kPortalOk ? 1 : 0;
            io.prevStatus = kPortalOk;
            P32(base, lenPtr, n);
            P16(base, statePtr, state);
        }
    }
    SetRet(ctx, ret);
    if (sampled) LogRawOutcome("GetRaw", call, ret, base, lenPtr, statePtr, bufPtr, lenIn, stateIn);
}

void XamInputNonControllerSetRaw(PPCContext& ctx, uint8_t* base) {
    const uint64_t call = PortalCallCount(false);
    LogRawCall("SetRaw", ctx, base);
    const uint32_t len = ctx.r3.u32, bufPtr = ctx.r4.u32;
    PortalIo& io = PortalIo::Get();
    uint32_t ret = kPortalOk;
    if (!bufPtr || len == 0 || len > kPortalReportSize || !GuestReadable(base, bufPtr, len)) {
        ret = kPortalInvalidParameter;
    } else if (!io.connected) {
        ret = kPortalNotConnected;
    } else if (g_xlate != PortalXlate::None) {
        // Traduccion Xbox 360 -> Portal: quitar `0B 14`, rellenar a 32 B; solo ordenes de lectura/activacion.
        const uint8_t* g = base + bufPtr;
        const std::string gameHex = HexOf(g, len);
        const char* action = nullptr;
        uint8_t dev[kPortalReportSize]{};
        if (len >= 3 && g[0] == 0x0B && g[1] == 0x14) {
            const uint8_t cmd = g[2];
            if (cmd == 'W') action = "descartada(escritura W: proteccion de la figura)";
            else if (cmd == 'R' || cmd == 'A' || cmd == 'S' || cmd == 'Q' || cmd == 'C') {
                std::memcpy(dev, g + 2, len - 2);
                action = "traducida y reenviada";
            } else action = "descartada(orden desconocida, no identificada como solo lectura)";
        } else {
            action = "descartada(sin cabecera 0B 14 o sin orden)";
        }
        const bool fwd = action[0] == 't';
        ImportLog::Get().Printf("PORTAL XLATE dir=OUT call=%llu game_len=%u game=%s accion=%s%s%s", 
                                static_cast<unsigned long long>(call), len, gameHex.c_str(), action,
                                fwd ? " dev[32]=" : "", fwd ? HexOf(dev, kPortalReportSize).c_str() : "");
        if (fwd && !io.PushWrite(dev, kPortalReportSize)) ret = kPortalBusy;
    } else if (!io.PushWrite(base + bufPtr, len)) {
        ret = kPortalBusy;   // cola llena (16): no se descarta ni reordena ningun paquete
    }
    // Paso directo: los bytes se copian tal cual y los escribe el hilo de E/S (registrado como PORTAL USB dir=OUT).
    SetRet(ctx, ret);
    if (call <= 8 || (call & 1023) == 0) LogRawOutcome("SetRaw", call, ret, base, 0, 0, bufPtr, len, 0);
}

} // namespace rt::hid
