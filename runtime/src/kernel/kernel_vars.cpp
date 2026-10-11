#include "kernel_vars.h"

#include <cstring>

#include "guest_memory.h"
#include "import_log.h"

namespace rt {

const KernelVarInfo kKernelVars[kKernelVarCount] = {
    {"KeCertMonitorData", 0x266, 0x820007E0, 4},
    {"VdGpuClockInMHz", 0x1C0, 0x82000808, 4},
    {"VdHSIOCalibrationLock", 0x1C1, 0x82000818, 0x40},
    {"VdGlobalXamDevice", 0x1BF, 0x8200088C, 4},
    {"VdGlobalDevice", 0x1BE, 0x82000890, 4},
    {"XboxHardwareInfo", 0x156, 0x82000894, 0x10},
    {"XboxKrnlVersion", 0x158, 0x82000900, 8},
    {"XexExecutableModuleHandle", 0x193, 0x82000974, 4},
    {"ExEventObjectType", 0x00E, 0x82000994, 0x20},
    {"KeTimeStampBundle", 0x0AD, 0x820009A4, 0x18},
    {"ExLoadedCommandLine", 0x1AE, 0x820009C4, 0x400},
    {"KeDebugMonitorData", 0x059, 0x820009E8, 4},
    {"ExThreadObjectType", 0x01B, 0x820009F4, 0x20},
};

constexpr uint32_t kVarArea = 0x85010000;   // datos de variables (0x400 por variable)
constexpr uint32_t kModuleStruct = 0x85020000; // estructura de modulo (LDR) minima

static uint32_t g_addr[kKernelVarCount];

uint32_t KernelVarAddress(const char* name) {
    for (int i = 0; i < kKernelVarCount; ++i)
        if (!std::strcmp(kKernelVars[i].name, name)) return g_addr[i];
    return 0;
}

int InitKernelVars(uint32_t xexHeaderGuest) {
    if (!GuestMemory::Commit(kVarArea, 0x10000) || !GuestMemory::Commit(kModuleStruct, 0x1000)) return -1;
    uint8_t* base = GuestMemory::Base();
    int ok = 0;
    for (int i = 0; i < kKernelVarCount; ++i) {
        const KernelVarInfo& v = kKernelVars[i];
        // El registro del import contiene el ordinal (be32, tipo 0 = variable).
        if (GuestMemory::Read32(v.recordAddr) != v.ordinal) {
            ImportLog::Get().Printf("kernel var %s: registro %08X = %08X, se esperaba ordinal %X", v.name, v.recordAddr,
                                    GuestMemory::Read32(v.recordAddr), v.ordinal);
            continue;
        }
        uint32_t a = kVarArea + i * 0x400;
        g_addr[i] = a;
        std::memset(base + a, 0, v.dataSize);
        GuestMemory::Write32(v.recordAddr, a); // Xenia: *record_slot = kernel_export->variable_ptr
        ++ok;
    }
    // Valores de Xenia (xboxkrnl_module.cc): XboxHardwareInfo flags=0x20, 6 CPU; XboxKrnlVersion.
    uint32_t a = KernelVarAddress("XboxHardwareInfo");
    GuestMemory::Write32(a, 0x20);
    base[a + 4] = 6;
    a = KernelVarAddress("XboxKrnlVersion");
    base[a + 0] = 0; base[a + 1] = 2; base[a + 2] = 0xFF; base[a + 3] = 0xFF; base[a + 4] = 0xFF; base[a + 5] = 0xFF; base[a + 6] = 0x80;
    GuestMemory::Write32(KernelVarAddress("VdGpuClockInMHz"), 500); // Xenia xboxkrnl_video.cc: 500 (Xenos 500 MHz)
    // XexExecutableModuleHandle: la variable contiene un puntero a la estructura de modulo; en ella
    // se guarda la direccion de la cabecera XEX en +0x58 [NV: offset de X_LDR_DATA_TABLE_ENTRY].
    GuestMemory::Write32(KernelVarAddress("XexExecutableModuleHandle"), kModuleStruct);
    GuestMemory::Write32(kModuleStruct + 0x58, xexHeaderGuest);
    return ok;
}

} // namespace rt
