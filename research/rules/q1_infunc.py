#!/usr/bin/env python3
"""Q1, third table: things inside the probe function itself, before the insertion point
(records inserted after the first statement of the body).   python research/rules/q1_infunc.py [msc51|msc50]
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from edge import edges, probe_src, name  # noqa: E402
from q1_costs import names, K  # noqa: E402

MARK = 'a1 = n + 1;'

def V():
    V = {}
    V['base'] = probe_src()
    ns4, ns28 = names(4), names(28)
    ext4 = '\n'.join('extern int %s;' % n for n in ns4)
    ext28 = '\n'.join('extern int %s;' % n for n in ns28)
    V['cmt50'] = probe_src(top='\n'.join('    /* c */' for i in range(K)))
    V['dedLines'] = probe_src(top='    if (0) {\n%s\n    }' % '\n'.join('        a%d = 0;' % (i % 10) for i in range(K)))
    V['dedOne'] = probe_src(top='    if (0) { %s }' % ' '.join('a%d = 0;' % (i % 10) for i in range(K)))
    V['dedP4'] = probe_src(prefix=ext4, top='    if (0) { %s }' % ' '.join('%s = 0;' % n for n in ns4))
    V['dedP28'] = probe_src(prefix=ext28, top='    if (0) { %s }' % ' '.join('%s = 0;' % n for n in ns28))
    V['dedP4L'] = probe_src(prefix=ext4, top='    if (0) {\n%s\n    }' % '\n'.join('        %s = 0;' % n for n in ns4))
    V['live50'] = probe_src(top='\n'.join('    a%d = n;' % (i % 10) for i in range(K)))
    V['live50one'] = probe_src(top='    %s' % ' '.join('a%d = n;' % (i % 10) for i in range(K)))
    V['ifdef50'] = probe_src(top='#if 0\n%s\n#endif' % '\n'.join('    a1 = 0;' for i in range(K)))
    return V

if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    edges(V(), comp, coarse=(0, 700, 10), mark=MARK)
