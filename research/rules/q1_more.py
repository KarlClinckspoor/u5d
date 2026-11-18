#!/usr/bin/env python3
"""Q1, second table: late references, dead references, dead lines, unused locals (MSC 5.1 by default).

    python research/rules/q1_more.py [msc51|msc50]
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from edge import edges, probe_src, name  # noqa: E402
from q1_costs import names, early_refs, K  # noqa: E402

def V():
    V = {}
    V['base'] = probe_src()
    ns4, ns28 = names(4), names(28)
    ext4 = '\n'.join('extern int %s;' % n for n in ns4)
    ext28 = '\n'.join('extern int %s;' % n for n in ns28)
    # references in a function after the probe
    late = lambda ns: '\nvoid late(void)\n{\n%s\n}\n' % '\n'.join('    %s = 0;' % n for n in ns)
    V['late4'] = probe_src(prefix=ext4) + late(ns4)
    V['late28'] = probe_src(prefix=ext28) + late(ns28)
    V['late0'] = probe_src() + late(['a%d' % (i % 10) for i in range(K)])
    # dead references, one line, in the early function
    V['dedE4'] = probe_src(prefix=ext4, early='void early(void)\n{\n    if (0) { %s }\n}\n' % ' '.join('%s = 0;' % n for n in ns4))
    V['dedE28'] = probe_src(prefix=ext28, early='void early(void)\n{\n    if (0) { %s }\n}\n' % ' '.join('%s = 0;' % n for n in ns28))
    # dead references in the probe, one line / one per line
    V['dedP4'] = probe_src(prefix=ext4, top='    if (0) { %s }' % ' '.join('%s = 0;' % n for n in ns4))
    V['dedP28'] = probe_src(prefix=ext28, top='    if (0) { %s }' % ' '.join('%s = 0;' % n for n in ns28))
    V['dedP4L'] = probe_src(prefix=ext4, top='    if (0) {\n%s\n    }' % '\n'.join('        %s = 0;' % n for n in ns4))
    # dead lines naming nothing new (existing globals), in the probe: line cost of dead code
    V['dedLines'] = probe_src(top='    if (0) {\n%s\n    }' % '\n'.join('        a%d = 0;' % (i % 10) for i in range(K)))
    V['dedOne'] = probe_src(top='    if (0) { %s }' % ' '.join('a%d = 0;' % (i % 10) for i in range(K)))
    # live references in the probe itself (one line), new externs
    V['liveP4'] = probe_src(prefix=ext4, top='    if (n == 12345) { %s }' % ' '.join('%s = 0;' % n for n in ns4))
    # unused locals in the probe: 10 of 2 chars, 10 of 28 chars
    V['loc10s'] = probe_src(top='    int %s;' % ', '.join(name(i, 2, 'q') for i in range(10)))
    V['loc10L'] = probe_src(top='    int %s;' % ', '.join(name(i, 28, 'q') for i in range(10)))
    # comment lines outside / inside an earlier function
    V['cmtOut'] = probe_src(prefix='\n'.join('/* c %d */' % i for i in range(K)))
    V['cmtEarly'] = probe_src(early='void early(void)\n{\n%s\n    a1 = 0;\n}\n' % '\n'.join('    /* c %d */' % i for i in range(K)))
    # string literals in an earlier function / in the probe
    V['strE'] = probe_src(early='void early(void)\n{\n%s\n}\n' % '\n'.join('    puts("string number %d");' % i for i in range(K)),
                          prefix='int puts(char *);')
    V['strP'] = probe_src(prefix='int puts(char *);', top='    if (n == 12345) { %s }' % ' '.join('puts("s%d");' % i for i in range(20)))
    V['strP0'] = probe_src(prefix='int puts(char *);', top='    if (n == 12345) { %s }' % ' '.join('puts((char *)%d);' % i for i in range(20)))
    return V

if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    edges(V(), comp, coarse=(0, 900, 10))
