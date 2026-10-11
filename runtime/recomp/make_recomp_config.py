#!/usr/bin/env python3
"""Genera runtime/recomp/SwapForce.toml (configuracion de XenonRecomp para runtime/ppc_gen).

Entradas (no se modifican): analysis/SwapForce.toml, analysis/image.bin, ppc/ppc_func_mapping.cpp.
Salida: SwapForce.toml = analysis/SwapForce.toml con rutas ajustadas
        + funciones adicionales (destinos de saltos sin simbolo, 'ERROR addr' de la primera pasada)
        + funciones inferidas por relleno de ceros entre funciones (ver abajo).

Motivo: XenonRecomp solo recompila funciones de .pdata, destinos de 'bl' y las del TOML; las funciones
alcanzadas unicamente por puntero (vtables, callbacks, tablas) no existen en ppc/ y una llamada indirecta
a ellas termina en 'LLAMADA SIN RESOLVER' en el runtime.

Candidatos: direccion D alineada a 4, dentro de .text, no inicio de funcion conocido, con palabra previa
0x00000000 (relleno entre funciones del compilador) precedida por una instruccion que termina el flujo
(blr, bctr o b incondicional). XenonRecomp (barrido lineal) fusiona esas funciones con la anterior.
Verificado: 825209E0 (destino de la primera llamada sin resolver) cumple el criterio."""
import bisect
import os
import re
import struct
import sys

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..'))
A = os.path.join(root, 'analysis')
B = 0x82000000
CB, CE = 0x823E0000, 0x823E0000 + 0xCC9EF4

img = open(os.path.join(A, 'image.bin'), 'rb').read()
words = struct.unpack('>%dI' % (len(img) // 4), img[:len(img) // 4 * 4])
known = set(int(x, 16) for x in re.findall(r'\{ 0x([0-9A-F]+), ', open(os.path.join(root, 'ppc', 'ppc_func_mapping.cpp')).read()))

pd_va, pd_sz = 0x82362600 - B, 0x6a048
pd = sorted((a, ((d >> 8) & 0x3FFFFF) * 4) for a, d in
            (struct.unpack('>II', img[pd_va + 8 * i:pd_va + 8 * i + 8]) for i in range(pd_sz // 8)))
pdata = set(a for a, _ in pd)

manual = [0x8246C700, 0x826917A0, 0x82E926E8, 0x82E92710, 0x82E92820, 0x82FFB38C, 0x82FFB45C, 0x82FFB624, 0x82FFB6F4]

textoff, textend = (CB - B) // 4, (CE - B) // 4
cand = set()
for i in range(textoff + 1, textend):
    if words[i] != 0 and words[i - 1] == 0:
        j = i - 1
        while words[j] == 0:
            j -= 1
        w = words[j]
        term = w in (0x4E800020, 0x4E800420) or ((w >> 26) == 18 and not (w & 1))
        a = B + i * 4
        if term and a not in known and a not in pdata:
            cand.add(a)
# Evidencia por puntero: palabras big-endian de secciones no ejecutables o inmediatos lis+addi/lis+ori
# que valen D; D no es destino de ningun salto directo (b/bc/bl) ni etiqueta de tabla de saltos.
switch_labels = set(int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]{8}),', open(os.path.join(A, 'SwapForce_switch_tables.toml')).read()))
# Tablas adicionales (find_missing_switch.py): sus etiquetas tampoco son inicios de funcion.
_extra = os.path.join(here, 'extra_switch_tables.toml')
if os.path.exists(_extra):
    switch_labels |= set(int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]{8}),', open(_extra).read()))
# Direcciones de datos de las tablas de saltos absolutas (lis+addi+rlwinm+lwzx+mtctr+bctr): no son funciones, aunque
# se carguen con lis+addi (evidencia de puntero falsa) y sigan a un bctr.
table_addrs = set()
for _i in range(textoff_ := (CB - B) // 4, (CE - B) // 4 - 6):
    _w0, _w1, _w2, _w3, _w4, _w5 = words[_i:_i + 6]
    if (_w0 >> 26) != 15 or ((_w0 >> 16) & 31) != 0 or _w5 != 0x4E800420 or (_w4 & 0xFC1FFFFF) != 0x7C0903A6:
        continue
    _ra = (_w0 >> 21) & 31
    _ad = None
    if (_w1 >> 26) == 14 and ((_w1 >> 16) & 31) == _ra and (_w2 >> 26) == 21 and (_w3 >> 26) == 31: _ad = _w1
    elif (_w1 >> 26) == 21 and (_w2 >> 26) == 14 and ((_w2 >> 16) & 31) == _ra and (_w3 >> 26) == 31: _ad = _w2
    if _ad is None:
        continue
    _s16 = lambda v: v - 0x10000 if v & 0x8000 else v
    table_addrs.add(((_s16(_w0 & 0xFFFF) << 16) + _s16(_ad & 0xFFFF)) & 0xFFFFFFFF)
branch_targets = set()
for a_ in range(CB, CE, 4):
    w = words[(a_ - B) // 4]
    op = w >> 26
    if op == 18:
        off = w & 0x03FFFFFC
        if off & 0x02000000:
            off -= 0x04000000
        branch_targets.add((a_ + off) & 0xFFFFFFFF)
    elif op == 16:
        off = w & 0xFFFC
        if off & 0x8000:
            off -= 0x10000
        branch_targets.add((a_ + off) & 0xFFFFFFFF)
ptrs = set()
for i, w in enumerate(words):
    if textoff <= i < textend:
        continue
    if CB <= w < CE and w % 4 == 0:
        ptrs.add(w)
for i in range(textoff, textend - 1):
    w = words[i]
    if (w >> 26) != 15 or ((w >> 16) & 0x1F) != 0:
        continue
    rd = (w >> 21) & 0x1F
    hi = (w & 0xFFFF) << 16
    for j in range(i + 1, min(i + 8, textend)):
        v = words[j]
        val = None
        if (v >> 26) == 14 and ((v >> 16) & 0x1F) == rd:
            lo = v & 0xFFFF
            lo = lo - 0x10000 if lo & 0x8000 else lo
            val = (hi + lo) & 0xFFFFFFFF
        elif (v >> 26) == 24 and ((v >> 21) & 0x1F) == rd:
            val = hi | (v & 0xFFFF)
        if val and CB <= val < CE and val % 4 == 0:
            ptrs.add(val)
n_data = 0
for a_ in ptrs:
    i = (a_ - B) // 4
    p_ = words[i - 1]
    term = p_ in (0x4E800020, 0x4E800420, 0) or ((p_ >> 26) == 18 and not (p_ & 1))
    if term and words[i] != 0 and a_ not in known and a_ not in pdata and a_ not in branch_targets and a_ not in switch_labels and a_ not in table_addrs:
        cand.add(a_)
        n_data += 1
cand -= set(manual)
cand -= table_addrs
pruned_path = os.path.join(here, 'pruned.txt')   # candidatos descartados por prune_loop.py
if os.path.exists(pruned_path):
    cand -= set(int(x, 16) for x in open(pruned_path).read().split())

ext_path = os.path.join(here, 'extents.txt')   # ampliaciones de tamano (prune_loop.py): 'DIRECCION TAMANO' en hex
overrides = {}
if os.path.exists(ext_path):
    for ln in open(ext_path):
        if ln.strip():
            a_, s_ = ln.split()
            overrides[int(a_, 16)] = int(s_, 16)
bounds = sorted(known | pdata | cand | set(manual))
base_cfg = open(os.path.join(A, 'SwapForce.toml')).read()
base_cfg = base_cfg.replace('"../iso_extract/default.xex"', '"../../iso_extract/default.xex"')
base_cfg = base_cfg.replace('"../ppc"', '"../ppc_gen"')
# Ayudantes de guardado/restauracion de FPR (f14-f31) del XDK: savefpr_14 = stfd f14,-144(r1) ... stfd f31,-8(r1); blr;
# restfpr_14 = lfd f14,-144(r1) ... lfd f31,-8(r1); blr (verificado en image.bin: 82FF3FD0 y 82FF401C). Sin estas direcciones
# XenonRecomp parte la secuencia en funciones de una instruccion y las entradas intermedias (f30) dejan de guardar/restaurar.
base_cfg = base_cfg.replace('savegprlr_14_address = 0x82FF2D00', 'savegprlr_14_address = 0x82FF2D00' + chr(10) + 'restfpr_14_address = 0x82FF401C' + chr(10) + 'savefpr_14_address = 0x82FF3FD0')
out = [base_cfg.rstrip(), '', '# --- funciones adicionales: destinos sin simbolo (ERROR addr en la primera pasada) ---']
for t in manual:
    nb = bounds[bisect.bisect_right(bounds, t)]
    out.append('[[main.functions]]\naddress = 0x%X\nsize = 0x%X\n' % (t, overrides.get(t, nb - t)))
out.append('# --- funciones inferidas por relleno de ceros entre funciones (%d) ---' % len(cand))
for t in sorted(cand):
    nb = bounds[bisect.bisect_right(bounds, t)]
    out.append('[[main.functions]]\naddress = 0x%X\nsize = 0x%X\n' % (t, overrides.get(t, min(nb - t, 0x10000))))
open(os.path.join(here, 'SwapForce.toml'), 'w', newline='\n').write('\n'.join(out) + '\n')
print('manual=%d candidatos_datos=%d candidatos_total=%d' % (len(manual), n_data, len(cand)))
