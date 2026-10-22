"""M8: lineas calientes (exclusivo por RIP) de un informe --perf, simbolizadas con llvm-symbolizer y el PDB.
Uso (desde runtime\\): python -I tools/m8/hotlines.py m8/perf_X.txt [N=80] [filtro]"""
import subprocess
import sys

SYM = r'C:\Program Files\LLVM\bin\llvm-symbolizer.exe'


def main():
    path = sys.argv[1]
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 80
    flt = sys.argv[3] if len(sys.argv) > 3 else None
    rows = []
    for line in open(path, encoding='utf-8', errors='replace'):
        p = line.split()
        if p and p[0] == 'excl':
            rows.append((int(p[1], 16), int(p[2])))
    rows = rows[:n]
    inp = '\n'.join('0x%X' % (0x140000000 + r) for r, _ in rows) + '\n'
    out = subprocess.run([SYM, '--obj=build/SwapForceRT.exe', '--output-style=GNU', '--functions=short'], input=inp,
                         capture_output=True, text=True).stdout.strip().split('\n')
    pairs = [(out[i], out[i + 1]) for i in range(0, len(out) - 1, 2)]
    for (rva, cnt), (fn, loc) in zip(rows, pairs):
        loc = loc.split('runtime\\src\\')[-1]
        if flt and flt not in fn and flt not in loc:
            continue
        print('%6d  %-40s %s' % (cnt, fn[:40], loc))


if __name__ == '__main__':
    main()
