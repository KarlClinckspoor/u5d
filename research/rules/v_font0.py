# variants.py spec: FONT_0000's C compiled in the DOS build (it is native-only in src/), with Q3b's shapes.
ON = ('#if !defined(TARGET_DOS16)\n// Prints text justified', '#if 1\n// Prints text justified')
DECL = '    int used;\n\n    ULTIMA_0c22'
LOOPB = '        while ((text[scan] != 0 || used >= lineWidth)'
COND = "(uint)(used + D_50f7 + 1) < lineWidth)\n            {\n                used += D_50f7 + 1;"


def hy(name, typ='int', cond='(uint)(%s + used + 1) < lineWidth', upd='used += %s + 1;', where='before'):
    pairs = [ON, (DECL, '    int used;\n    %s %s;\n\n    ULTIMA_0c22' % (typ, name)),
             (COND, '%s)\n            {\n                %s' % (cond.replace('%s', name), upd.replace('%s', name)))]
    if where == 'before':
        pairs.append((LOOPB, '        %s = D_50f7;\n' % name + LOOPB))
    elif where == 'top':
        pairs.append(('    ULTIMA_0c22_GRAP_0f_SelectPage(1);', '    ULTIMA_0c22_GRAP_0f_SelectPage(1);\n    %s = D_50f7;' % name))
    return pairs


VARIANTS = [('as is (C enabled)', [ON])]
for nm in ('hyph', 'hyphen', 'dashW', 'hw', 'dash', 'hyphW'):
    VARIANTS.append(('%s: hy+used, +=hy+1' % nm, hy(nm)))
VARIANTS += [
    ('hyph uint', hy('hyph', 'uint')),
    ('hyph used+hy', hy('hyph', cond='(uint)(used + %s + 1) < lineWidth')),
    ('hyph upd D_50f7', hy('hyph', upd='used += D_50f7 + 1;')),
    ('hyph at top', hy('hyph', where='top')),
    ('hyph int cmp', hy('hyph', cond='%s + used + 1 < lineWidth')),
]
