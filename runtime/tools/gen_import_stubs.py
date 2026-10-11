#!/usr/bin/env python3
"""Genera import_stubs.gen.cpp a partir de ppc/ppc_recomp_shared.h.
Uso: python gen_import_stubs.py   (rutas relativas a este fichero)"""
import os
import re

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..'))
hdr = os.path.join(root, 'ppc', 'ppc_recomp_shared.h')
out = os.path.join(here, '..', 'src', 'kernel', 'import_stubs.gen.cpp')

names = []
pat = re.compile(r'^PPC_EXTERN_FUNC\(__imp__([A-Za-z_][A-Za-z0-9_]*)\);')
with open(hdr, encoding='utf-8') as f:
    for line in f:
        m = pat.match(line)
        if m and not m.group(1).startswith('sub_'):
            names.append(m.group(1))

with open(out, 'w', encoding='utf-8', newline='\n') as f:
    f.write('// GENERADO por runtime/tools/gen_import_stubs.py. No editar.\n')
    f.write('// %d imports. Stubs debiles: una implementacion fuerte los sustituye.\n' % len(names))
    f.write('#include "import_log.h"\n\n')
    for n in names:
        f.write('GUEST_IMPORT_STUB(%s)\n' % n)
print('imports=%d -> %s' % (len(names), os.path.normpath(out)))
