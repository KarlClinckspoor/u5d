#!/usr/bin/env python3
"""A map of DGROUP (the data segment, DATA.OVL): which module each byte belongs to.

    python tools/dgroup.py                 the module extents and the variables in each
    python tools/dgroup.py --module FILE.c everything in one module's extent, in DS order
    python tools/dgroup.py --at 4dc5       what is at (or around) one DS address

In the original, every module's initialised data (its tables and string literals) sits in
DGROUP at a fixed place, in the order the module defines it. u5d keeps the tables as
global D_xxxx variables in vars.c, and the strings in the functions that use them, each
with its address in a comment before it (/*0x96b4*/ "..."). This tool puts both on one map:

  - a string's address is the one its comment gives, and it belongs to the file it is in;
  - a variable's address is in its name, its extent runs to the next known item, and the
    modules that use it are found by name;
  - a module's extent runs from its first string to the start of the next module's.

A variable inside another module's extent, used only by that module, is a candidate for
moving there (defined between the functions whose strings surround it).
"""
import argparse, glob, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402

LIT = r'(?:(?:"(?:[^"\\]|\\.)*"\s*)+|[A-Z_][A-Z0-9_]*\b)'
# the native port's form, which src/ no longer has: _TEXT(0x96b4, "..."), _STATIC_TEXT(0x2626, "...")
STR_RE = re.compile(r'_(?:STATIC_)?TEXT\(\s*0x([0-9a-fA-F]{1,4})\s*,\s*(' + LIT + r')\s*\)')
# a string whose address u5d wrote in a comment: /*0x2626*/ "TOWNE.DAT", /*0x96c6*/ CLASS_LIST
ANNOT_RE = re.compile(r'/\*\s*0x([0-9a-fA-F]{4})\s*\*/\s*(' + LIT + r')')
MACROS = {}


def macros():
    if not MACROS:
        for p in glob.glob(os.path.join(u5.SRC, '*.h')):
            for m in re.finditer(r'^#define\s+([A-Z_][A-Z0-9_]*)\s+((?:"(?:[^"\\]|\\.)*"\s*)+)$',
                                 open(p, encoding='utf-8', errors='replace').read(), re.M):
                MACROS[m.group(1)] = m.group(2).strip()
    return MACROS


def literal(text):
    """The C literal for a string written as a literal or as a macro naming one."""
    text = text.strip()
    return text if text.startswith('"') else macros().get(text)
VAR_RE = re.compile(r'^[A-Za-z][\w\s\*]*?\b(\w+)\s*(\[[^\]]*\])*\s*(=|;)', re.M)


def c_string_len(lit):
    """Length in bytes (with the NUL) of one or more adjacent C string literals."""
    lit = literal(lit) or '""'
    n = 0
    for part in re.findall(r'"((?:[^"\\]|\\.)*)"', lit):
        i = 0
        while i < len(part):
            if part[i] == '\\':
                c = part[i + 1]
                if c == 'x':
                    j = i + 2
                    while j < len(part) and j < i + 4 and part[j] in '0123456789abcdefABCDEF':
                        j += 1
                    i = j
                elif c in '01234567':
                    j = i + 1
                    while j < len(part) and j < i + 4 and part[j] in '01234567':
                        j += 1
                    i = j
                else:
                    i += 2
            else:
                i += 1
            n += 1
    return n + 1


def function_at(src, pos):
    best = None
    for m in re.finditer(r'^(?:static\s+)?[\w\s\*]*?\b(\w+)\s*\([^;{)]*\)\s*\n\{', src[:pos], re.M):
        best = m.group(1)
    return best


def build():
    items = []      # (addr, length, kind, name, file, function)
    files = {}
    for p in sorted(glob.glob(os.path.join(u5.SRC, '*.c'))):
        f = os.path.basename(p)
        s = open(p, encoding='utf-8', errors='replace').read()
        files[f] = s
        if f not in u5.MODULES:
            continue
        seen = set()
        for m in list(STR_RE.finditer(s)) + [x for x in ANNOT_RE.finditer(s) if literal(x.group(2))]:
            a = int(m.group(1), 16)
            if a in seen:
                continue
            seen.add(a)
            items.append([a, c_string_len(m.group(2)), 'str', m.group(2)[:40], f, function_at(s, m.start()),
                          m.group(2).strip()])
    for m in VAR_RE.finditer(files['vars.c']):
        info = u5.name_info(m.group(1))             # a global with an address (names.tsv)
        if info and info[0] == 'D':
            items.append([info[2], None, 'var', m.group(1), 'vars.c', None, None])
    items.sort(key=lambda x: (x[0], x[2] != 'var'))
    # extents of variables: up to the next item
    for i, it in enumerate(items):
        if it[1] is None:
            nxt = next((x[0] for x in items[i + 1:] if x[0] > it[0]), None)
            it[1] = (nxt - it[0]) if nxt else 2
    # users of each variable
    users = {}
    for it in items:
        if it[2] == 'var':
            short = re.match(r'D_[0-9a-f]{4}', it[3]).group(0)
            users[it[3]] = sorted(f for f, s in files.items() if f != 'vars.c' and f in u5.MODULES
                                  and re.search(r'\b%s\w*\b' % short, s))
    return items, users


def module_run(cfile, items):
    """(start, end) of a module's data: the largest cluster of its own strings (a module
    sometimes uses a string that lives in another module's data; gaps under 0x400)."""
    own = sorted((it for it in items if it[2] == 'str' and it[4] == cfile), key=lambda x: x[0])
    if not own:
        return None
    runs, cur = [], [own[0]]
    for x in own[1:]:
        if x[0] - (cur[-1][0] + cur[-1][1]) < 0x400:
            cur.append(x)
        else:
            runs.append(cur)
            cur = [x]
    runs.append(cur)
    run = max(runs, key=len)
    return run[0][0], run[-1][0] + run[-1][1]


def module_extents(items):
    """{file: [start, end)} from the strings, each running to the next module's start."""
    first = {}
    for a, l, kind, name, f, fn, lit in items:
        if kind == 'str' and f != 'vars.c':
            first.setdefault(f, a)
    order = sorted(first.items(), key=lambda kv: kv[1])
    ext = {}
    for i, (f, a) in enumerate(order):
        end = order[i + 1][1] if i + 1 < len(order) else None
        ext[f] = [a, end]
    return ext


def owner(addr, ext):
    for f, (a, e) in ext.items():
        if a <= addr < (e or 0x10000):
            return f
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--module')
    ap.add_argument('--at')
    a = ap.parse_args()
    items, users = build()
    ext = module_extents(items)
    if a.at:
        x = int(a.at, 16)
        for it in items:
            if it[0] <= x + 0x40 and it[0] + it[1] >= x - 0x40:
                print('%04x %4d %-3s %-40s %s %s' % (it[0], it[1], it[2], it[3], it[4], it[5] or ''))
        return
    if a.module:
        f = os.path.basename(a.module)
        s, e = ext[f]
        print('%s: DS %04x-%s' % (f, s, '%04x' % e if e else 'end'))
        for it in items:
            if s <= it[0] < (e or 0x10000):
                u = ('used by ' + ','.join(users[it[3]])) if it[2] == 'var' else ''
                print('  %04x %4d %-3s %-44s %-14s %s' % (it[0], it[1], it[2], it[3], it[5] or '', u))
        return
    print('module extents in DGROUP (from their strings):')
    for f, (s, e) in sorted(ext.items(), key=lambda kv: kv[1][0]):
        vs = [it for it in items if it[2] == 'var' and s <= it[0] < (e or 0x10000)]
        mine = [v for v in vs if users[v[3]] == [f]]
        print('  %-12s %04x-%s  %3d vars inside, %3d used only by it' % (f, s, '%04x' % e if e else 'end ', len(vs), len(mine)))
    lo = min(s for s, e in ext.values())
    pre = [it for it in items if it[2] == 'var' and it[0] < lo]
    print('variables before the first module string (DS < %04x): %d' % (lo, len(pre)))


if __name__ == '__main__':
    main()
