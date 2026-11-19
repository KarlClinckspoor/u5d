#!/usr/bin/env python3
"""Link the objects into ULTIMA.EXE, the overlays and DATA.OVL.

    python tools/u5link.py [--out DIR] [--script FILE] [--target T] [--map] [-v]

Origin linked Ultima V with Phoenix's PLINK86plus 2.24; this is a small linker for this one
program that follows PLINK's rules (docs/MATCHING.md, "Origin's link, reproduced"). It links
from the link command file (src/u5d.lnk) and the objects (build/obj, the runtime's modules
from build/msc51/LIB) alone: no game file is opened to link, and no Phoenix file is needed.

  placement (rules)  from the command file's processing order (files, overlay areas and
                     sections, libraries), as PLINK places things:
                     - the libraries are searched as one (MSC's SLIBCE.LIB is SLIBCR.LIB and
                       LIBH.LIB merged): for the names no file defines, in the order the files
                       first name them, then for the names the modules pulled name, breadth
                       first; the modules go where the library line is;
                     - segments go in the order of their classes as first met, within a class
                       in the order of their names as first met, each module's part in
                       processing order, on the segment's alignment (a COMMON segment's parts
                       on one another); DGROUP as DOSSEG orders it (crt0's COMENT): class
                       BEGDATA first, BSS and STACK last;
                     - the root from 0; the overlay areas one after the other from the
                       paragraph where the root ends, each as large as its largest section,
                       every section at its area's base; DGROUP from the paragraph after the
                       last area (DATA.OVL, the pre-loaded section);
                     - the communal variables after the last BSS segment, in the order of the
                       objects' COMDEF records, object by object in processing order (a name
                       allocated once), _edata where class BSS starts, _end at its end (word
                       aligned);
                     - the overlay table and call stubs (PLINK's $OVTB$, Linker.make_table),
                       an overlay's relocation header, DATA.OVL's header;
                     - Origin's copy-protection wrapper (PLSTART) appended at the end of the
                       program PLINK wrote;
  resolution (rules) a name is resolved to the public that defines it; a call from one
                     section into a function of another overlay goes to that function's stub
                     (STUB_<name>), as PLINK made it; segment references become the
                     paragraphs of their segments, and those in the resident image become
                     relocation entries, those in an overlay the entries of its header
                     (INTRO.OVL's, the only one);
  output             build/out/ULTIMA.EXE (the MZ header with its relocation table, then the
                     resident image), build/out/<NAME>.OVL, build/out/DATA.OVL (its header,
                     then DGROUP), then a per-file comparison: with the target's game files
                     when they are found (tools/u5.py, find_gamedata), else with their SHA-1s
                     (u5.release_sha1()). The exit status is 0 when all 25 are identical and
                     every name was defined.

What no rule settles is a fact of the command file, marked there: the processing order
itself, the order of the call stubs, and the stack of Origin's wrapper. tools/linkscript.py
--check compares every placement u5link computes with the original files. --map prints them.

The target (--target, else $U5_TARGET, else gog) chooses the objects (build/obj or
build/obj-1988, or $U5_OBJ), the command file's lines (those marked target= for another
target are left out) and the files compared with.

The EXE checksum is PLINK86's, by rule: the MZ checksum of the program as PLINK86 wrote it,
before Origin's copy-protection wrapper (PLSTART) was appended (see Linker.plink_header). The
GOG release is the 1988 link patched in 1992, and the patch did not write the checksum again:
for the gog target it is the checksum of the 1988 link (u5.PATCH_OF), linked from the 1988
objects of the modules that differ by target (build/obj-1988 beside build/obj, where
tools/asmbuild.py and dosbuild.py put them) and the gog objects of the others.

A name no object defines is still resolved by the address it carries (ULTIMA_1850 is CS:1850,
D_5893 is DS:5893), so that a partial build links: such references are counted and listed,
and the exit status is 1 while there are any. The gate (tools/check.py) accepts none.
"""
import argparse, hashlib, os, re, struct, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402

OUT = os.path.join(u5.BUILD, 'out')


def script_path():
    """The link command file, src/u5d.lnk (read when used: $U5_SRC may name another src/)."""
    return os.path.join(u5.SRC, 'u5d.lnk')


SKIP_SEGMENTS = {'STACK', '_BSS', 'c_common'}
LINKER_DEFINED = {'_edata', '_end'}
ALIGNMENT = {0: 1, 1: 1, 2: 2, 3: 16, 4: 256}     # SEGDEF alignment: byte, word, paragraph, page
COMMON = 6                                       # SEGDEF combine type: overlaid parts
DOSSEG = 0x9e                                    # COMENT class: DOSSEG segment order
# PLINK86plus 2.24's own EXE header fields (both Nantucket editions write them so): the
# relocation table at 0x1e, the word 1 at 0x1c.
PLINK_RELOC_TABLE, PLINK_WORD_1C = 0x1e, 0x0001
# The overlay table ($OVTB$): a section's record starts with its status word, as the loader
# (OVLYV, src/asm/PLMGR.ASM) reads it: bit 15 set while the section is loaded, bit 14 for a
# section loaded at start-up (its start-up loop loads those), the low 13 bits the section to
# load before it (0: the root). PLINK writes 6000 for the pre-loaded section (bit 13 too,
# which OVLYV never tests) and 0 for an overlay; ffff after the last record ends the
# loader's walk over the records.
STATUS_PRELOADED, STATUS_END = 0x6000, 0xffff
STUB = 12                                        # call far $OVLYV; dw overlay; jmp far function
STUB_NAME = 26                                   # STUB_ and 26 characters of the function's name
TABLE = 'PLTAB'                                  # the generated table's module and segment


def script_lines(text, target):
    """The lines of the command file that hold for target: a line with target=T holds for T
    only, and is given without that word."""
    out = []
    for line in text.splitlines():
        w = line.split('#')[0].split()
        t = [x for x in w if x.startswith('target=')]
        if t:
            if t[0][len('target='):] != target:
                continue
            line = line.replace(' ' + t[0], '', 1)
        out.append(line)
    return out


def exported(name):
    """A name as the objects hold it: upper case, 31 characters."""
    return name.upper()[:31]


class Script:
    """The link command file (src/u5d.lnk; its comments say what each kind of line is), the
    lines that hold for target (u5.TARGET)."""
    def __init__(self, path=None, target=None):
        self.path = path or script_path()
        self.target = target or u5.TARGET
        self.output = 'ULTIMA.EXE'
        self.order = []           # the processing order: (kind, name or libraries, section)
                                  #   kind 'file', 'library', 'loader' or 'table'; section None: the root
        self.sections = {}        # section -> area number (from 0), None for the pre-loaded one; in order
        self.areas = []           # [[sections]]
        self.defines = {}         # name -> the name of the place it stands for
        self.wrapper, self.wrapper_stack = None, {}
        self.stubs = []
        if not os.path.exists(self.path):
            raise SystemExit('the link command file %s is missing' % self.path)
        section, in_area = None, False
        for no, line in enumerate(open(self.path, encoding='ascii'), 1):
            w = line.split('#')[0].split()
            t = [x for x in w if x.startswith('target=')]
            if t:
                if t[0][len('target='):] not in u5.TARGETS:
                    raise SystemExit('%s, line %d: unknown target (%s)' % (self.path, no, line.strip()))
                if t[0][len('target='):] != self.target:
                    continue
                w.remove(t[0])
            if not w:
                continue
            kw = dict(x.split('=', 1) for x in w[1:] if '=' in x)
            names = [x for x in w[1:] if '=' not in x]
            try:
                if w[0] == 'output':
                    self.output = names[0]
                elif w[0] == 'define':
                    self.defines.update(kw)
                elif w[0] == 'begin':
                    if in_area or section is not None:
                        raise ValueError('an area inside an area or after the pre-loaded section')
                    in_area = True
                    self.areas.append([])
                elif w[0] == 'end':
                    if not in_area:
                        raise ValueError('end without begin')
                    in_area, section = False, None
                elif w[0] == 'section':
                    section = names[0]
                    if section in self.sections:
                        raise ValueError('a second section %s' % section)
                    self.sections[section] = len(self.areas) - 1 if in_area else None
                    if in_area:
                        self.areas[-1].append(section)
                    self.order += [('file', n, section) for n in names[1:]]
                elif w[0] == 'file':
                    if in_area:
                        raise ValueError('a file in an area belongs to a section')
                    self.order += [('file', n, section) for n in names]
                elif w[0] == 'library':
                    self.order.append(('library', tuple(names), section))
                elif w[0] == 'loader':
                    self.order.append(('loader', names[0], section))
                elif w[0] == 'table':
                    self.order.append(('table', TABLE, section))
                elif w[0] == 'wrapper':
                    self.wrapper = names[0]
                    self.wrapper_stack = {k: int(v, 16) for k, v in kw.items()}
                elif w[0] == 'stubs':
                    self.stubs += names
                else:
                    raise ValueError('unknown kind of line')
            except (ValueError, KeyError, IndexError) as e:
                raise SystemExit('%s, line %d: %s (%s)' % (self.path, no, line.strip(), e))
        self.preloaded = next((s for s, a in self.sections.items() if a is None), None)

    def overlays(self):
        """The overlay sections (in areas), in order: their numbers are their places from 1,
        the pre-loaded section's the next."""
        return [s for s, a in self.sections.items() if a is not None]

    def number(self, section):
        return list(self.sections).index(section) + 1


def code_segment(obj):
    for s in obj.segs[1:]:
        if s.cls == 'CODE' and s.length:
            return s
    return None


def module_publics(obj):
    return {n for s in obj.segs[1:] for n in s.publics} | set(obj.absolutes)


def align(at, alignment):
    a = ALIGNMENT.get(alignment, 1)
    return (at + a - 1) // a * a


def paragraph(at):
    return (at + 15) // 16 * 16


class Module:
    """An object and where its segments go. kind: 'lib' (a module of the runtime), 'res' (a
    resident object), 'ovl' (an object of an overlay), 'data' (an object of the pre-loaded
    section: communal variables only). section: the overlay or pre-loaded section, None for
    the root."""
    def __init__(self, name, obj, kind, section=None):
        self.name, self.obj, self.kind, self.section = name, obj, kind, section
        self.overlay = section if kind == 'ovl' else None
        self.code = None          # (segment, CS address)
        self.more_code = []       # its other segments outside DGROUP: [(segment, CS address)] (PLMGR's OVDATA)
        self.data = {}            # segment name -> DS address
        self.relocs = []          # addresses of its code's relocated words, in fixup order
        self.data_relocs = []     # (DS address, segment name) of its data's relocated words


class Generated:
    """An object u5link makes itself (the overlay table): one code segment, no fixups."""
    def __init__(self, name, length, publics):
        self.path, self.module = '(u5link)', name
        seg = omf.Segment(name, 'CODE', length, 3)
        seg.publics = dict(publics)
        self.segs, self.exts, self.comdefs, self.absolutes, self.groups, self.comments = \
            [None, seg], [None], {}, {}, {}, []

    def seg(self, name):
        return next((s for s in self.segs[1:] if s.name == name), None)


class Linker:
    def __init__(self, script, obj_path=None):
        """obj_path(name): the file of object name (default: in u5.OBJ)."""
        self.script = script
        self.target = script.target
        self.obj_path = obj_path or (lambda name: os.path.join(u5.OBJ, name + '.OBJ'))
        self.checksum_value = None
        self.modules = []         # in processing order (the wrapper last)
        self.publics = {}         # name -> (kind 'code'|'data', address, module)
        self.by_convention = {}   # name -> count of references resolved by their name only
        self.unresolved = {}
        self.notes = []
        self.fill = None          # --fill: the folder of the game files
        # the layout (Linker.layout)
        self.seg_start = {}       # a root segment's name -> its address
        self.sections = {}        # section -> {'base', 'size' (its code, to the paragraph), 'memory'}
        self.areas = []           # (base, size)
        self.program = 0          # the end of the program PLINK wrote (where the wrapper goes)
        self.root_end = 0         # the end of the resident image (with the wrapper)
        self.dgroup_para = 0
        self.dgroup_size = 0      # _end: DATA.OVL's DGROUP
        self.dgroup_memory = 0    # with the stack
        self.dgroup_segs = set()
        self.communal_at = {}
        self.table = None

    # -- collecting
    def add(self, m):
        self.modules.append(m)

    def load(self, name, section, kind=None):
        p = self.obj_path(name)
        if not os.path.exists(p):
            self.notes.append('%s: no object' % name)
            return None
        sec_kind = 'res' if section is None else 'data' if section == self.script.preloaded else 'ovl'
        return Module(name, omf.Object(p), kind or sec_kind, section)

    def gather(self):
        """The modules in processing order: the files, the library modules the search pulls
        where the library line is, the loader, the table; the wrapper last. An object that was
        not built is left out (a partial build links; the gate counts the objects)."""
        sc = self.script
        libs_at = None
        for kind, name, section in sc.order:
            if kind in ('file', 'loader'):
                m = self.load(name, section)
                if m:
                    m.loader = kind == 'loader'
                    self.add(m)
            elif kind == 'library':
                libs_at, libs = len(self.modules), name
            elif kind == 'table':
                self.table = self.table_module(section)
                self.add(self.table)
        if libs_at is not None:
            self.modules[libs_at:libs_at] = self.search(libs)
        if sc.wrapper:
            m = self.load(sc.wrapper, None)
            if m:
                m.wrapper = True
                self.add(m)

    def search(self, libs):
        """The library modules the link needs, PLINK's way: the libraries as one (SLIBCE.LIB),
        a name's module the first that defines it; the names no file defines, in the order the
        files first name them, then the names each pulled module adds, breadth first."""
        modules = []
        for lib in libs:
            modules += [(lib, n, o) for n, o in omf.library_modules(os.path.join(u5.MSC51, 'LIB', lib))]
        where = {}
        for i, (lib, n, o) in enumerate(modules):
            for p in sorted(module_publics(o)):
                where.setdefault(p, i)
        have = set(LINKER_DEFINED) | {exported(n) for n in self.script.defines}
        for m in self.modules:
            have |= module_publics(m.obj) | set(m.obj.comdefs)
        queue, seen = [], set()

        def met(names):
            for e in names:
                if e not in have and e not in seen:
                    seen.add(e)
                    queue.append(e)
        for m in self.modules:
            met(m.obj.exts[1:])
        out = []
        for e in queue:                   # grows while it is walked: breadth first
            if e in have or e not in where:
                continue
            lib, n, o = modules[where[e]]
            m = Module(n, o, 'lib')
            m.lib, m.loader = lib, False
            out.append(m)
            have |= module_publics(o)
            met(o.exts[1:])
        return out

    def table_module(self, section):
        """The overlay table and the call stubs, as PLINK lays them out (make_table writes
        them once the layout is known): the stubs' names are public from the start."""
        sc = self.script
        n = len(sc.sections)
        names = sum(len(s) + 5 for s in sc.sections) + len(sc.output) + 1
        stubs_at = 8 + 16 * n + names
        publics = {'STUB_' + exported(f)[:STUB_NAME]: stubs_at + STUB * k for k, f in enumerate(sc.stubs)}
        m = Module(TABLE, Generated(TABLE, stubs_at + STUB * len(sc.stubs), publics), 'res', section)
        m.loader, m.stubs_at = False, stubs_at
        return m

    # -- placement
    def segment_order(self, modules, dgroup):
        """The segments (class, name) of modules in DGROUP (dgroup) or outside it, as PLINK
        orders them: by class, in the order the classes are first met, within a class in the
        order the names are first met; DGROUP as DOSSEG orders it when a module asks for it
        (crt0): BEGDATA first, BSS and STACK last."""
        classes, segs = [], []
        for m in modules:
            for s in m.obj.segs[1:]:
                if s.cls == 'MARK' or (s.name in self.dgroup_segs) != dgroup:
                    continue
                if s.cls not in classes:
                    classes.append(s.cls)
                if (s.cls, s.name) not in segs:
                    segs.append((s.cls, s.name))
        dosseg = dgroup and any(c == DOSSEG for m in modules for c, _ in m.obj.comments)
        rank = {'BEGDATA': 0, 'BSS': 2, 'STACK': 3} if dosseg else {}
        return sorted(segs, key=lambda cs: (rank.get(cs[0], 1), classes.index(cs[0]), segs.index(cs)))

    def place_segment(self, modules, name, at, record):
        """Place every module's part of segment name from at; record(m, segment, address).
        Returns the end."""
        start, end = None, at
        for m in modules:
            s = m.obj.seg(name)
            if not s:
                continue
            if s.combine == COMMON:
                if start is None:
                    start = align(at, s.align)
                record(m, s, start)
                end = max(end, start + s.length)
            else:
                at = align(at, s.align)
                record(m, s, at)
                at += s.length
                end = max(end, at)
        return end

    def place_code(self, modules, at, root=False):
        """The code (the segments outside DGROUP) of modules from at; returns the end."""
        def record(m, s, a):
            if root:
                self.seg_start.setdefault(s.name, a)
            if s is code_segment(m.obj):
                m.code = (s, a)
            elif s.length:
                m.more_code.append((s, a))
        for cls, name in self.segment_order(modules, False):
            at = self.place_segment(modules, name, at, record)
        return at

    def layout(self):
        """Where everything goes, by PLINK's rules (the module docstring)."""
        sc = self.script
        for m in self.modules:
            m.more_code = []
            for g, segs in m.obj.groups.items():
                if g == 'DGROUP':
                    self.dgroup_segs |= set(segs)
        wrapper = [m for m in self.modules if getattr(m, 'wrapper', False)]
        root = [m for m in self.modules if m.section is None and m not in wrapper]
        self.program = paragraph(self.place_code(root, 0, root=True))
        # the overlay areas, from the paragraph where the root ends
        base = self.program
        for area in sc.areas:
            size = 0
            for sec in area:
                end = self.place_code([m for m in self.modules if m.section == sec], base)
                n = paragraph(end - base)
                self.sections[sec] = {'base': base, 'size': n, 'memory': n}
                size = max(size, n)
            self.areas.append((base, size))
            base += size
        # DGROUP, from the paragraph after the last area
        self.dgroup_para = paragraph(base) >> 4
        self.place_dgroup()
        if sc.preloaded:
            self.sections[sc.preloaded] = {'base': self.dgroup_para * 16, 'size': self.dgroup_size,
                                           'memory': paragraph(self.dgroup_memory)}
        # Origin's wrapper, appended to the program
        at = self.program
        for m in wrapper:
            at = self.place_code([m], at, root=True)
        self.root_end = at

    def place_dgroup(self):
        bss = []

        def record(m, s, a):
            m.data[s.name] = a
            if s.cls == 'BSS' and not bss:
                bss.append(a)                     # _edata: where the first BSS segment starts
        at, stack = 0, False
        for cls, name in self.segment_order(self.modules, True):
            if cls == 'STACK' and not stack:
                at, stack = self.allocate_communals(at), True
            at = self.place_segment(self.modules, name, at, record)
        if not stack:
            at = self.allocate_communals(at)
        self.publics_defined = {'_edata': bss[0] if bss else at}
        self.dgroup_memory = at
        if not self.dgroup_size:
            self.dgroup_size = at

    def allocate_communals(self, at):
        """Uninitialised variables (MSC's COMDEF records: a name and a size, in every object
        that declares the variable without extern) that no object defines otherwise get their
        place here, in the segment c_common after the last segment of class BSS (the modules'
        _BSS, their static variables, then the runtime's empty XOB/XO/XOE), by PLINK86plus's
        rule: in the order of the COMDEF records, object by object in processing order, a name
        already allocated skipped; one after the other, a variable of two bytes or more on an
        even address. (MSC writes an object's COMDEF records in the order of its symbol table,
        research/RULES.md, section 2: ultima.c declares the shared ones, each overlay module its
        own.) _end is the end, word aligned."""
        defined = set()
        for m in self.modules:
            defined |= module_publics(m.obj)
        sizes, order = {}, []
        for m in self.modules:
            for n, size in m.obj.comdefs.items():
                if n not in defined:
                    if sizes.get(n, size) != size:
                        self.notes.append('%s: communal %s has %d bytes here, %d elsewhere' % (m.name, n, size, sizes[n]))
                    if n not in sizes:
                        order.append(n)
                    sizes[n] = max(size, sizes.get(n, 0))
        for n in order:
            if sizes[n] >= 2:
                at += at & 1
            self.communal_at[n] = at
            a = u5.name_address(n)
            if a and a[1] != at:
                self.notes.append('communal %s is allocated at DS:%04x' % (n, at))
            at += sizes[n]
        at += at & 1
        self.dgroup_size = at
        return at

    def check_layout(self):
        """What the other tools take from tools/u5.py (the names' addresses, the overlay
        numbers, DGROUP's paragraph) must be what the link computes."""
        for sec in self.script.overlays():
            hdr = self.sections[sec].get('header', 0)
            if self.sections[sec]['base'] - hdr != u5.OVERLAY_BASE.get(sec):
                self.notes.append('%s starts at CS:%04x, u5.OVERLAY_BASE says %04x' % (
                    sec, self.sections[sec]['base'] - hdr, u5.OVERLAY_BASE.get(sec, 0)))
            if hdr != u5.OVERLAY_HEADER.get(sec, 0):
                self.notes.append('%s.OVL has a header of %d bytes, u5.OVERLAY_HEADER says %d' % (
                    sec, hdr, u5.OVERLAY_HEADER.get(sec, 0)))
        for sec in self.script.sections:
            if u5.OVERLAY_NUMBER.get(sec) != self.script.number(sec):
                self.notes.append('%s is overlay %d, u5.OVERLAY_NUMBER says %s' % (
                    sec, self.script.number(sec), u5.OVERLAY_NUMBER.get(sec)))
        if self.dgroup_para != u5.DGROUP_SEG:
            self.notes.append('DGROUP is at paragraph %04x, u5.DGROUP_SEG says %04x' % (self.dgroup_para, u5.DGROUP_SEG))

    def place(self):
        self.layout()
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
        # symbols the linker defines (the end of the initialised data and of DGROUP)
        self.publics.setdefault('_edata', ('data', self.publics_defined['_edata'], None))
        self.publics.setdefault('_end', ('data', self.dgroup_size, None))
        for n, at in self.communal_at.items():
            self.publics[n] = ('data', at, None)
        # DEFINE: a second name for a place another name defines
        for name, other in self.script.defines.items():
            p = self.publics.get(exported(other))
            if not p:
                self.notes.append('define %s=%s: %s is defined nowhere' % (name, other, other))
            elif exported(name) not in self.publics:
                self.publics[exported(name)] = p
        self.check_stubs()

    def stub_targets(self):
        """The functions that need a call stub, by PLINK's rule: those of an overlay that a
        module of another section names (or names by its stub, STUB_x), as 26-character keys."""
        need = set()
        for m in self.modules:
            for e in m.obj.exts[1:]:
                if e.startswith('STUB_'):
                    need.add(e[5:5 + STUB_NAME])
                    continue
                p = self.publics.get(e) or self.publics.get(e[:31])
                if p and p[0] == 'code' and p[2] and p[2].kind == 'ovl' and p[2].overlay != m.section:
                    need.add(e[:STUB_NAME])
        return need

    def check_stubs(self):
        need = self.stub_targets()
        have = [exported(f)[:STUB_NAME] for f in self.script.stubs]
        for k in sorted(need - set(have)):
            self.notes.append('a stub for %s is needed, but the command file\'s stubs do not list it' % k)
        for k in sorted(set(have) - need):
            self.notes.append('the command file lists a stub for %s, which no other section calls' % k)
        if len(set(have)) != len(have):
            self.notes.append('the command file lists a stub twice')

    def stub_sections(self):
        """For each stub of the command file, the section PLINK would make it in: PLINK makes
        a stub for a symbol in the order the symbols enter its symbol table (the DEFINEs first,
        then every file in processing order, its external names, then its public names;
        measured on PLINK's own link of u5d's objects, 164 of 164). With u5d's objects that
        says in which section's turn each stub comes, not the order inside one: that is the
        original objects' record order, and their names (docs/MATCHING.md)."""
        first = {}
        for name, other in self.script.defines.items():
            for n in (name, other):
                first.setdefault(exported(n)[:STUB_NAME], '(define)')
        for m in self.modules:
            sec = m.section or 'root'
            for e in m.obj.exts[1:]:
                first.setdefault(e[:STUB_NAME], sec if m.kind != 'res' else m.name)
            for s in m.obj.segs[1:]:
                for p in s.publics:
                    first.setdefault(p[:STUB_NAME], sec if m.kind != 'res' else m.name)
        return [(f, first.get(exported(f)[:STUB_NAME])) for f in self.script.stubs]

    # -- resolution
    def resolve(self, name, src):
        """(kind, address) for a name referenced from module src."""
        # names count in their first 31 characters: MSC 5.1 and the assembly modules cut a
        # longer name there, MSC 5.0 writes one character more
        p = self.publics.get(name) or self.publics.get(name[:31])
        if p:
            kind, addr, owner = p
            # into another overlay's function: through its stub
            if kind == 'code' and owner and owner.kind == 'ovl' and owner.overlay != src.section:
                st = self.publics.get('STUB_' + name[:STUB_NAME])
                if st:
                    return st[0], st[1]
                note = '%s is called from %s but has no stub (the command file\'s stubs)' % (name, src.name)
                if note not in self.notes:
                    self.notes.append(note)
            return kind, addr
        a = u5.name_address(name)
        if a:
            self.by_convention[name] = self.by_convention.get(name, 0) + 1
            if a[0] == 'code':
                owner = u5.module_owner(name)
                if owner in u5.OVERLAY_BASE and owner != src.section:
                    st = self.publics.get('STUB_' + name[:STUB_NAME])
                    if st:
                        return st[0], st[1]
            return a
        self.unresolved[name] = self.unresolved.get(name, 0) + 1
        return None

    def frame(self, target, m):
        """The paragraph a segment reference stands for: DGROUP's, 0 for _TEXT (overlays are
        loaded into it too) and for a function, a root segment's own (PLMGR_TEXT, OVDATA,
        PLTAB, PLSTART_TEXT), or the one an empty MARK segment SEG_xxxx names (asmgen.py's
        drafts: a paragraph that is no segment's)."""
        kind, name = target
        if (kind == 'grp' and name == 'DGROUP') or name == 'DGROUP' or name in self.dgroup_segs:
            return self.dgroup_para
        if name == '_TEXT' or kind == 'ext':
            return 0
        if name in self.seg_start:
            return self.seg_start[name] >> 4
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
        part_of = {n: g for g in u5.object_parts(self.target) for n in g}
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
        address: the code segment's own paragraph (0 for _TEXT, 072e and 0778 for the loader's
        and the table's), or, in DGROUP, the paragraph the combined data segment starts in
        (_DATA starts at DS:0042, after the runtime's NULL segment: paragraph 0f68)."""
        if data_seg:
            start = min(x.data[data_seg] for x in self.modules if data_seg in x.data
                        and x.obj.seg(data_seg) and x.obj.seg(data_seg).length)
            return (self.dgroup_para * 16 + start) >> 4
        name = m.code[0].name
        return 0 if name == '_TEXT' else self.seg_start.get(name, 0) >> 4

    def header_entries(self, entries):
        """A relocation header as PLINK86 writes it before an overlay's image (none for an
        overlay without relocated words): a dword (offset, segment) per relocated word, a zero
        dword, then zeros to the paragraph."""
        if not entries:
            return b''
        hdr = b''.join(struct.pack('<HH', o, s) for o, s in entries) + bytes(4)
        return hdr + bytes(-len(hdr) % 16)

    def module_of(self, name):
        return next((m for m in self.modules if m.name == name and m.code), None)

    def program_end(self, image):
        """Where the program PLINK86 linked ends: at Origin's copy-protection wrapper (appended
        to it), else at the end of the image."""
        return self.program if self.script.wrapper else len(image)

    def plink_header(self, size, checksum=0):
        """ULTIMA.EXE's MZ header as PLINK86 wrote it, for a program of size bytes (the image
        up to PLSTART). Rules: the file size, the relocation count and table (after the
        0x1e-byte header, in relocation_order), the header padded to 512 bytes, the entry point
        at PLINK86's start-up (PLMGR's PLINK_START, 072e:0322), the stack after DGROUP (crt0's
        STACK segment: SS its paragraph, 1b38, SP its length, 0800), and the minimum allocation
        from the end of the program to the end of that stack; PLINK86plus's own fields: the
        table at 0x1e, the word 1 at 0x1c."""
        relocs = []
        for loc in self.relocation_order([m for m in self.modules if m.kind != 'ovl']):
            m = next(x for x in self.modules if x.kind != 'ovl' and loc in x.relocs)
            f = self.segment_frame(m)
            relocs.append((loc - f * 16, f))
        table = b''.join(struct.pack('<HH', o, s) for o, s in relocs)
        hsize = (PLINK_RELOC_TABLE + len(table) + 511) // 512 * 512
        total = hsize + size
        entry = self.publics.get('PLINK_START')
        if entry and entry[2] and entry[2].code:
            cs = self.segment_frame(entry[2])
            ip = entry[1] - cs * 16
        else:
            cs = ip = 0
            note = 'no PLINK_START (src/asm/PLMGR.ASM): the EXE checksum\'s entry point is not known'
            if note not in self.notes:
                self.notes.append(note)
        # the stack: crt0's STACK segment, from the paragraph after DGROUP's end (_end)
        ss = self.dgroup_para + (self.dgroup_size + 15) // 16
        sp = self.stack_size()
        minalloc = ss + sp // 16 - size // 16
        hdr = struct.pack('<2s13H', b'MZ', total % 512, (total + 511) // 512, len(relocs), hsize // 16,
                          minalloc, 0xffff, ss, sp, checksum, ip, cs, PLINK_RELOC_TABLE, 0)
        hdr += struct.pack('<H', PLINK_WORD_1C)
        return hdr + table + bytes(hsize - len(hdr) - len(table))

    def checksum(self, image):
        """PLINK86's checksum (the MZ checksum: the one's complement of the sum of the file's
        words, the field itself zero) of the program it linked: plink_header and the image up
        to PLSTART."""
        size = self.program_end(image)
        f = self.plink_header(size) + bytes(image[:size])
        f += bytes(len(f) & 1)
        return ~sum(struct.unpack('<%dH' % (len(f) // 2), f)) & 0xffff

    def exe_header(self, image, checksum):
        """ULTIMA.EXE's MZ header: PLINK86's (plink_header, with the checksum given) as Origin's
        copy-protection wrapper left it when it was appended to the program: the file size
        counts the wrapper, the entry point is the wrapper's (PLSTART's segment, 081d:0000) and
        so is the stack (SS stackgap paragraphs past the end of the image, SP sp: the command
        file's wrapper line, a fact). The rest is PLINK86's: the minimum allocation, for one,
        still ends at the end of the stack counted from the end of the program without the
        wrapper."""
        x = self.script.wrapper_stack
        hdr = bytearray(self.plink_header(self.program_end(image), checksum))
        size = len(hdr) + len(image)
        start = self.module_of(self.script.wrapper) if self.script.wrapper else None
        struct.pack_into('<HH', hdr, 2, size % 512, (size + 511) // 512)
        if start:
            struct.pack_into('<HH', hdr, 0x0e, len(image) // 16 + x.get('stackgap', 0), x.get('sp', 0))
            struct.pack_into('<HH', hdr, 0x14, 0, start.code[1] >> 4)
        return bytes(hdr)

    def stack_size(self):
        for m in self.modules:
            s = m.obj.seg('STACK') if m.kind == 'lib' else None
            if s and s.length:
                return s.length
        return 0

    def data_entries(self):
        """DATA.OVL's relocation entries: the far pointers in DGROUP (the words holding a
        segment)."""
        entries = []
        mods = [m for m in self.modules if m.kind != 'ovl']
        for loc, sname in self.relocation_order(mods, 'data_relocs'):
            f = self.segment_frame(None, sname)
            entries.append((self.dgroup_para * 16 + loc - f * 16, f))
        return entries

    def data_header(self):
        return self.header_entries(self.data_entries())

    def make_table(self, overlay_relocs):
        """The overlay table and the call stubs (PLINK's $OVTB$; the loader, OVERLAY.LIB's
        OVLYV, reads it):
          a header: the loader's paragraph (its segment OVCODE, PLMGR_TEXT here; a number, not
            relocated), 0, the paragraph where memory ends (the pre-loaded section's end,
            relocated);
          a record of 16 bytes per section, in order (an overlay's number is its place, from
            1): its status word (0 for an overlay, STATUS_PRELOADED), the number of its
            relocated words (its header's entries), the paragraphs where it starts and ends in
            memory (relocated), the offset of its file name in the table, 0 and 0 (a dword:
            where it starts in its file), its length in the file in paragraphs (without the
            header); then STATUS_END;
          the file names: the program's (the output line's), then each section's NAME.OVL;
          a stub per function called from another section, in the command file's order: call
            far $OVLYV (the loader; its segment relocated), the overlay's number, jmp far the
            function (its segment relocated).
        Returns the bytes and the relocated words' offsets in it."""
        sc = self.script
        m = self.table
        out, relocs = bytearray(), []

        def word(v, relocated=False):
            if relocated:
                relocs.append(len(out))
            out.extend(struct.pack('<H', v & 0xffff))
        loader = next((x for x in self.modules if getattr(x, 'loader', False) and x.code), None)
        memory_end = max(s['base'] + s['memory'] for s in self.sections.values()) >> 4
        word(self.seg_start.get(loader.code[0].name, 0) >> 4 if loader else 0)
        word(0)
        word(memory_end, True)
        name_at = 8 + 16 * len(sc.sections) + len(sc.output) + 1
        secs = list(sc.sections)
        for sec in secs:
            s = self.sections[sec]
            count = len(self.data_entries()) if sec == sc.preloaded else len(overlay_relocs.get(sec, ()))
            word(STATUS_PRELOADED if sec == sc.preloaded else 0)
            word(count)
            word(s['base'] >> 4, True)
            word((s['base'] + s['memory']) >> 4, True)
            word(name_at)
            word(0)
            word(0)
            word(paragraph(s['size']) >> 4)
            name_at += len(sec) + 5
        word(STATUS_END)
        out += sc.output.encode('ascii') + b'\0'
        for sec in secs:
            out += sec.encode('ascii') + b'.OVL\0'
        assert len(out) == m.stubs_at, (len(out), m.stubs_at)
        entry = self.publics.get('$OVLYV')
        if not entry or not entry[2] or not entry[2].code:
            self.notes.append('no $OVLYV (src/asm/PLMGR.ASM): the stubs call nowhere')
            entry = ('code', 0, loader)
        lseg = self.segment_frame(entry[2]) if entry[2] and entry[2].code else 0
        for f in sc.stubs:
            p = self.publics.get(exported(f))
            if not p or p[0] != 'code' or not p[2] or p[2].kind != 'ovl':
                self.notes.append('stub %s: no overlay function of that name' % f)
                p = ('code', 0, None)
            out.append(0x9a)
            word(entry[1] - lseg * 16)
            word(lseg, True)
            word(sc.number(p[2].overlay) if p[2] else 0)
            out.append(0xea)
            word(p[1])
            word(0, True)
        return bytes(out), relocs

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
                segv = self.dgroup_para if r[0] == 'data' else 0
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
            # function (D_5394_diskSwapFn = ULTIMA_2322_DiskSwapMessage); it is placed as an offset
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
        """(resident image, {overlay: file bytes}, DGROUP)"""
        self.place()
        root, dgroup = bytearray(self.root_end), bytearray(self.dgroup_size)
        code = {sec: bytearray(self.sections[sec]['size']) for sec in self.script.overlays()}
        if self.fill:                      # --fill: start from the original's bytes
            orig = u5.exe_image(self.fill)
            root[:len(orig)] = orig[:len(root)]
            orig = u5.ds_image(self.fill)
            dgroup[:len(orig)] = orig[:len(dgroup)]
        for m in self.modules:
            if m.code and m is not self.table:
                for seg, base in [m.code] + m.more_code:
                    if m.kind == 'ovl':
                        self.apply(m, seg, base, code[m.overlay], self.sections[m.overlay]['base'])
                    else:
                        self.apply(m, seg, base, root, 0)
            for sname, dsbase in m.data.items():
                s = m.obj.seg(sname)
                if s and s.length and sname not in SKIP_SEGMENTS:
                    self.apply(m, s, dsbase, dgroup, 0, data_seg=True)
        # an overlay's relocation list is its header: a dword (CS offset, segment 0) per
        # relocated word, in relocation_order, then zeros
        relocs, overlays = {}, {}
        for sec in self.script.overlays():
            relocs[sec] = self.relocation_order([m for m in self.modules if m.kind == 'ovl' and m.overlay == sec])
            hdr = self.header_entries([(a, 0) for a in relocs[sec]])
            self.sections[sec]['header'] = len(hdr)
            overlays[sec] = hdr + bytes(code[sec])
        if self.table:
            data, words = self.make_table(relocs)
            base = self.table.code[1]
            root[base:base + len(data)] = data
            self.table.relocs = [base + w for w in words]
        self.check_layout()
        return root, overlays, dgroup


def compare(name, ours, orig):
    if ours == orig:
        return '%-13s identical (%d bytes)' % (name, len(orig))
    diff = [i for i in range(min(len(ours), len(orig))) if ours[i] != orig[i]]
    return '%-13s %d of %d bytes differ%s%s' % (name, len(diff) + abs(len(ours) - len(orig)), len(orig),
                                               ', first at %04x' % diff[0] if diff else '',
                                               '' if len(ours) == len(orig) else ', length %d' % len(ours))


def patch_base_checksum(lk, script=None):
    """The EXE checksum of a target that is another's link patched afterwards (u5.PATCH_OF:
    gog, the 1988 link patched in 1992): that link's, linked here as the other target from
    its objects of the modules that differ by target (u5.patch_base_obj(): build/obj-1988)
    and this target's objects of the others. Its notes join lk's."""
    base, folder = u5.PATCH_OF[lk.target], u5.patch_base_obj()
    differ = u5.target_dependent_objects(base)

    def obj_path(name):
        return os.path.join(folder if name in differ else u5.OBJ, name + '.OBJ')
    blk = Linker(Script(script, base), obj_path)
    blk.gather()
    root = blk.link()[0]
    what = 'EXE checksum (the %s link, with %s from %s)' % (
        base, ' '.join(sorted(differ)), os.path.relpath(folder, u5.ROOT))
    lk.notes += ['%s: %s' % (what, n) for n in blk.notes]
    if blk.by_convention or blk.unresolved:
        lk.notes.append('%s: %d names resolved by their address, %d unresolved' % (
            what, len(blk.by_convention), len(blk.unresolved)))
    return blk.checksum(root)


def link(out=OUT, fill=None, script=None):
    """Link the target's objects (u5.TARGET, u5.OBJ) as the command file says, and write the
    25 files to out. Returns (linker, {file name: our bytes}). fill: the folder of the game
    files, to start from the original's bytes; nothing else reads a game file."""
    lk = Linker(Script(script))
    lk.fill = fill
    lk.gather()
    root, overlays, dgroup = lk.link()
    lk.checksum_value = patch_base_checksum(lk, script) if lk.target in u5.PATCH_OF else lk.checksum(root)
    files = {ovl + '.OVL': bytes(buf) for ovl, buf in overlays.items()}
    files[lk.script.output] = lk.exe_header(root, lk.checksum_value) + bytes(root)
    if lk.script.preloaded:
        files[lk.script.preloaded + '.OVL'] = lk.data_header() + bytes(dgroup)
    os.makedirs(out, exist_ok=True)
    for name, data in files.items():
        open(os.path.join(out, name), 'wb').write(data)
    return lk, dict(sorted(files.items()))


def placement_map(lk):
    """The computed placement as text (--map)."""
    L = ['root     0000-%04x (the program PLINK wrote, to the paragraph), image to %04x' % (lk.program, lk.root_end)]
    for k, (base, size) in enumerate(lk.areas):
        L.append('area %d   %04x, %04x bytes: %s' % (k + 1, base, size, ' '.join(
            '%s(%04x%s)' % (s, lk.sections[s]['size'], ', header %x' % lk.sections[s]['header']
                            if lk.sections[s].get('header') else '') for s in lk.script.areas[k])))
    L.append('DGROUP   paragraph %04x, %04x bytes (_end), %04x with the stack' % (
        lk.dgroup_para, lk.dgroup_size, lk.dgroup_memory))
    for m in lk.modules:
        code = ''
        if m.code:
            # an overlay's object at its offset in the .OVL file, as the functions' names have it
            sec = lk.sections.get(m.overlay, {})
            code = '%s:%04x' % (m.overlay, m.code[1] - sec['base'] + sec.get('header', 0)) if m.kind == 'ovl' \
                else '%04x' % m.code[1]
        data = ' '.join('%s=%04x' % (s, a) for s, a in sorted(m.data.items(), key=lambda kv: (kv[1], kv[0]))
                        if m.obj.seg(s).length)
        L.append('%-4s %-16s %-13s %s' % (m.kind, m.name, code, data))
    return '\n'.join(L)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--out', default=OUT)
    ap.add_argument('--script', help='the link command file (default: src/u5d.lnk)')
    ap.add_argument('--fill', action='store_true',
                    help='start from the original bytes where no object covers them (shows how close the '
                         'parts are; needs the game files)')
    ap.add_argument('--map', action='store_true', help='print the computed placement')
    ap.add_argument('-v', '--verbose', action='store_true')
    u5.add_target_argument(ap)
    a = ap.parse_args()
    u5.set_target(a.target)
    gd = u5.find_gamedata() if a.fill else u5.find_gamedata(required=False)
    lk, files = link(a.out, gd if a.fill else None, a.script)
    if a.map:
        print(placement_map(lk))
    bad = 0
    table = u5.release_sha1()
    print('%s, compared with %s' % (u5.TARGET_TITLE[u5.TARGET], 'the game files in %s' % gd if gd else
                                    'the SHA-1s of its files (no game files found)'))
    print('EXE checksum %04x (%s)' % (lk.checksum_value, 'the %s link\'s' % u5.PATCH_OF[u5.TARGET]
                                       if u5.TARGET in u5.PATCH_OF else 'this link\'s'))
    for name in sorted(set(files) | set(table)):
        ours = files.get(name)
        if ours is None:
            print('%-13s not built' % name)
            bad += 1
            continue
        if not gd:
            same = hashlib.sha1(ours).hexdigest() == table.get(name)
            print('%-13s %s (%d bytes)' % (name, 'identical' if same else 'DIFFERENT', len(ours)))
            bad += not same
            continue
        orig = u5.read_file(gd, name)
        print(compare(name, ours, orig))
        bad += ours != orig
        if name == lk.script.output and ours != orig:
            hdr = len(ours) - lk.root_end
            print('  ' + compare('header', ours[:hdr], orig[:hdr]))
            print('  ' + compare('image', ours[hdr:], orig[hdr:]))
        if name == '%s.OVL' % lk.script.preloaded and ours != orig:
            hdr = len(lk.data_header())
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
