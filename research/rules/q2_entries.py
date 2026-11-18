#!/usr/bin/env python3
"""Q2: how many symbol-table entries (slot steps) does each kind of declaration take?

Puts X before 64 communals v000..v063; the first one written is v(64 - c) where c is the
slot of v000: c = 9 + entries(X).   python research/rules/q2_entries.py [msc51|msc50]
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q2_comdef import comdef_order  # noqa: E402

X = {
    'none': '',
    'ext1': 'extern int e1;',
    'fdef0': 'int f1(void) { return 0; }',
    'fdef_l2': 'int f1(void) { int a, b; a = 1; b = a; return b; }',
    'fdef_p2': 'int f1(int a, int b) { return a + b; }',
    'fdef_s': 'char *f1(void) { return "abc"; }',
    'fdef_x': 'extern int e1; int f1(void) { return e1; }',
    'fdef_loc_ext': 'int f1(void) { extern int e1; return e1; }',
    'proto_p2': 'void f1(int a, int b);',
    'proto_p2u': 'void f1(int, int);',
    'tag3': 'struct s1 { int a; int b; int c; };',
    'tag3b': 'struct s1 { int a; int b; int c; }; struct s2 { int a; int b; int c; };',
    'tdef3': 'typedef struct { int a; int b; int c; } T1;',
    'enum3': 'enum e1 { E1, E2, E3 };',
    'sfunc': 'static int f1(void) { return 0; }',
    'define': '#define D1 1',
    'tdefint': 'typedef int T1;',
    'arr': 'extern int e1[10];',
    'use_v': 'int f1(void) { return 0; }\nint f2(void) { return 0; }',
}

if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    S = {('X%02d' % i): v + '\n' + '\n'.join('int v%03d;' % k for k in range(64)) + '\n' for i, v in enumerate(X.values())}
    caps = rk.capture(S, comp)
    for (k, src), name in zip(S.items(), X):
        o = comdef_order(caps[k]['OBJ'])
        f = int(o[0][1:])
        c = (64 - f) % 64
        print('%-14s first v%03d  slot(v000)=%2d  entries=%d' % (name, f, c, (c - 9) % 64))
