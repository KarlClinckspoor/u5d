#!/usr/bin/env python3
"""Give an MSC 5.0 module its own data: tables from vars.c, strings as literals.

    python tools/own50.py FILE.c FIRST_TABLE LAST_TABLE [--apply]

In an MSC 5.0 module the string literals go to CONST and the tables to _DATA, each in
source order (docs/MATCHING.md, "Two compilers"). u5d had such modules' strings as
generated char arrays (tools/strings.py) and their tables in vars.c. This:

  - cuts every vars.c definition from FIRST_TABLE to LAST_TABLE (DS order, inclusive; the
    module's _DATA, found from which module uses each table) and puts them into FILE.c;
  - removes the generated string arrays, so _TEXT/_STATIC_TEXT are plain literals again;
  - places the tables so that CONST comes out in the original order: a table whose
    initialiser holds strings goes before the first function whose strings follow its
    own in DS; tables without strings go with the table that follows them (or with the
    last string table).

vars.h keeps the extern declarations. Check with tools/match.py afterwards: the module's
_DATA and CONST should both say "same".
"""
import argparse, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402
import movevar  # noqa: E402
import slots  # noqa: E402

DEF = re.compile(r'^(?:static\s+)?(?:const\s+)?[A-Za-z_]\w*\*?\s+\*?(\w+)((?:\[[^\]]*\])*)\s*(=|;)', re.M)
ADDR = re.compile(r'(?:_(?:STATIC_)?TEXT\(|/\*)0x([0-9a-fA-F]{4})')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('file')
    ap.add_argument('first')
    ap.add_argument('last')
    ap.add_argument('--apply', action='store_true')
    a = ap.parse_args()
    vp, mp = os.path.join(u5.SRC, 'vars.c'), os.path.join(u5.SRC, a.file)
    vs, ms = movevar.read(vp), movevar.read(mp)
    nl = '\r\n' if '\r\n' in ms else '\n'
    lo = int(re.match(r'D_([0-9a-f]{4})', a.first).group(1), 16)
    hi = int(re.match(r'D_([0-9a-f]{4})', a.last).group(1), 16)
    names = sorted({(info[2], m.group(1)) for m in DEF.finditer(vs)
                    for info in [u5.name_info(m.group(1))] if info and info[0] == 'D' and lo <= info[2] <= hi})

    # cut the definitions from vars.c
    tables = []
    for addr, name in names:
        d = movevar.find_definition(vs, name)
        block = vs[d[0]:d[1]]
        vs = vs[:d[0]] + vs[d[1]:]
        tables.append((addr, name, block.replace('\r\n', '\n').rstrip('\n')))

    # the generated string arrays go: literals again
    s = ms.replace('\r\n', '\n')
    b0 = s.find('// BEGIN DGROUP')
    if b0 >= 0:
        b1 = s.index('// END DGROUP', b0) + len('// END DGROUP')
        keep = [l for l in s[b0:b1].split('\n')
                if not re.match(r'\s*#(undef|define)\s+_(STATIC_)?TEXT\b', l)
                and not re.match(r'\s*(static\s+)?char S_0x[0-9a-f]+\[\d*\] = .*;$', l)]
        body = [l for l in keep if l.strip() and not l.strip().startswith('//')]
        if body:
            s = s[:b0] + '\n'.join(keep) + s[b1:]
        else:
            pre = s.rfind('#if defined(TARGET_DOS16)', 0, b0)
            post = s.index('#endif', b1) + len('#endif')
            s = s[:pre] + s[post:]
    if re.search(r'^\s*(static\s+)?char S_0x', s, re.M):
        sys.exit('%s: string arrays left that this does not recognise' % a.file)

    # where each table goes: by the first DS string address of each function
    funcs = []
    for name, fa, fb in slots.functions(s):
        addrs = [int(x, 16) for x in ADDR.findall(s[fa:fb])]
        funcs.append((name, min(addrs) if addrs else None))
    plan = {}            # function name (or None: end of file) -> [table blocks]
    pending = []
    last_target = None
    for addr, name, block in tables:
        pending.append(block)
        saddrs = [int(x, 16) for x in ADDR.findall(block)]
        if not saddrs:
            continue
        first = min(saddrs)
        target = next((f for f, fa in funcs if fa is not None and fa > first), None)
        plan.setdefault(target, []).extend(pending)
        pending, last_target = [], target
    if pending:
        plan.setdefault(last_target if last_target is not None or plan else
                        (funcs[0][0] if funcs else None), []).extend(pending)

    for target, blocks in plan.items():
        text = '\n\n'.join(blocks) + '\n\n'
        if target is None:
            s = s.rstrip('\n') + '\n\n' + text.rstrip('\n') + '\n'
        else:
            at = movevar.function_start(s, target)
            s = s[:at] + text + s[at:]
        print('%-34s <- %s' % (target or '(end of file)',
                               ' '.join(re.search(r'\b(D_[0-9a-f]{4}\w*)', b).group(1) for b in blocks)))
    s = re.sub(r'\n{3,}', '\n\n', s)
    if a.apply:
        movevar.write(mp, s.replace('\n', nl))
        movevar.write(vp, vs)
        print('moved %d tables from vars.c into %s' % (len(tables), a.file))


if __name__ == '__main__':
    main()
