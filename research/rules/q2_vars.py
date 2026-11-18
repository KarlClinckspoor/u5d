#!/usr/bin/env python3
"""Q2 on u5d: predict VARS.OBJ's COMDEF order from vars.h with the rule of research/RULES.md,
and say what u5.COMMUNALS (the original's allocation order) would need.

    python research/rules/q2_vars.py

The rule: every symbol-table entry C1 creates gets the next serial number s; a communal is
written in slot (s mod 64) order, slots ascending; within a slot by the sum of its name's
characters with bit 5 cleared (c & 0xDF), modulo 256, ascending; equal sums: the later
declared first. vars.h declares the BSS variables one per line with nothing between them, so
their serials are consecutive and only the slot of the first one (an offset) is unknown.
"""
import os, re, shutil, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
import u5  # noqa: E402
import omf  # noqa: E402
import dosbuild  # noqa: E402


def key(n):
    return sum(ord(c) & 0xDF for c in n) % 256


def order(names, slot0, serials=None):
    serials = serials or list(range(len(names)))
    return [names[i].upper() for i in sorted(range(len(names)), key=lambda i: ((serials[i] + slot0) % 64, key(names[i]), -i))]


def main():
    work = tempfile.mkdtemp(prefix='q2vars_', dir=rk.newdir('v'))
    u5.OBJ = work
    dosbuild.compile_files(['vars.c'], None, quiet=True)
    cd = list(omf.Object(os.path.join(work, 'VARS.OBJ')).comdefs)
    shutil.rmtree(os.path.dirname(work), ignore_errors=True)
    decl = re.findall(r'^BSS\s+[^;]*?\b(D_[0-9a-f]{4}\w*)', open(os.path.join(u5.SRC, 'vars.h')).read(), re.M)
    hits = [s for s in range(64) if order(decl, s) == cd]
    print('VARS.OBJ: %d COMDEF records; predicted exactly with slot(first BSS) = %s' % (len(cd), hits or 'NONE'))
    print('u5.COMMUNALS equals VARS.OBJ order: %s' % (list(u5.COMMUNALS) == cd))
    # how far is VARS.OBJ's order from u5.COMMUNALS (which is address order)?
    want = list(u5.COMMUNALS)
    pos = {n: i for i, n in enumerate(want)}
    print('VARS.OBJ order as positions in u5.COMMUNALS:')
    print(' ', ' '.join(str(pos.get(n, '?')) for n in cd))
    # the shared ones of the original (a9bc-bb17) in one object: same slot rule; they come out in
    # address order if their serials, taken mod 64, rise (after the slot offset) with no wrap
    shared = [n for n in want if int(n[2:6], 16) < 0xbb18]
    print('shared communals (a9bc-bb17): %d; any declaration order in which they are consecutive '
          'entries starting at slot 0..%d gives address order' % (len(shared), 64 - len(shared)))


if __name__ == '__main__':
    main()
