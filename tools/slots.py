#!/usr/bin/env python3
"""Microsoft C 5.1's stack-slot order, and renames that give the original's.

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
"""
import argparse, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402

BUCKETS = 16
SUFFIX_CHARS = 'abcdefghijklmnopqrstuvwxyz0123456789'
# a renamed local is local_N_<suffix>: the underscore keeps N readable (tools and people)


def bucket(name):
    return sum(map(ord, name)) % BUCKETS


def alloc_order(names):
    """names in declaration order -> the order MSC 5.1 gives them slots, nearest BP first."""
    idx = {n: i for i, n in enumerate(names)}
    return sorted(names, key=lambda n: (bucket(n), -idx[n]))


TYPE = (r'(?:(?:unsigned|signed|register|static|const|far|near)\s+)*'
        r'(?:int|char|byte|u8|s8|u16|s16|u32|s32|uint|word|bool|long|short|void|S_\w+|Pair\w*|struct\s+\w+)'
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


def plan(names):
    """names in declaration order -> (renames {old: new}, new declaration order) or None
    when nothing needs to change."""
    want = target_order(names)
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
        for c in SUFFIX_CHARS:
            for c2 in [''] + list(SUFFIX_CHARS):
                x = n + '_' + c + c2
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


def rewrite(src, only=None):
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
        p = plan(names)
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
    ap.add_argument('--apply', action='store_true')
    a = ap.parse_args()
    if a.hash:
        for n in a.hash:
            print('%-24s bucket %d' % (n, bucket(n)))
        return
    if not a.file:
        ap.error('give a file or --hash')
    path = a.file if os.path.exists(a.file) else os.path.join(u5.SRC, a.file)
    src = open(path, encoding='utf-8', newline='').read()
    only = {f.upper() for f in a.functions} or None
    new, report = rewrite(src, only)
    for name, ren, order in report:
        print('%-32s %s' % (name, ', '.join('%s -> %s' % kv for kv in ren.items()) or 'reorder only'))
    if a.apply and report:
        open(path, 'w', encoding='utf-8', newline='').write(new)
        print('%d functions changed in %s' % (len(report), path))
    elif not report:
        print('nothing to change')


if __name__ == '__main__':
    main()
