// Modulos: XexLoadImage / XexGetModuleHandle / XexGetProcedureAddress / XexUnloadImage (solo modulos del sistema).
// Los modulos del sistema (xam.xex, xboxkrnl.exe, ...) se representan por un handle sin exportaciones
// resolubles: XexGetProcedureAddress devuelve STATUS_DRIVER_ENTRYPOINT_NOT_FOUND (0xC0000225) y escribe 0.
#include <cctype>
#include <string>

#include "guest_abi.h"
#include "import_log.h"
#include "kobject.h"

using namespace rt;

namespace {

std::string GuestCString(uint8_t* base, uint32_t p) {
    std::string s;
    for (int i = 0; p && i < 260 && base[p + i]; ++i) s.push_back((char)tolower((unsigned char)base[p + i]));
    return s;
}

bool IsSystemModule(const std::string& n) {
    return n == "xam.xex" || n == "xboxkrnl.exe" || n == "xbdm.xex" || n == "xapi.xex" || n == "xbdm.dll";
}

} // namespace

GUEST_IMPORT_IMPL(XexLoadImage) {
    ImportScope sc("XexLoadImage", ctx, base);
    // (nombre, flags, version, HANDLE* out)
    std::string name = GuestCString(base, ctx.r3.u32);
    ImportLog::Get().Printf("  XexLoadImage nombre=\"%s\"", name.c_str());
    if (!IsSystemModule(name)) { SetRet(ctx, 0xC0000135); return; } // STATUS_DLL_NOT_FOUND
    auto o = std::make_shared<KObject>(KType::Other);
    uint32_t h = HandleTable::Get().Add(o);
    if (ctx.r6.u32) P32(base, ctx.r6.u32, h);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(XexGetModuleHandle) {
    ImportScope sc("XexGetModuleHandle", ctx, base);
    // (nombre, HANDLE* out)
    std::string name = GuestCString(base, ctx.r3.u32);
    ImportLog::Get().Printf("  XexGetModuleHandle nombre=\"%s\"", name.c_str());
    if (!IsSystemModule(name)) { SetRet(ctx, 0xC0000135); return; }
    auto o = std::make_shared<KObject>(KType::Other);
    if (ctx.r4.u32) P32(base, ctx.r4.u32, HandleTable::Get().Add(o));
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(XexGetProcedureAddress) {
    ImportScope sc("XexGetProcedureAddress", ctx, base);
    // (handle, ordinal o nombre, void** out)
    ImportLog::Get().Printf("  XexGetProcedureAddress handle=%08X ordinal=%08X", ctx.r3.u32, ctx.r4.u32);
    if (ctx.r5.u32) P32(base, ctx.r5.u32, 0);
    SetRet(ctx, 0xC0000225);
}

GUEST_IMPORT_IMPL(XexUnloadImage) {
    ImportScope sc("XexUnloadImage", ctx, base);
    HandleTable::Get().Close(ctx.r3.u32);
    SetRet(ctx, ST_SUCCESS);
}

// ---- XAM: notificaciones (sin eventos) ----
GUEST_IMPORT_IMPL(XamNotifyCreateListener) {
    ImportScope sc("XamNotifyCreateListener", ctx, base);
    auto o = std::make_shared<KObject>(KType::Other);
    SetRet(ctx, HandleTable::Get().Add(o));
}

GUEST_IMPORT_IMPL(XNotifyGetNext) {
    ImportScope sc("XNotifyGetNext", ctx, base);
    SetRet(ctx, 0); // sin notificaciones pendientes
}
