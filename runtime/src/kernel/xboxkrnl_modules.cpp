// Modulos: XexLoadImage / XexGetModuleHandle / XexGetProcedureAddress / XexUnloadImage (solo modulos del sistema).
#include <cctype>
#include <mutex>
#include <string>
#include <unordered_map>

#include "guest_abi.h"
#include "guest_heap.h"
#include "guest_memory.h"
#include "import_log.h"
#include "kobject.h"
#include "../hid/portal.h"

using namespace rt;

namespace {

constexpr uint32_t kXamGetRawGuestAddress = 0x830A9EE0;
constexpr uint32_t kXamSetRawGuestAddress = 0x830A9EE4;
constexpr uint32_t kStatusDriverEntryPointNotFound = 0xC0000225;

std::mutex g_moduleMu;
std::unordered_map<uint32_t, std::string> g_moduleHandles;
std::mutex g_xamExportsMu;

void TrackModule(uint32_t handle, const std::string& name) {
    std::lock_guard<std::mutex> lock(g_moduleMu);
    g_moduleHandles[handle] = name;
}

std::string TrackedModule(uint32_t handle) {
    std::lock_guard<std::mutex> lock(g_moduleMu);
    auto it = g_moduleHandles.find(handle);
    return it == g_moduleHandles.end() ? std::string() : it->second;
}

void ForgetModule(uint32_t handle) {
    std::lock_guard<std::mutex> lock(g_moduleMu);
    g_moduleHandles.erase(handle);
}

bool AddressAlreadyMapped(uint32_t guestAddress) {
    for (const PPCFuncMapping* mapping = PPCFuncMappings; mapping->guest; ++mapping)
        if (mapping->guest == guestAddress) return true;
    return false;
}

bool RegisterXamPortalExports(uint8_t* base) {
    std::lock_guard<std::mutex> lock(g_xamExportsMu);
    if (AddressAlreadyMapped(kXamGetRawGuestAddress) || AddressAlreadyMapped(kXamSetRawGuestAddress)) return false;

    const uint32_t getSlot = static_cast<uint32_t>(PPC_IMAGE_BASE + PPC_IMAGE_SIZE +
        (uint64_t(kXamGetRawGuestAddress - PPC_CODE_BASE) * 2));
    const uint32_t setSlot = static_cast<uint32_t>(PPC_IMAGE_BASE + PPC_IMAGE_SIZE +
        (uint64_t(kXamSetRawGuestAddress - PPC_CODE_BASE) * 2));
    const uint32_t page = getSlot & ~0xFFFu;
    if ((setSlot & ~0xFFFu) != page || !GuestMemory::Commit(page, 0x1000)) return false;

    PPC_LOOKUP_FUNC(base, kXamGetRawGuestAddress) = rt::hid::XamInputNonControllerGetRaw;
    PPC_LOOKUP_FUNC(base, kXamSetRawGuestAddress) = rt::hid::XamInputNonControllerSetRaw;
    return true;
}

uint32_t OrdinalValue(uint32_t value) {
    return (value & 0xFFFF0000u) == 0xFFFF0000u ? value & 0xFFFFu : value;
}

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
    // Xenia devuelve la direccion guest de X_LDR_DATA_TABLE_ENTRY. El juego
    // lee dll_base en +0x18 despues de XexGetProcedureAddress.
    uint32_t h = SystemHeap().Alloc(0x100);
    if (!h) { SetRet(ctx, 0xC0000017); return; }
    TrackModule(h, name);
    if (ctx.r6.u32) P32(base, ctx.r6.u32, h);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(XexGetModuleHandle) {
    ImportScope sc("XexGetModuleHandle", ctx, base);
    // (nombre, HANDLE* out)
    std::string name = GuestCString(base, ctx.r3.u32);
    ImportLog::Get().Printf("  XexGetModuleHandle nombre=\"%s\"", name.c_str());
    if (!IsSystemModule(name)) { SetRet(ctx, 0xC0000135); return; }
    uint32_t h = SystemHeap().Alloc(0x100);
    if (!h) { SetRet(ctx, 0xC0000017); return; }
    TrackModule(h, name);
    if (ctx.r4.u32) P32(base, ctx.r4.u32, h);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(XexGetProcedureAddress) {
    ImportScope sc("XexGetProcedureAddress", ctx, base);
    // (handle, ordinal o nombre, void** out)
    uint32_t handle = ctx.r3.u32;
    uint32_t ordinal = OrdinalValue(ctx.r4.u32);
    ImportLog::Get().Printf("  XexGetProcedureAddress handle=%08X ordinal=%08X", handle, ordinal);
    if (ctx.r5.u32) P32(base, ctx.r5.u32, 0);
    std::string module = TrackedModule(handle);
    if (module == "xam.xex" && ctx.r5.u32) {
        if (ordinal == 0x4A1 || ordinal == 0x4A2) {
            if (!RegisterXamPortalExports(base)) { SetRet(ctx, kStatusDriverEntryPointNotFound); return; }
            P32(base, ctx.r5.u32, ordinal == 0x4A1 ? kXamGetRawGuestAddress : kXamSetRawGuestAddress);
            SetRet(ctx, ST_SUCCESS);
            return;
        }
    }
    SetRet(ctx, kStatusDriverEntryPointNotFound);
}

GUEST_IMPORT_IMPL(XexUnloadImage) {
    ImportScope sc("XexUnloadImage", ctx, base);
    ForgetModule(ctx.r3.u32);
    SystemHeap().Free(ctx.r3.u32);
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
