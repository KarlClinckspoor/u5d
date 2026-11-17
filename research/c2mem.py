#!/usr/bin/env python3
"""Compile a module with limited DOS memory for pass 2 (C2.EXE) and compare with the original.

    python research/c2mem.py FILE.c FUNCTION FROM TO [STEP]     (paragraphs; 64 = 1 KB)
    python research/c2mem.py FILE.c FUNCTION 15152              (one value)

Pass 2 of MSC 5.0/5.1 optimizes a function in regions: it reads statements into a tree pool
and, when the pool is full, optimizes and emits what it has and goes on with an empty pool
(the "optimizer capacity" of docs/MATCHING.md). Where the cuts fall decides which loops get
registers. The pool's size depends on the DOS memory C2 finds free, so the original's code
can depend on how much memory the build machine had left at that compile. This tool runs
CL as usual but puts a small program in C2.EXE's place that allocates all DOS memory but
the given number of paragraphs and then runs the real C2. For each value it prints the
function's verdict and the other functions of the module that are not EXACT; a value is
printed only when the result changes. Nothing in src/ or build/obj is touched.

With plenty of memory (above about 249 KB free for C2 under 5.0) the pool has its full size
and the code does not change; below about 220 KB pass 2 runs out of heap.
"""
import os, shutil, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import u5  # noqa: E402
import dosbuild  # noqa: E402
import match  # noqa: E402

WORK = os.path.join(u5.BUILD, 'c2mem')
HELPER = os.path.join(WORK, 'C2LIM.EXE')
BS = chr(92)

HELPER_C = r'''#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <dos.h>
/* In C2.EXE's place: leaves only the paragraphs given in C:\C2MEM.TXT free, runs C:\RB\C2.EXE */
int main(int argc, char **argv)
{
    unsigned seg, max, want;
    FILE *f;
    char buf[20];
    f = fopen("C:/C2MEM.TXT", "r");
    if (f) {
        fgets(buf, 20, f);
        fclose(f);
        want = (unsigned)atol(buf);
        _dos_allocmem(0xffff, &max);        /* fails: max is the largest free block */
        if (want && max > want + 1)
            _dos_allocmem(max - want - 1, &seg);
    }
    return spawnv(P_WAIT, "C:/RB/C2.EXE", argv);
}
'''

real_run = subprocess.run
limit = [0]


def dosbox(cdir, ddir, bat):
    open(os.path.join(cdir, 'GO.BAT'), 'w', newline='').write('\r\n'.join(bat) + '\r\n')
    conf = os.path.join(cdir, 'dosbox.conf')
    open(conf, 'w').write('[sdl]\noutput=surface\n[dosbox]\nmemsize=16\n[cpu]\ncycles=max\n[log]\nlogfile=\n'
                          '[autoexec]\nmount c "%s"\nmount d "%s"\nc:\ncall %sGO.BAT\nexit\n' % (cdir, ddir, BS))
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
    real_run([u5.find_dosbox(None), '-conf', conf, '-nogui', '-exit', '-silent'], env=env,
             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=600)


def build_helper():
    if os.path.exists(HELPER):
        return
    d = os.path.join(WORK, 'helper')
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, 'C2LIM.C'), 'w').write(HELPER_C)
    dosbox(d, u5.MSC51, ['set PATH=D:%sBIN' % BS, 'set INCLUDE=D:%sINCLUDE' % BS, 'set LIB=D:%sLIB' % BS,
                         'set TMP=C:%s' % BS, 'CL /AS C2LIM.C /link /NOD SLIBCR LIBH > OUT.TXT'])
    if not os.path.exists(os.path.join(d, 'C2LIM.EXE')):
        sys.exit('the helper did not build: see %s' % os.path.join(d, 'OUT.TXT'))
    shutil.copy(os.path.join(d, 'C2LIM.EXE'), HELPER)


def run_limited(cmd, **kw):
    """dosbuild's DOSBox run, with C2.EXE replaced on a private copy of the compiler's BIN."""
    conf = cmd[cmd.index('-conf') + 1]
    dos = os.path.dirname(conf)
    tools_dir = [l.split('"')[1] for l in open(conf).read().splitlines() if l.startswith('mount d')][0]
    shutil.copytree(os.path.join(tools_dir, 'BIN'), os.path.join(dos, 'BIN'))
    os.makedirs(os.path.join(dos, 'RB'))
    shutil.move(os.path.join(dos, 'BIN', 'C2.EXE'), os.path.join(dos, 'RB', 'C2.EXE'))
    shutil.copy(HELPER, os.path.join(dos, 'BIN', 'C2.EXE'))
    open(os.path.join(dos, 'C2MEM.TXT'), 'w').write('%d\n' % limit[0])
    bat = os.path.join(dos, 'BUILD.BAT')
    t = open(bat, newline='').read()
    old = 'set PATH=D:%sBIN' % BS
    assert old in t
    open(bat, 'w', newline='').write(t.replace(old, 'set PATH=C:%sBIN' % BS))
    return real_run(cmd, **kw)


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    cfile, func = os.path.basename(sys.argv[1]), sys.argv[2].upper()
    lo = int(sys.argv[3])
    hi = int(sys.argv[4]) if len(sys.argv) > 4 else lo + 1
    step = int(sys.argv[5]) if len(sys.argv) > 5 else 64
    build_helper()
    gamedata = u5.find_gamedata(None)
    u5.OBJ = os.path.join(WORK, 'obj_%s_%d' % (os.path.splitext(cfile)[0], os.getpid()))
    os.makedirs(u5.OBJ)
    dosbuild.subprocess.run = run_limited
    rtsyms = None
    prev = None
    try:
        for v in range(lo, hi, step):
            limit[0] = v
            errs = dosbuild.compile_files([cfile], None, quiet=True).get(cfile)
            rtsyms = rtsyms or match.runtime_symbols()
            if errs:
                line = 'does not compile: ' + errs[0][-50:]
            else:
                rs = match.compare(match.Module(cfile, gamedata), rtsyms)
                t = [r for r in rs if r['func'].upper() == func] or [r for r in rs if func in r['func'].upper()]
                if not t:
                    sys.exit('no function %s in %s' % (func, cfile))
                bad = [r['func'] for r in rs if r['verdict'] != 'EXACT' and r is not t[0]]
                line = '%s | others not exact: %d %s' % (match.result_line(t[0]), len(bad), ' '.join(bad[:6]))
            if line != prev:
                print('%6d (%6.2f KB) %s' % (v, v / 64.0, line), flush=True)
                prev = line
    finally:
        shutil.rmtree(u5.OBJ, ignore_errors=True)


if __name__ == '__main__':
    main()
