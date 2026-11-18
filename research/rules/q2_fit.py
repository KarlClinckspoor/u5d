"""Q2: which key orders the entries that share a slot? Brute force over candidate hashes."""
import os, sys, random, pickle
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk
from q2_comdef import comdef_order
from q2_predict import rnd_name

def gen(seed, n=150):
    r = random.Random(seed)
    names = []; seen = set()
    while len(names) < n:
        x = rnd_name(r, 0)
        if x.upper() not in seen and x not in ('do', 'if', 'for', 'int', 'char'):
            seen.add(x.upper()); names.append(x)
    return names

cache = os.path.join(rk.WORK, 'q2fit.pkl')
if os.path.exists(cache):
    data = pickle.load(open(cache, 'rb'))
else:
    tests = {'R%d' % s: gen(s) for s in range(20, 28)}
    caps = rk.capture({k: '\n'.join('int %s;' % n for n in v) + '\n' for k, v in tests.items()}, 'msc51')
    data = {k: (tests[k], comdef_order(caps[k]['OBJ'])) for k in tests}
    pickle.dump(data, open(cache, 'wb'))

groups = []   # list of lists of (index, name) in output order, per slot
for k, (names, got) in data.items():
    idx = {n.upper(): i for i, n in enumerate(names)}
    by = {}
    for g in got:
        i = idx[g]
        by.setdefault((i + 9) % 64, []).append((i, names[i]))
    groups += [v for v in by.values() if len(v) > 1]
print(len(groups), 'groups')

def hashes():
    H = {}
    for case in ('U', 'L'):
        f = (lambda s: s.upper()) if case == 'U' else (lambda s: s)
        H['sum' + case] = lambda s, f=f: sum(map(ord, f(s)))
        H['xor' + case] = lambda s, f=f: __import__('functools').reduce(lambda a, b: a ^ b, map(ord, f(s)), 0)
        for m in (2, 3, 4, 5, 7, 31, 33, 37):
            def hm(s, f=f, m=m):
                h = 0
                for c in f(s):
                    h = (h * m + ord(c)) & 0xffff
                return h
            H['mul%d%s' % (m, case)] = hm
        def shl(s, f=f):
            h = 0
            for c in f(s):
                h = ((h << 1) ^ ord(c)) & 0xffff
            return h
        H['shlx' + case] = shl
        H['len'] = len
        H['first' + case] = lambda s, f=f: ord(f(s)[0])
        H['last' + case] = lambda s, f=f: ord(f(s)[-1])
        H['name' + case] = lambda s, f=f: f(s)
    return H

for hn, h in hashes().items():
    for mod in (None, 16, 32, 64, 128, 256, 1024):
        for sign in (1, -1):
            ok = 0
            for g in groups:
                keys = [h(n) if mod is None or isinstance(h(n), str) else h(n) % mod for _, n in g]
                if isinstance(keys[0], str) and sign == -1:
                    keys2 = keys[::-1]; good = keys2 == sorted(keys2)
                else:
                    good = all((keys[a] <= keys[a + 1]) if sign == 1 else (keys[a] >= keys[a + 1]) for a in range(len(keys) - 1))
                ok += good
            if ok >= len(groups) * 0.8:
                print(hn, mod, sign, ok, '/', len(groups))

if __name__ == '__main__' and len(sys.argv) > 1:
    for g in groups[:40]:
        print(' | '.join('%3d %-14s s%4d l%2d' % (i, n, sum(map(ord, n.upper())), len(n)) for i, n in g))
