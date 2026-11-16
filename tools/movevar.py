#!/usr/bin/env python3
"""Move data definitions from vars.c into the module whose data they are.

    python tools/movevar.py FILE.c --plan              print the moves tools/dgroup.py suggests
    python tools/movevar.py FILE.c --apply             make them
    python tools/movevar.py FILE.c D_xxxx --before FUNCTION [--size N]   move one by hand

In the original, a module's tables sit in DGROUP between its string literals, in the order
the module defines them. Microsoft C 5.1 writes a module's _DATA almost in source order:
a definition at file scope comes after the string literals of the function that follows
it (seen in shoppes3.c: a table defined before SHOPPES3_002c lands after its strings). So a
table that follows function F's strings in DGROUP is defined just before F (before the
first function if no string of the module precedes it).

The definition (with the comment lines directly above it) is cut from vars.c and inserted
before FUNCTION's definition (and its comment block). vars.h keeps the extern declaration.
With --size N (bytes), an array's element count is corrected in the definition and in
vars.h: u5d's sizes are sometimes guesses that run into the next item.
"""
import argparse, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402
import dgroup  # noqa: E402

ELEM = {'u8': 1, 's8': 1, 'byte': 1, 'char': 1, 'bool': 1, 'u16': 2, 's16': 2, 'int': 2, 'uint': 2,
        'word': 2, 'u32': 4, 's32': 4, 'long': 4}


def read(p):
    return open(p, encoding='utf-8', newline='').read()


def write(p, s):
    open(p, 'w', encoding='utf-8', newline='').write(s)


def find_definition(src, var):
    """(start, end) of var's definition in src (vars.c), with the comment lines just above."""
    m = re.search(r'^[A-Za-z][\w\s\*]*?\b%s\s*(\[[^\]]*\])*\s*(=|;)' % re.escape(var), src, re.M)
    if not m:
        return None
    i, depth = m.start(), 0
    while i < len(src):
        c = src[i]
        if c in '{(':
            depth += 1
        elif c in '})':
            depth -= 1
        elif c == ';' and depth == 0:
            break
        i += 1
    end = src.index('\n', i) + 1 if '\n' in src[i:] else len(src)
    start = m.start()
    while True:                                    # take comment lines directly above
        prev = src.rfind('\n', 0, start - 1)
        line = src[prev + 1:start]
        if line.strip().startswith('//'):
            start = prev + 1
        else:
            break
    return start, end


def function_start(src, func):
    """Offset where func's definition starts, including its comment block, or None."""
    m = re.search(r'^(?:static\s+)?[\w\s\*]*?\b%s\s*\([^;{)]*\)\s*\n\{' % re.escape(func), src, re.M | re.I)
    if not m:
        return None
    start = m.start()
    while True:
        prev = src.rfind('\n', 0, start - 1)
        line = src[prev + 1:start]
        if line.strip().startswith('//'):
            start = prev + 1
        else:
            break
    return start


def resize(text, var, nbytes):
    """Set the element count of var's (first) array dimension so it takes nbytes."""
    m = re.search(r'^(\s*(?:STUB\s+)?)([A-Za-z][\w\s\*]*?)\b(%s)\s*\[([^\]]*)\]((?:\s*\[[^\]]*\])*)' % re.escape(var),
                  text, re.M)
    if not m:
        return text, False
    typ = m.group(2).split()[-1].strip('*')
    size = ELEM.get(typ)
    if not size:
        return text, False
    for dim in re.findall(r'\[([^\]]*)\]', m.group(5)):    # further dimensions: [16][4]
        try:
            size *= int(dim, 0)
        except ValueError:
            return text, False
    if nbytes % size:
        return text, False
    n = nbytes // size
    text = text[:m.start(4)] + ('0x%x' % n if '0x' in m.group(4) else str(n)) + text[m.end(4):]
    # drop initialisers beyond the new size (simple flat lists only): they were the bytes
    # of the next item, which the old size ran into
    keep = nbytes // ELEM[typ]                  # scalar initialisers that fit
    im = re.search(r'=\s*\{([^{}]*)\}', text)
    if im:
        vals = [v.strip() for v in im.group(1).split(',') if v.strip()]
        if len(vals) > keep:
            text = text[:im.start(1)] + ' ' + ', '.join(vals[:keep]) + ' ' + text[im.end(1):]
    return text, True


def move(cfile, var, before, size=None):
    vp = os.path.join(u5.SRC, 'vars.c')
    hp = os.path.join(u5.SRC, 'vars.h')
    mp = os.path.join(u5.SRC, cfile)
    vs, ms = read(vp), read(mp)
    d = find_definition(vs, var)
    if not d:
        raise SystemExit('%s is not defined in vars.c' % var)
    block = vs[d[0]:d[1]]
    vs = vs[:d[0]] + vs[d[1]:]
    if size:
        block, ok = resize(block, var, size)
        if ok:
            hs, _ = resize(read(hp), var, size)
            write(hp, hs)
    if before:
        at = function_start(ms, before)
        if at is None:
            raise SystemExit('no function %s in %s' % (before, cfile))
    else:
        at = len(ms)
        if not ms.endswith('\n'):
            ms += '\n'
            at += 1
    if at > 0 and ms[at - 2:at] != '\n\n':
        block = '\n' + block
    ms = ms[:at] + block + '\n' + ms[at:]
    write(vp, vs)
    write(mp, ms)


def plan(cfile):
    """[(var, before_function or None, size)] for the variables of cfile's extent that only
    cfile uses (or nobody does), plus those just before it that only it uses."""
    items, users = dgroup.build()
    own = sorted((it for it in items if it[2] == 'str' and it[4] == cfile), key=lambda x: x[0])
    if not own:
        return []
    # the module's data is its main run of strings: a module sometimes uses a string that
    # lives in another module's data, so take the largest cluster (gaps under 0x400)
    runs, cur = [], [own[0]]
    for x in own[1:]:
        if x[0] - (cur[-1][0] + cur[-1][1]) < 0x400:
            cur.append(x)
        else:
            runs.append(cur)
            cur = [x]
    runs.append(cur)
    run = max(runs, key=len)
    s, e = run[0][0], run[-1][0] + run[-1][1]
    if any(it[2] == 'str' and it[4] != cfile and s <= it[0] < e for it in items):
        print('warning: strings of other modules inside %s\'s data %04x-%04x' % (cfile, s, e))
    # a table between the module's strings is the module's, whoever uses it (it is then a
    # global the others reach by name); at the edges, extend over tables only it uses
    inner_s, inner_e = s, e
    before = [it for it in items if it[0] < s]
    for it in reversed(before):
        if it[2] == 'var' and users[it[3]] == [cfile]:
            s = it[0]
        else:
            break
    after = [it for it in items if it[0] >= e and it[4] != cfile]
    for it in after:
        if it[2] == 'var' and users[it[3]] == [cfile] and it[0] == e:
            e = it[0] + it[1]
        else:
            break
    mine = [it for it in items if s <= it[0] < (e or 0x10000)]
    src = read(os.path.join(u5.SRC, cfile))
    first_func = re.search(r'^(?:static\s+)?[\w\s\*]*?\b(\w+)\s*\([^;{)]*\)\s*\n\{', src, re.M).group(1)
    out = []
    for i, it in enumerate(mine):
        if it[2] != 'var' or it[4] != 'vars.c':
            continue
        inside = inner_s <= it[0] < inner_e
        if not inside and users[it[3]] != [cfile]:
            continue
        # MSC 5.1 writes a file-scope definition after the string literals of the function
        # that follows it; so a table that comes after function F's strings is defined
        # just before F (before the first function if no string of the module precedes it)
        prv = next((x for x in reversed(mine[:i]) if x[2] == 'str' and x[4] == cfile), None)
        out.append((it[3], prv[5] if prv else first_func, it[1]))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('file')
    ap.add_argument('var', nargs='?')
    ap.add_argument('--before')
    ap.add_argument('--size', type=lambda x: int(x, 0))
    ap.add_argument('--plan', action='store_true')
    ap.add_argument('--apply', action='store_true')
    a = ap.parse_args()
    f = os.path.basename(a.file)
    if a.var:
        move(f, a.var, a.before, a.size)
        print('moved %s into %s' % (a.var, f))
        return
    p = plan(f)
    for var, fn, size in p:
        print('%-28s %4d bytes  before %s' % (var, size, fn or '(end of file)'))
    if a.apply:
        for var, fn, size in p:
            move(f, var, fn, size)
        print('%d moved' % len(p))


if __name__ == '__main__':
    main()
