#!/usr/bin/env python3
"""Link the objects into ULTIMA.EXE, the overlays and DATA.OVL, and compare with the GOG files.

    python tools/u5link.py [--out DIR] [-v]

PLINK86, which linked the original, is not available, and MS LINK's overlays are a different
scheme; this is a small linker for this one program (Exhume's docs/link.md calls it a link
stage of "facts": where things go is measured from the original, how references are
resolved is the linker's rule). What it does:

  placement (facts)  every object's code goes where the original has it: the resident
                     modules at their CS addresses, each overlay module at its area base,
                     PLINK86's segments at theirs; every object's data (_DATA) at its DGROUP
                     offset (found by tools/match.py's rule: where its references point);
  resolution (rules) a name is resolved to the public that defines it; a call from one
                     overlay (or the resident code) into a function of another overlay goes
                     to that function's stub (STUB_<name>, src/asm/PLTAB.ASM), as PLINK86
                     did; segment references become the paragraphs of their segments, and
                     those in the resident image become relocation entries;
  output             build/out/ULTIMA.EXE, build/out/<NAME>.OVL, build/out/DATA.OVL, then a
                     per-file comparison with the GOG files.

While parts of the source are still missing, a name no object defines may be resolved by the
address it carries (ULTIMA_1850 is CS:1850, D_5893 is DS:5893): such references are counted
and listed, and the gate (tools/check.py) accepts none.
"""
import argparse, glob, hashlib, os, re, struct, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402
import asmgen  # noqa: E402

OBJ = os.path.join(u5.BUILD, 'obj')
OUT = os.path.join(u5.BUILD, 'out')
ROOT_END = 0x86f0
# Runtime data segments no code refers to and whose bytes are not unique, measured: the
# message table's terminators. PAD is a COMMON segment (every module's 'ff ff' overlaid),
# followed by EPAD. crt0's STACK lies beyond DGROUP and is not part of DATA.OVL.
FIXED_DATA = {'PAD': 0xa520, 'EPAD': 0xa522}
SKIP_SEGMENTS = {'STACK', '_BSS', 'c_common'}
LINKER_DEFINED = {'_edata', '_end'}


class Module:
    """An object and where its segments go."""
    def __init__(self, name, obj, kind, overlay=None):
        self.name, self.obj, self.kind, self.overlay = name, obj, kind, overlay
        self.code = None          # (segment, CS address)
        self.data = {}            # segment name -> DS address


class Linker:
    def __init__(self, gamedata):
        self.gd = gamedata
        self.orig_img = u5.exe_image(gamedata)
        self.ds_orig = u5.ds_image(gamedata)
        self.modules = []
        self.publics = {}         # name -> (kind 'code'|'data', address, module)
        self.by_convention = {}   # name -> count of references resolved by their name only
        self.unresolved = {}
        self.relocs = []          # image addresses of relocated words
        self.notes = []

    # -- collecting
    def add(self, m):
        self.modules.append(m)

    def code_segment(self, obj):
        for s in obj.segs[1:]:
            if s.cls == 'CODE' and s.length:
                return s
        return None

    # -- placement
    def place(self):
        import match
        for m in self.modules:
            seg = self.code_segment(m.obj)
            if m.kind == 'ovl':
                m.code = (seg, u5.OVERLAY_BASE[m.overlay]) if seg else None
            elif m.kind in ('root', 'asm', 'lib'):
                m.code = (seg, m.addr) if seg else None
            # data: where the module's references into its own data point in the original
            if m.kind in ('ovl', 'root') and m.cfile:
                try:
                    mm = match.Module(m.cfile, self.gd)
                    mm.vote_data_bases(mm.functions())
                    m.data = dict(mm.data_base)
                except Exception as e:                       # noqa: BLE001
                    self.notes.append('%s: no data placement (%s)' % (m.name, e))
            elif m.kind in ('lib', 'asm') and m.code:
                m.data = self.place_data_by_code(m)
        for m in self.modules:
            if not m.code:
                continue
            seg, base = m.code
            for n, off in seg.publics.items():
                self.publics.setdefault(n, ('code', base + off, m))
            for sname, dsbase in m.data.items():
                s = m.obj.seg(sname)
                if s:
                    for n, off in s.publics.items():
                        self.publics.setdefault(n, ('data', dsbase + off, m))
        # symbols the linker defines (end of the initialised data, end of DGROUP): their
        # values, measured where the runtime's code uses them in the original
        for m in self.modules:
            if m.kind != 'lib' or not m.code:
                continue
            seg, base = m.code
            for fx in seg.fixups:
                if fx.target[0] == 'ext' and fx.target[1] in LINKER_DEFINED and not fx.self_rel \
                        and fx.kind == omf.LOC_OFFSET and fx.target[1] not in self.publics:
                    w = struct.unpack_from('<H', self.orig_img, base + fx.loc)[0]
                    v = (w - fx.disp - struct.unpack_from('<H', seg.data, fx.loc)[0]) & 0xffff
                    self.publics[fx.target[1]] = ('data', v, m)
                    self.notes.append('linker symbol %s = DS:%04x (measured)' % (fx.target[1], v))

    def place_data_by_code(self, m):
        """For a module whose code is the original's byte for byte (library, assembly): each
        reference from its code into one of its own data segments shows, in the original,
        where that segment starts. Segments no code refers to are found by their bytes."""
        import libmatch
        seg, base = m.code
        votes = {}
        for fx in seg.fixups:
            kind, name = fx.target
            if fx.self_rel or fx.kind != omf.LOC_OFFSET or kind != 'seg' or name == seg.name:
                continue
            s = m.obj.seg(name)
            if not s or s.cls == 'CODE':
                continue
            w = struct.unpack_from('<H', self.orig_img, base + fx.loc)[0]
            b = (w - fx.disp - struct.unpack_from('<H', seg.data, fx.loc)[0]) & 0xffff
            votes.setdefault(name, {}).setdefault(b, 0)
            votes[name][b] += 1
        out = {n: max(v, key=v.get) for n, v in votes.items()}
        for n, at in FIXED_DATA.items():
            if m.obj.seg(n) and n not in out:
                out[n] = at
        for s in m.obj.segs[1:]:
            if s.cls != 'CODE' and s.length and s.name not in out and any(s.data):
                at = libmatch.find(self.ds_orig, s)
                if len(at) == 1:
                    out[s.name] = at[0]
        return out

    # -- resolution
    def resolve(self, name, src):
        """(kind, address) for a name referenced from module src."""
        p = self.publics.get(name)
        if p:
            kind, addr, owner = p
            # into another overlay's function: through its stub
            if kind == 'code' and owner.kind == 'ovl' and owner.overlay != src.overlay:
                st = self.publics.get('STUB_' + name[:26])
                if st:
                    return st[0], st[1]
            return kind, addr
        a = u5.name_address(name)
        if a:
            self.by_convention[name] = self.by_convention.get(name, 0) + 1
            if a[0] == 'code':
                owner = u5.module_owner(name)
                if owner in u5.OVERLAY_BASE and owner != src.overlay:
                    st = self.publics.get('STUB_' + name[:26])
                    if st:
                        return st[0], st[1]
            return a
        self.unresolved[name] = self.unresolved.get(name, 0) + 1
        return None

    def frame(self, target, m):
        """The paragraph a segment reference stands for."""
        kind, name = target
        if kind == 'grp' and name == 'DGROUP' or name in ('_DATA', 'CONST', '_BSS', 'DGROUP'):
            return u5.DGROUP_SEG
        if name in ('_TEXT',) or kind == 'ext':
            return 0
        for v, n in asmgen.SEGVALUES.items():
            if n == name:
                return v
        mm = re.match(r'SEG_([0-9A-F]{4})$', name or '')
        if mm:
            return int(mm.group(1), 16)
        self.notes.append('%s: unknown segment %r' % (m.name, target))
        return 0

    def apply(self, m, seg, base, buf, bufbase, data_seg=False):
        """Copy seg's bytes into buf (whose index 0 is address bufbase) at base, applying
        its fixups. data_seg: the segment is in DGROUP (addresses are DS offsets)."""
        data = bytearray(seg.data)
        for fx in seg.fixups:
            loc = base + fx.loc
            kind, name = fx.target
            if fx.kind == omf.LOC_BASE:
                v = self.frame(fx.target, m)
                struct.pack_into('<H', data, fx.loc, v)
                if not data_seg and bufbase == 0 and loc < ROOT_END:
                    self.relocs.append(loc)
                continue
            if kind == 'ext':
                r = self.resolve(name, m)
                if r is None:
                    continue
                taddr = r[1]
            elif kind in ('seg', 'grp'):
                if name == '_TEXT' or (m.code and name == m.code[0].name):
                    taddr = m.code[1] if m.code else 0
                elif name in m.data:
                    taddr = m.data[name]
                elif name == 'DGROUP':
                    taddr = 0
                else:
                    self.notes.append('%s: fixup to %r without a placement' % (m.name, fx.target))
                    continue
            else:
                continue
            if fx.kind == omf.LOC_OFFSET:
                if fx.self_rel:
                    v = taddr + fx.disp - (loc + 2)
                else:
                    v = taddr + fx.disp + struct.unpack_from('<H', data, fx.loc)[0]
                struct.pack_into('<H', data, fx.loc, v & 0xffff)
            elif fx.kind == omf.LOC_LOBYTE and fx.self_rel:
                data[fx.loc] = (taddr + fx.disp - (loc + 1)) & 0xff
        off = base - bufbase
        buf[off:off + len(data)] = data

    # -- output
    def link(self):
        self.place()
        root = bytearray(self.orig_img) if self.fill else bytearray(ROOT_END)
        dgroup = bytearray(self.ds_orig) if self.fill else bytearray(len(self.ds_orig))
        overlays = {}
        for m in self.modules:
            if m.code:
                seg, base = m.code
                if m.kind == 'ovl':
                    size = len(u5.read_file(self.gd, m.overlay + '.OVL'))
                    buf = overlays.setdefault(m.overlay, bytearray(size))
                    self.apply(m, seg, base, buf, u5.OVERLAY_BASE[m.overlay])
                else:
                    self.apply(m, seg, base, root, 0)
            for sname, dsbase in m.data.items():
                s = m.obj.seg(sname)
                if s and s.length and sname not in SKIP_SEGMENTS:
                    self.apply(m, s, dsbase, dgroup, 0, data_seg=True)
        return root, overlays, dgroup


def gather(lk):
    """The modules the build has objects for."""
    import libmatch
    have = {os.path.splitext(os.path.basename(p))[0].upper(): p for p in glob.glob(os.path.join(OBJ, '*.OBJ'))}
    import asmbuild
    for p in sorted(glob.glob(os.path.join(u5.SRC, 'asm', '*.ASM'))):
        n = os.path.splitext(os.path.basename(p))[0].upper()
        if n in have:
            m = Module(n, omf.Object(have[n]), 'asm')
            m.addr = asmbuild.module_range(p)[0]
            m.cfile = None
            lk.add(m)
    for f, (kind, ovl) in sorted(u5.MODULES.items()):
        n = os.path.splitext(f)[0].upper()
        if n not in have or kind == 'data':
            continue
        if kind == 'ovl':
            m = Module(n, omf.Object(have[n]), 'ovl', ovl)
            m.cfile = f
            lk.add(m)
        elif kind == 'root':
            obj = omf.Object(have[n])
            seg = lk.code_segment(obj)
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
                lk.notes.append('%s: no function with an address; not placed' % f)
                continue
            m = Module(n, obj, 'root')
            m.addr, m.cfile = base, f
            lk.add(m)
    # the runtime library modules, where tools/libmatch.py finds them
    img = lk.orig_img
    found = {}
    for lib in ('SLIBCR.LIB', 'LIBH.LIB'):
        for name, obj in omf.library_modules(os.path.join(u5.MSC51, 'LIB', lib)):
            for seg in libmatch.code_segments(obj):
                at = libmatch.find(img, seg)
                if len(at) == 1 and at[0] < 0x878:
                    found.setdefault(at[0], []).append((name, obj))
    # modules with the same code (alshl, alshr, aulshr) match the same place: the one linked
    # is the one whose own references the other modules found satisfy
    defined = {p for c in found.values() for n, o in c for s in o.segs[1:] for p in s.publics}
    for addr, cands in sorted(found.items()):
        ok = [(n, o) for n, o in cands if all(e in defined or not e.startswith('__aN') for e in o.exts[1:])]
        name, obj = (ok or cands)[0]
        m = Module(name, obj, 'lib')
        m.addr, m.cfile = addr, None
        lk.add(m)


def compare(name, ours, orig):
    if ours == orig:
        return '%-13s identical (%d bytes)' % (name, len(orig))
    diff = [i for i in range(min(len(ours), len(orig))) if ours[i] != orig[i]]
    return '%-13s %d of %d bytes differ%s%s' % (name, len(diff) + abs(len(ours) - len(orig)), len(orig),
                                               ', first at %04x' % diff[0] if diff else '',
                                               '' if len(ours) == len(orig) else ', length %d' % len(ours))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--out', default=OUT)
    ap.add_argument('--fill', action='store_true',
                    help='start from the original bytes where no object covers them (shows how close the parts are)')
    ap.add_argument('-v', '--verbose', action='store_true')
    a = ap.parse_args()
    gd = u5.find_gamedata()
    lk = Linker(gd)
    lk.fill = a.fill
    gather(lk)
    root, overlays, dgroup = lk.link()
    os.makedirs(a.out, exist_ok=True)
    for ovl, buf in sorted(overlays.items()):
        open(os.path.join(a.out, ovl + '.OVL'), 'wb').write(buf)
        print(compare(ovl + '.OVL', bytes(buf), u5.read_file(gd, ovl + '.OVL')))
    print(compare('resident', bytes(root), lk.orig_img))
    print(compare('DGROUP', bytes(dgroup), lk.ds_orig))
    print('modules linked: %d (%s)' % (len(lk.modules), ', '.join('%s %d' % (k, sum(1 for m in lk.modules if m.kind == k))
                                                              for k in ('lib', 'asm', 'root', 'ovl'))))
    if lk.by_convention:
        print('references resolved only by the address in their name: %d names' % len(lk.by_convention))
        if a.verbose:
            print('  ' + ' '.join(sorted(lk.by_convention)[:60]))
    if lk.unresolved:
        print('unresolved: %s' % ', '.join(sorted(lk.unresolved)[:30]))
    for n in lk.notes[:20]:
        print('note: ' + n)


if __name__ == '__main__':
    main()
