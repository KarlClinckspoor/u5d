#!/usr/bin/env python3
"""Find the pool edge of a probe function in many test modules at once (research/RULES.md, Q1).

The probe function ends in two loops. Line records inserted after its entry (rk.insert_lines)
fill pass 2's pool; at some count n the pool's cut reaches the loops and the code changes.
The first such n ("the edge") is measured for each variant; what a variant adds to the module
shifts the edge by its cost, in line records (pool units).

    edges(variants, comp, func='PROBE', coarse=(lo, hi, step), paras=0) -> {name: edge or None}
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402


def name(k, length, prefix='x'):
    """A distinct identifier of exactly `length` characters (k < 10**(length-len(prefix)))."""
    w = length - len(prefix)
    s = '%s%0*d' % (prefix, w, k)
    assert len(s) == length, (k, length, prefix)
    return s


def probe_src(prefix='', early='', top='', locals_=('i', 'j'), param='n', stmts=20, extra_body=''):
    i, j = locals_
    body = '\n'.join('    a%d = %s + %d;' % (k % 10, param, k) for k in range(stmts))
    return '''int g[100];
int h[100];
int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
%(prefix)s
%(early)s
void probe(int %(param)s)
{
    int %(i)s, %(j)s;
%(top)s
%(body)s
%(extra)s
    for (%(i)s = 0; %(i)s < %(param)s; %(i)s++) { g[%(i)s] = h[%(i)s] + %(i)s; }
    for (%(j)s = 0; %(j)s < %(param)s; %(j)s++) { h[%(j)s] = g[%(j)s] * 3 + a1; }
}
''' % dict(prefix=prefix, early=early, top=top, i=i, j=j, param=param, body=body, extra=extra_body)


def mark_line(src, func, mark):
    if mark is None:
        return rk.brace_line(src, func.lower())
    lines = src.split(chr(10))
    return next(i + 1 for i, l in enumerate(lines) if mark in l)


def _scan(caps, srcs, comp, func, plan, paras, mark=None):
    """plan: {name: [n...]}. Returns {name: {n: sig}}."""
    jobs, keys = [], []
    for v, ns in plan.items():
        f = caps[v]
        line = mark_line(srcs[v], func, mark)
        for n in ns:
            j = dict(f, EX=rk.insert_lines(f['EX'], line, n))
            if paras:
                j['PARAS'] = paras
            jobs.append(j)
            keys.append((v, n))
    out = {v: {} for v in plan}
    # one session can take a long list; keep sessions under ~400 jobs
    for k in range(0, len(jobs), 400):
        objs = rk.passes(jobs[k:k + 400], comp)
        for (v, n), o in zip(keys[k:k + 400], objs):
            out[v][n] = rk.sig(rk.funcs(o).get(func))
    return out


def edges(srcs, comp='msc51', func='PROBE', coarse=(0, 600, 10), paras=0, verbose=True, caps=None, mark=None):
    caps = caps or rk.capture(srcs, comp)
    bad = [v for v in srcs if 'EX' not in caps.get(v, {})]
    for v in bad:
        print('NO PASS 1 OUTPUT', v, caps.get(v, {}).get('ERR', '')[-400:])
    srcs = {v: s for v, s in srcs.items() if v not in bad}
    lo, hi, st = coarse
    r1 = _scan(caps, srcs, comp, func, {v: list(range(lo, hi, st)) for v in srcs}, paras, mark)
    plan, res = {}, {}
    for v in srcs:
        s = r1[v]
        base = s[lo]
        e = next((n for n in sorted(s) if s[n] != base), None)
        res[v] = [base, e]
        if e is not None and e > lo:
            plan[v] = list(range(e - st + 1, e))
    r2 = _scan(caps, srcs, comp, func, plan, paras, mark) if plan else {}
    out = {}
    for v in srcs:
        base, e = res[v]
        if v in r2:
            e = next((n for n in sorted(r2[v]) if r2[v][n] != base), e)
        out[v] = e
        if verbose:
            print('%-10s base %-12s edge %s' % (v, base, e), flush=True)
    return out
