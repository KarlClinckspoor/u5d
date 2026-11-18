#!/usr/bin/env python3
"""Q2: predict COMDEF order from the declarations and compare with the compiler's.

Rule tested: every global symbol-table entry gets the next index k (from a start C0); a
communal's slot is (k mod 64); COMDEF records are written slot by slot, ascending, and within a
slot by the sum of the name's character codes, ascending (ties: declared first, first).

    python research/rules/q2_predict.py [msc51|msc50]
"""
import os, sys, random
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q2_comdef import comdef_order  # noqa: E402

C0 = 9


def predict(entries, case=str.upper):
    """entries: [(name, is_communal, n_entries)] in declaration order."""
    k = C0
    cs = []
    for i, (n, com, cost) in enumerate(entries):
        if com:
            cs.append(((k % 64), sum(map(ord, case(n))), i, n))
        k += cost
    return [c[3].upper() for c in sorted(cs)]


def rnd_name(r, i):
    s = r.choice('abcdefghijklmnopqrstuvwxyz') + ''.join(
        r.choice('abcdefghijklmnopqrstuvwxyz0123456789_') for _ in range(r.randint(1, 12)))
    return s


if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    r = random.Random(11)
    tests = {}
    for t in range(4):
        names = []
        seen = set()
        while len(names) < 150:
            n = rnd_name(r, len(names))
            if n.upper() not in seen and n not in ('do', 'if', 'for', 'int', 'char'):
                seen.add(n.upper()); names.append(n)
        tests['R%d' % t] = [(n, True, 1) for n in names]
    # mixed kinds, costs to fit
    names = ['m%03d' % i for i in range(150)]
    kinds = [r.choice(['int %s;', 'extern int %s;', 'void %s(void);', 'static int %s;', 'typedef int %s;', 'int %s = 1;'])
             for _ in names]
    tests['MIX'] = [(n, k == 'int %s;', 1) for n, k in zip(names, kinds)]
    S = {}
    for k, ents in tests.items():
        if k == 'MIX':
            S[k] = '\n'.join(kd % n for n, kd in zip(names, kinds)) + '\n'
        else:
            S[k] = '\n'.join('int %s;' % n for n, _, _ in ents) + '\n'
    # lower/upper case test: same letters in different case
    caps = rk.capture(S, comp)
    for k, ents in tests.items():
        got = comdef_order(caps[k]['OBJ'])
        for case in (str.upper, str.lower, lambda s: s):
            pred = predict(ents, case)
            print(k, case.__name__ if hasattr(case, '__name__') else 'asis', 'MATCH' if pred == got else
                  'differ at %d of %d' % (next(i for i in range(len(got)) if got[i] != pred[i]), len(got)))
