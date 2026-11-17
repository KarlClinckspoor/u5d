#!/usr/bin/env python3
"""Write an assembly source for a range of the original code, and make it assemble to it.

    python tools/asmgen.py NAME START END [--entry ADDR ...] [--keep]
    python tools/asmgen.py NAME START END --ovl OVERLAY

For the resident image (CS offsets START..END of ULTIMA.EXE), writes src/asm/NAME.ASM:
MASM 4.0 source in Microsoft C's small-model segments, which tools/asmbuild.py assembles to
the original bytes. With --ovl, START..END are offsets in OVERLAY.OVL (as u5d's names have
them: FONT_0000 is offset 0 of FONT.OVL), and the module is an assembly part of that overlay
(first line "OVERLAY.OVL START-END"; tools/u5link.py places it by its functions' names). The
code is decoded at the overlay's CS addresses, calls into the resident code name their
targets, and labels inside it carry file offsets. The source is drafted from the bytes, then
checked:

  1. instructions are found from the entry points (the u5d functions in the range, and any
     --entry) by following jumps and calls; whatever is never reached is written as data;
  2. jumps and calls inside the range get labels; calls and jumps out of it name their
     target (u5d's function names, as the C compiler exports them: upper case, 31 chars);
     each u5d function in the range is PUBLIC under that name;
  3. direct memory operands that point at a known DGROUP variable are written by name;
  4. a word the EXE relocates (a segment) is written as a segment reference, so the link
     can rebuild the relocation table;
  5. the source is assembled with MASM 4.0 in DOSBox-X and compared with the original, fixups
     masked and checked; every instruction MASM encodes differently is rewritten as bytes
     (db ...; with the instruction kept as a comment), and the loop repeats until the
     object's code is the original's.

This is the method of u1-decompiled's genasm.py, for MSC's segment conventions. What comes
out is a faithful, readable starting point: names and comments can be improved by hand, as
long as tools/asmbuild.py still reports the module identical.
"""
import argparse, glob, os, re, shutil, struct, subprocess, sys, tempfile

import iced_x86 as I

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402
import rootmap  # noqa: E402

ASM_DIR = os.path.join(u5.SRC, 'asm')
MASM_DIR = os.path.join(u5.BUILD, 'masm')


# PLINK86's far segments in ULTIMA.EXE (paragraph -> name, class, alignment), and what a
# relocated word holding each segment value is written as
SEGMENTS = {0x72e: ('PLMGR_TEXT', 'CODE', 'PARA'), 0x778: ('PLTAB', 'CODE', 'PARA'),
            0x81d: ('PLSTART_TEXT', 'CODE', 'PARA')}
SEGVALUES = {0: '_TEXT', u5.DGROUP_SEG: 'DGROUP', 0x72e: 'PLMGR_TEXT', 0x778: 'PLTAB', 0x81d: 'PLSTART_TEXT'}


def segname(val):
    """What a relocated word holding paragraph val is written as. Paragraphs that are no
    module's segment (PLINK86's overlay boundaries, the end of memory) are named SEG_xxxx:
    empty segments that tools/u5link.py places at that paragraph."""
    return SEGVALUES.get(val, 'SEG_%04X' % val)


def exported(name):
    """The name as MSC 5.1 (/Gc) puts it in an object: upper case, at most 31 characters."""
    return name.upper()[:31]


def relocations(gamedata):
    d = u5.read_file(gamedata, 'ULTIMA.EXE')
    n, off = struct.unpack_from('<H', d, 6)[0], struct.unpack_from('<H', d, 0x18)[0]
    out = set()
    for i in range(n):
        o, s = struct.unpack_from('<HH', d, off + 4 * i)
        out.add(s * 16 + o)
    return out


def overlay_functions(ovl):
    """{file offset: exported name} for the functions of overlay ovl: the u5d definitions in
    its C files, and the PUBLIC labels of its assembly parts."""
    out = {}
    for f, (kind, o) in u5.MODULES.items():
        if kind != 'ovl' or o != ovl:
            continue
        s = open(os.path.join(u5.SRC, f), encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'^(?:static\s+)?[\w\s\*]*?\b(%s_([0-9a-fA-F]{4})\w*)\s*\([^;{)]*\)\s*(?://[^\n]*)?\s*\{' % ovl,
                             s, re.M):
            out.setdefault(int(m.group(2), 16), exported(m.group(1)))
    for p in glob.glob(os.path.join(ASM_DIR, '*.ASM')):
        s = open(p, encoding='latin-1').read()
        if not re.search(r'\b%s\.OVL [0-9a-f]{4}-[0-9a-f]{4}' % ovl, s.split('\n', 1)[0]):
            continue
        for m in re.finditer(r'^\tPUBLIC\t(%s_([0-9A-Fa-f]{4})\w*)' % ovl, s, re.M):
            out.setdefault(int(m.group(2), 16), m.group(1))
    return out


def data_symbols():
    """{DS address: (name, size or None)} for the D_xxxx variables (vars.h) and S_ arrays."""
    out = {}
    p = os.path.join(u5.SRC, 'vars.h')
    for m in re.finditer(r'^\s*STUB\s+([\w\s\*]+?)\b(D_([0-9a-f]{4})\w*)\s*((?:\[[^\]]*\])*)\s*;', open(p, encoding='utf-8').read(), re.M):
        typ = m.group(1).strip()
        size = 1 if typ.split()[-1] in ('u8', 's8', 'byte', 'char', 'bool') and '*' not in typ else 2
        out[int(m.group(3), 16)] = (exported(m.group(2)), size if not m.group(4) else None, typ)
    return out


class Gen:
    def __init__(self, name, start, end, entries=(), gamedata=None, seg=0, ovl=None):
        # an overlay part: the overlay's bytes at its area base in a copy of the resident
        # image, so that everything below works in CS addresses; ovlbase turns them back
        # into file offsets for the names
        self.ovl = ovl
        self.ovlbase = u5.OVERLAY_BASE[ovl] if ovl else 0
        self.name, self.start, self.end = name, start + self.ovlbase, end + self.ovlbase
        start, end = self.start, self.end
        # code of a far segment (PLINK86's): offsets are relative to the segment's base
        self.seg = seg
        self.segbase = seg * 16
        self.segname = SEGMENTS.get(seg, ('_TEXT', 'CODE', 'BYTE'))
        self.gamedata = u5.find_gamedata(gamedata)
        self.img = u5.exe_image(self.gamedata)
        self.relocs = relocations(self.gamedata)
        tags = rootmap.function_tags()
        self.funcs = {a: exported(n) for a, (n, f, t) in tags.items()}
        if ovl:
            code = u5.read_file(self.gamedata, ovl + '.OVL')
            ovlend = self.ovlbase + len(code)
            img = bytearray(self.img) + bytes(max(0, ovlend - len(self.img)))
            img[self.ovlbase:ovlend] = code
            self.img = bytes(img)
            # an overlay's relocations are its header's (INTRO.OVL's only); what the resident
            # image has in the overlay's area (PLINK86's start-up code) is not this code
            self.relocs = set(u5.overlay_relocations(self.gamedata, ovl))
            self.funcs = {a: n for a, n in self.funcs.items() if not self.ovlbase <= a < ovlend}
            self.funcs.update({self.ovlbase + o: n for o, n in overlay_functions(ovl).items()})
        # runtime routines: the library's own (case-sensitive) public names
        self.funcs.update({a: '_' + n.split()[0] for a, n in rootmap.crt_names().items()
                           if a not in self.funcs and re.match(r'^\w+$', n.split()[0])})
        # overlay call stubs: STUB_<function> (src/asm/PLTAB.ASM)
        try:
            import plinkgen
            names = plinkgen.overlay_function_names()
            for s, (num, tgt) in u5.overlay_stubs(self.img).items():
                if (num, tgt) in names:
                    self.funcs.setdefault(s, 'STUB_' + names[(num, tgt)][:26])
        except Exception:                                    # noqa: BLE001
            pass
        # labels other assembly modules jump to must be PUBLIC here
        self.wanted = set()
        for p in glob.glob(os.path.join(ASM_DIR, '*.ASM')) if not ovl else ():
            if os.path.basename(p).upper() == name.upper() + '.ASM':
                continue
            for m in re.finditer(r'^\tEXTRN\tL_([0-9a-f]{4}):NEAR', open(p, encoding='latin-1').read(), re.M):
                a = int(m.group(1), 16)
                if start <= a < end:
                    self.wanted.add(a)
        # the address ranges of all assembly modules (a branch to an address no module
        # defines, such as PLINK's jump into an overlay area, stays as bytes)
        self.asm_ranges = []
        for p in glob.glob(os.path.join(ASM_DIR, '*.ASM')):
            line = open(p, encoding='latin-1').readline()
            m = re.search(r'CS:([0-9a-f]{4})-([0-9a-f]{4})', line)
            if m:
                self.asm_ranges.append((int(m.group(1), 16), int(m.group(2), 16)))
            m = re.search(r'\b([A-Z0-9]+)\.OVL ([0-9a-f]{4})-([0-9a-f]{4})', line)
            if m and ovl and m.group(1) == ovl:
                self.asm_ranges.append((self.ovlbase + int(m.group(2), 16), self.ovlbase + int(m.group(3), 16)))
        self.data = data_symbols() if not seg else {}
        self.entries = sorted(set([a for a in self.funcs if start <= a < end] + list(entries)))
        self.force_db = set()        # instruction addresses to write as bytes

    # -- analysis
    def decode_at(self, a):
        """The instruction at image address a; INVALID unless it is 8086 code (anything
        newer, or with 32-bit prefixes, is data the original happens to keep there)."""
        d = I.Decoder(16, self.img[a:self.end], ip=a - self.segbase)
        x = next(iter(d))
        if x.code != I.Code.INVALID and (any(f not in (I.CpuidFeature.INTEL8086, I.CpuidFeature.INTEL8086_ONLY)
                                             for f in x.cpuid_features()) or x.op_count and any(
                x.op_kind(k) == I.OpKind.MEMORY and x.memory_base in (I.Register.ESI, I.Register.EDI, I.Register.EBX,
                                                                      I.Register.EBP, I.Register.EAX, I.Register.ESP)
                for k in range(x.op_count))):
            return I.Instruction()             # code INVALID
        return x

    def target(self, x):
        """A near branch's target as an image address."""
        return (self.segbase + x.near_branch16) & 0xfffff

    def walk(self):
        """Instruction starts reached from the entries, plus code nothing calls directly
        (interrupt handlers installed through a vector): an unreached stretch that decodes
        as valid instructions up to a return or jump is taken as code too."""
        seen = self._walk(self.entries)
        while True:
            covered = set()
            for a, x in seen.items():
                covered.update(range(a, a + x.len))
            extra = []
            a = self.start
            while a < self.end:
                if a in covered:
                    a += 1
                    continue
                b = a
                ok = False
                while b < self.end and b not in covered:
                    x = self.decode_at(b)
                    if x.code == I.Code.INVALID or b + x.len > self.end:
                        break
                    b += x.len
                    if x.flow_control in (I.FlowControl.RETURN, I.FlowControl.UNCONDITIONAL_BRANCH,
                                          I.FlowControl.INDIRECT_BRANCH) or x.mnemonic == I.Mnemonic.IRET:
                        ok = True
                        break
                # padding (zeros) is not code
                if ok and any(self.img[k] for k in range(a, b)) and b - a > 2:
                    extra.append(a)
                    break
                a += 1
            if not extra:
                return seen
            self.entries = sorted(set(self.entries) | set(extra))
            seen = self._walk(self.entries)

    def _walk(self, entries):
        seen, todo = {}, list(entries)
        while todo:
            a = todo.pop()
            while self.start <= a < self.end and a not in seen:
                ins = self.decode_at(a)
                if ins.code == I.Code.INVALID or a + ins.len > self.end:
                    break
                seen[a] = ins
                fc = ins.flow_control
                if ins.op0_kind == I.OpKind.NEAR_BRANCH16 and fc in (
                        I.FlowControl.CALL, I.FlowControl.UNCONDITIONAL_BRANCH, I.FlowControl.CONDITIONAL_BRANCH):
                    t = self.target(ins)
                    if self.start <= t < self.end:
                        todo.append(t)
                if fc in (I.FlowControl.RETURN, I.FlowControl.UNCONDITIONAL_BRANCH, I.FlowControl.INDIRECT_BRANCH,
                          I.FlowControl.INTERRUPT if ins.mnemonic == I.Mnemonic.IRET else -1):
                    break
                a += ins.len
        return seen

    # -- writing
    def label(self, a):
        if a in self.funcs:
            return self.funcs[a]
        if self.ovl and a >= self.ovlbase:
            return 'L_%04x' % (a - self.ovlbase)          # an overlay's offset in its file
        return 'L_%04x' % a

    def emit(self):
        ins = self.walk()
        targets = set()
        externs = {}
        # an address inside an instruction (code that jumps into the middle of another
        # instruction) cannot carry a label: a branch there is written as bytes
        inside = {}
        for a, x in ins.items():
            for k in range(a + 1, a + x.len):
                inside[k] = a
        for a, x in ins.items():
            if x.op0_kind == I.OpKind.NEAR_BRANCH16:
                t = self.target(x)
                if t in inside and t in ins:
                    # both an instruction and the middle of another: the outer one becomes
                    # bytes with the label between them
                    self.force_db.add(inside[t])
                    targets.add(t)
                elif t in inside:
                    self.force_db.add(a)
                elif self.start <= t < self.end:
                    if a not in self.force_db:
                        targets.add(t)
                elif t in self.funcs or any(s <= t < e for s, e in self.asm_ranges):
                    externs[self.label(t)] = 'NEAR'
                else:
                    self.force_db.add(a)      # a target no module defines: keep the bytes
            if x.code in (I.Code.CALL_PTR1616, I.Code.JMP_PTR1616):
                t = struct.unpack_from('<H', self.img, a + 1)[0]
                if (a + 3) in self.relocs and not self.start <= t < self.end:
                    externs[self.label(t)] = 'NEAR'
        for t in self.wanted:                  # labels other modules use
            if t in ins:
                targets.add(t)
        fmt = I.Formatter(I.FormatterSyntax.MASM)
        fmt.uppercase_hex = False
        fmt.hex_prefix = ''
        fmt.hex_suffix = 'h'
        fmt.add_leading_zero_to_hex_numbers = True
        fmt.show_branch_size = False
        fmt.always_show_segment_register = False
        fmt.space_after_operand_separator = False
        body = []
        datarefs = {}
        a = self.start
        while a < self.end:
            if a in self.entries or a in targets:
                lab = self.label(a)
                if a in self.funcs or a in self.wanted:
                    body.append('')
                    body.append('\tPUBLIC\t%s' % lab)
                body.append('%s:' % lab)
            if a in ins and a not in self.force_db:
                x = ins[a]
                text = self.instr_text(x, fmt, externs, datarefs, a)
                body.append('\t%s' % text if text else '\tdb\t%s' % ', '.join('0%02xh' % b for b in self.img[a:a + x.len]))
                a += x.len
            else:
                n = 1
                if a in ins:                      # forced to bytes: keep it as one line with its text
                    n = ins[a].len
                    cut = [k for k in range(1, n) if a + k in targets]
                    if not cut:
                        body.append('\tdb\t%s\t; %s' % (', '.join('0%02xh' % b for b in self.img[a:a + n]), fmt.format(ins[a])))
                    else:                         # code jumps into the middle: labels between the bytes
                        body.append('\t\t\t; %s (overlapping code)' % fmt.format(ins[a]))
                        prev = 0
                        for k in cut + [n]:
                            if prev:
                                body.append('%s:' % self.label(a + prev))
                            body.append('\tdb\t%s' % ', '.join('0%02xh' % b for b in self.img[a + prev:a + k]))
                            prev = k
                else:
                    while a + n < self.end and a + n not in ins and a + n not in targets and a + n not in self.entries and n < 8:
                        n += 1
                    body.append('\tdb\t%s' % ', '.join('0%02xh' % b for b in self.img[a:a + n]))
                a += n
        if self.ovl:
            where = '%s.OVL %04x-%04x' % (self.ovl, self.start - self.ovlbase, self.end - self.ovlbase)
        else:
            where = 'ULTIMA.EXE CS:%04x-%04x' % (self.start, self.end)
        hdr = ['; %s.ASM: %s, hand-written assembly in the original.' % (self.name, where),
               '; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.',
               '', '\tTITLE\t%s' % self.name,
               "%s\tSEGMENT  %s PUBLIC '%s'" % (self.segname[0], self.segname[2], self.segname[1]),
               '%s\tENDS' % self.segname[0]]
        text = '\n'.join(body)
        others = [n for v, n in sorted(SEGVALUES.items())] + sorted(set(re.findall(r'\bSEG_[0-9A-F]{4}\b', text)))
        for n in others:                                # other segments a relocated word names
            if n not in ('_TEXT', 'DGROUP', self.segname[0]) and re.search(r'\b%s\b' % n, text):
                hdr += ["%s\tSEGMENT  PARA PUBLIC '%s'" % (n, 'MARK' if n.startswith('SEG_') else 'CODE'),
                        '%s\tENDS' % n]
        if self.segname[0] != '_TEXT' and re.search(r'\b_TEXT\b', text):
            hdr += ["_TEXT\tSEGMENT  BYTE PUBLIC 'CODE'", '_TEXT\tENDS']
        hdr += ["_DATA\tSEGMENT  WORD PUBLIC 'DATA'", '_DATA\tENDS',
                "CONST\tSEGMENT  WORD PUBLIC 'CONST'", 'CONST\tENDS',
                "_BSS\tSEGMENT  WORD PUBLIC 'BSS'", '_BSS\tENDS',
                'DGROUP\tGROUP\tCONST, _BSS, _DATA',
                '\tASSUME\tCS: %s, DS: DGROUP, SS: DGROUP, ES: DGROUP' % self.segname[0], '']
        for n in sorted(externs):
            hdr.append('\tEXTRN\t%s:%s' % (n, externs[n]))
        for n, size in sorted(datarefs.items()):
            hdr.append('\tEXTRN\t%s:%s' % (n, {1: 'BYTE', 2: 'WORD'}.get(size, 'BYTE')))
        out = hdr + ['', '%s\tSEGMENT' % self.segname[0]] + body + ['', '%s\tENDS' % self.segname[0], '\tEND', '']
        return '\r\n'.join(out)

    def instr_text(self, x, fmt, externs, datarefs, a):
        """MASM text for one instruction, symbolic where it can be; None to write bytes."""
        rel = [r for r in self.relocs if a <= r < a + x.len]
        if rel:
            # a word the EXE relocates is a segment: write it as one, so the object carries a
            # segment fixup and the link rebuilds the relocation entry
            r = rel[0]
            val = struct.unpack_from('<H', self.img, r)[0]
            seg = segname(val)
            if x.mnemonic == I.Mnemonic.MOV and x.op1_kind == I.OpKind.IMMEDIATE16 and r == a + x.len - 2:
                return 'mov\t%s,%s' % (fmt.format_operand(x, 0), seg)
            if x.code in (I.Code.CALL_PTR1616, I.Code.JMP_PTR1616) and r == a + 3 and val == 0:
                tgt = struct.unpack_from('<H', self.img, a + 1)[0]
                return 'db\t0%02xh\r\n\tdw\tOFFSET %s\r\n\tdw\tSEG %s' % (self.img[a], self.label(tgt), self.label(tgt))
            # any other form: its bytes, with the relocated word as the segment it holds
            parts, k = [], a
            while k < a + x.len:
                if k == r:
                    parts.append('dw\t%s' % seg)
                    k += 2
                else:
                    n = (r - k) if k < r else (a + x.len - k)
                    parts.append('db\t%s' % ', '.join('0%02xh' % c for c in self.img[k:k + n]))
                    k += n
            return '\r\n\t'.join(parts) + '\t; %s' % fmt.format(x)
        t = fmt.format(x)
        if x.mnemonic == I.Mnemonic.RETF or x.code in (I.Code.CALL_PTR1616, I.Code.JMP_PTR1616):
            # MASM 4.0 has no RETF outside a FAR PROC, nor a far jump to a literal seg:offset
            return 'db	%s	; %s' % (', '.join('0%02xh' % c for c in self.img[a:a + x.len]), t)
        if x.op0_kind == I.OpKind.NEAR_BRANCH16:
            tgt = self.target(x)
            lab = self.label(tgt)
            mn = t.split()[0]
            if x.mnemonic == I.Mnemonic.JMP and x.len == 2:
                return '%s\tSHORT %s' % (mn, lab)
            if x.mnemonic == I.Mnemonic.JMP:
                return '%s\tNEAR PTR %s' % (mn, lab)
            return '%s\t%s' % (mn, lab)
        # direct memory operand [xxxx] without base/index: a DGROUP variable
        for k in range(x.op_count):
            if x.op_kind(k) == I.OpKind.MEMORY and x.memory_base == I.Register.NONE and x.memory_index == I.Register.NONE \
                    and x.memory_segment in (I.Register.DS,) and x.segment_prefix == I.Register.NONE:
                disp = x.memory_displacement & 0xffff
                sym = None
                for base in sorted(self.data, reverse=True):
                    if base <= disp and disp - base < 0x400:
                        sym = (base, self.data[base])
                        break
                if sym and (sym[1][1] is not None and disp == sym[0] or sym[1][1] is None):
                    nm, size, typ = sym[1]
                    off = disp - sym[0]
                    width = x.memory_size
                    ptr = {I.MemorySize.UINT8: 'BYTE PTR ', I.MemorySize.UINT16: 'WORD PTR ',
                           I.MemorySize.INT8: 'BYTE PTR ', I.MemorySize.INT16: 'WORD PTR ',
                           I.MemorySize.UINT32: 'DWORD PTR ', I.MemorySize.INT32: 'DWORD PTR ',
                           I.MemorySize.SEG_PTR16: 'DWORD PTR '}.get(width, '')
                    datarefs[nm] = size or 1
                    ref = '%s%s%s' % (ptr, nm, '+%d' % off if off else '')
                    t = re.sub(r'(byte ptr |word ptr |dword ptr )?(ds:)?\[[0-9a-f]+h\]', ref, t, flags=re.I)
        return t.replace(',', ',', 1)


def build(name, gen, keep=False, rounds=80):
    """Write src/asm/NAME.ASM, assemble, compare, force differing instructions to bytes."""
    import asmbuild
    os.makedirs(ASM_DIR, exist_ok=True)
    for r in range(rounds):
        src = gen.emit()
        path = os.path.join(ASM_DIR, name + '.ASM')
        open(path, 'w', newline='').write(src)
        obj, log = asmbuild.assemble(path)
        if obj is None:
            print(log)
            sys.exit('MASM failed on %s' % path)
        seg = obj.seg(gen.segname[0])
        orig = gen.img[gen.start:gen.end]
        masked = {k for fx in seg.fixups for k in range(fx.loc, fx.loc + fx.size)}
        if seg.length != len(orig):
            print('round %d: length %d, original %d' % (r, seg.length, len(orig)))
        bad = [k for k in range(min(seg.length, len(orig))) if k not in masked and seg.data[k] != orig[k]]
        ins = gen.walk()
        starts = sorted(ins)
        newforce = set()

        def owner(k):
            a = gen.start + k
            s = max([x for x in starts if x <= a] or [None]) if starts else None
            return s if s is not None and a < s + ins[s].len else None
        if seg.length == len(orig):
            # same length: every differing instruction is an encoding choice; force them all
            newforce = {owner(k) for k in bad} - {None}
        else:
            # a length difference shifts everything after it: only the first difference is real
            first = min(bad) if bad else min(seg.length, len(orig))
            s = owner(first)
            if s is not None:
                newforce = {s}
        if not bad and seg.length == len(orig):
            unforce_branches(name, gen)
            print('%s: identical to the original (%d bytes, %d instructions as bytes)' % (name, len(orig), len(gen.force_db)))
            return True
        if not newforce - gen.force_db:
            print('%s: %d bytes still differ and no instruction left to force' % (name, len(bad)))
            return False
        gen.force_db |= newforce
    print('%s: not identical after %d rounds' % (name, rounds))
    return False


def identical(name, gen):
    import asmbuild
    path = os.path.join(ASM_DIR, name + '.ASM')
    open(path, 'w', newline='').write(gen.emit())
    obj, log = asmbuild.assemble(path)
    if obj is None:
        return False
    seg = obj.seg(gen.segname[0])
    orig = gen.img[gen.start:gen.end]
    masked = {k for fx in seg.fixups for k in range(fx.loc, fx.loc + fx.size)}
    return seg.length == len(orig) and all(k in masked or seg.data[k] == orig[k] for k in range(len(orig)))


def unforce_branches(name, gen):
    """Branches are forced to bytes when an instruction before their target assembled to a
    different length; once that instruction is bytes, the branch is fine as text again.
    Give each forced branch back its label, keeping only those that are really needed."""
    ins = gen.walk()
    br = sorted(a for a in gen.force_db if ins[a].op0_kind == I.OpKind.NEAR_BRANCH16)
    if not br:
        return
    keep = set(gen.force_db)
    gen.force_db = keep - set(br)
    if identical(name, gen):
        return
    gen.force_db = keep
    for a in br:
        gen.force_db = gen.force_db - {a}
        if not identical(name, gen):
            gen.force_db = gen.force_db | {a}
    identical(name, gen)           # leave the file as the final state


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('name')
    ap.add_argument('start', type=lambda x: int(x, 16))
    ap.add_argument('end', type=lambda x: int(x, 16))
    ap.add_argument('--entry', action='append', default=[], type=lambda x: int(x, 16))
    ap.add_argument('--seg', default='0', type=lambda x: int(x, 16), help='a far segment (paragraph), e.g. 072e')
    ap.add_argument('--ovl', type=str.upper, choices=u5.OVERLAYS, metavar='OVERLAY',
                    help='an assembly part of OVERLAY.OVL: START, END and --entry are offsets in that file')
    a = ap.parse_args()
    if a.ovl and a.seg:
        ap.error('--ovl and --seg do not go together')
    off = u5.OVERLAY_BASE[a.ovl] if a.ovl else 0
    g = Gen(a.name.upper(), a.start, a.end, [e + off for e in a.entry], seg=a.seg, ovl=a.ovl)
    ok = build(a.name.upper(), g)
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
