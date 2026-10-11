// Red (XAM): sin red. NetDll_XNetGetTitleXnAddr devuelve XNET_GET_XNADDR_NONE (valor documentado en Xenia
// xam_net.cc: "XNet is uninitialized"; Xenia devuelve STATIC con loopback, decision distinta). Sin XNADDR valido
// el juego dejaba de avanzar sondeando el import (valor 0 = XNET_GET_XNADDR_PENDING, 118 millones de llamadas medidas).
#include "guest_abi.h"
#include "import_log.h"

using namespace rt;

GUEST_IMPORT_IMPL(NetDll_XNetGetTitleXnAddr) {
    ImportScope sc("NetDll_XNetGetTitleXnAddr", ctx, base);
    // (caller, XNADDR* [0x24 bytes: ina, inaOnline, wPortOnline, abEnet[6], abOnline[20]])
    if (ctx.r4.u32) std::memset(base + ctx.r4.u32, 0, 0x24);
    SetRet(ctx, 1); // XNET_GET_XNADDR_NONE
}
