#!/usr/bin/env python3
"""Insert line records into pass 1's output and run pass 2 on it: where do the pool's cuts help?

    python research/c2lines.py FILE.c FUNCTION LINE [FROM TO STEP] [--kb N]

LINE is the source line of the function's opening brace. The tool compiles FILE.c once with
CL, keeping pass 1's intermediate files (a stand-in for C1/C2/C3.EXE copies them out of
TMP), then for n = FROM, FROM+STEP, ... inserts n line records after the function's entry,
runs C2.EXE and C3.EXE directly on the edited files and compares the object with the
original. A result is printed only when it changes. --kb N leaves only N KB of DOS memory to
C2. Nothing in src/ or build/obj is touched.

What this is for (docs/MATCHING.md, the optimizer's capacity): pass 2 optimizes a function
in regions, cutting when its tree pool is full; every non-blank source line inside the
function is one line record and takes one unit of the pool. Inserting records moves the
cuts through the function without touching the code, so the scan shows whether any position
of a cut gives the original's code (ULTIMA_1850: n = 123..563, period 612 under MSC 5.0).

The intermediate files: <base>EX holds the expression trees with the line records between
them, "4F 01 nn" for a line below 128 and "4F 01 80 lo hi" otherwise; SY the symbols.
CL passes each pass its arguments in the environment variable MSC_CMD_FLAGS.
"""
import argparse, os, shutil, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import u5  # noqa: E402
import dosbuild  # noqa: E402
import match  # noqa: E402

WORK = os.path.join(u5.BUILD, 'c2lines')
BS = chr(92)

# In place of C1/C2/C3.EXE: runs the real pass from C:\RB, then copies C:\TMP to C:\KEEPn.
SPY_C = r'''#include <stdio.h>
#include <process.h>
#include <string.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
    char real[80], cmd[80], *p, *q;
    int r;
    p = strrchr(argv[0], '/');
    q = strrchr(argv[0], 92);
    if (q > p) p = q;
    p = p ? p + 1 : argv[0];
    strcpy(real, "C:/RB/");
    strcat(real, p);
    r = spawnv(P_WAIT, real, argv);
    strcpy(cmd, "copy C:\\TMP\\*.* C:\\KEEPx");
    cmd[23] = p[1];
    spawnl(P_WAIT, "Z:\\COMMAND.COM", "COMMAND", "/C", cmd, NULL);
    return r;
}
'''
# HOG paragraphs program: leaves only that much DOS memory free, then runs the program.
HOG_C = r'''#include <process.h>
#include <stdlib.h>
#include <dos.h>
int main(int argc, char **argv)
{
    unsigned seg, max, want;
    want = (unsigned)atol(argv[1]);
    _dos_allocmem(0xffff, &max);
    if (max > want + 1)
        _dos_allocmem(max - want - 1, &seg);
    return spawnv(P_WAIT, argv[2], argv + 2);
}
'''


def dosbox(cdir, ddir, bat):
    open(os.path.join(cdir, 'GO.BAT'), 'w', newline='').write('\r\n'.join(bat) + '\r\n')
    conf = os.path.join(cdir, 'dosbox.conf')
    open(conf, 'w').write('[sdl]\noutput=surface\n[dosbox]\nmemsize=16\n[cpu]\ncycles=max\n[log]\nlogfile=\n'
                          '[autoexec]\nmount c "%s"\nmount d "%s"\nc:\ncall %sGO.BAT\nexit\n' % (cdir, ddir, BS))
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
    subprocess.run([u5.find_dosbox(None), '-conf', conf, '-nogui', '-exit', '-silent'], env=env,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=3600)


def helper(name, source):
    exe = os.path.join(WORK, name + '.EXE')
    if not os.path.exists(exe):
        d = os.path.join(WORK, 'helper')
        os.makedirs(d, exist_ok=True)
        open(os.path.join(d, name + '.C'), 'w').write(source)
        dosbox(d, u5.MSC51, ['set PATH=D:%sBIN' % BS, 'set INCLUDE=D:%sINCLUDE' % BS, 'set LIB=D:%sLIB' % BS,
                             'set TMP=C:%s' % BS, 'CL /AS %s.C /link /NOD SLIBCR LIBH > OUT.TXT' % name])
        if not os.path.exists(os.path.join(d, name + '.EXE')):
            sys.exit('%s did not build: see %s' % (name, os.path.join(d, 'OUT.TXT')))
        shutil.copy(os.path.join(d, name + '.EXE'), exe)
    return exe


def capture(cfile, comp):
    """Compile cfile with CL; returns pass 1's files {'EX': bytes, 'SY': ..., 'IN': ..., 'ST': ...}."""
    tools_dir = dosbuild.compiler_dir(comp)
    d = os.path.join(WORK, 'cap_' + os.path.splitext(cfile)[0])
    shutil.rmtree(d, ignore_errors=True)
    shutil.copytree(u5.SRC, os.path.join(d, 'S'), ignore=shutil.ignore_patterns('*.OBJ', '*.obj', '*.COD', '*.cod'))
    if comp == 'msc50':
        for dp, _, fs in os.walk(os.path.join(d, 'S')):
            for fn in fs:
                if fn.lower().endswith(('.c', '.h')):
                    p = os.path.join(dp, fn)
                    t = open(p, encoding='latin-1', newline='').read()
                    open(p, 'w', encoding='latin-1', newline='').write(dosbuild.c_comments(t))
    for k in ('BIN', 'RB', 'KEEP1', 'KEEP2', 'KEEP3', 'TMP'):
        os.makedirs(os.path.join(d, k))
    spy = helper('SPY', SPY_C)
    for fn in os.listdir(os.path.join(tools_dir, 'BIN')):
        shutil.copy(os.path.join(tools_dir, 'BIN', fn), os.path.join(d, 'RB', fn))
        shutil.copy(spy if fn in ('C1.EXE', 'C2.EXE', 'C3.EXE') else os.path.join(tools_dir, 'BIN', fn),
                    os.path.join(d, 'BIN', fn))
    flags = u5.CFLAGS_BY_FILE.get(cfile, u5.CFLAGS)
    dosbox(d, tools_dir, ['set PATH=C:%sBIN' % BS, 'set INCLUDE=D:%sINCLUDE' % BS, 'set LIB=D:%sLIB' % BS,
                          'set TMP=C:%sTMP' % BS, 'cd %sS' % BS, 'CL %s /c %s > C:%sOUT.TXT' % (flags, cfile, BS)])
    out = {}
    for fn in os.listdir(os.path.join(d, 'KEEP1')):
        out[fn[-2:]] = open(os.path.join(d, 'KEEP1', fn), 'rb').read()
    if 'EX' not in out:
        sys.exit('pass 1 left no files: see %s' % os.path.join(d, 'OUT.TXT'))
    return out


def run_passes(jobs, comp, name, paragraphs=0):
    """jobs: list of file dicts. Runs C2 and C3 on each; returns the objects (None if none)."""
    d = os.path.join(WORK, 'run_%d' % os.getpid())
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(os.path.join(d, 'TMP'))
    shutil.copy(helper('HOG', HOG_C), os.path.join(d, 'HOG.EXE'))
    tmp = 'C:%sTMP%s' % (BS, BS)
    bat = ['set PATH=D:%sBIN' % BS]
    for i, files in enumerate(jobs):
        b = 'J%03d' % i
        for k, v in files.items():
            open(os.path.join(d, 'TMP', b + k), 'wb').write(v)
        c2 = 'D:%sBIN%sC2.EXE' % (BS, BS)
        bat.append('set MSC_CMD_FLAGS=-il %s%s -ef D:%sBIN%sc23.err -Oc -Ol -Oa -Ot -G0 -A snd -W 1' % (tmp, b, BS, BS))
        bat.append(('C:%sHOG.EXE %d %s' % (BS, paragraphs, c2)) if paragraphs else c2)
        bat.append('set MSC_CMD_FLAGS=-il %s%s -ef D:%sBIN%sc23.err -T 0 -NM %s -FPi -Ot -Oa -Or -c C:%s%s.OBJ '
                   '-A snd -CC 0sO' % (tmp, b, BS, BS, name, BS, b))
        bat.append('D:%sBIN%sC3.EXE' % (BS, BS))
    dosbox(d, dosbuild.compiler_dir(comp), bat)
    res = []
    for i in range(len(jobs)):
        p = os.path.join(d, 'J%03d.OBJ' % i)
        res.append(open(p, 'rb').read() if os.path.exists(p) and os.path.getsize(p) else None)
    shutil.rmtree(d, ignore_errors=True)
    return res


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('file')
    ap.add_argument('function')
    ap.add_argument('line', type=int, help="source line of the function's opening brace")
    ap.add_argument('range', type=int, nargs='*', help='FROM TO STEP (default 0 620 4)')
    ap.add_argument('--kb', type=float, default=0, help='DOS memory left free for C2, in KB')
    a = ap.parse_args()
    cfile, func = os.path.basename(a.file), a.function.upper()
    lo, hi, step = (a.range + [0, 620, 4][len(a.range):])[:3]
    stem = os.path.splitext(cfile)[0]
    comp = u5.COMPILER_BY_FILE.get(cfile, 'msc51')
    os.makedirs(WORK, exist_ok=True)
    files = capture(cfile, comp)
    ex = files['EX']
    pat = bytes([0x4f, 1, 0x80, a.line & 255, a.line >> 8]) if a.line >= 128 else bytes([0x4f, 1, a.line])
    i = ex.find(pat)
    if i < 0:
        sys.exit('no line record for line %d' % a.line)
    # after the function's entry (records before it do not count), at its next line record
    i = ex.index(bytes([0x4f, 1]), i + len(pat))
    ns = list(range(lo, hi, step))
    objs = run_passes([dict(files, EX=ex[:i] + bytes([0x4f, 1, 1]) * n + ex[i:]) for n in ns], comp, stem,
                      int(a.kb * 64))
    u5.OBJ = os.path.join(WORK, 'obj_%d' % os.getpid())
    os.makedirs(u5.OBJ)
    gamedata = u5.find_gamedata(None)
    rtsyms = match.runtime_symbols()
    prev = None
    try:
        for n, obj in zip(ns, objs):
            if obj is None:
                line = 'no object'
            else:
                open(os.path.join(u5.OBJ, stem.upper() + '.OBJ'), 'wb').write(obj)
                rs = match.compare(match.Module(cfile, gamedata), rtsyms)
                t = [r for r in rs if r['func'].upper() == func] or [r for r in rs if func in r['func'].upper()]
                if not t:
                    sys.exit('no function %s in %s' % (func, cfile))
                bad = [r['func'] for r in rs if r['verdict'] != 'EXACT' and r is not t[0]]
                line = '%s | others not exact: %d %s' % (match.result_line(t[0]), len(bad), ' '.join(bad[:6]))
            if line != prev:
                print('%5d records: %s' % (n, line), flush=True)
                prev = line
    finally:
        shutil.rmtree(u5.OBJ, ignore_errors=True)


if __name__ == '__main__':
    main()
