#!/usr/bin/env python3
"""Q3b: where does MSC 5.0 put the common subexpression region*2 (index of two word arrays):
in SI (mov si,ax / shl si,1, as u5d's FONT_0000 at 025f) or in a stack temporary with a dead
store and BX (shl ax,1 / mov [bp-N],ax / mov bx,ax, as the original)?

    python research/rules/q3b_cse.py [msc50|msc51] [--show NAME]
"""
import os, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402

HEAD = '''typedef unsigned char byte;
extern int A1[2], A2[2];
extern int G1, G2, G3, G4;
extern byte D_50ea[];
'''
TAIL_STMTS = '''region = G4 > G1 && G4 < G2;
        G3 = A1[region];
        width = A2[region] - G3;'''

BODIES = {
    # the CSE after an inner loop that uses SI/DI, at the end of an outer loop's body
    'outer_end': ('', '''for (; text[start] != 0; 0) {
        for (scan = start; text[scan] != 0 && width > used; scan++) used += D_50ea[text[scan]];
        start = scan;
        if (text[start] == 0) goto next;
        %s
        start++;
next:;
    }''' % TAIL_STMTS),
    'outer_end_reg': ('register int r;', '''for (; text[start] != 0; 0) {
        for (scan = start; text[scan] != 0 && width > used; scan++) used += D_50ea[text[scan]];
        start = scan;
        if (text[start] == 0) goto next;
        %s
        start++;
        r = start;
next:;
    }''' % TAIL_STMTS),
    'straight': ('', TAIL_STMTS),
    'after_loop': ('', '''for (scan = start; text[scan] != 0 && width > used; scan++) used += D_50ea[text[scan]];
    %s''' % TAIL_STMTS),
    'reg_si_di': ('register int r1, r2;', '''r1 = G1; r2 = G2;
    %s
    G1 = r1 + r2;''' % TAIL_STMTS),
    'reg_si': ('register int r1;', '''r1 = G1;
    %s
    G1 = r1;''' % TAIL_STMTS),
}

INNER_A = 'for (scan = start; text[scan] != 0 && width > used; scan++) used += D_50ea[text[scan]];'
INNER_B = 'while (start < --scan) { if (text[scan] == 32) { used -= G1; break; } used -= D_50ea[text[scan]]; }'
INNER_C = 'scan = start; if (scan < G2) do { G3 += D_50ea[text[scan]]; } while (G2 != ++scan);'
def outer(inner, head='for (; text[start] != 0; 0)', tail_open='if (text[start] == 0) goto next;', close='next:;'):
    nl = chr(10) + '        '
    return (head + ' {' + nl + nl.join(inner) + nl + 'start = scan;' + nl + tail_open + nl + TAIL_STMTS + nl
            + 'start++;' + chr(10) + close + chr(10) + '    }')
BODIES['o_A'] = ('', outer([INNER_A]))
BODIES['o_AB'] = ('', outer([INNER_A, INNER_B]))
BODIES['o_ABC'] = ('', outer([INNER_A, INNER_B, INNER_C]))
BODIES['o_AC'] = ('', outer([INNER_A, INNER_C]))
BODIES['o_BC'] = ('', outer([INNER_B, INNER_C]))
BODIES['o_C'] = ('', outer([INNER_C]))
BODIES['o_B'] = ('', outer([INNER_B]))
BODIES['o_ABC_while'] = ('', outer([INNER_A, INNER_B, INNER_C], head='while (text[start] != 0)'))
BODIES['o_ABC_inc'] = ('', outer([INNER_A, INNER_B, INNER_C], head='for (; text[start] != 0; G4++)'))


def src(decl, body):
    return HEAD + '''
void f(byte *text, int width)
{
    int region, scan, start, used;
    %s
    start = 0; used = 0;
    %s
}
''' % (decl, body)


SI = re.compile(r'mov\s+si,ax\n.*shl\s+si,1')
BX = re.compile(r'shl\s+ax,1\n.*mov\s+\[bp-\w+\],ax\n.*mov\s+bx,ax')

if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith('-') else 'msc50'
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    S = {('C%02d' % i): src(*v) for i, v in enumerate(BODIES.values())}
    caps = rk.capture(S, comp)
    for (k, _), n in zip(S.items(), BODIES):
        code = rk.funcs(caps[k].get('OBJ')).get('F')
        if code is None:
            print('%-14s no code %s' % (n, caps[k].get('ERR', '')[-300:])); continue
        t = '\n'.join(rk.dis(code))
        print('%-14s %3d bytes  %s' % (n, len(code), 'TEMP+BX (original)' if BX.search(t) else 'SI' if SI.search(t) else 'other'))
        if show in (n, 'all'):
            print(t)

HEADC = 'region = G4 > G1 && G4 < G2; width = A2[region] - A1[region]; used = G3 - A1[region];'
BODIES2 = {
    'o_ABC_head': ('', HEADC + chr(10) + '    ' + outer([INNER_A, INNER_B, INNER_C])),
    'o_A_head': ('', HEADC + chr(10) + '    ' + outer([INNER_A])),
    'o_A_head1': ('', 'region = G4 > G1 && G4 < G2; width = A2[region];' + chr(10) + '    ' + outer([INNER_A])),
    'o_A_head2': ('', 'width = A2[G1] - A1[G1];' + chr(10) + '    ' + outer([INNER_A])),
}
if __name__ == '__main__' and '--head' in sys.argv:
    S = {('H%02d' % i): src(*v) for i, v in enumerate(BODIES2.values())}
    caps = rk.capture(S, 'msc50')
    for (k, _), n in zip(S.items(), BODIES2):
        code = rk.funcs(caps[k].get('OBJ')).get('F')
        t = '\n'.join(rk.dis(code))
        tail = t[t.rfind('shl'):] if 'shl' in t else ''
        print('%-14s %3d bytes  last CSE: %s' % (n, len(code), 'TEMP+BX (original)' if BX.search(t) else 'SI'))
