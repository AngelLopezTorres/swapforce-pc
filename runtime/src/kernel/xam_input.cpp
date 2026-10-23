#include <windows.h>
#include <atomic>
#include <cstdint>
#include <cstring>

#include "guest_abi.h"
#include "import_log.h"
#include "viewer/viewer.h"

using namespace rt;
namespace rt { void ConfigureInputStartInjection(uint32_t afterCalls, uint32_t durationCalls); }

namespace {
constexpr uint32_t kDeviceNotConnected = 0x48F;
constexpr uint16_t kDpadUp=0x0001, kDpadDown=0x0002, kDpadLeft=0x0004, kDpadRight=0x0008;
constexpr uint16_t kStart=0x0010, kBack=0x0020, kA=0x1000, kB=0x2000;
struct Gamepad { uint16_t buttons; uint8_t lt, rt; int16_t lx, ly, rx, ry; };
struct State { uint32_t packet; Gamepad pad; };
struct Vibration { uint16_t left, right; };
struct Capabilities { uint8_t type, subtype; uint16_t flags; Gamepad pad; Vibration vibration; };
using GetStateFn = DWORD (WINAPI*)(DWORD, void*);
using SetStateFn = DWORD (WINAPI*)(DWORD, const void*);
using GetCapsFn = DWORD (WINAPI*)(DWORD, DWORD, void*);

struct XInputApi { HMODULE dll=nullptr; GetStateFn get=nullptr; SetStateFn set=nullptr; GetCapsFn caps=nullptr; };
XInputApi& Api() {
    static XInputApi a;
    static INIT_ONCE once=INIT_ONCE_STATIC_INIT;
    InitOnceExecuteOnce(&once, [](PINIT_ONCE, PVOID p, PVOID*) -> BOOL {
        auto* x=static_cast<XInputApi*>(p);
        const wchar_t* names[]={L"xinput1_4.dll",L"xinput1_3.dll",L"xinput9_1_0.dll"};
        for (auto n:names) if ((x->dll=LoadLibraryW(n))) break;
        if (x->dll) { x->get=(GetStateFn)GetProcAddress(x->dll,"XInputGetState"); x->set=(SetStateFn)GetProcAddress(x->dll,"XInputSetState"); x->caps=(GetCapsFn)GetProcAddress(x->dll,"XInputGetCapabilities"); }
        return TRUE;
    }, &a, nullptr);
    return a;
}
std::atomic<uint32_t> g_inputStateCalls{0};
std::atomic<uint32_t> g_startInjectAfterCalls{0};
std::atomic<uint32_t> g_startInjectDurationCalls{0};
bool Key(int vk) { return (GetAsyncKeyState(vk)&0x8000)!=0; }
State KeyboardState() {
    State s{};
    if (Key(VK_UP)) s.pad.buttons|=kDpadUp;
    if (Key(VK_DOWN)) s.pad.buttons|=kDpadDown;
    if (Key(VK_LEFT)) s.pad.buttons|=kDpadLeft;
    if (Key(VK_RIGHT)) s.pad.buttons|=kDpadRight;
    if (Key(VK_RETURN)) s.pad.buttons|=kStart;
    if (Key('Z')) s.pad.buttons|=kA;
    if (Key('X')) s.pad.buttons|=kB;
    if (Key(VK_ESCAPE)) s.pad.buttons|=kBack;
    return s;
}
DWORD ReadState(DWORD user, State& s) {
    if (user>=4) return kDeviceNotConnected;
    auto& a=Api();
    if (a.get && a.get(user,&s)==ERROR_SUCCESS) return ERROR_SUCCESS;
    if (user==0) { s=KeyboardState(); return ERROR_SUCCESS; }
    return kDeviceNotConnected;
}
void StorePad(uint8_t* base, uint32_t at, const Gamepad& p) {
    P16(base,at,p.buttons); base[at+2]=p.lt; base[at+3]=p.rt;
    P16(base,at+4,(uint16_t)p.lx); P16(base,at+6,(uint16_t)p.ly);
    P16(base,at+8,(uint16_t)p.rx); P16(base,at+10,(uint16_t)p.ry);
}
}

void rt::ConfigureInputStartInjection(uint32_t afterCalls, uint32_t durationCalls) {
    g_inputStateCalls.store(0, std::memory_order_relaxed);
    g_startInjectAfterCalls.store(afterCalls, std::memory_order_relaxed);
    g_startInjectDurationCalls.store(durationCalls, std::memory_order_relaxed);
    if (afterCalls && durationCalls)
        ImportLog::Get().Printf("INPUT-INJECT configurado START despues_de=%u llamadas duracion=%u llamadas", afterCalls, durationCalls);
}

GUEST_IMPORT_IMPL(XamInputGetState) {
    ImportScope sc("XamInputGetState",ctx,base);
    State s{}; uint32_t user=ctx.r3.u32&0xFF;
    DWORD rc=ReadState(user,s);
    if (!rc && user == 0) {
        s.pad.buttons |= rt::viewer::Buttons();   // visor en vivo: OR con la entrada local
        const uint32_t call = g_inputStateCalls.fetch_add(1, std::memory_order_relaxed) + 1;
        const uint32_t after = g_startInjectAfterCalls.load(std::memory_order_relaxed);
        const uint32_t duration = g_startInjectDurationCalls.load(std::memory_order_relaxed);
        if (after && duration && call >= after && call - after < duration) {
            s.pad.buttons |= kStart;
            if (call == after) ImportLog::Get().Printf("INPUT-INJECT START activo desde llamada=%u", call);
        }
    }
    if (!rc && ctx.r5.u32) { P32(base,ctx.r5.u32,s.packet); StorePad(base,ctx.r5.u32+4,s.pad); }
    if (!rc) ImportLog::Get().Printf("INPUT user=%u buttons=0x%04X",user,s.pad.buttons);
    SetRet(ctx,rc);
}

GUEST_IMPORT_IMPL(XamInputGetCapabilities) {
    ImportScope sc("XamInputGetCapabilities",ctx,base);
    uint32_t user=ctx.r3.u32&0xFF; Capabilities c{}; c.type=1; c.subtype=1;
    auto& a=Api(); DWORD rc=kDeviceNotConnected;
    if (user<4 && a.caps) rc=a.caps(user,ctx.r4.u32,&c);
    if (rc && user==0) { State s{}; rc=ReadState(user,s); c.pad=s.pad; }
    if (!rc && ctx.r5.u32) {
        base[ctx.r5.u32]=c.type; base[ctx.r5.u32+1]=c.subtype; P16(base,ctx.r5.u32+2,c.flags);
        StorePad(base,ctx.r5.u32+4,c.pad);
        P16(base,ctx.r5.u32+16,c.vibration.left); P16(base,ctx.r5.u32+18,c.vibration.right);
    }
    SetRet(ctx,rc);
}

GUEST_IMPORT_IMPL(XamInputSetState) {
    ImportScope sc("XamInputSetState",ctx,base);
    uint32_t user=ctx.r3.u32&0xFF; DWORD rc=kDeviceNotConnected;
    auto& a=Api();
    if (user<4 && a.set && ctx.r4.u32) {
        Vibration v{G16(base,ctx.r4.u32),G16(base,ctx.r4.u32+2)};
        rc=a.set(user,&v);
    }
    if (user==0 && !a.set) rc=ERROR_SUCCESS; // teclado: vibración sin efecto
    SetRet(ctx,rc);
}

GUEST_IMPORT_IMPL(XamInputGetKeystrokeEx) {
    ImportScope sc("XamInputGetKeystrokeEx",ctx,base);
    SetRet(ctx,0x10D2);
}
