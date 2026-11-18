#!/usr/bin/env python3
"""Q3b: FONT_0000's C in a test module of its own (MSC 5.0), compared with the original's bytes
(FONT.OVL 0000-02a2), with edits applied to it. Shows the verdicts for the two open points:
the region*2 CSE at the end of the outer loop (SI or temp+BX) and loop B's add order.

    python research/rules/q3b_font0.py [--show NAME]
"""
import os, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
import u5  # noqa: E402
from q3b_cse import SI, BX  # noqa: E402
from q3b_font import ORIG, OURS  # noqa: E402

HEAD = '''typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned char u8;
typedef unsigned short u16;
extern u8 D_50ea[0x5b];
#define D_50f7 D_50ea[0xd]
extern u16 D_5146[2];
extern u16 D_514c[2];
extern u16 D_5150;
extern u16 D_5152;
extern u16 D_5154;
extern u16 D_5156;
extern u16 D_5158;
extern void ULTIMA_0c22_GRAP_0f_SelectPage(int a);
extern void ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(byte* a, int b, int c, int d);
'''


def function_text():
    t = open(os.path.join(u5.SRC, 'font.c'), encoding='utf-8').read().replace('\r\n', '\n')
    a = t.index('void FONT_0000_DrawJustifiedText(byte* glyphs, byte* text)\n{')
    b = t.index('\n}\n', a) + 3
    return t[a:b]


def original():
    g = u5.find_gamedata(None)
    return open(os.path.join(g, 'FONT.OVL'), 'rb').read()[:0x2a4]


def diff(a, b):
    n = sum(1 for x, y in zip(a, b) if x != y) + abs(len(a) - len(b))
    return n


EDITS = {
    'as_is': [],
    'no_tail_block': [('''            goto L_next;
        }
        {
''', '''            goto L_next;
        }
'''), ('''            lineStart++;
        }
L_next:;''', '''            lineStart++;
L_next:;''')],
    'if_else_tail': [('''        if (text[lineStart] == 0)
        {
            goto L_next;
        }
        {
''', '''        if (text[lineStart] != 0)
        {
''')],
}

if __name__ == '__main__':
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    base = function_text()
    orig = original()
    S, names = {}, []
    for i, (n, pairs) in enumerate(EDITS.items()):
        t = base
        ok = True
        for o, nw in pairs:
            if t.count(o) != 1:
                print(n, 'pattern count', t.count(o), repr(o[:40])); ok = False; break
            t = t.replace(o, nw)
        if ok:
            S['V%02d' % i] = HEAD + t
            names.append(n)
    caps = rk.capture(S, 'msc50')
    for k, n in zip(S, names):
        code = rk.funcs(caps[k].get('OBJ')).get('FONT_0000_DRAWJUSTIFIEDTEXT')
        if code is None:
            print(n, 'no code', caps[k].get('ERR', '')[-300:]); continue
        t = '\n'.join(rk.dis(code))
        tail = t[t.rfind('add word ptr ds:[0],9') if 'add word ptr ds:[0],9' in t else 0:]
        print('%-14s len %d/%d  bytes differing %d  tail CSE: %s  loopB add: %s' % (
            n, len(code), len(orig), diff(code, orig),
            'TEMP+BX (orig)' if BX.search(t) else 'SI' if SI.search(tail) else '?',
            'TEMP+REG (orig)' if ORIG.search(t) else 'REG+TEMP' if OURS.search(t) else '?'))
        if show in (n, 'all'):
            print(t)


def cut(t, start, end):
    """remove from the line holding `start` to the line holding `end` (inclusive)"""
    a = t.index(start)
    a = t.rfind('\n', 0, a) + 1
    b = t.index(end, a)
    b = t.index('\n', b) + 1
    return t[:a] + t[b:]


CUTS = {
    'as_is': lambda t: t,
    'noC': lambda t: cut(t, '        scan = lineStart;\n        if (scan < lineEnd)', '} while (lineEnd != ++scan);'),
    'noB': lambda t: cut(t, '        while ((text[scan] != 0 || used >= lineWidth)', '        slack = lineWidth - used;'),
    'noA': lambda t: cut(t, '        for (scan = lineStart; text[scan]', '        while ((text[scan] != 0 || used >= lineWidth)').replace(
        '        while ((text[scan]', '        scan = lineStart + 9;\n        while ((text[scan]', 1),
    'noCopy': lambda t: cut(t, "            if (text[lineStart] == '_' && (int)D_5158 < 0xc0)", '            used = 0;').replace(
        '            D_5158 += 9;', '            used = 0;\n            D_5158 += 9;', 1),
    'noTop': lambda t: t.replace('''    region = (int)D_5158 > (int)D_5150 && (int)D_5158 < (int)D_5152;
    lineWidth = D_514c[region] - D_5146[region];
    lineStart = lineEnd = 0;
    used = D_5156 - D_5146[region];''', '''    region = 0;
    lineWidth = 9;
    lineStart = lineEnd = 0;
    used = 0;''', 1),
}

if __name__ == '__main__' and '--cuts' in sys.argv:
    base = function_text()
    S = {('K%02d' % i): HEAD + f(base) for i, f in enumerate(CUTS.values())}
    caps = rk.capture(S, 'msc50')
    for (k, _), n in zip(S.items(), CUTS):
        code = rk.funcs(caps[k].get('OBJ')).get('FONT_0000_DRAWJUSTIFIEDTEXT')
        if code is None:
            print(n, 'no code', caps[k].get('ERR', '')[-300:]); continue
        t = '\n'.join(rk.dis(code))
        i = t.find('add word ptr ds:[0],9')
        tail = t[i:] if i >= 0 else t
        print('%-8s len %d  tail CSE: %s' % (n, len(code), 'TEMP+BX (orig)' if BX.search(tail) else 'SI' if SI.search(tail) else '?'))


def rename(t, pairs):
    for a, b in pairs:
        t = re.sub(r'\b%s\b' % a, b, t)
    return t


LOCALS = ['region', 'scan', 'lineStart', 'lineWidth', 'spaces', 'slack', 'lineEnd', 'used']
NAMES = {
    'long': lambda t: t,
    'short1': lambda t: rename(t, [(n, 'v%d' % i) for i, n in enumerate(LOCALS)] + [('glyphs', 'p1'), ('text', 'p2')]),
    'local_N': lambda t: rename(t, [(n, 'local_%x' % (2 * i + 2)) for i, n in enumerate(LOCALS)]),
    'longer': lambda t: rename(t, [(n, n + 'Padded_xxxxxxxx') for n in LOCALS]),
    'mem_minus': lambda t: t,
}

if __name__ == '__main__' and '--names' in sys.argv:
    base = function_text()
    S = {('N%02d' % i): HEAD + f(base) for i, f in enumerate(NAMES.values())}
    caps = rk.capture(S, 'msc50')
    for (k, _), n in zip(S.items(), NAMES):
        code = rk.funcs(caps[k].get('OBJ')).get('FONT_0000_DRAWJUSTIFIEDTEXT')
        t = '\n'.join(rk.dis(code))
        i = t.find('add word ptr ds:[0],9')
        tail = t[i:] if i >= 0 else t
        print('%-8s len %d diff %d tail CSE: %s' % (n, len(code), diff(code, original()),
                                                  'TEMP+BX (orig)' if BX.search(tail) else 'SI' if SI.search(tail) else '?'))
