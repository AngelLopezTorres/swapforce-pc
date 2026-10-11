// Imports de vídeo (Vd*, XGetVideoMode). Firmas y valores: Xenia xboxkrnl_video.cc y xam_video.cc
// (copias en runtime\ref\). Solo se implementa lo que el juego importa (import_stubs.gen.cpp).
#include <cstring>

#include "gpu.h"
#include "guest_abi.h"
#include "guest_heap.h"
#include "import_log.h"

using namespace rt;

namespace {

uint32_t F32Bits(float f) { uint32_t u; std::memcpy(&u, &f, 4); return u; }

// X_VIDEO_MODE (48 bytes, big-endian; xbox.h de Xenia).
void FillVideoMode(uint8_t* base, uint32_t p) {
    std::memset(base + p, 0, 48);
    P32(base, p + 0x00, 1280);               // display_width
    P32(base, p + 0x04, 720);                // display_height
    P32(base, p + 0x08, 0);                  // is_interlaced
    P32(base, p + 0x0C, 1);                  // is_widescreen
    P32(base, p + 0x10, 1);                  // is_hi_def
    P32(base, p + 0x14, F32Bits(60.0f));     // refresh_rate
    P32(base, p + 0x18, 1);                  // video_standard (NTSC)
    P32(base, p + 0x1C, 0x4A);               // unknown_0x8a
    P32(base, p + 0x20, 0x01);               // unknown_0x01
}

} // namespace

GUEST_IMPORT_IMPL(XGetVideoMode) {
    ImportScope sc("XGetVideoMode", ctx, base);
    if (ctx.r3.u32) FillVideoMode(base, ctx.r3.u32);
}

GUEST_IMPORT_IMPL(VdQueryVideoMode) {
    ImportScope sc("VdQueryVideoMode", ctx, base);
    if (ctx.r3.u32) FillVideoMode(base, ctx.r3.u32);
}

GUEST_IMPORT_IMPL(VdQueryVideoFlags) {
    ImportScope sc("VdQueryVideoFlags", ctx, base);
    SetRet(ctx, 1 | 2); // widescreen | ancho >= 1024 (1280 < 1920)
}

GUEST_IMPORT_IMPL(VdSetDisplayMode) { ImportScope sc("VdSetDisplayMode", ctx, base); SetRet(ctx, 0); }
GUEST_IMPORT_IMPL(VdSetDisplayModeOverride) { ImportScope sc("VdSetDisplayModeOverride", ctx, base); SetRet(ctx, 0); }
GUEST_IMPORT_IMPL(VdInitializeEngines) { ImportScope sc("VdInitializeEngines", ctx, base); SetRet(ctx, 1); }
GUEST_IMPORT_IMPL(VdShutdownEngines) { ImportScope sc("VdShutdownEngines", ctx, base); }
GUEST_IMPORT_IMPL(VdEnableDisableClockGating) { ImportScope sc("VdEnableDisableClockGating", ctx, base); SetRet(ctx, 0); }
GUEST_IMPORT_IMPL(VdIsHSIOTrainingSucceeded) { ImportScope sc("VdIsHSIOTrainingSucceeded", ctx, base); SetRet(ctx, 1); }
GUEST_IMPORT_IMPL(VdRetrainEDRAMWorker) { ImportScope sc("VdRetrainEDRAMWorker", ctx, base); SetRet(ctx, 0); }
GUEST_IMPORT_IMPL(VdRetrainEDRAM) { ImportScope sc("VdRetrainEDRAM", ctx, base); SetRet(ctx, 0); }
GUEST_IMPORT_IMPL(VdCallGraphicsNotificationRoutines) { ImportScope sc("VdCallGraphicsNotificationRoutines", ctx, base); SetRet(ctx, 0); }
GUEST_IMPORT_IMPL(VdSetSystemCommandBufferGpuIdentifierAddress) { ImportScope sc("VdSetSystemCommandBufferGpuIdentifierAddress", ctx, base); }

GUEST_IMPORT_IMPL(VdGetCurrentDisplayGamma) {
    ImportScope sc("VdGetCurrentDisplayGamma", ctx, base);
    // (uint32* tipo, float* potencia): tipo 2 = BT.709, potencia 2.22222233 (cvars de Xenia)
    if (ctx.r3.u32) P32(base, ctx.r3.u32, 2);
    if (ctx.r4.u32) P32(base, ctx.r4.u32, F32Bits(2.22222233f));
}

GUEST_IMPORT_IMPL(VdGetCurrentDisplayInformation) {
    ImportScope sc("VdGetCurrentDisplayInformation", ctx, base);
    uint32_t p = ctx.r3.u32;
    if (!p) return;
    std::memset(base + p, 0, 0x58);
    P16(base, p + 0x00, 1280); P16(base, p + 0x02, 720);          // front buffer
    P32(base, p + 0x08 + 0x08, 1280); P32(base, p + 0x08 + 0x0C, 720); // scaler_source_rect x2,y2
    P32(base, p + 0x08 + 0x10, 1280); P32(base, p + 0x08 + 0x14, 720); // scaled_output
    P32(base, p + 0x08 + 0x18, 1);                                     // vertical_filter_type
    P32(base, p + 0x08 + 0x28, 1);                                     // horizontal_filter_type
    P16(base, p + 0x40, 320); P16(base, p + 0x42, 180); P16(base, p + 0x44, 320); P16(base, p + 0x46, 180);
    P16(base, p + 0x48, 1280); P16(base, p + 0x4A, 720);
    P32(base, p + 0x4C, F32Bits(60.0f));
    P16(base, p + 0x56, 1280);
}

GUEST_IMPORT_IMPL(VdSetGraphicsInterruptCallback) {
    ImportScope sc("VdSetGraphicsInterruptCallback", ctx, base);
    gpu::SetInterruptCallback(ctx.r3.u32, ctx.r4.u32);
}

GUEST_IMPORT_IMPL(VdInitializeRingBuffer) {
    ImportScope sc("VdInitializeRingBuffer", ctx, base);
    gpu::InitializeRingBuffer(ctx.r3.u32, ctx.r4.u32);
}

GUEST_IMPORT_IMPL(VdEnableRingBufferRPtrWriteBack) {
    ImportScope sc("VdEnableRingBufferRPtrWriteBack", ctx, base);
    gpu::EnableReadPointerWriteBack(ctx.r3.u32, ctx.r4.u32);
}

GUEST_IMPORT_IMPL(VdGetSystemCommandBuffer) {
    ImportScope sc("VdGetSystemCommandBuffer", ctx, base);
    uint32_t p0 = ctx.r3.u32, p1 = ctx.r4.u32;
    if (p0) { std::memset(base + p0, 0, 0x94); P32(base, p0, 0xBEEF0000); }
    if (p1) P32(base, p1, 0xBEEF0001);
}

GUEST_IMPORT_IMPL(VdPersistDisplay) {
    ImportScope sc("VdPersistDisplay", ctx, base);
    // Xenia: *unk1_ptr = bloque de 64 bytes (heap fisico de 16K) que el juego libera con MmFreePhysicalMemory(1, *ptr).
    if (ctx.r4.u32) {
        uint32_t a = PhysicalHeap().Alloc(64, 32);
        P32(base, ctx.r4.u32, a);
    }
    SetRet(ctx, 1);
}

GUEST_IMPORT_IMPL(VdInitializeScalerCommandBuffer) {
    ImportScope sc("VdInitializeScalerCommandBuffer", ctx, base);
    // Argumentos 11 y 12: puntero destino y numero de palabras; se rellena con NOP (0x80000000), como Xenia.
    uint32_t dest = Arg32(ctx, base, 10), count = Arg32(ctx, base, 11);
    if (dest && count < 0x10000) for (uint32_t i = 0; i < count; ++i) P32(base, dest + i * 4, 0x80000000);
    SetRet(ctx, count);
}

GUEST_IMPORT_IMPL(VdSwap) {
    ImportScope sc("VdSwap", ctx, base);
    // (buffer_ptr, fetch_ptr, unk2, unk3, unk4, frontbuffer_ptr, texture_format_ptr, color_space_ptr, width_ptr, height_ptr)
    uint32_t buf = ctx.r3.u32, fetch = ctx.r4.u32, fbPtr = ctx.r8.u32;
    uint32_t wPtr = Arg32(ctx, base, 8), hPtr = Arg32(ctx, base, 9);
    if (!buf || !fetch || !fbPtr || !wPtr || !hPtr) return;
    gpu::RecordSwapCall();
    uint32_t f[6];
    for (int i = 0; i < 6; ++i) f[i] = G32(base, fetch + i * 4);
    uint32_t fbVirt = (f[1] >> 12) << 12;                 // xe_gpu_texture_fetch_t.base_address (dword_1, bits 12-31) << 12
    uint32_t fbPhys = gpu::VirtToPhys(fbVirt);
    f[1] = (f[1] & 0xFFF) | ((fbPhys >> 12) << 12);
    uint32_t w = G32(base, wPtr), h = G32(base, hPtr);
    // 64 palabras del anillo: type0(SHADER_CONSTANT_FETCH_00_0 = 0x4800, 6) + 6 dwords + type3 XE_SWAP(0x64, 4) + NOP type 2.
    uint32_t o = 0;
    for (uint32_t i = 0; i < 64; ++i) P32(base, buf + i * 4, 0);
    P32(base, buf + 4 * o++, (0u << 30) | (5u << 16) | 0x4800);
    for (int i = 0; i < 6; ++i) P32(base, buf + 4 * o++, f[i]);
    P32(base, buf + 4 * o++, (3u << 30) | (3u << 16) | (0x64u << 8));
    P32(base, buf + 4 * o++, 0x53574150); // kSwapSignature = fourcc "SWAP"
    P32(base, buf + 4 * o++, fbPhys);
    P32(base, buf + 4 * o++, w);
    P32(base, buf + 4 * o++, h);
    for (; o < 64; ++o) P32(base, buf + 4 * o, 2u << 30);
}
