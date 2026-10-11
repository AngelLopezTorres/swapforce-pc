#!/usr/bin/env python3
"""Detecta ficheros ppc/ppc_recomp.N.cpp obsoletos (restos de ejecuciones anteriores de XenonRecomp
con otro reparto de funciones) y escribe runtime/ppc_sources.cmake con la lista a excluir.
Un fichero es obsoleto si define funciones ya definidas en otros ficheros y NO figura
(completo) en ppc_func_mapping/ppc_recomp_shared.h de la ultima ejecucion.
Criterio verificable: cada funcion sub_X debe quedar definida exactamente una vez y el
conjunto debe coincidir con las PPC_EXTERN_FUNC(sub_X) de ppc_recomp_shared.h."""
import glob
import os
import re
import sys

here = os.path.dirname(os.path.abspath(__file__))
ppc = os.path.abspath(os.path.join(here, '..', 'ppc_gen'))
if not os.path.isdir(ppc):
    ppc = os.path.abspath(os.path.join(here, '..', '..', 'ppc'))
out = os.path.abspath(os.path.join(here, '..', 'ppc_sources.cmake'))

pat = re.compile(r'^PPC_FUNC_IMPL\(__imp__sub_([0-9A-F]+)\)', re.M)
files = {}
for f in glob.glob(os.path.join(ppc, 'ppc_recomp.*.cpp')):
    n = int(re.search(r'recomp\.(\d+)\.cpp', f).group(1))
    with open(f, encoding='utf-8') as fh:
        files[n] = set(pat.findall(fh.read()))

shared = set(re.findall(r'^PPC_EXTERN_FUNC\(sub_([0-9A-F]+)\);', open(os.path.join(ppc, 'ppc_recomp_shared.h'), encoding='utf-8').read(), re.M))

# La ejecucion mas reciente reparte las funciones en orden de direccion en bloques contiguos:
# se conservan, en orden de indice, los ficheros cuyo rango de direcciones continua al anterior.
keep, drop = [], []
last_max = -1
for n in sorted(files):
    s = files[n]
    if not s:
        keep.append(n); continue
    lo = min(int(x, 16) for x in s)
    hi = max(int(x, 16) for x in s)
    if lo > last_max:
        keep.append(n); last_max = hi
    else:
        drop.append(n)

defined = {}
for n in keep:
    for x in files[n]:
        defined[x] = defined.get(x, 0) + 1
dups = [x for x, c in defined.items() if c > 1]
missing = shared - set(defined)
extra = set(defined) - shared
print('keep=%d drop=%s dups=%d missing=%d extra=%d' % (len(keep), drop, len(dups), len(missing), len(extra)))
if dups or missing or extra:
    print('ERROR: el criterio no se cumple'); sys.exit(1)
with open(out, 'w', encoding='utf-8', newline='\n') as f:
    f.write('# GENERADO por tools/gen_ppc_sources.py\nset(PPC_STALE_INDICES %s)\n' % ' '.join(str(n) for n in drop))
