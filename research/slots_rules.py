import json, os, itertools
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
data = json.load(open(os.path.join(ROOT, 'build', 'slots.json')))
def actual(d):  # nearest BP first
    return [n for n, _ in sorted(d['slots'].items(), key=lambda kv: -kv[1])]
def decl(d): return [x[0] for x in d['decls']]
rules = {
 'decl': lambda d: decl(d),
 'rev decl': lambda d: decl(d)[::-1],
 'first use': lambda d: sorted(decl(d), key=lambda n: d['first'][n]),
 'rev first use': lambda d: sorted(decl(d), key=lambda n: -d['first'][n]),
 'uses desc, decl': lambda d: sorted(decl(d), key=lambda n: -d['uses'][n]),
 'uses asc, decl': lambda d: sorted(decl(d), key=lambda n: d['uses'][n]),
}
def hashrule(M, f, tie):
    def r(d):
        ds = decl(d); idx = {n: i for i, n in enumerate(ds)}
        return sorted(ds, key=lambda n: (f(n) % M, tie * idx[n]))
    return r
H = {'sum': lambda n: sum(map(ord, n)), 'sumU': lambda n: sum(map(ord, n.upper())),
     'h31': lambda n: __import__('functools').reduce(lambda h, c: (h * 31 + ord(c)) & 0xffff, n, 0),
     'h2': lambda n: __import__('functools').reduce(lambda h, c: ((h << 1) + ord(c)) & 0xffff, n, 0),
     'xor': lambda n: __import__('functools').reduce(lambda h, c: h ^ ord(c), n, 0)}
for hn, hf in H.items():
    for M in range(2, 260):
        for tie in (1, -1):
            for rev in (False, True):
                base = hashrule(M, hf, tie)
                rules['%s%%%d tie%+d%s' % (hn, M, tie, ' rev' if rev else '')] = (lambda b: (lambda d: b(d)[::-1]))(base) if rev else base
res = []
for name, r in rules.items():
    ok = sum(1 for d in data if r(d) == actual(d))
    res.append((ok, name))
res.sort(reverse=True)
print(len(data), 'functions')
for ok, name in res[:15]: print('%4d  %s' % (ok, name))
for n in ('decl','rev decl','first use','rev first use','uses desc, decl'): print(n, sum(1 for d in data if rules[n](d)==actual(d)))
