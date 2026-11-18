#!/usr/bin/env python3
"""Q2: does PLINK86 1.28 (1983) understand MSC's COMDEF records, and in what order does it lay out
communals? Links a test object (q2 test module) with PLINK86 and prints its map.

    python research/rules/q2_plink.py
"""
import os, sys, shutil
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q2_comdef import comdef_order  # noqa: E402

PL = os.path.join(rk.ROOT, '..', 'tools', 'plink86-1.28')

if __name__ == '__main__':
    names = ['zeta', 'alpha', 'mid', 'b2', 'qq', 'aa']
    src = '\n'.join('int %s[%d];' % (n, i + 1) for i, n in enumerate(names)) + \
        '\nint first(void)\n{\n    return zeta[0] + alpha[0] + mid[0] + b2[0] + qq[0] + aa[0];\n}\n'
    caps = rk.capture({'PT1': src}, 'msc51')
    obj = caps['PT1']['OBJ']
    print('COMDEF order:', comdef_order(obj))
    d = rk.newdir('plink')
    for fn in os.listdir(PL):
        if fn.upper().endswith('.EXE'):
            shutil.copy(os.path.join(PL, fn), d)
    open(os.path.join(d, 'PT1.OBJ'), 'wb').write(obj)
    open(os.path.join(d, 'T.LNK'), 'w', newline='').write('out pt1.exe\r\nfil pt1\r\nmap\r\n')
    rk.dosbox(d, 'msc51', ['PLINK86 @T.LNK > OUT.TXT'], timeout=120)
    for fn in os.listdir(d):
        if fn.upper().endswith(('.TXT', '.MAP')):
            print('==', fn)
            print(open(os.path.join(d, fn), encoding='latin-1').read()[:3000])
