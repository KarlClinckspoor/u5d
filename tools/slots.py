#!/usr/bin/env python3
"""Microsoft C 5.1's and 5.0's stack-slot order, and renames that give the original's.

    python tools/slots.py --hash NAME ...            print each name's bucket
    python tools/slots.py FILE.c [FUNCTION ...]      report functions whose slots differ
    python tools/slots.py FILE.c --apply [FUNCTION ...]   rename locals so they match

The rule (learned from 190 functions of MSC 5.1's own listings, every one consistent):
MSC 5.1 keeps a function's locals in a hash table of 16 buckets, the bucket being the sum
of the name's characters modulo 16. Locals get their stack slots in that order: bucket 0
first, nearest BP, and within a bucket the most recently declared first. So a local's name
decides where it lives on the stack, and the stack offsets are in the code.

The target comes from u5d's names: Ghidra named each local after its place in the original,
local_N being [bp-(N-2)]. So the locals must be ordered nearest-first by N. Where the hash
order disagrees, --apply renames as few locals as possible: each renamed local keeps its
name with a suffix chosen for the bucket it needs (local_4 -> local_4_b). Declarations
are reordered only where two locals share a bucket. Pick readable names later if you like;
any name works as long as it hashes the same (--hash).

MSC 5.0 (the compiler of the modules in u5.COMPILER_BY_FILE) has the same table and order
but another hash: the name is added up as little-endian 16-bit words (an odd last byte is a
word of its own), and the bucket is (low byte + high byte) of that sum, modulo 16. For
names of up to four characters this is the same bucket as 5.1's; for longer ones each carry
out of the low byte adds one. Measured with both compilers on 230 names and 300 pairs
(docs/MATCHING.md). The file's compiler picks the rule.
"""
import argparse, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402

BUCKETS = 16
SUFFIX_CHARS = 'abcdefghijklmnopqrstuvwxyz0123456789'
# a renamed local is local_N_<suffix>: the underscore keeps N readable (tools and people)


RULE = 'msc51'   # set from the file's compiler (u5.COMPILER_BY_FILE) or --compiler


def bucket(name, rule=None):
    if (rule or RULE) == 'msc50':
        b = name.encode('latin-1')
        if len(b) & 1:
            b += b'\0'
        w = sum(b[i] | b[i + 1] << 8 for i in range(0, len(b), 2)) & 0xffff
        return ((w & 0xff) + (w >> 8)) % BUCKETS
    return sum(map(ord, name)) % BUCKETS


def alloc_order(names):
    """names in declaration order -> the order the compiler gives them slots, nearest BP first."""
    idx = {n: i for i, n in enumerate(names)}
    return sorted(names, key=lambda n: (bucket(n), -idx[n]))


TYPE = (r'(?:(?:unsigned|signed|register|static|const|far|near)\s+)*'
        r'(?:int|char|byte|u8|s8|u16|s16|u32|s32|uint|word|bool|long|short|void|S_\w+|Pair\w*|struct\s+\w+|[A-Z][A-Za-z0-9]*[a-z]\w*)'  # last: typedef names (CombatEntity)
        r'[\s\*]*')
DECL = re.compile(r'^(\s*)(' + TYPE + r')(\w+)(\s*\[[^\]]*\])?(\s*=[^;]*)?;(.*)$')
LOCAL_N = re.compile(r'^local_([0-9a-f]+)(?:_|$)')


def functions(src):
    """[(name, body_start, body_end)] for every function definition in src."""
    out = []
    for m in re.finditer(r'^(?:static\s+)?[\w\s\*]*?\b(\w+)\s*\([^;{)]*\)\s*\n\{', src, re.M):
        depth, i = 1, m.end()
        while depth and i < len(src):
            depth += {'{': 1, '}': -1}.get(src[i], 0)
            i += 1
        out.append((m.group(1), m.end(), i - 1))
    return out


def declarations(body):
    """The declarations at the top of a function body: [(line_index, name, line)]."""
    lines = body.split('\n')
    out = []
    for i, line in enumerate(lines):
        t = line.strip()
        if not t or t.startswith('//'):
            continue
        if t.startswith('#'):
            break                     # conditional declarations: leave the function alone
        m = DECL.match(line)
        if not m or 'static' in m.group(2):
            break
        out.append((i, m.group(3), line))
    return lines, out


def target_order(names):
    """Nearest-BP first, from the local_N names; None if a local has no such name."""
    ns = []
    for n in names:
        m = LOCAL_N.match(n)
        if not m:
            return None
        ns.append((int(m.group(1), 16), n))
    return [n for _, n in sorted(ns)]


def plan(names, want=None):
    """names in declaration order -> (renames {old: new}, new declaration order) or None
    when nothing needs to change. want: the target order (nearest BP first); by default
    from the local_N names."""
    want = want or target_order(names)
    if not want or alloc_order(names) == want:
        return None
    # keep the longest run of names whose buckets already rise along the target order
    b = [bucket(n) for n in want]
    best = [1] * len(want)
    prev = [-1] * len(want)
    for i in range(len(want)):
        for j in range(i):
            if b[j] <= b[i] and best[j] + 1 > best[i]:
                best[i], prev[i] = best[j] + 1, j
    i = max(range(len(want)), key=lambda k: best[k])
    keep = set()
    while i >= 0:
        keep.add(want[i])
        i = prev[i]
    new = {}
    taken = set(names)
    for i, n in enumerate(want):
        if n in keep:
            continue
        lo = max([bucket(new.get(m, m)) for m in want[:i] if m in keep or m in new] or [0])
        hi = min([bucket(m) for m in want[i + 1:] if m in keep] or [BUCKETS - 1])
        cand = None
        m = re.match(r'^(local_[0-9a-f]+)_', n)
        base = m.group(1) if m else n          # local_10_ad -> local_10_xx, not local_10_ad_xx
        for c in SUFFIX_CHARS:
            for c2 in [''] + list(SUFFIX_CHARS):
                x = base + '_' + c + c2
                if lo <= bucket(x) <= hi and x not in taken:
                    cand = x
                    break
            if cand:
                break
        if not cand:
            return None
        new[n] = cand
        taken.add(cand)
    renamed = [new.get(n, n) for n in want]
    # declaration order: keep the current one if it already gives the target order,
    # otherwise declare in reverse target order (ties then resolve nearest-first)
    cur = [new.get(n, n) for n in names]
    order = cur if alloc_order(cur) == renamed else renamed[::-1]
    assert alloc_order(order) == renamed
    return new, order


def observed_targets(cfile):
    """Target orders read from the compiled object against the original: for every function
    that differs only in stack slots, map each bp offset we use to the one the original uses
    in the same instruction, and order our locals by their original offsets.
    {FUNCTION: [names nearest BP first]}. Needs build/obj/NAME.OBJ and .COD (dosbuild.py)."""
    import iced_x86
    import match
    mod = match.Module(cfile, u5.find_gamedata())
    funcs = mod.functions()
    cod = open(os.path.join(u5.OBJ, mod.stem + '.COD'), encoding='latin-1').read()
    out = {}
    for name, a, b, o, oe in funcs:
        if b - a != oe - o and not (b - a < oe - o and oe - o - (b - a) < 16):
            continue
        m = re.search(r'^%s\tPROC NEAR\n(.*?)^%s\tENDP' % (re.escape(name), re.escape(name)), cod, re.M | re.S)
        if not m:
            continue
        ours_off = {n: int(v) for n, v in re.findall(r'^;\t(\w+) = (-\d+)$', m.group(1), re.M)}
        if len(ours_off) < 2:
            continue
        L = match.decode(mod.text.data[a:b], 0)
        R = match.decode(mod.orig[o:o + (b - a)], 0)
        if [i.len for i in L] != [i.len for i in R]:
            continue
        mp, bad = {}, False
        for x, y in zip(L, R):
            for k in range(x.op_count):
                if x.op_kind(k) == iced_x86.OpKind.MEMORY and x.memory_base == iced_x86.Register.BP \
                        and y.memory_base == iced_x86.Register.BP:
                    dx = x.memory_displacement - (0x10000 if x.memory_displacement >= 0x8000 else 0)
                    dy = y.memory_displacement - (0x10000 if y.memory_displacement >= 0x8000 else 0)
                    if dx < 0:
                        if mp.get(dx, dy) != dy:
                            bad = True
                        mp[dx] = dy
        if bad or all(k == v for k, v in mp.items()):
            continue
        orig = {n: mp.get(v, v) for n, v in ours_off.items() if v < 0}
        out[name.upper()] = [n for n, _ in sorted(orig.items(), key=lambda kv: -kv[1])]
    return out


def rewrite(src, only=None, observed=None):
    """Apply plan() to every (or the named) function of src. Returns (new_src, report)."""
    report = []
    for name, a, b in reversed(functions(src)):
        if only and name.upper() not in only:
            continue
        body = src[a:b]
        lines, decls = declarations(body)
        if len(decls) < 2:
            continue
        names = [d[1] for d in decls]
        want = None
        if observed and name.upper() in observed:
            obs = observed[name.upper()]
            # locals the compiler gave no slot (registers, unused) keep their place
            want = obs + [n for n in alloc_order(names) if n not in obs]
            if sorted(want) != sorted(names):
                continue
        p = plan(names, want)
        if not p:
            continue
        new, order = p
        report.append((name, new, order))
        byname = {d[1]: d for d in decls}
        idxs = [d[0] for d in decls]
        newlines = list(lines)
        for slot, n in zip(idxs, order):
            old = next(k for k in names if new.get(k, k) == n)
            newlines[slot] = byname[old][2]
        body = '\n'.join(newlines)
        for old, nn in new.items():
            body = re.sub(r'\b%s\b' % re.escape(old), nn, body)
        src = src[:a] + body + src[b:]
    return src, report[::-1]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('file', nargs='?')
    ap.add_argument('functions', nargs='*')
    ap.add_argument('--hash', nargs='+', metavar='NAME')
    ap.add_argument('--compiler', choices=('msc51', 'msc50'),
                    help="the hash rule (default: the file's compiler, u5.COMPILER_BY_FILE)")
    ap.add_argument('--apply', action='store_true')
    ap.add_argument('--observed', action='store_true',
                    help='take the target order from the compiled object against the original '
                         '(any names; needs dosbuild.py output) instead of the local_N names')
    a = ap.parse_args()
    global RULE
    if a.hash:
        for n in a.hash:
            print('%-24s bucket %d (MSC 5.1), %d (MSC 5.0)' % (n, bucket(n, 'msc51'), bucket(n, 'msc50')))
        return
    if not a.file:
        ap.error('give a file or --hash')
    path = a.file if os.path.exists(a.file) else os.path.join(u5.SRC, a.file)
    RULE = a.compiler or u5.COMPILER_BY_FILE.get(os.path.basename(path), 'msc51')
    src = open(path, encoding='utf-8', newline='').read()
    only = {f.upper() for f in a.functions} or None
    observed = observed_targets(os.path.basename(path)) if a.observed else None
    new, report = rewrite(src, only, observed)
    for name, ren, order in report:
        print('%-32s %s' % (name, ', '.join('%s -> %s' % kv for kv in ren.items()) or 'reorder only'))
    if a.apply and report:
        open(path, 'w', encoding='utf-8', newline='').write(new)
        print('%d functions changed in %s' % (len(report), path))
    elif not report:
        print('nothing to change')


if __name__ == '__main__':
    main()
