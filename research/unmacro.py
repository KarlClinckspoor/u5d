#!/usr/bin/env python3
"""Expand the native port's macros where the DOS build makes them plain C.

    python research/unmacro.py FILE ... [--dry-run]

A one-time cleanup, after research/unifdef.py (the native port's removal). In the DOS
build (src/common/common.h):

    _TEXT(addr, text), _STATIC_TEXT(addr, text)   text            -> /*addr*/ text
    _XTEXT(addr, text), _XSTATIC_TEXT(addr, text) S_addr          -> S_addr
    U5_VERSION                                    "1.16"
    CDECL, FAR                                    cdecl, far
    u5_itoa                                       itoa
    RETURN_AX(value)                              return          -> return /* AX: value */
    ASSERT(x); CHECK_PARAM_RANGE(...); SET_UNINITIALIZED_16(x); CHECK_UNINITIALIZED_16(x);
                                                  nothing: the line goes

The address of a string stays in a comment before it (/*0x2d3e*/ "1.16"), the form the data
tools read (tools/dgroup.py, ANNOT_RE). Each file keeps its line endings.
"""
import re
import sys

CALL = re.compile(r'\b(_X?(?:STATIC_)?TEXT|RETURN_AX)\(')
EMPTY = re.compile(r'^[ \t]*(ASSERT|CHECK_PARAM_RANGE|SET_UNINITIALIZED_8|SET_UNINITIALIZED_16|'
                   r'CHECK_UNINITIALIZED_8|CHECK_UNINITIALIZED_16)\(.*\);[ \t]*(//.*)?\r?\n', re.M)
WORDS = {'U5_VERSION': '"1.16"', 'CDECL': 'cdecl', 'FAR': 'far', 'u5_itoa': 'itoa'}


def args_end(s, i):
    """The index of the ')' that closes the call whose '(' is at s[i-1], and the index of its
    first top-level comma (or None). Strings and character constants are skipped."""
    depth, comma = 0, None
    while i < len(s):
        c = s[i]
        if c in '"\'':
            j = i + 1
            while s[j] != c:
                j += 2 if s[j] == '\\' else 1
            i = j + 1
            continue
        if c == '\n':
            raise ValueError('a call over several lines at %r' % s[max(0, i - 60):i])
        if c == '(':
            depth += 1
        elif c == ')':
            if depth == 0:
                return i, comma
            depth -= 1
        elif c == ',' and depth == 0 and comma is None:
            comma = i
        i += 1
    raise ValueError('unclosed call')


def expand_words(text):
    return re.sub(r'\b(%s)\b' % '|'.join(WORDS), lambda m: WORDS[m.group(1)], text)


def process(s):
    s = EMPTY.sub('', s)
    out, last = [], 0
    for m in CALL.finditer(s):
        if m.start() < last:
            continue
        end, comma = args_end(s, m.end())
        name = m.group(1)
        out.append(s[last:m.start()])
        if name == 'RETURN_AX':
            out.append('return /* AX: %s */' % s[m.end():end].strip())
        else:
            addr = s[m.end():comma].strip()
            text = expand_words(s[comma + 1:end].strip())
            if name.startswith('_X'):
                out.append('S_' + addr)
            else:
                out.append('/*%s*/ %s' % (addr, text))
        last = end + 1
    out.append(s[last:])
    s = ''.join(out)
    # the words, outside comments and strings is not needed: none of them occurs in either
    return expand_words(s)


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    dry = '--dry-run' in sys.argv
    for p in args:
        text = open(p, encoding='latin-1', newline='').read()
        new = process(text)
        if new != text:
            print('%-24s %d -> %d lines' % (p, text.count('\n'), new.count('\n')))
            if not dry:
                open(p, 'w', encoding='latin-1', newline='').write(new)


if __name__ == '__main__':
    main()
