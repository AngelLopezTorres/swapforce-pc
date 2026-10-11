// Entrada (XAM): sin dispositivos reales. Decision propia (no copiada de Xenia, que usa un driver de entrada):
// usuario 0 conectado y en reposo (estado a cero), resto desconectados; sin pulsaciones pendientes.
// Codigos: ERROR_DEVICE_NOT_CONNECTED = 0x48F, ERROR_EMPTY = 0x10D2 (XInputGetKeystroke, Microsoft Learn).
#include "guest_abi.h"
#include "import_log.h"

using namespace rt;

namespace {
constexpr uint32_t kDeviceNotConnected = 0x48F;
constexpr uint32_t kEmpty = 0x10D2;
}

GUEST_IMPORT_IMPL(XamInputGetState) {
    ImportScope sc("XamInputGetState", ctx, base);
    // (user_index, flags, X_INPUT_STATE*)
    uint32_t user = ctx.r3.u32 & 0xFF, st = ctx.r5.u32;
    if (user != 0 && user != 0xFF) { SetRet(ctx, kDeviceNotConnected); return; }
    if (st) std::memset(base + st, 0, 16);
    SetRet(ctx, 0);
}

GUEST_IMPORT_IMPL(XamInputGetKeystrokeEx) {
    ImportScope sc("XamInputGetKeystrokeEx", ctx, base);
    // (uint32* user_index, flags, X_INPUT_KEYSTROKE*): sin pulsaciones pendientes
    SetRet(ctx, kEmpty);
}
