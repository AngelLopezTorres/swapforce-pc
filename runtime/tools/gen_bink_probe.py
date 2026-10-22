# Genera runtime/src/kernel/bink_probe_gen.inc: sondas de entrada (definiciones fuertes que sustituyen a los
# alias debiles sub_X de ppc_gen y delegan en __imp__sub_X) para todas las funciones de las secciones
# BINK (0x8307E000-0x830A9368) y RADCODE (0x830A9400-0x830A9EF4) del XEX (rangos de la tabla de secciones PE).
# Uso: python -I gen_bink_probe.py <ppc_gen/ppc_recomp_shared.h> <salida.inc> [HEX...]
# Las direcciones HEX opcionales se cuentan pero su definicion la escribe bink_probe.cpp (sondas con detalle).
# Uso actual: python -I runtime/tools/gen_bink_probe.py runtime/ppc_gen/ppc_recomp_shared.h runtime/src/kernel/bink_probe_gen.inc 830804C8
# La salida deriva del binario del juego: no se publica (.gitignore); bink_probe.cpp compila sin ella.
import re
import sys

LO, HI = 0x8307E000, 0x830A9EF4
src, out = sys.argv[1], sys.argv[2]
custom = {int(x, 16) for x in sys.argv[3:]}
funcs = []
for line in open(src, encoding='utf-8', errors='replace'):
    m = re.match(r'PPC_EXTERN_FUNC\(sub_([0-9A-F]{8})\);', line)
    if m:
        a = int(m.group(1), 16)
        if LO <= a < HI:
            funcs.append(a)
funcs.sort()
with open(out, 'w', newline='\n') as f:
    f.write('// GENERADO por runtime/tools/gen_bink_probe.py (no editar). Funciones: %d\n' % len(funcs))
    f.write('#define BINK_PROBE_N %d\n' % len(funcs))
    f.write('static const uint32_t kBinkProbeAddr[BINK_PROBE_N] = {\n')
    for i in range(0, len(funcs), 8):
        f.write('    ' + ', '.join('0x%08X' % a for a in funcs[i:i + 8]) + ',\n')
    f.write('};\n')
    for i, a in enumerate(funcs):
        f.write('extern "C" PPC_FUNC(__imp__sub_%08X);\n' % a)
        if a in custom:
            f.write('#define BINK_PROBE_IDX_%08X %d\n' % (a, i))
            continue
        f.write('PPC_FUNC(sub_%08X) { BinkHit(%d, ctx); __imp__sub_%08X(ctx, base); }\n' % (a, i, a))
print('funciones', len(funcs))
