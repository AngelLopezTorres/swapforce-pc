// GENERADO por runtime/recomp/cputest/gen_cputest.py (no editar). Codigo producido por XenonRecomp parcheado para
// palabras de instruccion sinteticas; no contiene codigo del juego.
#include "cpu_selftest.h"
#define PPC_CONFIG_H_INCLUDED
#include <ppc_context.h>

#define __builtin_debugtrap()

__attribute__((alias("__imp__cputest_10000"))) PPC_WEAK_FUNC(cputest_10000);
PPC_FUNC_IMPL(__imp__cputest_10000) {
	PPC_FUNC_PROLOGUE();
	// addc r3,r4,r5
	ctx.xer.ca = (ctx.r4.u64 + ctx.r5.u64) < ctx.r4.u64;
	ctx.r3.u64 = ctx.r4.u64 + ctx.r5.u64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10040"))) PPC_WEAK_FUNC(cputest_10040);
PPC_FUNC_IMPL(__imp__cputest_10040) {
	PPC_FUNC_PROLOGUE();
	// mulhdu r3,r4,r5
	ctx.r3.u64 = uint64_t((__uint128_t(ctx.r4.u64) * __uint128_t(ctx.r5.u64)) >> 64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10080"))) PPC_WEAK_FUNC(cputest_10080);
PPC_FUNC_IMPL(__imp__cputest_10080) {
	PPC_FUNC_PROLOGUE();
	// eqv r4,r3,r5
	ctx.r4.u64 = ~(ctx.r3.u64 ^ ctx.r5.u64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_100C0"))) PPC_WEAK_FUNC(cputest_100C0);
PPC_FUNC_IMPL(__imp__cputest_100C0) {
	PPC_FUNC_PROLOGUE();
	// addc. r3,r4,r5
	ctx.xer.ca = (ctx.r4.u64 + ctx.r5.u64) < ctx.r4.u64;
	ctx.r3.u64 = ctx.r4.u64 + ctx.r5.u64;
	ctx.cr0.compare<int64_t>(ctx.r3.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10100"))) PPC_WEAK_FUNC(cputest_10100);
PPC_FUNC_IMPL(__imp__cputest_10100) {
	PPC_FUNC_PROLOGUE();
	// mulhdu. r3,r4,r5
	ctx.r3.u64 = uint64_t((__uint128_t(ctx.r4.u64) * __uint128_t(ctx.r5.u64)) >> 64);
	ctx.cr0.compare<int64_t>(ctx.r3.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10140"))) PPC_WEAK_FUNC(cputest_10140);
PPC_FUNC_IMPL(__imp__cputest_10140) {
	PPC_FUNC_PROLOGUE();
	// eqv. r4,r3,r5
	ctx.r4.u64 = ~(ctx.r3.u64 ^ ctx.r5.u64);
	ctx.cr0.compare<int64_t>(ctx.r4.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10180"))) PPC_WEAK_FUNC(cputest_10180);
PPC_FUNC_IMPL(__imp__cputest_10180) {
	PPC_FUNC_PROLOGUE();
	// addc r4,r4,r5
	ctx.xer.ca = (ctx.r4.u64 + ctx.r5.u64) < ctx.r4.u64;
	ctx.r4.u64 = ctx.r4.u64 + ctx.r5.u64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_101C0"))) PPC_WEAK_FUNC(cputest_101C0);
PPC_FUNC_IMPL(__imp__cputest_101C0) {
	PPC_FUNC_PROLOGUE();
	// mulhdu r4,r4,r5
	ctx.r4.u64 = uint64_t((__uint128_t(ctx.r4.u64) * __uint128_t(ctx.r5.u64)) >> 64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10200"))) PPC_WEAK_FUNC(cputest_10200);
PPC_FUNC_IMPL(__imp__cputest_10200) {
	PPC_FUNC_PROLOGUE();
	// eqv r4,r4,r5
	ctx.r4.u64 = ~(ctx.r4.u64 ^ ctx.r5.u64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10240"))) PPC_WEAK_FUNC(cputest_10240);
PPC_FUNC_IMPL(__imp__cputest_10240) {
	PPC_FUNC_PROLOGUE();
	// addc. r4,r4,r5
	ctx.xer.ca = (ctx.r4.u64 + ctx.r5.u64) < ctx.r4.u64;
	ctx.r4.u64 = ctx.r4.u64 + ctx.r5.u64;
	ctx.cr0.compare<int64_t>(ctx.r4.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10280"))) PPC_WEAK_FUNC(cputest_10280);
PPC_FUNC_IMPL(__imp__cputest_10280) {
	PPC_FUNC_PROLOGUE();
	// mulhdu. r4,r4,r5
	ctx.r4.u64 = uint64_t((__uint128_t(ctx.r4.u64) * __uint128_t(ctx.r5.u64)) >> 64);
	ctx.cr0.compare<int64_t>(ctx.r4.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_102C0"))) PPC_WEAK_FUNC(cputest_102C0);
PPC_FUNC_IMPL(__imp__cputest_102C0) {
	PPC_FUNC_PROLOGUE();
	// eqv. r4,r4,r5
	ctx.r4.u64 = ~(ctx.r4.u64 ^ ctx.r5.u64);
	ctx.cr0.compare<int64_t>(ctx.r4.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10300"))) PPC_WEAK_FUNC(cputest_10300);
PPC_FUNC_IMPL(__imp__cputest_10300) {
	PPC_FUNC_PROLOGUE();
	// addc r5,r4,r5
	ctx.xer.ca = (ctx.r4.u64 + ctx.r5.u64) < ctx.r4.u64;
	ctx.r5.u64 = ctx.r4.u64 + ctx.r5.u64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10340"))) PPC_WEAK_FUNC(cputest_10340);
PPC_FUNC_IMPL(__imp__cputest_10340) {
	PPC_FUNC_PROLOGUE();
	// mulhdu r5,r4,r5
	ctx.r5.u64 = uint64_t((__uint128_t(ctx.r4.u64) * __uint128_t(ctx.r5.u64)) >> 64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10380"))) PPC_WEAK_FUNC(cputest_10380);
PPC_FUNC_IMPL(__imp__cputest_10380) {
	PPC_FUNC_PROLOGUE();
	// eqv r4,r5,r5
	ctx.r4.u64 = ~(ctx.r5.u64 ^ ctx.r5.u64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_103C0"))) PPC_WEAK_FUNC(cputest_103C0);
PPC_FUNC_IMPL(__imp__cputest_103C0) {
	PPC_FUNC_PROLOGUE();
	// addc. r5,r4,r5
	ctx.xer.ca = (ctx.r4.u64 + ctx.r5.u64) < ctx.r4.u64;
	ctx.r5.u64 = ctx.r4.u64 + ctx.r5.u64;
	ctx.cr0.compare<int64_t>(ctx.r5.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10400"))) PPC_WEAK_FUNC(cputest_10400);
PPC_FUNC_IMPL(__imp__cputest_10400) {
	PPC_FUNC_PROLOGUE();
	// mulhdu. r5,r4,r5
	ctx.r5.u64 = uint64_t((__uint128_t(ctx.r4.u64) * __uint128_t(ctx.r5.u64)) >> 64);
	ctx.cr0.compare<int64_t>(ctx.r5.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10440"))) PPC_WEAK_FUNC(cputest_10440);
PPC_FUNC_IMPL(__imp__cputest_10440) {
	PPC_FUNC_PROLOGUE();
	// eqv. r4,r5,r5
	ctx.r4.u64 = ~(ctx.r5.u64 ^ ctx.r5.u64);
	ctx.cr0.compare<int64_t>(ctx.r4.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10480"))) PPC_WEAK_FUNC(cputest_10480);
PPC_FUNC_IMPL(__imp__cputest_10480) {
	PPC_FUNC_PROLOGUE();
	// addc r7,r7,r7
	ctx.xer.ca = (ctx.r7.u64 + ctx.r7.u64) < ctx.r7.u64;
	ctx.r7.u64 = ctx.r7.u64 + ctx.r7.u64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_104C0"))) PPC_WEAK_FUNC(cputest_104C0);
PPC_FUNC_IMPL(__imp__cputest_104C0) {
	PPC_FUNC_PROLOGUE();
	// mulhdu r7,r7,r7
	ctx.r7.u64 = uint64_t((__uint128_t(ctx.r7.u64) * __uint128_t(ctx.r7.u64)) >> 64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10500"))) PPC_WEAK_FUNC(cputest_10500);
PPC_FUNC_IMPL(__imp__cputest_10500) {
	PPC_FUNC_PROLOGUE();
	// eqv r7,r7,r7
	ctx.r7.u64 = ~(ctx.r7.u64 ^ ctx.r7.u64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10540"))) PPC_WEAK_FUNC(cputest_10540);
PPC_FUNC_IMPL(__imp__cputest_10540) {
	PPC_FUNC_PROLOGUE();
	// addc. r7,r7,r7
	ctx.xer.ca = (ctx.r7.u64 + ctx.r7.u64) < ctx.r7.u64;
	ctx.r7.u64 = ctx.r7.u64 + ctx.r7.u64;
	ctx.cr0.compare<int64_t>(ctx.r7.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10580"))) PPC_WEAK_FUNC(cputest_10580);
PPC_FUNC_IMPL(__imp__cputest_10580) {
	PPC_FUNC_PROLOGUE();
	// mulhdu. r7,r7,r7
	ctx.r7.u64 = uint64_t((__uint128_t(ctx.r7.u64) * __uint128_t(ctx.r7.u64)) >> 64);
	ctx.cr0.compare<int64_t>(ctx.r7.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_105C0"))) PPC_WEAK_FUNC(cputest_105C0);
PPC_FUNC_IMPL(__imp__cputest_105C0) {
	PPC_FUNC_PROLOGUE();
	// eqv. r7,r7,r7
	ctx.r7.u64 = ~(ctx.r7.u64 ^ ctx.r7.u64);
	ctx.cr0.compare<int64_t>(ctx.r7.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10600"))) PPC_WEAK_FUNC(cputest_10600);
PPC_FUNC_IMPL(__imp__cputest_10600) {
	PPC_FUNC_PROLOGUE();
	// addc r31,r30,r29
	ctx.xer.ca = (ctx.r30.u64 + ctx.r29.u64) < ctx.r30.u64;
	ctx.r31.u64 = ctx.r30.u64 + ctx.r29.u64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10640"))) PPC_WEAK_FUNC(cputest_10640);
PPC_FUNC_IMPL(__imp__cputest_10640) {
	PPC_FUNC_PROLOGUE();
	// mulhdu r31,r30,r29
	ctx.r31.u64 = uint64_t((__uint128_t(ctx.r30.u64) * __uint128_t(ctx.r29.u64)) >> 64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10680"))) PPC_WEAK_FUNC(cputest_10680);
PPC_FUNC_IMPL(__imp__cputest_10680) {
	PPC_FUNC_PROLOGUE();
	// eqv r30,r31,r29
	ctx.r30.u64 = ~(ctx.r31.u64 ^ ctx.r29.u64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_106C0"))) PPC_WEAK_FUNC(cputest_106C0);
PPC_FUNC_IMPL(__imp__cputest_106C0) {
	PPC_FUNC_PROLOGUE();
	// addc. r31,r30,r29
	ctx.xer.ca = (ctx.r30.u64 + ctx.r29.u64) < ctx.r30.u64;
	ctx.r31.u64 = ctx.r30.u64 + ctx.r29.u64;
	ctx.cr0.compare<int64_t>(ctx.r31.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10700"))) PPC_WEAK_FUNC(cputest_10700);
PPC_FUNC_IMPL(__imp__cputest_10700) {
	PPC_FUNC_PROLOGUE();
	// mulhdu. r31,r30,r29
	ctx.r31.u64 = uint64_t((__uint128_t(ctx.r30.u64) * __uint128_t(ctx.r29.u64)) >> 64);
	ctx.cr0.compare<int64_t>(ctx.r31.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10740"))) PPC_WEAK_FUNC(cputest_10740);
PPC_FUNC_IMPL(__imp__cputest_10740) {
	PPC_FUNC_PROLOGUE();
	// eqv. r30,r31,r29
	ctx.r30.u64 = ~(ctx.r31.u64 ^ ctx.r29.u64);
	ctx.cr0.compare<int64_t>(ctx.r30.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10780"))) PPC_WEAK_FUNC(cputest_10780);
PPC_FUNC_IMPL(__imp__cputest_10780) {
	PPC_FUNC_PROLOGUE();
	// addc r0,r1,r2
	ctx.xer.ca = (ctx.r1.u64 + ctx.r2.u64) < ctx.r1.u64;
	ctx.r0.u64 = ctx.r1.u64 + ctx.r2.u64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_107C0"))) PPC_WEAK_FUNC(cputest_107C0);
PPC_FUNC_IMPL(__imp__cputest_107C0) {
	PPC_FUNC_PROLOGUE();
	// mulhdu r0,r1,r2
	ctx.r0.u64 = uint64_t((__uint128_t(ctx.r1.u64) * __uint128_t(ctx.r2.u64)) >> 64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10800"))) PPC_WEAK_FUNC(cputest_10800);
PPC_FUNC_IMPL(__imp__cputest_10800) {
	PPC_FUNC_PROLOGUE();
	// eqv r1,r0,r2
	ctx.r1.u64 = ~(ctx.r0.u64 ^ ctx.r2.u64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10840"))) PPC_WEAK_FUNC(cputest_10840);
PPC_FUNC_IMPL(__imp__cputest_10840) {
	PPC_FUNC_PROLOGUE();
	// addc. r0,r1,r2
	ctx.xer.ca = (ctx.r1.u64 + ctx.r2.u64) < ctx.r1.u64;
	ctx.r0.u64 = ctx.r1.u64 + ctx.r2.u64;
	ctx.cr0.compare<int64_t>(ctx.r0.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10880"))) PPC_WEAK_FUNC(cputest_10880);
PPC_FUNC_IMPL(__imp__cputest_10880) {
	PPC_FUNC_PROLOGUE();
	// mulhdu. r0,r1,r2
	ctx.r0.u64 = uint64_t((__uint128_t(ctx.r1.u64) * __uint128_t(ctx.r2.u64)) >> 64);
	ctx.cr0.compare<int64_t>(ctx.r0.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_108C0"))) PPC_WEAK_FUNC(cputest_108C0);
PPC_FUNC_IMPL(__imp__cputest_108C0) {
	PPC_FUNC_PROLOGUE();
	// eqv. r1,r0,r2
	ctx.r1.u64 = ~(ctx.r0.u64 ^ ctx.r2.u64);
	ctx.cr0.compare<int64_t>(ctx.r1.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10900"))) PPC_WEAK_FUNC(cputest_10900);
PPC_FUNC_IMPL(__imp__cputest_10900) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// addme r3,r4
	temp.u8 = (ctx.r4.u64 != 0) | ctx.xer.ca;
	ctx.r3.s64 = ctx.r4.s64 + ctx.xer.ca - 1;
	ctx.xer.ca = temp.u8;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10940"))) PPC_WEAK_FUNC(cputest_10940);
PPC_FUNC_IMPL(__imp__cputest_10940) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// subfze r3,r4
	temp.u8 = (uint64_t(~ctx.r4.u64) + ctx.xer.ca) < ctx.xer.ca;
	ctx.r3.u64 = ~ctx.r4.u64 + ctx.xer.ca;
	ctx.xer.ca = temp.u8;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10980"))) PPC_WEAK_FUNC(cputest_10980);
PPC_FUNC_IMPL(__imp__cputest_10980) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// addme. r3,r4
	temp.u8 = (ctx.r4.u64 != 0) | ctx.xer.ca;
	ctx.r3.s64 = ctx.r4.s64 + ctx.xer.ca - 1;
	ctx.xer.ca = temp.u8;
	ctx.cr0.compare<int64_t>(ctx.r3.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_109C0"))) PPC_WEAK_FUNC(cputest_109C0);
PPC_FUNC_IMPL(__imp__cputest_109C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// subfze. r3,r4
	temp.u8 = (uint64_t(~ctx.r4.u64) + ctx.xer.ca) < ctx.xer.ca;
	ctx.r3.u64 = ~ctx.r4.u64 + ctx.xer.ca;
	ctx.xer.ca = temp.u8;
	ctx.cr0.compare<int64_t>(ctx.r3.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10A00"))) PPC_WEAK_FUNC(cputest_10A00);
PPC_FUNC_IMPL(__imp__cputest_10A00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// addme r4,r4
	temp.u8 = (ctx.r4.u64 != 0) | ctx.xer.ca;
	ctx.r4.s64 = ctx.r4.s64 + ctx.xer.ca - 1;
	ctx.xer.ca = temp.u8;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10A40"))) PPC_WEAK_FUNC(cputest_10A40);
PPC_FUNC_IMPL(__imp__cputest_10A40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// subfze r4,r4
	temp.u8 = (uint64_t(~ctx.r4.u64) + ctx.xer.ca) < ctx.xer.ca;
	ctx.r4.u64 = ~ctx.r4.u64 + ctx.xer.ca;
	ctx.xer.ca = temp.u8;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10A80"))) PPC_WEAK_FUNC(cputest_10A80);
PPC_FUNC_IMPL(__imp__cputest_10A80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// addme. r4,r4
	temp.u8 = (ctx.r4.u64 != 0) | ctx.xer.ca;
	ctx.r4.s64 = ctx.r4.s64 + ctx.xer.ca - 1;
	ctx.xer.ca = temp.u8;
	ctx.cr0.compare<int64_t>(ctx.r4.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10AC0"))) PPC_WEAK_FUNC(cputest_10AC0);
PPC_FUNC_IMPL(__imp__cputest_10AC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// subfze. r4,r4
	temp.u8 = (uint64_t(~ctx.r4.u64) + ctx.xer.ca) < ctx.xer.ca;
	ctx.r4.u64 = ~ctx.r4.u64 + ctx.xer.ca;
	ctx.xer.ca = temp.u8;
	ctx.cr0.compare<int64_t>(ctx.r4.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10B00"))) PPC_WEAK_FUNC(cputest_10B00);
PPC_FUNC_IMPL(__imp__cputest_10B00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// addme r31,r30
	temp.u8 = (ctx.r30.u64 != 0) | ctx.xer.ca;
	ctx.r31.s64 = ctx.r30.s64 + ctx.xer.ca - 1;
	ctx.xer.ca = temp.u8;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10B40"))) PPC_WEAK_FUNC(cputest_10B40);
PPC_FUNC_IMPL(__imp__cputest_10B40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// subfze r31,r30
	temp.u8 = (uint64_t(~ctx.r30.u64) + ctx.xer.ca) < ctx.xer.ca;
	ctx.r31.u64 = ~ctx.r30.u64 + ctx.xer.ca;
	ctx.xer.ca = temp.u8;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10B80"))) PPC_WEAK_FUNC(cputest_10B80);
PPC_FUNC_IMPL(__imp__cputest_10B80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// addme. r31,r30
	temp.u8 = (ctx.r30.u64 != 0) | ctx.xer.ca;
	ctx.r31.s64 = ctx.r30.s64 + ctx.xer.ca - 1;
	ctx.xer.ca = temp.u8;
	ctx.cr0.compare<int64_t>(ctx.r31.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10BC0"))) PPC_WEAK_FUNC(cputest_10BC0);
PPC_FUNC_IMPL(__imp__cputest_10BC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// subfze. r31,r30
	temp.u8 = (uint64_t(~ctx.r30.u64) + ctx.xer.ca) < ctx.xer.ca;
	ctx.r31.u64 = ~ctx.r30.u64 + ctx.xer.ca;
	ctx.xer.ca = temp.u8;
	ctx.cr0.compare<int64_t>(ctx.r31.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10C00"))) PPC_WEAK_FUNC(cputest_10C00);
PPC_FUNC_IMPL(__imp__cputest_10C00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// addme r0,r1
	temp.u8 = (ctx.r1.u64 != 0) | ctx.xer.ca;
	ctx.r0.s64 = ctx.r1.s64 + ctx.xer.ca - 1;
	ctx.xer.ca = temp.u8;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10C40"))) PPC_WEAK_FUNC(cputest_10C40);
PPC_FUNC_IMPL(__imp__cputest_10C40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// subfze r0,r1
	temp.u8 = (uint64_t(~ctx.r1.u64) + ctx.xer.ca) < ctx.xer.ca;
	ctx.r0.u64 = ~ctx.r1.u64 + ctx.xer.ca;
	ctx.xer.ca = temp.u8;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10C80"))) PPC_WEAK_FUNC(cputest_10C80);
PPC_FUNC_IMPL(__imp__cputest_10C80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// addme. r0,r1
	temp.u8 = (ctx.r1.u64 != 0) | ctx.xer.ca;
	ctx.r0.s64 = ctx.r1.s64 + ctx.xer.ca - 1;
	ctx.xer.ca = temp.u8;
	ctx.cr0.compare<int64_t>(ctx.r0.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10CC0"))) PPC_WEAK_FUNC(cputest_10CC0);
PPC_FUNC_IMPL(__imp__cputest_10CC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// subfze. r0,r1
	temp.u8 = (uint64_t(~ctx.r1.u64) + ctx.xer.ca) < ctx.xer.ca;
	ctx.r0.u64 = ~ctx.r1.u64 + ctx.xer.ca;
	ctx.xer.ca = temp.u8;
	ctx.cr0.compare<int64_t>(ctx.r0.s64, 0, ctx.xer);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10D00"))) PPC_WEAK_FUNC(cputest_10D00);
PPC_FUNC_IMPL(__imp__cputest_10D00) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhzu r3,16(r4)
	ea = 16 + ctx.r4.u32;
	ctx.r3.u64 = PPC_LOAD_U16(ea);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10D40"))) PPC_WEAK_FUNC(cputest_10D40);
PPC_FUNC_IMPL(__imp__cputest_10D40) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhau r3,16(r4)
	ea = 16 + ctx.r4.u32;
	ctx.r3.s64 = int16_t(PPC_LOAD_U16(ea));
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10D80"))) PPC_WEAK_FUNC(cputest_10D80);
PPC_FUNC_IMPL(__imp__cputest_10D80) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// sthu r3,16(r4)
	ea = 16 + ctx.r4.u32;
	PPC_STORE_U16(ea, ctx.r3.u16);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10DC0"))) PPC_WEAK_FUNC(cputest_10DC0);
PPC_FUNC_IMPL(__imp__cputest_10DC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// lfsu f3,16(r4)
	ctx.fpscr.disableFlushMode();
	ea = 16 + ctx.r4.u32;
	temp.u32 = PPC_LOAD_U32(ea);
	ctx.f3.f64 = double(temp.f32);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10E00"))) PPC_WEAK_FUNC(cputest_10E00);
PPC_FUNC_IMPL(__imp__cputest_10E00) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lfdu f3,16(r4)
	ctx.fpscr.disableFlushMode();
	ea = 16 + ctx.r4.u32;
	ctx.f3.u64 = PPC_LOAD_U64(ea);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10E40"))) PPC_WEAK_FUNC(cputest_10E40);
PPC_FUNC_IMPL(__imp__cputest_10E40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// stfsu f3,16(r4)
	ctx.fpscr.disableFlushMode();
	ea = 16 + ctx.r4.u32;
	temp.f32 = float(ctx.f3.f64);
	PPC_STORE_U32(ea, temp.u32);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10E80"))) PPC_WEAK_FUNC(cputest_10E80);
PPC_FUNC_IMPL(__imp__cputest_10E80) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stfdu f3,16(r4)
	ctx.fpscr.disableFlushMode();
	ea = 16 + ctx.r4.u32;
	PPC_STORE_U64(ea, ctx.f3.u64);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10EC0"))) PPC_WEAK_FUNC(cputest_10EC0);
PPC_FUNC_IMPL(__imp__cputest_10EC0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhzu r31,-32(r30)
	ea = -32 + ctx.r30.u32;
	ctx.r31.u64 = PPC_LOAD_U16(ea);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10F00"))) PPC_WEAK_FUNC(cputest_10F00);
PPC_FUNC_IMPL(__imp__cputest_10F00) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhau r31,-32(r30)
	ea = -32 + ctx.r30.u32;
	ctx.r31.s64 = int16_t(PPC_LOAD_U16(ea));
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10F40"))) PPC_WEAK_FUNC(cputest_10F40);
PPC_FUNC_IMPL(__imp__cputest_10F40) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// sthu r31,-32(r30)
	ea = -32 + ctx.r30.u32;
	PPC_STORE_U16(ea, ctx.r31.u16);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10F80"))) PPC_WEAK_FUNC(cputest_10F80);
PPC_FUNC_IMPL(__imp__cputest_10F80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// lfsu f31,-32(r30)
	ctx.fpscr.disableFlushMode();
	ea = -32 + ctx.r30.u32;
	temp.u32 = PPC_LOAD_U32(ea);
	ctx.f31.f64 = double(temp.f32);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_10FC0"))) PPC_WEAK_FUNC(cputest_10FC0);
PPC_FUNC_IMPL(__imp__cputest_10FC0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lfdu f31,-32(r30)
	ctx.fpscr.disableFlushMode();
	ea = -32 + ctx.r30.u32;
	ctx.f31.u64 = PPC_LOAD_U64(ea);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11000"))) PPC_WEAK_FUNC(cputest_11000);
PPC_FUNC_IMPL(__imp__cputest_11000) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// stfsu f31,-32(r30)
	ctx.fpscr.disableFlushMode();
	ea = -32 + ctx.r30.u32;
	temp.f32 = float(ctx.f31.f64);
	PPC_STORE_U32(ea, temp.u32);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11040"))) PPC_WEAK_FUNC(cputest_11040);
PPC_FUNC_IMPL(__imp__cputest_11040) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stfdu f31,-32(r30)
	ctx.fpscr.disableFlushMode();
	ea = -32 + ctx.r30.u32;
	PPC_STORE_U64(ea, ctx.f31.u64);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11080"))) PPC_WEAK_FUNC(cputest_11080);
PPC_FUNC_IMPL(__imp__cputest_11080) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhzu r10,510(r5)
	ea = 510 + ctx.r5.u32;
	ctx.r10.u64 = PPC_LOAD_U16(ea);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_110C0"))) PPC_WEAK_FUNC(cputest_110C0);
PPC_FUNC_IMPL(__imp__cputest_110C0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhau r10,510(r5)
	ea = 510 + ctx.r5.u32;
	ctx.r10.s64 = int16_t(PPC_LOAD_U16(ea));
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11100"))) PPC_WEAK_FUNC(cputest_11100);
PPC_FUNC_IMPL(__imp__cputest_11100) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// sthu r10,510(r5)
	ea = 510 + ctx.r5.u32;
	PPC_STORE_U16(ea, ctx.r10.u16);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11140"))) PPC_WEAK_FUNC(cputest_11140);
PPC_FUNC_IMPL(__imp__cputest_11140) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// lfsu f10,510(r5)
	ctx.fpscr.disableFlushMode();
	ea = 510 + ctx.r5.u32;
	temp.u32 = PPC_LOAD_U32(ea);
	ctx.f10.f64 = double(temp.f32);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11180"))) PPC_WEAK_FUNC(cputest_11180);
PPC_FUNC_IMPL(__imp__cputest_11180) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lfdu f10,504(r5)
	ctx.fpscr.disableFlushMode();
	ea = 504 + ctx.r5.u32;
	ctx.f10.u64 = PPC_LOAD_U64(ea);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_111C0"))) PPC_WEAK_FUNC(cputest_111C0);
PPC_FUNC_IMPL(__imp__cputest_111C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// stfsu f10,510(r5)
	ctx.fpscr.disableFlushMode();
	ea = 510 + ctx.r5.u32;
	temp.f32 = float(ctx.f10.f64);
	PPC_STORE_U32(ea, temp.u32);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11200"))) PPC_WEAK_FUNC(cputest_11200);
PPC_FUNC_IMPL(__imp__cputest_11200) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stfdu f10,504(r5)
	ctx.fpscr.disableFlushMode();
	ea = 504 + ctx.r5.u32;
	PPC_STORE_U64(ea, ctx.f10.u64);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11240"))) PPC_WEAK_FUNC(cputest_11240);
PPC_FUNC_IMPL(__imp__cputest_11240) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhzu r0,-2(r9)
	ea = -2 + ctx.r9.u32;
	ctx.r0.u64 = PPC_LOAD_U16(ea);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11280"))) PPC_WEAK_FUNC(cputest_11280);
PPC_FUNC_IMPL(__imp__cputest_11280) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhau r0,-2(r9)
	ea = -2 + ctx.r9.u32;
	ctx.r0.s64 = int16_t(PPC_LOAD_U16(ea));
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_112C0"))) PPC_WEAK_FUNC(cputest_112C0);
PPC_FUNC_IMPL(__imp__cputest_112C0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// sthu r0,-2(r9)
	ea = -2 + ctx.r9.u32;
	PPC_STORE_U16(ea, ctx.r0.u16);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11300"))) PPC_WEAK_FUNC(cputest_11300);
PPC_FUNC_IMPL(__imp__cputest_11300) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// lfsu f0,-2(r9)
	ctx.fpscr.disableFlushMode();
	ea = -2 + ctx.r9.u32;
	temp.u32 = PPC_LOAD_U32(ea);
	ctx.f0.f64 = double(temp.f32);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11340"))) PPC_WEAK_FUNC(cputest_11340);
PPC_FUNC_IMPL(__imp__cputest_11340) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lfdu f0,-8(r9)
	ctx.fpscr.disableFlushMode();
	ea = -8 + ctx.r9.u32;
	ctx.f0.u64 = PPC_LOAD_U64(ea);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11380"))) PPC_WEAK_FUNC(cputest_11380);
PPC_FUNC_IMPL(__imp__cputest_11380) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// stfsu f0,-2(r9)
	ctx.fpscr.disableFlushMode();
	ea = -2 + ctx.r9.u32;
	temp.f32 = float(ctx.f0.f64);
	PPC_STORE_U32(ea, temp.u32);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_113C0"))) PPC_WEAK_FUNC(cputest_113C0);
PPC_FUNC_IMPL(__imp__cputest_113C0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stfdu f0,-8(r9)
	ctx.fpscr.disableFlushMode();
	ea = -8 + ctx.r9.u32;
	PPC_STORE_U64(ea, ctx.f0.u64);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11400"))) PPC_WEAK_FUNC(cputest_11400);
PPC_FUNC_IMPL(__imp__cputest_11400) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lbzux r3,r4,r5
	ea = ctx.r4.u32 + ctx.r5.u32;
	ctx.r3.u64 = PPC_LOAD_U8(ea);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11440"))) PPC_WEAK_FUNC(cputest_11440);
PPC_FUNC_IMPL(__imp__cputest_11440) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhzux r3,r4,r5
	ea = ctx.r4.u32 + ctx.r5.u32;
	ctx.r3.u64 = PPC_LOAD_U16(ea);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11480"))) PPC_WEAK_FUNC(cputest_11480);
PPC_FUNC_IMPL(__imp__cputest_11480) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lwzux r3,r4,r5
	ea = ctx.r4.u32 + ctx.r5.u32;
	ctx.r3.u64 = PPC_LOAD_U32(ea);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_114C0"))) PPC_WEAK_FUNC(cputest_114C0);
PPC_FUNC_IMPL(__imp__cputest_114C0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// ldux r3,r4,r5
	ea = ctx.r4.u32 + ctx.r5.u32;
	ctx.r3.u64 = PPC_LOAD_U64(ea);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11500"))) PPC_WEAK_FUNC(cputest_11500);
PPC_FUNC_IMPL(__imp__cputest_11500) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stbux r3,r4,r5
	ea = ctx.r4.u32 + ctx.r5.u32;
	PPC_STORE_U8(ea, ctx.r3.u8);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11540"))) PPC_WEAK_FUNC(cputest_11540);
PPC_FUNC_IMPL(__imp__cputest_11540) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// sthux r3,r4,r5
	ea = ctx.r4.u32 + ctx.r5.u32;
	PPC_STORE_U16(ea, ctx.r3.u16);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11580"))) PPC_WEAK_FUNC(cputest_11580);
PPC_FUNC_IMPL(__imp__cputest_11580) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stdux r3,r4,r5
	ea = ctx.r4.u32 + ctx.r5.u32;
	PPC_STORE_U64(ea, ctx.r3.u64);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_115C0"))) PPC_WEAK_FUNC(cputest_115C0);
PPC_FUNC_IMPL(__imp__cputest_115C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// lfsux f3,r4,r5
	ctx.fpscr.disableFlushMode();
	ea = ctx.r4.u32 + ctx.r5.u32;
	temp.u32 = PPC_LOAD_U32(ea);
	ctx.f3.f64 = double(temp.f32);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11600"))) PPC_WEAK_FUNC(cputest_11600);
PPC_FUNC_IMPL(__imp__cputest_11600) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// stfsux f3,r4,r5
	ctx.fpscr.disableFlushMode();
	ea = ctx.r4.u32 + ctx.r5.u32;
	temp.f32 = float(ctx.f3.f64);
	PPC_STORE_U32(ea, temp.u32);
	ctx.r4.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11640"))) PPC_WEAK_FUNC(cputest_11640);
PPC_FUNC_IMPL(__imp__cputest_11640) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lbzux r31,r30,r29
	ea = ctx.r30.u32 + ctx.r29.u32;
	ctx.r31.u64 = PPC_LOAD_U8(ea);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11680"))) PPC_WEAK_FUNC(cputest_11680);
PPC_FUNC_IMPL(__imp__cputest_11680) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhzux r31,r30,r29
	ea = ctx.r30.u32 + ctx.r29.u32;
	ctx.r31.u64 = PPC_LOAD_U16(ea);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_116C0"))) PPC_WEAK_FUNC(cputest_116C0);
PPC_FUNC_IMPL(__imp__cputest_116C0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lwzux r31,r30,r29
	ea = ctx.r30.u32 + ctx.r29.u32;
	ctx.r31.u64 = PPC_LOAD_U32(ea);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11700"))) PPC_WEAK_FUNC(cputest_11700);
PPC_FUNC_IMPL(__imp__cputest_11700) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// ldux r31,r30,r29
	ea = ctx.r30.u32 + ctx.r29.u32;
	ctx.r31.u64 = PPC_LOAD_U64(ea);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11740"))) PPC_WEAK_FUNC(cputest_11740);
PPC_FUNC_IMPL(__imp__cputest_11740) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stbux r31,r30,r29
	ea = ctx.r30.u32 + ctx.r29.u32;
	PPC_STORE_U8(ea, ctx.r31.u8);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11780"))) PPC_WEAK_FUNC(cputest_11780);
PPC_FUNC_IMPL(__imp__cputest_11780) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// sthux r31,r30,r29
	ea = ctx.r30.u32 + ctx.r29.u32;
	PPC_STORE_U16(ea, ctx.r31.u16);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_117C0"))) PPC_WEAK_FUNC(cputest_117C0);
PPC_FUNC_IMPL(__imp__cputest_117C0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stdux r31,r30,r29
	ea = ctx.r30.u32 + ctx.r29.u32;
	PPC_STORE_U64(ea, ctx.r31.u64);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11800"))) PPC_WEAK_FUNC(cputest_11800);
PPC_FUNC_IMPL(__imp__cputest_11800) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// lfsux f31,r30,r29
	ctx.fpscr.disableFlushMode();
	ea = ctx.r30.u32 + ctx.r29.u32;
	temp.u32 = PPC_LOAD_U32(ea);
	ctx.f31.f64 = double(temp.f32);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11840"))) PPC_WEAK_FUNC(cputest_11840);
PPC_FUNC_IMPL(__imp__cputest_11840) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// stfsux f31,r30,r29
	ctx.fpscr.disableFlushMode();
	ea = ctx.r30.u32 + ctx.r29.u32;
	temp.f32 = float(ctx.f31.f64);
	PPC_STORE_U32(ea, temp.u32);
	ctx.r30.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11880"))) PPC_WEAK_FUNC(cputest_11880);
PPC_FUNC_IMPL(__imp__cputest_11880) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lbzux r10,r5,r6
	ea = ctx.r5.u32 + ctx.r6.u32;
	ctx.r10.u64 = PPC_LOAD_U8(ea);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_118C0"))) PPC_WEAK_FUNC(cputest_118C0);
PPC_FUNC_IMPL(__imp__cputest_118C0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhzux r10,r5,r6
	ea = ctx.r5.u32 + ctx.r6.u32;
	ctx.r10.u64 = PPC_LOAD_U16(ea);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11900"))) PPC_WEAK_FUNC(cputest_11900);
PPC_FUNC_IMPL(__imp__cputest_11900) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lwzux r10,r5,r6
	ea = ctx.r5.u32 + ctx.r6.u32;
	ctx.r10.u64 = PPC_LOAD_U32(ea);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11940"))) PPC_WEAK_FUNC(cputest_11940);
PPC_FUNC_IMPL(__imp__cputest_11940) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// ldux r10,r5,r6
	ea = ctx.r5.u32 + ctx.r6.u32;
	ctx.r10.u64 = PPC_LOAD_U64(ea);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11980"))) PPC_WEAK_FUNC(cputest_11980);
PPC_FUNC_IMPL(__imp__cputest_11980) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stbux r10,r5,r6
	ea = ctx.r5.u32 + ctx.r6.u32;
	PPC_STORE_U8(ea, ctx.r10.u8);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_119C0"))) PPC_WEAK_FUNC(cputest_119C0);
PPC_FUNC_IMPL(__imp__cputest_119C0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// sthux r10,r5,r6
	ea = ctx.r5.u32 + ctx.r6.u32;
	PPC_STORE_U16(ea, ctx.r10.u16);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11A00"))) PPC_WEAK_FUNC(cputest_11A00);
PPC_FUNC_IMPL(__imp__cputest_11A00) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stdux r10,r5,r6
	ea = ctx.r5.u32 + ctx.r6.u32;
	PPC_STORE_U64(ea, ctx.r10.u64);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11A40"))) PPC_WEAK_FUNC(cputest_11A40);
PPC_FUNC_IMPL(__imp__cputest_11A40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// lfsux f10,r5,r6
	ctx.fpscr.disableFlushMode();
	ea = ctx.r5.u32 + ctx.r6.u32;
	temp.u32 = PPC_LOAD_U32(ea);
	ctx.f10.f64 = double(temp.f32);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11A80"))) PPC_WEAK_FUNC(cputest_11A80);
PPC_FUNC_IMPL(__imp__cputest_11A80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// stfsux f10,r5,r6
	ctx.fpscr.disableFlushMode();
	ea = ctx.r5.u32 + ctx.r6.u32;
	temp.f32 = float(ctx.f10.f64);
	PPC_STORE_U32(ea, temp.u32);
	ctx.r5.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11AC0"))) PPC_WEAK_FUNC(cputest_11AC0);
PPC_FUNC_IMPL(__imp__cputest_11AC0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lbzux r0,r9,r8
	ea = ctx.r9.u32 + ctx.r8.u32;
	ctx.r0.u64 = PPC_LOAD_U8(ea);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11B00"))) PPC_WEAK_FUNC(cputest_11B00);
PPC_FUNC_IMPL(__imp__cputest_11B00) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lhzux r0,r9,r8
	ea = ctx.r9.u32 + ctx.r8.u32;
	ctx.r0.u64 = PPC_LOAD_U16(ea);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11B40"))) PPC_WEAK_FUNC(cputest_11B40);
PPC_FUNC_IMPL(__imp__cputest_11B40) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// lwzux r0,r9,r8
	ea = ctx.r9.u32 + ctx.r8.u32;
	ctx.r0.u64 = PPC_LOAD_U32(ea);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11B80"))) PPC_WEAK_FUNC(cputest_11B80);
PPC_FUNC_IMPL(__imp__cputest_11B80) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// ldux r0,r9,r8
	ea = ctx.r9.u32 + ctx.r8.u32;
	ctx.r0.u64 = PPC_LOAD_U64(ea);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11BC0"))) PPC_WEAK_FUNC(cputest_11BC0);
PPC_FUNC_IMPL(__imp__cputest_11BC0) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stbux r0,r9,r8
	ea = ctx.r9.u32 + ctx.r8.u32;
	PPC_STORE_U8(ea, ctx.r0.u8);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11C00"))) PPC_WEAK_FUNC(cputest_11C00);
PPC_FUNC_IMPL(__imp__cputest_11C00) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// sthux r0,r9,r8
	ea = ctx.r9.u32 + ctx.r8.u32;
	PPC_STORE_U16(ea, ctx.r0.u16);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11C40"))) PPC_WEAK_FUNC(cputest_11C40);
PPC_FUNC_IMPL(__imp__cputest_11C40) {
	PPC_FUNC_PROLOGUE();
	uint32_t ea{};
	// stdux r0,r9,r8
	ea = ctx.r9.u32 + ctx.r8.u32;
	PPC_STORE_U64(ea, ctx.r0.u64);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11C80"))) PPC_WEAK_FUNC(cputest_11C80);
PPC_FUNC_IMPL(__imp__cputest_11C80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// lfsux f0,r9,r8
	ctx.fpscr.disableFlushMode();
	ea = ctx.r9.u32 + ctx.r8.u32;
	temp.u32 = PPC_LOAD_U32(ea);
	ctx.f0.f64 = double(temp.f32);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11CC0"))) PPC_WEAK_FUNC(cputest_11CC0);
PPC_FUNC_IMPL(__imp__cputest_11CC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	uint32_t ea{};
	// stfsux f0,r9,r8
	ctx.fpscr.disableFlushMode();
	ea = ctx.r9.u32 + ctx.r8.u32;
	temp.f32 = float(ctx.f0.f64);
	PPC_STORE_U32(ea, temp.u32);
	ctx.r9.u32 = ea;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11D00"))) PPC_WEAK_FUNC(cputest_11D00);
PPC_FUNC_IMPL(__imp__cputest_11D00) {
	PPC_FUNC_PROLOGUE();
	// lhbrx r3,r4,r5
	ctx.r3.u64 = __builtin_bswap16(PPC_LOAD_U16(ctx.r4.u32 + ctx.r5.u32));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11D40"))) PPC_WEAK_FUNC(cputest_11D40);
PPC_FUNC_IMPL(__imp__cputest_11D40) {
	PPC_FUNC_PROLOGUE();
	// lhbrx r3,0,r5
	ctx.r3.u64 = __builtin_bswap16(PPC_LOAD_U16(ctx.r5.u32));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11D80"))) PPC_WEAK_FUNC(cputest_11D80);
PPC_FUNC_IMPL(__imp__cputest_11D80) {
	PPC_FUNC_PROLOGUE();
	// lhbrx r31,r30,r29
	ctx.r31.u64 = __builtin_bswap16(PPC_LOAD_U16(ctx.r30.u32 + ctx.r29.u32));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11DC0"))) PPC_WEAK_FUNC(cputest_11DC0);
PPC_FUNC_IMPL(__imp__cputest_11DC0) {
	PPC_FUNC_PROLOGUE();
	// lhbrx r6,r6,r7
	ctx.r6.u64 = __builtin_bswap16(PPC_LOAD_U16(ctx.r6.u32 + ctx.r7.u32));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11E00"))) PPC_WEAK_FUNC(cputest_11E00);
PPC_FUNC_IMPL(__imp__cputest_11E00) {
	PPC_FUNC_PROLOGUE();
	// lvehx v3,r4,r5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_shuffle_epi8(simde_mm_load_si128((simde__m128i*)(base + ((ctx.r4.u32 + ctx.r5.u32) & ~0xF))), simde_mm_load_si128((simde__m128i*)VectorMaskL)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11E40"))) PPC_WEAK_FUNC(cputest_11E40);
PPC_FUNC_IMPL(__imp__cputest_11E40) {
	PPC_FUNC_PROLOGUE();
	// lvehx v3,r0,r5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_shuffle_epi8(simde_mm_load_si128((simde__m128i*)(base + ((ctx.r5.u32) & ~0xF))), simde_mm_load_si128((simde__m128i*)VectorMaskL)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11E80"))) PPC_WEAK_FUNC(cputest_11E80);
PPC_FUNC_IMPL(__imp__cputest_11E80) {
	PPC_FUNC_PROLOGUE();
	// lvehx v31,r30,r29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_shuffle_epi8(simde_mm_load_si128((simde__m128i*)(base + ((ctx.r30.u32 + ctx.r29.u32) & ~0xF))), simde_mm_load_si128((simde__m128i*)VectorMaskL)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11EC0"))) PPC_WEAK_FUNC(cputest_11EC0);
PPC_FUNC_IMPL(__imp__cputest_11EC0) {
	PPC_FUNC_PROLOGUE();
	// dcbst r4,r5
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11F00"))) PPC_WEAK_FUNC(cputest_11F00);
PPC_FUNC_IMPL(__imp__cputest_11F00) {
	PPC_FUNC_PROLOGUE();
	// dcbst r0,r5
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11F40"))) PPC_WEAK_FUNC(cputest_11F40);
PPC_FUNC_IMPL(__imp__cputest_11F40) {
	PPC_FUNC_PROLOGUE();
	// dcbst r30,r29
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11F80"))) PPC_WEAK_FUNC(cputest_11F80);
PPC_FUNC_IMPL(__imp__cputest_11F80) {
	PPC_FUNC_PROLOGUE();
	// frsqrte f1,f2
	ctx.fpscr.disableFlushMode();
	ctx.f1.f64 = 1.0 / sqrt(ctx.f2.f64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_11FC0"))) PPC_WEAK_FUNC(cputest_11FC0);
PPC_FUNC_IMPL(__imp__cputest_11FC0) {
	PPC_FUNC_PROLOGUE();
	// frsqrte f5,f5
	ctx.fpscr.disableFlushMode();
	ctx.f5.f64 = 1.0 / sqrt(ctx.f5.f64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12000"))) PPC_WEAK_FUNC(cputest_12000);
PPC_FUNC_IMPL(__imp__cputest_12000) {
	PPC_FUNC_PROLOGUE();
	// frsqrte f31,f30
	ctx.fpscr.disableFlushMode();
	ctx.f31.f64 = 1.0 / sqrt(ctx.f30.f64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12040"))) PPC_WEAK_FUNC(cputest_12040);
PPC_FUNC_IMPL(__imp__cputest_12040) {
	PPC_FUNC_PROLOGUE();
	// frsqrte f0,f13
	ctx.fpscr.disableFlushMode();
	ctx.f0.f64 = 1.0 / sqrt(ctx.f13.f64);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12080"))) PPC_WEAK_FUNC(cputest_12080);
PPC_FUNC_IMPL(__imp__cputest_12080) {
	PPC_FUNC_PROLOGUE();
	// bdzf lt,0x12088
	--ctx.ctr.u64;
	if (ctx.ctr.u64 == 0 && !ctx.cr0.lt) goto loc_12088;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12088:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_120C0"))) PPC_WEAK_FUNC(cputest_120C0);
PPC_FUNC_IMPL(__imp__cputest_120C0) {
	PPC_FUNC_PROLOGUE();
	// bdzf eq,0x120c8
	--ctx.ctr.u64;
	if (ctx.ctr.u64 == 0 && !ctx.cr0.eq) goto loc_120C8;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_120C8:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12100"))) PPC_WEAK_FUNC(cputest_12100);
PPC_FUNC_IMPL(__imp__cputest_12100) {
	PPC_FUNC_PROLOGUE();
	// bdzf so,0x12108
	--ctx.ctr.u64;
	if (ctx.ctr.u64 == 0 && !ctx.cr0.so) goto loc_12108;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12108:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12140"))) PPC_WEAK_FUNC(cputest_12140);
PPC_FUNC_IMPL(__imp__cputest_12140) {
	PPC_FUNC_PROLOGUE();
	// bdzf 4*cr1+eq,0x12148
	--ctx.ctr.u64;
	if (ctx.ctr.u64 == 0 && !ctx.cr1.eq) goto loc_12148;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12148:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12180"))) PPC_WEAK_FUNC(cputest_12180);
PPC_FUNC_IMPL(__imp__cputest_12180) {
	PPC_FUNC_PROLOGUE();
	// bdzf 4*cr2+gt,0x12188
	--ctx.ctr.u64;
	if (ctx.ctr.u64 == 0 && !ctx.cr2.gt) goto loc_12188;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12188:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_121C0"))) PPC_WEAK_FUNC(cputest_121C0);
PPC_FUNC_IMPL(__imp__cputest_121C0) {
	PPC_FUNC_PROLOGUE();
	// bdzf 4*cr4+gt,0x121c8
	--ctx.ctr.u64;
	if (ctx.ctr.u64 == 0 && !ctx.cr4.gt) goto loc_121C8;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_121C8:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12200"))) PPC_WEAK_FUNC(cputest_12200);
PPC_FUNC_IMPL(__imp__cputest_12200) {
	PPC_FUNC_PROLOGUE();
	// bdzf 4*cr5+eq,0x12208
	--ctx.ctr.u64;
	if (ctx.ctr.u64 == 0 && !ctx.cr5.eq) goto loc_12208;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12208:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12240"))) PPC_WEAK_FUNC(cputest_12240);
PPC_FUNC_IMPL(__imp__cputest_12240) {
	PPC_FUNC_PROLOGUE();
	// bdzf 4*cr7+so,0x12248
	--ctx.ctr.u64;
	if (ctx.ctr.u64 == 0 && !ctx.cr7.so) goto loc_12248;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12248:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12280"))) PPC_WEAK_FUNC(cputest_12280);
PPC_FUNC_IMPL(__imp__cputest_12280) {
	PPC_FUNC_PROLOGUE();
	// bdnzt lt,0x12288
	--ctx.ctr.u64;
	if (ctx.ctr.u64 != 0 && ctx.cr0.lt) goto loc_12288;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12288:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_122C0"))) PPC_WEAK_FUNC(cputest_122C0);
PPC_FUNC_IMPL(__imp__cputest_122C0) {
	PPC_FUNC_PROLOGUE();
	// bdnzt eq,0x122c8
	--ctx.ctr.u64;
	if (ctx.ctr.u64 != 0 && ctx.cr0.eq) goto loc_122C8;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_122C8:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12300"))) PPC_WEAK_FUNC(cputest_12300);
PPC_FUNC_IMPL(__imp__cputest_12300) {
	PPC_FUNC_PROLOGUE();
	// bdnzt so,0x12308
	--ctx.ctr.u64;
	if (ctx.ctr.u64 != 0 && ctx.cr0.so) goto loc_12308;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12308:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12340"))) PPC_WEAK_FUNC(cputest_12340);
PPC_FUNC_IMPL(__imp__cputest_12340) {
	PPC_FUNC_PROLOGUE();
	// bdnzt 4*cr1+eq,0x12348
	--ctx.ctr.u64;
	if (ctx.ctr.u64 != 0 && ctx.cr1.eq) goto loc_12348;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12348:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12380"))) PPC_WEAK_FUNC(cputest_12380);
PPC_FUNC_IMPL(__imp__cputest_12380) {
	PPC_FUNC_PROLOGUE();
	// bdnzt 4*cr2+gt,0x12388
	--ctx.ctr.u64;
	if (ctx.ctr.u64 != 0 && ctx.cr2.gt) goto loc_12388;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12388:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_123C0"))) PPC_WEAK_FUNC(cputest_123C0);
PPC_FUNC_IMPL(__imp__cputest_123C0) {
	PPC_FUNC_PROLOGUE();
	// bdnzt 4*cr4+gt,0x123c8
	--ctx.ctr.u64;
	if (ctx.ctr.u64 != 0 && ctx.cr4.gt) goto loc_123C8;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_123C8:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12400"))) PPC_WEAK_FUNC(cputest_12400);
PPC_FUNC_IMPL(__imp__cputest_12400) {
	PPC_FUNC_PROLOGUE();
	// bdnzt 4*cr5+eq,0x12408
	--ctx.ctr.u64;
	if (ctx.ctr.u64 != 0 && ctx.cr5.eq) goto loc_12408;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12408:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12440"))) PPC_WEAK_FUNC(cputest_12440);
PPC_FUNC_IMPL(__imp__cputest_12440) {
	PPC_FUNC_PROLOGUE();
	// bdnzt 4*cr7+so,0x12448
	--ctx.ctr.u64;
	if (ctx.ctr.u64 != 0 && ctx.cr7.so) goto loc_12448;
	// addi r3,r3,1
	ctx.r3.s64 = ctx.r3.s64 + 1;
loc_12448:
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12480"))) PPC_WEAK_FUNC(cputest_12480);
PPC_FUNC_IMPL(__imp__cputest_12480) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// blrl 
	temp.u32 = uint32_t(ctx.lr) & ~3u;
	ctx.lr = 0x12484;
	PPC_CALL_INDIRECT_FUNC(temp.u32);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_124C0"))) PPC_WEAK_FUNC(cputest_124C0);
PPC_FUNC_IMPL(__imp__cputest_124C0) {
	PPC_FUNC_PROLOGUE();
	// vnor v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12500"))) PPC_WEAK_FUNC(cputest_12500);
PPC_FUNC_IMPL(__imp__cputest_12500) {
	PPC_FUNC_PROLOGUE();
	// vandc v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v5.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12540"))) PPC_WEAK_FUNC(cputest_12540);
PPC_FUNC_IMPL(__imp__cputest_12540) {
	PPC_FUNC_PROLOGUE();
	// vslh v3,v4,v5
	ctx.v3.u16[0] = ctx.v4.u16[0] << (ctx.v5.u16[0] & 0xF);
	ctx.v3.u16[1] = ctx.v4.u16[1] << (ctx.v5.u16[1] & 0xF);
	ctx.v3.u16[2] = ctx.v4.u16[2] << (ctx.v5.u16[2] & 0xF);
	ctx.v3.u16[3] = ctx.v4.u16[3] << (ctx.v5.u16[3] & 0xF);
	ctx.v3.u16[4] = ctx.v4.u16[4] << (ctx.v5.u16[4] & 0xF);
	ctx.v3.u16[5] = ctx.v4.u16[5] << (ctx.v5.u16[5] & 0xF);
	ctx.v3.u16[6] = ctx.v4.u16[6] << (ctx.v5.u16[6] & 0xF);
	ctx.v3.u16[7] = ctx.v4.u16[7] << (ctx.v5.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12580"))) PPC_WEAK_FUNC(cputest_12580);
PPC_FUNC_IMPL(__imp__cputest_12580) {
	PPC_FUNC_PROLOGUE();
	// vsrh v3,v4,v5
	ctx.v3.u16[0] = ctx.v4.u16[0] >> (ctx.v5.u16[0] & 0xF);
	ctx.v3.u16[1] = ctx.v4.u16[1] >> (ctx.v5.u16[1] & 0xF);
	ctx.v3.u16[2] = ctx.v4.u16[2] >> (ctx.v5.u16[2] & 0xF);
	ctx.v3.u16[3] = ctx.v4.u16[3] >> (ctx.v5.u16[3] & 0xF);
	ctx.v3.u16[4] = ctx.v4.u16[4] >> (ctx.v5.u16[4] & 0xF);
	ctx.v3.u16[5] = ctx.v4.u16[5] >> (ctx.v5.u16[5] & 0xF);
	ctx.v3.u16[6] = ctx.v4.u16[6] >> (ctx.v5.u16[6] & 0xF);
	ctx.v3.u16[7] = ctx.v4.u16[7] >> (ctx.v5.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_125C0"))) PPC_WEAK_FUNC(cputest_125C0);
PPC_FUNC_IMPL(__imp__cputest_125C0) {
	PPC_FUNC_PROLOGUE();
	// vsrah v3,v4,v5
	ctx.v3.s16[0] = ctx.v4.s16[0] >> (ctx.v5.u16[0] & 0xF);
	ctx.v3.s16[1] = ctx.v4.s16[1] >> (ctx.v5.u16[1] & 0xF);
	ctx.v3.s16[2] = ctx.v4.s16[2] >> (ctx.v5.u16[2] & 0xF);
	ctx.v3.s16[3] = ctx.v4.s16[3] >> (ctx.v5.u16[3] & 0xF);
	ctx.v3.s16[4] = ctx.v4.s16[4] >> (ctx.v5.u16[4] & 0xF);
	ctx.v3.s16[5] = ctx.v4.s16[5] >> (ctx.v5.u16[5] & 0xF);
	ctx.v3.s16[6] = ctx.v4.s16[6] >> (ctx.v5.u16[6] & 0xF);
	ctx.v3.s16[7] = ctx.v4.s16[7] >> (ctx.v5.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12600"))) PPC_WEAK_FUNC(cputest_12600);
PPC_FUNC_IMPL(__imp__cputest_12600) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro v3,v4,v5
	temp.u8 = (ctx.v5.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v4.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v3.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12640"))) PPC_WEAK_FUNC(cputest_12640);
PPC_FUNC_IMPL(__imp__cputest_12640) {
	PPC_FUNC_PROLOGUE();
	// vminsw v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v5.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12680"))) PPC_WEAK_FUNC(cputest_12680);
PPC_FUNC_IMPL(__imp__cputest_12680) {
	PPC_FUNC_PROLOGUE();
	// vminuh v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_min_epu16(simde_mm_load_si128((simde__m128i*)ctx.v4.u16), simde_mm_load_si128((simde__m128i*)ctx.v5.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_126C0"))) PPC_WEAK_FUNC(cputest_126C0);
PPC_FUNC_IMPL(__imp__cputest_126C0) {
	PPC_FUNC_PROLOGUE();
	// vmaxuh v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_max_epu16(simde_mm_load_si128((simde__m128i*)ctx.v4.u16), simde_mm_load_si128((simde__m128i*)ctx.v5.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12700"))) PPC_WEAK_FUNC(cputest_12700);
PPC_FUNC_IMPL(__imp__cputest_12700) {
	PPC_FUNC_PROLOGUE();
	// vavguh v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_avg_epu16(simde_mm_load_si128((simde__m128i*)ctx.v4.u16), simde_mm_load_si128((simde__m128i*)ctx.v5.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12740"))) PPC_WEAK_FUNC(cputest_12740);
PPC_FUNC_IMPL(__imp__cputest_12740) {
	PPC_FUNC_PROLOGUE();
	// vsubuwm v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u32, simde_mm_sub_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.u32), simde_mm_load_si128((simde__m128i*)ctx.v5.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12780"))) PPC_WEAK_FUNC(cputest_12780);
PPC_FUNC_IMPL(__imp__cputest_12780) {
	PPC_FUNC_PROLOGUE();
	// vsububm v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_sub_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_127C0"))) PPC_WEAK_FUNC(cputest_127C0);
PPC_FUNC_IMPL(__imp__cputest_127C0) {
	PPC_FUNC_PROLOGUE();
	// vsubshs v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.s16, simde_mm_subs_epi16(simde_mm_load_si128((simde__m128i*)ctx.v4.s16), simde_mm_load_si128((simde__m128i*)ctx.v5.s16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12800"))) PPC_WEAK_FUNC(cputest_12800);
PPC_FUNC_IMPL(__imp__cputest_12800) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vaddsws v3,v4,v5
	temp.s64 = int64_t(ctx.v4.s32[0]) + int64_t(ctx.v5.s32[0]);
	ctx.v3.s32[0] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v4.s32[1]) + int64_t(ctx.v5.s32[1]);
	ctx.v3.s32[1] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v4.s32[2]) + int64_t(ctx.v5.s32[2]);
	ctx.v3.s32[2] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v4.s32[3]) + int64_t(ctx.v5.s32[3]);
	ctx.v3.s32[3] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12840"))) PPC_WEAK_FUNC(cputest_12840);
PPC_FUNC_IMPL(__imp__cputest_12840) {
	PPC_FUNC_PROLOGUE();
	// vpkswss v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v5.s32), simde_mm_load_si128((simde__m128i*)ctx.v4.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12880"))) PPC_WEAK_FUNC(cputest_12880);
PPC_FUNC_IMPL(__imp__cputest_12880) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.s8), simde_mm_load_si128((simde__m128i*)ctx.v5.s8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_128C0"))) PPC_WEAK_FUNC(cputest_128C0);
PPC_FUNC_IMPL(__imp__cputest_128C0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb. v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.s8), simde_mm_load_si128((simde__m128i*)ctx.v5.s8)));
	ctx.cr6.setFromMask(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 0xFFFF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12900"))) PPC_WEAK_FUNC(cputest_12900);
PPC_FUNC_IMPL(__imp__cputest_12900) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v5.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12940"))) PPC_WEAK_FUNC(cputest_12940);
PPC_FUNC_IMPL(__imp__cputest_12940) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw. v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v5.s32)));
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v3.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12980"))) PPC_WEAK_FUNC(cputest_12980);
PPC_FUNC_IMPL(__imp__cputest_12980) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v3,v4,v5
	ctx.v3.u32[0] = ctx.v4.u32[0] > ctx.v5.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[1] = ctx.v4.u32[1] > ctx.v5.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[2] = ctx.v4.u32[2] > ctx.v5.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[3] = ctx.v4.u32[3] > ctx.v5.u32[3] ? 0xFFFFFFFF : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_129C0"))) PPC_WEAK_FUNC(cputest_129C0);
PPC_FUNC_IMPL(__imp__cputest_129C0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw. v3,v4,v5
	ctx.v3.u32[0] = ctx.v4.u32[0] > ctx.v5.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[1] = ctx.v4.u32[1] > ctx.v5.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[2] = ctx.v4.u32[2] > ctx.v5.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[3] = ctx.v4.u32[3] > ctx.v5.u32[3] ? 0xFFFFFFFF : 0;
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v3.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12A00"))) PPC_WEAK_FUNC(cputest_12A00);
PPC_FUNC_IMPL(__imp__cputest_12A00) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0x0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12A40"))) PPC_WEAK_FUNC(cputest_12A40);
PPC_FUNC_IMPL(__imp__cputest_12A40) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0x1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12A80"))) PPC_WEAK_FUNC(cputest_12A80);
PPC_FUNC_IMPL(__imp__cputest_12A80) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12AC0"))) PPC_WEAK_FUNC(cputest_12AC0);
PPC_FUNC_IMPL(__imp__cputest_12AC0) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,-16
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xFFFFFFF0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12B00"))) PPC_WEAK_FUNC(cputest_12B00);
PPC_FUNC_IMPL(__imp__cputest_12B00) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,-1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xFFFFFFFF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12B40"))) PPC_WEAK_FUNC(cputest_12B40);
PPC_FUNC_IMPL(__imp__cputest_12B40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v5,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 1.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 1.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 1.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 1.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12B80"))) PPC_WEAK_FUNC(cputest_12B80);
PPC_FUNC_IMPL(__imp__cputest_12B80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v5,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 2.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 2.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 2.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 2.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12BC0"))) PPC_WEAK_FUNC(cputest_12BC0);
PPC_FUNC_IMPL(__imp__cputest_12BC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v5,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 256.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 256.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 256.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 256.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12C00"))) PPC_WEAK_FUNC(cputest_12C00);
PPC_FUNC_IMPL(__imp__cputest_12C00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v5,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 2147483648.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 2147483648.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 2147483648.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 2147483648.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12C40"))) PPC_WEAK_FUNC(cputest_12C40);
PPC_FUNC_IMPL(__imp__cputest_12C40) {
	PPC_FUNC_PROLOGUE();
	// vsel v3,v4,v5,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12C80"))) PPC_WEAK_FUNC(cputest_12C80);
PPC_FUNC_IMPL(__imp__cputest_12C80) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v5,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12CC0"))) PPC_WEAK_FUNC(cputest_12CC0);
PPC_FUNC_IMPL(__imp__cputest_12CC0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v5,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12D00"))) PPC_WEAK_FUNC(cputest_12D00);
PPC_FUNC_IMPL(__imp__cputest_12D00) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v5,5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12D40"))) PPC_WEAK_FUNC(cputest_12D40);
PPC_FUNC_IMPL(__imp__cputest_12D40) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v5,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12D80"))) PPC_WEAK_FUNC(cputest_12D80);
PPC_FUNC_IMPL(__imp__cputest_12D80) {
	PPC_FUNC_PROLOGUE();
	// vnor v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12DC0"))) PPC_WEAK_FUNC(cputest_12DC0);
PPC_FUNC_IMPL(__imp__cputest_12DC0) {
	PPC_FUNC_PROLOGUE();
	// vandc v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v5.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12E00"))) PPC_WEAK_FUNC(cputest_12E00);
PPC_FUNC_IMPL(__imp__cputest_12E00) {
	PPC_FUNC_PROLOGUE();
	// vslh v3,v3,v5
	ctx.v3.u16[0] = ctx.v3.u16[0] << (ctx.v5.u16[0] & 0xF);
	ctx.v3.u16[1] = ctx.v3.u16[1] << (ctx.v5.u16[1] & 0xF);
	ctx.v3.u16[2] = ctx.v3.u16[2] << (ctx.v5.u16[2] & 0xF);
	ctx.v3.u16[3] = ctx.v3.u16[3] << (ctx.v5.u16[3] & 0xF);
	ctx.v3.u16[4] = ctx.v3.u16[4] << (ctx.v5.u16[4] & 0xF);
	ctx.v3.u16[5] = ctx.v3.u16[5] << (ctx.v5.u16[5] & 0xF);
	ctx.v3.u16[6] = ctx.v3.u16[6] << (ctx.v5.u16[6] & 0xF);
	ctx.v3.u16[7] = ctx.v3.u16[7] << (ctx.v5.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12E40"))) PPC_WEAK_FUNC(cputest_12E40);
PPC_FUNC_IMPL(__imp__cputest_12E40) {
	PPC_FUNC_PROLOGUE();
	// vsrh v3,v3,v5
	ctx.v3.u16[0] = ctx.v3.u16[0] >> (ctx.v5.u16[0] & 0xF);
	ctx.v3.u16[1] = ctx.v3.u16[1] >> (ctx.v5.u16[1] & 0xF);
	ctx.v3.u16[2] = ctx.v3.u16[2] >> (ctx.v5.u16[2] & 0xF);
	ctx.v3.u16[3] = ctx.v3.u16[3] >> (ctx.v5.u16[3] & 0xF);
	ctx.v3.u16[4] = ctx.v3.u16[4] >> (ctx.v5.u16[4] & 0xF);
	ctx.v3.u16[5] = ctx.v3.u16[5] >> (ctx.v5.u16[5] & 0xF);
	ctx.v3.u16[6] = ctx.v3.u16[6] >> (ctx.v5.u16[6] & 0xF);
	ctx.v3.u16[7] = ctx.v3.u16[7] >> (ctx.v5.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12E80"))) PPC_WEAK_FUNC(cputest_12E80);
PPC_FUNC_IMPL(__imp__cputest_12E80) {
	PPC_FUNC_PROLOGUE();
	// vsrah v3,v3,v5
	ctx.v3.s16[0] = ctx.v3.s16[0] >> (ctx.v5.u16[0] & 0xF);
	ctx.v3.s16[1] = ctx.v3.s16[1] >> (ctx.v5.u16[1] & 0xF);
	ctx.v3.s16[2] = ctx.v3.s16[2] >> (ctx.v5.u16[2] & 0xF);
	ctx.v3.s16[3] = ctx.v3.s16[3] >> (ctx.v5.u16[3] & 0xF);
	ctx.v3.s16[4] = ctx.v3.s16[4] >> (ctx.v5.u16[4] & 0xF);
	ctx.v3.s16[5] = ctx.v3.s16[5] >> (ctx.v5.u16[5] & 0xF);
	ctx.v3.s16[6] = ctx.v3.s16[6] >> (ctx.v5.u16[6] & 0xF);
	ctx.v3.s16[7] = ctx.v3.s16[7] >> (ctx.v5.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12EC0"))) PPC_WEAK_FUNC(cputest_12EC0);
PPC_FUNC_IMPL(__imp__cputest_12EC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro v3,v3,v5
	temp.u8 = (ctx.v5.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v3.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v3.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12F00"))) PPC_WEAK_FUNC(cputest_12F00);
PPC_FUNC_IMPL(__imp__cputest_12F00) {
	PPC_FUNC_PROLOGUE();
	// vminsw v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v3.s32), simde_mm_load_si128((simde__m128i*)ctx.v5.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12F40"))) PPC_WEAK_FUNC(cputest_12F40);
PPC_FUNC_IMPL(__imp__cputest_12F40) {
	PPC_FUNC_PROLOGUE();
	// vminuh v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_min_epu16(simde_mm_load_si128((simde__m128i*)ctx.v3.u16), simde_mm_load_si128((simde__m128i*)ctx.v5.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12F80"))) PPC_WEAK_FUNC(cputest_12F80);
PPC_FUNC_IMPL(__imp__cputest_12F80) {
	PPC_FUNC_PROLOGUE();
	// vmaxuh v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_max_epu16(simde_mm_load_si128((simde__m128i*)ctx.v3.u16), simde_mm_load_si128((simde__m128i*)ctx.v5.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_12FC0"))) PPC_WEAK_FUNC(cputest_12FC0);
PPC_FUNC_IMPL(__imp__cputest_12FC0) {
	PPC_FUNC_PROLOGUE();
	// vavguh v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_avg_epu16(simde_mm_load_si128((simde__m128i*)ctx.v3.u16), simde_mm_load_si128((simde__m128i*)ctx.v5.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13000"))) PPC_WEAK_FUNC(cputest_13000);
PPC_FUNC_IMPL(__imp__cputest_13000) {
	PPC_FUNC_PROLOGUE();
	// vsubuwm v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u32, simde_mm_sub_epi32(simde_mm_load_si128((simde__m128i*)ctx.v3.u32), simde_mm_load_si128((simde__m128i*)ctx.v5.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13040"))) PPC_WEAK_FUNC(cputest_13040);
PPC_FUNC_IMPL(__imp__cputest_13040) {
	PPC_FUNC_PROLOGUE();
	// vsububm v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_sub_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13080"))) PPC_WEAK_FUNC(cputest_13080);
PPC_FUNC_IMPL(__imp__cputest_13080) {
	PPC_FUNC_PROLOGUE();
	// vsubshs v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.s16, simde_mm_subs_epi16(simde_mm_load_si128((simde__m128i*)ctx.v3.s16), simde_mm_load_si128((simde__m128i*)ctx.v5.s16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_130C0"))) PPC_WEAK_FUNC(cputest_130C0);
PPC_FUNC_IMPL(__imp__cputest_130C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vaddsws v3,v3,v5
	temp.s64 = int64_t(ctx.v3.s32[0]) + int64_t(ctx.v5.s32[0]);
	ctx.v3.s32[0] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v3.s32[1]) + int64_t(ctx.v5.s32[1]);
	ctx.v3.s32[1] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v3.s32[2]) + int64_t(ctx.v5.s32[2]);
	ctx.v3.s32[2] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v3.s32[3]) + int64_t(ctx.v5.s32[3]);
	ctx.v3.s32[3] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13100"))) PPC_WEAK_FUNC(cputest_13100);
PPC_FUNC_IMPL(__imp__cputest_13100) {
	PPC_FUNC_PROLOGUE();
	// vpkswss v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v5.s32), simde_mm_load_si128((simde__m128i*)ctx.v3.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13140"))) PPC_WEAK_FUNC(cputest_13140);
PPC_FUNC_IMPL(__imp__cputest_13140) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.s8), simde_mm_load_si128((simde__m128i*)ctx.v5.s8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13180"))) PPC_WEAK_FUNC(cputest_13180);
PPC_FUNC_IMPL(__imp__cputest_13180) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb. v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.s8), simde_mm_load_si128((simde__m128i*)ctx.v5.s8)));
	ctx.cr6.setFromMask(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 0xFFFF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_131C0"))) PPC_WEAK_FUNC(cputest_131C0);
PPC_FUNC_IMPL(__imp__cputest_131C0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v3.s32), simde_mm_load_si128((simde__m128i*)ctx.v5.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13200"))) PPC_WEAK_FUNC(cputest_13200);
PPC_FUNC_IMPL(__imp__cputest_13200) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw. v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v3.s32), simde_mm_load_si128((simde__m128i*)ctx.v5.s32)));
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v3.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13240"))) PPC_WEAK_FUNC(cputest_13240);
PPC_FUNC_IMPL(__imp__cputest_13240) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v3,v3,v5
	ctx.v3.u32[0] = ctx.v3.u32[0] > ctx.v5.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[1] = ctx.v3.u32[1] > ctx.v5.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[2] = ctx.v3.u32[2] > ctx.v5.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[3] = ctx.v3.u32[3] > ctx.v5.u32[3] ? 0xFFFFFFFF : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13280"))) PPC_WEAK_FUNC(cputest_13280);
PPC_FUNC_IMPL(__imp__cputest_13280) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw. v3,v3,v5
	ctx.v3.u32[0] = ctx.v3.u32[0] > ctx.v5.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[1] = ctx.v3.u32[1] > ctx.v5.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[2] = ctx.v3.u32[2] > ctx.v5.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[3] = ctx.v3.u32[3] > ctx.v5.u32[3] ? 0xFFFFFFFF : 0;
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v3.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_132C0"))) PPC_WEAK_FUNC(cputest_132C0);
PPC_FUNC_IMPL(__imp__cputest_132C0) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0x0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13300"))) PPC_WEAK_FUNC(cputest_13300);
PPC_FUNC_IMPL(__imp__cputest_13300) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0x1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13340"))) PPC_WEAK_FUNC(cputest_13340);
PPC_FUNC_IMPL(__imp__cputest_13340) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13380"))) PPC_WEAK_FUNC(cputest_13380);
PPC_FUNC_IMPL(__imp__cputest_13380) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,-16
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xFFFFFFF0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_133C0"))) PPC_WEAK_FUNC(cputest_133C0);
PPC_FUNC_IMPL(__imp__cputest_133C0) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,-1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xFFFFFFFF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13400"))) PPC_WEAK_FUNC(cputest_13400);
PPC_FUNC_IMPL(__imp__cputest_13400) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v5,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 1.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 1.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 1.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 1.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13440"))) PPC_WEAK_FUNC(cputest_13440);
PPC_FUNC_IMPL(__imp__cputest_13440) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v5,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 2.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 2.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 2.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 2.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13480"))) PPC_WEAK_FUNC(cputest_13480);
PPC_FUNC_IMPL(__imp__cputest_13480) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v5,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 256.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 256.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 256.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 256.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_134C0"))) PPC_WEAK_FUNC(cputest_134C0);
PPC_FUNC_IMPL(__imp__cputest_134C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v5,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 2147483648.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 2147483648.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 2147483648.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 2147483648.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13500"))) PPC_WEAK_FUNC(cputest_13500);
PPC_FUNC_IMPL(__imp__cputest_13500) {
	PPC_FUNC_PROLOGUE();
	// vsel v3,v3,v5,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13540"))) PPC_WEAK_FUNC(cputest_13540);
PPC_FUNC_IMPL(__imp__cputest_13540) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v3,v5,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13580"))) PPC_WEAK_FUNC(cputest_13580);
PPC_FUNC_IMPL(__imp__cputest_13580) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v3,v5,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_135C0"))) PPC_WEAK_FUNC(cputest_135C0);
PPC_FUNC_IMPL(__imp__cputest_135C0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v3,v5,5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13600"))) PPC_WEAK_FUNC(cputest_13600);
PPC_FUNC_IMPL(__imp__cputest_13600) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v3,v5,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13640"))) PPC_WEAK_FUNC(cputest_13640);
PPC_FUNC_IMPL(__imp__cputest_13640) {
	PPC_FUNC_PROLOGUE();
	// vnor v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13680"))) PPC_WEAK_FUNC(cputest_13680);
PPC_FUNC_IMPL(__imp__cputest_13680) {
	PPC_FUNC_PROLOGUE();
	// vandc v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_136C0"))) PPC_WEAK_FUNC(cputest_136C0);
PPC_FUNC_IMPL(__imp__cputest_136C0) {
	PPC_FUNC_PROLOGUE();
	// vslh v3,v4,v3
	ctx.v3.u16[0] = ctx.v4.u16[0] << (ctx.v3.u16[0] & 0xF);
	ctx.v3.u16[1] = ctx.v4.u16[1] << (ctx.v3.u16[1] & 0xF);
	ctx.v3.u16[2] = ctx.v4.u16[2] << (ctx.v3.u16[2] & 0xF);
	ctx.v3.u16[3] = ctx.v4.u16[3] << (ctx.v3.u16[3] & 0xF);
	ctx.v3.u16[4] = ctx.v4.u16[4] << (ctx.v3.u16[4] & 0xF);
	ctx.v3.u16[5] = ctx.v4.u16[5] << (ctx.v3.u16[5] & 0xF);
	ctx.v3.u16[6] = ctx.v4.u16[6] << (ctx.v3.u16[6] & 0xF);
	ctx.v3.u16[7] = ctx.v4.u16[7] << (ctx.v3.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13700"))) PPC_WEAK_FUNC(cputest_13700);
PPC_FUNC_IMPL(__imp__cputest_13700) {
	PPC_FUNC_PROLOGUE();
	// vsrh v3,v4,v3
	ctx.v3.u16[0] = ctx.v4.u16[0] >> (ctx.v3.u16[0] & 0xF);
	ctx.v3.u16[1] = ctx.v4.u16[1] >> (ctx.v3.u16[1] & 0xF);
	ctx.v3.u16[2] = ctx.v4.u16[2] >> (ctx.v3.u16[2] & 0xF);
	ctx.v3.u16[3] = ctx.v4.u16[3] >> (ctx.v3.u16[3] & 0xF);
	ctx.v3.u16[4] = ctx.v4.u16[4] >> (ctx.v3.u16[4] & 0xF);
	ctx.v3.u16[5] = ctx.v4.u16[5] >> (ctx.v3.u16[5] & 0xF);
	ctx.v3.u16[6] = ctx.v4.u16[6] >> (ctx.v3.u16[6] & 0xF);
	ctx.v3.u16[7] = ctx.v4.u16[7] >> (ctx.v3.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13740"))) PPC_WEAK_FUNC(cputest_13740);
PPC_FUNC_IMPL(__imp__cputest_13740) {
	PPC_FUNC_PROLOGUE();
	// vsrah v3,v4,v3
	ctx.v3.s16[0] = ctx.v4.s16[0] >> (ctx.v3.u16[0] & 0xF);
	ctx.v3.s16[1] = ctx.v4.s16[1] >> (ctx.v3.u16[1] & 0xF);
	ctx.v3.s16[2] = ctx.v4.s16[2] >> (ctx.v3.u16[2] & 0xF);
	ctx.v3.s16[3] = ctx.v4.s16[3] >> (ctx.v3.u16[3] & 0xF);
	ctx.v3.s16[4] = ctx.v4.s16[4] >> (ctx.v3.u16[4] & 0xF);
	ctx.v3.s16[5] = ctx.v4.s16[5] >> (ctx.v3.u16[5] & 0xF);
	ctx.v3.s16[6] = ctx.v4.s16[6] >> (ctx.v3.u16[6] & 0xF);
	ctx.v3.s16[7] = ctx.v4.s16[7] >> (ctx.v3.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13780"))) PPC_WEAK_FUNC(cputest_13780);
PPC_FUNC_IMPL(__imp__cputest_13780) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro v3,v4,v3
	temp.u8 = (ctx.v3.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v4.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v3.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_137C0"))) PPC_WEAK_FUNC(cputest_137C0);
PPC_FUNC_IMPL(__imp__cputest_137C0) {
	PPC_FUNC_PROLOGUE();
	// vminsw v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v3.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13800"))) PPC_WEAK_FUNC(cputest_13800);
PPC_FUNC_IMPL(__imp__cputest_13800) {
	PPC_FUNC_PROLOGUE();
	// vminuh v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_min_epu16(simde_mm_load_si128((simde__m128i*)ctx.v4.u16), simde_mm_load_si128((simde__m128i*)ctx.v3.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13840"))) PPC_WEAK_FUNC(cputest_13840);
PPC_FUNC_IMPL(__imp__cputest_13840) {
	PPC_FUNC_PROLOGUE();
	// vmaxuh v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_max_epu16(simde_mm_load_si128((simde__m128i*)ctx.v4.u16), simde_mm_load_si128((simde__m128i*)ctx.v3.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13880"))) PPC_WEAK_FUNC(cputest_13880);
PPC_FUNC_IMPL(__imp__cputest_13880) {
	PPC_FUNC_PROLOGUE();
	// vavguh v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_avg_epu16(simde_mm_load_si128((simde__m128i*)ctx.v4.u16), simde_mm_load_si128((simde__m128i*)ctx.v3.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_138C0"))) PPC_WEAK_FUNC(cputest_138C0);
PPC_FUNC_IMPL(__imp__cputest_138C0) {
	PPC_FUNC_PROLOGUE();
	// vsubuwm v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u32, simde_mm_sub_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.u32), simde_mm_load_si128((simde__m128i*)ctx.v3.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13900"))) PPC_WEAK_FUNC(cputest_13900);
PPC_FUNC_IMPL(__imp__cputest_13900) {
	PPC_FUNC_PROLOGUE();
	// vsububm v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_sub_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13940"))) PPC_WEAK_FUNC(cputest_13940);
PPC_FUNC_IMPL(__imp__cputest_13940) {
	PPC_FUNC_PROLOGUE();
	// vsubshs v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.s16, simde_mm_subs_epi16(simde_mm_load_si128((simde__m128i*)ctx.v4.s16), simde_mm_load_si128((simde__m128i*)ctx.v3.s16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13980"))) PPC_WEAK_FUNC(cputest_13980);
PPC_FUNC_IMPL(__imp__cputest_13980) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vaddsws v3,v4,v3
	temp.s64 = int64_t(ctx.v4.s32[0]) + int64_t(ctx.v3.s32[0]);
	ctx.v3.s32[0] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v4.s32[1]) + int64_t(ctx.v3.s32[1]);
	ctx.v3.s32[1] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v4.s32[2]) + int64_t(ctx.v3.s32[2]);
	ctx.v3.s32[2] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v4.s32[3]) + int64_t(ctx.v3.s32[3]);
	ctx.v3.s32[3] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_139C0"))) PPC_WEAK_FUNC(cputest_139C0);
PPC_FUNC_IMPL(__imp__cputest_139C0) {
	PPC_FUNC_PROLOGUE();
	// vpkswss v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v3.s32), simde_mm_load_si128((simde__m128i*)ctx.v4.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13A00"))) PPC_WEAK_FUNC(cputest_13A00);
PPC_FUNC_IMPL(__imp__cputest_13A00) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.s8), simde_mm_load_si128((simde__m128i*)ctx.v3.s8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13A40"))) PPC_WEAK_FUNC(cputest_13A40);
PPC_FUNC_IMPL(__imp__cputest_13A40) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb. v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.s8), simde_mm_load_si128((simde__m128i*)ctx.v3.s8)));
	ctx.cr6.setFromMask(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 0xFFFF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13A80"))) PPC_WEAK_FUNC(cputest_13A80);
PPC_FUNC_IMPL(__imp__cputest_13A80) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v3.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13AC0"))) PPC_WEAK_FUNC(cputest_13AC0);
PPC_FUNC_IMPL(__imp__cputest_13AC0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw. v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v3.s32)));
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v3.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13B00"))) PPC_WEAK_FUNC(cputest_13B00);
PPC_FUNC_IMPL(__imp__cputest_13B00) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v3,v4,v3
	ctx.v3.u32[0] = ctx.v4.u32[0] > ctx.v3.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[1] = ctx.v4.u32[1] > ctx.v3.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[2] = ctx.v4.u32[2] > ctx.v3.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[3] = ctx.v4.u32[3] > ctx.v3.u32[3] ? 0xFFFFFFFF : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13B40"))) PPC_WEAK_FUNC(cputest_13B40);
PPC_FUNC_IMPL(__imp__cputest_13B40) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw. v3,v4,v3
	ctx.v3.u32[0] = ctx.v4.u32[0] > ctx.v3.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[1] = ctx.v4.u32[1] > ctx.v3.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[2] = ctx.v4.u32[2] > ctx.v3.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[3] = ctx.v4.u32[3] > ctx.v3.u32[3] ? 0xFFFFFFFF : 0;
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v3.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13B80"))) PPC_WEAK_FUNC(cputest_13B80);
PPC_FUNC_IMPL(__imp__cputest_13B80) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0x0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13BC0"))) PPC_WEAK_FUNC(cputest_13BC0);
PPC_FUNC_IMPL(__imp__cputest_13BC0) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0x1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13C00"))) PPC_WEAK_FUNC(cputest_13C00);
PPC_FUNC_IMPL(__imp__cputest_13C00) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13C40"))) PPC_WEAK_FUNC(cputest_13C40);
PPC_FUNC_IMPL(__imp__cputest_13C40) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,-16
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xFFFFFFF0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13C80"))) PPC_WEAK_FUNC(cputest_13C80);
PPC_FUNC_IMPL(__imp__cputest_13C80) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,-1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xFFFFFFFF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13CC0"))) PPC_WEAK_FUNC(cputest_13CC0);
PPC_FUNC_IMPL(__imp__cputest_13CC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v3,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v3.f32[0]) * 1.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[1]) * 1.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[2]) * 1.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[3]) * 1.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13D00"))) PPC_WEAK_FUNC(cputest_13D00);
PPC_FUNC_IMPL(__imp__cputest_13D00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v3,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v3.f32[0]) * 2.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[1]) * 2.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[2]) * 2.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[3]) * 2.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13D40"))) PPC_WEAK_FUNC(cputest_13D40);
PPC_FUNC_IMPL(__imp__cputest_13D40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v3,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v3.f32[0]) * 256.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[1]) * 256.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[2]) * 256.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[3]) * 256.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13D80"))) PPC_WEAK_FUNC(cputest_13D80);
PPC_FUNC_IMPL(__imp__cputest_13D80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v3,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v3.f32[0]) * 2147483648.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[1]) * 2147483648.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[2]) * 2147483648.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[3]) * 2147483648.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13DC0"))) PPC_WEAK_FUNC(cputest_13DC0);
PPC_FUNC_IMPL(__imp__cputest_13DC0) {
	PPC_FUNC_PROLOGUE();
	// vsel v3,v4,v3,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13E00"))) PPC_WEAK_FUNC(cputest_13E00);
PPC_FUNC_IMPL(__imp__cputest_13E00) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v3,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13E40"))) PPC_WEAK_FUNC(cputest_13E40);
PPC_FUNC_IMPL(__imp__cputest_13E40) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v3,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13E80"))) PPC_WEAK_FUNC(cputest_13E80);
PPC_FUNC_IMPL(__imp__cputest_13E80) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v3,5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13EC0"))) PPC_WEAK_FUNC(cputest_13EC0);
PPC_FUNC_IMPL(__imp__cputest_13EC0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v3,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13F00"))) PPC_WEAK_FUNC(cputest_13F00);
PPC_FUNC_IMPL(__imp__cputest_13F00) {
	PPC_FUNC_PROLOGUE();
	// vnor v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13F40"))) PPC_WEAK_FUNC(cputest_13F40);
PPC_FUNC_IMPL(__imp__cputest_13F40) {
	PPC_FUNC_PROLOGUE();
	// vandc v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13F80"))) PPC_WEAK_FUNC(cputest_13F80);
PPC_FUNC_IMPL(__imp__cputest_13F80) {
	PPC_FUNC_PROLOGUE();
	// vslh v3,v4,v4
	ctx.v3.u16[0] = ctx.v4.u16[0] << (ctx.v4.u16[0] & 0xF);
	ctx.v3.u16[1] = ctx.v4.u16[1] << (ctx.v4.u16[1] & 0xF);
	ctx.v3.u16[2] = ctx.v4.u16[2] << (ctx.v4.u16[2] & 0xF);
	ctx.v3.u16[3] = ctx.v4.u16[3] << (ctx.v4.u16[3] & 0xF);
	ctx.v3.u16[4] = ctx.v4.u16[4] << (ctx.v4.u16[4] & 0xF);
	ctx.v3.u16[5] = ctx.v4.u16[5] << (ctx.v4.u16[5] & 0xF);
	ctx.v3.u16[6] = ctx.v4.u16[6] << (ctx.v4.u16[6] & 0xF);
	ctx.v3.u16[7] = ctx.v4.u16[7] << (ctx.v4.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_13FC0"))) PPC_WEAK_FUNC(cputest_13FC0);
PPC_FUNC_IMPL(__imp__cputest_13FC0) {
	PPC_FUNC_PROLOGUE();
	// vsrh v3,v4,v4
	ctx.v3.u16[0] = ctx.v4.u16[0] >> (ctx.v4.u16[0] & 0xF);
	ctx.v3.u16[1] = ctx.v4.u16[1] >> (ctx.v4.u16[1] & 0xF);
	ctx.v3.u16[2] = ctx.v4.u16[2] >> (ctx.v4.u16[2] & 0xF);
	ctx.v3.u16[3] = ctx.v4.u16[3] >> (ctx.v4.u16[3] & 0xF);
	ctx.v3.u16[4] = ctx.v4.u16[4] >> (ctx.v4.u16[4] & 0xF);
	ctx.v3.u16[5] = ctx.v4.u16[5] >> (ctx.v4.u16[5] & 0xF);
	ctx.v3.u16[6] = ctx.v4.u16[6] >> (ctx.v4.u16[6] & 0xF);
	ctx.v3.u16[7] = ctx.v4.u16[7] >> (ctx.v4.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14000"))) PPC_WEAK_FUNC(cputest_14000);
PPC_FUNC_IMPL(__imp__cputest_14000) {
	PPC_FUNC_PROLOGUE();
	// vsrah v3,v4,v4
	ctx.v3.s16[0] = ctx.v4.s16[0] >> (ctx.v4.u16[0] & 0xF);
	ctx.v3.s16[1] = ctx.v4.s16[1] >> (ctx.v4.u16[1] & 0xF);
	ctx.v3.s16[2] = ctx.v4.s16[2] >> (ctx.v4.u16[2] & 0xF);
	ctx.v3.s16[3] = ctx.v4.s16[3] >> (ctx.v4.u16[3] & 0xF);
	ctx.v3.s16[4] = ctx.v4.s16[4] >> (ctx.v4.u16[4] & 0xF);
	ctx.v3.s16[5] = ctx.v4.s16[5] >> (ctx.v4.u16[5] & 0xF);
	ctx.v3.s16[6] = ctx.v4.s16[6] >> (ctx.v4.u16[6] & 0xF);
	ctx.v3.s16[7] = ctx.v4.s16[7] >> (ctx.v4.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14040"))) PPC_WEAK_FUNC(cputest_14040);
PPC_FUNC_IMPL(__imp__cputest_14040) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro v3,v4,v4
	temp.u8 = (ctx.v4.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v4.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v3.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14080"))) PPC_WEAK_FUNC(cputest_14080);
PPC_FUNC_IMPL(__imp__cputest_14080) {
	PPC_FUNC_PROLOGUE();
	// vminsw v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v4.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_140C0"))) PPC_WEAK_FUNC(cputest_140C0);
PPC_FUNC_IMPL(__imp__cputest_140C0) {
	PPC_FUNC_PROLOGUE();
	// vminuh v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_min_epu16(simde_mm_load_si128((simde__m128i*)ctx.v4.u16), simde_mm_load_si128((simde__m128i*)ctx.v4.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14100"))) PPC_WEAK_FUNC(cputest_14100);
PPC_FUNC_IMPL(__imp__cputest_14100) {
	PPC_FUNC_PROLOGUE();
	// vmaxuh v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_max_epu16(simde_mm_load_si128((simde__m128i*)ctx.v4.u16), simde_mm_load_si128((simde__m128i*)ctx.v4.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14140"))) PPC_WEAK_FUNC(cputest_14140);
PPC_FUNC_IMPL(__imp__cputest_14140) {
	PPC_FUNC_PROLOGUE();
	// vavguh v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_avg_epu16(simde_mm_load_si128((simde__m128i*)ctx.v4.u16), simde_mm_load_si128((simde__m128i*)ctx.v4.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14180"))) PPC_WEAK_FUNC(cputest_14180);
PPC_FUNC_IMPL(__imp__cputest_14180) {
	PPC_FUNC_PROLOGUE();
	// vsubuwm v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u32, simde_mm_sub_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.u32), simde_mm_load_si128((simde__m128i*)ctx.v4.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_141C0"))) PPC_WEAK_FUNC(cputest_141C0);
PPC_FUNC_IMPL(__imp__cputest_141C0) {
	PPC_FUNC_PROLOGUE();
	// vsububm v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_sub_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14200"))) PPC_WEAK_FUNC(cputest_14200);
PPC_FUNC_IMPL(__imp__cputest_14200) {
	PPC_FUNC_PROLOGUE();
	// vsubshs v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.s16, simde_mm_subs_epi16(simde_mm_load_si128((simde__m128i*)ctx.v4.s16), simde_mm_load_si128((simde__m128i*)ctx.v4.s16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14240"))) PPC_WEAK_FUNC(cputest_14240);
PPC_FUNC_IMPL(__imp__cputest_14240) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vaddsws v3,v4,v4
	temp.s64 = int64_t(ctx.v4.s32[0]) + int64_t(ctx.v4.s32[0]);
	ctx.v3.s32[0] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v4.s32[1]) + int64_t(ctx.v4.s32[1]);
	ctx.v3.s32[1] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v4.s32[2]) + int64_t(ctx.v4.s32[2]);
	ctx.v3.s32[2] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v4.s32[3]) + int64_t(ctx.v4.s32[3]);
	ctx.v3.s32[3] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14280"))) PPC_WEAK_FUNC(cputest_14280);
PPC_FUNC_IMPL(__imp__cputest_14280) {
	PPC_FUNC_PROLOGUE();
	// vpkswss v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v4.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_142C0"))) PPC_WEAK_FUNC(cputest_142C0);
PPC_FUNC_IMPL(__imp__cputest_142C0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.s8), simde_mm_load_si128((simde__m128i*)ctx.v4.s8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14300"))) PPC_WEAK_FUNC(cputest_14300);
PPC_FUNC_IMPL(__imp__cputest_14300) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb. v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.s8), simde_mm_load_si128((simde__m128i*)ctx.v4.s8)));
	ctx.cr6.setFromMask(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 0xFFFF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14340"))) PPC_WEAK_FUNC(cputest_14340);
PPC_FUNC_IMPL(__imp__cputest_14340) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v4.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14380"))) PPC_WEAK_FUNC(cputest_14380);
PPC_FUNC_IMPL(__imp__cputest_14380) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw. v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v4.s32)));
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v3.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_143C0"))) PPC_WEAK_FUNC(cputest_143C0);
PPC_FUNC_IMPL(__imp__cputest_143C0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v3,v4,v4
	ctx.v3.u32[0] = ctx.v4.u32[0] > ctx.v4.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[1] = ctx.v4.u32[1] > ctx.v4.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[2] = ctx.v4.u32[2] > ctx.v4.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[3] = ctx.v4.u32[3] > ctx.v4.u32[3] ? 0xFFFFFFFF : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14400"))) PPC_WEAK_FUNC(cputest_14400);
PPC_FUNC_IMPL(__imp__cputest_14400) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw. v3,v4,v4
	ctx.v3.u32[0] = ctx.v4.u32[0] > ctx.v4.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[1] = ctx.v4.u32[1] > ctx.v4.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[2] = ctx.v4.u32[2] > ctx.v4.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v3.u32[3] = ctx.v4.u32[3] > ctx.v4.u32[3] ? 0xFFFFFFFF : 0;
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v3.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14440"))) PPC_WEAK_FUNC(cputest_14440);
PPC_FUNC_IMPL(__imp__cputest_14440) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0x0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14480"))) PPC_WEAK_FUNC(cputest_14480);
PPC_FUNC_IMPL(__imp__cputest_14480) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0x1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_144C0"))) PPC_WEAK_FUNC(cputest_144C0);
PPC_FUNC_IMPL(__imp__cputest_144C0) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14500"))) PPC_WEAK_FUNC(cputest_14500);
PPC_FUNC_IMPL(__imp__cputest_14500) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,-16
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xFFFFFFF0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14540"))) PPC_WEAK_FUNC(cputest_14540);
PPC_FUNC_IMPL(__imp__cputest_14540) {
	PPC_FUNC_PROLOGUE();
	// vspltish v3,-1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u16, simde_mm_set1_epi16(short(0xFFFFFFFF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14580"))) PPC_WEAK_FUNC(cputest_14580);
PPC_FUNC_IMPL(__imp__cputest_14580) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v4,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v4.f32[0]) * 1.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[1]) * 1.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[2]) * 1.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[3]) * 1.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_145C0"))) PPC_WEAK_FUNC(cputest_145C0);
PPC_FUNC_IMPL(__imp__cputest_145C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v4,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v4.f32[0]) * 2.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[1]) * 2.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[2]) * 2.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[3]) * 2.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14600"))) PPC_WEAK_FUNC(cputest_14600);
PPC_FUNC_IMPL(__imp__cputest_14600) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v4,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v4.f32[0]) * 256.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[1]) * 256.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[2]) * 256.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[3]) * 256.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14640"))) PPC_WEAK_FUNC(cputest_14640);
PPC_FUNC_IMPL(__imp__cputest_14640) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v3,v4,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v4.f32[0]) * 2147483648.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[1]) * 2147483648.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[2]) * 2147483648.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[3]) * 2147483648.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14680"))) PPC_WEAK_FUNC(cputest_14680);
PPC_FUNC_IMPL(__imp__cputest_14680) {
	PPC_FUNC_PROLOGUE();
	// vsel v3,v4,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_146C0"))) PPC_WEAK_FUNC(cputest_146C0);
PPC_FUNC_IMPL(__imp__cputest_146C0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v4,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14700"))) PPC_WEAK_FUNC(cputest_14700);
PPC_FUNC_IMPL(__imp__cputest_14700) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v4,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14740"))) PPC_WEAK_FUNC(cputest_14740);
PPC_FUNC_IMPL(__imp__cputest_14740) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v4,5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14780"))) PPC_WEAK_FUNC(cputest_14780);
PPC_FUNC_IMPL(__imp__cputest_14780) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v3,v4,v4,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_147C0"))) PPC_WEAK_FUNC(cputest_147C0);
PPC_FUNC_IMPL(__imp__cputest_147C0) {
	PPC_FUNC_PROLOGUE();
	// vnor v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v6.u8), simde_mm_load_si128((simde__m128i*)ctx.v6.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14800"))) PPC_WEAK_FUNC(cputest_14800);
PPC_FUNC_IMPL(__imp__cputest_14800) {
	PPC_FUNC_PROLOGUE();
	// vandc v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v6.u8), simde_mm_load_si128((simde__m128i*)ctx.v6.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14840"))) PPC_WEAK_FUNC(cputest_14840);
PPC_FUNC_IMPL(__imp__cputest_14840) {
	PPC_FUNC_PROLOGUE();
	// vslh v6,v6,v6
	ctx.v6.u16[0] = ctx.v6.u16[0] << (ctx.v6.u16[0] & 0xF);
	ctx.v6.u16[1] = ctx.v6.u16[1] << (ctx.v6.u16[1] & 0xF);
	ctx.v6.u16[2] = ctx.v6.u16[2] << (ctx.v6.u16[2] & 0xF);
	ctx.v6.u16[3] = ctx.v6.u16[3] << (ctx.v6.u16[3] & 0xF);
	ctx.v6.u16[4] = ctx.v6.u16[4] << (ctx.v6.u16[4] & 0xF);
	ctx.v6.u16[5] = ctx.v6.u16[5] << (ctx.v6.u16[5] & 0xF);
	ctx.v6.u16[6] = ctx.v6.u16[6] << (ctx.v6.u16[6] & 0xF);
	ctx.v6.u16[7] = ctx.v6.u16[7] << (ctx.v6.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14880"))) PPC_WEAK_FUNC(cputest_14880);
PPC_FUNC_IMPL(__imp__cputest_14880) {
	PPC_FUNC_PROLOGUE();
	// vsrh v6,v6,v6
	ctx.v6.u16[0] = ctx.v6.u16[0] >> (ctx.v6.u16[0] & 0xF);
	ctx.v6.u16[1] = ctx.v6.u16[1] >> (ctx.v6.u16[1] & 0xF);
	ctx.v6.u16[2] = ctx.v6.u16[2] >> (ctx.v6.u16[2] & 0xF);
	ctx.v6.u16[3] = ctx.v6.u16[3] >> (ctx.v6.u16[3] & 0xF);
	ctx.v6.u16[4] = ctx.v6.u16[4] >> (ctx.v6.u16[4] & 0xF);
	ctx.v6.u16[5] = ctx.v6.u16[5] >> (ctx.v6.u16[5] & 0xF);
	ctx.v6.u16[6] = ctx.v6.u16[6] >> (ctx.v6.u16[6] & 0xF);
	ctx.v6.u16[7] = ctx.v6.u16[7] >> (ctx.v6.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_148C0"))) PPC_WEAK_FUNC(cputest_148C0);
PPC_FUNC_IMPL(__imp__cputest_148C0) {
	PPC_FUNC_PROLOGUE();
	// vsrah v6,v6,v6
	ctx.v6.s16[0] = ctx.v6.s16[0] >> (ctx.v6.u16[0] & 0xF);
	ctx.v6.s16[1] = ctx.v6.s16[1] >> (ctx.v6.u16[1] & 0xF);
	ctx.v6.s16[2] = ctx.v6.s16[2] >> (ctx.v6.u16[2] & 0xF);
	ctx.v6.s16[3] = ctx.v6.s16[3] >> (ctx.v6.u16[3] & 0xF);
	ctx.v6.s16[4] = ctx.v6.s16[4] >> (ctx.v6.u16[4] & 0xF);
	ctx.v6.s16[5] = ctx.v6.s16[5] >> (ctx.v6.u16[5] & 0xF);
	ctx.v6.s16[6] = ctx.v6.s16[6] >> (ctx.v6.u16[6] & 0xF);
	ctx.v6.s16[7] = ctx.v6.s16[7] >> (ctx.v6.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14900"))) PPC_WEAK_FUNC(cputest_14900);
PPC_FUNC_IMPL(__imp__cputest_14900) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro v6,v6,v6
	temp.u8 = (ctx.v6.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v6.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v6.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14940"))) PPC_WEAK_FUNC(cputest_14940);
PPC_FUNC_IMPL(__imp__cputest_14940) {
	PPC_FUNC_PROLOGUE();
	// vminsw v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v6.s32), simde_mm_load_si128((simde__m128i*)ctx.v6.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14980"))) PPC_WEAK_FUNC(cputest_14980);
PPC_FUNC_IMPL(__imp__cputest_14980) {
	PPC_FUNC_PROLOGUE();
	// vminuh v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u16, simde_mm_min_epu16(simde_mm_load_si128((simde__m128i*)ctx.v6.u16), simde_mm_load_si128((simde__m128i*)ctx.v6.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_149C0"))) PPC_WEAK_FUNC(cputest_149C0);
PPC_FUNC_IMPL(__imp__cputest_149C0) {
	PPC_FUNC_PROLOGUE();
	// vmaxuh v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u16, simde_mm_max_epu16(simde_mm_load_si128((simde__m128i*)ctx.v6.u16), simde_mm_load_si128((simde__m128i*)ctx.v6.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14A00"))) PPC_WEAK_FUNC(cputest_14A00);
PPC_FUNC_IMPL(__imp__cputest_14A00) {
	PPC_FUNC_PROLOGUE();
	// vavguh v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u16, simde_mm_avg_epu16(simde_mm_load_si128((simde__m128i*)ctx.v6.u16), simde_mm_load_si128((simde__m128i*)ctx.v6.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14A40"))) PPC_WEAK_FUNC(cputest_14A40);
PPC_FUNC_IMPL(__imp__cputest_14A40) {
	PPC_FUNC_PROLOGUE();
	// vsubuwm v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u32, simde_mm_sub_epi32(simde_mm_load_si128((simde__m128i*)ctx.v6.u32), simde_mm_load_si128((simde__m128i*)ctx.v6.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14A80"))) PPC_WEAK_FUNC(cputest_14A80);
PPC_FUNC_IMPL(__imp__cputest_14A80) {
	PPC_FUNC_PROLOGUE();
	// vsububm v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_sub_epi8(simde_mm_load_si128((simde__m128i*)ctx.v6.u8), simde_mm_load_si128((simde__m128i*)ctx.v6.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14AC0"))) PPC_WEAK_FUNC(cputest_14AC0);
PPC_FUNC_IMPL(__imp__cputest_14AC0) {
	PPC_FUNC_PROLOGUE();
	// vsubshs v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.s16, simde_mm_subs_epi16(simde_mm_load_si128((simde__m128i*)ctx.v6.s16), simde_mm_load_si128((simde__m128i*)ctx.v6.s16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14B00"))) PPC_WEAK_FUNC(cputest_14B00);
PPC_FUNC_IMPL(__imp__cputest_14B00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vaddsws v6,v6,v6
	temp.s64 = int64_t(ctx.v6.s32[0]) + int64_t(ctx.v6.s32[0]);
	ctx.v6.s32[0] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v6.s32[1]) + int64_t(ctx.v6.s32[1]);
	ctx.v6.s32[1] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v6.s32[2]) + int64_t(ctx.v6.s32[2]);
	ctx.v6.s32[2] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v6.s32[3]) + int64_t(ctx.v6.s32[3]);
	ctx.v6.s32[3] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14B40"))) PPC_WEAK_FUNC(cputest_14B40);
PPC_FUNC_IMPL(__imp__cputest_14B40) {
	PPC_FUNC_PROLOGUE();
	// vpkswss v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v6.s32), simde_mm_load_si128((simde__m128i*)ctx.v6.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14B80"))) PPC_WEAK_FUNC(cputest_14B80);
PPC_FUNC_IMPL(__imp__cputest_14B80) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v6.s8), simde_mm_load_si128((simde__m128i*)ctx.v6.s8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14BC0"))) PPC_WEAK_FUNC(cputest_14BC0);
PPC_FUNC_IMPL(__imp__cputest_14BC0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb. v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v6.s8), simde_mm_load_si128((simde__m128i*)ctx.v6.s8)));
	ctx.cr6.setFromMask(simde_mm_load_si128((simde__m128i*)ctx.v6.u8), 0xFFFF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14C00"))) PPC_WEAK_FUNC(cputest_14C00);
PPC_FUNC_IMPL(__imp__cputest_14C00) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v6.s32), simde_mm_load_si128((simde__m128i*)ctx.v6.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14C40"))) PPC_WEAK_FUNC(cputest_14C40);
PPC_FUNC_IMPL(__imp__cputest_14C40) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw. v6,v6,v6
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v6.s32), simde_mm_load_si128((simde__m128i*)ctx.v6.s32)));
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v6.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14C80"))) PPC_WEAK_FUNC(cputest_14C80);
PPC_FUNC_IMPL(__imp__cputest_14C80) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v6,v6,v6
	ctx.v6.u32[0] = ctx.v6.u32[0] > ctx.v6.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v6.u32[1] = ctx.v6.u32[1] > ctx.v6.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v6.u32[2] = ctx.v6.u32[2] > ctx.v6.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v6.u32[3] = ctx.v6.u32[3] > ctx.v6.u32[3] ? 0xFFFFFFFF : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14CC0"))) PPC_WEAK_FUNC(cputest_14CC0);
PPC_FUNC_IMPL(__imp__cputest_14CC0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw. v6,v6,v6
	ctx.v6.u32[0] = ctx.v6.u32[0] > ctx.v6.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v6.u32[1] = ctx.v6.u32[1] > ctx.v6.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v6.u32[2] = ctx.v6.u32[2] > ctx.v6.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v6.u32[3] = ctx.v6.u32[3] > ctx.v6.u32[3] ? 0xFFFFFFFF : 0;
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v6.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14D00"))) PPC_WEAK_FUNC(cputest_14D00);
PPC_FUNC_IMPL(__imp__cputest_14D00) {
	PPC_FUNC_PROLOGUE();
	// vspltish v6,0
	simde_mm_store_si128((simde__m128i*)ctx.v6.u16, simde_mm_set1_epi16(short(0x0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14D40"))) PPC_WEAK_FUNC(cputest_14D40);
PPC_FUNC_IMPL(__imp__cputest_14D40) {
	PPC_FUNC_PROLOGUE();
	// vspltish v6,1
	simde_mm_store_si128((simde__m128i*)ctx.v6.u16, simde_mm_set1_epi16(short(0x1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14D80"))) PPC_WEAK_FUNC(cputest_14D80);
PPC_FUNC_IMPL(__imp__cputest_14D80) {
	PPC_FUNC_PROLOGUE();
	// vspltish v6,15
	simde_mm_store_si128((simde__m128i*)ctx.v6.u16, simde_mm_set1_epi16(short(0xF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14DC0"))) PPC_WEAK_FUNC(cputest_14DC0);
PPC_FUNC_IMPL(__imp__cputest_14DC0) {
	PPC_FUNC_PROLOGUE();
	// vspltish v6,-16
	simde_mm_store_si128((simde__m128i*)ctx.v6.u16, simde_mm_set1_epi16(short(0xFFFFFFF0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14E00"))) PPC_WEAK_FUNC(cputest_14E00);
PPC_FUNC_IMPL(__imp__cputest_14E00) {
	PPC_FUNC_PROLOGUE();
	// vspltish v6,-1
	simde_mm_store_si128((simde__m128i*)ctx.v6.u16, simde_mm_set1_epi16(short(0xFFFFFFFF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14E40"))) PPC_WEAK_FUNC(cputest_14E40);
PPC_FUNC_IMPL(__imp__cputest_14E40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v6,v6,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v6.f32[0]) * 1.0;
	ctx.v6.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[1]) * 1.0;
	ctx.v6.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[2]) * 1.0;
	ctx.v6.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[3]) * 1.0;
	ctx.v6.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14E80"))) PPC_WEAK_FUNC(cputest_14E80);
PPC_FUNC_IMPL(__imp__cputest_14E80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v6,v6,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v6.f32[0]) * 2.0;
	ctx.v6.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[1]) * 2.0;
	ctx.v6.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[2]) * 2.0;
	ctx.v6.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[3]) * 2.0;
	ctx.v6.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14EC0"))) PPC_WEAK_FUNC(cputest_14EC0);
PPC_FUNC_IMPL(__imp__cputest_14EC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v6,v6,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v6.f32[0]) * 256.0;
	ctx.v6.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[1]) * 256.0;
	ctx.v6.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[2]) * 256.0;
	ctx.v6.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[3]) * 256.0;
	ctx.v6.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14F00"))) PPC_WEAK_FUNC(cputest_14F00);
PPC_FUNC_IMPL(__imp__cputest_14F00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v6,v6,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v6.f32[0]) * 2147483648.0;
	ctx.v6.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[1]) * 2147483648.0;
	ctx.v6.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[2]) * 2147483648.0;
	ctx.v6.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v6.f32[3]) * 2147483648.0;
	ctx.v6.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14F40"))) PPC_WEAK_FUNC(cputest_14F40);
PPC_FUNC_IMPL(__imp__cputest_14F40) {
	PPC_FUNC_PROLOGUE();
	// vsel v6,v6,v6,v7
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v7.u8), simde_mm_load_si128((simde__m128i*)ctx.v6.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v7.u8), simde_mm_load_si128((simde__m128i*)ctx.v6.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14F80"))) PPC_WEAK_FUNC(cputest_14F80);
PPC_FUNC_IMPL(__imp__cputest_14F80) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v6,v6,v6,0
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v6.u8), simde_mm_load_si128((simde__m128i*)ctx.v6.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_14FC0"))) PPC_WEAK_FUNC(cputest_14FC0);
PPC_FUNC_IMPL(__imp__cputest_14FC0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v6,v6,v6,1
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v6.u8), simde_mm_load_si128((simde__m128i*)ctx.v6.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15000"))) PPC_WEAK_FUNC(cputest_15000);
PPC_FUNC_IMPL(__imp__cputest_15000) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v6,v6,v6,5
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v6.u8), simde_mm_load_si128((simde__m128i*)ctx.v6.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15040"))) PPC_WEAK_FUNC(cputest_15040);
PPC_FUNC_IMPL(__imp__cputest_15040) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v6,v6,v6,15
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v6.u8), simde_mm_load_si128((simde__m128i*)ctx.v6.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15080"))) PPC_WEAK_FUNC(cputest_15080);
PPC_FUNC_IMPL(__imp__cputest_15080) {
	PPC_FUNC_PROLOGUE();
	// vnor v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v30.u8), simde_mm_load_si128((simde__m128i*)ctx.v29.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_150C0"))) PPC_WEAK_FUNC(cputest_150C0);
PPC_FUNC_IMPL(__imp__cputest_150C0) {
	PPC_FUNC_PROLOGUE();
	// vandc v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v29.u8), simde_mm_load_si128((simde__m128i*)ctx.v30.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15100"))) PPC_WEAK_FUNC(cputest_15100);
PPC_FUNC_IMPL(__imp__cputest_15100) {
	PPC_FUNC_PROLOGUE();
	// vslh v31,v30,v29
	ctx.v31.u16[0] = ctx.v30.u16[0] << (ctx.v29.u16[0] & 0xF);
	ctx.v31.u16[1] = ctx.v30.u16[1] << (ctx.v29.u16[1] & 0xF);
	ctx.v31.u16[2] = ctx.v30.u16[2] << (ctx.v29.u16[2] & 0xF);
	ctx.v31.u16[3] = ctx.v30.u16[3] << (ctx.v29.u16[3] & 0xF);
	ctx.v31.u16[4] = ctx.v30.u16[4] << (ctx.v29.u16[4] & 0xF);
	ctx.v31.u16[5] = ctx.v30.u16[5] << (ctx.v29.u16[5] & 0xF);
	ctx.v31.u16[6] = ctx.v30.u16[6] << (ctx.v29.u16[6] & 0xF);
	ctx.v31.u16[7] = ctx.v30.u16[7] << (ctx.v29.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15140"))) PPC_WEAK_FUNC(cputest_15140);
PPC_FUNC_IMPL(__imp__cputest_15140) {
	PPC_FUNC_PROLOGUE();
	// vsrh v31,v30,v29
	ctx.v31.u16[0] = ctx.v30.u16[0] >> (ctx.v29.u16[0] & 0xF);
	ctx.v31.u16[1] = ctx.v30.u16[1] >> (ctx.v29.u16[1] & 0xF);
	ctx.v31.u16[2] = ctx.v30.u16[2] >> (ctx.v29.u16[2] & 0xF);
	ctx.v31.u16[3] = ctx.v30.u16[3] >> (ctx.v29.u16[3] & 0xF);
	ctx.v31.u16[4] = ctx.v30.u16[4] >> (ctx.v29.u16[4] & 0xF);
	ctx.v31.u16[5] = ctx.v30.u16[5] >> (ctx.v29.u16[5] & 0xF);
	ctx.v31.u16[6] = ctx.v30.u16[6] >> (ctx.v29.u16[6] & 0xF);
	ctx.v31.u16[7] = ctx.v30.u16[7] >> (ctx.v29.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15180"))) PPC_WEAK_FUNC(cputest_15180);
PPC_FUNC_IMPL(__imp__cputest_15180) {
	PPC_FUNC_PROLOGUE();
	// vsrah v31,v30,v29
	ctx.v31.s16[0] = ctx.v30.s16[0] >> (ctx.v29.u16[0] & 0xF);
	ctx.v31.s16[1] = ctx.v30.s16[1] >> (ctx.v29.u16[1] & 0xF);
	ctx.v31.s16[2] = ctx.v30.s16[2] >> (ctx.v29.u16[2] & 0xF);
	ctx.v31.s16[3] = ctx.v30.s16[3] >> (ctx.v29.u16[3] & 0xF);
	ctx.v31.s16[4] = ctx.v30.s16[4] >> (ctx.v29.u16[4] & 0xF);
	ctx.v31.s16[5] = ctx.v30.s16[5] >> (ctx.v29.u16[5] & 0xF);
	ctx.v31.s16[6] = ctx.v30.s16[6] >> (ctx.v29.u16[6] & 0xF);
	ctx.v31.s16[7] = ctx.v30.s16[7] >> (ctx.v29.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_151C0"))) PPC_WEAK_FUNC(cputest_151C0);
PPC_FUNC_IMPL(__imp__cputest_151C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro v31,v30,v29
	temp.u8 = (ctx.v29.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v30.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v31.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15200"))) PPC_WEAK_FUNC(cputest_15200);
PPC_FUNC_IMPL(__imp__cputest_15200) {
	PPC_FUNC_PROLOGUE();
	// vminsw v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v30.s32), simde_mm_load_si128((simde__m128i*)ctx.v29.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15240"))) PPC_WEAK_FUNC(cputest_15240);
PPC_FUNC_IMPL(__imp__cputest_15240) {
	PPC_FUNC_PROLOGUE();
	// vminuh v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u16, simde_mm_min_epu16(simde_mm_load_si128((simde__m128i*)ctx.v30.u16), simde_mm_load_si128((simde__m128i*)ctx.v29.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15280"))) PPC_WEAK_FUNC(cputest_15280);
PPC_FUNC_IMPL(__imp__cputest_15280) {
	PPC_FUNC_PROLOGUE();
	// vmaxuh v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u16, simde_mm_max_epu16(simde_mm_load_si128((simde__m128i*)ctx.v30.u16), simde_mm_load_si128((simde__m128i*)ctx.v29.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_152C0"))) PPC_WEAK_FUNC(cputest_152C0);
PPC_FUNC_IMPL(__imp__cputest_152C0) {
	PPC_FUNC_PROLOGUE();
	// vavguh v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u16, simde_mm_avg_epu16(simde_mm_load_si128((simde__m128i*)ctx.v30.u16), simde_mm_load_si128((simde__m128i*)ctx.v29.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15300"))) PPC_WEAK_FUNC(cputest_15300);
PPC_FUNC_IMPL(__imp__cputest_15300) {
	PPC_FUNC_PROLOGUE();
	// vsubuwm v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u32, simde_mm_sub_epi32(simde_mm_load_si128((simde__m128i*)ctx.v30.u32), simde_mm_load_si128((simde__m128i*)ctx.v29.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15340"))) PPC_WEAK_FUNC(cputest_15340);
PPC_FUNC_IMPL(__imp__cputest_15340) {
	PPC_FUNC_PROLOGUE();
	// vsububm v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_sub_epi8(simde_mm_load_si128((simde__m128i*)ctx.v30.u8), simde_mm_load_si128((simde__m128i*)ctx.v29.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15380"))) PPC_WEAK_FUNC(cputest_15380);
PPC_FUNC_IMPL(__imp__cputest_15380) {
	PPC_FUNC_PROLOGUE();
	// vsubshs v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.s16, simde_mm_subs_epi16(simde_mm_load_si128((simde__m128i*)ctx.v30.s16), simde_mm_load_si128((simde__m128i*)ctx.v29.s16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_153C0"))) PPC_WEAK_FUNC(cputest_153C0);
PPC_FUNC_IMPL(__imp__cputest_153C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vaddsws v31,v30,v29
	temp.s64 = int64_t(ctx.v30.s32[0]) + int64_t(ctx.v29.s32[0]);
	ctx.v31.s32[0] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v30.s32[1]) + int64_t(ctx.v29.s32[1]);
	ctx.v31.s32[1] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v30.s32[2]) + int64_t(ctx.v29.s32[2]);
	ctx.v31.s32[2] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v30.s32[3]) + int64_t(ctx.v29.s32[3]);
	ctx.v31.s32[3] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15400"))) PPC_WEAK_FUNC(cputest_15400);
PPC_FUNC_IMPL(__imp__cputest_15400) {
	PPC_FUNC_PROLOGUE();
	// vpkswss v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v29.s32), simde_mm_load_si128((simde__m128i*)ctx.v30.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15440"))) PPC_WEAK_FUNC(cputest_15440);
PPC_FUNC_IMPL(__imp__cputest_15440) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v30.s8), simde_mm_load_si128((simde__m128i*)ctx.v29.s8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15480"))) PPC_WEAK_FUNC(cputest_15480);
PPC_FUNC_IMPL(__imp__cputest_15480) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb. v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v30.s8), simde_mm_load_si128((simde__m128i*)ctx.v29.s8)));
	ctx.cr6.setFromMask(simde_mm_load_si128((simde__m128i*)ctx.v31.u8), 0xFFFF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_154C0"))) PPC_WEAK_FUNC(cputest_154C0);
PPC_FUNC_IMPL(__imp__cputest_154C0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v30.s32), simde_mm_load_si128((simde__m128i*)ctx.v29.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15500"))) PPC_WEAK_FUNC(cputest_15500);
PPC_FUNC_IMPL(__imp__cputest_15500) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw. v31,v30,v29
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v30.s32), simde_mm_load_si128((simde__m128i*)ctx.v29.s32)));
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v31.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15540"))) PPC_WEAK_FUNC(cputest_15540);
PPC_FUNC_IMPL(__imp__cputest_15540) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v31,v30,v29
	ctx.v31.u32[0] = ctx.v30.u32[0] > ctx.v29.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v31.u32[1] = ctx.v30.u32[1] > ctx.v29.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v31.u32[2] = ctx.v30.u32[2] > ctx.v29.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v31.u32[3] = ctx.v30.u32[3] > ctx.v29.u32[3] ? 0xFFFFFFFF : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15580"))) PPC_WEAK_FUNC(cputest_15580);
PPC_FUNC_IMPL(__imp__cputest_15580) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw. v31,v30,v29
	ctx.v31.u32[0] = ctx.v30.u32[0] > ctx.v29.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v31.u32[1] = ctx.v30.u32[1] > ctx.v29.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v31.u32[2] = ctx.v30.u32[2] > ctx.v29.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v31.u32[3] = ctx.v30.u32[3] > ctx.v29.u32[3] ? 0xFFFFFFFF : 0;
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v31.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_155C0"))) PPC_WEAK_FUNC(cputest_155C0);
PPC_FUNC_IMPL(__imp__cputest_155C0) {
	PPC_FUNC_PROLOGUE();
	// vspltish v31,0
	simde_mm_store_si128((simde__m128i*)ctx.v31.u16, simde_mm_set1_epi16(short(0x0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15600"))) PPC_WEAK_FUNC(cputest_15600);
PPC_FUNC_IMPL(__imp__cputest_15600) {
	PPC_FUNC_PROLOGUE();
	// vspltish v31,1
	simde_mm_store_si128((simde__m128i*)ctx.v31.u16, simde_mm_set1_epi16(short(0x1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15640"))) PPC_WEAK_FUNC(cputest_15640);
PPC_FUNC_IMPL(__imp__cputest_15640) {
	PPC_FUNC_PROLOGUE();
	// vspltish v31,15
	simde_mm_store_si128((simde__m128i*)ctx.v31.u16, simde_mm_set1_epi16(short(0xF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15680"))) PPC_WEAK_FUNC(cputest_15680);
PPC_FUNC_IMPL(__imp__cputest_15680) {
	PPC_FUNC_PROLOGUE();
	// vspltish v31,-16
	simde_mm_store_si128((simde__m128i*)ctx.v31.u16, simde_mm_set1_epi16(short(0xFFFFFFF0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_156C0"))) PPC_WEAK_FUNC(cputest_156C0);
PPC_FUNC_IMPL(__imp__cputest_156C0) {
	PPC_FUNC_PROLOGUE();
	// vspltish v31,-1
	simde_mm_store_si128((simde__m128i*)ctx.v31.u16, simde_mm_set1_epi16(short(0xFFFFFFFF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15700"))) PPC_WEAK_FUNC(cputest_15700);
PPC_FUNC_IMPL(__imp__cputest_15700) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v31,v29,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v29.f32[0]) * 1.0;
	ctx.v31.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[1]) * 1.0;
	ctx.v31.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[2]) * 1.0;
	ctx.v31.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[3]) * 1.0;
	ctx.v31.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15740"))) PPC_WEAK_FUNC(cputest_15740);
PPC_FUNC_IMPL(__imp__cputest_15740) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v31,v29,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v29.f32[0]) * 2.0;
	ctx.v31.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[1]) * 2.0;
	ctx.v31.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[2]) * 2.0;
	ctx.v31.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[3]) * 2.0;
	ctx.v31.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15780"))) PPC_WEAK_FUNC(cputest_15780);
PPC_FUNC_IMPL(__imp__cputest_15780) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v31,v29,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v29.f32[0]) * 256.0;
	ctx.v31.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[1]) * 256.0;
	ctx.v31.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[2]) * 256.0;
	ctx.v31.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[3]) * 256.0;
	ctx.v31.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_157C0"))) PPC_WEAK_FUNC(cputest_157C0);
PPC_FUNC_IMPL(__imp__cputest_157C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v31,v29,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v29.f32[0]) * 2147483648.0;
	ctx.v31.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[1]) * 2147483648.0;
	ctx.v31.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[2]) * 2147483648.0;
	ctx.v31.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v29.f32[3]) * 2147483648.0;
	ctx.v31.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15800"))) PPC_WEAK_FUNC(cputest_15800);
PPC_FUNC_IMPL(__imp__cputest_15800) {
	PPC_FUNC_PROLOGUE();
	// vsel v31,v30,v29,v0
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v0.u8), simde_mm_load_si128((simde__m128i*)ctx.v30.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v0.u8), simde_mm_load_si128((simde__m128i*)ctx.v29.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15840"))) PPC_WEAK_FUNC(cputest_15840);
PPC_FUNC_IMPL(__imp__cputest_15840) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v31,v30,v29,0
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v30.u8), simde_mm_load_si128((simde__m128i*)ctx.v29.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15880"))) PPC_WEAK_FUNC(cputest_15880);
PPC_FUNC_IMPL(__imp__cputest_15880) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v31,v30,v29,1
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v30.u8), simde_mm_load_si128((simde__m128i*)ctx.v29.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_158C0"))) PPC_WEAK_FUNC(cputest_158C0);
PPC_FUNC_IMPL(__imp__cputest_158C0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v31,v30,v29,5
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v30.u8), simde_mm_load_si128((simde__m128i*)ctx.v29.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15900"))) PPC_WEAK_FUNC(cputest_15900);
PPC_FUNC_IMPL(__imp__cputest_15900) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v31,v30,v29,15
	simde_mm_store_si128((simde__m128i*)ctx.v31.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v30.u8), simde_mm_load_si128((simde__m128i*)ctx.v29.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15940"))) PPC_WEAK_FUNC(cputest_15940);
PPC_FUNC_IMPL(__imp__cputest_15940) {
	PPC_FUNC_PROLOGUE();
	// vnor v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v1.u8), simde_mm_load_si128((simde__m128i*)ctx.v2.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15980"))) PPC_WEAK_FUNC(cputest_15980);
PPC_FUNC_IMPL(__imp__cputest_15980) {
	PPC_FUNC_PROLOGUE();
	// vandc v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v2.u8), simde_mm_load_si128((simde__m128i*)ctx.v1.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_159C0"))) PPC_WEAK_FUNC(cputest_159C0);
PPC_FUNC_IMPL(__imp__cputest_159C0) {
	PPC_FUNC_PROLOGUE();
	// vslh v0,v1,v2
	ctx.v0.u16[0] = ctx.v1.u16[0] << (ctx.v2.u16[0] & 0xF);
	ctx.v0.u16[1] = ctx.v1.u16[1] << (ctx.v2.u16[1] & 0xF);
	ctx.v0.u16[2] = ctx.v1.u16[2] << (ctx.v2.u16[2] & 0xF);
	ctx.v0.u16[3] = ctx.v1.u16[3] << (ctx.v2.u16[3] & 0xF);
	ctx.v0.u16[4] = ctx.v1.u16[4] << (ctx.v2.u16[4] & 0xF);
	ctx.v0.u16[5] = ctx.v1.u16[5] << (ctx.v2.u16[5] & 0xF);
	ctx.v0.u16[6] = ctx.v1.u16[6] << (ctx.v2.u16[6] & 0xF);
	ctx.v0.u16[7] = ctx.v1.u16[7] << (ctx.v2.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15A00"))) PPC_WEAK_FUNC(cputest_15A00);
PPC_FUNC_IMPL(__imp__cputest_15A00) {
	PPC_FUNC_PROLOGUE();
	// vsrh v0,v1,v2
	ctx.v0.u16[0] = ctx.v1.u16[0] >> (ctx.v2.u16[0] & 0xF);
	ctx.v0.u16[1] = ctx.v1.u16[1] >> (ctx.v2.u16[1] & 0xF);
	ctx.v0.u16[2] = ctx.v1.u16[2] >> (ctx.v2.u16[2] & 0xF);
	ctx.v0.u16[3] = ctx.v1.u16[3] >> (ctx.v2.u16[3] & 0xF);
	ctx.v0.u16[4] = ctx.v1.u16[4] >> (ctx.v2.u16[4] & 0xF);
	ctx.v0.u16[5] = ctx.v1.u16[5] >> (ctx.v2.u16[5] & 0xF);
	ctx.v0.u16[6] = ctx.v1.u16[6] >> (ctx.v2.u16[6] & 0xF);
	ctx.v0.u16[7] = ctx.v1.u16[7] >> (ctx.v2.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15A40"))) PPC_WEAK_FUNC(cputest_15A40);
PPC_FUNC_IMPL(__imp__cputest_15A40) {
	PPC_FUNC_PROLOGUE();
	// vsrah v0,v1,v2
	ctx.v0.s16[0] = ctx.v1.s16[0] >> (ctx.v2.u16[0] & 0xF);
	ctx.v0.s16[1] = ctx.v1.s16[1] >> (ctx.v2.u16[1] & 0xF);
	ctx.v0.s16[2] = ctx.v1.s16[2] >> (ctx.v2.u16[2] & 0xF);
	ctx.v0.s16[3] = ctx.v1.s16[3] >> (ctx.v2.u16[3] & 0xF);
	ctx.v0.s16[4] = ctx.v1.s16[4] >> (ctx.v2.u16[4] & 0xF);
	ctx.v0.s16[5] = ctx.v1.s16[5] >> (ctx.v2.u16[5] & 0xF);
	ctx.v0.s16[6] = ctx.v1.s16[6] >> (ctx.v2.u16[6] & 0xF);
	ctx.v0.s16[7] = ctx.v1.s16[7] >> (ctx.v2.u16[7] & 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15A80"))) PPC_WEAK_FUNC(cputest_15A80);
PPC_FUNC_IMPL(__imp__cputest_15A80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro v0,v1,v2
	temp.u8 = (ctx.v2.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v1.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v0.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15AC0"))) PPC_WEAK_FUNC(cputest_15AC0);
PPC_FUNC_IMPL(__imp__cputest_15AC0) {
	PPC_FUNC_PROLOGUE();
	// vminsw v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u32, simde_mm_min_epi32(simde_mm_load_si128((simde__m128i*)ctx.v1.s32), simde_mm_load_si128((simde__m128i*)ctx.v2.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15B00"))) PPC_WEAK_FUNC(cputest_15B00);
PPC_FUNC_IMPL(__imp__cputest_15B00) {
	PPC_FUNC_PROLOGUE();
	// vminuh v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u16, simde_mm_min_epu16(simde_mm_load_si128((simde__m128i*)ctx.v1.u16), simde_mm_load_si128((simde__m128i*)ctx.v2.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15B40"))) PPC_WEAK_FUNC(cputest_15B40);
PPC_FUNC_IMPL(__imp__cputest_15B40) {
	PPC_FUNC_PROLOGUE();
	// vmaxuh v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u16, simde_mm_max_epu16(simde_mm_load_si128((simde__m128i*)ctx.v1.u16), simde_mm_load_si128((simde__m128i*)ctx.v2.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15B80"))) PPC_WEAK_FUNC(cputest_15B80);
PPC_FUNC_IMPL(__imp__cputest_15B80) {
	PPC_FUNC_PROLOGUE();
	// vavguh v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u16, simde_mm_avg_epu16(simde_mm_load_si128((simde__m128i*)ctx.v1.u16), simde_mm_load_si128((simde__m128i*)ctx.v2.u16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15BC0"))) PPC_WEAK_FUNC(cputest_15BC0);
PPC_FUNC_IMPL(__imp__cputest_15BC0) {
	PPC_FUNC_PROLOGUE();
	// vsubuwm v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u32, simde_mm_sub_epi32(simde_mm_load_si128((simde__m128i*)ctx.v1.u32), simde_mm_load_si128((simde__m128i*)ctx.v2.u32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15C00"))) PPC_WEAK_FUNC(cputest_15C00);
PPC_FUNC_IMPL(__imp__cputest_15C00) {
	PPC_FUNC_PROLOGUE();
	// vsububm v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_sub_epi8(simde_mm_load_si128((simde__m128i*)ctx.v1.u8), simde_mm_load_si128((simde__m128i*)ctx.v2.u8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15C40"))) PPC_WEAK_FUNC(cputest_15C40);
PPC_FUNC_IMPL(__imp__cputest_15C40) {
	PPC_FUNC_PROLOGUE();
	// vsubshs v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.s16, simde_mm_subs_epi16(simde_mm_load_si128((simde__m128i*)ctx.v1.s16), simde_mm_load_si128((simde__m128i*)ctx.v2.s16)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15C80"))) PPC_WEAK_FUNC(cputest_15C80);
PPC_FUNC_IMPL(__imp__cputest_15C80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vaddsws v0,v1,v2
	temp.s64 = int64_t(ctx.v1.s32[0]) + int64_t(ctx.v2.s32[0]);
	ctx.v0.s32[0] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v1.s32[1]) + int64_t(ctx.v2.s32[1]);
	ctx.v0.s32[1] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v1.s32[2]) + int64_t(ctx.v2.s32[2]);
	ctx.v0.s32[2] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	temp.s64 = int64_t(ctx.v1.s32[3]) + int64_t(ctx.v2.s32[3]);
	ctx.v0.s32[3] = temp.s64 > INT_MAX ? INT_MAX : temp.s64 < INT_MIN ? INT_MIN : temp.s64;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15CC0"))) PPC_WEAK_FUNC(cputest_15CC0);
PPC_FUNC_IMPL(__imp__cputest_15CC0) {
	PPC_FUNC_PROLOGUE();
	// vpkswss v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v2.s32), simde_mm_load_si128((simde__m128i*)ctx.v1.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15D00"))) PPC_WEAK_FUNC(cputest_15D00);
PPC_FUNC_IMPL(__imp__cputest_15D00) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v1.s8), simde_mm_load_si128((simde__m128i*)ctx.v2.s8)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15D40"))) PPC_WEAK_FUNC(cputest_15D40);
PPC_FUNC_IMPL(__imp__cputest_15D40) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsb. v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_cmpgt_epi8(simde_mm_load_si128((simde__m128i*)ctx.v1.s8), simde_mm_load_si128((simde__m128i*)ctx.v2.s8)));
	ctx.cr6.setFromMask(simde_mm_load_si128((simde__m128i*)ctx.v0.u8), 0xFFFF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15D80"))) PPC_WEAK_FUNC(cputest_15D80);
PPC_FUNC_IMPL(__imp__cputest_15D80) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v1.s32), simde_mm_load_si128((simde__m128i*)ctx.v2.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15DC0"))) PPC_WEAK_FUNC(cputest_15DC0);
PPC_FUNC_IMPL(__imp__cputest_15DC0) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtsw. v0,v1,v2
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_cmpgt_epi32(simde_mm_load_si128((simde__m128i*)ctx.v1.s32), simde_mm_load_si128((simde__m128i*)ctx.v2.s32)));
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v0.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15E00"))) PPC_WEAK_FUNC(cputest_15E00);
PPC_FUNC_IMPL(__imp__cputest_15E00) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw v0,v1,v2
	ctx.v0.u32[0] = ctx.v1.u32[0] > ctx.v2.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v0.u32[1] = ctx.v1.u32[1] > ctx.v2.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v0.u32[2] = ctx.v1.u32[2] > ctx.v2.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v0.u32[3] = ctx.v1.u32[3] > ctx.v2.u32[3] ? 0xFFFFFFFF : 0;
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15E40"))) PPC_WEAK_FUNC(cputest_15E40);
PPC_FUNC_IMPL(__imp__cputest_15E40) {
	PPC_FUNC_PROLOGUE();
	// vcmpgtuw. v0,v1,v2
	ctx.v0.u32[0] = ctx.v1.u32[0] > ctx.v2.u32[0] ? 0xFFFFFFFF : 0;
	ctx.v0.u32[1] = ctx.v1.u32[1] > ctx.v2.u32[1] ? 0xFFFFFFFF : 0;
	ctx.v0.u32[2] = ctx.v1.u32[2] > ctx.v2.u32[2] ? 0xFFFFFFFF : 0;
	ctx.v0.u32[3] = ctx.v1.u32[3] > ctx.v2.u32[3] ? 0xFFFFFFFF : 0;
	ctx.cr6.setFromMask(simde_mm_load_ps(ctx.v0.f32), 0xF);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15E80"))) PPC_WEAK_FUNC(cputest_15E80);
PPC_FUNC_IMPL(__imp__cputest_15E80) {
	PPC_FUNC_PROLOGUE();
	// vspltish v0,0
	simde_mm_store_si128((simde__m128i*)ctx.v0.u16, simde_mm_set1_epi16(short(0x0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15EC0"))) PPC_WEAK_FUNC(cputest_15EC0);
PPC_FUNC_IMPL(__imp__cputest_15EC0) {
	PPC_FUNC_PROLOGUE();
	// vspltish v0,1
	simde_mm_store_si128((simde__m128i*)ctx.v0.u16, simde_mm_set1_epi16(short(0x1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15F00"))) PPC_WEAK_FUNC(cputest_15F00);
PPC_FUNC_IMPL(__imp__cputest_15F00) {
	PPC_FUNC_PROLOGUE();
	// vspltish v0,15
	simde_mm_store_si128((simde__m128i*)ctx.v0.u16, simde_mm_set1_epi16(short(0xF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15F40"))) PPC_WEAK_FUNC(cputest_15F40);
PPC_FUNC_IMPL(__imp__cputest_15F40) {
	PPC_FUNC_PROLOGUE();
	// vspltish v0,-16
	simde_mm_store_si128((simde__m128i*)ctx.v0.u16, simde_mm_set1_epi16(short(0xFFFFFFF0)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15F80"))) PPC_WEAK_FUNC(cputest_15F80);
PPC_FUNC_IMPL(__imp__cputest_15F80) {
	PPC_FUNC_PROLOGUE();
	// vspltish v0,-1
	simde_mm_store_si128((simde__m128i*)ctx.v0.u16, simde_mm_set1_epi16(short(0xFFFFFFFF)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_15FC0"))) PPC_WEAK_FUNC(cputest_15FC0);
PPC_FUNC_IMPL(__imp__cputest_15FC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v0,v2,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v2.f32[0]) * 1.0;
	ctx.v0.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[1]) * 1.0;
	ctx.v0.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[2]) * 1.0;
	ctx.v0.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[3]) * 1.0;
	ctx.v0.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16000"))) PPC_WEAK_FUNC(cputest_16000);
PPC_FUNC_IMPL(__imp__cputest_16000) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v0,v2,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v2.f32[0]) * 2.0;
	ctx.v0.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[1]) * 2.0;
	ctx.v0.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[2]) * 2.0;
	ctx.v0.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[3]) * 2.0;
	ctx.v0.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16040"))) PPC_WEAK_FUNC(cputest_16040);
PPC_FUNC_IMPL(__imp__cputest_16040) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v0,v2,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v2.f32[0]) * 256.0;
	ctx.v0.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[1]) * 256.0;
	ctx.v0.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[2]) * 256.0;
	ctx.v0.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[3]) * 256.0;
	ctx.v0.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16080"))) PPC_WEAK_FUNC(cputest_16080);
PPC_FUNC_IMPL(__imp__cputest_16080) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vctuxs v0,v2,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v2.f32[0]) * 2147483648.0;
	ctx.v0.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[1]) * 2147483648.0;
	ctx.v0.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[2]) * 2147483648.0;
	ctx.v0.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v2.f32[3]) * 2147483648.0;
	ctx.v0.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_160C0"))) PPC_WEAK_FUNC(cputest_160C0);
PPC_FUNC_IMPL(__imp__cputest_160C0) {
	PPC_FUNC_PROLOGUE();
	// vsel v0,v1,v2,v1
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v1.u8), simde_mm_load_si128((simde__m128i*)ctx.v1.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v1.u8), simde_mm_load_si128((simde__m128i*)ctx.v2.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16100"))) PPC_WEAK_FUNC(cputest_16100);
PPC_FUNC_IMPL(__imp__cputest_16100) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v0,v1,v2,0
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v1.u8), simde_mm_load_si128((simde__m128i*)ctx.v2.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16140"))) PPC_WEAK_FUNC(cputest_16140);
PPC_FUNC_IMPL(__imp__cputest_16140) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v0,v1,v2,1
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v1.u8), simde_mm_load_si128((simde__m128i*)ctx.v2.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16180"))) PPC_WEAK_FUNC(cputest_16180);
PPC_FUNC_IMPL(__imp__cputest_16180) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v0,v1,v2,5
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v1.u8), simde_mm_load_si128((simde__m128i*)ctx.v2.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_161C0"))) PPC_WEAK_FUNC(cputest_161C0);
PPC_FUNC_IMPL(__imp__cputest_161C0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi v0,v1,v2,15
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v1.u8), simde_mm_load_si128((simde__m128i*)ctx.v2.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16200"))) PPC_WEAK_FUNC(cputest_16200);
PPC_FUNC_IMPL(__imp__cputest_16200) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16240"))) PPC_WEAK_FUNC(cputest_16240);
PPC_FUNC_IMPL(__imp__cputest_16240) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro128 v3,v4,v5
	temp.u8 = (ctx.v5.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v4.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v3.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16280"))) PPC_WEAK_FUNC(cputest_16280);
PPC_FUNC_IMPL(__imp__cputest_16280) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v3,v4,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v5.s32), simde_mm_load_si128((simde__m128i*)ctx.v4.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_162C0"))) PPC_WEAK_FUNC(cputest_162C0);
PPC_FUNC_IMPL(__imp__cputest_162C0) {
	PPC_FUNC_PROLOGUE();
	// vsel128 v3,v4,v5,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16300"))) PPC_WEAK_FUNC(cputest_16300);
PPC_FUNC_IMPL(__imp__cputest_16300) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v5,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 1.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 1.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 1.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 1.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16340"))) PPC_WEAK_FUNC(cputest_16340);
PPC_FUNC_IMPL(__imp__cputest_16340) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v5,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 2.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 2.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 2.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 2.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16380"))) PPC_WEAK_FUNC(cputest_16380);
PPC_FUNC_IMPL(__imp__cputest_16380) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v5,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 256.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 256.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 256.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 256.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_163C0"))) PPC_WEAK_FUNC(cputest_163C0);
PPC_FUNC_IMPL(__imp__cputest_163C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v5,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 2147483648.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 2147483648.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 2147483648.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 2147483648.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16400"))) PPC_WEAK_FUNC(cputest_16400);
PPC_FUNC_IMPL(__imp__cputest_16400) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v5,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16440"))) PPC_WEAK_FUNC(cputest_16440);
PPC_FUNC_IMPL(__imp__cputest_16440) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v5,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16480"))) PPC_WEAK_FUNC(cputest_16480);
PPC_FUNC_IMPL(__imp__cputest_16480) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v5,5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_164C0"))) PPC_WEAK_FUNC(cputest_164C0);
PPC_FUNC_IMPL(__imp__cputest_164C0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v5,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16500"))) PPC_WEAK_FUNC(cputest_16500);
PPC_FUNC_IMPL(__imp__cputest_16500) {
	PPC_FUNC_PROLOGUE();
	// vupkd3d128 v3,v5,20
	{
		uint32_t h0 = ppc_f16_to_f32bits(ctx.v5.u16[0]);
		uint32_t h1 = ppc_f16_to_f32bits(ctx.v5.u16[1]);
		uint32_t h2 = ppc_f16_to_f32bits(ctx.v5.u16[2]);
		uint32_t h3 = ppc_f16_to_f32bits(ctx.v5.u16[3]);
		ctx.v3.u32[0] = h0;
		ctx.v3.u32[1] = h1;
		ctx.v3.u32[2] = h2;
		ctx.v3.u32[3] = h3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16540"))) PPC_WEAK_FUNC(cputest_16540);
PPC_FUNC_IMPL(__imp__cputest_16540) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16580"))) PPC_WEAK_FUNC(cputest_16580);
PPC_FUNC_IMPL(__imp__cputest_16580) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_165C0"))) PPC_WEAK_FUNC(cputest_165C0);
PPC_FUNC_IMPL(__imp__cputest_165C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16600"))) PPC_WEAK_FUNC(cputest_16600);
PPC_FUNC_IMPL(__imp__cputest_16600) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16640"))) PPC_WEAK_FUNC(cputest_16640);
PPC_FUNC_IMPL(__imp__cputest_16640) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16680"))) PPC_WEAK_FUNC(cputest_16680);
PPC_FUNC_IMPL(__imp__cputest_16680) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_166C0"))) PPC_WEAK_FUNC(cputest_166C0);
PPC_FUNC_IMPL(__imp__cputest_166C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16700"))) PPC_WEAK_FUNC(cputest_16700);
PPC_FUNC_IMPL(__imp__cputest_16700) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16740"))) PPC_WEAK_FUNC(cputest_16740);
PPC_FUNC_IMPL(__imp__cputest_16740) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16780"))) PPC_WEAK_FUNC(cputest_16780);
PPC_FUNC_IMPL(__imp__cputest_16780) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_167C0"))) PPC_WEAK_FUNC(cputest_167C0);
PPC_FUNC_IMPL(__imp__cputest_167C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16800"))) PPC_WEAK_FUNC(cputest_16800);
PPC_FUNC_IMPL(__imp__cputest_16800) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16840"))) PPC_WEAK_FUNC(cputest_16840);
PPC_FUNC_IMPL(__imp__cputest_16840) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16880"))) PPC_WEAK_FUNC(cputest_16880);
PPC_FUNC_IMPL(__imp__cputest_16880) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_168C0"))) PPC_WEAK_FUNC(cputest_168C0);
PPC_FUNC_IMPL(__imp__cputest_168C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16900"))) PPC_WEAK_FUNC(cputest_16900);
PPC_FUNC_IMPL(__imp__cputest_16900) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16940"))) PPC_WEAK_FUNC(cputest_16940);
PPC_FUNC_IMPL(__imp__cputest_16940) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16980"))) PPC_WEAK_FUNC(cputest_16980);
PPC_FUNC_IMPL(__imp__cputest_16980) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_169C0"))) PPC_WEAK_FUNC(cputest_169C0);
PPC_FUNC_IMPL(__imp__cputest_169C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16A00"))) PPC_WEAK_FUNC(cputest_16A00);
PPC_FUNC_IMPL(__imp__cputest_16A00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16A40"))) PPC_WEAK_FUNC(cputest_16A40);
PPC_FUNC_IMPL(__imp__cputest_16A40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16A80"))) PPC_WEAK_FUNC(cputest_16A80);
PPC_FUNC_IMPL(__imp__cputest_16A80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16AC0"))) PPC_WEAK_FUNC(cputest_16AC0);
PPC_FUNC_IMPL(__imp__cputest_16AC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16B00"))) PPC_WEAK_FUNC(cputest_16B00);
PPC_FUNC_IMPL(__imp__cputest_16B00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16B40"))) PPC_WEAK_FUNC(cputest_16B40);
PPC_FUNC_IMPL(__imp__cputest_16B40) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16B80"))) PPC_WEAK_FUNC(cputest_16B80);
PPC_FUNC_IMPL(__imp__cputest_16B80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro128 v3,v3,v5
	temp.u8 = (ctx.v5.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v3.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v3.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16BC0"))) PPC_WEAK_FUNC(cputest_16BC0);
PPC_FUNC_IMPL(__imp__cputest_16BC0) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v3,v3,v5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v5.s32), simde_mm_load_si128((simde__m128i*)ctx.v3.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16C00"))) PPC_WEAK_FUNC(cputest_16C00);
PPC_FUNC_IMPL(__imp__cputest_16C00) {
	PPC_FUNC_PROLOGUE();
	// vsel128 v3,v3,v5,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16C40"))) PPC_WEAK_FUNC(cputest_16C40);
PPC_FUNC_IMPL(__imp__cputest_16C40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v5,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 1.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 1.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 1.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 1.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16C80"))) PPC_WEAK_FUNC(cputest_16C80);
PPC_FUNC_IMPL(__imp__cputest_16C80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v5,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 2.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 2.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 2.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 2.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16CC0"))) PPC_WEAK_FUNC(cputest_16CC0);
PPC_FUNC_IMPL(__imp__cputest_16CC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v5,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 256.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 256.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 256.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 256.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16D00"))) PPC_WEAK_FUNC(cputest_16D00);
PPC_FUNC_IMPL(__imp__cputest_16D00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v5,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v5.f32[0]) * 2147483648.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[1]) * 2147483648.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[2]) * 2147483648.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v5.f32[3]) * 2147483648.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16D40"))) PPC_WEAK_FUNC(cputest_16D40);
PPC_FUNC_IMPL(__imp__cputest_16D40) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v3,v5,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16D80"))) PPC_WEAK_FUNC(cputest_16D80);
PPC_FUNC_IMPL(__imp__cputest_16D80) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v3,v5,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16DC0"))) PPC_WEAK_FUNC(cputest_16DC0);
PPC_FUNC_IMPL(__imp__cputest_16DC0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v3,v5,5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16E00"))) PPC_WEAK_FUNC(cputest_16E00);
PPC_FUNC_IMPL(__imp__cputest_16E00) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v3,v5,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v5.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16E40"))) PPC_WEAK_FUNC(cputest_16E40);
PPC_FUNC_IMPL(__imp__cputest_16E40) {
	PPC_FUNC_PROLOGUE();
	// vupkd3d128 v3,v5,20
	{
		uint32_t h0 = ppc_f16_to_f32bits(ctx.v5.u16[0]);
		uint32_t h1 = ppc_f16_to_f32bits(ctx.v5.u16[1]);
		uint32_t h2 = ppc_f16_to_f32bits(ctx.v5.u16[2]);
		uint32_t h3 = ppc_f16_to_f32bits(ctx.v5.u16[3]);
		ctx.v3.u32[0] = h0;
		ctx.v3.u32[1] = h1;
		ctx.v3.u32[2] = h2;
		ctx.v3.u32[3] = h3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16E80"))) PPC_WEAK_FUNC(cputest_16E80);
PPC_FUNC_IMPL(__imp__cputest_16E80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16EC0"))) PPC_WEAK_FUNC(cputest_16EC0);
PPC_FUNC_IMPL(__imp__cputest_16EC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16F00"))) PPC_WEAK_FUNC(cputest_16F00);
PPC_FUNC_IMPL(__imp__cputest_16F00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16F40"))) PPC_WEAK_FUNC(cputest_16F40);
PPC_FUNC_IMPL(__imp__cputest_16F40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16F80"))) PPC_WEAK_FUNC(cputest_16F80);
PPC_FUNC_IMPL(__imp__cputest_16F80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_16FC0"))) PPC_WEAK_FUNC(cputest_16FC0);
PPC_FUNC_IMPL(__imp__cputest_16FC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17000"))) PPC_WEAK_FUNC(cputest_17000);
PPC_FUNC_IMPL(__imp__cputest_17000) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17040"))) PPC_WEAK_FUNC(cputest_17040);
PPC_FUNC_IMPL(__imp__cputest_17040) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17080"))) PPC_WEAK_FUNC(cputest_17080);
PPC_FUNC_IMPL(__imp__cputest_17080) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_170C0"))) PPC_WEAK_FUNC(cputest_170C0);
PPC_FUNC_IMPL(__imp__cputest_170C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17100"))) PPC_WEAK_FUNC(cputest_17100);
PPC_FUNC_IMPL(__imp__cputest_17100) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17140"))) PPC_WEAK_FUNC(cputest_17140);
PPC_FUNC_IMPL(__imp__cputest_17140) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,3,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		ctx.v3.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17180"))) PPC_WEAK_FUNC(cputest_17180);
PPC_FUNC_IMPL(__imp__cputest_17180) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_171C0"))) PPC_WEAK_FUNC(cputest_171C0);
PPC_FUNC_IMPL(__imp__cputest_171C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17200"))) PPC_WEAK_FUNC(cputest_17200);
PPC_FUNC_IMPL(__imp__cputest_17200) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17240"))) PPC_WEAK_FUNC(cputest_17240);
PPC_FUNC_IMPL(__imp__cputest_17240) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17280"))) PPC_WEAK_FUNC(cputest_17280);
PPC_FUNC_IMPL(__imp__cputest_17280) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_172C0"))) PPC_WEAK_FUNC(cputest_172C0);
PPC_FUNC_IMPL(__imp__cputest_172C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17300"))) PPC_WEAK_FUNC(cputest_17300);
PPC_FUNC_IMPL(__imp__cputest_17300) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17340"))) PPC_WEAK_FUNC(cputest_17340);
PPC_FUNC_IMPL(__imp__cputest_17340) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17380"))) PPC_WEAK_FUNC(cputest_17380);
PPC_FUNC_IMPL(__imp__cputest_17380) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_173C0"))) PPC_WEAK_FUNC(cputest_173C0);
PPC_FUNC_IMPL(__imp__cputest_173C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17400"))) PPC_WEAK_FUNC(cputest_17400);
PPC_FUNC_IMPL(__imp__cputest_17400) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17440"))) PPC_WEAK_FUNC(cputest_17440);
PPC_FUNC_IMPL(__imp__cputest_17440) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v5,5,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v5.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v5.u32[0]);
		ctx.v3.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17480"))) PPC_WEAK_FUNC(cputest_17480);
PPC_FUNC_IMPL(__imp__cputest_17480) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_174C0"))) PPC_WEAK_FUNC(cputest_174C0);
PPC_FUNC_IMPL(__imp__cputest_174C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro128 v3,v4,v3
	temp.u8 = (ctx.v3.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v4.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v3.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17500"))) PPC_WEAK_FUNC(cputest_17500);
PPC_FUNC_IMPL(__imp__cputest_17500) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v3,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v3.s32), simde_mm_load_si128((simde__m128i*)ctx.v4.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17540"))) PPC_WEAK_FUNC(cputest_17540);
PPC_FUNC_IMPL(__imp__cputest_17540) {
	PPC_FUNC_PROLOGUE();
	// vsel128 v3,v4,v3,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17580"))) PPC_WEAK_FUNC(cputest_17580);
PPC_FUNC_IMPL(__imp__cputest_17580) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v3,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v3.f32[0]) * 1.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[1]) * 1.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[2]) * 1.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[3]) * 1.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_175C0"))) PPC_WEAK_FUNC(cputest_175C0);
PPC_FUNC_IMPL(__imp__cputest_175C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v3,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v3.f32[0]) * 2.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[1]) * 2.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[2]) * 2.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[3]) * 2.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17600"))) PPC_WEAK_FUNC(cputest_17600);
PPC_FUNC_IMPL(__imp__cputest_17600) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v3,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v3.f32[0]) * 256.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[1]) * 256.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[2]) * 256.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[3]) * 256.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17640"))) PPC_WEAK_FUNC(cputest_17640);
PPC_FUNC_IMPL(__imp__cputest_17640) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v3,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v3.f32[0]) * 2147483648.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[1]) * 2147483648.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[2]) * 2147483648.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v3.f32[3]) * 2147483648.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17680"))) PPC_WEAK_FUNC(cputest_17680);
PPC_FUNC_IMPL(__imp__cputest_17680) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v3,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_176C0"))) PPC_WEAK_FUNC(cputest_176C0);
PPC_FUNC_IMPL(__imp__cputest_176C0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v3,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17700"))) PPC_WEAK_FUNC(cputest_17700);
PPC_FUNC_IMPL(__imp__cputest_17700) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v3,5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17740"))) PPC_WEAK_FUNC(cputest_17740);
PPC_FUNC_IMPL(__imp__cputest_17740) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v3,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v3.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17780"))) PPC_WEAK_FUNC(cputest_17780);
PPC_FUNC_IMPL(__imp__cputest_17780) {
	PPC_FUNC_PROLOGUE();
	// vupkd3d128 v3,v3,20
	{
		uint32_t h0 = ppc_f16_to_f32bits(ctx.v3.u16[0]);
		uint32_t h1 = ppc_f16_to_f32bits(ctx.v3.u16[1]);
		uint32_t h2 = ppc_f16_to_f32bits(ctx.v3.u16[2]);
		uint32_t h3 = ppc_f16_to_f32bits(ctx.v3.u16[3]);
		ctx.v3.u32[0] = h0;
		ctx.v3.u32[1] = h1;
		ctx.v3.u32[2] = h2;
		ctx.v3.u32[3] = h3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_177C0"))) PPC_WEAK_FUNC(cputest_177C0);
PPC_FUNC_IMPL(__imp__cputest_177C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17800"))) PPC_WEAK_FUNC(cputest_17800);
PPC_FUNC_IMPL(__imp__cputest_17800) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17840"))) PPC_WEAK_FUNC(cputest_17840);
PPC_FUNC_IMPL(__imp__cputest_17840) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17880"))) PPC_WEAK_FUNC(cputest_17880);
PPC_FUNC_IMPL(__imp__cputest_17880) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_178C0"))) PPC_WEAK_FUNC(cputest_178C0);
PPC_FUNC_IMPL(__imp__cputest_178C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17900"))) PPC_WEAK_FUNC(cputest_17900);
PPC_FUNC_IMPL(__imp__cputest_17900) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17940"))) PPC_WEAK_FUNC(cputest_17940);
PPC_FUNC_IMPL(__imp__cputest_17940) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17980"))) PPC_WEAK_FUNC(cputest_17980);
PPC_FUNC_IMPL(__imp__cputest_17980) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_179C0"))) PPC_WEAK_FUNC(cputest_179C0);
PPC_FUNC_IMPL(__imp__cputest_179C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17A00"))) PPC_WEAK_FUNC(cputest_17A00);
PPC_FUNC_IMPL(__imp__cputest_17A00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17A40"))) PPC_WEAK_FUNC(cputest_17A40);
PPC_FUNC_IMPL(__imp__cputest_17A40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17A80"))) PPC_WEAK_FUNC(cputest_17A80);
PPC_FUNC_IMPL(__imp__cputest_17A80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,3,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		ctx.v3.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17AC0"))) PPC_WEAK_FUNC(cputest_17AC0);
PPC_FUNC_IMPL(__imp__cputest_17AC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17B00"))) PPC_WEAK_FUNC(cputest_17B00);
PPC_FUNC_IMPL(__imp__cputest_17B00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17B40"))) PPC_WEAK_FUNC(cputest_17B40);
PPC_FUNC_IMPL(__imp__cputest_17B40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17B80"))) PPC_WEAK_FUNC(cputest_17B80);
PPC_FUNC_IMPL(__imp__cputest_17B80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17BC0"))) PPC_WEAK_FUNC(cputest_17BC0);
PPC_FUNC_IMPL(__imp__cputest_17BC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17C00"))) PPC_WEAK_FUNC(cputest_17C00);
PPC_FUNC_IMPL(__imp__cputest_17C00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17C40"))) PPC_WEAK_FUNC(cputest_17C40);
PPC_FUNC_IMPL(__imp__cputest_17C40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17C80"))) PPC_WEAK_FUNC(cputest_17C80);
PPC_FUNC_IMPL(__imp__cputest_17C80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17CC0"))) PPC_WEAK_FUNC(cputest_17CC0);
PPC_FUNC_IMPL(__imp__cputest_17CC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17D00"))) PPC_WEAK_FUNC(cputest_17D00);
PPC_FUNC_IMPL(__imp__cputest_17D00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17D40"))) PPC_WEAK_FUNC(cputest_17D40);
PPC_FUNC_IMPL(__imp__cputest_17D40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17D80"))) PPC_WEAK_FUNC(cputest_17D80);
PPC_FUNC_IMPL(__imp__cputest_17D80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v3,5,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v3.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v3.u32[0]);
		ctx.v3.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17DC0"))) PPC_WEAK_FUNC(cputest_17DC0);
PPC_FUNC_IMPL(__imp__cputest_17DC0) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17E00"))) PPC_WEAK_FUNC(cputest_17E00);
PPC_FUNC_IMPL(__imp__cputest_17E00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro128 v3,v4,v4
	temp.u8 = (ctx.v4.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v4.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v3.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17E40"))) PPC_WEAK_FUNC(cputest_17E40);
PPC_FUNC_IMPL(__imp__cputest_17E40) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v3,v4,v4
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v4.s32), simde_mm_load_si128((simde__m128i*)ctx.v4.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17E80"))) PPC_WEAK_FUNC(cputest_17E80);
PPC_FUNC_IMPL(__imp__cputest_17E80) {
	PPC_FUNC_PROLOGUE();
	// vsel128 v3,v4,v4,v3
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v3.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17EC0"))) PPC_WEAK_FUNC(cputest_17EC0);
PPC_FUNC_IMPL(__imp__cputest_17EC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v4,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v4.f32[0]) * 1.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[1]) * 1.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[2]) * 1.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[3]) * 1.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17F00"))) PPC_WEAK_FUNC(cputest_17F00);
PPC_FUNC_IMPL(__imp__cputest_17F00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v4,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v4.f32[0]) * 2.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[1]) * 2.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[2]) * 2.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[3]) * 2.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17F40"))) PPC_WEAK_FUNC(cputest_17F40);
PPC_FUNC_IMPL(__imp__cputest_17F40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v4,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v4.f32[0]) * 256.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[1]) * 256.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[2]) * 256.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[3]) * 256.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17F80"))) PPC_WEAK_FUNC(cputest_17F80);
PPC_FUNC_IMPL(__imp__cputest_17F80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v3,v4,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v4.f32[0]) * 2147483648.0;
	ctx.v3.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[1]) * 2147483648.0;
	ctx.v3.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[2]) * 2147483648.0;
	ctx.v3.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v4.f32[3]) * 2147483648.0;
	ctx.v3.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_17FC0"))) PPC_WEAK_FUNC(cputest_17FC0);
PPC_FUNC_IMPL(__imp__cputest_17FC0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v4,0
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18000"))) PPC_WEAK_FUNC(cputest_18000);
PPC_FUNC_IMPL(__imp__cputest_18000) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v4,1
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18040"))) PPC_WEAK_FUNC(cputest_18040);
PPC_FUNC_IMPL(__imp__cputest_18040) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v4,5
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18080"))) PPC_WEAK_FUNC(cputest_18080);
PPC_FUNC_IMPL(__imp__cputest_18080) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v3,v4,v4,15
	simde_mm_store_si128((simde__m128i*)ctx.v3.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v4.u8), simde_mm_load_si128((simde__m128i*)ctx.v4.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_180C0"))) PPC_WEAK_FUNC(cputest_180C0);
PPC_FUNC_IMPL(__imp__cputest_180C0) {
	PPC_FUNC_PROLOGUE();
	// vupkd3d128 v3,v4,20
	{
		uint32_t h0 = ppc_f16_to_f32bits(ctx.v4.u16[0]);
		uint32_t h1 = ppc_f16_to_f32bits(ctx.v4.u16[1]);
		uint32_t h2 = ppc_f16_to_f32bits(ctx.v4.u16[2]);
		uint32_t h3 = ppc_f16_to_f32bits(ctx.v4.u16[3]);
		ctx.v3.u32[0] = h0;
		ctx.v3.u32[1] = h1;
		ctx.v3.u32[2] = h2;
		ctx.v3.u32[3] = h3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18100"))) PPC_WEAK_FUNC(cputest_18100);
PPC_FUNC_IMPL(__imp__cputest_18100) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18140"))) PPC_WEAK_FUNC(cputest_18140);
PPC_FUNC_IMPL(__imp__cputest_18140) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18180"))) PPC_WEAK_FUNC(cputest_18180);
PPC_FUNC_IMPL(__imp__cputest_18180) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_181C0"))) PPC_WEAK_FUNC(cputest_181C0);
PPC_FUNC_IMPL(__imp__cputest_181C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18200"))) PPC_WEAK_FUNC(cputest_18200);
PPC_FUNC_IMPL(__imp__cputest_18200) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18240"))) PPC_WEAK_FUNC(cputest_18240);
PPC_FUNC_IMPL(__imp__cputest_18240) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18280"))) PPC_WEAK_FUNC(cputest_18280);
PPC_FUNC_IMPL(__imp__cputest_18280) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_182C0"))) PPC_WEAK_FUNC(cputest_182C0);
PPC_FUNC_IMPL(__imp__cputest_182C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18300"))) PPC_WEAK_FUNC(cputest_18300);
PPC_FUNC_IMPL(__imp__cputest_18300) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18340"))) PPC_WEAK_FUNC(cputest_18340);
PPC_FUNC_IMPL(__imp__cputest_18340) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18380"))) PPC_WEAK_FUNC(cputest_18380);
PPC_FUNC_IMPL(__imp__cputest_18380) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_183C0"))) PPC_WEAK_FUNC(cputest_183C0);
PPC_FUNC_IMPL(__imp__cputest_183C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,3,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		ctx.v3.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18400"))) PPC_WEAK_FUNC(cputest_18400);
PPC_FUNC_IMPL(__imp__cputest_18400) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18440"))) PPC_WEAK_FUNC(cputest_18440);
PPC_FUNC_IMPL(__imp__cputest_18440) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18480"))) PPC_WEAK_FUNC(cputest_18480);
PPC_FUNC_IMPL(__imp__cputest_18480) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_184C0"))) PPC_WEAK_FUNC(cputest_184C0);
PPC_FUNC_IMPL(__imp__cputest_184C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18500"))) PPC_WEAK_FUNC(cputest_18500);
PPC_FUNC_IMPL(__imp__cputest_18500) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18540"))) PPC_WEAK_FUNC(cputest_18540);
PPC_FUNC_IMPL(__imp__cputest_18540) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18580"))) PPC_WEAK_FUNC(cputest_18580);
PPC_FUNC_IMPL(__imp__cputest_18580) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_185C0"))) PPC_WEAK_FUNC(cputest_185C0);
PPC_FUNC_IMPL(__imp__cputest_185C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18600"))) PPC_WEAK_FUNC(cputest_18600);
PPC_FUNC_IMPL(__imp__cputest_18600) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[1] = n2;
		ctx.v3.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18640"))) PPC_WEAK_FUNC(cputest_18640);
PPC_FUNC_IMPL(__imp__cputest_18640) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[2] = n2;
		ctx.v3.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18680"))) PPC_WEAK_FUNC(cputest_18680);
PPC_FUNC_IMPL(__imp__cputest_18680) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[3] = n2;
		ctx.v3.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_186C0"))) PPC_WEAK_FUNC(cputest_186C0);
PPC_FUNC_IMPL(__imp__cputest_186C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v3,v4,5,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v4.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v4.u32[0]);
		ctx.v3.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18700"))) PPC_WEAK_FUNC(cputest_18700);
PPC_FUNC_IMPL(__imp__cputest_18700) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v40,v40,v40
	simde_mm_store_si128((simde__m128i*)ctx.v40.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v40.u8), simde_mm_load_si128((simde__m128i*)ctx.v40.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18740"))) PPC_WEAK_FUNC(cputest_18740);
PPC_FUNC_IMPL(__imp__cputest_18740) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro128 v40,v40,v40
	temp.u8 = (ctx.v40.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v40.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v40.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18780"))) PPC_WEAK_FUNC(cputest_18780);
PPC_FUNC_IMPL(__imp__cputest_18780) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v40,v40,v40
	simde_mm_store_si128((simde__m128i*)ctx.v40.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v40.s32), simde_mm_load_si128((simde__m128i*)ctx.v40.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_187C0"))) PPC_WEAK_FUNC(cputest_187C0);
PPC_FUNC_IMPL(__imp__cputest_187C0) {
	PPC_FUNC_PROLOGUE();
	// vsel128 v40,v40,v40,v40
	simde_mm_store_si128((simde__m128i*)ctx.v40.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v40.u8), simde_mm_load_si128((simde__m128i*)ctx.v40.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v40.u8), simde_mm_load_si128((simde__m128i*)ctx.v40.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18800"))) PPC_WEAK_FUNC(cputest_18800);
PPC_FUNC_IMPL(__imp__cputest_18800) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v40,v40,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v40.f32[0]) * 1.0;
	ctx.v40.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[1]) * 1.0;
	ctx.v40.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[2]) * 1.0;
	ctx.v40.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[3]) * 1.0;
	ctx.v40.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18840"))) PPC_WEAK_FUNC(cputest_18840);
PPC_FUNC_IMPL(__imp__cputest_18840) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v40,v40,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v40.f32[0]) * 2.0;
	ctx.v40.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[1]) * 2.0;
	ctx.v40.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[2]) * 2.0;
	ctx.v40.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[3]) * 2.0;
	ctx.v40.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18880"))) PPC_WEAK_FUNC(cputest_18880);
PPC_FUNC_IMPL(__imp__cputest_18880) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v40,v40,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v40.f32[0]) * 256.0;
	ctx.v40.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[1]) * 256.0;
	ctx.v40.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[2]) * 256.0;
	ctx.v40.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[3]) * 256.0;
	ctx.v40.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_188C0"))) PPC_WEAK_FUNC(cputest_188C0);
PPC_FUNC_IMPL(__imp__cputest_188C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v40,v40,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v40.f32[0]) * 2147483648.0;
	ctx.v40.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[1]) * 2147483648.0;
	ctx.v40.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[2]) * 2147483648.0;
	ctx.v40.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v40.f32[3]) * 2147483648.0;
	ctx.v40.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18900"))) PPC_WEAK_FUNC(cputest_18900);
PPC_FUNC_IMPL(__imp__cputest_18900) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v40,v40,v40,0
	simde_mm_store_si128((simde__m128i*)ctx.v40.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v40.u8), simde_mm_load_si128((simde__m128i*)ctx.v40.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18940"))) PPC_WEAK_FUNC(cputest_18940);
PPC_FUNC_IMPL(__imp__cputest_18940) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v40,v40,v40,1
	simde_mm_store_si128((simde__m128i*)ctx.v40.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v40.u8), simde_mm_load_si128((simde__m128i*)ctx.v40.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18980"))) PPC_WEAK_FUNC(cputest_18980);
PPC_FUNC_IMPL(__imp__cputest_18980) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v40,v40,v40,5
	simde_mm_store_si128((simde__m128i*)ctx.v40.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v40.u8), simde_mm_load_si128((simde__m128i*)ctx.v40.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_189C0"))) PPC_WEAK_FUNC(cputest_189C0);
PPC_FUNC_IMPL(__imp__cputest_189C0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v40,v40,v40,15
	simde_mm_store_si128((simde__m128i*)ctx.v40.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v40.u8), simde_mm_load_si128((simde__m128i*)ctx.v40.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18A00"))) PPC_WEAK_FUNC(cputest_18A00);
PPC_FUNC_IMPL(__imp__cputest_18A00) {
	PPC_FUNC_PROLOGUE();
	// vupkd3d128 v40,v40,20
	{
		uint32_t h0 = ppc_f16_to_f32bits(ctx.v40.u16[0]);
		uint32_t h1 = ppc_f16_to_f32bits(ctx.v40.u16[1]);
		uint32_t h2 = ppc_f16_to_f32bits(ctx.v40.u16[2]);
		uint32_t h3 = ppc_f16_to_f32bits(ctx.v40.u16[3]);
		ctx.v40.u32[0] = h0;
		ctx.v40.u32[1] = h1;
		ctx.v40.u32[2] = h2;
		ctx.v40.u32[3] = h3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18A40"))) PPC_WEAK_FUNC(cputest_18A40);
PPC_FUNC_IMPL(__imp__cputest_18A40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18A80"))) PPC_WEAK_FUNC(cputest_18A80);
PPC_FUNC_IMPL(__imp__cputest_18A80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18AC0"))) PPC_WEAK_FUNC(cputest_18AC0);
PPC_FUNC_IMPL(__imp__cputest_18AC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18B00"))) PPC_WEAK_FUNC(cputest_18B00);
PPC_FUNC_IMPL(__imp__cputest_18B00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18B40"))) PPC_WEAK_FUNC(cputest_18B40);
PPC_FUNC_IMPL(__imp__cputest_18B40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[1] = n2;
		ctx.v40.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18B80"))) PPC_WEAK_FUNC(cputest_18B80);
PPC_FUNC_IMPL(__imp__cputest_18B80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[2] = n2;
		ctx.v40.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18BC0"))) PPC_WEAK_FUNC(cputest_18BC0);
PPC_FUNC_IMPL(__imp__cputest_18BC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[3] = n2;
		ctx.v40.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18C00"))) PPC_WEAK_FUNC(cputest_18C00);
PPC_FUNC_IMPL(__imp__cputest_18C00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18C40"))) PPC_WEAK_FUNC(cputest_18C40);
PPC_FUNC_IMPL(__imp__cputest_18C40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[1] = n2;
		ctx.v40.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18C80"))) PPC_WEAK_FUNC(cputest_18C80);
PPC_FUNC_IMPL(__imp__cputest_18C80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[2] = n2;
		ctx.v40.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18CC0"))) PPC_WEAK_FUNC(cputest_18CC0);
PPC_FUNC_IMPL(__imp__cputest_18CC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[3] = n2;
		ctx.v40.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18D00"))) PPC_WEAK_FUNC(cputest_18D00);
PPC_FUNC_IMPL(__imp__cputest_18D00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,3,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		ctx.v40.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18D40"))) PPC_WEAK_FUNC(cputest_18D40);
PPC_FUNC_IMPL(__imp__cputest_18D40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18D80"))) PPC_WEAK_FUNC(cputest_18D80);
PPC_FUNC_IMPL(__imp__cputest_18D80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18DC0"))) PPC_WEAK_FUNC(cputest_18DC0);
PPC_FUNC_IMPL(__imp__cputest_18DC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18E00"))) PPC_WEAK_FUNC(cputest_18E00);
PPC_FUNC_IMPL(__imp__cputest_18E00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18E40"))) PPC_WEAK_FUNC(cputest_18E40);
PPC_FUNC_IMPL(__imp__cputest_18E40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[1] = n2;
		ctx.v40.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18E80"))) PPC_WEAK_FUNC(cputest_18E80);
PPC_FUNC_IMPL(__imp__cputest_18E80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[2] = n2;
		ctx.v40.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18EC0"))) PPC_WEAK_FUNC(cputest_18EC0);
PPC_FUNC_IMPL(__imp__cputest_18EC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[3] = n2;
		ctx.v40.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18F00"))) PPC_WEAK_FUNC(cputest_18F00);
PPC_FUNC_IMPL(__imp__cputest_18F00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18F40"))) PPC_WEAK_FUNC(cputest_18F40);
PPC_FUNC_IMPL(__imp__cputest_18F40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[1] = n2;
		ctx.v40.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18F80"))) PPC_WEAK_FUNC(cputest_18F80);
PPC_FUNC_IMPL(__imp__cputest_18F80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[2] = n2;
		ctx.v40.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_18FC0"))) PPC_WEAK_FUNC(cputest_18FC0);
PPC_FUNC_IMPL(__imp__cputest_18FC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[3] = n2;
		ctx.v40.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19000"))) PPC_WEAK_FUNC(cputest_19000);
PPC_FUNC_IMPL(__imp__cputest_19000) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v40,v40,5,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v40.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v40.u32[0]);
		ctx.v40.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19040"))) PPC_WEAK_FUNC(cputest_19040);
PPC_FUNC_IMPL(__imp__cputest_19040) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v127,v64,v100
	simde_mm_store_si128((simde__m128i*)ctx.v127.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v64.u8), simde_mm_load_si128((simde__m128i*)ctx.v100.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19080"))) PPC_WEAK_FUNC(cputest_19080);
PPC_FUNC_IMPL(__imp__cputest_19080) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro128 v127,v64,v100
	temp.u8 = (ctx.v100.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v64.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v127.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_190C0"))) PPC_WEAK_FUNC(cputest_190C0);
PPC_FUNC_IMPL(__imp__cputest_190C0) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v127,v64,v100
	simde_mm_store_si128((simde__m128i*)ctx.v127.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v100.s32), simde_mm_load_si128((simde__m128i*)ctx.v64.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19100"))) PPC_WEAK_FUNC(cputest_19100);
PPC_FUNC_IMPL(__imp__cputest_19100) {
	PPC_FUNC_PROLOGUE();
	// vsel128 v127,v64,v100,v127
	simde_mm_store_si128((simde__m128i*)ctx.v127.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v127.u8), simde_mm_load_si128((simde__m128i*)ctx.v64.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v127.u8), simde_mm_load_si128((simde__m128i*)ctx.v100.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19140"))) PPC_WEAK_FUNC(cputest_19140);
PPC_FUNC_IMPL(__imp__cputest_19140) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v127,v100,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v100.f32[0]) * 1.0;
	ctx.v127.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[1]) * 1.0;
	ctx.v127.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[2]) * 1.0;
	ctx.v127.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[3]) * 1.0;
	ctx.v127.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19180"))) PPC_WEAK_FUNC(cputest_19180);
PPC_FUNC_IMPL(__imp__cputest_19180) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v127,v100,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v100.f32[0]) * 2.0;
	ctx.v127.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[1]) * 2.0;
	ctx.v127.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[2]) * 2.0;
	ctx.v127.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[3]) * 2.0;
	ctx.v127.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_191C0"))) PPC_WEAK_FUNC(cputest_191C0);
PPC_FUNC_IMPL(__imp__cputest_191C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v127,v100,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v100.f32[0]) * 256.0;
	ctx.v127.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[1]) * 256.0;
	ctx.v127.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[2]) * 256.0;
	ctx.v127.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[3]) * 256.0;
	ctx.v127.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19200"))) PPC_WEAK_FUNC(cputest_19200);
PPC_FUNC_IMPL(__imp__cputest_19200) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v127,v100,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v100.f32[0]) * 2147483648.0;
	ctx.v127.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[1]) * 2147483648.0;
	ctx.v127.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[2]) * 2147483648.0;
	ctx.v127.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v100.f32[3]) * 2147483648.0;
	ctx.v127.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19240"))) PPC_WEAK_FUNC(cputest_19240);
PPC_FUNC_IMPL(__imp__cputest_19240) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v127,v64,v100,0
	simde_mm_store_si128((simde__m128i*)ctx.v127.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v64.u8), simde_mm_load_si128((simde__m128i*)ctx.v100.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19280"))) PPC_WEAK_FUNC(cputest_19280);
PPC_FUNC_IMPL(__imp__cputest_19280) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v127,v64,v100,1
	simde_mm_store_si128((simde__m128i*)ctx.v127.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v64.u8), simde_mm_load_si128((simde__m128i*)ctx.v100.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_192C0"))) PPC_WEAK_FUNC(cputest_192C0);
PPC_FUNC_IMPL(__imp__cputest_192C0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v127,v64,v100,5
	simde_mm_store_si128((simde__m128i*)ctx.v127.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v64.u8), simde_mm_load_si128((simde__m128i*)ctx.v100.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19300"))) PPC_WEAK_FUNC(cputest_19300);
PPC_FUNC_IMPL(__imp__cputest_19300) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v127,v64,v100,15
	simde_mm_store_si128((simde__m128i*)ctx.v127.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v64.u8), simde_mm_load_si128((simde__m128i*)ctx.v100.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19340"))) PPC_WEAK_FUNC(cputest_19340);
PPC_FUNC_IMPL(__imp__cputest_19340) {
	PPC_FUNC_PROLOGUE();
	// vupkd3d128 v127,v100,20
	{
		uint32_t h0 = ppc_f16_to_f32bits(ctx.v100.u16[0]);
		uint32_t h1 = ppc_f16_to_f32bits(ctx.v100.u16[1]);
		uint32_t h2 = ppc_f16_to_f32bits(ctx.v100.u16[2]);
		uint32_t h3 = ppc_f16_to_f32bits(ctx.v100.u16[3]);
		ctx.v127.u32[0] = h0;
		ctx.v127.u32[1] = h1;
		ctx.v127.u32[2] = h2;
		ctx.v127.u32[3] = h3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19380"))) PPC_WEAK_FUNC(cputest_19380);
PPC_FUNC_IMPL(__imp__cputest_19380) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_193C0"))) PPC_WEAK_FUNC(cputest_193C0);
PPC_FUNC_IMPL(__imp__cputest_193C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19400"))) PPC_WEAK_FUNC(cputest_19400);
PPC_FUNC_IMPL(__imp__cputest_19400) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19440"))) PPC_WEAK_FUNC(cputest_19440);
PPC_FUNC_IMPL(__imp__cputest_19440) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19480"))) PPC_WEAK_FUNC(cputest_19480);
PPC_FUNC_IMPL(__imp__cputest_19480) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[1] = n2;
		ctx.v127.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_194C0"))) PPC_WEAK_FUNC(cputest_194C0);
PPC_FUNC_IMPL(__imp__cputest_194C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[2] = n2;
		ctx.v127.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19500"))) PPC_WEAK_FUNC(cputest_19500);
PPC_FUNC_IMPL(__imp__cputest_19500) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[3] = n2;
		ctx.v127.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19540"))) PPC_WEAK_FUNC(cputest_19540);
PPC_FUNC_IMPL(__imp__cputest_19540) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19580"))) PPC_WEAK_FUNC(cputest_19580);
PPC_FUNC_IMPL(__imp__cputest_19580) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[1] = n2;
		ctx.v127.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_195C0"))) PPC_WEAK_FUNC(cputest_195C0);
PPC_FUNC_IMPL(__imp__cputest_195C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[2] = n2;
		ctx.v127.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19600"))) PPC_WEAK_FUNC(cputest_19600);
PPC_FUNC_IMPL(__imp__cputest_19600) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[3] = n2;
		ctx.v127.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19640"))) PPC_WEAK_FUNC(cputest_19640);
PPC_FUNC_IMPL(__imp__cputest_19640) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,3,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		ctx.v127.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19680"))) PPC_WEAK_FUNC(cputest_19680);
PPC_FUNC_IMPL(__imp__cputest_19680) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_196C0"))) PPC_WEAK_FUNC(cputest_196C0);
PPC_FUNC_IMPL(__imp__cputest_196C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19700"))) PPC_WEAK_FUNC(cputest_19700);
PPC_FUNC_IMPL(__imp__cputest_19700) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19740"))) PPC_WEAK_FUNC(cputest_19740);
PPC_FUNC_IMPL(__imp__cputest_19740) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19780"))) PPC_WEAK_FUNC(cputest_19780);
PPC_FUNC_IMPL(__imp__cputest_19780) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[1] = n2;
		ctx.v127.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_197C0"))) PPC_WEAK_FUNC(cputest_197C0);
PPC_FUNC_IMPL(__imp__cputest_197C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[2] = n2;
		ctx.v127.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19800"))) PPC_WEAK_FUNC(cputest_19800);
PPC_FUNC_IMPL(__imp__cputest_19800) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[3] = n2;
		ctx.v127.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19840"))) PPC_WEAK_FUNC(cputest_19840);
PPC_FUNC_IMPL(__imp__cputest_19840) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19880"))) PPC_WEAK_FUNC(cputest_19880);
PPC_FUNC_IMPL(__imp__cputest_19880) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[1] = n2;
		ctx.v127.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_198C0"))) PPC_WEAK_FUNC(cputest_198C0);
PPC_FUNC_IMPL(__imp__cputest_198C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[2] = n2;
		ctx.v127.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19900"))) PPC_WEAK_FUNC(cputest_19900);
PPC_FUNC_IMPL(__imp__cputest_19900) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[3] = n2;
		ctx.v127.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19940"))) PPC_WEAK_FUNC(cputest_19940);
PPC_FUNC_IMPL(__imp__cputest_19940) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v127,v100,5,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v100.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v100.u32[0]);
		ctx.v127.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19980"))) PPC_WEAK_FUNC(cputest_19980);
PPC_FUNC_IMPL(__imp__cputest_19980) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v96,v33,v65
	simde_mm_store_si128((simde__m128i*)ctx.v96.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v33.u8), simde_mm_load_si128((simde__m128i*)ctx.v65.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_199C0"))) PPC_WEAK_FUNC(cputest_199C0);
PPC_FUNC_IMPL(__imp__cputest_199C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro128 v96,v33,v65
	temp.u8 = (ctx.v65.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v33.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v96.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19A00"))) PPC_WEAK_FUNC(cputest_19A00);
PPC_FUNC_IMPL(__imp__cputest_19A00) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v96,v33,v65
	simde_mm_store_si128((simde__m128i*)ctx.v96.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v65.s32), simde_mm_load_si128((simde__m128i*)ctx.v33.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19A40"))) PPC_WEAK_FUNC(cputest_19A40);
PPC_FUNC_IMPL(__imp__cputest_19A40) {
	PPC_FUNC_PROLOGUE();
	// vsel128 v96,v33,v65,v96
	simde_mm_store_si128((simde__m128i*)ctx.v96.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v96.u8), simde_mm_load_si128((simde__m128i*)ctx.v33.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v96.u8), simde_mm_load_si128((simde__m128i*)ctx.v65.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19A80"))) PPC_WEAK_FUNC(cputest_19A80);
PPC_FUNC_IMPL(__imp__cputest_19A80) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v96,v65,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v65.f32[0]) * 1.0;
	ctx.v96.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[1]) * 1.0;
	ctx.v96.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[2]) * 1.0;
	ctx.v96.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[3]) * 1.0;
	ctx.v96.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19AC0"))) PPC_WEAK_FUNC(cputest_19AC0);
PPC_FUNC_IMPL(__imp__cputest_19AC0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v96,v65,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v65.f32[0]) * 2.0;
	ctx.v96.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[1]) * 2.0;
	ctx.v96.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[2]) * 2.0;
	ctx.v96.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[3]) * 2.0;
	ctx.v96.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19B00"))) PPC_WEAK_FUNC(cputest_19B00);
PPC_FUNC_IMPL(__imp__cputest_19B00) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v96,v65,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v65.f32[0]) * 256.0;
	ctx.v96.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[1]) * 256.0;
	ctx.v96.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[2]) * 256.0;
	ctx.v96.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[3]) * 256.0;
	ctx.v96.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19B40"))) PPC_WEAK_FUNC(cputest_19B40);
PPC_FUNC_IMPL(__imp__cputest_19B40) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v96,v65,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v65.f32[0]) * 2147483648.0;
	ctx.v96.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[1]) * 2147483648.0;
	ctx.v96.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[2]) * 2147483648.0;
	ctx.v96.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v65.f32[3]) * 2147483648.0;
	ctx.v96.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19B80"))) PPC_WEAK_FUNC(cputest_19B80);
PPC_FUNC_IMPL(__imp__cputest_19B80) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v96,v33,v65,0
	simde_mm_store_si128((simde__m128i*)ctx.v96.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v33.u8), simde_mm_load_si128((simde__m128i*)ctx.v65.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19BC0"))) PPC_WEAK_FUNC(cputest_19BC0);
PPC_FUNC_IMPL(__imp__cputest_19BC0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v96,v33,v65,1
	simde_mm_store_si128((simde__m128i*)ctx.v96.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v33.u8), simde_mm_load_si128((simde__m128i*)ctx.v65.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19C00"))) PPC_WEAK_FUNC(cputest_19C00);
PPC_FUNC_IMPL(__imp__cputest_19C00) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v96,v33,v65,5
	simde_mm_store_si128((simde__m128i*)ctx.v96.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v33.u8), simde_mm_load_si128((simde__m128i*)ctx.v65.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19C40"))) PPC_WEAK_FUNC(cputest_19C40);
PPC_FUNC_IMPL(__imp__cputest_19C40) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v96,v33,v65,15
	simde_mm_store_si128((simde__m128i*)ctx.v96.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v33.u8), simde_mm_load_si128((simde__m128i*)ctx.v65.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19C80"))) PPC_WEAK_FUNC(cputest_19C80);
PPC_FUNC_IMPL(__imp__cputest_19C80) {
	PPC_FUNC_PROLOGUE();
	// vupkd3d128 v96,v65,20
	{
		uint32_t h0 = ppc_f16_to_f32bits(ctx.v65.u16[0]);
		uint32_t h1 = ppc_f16_to_f32bits(ctx.v65.u16[1]);
		uint32_t h2 = ppc_f16_to_f32bits(ctx.v65.u16[2]);
		uint32_t h3 = ppc_f16_to_f32bits(ctx.v65.u16[3]);
		ctx.v96.u32[0] = h0;
		ctx.v96.u32[1] = h1;
		ctx.v96.u32[2] = h2;
		ctx.v96.u32[3] = h3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19CC0"))) PPC_WEAK_FUNC(cputest_19CC0);
PPC_FUNC_IMPL(__imp__cputest_19CC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19D00"))) PPC_WEAK_FUNC(cputest_19D00);
PPC_FUNC_IMPL(__imp__cputest_19D00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19D40"))) PPC_WEAK_FUNC(cputest_19D40);
PPC_FUNC_IMPL(__imp__cputest_19D40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19D80"))) PPC_WEAK_FUNC(cputest_19D80);
PPC_FUNC_IMPL(__imp__cputest_19D80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19DC0"))) PPC_WEAK_FUNC(cputest_19DC0);
PPC_FUNC_IMPL(__imp__cputest_19DC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[1] = n2;
		ctx.v96.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19E00"))) PPC_WEAK_FUNC(cputest_19E00);
PPC_FUNC_IMPL(__imp__cputest_19E00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[2] = n2;
		ctx.v96.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19E40"))) PPC_WEAK_FUNC(cputest_19E40);
PPC_FUNC_IMPL(__imp__cputest_19E40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[3] = n2;
		ctx.v96.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19E80"))) PPC_WEAK_FUNC(cputest_19E80);
PPC_FUNC_IMPL(__imp__cputest_19E80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19EC0"))) PPC_WEAK_FUNC(cputest_19EC0);
PPC_FUNC_IMPL(__imp__cputest_19EC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[1] = n2;
		ctx.v96.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19F00"))) PPC_WEAK_FUNC(cputest_19F00);
PPC_FUNC_IMPL(__imp__cputest_19F00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[2] = n2;
		ctx.v96.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19F40"))) PPC_WEAK_FUNC(cputest_19F40);
PPC_FUNC_IMPL(__imp__cputest_19F40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[3] = n2;
		ctx.v96.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19F80"))) PPC_WEAK_FUNC(cputest_19F80);
PPC_FUNC_IMPL(__imp__cputest_19F80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,3,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		ctx.v96.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_19FC0"))) PPC_WEAK_FUNC(cputest_19FC0);
PPC_FUNC_IMPL(__imp__cputest_19FC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A000"))) PPC_WEAK_FUNC(cputest_1A000);
PPC_FUNC_IMPL(__imp__cputest_1A000) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A040"))) PPC_WEAK_FUNC(cputest_1A040);
PPC_FUNC_IMPL(__imp__cputest_1A040) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A080"))) PPC_WEAK_FUNC(cputest_1A080);
PPC_FUNC_IMPL(__imp__cputest_1A080) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A0C0"))) PPC_WEAK_FUNC(cputest_1A0C0);
PPC_FUNC_IMPL(__imp__cputest_1A0C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[1] = n2;
		ctx.v96.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A100"))) PPC_WEAK_FUNC(cputest_1A100);
PPC_FUNC_IMPL(__imp__cputest_1A100) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[2] = n2;
		ctx.v96.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A140"))) PPC_WEAK_FUNC(cputest_1A140);
PPC_FUNC_IMPL(__imp__cputest_1A140) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[3] = n2;
		ctx.v96.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A180"))) PPC_WEAK_FUNC(cputest_1A180);
PPC_FUNC_IMPL(__imp__cputest_1A180) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A1C0"))) PPC_WEAK_FUNC(cputest_1A1C0);
PPC_FUNC_IMPL(__imp__cputest_1A1C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[1] = n2;
		ctx.v96.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A200"))) PPC_WEAK_FUNC(cputest_1A200);
PPC_FUNC_IMPL(__imp__cputest_1A200) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[2] = n2;
		ctx.v96.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A240"))) PPC_WEAK_FUNC(cputest_1A240);
PPC_FUNC_IMPL(__imp__cputest_1A240) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[3] = n2;
		ctx.v96.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A280"))) PPC_WEAK_FUNC(cputest_1A280);
PPC_FUNC_IMPL(__imp__cputest_1A280) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v96,v65,5,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v65.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v65.u32[0]);
		ctx.v96.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A2C0"))) PPC_WEAK_FUNC(cputest_1A2C0);
PPC_FUNC_IMPL(__imp__cputest_1A2C0) {
	PPC_FUNC_PROLOGUE();
	// vnor128 v0,v127,v1
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_xor_si128(simde_mm_or_si128(simde_mm_load_si128((simde__m128i*)ctx.v127.u8), simde_mm_load_si128((simde__m128i*)ctx.v1.u8)), simde_mm_set1_epi32(-1)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A300"))) PPC_WEAK_FUNC(cputest_1A300);
PPC_FUNC_IMPL(__imp__cputest_1A300) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vsro128 v0,v127,v1
	temp.u8 = (ctx.v1.u8[0] >> 3) & 0xF;
	{ uint8_t tmp_vsro[16]; for (int i = 0; i < 16; i++) tmp_vsro[i] = (i + temp.u8 < 16) ? ctx.v127.u8[i + temp.u8] : 0; for (int i = 0; i < 16; i++) ctx.v0.u8[i] = tmp_vsro[i]; }
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A340"))) PPC_WEAK_FUNC(cputest_1A340);
PPC_FUNC_IMPL(__imp__cputest_1A340) {
	PPC_FUNC_PROLOGUE();
	// vpkswss128 v0,v127,v1
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_packs_epi32(simde_mm_load_si128((simde__m128i*)ctx.v1.s32), simde_mm_load_si128((simde__m128i*)ctx.v127.s32)));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A380"))) PPC_WEAK_FUNC(cputest_1A380);
PPC_FUNC_IMPL(__imp__cputest_1A380) {
	PPC_FUNC_PROLOGUE();
	// vsel128 v0,v127,v1,v0
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_or_si128(simde_mm_andnot_si128(simde_mm_load_si128((simde__m128i*)ctx.v0.u8), simde_mm_load_si128((simde__m128i*)ctx.v127.u8)), simde_mm_and_si128(simde_mm_load_si128((simde__m128i*)ctx.v0.u8), simde_mm_load_si128((simde__m128i*)ctx.v1.u8))));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A3C0"))) PPC_WEAK_FUNC(cputest_1A3C0);
PPC_FUNC_IMPL(__imp__cputest_1A3C0) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v0,v1,0
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v1.f32[0]) * 1.0;
	ctx.v0.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[1]) * 1.0;
	ctx.v0.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[2]) * 1.0;
	ctx.v0.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[3]) * 1.0;
	ctx.v0.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A400"))) PPC_WEAK_FUNC(cputest_1A400);
PPC_FUNC_IMPL(__imp__cputest_1A400) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v0,v1,1
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v1.f32[0]) * 2.0;
	ctx.v0.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[1]) * 2.0;
	ctx.v0.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[2]) * 2.0;
	ctx.v0.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[3]) * 2.0;
	ctx.v0.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A440"))) PPC_WEAK_FUNC(cputest_1A440);
PPC_FUNC_IMPL(__imp__cputest_1A440) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v0,v1,8
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v1.f32[0]) * 256.0;
	ctx.v0.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[1]) * 256.0;
	ctx.v0.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[2]) * 256.0;
	ctx.v0.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[3]) * 256.0;
	ctx.v0.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A480"))) PPC_WEAK_FUNC(cputest_1A480);
PPC_FUNC_IMPL(__imp__cputest_1A480) {
	PPC_FUNC_PROLOGUE();
	PPCRegister temp{};
	// vcfpuxws128 v0,v1,31
	ctx.fpscr.enableFlushMode();
	temp.f64 = double(ctx.v1.f32[0]) * 2147483648.0;
	ctx.v0.u32[0] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[1]) * 2147483648.0;
	ctx.v0.u32[1] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[2]) * 2147483648.0;
	ctx.v0.u32[2] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	temp.f64 = double(ctx.v1.f32[3]) * 2147483648.0;
	ctx.v0.u32[3] = temp.f64 >= 4294967296.0 ? 0xFFFFFFFFu : (temp.f64 > 0.0 ? uint32_t(temp.f64) : 0u);
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A4C0"))) PPC_WEAK_FUNC(cputest_1A4C0);
PPC_FUNC_IMPL(__imp__cputest_1A4C0) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v0,v127,v1,0
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v127.u8), simde_mm_load_si128((simde__m128i*)ctx.v1.u8), 16));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A500"))) PPC_WEAK_FUNC(cputest_1A500);
PPC_FUNC_IMPL(__imp__cputest_1A500) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v0,v127,v1,1
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v127.u8), simde_mm_load_si128((simde__m128i*)ctx.v1.u8), 15));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A540"))) PPC_WEAK_FUNC(cputest_1A540);
PPC_FUNC_IMPL(__imp__cputest_1A540) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v0,v127,v1,5
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v127.u8), simde_mm_load_si128((simde__m128i*)ctx.v1.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A580"))) PPC_WEAK_FUNC(cputest_1A580);
PPC_FUNC_IMPL(__imp__cputest_1A580) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v0,v127,v1,15
	simde_mm_store_si128((simde__m128i*)ctx.v0.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v127.u8), simde_mm_load_si128((simde__m128i*)ctx.v1.u8), 1));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A5C0"))) PPC_WEAK_FUNC(cputest_1A5C0);
PPC_FUNC_IMPL(__imp__cputest_1A5C0) {
	PPC_FUNC_PROLOGUE();
	// vupkd3d128 v0,v1,20
	{
		uint32_t h0 = ppc_f16_to_f32bits(ctx.v1.u16[0]);
		uint32_t h1 = ppc_f16_to_f32bits(ctx.v1.u16[1]);
		uint32_t h2 = ppc_f16_to_f32bits(ctx.v1.u16[2]);
		uint32_t h3 = ppc_f16_to_f32bits(ctx.v1.u16[3]);
		ctx.v0.u32[0] = h0;
		ctx.v0.u32[1] = h1;
		ctx.v0.u32[2] = h2;
		ctx.v0.u32[3] = h3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A600"))) PPC_WEAK_FUNC(cputest_1A600);
PPC_FUNC_IMPL(__imp__cputest_1A600) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A640"))) PPC_WEAK_FUNC(cputest_1A640);
PPC_FUNC_IMPL(__imp__cputest_1A640) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A680"))) PPC_WEAK_FUNC(cputest_1A680);
PPC_FUNC_IMPL(__imp__cputest_1A680) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A6C0"))) PPC_WEAK_FUNC(cputest_1A6C0);
PPC_FUNC_IMPL(__imp__cputest_1A6C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A700"))) PPC_WEAK_FUNC(cputest_1A700);
PPC_FUNC_IMPL(__imp__cputest_1A700) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[1] = n2;
		ctx.v0.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A740"))) PPC_WEAK_FUNC(cputest_1A740);
PPC_FUNC_IMPL(__imp__cputest_1A740) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[2] = n2;
		ctx.v0.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A780"))) PPC_WEAK_FUNC(cputest_1A780);
PPC_FUNC_IMPL(__imp__cputest_1A780) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[3] = n2;
		ctx.v0.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A7C0"))) PPC_WEAK_FUNC(cputest_1A7C0);
PPC_FUNC_IMPL(__imp__cputest_1A7C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A800"))) PPC_WEAK_FUNC(cputest_1A800);
PPC_FUNC_IMPL(__imp__cputest_1A800) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[1] = n2;
		ctx.v0.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A840"))) PPC_WEAK_FUNC(cputest_1A840);
PPC_FUNC_IMPL(__imp__cputest_1A840) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[2] = n2;
		ctx.v0.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A880"))) PPC_WEAK_FUNC(cputest_1A880);
PPC_FUNC_IMPL(__imp__cputest_1A880) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[3] = n2;
		ctx.v0.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A8C0"))) PPC_WEAK_FUNC(cputest_1A8C0);
PPC_FUNC_IMPL(__imp__cputest_1A8C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,3,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = 0;
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		ctx.v0.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A900"))) PPC_WEAK_FUNC(cputest_1A900);
PPC_FUNC_IMPL(__imp__cputest_1A900) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,1,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A940"))) PPC_WEAK_FUNC(cputest_1A940);
PPC_FUNC_IMPL(__imp__cputest_1A940) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,1,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A980"))) PPC_WEAK_FUNC(cputest_1A980);
PPC_FUNC_IMPL(__imp__cputest_1A980) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,1,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1A9C0"))) PPC_WEAK_FUNC(cputest_1A9C0);
PPC_FUNC_IMPL(__imp__cputest_1A9C0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,1,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1AA00"))) PPC_WEAK_FUNC(cputest_1AA00);
PPC_FUNC_IMPL(__imp__cputest_1AA00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,2,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[1] = n2;
		ctx.v0.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1AA40"))) PPC_WEAK_FUNC(cputest_1AA40);
PPC_FUNC_IMPL(__imp__cputest_1AA40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,2,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[2] = n2;
		ctx.v0.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1AA80"))) PPC_WEAK_FUNC(cputest_1AA80);
PPC_FUNC_IMPL(__imp__cputest_1AA80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,2,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[3] = n2;
		ctx.v0.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1AAC0"))) PPC_WEAK_FUNC(cputest_1AAC0);
PPC_FUNC_IMPL(__imp__cputest_1AAC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,2,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[3] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1AB00"))) PPC_WEAK_FUNC(cputest_1AB00);
PPC_FUNC_IMPL(__imp__cputest_1AB00) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,3,0
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[1] = n2;
		ctx.v0.u32[0] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1AB40"))) PPC_WEAK_FUNC(cputest_1AB40);
PPC_FUNC_IMPL(__imp__cputest_1AB40) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,3,1
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[2] = n2;
		ctx.v0.u32[1] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1AB80"))) PPC_WEAK_FUNC(cputest_1AB80);
PPC_FUNC_IMPL(__imp__cputest_1AB80) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,3,2
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[3] = n2;
		ctx.v0.u32[2] = n3;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1ABC0"))) PPC_WEAK_FUNC(cputest_1ABC0);
PPC_FUNC_IMPL(__imp__cputest_1ABC0) {
	PPC_FUNC_PROLOGUE();
	// vpkd3d128 v0,v1,5,3,3
	ctx.fpscr.enableFlushMode();
	{
		uint32_t n2 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[3])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[2]);
		uint32_t n3 = (uint32_t(ppc_f32bits_to_f16_rz(ctx.v1.u32[1])) << 16) | ppc_f32bits_to_f16_rz(ctx.v1.u32[0]);
		ctx.v0.u32[0] = n2;
	}
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1AC00"))) PPC_WEAK_FUNC(cputest_1AC00);
PPC_FUNC_IMPL(__imp__cputest_1AC00) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v9,v13,v59,5
	simde_mm_store_si128((simde__m128i*)ctx.v9.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v13.u8), simde_mm_load_si128((simde__m128i*)ctx.v59.u8), 11));
	// blr 
	return;
}

__attribute__((alias("__imp__cputest_1AC40"))) PPC_WEAK_FUNC(cputest_1AC40);
PPC_FUNC_IMPL(__imp__cputest_1AC40) {
	PPC_FUNC_PROLOGUE();
	// vsldoi128 v6,v12,v59,5
	simde_mm_store_si128((simde__m128i*)ctx.v6.u8, simde_mm_alignr_epi8(simde_mm_load_si128((simde__m128i*)ctx.v12.u8), simde_mm_load_si128((simde__m128i*)ctx.v59.u8), 11));
	// blr 
	return;
}


PPC_EXTERN_FUNC(cputest_10000);
PPC_EXTERN_FUNC(cputest_10040);
PPC_EXTERN_FUNC(cputest_10080);
PPC_EXTERN_FUNC(cputest_100C0);
PPC_EXTERN_FUNC(cputest_10100);
PPC_EXTERN_FUNC(cputest_10140);
PPC_EXTERN_FUNC(cputest_10180);
PPC_EXTERN_FUNC(cputest_101C0);
PPC_EXTERN_FUNC(cputest_10200);
PPC_EXTERN_FUNC(cputest_10240);
PPC_EXTERN_FUNC(cputest_10280);
PPC_EXTERN_FUNC(cputest_102C0);
PPC_EXTERN_FUNC(cputest_10300);
PPC_EXTERN_FUNC(cputest_10340);
PPC_EXTERN_FUNC(cputest_10380);
PPC_EXTERN_FUNC(cputest_103C0);
PPC_EXTERN_FUNC(cputest_10400);
PPC_EXTERN_FUNC(cputest_10440);
PPC_EXTERN_FUNC(cputest_10480);
PPC_EXTERN_FUNC(cputest_104C0);
PPC_EXTERN_FUNC(cputest_10500);
PPC_EXTERN_FUNC(cputest_10540);
PPC_EXTERN_FUNC(cputest_10580);
PPC_EXTERN_FUNC(cputest_105C0);
PPC_EXTERN_FUNC(cputest_10600);
PPC_EXTERN_FUNC(cputest_10640);
PPC_EXTERN_FUNC(cputest_10680);
PPC_EXTERN_FUNC(cputest_106C0);
PPC_EXTERN_FUNC(cputest_10700);
PPC_EXTERN_FUNC(cputest_10740);
PPC_EXTERN_FUNC(cputest_10780);
PPC_EXTERN_FUNC(cputest_107C0);
PPC_EXTERN_FUNC(cputest_10800);
PPC_EXTERN_FUNC(cputest_10840);
PPC_EXTERN_FUNC(cputest_10880);
PPC_EXTERN_FUNC(cputest_108C0);
PPC_EXTERN_FUNC(cputest_10900);
PPC_EXTERN_FUNC(cputest_10940);
PPC_EXTERN_FUNC(cputest_10980);
PPC_EXTERN_FUNC(cputest_109C0);
PPC_EXTERN_FUNC(cputest_10A00);
PPC_EXTERN_FUNC(cputest_10A40);
PPC_EXTERN_FUNC(cputest_10A80);
PPC_EXTERN_FUNC(cputest_10AC0);
PPC_EXTERN_FUNC(cputest_10B00);
PPC_EXTERN_FUNC(cputest_10B40);
PPC_EXTERN_FUNC(cputest_10B80);
PPC_EXTERN_FUNC(cputest_10BC0);
PPC_EXTERN_FUNC(cputest_10C00);
PPC_EXTERN_FUNC(cputest_10C40);
PPC_EXTERN_FUNC(cputest_10C80);
PPC_EXTERN_FUNC(cputest_10CC0);
PPC_EXTERN_FUNC(cputest_10D00);
PPC_EXTERN_FUNC(cputest_10D40);
PPC_EXTERN_FUNC(cputest_10D80);
PPC_EXTERN_FUNC(cputest_10DC0);
PPC_EXTERN_FUNC(cputest_10E00);
PPC_EXTERN_FUNC(cputest_10E40);
PPC_EXTERN_FUNC(cputest_10E80);
PPC_EXTERN_FUNC(cputest_10EC0);
PPC_EXTERN_FUNC(cputest_10F00);
PPC_EXTERN_FUNC(cputest_10F40);
PPC_EXTERN_FUNC(cputest_10F80);
PPC_EXTERN_FUNC(cputest_10FC0);
PPC_EXTERN_FUNC(cputest_11000);
PPC_EXTERN_FUNC(cputest_11040);
PPC_EXTERN_FUNC(cputest_11080);
PPC_EXTERN_FUNC(cputest_110C0);
PPC_EXTERN_FUNC(cputest_11100);
PPC_EXTERN_FUNC(cputest_11140);
PPC_EXTERN_FUNC(cputest_11180);
PPC_EXTERN_FUNC(cputest_111C0);
PPC_EXTERN_FUNC(cputest_11200);
PPC_EXTERN_FUNC(cputest_11240);
PPC_EXTERN_FUNC(cputest_11280);
PPC_EXTERN_FUNC(cputest_112C0);
PPC_EXTERN_FUNC(cputest_11300);
PPC_EXTERN_FUNC(cputest_11340);
PPC_EXTERN_FUNC(cputest_11380);
PPC_EXTERN_FUNC(cputest_113C0);
PPC_EXTERN_FUNC(cputest_11400);
PPC_EXTERN_FUNC(cputest_11440);
PPC_EXTERN_FUNC(cputest_11480);
PPC_EXTERN_FUNC(cputest_114C0);
PPC_EXTERN_FUNC(cputest_11500);
PPC_EXTERN_FUNC(cputest_11540);
PPC_EXTERN_FUNC(cputest_11580);
PPC_EXTERN_FUNC(cputest_115C0);
PPC_EXTERN_FUNC(cputest_11600);
PPC_EXTERN_FUNC(cputest_11640);
PPC_EXTERN_FUNC(cputest_11680);
PPC_EXTERN_FUNC(cputest_116C0);
PPC_EXTERN_FUNC(cputest_11700);
PPC_EXTERN_FUNC(cputest_11740);
PPC_EXTERN_FUNC(cputest_11780);
PPC_EXTERN_FUNC(cputest_117C0);
PPC_EXTERN_FUNC(cputest_11800);
PPC_EXTERN_FUNC(cputest_11840);
PPC_EXTERN_FUNC(cputest_11880);
PPC_EXTERN_FUNC(cputest_118C0);
PPC_EXTERN_FUNC(cputest_11900);
PPC_EXTERN_FUNC(cputest_11940);
PPC_EXTERN_FUNC(cputest_11980);
PPC_EXTERN_FUNC(cputest_119C0);
PPC_EXTERN_FUNC(cputest_11A00);
PPC_EXTERN_FUNC(cputest_11A40);
PPC_EXTERN_FUNC(cputest_11A80);
PPC_EXTERN_FUNC(cputest_11AC0);
PPC_EXTERN_FUNC(cputest_11B00);
PPC_EXTERN_FUNC(cputest_11B40);
PPC_EXTERN_FUNC(cputest_11B80);
PPC_EXTERN_FUNC(cputest_11BC0);
PPC_EXTERN_FUNC(cputest_11C00);
PPC_EXTERN_FUNC(cputest_11C40);
PPC_EXTERN_FUNC(cputest_11C80);
PPC_EXTERN_FUNC(cputest_11CC0);
PPC_EXTERN_FUNC(cputest_11D00);
PPC_EXTERN_FUNC(cputest_11D40);
PPC_EXTERN_FUNC(cputest_11D80);
PPC_EXTERN_FUNC(cputest_11DC0);
PPC_EXTERN_FUNC(cputest_11E00);
PPC_EXTERN_FUNC(cputest_11E40);
PPC_EXTERN_FUNC(cputest_11E80);
PPC_EXTERN_FUNC(cputest_11EC0);
PPC_EXTERN_FUNC(cputest_11F00);
PPC_EXTERN_FUNC(cputest_11F40);
PPC_EXTERN_FUNC(cputest_11F80);
PPC_EXTERN_FUNC(cputest_11FC0);
PPC_EXTERN_FUNC(cputest_12000);
PPC_EXTERN_FUNC(cputest_12040);
PPC_EXTERN_FUNC(cputest_12080);
PPC_EXTERN_FUNC(cputest_120C0);
PPC_EXTERN_FUNC(cputest_12100);
PPC_EXTERN_FUNC(cputest_12140);
PPC_EXTERN_FUNC(cputest_12180);
PPC_EXTERN_FUNC(cputest_121C0);
PPC_EXTERN_FUNC(cputest_12200);
PPC_EXTERN_FUNC(cputest_12240);
PPC_EXTERN_FUNC(cputest_12280);
PPC_EXTERN_FUNC(cputest_122C0);
PPC_EXTERN_FUNC(cputest_12300);
PPC_EXTERN_FUNC(cputest_12340);
PPC_EXTERN_FUNC(cputest_12380);
PPC_EXTERN_FUNC(cputest_123C0);
PPC_EXTERN_FUNC(cputest_12400);
PPC_EXTERN_FUNC(cputest_12440);
PPC_EXTERN_FUNC(cputest_12480);
PPC_EXTERN_FUNC(cputest_124C0);
PPC_EXTERN_FUNC(cputest_12500);
PPC_EXTERN_FUNC(cputest_12540);
PPC_EXTERN_FUNC(cputest_12580);
PPC_EXTERN_FUNC(cputest_125C0);
PPC_EXTERN_FUNC(cputest_12600);
PPC_EXTERN_FUNC(cputest_12640);
PPC_EXTERN_FUNC(cputest_12680);
PPC_EXTERN_FUNC(cputest_126C0);
PPC_EXTERN_FUNC(cputest_12700);
PPC_EXTERN_FUNC(cputest_12740);
PPC_EXTERN_FUNC(cputest_12780);
PPC_EXTERN_FUNC(cputest_127C0);
PPC_EXTERN_FUNC(cputest_12800);
PPC_EXTERN_FUNC(cputest_12840);
PPC_EXTERN_FUNC(cputest_12880);
PPC_EXTERN_FUNC(cputest_128C0);
PPC_EXTERN_FUNC(cputest_12900);
PPC_EXTERN_FUNC(cputest_12940);
PPC_EXTERN_FUNC(cputest_12980);
PPC_EXTERN_FUNC(cputest_129C0);
PPC_EXTERN_FUNC(cputest_12A00);
PPC_EXTERN_FUNC(cputest_12A40);
PPC_EXTERN_FUNC(cputest_12A80);
PPC_EXTERN_FUNC(cputest_12AC0);
PPC_EXTERN_FUNC(cputest_12B00);
PPC_EXTERN_FUNC(cputest_12B40);
PPC_EXTERN_FUNC(cputest_12B80);
PPC_EXTERN_FUNC(cputest_12BC0);
PPC_EXTERN_FUNC(cputest_12C00);
PPC_EXTERN_FUNC(cputest_12C40);
PPC_EXTERN_FUNC(cputest_12C80);
PPC_EXTERN_FUNC(cputest_12CC0);
PPC_EXTERN_FUNC(cputest_12D00);
PPC_EXTERN_FUNC(cputest_12D40);
PPC_EXTERN_FUNC(cputest_12D80);
PPC_EXTERN_FUNC(cputest_12DC0);
PPC_EXTERN_FUNC(cputest_12E00);
PPC_EXTERN_FUNC(cputest_12E40);
PPC_EXTERN_FUNC(cputest_12E80);
PPC_EXTERN_FUNC(cputest_12EC0);
PPC_EXTERN_FUNC(cputest_12F00);
PPC_EXTERN_FUNC(cputest_12F40);
PPC_EXTERN_FUNC(cputest_12F80);
PPC_EXTERN_FUNC(cputest_12FC0);
PPC_EXTERN_FUNC(cputest_13000);
PPC_EXTERN_FUNC(cputest_13040);
PPC_EXTERN_FUNC(cputest_13080);
PPC_EXTERN_FUNC(cputest_130C0);
PPC_EXTERN_FUNC(cputest_13100);
PPC_EXTERN_FUNC(cputest_13140);
PPC_EXTERN_FUNC(cputest_13180);
PPC_EXTERN_FUNC(cputest_131C0);
PPC_EXTERN_FUNC(cputest_13200);
PPC_EXTERN_FUNC(cputest_13240);
PPC_EXTERN_FUNC(cputest_13280);
PPC_EXTERN_FUNC(cputest_132C0);
PPC_EXTERN_FUNC(cputest_13300);
PPC_EXTERN_FUNC(cputest_13340);
PPC_EXTERN_FUNC(cputest_13380);
PPC_EXTERN_FUNC(cputest_133C0);
PPC_EXTERN_FUNC(cputest_13400);
PPC_EXTERN_FUNC(cputest_13440);
PPC_EXTERN_FUNC(cputest_13480);
PPC_EXTERN_FUNC(cputest_134C0);
PPC_EXTERN_FUNC(cputest_13500);
PPC_EXTERN_FUNC(cputest_13540);
PPC_EXTERN_FUNC(cputest_13580);
PPC_EXTERN_FUNC(cputest_135C0);
PPC_EXTERN_FUNC(cputest_13600);
PPC_EXTERN_FUNC(cputest_13640);
PPC_EXTERN_FUNC(cputest_13680);
PPC_EXTERN_FUNC(cputest_136C0);
PPC_EXTERN_FUNC(cputest_13700);
PPC_EXTERN_FUNC(cputest_13740);
PPC_EXTERN_FUNC(cputest_13780);
PPC_EXTERN_FUNC(cputest_137C0);
PPC_EXTERN_FUNC(cputest_13800);
PPC_EXTERN_FUNC(cputest_13840);
PPC_EXTERN_FUNC(cputest_13880);
PPC_EXTERN_FUNC(cputest_138C0);
PPC_EXTERN_FUNC(cputest_13900);
PPC_EXTERN_FUNC(cputest_13940);
PPC_EXTERN_FUNC(cputest_13980);
PPC_EXTERN_FUNC(cputest_139C0);
PPC_EXTERN_FUNC(cputest_13A00);
PPC_EXTERN_FUNC(cputest_13A40);
PPC_EXTERN_FUNC(cputest_13A80);
PPC_EXTERN_FUNC(cputest_13AC0);
PPC_EXTERN_FUNC(cputest_13B00);
PPC_EXTERN_FUNC(cputest_13B40);
PPC_EXTERN_FUNC(cputest_13B80);
PPC_EXTERN_FUNC(cputest_13BC0);
PPC_EXTERN_FUNC(cputest_13C00);
PPC_EXTERN_FUNC(cputest_13C40);
PPC_EXTERN_FUNC(cputest_13C80);
PPC_EXTERN_FUNC(cputest_13CC0);
PPC_EXTERN_FUNC(cputest_13D00);
PPC_EXTERN_FUNC(cputest_13D40);
PPC_EXTERN_FUNC(cputest_13D80);
PPC_EXTERN_FUNC(cputest_13DC0);
PPC_EXTERN_FUNC(cputest_13E00);
PPC_EXTERN_FUNC(cputest_13E40);
PPC_EXTERN_FUNC(cputest_13E80);
PPC_EXTERN_FUNC(cputest_13EC0);
PPC_EXTERN_FUNC(cputest_13F00);
PPC_EXTERN_FUNC(cputest_13F40);
PPC_EXTERN_FUNC(cputest_13F80);
PPC_EXTERN_FUNC(cputest_13FC0);
PPC_EXTERN_FUNC(cputest_14000);
PPC_EXTERN_FUNC(cputest_14040);
PPC_EXTERN_FUNC(cputest_14080);
PPC_EXTERN_FUNC(cputest_140C0);
PPC_EXTERN_FUNC(cputest_14100);
PPC_EXTERN_FUNC(cputest_14140);
PPC_EXTERN_FUNC(cputest_14180);
PPC_EXTERN_FUNC(cputest_141C0);
PPC_EXTERN_FUNC(cputest_14200);
PPC_EXTERN_FUNC(cputest_14240);
PPC_EXTERN_FUNC(cputest_14280);
PPC_EXTERN_FUNC(cputest_142C0);
PPC_EXTERN_FUNC(cputest_14300);
PPC_EXTERN_FUNC(cputest_14340);
PPC_EXTERN_FUNC(cputest_14380);
PPC_EXTERN_FUNC(cputest_143C0);
PPC_EXTERN_FUNC(cputest_14400);
PPC_EXTERN_FUNC(cputest_14440);
PPC_EXTERN_FUNC(cputest_14480);
PPC_EXTERN_FUNC(cputest_144C0);
PPC_EXTERN_FUNC(cputest_14500);
PPC_EXTERN_FUNC(cputest_14540);
PPC_EXTERN_FUNC(cputest_14580);
PPC_EXTERN_FUNC(cputest_145C0);
PPC_EXTERN_FUNC(cputest_14600);
PPC_EXTERN_FUNC(cputest_14640);
PPC_EXTERN_FUNC(cputest_14680);
PPC_EXTERN_FUNC(cputest_146C0);
PPC_EXTERN_FUNC(cputest_14700);
PPC_EXTERN_FUNC(cputest_14740);
PPC_EXTERN_FUNC(cputest_14780);
PPC_EXTERN_FUNC(cputest_147C0);
PPC_EXTERN_FUNC(cputest_14800);
PPC_EXTERN_FUNC(cputest_14840);
PPC_EXTERN_FUNC(cputest_14880);
PPC_EXTERN_FUNC(cputest_148C0);
PPC_EXTERN_FUNC(cputest_14900);
PPC_EXTERN_FUNC(cputest_14940);
PPC_EXTERN_FUNC(cputest_14980);
PPC_EXTERN_FUNC(cputest_149C0);
PPC_EXTERN_FUNC(cputest_14A00);
PPC_EXTERN_FUNC(cputest_14A40);
PPC_EXTERN_FUNC(cputest_14A80);
PPC_EXTERN_FUNC(cputest_14AC0);
PPC_EXTERN_FUNC(cputest_14B00);
PPC_EXTERN_FUNC(cputest_14B40);
PPC_EXTERN_FUNC(cputest_14B80);
PPC_EXTERN_FUNC(cputest_14BC0);
PPC_EXTERN_FUNC(cputest_14C00);
PPC_EXTERN_FUNC(cputest_14C40);
PPC_EXTERN_FUNC(cputest_14C80);
PPC_EXTERN_FUNC(cputest_14CC0);
PPC_EXTERN_FUNC(cputest_14D00);
PPC_EXTERN_FUNC(cputest_14D40);
PPC_EXTERN_FUNC(cputest_14D80);
PPC_EXTERN_FUNC(cputest_14DC0);
PPC_EXTERN_FUNC(cputest_14E00);
PPC_EXTERN_FUNC(cputest_14E40);
PPC_EXTERN_FUNC(cputest_14E80);
PPC_EXTERN_FUNC(cputest_14EC0);
PPC_EXTERN_FUNC(cputest_14F00);
PPC_EXTERN_FUNC(cputest_14F40);
PPC_EXTERN_FUNC(cputest_14F80);
PPC_EXTERN_FUNC(cputest_14FC0);
PPC_EXTERN_FUNC(cputest_15000);
PPC_EXTERN_FUNC(cputest_15040);
PPC_EXTERN_FUNC(cputest_15080);
PPC_EXTERN_FUNC(cputest_150C0);
PPC_EXTERN_FUNC(cputest_15100);
PPC_EXTERN_FUNC(cputest_15140);
PPC_EXTERN_FUNC(cputest_15180);
PPC_EXTERN_FUNC(cputest_151C0);
PPC_EXTERN_FUNC(cputest_15200);
PPC_EXTERN_FUNC(cputest_15240);
PPC_EXTERN_FUNC(cputest_15280);
PPC_EXTERN_FUNC(cputest_152C0);
PPC_EXTERN_FUNC(cputest_15300);
PPC_EXTERN_FUNC(cputest_15340);
PPC_EXTERN_FUNC(cputest_15380);
PPC_EXTERN_FUNC(cputest_153C0);
PPC_EXTERN_FUNC(cputest_15400);
PPC_EXTERN_FUNC(cputest_15440);
PPC_EXTERN_FUNC(cputest_15480);
PPC_EXTERN_FUNC(cputest_154C0);
PPC_EXTERN_FUNC(cputest_15500);
PPC_EXTERN_FUNC(cputest_15540);
PPC_EXTERN_FUNC(cputest_15580);
PPC_EXTERN_FUNC(cputest_155C0);
PPC_EXTERN_FUNC(cputest_15600);
PPC_EXTERN_FUNC(cputest_15640);
PPC_EXTERN_FUNC(cputest_15680);
PPC_EXTERN_FUNC(cputest_156C0);
PPC_EXTERN_FUNC(cputest_15700);
PPC_EXTERN_FUNC(cputest_15740);
PPC_EXTERN_FUNC(cputest_15780);
PPC_EXTERN_FUNC(cputest_157C0);
PPC_EXTERN_FUNC(cputest_15800);
PPC_EXTERN_FUNC(cputest_15840);
PPC_EXTERN_FUNC(cputest_15880);
PPC_EXTERN_FUNC(cputest_158C0);
PPC_EXTERN_FUNC(cputest_15900);
PPC_EXTERN_FUNC(cputest_15940);
PPC_EXTERN_FUNC(cputest_15980);
PPC_EXTERN_FUNC(cputest_159C0);
PPC_EXTERN_FUNC(cputest_15A00);
PPC_EXTERN_FUNC(cputest_15A40);
PPC_EXTERN_FUNC(cputest_15A80);
PPC_EXTERN_FUNC(cputest_15AC0);
PPC_EXTERN_FUNC(cputest_15B00);
PPC_EXTERN_FUNC(cputest_15B40);
PPC_EXTERN_FUNC(cputest_15B80);
PPC_EXTERN_FUNC(cputest_15BC0);
PPC_EXTERN_FUNC(cputest_15C00);
PPC_EXTERN_FUNC(cputest_15C40);
PPC_EXTERN_FUNC(cputest_15C80);
PPC_EXTERN_FUNC(cputest_15CC0);
PPC_EXTERN_FUNC(cputest_15D00);
PPC_EXTERN_FUNC(cputest_15D40);
PPC_EXTERN_FUNC(cputest_15D80);
PPC_EXTERN_FUNC(cputest_15DC0);
PPC_EXTERN_FUNC(cputest_15E00);
PPC_EXTERN_FUNC(cputest_15E40);
PPC_EXTERN_FUNC(cputest_15E80);
PPC_EXTERN_FUNC(cputest_15EC0);
PPC_EXTERN_FUNC(cputest_15F00);
PPC_EXTERN_FUNC(cputest_15F40);
PPC_EXTERN_FUNC(cputest_15F80);
PPC_EXTERN_FUNC(cputest_15FC0);
PPC_EXTERN_FUNC(cputest_16000);
PPC_EXTERN_FUNC(cputest_16040);
PPC_EXTERN_FUNC(cputest_16080);
PPC_EXTERN_FUNC(cputest_160C0);
PPC_EXTERN_FUNC(cputest_16100);
PPC_EXTERN_FUNC(cputest_16140);
PPC_EXTERN_FUNC(cputest_16180);
PPC_EXTERN_FUNC(cputest_161C0);
PPC_EXTERN_FUNC(cputest_16200);
PPC_EXTERN_FUNC(cputest_16240);
PPC_EXTERN_FUNC(cputest_16280);
PPC_EXTERN_FUNC(cputest_162C0);
PPC_EXTERN_FUNC(cputest_16300);
PPC_EXTERN_FUNC(cputest_16340);
PPC_EXTERN_FUNC(cputest_16380);
PPC_EXTERN_FUNC(cputest_163C0);
PPC_EXTERN_FUNC(cputest_16400);
PPC_EXTERN_FUNC(cputest_16440);
PPC_EXTERN_FUNC(cputest_16480);
PPC_EXTERN_FUNC(cputest_164C0);
PPC_EXTERN_FUNC(cputest_16500);
PPC_EXTERN_FUNC(cputest_16540);
PPC_EXTERN_FUNC(cputest_16580);
PPC_EXTERN_FUNC(cputest_165C0);
PPC_EXTERN_FUNC(cputest_16600);
PPC_EXTERN_FUNC(cputest_16640);
PPC_EXTERN_FUNC(cputest_16680);
PPC_EXTERN_FUNC(cputest_166C0);
PPC_EXTERN_FUNC(cputest_16700);
PPC_EXTERN_FUNC(cputest_16740);
PPC_EXTERN_FUNC(cputest_16780);
PPC_EXTERN_FUNC(cputest_167C0);
PPC_EXTERN_FUNC(cputest_16800);
PPC_EXTERN_FUNC(cputest_16840);
PPC_EXTERN_FUNC(cputest_16880);
PPC_EXTERN_FUNC(cputest_168C0);
PPC_EXTERN_FUNC(cputest_16900);
PPC_EXTERN_FUNC(cputest_16940);
PPC_EXTERN_FUNC(cputest_16980);
PPC_EXTERN_FUNC(cputest_169C0);
PPC_EXTERN_FUNC(cputest_16A00);
PPC_EXTERN_FUNC(cputest_16A40);
PPC_EXTERN_FUNC(cputest_16A80);
PPC_EXTERN_FUNC(cputest_16AC0);
PPC_EXTERN_FUNC(cputest_16B00);
PPC_EXTERN_FUNC(cputest_16B40);
PPC_EXTERN_FUNC(cputest_16B80);
PPC_EXTERN_FUNC(cputest_16BC0);
PPC_EXTERN_FUNC(cputest_16C00);
PPC_EXTERN_FUNC(cputest_16C40);
PPC_EXTERN_FUNC(cputest_16C80);
PPC_EXTERN_FUNC(cputest_16CC0);
PPC_EXTERN_FUNC(cputest_16D00);
PPC_EXTERN_FUNC(cputest_16D40);
PPC_EXTERN_FUNC(cputest_16D80);
PPC_EXTERN_FUNC(cputest_16DC0);
PPC_EXTERN_FUNC(cputest_16E00);
PPC_EXTERN_FUNC(cputest_16E40);
PPC_EXTERN_FUNC(cputest_16E80);
PPC_EXTERN_FUNC(cputest_16EC0);
PPC_EXTERN_FUNC(cputest_16F00);
PPC_EXTERN_FUNC(cputest_16F40);
PPC_EXTERN_FUNC(cputest_16F80);
PPC_EXTERN_FUNC(cputest_16FC0);
PPC_EXTERN_FUNC(cputest_17000);
PPC_EXTERN_FUNC(cputest_17040);
PPC_EXTERN_FUNC(cputest_17080);
PPC_EXTERN_FUNC(cputest_170C0);
PPC_EXTERN_FUNC(cputest_17100);
PPC_EXTERN_FUNC(cputest_17140);
PPC_EXTERN_FUNC(cputest_17180);
PPC_EXTERN_FUNC(cputest_171C0);
PPC_EXTERN_FUNC(cputest_17200);
PPC_EXTERN_FUNC(cputest_17240);
PPC_EXTERN_FUNC(cputest_17280);
PPC_EXTERN_FUNC(cputest_172C0);
PPC_EXTERN_FUNC(cputest_17300);
PPC_EXTERN_FUNC(cputest_17340);
PPC_EXTERN_FUNC(cputest_17380);
PPC_EXTERN_FUNC(cputest_173C0);
PPC_EXTERN_FUNC(cputest_17400);
PPC_EXTERN_FUNC(cputest_17440);
PPC_EXTERN_FUNC(cputest_17480);
PPC_EXTERN_FUNC(cputest_174C0);
PPC_EXTERN_FUNC(cputest_17500);
PPC_EXTERN_FUNC(cputest_17540);
PPC_EXTERN_FUNC(cputest_17580);
PPC_EXTERN_FUNC(cputest_175C0);
PPC_EXTERN_FUNC(cputest_17600);
PPC_EXTERN_FUNC(cputest_17640);
PPC_EXTERN_FUNC(cputest_17680);
PPC_EXTERN_FUNC(cputest_176C0);
PPC_EXTERN_FUNC(cputest_17700);
PPC_EXTERN_FUNC(cputest_17740);
PPC_EXTERN_FUNC(cputest_17780);
PPC_EXTERN_FUNC(cputest_177C0);
PPC_EXTERN_FUNC(cputest_17800);
PPC_EXTERN_FUNC(cputest_17840);
PPC_EXTERN_FUNC(cputest_17880);
PPC_EXTERN_FUNC(cputest_178C0);
PPC_EXTERN_FUNC(cputest_17900);
PPC_EXTERN_FUNC(cputest_17940);
PPC_EXTERN_FUNC(cputest_17980);
PPC_EXTERN_FUNC(cputest_179C0);
PPC_EXTERN_FUNC(cputest_17A00);
PPC_EXTERN_FUNC(cputest_17A40);
PPC_EXTERN_FUNC(cputest_17A80);
PPC_EXTERN_FUNC(cputest_17AC0);
PPC_EXTERN_FUNC(cputest_17B00);
PPC_EXTERN_FUNC(cputest_17B40);
PPC_EXTERN_FUNC(cputest_17B80);
PPC_EXTERN_FUNC(cputest_17BC0);
PPC_EXTERN_FUNC(cputest_17C00);
PPC_EXTERN_FUNC(cputest_17C40);
PPC_EXTERN_FUNC(cputest_17C80);
PPC_EXTERN_FUNC(cputest_17CC0);
PPC_EXTERN_FUNC(cputest_17D00);
PPC_EXTERN_FUNC(cputest_17D40);
PPC_EXTERN_FUNC(cputest_17D80);
PPC_EXTERN_FUNC(cputest_17DC0);
PPC_EXTERN_FUNC(cputest_17E00);
PPC_EXTERN_FUNC(cputest_17E40);
PPC_EXTERN_FUNC(cputest_17E80);
PPC_EXTERN_FUNC(cputest_17EC0);
PPC_EXTERN_FUNC(cputest_17F00);
PPC_EXTERN_FUNC(cputest_17F40);
PPC_EXTERN_FUNC(cputest_17F80);
PPC_EXTERN_FUNC(cputest_17FC0);
PPC_EXTERN_FUNC(cputest_18000);
PPC_EXTERN_FUNC(cputest_18040);
PPC_EXTERN_FUNC(cputest_18080);
PPC_EXTERN_FUNC(cputest_180C0);
PPC_EXTERN_FUNC(cputest_18100);
PPC_EXTERN_FUNC(cputest_18140);
PPC_EXTERN_FUNC(cputest_18180);
PPC_EXTERN_FUNC(cputest_181C0);
PPC_EXTERN_FUNC(cputest_18200);
PPC_EXTERN_FUNC(cputest_18240);
PPC_EXTERN_FUNC(cputest_18280);
PPC_EXTERN_FUNC(cputest_182C0);
PPC_EXTERN_FUNC(cputest_18300);
PPC_EXTERN_FUNC(cputest_18340);
PPC_EXTERN_FUNC(cputest_18380);
PPC_EXTERN_FUNC(cputest_183C0);
PPC_EXTERN_FUNC(cputest_18400);
PPC_EXTERN_FUNC(cputest_18440);
PPC_EXTERN_FUNC(cputest_18480);
PPC_EXTERN_FUNC(cputest_184C0);
PPC_EXTERN_FUNC(cputest_18500);
PPC_EXTERN_FUNC(cputest_18540);
PPC_EXTERN_FUNC(cputest_18580);
PPC_EXTERN_FUNC(cputest_185C0);
PPC_EXTERN_FUNC(cputest_18600);
PPC_EXTERN_FUNC(cputest_18640);
PPC_EXTERN_FUNC(cputest_18680);
PPC_EXTERN_FUNC(cputest_186C0);
PPC_EXTERN_FUNC(cputest_18700);
PPC_EXTERN_FUNC(cputest_18740);
PPC_EXTERN_FUNC(cputest_18780);
PPC_EXTERN_FUNC(cputest_187C0);
PPC_EXTERN_FUNC(cputest_18800);
PPC_EXTERN_FUNC(cputest_18840);
PPC_EXTERN_FUNC(cputest_18880);
PPC_EXTERN_FUNC(cputest_188C0);
PPC_EXTERN_FUNC(cputest_18900);
PPC_EXTERN_FUNC(cputest_18940);
PPC_EXTERN_FUNC(cputest_18980);
PPC_EXTERN_FUNC(cputest_189C0);
PPC_EXTERN_FUNC(cputest_18A00);
PPC_EXTERN_FUNC(cputest_18A40);
PPC_EXTERN_FUNC(cputest_18A80);
PPC_EXTERN_FUNC(cputest_18AC0);
PPC_EXTERN_FUNC(cputest_18B00);
PPC_EXTERN_FUNC(cputest_18B40);
PPC_EXTERN_FUNC(cputest_18B80);
PPC_EXTERN_FUNC(cputest_18BC0);
PPC_EXTERN_FUNC(cputest_18C00);
PPC_EXTERN_FUNC(cputest_18C40);
PPC_EXTERN_FUNC(cputest_18C80);
PPC_EXTERN_FUNC(cputest_18CC0);
PPC_EXTERN_FUNC(cputest_18D00);
PPC_EXTERN_FUNC(cputest_18D40);
PPC_EXTERN_FUNC(cputest_18D80);
PPC_EXTERN_FUNC(cputest_18DC0);
PPC_EXTERN_FUNC(cputest_18E00);
PPC_EXTERN_FUNC(cputest_18E40);
PPC_EXTERN_FUNC(cputest_18E80);
PPC_EXTERN_FUNC(cputest_18EC0);
PPC_EXTERN_FUNC(cputest_18F00);
PPC_EXTERN_FUNC(cputest_18F40);
PPC_EXTERN_FUNC(cputest_18F80);
PPC_EXTERN_FUNC(cputest_18FC0);
PPC_EXTERN_FUNC(cputest_19000);
PPC_EXTERN_FUNC(cputest_19040);
PPC_EXTERN_FUNC(cputest_19080);
PPC_EXTERN_FUNC(cputest_190C0);
PPC_EXTERN_FUNC(cputest_19100);
PPC_EXTERN_FUNC(cputest_19140);
PPC_EXTERN_FUNC(cputest_19180);
PPC_EXTERN_FUNC(cputest_191C0);
PPC_EXTERN_FUNC(cputest_19200);
PPC_EXTERN_FUNC(cputest_19240);
PPC_EXTERN_FUNC(cputest_19280);
PPC_EXTERN_FUNC(cputest_192C0);
PPC_EXTERN_FUNC(cputest_19300);
PPC_EXTERN_FUNC(cputest_19340);
PPC_EXTERN_FUNC(cputest_19380);
PPC_EXTERN_FUNC(cputest_193C0);
PPC_EXTERN_FUNC(cputest_19400);
PPC_EXTERN_FUNC(cputest_19440);
PPC_EXTERN_FUNC(cputest_19480);
PPC_EXTERN_FUNC(cputest_194C0);
PPC_EXTERN_FUNC(cputest_19500);
PPC_EXTERN_FUNC(cputest_19540);
PPC_EXTERN_FUNC(cputest_19580);
PPC_EXTERN_FUNC(cputest_195C0);
PPC_EXTERN_FUNC(cputest_19600);
PPC_EXTERN_FUNC(cputest_19640);
PPC_EXTERN_FUNC(cputest_19680);
PPC_EXTERN_FUNC(cputest_196C0);
PPC_EXTERN_FUNC(cputest_19700);
PPC_EXTERN_FUNC(cputest_19740);
PPC_EXTERN_FUNC(cputest_19780);
PPC_EXTERN_FUNC(cputest_197C0);
PPC_EXTERN_FUNC(cputest_19800);
PPC_EXTERN_FUNC(cputest_19840);
PPC_EXTERN_FUNC(cputest_19880);
PPC_EXTERN_FUNC(cputest_198C0);
PPC_EXTERN_FUNC(cputest_19900);
PPC_EXTERN_FUNC(cputest_19940);
PPC_EXTERN_FUNC(cputest_19980);
PPC_EXTERN_FUNC(cputest_199C0);
PPC_EXTERN_FUNC(cputest_19A00);
PPC_EXTERN_FUNC(cputest_19A40);
PPC_EXTERN_FUNC(cputest_19A80);
PPC_EXTERN_FUNC(cputest_19AC0);
PPC_EXTERN_FUNC(cputest_19B00);
PPC_EXTERN_FUNC(cputest_19B40);
PPC_EXTERN_FUNC(cputest_19B80);
PPC_EXTERN_FUNC(cputest_19BC0);
PPC_EXTERN_FUNC(cputest_19C00);
PPC_EXTERN_FUNC(cputest_19C40);
PPC_EXTERN_FUNC(cputest_19C80);
PPC_EXTERN_FUNC(cputest_19CC0);
PPC_EXTERN_FUNC(cputest_19D00);
PPC_EXTERN_FUNC(cputest_19D40);
PPC_EXTERN_FUNC(cputest_19D80);
PPC_EXTERN_FUNC(cputest_19DC0);
PPC_EXTERN_FUNC(cputest_19E00);
PPC_EXTERN_FUNC(cputest_19E40);
PPC_EXTERN_FUNC(cputest_19E80);
PPC_EXTERN_FUNC(cputest_19EC0);
PPC_EXTERN_FUNC(cputest_19F00);
PPC_EXTERN_FUNC(cputest_19F40);
PPC_EXTERN_FUNC(cputest_19F80);
PPC_EXTERN_FUNC(cputest_19FC0);
PPC_EXTERN_FUNC(cputest_1A000);
PPC_EXTERN_FUNC(cputest_1A040);
PPC_EXTERN_FUNC(cputest_1A080);
PPC_EXTERN_FUNC(cputest_1A0C0);
PPC_EXTERN_FUNC(cputest_1A100);
PPC_EXTERN_FUNC(cputest_1A140);
PPC_EXTERN_FUNC(cputest_1A180);
PPC_EXTERN_FUNC(cputest_1A1C0);
PPC_EXTERN_FUNC(cputest_1A200);
PPC_EXTERN_FUNC(cputest_1A240);
PPC_EXTERN_FUNC(cputest_1A280);
PPC_EXTERN_FUNC(cputest_1A2C0);
PPC_EXTERN_FUNC(cputest_1A300);
PPC_EXTERN_FUNC(cputest_1A340);
PPC_EXTERN_FUNC(cputest_1A380);
PPC_EXTERN_FUNC(cputest_1A3C0);
PPC_EXTERN_FUNC(cputest_1A400);
PPC_EXTERN_FUNC(cputest_1A440);
PPC_EXTERN_FUNC(cputest_1A480);
PPC_EXTERN_FUNC(cputest_1A4C0);
PPC_EXTERN_FUNC(cputest_1A500);
PPC_EXTERN_FUNC(cputest_1A540);
PPC_EXTERN_FUNC(cputest_1A580);
PPC_EXTERN_FUNC(cputest_1A5C0);
PPC_EXTERN_FUNC(cputest_1A600);
PPC_EXTERN_FUNC(cputest_1A640);
PPC_EXTERN_FUNC(cputest_1A680);
PPC_EXTERN_FUNC(cputest_1A6C0);
PPC_EXTERN_FUNC(cputest_1A700);
PPC_EXTERN_FUNC(cputest_1A740);
PPC_EXTERN_FUNC(cputest_1A780);
PPC_EXTERN_FUNC(cputest_1A7C0);
PPC_EXTERN_FUNC(cputest_1A800);
PPC_EXTERN_FUNC(cputest_1A840);
PPC_EXTERN_FUNC(cputest_1A880);
PPC_EXTERN_FUNC(cputest_1A8C0);
PPC_EXTERN_FUNC(cputest_1A900);
PPC_EXTERN_FUNC(cputest_1A940);
PPC_EXTERN_FUNC(cputest_1A980);
PPC_EXTERN_FUNC(cputest_1A9C0);
PPC_EXTERN_FUNC(cputest_1AA00);
PPC_EXTERN_FUNC(cputest_1AA40);
PPC_EXTERN_FUNC(cputest_1AA80);
PPC_EXTERN_FUNC(cputest_1AAC0);
PPC_EXTERN_FUNC(cputest_1AB00);
PPC_EXTERN_FUNC(cputest_1AB40);
PPC_EXTERN_FUNC(cputest_1AB80);
PPC_EXTERN_FUNC(cputest_1ABC0);
PPC_EXTERN_FUNC(cputest_1AC00);
PPC_EXTERN_FUNC(cputest_1AC40);

const CpuTest g_cpu_tests[] = {
    { "addc", "addc r3,r4,r5", 0x7C642814U, 0x10000U, cputest_10000, 1 },
    { "mulhdu", "mulhdu r3,r4,r5", 0x7C642812U, 0x10040U, cputest_10040, 1 },
    { "eqv", "eqv r4,r3,r5", 0x7C642A38U, 0x10080U, cputest_10080, 1 },
    { "addc", "addc. r3,r4,r5", 0x7C642815U, 0x100C0U, cputest_100C0, 1 },
    { "mulhdu", "mulhdu. r3,r4,r5", 0x7C642813U, 0x10100U, cputest_10100, 1 },
    { "eqv", "eqv. r4,r3,r5", 0x7C642A39U, 0x10140U, cputest_10140, 1 },
    { "addc", "addc r4,r4,r5", 0x7C842814U, 0x10180U, cputest_10180, 1 },
    { "mulhdu", "mulhdu r4,r4,r5", 0x7C842812U, 0x101C0U, cputest_101C0, 1 },
    { "eqv", "eqv r4,r4,r5", 0x7C842A38U, 0x10200U, cputest_10200, 1 },
    { "addc", "addc. r4,r4,r5", 0x7C842815U, 0x10240U, cputest_10240, 1 },
    { "mulhdu", "mulhdu. r4,r4,r5", 0x7C842813U, 0x10280U, cputest_10280, 1 },
    { "eqv", "eqv. r4,r4,r5", 0x7C842A39U, 0x102C0U, cputest_102C0, 1 },
    { "addc", "addc r5,r4,r5", 0x7CA42814U, 0x10300U, cputest_10300, 1 },
    { "mulhdu", "mulhdu r5,r4,r5", 0x7CA42812U, 0x10340U, cputest_10340, 1 },
    { "eqv", "eqv r4,r5,r5", 0x7CA42A38U, 0x10380U, cputest_10380, 1 },
    { "addc", "addc. r5,r4,r5", 0x7CA42815U, 0x103C0U, cputest_103C0, 1 },
    { "mulhdu", "mulhdu. r5,r4,r5", 0x7CA42813U, 0x10400U, cputest_10400, 1 },
    { "eqv", "eqv. r4,r5,r5", 0x7CA42A39U, 0x10440U, cputest_10440, 1 },
    { "addc", "addc r7,r7,r7", 0x7CE73814U, 0x10480U, cputest_10480, 1 },
    { "mulhdu", "mulhdu r7,r7,r7", 0x7CE73812U, 0x104C0U, cputest_104C0, 1 },
    { "eqv", "eqv r7,r7,r7", 0x7CE73A38U, 0x10500U, cputest_10500, 1 },
    { "addc", "addc. r7,r7,r7", 0x7CE73815U, 0x10540U, cputest_10540, 1 },
    { "mulhdu", "mulhdu. r7,r7,r7", 0x7CE73813U, 0x10580U, cputest_10580, 1 },
    { "eqv", "eqv. r7,r7,r7", 0x7CE73A39U, 0x105C0U, cputest_105C0, 1 },
    { "addc", "addc r31,r30,r29", 0x7FFEE814U, 0x10600U, cputest_10600, 1 },
    { "mulhdu", "mulhdu r31,r30,r29", 0x7FFEE812U, 0x10640U, cputest_10640, 1 },
    { "eqv", "eqv r30,r31,r29", 0x7FFEEA38U, 0x10680U, cputest_10680, 1 },
    { "addc", "addc. r31,r30,r29", 0x7FFEE815U, 0x106C0U, cputest_106C0, 1 },
    { "mulhdu", "mulhdu. r31,r30,r29", 0x7FFEE813U, 0x10700U, cputest_10700, 1 },
    { "eqv", "eqv. r30,r31,r29", 0x7FFEEA39U, 0x10740U, cputest_10740, 1 },
    { "addc", "addc r0,r1,r2", 0x7C011014U, 0x10780U, cputest_10780, 1 },
    { "mulhdu", "mulhdu r0,r1,r2", 0x7C011012U, 0x107C0U, cputest_107C0, 1 },
    { "eqv", "eqv r1,r0,r2", 0x7C011238U, 0x10800U, cputest_10800, 1 },
    { "addc", "addc. r0,r1,r2", 0x7C011015U, 0x10840U, cputest_10840, 1 },
    { "mulhdu", "mulhdu. r0,r1,r2", 0x7C011013U, 0x10880U, cputest_10880, 1 },
    { "eqv", "eqv. r1,r0,r2", 0x7C011239U, 0x108C0U, cputest_108C0, 1 },
    { "addme", "addme r3,r4", 0x7C6401D4U, 0x10900U, cputest_10900, 1 },
    { "subfze", "subfze r3,r4", 0x7C640190U, 0x10940U, cputest_10940, 1 },
    { "addme", "addme. r3,r4", 0x7C6401D5U, 0x10980U, cputest_10980, 1 },
    { "subfze", "subfze. r3,r4", 0x7C640191U, 0x109C0U, cputest_109C0, 1 },
    { "addme", "addme r4,r4", 0x7C8401D4U, 0x10A00U, cputest_10A00, 1 },
    { "subfze", "subfze r4,r4", 0x7C840190U, 0x10A40U, cputest_10A40, 1 },
    { "addme", "addme. r4,r4", 0x7C8401D5U, 0x10A80U, cputest_10A80, 1 },
    { "subfze", "subfze. r4,r4", 0x7C840191U, 0x10AC0U, cputest_10AC0, 1 },
    { "addme", "addme r31,r30", 0x7FFE01D4U, 0x10B00U, cputest_10B00, 1 },
    { "subfze", "subfze r31,r30", 0x7FFE0190U, 0x10B40U, cputest_10B40, 1 },
    { "addme", "addme. r31,r30", 0x7FFE01D5U, 0x10B80U, cputest_10B80, 1 },
    { "subfze", "subfze. r31,r30", 0x7FFE0191U, 0x10BC0U, cputest_10BC0, 1 },
    { "addme", "addme r0,r1", 0x7C0101D4U, 0x10C00U, cputest_10C00, 1 },
    { "subfze", "subfze r0,r1", 0x7C010190U, 0x10C40U, cputest_10C40, 1 },
    { "addme", "addme. r0,r1", 0x7C0101D5U, 0x10C80U, cputest_10C80, 1 },
    { "subfze", "subfze. r0,r1", 0x7C010191U, 0x10CC0U, cputest_10CC0, 1 },
    { "lhzu", "lhzu r3,16(r4)", 0xA4640010U, 0x10D00U, cputest_10D00, 1 },
    { "lhau", "lhau r3,16(r4)", 0xAC640010U, 0x10D40U, cputest_10D40, 1 },
    { "sthu", "sthu r3,16(r4)", 0xB4640010U, 0x10D80U, cputest_10D80, 1 },
    { "lfsu", "lfsu f3,16(r4)", 0xC4640010U, 0x10DC0U, cputest_10DC0, 1 },
    { "lfdu", "lfdu f3,16(r4)", 0xCC640010U, 0x10E00U, cputest_10E00, 1 },
    { "stfsu", "stfsu f3,16(r4)", 0xD4640010U, 0x10E40U, cputest_10E40, 1 },
    { "stfdu", "stfdu f3,16(r4)", 0xDC640010U, 0x10E80U, cputest_10E80, 1 },
    { "lhzu", "lhzu r31,-32(r30)", 0xA7FEFFE0U, 0x10EC0U, cputest_10EC0, 1 },
    { "lhau", "lhau r31,-32(r30)", 0xAFFEFFE0U, 0x10F00U, cputest_10F00, 1 },
    { "sthu", "sthu r31,-32(r30)", 0xB7FEFFE0U, 0x10F40U, cputest_10F40, 1 },
    { "lfsu", "lfsu f31,-32(r30)", 0xC7FEFFE0U, 0x10F80U, cputest_10F80, 1 },
    { "lfdu", "lfdu f31,-32(r30)", 0xCFFEFFE0U, 0x10FC0U, cputest_10FC0, 1 },
    { "stfsu", "stfsu f31,-32(r30)", 0xD7FEFFE0U, 0x11000U, cputest_11000, 1 },
    { "stfdu", "stfdu f31,-32(r30)", 0xDFFEFFE0U, 0x11040U, cputest_11040, 1 },
    { "lhzu", "lhzu r10,510(r5)", 0xA54501FEU, 0x11080U, cputest_11080, 1 },
    { "lhau", "lhau r10,510(r5)", 0xAD4501FEU, 0x110C0U, cputest_110C0, 1 },
    { "sthu", "sthu r10,510(r5)", 0xB54501FEU, 0x11100U, cputest_11100, 1 },
    { "lfsu", "lfsu f10,510(r5)", 0xC54501FEU, 0x11140U, cputest_11140, 1 },
    { "lfdu", "lfdu f10,510(r5)", 0xCD4501F8U, 0x11180U, cputest_11180, 1 },
    { "stfsu", "stfsu f10,510(r5)", 0xD54501FEU, 0x111C0U, cputest_111C0, 1 },
    { "stfdu", "stfdu f10,510(r5)", 0xDD4501F8U, 0x11200U, cputest_11200, 1 },
    { "lhzu", "lhzu r0,-2(r9)", 0xA409FFFEU, 0x11240U, cputest_11240, 1 },
    { "lhau", "lhau r0,-2(r9)", 0xAC09FFFEU, 0x11280U, cputest_11280, 1 },
    { "sthu", "sthu r0,-2(r9)", 0xB409FFFEU, 0x112C0U, cputest_112C0, 1 },
    { "lfsu", "lfsu f0,-2(r9)", 0xC409FFFEU, 0x11300U, cputest_11300, 1 },
    { "lfdu", "lfdu f0,-2(r9)", 0xCC09FFF8U, 0x11340U, cputest_11340, 1 },
    { "stfsu", "stfsu f0,-2(r9)", 0xD409FFFEU, 0x11380U, cputest_11380, 1 },
    { "stfdu", "stfdu f0,-2(r9)", 0xDC09FFF8U, 0x113C0U, cputest_113C0, 1 },
    { "lbzux", "lbzux r3,r4,r5", 0x7C6428EEU, 0x11400U, cputest_11400, 1 },
    { "lhzux", "lhzux r3,r4,r5", 0x7C642A6EU, 0x11440U, cputest_11440, 1 },
    { "lwzux", "lwzux r3,r4,r5", 0x7C64286EU, 0x11480U, cputest_11480, 1 },
    { "ldux", "ldux r3,r4,r5", 0x7C64286AU, 0x114C0U, cputest_114C0, 1 },
    { "stbux", "stbux r3,r4,r5", 0x7C6429EEU, 0x11500U, cputest_11500, 1 },
    { "sthux", "sthux r3,r4,r5", 0x7C642B6EU, 0x11540U, cputest_11540, 1 },
    { "stdux", "stdux r3,r4,r5", 0x7C64296AU, 0x11580U, cputest_11580, 1 },
    { "lfsux", "lfsux f3,r4,r5", 0x7C642C6EU, 0x115C0U, cputest_115C0, 1 },
    { "stfsux", "stfsux f3,r4,r5", 0x7C642D6EU, 0x11600U, cputest_11600, 1 },
    { "lbzux", "lbzux r31,r30,r29", 0x7FFEE8EEU, 0x11640U, cputest_11640, 1 },
    { "lhzux", "lhzux r31,r30,r29", 0x7FFEEA6EU, 0x11680U, cputest_11680, 1 },
    { "lwzux", "lwzux r31,r30,r29", 0x7FFEE86EU, 0x116C0U, cputest_116C0, 1 },
    { "ldux", "ldux r31,r30,r29", 0x7FFEE86AU, 0x11700U, cputest_11700, 1 },
    { "stbux", "stbux r31,r30,r29", 0x7FFEE9EEU, 0x11740U, cputest_11740, 1 },
    { "sthux", "sthux r31,r30,r29", 0x7FFEEB6EU, 0x11780U, cputest_11780, 1 },
    { "stdux", "stdux r31,r30,r29", 0x7FFEE96AU, 0x117C0U, cputest_117C0, 1 },
    { "lfsux", "lfsux f31,r30,r29", 0x7FFEEC6EU, 0x11800U, cputest_11800, 1 },
    { "stfsux", "stfsux f31,r30,r29", 0x7FFEED6EU, 0x11840U, cputest_11840, 1 },
    { "lbzux", "lbzux r10,r5,r6", 0x7D4530EEU, 0x11880U, cputest_11880, 1 },
    { "lhzux", "lhzux r10,r5,r6", 0x7D45326EU, 0x118C0U, cputest_118C0, 1 },
    { "lwzux", "lwzux r10,r5,r6", 0x7D45306EU, 0x11900U, cputest_11900, 1 },
    { "ldux", "ldux r10,r5,r6", 0x7D45306AU, 0x11940U, cputest_11940, 1 },
    { "stbux", "stbux r10,r5,r6", 0x7D4531EEU, 0x11980U, cputest_11980, 1 },
    { "sthux", "sthux r10,r5,r6", 0x7D45336EU, 0x119C0U, cputest_119C0, 1 },
    { "stdux", "stdux r10,r5,r6", 0x7D45316AU, 0x11A00U, cputest_11A00, 1 },
    { "lfsux", "lfsux f10,r5,r6", 0x7D45346EU, 0x11A40U, cputest_11A40, 1 },
    { "stfsux", "stfsux f10,r5,r6", 0x7D45356EU, 0x11A80U, cputest_11A80, 1 },
    { "lbzux", "lbzux r0,r9,r8", 0x7C0940EEU, 0x11AC0U, cputest_11AC0, 1 },
    { "lhzux", "lhzux r0,r9,r8", 0x7C09426EU, 0x11B00U, cputest_11B00, 1 },
    { "lwzux", "lwzux r0,r9,r8", 0x7C09406EU, 0x11B40U, cputest_11B40, 1 },
    { "ldux", "ldux r0,r9,r8", 0x7C09406AU, 0x11B80U, cputest_11B80, 1 },
    { "stbux", "stbux r0,r9,r8", 0x7C0941EEU, 0x11BC0U, cputest_11BC0, 1 },
    { "sthux", "sthux r0,r9,r8", 0x7C09436EU, 0x11C00U, cputest_11C00, 1 },
    { "stdux", "stdux r0,r9,r8", 0x7C09416AU, 0x11C40U, cputest_11C40, 1 },
    { "lfsux", "lfsux f0,r9,r8", 0x7C09446EU, 0x11C80U, cputest_11C80, 1 },
    { "stfsux", "stfsux f0,r9,r8", 0x7C09456EU, 0x11CC0U, cputest_11CC0, 1 },
    { "lhbrx", "lhbrx r3,r4,r5", 0x7C642E2CU, 0x11D00U, cputest_11D00, 1 },
    { "lhbrx", "lhbrx r3,r0,r5", 0x7C602E2CU, 0x11D40U, cputest_11D40, 1 },
    { "lhbrx", "lhbrx r31,r30,r29", 0x7FFEEE2CU, 0x11D80U, cputest_11D80, 1 },
    { "lhbrx", "lhbrx r6,r6,r7", 0x7CC63E2CU, 0x11DC0U, cputest_11DC0, 1 },
    { "lvehx", "lvehx v3,r4,r5", 0x7C64284EU, 0x11E00U, cputest_11E00, 1 },
    { "lvehx", "lvehx v3,r0,r5", 0x7C60284EU, 0x11E40U, cputest_11E40, 1 },
    { "lvehx", "lvehx v31,r30,r29", 0x7FFEE84EU, 0x11E80U, cputest_11E80, 1 },
    { "dcbst", "dcbst r4,r5", 0x7C04286CU, 0x11EC0U, cputest_11EC0, 1 },
    { "dcbst", "dcbst r0,r5", 0x7C00286CU, 0x11F00U, cputest_11F00, 1 },
    { "dcbst", "dcbst r30,r29", 0x7C1EE86CU, 0x11F40U, cputest_11F40, 1 },
    { "frsqrte", "frsqrte f1,f2", 0xFC201034U, 0x11F80U, cputest_11F80, 1 },
    { "frsqrte", "frsqrte f5,f5", 0xFCA02834U, 0x11FC0U, cputest_11FC0, 1 },
    { "frsqrte", "frsqrte f31,f30", 0xFFE0F034U, 0x12000U, cputest_12000, 1 },
    { "frsqrte", "frsqrte f0,f13", 0xFC006834U, 0x12040U, cputest_12040, 1 },
    { "bdzf", "bdzf 0,+8", 0x40400008U, 0x12080U, cputest_12080, 2 },
    { "bdzf", "bdzf 2,+8", 0x40420008U, 0x120C0U, cputest_120C0, 2 },
    { "bdzf", "bdzf 3,+8", 0x40430008U, 0x12100U, cputest_12100, 2 },
    { "bdzf", "bdzf 6,+8", 0x40460008U, 0x12140U, cputest_12140, 2 },
    { "bdzf", "bdzf 9,+8", 0x40490008U, 0x12180U, cputest_12180, 2 },
    { "bdzf", "bdzf 17,+8", 0x40510008U, 0x121C0U, cputest_121C0, 2 },
    { "bdzf", "bdzf 22,+8", 0x40560008U, 0x12200U, cputest_12200, 2 },
    { "bdzf", "bdzf 31,+8", 0x405F0008U, 0x12240U, cputest_12240, 2 },
    { "bdnzt", "bdnzt 0,+8", 0x41000008U, 0x12280U, cputest_12280, 2 },
    { "bdnzt", "bdnzt 2,+8", 0x41020008U, 0x122C0U, cputest_122C0, 2 },
    { "bdnzt", "bdnzt 3,+8", 0x41030008U, 0x12300U, cputest_12300, 2 },
    { "bdnzt", "bdnzt 6,+8", 0x41060008U, 0x12340U, cputest_12340, 2 },
    { "bdnzt", "bdnzt 9,+8", 0x41090008U, 0x12380U, cputest_12380, 2 },
    { "bdnzt", "bdnzt 17,+8", 0x41110008U, 0x123C0U, cputest_123C0, 2 },
    { "bdnzt", "bdnzt 22,+8", 0x41160008U, 0x12400U, cputest_12400, 2 },
    { "bdnzt", "bdnzt 31,+8", 0x411F0008U, 0x12440U, cputest_12440, 2 },
    { "blrl", "blrl", 0x4E800021U, 0x12480U, cputest_12480, 1 },
    { "vnor", "vnor v3,v4,v5", 0x10642D04U, 0x124C0U, cputest_124C0, 1 },
    { "vandc", "vandc v3,v4,v5", 0x10642C44U, 0x12500U, cputest_12500, 1 },
    { "vslh", "vslh v3,v4,v5", 0x10642944U, 0x12540U, cputest_12540, 1 },
    { "vsrh", "vsrh v3,v4,v5", 0x10642A44U, 0x12580U, cputest_12580, 1 },
    { "vsrah", "vsrah v3,v4,v5", 0x10642B44U, 0x125C0U, cputest_125C0, 1 },
    { "vsro", "vsro v3,v4,v5", 0x10642C4CU, 0x12600U, cputest_12600, 1 },
    { "vminsw", "vminsw v3,v4,v5", 0x10642B82U, 0x12640U, cputest_12640, 1 },
    { "vminuh", "vminuh v3,v4,v5", 0x10642A42U, 0x12680U, cputest_12680, 1 },
    { "vmaxuh", "vmaxuh v3,v4,v5", 0x10642842U, 0x126C0U, cputest_126C0, 1 },
    { "vavguh", "vavguh v3,v4,v5", 0x10642C42U, 0x12700U, cputest_12700, 1 },
    { "vsubuwm", "vsubuwm v3,v4,v5", 0x10642C80U, 0x12740U, cputest_12740, 1 },
    { "vsububm", "vsububm v3,v4,v5", 0x10642C00U, 0x12780U, cputest_12780, 1 },
    { "vsubshs", "vsubshs v3,v4,v5", 0x10642F40U, 0x127C0U, cputest_127C0, 1 },
    { "vaddsws", "vaddsws v3,v4,v5", 0x10642B80U, 0x12800U, cputest_12800, 1 },
    { "vpkswss", "vpkswss v3,v4,v5", 0x106429CEU, 0x12840U, cputest_12840, 1 },
    { "vcmpgtsb", "vcmpgtsb v3,v4,v5", 0x10642B06U, 0x12880U, cputest_12880, 1 },
    { "vcmpgtsb.", "vcmpgtsb. v3,v4,v5", 0x10642F06U, 0x128C0U, cputest_128C0, 1 },
    { "vcmpgtsw", "vcmpgtsw v3,v4,v5", 0x10642B86U, 0x12900U, cputest_12900, 1 },
    { "vcmpgtsw.", "vcmpgtsw. v3,v4,v5", 0x10642F86U, 0x12940U, cputest_12940, 1 },
    { "vcmpgtuw", "vcmpgtuw v3,v4,v5", 0x10642A86U, 0x12980U, cputest_12980, 1 },
    { "vcmpgtuw.", "vcmpgtuw. v3,v4,v5", 0x10642E86U, 0x129C0U, cputest_129C0, 1 },
    { "vspltish", "vspltish v3,0", 0x1060034CU, 0x12A00U, cputest_12A00, 1 },
    { "vspltish", "vspltish v3,1", 0x1061034CU, 0x12A40U, cputest_12A40, 1 },
    { "vspltish", "vspltish v3,15", 0x106F034CU, 0x12A80U, cputest_12A80, 1 },
    { "vspltish", "vspltish v3,-16", 0x1070034CU, 0x12AC0U, cputest_12AC0, 1 },
    { "vspltish", "vspltish v3,-1", 0x107F034CU, 0x12B00U, cputest_12B00, 1 },
    { "vctuxs", "vctuxs v3,v5,0", 0x10602B8AU, 0x12B40U, cputest_12B40, 1 },
    { "vctuxs", "vctuxs v3,v5,1", 0x10612B8AU, 0x12B80U, cputest_12B80, 1 },
    { "vctuxs", "vctuxs v3,v5,8", 0x10682B8AU, 0x12BC0U, cputest_12BC0, 1 },
    { "vctuxs", "vctuxs v3,v5,31", 0x107F2B8AU, 0x12C00U, cputest_12C00, 1 },
    { "vsel", "vsel v3,v4,v5,v4", 0x1064292AU, 0x12C40U, cputest_12C40, 1 },
    { "vsldoi", "vsldoi v3,v4,v5,0", 0x1064282CU, 0x12C80U, cputest_12C80, 1 },
    { "vsldoi", "vsldoi v3,v4,v5,1", 0x1064286CU, 0x12CC0U, cputest_12CC0, 1 },
    { "vsldoi", "vsldoi v3,v4,v5,5", 0x1064296CU, 0x12D00U, cputest_12D00, 1 },
    { "vsldoi", "vsldoi v3,v4,v5,15", 0x10642BECU, 0x12D40U, cputest_12D40, 1 },
    { "vnor", "vnor v3,v3,v5", 0x10632D04U, 0x12D80U, cputest_12D80, 1 },
    { "vandc", "vandc v3,v3,v5", 0x10632C44U, 0x12DC0U, cputest_12DC0, 1 },
    { "vslh", "vslh v3,v3,v5", 0x10632944U, 0x12E00U, cputest_12E00, 1 },
    { "vsrh", "vsrh v3,v3,v5", 0x10632A44U, 0x12E40U, cputest_12E40, 1 },
    { "vsrah", "vsrah v3,v3,v5", 0x10632B44U, 0x12E80U, cputest_12E80, 1 },
    { "vsro", "vsro v3,v3,v5", 0x10632C4CU, 0x12EC0U, cputest_12EC0, 1 },
    { "vminsw", "vminsw v3,v3,v5", 0x10632B82U, 0x12F00U, cputest_12F00, 1 },
    { "vminuh", "vminuh v3,v3,v5", 0x10632A42U, 0x12F40U, cputest_12F40, 1 },
    { "vmaxuh", "vmaxuh v3,v3,v5", 0x10632842U, 0x12F80U, cputest_12F80, 1 },
    { "vavguh", "vavguh v3,v3,v5", 0x10632C42U, 0x12FC0U, cputest_12FC0, 1 },
    { "vsubuwm", "vsubuwm v3,v3,v5", 0x10632C80U, 0x13000U, cputest_13000, 1 },
    { "vsububm", "vsububm v3,v3,v5", 0x10632C00U, 0x13040U, cputest_13040, 1 },
    { "vsubshs", "vsubshs v3,v3,v5", 0x10632F40U, 0x13080U, cputest_13080, 1 },
    { "vaddsws", "vaddsws v3,v3,v5", 0x10632B80U, 0x130C0U, cputest_130C0, 1 },
    { "vpkswss", "vpkswss v3,v3,v5", 0x106329CEU, 0x13100U, cputest_13100, 1 },
    { "vcmpgtsb", "vcmpgtsb v3,v3,v5", 0x10632B06U, 0x13140U, cputest_13140, 1 },
    { "vcmpgtsb.", "vcmpgtsb. v3,v3,v5", 0x10632F06U, 0x13180U, cputest_13180, 1 },
    { "vcmpgtsw", "vcmpgtsw v3,v3,v5", 0x10632B86U, 0x131C0U, cputest_131C0, 1 },
    { "vcmpgtsw.", "vcmpgtsw. v3,v3,v5", 0x10632F86U, 0x13200U, cputest_13200, 1 },
    { "vcmpgtuw", "vcmpgtuw v3,v3,v5", 0x10632A86U, 0x13240U, cputest_13240, 1 },
    { "vcmpgtuw.", "vcmpgtuw. v3,v3,v5", 0x10632E86U, 0x13280U, cputest_13280, 1 },
    { "vspltish", "vspltish v3,0", 0x1060034CU, 0x132C0U, cputest_132C0, 1 },
    { "vspltish", "vspltish v3,1", 0x1061034CU, 0x13300U, cputest_13300, 1 },
    { "vspltish", "vspltish v3,15", 0x106F034CU, 0x13340U, cputest_13340, 1 },
    { "vspltish", "vspltish v3,-16", 0x1070034CU, 0x13380U, cputest_13380, 1 },
    { "vspltish", "vspltish v3,-1", 0x107F034CU, 0x133C0U, cputest_133C0, 1 },
    { "vctuxs", "vctuxs v3,v5,0", 0x10602B8AU, 0x13400U, cputest_13400, 1 },
    { "vctuxs", "vctuxs v3,v5,1", 0x10612B8AU, 0x13440U, cputest_13440, 1 },
    { "vctuxs", "vctuxs v3,v5,8", 0x10682B8AU, 0x13480U, cputest_13480, 1 },
    { "vctuxs", "vctuxs v3,v5,31", 0x107F2B8AU, 0x134C0U, cputest_134C0, 1 },
    { "vsel", "vsel v3,v3,v5,v4", 0x1063292AU, 0x13500U, cputest_13500, 1 },
    { "vsldoi", "vsldoi v3,v3,v5,0", 0x1063282CU, 0x13540U, cputest_13540, 1 },
    { "vsldoi", "vsldoi v3,v3,v5,1", 0x1063286CU, 0x13580U, cputest_13580, 1 },
    { "vsldoi", "vsldoi v3,v3,v5,5", 0x1063296CU, 0x135C0U, cputest_135C0, 1 },
    { "vsldoi", "vsldoi v3,v3,v5,15", 0x10632BECU, 0x13600U, cputest_13600, 1 },
    { "vnor", "vnor v3,v4,v3", 0x10641D04U, 0x13640U, cputest_13640, 1 },
    { "vandc", "vandc v3,v4,v3", 0x10641C44U, 0x13680U, cputest_13680, 1 },
    { "vslh", "vslh v3,v4,v3", 0x10641944U, 0x136C0U, cputest_136C0, 1 },
    { "vsrh", "vsrh v3,v4,v3", 0x10641A44U, 0x13700U, cputest_13700, 1 },
    { "vsrah", "vsrah v3,v4,v3", 0x10641B44U, 0x13740U, cputest_13740, 1 },
    { "vsro", "vsro v3,v4,v3", 0x10641C4CU, 0x13780U, cputest_13780, 1 },
    { "vminsw", "vminsw v3,v4,v3", 0x10641B82U, 0x137C0U, cputest_137C0, 1 },
    { "vminuh", "vminuh v3,v4,v3", 0x10641A42U, 0x13800U, cputest_13800, 1 },
    { "vmaxuh", "vmaxuh v3,v4,v3", 0x10641842U, 0x13840U, cputest_13840, 1 },
    { "vavguh", "vavguh v3,v4,v3", 0x10641C42U, 0x13880U, cputest_13880, 1 },
    { "vsubuwm", "vsubuwm v3,v4,v3", 0x10641C80U, 0x138C0U, cputest_138C0, 1 },
    { "vsububm", "vsububm v3,v4,v3", 0x10641C00U, 0x13900U, cputest_13900, 1 },
    { "vsubshs", "vsubshs v3,v4,v3", 0x10641F40U, 0x13940U, cputest_13940, 1 },
    { "vaddsws", "vaddsws v3,v4,v3", 0x10641B80U, 0x13980U, cputest_13980, 1 },
    { "vpkswss", "vpkswss v3,v4,v3", 0x106419CEU, 0x139C0U, cputest_139C0, 1 },
    { "vcmpgtsb", "vcmpgtsb v3,v4,v3", 0x10641B06U, 0x13A00U, cputest_13A00, 1 },
    { "vcmpgtsb.", "vcmpgtsb. v3,v4,v3", 0x10641F06U, 0x13A40U, cputest_13A40, 1 },
    { "vcmpgtsw", "vcmpgtsw v3,v4,v3", 0x10641B86U, 0x13A80U, cputest_13A80, 1 },
    { "vcmpgtsw.", "vcmpgtsw. v3,v4,v3", 0x10641F86U, 0x13AC0U, cputest_13AC0, 1 },
    { "vcmpgtuw", "vcmpgtuw v3,v4,v3", 0x10641A86U, 0x13B00U, cputest_13B00, 1 },
    { "vcmpgtuw.", "vcmpgtuw. v3,v4,v3", 0x10641E86U, 0x13B40U, cputest_13B40, 1 },
    { "vspltish", "vspltish v3,0", 0x1060034CU, 0x13B80U, cputest_13B80, 1 },
    { "vspltish", "vspltish v3,1", 0x1061034CU, 0x13BC0U, cputest_13BC0, 1 },
    { "vspltish", "vspltish v3,15", 0x106F034CU, 0x13C00U, cputest_13C00, 1 },
    { "vspltish", "vspltish v3,-16", 0x1070034CU, 0x13C40U, cputest_13C40, 1 },
    { "vspltish", "vspltish v3,-1", 0x107F034CU, 0x13C80U, cputest_13C80, 1 },
    { "vctuxs", "vctuxs v3,v3,0", 0x10601B8AU, 0x13CC0U, cputest_13CC0, 1 },
    { "vctuxs", "vctuxs v3,v3,1", 0x10611B8AU, 0x13D00U, cputest_13D00, 1 },
    { "vctuxs", "vctuxs v3,v3,8", 0x10681B8AU, 0x13D40U, cputest_13D40, 1 },
    { "vctuxs", "vctuxs v3,v3,31", 0x107F1B8AU, 0x13D80U, cputest_13D80, 1 },
    { "vsel", "vsel v3,v4,v3,v4", 0x1064192AU, 0x13DC0U, cputest_13DC0, 1 },
    { "vsldoi", "vsldoi v3,v4,v3,0", 0x1064182CU, 0x13E00U, cputest_13E00, 1 },
    { "vsldoi", "vsldoi v3,v4,v3,1", 0x1064186CU, 0x13E40U, cputest_13E40, 1 },
    { "vsldoi", "vsldoi v3,v4,v3,5", 0x1064196CU, 0x13E80U, cputest_13E80, 1 },
    { "vsldoi", "vsldoi v3,v4,v3,15", 0x10641BECU, 0x13EC0U, cputest_13EC0, 1 },
    { "vnor", "vnor v3,v4,v4", 0x10642504U, 0x13F00U, cputest_13F00, 1 },
    { "vandc", "vandc v3,v4,v4", 0x10642444U, 0x13F40U, cputest_13F40, 1 },
    { "vslh", "vslh v3,v4,v4", 0x10642144U, 0x13F80U, cputest_13F80, 1 },
    { "vsrh", "vsrh v3,v4,v4", 0x10642244U, 0x13FC0U, cputest_13FC0, 1 },
    { "vsrah", "vsrah v3,v4,v4", 0x10642344U, 0x14000U, cputest_14000, 1 },
    { "vsro", "vsro v3,v4,v4", 0x1064244CU, 0x14040U, cputest_14040, 1 },
    { "vminsw", "vminsw v3,v4,v4", 0x10642382U, 0x14080U, cputest_14080, 1 },
    { "vminuh", "vminuh v3,v4,v4", 0x10642242U, 0x140C0U, cputest_140C0, 1 },
    { "vmaxuh", "vmaxuh v3,v4,v4", 0x10642042U, 0x14100U, cputest_14100, 1 },
    { "vavguh", "vavguh v3,v4,v4", 0x10642442U, 0x14140U, cputest_14140, 1 },
    { "vsubuwm", "vsubuwm v3,v4,v4", 0x10642480U, 0x14180U, cputest_14180, 1 },
    { "vsububm", "vsububm v3,v4,v4", 0x10642400U, 0x141C0U, cputest_141C0, 1 },
    { "vsubshs", "vsubshs v3,v4,v4", 0x10642740U, 0x14200U, cputest_14200, 1 },
    { "vaddsws", "vaddsws v3,v4,v4", 0x10642380U, 0x14240U, cputest_14240, 1 },
    { "vpkswss", "vpkswss v3,v4,v4", 0x106421CEU, 0x14280U, cputest_14280, 1 },
    { "vcmpgtsb", "vcmpgtsb v3,v4,v4", 0x10642306U, 0x142C0U, cputest_142C0, 1 },
    { "vcmpgtsb.", "vcmpgtsb. v3,v4,v4", 0x10642706U, 0x14300U, cputest_14300, 1 },
    { "vcmpgtsw", "vcmpgtsw v3,v4,v4", 0x10642386U, 0x14340U, cputest_14340, 1 },
    { "vcmpgtsw.", "vcmpgtsw. v3,v4,v4", 0x10642786U, 0x14380U, cputest_14380, 1 },
    { "vcmpgtuw", "vcmpgtuw v3,v4,v4", 0x10642286U, 0x143C0U, cputest_143C0, 1 },
    { "vcmpgtuw.", "vcmpgtuw. v3,v4,v4", 0x10642686U, 0x14400U, cputest_14400, 1 },
    { "vspltish", "vspltish v3,0", 0x1060034CU, 0x14440U, cputest_14440, 1 },
    { "vspltish", "vspltish v3,1", 0x1061034CU, 0x14480U, cputest_14480, 1 },
    { "vspltish", "vspltish v3,15", 0x106F034CU, 0x144C0U, cputest_144C0, 1 },
    { "vspltish", "vspltish v3,-16", 0x1070034CU, 0x14500U, cputest_14500, 1 },
    { "vspltish", "vspltish v3,-1", 0x107F034CU, 0x14540U, cputest_14540, 1 },
    { "vctuxs", "vctuxs v3,v4,0", 0x1060238AU, 0x14580U, cputest_14580, 1 },
    { "vctuxs", "vctuxs v3,v4,1", 0x1061238AU, 0x145C0U, cputest_145C0, 1 },
    { "vctuxs", "vctuxs v3,v4,8", 0x1068238AU, 0x14600U, cputest_14600, 1 },
    { "vctuxs", "vctuxs v3,v4,31", 0x107F238AU, 0x14640U, cputest_14640, 1 },
    { "vsel", "vsel v3,v4,v4,v4", 0x1064212AU, 0x14680U, cputest_14680, 1 },
    { "vsldoi", "vsldoi v3,v4,v4,0", 0x1064202CU, 0x146C0U, cputest_146C0, 1 },
    { "vsldoi", "vsldoi v3,v4,v4,1", 0x1064206CU, 0x14700U, cputest_14700, 1 },
    { "vsldoi", "vsldoi v3,v4,v4,5", 0x1064216CU, 0x14740U, cputest_14740, 1 },
    { "vsldoi", "vsldoi v3,v4,v4,15", 0x106423ECU, 0x14780U, cputest_14780, 1 },
    { "vnor", "vnor v6,v6,v6", 0x10C63504U, 0x147C0U, cputest_147C0, 1 },
    { "vandc", "vandc v6,v6,v6", 0x10C63444U, 0x14800U, cputest_14800, 1 },
    { "vslh", "vslh v6,v6,v6", 0x10C63144U, 0x14840U, cputest_14840, 1 },
    { "vsrh", "vsrh v6,v6,v6", 0x10C63244U, 0x14880U, cputest_14880, 1 },
    { "vsrah", "vsrah v6,v6,v6", 0x10C63344U, 0x148C0U, cputest_148C0, 1 },
    { "vsro", "vsro v6,v6,v6", 0x10C6344CU, 0x14900U, cputest_14900, 1 },
    { "vminsw", "vminsw v6,v6,v6", 0x10C63382U, 0x14940U, cputest_14940, 1 },
    { "vminuh", "vminuh v6,v6,v6", 0x10C63242U, 0x14980U, cputest_14980, 1 },
    { "vmaxuh", "vmaxuh v6,v6,v6", 0x10C63042U, 0x149C0U, cputest_149C0, 1 },
    { "vavguh", "vavguh v6,v6,v6", 0x10C63442U, 0x14A00U, cputest_14A00, 1 },
    { "vsubuwm", "vsubuwm v6,v6,v6", 0x10C63480U, 0x14A40U, cputest_14A40, 1 },
    { "vsububm", "vsububm v6,v6,v6", 0x10C63400U, 0x14A80U, cputest_14A80, 1 },
    { "vsubshs", "vsubshs v6,v6,v6", 0x10C63740U, 0x14AC0U, cputest_14AC0, 1 },
    { "vaddsws", "vaddsws v6,v6,v6", 0x10C63380U, 0x14B00U, cputest_14B00, 1 },
    { "vpkswss", "vpkswss v6,v6,v6", 0x10C631CEU, 0x14B40U, cputest_14B40, 1 },
    { "vcmpgtsb", "vcmpgtsb v6,v6,v6", 0x10C63306U, 0x14B80U, cputest_14B80, 1 },
    { "vcmpgtsb.", "vcmpgtsb. v6,v6,v6", 0x10C63706U, 0x14BC0U, cputest_14BC0, 1 },
    { "vcmpgtsw", "vcmpgtsw v6,v6,v6", 0x10C63386U, 0x14C00U, cputest_14C00, 1 },
    { "vcmpgtsw.", "vcmpgtsw. v6,v6,v6", 0x10C63786U, 0x14C40U, cputest_14C40, 1 },
    { "vcmpgtuw", "vcmpgtuw v6,v6,v6", 0x10C63286U, 0x14C80U, cputest_14C80, 1 },
    { "vcmpgtuw.", "vcmpgtuw. v6,v6,v6", 0x10C63686U, 0x14CC0U, cputest_14CC0, 1 },
    { "vspltish", "vspltish v6,0", 0x10C0034CU, 0x14D00U, cputest_14D00, 1 },
    { "vspltish", "vspltish v6,1", 0x10C1034CU, 0x14D40U, cputest_14D40, 1 },
    { "vspltish", "vspltish v6,15", 0x10CF034CU, 0x14D80U, cputest_14D80, 1 },
    { "vspltish", "vspltish v6,-16", 0x10D0034CU, 0x14DC0U, cputest_14DC0, 1 },
    { "vspltish", "vspltish v6,-1", 0x10DF034CU, 0x14E00U, cputest_14E00, 1 },
    { "vctuxs", "vctuxs v6,v6,0", 0x10C0338AU, 0x14E40U, cputest_14E40, 1 },
    { "vctuxs", "vctuxs v6,v6,1", 0x10C1338AU, 0x14E80U, cputest_14E80, 1 },
    { "vctuxs", "vctuxs v6,v6,8", 0x10C8338AU, 0x14EC0U, cputest_14EC0, 1 },
    { "vctuxs", "vctuxs v6,v6,31", 0x10DF338AU, 0x14F00U, cputest_14F00, 1 },
    { "vsel", "vsel v6,v6,v6,v7", 0x10C631EAU, 0x14F40U, cputest_14F40, 1 },
    { "vsldoi", "vsldoi v6,v6,v6,0", 0x10C6302CU, 0x14F80U, cputest_14F80, 1 },
    { "vsldoi", "vsldoi v6,v6,v6,1", 0x10C6306CU, 0x14FC0U, cputest_14FC0, 1 },
    { "vsldoi", "vsldoi v6,v6,v6,5", 0x10C6316CU, 0x15000U, cputest_15000, 1 },
    { "vsldoi", "vsldoi v6,v6,v6,15", 0x10C633ECU, 0x15040U, cputest_15040, 1 },
    { "vnor", "vnor v31,v30,v29", 0x13FEED04U, 0x15080U, cputest_15080, 1 },
    { "vandc", "vandc v31,v30,v29", 0x13FEEC44U, 0x150C0U, cputest_150C0, 1 },
    { "vslh", "vslh v31,v30,v29", 0x13FEE944U, 0x15100U, cputest_15100, 1 },
    { "vsrh", "vsrh v31,v30,v29", 0x13FEEA44U, 0x15140U, cputest_15140, 1 },
    { "vsrah", "vsrah v31,v30,v29", 0x13FEEB44U, 0x15180U, cputest_15180, 1 },
    { "vsro", "vsro v31,v30,v29", 0x13FEEC4CU, 0x151C0U, cputest_151C0, 1 },
    { "vminsw", "vminsw v31,v30,v29", 0x13FEEB82U, 0x15200U, cputest_15200, 1 },
    { "vminuh", "vminuh v31,v30,v29", 0x13FEEA42U, 0x15240U, cputest_15240, 1 },
    { "vmaxuh", "vmaxuh v31,v30,v29", 0x13FEE842U, 0x15280U, cputest_15280, 1 },
    { "vavguh", "vavguh v31,v30,v29", 0x13FEEC42U, 0x152C0U, cputest_152C0, 1 },
    { "vsubuwm", "vsubuwm v31,v30,v29", 0x13FEEC80U, 0x15300U, cputest_15300, 1 },
    { "vsububm", "vsububm v31,v30,v29", 0x13FEEC00U, 0x15340U, cputest_15340, 1 },
    { "vsubshs", "vsubshs v31,v30,v29", 0x13FEEF40U, 0x15380U, cputest_15380, 1 },
    { "vaddsws", "vaddsws v31,v30,v29", 0x13FEEB80U, 0x153C0U, cputest_153C0, 1 },
    { "vpkswss", "vpkswss v31,v30,v29", 0x13FEE9CEU, 0x15400U, cputest_15400, 1 },
    { "vcmpgtsb", "vcmpgtsb v31,v30,v29", 0x13FEEB06U, 0x15440U, cputest_15440, 1 },
    { "vcmpgtsb.", "vcmpgtsb. v31,v30,v29", 0x13FEEF06U, 0x15480U, cputest_15480, 1 },
    { "vcmpgtsw", "vcmpgtsw v31,v30,v29", 0x13FEEB86U, 0x154C0U, cputest_154C0, 1 },
    { "vcmpgtsw.", "vcmpgtsw. v31,v30,v29", 0x13FEEF86U, 0x15500U, cputest_15500, 1 },
    { "vcmpgtuw", "vcmpgtuw v31,v30,v29", 0x13FEEA86U, 0x15540U, cputest_15540, 1 },
    { "vcmpgtuw.", "vcmpgtuw. v31,v30,v29", 0x13FEEE86U, 0x15580U, cputest_15580, 1 },
    { "vspltish", "vspltish v31,0", 0x13E0034CU, 0x155C0U, cputest_155C0, 1 },
    { "vspltish", "vspltish v31,1", 0x13E1034CU, 0x15600U, cputest_15600, 1 },
    { "vspltish", "vspltish v31,15", 0x13EF034CU, 0x15640U, cputest_15640, 1 },
    { "vspltish", "vspltish v31,-16", 0x13F0034CU, 0x15680U, cputest_15680, 1 },
    { "vspltish", "vspltish v31,-1", 0x13FF034CU, 0x156C0U, cputest_156C0, 1 },
    { "vctuxs", "vctuxs v31,v29,0", 0x13E0EB8AU, 0x15700U, cputest_15700, 1 },
    { "vctuxs", "vctuxs v31,v29,1", 0x13E1EB8AU, 0x15740U, cputest_15740, 1 },
    { "vctuxs", "vctuxs v31,v29,8", 0x13E8EB8AU, 0x15780U, cputest_15780, 1 },
    { "vctuxs", "vctuxs v31,v29,31", 0x13FFEB8AU, 0x157C0U, cputest_157C0, 1 },
    { "vsel", "vsel v31,v30,v29,v0", 0x13FEE82AU, 0x15800U, cputest_15800, 1 },
    { "vsldoi", "vsldoi v31,v30,v29,0", 0x13FEE82CU, 0x15840U, cputest_15840, 1 },
    { "vsldoi", "vsldoi v31,v30,v29,1", 0x13FEE86CU, 0x15880U, cputest_15880, 1 },
    { "vsldoi", "vsldoi v31,v30,v29,5", 0x13FEE96CU, 0x158C0U, cputest_158C0, 1 },
    { "vsldoi", "vsldoi v31,v30,v29,15", 0x13FEEBECU, 0x15900U, cputest_15900, 1 },
    { "vnor", "vnor v0,v1,v2", 0x10011504U, 0x15940U, cputest_15940, 1 },
    { "vandc", "vandc v0,v1,v2", 0x10011444U, 0x15980U, cputest_15980, 1 },
    { "vslh", "vslh v0,v1,v2", 0x10011144U, 0x159C0U, cputest_159C0, 1 },
    { "vsrh", "vsrh v0,v1,v2", 0x10011244U, 0x15A00U, cputest_15A00, 1 },
    { "vsrah", "vsrah v0,v1,v2", 0x10011344U, 0x15A40U, cputest_15A40, 1 },
    { "vsro", "vsro v0,v1,v2", 0x1001144CU, 0x15A80U, cputest_15A80, 1 },
    { "vminsw", "vminsw v0,v1,v2", 0x10011382U, 0x15AC0U, cputest_15AC0, 1 },
    { "vminuh", "vminuh v0,v1,v2", 0x10011242U, 0x15B00U, cputest_15B00, 1 },
    { "vmaxuh", "vmaxuh v0,v1,v2", 0x10011042U, 0x15B40U, cputest_15B40, 1 },
    { "vavguh", "vavguh v0,v1,v2", 0x10011442U, 0x15B80U, cputest_15B80, 1 },
    { "vsubuwm", "vsubuwm v0,v1,v2", 0x10011480U, 0x15BC0U, cputest_15BC0, 1 },
    { "vsububm", "vsububm v0,v1,v2", 0x10011400U, 0x15C00U, cputest_15C00, 1 },
    { "vsubshs", "vsubshs v0,v1,v2", 0x10011740U, 0x15C40U, cputest_15C40, 1 },
    { "vaddsws", "vaddsws v0,v1,v2", 0x10011380U, 0x15C80U, cputest_15C80, 1 },
    { "vpkswss", "vpkswss v0,v1,v2", 0x100111CEU, 0x15CC0U, cputest_15CC0, 1 },
    { "vcmpgtsb", "vcmpgtsb v0,v1,v2", 0x10011306U, 0x15D00U, cputest_15D00, 1 },
    { "vcmpgtsb.", "vcmpgtsb. v0,v1,v2", 0x10011706U, 0x15D40U, cputest_15D40, 1 },
    { "vcmpgtsw", "vcmpgtsw v0,v1,v2", 0x10011386U, 0x15D80U, cputest_15D80, 1 },
    { "vcmpgtsw.", "vcmpgtsw. v0,v1,v2", 0x10011786U, 0x15DC0U, cputest_15DC0, 1 },
    { "vcmpgtuw", "vcmpgtuw v0,v1,v2", 0x10011286U, 0x15E00U, cputest_15E00, 1 },
    { "vcmpgtuw.", "vcmpgtuw. v0,v1,v2", 0x10011686U, 0x15E40U, cputest_15E40, 1 },
    { "vspltish", "vspltish v0,0", 0x1000034CU, 0x15E80U, cputest_15E80, 1 },
    { "vspltish", "vspltish v0,1", 0x1001034CU, 0x15EC0U, cputest_15EC0, 1 },
    { "vspltish", "vspltish v0,15", 0x100F034CU, 0x15F00U, cputest_15F00, 1 },
    { "vspltish", "vspltish v0,-16", 0x1010034CU, 0x15F40U, cputest_15F40, 1 },
    { "vspltish", "vspltish v0,-1", 0x101F034CU, 0x15F80U, cputest_15F80, 1 },
    { "vctuxs", "vctuxs v0,v2,0", 0x1000138AU, 0x15FC0U, cputest_15FC0, 1 },
    { "vctuxs", "vctuxs v0,v2,1", 0x1001138AU, 0x16000U, cputest_16000, 1 },
    { "vctuxs", "vctuxs v0,v2,8", 0x1008138AU, 0x16040U, cputest_16040, 1 },
    { "vctuxs", "vctuxs v0,v2,31", 0x101F138AU, 0x16080U, cputest_16080, 1 },
    { "vsel", "vsel v0,v1,v2,v1", 0x1001106AU, 0x160C0U, cputest_160C0, 1 },
    { "vsldoi", "vsldoi v0,v1,v2,0", 0x1001102CU, 0x16100U, cputest_16100, 1 },
    { "vsldoi", "vsldoi v0,v1,v2,1", 0x1001106CU, 0x16140U, cputest_16140, 1 },
    { "vsldoi", "vsldoi v0,v1,v2,5", 0x1001116CU, 0x16180U, cputest_16180, 1 },
    { "vsldoi", "vsldoi v0,v1,v2,15", 0x100113ECU, 0x161C0U, cputest_161C0, 1 },
    { "vnor128", "vnor128 v3,v4,v5", 0x14642A90U, 0x16200U, cputest_16200, 1 },
    { "vsro128", "vsro128 v3,v4,v5", 0x14642BD0U, 0x16240U, cputest_16240, 1 },
    { "vpkswss128", "vpkswss128 v3,v4,v5", 0x14642A80U, 0x16280U, cputest_16280, 1 },
    { "vsel128", "vsel128 v3,v4,v5", 0x14642B50U, 0x162C0U, cputest_162C0, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v5,0", 0x18602A70U, 0x16300U, cputest_16300, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v5,1", 0x18612A70U, 0x16340U, cputest_16340, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v5,8", 0x18682A70U, 0x16380U, cputest_16380, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v5,31", 0x187F2A70U, 0x163C0U, cputest_163C0, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v5,0", 0x10642810U, 0x16400U, cputest_16400, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v5,1", 0x10642850U, 0x16440U, cputest_16440, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v5,5", 0x10642950U, 0x16480U, cputest_16480, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v5,15", 0x10642BD0U, 0x164C0U, cputest_164C0, 1 },
    { "vupkd3d128_5", "vupkd3d128 v3,v5,20", 0x18742FF0U, 0x16500U, cputest_16500, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,1,0", 0x186D2E10U, 0x16540U, cputest_16540, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,1,1", 0x186D2E50U, 0x16580U, cputest_16580, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,1,2", 0x186D2E90U, 0x165C0U, cputest_165C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,1,3", 0x186D2ED0U, 0x16600U, cputest_16600, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,2,0", 0x186E2E10U, 0x16640U, cputest_16640, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,2,1", 0x186E2E50U, 0x16680U, cputest_16680, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,2,2", 0x186E2E90U, 0x166C0U, cputest_166C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,2,3", 0x186E2ED0U, 0x16700U, cputest_16700, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,3,0", 0x186F2E10U, 0x16740U, cputest_16740, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,3,1", 0x186F2E50U, 0x16780U, cputest_16780, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,3,2", 0x186F2E90U, 0x167C0U, cputest_167C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,3,3", 0x186F2ED0U, 0x16800U, cputest_16800, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,1,0", 0x18752E10U, 0x16840U, cputest_16840, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,1,1", 0x18752E50U, 0x16880U, cputest_16880, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,1,2", 0x18752E90U, 0x168C0U, cputest_168C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,1,3", 0x18752ED0U, 0x16900U, cputest_16900, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,2,0", 0x18762E10U, 0x16940U, cputest_16940, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,2,1", 0x18762E50U, 0x16980U, cputest_16980, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,2,2", 0x18762E90U, 0x169C0U, cputest_169C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,2,3", 0x18762ED0U, 0x16A00U, cputest_16A00, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,3,0", 0x18772E10U, 0x16A40U, cputest_16A40, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,3,1", 0x18772E50U, 0x16A80U, cputest_16A80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,3,2", 0x18772E90U, 0x16AC0U, cputest_16AC0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,3,3", 0x18772ED0U, 0x16B00U, cputest_16B00, 1 },
    { "vnor128", "vnor128 v3,v3,v5", 0x14632A90U, 0x16B40U, cputest_16B40, 1 },
    { "vsro128", "vsro128 v3,v3,v5", 0x14632BD0U, 0x16B80U, cputest_16B80, 1 },
    { "vpkswss128", "vpkswss128 v3,v3,v5", 0x14632A80U, 0x16BC0U, cputest_16BC0, 1 },
    { "vsel128", "vsel128 v3,v3,v5", 0x14632B50U, 0x16C00U, cputest_16C00, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v5,0", 0x18602A70U, 0x16C40U, cputest_16C40, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v5,1", 0x18612A70U, 0x16C80U, cputest_16C80, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v5,8", 0x18682A70U, 0x16CC0U, cputest_16CC0, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v5,31", 0x187F2A70U, 0x16D00U, cputest_16D00, 1 },
    { "vsldoi128", "vsldoi128 v3,v3,v5,0", 0x10632810U, 0x16D40U, cputest_16D40, 1 },
    { "vsldoi128", "vsldoi128 v3,v3,v5,1", 0x10632850U, 0x16D80U, cputest_16D80, 1 },
    { "vsldoi128", "vsldoi128 v3,v3,v5,5", 0x10632950U, 0x16DC0U, cputest_16DC0, 1 },
    { "vsldoi128", "vsldoi128 v3,v3,v5,15", 0x10632BD0U, 0x16E00U, cputest_16E00, 1 },
    { "vupkd3d128_5", "vupkd3d128 v3,v5,20", 0x18742FF0U, 0x16E40U, cputest_16E40, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,1,0", 0x186D2E10U, 0x16E80U, cputest_16E80, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,1,1", 0x186D2E50U, 0x16EC0U, cputest_16EC0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,1,2", 0x186D2E90U, 0x16F00U, cputest_16F00, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,1,3", 0x186D2ED0U, 0x16F40U, cputest_16F40, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,2,0", 0x186E2E10U, 0x16F80U, cputest_16F80, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,2,1", 0x186E2E50U, 0x16FC0U, cputest_16FC0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,2,2", 0x186E2E90U, 0x17000U, cputest_17000, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,2,3", 0x186E2ED0U, 0x17040U, cputest_17040, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,3,0", 0x186F2E10U, 0x17080U, cputest_17080, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,3,1", 0x186F2E50U, 0x170C0U, cputest_170C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,3,2", 0x186F2E90U, 0x17100U, cputest_17100, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v5,3,3,3", 0x186F2ED0U, 0x17140U, cputest_17140, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,1,0", 0x18752E10U, 0x17180U, cputest_17180, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,1,1", 0x18752E50U, 0x171C0U, cputest_171C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,1,2", 0x18752E90U, 0x17200U, cputest_17200, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,1,3", 0x18752ED0U, 0x17240U, cputest_17240, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,2,0", 0x18762E10U, 0x17280U, cputest_17280, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,2,1", 0x18762E50U, 0x172C0U, cputest_172C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,2,2", 0x18762E90U, 0x17300U, cputest_17300, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,2,3", 0x18762ED0U, 0x17340U, cputest_17340, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,3,0", 0x18772E10U, 0x17380U, cputest_17380, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,3,1", 0x18772E50U, 0x173C0U, cputest_173C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,3,2", 0x18772E90U, 0x17400U, cputest_17400, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v5,5,3,3", 0x18772ED0U, 0x17440U, cputest_17440, 1 },
    { "vnor128", "vnor128 v3,v4,v3", 0x14641A90U, 0x17480U, cputest_17480, 1 },
    { "vsro128", "vsro128 v3,v4,v3", 0x14641BD0U, 0x174C0U, cputest_174C0, 1 },
    { "vpkswss128", "vpkswss128 v3,v4,v3", 0x14641A80U, 0x17500U, cputest_17500, 1 },
    { "vsel128", "vsel128 v3,v4,v3", 0x14641B50U, 0x17540U, cputest_17540, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v3,0", 0x18601A70U, 0x17580U, cputest_17580, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v3,1", 0x18611A70U, 0x175C0U, cputest_175C0, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v3,8", 0x18681A70U, 0x17600U, cputest_17600, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v3,31", 0x187F1A70U, 0x17640U, cputest_17640, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v3,0", 0x10641810U, 0x17680U, cputest_17680, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v3,1", 0x10641850U, 0x176C0U, cputest_176C0, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v3,5", 0x10641950U, 0x17700U, cputest_17700, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v3,15", 0x10641BD0U, 0x17740U, cputest_17740, 1 },
    { "vupkd3d128_5", "vupkd3d128 v3,v3,20", 0x18741FF0U, 0x17780U, cputest_17780, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,1,0", 0x186D1E10U, 0x177C0U, cputest_177C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,1,1", 0x186D1E50U, 0x17800U, cputest_17800, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,1,2", 0x186D1E90U, 0x17840U, cputest_17840, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,1,3", 0x186D1ED0U, 0x17880U, cputest_17880, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,2,0", 0x186E1E10U, 0x178C0U, cputest_178C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,2,1", 0x186E1E50U, 0x17900U, cputest_17900, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,2,2", 0x186E1E90U, 0x17940U, cputest_17940, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,2,3", 0x186E1ED0U, 0x17980U, cputest_17980, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,3,0", 0x186F1E10U, 0x179C0U, cputest_179C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,3,1", 0x186F1E50U, 0x17A00U, cputest_17A00, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,3,2", 0x186F1E90U, 0x17A40U, cputest_17A40, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v3,3,3,3", 0x186F1ED0U, 0x17A80U, cputest_17A80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,1,0", 0x18751E10U, 0x17AC0U, cputest_17AC0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,1,1", 0x18751E50U, 0x17B00U, cputest_17B00, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,1,2", 0x18751E90U, 0x17B40U, cputest_17B40, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,1,3", 0x18751ED0U, 0x17B80U, cputest_17B80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,2,0", 0x18761E10U, 0x17BC0U, cputest_17BC0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,2,1", 0x18761E50U, 0x17C00U, cputest_17C00, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,2,2", 0x18761E90U, 0x17C40U, cputest_17C40, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,2,3", 0x18761ED0U, 0x17C80U, cputest_17C80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,3,0", 0x18771E10U, 0x17CC0U, cputest_17CC0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,3,1", 0x18771E50U, 0x17D00U, cputest_17D00, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,3,2", 0x18771E90U, 0x17D40U, cputest_17D40, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v3,5,3,3", 0x18771ED0U, 0x17D80U, cputest_17D80, 1 },
    { "vnor128", "vnor128 v3,v4,v4", 0x14642290U, 0x17DC0U, cputest_17DC0, 1 },
    { "vsro128", "vsro128 v3,v4,v4", 0x146423D0U, 0x17E00U, cputest_17E00, 1 },
    { "vpkswss128", "vpkswss128 v3,v4,v4", 0x14642280U, 0x17E40U, cputest_17E40, 1 },
    { "vsel128", "vsel128 v3,v4,v4", 0x14642350U, 0x17E80U, cputest_17E80, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v4,0", 0x18602270U, 0x17EC0U, cputest_17EC0, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v4,1", 0x18612270U, 0x17F00U, cputest_17F00, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v4,8", 0x18682270U, 0x17F40U, cputest_17F40, 1 },
    { "vcfpuxws128", "vcfpuxws128 v3,v4,31", 0x187F2270U, 0x17F80U, cputest_17F80, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v4,0", 0x10642010U, 0x17FC0U, cputest_17FC0, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v4,1", 0x10642050U, 0x18000U, cputest_18000, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v4,5", 0x10642150U, 0x18040U, cputest_18040, 1 },
    { "vsldoi128", "vsldoi128 v3,v4,v4,15", 0x106423D0U, 0x18080U, cputest_18080, 1 },
    { "vupkd3d128_5", "vupkd3d128 v3,v4,20", 0x187427F0U, 0x180C0U, cputest_180C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,1,0", 0x186D2610U, 0x18100U, cputest_18100, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,1,1", 0x186D2650U, 0x18140U, cputest_18140, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,1,2", 0x186D2690U, 0x18180U, cputest_18180, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,1,3", 0x186D26D0U, 0x181C0U, cputest_181C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,2,0", 0x186E2610U, 0x18200U, cputest_18200, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,2,1", 0x186E2650U, 0x18240U, cputest_18240, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,2,2", 0x186E2690U, 0x18280U, cputest_18280, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,2,3", 0x186E26D0U, 0x182C0U, cputest_182C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,3,0", 0x186F2610U, 0x18300U, cputest_18300, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,3,1", 0x186F2650U, 0x18340U, cputest_18340, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,3,2", 0x186F2690U, 0x18380U, cputest_18380, 1 },
    { "vpkd3d128_3", "vpkd3d128 v3,v4,3,3,3", 0x186F26D0U, 0x183C0U, cputest_183C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,1,0", 0x18752610U, 0x18400U, cputest_18400, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,1,1", 0x18752650U, 0x18440U, cputest_18440, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,1,2", 0x18752690U, 0x18480U, cputest_18480, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,1,3", 0x187526D0U, 0x184C0U, cputest_184C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,2,0", 0x18762610U, 0x18500U, cputest_18500, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,2,1", 0x18762650U, 0x18540U, cputest_18540, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,2,2", 0x18762690U, 0x18580U, cputest_18580, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,2,3", 0x187626D0U, 0x185C0U, cputest_185C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,3,0", 0x18772610U, 0x18600U, cputest_18600, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,3,1", 0x18772650U, 0x18640U, cputest_18640, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,3,2", 0x18772690U, 0x18680U, cputest_18680, 1 },
    { "vpkd3d128_5", "vpkd3d128 v3,v4,5,3,3", 0x187726D0U, 0x186C0U, cputest_186C0, 1 },
    { "vnor128", "vnor128 v40,v40,v40", 0x150842B5U, 0x18700U, cputest_18700, 1 },
    { "vsro128", "vsro128 v40,v40,v40", 0x150843F5U, 0x18740U, cputest_18740, 1 },
    { "vpkswss128", "vpkswss128 v40,v40,v40", 0x150842A5U, 0x18780U, cputest_18780, 1 },
    { "vsel128", "vsel128 v40,v40,v40", 0x15084375U, 0x187C0U, cputest_187C0, 1 },
    { "vcfpuxws128", "vcfpuxws128 v40,v40,0", 0x19004275U, 0x18800U, cputest_18800, 1 },
    { "vcfpuxws128", "vcfpuxws128 v40,v40,1", 0x19014275U, 0x18840U, cputest_18840, 1 },
    { "vcfpuxws128", "vcfpuxws128 v40,v40,8", 0x19084275U, 0x18880U, cputest_18880, 1 },
    { "vcfpuxws128", "vcfpuxws128 v40,v40,31", 0x191F4275U, 0x188C0U, cputest_188C0, 1 },
    { "vsldoi128", "vsldoi128 v40,v40,v40,0", 0x11084035U, 0x18900U, cputest_18900, 1 },
    { "vsldoi128", "vsldoi128 v40,v40,v40,1", 0x11084075U, 0x18940U, cputest_18940, 1 },
    { "vsldoi128", "vsldoi128 v40,v40,v40,5", 0x11084175U, 0x18980U, cputest_18980, 1 },
    { "vsldoi128", "vsldoi128 v40,v40,v40,15", 0x110843F5U, 0x189C0U, cputest_189C0, 1 },
    { "vupkd3d128_5", "vupkd3d128 v40,v40,20", 0x191447F5U, 0x18A00U, cputest_18A00, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,1,0", 0x190D4615U, 0x18A40U, cputest_18A40, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,1,1", 0x190D4655U, 0x18A80U, cputest_18A80, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,1,2", 0x190D4695U, 0x18AC0U, cputest_18AC0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,1,3", 0x190D46D5U, 0x18B00U, cputest_18B00, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,2,0", 0x190E4615U, 0x18B40U, cputest_18B40, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,2,1", 0x190E4655U, 0x18B80U, cputest_18B80, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,2,2", 0x190E4695U, 0x18BC0U, cputest_18BC0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,2,3", 0x190E46D5U, 0x18C00U, cputest_18C00, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,3,0", 0x190F4615U, 0x18C40U, cputest_18C40, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,3,1", 0x190F4655U, 0x18C80U, cputest_18C80, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,3,2", 0x190F4695U, 0x18CC0U, cputest_18CC0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v40,v40,3,3,3", 0x190F46D5U, 0x18D00U, cputest_18D00, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,1,0", 0x19154615U, 0x18D40U, cputest_18D40, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,1,1", 0x19154655U, 0x18D80U, cputest_18D80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,1,2", 0x19154695U, 0x18DC0U, cputest_18DC0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,1,3", 0x191546D5U, 0x18E00U, cputest_18E00, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,2,0", 0x19164615U, 0x18E40U, cputest_18E40, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,2,1", 0x19164655U, 0x18E80U, cputest_18E80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,2,2", 0x19164695U, 0x18EC0U, cputest_18EC0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,2,3", 0x191646D5U, 0x18F00U, cputest_18F00, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,3,0", 0x19174615U, 0x18F40U, cputest_18F40, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,3,1", 0x19174655U, 0x18F80U, cputest_18F80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,3,2", 0x19174695U, 0x18FC0U, cputest_18FC0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v40,v40,5,3,3", 0x191746D5U, 0x19000U, cputest_19000, 1 },
    { "vnor128", "vnor128 v127,v64,v100", 0x17E0269FU, 0x19040U, cputest_19040, 1 },
    { "vsro128", "vsro128 v127,v64,v100", 0x17E027DFU, 0x19080U, cputest_19080, 1 },
    { "vpkswss128", "vpkswss128 v127,v64,v100", 0x17E0268FU, 0x190C0U, cputest_190C0, 1 },
    { "vsel128", "vsel128 v127,v64,v100", 0x17E0275FU, 0x19100U, cputest_19100, 1 },
    { "vcfpuxws128", "vcfpuxws128 v127,v100,0", 0x1BE0227FU, 0x19140U, cputest_19140, 1 },
    { "vcfpuxws128", "vcfpuxws128 v127,v100,1", 0x1BE1227FU, 0x19180U, cputest_19180, 1 },
    { "vcfpuxws128", "vcfpuxws128 v127,v100,8", 0x1BE8227FU, 0x191C0U, cputest_191C0, 1 },
    { "vcfpuxws128", "vcfpuxws128 v127,v100,31", 0x1BFF227FU, 0x19200U, cputest_19200, 1 },
    { "vsldoi128", "vsldoi128 v127,v64,v100,0", 0x13E0241FU, 0x19240U, cputest_19240, 1 },
    { "vsldoi128", "vsldoi128 v127,v64,v100,1", 0x13E0245FU, 0x19280U, cputest_19280, 1 },
    { "vsldoi128", "vsldoi128 v127,v64,v100,5", 0x13E0255FU, 0x192C0U, cputest_192C0, 1 },
    { "vsldoi128", "vsldoi128 v127,v64,v100,15", 0x13E027DFU, 0x19300U, cputest_19300, 1 },
    { "vupkd3d128_5", "vupkd3d128 v127,v100,20", 0x1BF427FFU, 0x19340U, cputest_19340, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,1,0", 0x1BED261FU, 0x19380U, cputest_19380, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,1,1", 0x1BED265FU, 0x193C0U, cputest_193C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,1,2", 0x1BED269FU, 0x19400U, cputest_19400, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,1,3", 0x1BED26DFU, 0x19440U, cputest_19440, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,2,0", 0x1BEE261FU, 0x19480U, cputest_19480, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,2,1", 0x1BEE265FU, 0x194C0U, cputest_194C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,2,2", 0x1BEE269FU, 0x19500U, cputest_19500, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,2,3", 0x1BEE26DFU, 0x19540U, cputest_19540, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,3,0", 0x1BEF261FU, 0x19580U, cputest_19580, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,3,1", 0x1BEF265FU, 0x195C0U, cputest_195C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,3,2", 0x1BEF269FU, 0x19600U, cputest_19600, 1 },
    { "vpkd3d128_3", "vpkd3d128 v127,v100,3,3,3", 0x1BEF26DFU, 0x19640U, cputest_19640, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,1,0", 0x1BF5261FU, 0x19680U, cputest_19680, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,1,1", 0x1BF5265FU, 0x196C0U, cputest_196C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,1,2", 0x1BF5269FU, 0x19700U, cputest_19700, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,1,3", 0x1BF526DFU, 0x19740U, cputest_19740, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,2,0", 0x1BF6261FU, 0x19780U, cputest_19780, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,2,1", 0x1BF6265FU, 0x197C0U, cputest_197C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,2,2", 0x1BF6269FU, 0x19800U, cputest_19800, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,2,3", 0x1BF626DFU, 0x19840U, cputest_19840, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,3,0", 0x1BF7261FU, 0x19880U, cputest_19880, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,3,1", 0x1BF7265FU, 0x198C0U, cputest_198C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,3,2", 0x1BF7269FU, 0x19900U, cputest_19900, 1 },
    { "vpkd3d128_5", "vpkd3d128 v127,v100,5,3,3", 0x1BF726DFU, 0x19940U, cputest_19940, 1 },
    { "vnor128", "vnor128 v96,v33,v65", 0x14010ABEU, 0x19980U, cputest_19980, 1 },
    { "vsro128", "vsro128 v96,v33,v65", 0x14010BFEU, 0x199C0U, cputest_199C0, 1 },
    { "vpkswss128", "vpkswss128 v96,v33,v65", 0x14010AAEU, 0x19A00U, cputest_19A00, 1 },
    { "vsel128", "vsel128 v96,v33,v65", 0x14010B7EU, 0x19A40U, cputest_19A40, 1 },
    { "vcfpuxws128", "vcfpuxws128 v96,v65,0", 0x18000A7EU, 0x19A80U, cputest_19A80, 1 },
    { "vcfpuxws128", "vcfpuxws128 v96,v65,1", 0x18010A7EU, 0x19AC0U, cputest_19AC0, 1 },
    { "vcfpuxws128", "vcfpuxws128 v96,v65,8", 0x18080A7EU, 0x19B00U, cputest_19B00, 1 },
    { "vcfpuxws128", "vcfpuxws128 v96,v65,31", 0x181F0A7EU, 0x19B40U, cputest_19B40, 1 },
    { "vsldoi128", "vsldoi128 v96,v33,v65,0", 0x1001083EU, 0x19B80U, cputest_19B80, 1 },
    { "vsldoi128", "vsldoi128 v96,v33,v65,1", 0x1001087EU, 0x19BC0U, cputest_19BC0, 1 },
    { "vsldoi128", "vsldoi128 v96,v33,v65,5", 0x1001097EU, 0x19C00U, cputest_19C00, 1 },
    { "vsldoi128", "vsldoi128 v96,v33,v65,15", 0x10010BFEU, 0x19C40U, cputest_19C40, 1 },
    { "vupkd3d128_5", "vupkd3d128 v96,v65,20", 0x18140FFEU, 0x19C80U, cputest_19C80, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,1,0", 0x180D0E1EU, 0x19CC0U, cputest_19CC0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,1,1", 0x180D0E5EU, 0x19D00U, cputest_19D00, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,1,2", 0x180D0E9EU, 0x19D40U, cputest_19D40, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,1,3", 0x180D0EDEU, 0x19D80U, cputest_19D80, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,2,0", 0x180E0E1EU, 0x19DC0U, cputest_19DC0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,2,1", 0x180E0E5EU, 0x19E00U, cputest_19E00, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,2,2", 0x180E0E9EU, 0x19E40U, cputest_19E40, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,2,3", 0x180E0EDEU, 0x19E80U, cputest_19E80, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,3,0", 0x180F0E1EU, 0x19EC0U, cputest_19EC0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,3,1", 0x180F0E5EU, 0x19F00U, cputest_19F00, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,3,2", 0x180F0E9EU, 0x19F40U, cputest_19F40, 1 },
    { "vpkd3d128_3", "vpkd3d128 v96,v65,3,3,3", 0x180F0EDEU, 0x19F80U, cputest_19F80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,1,0", 0x18150E1EU, 0x19FC0U, cputest_19FC0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,1,1", 0x18150E5EU, 0x1A000U, cputest_1A000, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,1,2", 0x18150E9EU, 0x1A040U, cputest_1A040, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,1,3", 0x18150EDEU, 0x1A080U, cputest_1A080, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,2,0", 0x18160E1EU, 0x1A0C0U, cputest_1A0C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,2,1", 0x18160E5EU, 0x1A100U, cputest_1A100, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,2,2", 0x18160E9EU, 0x1A140U, cputest_1A140, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,2,3", 0x18160EDEU, 0x1A180U, cputest_1A180, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,3,0", 0x18170E1EU, 0x1A1C0U, cputest_1A1C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,3,1", 0x18170E5EU, 0x1A200U, cputest_1A200, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,3,2", 0x18170E9EU, 0x1A240U, cputest_1A240, 1 },
    { "vpkd3d128_5", "vpkd3d128 v96,v65,5,3,3", 0x18170EDEU, 0x1A280U, cputest_1A280, 1 },
    { "vnor128", "vnor128 v0,v127,v1", 0x141F0EB0U, 0x1A2C0U, cputest_1A2C0, 1 },
    { "vsro128", "vsro128 v0,v127,v1", 0x141F0FF0U, 0x1A300U, cputest_1A300, 1 },
    { "vpkswss128", "vpkswss128 v0,v127,v1", 0x141F0EA0U, 0x1A340U, cputest_1A340, 1 },
    { "vsel128", "vsel128 v0,v127,v1", 0x141F0F70U, 0x1A380U, cputest_1A380, 1 },
    { "vcfpuxws128", "vcfpuxws128 v0,v1,0", 0x18000A70U, 0x1A3C0U, cputest_1A3C0, 1 },
    { "vcfpuxws128", "vcfpuxws128 v0,v1,1", 0x18010A70U, 0x1A400U, cputest_1A400, 1 },
    { "vcfpuxws128", "vcfpuxws128 v0,v1,8", 0x18080A70U, 0x1A440U, cputest_1A440, 1 },
    { "vcfpuxws128", "vcfpuxws128 v0,v1,31", 0x181F0A70U, 0x1A480U, cputest_1A480, 1 },
    { "vsldoi128", "vsldoi128 v0,v127,v1,0", 0x101F0C30U, 0x1A4C0U, cputest_1A4C0, 1 },
    { "vsldoi128", "vsldoi128 v0,v127,v1,1", 0x101F0C70U, 0x1A500U, cputest_1A500, 1 },
    { "vsldoi128", "vsldoi128 v0,v127,v1,5", 0x101F0D70U, 0x1A540U, cputest_1A540, 1 },
    { "vsldoi128", "vsldoi128 v0,v127,v1,15", 0x101F0FF0U, 0x1A580U, cputest_1A580, 1 },
    { "vupkd3d128_5", "vupkd3d128 v0,v1,20", 0x18140FF0U, 0x1A5C0U, cputest_1A5C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,1,0", 0x180D0E10U, 0x1A600U, cputest_1A600, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,1,1", 0x180D0E50U, 0x1A640U, cputest_1A640, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,1,2", 0x180D0E90U, 0x1A680U, cputest_1A680, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,1,3", 0x180D0ED0U, 0x1A6C0U, cputest_1A6C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,2,0", 0x180E0E10U, 0x1A700U, cputest_1A700, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,2,1", 0x180E0E50U, 0x1A740U, cputest_1A740, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,2,2", 0x180E0E90U, 0x1A780U, cputest_1A780, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,2,3", 0x180E0ED0U, 0x1A7C0U, cputest_1A7C0, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,3,0", 0x180F0E10U, 0x1A800U, cputest_1A800, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,3,1", 0x180F0E50U, 0x1A840U, cputest_1A840, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,3,2", 0x180F0E90U, 0x1A880U, cputest_1A880, 1 },
    { "vpkd3d128_3", "vpkd3d128 v0,v1,3,3,3", 0x180F0ED0U, 0x1A8C0U, cputest_1A8C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,1,0", 0x18150E10U, 0x1A900U, cputest_1A900, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,1,1", 0x18150E50U, 0x1A940U, cputest_1A940, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,1,2", 0x18150E90U, 0x1A980U, cputest_1A980, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,1,3", 0x18150ED0U, 0x1A9C0U, cputest_1A9C0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,2,0", 0x18160E10U, 0x1AA00U, cputest_1AA00, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,2,1", 0x18160E50U, 0x1AA40U, cputest_1AA40, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,2,2", 0x18160E90U, 0x1AA80U, cputest_1AA80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,2,3", 0x18160ED0U, 0x1AAC0U, cputest_1AAC0, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,3,0", 0x18170E10U, 0x1AB00U, cputest_1AB00, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,3,1", 0x18170E50U, 0x1AB40U, cputest_1AB40, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,3,2", 0x18170E90U, 0x1AB80U, cputest_1AB80, 1 },
    { "vpkd3d128_5", "vpkd3d128 v0,v1,5,3,3", 0x18170ED0U, 0x1ABC0U, cputest_1ABC0, 1 },
    { "vsldoi128", "vsldoi128 v9,v13,v59,5   (0x112DD951, game @83088AE0)", 0x112DD951U, 0x1AC00U, cputest_1AC00, 1 },
    { "vsldoi128", "vsldoi128 v6,v12,v59,5   (0x10CCD951, game @83088C0C)", 0x10CCD951U, 0x1AC40U, cputest_1AC40, 1 },
};
const size_t g_cpu_tests_count = sizeof(g_cpu_tests) / sizeof(g_cpu_tests[0]);
