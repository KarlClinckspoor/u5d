#!/usr/bin/env python3
"""Compile u5d's C files with Microsoft C 5.1 in a headless DOSBox-X.

    python tools/dosbuild.py [FILE.c ...] [--dosbox PATH] [--log]

With no files, compiles every file of the DOS build (tools/u5.py, MODULES). The objects
and their listings (/Fc: source, assembly and code bytes interleaved) are written to
build/obj/NAME.OBJ and build/obj/NAME.COD.

src/ is copied to a scratch drive C: (a new folder under build/dos per run, so builds of
different files can run at the same time), the compiler in build/msc51 is mounted as
D:, and one batch file compiles everything in a single DOSBox-X session. Each file gets
the switches of src/build.bat (u5.CFLAGS, with u5.CFLAGS_BY_FILE for exceptions).
"""
import argparse, os, shutil, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402

DOS_ROOT = os.path.join(u5.BUILD, 'dos')
OBJ = os.path.join(u5.BUILD, 'obj')


def dos_name(path):
    base = os.path.basename(path)
    stem, ext = os.path.splitext(base)
    if len(stem) > 8 or len(ext) > 4:
        sys.exit('%s is not an 8.3 name' % base)
    return base


def c_comments(text):
    """Rewrite // comments as /* */ (MSC 5.0 knows only the latter); strings are left alone."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c in '"\'':
            j = i + 1
            while j < n and text[j] != c and text[j] != '\n':
                j += 2 if text[j] == '\\' else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append(text[i:j])
            i = j
        elif text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
            body = text[i + 2:j].rstrip('\r').replace('*/', '* /')
            out.append('/*' + body + ' */' + ('\r' if text[i:j].endswith('\r') else ''))
            i = j
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def compiler_dir(compiler):
    return u5.MSC51 if compiler in (None, 'msc51') else os.path.join(u5.BUILD, compiler)


def compile_files(files, dosbox=None, flags=None, show_log=False, src=None, quiet=False, compiler=None):
    """Compile files (names relative to src/). Returns {file: [error lines]}.
    compiler: 'msc51' (build/msc51) or 'msc50' (build/msc50); by default each file with the
    compiler the original used for it (u5.COMPILER_BY_FILE, MSC 5.1 otherwise)."""
    if compiler is None:
        groups = {}
        for f in files:
            groups.setdefault(u5.COMPILER_BY_FILE.get(os.path.basename(f), 'msc51'), []).append(f)
        res = {}
        for comp, fs in groups.items():
            res.update(compile_files(fs, dosbox, flags, show_log, src, quiet, comp))
        return res
    src = src or u5.SRC
    tools_dir = compiler_dir(compiler)
    if not os.path.exists(os.path.join(tools_dir, 'BIN', 'CL.EXE')):
        sys.exit('%s is not installed: run tools/setup_msc51.py first' % (compiler or 'MSC 5.1'))
    dosbox = u5.find_dosbox(dosbox)
    # a scratch drive per run, so that several builds (of different files) can run at once
    os.makedirs(DOS_ROOT, exist_ok=True)
    DOS = tempfile.mkdtemp(prefix='run', dir=DOS_ROOT)
    stage = os.path.join(DOS, 'S')
    shutil.copytree(src, stage, ignore=shutil.ignore_patterns('*.OBJ', '*.obj', '*.COD', '*.cod'))
    if compiler == 'msc50':
        for dp, _, fs in os.walk(stage):
            for fn in fs:
                if fn.lower().endswith(('.c', '.h')):
                    p = os.path.join(dp, fn)
                    t = open(p, encoding='latin-1', newline='').read()
                    open(p, 'w', encoding='latin-1', newline='').write(c_comments(t))
    os.makedirs(os.path.join(DOS, 'TMP'))
    os.makedirs(OBJ, exist_ok=True)
    lines = ['@echo off', r'set PATH=D:\BIN', r'set INCLUDE=D:\INCLUDE', r'set LIB=D:\LIB',
             r'set TMP=C:\TMP', r'cd \S', r'echo. > C:\BUILD.LOG']
    for f in files:
        dos_name(f)
        opts = flags or u5.CFLAGS_BY_FILE.get(f, u5.CFLAGS)
        lines.append(r'echo ==== %s >> C:\BUILD.LOG' % f)
        lines.append(r'CL %s /c /Fc %s >> C:\BUILD.LOG' % (opts, f))
    open(os.path.join(DOS, 'BUILD.BAT'), 'w', newline='').write('\r\n'.join(lines) + '\r\n')
    conf = os.path.join(DOS, 'dosbox.conf')
    open(conf, 'w').write(
        '[sdl]\noutput=surface\n[dosbox]\nmemsize=16\n[cpu]\ncycles=max\n[log]\nlogfile=\n'
        '[autoexec]\nmount c "%s"\nmount d "%s"\nc:\ncall \\BUILD.BAT\nexit\n' % (DOS, tools_dir))
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
    subprocess.run([dosbox, '-conf', conf, '-nogui', '-exit', '-silent'], env=env,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1800)
    logp = os.path.join(DOS, 'BUILD.LOG')
    log = open(logp, encoding='latin-1').read() if os.path.exists(logp) else ''
    if show_log:
        print(log)
    results = {}
    cur = None
    for line in log.splitlines():
        if line.startswith('==== '):
            cur = line[5:].strip()
            results[cur] = []
        elif cur and (' error C' in line or 'fatal error' in line):
            results[cur].append(line.strip())
    for f in files:
        stem = os.path.splitext(f)[0].upper()
        found = False
        for ext in ('.OBJ', '.COD'):
            p = next((os.path.join(stage, n) for n in os.listdir(stage) if n.upper() == stem + ext), None)
            dst = os.path.join(OBJ, stem + ext)
            if os.path.exists(dst):
                os.remove(dst)
            if p:
                shutil.copy(p, dst)
                found = found or ext == '.OBJ'
        if not found:
            results.setdefault(f, []).append('no object was produced')
        if not quiet:
            errs = results.get(f, [])
            print('%-12s %s' % (f, 'ok' if not errs else 'FAILED: ' + errs[0]))
    if not log:
        print('DOSBox-X wrote no log: did it run? (%s)' % dosbox)
    if not show_log:
        shutil.rmtree(DOS, ignore_errors=True)
    return results


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('files', nargs='*')
    ap.add_argument('--dosbox')
    ap.add_argument('--flags', help='override the compiler switches for every file')
    ap.add_argument('--log', action='store_true', help='print the compiler output')
    a = ap.parse_args()
    files = [os.path.basename(f) for f in a.files] or sorted(u5.MODULES)
    res = compile_files(files, a.dosbox, a.flags, a.log)
    if any(res.get(f) for f in files):
        sys.exit(1)


if __name__ == '__main__':
    main()
