import sys, os, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk

def probe_src(prefix='', body_stmts=20):
    stmts = '\n'.join('    a%d = n + %d;' % (k % 10, k) for k in range(body_stmts))
    return '''int g[100];
int h[100];
int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
%s
void probe(int n)
{
    int i, j;
%s
    for (i = 0; i < n; i++) { g[i] = h[i] + i; }
    for (j = 0; j < n; j++) { h[j] = g[j] * 3 + a1; }
}
''' % (prefix, stmts)

if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    src = probe_src()
    t = time.time()
    cap = rk.capture({'P0': src}, comp)
    print('capture', time.time() - t, cap['_flags'])
    f = cap['P0']
    print(sorted(f), f.get('ERR', '')[:300])
    line = rk.brace_line(src, 'probe')
    ns = list(range(0, 1300, 10))
    t = time.time()
    objs = rk.passes([dict(f, EX=rk.insert_lines(f['EX'], line, n)) for n in ns], comp)
    print('passes', time.time() - t)
    prev = None
    for n, o in zip(ns, objs):
        s = rk.sig(rk.funcs(o).get('PROBE'))
        if s != prev:
            print(n, s)
            prev = s
