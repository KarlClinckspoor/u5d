#!/usr/bin/env python3
"""Q1, fourth table: additivity, string lengths, called functions, arrays and structs as
referenced symbols.   python research/rules/q1_add.py [msc51|msc50]
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from edge import edges, probe_src, name  # noqa: E402
from q1_costs import names, early_refs, K  # noqa: E402


def V():
    V = {}
    V['base'] = probe_src()
    n12 = names(12)
    n4s = names(4, p='s')
    ext12 = '\n'.join('extern int %s;' % n for n in n12)
    sta4 = '\n'.join('static int %s;' % n for n in n4s)
    strs = '\n'.join('    puts("string number %d");' % i for i in range(K))
    # three parts alone (each with the same early function) and together
    V['A_ext12'] = probe_src(prefix='int puts(char *);\n' + ext12, early='void early(void)\n{\n%s\n}\n' % '\n'.join('    %s = 0;' % n for n in n12))
    V['B_sta4'] = probe_src(prefix='int puts(char *);\n' + sta4, early='void early(void)\n{\n    a1 = 0;\n}\n')
    V['C_str'] = probe_src(prefix='int puts(char *);', early='void early(void)\n{\n%s\n}\n' % strs)
    V['ABC'] = probe_src(prefix='int puts(char *);\n' + ext12 + '\n' + sta4,
                         early='void early(void)\n{\n%s\n%s\n}\n' % ('\n'.join('    %s = 0;' % n for n in n12), strs))
    # string length: 50 literals of 1 / 60 characters
    V['str1'] = probe_src(prefix='int puts(char *);', early='void early(void)\n{\n%s\n}\n' % '\n'.join('    puts("%s");' % chr(65 + i % 26) for i in range(K)))
    V['str60'] = probe_src(prefix='int puts(char *);', early='void early(void)\n{\n%s\n}\n' % '\n'.join('    puts("%s%02d");' % ('x' * 58, i) for i in range(K)))
    # the same literal 50 times
    V['strSame'] = probe_src(prefix='int puts(char *);', early='void early(void)\n{\n%s\n}\n' % '\n'.join('    puts("same");' for i in range(K)))
    # referenced arrays and a struct type
    V['arr12'] = probe_src(prefix='\n'.join('extern int %s[10];' % n for n in n12), early='void early(void)\n{\n%s\n}\n' % '\n'.join('    %s[3] = 0;' % n for n in n12))
    V['st12'] = probe_src(prefix='struct S { int a, b, c; };\n' + '\n'.join('extern struct S %s;' % n for n in n12),
                          early='void early(void)\n{\n%s\n}\n' % '\n'.join('    %s.b = 0;' % n for n in n12))
    # 100 instead of 50 (linearity in the count)
    n12b = names(12, 100)
    V['ext12x100'] = probe_src(prefix='\n'.join('extern int %s;' % n for n in n12b), early='void early(void)\n{\n%s\n}\n' % '\n'.join('    %s = 0;' % n for n in n12b))
    return V


if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    edges(V(), comp, coarse=(0, 600, 10))
