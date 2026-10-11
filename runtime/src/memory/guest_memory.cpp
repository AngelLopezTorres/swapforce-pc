#include "guest_memory.h"

#include <windows.h>
#include <cstring>

namespace rt {

static uint8_t* g_base = nullptr;
static bool g_fixed = false;

using VirtualAlloc2_t = PVOID(WINAPI*)(HANDLE, PVOID, SIZE_T, ULONG, ULONG, MEM_EXTENDED_PARAMETER*, ULONG);
using MapViewOfFile3_t = PVOID(WINAPI*)(HANDLE, HANDLE, PVOID, ULONG64, SIZE_T, ULONG, ULONG, MEM_EXTENDED_PARAMETER*, ULONG);
static bool g_physViews = false;

// Memoria fisica de 512 MB (Xenia memory.cc: las vistas 0xA0000000, 0xC0000000 y 0xE0000000 son vistas del mismo
// mapeo; direccion fisica = addr & 0x1FFFFFFF). Se mapean A0 y C0 sobre una seccion con placeholders
// (Microsoft Learn: VirtualAlloc2, MapViewOfFile3). La vista E0 no se mapea: su desplazamiento de 4 KB
// (Xenia: 0xE0000000 -> fisica + 0x1000) no es reproducible con las macros de carga/almacenamiento del codigo
// recompilado (base + addr) y la granularidad de asignacion de 64 KB.
static bool MapPhysicalViews() {
    HMODULE kb = GetModuleHandleA("kernelbase.dll");
    if (!kb) return false;
    auto va2 = reinterpret_cast<VirtualAlloc2_t>(GetProcAddress(kb, "VirtualAlloc2"));
    auto mv3 = reinterpret_cast<MapViewOfFile3_t>(GetProcAddress(kb, "MapViewOfFile3"));
    if (!va2 || !mv3) return false;
    HANDLE proc = GetCurrentProcess();
    HANDLE sec = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE | SEC_COMMIT, 0, 0x20000000, nullptr);
    if (!sec) return false;
    // Se parte del placeholder de 4 GB reservado en Init().
    constexpr ULONG64 kViewSize = 0x20000000;
    auto split = [&](uint32_t g, uint64_t n) { return VirtualFree(g_base + g, (SIZE_T)n, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER) != 0; };
    if (!split(0xA0000000u, kViewSize) || !split(0xC0000000u, kViewSize)) return false;
    // [0, A0000000) y [E0000000, 4 GB): memoria privada reservada (confirmacion por regiones, como antes).
    if (!va2(proc, g_base, 0xA0000000ull, MEM_RESERVE | MEM_REPLACE_PLACEHOLDER, PAGE_NOACCESS, nullptr, 0)) return false;
    if (!va2(proc, g_base + 0xE0000000u, 0x20000000ull, MEM_RESERVE | MEM_REPLACE_PLACEHOLDER, PAGE_NOACCESS, nullptr, 0)) return false;
    if (!mv3(sec, proc, g_base + 0xA0000000u, 0, kViewSize, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0)) return false;
    if (!mv3(sec, proc, g_base + 0xC0000000u, 0, kViewSize, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0)) return false;
    return true;
}

bool GuestMemory::Init() {
    if (g_base) return true;
    // Reserva de 4 GB como placeholder (para poder sustituir parte por vistas de la seccion fisica).
    HMODULE kb = GetModuleHandleA("kernelbase.dll");
    auto va2 = kb ? reinterpret_cast<VirtualAlloc2_t>(GetProcAddress(kb, "VirtualAlloc2")) : nullptr;
    void* p = nullptr;
    if (va2) {
        p = va2(GetCurrentProcess(), reinterpret_cast<void*>(0x100000000ull), 0x100000000ull, MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, nullptr, 0);
        if (p) g_fixed = true;
        else p = va2(GetCurrentProcess(), nullptr, 0x100000000ull, MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, nullptr, 0);
    }
    bool placeholder = p != nullptr;
    if (!p) {
        p = VirtualAlloc(reinterpret_cast<void*>(0x100000000ull), 0x100000000ull, MEM_RESERVE, PAGE_NOACCESS);
        if (p) g_fixed = true;
        else p = VirtualAlloc(nullptr, 0x100000000ull, MEM_RESERVE, PAGE_NOACCESS);
    }
    if (!p) return false;
    g_base = static_cast<uint8_t*>(p);
    if (placeholder) {
        g_physViews = MapPhysicalViews();
        if (!g_physViews) return false; // el placeholder queda parcialmente dividido: sin vuelta atras
    }
    // Registros GPU 0x7FC80000-0x7FC8FFFF (Xenia graphics_system.cc: AddVirtualMappedRange(0x7FC80000, 0xFFFF0000, ...);
    // indice = (addr & 0xFFFF) / 4). Respaldados por memoria normal (sin trampa de acceso).
    if (!VirtualAlloc(g_base + kGpuRegBase, kGpuRegSize, MEM_COMMIT, PAGE_READWRITE)) return false;
    // Registros XMA 0x7FEA0000-0x7FEAFFFF (Xenia xma_decoder.cc: AddVirtualMappedRange(0x7FEA0000, ...)): memoria normal.
    if (!VirtualAlloc(g_base + kXmaRegBase, kXmaRegSize, MEM_COMMIT, PAGE_READWRITE)) return false;
    return (reinterpret_cast<uintptr_t>(g_base) & 0x1F) == 0;
}

bool GuestMemory::PhysicalViewsMapped() { return g_physViews; }

uint8_t* GuestMemory::Base() { return g_base; }
bool GuestMemory::ReservedAtFixed() { return g_fixed; }

static bool InPhysView(uint32_t g) { return g_physViews && g >= 0xA0000000u && g < 0xE0000000u; }

bool GuestMemory::Commit(uint32_t guest, uint32_t size) {
    if (InPhysView(guest)) { std::memset(g_base + guest, 0, size); return true; } // ya confirmada (seccion); se entrega a cero
    return VirtualAlloc(g_base + guest, size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
}

bool GuestMemory::Decommit(uint32_t guest, uint32_t size) {
    if (InPhysView(guest)) { std::memset(g_base + guest, 0, size); return true; }
    return VirtualFree(g_base + guest, size, MEM_DECOMMIT) != 0;
}

bool GuestMemory::IsInProtectedRange(uint32_t guest, uint32_t size) {
    uint64_t a = guest, b = a + size;
    return a < kFuncTableEnd && b > kImageBase;
}

void GuestMemory::Write32(uint32_t g, uint32_t v) { *reinterpret_cast<uint32_t*>(g_base + g) = __builtin_bswap32(v); }
uint32_t GuestMemory::Read32(uint32_t g) { return __builtin_bswap32(*reinterpret_cast<uint32_t*>(g_base + g)); }
void GuestMemory::Write64(uint32_t g, uint64_t v) { *reinterpret_cast<uint64_t*>(g_base + g) = __builtin_bswap64(v); }
uint64_t GuestMemory::Read64(uint32_t g) { return __builtin_bswap64(*reinterpret_cast<uint64_t*>(g_base + g)); }

} // namespace rt
