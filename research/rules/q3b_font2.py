#!/usr/bin/env python3
"""Q3b, second set: the hoisted D_50f7 as a real local (several types), MSC 5.0.

    python research/rules/q3b_font2.py [msc50|msc51] [--show NAME]
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
from q3b_font import HEAD, ORIG, OURS  # noqa: E402

def src(decl, init, cond, upd):
    return HEAD + '''
int f(byte *text, int lineStart, int scan, int used, int lineWidth, int spaces)
{
    %s
    %s
    while ((text[scan] != 0 || used >= lineWidth) && text[scan] != 10 && lineStart < --scan)
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
''' % (decl, init, cond, upd)

V = {}
for t in ('int', 'uint', 'byte'):
    V['%s_w' % t] = ('%s w;' % t, 'w = D_50f7;', '(uint)(used + w + 1) < lineWidth', 'used += w + 1;')
    V['%s_w_d' % t] = ('%s w;' % t, 'w = D_50f7;', '(uint)(w + used + 1) < lineWidth', 'used += w + 1;')
    V['%s_w1' % t] = ('%s w;' % t, 'w = D_50f7 + 1;', '(uint)(used + w) < lineWidth', 'used += w;')
    V['%s_w_mix' % t] = ('%s w;' % t, 'w = D_50f7;', '(uint)(used + w + 1) < lineWidth', 'used += D_50f7 + 1;')
V['reg_w'] = ('register int w;', 'w = D_50f7;', '(uint)(used + w + 1) < lineWidth', 'used += w + 1;')
V['ptr'] = ('byte *hp;', 'hp = &D_50f7;', '(uint)(used + *hp + 1) < lineWidth', 'used += *hp + 1;')
V['inloop'] = ('int w;', '', '(uint)(used + (w = D_50f7) + 1) < lineWidth', 'used += w + 1;')

if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith('-') else 'msc50'
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    S = {('G%02d' % i): src(*v) for i, v in enumerate(V.values())}
    caps = rk.capture(S, comp)
    for (k, _), n in zip(S.items(), V):
        code = rk.funcs(caps[k].get('OBJ')).get('F')
        if code is None:
            print('%-10s no code %s' % (n, caps[k].get('ERR', '')[-200:])); continue
        t = '\n'.join(rk.dis(code))
        print('%-12s %3d bytes  %s' % (n, len(code), 'TEMP+REG (original)' if ORIG.search(t) else 'REG+TEMP' if OURS.search(t) else 'other'))
        if show in (n, 'all'):
            print(t)
