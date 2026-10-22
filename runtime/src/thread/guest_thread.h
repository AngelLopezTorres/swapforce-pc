#pragma once
// Hilo guest: contexto PPC, PCR, TLS, pila y KTHREAD en memoria guest; ejecutado en un hilo host.
#include <windows.h>

#include <cstdint>
#include <string>

#include "ppc_context.h"

namespace rt {

struct GuestThreadParams {
    uint32_t entry = 0;        // direccion guest de la funcion (se resuelve por la tabla de funciones)
    uint32_t arg = 0;          // r3 inicial
    uint32_t stackSize = 0;    // 0 = valor del XEX o 0x40000
    bool suspended = false;
    const char* name = "guest";
};

class GuestThread {
public:
    static GuestThread* Create(const GuestThreadParams& p, std::string* err = nullptr);
    bool Start();                       // crea el hilo host (si suspended, queda a la espera de Resume)
    void Resume();
    bool Join(uint32_t timeoutMs);      // true si ha terminado
    bool Finished() const { return finished_; }
    int ExitCode() const { return exitCode_; }

    // Contexto guest para un hilo host propio del runtime (GPU, DPC): sin hilo host asociado.
    static GuestThread* CreateAttached(const char* name, std::string* err = nullptr);
    void Attach();                      // el hilo host que llama pasa a ser el hilo guest actual
    void SetCpu(uint32_t cpu);          // PCR+0x10C (numero de CPU)
    // Ejecuta una funcion guest (r3, r4) en el hilo host actual (Attach previo); false si hubo excepcion.
    bool CallGuest(uint32_t addr, uint32_t r3, uint32_t r4, uint32_t* ret = nullptr, uint32_t r5 = 0, uint32_t r6 = 0);

    static GuestThread* Current();
    static GuestThread* ById(uint32_t id);   // hilo guest por id (el que devuelve ExCreateThread); nullptr si no existe
    // Diagnostico: para cada hilo guest vivo, registra RIP host y marcos candidatos (funciones guest).
    static void DumpAllThreads();
    static void DumpWaitStates();       // M8: sin suspender hilos (estado de espera, ultimo import, lr)
    PPCContext& Ctx() { return *ctx_; }

    HANDLE HostHandle() const { return host_; }
    bool IsSuspended() const { return suspendedFlag_; }
    uint32_t ResumeThread();            // devuelve contador previo de suspension
    struct TraceRingPtr { const void* p = nullptr; };
    const void* traceRing = nullptr;    // anillo de llamadas indirectas del hilo
    const char* lastImport = nullptr;   // ultimo import llamado por este hilo
    uint32_t id = 0;
    char dynName[48] = {};              // nombre fijado por el juego (excepcion 0x406D1388)
    volatile uint32_t waitKind = 0;     // 0 = ninguna, 1 = Nt single, 2 = Ke single, 3 = Ke multiple (diagnostico M0)
    volatile uint32_t waitHandle = 0;   // handle (Nt) o puntero guest (Ke) del objeto esperado; en multiple, el primero
    volatile uint32_t waitType = 0;     // KType del objeto
    volatile uint32_t waitCount = 0;
    uint32_t tlsDynamic = 0;            // inicio de los slots dinamicos (KeTls*)
    uint32_t pcr = 0, kthread = 0, tls = 0, tlsSize = 0, stackLow = 0, stackHigh = 0;

private:
    friend DWORD WINAPI GuestThreadProc(void*);
    void Run();
    GuestThreadParams params_;
    PPCContext* ctx_ = nullptr;
    HANDLE host_ = nullptr;
    HANDLE resumeEvent_ = nullptr;
    volatile bool finished_ = false;
    volatile bool suspendedFlag_ = false;
    int exitCode_ = 0;
};

uint32_t XexDefaultStackSize();

} // namespace rt
