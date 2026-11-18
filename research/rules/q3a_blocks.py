#!/usr/bin/env python3
"""Q3a: when does a block's stack slot go back for a later sibling block's register local (MSC 5.0/5.1)?

Block A declares `int cell` (in memory), block B after it declares `register int p` (in SI,
with a reserved slot under 5.0). Frame 4 = kind + one shared slot; 6 = no sharing.

    python research/rules/q3a_blocks.py [msc50|msc51]
"""
import os, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q3a_si import frame, PAT  # noqa: E402

HEAD = '''extern unsigned char tbl[8];
extern void DrawPiece(int, int, int);
extern int CellAt(int, int);
extern int g;
'''
A_BODY = 'int cell; cell = CellAt(x, y); kind = cell >> 4; if (cell & 7) DrawPiece(kind, 0, 0);'
B_BODY = 'register int p; p = tbl[kind]; p += dist * 2; DrawPiece(p, 1, 0); DrawPiece(p, 1, 1);'
A = {
    'bare':  '{ %s }',
    'if1':   'if (1) { %s }',
    'ifx':   'if (x) { %s }',
    'ifelse': 'if (x) { %s } else g = 1;',
    'else':  'if (x) g = 1; else { %s }',
    'for1':  'for (;;) { %s break; }',
    'fori':  'for (g = 0; g < x; g++) { %s }',
    'while': 'while (x) { %s break; }',
    'do':    'do { %s } while (0);',
    'switch': 'switch (x) { case 1: { %s } }',
    'case':  'switch (x) { case 1: { %s } break; }',
}
B = {
    'bare': '{ %s }',
    'if':   'if (y) { %s }',
}


def src(a, b):
    return HEAD + '''
void f(int x, int y, int dist)
{
    int kind;
    kind = 0;
    %s
    %s
}
''' % (A[a] % A_BODY, B[b] % B_BODY)


if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc50'
    combos = [(a, b) for a in A for b in B]
    S = {('B%02d' % i): src(a, b) for i, (a, b) in enumerate(combos)}
    caps = rk.capture(S, comp)
    for (k, _), (a, b) in zip(S.items(), combos):
        code = rk.funcs(caps[k].get('OBJ')).get('F')
        if code is None:
            print('%-8s %-5s no code %s' % (a, b, caps[k].get('ERR', '')[-200:]))
            continue
        lines = rk.dis(code)
        print('A %-8s B %-5s frame %d  two-step-SI %s' % (a, b, frame(lines), 'YES' if PAT.search('\n'.join(lines)) else 'no'))
