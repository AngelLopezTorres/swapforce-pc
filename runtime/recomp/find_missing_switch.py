#!/usr/bin/env python3
"""Tablas de saltos absolutas (lis/addi/rlwinm/lwzx/mtctr/bctr y la variante con rlwinm antes de addi) que
XenonAnalyse no detecto (ScanTable no encuentra el cmplwi/bgt dentro de 32 instrucciones: tablas anidadas).
Criterio de inclusion: patron completo + palabras consecutivas de la tabla dentro de .text (4 alineadas) +
un cmplwi/cmpwi previo (hasta 400 instrucciones atras) con inmediato N-1, donde N es el numero de entradas.
Variante 2 (indice acotado por mascara, sin cmplwi): el registro indice proviene de rlwinm rX,rY,0,mb,31 (valores 0..2^(32-mb)-1),
opcionalmente seguido de addi rX,rX,K; se acepta si ese maximo vale N-1 (N = numero de entradas validas consecutivas).
Salida: extra_switch_tables.toml (formato [[switch]] de XenonAnalyse) y lista de no confirmadas."""
import os, re, struct, sys
here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..'))
img = open(os.path.join(root, 'analysis', 'image.bin'), 'rb').read()
B = 0x82000000
words = struct.unpack('>%dI' % (len(img) // 4), img[:len(img) // 4 * 4])
CB, CE = 0x823E0000, 0x823E0000 + 0xCC9EF4
base_toml = os.path.join(here, 'SwapForce_switch_tables.toml')
bases = set(int(x, 16) for x in re.findall(r'base = 0x([0-9A-Fa-f]+)', open(base_toml).read()))
extra_path = os.path.join(here, 'extra_switch_tables.toml')
if os.path.exists(extra_path):
    bases |= set(int(x, 16) for x in re.findall(r'base = 0x([0-9A-Fa-f]+)', open(extra_path).read()))
op = lambda w: w >> 26
xo = lambda w: (w >> 1) & 0x3FF
s16 = lambda v: v - 0x10000 if v & 0x8000 else v
def index_bound(reg, pos, depth=0):
    """Maximo valor del registro `reg` justo antes de la instruccion `pos` (indice de palabra), o None si no se puede acotar.
    Solo entiende rlwinm con sh=0 y me=31 (mascara de los bits bajos) y addi con constante >= 0 sobre un registro acotado."""
    if depth > 3: return None
    for j in range(pos - 1, pos - 40, -1):
        w = words[j]
        if op(w) == 21 and ((w >> 16) & 31) == reg:
            if ((w >> 11) & 31) == 0 and ((w >> 1) & 31) == 31 and ((w >> 6) & 31) > 0:
                return (1 << (32 - ((w >> 6) & 31))) - 1
            return None
        if op(w) == 14 and ((w >> 21) & 31) == reg:
            src, k = (w >> 16) & 31, s16(w & 0xFFFF)
            if src == 0 or k < 0: return None
            b = index_bound(src, j, depth + 1)
            return None if b is None else b + k
    return None
out, unconfirmed = [], []
for i in range((CB - B) // 4, (CE - B) // 4 - 6):
    w0, w1, w2, w3, w4, w5 = words[i:i + 6]
    if op(w0) != 15 or ((w0 >> 16) & 31) != 0 or w5 != 0x4E800420 or (w4 & 0xFC1FFFFF) != 0x7C0903A6: continue
    ra = (w0 >> 21) & 31
    isaddi = lambda w: op(w) == 14 and ((w >> 16) & 31) == ra
    isrl = lambda w: op(w) == 21
    islwzx = lambda w: op(w) == 31 and xo(w) == 23
    if isaddi(w1) and isrl(w2) and islwzx(w3): ad, rl = w1, w2
    elif isrl(w1) and isaddi(w2) and islwzx(w3): ad, rl = w2, w1
    else: continue
    a = B + i * 4
    if a in bases: continue
    sh, mb, me = (rl >> 11) & 31, (rl >> 6) & 31, (rl >> 1) & 31
    if (sh, mb, me) != (2, 0, 29): unconfirmed.append((a, 'rlwinm no es slwi 2')); continue
    rs = (rl >> 21) & 31
    tab = ((s16(w0 & 0xFFFF) << 16) + s16(ad & 0xFFFF)) & 0xFFFFFFFF
    labels = []
    while True:
        v = words[(tab - B) // 4 + len(labels)]
        if v % 4 or not (CB <= v < CE): break
        labels.append(v)
    n = len(labels)
    conf = False
    for k in range(1, 400):
        w = words[i - k]
        if op(w) in (10, 11):
            if (w & 0xFFFF) == n - 1: conf = True
            break
    if not conf:
        # busqueda ampliada: cualquier cmplwi/cmpwi con N-1 en las 400 instrucciones previas
        conf = any(op(words[i - k]) in (10, 11) and (words[i - k] & 0xFFFF) == n - 1 for k in range(1, 215))
    if not conf:
        b = index_bound(rs, i + 1 if isrl(w1) else i + 2)   # posicion del rlwinm (el indice rs se lee ahi)
        conf = b is not None and b == n - 1
    if not conf: unconfirmed.append((a, 'n=%d sin cmplwi N-1 cercano' % n)); continue
    out.append((a, rs, labels))
with open(extra_path, 'w') as f:
    f.write('# Tablas de saltos no detectadas por XenonAnalyse (find_missing_switch.py)\n')
    for a, rs, labels in out:
        f.write('\n[[switch]]\nbase = 0x%X\nr = %d\ndefault = 0x%X\nlabels = [\n' % (a, rs, labels[-1]))
        for l in labels: f.write('    0x%X,\n' % l)
        f.write(']\n')
print('anadidas:', len(out), 'no confirmadas:', len(unconfirmed))
for a, why in unconfirmed: print('  %08X %s' % (a, why))
