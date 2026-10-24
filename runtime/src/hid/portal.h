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

// Traduccion de formato Xbox 360 <-> Portal PC/PS3 (autorizada por el usuario 23/10/2026).
//   None        : paso directo sin traducir (comportamiento anterior).
//   Strip       : OUT: quita la cabecera `0B 14` de lo que emite el juego y rellena a 32 B; IN: sin cambios.
//   StripHdr    : Strip + IN: antepone `0B 14` al informe de 32 B (se truncan los 2 ultimos bytes, siempre 0 salvo contador).
//   StripHdr20  : Strip + IN: `0B 14` + 18 B de informe, longitud entregada 20 (0x14, [NV]).
//   StripHdrLen : Strip + IN: antepone `0B 14 1E` (1E = longitud de carga, [NV]) y trunca a 32 B.
// En OUT (Strip*) solo se reenvian las ordenes R,A,S,Q,C; 'W' (escritura) y las desconocidas se registran y se descartan
// (se devuelve exito al juego).
enum class PortalXlate { None, Strip, StripHdr, StripHdrLen, StripHdr20 };
bool SetPortalXlate(const char* text);   // "none" | "strip" | "strip-hdr" | "strip-hdrlen" | "strip-hdr20"

// Cola de informes IN: "coalesce" (defecto) conserva solo el 'S' mas reciente; "fifo" conserva todos (64 max).
bool SetPortalQueue(const char* text);

// Normalizacion opcional de informes de estado del Portal.
// Stable elimina el bit "cambiado" cuando el informe conserva presencia y posición.
bool SetPortalState(const char* text);   // "raw" | "stable"

// Portal VIRTUAL (solo software, sin USB): reproduce las respuestas R/A/Q grabadas en un registro de ejecucion previa
// (lineas "PORTAL USB dir=IN"). Permite probar el juego sin ocupar el Portal fisico.
bool SetPortalReplay(const char* path);

bool ProbePortal(PortalProbeResult& result);
bool PortalProbeAccepted(const PortalProbeResult& result);

void XamInputNonControllerGetRaw(PPCContext& ctx, uint8_t* base);
void XamInputNonControllerSetRaw(PPCContext& ctx, uint8_t* base);

} // namespace rt::hid
