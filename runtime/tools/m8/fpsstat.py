"""M8: resumen de una ejecucion con --perf.
Uso: python -I fpsstat.py <salida_stdout> [ventana_s=30]
Lineas usadas: 'PERF t=N ... fps_vdswap=X', 'PERF zona ...', 'PERF cp_wait ...', 'GPU resumen'."""
import re
import sys


def main():
    path = sys.argv[1]
    win = int(sys.argv[2]) if len(sys.argv) > 2 else 30
    fps = []
    other = []
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'PERF t=(\d+) .*fps_vdswap=([\d.]+)', line)
        if m:
            fps.append(float(m.group(2)))
        elif line.startswith('PERF zona') or line.startswith('PERF cp_wait') or line.startswith('GPU resumen'):
            other.append(line.rstrip())
    n = len(fps)
    if not n:
        print('sin lineas PERF')
        return
    tot = sum(fps)
    best = 0.0
    bestStart = 0
    minInBest = 0.0
    for i in range(0, max(1, n - win + 1)):
        w = fps[i:i + win]
        if len(w) < win:
            break
        m = sum(w) / len(w)
        if m > best:
            best, bestStart, minInBest = m, i + 1, min(w)
    sust = 0
    run = 0
    for v in fps:
        run = run + 1 if v >= 20.0 else 0
        sust = max(sust, run)
    print('segundos=%d swaps_total=%.0f media=%.2f fps; media t>=10: %.2f; mejor ventana %ds desde t=%d: media=%.2f min=%.2f; max racha >=20 fps: %d s' % (
        n, tot, tot / n, sum(fps[9:]) / max(1, len(fps[9:])), win, bestStart, best, minInBest, sust))
    print('fps por segundo:', ' '.join('%.0f' % v for v in fps))
    for o in other:
        print(o)


if __name__ == '__main__':
    main()
