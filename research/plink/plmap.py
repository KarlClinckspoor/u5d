"""Compare a PLINK86plus map (MAP = ULTIMA.MAP M: segments per module) with src/u5d.lnk.

    plmap.py WORKDIR [--target gog|1988] [-a]

For every object and library module: its code address (CS offset, or OVERLAY:offset) and the
DGROUP offsets of its _DATA/CONST/_BSS (and the library's other data segments) as PLINK placed
them, against the link script. Prints the mismatches (all lines with -a), then the module
order PLINK processed (the map's order)."""
import argparse, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import pl  # noqa: E402


def parse_map(path):
    """[(module, file, [(seg, cls, addr, size, ovly)])], {'sections': [(ovly, maddr)], 'dgroup': addr}"""
    mods, cur = [], None
    secs, dgroup = {}, None
    for line in open(path, encoding='latin-1'):
        m = re.match(r'\s+Module (\S+) from file (\S+)', line)
        if m:
            cur = (m.group(1), m.group(2), [])
            mods.append(cur)
            continue
        m = re.match(r'\s+Segment\s+(\S+?)\.(\S*), Addr = ([0-9a-f]+), Size = ([0-9a-f]+)(?:, Ovly# = (\d+))?', line)
        if m and cur:
            cur[2].append((m.group(1), m.group(2), int(m.group(3), 16), int(m.group(4), 16), int(m.group(5) or 0)))
            continue
        m = re.match(r'\s+Section .*Maddr=([0-9a-f]+).*Ovly#=(\d+)', line)
        if m:
            secs[int(m.group(2))] = int(m.group(1), 16)
        m = re.match(r'\s+DGROUP\s+([0-9a-f]+)', line)
        if m:
            dgroup = int(m.group(1), 16)
    return mods, {'sections': secs, 'dgroup': dgroup}


def expected(target):
    """{(kind, name): {seg: value}} from the link script; code as 'xxxx' or 'OVL:xxxx'."""
    out = {}
    for line in open(os.path.join(ROOT, 'src', 'u5d.lnk')).read().splitlines():
        f = line.split('#')[0].split()
        if not f or f[0] not in ('object', 'library'):
            continue
        t = [x[7:] for x in f if x.startswith('target=')]
        if t and t[0] != target:
            continue
        if f[0] == 'object':
            key = f[1].upper()
            kv = f[2:]
        else:
            key = os.path.basename(f[2].replace('\\', '/')).upper()
            kv = f[4:]
        d = {}
        for x in kv:
            if '=' in x and not x.startswith('target='):
                k, v = x.split('=', 1)
                d[k] = v.upper()
        out[key] = d
    return out


def actual(mods, info):
    order = pl.script(TARGET)[2]
    num = {i + 1: n for i, n in enumerate(order)}
    secs, dg = dict(info['sections']), info['dgroup']
    for mod, fil, segs in mods:              # an M map has no section list: the lowest address
        for s, c, a, size, ov in segs:
            if ov and ov in num and (c.upper().startswith('CODE') or s.upper() == '_TEXT'):
                secs[ov] = min(secs.get(ov, a), a)
    out = {}
    for mod, fil, segs in mods:
        key = (fil[:-4] if fil.upper().endswith('.OBJ') else os.path.basename(mod.replace('\\', '/'))).upper()
        if key.endswith('.C'):               # a u5d object pulled from a library: its THEADR
            key = key[:-2]
        d = {}
        for s, c, a, size, ov in segs:
            if c.upper().startswith('CODE') or s.upper() in ('_TEXT', 'XTEXT'):
                if ov and ov in num:
                    base = secs[ov]
                    hdr = 0x10 if num[ov] == 'INTRO' else 0
                    d.setdefault('code', '%s:%04X' % (num[ov], a - base + hdr))
                else:
                    d.setdefault('code', '%04X' % a)
            elif dg is not None and size:
                d.setdefault(s.upper(), '%04X' % (a - dg))
        out.setdefault(key, d)
    return out


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('work')
    ap.add_argument('--target', default='gog')
    ap.add_argument('-a', action='store_true')
    a = ap.parse_args()
    TARGET = a.target
    mods, info = parse_map(os.path.join(a.work, 'ULTIMA.MAP'))
    exp, act = expected(a.target), actual(mods, info)
    bad = 0
    for k, d in exp.items():
        g = act.get(k)
        if g is None:
            print('%-12s not in the map' % k)
            bad += 1
            continue
        diffs = [(s, v, g.get(s)) for s, v in d.items() if g.get(s) != v.lstrip('0').rjust(4, '0') and g.get(s) != v]
        if diffs:
            bad += 1
        if diffs or a.a:
            print('%-12s %s' % (k, ' '.join('%s=%s(ours %s)' % x for x in diffs) if diffs else 'ok'))
    print('%d of %d modules differ; DGROUP at %s' % (bad, len(exp), '%x' % info['dgroup'] if info['dgroup'] else '?'))
    print('order:', ' '.join(m[0].split('\\')[-1].replace('.ASM', '').replace('.C', '') for m in mods))
