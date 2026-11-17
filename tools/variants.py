#!/usr/bin/env python3
"""Try several variants of a source file against the original, without touching src/.

    python tools/variants.py src/FILE.c SPEC.py FUNCTION [--keep N] [--only N ...] [--notes]

SPEC.py defines VARIANTS, a list of (name, PAIRS). PAIRS is a list of (old, new) strings;
each old text must occur exactly once in the file (compared with LF line ends):

    VARIANTS = [
        ('unsigned i', [('int i;', 'unsigned i;')]),
        ('while loop', [('for (i = 0; i < 8; i++) {', 'i = 0;\\n    while (i < 8) {'),
                        ('        n += t[i];\\n    }', '        n += t[i];\\n        i++;\\n    }')]),
    ]

Each variant is applied to the file as it is in src/, compiled and compared, and the line
of FUNCTION is printed as match.py -v prints it. A variant with no pairs is the file as it is.

Nothing is written to src/, build/obj or build/match/status.tsv: the tool works in a
folder of its own under build/variants (a copy of src/ and a private object folder), which
it removes at the end. So any number of these can run at the same time, on the same file too.

--keep N writes variant N into src/FILE.c, after showing its line. Only then is src/
changed: the pairs are applied to the file as it is at that moment, so a change somebody
else kept in the meantime stays. build/obj is not updated: run match.py FILE.c --compile.
"""
import argparse, os, shutil, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402


def apply_pairs(text, pairs):
    """(new text, None) or (None, why not)."""
    for old, new in pairs:
        n = text.count(old)
        if n != 1:
            return None, 'PATTERN COUNT %d %r' % (n, old[:60])
        text = text.replace(old, new)
    return text, None


def read(path):
    """(text with LF line ends, True if the file has CRLF)."""
    raw = open(path, encoding='utf-8', newline='').read()
    return raw.replace('\r\n', '\n'), '\r\n' in raw


def write(path, text, crlf):
    open(path, 'w', encoding='utf-8', newline='').write(text.replace('\n', '\r\n') if crlf else text)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('file', help='src/FILE.c')
    ap.add_argument('spec', help='a Python file defining VARIANTS')
    ap.add_argument('function')
    ap.add_argument('--keep', type=int, metavar='N', help='try variant N only, then write it into src/')
    ap.add_argument('--only', type=int, nargs='+', metavar='N', help='try these variants only')
    ap.add_argument('--notes', action='store_true', help="also print match.py's notes for the function")
    ap.add_argument('--gamedata')
    ap.add_argument('--dosbox')
    a = ap.parse_args()

    cfile = os.path.basename(a.file)
    if cfile not in u5.MODULES or u5.MODULES[cfile][0] == 'data':
        sys.exit('%s is not a code module of the DOS build (tools/u5.py MODULES)' % cfile)
    real_src = u5.SRC
    path = os.path.join(real_src, cfile)
    ns = {}
    exec(compile(open(a.spec, encoding='utf-8').read(), a.spec, 'exec'), ns)
    variants = ns['VARIANTS']
    which = [a.keep] if a.keep is not None else (a.only or range(len(variants)))
    for i in which:
        if not 0 <= i < len(variants):
            sys.exit('there is no variant %d (%d variants)' % (i, len(variants)))
    gamedata = u5.find_gamedata(a.gamedata)
    base, crlf = read(path)

    root = os.path.join(u5.BUILD, 'variants')
    os.makedirs(root, exist_ok=True)
    tmp = tempfile.mkdtemp(prefix='v', dir=root)
    try:
        # a private copy of the sources and a private object folder; the tools read u5.SRC
        # and u5.OBJ, so from here on this process compiles and compares only there
        u5.SRC = os.path.join(tmp, 'src')
        u5.OBJ = os.path.join(tmp, 'obj')
        shutil.copytree(real_src, u5.SRC, ignore=shutil.ignore_patterns('*.OBJ', '*.obj', '*.COD', '*.cod'))
        os.makedirs(u5.OBJ)
        import dosbuild
        import match
        # the other object of a two-object overlay tells where this one's functions end
        kind, ovl = u5.MODULES[cfile]
        others = [f for f, (k, o) in u5.MODULES.items() if kind == 'ovl' and k == 'ovl' and o == ovl and f != cfile]
        if others:
            res = dosbuild.compile_files(others, a.dosbox, quiet=True)
            for f in others:
                if res.get(f):
                    print('%s (same overlay) does not compile: %s' % (f, res[f][0]))
        rtsyms = match.runtime_symbols()
        kept = None
        for i in which:
            name, pairs = variants[i]
            text, why = apply_pairs(base, pairs)
            if text is None:
                print(i, name, why, flush=True)
                continue
            write(os.path.join(u5.SRC, cfile), text, crlf)
            errs = dosbuild.compile_files([cfile], a.dosbox, quiet=True).get(cfile)
            if errs:
                print(i, name, '|', 'COMPILE ERROR', ' / '.join(errs[:3]), flush=True)
                continue
            res = [r for r in match.compare(match.Module(cfile, gamedata), rtsyms)
                   if a.function.upper() in r['func'].upper()]
            res = [r for r in res if r['func'].upper() == a.function.upper()] or res   # the whole name, if given
            if not res:
                print(i, name, '|', 'no function %s in the object' % a.function, flush=True)
                continue
            print(i, name, '|', ' / '.join(match.result_line(r) for r in res), flush=True)
            if a.notes:
                for r in res:
                    for n in r['notes']:
                        print('        ' + n)
            kept = i
        if a.keep is not None and kept == a.keep:
            now, crlf_now = read(path)
            text, why = apply_pairs(now, variants[a.keep][1])
            if text is None:
                sys.exit('%s has changed and variant %d no longer applies: %s' % (path, a.keep, why))
            if now != base:
                print('%s was changed by somebody else meanwhile: variant %d is applied on top, '
                      'and what was compared is not what is there now' % (path, a.keep))
            write(path, text, crlf_now)
            print('kept variant %d in %s (build/obj is not updated: run match.py %s --compile)' % (a.keep, path, cfile))
        elif a.keep is not None:
            sys.exit('variant %d was not kept' % a.keep)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == '__main__':
    main()
