#!/usr/bin/env python3
"""Q1, first table: what each kind of declaration costs in pass 2's pool (MSC 5.0 / 5.1).

    python research/rules/q1_costs.py msc51|msc50 [GROUP ...]

Each variant adds K things of one kind to the probe module; the edge shift against `base`
is their cost in line records. Results: research/RULES.md.
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from edge import edges, probe_src, name  # noqa: E402

K = 50


def names(L, k=K, p='x'):
    return [name(i, L, p) for i in range(k)]


def early_refs(ns, kind='var', one_line=False):
    if kind == 'var':
        st = ['%s = 0;' % n for n in ns]
    else:
        st = ['%s();' % n for n in ns]
    if one_line:
        return 'void early(void)\n{\n    %s\n}\n' % ' '.join(st)
    return 'void early(void)\n{\n%s\n}\n' % '\n'.join('    ' + s for s in st)


def variants(groups):
    V = {}
    if 'basic' in groups:
        V['base'] = probe_src()
        V['early0'] = probe_src(early=early_refs(['a%d' % (i % 10) for i in range(K)]))
        V['early0L'] = probe_src(early=early_refs(['a%d' % (i % 10) for i in range(K)], one_line=True))
        for L in (4, 12, 28):
            ns = names(L)
            V['extD%d' % L] = probe_src(prefix='\n'.join('extern int %s;' % n for n in ns))
            V['extR%d' % L] = probe_src(prefix='\n'.join('extern int %s;' % n for n in ns), early=early_refs(ns))
        for L in (4, 28):
            ns = names(L)
            V['proD%d' % L] = probe_src(prefix='\n'.join('void %s(void);' % n for n in ns))
            V['proR%d' % L] = probe_src(prefix='\n'.join('void %s(void);' % n for n in ns), early=early_refs(ns, 'fn'))
            V['staD%d' % L] = probe_src(prefix='\n'.join('static int %s;' % n for n in ns))
            V['staR%d' % L] = probe_src(prefix='\n'.join('static int %s;' % n for n in ns), early=early_refs(ns))
            V['defD%d' % L] = probe_src(prefix='\n'.join('int %s;' % n for n in ns))
            V['iniD%d' % L] = probe_src(prefix='\n'.join('int %s = 1;' % n for n in ns))
            V['dedP%d' % L] = probe_src(prefix='\n'.join('extern int %s;' % n for n in ns),
                                        top='    if (0) { %s }' % ' '.join('%s = 0;' % n for n in ns))
        V['struct'] = probe_src(prefix='\n'.join('struct %s { int a; int b; };' % n for n in names(8, p='s')))
        V['typedef'] = probe_src(prefix='\n'.join('typedef struct { int a; int b; } %s;' % n for n in names(8, p='T')))
        V['macU'] = probe_src(prefix='\n'.join('#define %s 1' % n for n in names(8, 200, 'M')))
        V['macUsed'] = probe_src(prefix='\n'.join('#define %s 1' % n for n in names(8, K, 'M')),
                                 early='void early(void)\n{\n%s\n}\n' % '\n'.join(
                                     '    a1 = %s;' % n for n in names(8, K, 'M')))
        V['locL'] = probe_src(locals_=('i' + 'x' * 28, 'j' + 'x' * 28))
        V['parL'] = probe_src(param='n' + 'x' * 28)
    return V


if __name__ == '__main__':
    comp = sys.argv[1]
    groups = sys.argv[2:] or ['basic']
    V = variants(groups)
    edges(V, comp)
