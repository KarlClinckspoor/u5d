#!/usr/bin/env python3
"""Q3b: FONT_0000's loop B compare, in isolation (MSC 5.0): which spelling adds the hoisted
D_50f7 temporary first (mov ax,[bp-N] / add ax,REG / inc ax, as the original at 0173) rather
than the register (mov ax,REG / add ax,[bp-N])?

    python research/rules/q3b_font.py [msc50|msc51] [--show NAME]
"""
import os, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402

HEAD = '''typedef unsigned int uint;
typedef unsigned char byte;
extern byte D_50ea[];
#define D_50f7 D_50ea[0xd]
extern uint D_5154;
'''
COND = {
    'u_d_1':     '(uint)(used + D_50f7 + 1) < lineWidth',
    'd_u_1':     '(uint)(D_50f7 + used + 1) < lineWidth',
    'd_1_u':     '(uint)(D_50f7 + 1 + used) < lineWidth',
    'u_(d1)':    '(uint)(used + (D_50f7 + 1)) < lineWidth',
    '1_d_u':     '(uint)(1 + D_50f7 + used) < lineWidth',
    'u_1_d':     '(uint)(used + 1 + D_50f7) < lineWidth',
    '(d1)_u':    '(uint)((D_50f7 + 1) + used) < lineWidth',
    'castR':     'used + D_50f7 + 1 < (uint)lineWidth',
    'castU':     '(uint)used + D_50f7 + 1 < lineWidth',
    'd_castU':   'D_50f7 + (uint)used + 1 < lineWidth',
    'int_cmp':   'used + D_50f7 + 1 < lineWidth',
    'int_dcmp':  'D_50f7 + used + 1 < lineWidth',
    'swap_gt':   'lineWidth > (uint)(used + D_50f7 + 1)',
    'swap_gt_d': 'lineWidth > (uint)(D_50f7 + used + 1)',
    'ud_le':     '(uint)(used + D_50f7) < lineWidth - 1',
    'int_d':     '(int)D_50f7 + used + 1 < lineWidth',
    'uint_d':    '(uint)D_50f7 + used + 1 < lineWidth',
    'uint_d2':   '(uint)D_50f7 + (uint)used + 1 < (uint)lineWidth',
    'not_ge':    '!((uint)(used + D_50f7 + 1) >= lineWidth)',
}
UPD = {
    'pe': 'used += D_50f7 + 1;',
    'pe_1d': 'used += 1 + D_50f7;',
    'assign': 'used = used + D_50f7 + 1;',
    'assign_d': 'used = D_50f7 + used + 1;',
}


def src(cond, upd):
    return HEAD + '''
int f(byte *text, int lineStart, int scan, int used, int lineWidth, int spaces)
{
    while ((text[scan] != 0 || used >= lineWidth) && text[scan] != '\\n' && lineStart < --scan)
    {
        if (text[scan] == ' ')
        {
            used -= D_5154;
            spaces--;
            break;
        }
        if (text[scan] == '_' && %s)
        {
            %s
            break;
        }
        if (text[scan] != '_' && text[scan] != '{')
        {
            used -= D_50ea[text[scan] - 0x20] + 1;
        }
    }
    return used + spaces;
}
''' % (cond, upd)


ORIG = re.compile(r'mov\s+ax,\[bp-\w+\]\n\S+\s+\S+\s+add\s+ax,(si|di|cx|dx|bx)\n\S+\s+\S+\s+inc\s+ax')
OURS = re.compile(r'mov\s+ax,(si|di|cx|dx|bx)\n\S+\s+\S+\s+add\s+ax,\[bp-\w+\]')

if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith('-') else 'msc50'
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    combos = [(c, 'pe') for c in COND] + [('u_d_1', u) for u in UPD if u != 'pe'] + [('d_u_1', u) for u in UPD if u != 'pe']
    S = {('F%02d' % i): src(COND[c], UPD[u]) for i, (c, u) in enumerate(combos)}
    caps = rk.capture(S, comp)
    for (k, _), (c, u) in zip(S.items(), combos):
        code = rk.funcs(caps[k].get('OBJ')).get('F')
        if code is None:
            print('%-10s %-8s no code %s' % (c, u, caps[k].get('ERR', '')[-200:]))
            continue
        lines = rk.dis(code)
        t = '\n'.join(lines)
        verdict = 'TEMP+REG (original)' if ORIG.search(t) else 'REG+TEMP' if OURS.search(t) else 'other'
        print('%-10s %-8s %3d bytes  %s' % (c, u, len(code), verdict))
        if show in (c, 'all'):
            print('\n'.join('      ' + l for l in lines))
