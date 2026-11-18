#!/usr/bin/env python3
"""Q1: bytes per pool unit. The probe's edge with N paragraphs left free for C2; where the heap is
limited by DOS memory, the edge moves by (16 / bytes per unit) per paragraph.
    python research/rules/q1_mem.py msc51|msc50 P1 P2 ...
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from edge import edges, probe_src  # noqa: E402

if __name__ == '__main__':
    comp = sys.argv[1]
    src = {'base': probe_src()}
    caps = rk.capture(src, comp)
    for p in map(int, sys.argv[2:]):
        e = edges(src, comp, paras=p, caps=caps, verbose=False, coarse=(0, 700, 10))
        print('paras %5d (%6.2f KB): edge %s' % (p, p / 64.0, e['base']), flush=True)
