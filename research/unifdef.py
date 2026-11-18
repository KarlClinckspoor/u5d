#!/usr/bin/env python3
"""Resolve the preprocessor conditionals of C sources to what the DOS build sees.

    python research/unifdef.py FILE ... [--dry-run]

A one-time cleanup (the native port's removal): every #if/#ifdef/#ifndef/#elif/#else/#endif
whose condition is decided by the DOS build's configuration is replaced by the branch the
DOS build compiles, without the directive lines. Conditions on macros it does not know
(RELEASE_1988, the header guards, STUB, BSS, ...) stay; a condition that only partly depends
on known macros is simplified (`!defined(TARGET_DOS16) || defined(RELEASE_1988)` becomes
`defined(RELEASE_1988)`). Each file keeps its line endings.

The DOS build (MSC 5.x, tools/dosbuild.py) defines MSDOS and M_I86 (the compiler), and
through src/common/platform.h and config.h TARGET_DOS16, OS_DOS16, COMPILER_UNKNOWN and
MATCHING_BUILD; everything in UNDEFINED is not defined there. RELEASE_1988 (the 1988
target's /D) is unknown on purpose: it chooses the release.
"""
import re
import sys

DEFINED = {'TARGET_DOS16': 1, 'MATCHING_BUILD': 1, 'OS_DOS16': 1, 'COMPILER_UNKNOWN': 1,
           'MSDOS': 1, 'M_I86': 1}
UNDEFINED = set('''GOG_BUILD RELEASE_BUILD ENABLE_BGM ENABLE_SPEED_FIX ENABLE_BUG_FIX
ENABLE_TRANSLATION ENABLE_GRAP_OVERLAY ENABLE_GRAP_OVERLAY_DEBUG TARGET_SDL TARGET_DOS32
OS_WINDOWS OS_DOS32 OS_LINUX OS_MACOS OS_UNKNOWN COMPILER_MSVC COMPILER_GCC _MSC_VER __GNUC__
_WIN32 _WIN64 __DJGPP__ __linux__ __APPLE__ __MACH__ VERBOSE_LOG CDDA_TRACE _DEBUG DOS'''.split())

TOKEN = re.compile(r'\s*(defined\b|\|\||&&|!|\(|\)|[A-Za-z_]\w*|\d+)')


def split_comment(text):
    """(code, comment) of a directive's text: the comment keeps its leading space."""
    m = re.search(r'\s*(//|/\*)', text)
    return (text[:m.start()], text[m.start():]) if m else (text, '')


def tokens(expr):
    out, i = [], 0
    expr = expr.rstrip()
    while i < len(expr):
        m = TOKEN.match(expr, i)
        if not m:
            raise ValueError('cannot parse %r' % expr)
        out.append(m.group(1))
        i = m.end()
    return out


# an expression is True, False, or a string (the unknown remainder, as C)
def parse(toks):
    pos = [0]

    def peek():
        return toks[pos[0]] if pos[0] < len(toks) else None

    def take():
        pos[0] += 1
        return toks[pos[0] - 1]

    def primary():
        t = take()
        if t == '!':
            v = primary()
            if isinstance(v, bool):
                return not v
            return '!(' + v + ')' if ('&&' in v or '||' in v) and not v.startswith('(') else '!' + v
        if t == '(':
            v = orx()
            assert take() == ')'
            return '(' + v + ')' if isinstance(v, str) and ('&&' in v or '||' in v) else v
        if t == 'defined':
            paren = peek() == '('
            if paren:
                take()
            name = take()
            if paren:
                assert take() == ')'
            if name in DEFINED:
                return True
            if name in UNDEFINED:
                return False
            return 'defined(%s)' % name
        if t.isdigit():
            return int(t) != 0
        if t in DEFINED:
            return DEFINED[t] != 0
        if t in UNDEFINED:
            return False
        return t

    def andx():
        v = primary()
        while peek() == '&&':
            take()
            w = primary()
            if v is False or w is False:
                v = False
            elif v is True:
                v = w
            elif w is not True:
                v = v + ' && ' + w
        return v

    def orx():
        v = andx()
        while peek() == '||':
            take()
            w = andx()
            if v is True or w is True:
                v = True
            elif v is False:
                v = w
            elif w is not False:
                v = v + ' || ' + w
        return v

    v = orx()
    assert pos[0] == len(toks), toks
    return v


DIRECTIVE = re.compile(r'^(\s*)#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*?)(\r?\n?)$', re.S)


def condition(kind, rest):
    """(value, expression text, comment) of an #if/#ifdef/#ifndef/#elif line."""
    code, comment = split_comment(rest)
    code = code.strip()
    if kind == 'ifdef':
        expr = 'defined(%s)' % code
    elif kind == 'ifndef':
        expr = '!defined(%s)' % code
    else:
        expr = code
    return parse(tokens(expr)), code, comment


def process(text):
    lines = text.splitlines(keepends=True)
    out = []
    # a stack of frames, one per open #if: emitting (are its lines kept), state
    # state: 'pending' (no branch taken yet), 'taken' (a True branch: the rest is dropped),
    # kept: whether a conditional directive was written for this block
    stack = []
    changed = False

    def emitting():
        return all(f['emit'] for f in stack)

    for line in lines:
        m = DIRECTIVE.match(line)
        if not m:
            if emitting():
                out.append(line)
            else:
                out.append(CUT)
            continue
        indent, kind, rest, eol = m.groups()
        outer = emitting() if kind in ('if', 'ifdef', 'ifndef') else all(f['emit'] for f in stack[:-1])
        if kind in ('if', 'ifdef', 'ifndef'):
            f = {'outer': outer, 'done': False, 'written': False, 'emit': False}
            stack.append(f)
            if not outer:
                continue
            v, code, comment = condition(kind, rest)
            if v is True:
                f['done'] = True
                f['emit'] = True
                changed = True
            elif v is False:
                changed = True
            else:
                f['emit'] = True
                f['written'] = True
                if v == ('defined(%s)' % code if kind == 'ifdef' else '!defined(%s)' % code if kind == 'ifndef' else code):
                    out.append(line)
                else:
                    out.append('%s#if %s%s%s' % (indent, v, comment, eol))
                    changed = True
        elif kind == 'elif':
            f = stack[-1]
            if not f['outer']:
                continue
            if f['done']:
                f['emit'] = False
                changed = True
                continue
            v, code, comment = condition(kind, rest)
            if v is True:
                f['done'] = True
                f['emit'] = True
                changed = True
                if f['written']:
                    out.append('%s#else%s%s' % (indent, comment, eol))
            elif v is False:
                f['emit'] = False
                changed = True
            else:
                f['emit'] = True
                if f['written']:
                    if v == code:
                        out.append(line)
                    else:
                        out.append('%s#elif %s%s%s' % (indent, v, comment, eol))
                        changed = True
                else:
                    out.append('%s#if %s%s%s' % (indent, v, comment, eol))
                    f['written'] = True
                    changed = True
        elif kind == 'else':
            f = stack[-1]
            if not f['outer']:
                continue
            if f['done']:
                f['emit'] = False
                changed = True
            else:
                f['emit'] = True
                f['done'] = True
                if f['written']:
                    out.append(line)
                else:
                    changed = True
        else:  # endif
            f = stack.pop()
            if not f['outer']:
                continue
            if f['written']:
                out.append(line)
            else:
                changed = True
    assert not stack, 'unbalanced #if'
    return ''.join(tidy(out)), changed


CUT = None  # in process()'s output: a line was removed here


def tidy(out):
    """The lines without the CUT marks, and without the blank lines that a removal left
    doubled or at the edge of a block: a blank line next to a cut goes when the kept line
    on its other side of the cut is blank, an opening brace or a closing one."""
    lines = []
    cut = False      # a cut since the last kept line
    for line in out:
        if line is CUT:
            cut = True
            continue
        blank = not line.strip()
        if cut and lines:
            prev = lines[-1]
            if blank and (not prev.strip() or prev.rstrip().endswith('{')):
                continue                       # blank after a cut, after blank or '{'
            if not blank and not prev.strip() and line.strip().startswith('}'):
                lines.pop()                    # blank, cut, '}'
        if cut and not lines and blank:
            continue
        lines.append(line)
        cut = False if not blank else cut
    return lines


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    dry = '--dry-run' in sys.argv
    for p in args:
        text = open(p, encoding='latin-1', newline='').read()
        new, changed = process(text)
        if changed and new != text:
            print('%-24s %d -> %d lines' % (p, text.count('\n'), new.count('\n')))
            if not dry:
                open(p, 'w', encoding='latin-1', newline='').write(new)


if __name__ == '__main__':
    main()
