// Rtl*: cadenas, relleno/comparacion, conversion de estados, cabecera XEX.
#include <windows.h>

#include <cstring>
#include <cwctype>
#include <string>

#include "guest_abi.h"
#include "guest_thread.h"
#include "import_log.h"
#include "xex_header.h"

using namespace rt;

static constexpr uint32_t ST_BUFFER_TOO_SMALL = 0xC0000023;

GUEST_IMPORT_IMPL(RtlImageXexHeaderField) {
    ImportScope sc("RtlImageXexHeaderField", ctx, base);
    // (header_base, key) -> valor inline o direccion guest de la estructura; 0 si no existe (Xenia).
    uint32_t v = 0, s = 0;
    if (ctx.r3.u32 == kXexHeaderGuest && XexFindOptHeader(ctx.r4.u32, &v, &s)) SetRet(ctx, s ? s : v);
    else SetRet(ctx, 0);
}

// ---- ANSI_STRING / UNICODE_STRING: {u16 Length, u16 MaximumLength, u32 Buffer} (big-endian) ----
GUEST_IMPORT_IMPL(RtlInitAnsiString) {
    ImportScope sc("RtlInitAnsiString", ctx, base);
    uint32_t dst = ctx.r3.u32, src = ctx.r4.u32;
    uint32_t len = 0;
    if (src) while (base[src + len]) ++len;
    if (len > 0xFFFE) len = 0xFFFE;
    P16(base, dst, (uint16_t)len);
    P16(base, dst + 2, src ? (uint16_t)(len + 1) : 0);
    P32(base, dst + 4, src);
}

GUEST_IMPORT_IMPL(RtlInitUnicodeString) {
    ImportScope sc("RtlInitUnicodeString", ctx, base);
    uint32_t dst = ctx.r3.u32, src = ctx.r4.u32;
    uint32_t len = 0;
    if (src) while (G16(base, src + len * 2)) ++len;
    if (len > 0x7FFE) len = 0x7FFE;
    P16(base, dst, (uint16_t)(len * 2));
    P16(base, dst + 2, src ? (uint16_t)(len * 2 + 2) : 0);
    P32(base, dst + 4, src);
}

GUEST_IMPORT_IMPL(RtlFillMemoryUlong) {
    ImportScope sc("RtlFillMemoryUlong", ctx, base);
    // (destino, longitud en bytes, patron)
    uint32_t d = ctx.r3.u32, n = ctx.r4.u32 / 4, v = ctx.r5.u32;
    for (uint32_t i = 0; i < n; ++i) P32(base, d + i * 4, v);
}

GUEST_IMPORT_IMPL(RtlCompareMemoryUlong) {
    ImportScope sc("RtlCompareMemoryUlong", ctx, base);
    // (origen, longitud, patron) -> numero de bytes coincidentes
    uint32_t s = ctx.r3.u32, n = ctx.r4.u32 / 4, v = ctx.r5.u32, i = 0;
    while (i < n && G32(base, s + i * 4) == v) ++i;
    SetRet(ctx, i * 4);
}

GUEST_IMPORT_IMPL(RtlUpcaseUnicodeChar) {
    ImportScope sc("RtlUpcaseUnicodeChar", ctx, base);
    uint16_t c = (uint16_t)ctx.r3.u32;
    SetRet(ctx, (uint16_t)towupper(c));
}

GUEST_IMPORT_IMPL(RtlNtStatusToDosError) {
    ImportScope sc("RtlNtStatusToDosError", ctx, base);
    // Funcion de ntdll del host (misma tabla NT documentada).
    using Fn = ULONG(WINAPI*)(LONG);
    static Fn fn = (Fn)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlNtStatusToDosError");
    SetRet(ctx, fn ? fn((LONG)ctx.r3.u32) : 317);
}

GUEST_IMPORT_IMPL(RtlMultiByteToUnicodeN) {
    ImportScope sc("RtlMultiByteToUnicodeN", ctx, base);
    // (UnicodeString*, MaxBytesInUnicode, BytesInUnicode*, MultiByteString*, BytesInMultiByte): conversion ASCII->UTF-16BE
    uint32_t dst = ctx.r3.u32, maxb = ctx.r4.u32, outp = ctx.r5.u32, src = ctx.r6.u32, n = ctx.r7.u32;
    uint32_t cnt = n < maxb / 2 ? n : maxb / 2;
    for (uint32_t i = 0; i < cnt; ++i) P16(base, dst + i * 2, base[src + i]);
    if (outp) P32(base, outp, cnt * 2);
    SetRet(ctx, ST_SUCCESS);
}

GUEST_IMPORT_IMPL(RtlRaiseException) {
    ImportScope sc("RtlRaiseException", ctx, base);
    uint32_t rec = ctx.r3.u32;
    uint32_t code = G32(base, rec);
    if (code == 0x406D1388) { // convencion MS_VC_EXCEPTION: nombre de hilo
        uint32_t name = G32(base, rec + 0x18);
        std::string s;
        for (int i = 0; name && i < 64 && base[name + i]; ++i) s.push_back((char)base[name + i]);
        uint32_t tid = G32(base, rec + 0x1C);   // THREADNAME_INFO.dwThreadID: 0xFFFFFFFF = hilo que llama; si no, id devuelto por ExCreateThread
        GuestThread* gt = tid == 0xFFFFFFFFu ? GuestThread::Current() : GuestThread::ById(tid);
        if (gt) std::strncpy(gt->dynName, s.c_str(), sizeof(gt->dynName) - 1);
        ImportLog::Get().Printf("  RtlRaiseException 0x406D1388 nombre de hilo \"%s\"", s.c_str());
        return;
    }
    ImportLog::Get().Printf("  RtlRaiseException codigo=%08X flags=%08X addr=%08X", code, G32(base, rec + 4), G32(base, rec + 12));
}

// ---- ExGetXConfigSetting(categoria, ajuste, buffer, tamano, tamano requerido*) (valores de Xenia, xboxkrnl_xconfig.cc) ----
GUEST_IMPORT_IMPL(ExGetXConfigSetting) {
    ImportScope sc("ExGetXConfigSetting", ctx, base);
    uint32_t cat = ctx.r3.u32 & 0xFFFF, set = ctx.r4.u32 & 0xFFFF, buf = ctx.r5.u32, bsz = ctx.r6.u32 & 0xFFFF, req = ctx.r7.u32;
    uint32_t value = 0, size = 4;
    bool ok = false;
    if (cat == 2 && set == 2) { value = 0x1000; ok = true; }                         // AV_REGION
    else if (cat == 3 && set >= 1 && set <= 7) { value = 0; ok = true; }            // TIME_ZONE_*
    else if (cat == 3 && set == 9) { value = 1; ok = true; }                         // LANGUAGE (Xenia: user_language=1) [NV]
    else if (cat == 3 && set == 0xA) { value = 0x00040000; ok = true; }              // VIDEO_FLAGS
    else if (cat == 3 && set == 0xC) { value = 0; ok = true; }                       // RETAIL_FLAGS
    else if (cat == 3 && set == 0xE) { value = 103; size = 1; ok = true; }           // COUNTRY (Xenia: user_country=103) [NV]
    if (!ok) { SetRet(ctx, cat > 3 ? 0xC00000F0 : 0xC00000EF); return; }             // INVALID_PARAMETER_1 / _2
    if (req) P16(base, req, (uint16_t)size);
    if (bsz < size) { SetRet(ctx, ST_BUFFER_TOO_SMALL); return; }
    if (size == 4) P32(base, buf, value); else base[buf] = (uint8_t)value;
    SetRet(ctx, ST_SUCCESS);
}
