#include "perf.h"
#include "kernel_vars.h"

#include <atomic>
#include <cstring>
#include <thread>

#include <windows.h>

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
static void StartTimeStampBundleTimer();

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
    StartTimeStampBundleTimer();
    return ok;
}

// KeTimeStampBundle (X_TIME_STAMP_BUNDLE: +0 interrupt_time u64, +8 system_time u64, +16 tick_count u32, +20 relleno).
// Xenia (xboxkrnl_module.cc, XboxkrnlModule::XboxkrnlModule): +0/+8 = 0, +16 = Clock::QueryGuestUptimeMillis(),
// actualizado por un temporizador de 1 ms. El GetTickCount del XDK lee +16 (sub_82F42528: lwz r3,16([0x820009A4]));
// sin esta actualizacion el reloj de Bink (sub_83085560) devuelve 0 siempre y BinkWait (sub_830804C8) no libera
// ningun fotograma. Aqui: milisegundos desde el arranque por QueryPerformanceCounter, hilo host con temporizador
// de alta resolucion de 1 ms (Sleep(1) si no esta disponible).
void StartTimeStampBundleTimer() {
    static std::atomic<bool> started{false};
    if (started.exchange(true)) return;
    const uint32_t a = KernelVarAddress("KeTimeStampBundle");
    if (!a) return;
    LARGE_INTEGER f, t0;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&t0);
    auto update = [a, f, t0]() {
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        uint64_t ms = (uint64_t)(t.QuadPart - t0.QuadPart) * 1000ull / (uint64_t)f.QuadPart;
        GuestMemory::Write32(a + 16, (uint32_t)ms);
    };
    update();
    std::thread([update]() {
        rt::perf::NameCurrentThread("timestamp-1ms");
        HANDLE tm = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        if (tm) {
            LARGE_INTEGER due;
            due.QuadPart = -10000; // 1 ms (unidades de 100 ns, relativo)
            SetWaitableTimer(tm, &due, 1, nullptr, nullptr, FALSE);
        }
        ImportLog::Get().Printf("KeTimeStampBundle: temporizador 1 ms %s", tm ? "alta resolucion" : "Sleep(1)");
        for (;;) {
            if (tm) WaitForSingleObject(tm, INFINITE); else Sleep(1);
            update();
        }
    }).detach();
}

} // namespace rt
