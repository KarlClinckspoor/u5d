#!/usr/bin/env python3
"""The table of addressed names: tools/names.tsv, one row per function or global that carries
an address, with the module that owns it.

    python tools/names.py            write tools/names.tsv from the C sources (src/*.c, *.h)
    python tools/names.py --check    check the table against the sources (the gate runs it)

A name with an address is MODULE_addr_Name (a function: ULTIMA or an overlay's module) or
D_addr_name (a global). Once the readable names came, a name carries no address; the tools
find its address here (u5.name_address, u5.name_info). A row's name is the name in the
sources; its module is the prefix (ULTIMA, TALK, ..., or D), its kind is code or data, and
its address is the hex of the name's first address field (for an overlay, the offset in
the overlay). rename.py changes the name column when it renames a row, as it renames the
sources (it is in the list of files it changes).
"""
import os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename  # noqa: E402
import u5  # noqa: E402

TSV = os.path.join(u5.HERE, 'names.tsv')
HEADER = 'name\tkind\tmodule\taddress'
NAME = re.compile(r'^_?([A-Z][A-Z0-9]*)_([0-9A-Fa-f]{4})(?:_|$)')


def addressed(name):
    """(kind, module, address) if NAME is a name with an address this table holds."""
    m = NAME.match(name)
    if not m:
        return None
    pre, off = m.group(1), int(m.group(2), 16)
    if pre == 'ULTIMA':
        return ('code', 'ULTIMA', off)
    if pre == 'D':
        return ('data', 'D', off)
    if pre in u5.OVERLAY_BASE:
        return ('code', pre, off)
    return None


def c_identifiers():
    """{name: set of files} of the identifiers of the C sources (not in strings or comments)."""
    out = {}
    for p, kind in rename.source_files().items():
        if kind == 'c' and os.path.dirname(p) == u5.SRC:
            for n in rename.identifiers(rename.read(p)):
                out.setdefault(n, set()).add(os.path.basename(p))
    return out


def build():
    rows, problems = {}, []
    for n in c_identifiers():
        a = addressed(n)
        if a:
            rows[n] = a
    return rows, problems


def write(rows):
    with open(TSV, 'w', encoding='utf-8', newline='\n') as f:
        f.write(HEADER + '\n')
        for n in sorted(rows, key=lambda n: (rows[n][1], rows[n][2], n)):
            kind, mod, off = rows[n]
            f.write('%s\t%s\t%s\t%04x\n' % (n, kind, mod, off))


def read_rows(path=TSV):
    rows = {}
    for line in open(path, encoding='utf-8').read().split('\n')[1:]:
        if line:
            n, kind, mod, off = line.split('\t')
            rows[n] = (kind, mod, int(off, 16))
    return rows


def check():
    problems = []
    rows = read_rows()
    ids = c_identifiers()
    for n, v in rows.items():
        if n not in ids:
            problems.append('%s is in names.tsv but not in the sources' % n)
        if n.upper()[:31] != n[:31].upper():
            problems.append('%s: key' % n)
        if NAME.match(n) and addressed(n) != v:
            problems.append('%s: the row says %r, the name says %r' % (n, v, addressed(n)))
        if not NAME.match(n) and not re.match(r'^[a-z][A-Za-z0-9]*$', n) and not re.match(r'^[A-Z][A-Za-z0-9]*$', n):
            problems.append('%s is not a name the table can give (lower or upper camel)' % n)
    for n in ids:
        a = addressed(n)
        if a and rows.get(n) != a:
            problems.append('%s carries address %r and is not in names.tsv' % (n, a))
    seen, stubs = {}, {}
    for n, v in rows.items():
        k = n[:31].upper()
        if k in seen and seen[k] != n:
            problems.append('%s and %s are one symbol (31 characters, upper case)' % (seen[k], n))
        seen[k] = n
        if NAME.match(n) and addressed(n) is None:
            problems.append('%s: prefix-like name the table cannot place' % n)
        if v[0] == 'code' and v[1] != 'ULTIMA':
            # an overlay function's call stub is STUB_ and 26 characters of its name (u5link)
            s = n[:26].upper()
            if s in stubs and stubs[s] != n:
                problems.append('%s and %s have one call stub (STUB_%s)' % (stubs[s], n, s))
            stubs[s] = n
    return problems


def main():
    if '--check' in sys.argv:
        problems = check()
        for p in problems:
            print('problem: ' + p)
        print('names.tsv: %d rows, %s' % (len(read_rows()), 'OK' if not problems else '%d problems' % len(problems)))
        sys.exit(1 if problems else 0)
    rows, _ = build()
    write(rows)
    print('names.tsv: %d rows (%d code, %d data)' % (len(rows), sum(1 for v in rows.values() if v[0] == 'code'),
                                                 sum(1 for v in rows.values() if v[0] == 'data')))


if __name__ == '__main__':
    main()
