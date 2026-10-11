"""Iterate: run XenonRecomp, find switch sites whose cases fall outside their function, append manual function extents."""
import re, subprocess, struct, bisect, os

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
A = os.path.join(ROOT, 'analysis')
TOML = os.path.join(A, 'SwapForce.toml')
EXE = os.path.join(ROOT, 'tools', 'XenonRecomp', 'out', 'XenonRecomp', 'XenonRecomp.exe')
CTX = os.path.join(ROOT, 'tools', 'XenonRecomp', 'XenonUtils', 'ppc_context.h')
B = 0x82000000
img = open(os.path.join(A, 'image.bin'), 'rb').read()

pd_va, pd_sz = 0x82362600 - B, 0x6a048
_pd = sorted((a, ((d >> 8) & 0x3FFFFF) * 4) for a, d in (struct.unpack('>II', img[pd_va + 8 * i:pd_va + 8 * i + 8]) for i in range(pd_sz // 8)))
pdata = [a for a, _ in _pd]
pd_len = dict(_pd)
tx_va, tx_sz = 0x823e0000 - B, 0xc9de9c
W = struct.unpack('>%dI' % (tx_sz // 4), img[tx_va:tx_va + tx_sz // 4 * 4])
bl = set()
for i, w in enumerate(W):
    if (w >> 26) == 18 and (w & 1) and not (w & 2):
        off = w & 0x03FFFFFC
        if off & 0x02000000:
            off -= 0x04000000
        t = (B + tx_va + i * 4 + off) & 0xFFFFFFFF
        if B + tx_va <= t < B + tx_va + tx_sz:
            bl.add(t)
bound = sorted(set(pdata) | bl)
bl_sorted = sorted(bl)

def load_manual():
    t = open(TOML).read()
    return {int(a, 16): int(a, 16) + int(z, 16) for a, z in re.findall(r'address = 0x([0-9A-Fa-f]+)\s+size = 0x([0-9A-Fa-f]+)', t)}

import shutil
shutil.copy(os.path.join(A, 'SwapForce_base.toml'), TOML)   # always start from the base config

pat = re.compile(r'Switch case at ([0-9A-F]+) is trying to jump outside function: ([0-9A-F]+)')
for rnd in range(1, 9):
    r = subprocess.run([EXE, TOML, CTX], capture_output=True, text=True, errors='ignore')
    log = r.stdout + r.stderr
    open(os.path.join(A, 'recomp_iter%d.txt' % rnd), 'w').write(log)
    sites = {}
    for m in pat.finditer(log):
        sites.setdefault(int(m.group(1), 16), []).append(int(m.group(2), 16))
    ncases = sum(len(v) for v in sites.values())
    print('round', rnd, 'sites', len(sites), 'cases', ncases, flush=True)
    if not sites:
        break
    manual = load_manual()
    new = {}
    for site, labels in sorted(sites.items()):
        mx = max(labels)
        cont = [(a, e) for a, e in manual.items() if a <= site < e]
        if cont:
            a, e = cont[0]
            nb = [x for x in pdata if x > mx]
            new[a] = max(new.get(a, 0), nb[0] if nb else mx + 4)   # extend existing
            continue
        ip = bisect.bisect_right(pdata, site) - 1
        start = pdata[ip] if ip >= 0 else None
        in_pdata = start is not None and site < start + pd_len[start]
        if not in_pdata:
            # leaf function without .pdata entry: nearest bl target before the site
            ib = bisect.bisect_right(bl_sorted, site) - 1
            if ib >= 0 and (start is None or bl_sorted[ib] > start):
                start = bl_sorted[ib]
        # end: first function boundary (pdata start or bl target) after the highest case label
        nb = [x for x in (pdata if in_pdata else bound) if x > mx]
        end = nb[0] if nb else mx + 4
        new[start] = max(new.get(start, 0), end)
    # rewrite: drop old entries for addresses being extended, append new ones
    t = open(TOML).read()
    for a, end in new.items():
        if a in manual:
            t = re.sub(r'\[\[main\.functions\]\]\s+address = 0x%X\s+size = 0x[0-9A-Fa-f]+\s*' % a, '', t)
    for a, end in sorted(new.items()):
        t = t.rstrip('\n') + '\n\n[[main.functions]]\naddress = 0x%X\nsize = 0x%X\n' % (a, end - a)
    open(TOML, 'w').write(t)
    print('  wrote', len(new), 'function extents', flush=True)
