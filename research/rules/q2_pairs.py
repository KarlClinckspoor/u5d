#!/usr/bin/env python3
"""Q2: two communals in one slot (declared 64 entries apart): which is written first?

    python research/rules/q2_pairs.py [msc51|msc50]
"""
import os, sys, itertools
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q2_comdef import comdef_order  # noqa: E402

def mk(first, second):
    n = len(first)
    assert len(second) == n and n <= 61
    lines = ['int %s;' % a for a in first] + ['extern int zz%d;' % i for i in range(64 - n)] + ['int %s;' % b for b in second]
    return '\n'.join(lines) + '\n'

def word(s):
    return s

def name_with_sum(L, target, k):
    """a lower-case name of length L with character sum target, distinct by k"""
    base = [ord('a')] * L
    base[0] = ord('a') + (k % 26)
    rest = target - sum(base)
    i = L - 1
    while rest > 0 and i > 0:
        add = min(rest, ord('z') - base[i])
        base[i] += add; rest -= add; i -= 1
    s = ''.join(map(chr, base))
    return s if sum(base) == target else None

if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    T = {}
    perms = [''.join(p) for p in itertools.permutations('kmpq')][:24] + [''.join(p) for p in itertools.permutations('lmpp')][:12]
    perms = list(dict.fromkeys(perms))[:30]   # A: length 4, sum 'k'+'m'+'p'+'q' = 107+109+112+113 = 441
    B1 = []
    for k, s in enumerate(range(420, 465, 1)):
        nm = name_with_sum(4, s, k + 3)
        if nm and nm not in perms and nm not in B1:
            B1.append(nm)
    B1 = B1[:len(perms)]
    T['SAMELEN'] = (perms[:len(B1)], B1)
    T['SAMELENR'] = (B1, perms[:len(B1)])
    # length effect: B of lengths 2..12 with sums around A's
    B2, A2 = [], []
    k = 0
    for L in range(2, 13):
        for s in (L * 97 + 5, 441, L * 110):
            nm = name_with_sum(L, s, k)
            k += 1
            if nm and nm not in B2:
                B2.append(nm); A2.append(perms[len(A2) % len(perms)] + 'x' * 0)
    A2 = ['%s' % p for p in (perms * 3)[:len(B2)]]
    A2 = [a if A2.count(a) == 1 else None for a in A2]
    A2 = [(a or 'kmpq') for a in A2]
    # make A2 distinct
    seen = set(); A2d = []
    for i, a in enumerate(A2):
        while a in seen or a in B2:
            a = a[1:] + a[0] if len(set(a)) > 1 else a
            a = a if a not in seen else chr(ord('a') + i % 26) + a[1:]
        seen.add(a); A2d.append(a)
    T['LEN'] = (A2d[:61], B2[:61])
    S = {k: mk(*v) for k, v in T.items()}
    caps = rk.capture(S, comp)
    for k, (first, second) in T.items():
        got = comdef_order(caps[k]['OBJ'])
        pos = {n: i for i, n in enumerate(got)}
        print('==', k)
        for a, b in zip(first, second):
            pa, pb = pos[a.upper()], pos[b.upper()]
            print('  %-14s s%4d l%2d  %s  %-14s s%4d l%2d' % (a, sum(map(ord, a)), len(a), '<' if pa < pb else '>', b,
                                                           sum(map(ord, b)), len(b)))
