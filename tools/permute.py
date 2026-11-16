#!/usr/bin/env python3
"""Search for a source form of one function that compiles to the original's bytes.

    python tools/permute.py FILE.c FUNCTION [--rounds N] [--batch N] [--apply]

MSC 5.1's register allocation and code layout depend on things that do not change what the
code means: the names of locals (they are hashed, see docs/MATCHING.md), declaration order,
`register`, and the order of operands in comparisons. When a function is close but will
not match, this tries such variants automatically:

  - rename a local into another hash bucket (local_6 -> local_6_k);
  - add or remove `register` on a local;
  - swap two declarations;
  - swap the operands of a comparison (a == b -> b == a, a < b -> b > a).

Each round compiles a batch of variants (each a full copy of the module with one or more
changes to the function) in one DOSBox-X session, scores them against the original (verdict,
length difference, differing bytes) and continues from the best. Every variant keeps the
meaning of the code; the result is still checked by match.py and nativecheck.py as usual.
With --apply, the best variant found replaces the function in src/ (only if it is better).
"""
import argparse, os, random, re, shutil, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402
import slots  # noqa: E402
import dosbuild  # noqa: E402
import match  # noqa: E402

RANK = {'EXACT': 0, 'FIXUP': 1, 'SLOTS': 2, 'CODE': 3, 'SIZE': 4}
OPERAND = r'[A-Za-z_][\w]*(?:\[[^\[\]]+\])?(?:(?:\.|->)\w+)*|0x[0-9a-fA-F]+|\d+'
CMP = re.compile(r'(?<![\w\]\)])(%s)\s*(==|!=|<=|>=|<|>)\s*(%s)(?![\w\[\(])' % (OPERAND, OPERAND))
FLIP = {'==': '==', '!=': '!=', '<': '>', '>': '<', '<=': '>=', '>=': '<='}


def find(src, func):
    for name, a, b in slots.functions(src):
        if name.upper() == func.upper():
            return a, b
    raise SystemExit('no function %s' % func)


def mutate(body, rng):
    """One meaning-preserving change to a function body (the text inside its braces)."""
    lines, decls = slots.declarations(body)
    kinds = []
    if decls:
        kinds += ['rename', 'register']
    if len(decls) > 1:
        kinds.append('swap')
    if CMP.search(body[sum(len(l) + 1 for l in lines[:decls[-1][0] + 1]) if decls else 0:]):
        kinds.append('cmp')
    if not kinds:
        return body
    k = rng.choice(kinds)
    if k == 'rename':
        i, name, line = rng.choice(decls)
        base = name.split('_x')[0]
        new = '%s_x%s' % (base, rng.choice('abcdefghijklmnopqrstuvwxyz'))
        return re.sub(r'\b%s\b' % re.escape(name), new, body)
    if k == 'register':
        i, name, line = rng.choice(decls)
        if 'register ' in line:
            lines[i] = line.replace('register ', '', 1)
        elif '[' not in line:
            lines[i] = re.sub(r'^(\s*)', r'\1register ', line, count=1)
        return '\n'.join(lines)
    if k == 'swap':
        (i, _, a), (j, _, b) = rng.sample(decls, 2)
        lines[i], lines[j] = b, a
        return '\n'.join(lines)
    start = sum(len(l) + 1 for l in lines[:decls[-1][0] + 1]) if decls else 0
    ms = list(CMP.finditer(body, start))
    m = rng.choice(ms)
    return body[:m.start()] + '%s %s %s' % (m.group(3), FLIP[m.group(2)], m.group(1)) + body[m.end():]


def score(cfile, func, objpath, gamedata):
    stem = os.path.splitext(cfile)[0].upper()
    dst = os.path.join(u5.BUILD, 'obj', stem + '.OBJ')
    shutil.copy(objpath, dst)
    mod = match.Module(cfile, gamedata)
    for r in match.compare(mod, match.runtime_symbols()):
        if r['func'].upper() == func.upper():
            # exact first, then the length difference, then the differing bytes
            return (0 if r['verdict'] == 'EXACT' else 1, abs(r['ours'] - r['orig']), r['diffs'] + r['bad_fixups'])
    return (9, 9999, 9999)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('file')
    ap.add_argument('function')
    ap.add_argument('--rounds', type=int, default=8)
    ap.add_argument('--batch', type=int, default=16)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--apply', action='store_true')
    a = ap.parse_args()
    cfile = os.path.basename(a.file)
    gd = u5.find_gamedata()
    rng = random.Random(a.seed)
    path = os.path.join(u5.SRC, cfile)
    src = open(path, encoding='utf-8', newline='').read()
    fa, fb = find(src, a.function)
    best_body = src[fa:fb]
    tmp = tempfile.mkdtemp(prefix='perm', dir=u5.BUILD)
    try:
        shutil.copytree(u5.SRC, os.path.join(tmp, 'src'))
        objsave = os.path.join(tmp, 'obj')
        os.makedirs(objsave)

        def run(bodies):
            """Compile each body as cfile, in one session per body (the module name must stay)."""
            out = []
            for k, body in enumerate(bodies):
                s = src[:fa] + body + src[fb:]
                open(os.path.join(tmp, 'src', cfile), 'w', encoding='utf-8', newline='').write(s)
                res = dosbuild.compile_files([cfile], src=os.path.join(tmp, 'src'), quiet=True)
                if res.get(cfile):
                    out.append((9, 9999, 9999))
                    continue
                o = os.path.join(objsave, '%d.OBJ' % k)
                shutil.copy(os.path.join(u5.BUILD, 'obj', os.path.splitext(cfile)[0].upper() + '.OBJ'), o)
                out.append(score(cfile, a.function, o, gd))
            return out
        best = run([best_body])[0]
        start = best
        print('start: %s' % (best,))
        for r in range(a.rounds):
            cands = []
            for _ in range(a.batch):
                b = best_body
                for _ in range(rng.choice((1, 1, 2, 3))):
                    b = mutate(b, rng)
                if b != best_body:
                    cands.append(b)
            scores = run(cands)
            for b, sc in zip(cands, scores):
                if sc < best:
                    best, best_body = sc, b
            print('round %d: best %s' % (r + 1, best))
            if best[0] == 0:
                break
        if best < start and a.apply:
            open(path, 'w', encoding='utf-8', newline='').write(src[:fa] + best_body + src[fb:])
            print('applied the best variant to %s' % path)
        elif best < start:
            print('better variant found (rerun with --apply to write it)')
        # leave the module's object as the source in src/ compiles
        dosbuild.compile_files([cfile], quiet=True)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == '__main__':
    main()
