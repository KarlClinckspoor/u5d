#!/usr/bin/env python3
"""Q1 on u5d: compile a module with its pool device removed and K extra defined globals at its
top (as if the original's headers defined more names), and compare every function.

    python research/rules/kscan.py FILE.c K [K ...] [--L 8] [--keep-device] [--paras N] [--kind static|tent|ref]

FILE.c: r1850.c, town.c or cmds.c (the device of each is removed unless --keep-device).
Each K adds K names of L characters after the module's last #include:
    static  `static int Qnnnn;`             (a defined symbol; costs as tentative/initialised ones)
    tent    `int Qnnnn;`                    (tentative definition; becomes a COMDEF)
    ref     `extern int Qnnnn;` plus one dead-reference line `if (0) { Q.. = 0; ... }` at the top of
            the module's first function (the TOWN_0f02 device, moved to the start of the module)
Prints, per K, the verdict of each device function and the other functions that are not EXACT.
Nothing in src/ or build/obj is touched (a copy of src/ and a private object folder).
"""
import argparse, os, re, shutil, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk  # noqa: E402
import u5  # noqa: E402
import dosbuild  # noqa: E402
import match  # noqa: E402

TARGET = {'r1850.c': 'ULTIMA_1850', 'town.c': 'TOWN_0F02', 'cmds.c': 'CMDS_0000'}


def remove_device(cfile, t):
    if cfile == 'r1850.c':
        a = t.index("    // Padding: each of these lines is one more unit in pass 2's pool (see above).\n")
        b = a
        lines = t[a:].split('\n')
        n = 1
        while lines[n].strip() == '//':
            n += 1
        return t[:a] + '\n'.join(lines[n:])
    if cfile == 'town.c':
        a = t.index('#if defined(TARGET_DOS16)\n    // pass 2 memory fill')
        b = t.index('#endif\n', a) + len('#endif\n')
        return t[:a] + t[b:]
    if cfile == 'cmds.c':
        a = t.index('#if defined(TARGET_DOS16)\n#define D_5876_tempX D_5876')
        b = t.index('#endif\n', a) + len('#endif\n')
        return t[:a] + t[b:]
    raise SystemExit('no device known for %s' % cfile)


def first_body(t):
    """index just after the opening brace line of the first function definition"""
    m = re.search(r'^\w[\w \*]*\b(\w+)\s*\([^;{]*\)\s*\n\{\n', t, re.M)
    return m.end()


def add_names(t, k, L, kind, where='last'):
    if k == 0:
        return t
    names = ['Q%0*d' % (L - 1, i) for i in range(k)]
    ends = [m.end() for m in re.finditer(r'^#include .*\n', t, re.M)]
    last_inc = max(ends) if where == 'last' else min(ends)
    if kind == 'static':
        block = ''.join('static int %s;\n' % n for n in names)
    elif kind == 'tent':
        block = ''.join('int %s;\n' % n for n in names)
    else:
        block = ''.join('extern int %s;\n' % n for n in names)
    t = t[:last_inc] + block + t[last_inc:]
    if kind == 'ref':
        i = first_body(t)
        # after the declarations of the first function: the first line that is not a declaration
        body = t[i:]
        lines = body.split('\n')
        j = 0
        while j < len(lines) and (re.match(r'\s+(register\s+)?[\w\*]+[\w\s\*,\[\]]*;\s*(//.*)?$', lines[j]) and
                                  not re.search(r'=|\(', lines[j]) or lines[j].strip() == ''):
            j += 1
        lines.insert(j, '    if (0) { %s }' % ' '.join('%s = 0;' % n for n in names))
        t = t[:i] + '\n'.join(lines)
    return t


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('file')
    ap.add_argument('ks', type=int, nargs='+')
    ap.add_argument('--L', type=int, default=8)
    ap.add_argument('--kind', default='static', choices=['static', 'tent', 'ref'])
    ap.add_argument('--keep-device', action='store_true')
    ap.add_argument('--target')
    a = ap.parse_args()
    cfile = os.path.basename(a.file)
    target = (a.target or TARGET.get(cfile, '')).upper()
    gamedata = u5.find_gamedata(None)
    work = tempfile.mkdtemp(prefix='kscan_', dir=rk.newdir('k'))
    src_copy = os.path.join(work, 'src')
    shutil.copytree(u5.SRC, src_copy, ignore=shutil.ignore_patterns('*.OBJ', '*.obj', '*.COD', '*.cod'))
    raw = open(os.path.join(u5.SRC, cfile), encoding='utf-8', newline='').read()
    crlf = '\r\n' in raw
    base = raw.replace('\r\n', '\n')
    if not a.keep_device:
        base = remove_device(cfile, base)
    u5.OBJ = os.path.join(work, 'obj')
    os.makedirs(u5.OBJ)
    rtsyms = match.runtime_symbols()
    try:
        for k in a.ks:
            t = add_names(base, k, a.L, a.kind)
            open(os.path.join(src_copy, cfile), 'w', encoding='utf-8', newline='').write(t.replace('\n', '\r\n') if crlf else t)
            errs = dosbuild.compile_files([cfile], None, quiet=True, src=src_copy).get(cfile)
            if errs:
                print('K=%4d does not compile: %s' % (k, errs[0][-80:]), flush=True)
                continue
            rs = match.compare(match.Module(cfile, gamedata), rtsyms)
            tg = [r for r in rs if r['func'].upper().startswith(target)] if target else []
            bad = [r['func'] for r in rs if r['verdict'] != 'EXACT' and r not in tg]
            print('K=%4d %s | others not exact: %d %s' % (k, match.result_line(tg[0]) if tg else '-', len(bad),
                                                       ' '.join(b[:12] for b in bad[:8])), flush=True)
    finally:
        shutil.rmtree(work, ignore_errors=True)


if __name__ == '__main__':
    main()
