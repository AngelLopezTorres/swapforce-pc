"""M8: simboliza el primer volcado de m8_stall_stacks.txt (pilas host) con llvm-symbolizer.
Uso (desde runtime\\): python -I tools/m8/stall_sym.py m8_stall_stacks.txt [n_volcado=1] [max_marcos=14]"""
import subprocess
import sys

SYM = r'C:\Program Files\LLVM\bin\llvm-symbolizer.exe'


def main():
    path = sys.argv[1]
    which = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    maxf = int(sys.argv[3]) if len(sys.argv) > 3 else 14
    dumps, cur = [], None
    for line in open(path, encoding='utf-8', errors='replace'):
        if line.startswith('=='):
            cur = {'base': int(line.split('base=')[1].split(')')[0], 16), 'mods': [], 'th': []}
            dumps.append(cur)
        elif line.startswith('modulo '):
            p = line.split()
            cur['mods'].append((int(p[1], 16), int(p[2], 16), p[3]))
        elif line.startswith('hilo '):
            head, _, rest = line.rpartition('":')
            cur['th'].append((head, [int(x, 16) for x in rest.split()]))
    d = dumps[which - 1]
    base = d['base']
    exe_hi = next((hi for lo, hi, nm in d['mods'] if lo == base), base + 0x10000000)
    addrs = sorted({a for _, fr in d['th'] for a in fr[:maxf] if base <= a < exe_hi})
    inp = '\n'.join('0x%X' % (a - base + 0x140000000 - (1 if True else 0)) for a in addrs) + '\n'
    out = subprocess.run([SYM, '--obj=build/SwapForceRT.exe', '--functions=short', '--no-inlines'], input=inp, capture_output=True, text=True).stdout
    blocks = [b.strip().split('\n') for b in out.strip().split('\n\n')]
    names = {a: (b[0] + ' ' + b[1].split('\\')[-1] if len(b) > 1 else b[0]) for a, b in zip(addrs, blocks)}
    for head, fr in d['th']:
        desc = []
        for a in fr[:maxf]:
            if a in names:
                desc.append(names[a])
            else:
                m = next((nm for lo, hi, nm in d['mods'] if lo <= a < hi), '?')
                desc.append('[%s+%X]' % (m, a - next((lo for lo, hi, nm in d['mods'] if lo <= a < hi), 0)))
        print(head)
        for x in desc:
            print('    ' + x)


if __name__ == '__main__':
    main()
