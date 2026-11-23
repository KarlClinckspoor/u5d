#!/usr/bin/env python3
"""Check the link: what tools/u5link.py computes from the command file (src/u5d.lnk) against
the original files.

    python tools/linkscript.py [--target T] [--check] [-v]

u5link places everything by PLINK's rules from the command file's processing order and the
objects; the command file states what no rule settles, as facts. This tool is where the
original's placement is measured (it needs the game files, tools/u5.py find_gamedata, and a
full set of objects, build/obj or $U5_OBJ; for the 1988 target build/obj-1988, the gog build's
objects standing for those that do not depend on the target). It compares, placement by
placement, the measurement with u5link's computation, and the facts with the files:

  the original      library modules: which modules of SLIBCR.LIB and LIBH.LIB the image holds,
                    and where (a module's code is searched in the resident image, the bytes
                    under its fixups masked, tools/libmatch.py, and must occur exactly once);
                    code: a resident C object so that its first function lands on the address
                    in its name, a resident assembly module at the address in its first line,
                    an overlay's object at the offset its first function's name gives; data:
                    a C object's _DATA, CONST and _BSS where its code's references into them
                    point in the original (tools/match.py, vote_data_bases), an assembly or
                    library module's data segments likewise, and those no code refers to where
                    their bytes occur in DGROUP (once), or at FIXED_DATA; _edata and _end where
                    the runtime's code uses them; the overlay table's records (each section's
                    paragraphs, length, relocation count); each .OVL file's length and header;
                    DGROUP's length; the length of the resident image; the stubs; the
                    communal variables, each where its name says (the names' addresses are
                    the original's: match.py checks every reference to them);
  computed          every placement u5link computes (Linker.place, Linker.link);
  facts             the stub order (against the original's stubs), the wrapper's stack (against
                    the EXE header); the processing order is the command file's input.

Every placement is reported as computed or a fact, with the counts. It exits 1 when anything
differs (--check, the default); tools/check.py runs the same check when the game files are
there.
"""
import argparse, glob, os, struct, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402
import u5link  # noqa: E402
from u5link import Module  # noqa: E402

# Runtime data segments no code refers to and whose bytes are not unique, measured: the
# message table's terminators. PAD is a COMMON segment (every module's 'ff ff' overlaid),
# followed by EPAD.
FIXED_DATA = {'PAD': 0xa520, 'EPAD': 0xa522}
LIBRARIES = ('SLIBCR.LIB', 'LIBH.LIB')
LIBRARY_END = 0x878           # the runtime's modules lie below the first of the game's own
TABLE = 0x7780                # the overlay table in the original (PLINK's $OVTB$)


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
            info = u5.name_info(pn)
            if info and info[0] == ovl and info[1] == 'code':
                base += info[2] - off
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
        if kind == 'data':                # no code or data of its own
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


def original_table(gd, sections):
    """The overlay table's records in the original: {section: (relocations, start, end,
    length)} (paragraphs), in the order of the overlay numbers."""
    img = u5.exe_image(gd)
    out = {}
    for k, sec in enumerate(sections):
        w = struct.unpack_from('<8H', img, TABLE + 8 + 16 * k)
        out[sec] = (w[0], w[1], w[2], w[6])
    return out


def header_length(count):
    """The length of a relocation header of count entries (PLINK's: a dword each, a zero
    dword, zeros to the paragraph; none without entries)."""
    return 0 if not count else (count * 4 + 4 + 15) // 16 * 16


def compute():
    """u5link's link of the target's objects, as tools/u5link.py makes it (nothing written)."""
    lk = u5link.Linker(u5link.Script())
    if u5.TARGET != 'gog' and os.path.abspath(u5.OBJ) != os.path.abspath(u5.DEFAULT_OBJ):
        # an object whose source does not depend on the target is the gog build's (match.py
        # does not compile vars.c, so build/obj-1988 need not have VARS.OBJ)
        have = {os.path.splitext(os.path.basename(p))[0].upper() for p in glob.glob(os.path.join(u5.OBJ, '*.OBJ'))}
        dep = u5.target_dependent_objects()
        lk.obj_path = lambda n: os.path.join(u5.OBJ if n in have or n in dep else u5.DEFAULT_OBJ, n + '.OBJ')
    lk.gather()
    return lk, lk.link()


def fmt(v):
    if isinstance(v, int):
        return '%04x' % v
    return 'none' if v is None else str(v)


class Report:
    def __init__(self):
        self.kinds = []            # (kind, number compared, number differing)
        self.problems = []

    def compare(self, kind, pairs):
        """pairs: [(what, the original's, u5link's)]"""
        bad = [(w, a, b) for w, a, b in pairs if a != b]
        self.kinds.append((kind, len(pairs), len(bad)))
        for w, a, b in bad:
            self.problems.append('%s: %s is %s in the original, %s computed' % (kind, w, fmt(a), fmt(b)))


def check(gd, verbose=False):
    """(report lines, problems): the original's placement against u5link's, and the command
    file's facts against the files."""
    mods, defines, notes = measure(gd)
    lk, (root, overlays, dgroup) = compute()
    sc = lk.script
    r = Report()
    r.problems += ['measurement: ' + n for n in notes]
    r.problems += ['link: ' + n for n in lk.notes]
    if lk.by_convention or lk.unresolved:
        r.problems.append('link: %d names resolved only by their address, %d unresolved' % (
            len(lk.by_convention), len(lk.unresolved)))
    ours = {(m.kind == 'lib', m.name): m for m in lk.modules}
    # the library modules: which, in which order (the original's by address)
    lib = sorted((m for m in mods if m.kind == 'lib'), key=lambda m: m.code[1])
    clib = [m for m in lk.modules if m.kind == 'lib']
    pairs = []
    for k in range(max(len(lib), len(clib))):
        a = lib[k] if k < len(lib) else None
        b = clib[k] if k < len(clib) else None
        pairs.append(('module %d' % (k + 1), a and (a.lib, a.name, sorted(u5link.module_publics(a.obj))),
                      b and (b.lib, b.name, sorted(u5link.module_publics(b.obj)))))
    r.compare('library modules (which, in order)', pairs)
    code, data = {'lib': [], 'res': [], 'ovl': []}, []
    for m in mods:
        c = ours.get((m.kind == 'lib', m.name))
        if c is None:
            r.problems.append('%s: measured in the original, not linked' % m.name)
            continue
        if m.code and m.code[0]:
            code[m.kind].append((m.name, m.code[1], c.code[1] if c.code else None))
        for s, at in sorted(m.data.items()):
            if m.obj.seg(s) is not None:
                data.append(('%s %s' % (m.name, s), at, c.data.get(s)))
    r.compare('code addresses: resident objects', code['res'])
    r.compare('code addresses: library modules', code['lib'])
    r.compare('code addresses: the overlays\' objects', code['ovl'])
    r.compare('DGROUP offsets of the data segments', data)
    # the sections (the overlay table's records), the overlay files, DGROUP, the image
    table = original_table(gd, list(sc.sections))
    pairs = []
    for sec in sc.sections:
        s = lk.sections[sec]
        count = len(lk.data_entries()) if sec == sc.preloaded else max(0, s.get('header', 0) - 4) // 4
        if sec != sc.preloaded:
            count = len(lk.relocation_order([m for m in lk.modules if m.kind == 'ovl' and m.overlay == sec]))
        pairs.append(('%s (relocations, start, end, length)' % sec, table[sec],
                      (count, s['base'] >> 4, (s['base'] + s['memory']) >> 4, u5link.paragraph(s['size']) >> 4)))
    r.compare('sections: the overlay table\'s records', pairs)
    pairs = []
    for sec in sc.overlays():
        orig = u5.read_file(gd, sec + '.OVL')
        s = lk.sections[sec]
        pairs += [('%s.OVL length' % sec, len(orig), len(overlays[sec])),
                  ('%s.OVL header (by its relocation count)' % sec, header_length(table[sec][0]), s.get('header', 0)),
                  ('%s.OVL address (CS of its offset 0)' % sec, table[sec][1] * 16 - header_length(table[sec][0]),
                   s['base'] - s.get('header', 0))]
    r.compare('overlay files: length, header, address', pairs)
    r.compare('DGROUP and the resident image', [
        ('DGROUP length (DATA.OVL less its header)', len(u5.ds_image(gd)), len(dgroup)),
        ('DATA.OVL header', u5.DATA_OVL_HEADER, len(lk.data_header())),
        ('_edata', defines.get('_edata'), lk.publics['_edata'][1]),
        ('_end', defines.get('_end'), lk.publics['_end'][1]),
        ('resident image length', len(u5.exe_image(gd)), len(root))])
    r.compare('DGROUP offsets of the communal variables', [
        (n, (u5.name_address(n) or (None, None))[1], at) for n, at in lk.communal_at.items()])
    computed = sum(n for k, n, b in r.kinds)
    # the facts
    img = u5.exe_image(gd)
    func = {}
    for n, (kind, a, owner) in lk.publics.items():
        if kind == 'code' and owner and owner.kind == 'ovl':
            func[(sc.number(owner.overlay), a)] = n
    orig_stubs = [func.get(v, '?') for k, v in sorted(u5.overlay_stubs(img).items())]
    fact_stubs = [u5link.exported(f) for f in sc.stubs]
    stubs_ok = sum(1 for a, b in zip(orig_stubs, fact_stubs) if a[:31] == b)
    if stubs_ok != len(orig_stubs) or len(fact_stubs) != len(orig_stubs):
        r.problems.append('the stubs: %d of the original\'s %d in the command file\'s order' % (stubs_ok, len(orig_stubs)))
    groups = lk.stub_sections()
    turns = []
    for f, sec in groups:
        if not turns or turns[-1] != sec:
            turns.append(sec)
    exe = u5.read_file(gd, 'ULTIMA.EXE')
    ss, sp = struct.unpack_from('<HH', exe, 0x0e)
    gap = ss - len(img) // 16
    stack_ok = (gap, sp) == (sc.wrapper_stack.get('stackgap'), sc.wrapper_stack.get('sp'))
    if not stack_ok:
        r.problems.append('the wrapper\'s stack: SS %04x paragraphs past the image, SP %04x, in the original' % (gap, sp))
    files = sum(1 for k, n, s in sc.order if k == 'file')
    L = ['%s against the original files (%s):' % (os.path.relpath(u5link.script_path(), u5.ROOT),
                                                  u5.TARGET_TITLE[u5.TARGET]),
         '  computed by u5link: %d placements' % computed]
    for kind, n, bad in r.kinds:
        L.append('    %-46s %4d  %s' % (kind, n, 'equal' if not bad else '%d DIFFER' % bad))
    L += ['  facts (the command file\'s):',
          '    %-46s %d files, %d sections in %d areas, the library after the areas' % (
              'the processing order', files, len(sc.sections), len(sc.areas)),
          '    %-46s %d, %d in the original\'s order (the rule gives the set, and the sections\' '
          'turns: %s)' % ('the order of the call stubs', len(fact_stubs), stubs_ok,
                          'in processing order' if len(turns) == len(set(turns)) else 'NOT in processing order'),
          '    %-46s SS %04x paragraphs past the image, SP %04x: %s' % (
              'the wrapper\'s stack', gap, sp, 'the original\'s' if stack_ok else 'DIFFERS')]
    if verbose:
        L.append('  the stubs by the turn PLINK makes them in (the rule, u5d\'s objects): ' +
                 ', '.join('%s %d' % (s, sum(1 for f, x in groups if x == s)) for s in turns))
    return L, r.problems


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--check', action='store_true', help='compare (the default)')
    ap.add_argument('--gamedata')
    ap.add_argument('-v', '--verbose', action='store_true')
    u5.add_target_argument(ap)
    a = ap.parse_args()
    u5.set_target(a.target)
    lines, problems = check(u5.find_gamedata(a.gamedata), a.verbose)
    print('\n'.join(lines))
    for p in problems:
        print('problem: ' + p)
    rel = os.path.relpath(u5link.script_path(), u5.ROOT)
    if problems:
        print('%s: the link is not the original\'s (%s)' % (rel, u5.TARGET_TITLE[u5.TARGET]))
        sys.exit(1)
    print('%s: every placement u5link computes is the original\'s (%s)' % (rel, u5.TARGET_TITLE[u5.TARGET]))


if __name__ == '__main__':
    main()
