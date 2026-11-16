#!/usr/bin/env python3
"""Give an assembly module its own data: the bytes of DGROUP it owns, as its _DATA segment.

    python tools/asmdata.py NAME.ASM LO HI [--apply]

The resident data from DS:52ba to 545f is the assembly modules' and the small C modules'
own, in link order (docs/MATCHING.md, "The resident data"). For an assembly module this:

  - writes a _DATA segment holding the original bytes of DS:LO..HI, with a PUBLIC label at
    each variable vars.h names there (LABEL BYTE or WORD by its type) and at each address
    the module's code refers to by number;
  - removes the module's EXTRNs for those names;
  - turns its numeric references into the labels: ds:[544ch] -> D_544C, [bx+5426h] ->
    D_5426[bx], mov si,5322h -> mov si,OFFSET DGROUP:D_5322. MASM encodes these the same,
    with a fixup, so the linker can place the segment and check the references.

tools/asmbuild.py must still report the module identical afterwards.
"""
import argparse, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402


def vars_names():
    """{DS address: (NAME as the objects spell it, is_byte)} from vars.h."""
    h = open(os.path.join(u5.SRC, 'vars.h'), encoding='utf-8').read()
    out = {}
    for m in re.finditer(r'^STUB\s+([^;]*?)\b(D_([0-9a-f]{4})\w*)\s*((?:\[[^\]]*\])*)\s*;', h, re.M):
        typ = m.group(1)
        out[int(m.group(3), 16)] = (m.group(2).upper(), bool(re.search(r'\b(u8|s8|char|byte)\b', typ)) and '*' not in typ)
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('file')
    ap.add_argument('lo')
    ap.add_argument('hi')
    ap.add_argument('--apply', action='store_true')
    a = ap.parse_args()
    lo, hi = int(a.lo, 16), int(a.hi, 16)
    path = a.file if os.path.exists(a.file) else os.path.join(u5.SRC, 'asm', a.file)
    src = open(path, encoding='latin-1', newline='').read()
    nl = '\r\n' if '\r\n' in src else '\n'
    ds = u5.ds_image(u5.find_gamedata())
    names = vars_names()

    # numeric references into the range
    REF = re.compile(r'ds:\[([0-9a-f]{4})h\]|\[(bx|si|di|bp)\+([0-9a-f]{4})h\]|^(\tmov (?:si|di|dx|bx|ax|cx),)([0-9a-f]{4})h(?=\r?$)', re.M | re.I)
    used, bytewise = set(), set()
    for m in REF.finditer(src):
        v = int(m.group(1) or m.group(3) or m.group(5), 16)
        if lo <= v < hi:
            used.add(v)
            line = src[src.rfind('\n', 0, m.start()) + 1:src.find('\n', m.end())]
            if re.search(r'\b(al|ah|bl|bh|cl|ch|dl|dh)\b|byte ptr', line, re.I):
                bytewise.add(v)
    labels = {ad: n for ad, (n, _) in names.items() if lo <= ad < hi}
    isbyte = {ad: b for ad, (n, b) in names.items() if lo <= ad < hi}
    # a numeric reference inside a named variable stays relative to it; otherwise it gets a label
    starts = sorted(labels)
    for v in sorted(used):
        inside = [s for s in starts if s <= v]
        if v not in labels and not (inside and v - inside[-1] < 2 and not isbyte.get(inside[-1])):
            labels[v] = 'D_%04X' % v
            isbyte[v] = v in bytewise
    starts = sorted(labels)

    def label_for(v):
        s = max(x for x in starts if x <= v)
        return labels[s] if s == v else '%s+%d' % (labels[s], v - s)

    def rep(m):
        if m.group(1):
            v = int(m.group(1), 16)
            return label_for(v) if lo <= v < hi else m.group(0)
        if m.group(3):
            v = int(m.group(3), 16)
            return '%s[%s]' % (label_for(v), m.group(2)) if lo <= v < hi else m.group(0)
        v = int(m.group(5), 16)
        return '%sOFFSET DGROUP:%s' % (m.group(4), label_for(v)) if lo <= v < hi else m.group(0)
    src = REF.sub(rep, src)

    # the EXTRNs of names now defined here go
    defined = set(labels.values())
    src = re.sub(r'^\tEXTRN\t(\w+):\w+\r?\n', lambda m: '' if m.group(1) in defined else m.group(0), src, flags=re.M)

    # the _DATA segment
    out = ['; this module\'s own data in DGROUP (DS:%04x-%04x)' % (lo, hi), '_DATA\tSEGMENT']
    cuts = sorted(set(starts) | {lo, hi})
    for i, s in enumerate(cuts[:-1]):
        e = cuts[i + 1]
        if s in labels:
            out.append('\tPUBLIC\t%s' % labels[s])
            out.append('%s\tLABEL\t%s' % (labels[s], 'BYTE' if isbyte.get(s) else 'WORD'))
        chunk = ds[s:e]
        for k in range(0, len(chunk), 12):
            row = chunk[k:k + 12]
            out.append('\tdb\t' + ', '.join('%03Xh' % b for b in row) + ('\t; ' + ''.join(chr(c) if 32 <= c < 127 else '.' for c in row) if any(32 <= c < 127 for c in row) and len(row) > 3 else ''))
    out.append('_DATA\tENDS')
    block = nl.join(out) + nl + nl
    i = src.index('_TEXT\tSEGMENT' + nl, src.index('ASSUME'))
    src = src[:i] + block + src[i:]
    print('%s: %d labels %s' % (os.path.basename(path), len(labels), ' '.join(labels[s] for s in starts)))
    if a.apply:
        open(path, 'w', encoding='latin-1', newline='').write(src)


if __name__ == '__main__':
    main()
