#!/usr/bin/env python3
"""Compare compiled functions with the original game, function by function.

    python tools/match.py [FILE.c ...] [--compile] [-v] [--dis FUNCTION] [--gamedata DIR]

Reads build/obj/NAME.OBJ (run tools/dosbuild.py first, or pass --compile) and compares
every function in its _TEXT segment with the bytes at the same function in the GOG files:

  - overlay modules (town.c ...): the function TOWN_0052 is at offset 0x52 of TOWN.OVL;
  - resident modules (0000.c-7000.c, ultima.c): ULTIMA_1850 is at CS:1850 of ULTIMA.EXE.

Bytes covered by a fixup are not compared as bytes. Each fixup is checked instead against
what the original holds there, using the addresses u5d's names carry:

  - a call or jump to ULTIMA_xxxx / OVERLAY_xxxx must reach that address in the original
    (overlay addresses are area base + offset, see tools/u5.py);
  - an offset of D_xxxx (+ displacement) must be xxxx (+ displacement);
  - an offset into the module's own _DATA/CONST/_BSS (string literals, statics) must be the
    same segment base for every reference; the base is learned from the first one;
  - an offset into the module's own _TEXT must point to the corresponding original place;
  - names without an address (runtime library) are looked up in the comments of 0000.c
    ("// 0442: aNlmul"); anything else is reported as UNVERIFIED, never passed silently.

Each function gets a verdict:
  EXACT    every byte equal and every fixup verified
  SLOTS    the same except for bp-relative displacements (stack slots of locals)
  FIXUP    the bytes equal, but a fixup points somewhere else than the original
  SIZE     the function has a different length
  CODE     anything else
and the results are written to build/match/status.tsv. Exit status 0 when every function
of the files given is EXACT.
"""
import argparse, glob, os, re, struct, sys

try:
    import iced_x86
except ImportError:
    sys.exit('iced-x86 is needed: python -m venv .venv && .venv/Scripts/pip install -r tools/requirements.txt '
             '(on Linux/macOS .venv/bin/pip), then run the tools with that Python')

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402

OBJ = os.path.join(u5.BUILD, 'obj')
OUT = os.path.join(u5.BUILD, 'match')
VERDICTS = ['EXACT', 'SLOTS', 'FIXUP', 'SIZE', 'CODE']


def runtime_symbols():
    """CS addresses of runtime-library routines, from the '// 0442: aNlmul' comments."""
    syms = {}
    for p in glob.glob(os.path.join(u5.SRC, '[0-7]000.c')):
        for m in re.finditer(r'^// ([0-9a-f]{4}): (\w+)', open(p, encoding='utf-8', errors='replace').read(), re.M):
            off, name = int(m.group(1), 16), m.group(2)
            for k in (name, '_' + name, '__' + name):
                syms[k.upper()] = ('code', off)
    return syms


def root_starts():
    """Every known function start in ULTIMA.EXE's resident code."""
    starts = set()
    for p in glob.glob(os.path.join(u5.SRC, '*.c')):
        for m in re.finditer(r'\bULTIMA_([0-9a-fA-F]{4})', open(p, encoding='utf-8', errors='replace').read()):
            starts.add(int(m.group(1), 16))
    starts.update(off for kind, off in runtime_symbols().values())
    return sorted(starts)


class Module:
    def __init__(self, cfile, gamedata):
        self.cfile = cfile
        self.kind, self.ovl = u5.MODULES[cfile]
        self.stem = os.path.splitext(cfile)[0].upper()
        path = os.path.join(OBJ, self.stem + '.OBJ')
        if not os.path.exists(path):
            raise FileNotFoundError(path)
        self.obj = omf.Object(path)
        self.text = self.obj.seg('_TEXT')
        self.exe = u5.exe_image(gamedata)
        self.stubs = u5.overlay_stubs(self.exe)
        self.ds = u5.ds_image(gamedata)
        if self.kind == 'ovl':
            self.orig = u5.read_file(gamedata, self.ovl + '.OVL')
            self.base = u5.OVERLAY_BASE.get(self.ovl, 0)
        else:
            self.orig = self.exe
            self.base = 0
        self.data_base = {}       # module data segment -> DS base (the most common one)

    def vote_data_bases(self, funcs):
        """Each fixup into the module's own data implies where that segment starts in DS.
        Take the answer of the reference nearest the start of the segment, so that a
        disagreeing fixup's drift is the number of bytes missing (or extra) before it."""
        votes = {}
        first = {}
        for fx in self.text.fixups:
            kind, tname = fx.target
            if fx.self_rel or fx.kind != omf.LOC_OFFSET or kind not in ('seg', 'grp') or tname == '_TEXT':
                continue
            o = self.orig_loc(fx.loc, funcs)
            if o is None or o + 2 > len(self.orig):
                continue
            w = struct.unpack_from('<H', self.orig, o)[0]
            d = eff_disp(self.text, fx)
            base = (w - d) & 0xffff
            votes.setdefault(tname, {}).setdefault(base, 0)
            votes[tname][base] += 1
            if tname not in first or d < first[tname][0]:
                first[tname] = (d, base)
        for seg in votes:
            self.data_base[seg] = first[seg][1]

    def orig_loc(self, loc, funcs):
        for name, a, b, o, oe in funcs:
            if a <= loc < b:
                return o + loc - a
        return None

    def compare_data(self):
        """Compare the module's initialised _DATA with DGROUP at the voted base:
        (length, differing bytes, first difference) or None."""
        seg = self.obj.seg('_DATA')
        if not seg or not seg.length or '_DATA' not in self.data_base:
            return None
        base = self.data_base['_DATA']
        masked = set()
        for fx in seg.fixups:
            masked.update(range(fx.loc, fx.loc + fx.size))
        theirs = self.ds[base:base + seg.length]
        diffs = [k for k in range(min(len(theirs), seg.length)) if k not in masked and seg.data[k] != theirs[k]]
        return base, seg.length, len(diffs), (diffs[0] if diffs else None)

    def functions(self):
        """[(name, our_offset, our_end, orig_offset_in_self.orig, orig_end)]"""
        pubs = sorted(self.text.publics.items(), key=lambda kv: kv[1])
        ours = []
        for i, (name, off) in enumerate(pubs):
            end = pubs[i + 1][1] if i + 1 < len(pubs) else self.text.length
            ours.append((name, off, end))
        out = []
        if self.kind == 'ovl':
            known = []
            for name, off, end in ours:
                m = u5.NAME_RE.match(name)
                if m and m.group(1).upper() == self.ovl:
                    known.append((name, off, end, int(m.group(2), 16)))
                elif self.ovl == 'FLAMES' and len(ours) == 1:
                    known.append((name, off, end, 0))
            known_starts = sorted(k[3] for k in known)
            for name, off, end, o in known:
                nxt = [s for s in known_starts if s > o]
                oend = nxt[0] if nxt else len(self.orig)
                out.append((name, off, end, o, oend))
        else:
            starts = root_starts()
            for name, off, end in ours:
                a = u5.name_address(name)
                if name.upper() == '_MAIN':
                    a = ('code', 0)                  # main() is the first function, CS:0000
                elif not a or a[0] != 'code' or u5.module_owner(name) != 'ULTIMA':
                    continue
                o = a[1]
                nxt = [s for s in starts if s > o]
                oend = nxt[0] if nxt else len(self.orig)
                out.append((name, off, end, o, oend))
        return out

    # mapping of our _TEXT offsets to original offsets (for fixups into our own code)
    def map_text(self, off, funcs):
        for name, a, b, o, oe in funcs:
            if a <= off < b or (off == b and b == self.text.length):
                return o + (off - a)
        return None


def eff_disp(seg, fx):
    d = fx.disp
    if fx.kind == omf.LOC_OFFSET and not fx.self_rel:
        d += struct.unpack_from('<H', seg.data, fx.loc)[0]
    return d & 0xffff


def check_fixup(mod, fx, o_loc, funcs, rtsyms):
    """Returns (ok, message). o_loc: offset of the fixup in mod.orig."""
    if fx.kind not in (omf.LOC_OFFSET,):
        return None, 'fixup kind %d not checked' % fx.kind
    orig_word = struct.unpack_from('<H', mod.orig, o_loc)[0]
    cs_loc = mod.base + o_loc
    kind, tname = fx.target
    if fx.self_rel:
        actual = (cs_loc + 2 + orig_word) & 0xffff      # where the original call/jmp goes
        if kind == 'ext':
            want = u5.name_address(tname) or rtsyms.get(tname.upper())
            if not want:
                return None, 'call to %s: no address known' % tname
            if want[0] != 'code':
                return False, 'call to data %s' % tname
            if actual == want[1]:
                return True, 'call %s' % tname
            # a function of another overlay is called through its stub in the resident code
            owner = u5.module_owner(tname)
            if actual in mod.stubs and owner in u5.OVERLAY_NUMBER:
                num, tgt = mod.stubs[actual]
                ok = tgt == want[1] and num == u5.OVERLAY_NUMBER[owner]
                return ok, 'call %s via stub %04x (overlay %d -> %04x)' % (tname, actual, num, tgt)
            return False, 'call %s: original goes to %04x, name says %04x' % (tname, actual, want[1])
        if kind == 'seg' and tname == '_TEXT':
            t = mod.map_text(fx.disp, funcs)
            return (t is not None and actual == (mod.base + t) & 0xffff), \
                'near jump into own code: original %04x' % actual
        return None, 'relative fixup to %r not checked' % (fx.target,)
    d = eff_disp(mod.text, fx)
    if kind == 'ext':
        want = u5.name_address(tname) or rtsyms.get(tname.upper())
        if not want:
            return None, 'offset of %s: no address known' % tname
        exp = (want[1] + d) & 0xffff
        return orig_word == exp, 'offset %s+%d: original %04x, expected %04x' % (tname, d, orig_word, exp)
    if kind in ('seg', 'grp') and tname in ('_DATA', 'CONST', '_BSS', 'DGROUP'):
        # fixups relative to DGROUP carry the displacement within the segment named in the
        # frame; MSC writes data offsets as target segment + displacement
        key = tname
        base = (orig_word - d) & 0xffff
        if key not in mod.data_base:
            return None, 'offset in %s: no base known' % key
        return mod.data_base[key] == base, 'offset %s+%04x: original DS:%04x, expected DS:%04x (layout drift %+d)' % (
            key, d, orig_word, (mod.data_base[key] + d) & 0xffff, (base - mod.data_base[key] + 0x8000) % 0x10000 - 0x8000)
    if kind == 'seg' and tname == '_TEXT':
        t = mod.map_text(d, funcs)
        exp = None if t is None else (mod.base + t) & 0xffff
        return orig_word == exp, 'offset in own code: original %04x, expected %s' % (
            orig_word, '%04x' % exp if exp is not None else '?')
    return None, 'fixup to %r not checked' % (fx.target,)


BRANCH = (iced_x86.FlowControl.CALL, iced_x86.FlowControl.UNCONDITIONAL_BRANCH,
          iced_x86.FlowControl.CONDITIONAL_BRANCH)


def decode(data, ip):
    """16-bit instructions of data, ip = address of data[0]."""
    return list(iced_x86.Decoder(16, bytes(data), ip=ip))


def check_local_branches(mod, a, b, o, ours, funcs, fixed):
    """Relative calls and jumps from this function into another function of the same
    module carry no fixup: the compiler resolved them. Check each against the original's
    layout instead. Returns (masked byte offsets, bad count, notes)."""
    masked, bad, notes = set(), 0, []
    for ins in decode(ours, a):
        if ins.flow_control not in BRANCH or ins.op0_kind != iced_x86.OpKind.NEAR_BRANCH16:
            continue
        pos = ins.ip - a
        size = 1 if ins.len == 2 else 2          # rel8 (short jumps) or rel16
        rel_at = pos + ins.len - size
        if any(k in fixed for k in range(rel_at, rel_at + size)):
            continue                             # a fixup: checked elsewhere
        tgt = ins.near_branch16
        if a <= tgt < b:
            continue                             # inside the function: bytes are compared
        want = mod.map_text(tgt, funcs)
        masked.update(range(rel_at, rel_at + size))
        oloc = o + rel_at
        if oloc + size > len(mod.orig):
            continue
        rel = struct.unpack_from('<h' if size == 2 else '<b', mod.orig, oloc)[0]
        actual = (mod.base + o + pos + ins.len + rel) & 0xffff
        exp = None if want is None else (mod.base + want) & 0xffff
        if actual != exp:
            bad += 1
            notes.append('local branch @%04x: original goes to %04x, expected %s' % (
                pos, actual, '%04x' % exp if exp is not None else '?'))
    return masked, bad, notes


def bp_disp(data, k):
    """True if byte k is (part of) the displacement of a [bp+disp] operand."""
    for back, mask, val in ((1, 0xC7, 0x46), (1, 0xC7, 0x86), (2, 0xC7, 0x86)):
        if k - back >= 0 and (data[k - back] & mask) == val:
            return True
    return False


def compare(mod, rtsyms, verbose=False):
    funcs = mod.functions()
    mod.vote_data_bases(funcs)
    fixups = sorted(mod.text.fixups, key=lambda f: f.loc)
    results = []
    for name, a, b, o, oe in funcs:
        ours = bytes(mod.text.data[a:b])
        olen = oe - o
        theirs = mod.orig[o:o + len(ours)]
        masked = set()
        notes = []
        fix_bad = fix_unv = 0
        for fx in fixups:
            if a <= fx.loc < b:
                masked.update(range(fx.loc - a, fx.loc - a + fx.size))
                if fx.loc - a + fx.size > len(theirs):
                    continue
                ok, msg = check_fixup(mod, fx, o + fx.loc - a, funcs, rtsyms)
                if ok is False:
                    fix_bad += 1
                    notes.append('fixup @%04x: %s' % (fx.loc - a, msg))
                elif ok is None:
                    fix_unv += 1
                    notes.append('unverified @%04x: %s' % (fx.loc - a, msg))
        lm, lbad, lnotes = check_local_branches(mod, a, b, o, ours, funcs, masked)
        masked |= lm
        fix_bad += lbad
        notes += lnotes
        diffs =[k for k in range(min(len(ours), len(theirs))) if k not in masked and ours[k] != theirs[k]]
        # a function may be followed by alignment padding in the original
        size_ok = len(ours) == olen or (len(ours) < olen and all(x in (0, 0x90) for x in mod.orig[o + len(ours):oe])
                                        and oe - o - len(ours) < 16)
        # worst first: SIZE, CODE, SLOTS, FIXUP, EXACT. A SLOTS function may also have bad
        # fixups; they are counted in bad_fixups and listed in the notes.
        if not size_ok:
            v = 'SIZE'
        elif diffs and not all(bp_disp(ours, k) and bp_disp(theirs, k) for k in diffs):
            v = 'CODE'
        elif diffs:
            v = 'SLOTS'
        elif fix_bad:
            v = 'FIXUP'
        else:
            v = 'EXACT'
        if v == 'SIZE':
            notes.insert(0, 'length %d, original %d' % (len(ours), olen))
        results.append(dict(module=mod.cfile, func=name, addr=(mod.base + o) & 0xffff, verdict=v,
                            ours=len(ours), orig=olen, diffs=len(diffs), bad_fixups=fix_bad,
                            unverified=fix_unv, first=(diffs[0] if diffs else None), notes=notes))
    return results


def dis(mod, fname, rtsyms):
    """Print our and the original bytes of one function side by side, with the listing."""
    for name, a, b, o, oe in mod.functions():
        if name.upper() != fname.upper():
            continue
        ours = mod.text.data[a:b]
        theirs = mod.orig[o:max(oe, o + len(ours))]
        fx = {f.loc - a: f for f in mod.text.fixups if a <= f.loc < b}
        print('%s: ours %d bytes at _TEXT:%04x, original %d bytes at %04x' % (name, b - a, a, oe - o, o))
        fmt = iced_x86.Formatter(iced_x86.FormatterSyntax.MASM)
        L = decode(ours, 0)
        R = decode(theirs[:oe - o], 0)
        print('  %-44s | %s' % ('ours (fixup targets as in the object: 0000)', 'original'))
        for i in range(max(len(L), len(R))):
            l = L[i] if i < len(L) else None
            r = R[i] if i < len(R) else None
            ls = '%04x %-12s %s' % (l.ip, ours[l.ip:l.ip + l.len].hex(), fmt.format(l)) if l else ''
            rs = '%04x %-12s %s' % (r.ip, theirs[r.ip:r.ip + r.len].hex(), fmt.format(r)) if r else ''
            same = l and r and ours[l.ip:l.ip + l.len] == theirs[r.ip:r.ip + r.len]
            print('  %-44s | %-44s %s' % (ls[:44], rs[:44], '' if same else '<'))
        for k in sorted(fx):
            ok, msg = check_fixup(mod, fx[k], o + k, mod.functions(), rtsyms)
            print('  fixup %04x %-5s %s' % (k, {True: 'ok', False: 'BAD', None: '?'}[ok], msg))
        cod = os.path.join(OBJ, mod.stem + '.COD')
        if os.path.exists(cod):
            print('\nlisting: %s (search for "%s\tPROC")' % (cod, name))
        return
    sys.exit('no function %s in %s' % (fname, mod.cfile))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('files', nargs='*')
    ap.add_argument('--compile', action='store_true', help='compile the files first (tools/dosbuild.py)')
    ap.add_argument('--gamedata')
    ap.add_argument('--dosbox')
    ap.add_argument('-v', '--verbose', action='store_true', help='list every function, with notes')
    ap.add_argument('--dis', metavar='FUNCTION', help='show one function byte by byte')
    a = ap.parse_args()
    gamedata = u5.find_gamedata(a.gamedata)
    files = [os.path.basename(f) for f in a.files] or sorted(f for f, (k, _) in u5.MODULES.items() if k != 'data')
    for f in files:
        if f not in u5.MODULES:
            sys.exit('%s is not a module of the DOS build (tools/u5.py MODULES)' % f)
        if u5.MODULES[f][0] == 'data':
            sys.exit('%s holds data; data is not compared yet' % f)
    if a.compile:
        import dosbuild
        dosbuild.compile_files(files, a.dosbox, quiet=True)
    rtsyms = runtime_symbols()
    allres = []
    for f in files:
        try:
            mod = Module(f, gamedata)
        except FileNotFoundError as e:
            print('%-12s no object (%s): compile it first' % (f, e))
            continue
        if a.dis:
            dis(mod, a.dis, rtsyms)
            return
        res = compare(mod, rtsyms)
        allres += res
        counts = {v: sum(1 for r in res if r['verdict'] == v) for v in VERDICTS}
        extra = ''
        if mod.kind == 'ovl':
            pad = len(mod.orig) - mod.text.length
            extra = '  _TEXT %d/%d bytes' % (mod.text.length, len(mod.orig))
        unv = sum(r['unverified'] for r in res)
        bad = sum(r['bad_fixups'] for r in res)
        dc = mod.compare_data()
        if dc:
            extra += '  _DATA@DS:%04x %d bytes, %s' % (dc[0], dc[1], 'same' if not dc[2] else
                                                       '%d differ from +%04x' % (dc[2], dc[3]))
        print('%-12s %3d functions: %s%s%s%s' % (f, len(res), ' '.join('%s %d' % (v, c) for v, c in counts.items() if c),
                                               extra, '  bad fixups %d' % bad if bad else '',
                                               '  unverified %d' % unv if unv else ''))
        if a.verbose:
            for r in res:
                print('    %-34s %04x %-5s len %4d/%-4d diffs %3d%s' % (
                    r['func'], r['addr'], r['verdict'], r['ours'], r['orig'], r['diffs'],
                    '' if r['first'] is None else ' first @%04x' % r['first']))
                for n in r['notes'][:6]:
                    print('        ' + n)
    os.makedirs(OUT, exist_ok=True)
    tsv = os.path.join(OUT, 'status.tsv')
    old = {}
    if os.path.exists(tsv):
        for line in open(tsv).read().splitlines()[1:]:
            p = line.split('\t')
            old[(p[0], p[1])] = p
    for r in allres:
        old[(r['module'], r['func'])] = [r['module'], r['func'], '%04x' % r['addr'], r['verdict'],
                                        str(r['ours']), str(r['orig']), str(r['diffs'])]
    with open(tsv, 'w', newline='\n') as fh:
        fh.write('module\tfunction\taddress\tverdict\tlength\toriginal\tdiffering_bytes\n')
        for k in sorted(old):
            fh.write('\t'.join(old[k]) + '\n')
    tot = {v: sum(1 for r in allres if r['verdict'] == v) for v in VERDICTS}
    print('total %d functions: %s' % (len(allres), ' '.join('%s %d' % kv for kv in tot.items())))
    if any(r['verdict'] != 'EXACT' for r in allres):
        sys.exit(1)


if __name__ == '__main__':
    main()
