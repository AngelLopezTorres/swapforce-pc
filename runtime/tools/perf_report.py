"""M8: simboliza el informe de --perf con el mapa del enlazador (SwapForceRT.map).
Uso: python -I perf_report.py <perf_report.txt> <SwapForceRT.map> [top=40]
Salida: hilos por ciclos, top exclusivo global, top inclusivo, top por hilo (texto)."""
import bisect
import re
import sys

BASE = 0x140000000


def load_map(path):
    syms = []
    rx = re.compile(r'^\s*[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+(\S+)\s+([0-9a-fA-F]{16})\s+(?:f\s+)?(\S+)?')
    with open(path, encoding='latin-1') as f:
        for line in f:
            m = rx.match(line)
            if not m:
                continue
            va = int(m.group(2), 16)
            if va < BASE:
                continue
            syms.append((va - BASE, demangle(m.group(1)), m.group(3) or ''))
    syms.sort()
    return syms


def demangle(n):
    if n.startswith('?'):
        m = re.match(r'\?([^@]+)@(.*)', n)
        if m:
            name = m.group(1)
            rest = m.group(2)
            ns = []
            for part in rest.split('@'):
                if not part or part.startswith('Y') or part.startswith('Q') or part.startswith('A') or part.startswith('U'):
                    break
                ns.append(part)
            return '::'.join(reversed(ns)) + ('::' if ns else '') + name
    return n


def main():
    rep, mp = sys.argv[1], sys.argv[2]
    top = int(sys.argv[3]) if len(sys.argv) > 3 else 40
    syms = load_map(mp)
    keys = [s[0] for s in syms]

    def sym(rva):
        i = bisect.bisect_right(keys, rva) - 1
        if i < 0:
            return '?%08X' % rva
        s = syms[i]
        return '%s+0x%X [%s]' % (s[1], rva - s[0], s[2])

    def fn(rva):
        i = bisect.bisect_right(keys, rva) - 1
        return syms[i][1] if i >= 0 else '?%08X' % rva

    mods = {}
    threads = []
    excl, incl, inclwall, exclmod = {}, {}, {}, {}
    cur = None
    head = []
    for line in open(rep, encoding='utf-8', errors='replace'):
        p = line.split()
        if not p:
            continue
        if p[0] in ('duracion_s', 'zona'):
            head.append(line.rstrip())
        elif p[0] == 'modulo':
            mods[int(p[1])] = p[2]
        elif p[0] == 'hilo':
            d = dict(x.split('=', 1) for x in re.findall(r'(\w+=(?:"[^"]*"|\S+))', line))
            cur = {'tid': d['tid'], 'name': d['nombre'].strip('"'), 'cyc': int(d['ciclos']), 'busy': int(d['ocupadas']),
                   'idle': int(d['inactivas']), 'top': [], 'incl': []}
            threads.append(cur)
        elif p[0] == 'hexcl':
            if p[1] == 'mod':
                cur['top'].append(('[%s]' % mods.get(int(p[2]), p[2]), int(p[3])))
            else:
                cur['top'].append((fn(int(p[2], 16)), int(p[3])))
        elif p[0] == 'hincl':
            cur['incl'].append((fn(int(p[2], 16)), int(p[3])))
        elif p[0] == 'excl':
            k = fn(int(p[1], 16))
            excl[k] = excl.get(k, 0) + int(p[2])
        elif p[0] == 'exclmod':
            exclmod['[%s]' % mods.get(int(p[1]), p[1])] = int(p[2])
        elif p[0] == 'incl':
            k = fn(int(p[1], 16))
            incl[k] = max(incl.get(k, 0), int(p[2]))
        elif p[0] == 'inclwall':
            k = fn(int(p[1], 16))
            inclwall[k] = max(inclwall.get(k, 0), int(p[2]))
    for h in head:
        print(h)
    total_busy = sum(t['busy'] for t in threads) or 1
    print('\n== Hilos (ciclos de CPU; muestras ocupadas) ==')
    for t in sorted(threads, key=lambda t: -t['cyc'])[:40]:
        print('%-36s tid=%-6s Gciclos=%8.2f ocupadas=%7d (%5.1f%%) inactivas=%7d' % (t['name'][:36], t['tid'], t['cyc'] / 1e9, t['busy'],
                                                                                   100.0 * t['busy'] / total_busy, t['idle']))
    allex = dict(excl)
    allex.update(exclmod)
    print('\n== Exclusivo global (muestras ocupadas) ==')
    for k, v in sorted(allex.items(), key=lambda kv: -kv[1])[:top]:
        print('%7d %5.1f%%  %s' % (v, 100.0 * v / total_busy, k))
    print('\n== Inclusivo global (muestras ocupadas que contienen la funcion) ==')
    for k, v in sorted(incl.items(), key=lambda kv: -kv[1])[:top * 2]:
        print('%7d %5.1f%%  %s' % (v, 100.0 * v / total_busy, k))
    print('\n== Por hilo (top exclusivo; hilos con >1% de muestras) ==')
    for t in sorted(threads, key=lambda t: -t['busy']):
        if t['busy'] < total_busy * 0.01:
            continue
        print('-- %s (tid %s) ocupadas=%d' % (t['name'], t['tid'], t['busy']))
        agg = {}
        for k, v in t['top']:
            agg[k] = agg.get(k, 0) + v
        for k, v in sorted(agg.items(), key=lambda kv: -kv[1])[:15]:
            print('   %7d %5.1f%%  %s' % (v, 100.0 * v / max(t['busy'], 1), k))
        print('   [inclusivo]')
        for k, v in t['incl'][:30]:
            print('   %7d %5.1f%%  %s' % (v, 100.0 * v / max(t['busy'], 1), k))
    print('\n== Inclusivo en espera (muestras inactivas) ==')
    for k, v in sorted(inclwall.items(), key=lambda kv: -kv[1])[:30]:
        print('%7d  %s' % (v, k))


if __name__ == '__main__':
    main()
