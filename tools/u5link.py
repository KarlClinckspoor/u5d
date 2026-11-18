#!/usr/bin/env python3
"""Link the objects into ULTIMA.EXE, the overlays and DATA.OVL.

    python tools/u5link.py [--out DIR] [--script FILE] [-v]

PLINK86, which linked the original, is not available, and MS LINK's overlays are a different
scheme; this is a small linker for this one program (Exhume's docs/link.md calls it a link
stage of "facts": where things go is a fact about the original, how references are resolved
is the linker's rule). It links from the link script (src/u5d.lnk) and the objects
(build/obj, the runtime's modules from build/msc51/LIB) alone: no game file is opened to
link. What it does:

  placement (facts)  every module's code and data go where the link script says: the
                     resident modules at their CS addresses, each overlay's objects at their
                     offsets in the overlay, every data segment at its DGROUP offset. The
                     script is measured from the original by tools/linkscript.py;
  resolution (rules) a name is resolved to the public that defines it; a call from one
                     overlay (or the resident code) into a function of another overlay goes
                     to that function's stub (STUB_<name>, src/asm/PLTAB.ASM), as PLINK86
                     did; segment references become the paragraphs of their segments, and
                     those in the resident image become relocation entries, those in
                     an overlay the entries of its header (INTRO.OVL's, the only one);
                     uninitialised (communal) variables are allocated after the last _BSS;
  output             build/out/ULTIMA.EXE (the MZ header with its relocation table, then the
                     resident image), build/out/<NAME>.OVL, build/out/DATA.OVL (its header,
                     then DGROUP), then a per-file comparison: with the GOG files when they
                     are found (tools/u5.py, find_gamedata), else with their SHA-1s
                     (u5.GOG_SHA1). The exit status is 0 when all 25 are identical and every
                     name was defined.

A name no object defines is still resolved by the address it carries (ULTIMA_1850 is CS:1850,
D_5893 is DS:5893), so that a partial build links: such references are counted and listed,
and the exit status is 1 while there are any. The gate (tools/check.py) accepts none.
"""
import argparse, hashlib, os, re, struct, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402
import asmgen  # noqa: E402

OUT = os.path.join(u5.BUILD, 'out')
SCRIPT = os.path.join(u5.SRC, 'u5d.lnk')
SKIP_SEGMENTS = {'STACK', '_BSS', 'c_common'}
LINKER_DEFINED = {'_edata', '_end'}


class Script:
    """The link script (src/u5d.lnk; its comments say what each kind of line is)."""
    def __init__(self, path=None):
        self.path = path or SCRIPT
        self.exe, self.dgroup, self.defines = {}, {}, {}
        self.overlays = {}        # name -> {'base', 'size', 'header'}, in the script's order
        self.modules = []         # (library and public or None, name, overlay or None, code address or None, {segment: DS offset})
        if not os.path.exists(self.path):
            raise SystemExit('the link script %s is missing (tools/linkscript.py writes it)' % self.path)
        for no, line in enumerate(open(self.path, encoding='ascii'), 1):
            w = line.split('#')[0].split()
            if not w:
                continue
            try:
                kw = dict(x.split('=', 1) for x in w[1:] if '=' in x)
                names = [x for x in w[1:] if '=' not in x]
                if w[0] == 'exe':
                    self.exe.update((k, int(v, 16)) for k, v in kw.items())
                elif w[0] == 'dgroup':
                    self.dgroup.update((k, int(v, 16)) for k, v in kw.items())
                elif w[0] == 'define':
                    self.defines.update((k, int(v, 16)) for k, v in kw.items())
                elif w[0] == 'overlay':
                    self.overlays[names[0]] = {k: int(v, 16) for k, v in kw.items()}
                elif w[0] in ('object', 'library'):
                    lib = (names.pop(0), names.pop(1)) if w[0] == 'library' else None
                    code, ovl = kw.pop('code', None), None
                    if code and ':' in code:
                        ovl, code = code.split(':')
                        code = self.overlays[ovl]['base'] + int(code, 16)
                    elif code:
                        code = int(code, 16)
                    self.modules.append((lib, names[0], ovl, code, {k: int(v, 16) for k, v in kw.items()}))
                else:
                    raise ValueError('unknown kind of line')
            except (ValueError, KeyError, IndexError) as e:
                raise SystemExit('%s, line %d: %s (%s)' % (self.path, no, line.strip(), e))


def code_segment(obj):
    for s in obj.segs[1:]:
        if s.cls == 'CODE' and s.length:
            return s
    return None


def module_publics(obj):
    return {n for s in obj.segs[1:] for n in s.publics}


class Module:
    """An object and where its segments go. kind: 'lib' (a module of the runtime), 'res' (a
    resident object), 'ovl' (an object of an overlay), 'data' (no code: communal variables)."""
    def __init__(self, name, obj, kind, overlay=None):
        self.name, self.obj, self.kind, self.overlay = name, obj, kind, overlay
        self.code = None          # (segment, CS address)
        self.data = {}            # segment name -> DS address
        self.relocs = []          # addresses of its code's relocated words, in fixup order
        self.data_relocs = []     # (DS address, segment name) of its data's relocated words


class Linker:
    def __init__(self, script):
        self.script = script
        self.root_end = script.exe['image']
        self.modules = []
        self.publics = {}         # name -> (kind 'code'|'data', address, module)
        self.by_convention = {}   # name -> count of references resolved by their name only
        self.unresolved = {}
        self.notes = []
        self.fill = None          # --fill: the folder of the game files

    # -- collecting
    def add(self, m):
        self.modules.append(m)

    def gather(self):
        """The modules the script names, each with its placement. An object that was not
        built is left out (a partial build links; the gate counts the objects)."""
        libs = {}
        for lib, name, ovl, code, data in self.script.modules:
            if lib:
                lib, public = lib
                if lib not in libs:
                    libs[lib] = omf.library_modules(os.path.join(u5.MSC51, 'LIB', lib))
                objs = [o for n, o in libs[lib] if n == name and public in module_publics(o)]
                if len(objs) != 1:
                    self.notes.append('%s has %d modules %s that define %s' % (lib, len(objs), name, public))
                    continue
                m = Module(name, objs[0], 'lib')
            else:
                p = os.path.join(u5.OBJ, name + '.OBJ')
                if not os.path.exists(p):
                    self.notes.append('%s: no object' % name)
                    continue
                m = Module(name, omf.Object(p), 'ovl' if ovl else 'res' if code is not None else 'data', ovl)
            seg = code_segment(m.obj)
            if seg and code is None and m.kind != 'data':
                self.notes.append('%s has code, but no code address in the script' % name)
            m.code = (seg, code) if seg and code is not None else None
            m.data = dict(data)
            for s in data:
                if not m.obj.seg(s):
                    self.notes.append('%s has no segment %s' % (name, s))
            self.add(m)

    # -- placement
    def place(self):
        for m in self.modules:
            if m.code:
                seg, base = m.code
                for n, off in seg.publics.items():
                    self.publics.setdefault(n, ('code', base + off, m))
            for sname, dsbase in m.data.items():
                s = m.obj.seg(sname)
                if s:
                    for n, off in s.publics.items():
                        self.publics.setdefault(n, ('data', dsbase + off, m))
        # symbols the linker defines (end of the initialised data, end of DGROUP)
        for n, v in self.script.defines.items():
            self.publics.setdefault(n, ('data', v, None))
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
                # a call into another overlay with no stub would be linked straight to the
                # function: say so (a function renamed without its STUB_ name in PLTAB.ASM)
                note = '%s is called from %s but has no stub STUB_%s (src/asm/PLTAB.ASM)' % (
                    name, src.name, name[:26])
                if note not in self.notes:
                    self.notes.append(note)
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
        elif (bufbase == 0 and loc < self.root_end) or m.kind == 'ovl':
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
        point at PLINK86's start-up segment. The rest is PLINK86's, as the link script's exe
        line has it."""
        x = self.script.exe
        relocs = []
        for loc in self.relocation_order([m for m in self.modules if m.kind != 'ovl']):
            m = next(x for x in self.modules if x.kind != 'ovl' and loc in x.relocs)
            f = self.segment_frame(m)
            relocs.append((loc - f * 16, f))
        table = b''.join(struct.pack('<HH', o, s) for o, s in relocs)
        hsize = (x['fixed'] + len(table) + 511) // 512 * 512
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
                          len(image) // 16 + x['stackgap'], x['sp'],    # SS:SP, PLINK86's start-up stack
                          x['checksum'], 0, entry, x['fixed'], 0)
        hdr += struct.pack('<H', x['word1c'])                                    # PLINK86 writes 1 here
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
        return self.header_entries(entries, self.script.dgroup['header'], 'DATA.OVL')

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
        sc = self.script
        root, dgroup = bytearray(self.root_end), bytearray(sc.dgroup['size'])
        overlays = {}
        if self.fill:                      # --fill: start from the original's bytes
            root = bytearray(u5.exe_image(self.fill))
            dgroup = bytearray(u5.ds_image(self.fill))
        for m in self.modules:
            if m.code:
                seg, base = m.code
                if m.kind == 'ovl':
                    o = sc.overlays[m.overlay]
                    buf = overlays.setdefault(m.overlay, bytearray(o['size']))
                    self.apply(m, seg, base, buf, o['base'])
                else:
                    self.apply(m, seg, base, root, 0)
            for sname, dsbase in m.data.items():
                s = m.obj.seg(sname)
                if s and s.length and sname not in SKIP_SEGMENTS:
                    self.apply(m, s, dsbase, dgroup, 0, data_seg=True)
        # an overlay's relocation list is its header: a dword (CS offset, segment 0) per
        # relocated word, in relocation_order, then zeros
        for ovl in sorted(overlays):
            mods = [m for m in self.modules if m.kind == 'ovl' and m.overlay == ovl]
            locs = self.relocation_order(mods)
            size = sc.overlays[ovl]['header']
            if locs or size:
                overlays[ovl][:size] = self.header_entries([(a, 0) for a in locs], size, ovl + '.OVL')
        return root, overlays, dgroup


def compare(name, ours, orig):
    if ours == orig:
        return '%-13s identical (%d bytes)' % (name, len(orig))
    diff = [i for i in range(min(len(ours), len(orig))) if ours[i] != orig[i]]
    return '%-13s %d of %d bytes differ%s%s' % (name, len(diff) + abs(len(ours) - len(orig)), len(orig),
                                               ', first at %04x' % diff[0] if diff else '',
                                               '' if len(ours) == len(orig) else ', length %d' % len(ours))


def link(out=OUT, fill=None, script=None):
    """Link what build/obj holds, as the link script says, and write the 25 files to out.
    Returns (linker, {file name: our bytes}). fill: the folder of the game files, to start
    from the original's bytes; nothing else reads a game file."""
    lk = Linker(Script(script))
    lk.fill = fill
    lk.gather()
    root, overlays, dgroup = lk.link()
    files = {ovl + '.OVL': bytes(buf) for ovl, buf in overlays.items()}
    files['ULTIMA.EXE'] = lk.exe_header(root) + bytes(root)
    files['DATA.OVL'] = lk.data_header() + bytes(dgroup)
    os.makedirs(out, exist_ok=True)
    for name, data in files.items():
        open(os.path.join(out, name), 'wb').write(data)
    return lk, dict(sorted(files.items()))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--out', default=OUT)
    ap.add_argument('--script', help='the link script (default: src/u5d.lnk)')
    ap.add_argument('--fill', action='store_true',
                    help='start from the original bytes where no object covers them (shows how close the '
                         'parts are; needs the game files)')
    ap.add_argument('-v', '--verbose', action='store_true')
    a = ap.parse_args()
    gd = u5.find_gamedata() if a.fill else u5.find_gamedata(required=False)
    lk, files = link(a.out, gd if a.fill else None, a.script)
    bad = 0
    print('compared with ' + ('the game files in %s' % gd if gd else 'the SHA-1s of the GOG files (no game files found)'))
    for name in sorted(set(files) | set(u5.GOG_SHA1)):
        ours = files.get(name)
        if ours is None:
            print('%-13s not built' % name)
            bad += 1
            continue
        if not gd:
            same = hashlib.sha1(ours).hexdigest() == u5.GOG_SHA1.get(name)
            print('%-13s %s (%d bytes)' % (name, 'identical' if same else 'DIFFERENT', len(ours)))
            bad += not same
            continue
        orig = u5.read_file(gd, name)
        print(compare(name, ours, orig))
        bad += ours != orig
        if name == 'ULTIMA.EXE' and ours != orig:
            hdr = len(ours) - lk.root_end
            print('  ' + compare('header', ours[:hdr], orig[:hdr]))
            print('  ' + compare('image', ours[hdr:], orig[hdr:]))
        if name == 'DATA.OVL' and ours != orig:
            hdr = lk.script.dgroup['header']
            print('  ' + compare('header', ours[:hdr], orig[:hdr]))
            print('  ' + compare('DGROUP', ours[hdr:], orig[hdr:]))
    print('modules linked: %d (%s)' % (len(lk.modules), ', '.join(
        '%s %d' % (t, sum(1 for m in lk.modules if m.kind == k))
        for k, t in (('lib', 'library'), ('res', 'resident'), ('ovl', 'overlay'), ('data', 'data only')))))
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
