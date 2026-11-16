#!/usr/bin/env python3
"""A map of ULTIMA.EXE's resident image: which source covers each range.

    python tools/rootmap.py            ranges in CS order, with their kind and source
    python tools/rootmap.py --gaps     only the ranges nothing covers yet

Kinds:
  C       a C function of u5d (its last match verdict from build/match/status.tsv)
  ASM     a function the original wrote in assembly (u5d's // ASM or // STUB (asm) tag)
  CRT     Microsoft C runtime (the '// xxxx: name' list in 0000.c)
  STUBS   PLINK86 overlay call stubs (12 bytes each)
  PLINK   PLINK86's overlay manager and start-up (the far segments)
  ?       not covered by anything known
"""
import argparse, glob, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402


def function_tags():
    """{addr: (name, file, tag)} for every ULTIMA_xxxx definition in the resident files."""
    out = {}
    for p in sorted(glob.glob(os.path.join(u5.SRC, '*.c'))):
        f = os.path.basename(p)
        s = open(p, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'^(?:static\s+)?(?:FAR\s+)?[\w\s\*]*?\b(ULTIMA_([0-9a-f]{4})\w*)\s*\([^;{)]*\)\s*(?://[^\n]*)?\s*\{', s, re.M | re.I):
            pre = s[max(0, m.start() - 200):m.start()]
            last = pre.strip().split('\n')[-3:]
            tag = 'C'
            for l in last:
                if re.match(r'\s*//\s*(STUB \(asm\)|ASM)', l):
                    tag = 'ASM'
                elif re.match(r'\s*//\s*STUB\b', l) and tag == 'C':
                    tag = 'STUB'
            a = int(m.group(2), 16)
            if a not in out or out[a][2] == 'C':
                out[a] = (m.group(1), f, tag)
    if 'ultima.c' in [os.path.basename(p) for p in glob.glob(os.path.join(u5.SRC, '*.c'))]:
        out.setdefault(0, ('main', 'ultima.c', 'C'))
    return out


def crt_names():
    out = {}
    for p in glob.glob(os.path.join(u5.SRC, '[0-7]000.c')):
        for m in re.finditer(r'^// ([0-9a-f]{4}): (.+)$', open(p, encoding='utf-8', errors='replace').read(), re.M):
            out[int(m.group(1), 16)] = m.group(2).strip()
    return out


def verdicts():
    p = os.path.join(u5.BUILD, 'match', 'status.tsv')
    out = {}
    if os.path.exists(p):
        for line in open(p).read().splitlines()[1:]:
            c = line.split('\t')
            if c[0] in u5.MODULES and u5.MODULES[c[0]][0] == 'root':
                out[int(c[2], 16)] = c[3]
    return out


def build(img):
    starts = []
    for a, (name, f, tag) in function_tags().items():
        starts.append((a, tag, name, f))
    for a, name in crt_names().items():
        starts.append((a, 'CRT', name, 'library'))
    stubs = u5.overlay_stubs(img)
    if stubs:
        lo, hi = min(stubs), max(stubs) + 12
        starts.append((lo, 'STUBS', '%d overlay stubs' % len(stubs), 'PLINK86'))
    starts.append((0x72e0, 'PLINK', 'segment 072e (overlay manager)', 'PLINK86'))
    starts.append((0x7780, 'PLINK', 'segment 0778', 'PLINK86'))
    starts.append((0x81d0, 'PLINK', 'segment 081d (start-up, entry point)', 'PLINK86'))
    starts.sort()
    out = []
    for i, (a, tag, name, f) in enumerate(starts):
        end = starts[i + 1][0] if i + 1 < len(starts) else len(img)
        if tag == 'STUBS':
            end = hi
        out.append((a, end, tag, name, f))
    # gaps
    full = []
    pos = 0
    for a, e, tag, name, f in out:
        if a > pos:
            full.append((pos, a, '?', '', ''))
        full.append((a, e, tag, name, f))
        pos = max(pos, e)
    return full


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--gaps', action='store_true')
    a = ap.parse_args()
    img = u5.exe_image(u5.find_gamedata())
    v = verdicts()
    tot = {}
    for s, e, tag, name, f in build(img):
        tot[tag] = tot.get(tag, 0) + e - s
        if a.gaps and tag != '?':
            continue
        extra = v.get(s, '') if tag in ('C', 'ASM', 'STUB') else ''
        print('%04x-%04x %5d  %-5s %-6s %-36s %s' % (s, e, e - s, tag, extra, name, f))
    print('bytes by kind: ' + ', '.join('%s %d' % kv for kv in sorted(tot.items())))


if __name__ == '__main__':
    main()
