#pragma once
// Registro thread-safe de llamadas a imports del kernel (xboxkrnl/xam).
#include <cstdint>
#include <string>

#include "ppc_config.h"
#include "ppc_context.h"

namespace rt {

extern bool g_importLogTime;   // M60: --log-time
extern bool g_m60Diag;         // M60: diagnosticos de tiempo del kernel (--gpu-timing)
extern bool g_legacySleep;     // M60: --legacy-sleep (xboxkrnl_sync.cpp)

// NTSTATUS usados por el runtime.
constexpr uint32_t ST_SUCCESS = 0;
constexpr uint32_t ST_PENDING = 0x103;
constexpr uint32_t ST_NO_MORE_FILES = 0x80000006;
constexpr uint32_t ST_NOT_IMPLEMENTED = 0xC0000002;
constexpr uint32_t ST_INVALID_HANDLE = 0xC0000008;
constexpr uint32_t ST_END_OF_FILE = 0xC0000011;
constexpr uint32_t ST_OBJECT_NAME_NOT_FOUND = 0xC0000034;
constexpr uint32_t ST_OBJECT_NAME_COLLISION = 0xC0000035;
constexpr uint32_t ST_OBJECT_PATH_NOT_FOUND = 0xC000003A;

struct ImportLogConfig {
    uint32_t maxFullLogsPerName = 25;  // llamadas registradas en detalle por nombre
    bool logAll = false;               // registrar todas las llamadas
    uint64_t maxImports = 0;           // 0 = sin limite; al alcanzarlo, finaliza el proceso
    bool stubsReturnZeroForNt = false; // true: Nt* devuelve 0 en lugar de 0xC0000002
};

class ImportLog {
public:
    static ImportLog& Get();
    bool Open(const std::string& path);
    void Close();
    ImportLogConfig& Config() { return cfg_; }

    // Registra entrada de un import. impl=false: stub. Devuelve true si se registro en detalle.
    void Enter(const char* name, bool impl, const PPCContext& ctx, uint8_t* base);
    void Leave(const char* name, bool impl, const PPCContext& ctx);

    // Mensaje libre (se escribe siempre).
    void Printf(const char* fmt, ...);

    // Ultimo import llamado por hilo y direccion guest conocida.
    static const char* LastImport();
    static void SetGuestAddr(uint32_t a);
    static uint32_t GuestAddr();

    void Summary();
    uint64_t Total() const;
    uint64_t Unique() const;

private:
    ImportLog() = default;
    struct Impl;
    Impl* impl_ = nullptr;
    ImportLogConfig cfg_;
};

// Comportamiento por defecto de un stub segun el nombre.
uint32_t StubDefaultStatus(const char* name);

// Cuerpo comun de un stub: registra y devuelve el valor por defecto en r3.
void ImportStubBody(PPCContext& ctx, uint8_t* base, const char* name);

// Ambito RAII para implementaciones reales: registra entrada y salida (r3).
struct ImportScope {
    ImportScope(const char* n, PPCContext& c, uint8_t* b) : name(n), ctx(c) { ImportLog::Get().Enter(n, true, c, b); }
    ~ImportScope() { ImportLog::Get().Leave(name, true, ctx); }
    const char* name;
    PPCContext& ctx;
};

} // namespace rt

// Stub debil: registra la llamada y devuelve ST_NOT_IMPLEMENTED (Nt*) o 0.
#define GUEST_IMPORT_STUB(X) \
    __attribute__((weak)) PPC_FUNC(__imp__##X) { ::rt::ImportStubBody(ctx, base, #X); }

// Implementacion real (fuerte): usar como cabecera de funcion.
#define GUEST_IMPORT_IMPL(X) PPC_FUNC(__imp__##X)
