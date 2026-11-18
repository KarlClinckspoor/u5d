#!/usr/bin/env python3
"""Measure the link script (src/u5d.lnk) from the game files and the objects.

    python tools/linkscript.py [--target T]            measure T's lines, write src/u5d.lnk
    python tools/linkscript.py [--target T] --check    measure again and compare

The script holds both targets (tools/u5.py, TARGETS). A line that holds for one target only
ends in target=gog or target=1988 (the 1992 patch's assembly, A2322 and AINTRO2, is the gog
target's; the 1988 target's r2320.c and intro2.c hold that code and data themselves); the
other lines hold for both. A target's lines are measured from its game files (gamedata/,
gamedata-1988/, ...) and its objects (build/obj, build/obj-1988, or $U5_OBJ); writing
measures the target given (--target, else $U5_TARGET, else gog), and the other target too when
its game files and its object folder (build/obj-1988, build/obj) are there, else keeps its
lines as the script has them. --check compares the target's lines only.

tools/u5link.py links from the script and the objects alone; it opens no game file. This
tool is where the script's facts come from: it needs the game files (tools/u5.py,
find_gamedata) and a full set of objects (build/obj, or $U5_OBJ), and measures

  the EXE header   PLINK86's own fields, read from ULTIMA.EXE's header, and the length of
                   the resident image;
  DGROUP           its length and DATA.OVL's header length;
  the overlays     each file's length; its area base and header length are tools/u5.py's
                   (OVERLAY_BASE, OVERLAY_HEADER: the names of the functions carry them);
  library modules  which modules of SLIBCR.LIB and LIBH.LIB the image holds, and where:
                   a module's code is searched in the resident image, the bytes under its
                   fixups masked (tools/libmatch.py), and must occur exactly once;
  code addresses   a resident C object so that its first function lands on the address in
                   its name; a resident assembly module at the address in its first line;
                   an overlay's object at the offset its first function's name gives;
  data offsets     a C object's _DATA, CONST and _BSS where its code's references into them
                   point in the original (tools/match.py, vote_data_bases); an assembly or
                   library module's data segments likewise, and those no code refers to
                   where their bytes occur in DGROUP (once), or at FIXED_DATA;
  _edata, _end     the values the runtime's code holds for them in the original.

--check is how the script stays a statement about the original: tools/check.py runs it
when the game files are there.
"""
import argparse, glob, os, struct, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402
import u5link  # noqa: E402
from u5link import Module  # noqa: E402

# Runtime data segments no code refers to and whose bytes are not unique, measured: the
# message table's terminators. PAD is a COMMON segment (every module's 'ff ff' overlaid),
# followed by EPAD. crt0's STACK lies beyond DGROUP and is not part of DATA.OVL.
FIXED_DATA = {'PAD': 0xa520, 'EPAD': 0xa522}
LIBRARIES = ('SLIBCR.LIB', 'LIBH.LIB')
LIBRARY_END = 0x878           # the runtime's modules lie below the first of the game's own


def code_segment(obj):
    return u5link.code_segment(obj)


def gather(gd, notes):
    """The modules the build has objects for, each with the address of its code."""
    import asmbuild, libmatch
    mods = []
    have = {os.path.splitext(os.path.basename(p))[0].upper(): p for p in glob.glob(os.path.join(u5.OBJ, '*.OBJ'))}
    if u5.TARGET != 'gog' and os.path.abspath(u5.OBJ) != os.path.abspath(u5.DEFAULT_OBJ):
        # an object whose source does not depend on the target is the gog build's (match.py
        # does not compile vars.c, so build/obj-1988 need not have VARS.OBJ)
        dep = u5.target_dependent_objects()
        for p in glob.glob(os.path.join(u5.DEFAULT_OBJ, '*.OBJ')):
            n = os.path.splitext(os.path.basename(p))[0].upper()
            if n not in have and n not in dep:
                have[n] = p

    def overlay_module(n, ovl, cfile):
        # at the area base, or, for a later object of an overlay built from several, where
        # its first function's name says it starts
        m = Module(n, omf.Object(have[n]), 'ovl', ovl)
        seg = code_segment(m.obj)
        base = u5.OVERLAY_BASE[ovl]
        for pn, off in sorted(seg.publics.items(), key=lambda kv: kv[1]) if seg else ():
            mm = u5.NAME_RE.match(pn)
            if mm and mm.group(1).upper() == ovl:
                base += int(mm.group(2), 16) - off
                break
        m.code = (seg, base) if seg else None
        m.cfile = cfile
        return m

    for p in u5.asm_modules():
        n = os.path.splitext(os.path.basename(p))[0].upper()
        if n not in have:
            notes.append('%s: no object' % n)
            continue
        place = asmbuild.module_place(p)
        if place[0]:              # part of an overlay: placed by its functions' names
            m = overlay_module(n, place[0], None)
        else:
            m = Module(n, omf.Object(have[n]), 'res')
            m.code = (code_segment(m.obj), place[1])
            m.cfile = None
        mods.append(m)
    for f, (kind, ovl) in sorted(u5.MODULES.items()):
        n = os.path.splitext(f)[0].upper()
        if n not in have:
            notes.append('%s: no object' % n)
            continue
        if kind == 'data':                # no code or data of its own: communal variables
            m = Module(n, omf.Object(have[n]), 'data')
            m.cfile = None
            mods.append(m)
        elif kind == 'ovl':
            mods.append(overlay_module(n, ovl, f))
        elif kind == 'root':
            obj = omf.Object(have[n])
            seg = code_segment(obj)
            if not seg:
                continue                  # all of it is assembly in the original (src/asm)
            # placed so that its functions land at the addresses their names give (main: 0)
            base = None
            for pn, off in sorted(seg.publics.items(), key=lambda kv: kv[1]):
                a = ('code', 0) if pn.upper() == '_MAIN' else u5.name_address(pn)
                if a and a[0] == 'code':
                    base = a[1] - off
                    break
            if base is None:
                notes.append('%s: no function with an address; not placed' % f)
                continue
            m = Module(n, obj, 'res')
            m.code, m.cfile = (seg, base), f
            mods.append(m)
    # the runtime library modules, where tools/libmatch.py finds them
    img = u5.exe_image(gd)
    found = {}
    for lib in LIBRARIES:
        for name, obj in omf.library_modules(os.path.join(u5.MSC51, 'LIB', lib)):
            for seg in libmatch.code_segments(obj):
                at = libmatch.find(img, seg)
                if len(at) == 1 and at[0] < LIBRARY_END:
                    found.setdefault(at[0], []).append((lib, name, obj))
    # modules with the same code (alshl, alshr, aulshr) match the same place: the one linked
    # is the one whose own references the other modules found satisfy
    defined = {p for c in found.values() for l, n, o in c for s in o.segs[1:] for p in s.publics}
    for addr, cands in sorted(found.items()):
        ok = [c for c in cands if all(e in defined or not e.startswith('__aN') for e in c[2].exts[1:])]
        lib, name, obj = (ok or cands)[0]
        m = Module(name, obj, 'lib')
        m.lib, m.cfile = lib, None
        # several modules of a library may have one name (the near and far forms of a
        # helper): the script names the module by a public symbol too, as a linker finds it
        m.public = min(u5link.module_publics(obj))
        m.code = (code_segment(obj), addr)
        mods.append(m)
    return mods


def place_data_by_code(m, gd, img, ds):
    """For a module whose code is the original's byte for byte (library, assembly): each
    reference from its code into one of its own data segments shows, in the original,
    where that segment starts. Segments no code refers to are found by their bytes."""
    import libmatch
    seg, base = m.code
    at0 = 0                                    # the original bytes the code sits in
    if m.kind == 'ovl':
        img, at0 = u5.read_file(gd, m.overlay + '.OVL'), u5.OVERLAY_BASE[m.overlay]
    votes = {}
    for fx in seg.fixups:
        kind, name = fx.target
        if fx.self_rel or fx.kind != omf.LOC_OFFSET or kind != 'seg' or name == seg.name:
            continue
        s = m.obj.seg(name)
        if not s or s.cls == 'CODE':
            continue
        w = struct.unpack_from('<H', img, base - at0 + fx.loc)[0]
        b = (w - fx.disp - struct.unpack_from('<H', seg.data, fx.loc)[0]) & 0xffff
        votes.setdefault(name, {}).setdefault(b, 0)
        votes[name][b] += 1
    out = {n: max(v, key=v.get) for n, v in votes.items()}
    for n, at in FIXED_DATA.items():
        if m.obj.seg(n) and n not in out:
            out[n] = at
    for s in m.obj.segs[1:]:
        if s.cls != 'CODE' and s.length and s.name not in out and any(s.data):
            at = libmatch.find(ds, s)
            if len(at) == 1:
                out[s.name] = at[0]
    return out


def measure(gd):
    """(modules with their code and data placed, {linker symbol: DS offset}, notes)"""
    import match
    notes = []
    mods = gather(gd, notes)
    img, ds = u5.exe_image(gd), u5.ds_image(gd)
    for m in mods:
        # data: where the module's references into its own data point in the original
        if m.cfile and m.kind != 'data':
            mm = match.Module(m.cfile, gd)
            mm.vote_data_bases(mm.functions())
            m.data = dict(mm.data_base)
        elif m.code and m.code[0]:
            m.data = place_data_by_code(m, gd, img, ds)
    # symbols the linker defines (end of the initialised data, end of DGROUP): their
    # values, measured where the runtime's code uses them in the original
    defines = {}
    for m in mods:
        if m.kind != 'lib':
            continue
        seg, base = m.code
        for fx in seg.fixups:
            if fx.target[0] == 'ext' and fx.target[1] in u5link.LINKER_DEFINED and not fx.self_rel \
                    and fx.kind == omf.LOC_OFFSET:
                w = struct.unpack_from('<H', img, base + fx.loc)[0]
                v = (w - fx.disp - struct.unpack_from('<H', seg.data, fx.loc)[0]) & 0xffff
                if defines.setdefault(fx.target[1], v) != v:
                    notes.append('%s: %s is DS:%04x here, DS:%04x elsewhere' % (m.name, fx.target[1], v, defines[fx.target[1]]))
    return mods, defines, notes


HEAD = '''\
# u5d.lnk: the link script. tools/u5link.py links ULTIMA.EXE, the overlays and DATA.OVL
# from this file and the objects alone; it opens no game file.
#
# Written by tools/linkscript.py, which measures every line from the GOG files and the
# objects (its docstring says how); "tools/linkscript.py --check" measures again and
# compares. Do not edit by hand: change the measurement and write the file again.
# All numbers are hexadecimal. A '#' starts a comment.
'''


def text(gd):
    """The script as text, and the notes of the measurement."""
    mods, defines, notes = measure(gd)
    d = u5.read_file(gd, 'ULTIMA.EXE')
    (sig, last, pages, nrel, hpar, minalloc, maxalloc, ss, sp, csum, ip, cs, reloff, ovno, w1c) = \
        struct.unpack_from('<2s14H', d, 0)
    image = len(d) - hpar * 16
    L = [HEAD]
    L += ['# exe: ULTIMA.EXE. image is the length of the resident image (the file after its',
          '# header). The other fields are PLINK86\'s own, read from the original\'s header: the',
          '# relocation table starts at fixed (the header\'s length before it), the word at 1c',
          '# is word1c, and SS is stackgap paragraphs past the end of the image and SP is sp',
          '# (the stack of Origin\'s wrapper, PLSTART, which took the entry point). Sizes,',
          '# relocation count and table, header length, minimum allocation, entry point and the',
          '# checksum (PLINK86\'s MZ checksum of the program without the wrapper) are the',
          '# linker\'s rules.',
          'exe      image=%04x fixed=%02x word1c=%04x stackgap=%04x sp=%04x' % (
              image, reloff, w1c, ss - image // 16, sp), '']
    L += ['# dgroup: DATA.OVL is a header of this length (the far pointers in DGROUP, then zeros)',
          '# and DGROUP, size bytes from DS:0000.',
          'dgroup   size=%04x header=%02x' % (len(u5.ds_image(gd)), u5.DATA_OVL_HEADER), '']
    L += ['# define: symbols the linker defines, the end of the initialised data and the end of',
          '# DGROUP, as the runtime\'s code holds them in the original.']
    L += ['define   %s=%04x' % kv for kv in sorted(defines.items())] + ['']
    L += ['# overlay: a .OVL file. base is the CS address its file offset 0 is loaded at (the',
          '# area base, less the header), size the file\'s length, header the length of the',
          '# relocation list PLINK86 wrote before the code (INTRO.OVL only). In the order of',
          '# the overlay numbers (the file names in PLINK86\'s table, src/asm/PLTAB.ASM).']
    ovls = sorted({m.overlay for m in mods if m.kind == 'ovl'}, key=lambda o: u5.OVERLAY_NUMBER[o])
    for o in ovls:
        L.append('overlay  %-8s base=%04x size=%04x header=%02x' % (
            o, u5.OVERLAY_BASE[o], len(u5.read_file(gd, o + '.OVL')), u5.OVERLAY_HEADER.get(o, 0)))
    L.append('')

    def placement(m):
        if m.code and m.code[0]:
            c = m.code[1]
            code = 'code=%s:%04x' % (m.overlay, c - u5.OVERLAY_BASE[m.overlay]) if m.kind == 'ovl' else 'code=%04x' % c
        else:
            code = ''
        segs = sorted(((b, s) for s, b in m.data.items() if m.obj.seg(s)), key=lambda t: (t[0], t[1]))
        return ' '.join([code] + ['%s=%04x' % (s, b) for b, s in segs]).strip()

    res = sorted((m for m in mods if m.kind in ('res', 'lib')), key=lambda m: m.code[1])
    L += ['# The resident modules, in the order of their code (the link order). Each line: where',
          '# the module\'s code starts (a CS offset) and where each of its data segments starts',
          '# in DGROUP (DS offsets).',
          '#   library LIB MODULE PUBLIC   the module of the Microsoft C 5.1 runtime library LIB',
          '#                        (build/msc51/LIB) that defines PUBLIC; MODULE is its name',
          '#   object NAME          build/obj/NAME.OBJ: compiled from src/NAME.c, or assembled',
          '#                        from src/asm/NAME.ASM']
    for m in res:
        if m.kind == 'lib':
            L.append(('library  %-10s %-16s %-11s %s' % (m.lib, m.name, m.public, placement(m))).rstrip())
        else:
            L.append(('object   %-8s %s' % (m.name, placement(m))).rstrip())
    L += ['', '# The overlays\' objects: code=OVERLAY:offset, the offset in the .OVL file (as in the',
          '# functions\' names), then the data segments as above.']
    for o in ovls:
        for m in sorted((m for m in mods if m.kind == 'ovl' and m.overlay == o), key=lambda m: m.code[1] if m.code else 0):
            L.append(('object   %-8s %s' % (m.name, placement(m))).rstrip())
    L += ['', '# Objects without code or data of their own: the communal variables, which the',
          '# linker allocates after the last _BSS (tools/u5.py, COMMUNALS).']
    for m in mods:
        if m.kind == 'data':
            L.append('object   %s' % m.name)
    return '\n'.join(L) + '\n', notes


def merge(views):
    """The script from each target's lines ({target: text}): lines all targets have once,
    the others marked target=T (u5link.script_lines takes a target's lines back out)."""
    import difflib
    a, b = (views[t].splitlines() for t in u5.TARGETS)    # two targets
    ta, tb = u5.TARGETS
    out = []
    for op, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
        if op == 'equal':
            out += a[i1:i2]
            continue
        for t, lines in ((ta, a[i1:i2]), (tb, b[j1:j2])):
            for line in lines:
                if not line.split('#')[0].strip():
                    raise SystemExit('the targets\' scripts differ in a comment or blank line: %r' % line)
                out.append('%s target=%s' % (line, t))
    return '\n'.join(out) + '\n'


def script_text():
    p = u5link.script_path()
    return open(p, encoding='ascii', newline='').read().replace('\r\n', '\n') if os.path.exists(p) else ''


def check_text(measured, target=None):
    """'' when the script's lines for target are measured, else what differs first."""
    ol, nl = u5link.script_lines(script_text(), target or u5.TARGET), measured.splitlines()
    for k in range(max(len(ol), len(nl))):
        o, n = (ol[k] if k < len(ol) else None), (nl[k] if k < len(nl) else None)
        if o != n:
            return 'line %d of the %s lines:\n  script:   %s\n  measured: %s' % (k + 1, target or u5.TARGET, o, n)
    return ''


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--check', action='store_true', help='compare the measurement with the script; write nothing')
    ap.add_argument('--gamedata')
    u5.add_target_argument(ap)
    a = ap.parse_args()
    u5.set_target(a.target)
    new, notes = text(u5.find_gamedata(a.gamedata))
    for n in notes:
        print('note: ' + n)
    rel = os.path.relpath(u5link.script_path(), u5.ROOT)
    if a.check:
        diff = check_text(new)
        if diff:
            print(diff)
            print('%s differs from the measurement (%s)' % (rel, u5.TARGET_TITLE[u5.TARGET]))
            sys.exit(1)
        print('%s is what the original files give (%s: %d lines)' % (rel, u5.TARGET_TITLE[u5.TARGET], len(new.splitlines())))
        sys.exit(1 if notes else 0)
    # the other targets' lines: measured too when their game files and their shared object
    # folder are there (and this target's objects are in its shared folder), else kept
    old, views, kept = script_text(), {u5.TARGET: new}, []
    shared = os.path.normcase(u5.OBJ) == os.path.normcase(os.path.abspath(u5.default_obj()))
    for t in u5.TARGETS:
        if t == u5.TARGET:
            continue
        gd = u5.find_gamedata(target=t, required=False) if not a.gamedata else None
        if gd and shared and os.path.isdir(u5.default_obj(t)):
            with u5.building(t, u5.default_obj(t)):
                views[t], tnotes = text(gd)
            for n in tnotes:
                print('note (%s): %s' % (t, n))
        else:
            views[t] = '\n'.join(u5link.script_lines(old, t)) + '\n' if old else new
            kept.append(t)
    out = merge(views)
    open(u5link.script_path(), 'w', encoding='ascii', newline='\r\n').write(out)
    print('wrote %s (%d lines; %s)' % (rel, len(out.splitlines()), 'the %s lines kept as they were' % ', '.join(kept)
                                       if kept else 'every target measured'))


if __name__ == '__main__':
    main()
