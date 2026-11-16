#!/usr/bin/env python3
"""Assemble the assembly modules (src/asm/*.ASM) with MASM 4.0 in DOSBox-X, and check them.

    python tools/asmbuild.py [NAME.ASM ...]      assemble (default: all) and compare

Each module's first line says which range of ULTIMA.EXE it is ("CS:xxxx-yyyy"). Its _TEXT is
compared with those bytes, fixups masked and checked against the names u5d's symbols carry.
MASM 4.0 (MASM.EXE, from the sibling U1 project or your own copy) is expected in build/masm.
"""
import argparse, glob, os, re, shutil, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402

MASM_DIR = os.path.join(u5.BUILD, 'masm')


def assemble_many(paths, dosbox=None):
    """{path: (Object or None, log)}"""
    if not os.path.exists(os.path.join(MASM_DIR, 'MASM.EXE')):
        sys.exit('MASM 4.0 is missing: copy MASM.EXE into build/masm')
    root = os.path.join(u5.BUILD, 'dos')
    os.makedirs(root, exist_ok=True)
    d = tempfile.mkdtemp(prefix='asm', dir=root)
    lines = ['@echo off']
    for p in paths:
        n = os.path.splitext(os.path.basename(p))[0].upper()
        shutil.copy(p, os.path.join(d, n + '.ASM'))
        lines.append('D:\\MASM /Mx %s.ASM,%s.OBJ,%s.LST; > %s.LOG' % (n, n, n, n))
    open(os.path.join(d, 'B.BAT'), 'w', newline='').write('\r\n'.join(lines) + '\r\n')
    conf = os.path.join(d, 'c.conf')
    open(conf, 'w').write('[sdl]\noutput=surface\n[cpu]\ncycles=max\n[log]\nlogfile=\n[autoexec]\n'
                          'mount c "%s"\nmount d "%s"\nc:\ncall B.BAT\nexit\n' % (d, MASM_DIR))
    subprocess.run([u5.find_dosbox(dosbox), '-conf', conf, '-nogui', '-exit', '-silent'],
                   env=dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy'),
                   capture_output=True, timeout=600)
    out = {}
    objdir = os.path.join(u5.BUILD, 'obj')
    os.makedirs(objdir, exist_ok=True)
    for p in paths:
        n = os.path.splitext(os.path.basename(p))[0].upper()
        log = open(os.path.join(d, n + '.LOG'), encoding='latin-1').read() if os.path.exists(os.path.join(d, n + '.LOG')) else ''
        lst = os.path.join(d, n + '.LST')
        if os.path.exists(lst):                  # each error with the line it is about
            L = open(lst, encoding='latin-1').read().splitlines()
            log += ''.join('%s\n    %s\n' % (L[i - 1].strip()[:100], L[i].strip())
                           for i in range(1, len(L)) if re.search(r'error \d+:', L[i]))
        o = os.path.join(d, n + '.OBJ')
        ok = os.path.exists(o) and re.search(r'\b0 Severe', log)
        if ok:
            shutil.copy(o, os.path.join(objdir, n + '.OBJ'))
        out[p] = (omf.Object(o) if ok else None, log)
    shutil.rmtree(d, ignore_errors=True)
    return out


def assemble(path, dosbox=None):
    return assemble_many([path], dosbox)[path]


def module_range(path):
    m = re.search(r'CS:([0-9a-f]{4})-([0-9a-f]{4})', open(path, encoding='latin-1').readline())
    return (int(m.group(1), 16), int(m.group(2), 16)) if m else None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('files', nargs='*')
    a = ap.parse_args()
    paths = [p if os.path.exists(p) else os.path.join(u5.SRC, 'asm', p) for p in a.files] or \
        sorted(glob.glob(os.path.join(u5.SRC, 'asm', '*.ASM')))
    img = u5.exe_image(u5.find_gamedata())
    bad = 0
    for p, (obj, log) in assemble_many(paths).items():
        r = module_range(p)
        if obj is None:
            print('%-14s MASM failed:\n%s' % (os.path.basename(p), log))
            bad += 1
            continue
        seg = next(s for s in obj.segs[1:] if s.cls == 'CODE' and s.length)
        orig = img[r[0]:r[1]]
        masked = {k for fx in seg.fixups for k in range(fx.loc, fx.loc + fx.size)}
        diffs = [k for k in range(min(len(orig), seg.length)) if k not in masked and seg.data[k] != orig[k]]
        same = not diffs and seg.length == len(orig)
        print('%-14s CS:%04x-%04x %5d bytes  %s' % (os.path.basename(p), r[0], r[1], len(orig),
                                                    'identical' if same else '%d differ, length %d' % (len(diffs), seg.length)))
        bad += not same
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
