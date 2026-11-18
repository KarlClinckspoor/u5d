#!/usr/bin/env python3
"""Q3b: FONT_0000 in isolation, n line records inserted after its entry (pool cut scan), and
limited memory for C2: do the tail CSE (SI vs temp+BX) and loop B's add order follow?

    python research/rules/q3b_font0_pool.py lines FROM TO STEP | mem FROM TO STEP
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q3b_font0 import HEAD, function_text, original, diff  # noqa: E402
from q3b_cse import SI, BX  # noqa: E402
from q3b_font import ORIG, OURS  # noqa: E402

if __name__ == '__main__':
    mode = sys.argv[1]
    lo, hi, st = map(int, sys.argv[2:5])
    src = HEAD + function_text()
    caps = rk.capture({'FONT0': src}, 'msc50')
    f = caps['FONT0']
    line = rk.brace_line(src, 'FONT_0000_DrawJustifiedText')
    ns = list(range(lo, hi, st))
    if mode == 'lines':
        jobs = [dict(f, EX=rk.insert_lines(f['EX'], line, n)) for n in ns]
    else:
        jobs = [dict(f, PARAS=n) for n in ns]
    objs = []
    for k in range(0, len(jobs), 300):
        objs += rk.passes(jobs[k:k + 300], 'msc50')
    orig = original()
    prev = None
    for n, o in zip(ns, objs):
        code = rk.funcs(o).get('FONT_0000_DRAWJUSTIFIEDTEXT')
        if code is None:
            line_ = 'no code'
        else:
            t = '\n'.join(rk.dis(code))
            i = t.find('add word ptr ds:[0],9')
            tail = t[i:] if i >= 0 else t
            line_ = 'len %d diff %d tail %s loopB %s' % (len(code), diff(code, orig),
                                                     'TEMP+BX' if BX.search(tail) else 'SI' if SI.search(tail) else '?',
                                                     'TEMP+REG' if ORIG.search(t) else 'REG+TEMP' if OURS.search(t) else '?')
        if line_ != prev:
            print('%6d %s' % (n, line_), flush=True)
            prev = line_
