#!/usr/bin/env python3
"""Q2: in what order do MSC 5.0 / 5.1 write COMDEF (communal) records?

    python research/rules/q2_comdef.py [msc51|msc50]

Prints, for each test module, the declaration order and the COMDEF order.
"""
import os, sys, random
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
import omf  # noqa: E402


def comdef_order(obj):
    o = omf.Object('x', obj)
    return list(o.comdefs)


def ext_order(obj):
    o = omf.Object('x', obj)
    return [e for e in o.exts[1:]]


def tests():
    T = {}
    a = ['v%02d' % i for i in range(40)]
    T['FWD'] = a
    T['REV'] = a[::-1]
    rnd = random.Random(5)
    b = ['%s%d' % (''.join(rnd.choice('abcdefghijklmnopqrstuvwxyz') for _ in range(rnd.randint(2, 9))), i)
         for i in range(60)]
    T['RND'] = b
    T['RNDR'] = b[::-1]
    T['RNDS'] = sorted(b)
    return T


def src(names, use=None):
    s = '\n'.join('int %s;' % n for n in names) + '\n'
    if use:
        s += 'void f(void)\n{\n%s\n}\n' % '\n'.join('    %s = 1;' % n for n in use)
    return s


if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    T = tests()
    S = {k: src(v) for k, v in T.items()}
    S['RNDU'] = src(T['RND'], use=T['RND'][::-1])
    T['RNDU'] = T['RND']
    caps = rk.capture(S, comp)
    for k in S:
        if 'OBJ' not in caps[k]:
            print(k, 'no obj', caps[k].get('ERR'))
            continue
        cd = comdef_order(caps[k]['OBJ'])
        print('==', k)
        print(' decl  :', ' '.join(T[k]))
        print(' comdef:', ' '.join(cd))
