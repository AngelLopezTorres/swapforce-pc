#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""Genera el banco de pruebas de CPU (--selftest-cpu) a partir del recompilador XenonRecomp parcheado.

1. Escribe un ELF32 PPC big-endian (cputest.o) con una funcion por prueba: <palabra de instruccion> + (blr).
   No contiene ningun codigo del juego: las palabras son codificaciones propias (campos segun ALTIVECPEM Rev.3,
   Power ISA v2.07B y, para VMX128, runtime/ref/ppc_decode_data.h + ppc_opcode_table_gen.cc de Xenia), mas las
   dos palabras 0x112DD951 y 0x10CCD951 (vsldoi128 que binutils decodifica como mulchw.).
2. Ejecuta XenonRecomp en modo prueba (XenonRecomp.exe <dir con .o> <dir salida>) -> cputest.cpp.
3. Escribe runtime/src/selftest/cpu_tests_gen.cpp (cabecera con el gancho de llamada indirecta + codigo recompilado
   + tabla g_cpu_tests) y runtime/src/selftest/cpu_xenia_tables.inc (tabla de permutacion de vpkd3d128 extraida
   automaticamente de ppc_emit_altivec.cc de Xenia, independiente de la tabla escrita a mano en el recompilador).
Uso: python gen_cputest.py"""
import os
import re
import struct
import subprocess
import sys
import shutil

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..', '..'))
RTDIR = os.path.join(root, 'runtime')
exe = os.path.join(root, 'tools', 'XenonRecomp', 'out', 'XenonRecomp', 'XenonRecomp.exe')
work = os.path.join(here, 'work')
outsrc = os.path.join(RTDIR, 'src', 'selftest')

BASE = 0x10000
STRIDE = 0x40
BLR = 0x4E800020


# ---------------- codificadores ----------------
def vx(base, vd, va, vb):
    return base | (vd << 21) | (va << 16) | (vb << 11)


def vx128(base, vd, va, vb):
    """VX128 (ppc_decode_data.h FormatVX128): VD=[25:21]|[3:2]<<5, VA=[20:16]|[5]<<5|[10]<<6, VB=[15:11]|[1:0]<<5."""
    return (base | ((vd & 31) << 21) | ((vd >> 5) << 2) | ((va & 31) << 16) | (((va >> 5) & 1) << 5) | (((va >> 6) & 1) << 10)
            | ((vb & 31) << 11) | (vb >> 5))


def vx128_3(base, vd, vb, uimm):
    return (base | ((vd & 31) << 21) | ((vd >> 5) << 2) | (uimm << 16) | ((vb & 31) << 11) | (vb >> 5))


def vx128_4(base, vd, vb, imm, z):
    return (base | ((vd & 31) << 21) | ((vd >> 5) << 2) | (imm << 16) | ((vb & 31) << 11) | (vb >> 5) | (z << 6))


def vx128_5(base, vd, va, vb, sh):
    return vx128(base, vd, va, vb) | (sh << 6)


def xo(base, rt, ra, rb, rc=0):
    return base | (rt << 21) | (ra << 16) | (rb << 11) | rc


def dform(base, rt, ra, d):
    return base | (rt << 21) | (ra << 16) | (d & 0xFFFF)


def bc(bo, bi, disp):
    return (16 << 26) | (bo << 21) | (bi << 16) | (disp & 0xFFFC)


# ---------------- inventario ----------------
tests = []   # (kind, texto, [palabras])


def add(kind, text, *words):
    tests.append((kind, text, list(words)))


# enteros (XO-form: RT,RA,RB)
INT_VARIANTS = [(3, 4, 5), (4, 4, 5), (5, 4, 5), (7, 7, 7), (31, 30, 29), (0, 1, 2)]
for rt, ra, rb in INT_VARIANTS:
    for rc in (0, 1):
        sfx = '.' if rc else ''
        add('addc', f'addc{sfx} r{rt},r{ra},r{rb}', xo(0x7C000014, rt, ra, rb, rc))
        add('mulhdu', f'mulhdu{sfx} r{rt},r{ra},r{rb}', xo(0x7C000012, rt, ra, rb, rc))
        # eqv RA,RS,RB: RS en bits 21-25, RA en 16-20
        add('eqv', f'eqv{sfx} r{ra},r{rt},r{rb}', xo(0x7C000238, rt, ra, rb, rc))
for rt, ra in [(3, 4), (4, 4), (31, 30), (0, 1)]:
    for rc in (0, 1):
        sfx = '.' if rc else ''
        add('addme', f'addme{sfx} r{rt},r{ra}', xo(0x7C0001D4, rt, ra, 0, rc))
        add('subfze', f'subfze{sfx} r{rt},r{ra}', xo(0x7C000190, rt, ra, 0, rc))

# carga/almacenamiento con actualizacion (D-form)
for rt, ra, d in [(3, 4, 0x10), (31, 30, -0x20), (10, 5, 0x1FE), (0, 9, -2)]:
    add('lhzu', f'lhzu r{rt},{d}(r{ra})', dform(0xA4000000, rt, ra, d))
    add('lhau', f'lhau r{rt},{d}(r{ra})', dform(0xAC000000, rt, ra, d))
    add('sthu', f'sthu r{rt},{d}(r{ra})', dform(0xB4000000, rt, ra, d))
    add('lfsu', f'lfsu f{rt},{d}(r{ra})', dform(0xC4000000, rt, ra, d))
    add('lfdu', f'lfdu f{rt},{d}(r{ra})', dform(0xCC000000, rt, ra, d & ~7))
    add('stfsu', f'stfsu f{rt},{d}(r{ra})', dform(0xD4000000, rt, ra, d))
    add('stfdu', f'stfdu f{rt},{d}(r{ra})', dform(0xDC000000, rt, ra, d & ~7))
# X-form con actualizacion (RT,RA,RB)
for rt, ra, rb in [(3, 4, 5), (31, 30, 29), (10, 5, 6), (0, 9, 8)]:
    for kind, base in [('lbzux', 0x7C0000EE), ('lhzux', 0x7C00026E), ('lwzux', 0x7C00006E), ('ldux', 0x7C00006A),
                       ('stbux', 0x7C0001EE), ('sthux', 0x7C00036E), ('stdux', 0x7C00016A),
                       ('lfsux', 0x7C00046E), ('stfsux', 0x7C00056E)]:
        f = 'f' if kind in ('lfsux', 'stfsux') else 'r'
        add(kind, f'{kind} {f}{rt},r{ra},r{rb}', xo(base, rt, ra, rb))
# sin actualizacion
for rt, ra, rb in [(3, 4, 5), (3, 0, 5), (31, 30, 29), (6, 6, 7)]:
    add('lhbrx', f'lhbrx r{rt},r{ra},r{rb}', xo(0x7C00062C, rt, ra, rb))
for vd, ra, rb in [(3, 4, 5), (3, 0, 5), (31, 30, 29)]:
    add('lvehx', f'lvehx v{vd},r{ra},r{rb}', xo(0x7C00004E, vd, ra, rb))
for ra, rb in [(4, 5), (0, 5), (30, 29)]:
    add('dcbst', f'dcbst r{ra},r{rb}', xo(0x7C00006C, 0, ra, rb))
# coma flotante
for frt, frb in [(1, 2), (5, 5), (31, 30), (0, 13)]:
    add('frsqrte', f'frsqrte f{frt},f{frb}', 0xFC000034 | (frt << 21) | (frb << 11))
# bifurcaciones: bc BO,BI,+8 ; addi r3,r3,1 ; blr
for bo, name in [(2, 'bdzf'), (8, 'bdnzt')]:
    for bi in (0, 2, 3, 6, 9, 17, 22, 31):
        add(name, f'{name} {bi},+8', bc(bo, bi, 8), 0x38630001)
# blrl
add('blrl', 'blrl', 0x4E800021)

# VMX / VMX128
VV = [(3, 4, 5), (3, 3, 5), (3, 4, 3), (3, 4, 4), (6, 6, 6), (31, 30, 29), (0, 1, 2)]
VV128 = [(3, 4, 5), (3, 3, 5), (3, 4, 3), (3, 4, 4), (40, 40, 40), (127, 64, 100), (96, 33, 65), (0, 127, 1)]
for vd, va, vb in VV:
    for kind, base in [('vnor', 0x10000504), ('vandc', 0x10000444), ('vslh', 0x10000144), ('vsrh', 0x10000244),
                       ('vsrah', 0x10000344), ('vsro', 0x1000044C), ('vminsw', 0x10000382), ('vminuh', 0x10000242),
                       ('vmaxuh', 0x10000042), ('vavguh', 0x10000442), ('vsubuwm', 0x10000480), ('vsububm', 0x10000400),
                       ('vsubshs', 0x10000740), ('vaddsws', 0x10000380), ('vpkswss', 0x100001CE)]:
        add(kind, f'{kind} v{vd},v{va},v{vb}', vx(base, vd, va, vb))
    for kind, base in [('vcmpgtsb', 0x10000306), ('vcmpgtsw', 0x10000386), ('vcmpgtuw', 0x10000286)]:
        add(kind, f'{kind} v{vd},v{va},v{vb}', vx(base, vd, va, vb))
        add(kind + '.', f'{kind}. v{vd},v{va},v{vb}', vx(base, vd, va, vb) | 0x400)
    for simm in (0, 1, 15, 16, 31):       # SIMM de 5 bits con signo: 16 -> -16, 31 -> -1
        add('vspltish', f'vspltish v{vd},{simm if simm < 16 else simm - 32}', vx(0x1000034C, vd, simm, 0))
    for uimm in (0, 1, 8, 31):
        add('vctuxs', f'vctuxs v{vd},v{vb},{uimm}', vx(0x1000038A, vd, uimm, vb))
    add('vsel', f'vsel v{vd},v{va},v{vb},v{(vd + 1) % 32}', vx(0x1000002A, vd, va, vb) | (((vd + 1) % 32) << 6))
    for sh in (0, 1, 5, 15):
        add('vsldoi', f'vsldoi v{vd},v{va},v{vb},{sh}', vx(0x1000002C, vd, va, vb) | (sh << 6))
for vd, va, vb in VV128:
    for kind, base in [('vnor128', 0x14000290), ('vsro128', 0x140003D0), ('vpkswss128', 0x14000280), ('vsel128', 0x14000350)]:
        add(kind, f'{kind} v{vd},v{va},v{vb}', vx128(base, vd, va, vb))
    for uimm in (0, 1, 8, 31):
        add('vcfpuxws128', f'vcfpuxws128 v{vd},v{vb},{uimm}', vx128_3(0x18000270, vd, vb, uimm))
    for sh in (0, 1, 5, 15):
        add('vsldoi128', f'vsldoi128 v{vd},v{va},v{vb},{sh}', vx128_5(0x10000010, vd, va, vb, sh))
    # vupkd3d128: tipo en IMM>>2 (5 = FLOAT16_4); IMM=20
    add('vupkd3d128_5', f'vupkd3d128 v{vd},v{vb},20', vx128_3(0x180007F0, vd, vb, 20))
    for typ, kind in ((3, 'vpkd3d128_3'), (5, 'vpkd3d128_5')):
        for pack in (1, 2, 3):
            for shift in (0, 1, 2, 3):
                add(kind, f'vpkd3d128 v{vd},v{vb},{typ},{pack},{shift}', vx128_4(0x18000610, vd, vb, (typ << 2) | pack, shift))
# palabras del juego decodificadas como mulchw. por binutils (VX128_5, SH=5): vD=9/6, vA=13/12, vB=59
add('vsldoi128', 'vsldoi128 v9,v13,v59,5   (0x112DD951, game @83088AE0)', 0x112DD951)
add('vsldoi128', 'vsldoi128 v6,v12,v59,5   (0x10CCD951, game @83088C0C)', 0x10CCD951)


# ---------------- ELF ----------------
def build_elf(funcs):
    text = bytearray()
    addrs = []
    for i, words in enumerate(funcs):
        off = i * STRIDE
        text.extend(b'\0' * (off - len(text)))
        addrs.append(BASE + off)
        for w in words:
            text.extend(struct.pack('>I', w))
        text.extend(struct.pack('>I', BLR))
    text.extend(b'\0' * ((-len(text)) % 16 or 16))
    shstr = b'\0.text\0.shstrtab\0'
    ehsize, phsize, shsize = 52, 32, 40
    text_off = ehsize + phsize
    shstr_off = text_off + len(text)
    sh_off = (shstr_off + len(shstr) + 3) & ~3
    eh = bytearray()
    eh += b'\x7fELF' + bytes([1, 2, 1, 0]) + b'\0' * 8
    eh += struct.pack('>HHIIIIIHHHHHH', 2, 20, 1, BASE, ehsize, sh_off, 0, ehsize, phsize, 1, shsize, 3, 2)
    ph = struct.pack('>IIIIIIII', 1, text_off, BASE, BASE, len(text), len(text), 5, 16)
    out = bytearray(eh) + ph + text + shstr
    out += b'\0' * (sh_off - len(out))
    out += b'\0' * shsize
    out += struct.pack('>IIIIIIIIII', 1, 1, 0x6, BASE, text_off, len(text), 0, 0, 16, 0)
    out += struct.pack('>IIIIIIIIII', 7, 3, 0, 0, shstr_off, len(shstr), 0, 0, 1, 0)
    return bytes(out), addrs


def xenia_pack_table():
    src = open(os.path.join(RTDIR, 'ref', 'ppc_emit_altivec.cc'), encoding='utf-8').read()
    blk = src[src.index('int InstrEmit_vpkd3d128'):src.index('int InstrEmit_vupkd3d128')]
    ctl = blk[blk.index('uint32_t control = kIdentityPermuteMask'):]
    tab = {}
    for pm in re.finditer(r'case (\d):\s*(?://[^\n]*)?\s*(?://[^\n]*)?\s*switch \(shift\) \{(.*?)\n      \}\s*break;', ctl, re.S):
        pack = int(pm.group(1))
        for sm in re.finditer(r'case (\d):\s*control = MakePermuteMask\(([^)]*)\);', pm.group(2)):
            tab[(pack, int(sm.group(1)))] = [int(x) for x in sm.group(2).split(',')]
    assert len(tab) == 12, tab
    return tab


def main():
    shutil.rmtree(work, ignore_errors=True)
    os.makedirs(os.path.join(work, 'src'))
    os.makedirs(os.path.join(work, 'dst'))
    elf, addrs = build_elf([w for _, _, w in tests])
    open(os.path.join(work, 'src', 'cputest.o'), 'wb').write(elf)
    r = subprocess.run([exe, os.path.join(work, 'src'), os.path.join(work, 'dst')], capture_output=True, text=True, errors='ignore')
    log = r.stdout + r.stderr
    open(os.path.join(work, 'recomp.log'), 'w').write(log)
    bad = [l for l in log.splitlines() if 'unimplemented' in l or 'Unrecognized' in l or 'Unable to decode' in l]
    if bad:
        print('\n'.join(bad[:20]))
        sys.exit('XenonRecomp: instrucciones sin soporte en las pruebas')
    gen = open(os.path.join(work, 'dst', 'cputest.cpp'), encoding='utf-8').read()
    hdr = ('// GENERADO por runtime/recomp/cputest/gen_cputest.py (no editar). Codigo producido por XenonRecomp parcheado para\n'
           '// palabras de instruccion sinteticas; no contiene codigo del juego.\n'
           '#include "cpu_selftest.h"\n')
    funcs = re.findall(r'PPC_FUNC_IMPL\(__imp__(cputest_[0-9A-F]+)\)', gen)
    expect = ['cputest_%X' % a for a in addrs]
    missing = [e for e in expect if e not in funcs]
    if missing:
        sys.exit('faltan funciones recompiladas: %s' % missing[:5])
    out = [hdr, gen, '\n']
    for e in expect:
        out.append('PPC_EXTERN_FUNC(%s);\n' % e)
    out.append('\nconst CpuTest g_cpu_tests[] = {\n')
    for (kind, text, words), a, e in zip(tests, addrs, expect):
        out.append('    { "%s", "%s", 0x%08XU, 0x%XU, %s, %d },\n' % (kind, text.replace('"', "'"), words[0], a, e, len(words)))
    out.append('};\nconst size_t g_cpu_tests_count = sizeof(g_cpu_tests) / sizeof(g_cpu_tests[0]);\n')
    os.makedirs(outsrc, exist_ok=True)
    open(os.path.join(outsrc, 'cpu_tests_gen.cpp'), 'w', encoding='utf-8', newline='\n').write(''.join(out))
    NL = chr(10)
    m = ['// GENERADO por gen_cputest.py: punteros a los registros de PPCContext.' + NL,
         'static void BuildCtxMap(PPCContext& c, PPCRegister** G, PPCRegister** F, PPCVRegister** V, PPCCRRegister** CR) {' + NL]
    for i in range(32):
        m.append('    G[%d] = &c.r%d; F[%d] = &c.f%d;' % (i, i, i, i) + NL)
    for i in range(128):
        m.append('    V[%d] = &c.v%d;' % (i, i) + NL)
    for i in range(8):
        m.append('    CR[%d] = &c.cr%d;' % (i, i) + NL)
    m.append('}' + NL)
    open(os.path.join(outsrc, 'cpu_ctx_map.inc'), 'w', encoding='utf-8', newline=NL).write(''.join(m))
    tab = xenia_pack_table()
    inc = ['// GENERADO por gen_cputest.py a partir de runtime/ref/ppc_emit_altivec.cc (Xenia, InstrEmit_vpkd3d128).\n',
           '// kXeniaPackCtl[pack][shift] = MakePermuteMask(x_sel,x_idx,y_sel,y_idx,z_sel,z_idx,w_sel,w_idx); sel 0 = VD previo, 1 = nuevo.\n',
           'static const int kXeniaPackCtl[4][4][8] = {\n    {{0}},\n']
    for pack in (1, 2, 3):
        inc.append('    {')
        inc.append(', '.join('{%s}' % ','.join(map(str, tab[(pack, s)])) for s in range(4)))
        inc.append('},\n')
    inc.append('};\n')
    open(os.path.join(outsrc, 'cpu_xenia_tables.inc'), 'w', encoding='utf-8', newline='\n').write(''.join(inc))
    print('pruebas:', len(tests), ' funciones recompiladas:', len(funcs), ' salida:', outsrc)


if __name__ == '__main__':
    main()
