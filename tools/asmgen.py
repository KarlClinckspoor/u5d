#!/usr/bin/env python3
"""Write an assembly source for a range of the original code, and make it assemble to it.

    python tools/asmgen.py NAME START END [--entry ADDR ...] [--keep]

For the resident image (CS offsets START..END of ULTIMA.EXE), writes src/asm/NAME.ASM:
MASM 4.0 source in Microsoft C's small-model segments, which tools/asmbuild.py assembles to
the original bytes. The source is drafted from the bytes, then checked:

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
import argparse, os, re, shutil, struct, subprocess, sys, tempfile

import iced_x86 as I

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402
import rootmap  # noqa: E402

ASM_DIR = os.path.join(u5.SRC, 'asm')
MASM_DIR = os.path.join(u5.BUILD, 'masm')


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
    def __init__(self, name, start, end, entries=(), gamedata=None):
        self.name, self.start, self.end = name, start, end
        self.gamedata = u5.find_gamedata(gamedata)
        self.img = u5.exe_image(self.gamedata)
        self.relocs = relocations(self.gamedata)
        tags = rootmap.function_tags()
        self.funcs = {a: exported(n) for a, (n, f, t) in tags.items()}
        self.funcs.update({a: exported('_' + n.split()[0]) for a, n in rootmap.crt_names().items()
                           if a not in self.funcs and re.match(r'^\w+$', n.split()[0])})
        self.data = data_symbols()
        self.entries = sorted(set([a for a in self.funcs if start <= a < end] + list(entries)))
        self.force_db = set()        # instruction addresses to write as bytes

    # -- analysis
    def decode_at(self, a):
        d = I.Decoder(16, self.img[a:self.end], ip=a)
        return next(iter(d))

    def walk(self):
        """Instruction starts reached from the entries."""
        seen, todo = {}, list(self.entries)
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
                    t = ins.near_branch16
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
        return 'L_%04x' % a

    def emit(self):
        ins = self.walk()
        targets = set()
        externs = {}
        for a, x in ins.items():
            if x.op0_kind == I.OpKind.NEAR_BRANCH16:
                t = x.near_branch16
                if self.start <= t < self.end:
                    targets.add(t)
                else:
                    externs[self.label(t)] = 'NEAR'
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
                if a in self.funcs:
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
                    body.append('\tdb\t%s\t; %s' % (', '.join('0%02xh' % b for b in self.img[a:a + n]), fmt.format(ins[a])))
                else:
                    while a + n < self.end and a + n not in ins and a + n not in targets and a + n not in self.entries and n < 8:
                        n += 1
                    body.append('\tdb\t%s' % ', '.join('0%02xh' % b for b in self.img[a:a + n]))
                a += n
        hdr = ['; %s.ASM: ULTIMA.EXE CS:%04x-%04x, hand-written assembly in the original.' % (self.name, self.start, self.end),
               '; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.',
               '', '\tTITLE\t%s' % self.name,
               "_TEXT\tSEGMENT  BYTE PUBLIC 'CODE'", '_TEXT\tENDS',
               "_DATA\tSEGMENT  WORD PUBLIC 'DATA'", '_DATA\tENDS',
               "CONST\tSEGMENT  WORD PUBLIC 'CONST'", 'CONST\tENDS',
               "_BSS\tSEGMENT  WORD PUBLIC 'BSS'", '_BSS\tENDS',
               'DGROUP\tGROUP\tCONST, _BSS, _DATA',
               '\tASSUME\tCS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP', '']
        for n in sorted(externs):
            hdr.append('\tEXTRN\t%s:%s' % (n, externs[n]))
        for n, size in sorted(datarefs.items()):
            hdr.append('\tEXTRN\t%s:%s' % (n, {1: 'BYTE', 2: 'WORD'}.get(size, 'BYTE')))
        out = hdr + ['', '_TEXT\tSEGMENT'] + body + ['', '_TEXT\tENDS', '\tEND', '']
        return '\r\n'.join(out)

    def instr_text(self, x, fmt, externs, datarefs, a):
        """MASM text for one instruction, symbolic where it can be; None to write bytes."""
        if any(r in self.relocs for r in range(a, a + x.len)):
            return None                          # far pointers: written as bytes (with a SEG fixup later)
        t = fmt.format(x)
        if x.op0_kind == I.OpKind.NEAR_BRANCH16:
            tgt = x.near_branch16
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
        seg = obj.seg('_TEXT')
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
    seg = obj.seg('_TEXT')
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
    a = ap.parse_args()
    g = Gen(a.name.upper(), a.start, a.end, a.entry)
    ok = build(a.name.upper(), g)
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
