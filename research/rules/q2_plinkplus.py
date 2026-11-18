#!/usr/bin/env python3
"""Q2 with Ultima V's own linker, PLINK86plus 2.24 (../tools/plink86plus-2.24, Nantucket edition):
link MSC objects with communals and read where each communal lands (the map).

    python research/rules/q2_plinkplus.py [msc51|msc50]

Objects are tagged with the COMENT class 0xC8 record (00 5e a1 07) the Nantucket edition wants.
"""
import os, re, shutil, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q2_comdef import comdef_order  # noqa: E402

PL = os.path.join(rk.ROOT, '..', 'tools', 'plink86plus-2.24')
TAG = bytes.fromhex('880700c0c8005ea107e3')


def tag(obj):
    n = obj[1] | obj[2] << 8
    return obj[:3 + n] + TAG + obj[3 + n:]


def objects():
    # A: 70 communals, mixed sizes, so that slots collide (serials 64 apart) and alignment shows
    a = ['a%02d' % i for i in range(70)]
    decl_a = []
    for i, n in enumerate(a):
        decl_a.append(('char %s;' if i % 3 == 0 else 'int %s;' if i % 3 == 1 else 'char %s[3];') % n)
    A = '\n'.join(decl_a) + '\nint fa(void)\n{\n    return a01 + a05;\n}\n'
    # B: some of A's (in another order), then its own, then more of A's
    B = '\n'.join(['int b%02d;' % i for i in range(5)] + ['int a04;', 'char a03;'] +
                  ['char b%02d;' % i for i in range(5, 9)]) + '\nint fb(void)\n{\n    return b01;\n}\n'
    # C: only its own, in reverse name order
    C = '\n'.join('int c%02d;' % i for i in range(5, -1, -1)) + '\nint fc(void)\n{\n    return c01;\n}\n'
    Z = 'int _acrtused = 1;\nint main(void)\n{\n    return fa() + fb() + fc();\n}\nint fa(void); int fb(void); int fc(void);\n'
    Z = 'int fa(void); int fb(void); int fc(void);\nint _acrtused = 1;\nint main(void)\n{\n    return fa() + fb() + fc();\n}\n'
    def table(tagv, names):
        return chr(10) + 'char *t%s[] = {(char *)0x%s, %s, (char *)0x7777};' % (tagv, tagv * 4, ', '.join('(char *)&%s' % n for n in names))
    A += table('A', a)
    B += table('B', ['b%02d' % i for i in range(9)] + ['a04', 'a03'])
    C += table('C', ['c%02d' % i for i in range(6)])
    return {'PZ': Z, 'PA': A, 'PB': B, 'PC': C}


def addresses(exe, tagv, names):
    hdr = (exe[8] | exe[9] << 8) * 16
    img = exe[hdr:]
    s = bytes([int(tagv * 2, 16)] * 2)
    i = img.find(s)
    words = [img[i + 2 + 2 * k] | img[i + 3 + 2 * k] << 8 for k in range(len(names))]
    return dict(zip(names, words))


if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 else 'msc51'
    S = objects()
    caps = rk.capture(S, comp, flags='/Ox /Zl /c')
    d = rk.newdir('plinkplus')
    for fn in os.listdir(PL):
        if fn.upper().endswith(('.EXE', '.LIB')):
            shutil.copy(os.path.join(PL, fn), d)
    for k in S:
        print(k, 'COMDEF order:', ' '.join(comdef_order(caps[k]['OBJ'])) or '-')
        open(os.path.join(d, k + '.OBJ'), 'wb').write(tag(caps[k]['OBJ']))
    open(os.path.join(d, 'T.LNK'), 'w', newline='').write('out t.exe\r\nmap\r\nfi pz,pa,pb,pc\r\n')
    open(os.path.join(d, 'DOT.TXT'), 'w', newline='').write('.\r\n' * 10)
    rk.dosbox(d, comp, ['PLINK86 @T.LNK < DOT.TXT > OUT.TXT'], timeout=120)
    for fn in sorted(os.listdir(d)):
        if fn.upper().endswith(('.TXT', '.MAP')) and fn.upper() != 'DOT.TXT':
            t = open(os.path.join(d, fn), encoding='latin-1').read()
            print('==', fn, len(t))
            print(t[:6000])
    exe = open(os.path.join(d, 'T.EXE'), 'rb').read()
    allw = {}
    allw.update(addresses(exe, 'A', ['a%02d' % i for i in range(70)]))
    allw.update(addresses(exe, 'B', ['b%02d' % i for i in range(9)]))
    allw.update(addresses(exe, 'C', ['c%02d' % i for i in range(6)]))
    print('allocation order (by address):')
    print(' '.join('%s@%x' % (n, a) for n, a in sorted(allw.items(), key=lambda kv: kv[1])))
    print('dir:', d)
