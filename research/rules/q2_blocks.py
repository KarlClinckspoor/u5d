#!/usr/bin/env python3
"""Q2: COMDEF order = blocks of symbol-table entries? Vary the count and what else is declared.

    python research/rules/q2_blocks.py [msc51|msc50]
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q2_comdef import comdef_order  # noqa: E402


def runs(order):
    """the COMDEF order as runs of consecutive declaration indexes: '55-59 0-54'"""
    idx = [int(n[1:]) for n in order]
    out, s = [], 0
    for i in range(1, len(idx) + 1):
        if i == len(idx) or idx[i] != idx[i - 1] + 1:
            out.append('%d-%d' % (idx[s], idx[i - 1]) if i - 1 > s else str(idx[s]))
            s = i
    return ' '.join(out)


def module(n, other=None, every=1, before=0, other_before=None):
    lines = []
    if other_before:
        lines += [other_before % ('y%03d' % k) for k in range(before)]
    for k in range(n):
        lines.append('int v%03d;' % k)
        if other and k % every == every - 1:
            lines.append(other % ('w%03d' % k))
    return '\n'.join(lines) + '\n'


if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    S = {}
    for n in (50, 54, 55, 56, 60, 64, 100, 118, 119, 120, 130, 200):
        S['N%d' % n] = module(n)
    S['EXT'] = module(60, 'extern int %s;')
    S['PROTO'] = module(60, 'void %s(void);')
    S['STAT'] = module(60, 'static int %s;')
    S['INIT'] = module(60, 'int %s = 1;')
    S['TYPEDEF'] = module(60, 'typedef int %s;')
    S['TAG'] = module(60, 'struct %s { int a; };')
    S['DEFINE'] = module(60, '#define %s 1')
    S['EXTB10'] = module(60, before=10, other_before='extern int %s;')
    S['FUNC'] = module(60, 'int %s(void) { return 0; }', every=10)
    caps = rk.capture(S, comp)
    for k in S:
        if 'OBJ' not in caps[k]:
            print(k, 'no obj', caps[k].get('ERR'))
            continue
        print('%-8s %s' % (k, runs(comdef_order(caps[k]['OBJ']))))
