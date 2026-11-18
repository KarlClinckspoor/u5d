#!/usr/bin/env python3
"""Q1 on u5d, the whole program: remove the three pool devices (ULTIMA_1850's padding lines,
TOWN_0f02's dead references, cmds.c's short link names), add K tentative definitions of
L-character names after the last #include of EVERY C file of the DOS build (as if the
original's shared header had defined K more names), compile everything and compare every
function with the original.

    python research/rules/kall.py K [K ...] [--L 8] [--kind tent|static] [--keep-devices] [--files a.c b.c]

Prints, per K, the number of functions that are not EXACT and their names.
Nothing in src/ or build/obj is touched.
"""
import argparse, os, shutil, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
import u5  # noqa: E402
import dosbuild  # noqa: E402
import match  # noqa: E402
from kscan import remove_device, add_names  # noqa: E402

DEVICES = ('r1850.c', 'town.c', 'cmds.c')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ks', type=int, nargs='+')
    ap.add_argument('--L', type=int, default=8)
    ap.add_argument('--kind', default='tent', choices=['tent', 'static'])
    ap.add_argument('--keep-devices', action='store_true')
    ap.add_argument('--files', nargs='*')
    a = ap.parse_args()
    files = a.files or sorted(u5.MODULES)
    gamedata = u5.find_gamedata(None)
    work = tempfile.mkdtemp(prefix='kall_', dir=rk.newdir('ka'))
    src_copy = os.path.join(work, 'src')
    shutil.copytree(u5.SRC, src_copy, ignore=shutil.ignore_patterns('*.OBJ', '*.obj', '*.COD', '*.cod'))
    base = {}
    for f in files:
        raw = open(os.path.join(u5.SRC, f), encoding='utf-8', newline='').read()
        t = raw.replace('\r\n', '\n')
        if f in DEVICES and not a.keep_devices:
            t = remove_device(f, t)
        base[f] = (t, '\r\n' in raw)
    u5.OBJ = os.path.join(work, 'obj')
    os.makedirs(u5.OBJ)
    rtsyms = match.runtime_symbols()
    try:
        for k in a.ks:
            for f, (t, crlf) in base.items():
                t2 = add_names(t, k, a.L, a.kind, 'first') if '#include' in t else t
                open(os.path.join(src_copy, f), 'w', encoding='utf-8', newline='').write(t2.replace('\n', '\r\n') if crlf else t2)
            res = dosbuild.compile_files(files, None, quiet=True, src=src_copy)
            failed = [f for f in files if res.get(f)]
            bad = []
            total = 0
            for f in files:
                if f in failed:
                    continue
                rs = match.compare(match.Module(f, gamedata), rtsyms)
                total += len(rs)
                bad += [r['func'] for r in rs if r['verdict'] != 'EXACT']
            print('K=%4d L=%d: %d functions compared, %d not EXACT%s: %s' % (
                k, a.L, total, len(bad), (' (did not compile: %s)' % ' '.join(failed)) if failed else '',
                ' '.join(b[:16] for b in bad)), flush=True)
    finally:
        shutil.rmtree(os.path.dirname(work), ignore_errors=True)


if __name__ == '__main__':
    main()
