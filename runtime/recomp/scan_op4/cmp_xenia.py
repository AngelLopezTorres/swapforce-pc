#!/usr/bin/env python3
r"""Compara la decodificacion de las palabras con opcodes primarios 4, 5 y 6 (VMX/VMX128; salida de scan_op4.exe)
con la de Xenia (runtime/ref/ppc_opcode_lookup_gen.cc: arbol de decision autogenerado de ppc_opcode_table_gen.cc).
Compara (1) el nombre del opcodo y (2) los operandos (conjunto de valores segun el formato de Xenia,
runtime/ref/ppc_decode_data.h). Uso: cmp_xenia.py op4_xr.txt"""
import re, sys, collections, os
here = os.path.dirname(os.path.abspath(__file__))
src = open(os.path.join(here, '..', '..', 'ref', 'ppc_opcode_lookup_gen.cc')).read()
tab = open(os.path.join(here, '..', '..', 'ref', 'ppc_opcode_table_gen.cc')).read()


def parse(op):
    blk = src.split('  case %d:\n' % op, 1)[1].split('    PPC_DECODER_MISS;', 1)[0]
    sw = []
    for m in re.finditer(r'switch \(((?:\(ExtractBits\(code, \d+, \d+\) << \d+\)\|?)+)\) \{(.*?)\n    \}', blk, re.S):
        parts = [(int(a), int(b), int(s)) for a, b, s in re.findall(r'ExtractBits\(code, (\d+), (\d+)\) << (\d+)', m.group(1))]
        cases = {int(k, 2): v for k, v in re.findall(r'case 0b([01]+): PPC_DECODER_HIT\((\w+)\)', m.group(2))}
        sw.append((parts, cases))
    return sw


SW = {op: parse(op) for op in (4, 5, 6)}


def eb(v, a, b):
    return (v >> (32 - 1 - b)) & ((1 << (b - a + 1)) - 1)


def xenia(w):
    for parts, cases in SW.get(w >> 26, []):
        k = 0
        for a, b, s in parts:
            k |= eb(w, a, b) << s
        if k in cases:
            return cases[k]
    return 'MISS'


fmt = {n.lower(): f for n, f in re.findall(r'INSTRUCTION\(0x[0-9a-f]+, "(\w+)"\s*, (kV\w*)\s*,', tab)}


def b(w, lo, n):
    return (w >> lo) & ((1 << n) - 1)


def fields(f, w):
    if f in ('kVX', 'kVC'):
        return [b(w, 21, 5), b(w, 16, 5), b(w, 11, 5)]
    if f == 'kVA':
        return [b(w, 21, 5), b(w, 16, 5), b(w, 11, 5), b(w, 6, 5)]
    vd = b(w, 21, 5) | b(w, 2, 2) << 5
    vb = b(w, 11, 5) | b(w, 0, 2) << 5
    va = b(w, 16, 5) | b(w, 5, 1) << 5 | b(w, 10, 1) << 6
    if f in ('kVX128', 'kVX128_R'):
        return [vd, va, vb]
    if f == 'kVX128_1':
        return [vd, b(w, 16, 5), b(w, 11, 5)]
    if f == 'kVX128_2':
        return [vd, va, vb, b(w, 6, 3)]
    if f == 'kVX128_3':
        return [vd, vb, b(w, 16, 5)]
    if f == 'kVX128_4':
        return [vd, vb, b(w, 16, 5), b(w, 6, 2)]            # VD, VB, IMM, z
    if f == 'kVX128_5':
        return [vd, va, vb, b(w, 6, 4)]
    if f == 'kVX128_P':
        return [vd, vb, b(w, 16, 5) | b(w, 6, 3) << 5]      # VD, VB, PERM
    return None


tot = 0
bad = collections.defaultdict(list)
opbad = collections.Counter()
opex = {}
nchk = 0
per_op = collections.Counter()
for ln in open(sys.argv[1]):
    p = ln.split()
    w = int(p[0], 16)
    xr = p[3].lower().rstrip('.')
    xe = xenia(w).lower()
    tot += 1
    per_op[w >> 26] += 1
    if xr != xe:
        bad[(xr, xe)].append((p[0], p[1], p[2]))
        continue
    f = fmt.get(xe)
    fl = fields(f, w) if f else None
    if fl is None:
        continue
    ops = [(int(x) & 0x1F) if int(x) >= 2**31 else int(x) for x in p[4:9]]  # SIMM de 5 bits con signo (vspltis*)
    nchk += 1
    if xe in ('vmaddcfp128', 'vmaddfp128', 'vnmsubfp128'):  # binutils: VD, VA, (VB|VS=VD), (VS=VD|VB)
        d, a, bb = fl[0], fl[1], fl[2]
        exp = [d, a, d, bb] if xe == 'vmaddcfp128' else [d, a, bb, d]
        if ops[:4] != exp:
            opbad[xe] += 1
            opex.setdefault(xe, (p[0], p[1], ops[:4], exp))
        continue
    if xe == 'vpkd3d128':  # binutils: VD, VB, tipo=IMM>>2, mascara=IMM&3, desplazamiento=z
        if ops[:5] != [fl[0], fl[1], fl[2] >> 2, fl[2] & 3, fl[3]]:
            opbad[xe] += 1
            opex.setdefault(xe, (p[0], p[1], ops[:5], fl))
        continue
    if sorted(ops[:len(fl)]) != sorted(fl):
        opbad[xe] += 1
        opex.setdefault(xe, (p[0], p[1], ops[:len(fl)], fl))
print('palabras distintas:', tot, dict(per_op), ' discrepancias de nombre:', sum(len(v) for v in bad.values()))
for (xr, xe), v in sorted(bad.items(), key=lambda kv: -len(kv[1])):
    print(f'  XenonRecomp={xr:12s} Xenia={xe:12s} n={len(v):5d} ej: ' + ', '.join(f'{a}@{c}(x{d})' for a, c, d in v[:3]).replace('@', '@') if False else
          f'  XenonRecomp={xr:12s} Xenia={xe:12s} n={len(v):5d} ej: ' + ', '.join(f'{a}@{c}' for a, c, d in [(x[0], x[1], x[2]) for x in v[:3]]))
print('operandos: palabras comprobadas', nchk, ' con discrepancia (mismo nombre):', sum(opbad.values()))
for n, c in opbad.most_common():
    print(' ', n, c, opex[n])
