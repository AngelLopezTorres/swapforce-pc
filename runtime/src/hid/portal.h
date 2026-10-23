#pragma once

#include <cstdint>
#include <string>

#include "ppc_config.h"
#include "ppc_context.h"

namespace rt::hid {

struct PortalProbeResult {
    bool found = false;
    bool opened = false;
    bool interfaceZero = false;
    bool interruptInFound = false;
    bool readAttempted = false;
    bool readSucceeded = false;
    bool readTimedOut = false;
    uint32_t error = 0;
    uint32_t bytesRead = 0;
    uint8_t endpoint = 0;
    std::string detail;
};

// Medio de transporte de las escrituras del juego (SetRaw). Solo cambia el transporte de los
// mismos bytes; no se envían comandos propios.
//   Auto           : según VID:PID (1430:0150 -> valor medido por defecto; otros -> Interrupt).
//   Interrupt      : UsbK_WritePipe sobre el endpoint interrupt OUT del descriptor.
//   ControlExact   : SET_REPORT (bmRequestType 0x21, bRequest 0x09, wValue 0x0200, wIndex 0) con len exacta.
//   ControlPad32   : igual, rellenando a ceros hasta 32 B (longitud del informe Xenia).
enum class PortalWriteMode { Auto, Interrupt, ControlExact, ControlPad32 };
bool SetPortalWriteMode(const char* text);   // "auto" | "interrupt" | "control" | "control-pad32"

bool ProbePortal(PortalProbeResult& result);
bool PortalProbeAccepted(const PortalProbeResult& result);

void XamInputNonControllerGetRaw(PPCContext& ctx, uint8_t* base);
void XamInputNonControllerSetRaw(PPCContext& ctx, uint8_t* base);

} // namespace rt::hid
