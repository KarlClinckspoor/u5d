#!/usr/bin/env python3
"""Q2: test 'COMDEF order = walk of a 64-slot table, entry k in slot (k + c) mod 64'.

    python research/rules/q2_slots.py [msc51|msc50]
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q2_comdef import comdef_order  # noqa: E402
from q2_blocks import module  # noqa: E402


def analyse(order, c=9):
    idx = [int(n[1:]) for n in order]
    groups = {}
    for pos, k in enumerate(idx):
        groups.setdefault((k + c) % 64, []).append(k)
    # is the output the slots in ascending order?
    slots = [(k + c) % 64 for k in idx]
    viol = [(idx[i], slots[i], idx[i + 1], slots[i + 1]) for i in range(len(idx) - 1) if slots[i + 1] < slots[i]]
    odd = {s: g for s, g in groups.items() if g != sorted(g)}
    return viol, odd


if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    S = {}
    for n in (64, 65, 100, 110, 112, 113, 114, 116, 118, 128, 140, 160, 192, 256, 300):
        S['N%d' % n] = module(n)
    caps = rk.capture(S, comp)
    for k in S:
        o = comdef_order(caps[k]['OBJ'])
        viol, odd = analyse(o)
        print('%-6s slot-order violations %s | slots not oldest-first: %s' % (k, viol[:6], dict(list(odd.items())[:8])))
