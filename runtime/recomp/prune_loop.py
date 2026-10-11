#!/usr/bin/env python3
"""Ajusta las funciones adicionales de runtime/recomp/SwapForce.toml hasta que XenonRecomp no emita errores:
  - si un 'switch' de una funcion adicional salta fuera de ella: se amplia su tamano hasta cubrir sus etiquetas
    (extents.txt); si aun asi falla, el candidato se descarta (pruned.txt);
  - si una funcion contiene un salto/llamada a un destino sin simbolo ('// ERROR T'): se descarta el candidato
    culpable (la propia funcion si es candidata; si no, el candidato que la recorta).
Los candidatos descartados o ampliados se guardan en pruned.txt / extents.txt (leidos por make_recomp_config.py)."""
import bisect
import glob
import os
import re
import subprocess
import sys

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..'))
exe = os.path.join(root, 'tools', 'XenonRecomp', 'out', 'XenonRecomp', 'XenonRecomp.exe')
ctx = os.path.join(root, 'tools', 'XenonRecomp', 'XenonUtils', 'ppc_context.h')
pruned_path = os.path.join(here, 'pruned.txt')
ext_path = os.path.join(here, 'extents.txt')
pruned = set(int(x, 16) for x in open(pruned_path).read().split()) if os.path.exists(pruned_path) else set()
extents = {}
if os.path.exists(ext_path):
    for ln in open(ext_path):
        if ln.strip():
            a_, s_ = ln.split()
            extents[int(a_, 16)] = int(s_, 16)
extended_once = set(extents)

SW = re.compile(r'Switch case at ([0-9A-F]+) is trying to jump outside function: ([0-9A-F]+) \(function ([0-9A-F]+) size ([0-9A-F]+)\)')


def run():
    subprocess.run([sys.executable, os.path.join(here, 'make_recomp_config.py')], check=True, capture_output=True)
    cfg = open(os.path.join(here, 'SwapForce.toml')).read()
    part = cfg.split('# --- funciones adicionales')[1]
    allfn = set(int(a, 16) for a in re.findall(r'address = 0x([0-9A-F]+)', part))
    cands = set(int(a, 16) for a in re.findall(r'address = 0x([0-9A-F]+)', part.split('# --- funciones inferidas')[1]))
    out = os.path.join(root, 'runtime', 'ppc_gen')
    for f in glob.glob(os.path.join(out, '*')):
        os.remove(f)
    r = subprocess.run([exe, os.path.join(here, 'SwapForce.toml'), ctx], capture_output=True, text=True, errors='ignore')
    log = r.stdout + r.stderr
    open(os.path.join(here, 'recomp_last.txt'), 'w').write(log)
    return allfn, cands, log, out


for rnd in range(1, 60):
    allfn, cands, log, out = run()
    mapping = sorted(int(x, 16) for x in re.findall(r'\{ 0x([0-9A-F]+), ', open(os.path.join(out, 'ppc_func_mapping.cpp')).read()))
    bad = set()
    grew = False
    maxlabel = {}
    for m in SW.finditer(log):
        f, lab = int(m.group(3), 16), int(m.group(2), 16)
        maxlabel[f] = max(maxlabel.get(f, 0), lab)
    for f, lab in maxlabel.items():
        if f not in allfn:
            continue
        if f not in extended_once:
            nb = [x for x in mapping if x > lab]
            end = nb[0] if nb else lab + 4
            extents[f] = min(end - f, 0x8000)
            extended_once.add(f)
            grew = True
        elif f in cands:
            bad.add(f)
    for fn in glob.glob(os.path.join(out, 'ppc_recomp.*.cpp')):
        cur = None
        for line in open(fn, encoding='utf-8'):
            mm = re.match(r'PPC_FUNC_IMPL\(__imp__sub_([0-9A-F]+)\)', line)
            if mm:
                cur = int(mm.group(1), 16)
            elif '// ERROR ' in line and cur is not None:
                i = bisect.bisect_right(mapping, cur)
                nxt = mapping[i] if i < len(mapping) else None
                if nxt in cands:
                    bad.add(nxt)      # candidato que recorta a la funcion con error
                elif cur in cands:
                    bad.add(cur)
    open(ext_path, 'w').write('\n'.join('%X %X' % kv for kv in sorted(extents.items())) + '\n')
    print('ronda', rnd, 'candidatos', len(cands), 'ampliados', len(extents), 'descartar', len(bad), flush=True)
    if not bad and not grew:
        break
    pruned |= bad
    open(pruned_path, 'w').write('\n'.join('%X' % x for x in sorted(pruned)) + '\n')
print('descartados', len(pruned), 'ampliados', len(extents))
