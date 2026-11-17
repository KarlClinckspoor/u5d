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
                     those in the resident image become relocation entries, those in
                     an overlay the entries of its header (INTRO.OVL's, the only one);
  output             build/out/ULTIMA.EXE (the MZ header with its relocation table, then the
                     resident image), build/out/<NAME>.OVL, build/out/DATA.OVL (its header,
                     then DGROUP), then a per-file comparison with the GOG files. The exit
                     status is 0 when all 25 are identical and every name was defined.

A name no object defines is still resolved by the address it carries (ULTIMA_1850 is CS:1850,
D_5893 is DS:5893), so that a partial build links: such references are counted and listed,
and the exit status is 1 while there are any. The gate (tools/check.py) accepts none.
"""
import argparse, glob, os, re, struct, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402
import asmgen  # noqa: E402

OUT = os.path.join(u5.BUILD, 'out')
ROOT_END = 0x86f0
# Runtime data segments no code refers to and whose bytes are not unique, measured: the
# message table's terminators. PAD is a COMMON segment (every module's 'ff ff' overlaid),
# followed by EPAD. crt0's STACK lies beyond DGROUP and is not part of DATA.OVL.
FIXED_DATA = {'PAD': 0xa520, 'EPAD': 0xa522}
SKIP_SEGMENTS = {'STACK', '_BSS', 'c_common'}
LINKER_DEFINED = {'_edata', '_end'}
# ULTIMA.EXE's header, the fields that are PLINK86's own (measured, not derived):
EXE_FIXED = 0x1e          # the relocation table follows a 0x1e-byte header (a word, 1, at 0x1c)
EXE_STACK_GAP = 0x100     # SS is this many paragraphs past the end of the image,
EXE_SP = 0x64             # and SP 0x64: the start-up code's own stack
EXE_CHECKSUM = 0x7e39     # not the MZ checksum of the file (that would be 0x0d37); what
                          # PLINK86 summed is not known


class Module:
    """An object and where its segments go."""
    def __init__(self, name, obj, kind, overlay=None):
        self.name, self.obj, self.kind, self.overlay = name, obj, kind, overlay
        self.code = None          # (segment, CS address)
        self.data = {}            # segment name -> DS address
        self.relocs = []          # addresses of its code's relocated words, in fixup order
        self.data_relocs = []     # (DS address, segment name) of its data's relocated words


class Linker:
    def __init__(self, gamedata):
        self.gd = gamedata
        self.orig_img = u5.exe_image(gamedata)
        self.ds_orig = u5.ds_image(gamedata)
        self.modules = []
        self.publics = {}         # name -> (kind 'code'|'data', address, module)
        self.by_convention = {}   # name -> count of references resolved by their name only
        self.unresolved = {}
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
                # at the area base, or, for the second object of an overlay built from two,
                # where its first function's name says it starts
                base = u5.OVERLAY_BASE[m.overlay]
                for pn, off in sorted(seg.publics.items(), key=lambda kv: kv[1]) if seg else ():
                    mm = u5.NAME_RE.match(pn)
                    if mm and mm.group(1).upper() == m.overlay:
                        base += int(mm.group(2), 16) - off
                        break
                m.code = (seg, base) if seg else None
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
            elif m.code and (m.kind in ('lib', 'asm') or not m.cfile):   # assembly, also in an overlay
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
        self.allocate_communals()
        # a second name for a place another name defines (u5.LINK_DEFINES)
        for name, other in u5.LINK_DEFINES.items():
            if other in self.publics and name not in self.publics:
                self.publics[name] = self.publics[other]

    def allocate_communals(self):
        """Uninitialised variables (MSC's COMDEF records: a name and a size, in every object
        that declares the variable without extern) that no object defines otherwise get their
        place here, in the segment c_common: after the last segment of class BSS (the modules'
        _BSS, their static variables, then the runtime's empty XOB/XO/XOE), one after the
        other, a variable of two bytes or more on an even address. The order is PLINK86's
        and is a fact (u5.COMMUNALS): MSC writes an object's COMDEF records in the order of
        its symbol table, which depends on the original's names."""
        sizes = {}
        for m in self.modules:
            for n, size in m.obj.comdefs.items():
                if n not in self.publics:
                    if sizes.get(n, size) != size:
                        self.notes.append('%s: communal %s has %d bytes here, %d elsewhere' % (m.name, n, size, sizes[n]))
                    sizes[n] = max(size, sizes.get(n, 0))
        if not sizes:
            return
        at = max([b + m.obj.seg(s).length for m in self.modules for s, b in m.data.items()
                  if m.obj.seg(s) and m.obj.seg(s).cls == 'BSS'] or [self.publics.get('_edata', (0, 0))[1]])
        self.notes.append('c_common starts at DS:%04x (measured: the end of the BSS segments)' % at)
        for n in u5.COMMUNALS:
            if n not in sizes:
                self.notes.append('communal %s (u5.COMMUNALS) is declared by no object' % n)
                continue
            if sizes[n] >= 2:
                at += at & 1
            self.publics[n] = ('data', at, None)
            a = u5.name_address(n)
            if a and a[1] != at:
                self.notes.append('communal %s is allocated at DS:%04x' % (n, at))
            at += sizes.pop(n)
        for n in sorted(sizes):
            self.notes.append('communal %s is not in u5.COMMUNALS: not allocated' % n)
        end = self.publics.get('_end')
        if end and end[1] != at + (at & 1):
            self.notes.append('c_common ends at DS:%04x, _end is DS:%04x' % (at, end[1]))

    def place_data_by_code(self, m):
        """For a module whose code is the original's byte for byte (library, assembly): each
        reference from its code into one of its own data segments shows, in the original,
        where that segment starts. Segments no code refers to are found by their bytes."""
        import libmatch
        seg, base = m.code
        img, at0 = self.orig_img, 0            # the original bytes the code sits in
        if m.kind == 'ovl':
            img, at0 = u5.read_file(self.gd, m.overlay + '.OVL'), u5.OVERLAY_BASE[m.overlay]
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
                at = libmatch.find(self.ds_orig, s)
                if len(at) == 1:
                    out[s.name] = at[0]
        return out

    # -- resolution
    def resolve(self, name, src):
        """(kind, address) for a name referenced from module src."""
        # the stubs themselves (PLTAB) jump to the function, not to a stub
        via_stub = src.name != 'PLTAB'
        # names count in their first 31 characters: MSC 5.1 and the assembly modules cut a
        # longer name there, MSC 5.0 writes one character more
        p = self.publics.get(name) or self.publics.get(name[:31])
        if p:
            kind, addr, owner = p
            # into another overlay's function: through its stub
            if via_stub and kind == 'code' and owner.kind == 'ovl' and owner.overlay != src.overlay:
                st = self.publics.get('STUB_' + name[:26])
                if st:
                    return st[0], st[1]
            return kind, addr
        a = u5.name_address(name)
        if a:
            self.by_convention[name] = self.by_convention.get(name, 0) + 1
            if a[0] == 'code':
                owner = u5.module_owner(name)
                if via_stub and owner in u5.OVERLAY_BASE and owner != src.overlay:
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

    def relocate(self, m, loc, bufbase, data_seg):
        """Note a word at loc that holds a segment: an entry of the EXE's relocation table,
        of the overlay's header, or (a word in DGROUP) of DATA.OVL's header. Each module
        keeps its own in the order of its fixup records."""
        if data_seg:
            m.data_relocs.append((loc, data_seg))
        elif (bufbase == 0 and loc < ROOT_END) or m.kind == 'ovl':
            m.relocs.append(loc)

    def relocation_order(self, modules, attr='relocs'):
        """The relocated words of modules in the order PLINK86 wrote them: module by module
        as linked (the order of their code), each module's in the order of its fixup records.
        MSC writes the fixups of a code record from the last to the first, MASM from the first
        to the last, so the original's table falls where a C object ends and rises through an
        assembly module. The parts of one original object (u5.OBJECT_PARTS: a C file and the
        assembly that stands for one of its functions) are taken together, from the last word
        to the first, as MSC wrote that object's single fixup record."""
        part_of = {n: g for g in u5.OBJECT_PARTS for n in g}
        out, done = [], set()
        for m in sorted((m for m in modules if m.code), key=lambda m: m.code[1]):
            if m.name in done:
                continue
            g = part_of.get(m.name)
            if g:
                done.update(g)
                out += sorted((r for x in modules if x.name in g for r in getattr(x, attr)), reverse=True)
            else:
                out += getattr(m, attr)
        return out

    def segment_frame(self, m, data_seg=None):
        """The paragraph PLINK86 names a relocated word's segment by, and that segment's
        address: the code segment's own paragraph (0 for _TEXT, 072e and 0778 for PLINK86's),
        or, in DGROUP, the paragraph the combined data segment starts in (_DATA starts at
        DS:0042, after the runtime's NULL segment: paragraph 0f68)."""
        if data_seg:
            start = min(x.data[data_seg] for x in self.modules if data_seg in x.data
                        and x.obj.seg(data_seg) and x.obj.seg(data_seg).length)
            return (u5.DGROUP_SEG * 16 + start) >> 4
        name = m.code[0].name
        for v, n in asmgen.SEGVALUES.items():
            if n == name and v != u5.DGROUP_SEG:
                return v
        return 0

    def header_entries(self, entries, size, what):
        """A relocation header as PLINK86 writes it before an overlay's image: a dword
        (offset, segment) per relocated word, a zero dword, then zeros to the paragraph."""
        hdr = b''.join(struct.pack('<HH', o, s) for o, s in entries) + bytes(4)
        hdr += bytes(-len(hdr) % 16)
        if len(hdr) != size:
            self.notes.append('%s: a header of %d bytes, where the original has %d' % (what, len(hdr), size))
        return hdr

    def exe_header(self, image):
        """ULTIMA.EXE's MZ header. Rules: the file size, the relocation count and table (after
        the 0x1e-byte header, in relocation_order), the header padded to 512 bytes, the entry
        point at PLINK86's start-up segment. The rest is PLINK86's, written as measured."""
        relocs = []
        for loc in self.relocation_order([m for m in self.modules if m.kind != 'ovl']):
            m = next(x for x in self.modules if x.kind != 'ovl' and loc in x.relocs)
            f = self.segment_frame(m)
            relocs.append((loc - f * 16, f))
        table = b''.join(struct.pack('<HH', o, s) for o, s in relocs)
        hsize = (EXE_FIXED + len(table) + 511) // 512 * 512
        size = hsize + len(image)
        start = next((m for m in self.modules if m.name == 'PLSTART' and m.code), None)
        entry = start.code[1] >> 4 if start else 0
        # memory beyond the image: up to the end of the stack that follows DGROUP (crt0's
        # STACK segment), plus the start-up segment's size once more (measured: PLINK86 adds
        # every segment's size, and the start-up code lies under overlay area 1)
        top = u5.DGROUP_SEG + (self.publics.get('_end', (0, 0))[1] + 15) // 16 + self.stack_size() // 16
        minalloc = top + ((start.code[0].length + 15) // 16 if start else 0) - len(image) // 16
        hdr = struct.pack('<2s13H', b'MZ', size % 512, (size + 511) // 512, len(relocs), hsize // 16,
                          minalloc, 0xffff,
                          len(image) // 16 + EXE_STACK_GAP, EXE_SP,      # SS:SP, PLINK86's start-up stack
                          EXE_CHECKSUM, 0, entry, EXE_FIXED, 0)
        hdr += struct.pack('<H', 1)                                     # PLINK86 writes 1 here
        return hdr + table + bytes(hsize - len(hdr) - len(table))

    def stack_size(self):
        for m in self.modules:
            s = m.obj.seg('STACK') if m.kind == 'lib' else None
            if s and s.length:
                return s.length
        return 0

    def data_header(self):
        """DATA.OVL's header: the far pointers in DGROUP (the words holding a segment)."""
        entries = []
        mods = [m for m in self.modules if m.kind != 'ovl']
        for loc, sname in self.relocation_order(mods, 'data_relocs'):
            f = self.segment_frame(None, sname)
            entries.append((u5.DGROUP_SEG * 16 + loc - f * 16, f))
        return self.header_entries(entries, u5.DATA_OVL_HEADER, 'DATA.OVL')

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
                self.relocate(m, loc, bufbase, data_seg and seg.name)
                continue
            if kind == 'ext':
                r = self.resolve(name, m)
                if r is None:
                    continue
                taddr = r[1]
                # the segment of a far pointer to it (a function's is the code segment, 0)
                segv = u5.DGROUP_SEG if r[0] == 'data' else 0
            elif kind in ('seg', 'grp'):
                segv = None
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
            # 5: an offset the loader may resolve, which MSC writes for the address of a far
            # function (D_5394_fn = ULTIMA_2322_DiskSwapMessage); it is placed as an offset
            if fx.kind in (omf.LOC_OFFSET, 5):
                if fx.self_rel:
                    v = taddr + fx.disp - (loc + 2)
                else:
                    v = taddr + fx.disp + struct.unpack_from('<H', data, fx.loc)[0]
                struct.pack_into('<H', data, fx.loc, v & 0xffff)
            elif fx.kind == omf.LOC_POINTER and not fx.self_rel:
                # offset and segment (a far call to a far function of another object)
                v = taddr + fx.disp + struct.unpack_from('<H', data, fx.loc)[0]
                if segv is None:
                    segv = self.frame(fx.target, m)
                struct.pack_into('<HH', data, fx.loc, v & 0xffff, segv)
                self.relocate(m, loc + 2, bufbase, data_seg and seg.name)
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
        # an overlay's relocation list is its header (tools/u5.py OVERLAY_HEADER): a dword
        # (CS offset, segment 0) per relocated word, in relocation_order, then zeros
        for ovl in sorted(overlays):
            mods = [m for m in self.modules if m.kind == 'ovl' and m.overlay == ovl]
            locs = self.relocation_order(mods)
            size = u5.OVERLAY_HEADER.get(ovl, 0)
            if locs or size:
                overlays[ovl][:size] = self.header_entries([(a, 0) for a in locs], size, ovl + '.OVL')
        return root, overlays, dgroup


def gather(lk):
    """The modules the build has objects for."""
    import libmatch
    have = {os.path.splitext(os.path.basename(p))[0].upper(): p for p in glob.glob(os.path.join(u5.OBJ, '*.OBJ'))}
    import asmbuild
    for p in sorted(glob.glob(os.path.join(u5.SRC, 'asm', '*.ASM'))):
        n = os.path.splitext(os.path.basename(p))[0].upper()
        if n in have:
            place = asmbuild.module_place(p)
            if place[0]:              # part of an overlay: placed by its functions' names
                m = Module(n, omf.Object(have[n]), 'ovl', place[0])
            else:
                m = Module(n, omf.Object(have[n]), 'asm')
                m.addr = place[1]
            m.cfile = None
            lk.add(m)
    for f, (kind, ovl) in sorted(u5.MODULES.items()):
        n = os.path.splitext(f)[0].upper()
        if n not in have:
            continue
        if kind == 'data':                # no code or data of its own: communal variables
            m = Module(n, omf.Object(have[n]), 'data')
            m.cfile = None
            lk.add(m)
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


def link(out=OUT, fill=False):
    """Link what build/obj holds and write the 25 files to out. Returns (linker, {file name:
    (our bytes, the original's)})."""
    gd = u5.find_gamedata()
    lk = Linker(gd)
    lk.fill = fill
    gather(lk)
    root, overlays, dgroup = lk.link()
    files = {ovl + '.OVL': bytes(buf) for ovl, buf in overlays.items()}
    files['ULTIMA.EXE'] = lk.exe_header(root) + bytes(root)
    files['DATA.OVL'] = lk.data_header() + bytes(dgroup)
    os.makedirs(out, exist_ok=True)
    for name, data in files.items():
        open(os.path.join(out, name), 'wb').write(data)
    return lk, {name: (data, u5.read_file(gd, name)) for name, data in sorted(files.items())}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--out', default=OUT)
    ap.add_argument('--fill', action='store_true',
                    help='start from the original bytes where no object covers them (shows how close the parts are)')
    ap.add_argument('-v', '--verbose', action='store_true')
    a = ap.parse_args()
    lk, files = link(a.out, a.fill)
    bad = 0
    for name, (ours, orig) in files.items():
        print(compare(name, ours, orig))
        bad += ours != orig
        if name == 'ULTIMA.EXE' and ours != orig:
            hdr = u5.EXE_HEADER
            print('  ' + compare('header', ours[:hdr], orig[:hdr]))
            print('  ' + compare('image', ours[hdr:], orig[hdr:]))
        if name == 'DATA.OVL' and ours != orig:
            hdr = u5.DATA_OVL_HEADER
            print('  ' + compare('header', ours[:hdr], orig[:hdr]))
            print('  ' + compare('DGROUP', ours[hdr:], orig[hdr:]))
    print('modules linked: %d (%s)' % (len(lk.modules), ', '.join('%s %d' % (k, sum(1 for m in lk.modules if m.kind == k))
                                                              for k in ('lib', 'asm', 'root', 'ovl'))))
    if lk.by_convention:
        print('references resolved only by the address in their name: %d names' % len(lk.by_convention))
        if a.verbose:
            print('  ' + ' '.join(sorted(lk.by_convention)))
    if lk.unresolved:
        print('unresolved: %s' % ', '.join(sorted(lk.unresolved)[:30]))
    for n in lk.notes[:20]:
        print('note: ' + n)
    sys.exit(1 if bad or lk.by_convention or lk.unresolved else 0)


if __name__ == '__main__':
    main()
