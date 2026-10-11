#include "image_loader.h"

#include <cstdio>
#include <cstring>

#include <windows.h>

#include "guest_memory.h"
#include "import_log.h"
#include "ppc_config.h"
#include "ppc_context.h"

namespace rt {

// Destino por defecto de toda entrada sin funcion recompilada: registra y detiene el hilo guest.
static void UnresolvedCall(PPCContext& ctx, uint8_t*) {
    ImportLog::Get().Printf("LLAMADA SIN RESOLVER: ctr=%08X lr=%08X r3=%08X r4=%08X ultimo import=%s", ctx.ctr.u32, (uint32_t)ctx.lr,
                            ctx.r3.u32, ctx.r4.u32, ImportLog::LastImport());
    RaiseException(0xE0000001, EXCEPTION_NONCONTINUABLE, 0, nullptr);
}

bool LoadGameImage(const std::string& imagePath, ImageLoadResult& out, std::string* err) {
    uint8_t* base = GuestMemory::Base();
    if (!GuestMemory::Commit(kImageBase, kImageSize)) { if (err) *err = "commit imagen"; return false; }
    if (!GuestMemory::Commit(kFuncTableBase, kFuncTableSize)) { if (err) *err = "commit tabla de funciones"; return false; }

    FILE* f = std::fopen(imagePath.c_str(), "rb");
    if (!f) { if (err) *err = "no se puede abrir " + imagePath; return false; }
    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (sz <= 0 || (uint64_t)sz > kImageSize) { std::fclose(f); if (err) *err = "tamano de image.bin invalido"; return false; }
    size_t n = std::fread(base + kImageBase, 1, (size_t)sz, f);
    std::fclose(f);
    if (n != (size_t)sz) { if (err) *err = "lectura incompleta de image.bin"; return false; }
    out.fileSize = (uint64_t)sz;
    out.mz = base[kImageBase] == 'M' && base[kImageBase + 1] == 'Z';

    {
        PPCFunc** tab = reinterpret_cast<PPCFunc**>(base + kFuncTableBase);
        size_t n = kFuncTableSize / sizeof(PPCFunc*);
        for (size_t i = 0; i < n; ++i) tab[i] = UnresolvedCall;
    }
    size_t cnt = 0;
    for (const PPCFuncMapping* m = PPCFuncMappings; m->guest != 0; ++m) {
        if (m->guest < PPC_CODE_BASE || m->guest >= PPC_CODE_BASE + PPC_CODE_SIZE) {
            if (err) *err = "entrada de PPCFuncMappings fuera de codigo"; return false;
        }
        PPCFunc** slot = &PPC_LOOKUP_FUNC(base, m->guest);
        *slot = m->host;
        ++cnt;
    }
    out.funcTableEntries = cnt;
    return true;
}

} // namespace rt
