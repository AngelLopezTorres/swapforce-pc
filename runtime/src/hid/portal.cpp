#include "portal.h"

#include <windows.h>
#include <setupapi.h>
#include <winusb.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <sstream>
#include <vector>

#include "guest_abi.h"

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

struct LibUsbKApi {
    HMODULE dll = nullptr;
    UsbKInitializeFn initialize = nullptr;
    UsbKFreeFn free = nullptr;
    UsbKClaimInterfaceFn claimInterface = nullptr;
    UsbKQueryInterfaceSettingsFn queryInterfaceSettings = nullptr;
    UsbKQueryPipeFn queryPipe = nullptr;
    UsbKSetPipePolicyFn setPipePolicy = nullptr;
    UsbKReadPipeFn readPipe = nullptr;

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
        return initialize && free && claimInterface && queryInterfaceSettings && queryPipe && setPipePolicy && readPipe;
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

void XamInputNonControllerGetRaw(PPCContext& ctx, uint8_t*) {
    // ABI y formato del buffer guest pendientes de documentar; no se adivinan punteros guest.
    SetRet(ctx, 0xC00000BB); // STATUS_NOT_SUPPORTED
}

void XamInputNonControllerSetRaw(PPCContext& ctx, uint8_t*) {
    // Escrituras USB desde el juego quedan deshabilitadas; el ABI del export es [NV].
    SetRet(ctx, 0xC00000BB); // STATUS_NOT_SUPPORTED
}

} // namespace rt::hid
