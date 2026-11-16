#!/usr/bin/env python3
"""Write PLINK86's overlay table and call stubs as an assembly data module (src/asm/PLTAB.ASM).

    python tools/plinkgen.py

ULTIMA.EXE CS:7780-81d0 is what PLINK86 generated for the overlays (segment 0778):

  - 7780-7a16: the overlay table (records the overlay manager reads) and the overlay file
    names (TOWN.OVL ... DATA.OVL);
  - 7a16-81c6: 164 call stubs, 12 bytes each: call far PLMGR_TEXT:02ec (the overlay manager);
    dw overlay number; jmp far the function. A call into another overlay's function goes to
    its stub, which makes sure the overlay is loaded;
  - 81c6-81d0: padding to the next paragraph.

The table is written as data: words the EXE relocates (segments) as segment names, the file
names as strings, the rest as bytes. Each stub is written with its target by name
(STUB_<function>: ... dw OFFSET <function>), so the link resolves it, and tools/u5link.py
sends calls into other overlays to these labels. tools/asmbuild.py checks the result.
"""
import os, re, struct, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402
import asmgen  # noqa: E402

START, STUBS, PAD, END = 0x7780, 0x7a16, 0x81c6, 0x81d0


def overlay_function_names():
    """{(overlay number, CS address): exported name} for every function of every overlay
    (overlays of one area share addresses, so the overlay number tells them apart)."""
    out = {}
    for f, (kind, ovl) in u5.MODULES.items():
        if kind != 'ovl' or ovl not in u5.OVERLAY_BASE:
            continue
        s = open(os.path.join(u5.SRC, f), encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'^(?:static\s+)?[\w\s\*]*?\b(%s_([0-9a-fA-F]{4})\w*)\s*\([^;{)]*\)\s*(?://[^\n]*)?\s*\{' % ovl, s, re.M):
            out[(u5.OVERLAY_NUMBER[ovl], u5.OVERLAY_BASE[ovl] + int(m.group(2), 16))] = asmgen.exported(m.group(1))
    return out


def main():
    gd = u5.find_gamedata()
    img = u5.exe_image(gd)
    relocs = asmgen.relocations(gd)
    stubs = u5.overlay_stubs(img)
    names = overlay_function_names()
    numbers = {v: k for k, v in u5.OVERLAY_NUMBER.items()}
    out = ['; PLTAB.ASM: ULTIMA.EXE CS:%04x-%04x, PLINK86\'s overlay table and call stubs.' % (START, END),
           '; Generated from the original bytes by tools/plinkgen.py and checked by tools/asmbuild.py.',
           '', '\tTITLE\tPLTAB',
           "PLTAB\tSEGMENT  PARA PUBLIC 'CODE'", 'PLTAB\tENDS',
           "PLMGR_TEXT\tSEGMENT  PARA PUBLIC 'CODE'", 'PLMGR_TEXT\tENDS',
           "PLSTART_TEXT\tSEGMENT  PARA PUBLIC 'CODE'", 'PLSTART_TEXT\tENDS',
           "_TEXT\tSEGMENT  BYTE PUBLIC 'CODE'", '_TEXT\tENDS',
           "_DATA\tSEGMENT  WORD PUBLIC 'DATA'", '_DATA\tENDS',
           'DGROUP\tGROUP\t_DATA', '\tASSUME\tCS: PLTAB', '']
    body, externs = [], set()
    a = START
    line = []

    def flush():
        if line:
            body.append('\tdb\t' + ', '.join(line))
            line.clear()
    while a < STUBS:
        if a in relocs:
            flush()
            v = struct.unpack_from('<H', img, a)[0]
            body.append('\tdw\t%s' % asmgen.segname(v))
            a += 2
            continue
        m = re.match(rb'[A-Z0-9]{1,8}\.OVL\x00', img[a:a + 13])
        if m:
            flush()
            body.append("\tdb\t'%s', 0" % m.group(0)[:-1].decode())
            a += len(m.group(0))
            continue
        line.append('0%02xh' % img[a])
        a += 1
        if len(line) == 8:
            flush()
    flush()
    body.append('')
    for s in range(STUBS, PAD, 12):
        num, tgt = stubs[s]
        name = names.get((num, tgt))
        if not name:
            sys.exit('no function at %04x for the stub at %04x' % (tgt, s))
        externs.add(name)
        body += ['\tPUBLIC\tSTUB_%s' % name[:26], 'STUB_%s:' % name[:26],
                 '\tdb\t09ah', '\tdw\t02ech', '\tdw\tPLMGR_TEXT',
                 '\tdw\t%d\t\t; %s' % (num, numbers.get(num, '?')),
                 '\tdb\t0eah', '\tdw\tOFFSET %s' % name, '\tdw\tSEG %s' % name]
    body.append('\tdb\t%s' % ', '.join('0%02xh' % img[k] for k in range(PAD, END)))
    marks = sorted(set(re.findall(r'\bSEG_[0-9A-F]{4}\b', '\n'.join(body))))
    at = out.index('DGROUP\tGROUP\t_DATA')
    out[at:at] = sum((["%s\tSEGMENT  PARA PUBLIC 'MARK'" % m, '%s\tENDS' % m] for m in marks), [])
    out += ['\tEXTRN\t%s:NEAR' % n for n in sorted(externs)]
    out += ['', 'PLTAB\tSEGMENT'] + body + ['PLTAB\tENDS', '\tEND', '']
    p = os.path.join(u5.SRC, 'asm', 'PLTAB.ASM')
    open(p, 'w', newline='').write('\r\n'.join(out))
    print('wrote %s (%d stubs)' % (p, len(stubs)))


if __name__ == '__main__':
    main()
