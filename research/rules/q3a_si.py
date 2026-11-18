#!/usr/bin/env python3
"""Q3a: spellings that build a value in SI in two steps with no stack slot of its own (MSC 5.0).

DUNGEON_1952_DrawCell (and SJOG_0514's register local) in the original:
    mov al,[bx+tbl] / sub ah,ah / mov si,ax / mov ax,[bp+4] / shl ax,1 / add si,ax / push si ... push si
with a frame of only the function's two int locals (4 bytes). u5d gets it with a register
pointer whose slot is shared with another block's local. Each spelling below is compiled in a
small function with the same surroundings; the report gives the frame size, whether SI is
built in two steps (the original's pattern), and the code.

    python research/rules/q3a_si.py [msc50|msc51] [--show NAME]
"""
import os, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402

HEAD = '''typedef unsigned char byte;
extern unsigned char tbl[8];
extern void DrawPiece(int, int, int);
extern int CellAt(int, int);
#define TWO(e) DrawPiece((int)(e), 1, 0); DrawPiece((int)(e), 1, 1)
'''

# each: (declarations at function level, statements in the then-branch)
SPELL = {
    'reg_int':      ('register int p;', 'p = tbl[kind]; p += dist * 2; TWO(p);'),
    'reg_uns':      ('register unsigned p;', 'p = tbl[kind]; p += dist * 2; TWO(p);'),
    'reg_bptr':     ('register byte *p;', 'p = (byte *)tbl[kind]; p += dist * 2; TWO(p);'),
    'reg_cptr':     ('register char *p;', 'p = (char *)tbl[kind]; p += dist * 2; TWO(p);'),
    'reg_int1':     ('register int p;', 'p = tbl[kind] + dist * 2; TWO(p);'),
    'reg_int1sw':   ('register int p;', 'p = dist * 2 + tbl[kind]; TWO(p);'),
    'reg_bptr1':    ('register byte *p;', 'p = (byte *)tbl[kind] + dist * 2; TWO(p);'),
    'reg_bptr_idx': ('register byte *p;', 'p = &((byte *)tbl[kind])[dist * 2]; TWO(p);'),
    'reg_wptr':     ('register int *p;', 'p = (int *)tbl[kind]; p += dist; TWO(p);'),
    'reg_wptr1':    ('register int *p;', 'p = (int *)tbl[kind] + dist; TWO(p);'),
    'reg_comma':    ('register int p;', 'TWO((p = tbl[kind], p += dist * 2, p));'),
    'cse':          ('', 'TWO(tbl[kind] + dist * 2);'),
    'cse_sw':       ('', 'TWO(dist * 2 + tbl[kind]);'),
    'cse_ptr':      ('', 'TWO((byte *)tbl[kind] + dist * 2);'),
    'cse_wptr':     ('', 'TWO((int *)tbl[kind] + dist);'),
    'cse_uns':      ('', 'TWO((unsigned)tbl[kind] + (unsigned)dist * 2);'),
    'cse_shl':      ('', 'TWO(tbl[kind] + (dist << 1));'),
    'cse_add2':     ('', 'TWO(tbl[kind] + dist + dist);'),
    'loc_int':      ('int p;', 'p = tbl[kind]; p += dist * 2; TWO(p);'),
    'blk_reg':      ('', '{ register int p; p = tbl[kind]; p += dist * 2; TWO(p); }'),
    'blk_regptr':   ('', '{ register byte *p; p = (byte *)tbl[kind]; p += dist * 2; TWO(p); }'),
    'reg_char':     ('register byte p;', 'p = tbl[kind]; p += dist * 2; TWO(p);'),
    'reg_two':      ('register int p, q;', 'p = tbl[kind]; q = dist * 2; p += q; TWO(p);'),
    'reg_param':    ('', 'dist = tbl[kind] + dist * 2; TWO(dist);'),
    'reg_param2':   ('', 'dist *= 2; dist += tbl[kind]; TWO(dist);'),
    'reg_ptrpar':   ('register byte *p;', 'p = tbl + kind; p = (byte *)*p + dist * 2; TWO(p);'),
}
REGPARAM = ('reg_param', 'reg_param2')


def src(name, decl, body):
    par = 'register int dist' if name in REGPARAM else 'int dist'
    return HEAD + '''
void f(int x, int y, %s)
{
    int kind;
    int cell;
    %s

    cell = CellAt(x, y);
    kind = cell >> 4;
    if (tbl[kind] != 0) {
        %s
    }
    if (cell & 7) {
        DrawPiece(kind, 0, 0);
    }
}
''' % (par, decl, body)


PAT = re.compile(r'mov\s+al,.*\n.*sub\s+ah,ah\n.*mov\s+si,ax\n.*mov\s+ax,\[bp\+4\]\n.*shl\s+ax,1\n.*add\s+si,ax')


def frame(lines):
    for l in lines[:6]:
        m = re.search(r'sub\s+sp,(\w+)', l)
        if m:
            return int(m.group(1).rstrip('h'), 16)
    return 0


if __name__ == '__main__':
    comp = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith('-') else 'msc50'
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    S = {('S%02d' % i): src(n, *v) for i, (n, v) in enumerate(SPELL.items())}
    caps = rk.capture(S, comp)
    for (k, _), n in zip(S.items(), SPELL):
        code = rk.funcs(caps[k].get('OBJ')).get('F')
        if code is None:
            print('%-13s no code %s' % (n, caps[k].get('ERR', '')[-200:]))
            continue
        lines = rk.dis(code)
        txt = '\n'.join(lines)
        print('%-13s frame %2d  two-step-SI %-3s  %d bytes' % (n, frame(lines), 'YES' if PAT.search(txt) else 'no', len(code)))
        if show in (n, 'all'):
            print('\n'.join('      ' + l for l in lines))
