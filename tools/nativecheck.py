#!/usr/bin/env python3
"""Check that the game's C files still compile for the native (SDL) build.

    python tools/nativecheck.py [FILE.c ...]

Matching changes the C a lot; the native build must keep compiling from the same sources.
This compiles the given files (default: every module of the DOS build except vars.c's
data-only role is included too) with the host compiler, as the native build would
(TARGET_SDL), into a scratch folder: only compile errors matter, nothing is linked.

Windows: Visual Studio's cl, found with vswhere. Elsewhere: cc (gcc or clang).
SDL's headers are not needed: an empty stand-in for SDL3/SDL_main.h is used.
"""
import argparse, glob, os, shutil, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402


def vcvars():
    vw = os.path.join(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)'),
                      'Microsoft Visual Studio', 'Installer', 'vswhere.exe')
    if not os.path.exists(vw):
        return None
    out = subprocess.run([vw, '-latest', '-property', 'installationPath'], capture_output=True, text=True).stdout.strip()
    p = os.path.join(out, 'VC', 'Auxiliary', 'Build', 'vcvars64.bat')
    return p if os.path.exists(p) else None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('files', nargs='*')
    a = ap.parse_args()
    files = [os.path.basename(f) for f in a.files] or sorted(u5.MODULES)
    with tempfile.TemporaryDirectory() as tmp:
        stub = os.path.join(tmp, 'inc', 'SDL3')
        os.makedirs(stub)
        open(os.path.join(stub, 'SDL_main.h'), 'w').close()
        srcs = [os.path.join(u5.SRC, f) for f in files]
        if os.name == 'nt':
            vv = vcvars()
            if not vv:
                sys.exit('Visual Studio (cl.exe) was not found')
            bat = os.path.join(tmp, 'b.cmd')
            open(bat, 'w').write('@call "%s" >nul\r\ncd /d "%s"\r\ncl /nologo /c /W2 /I"%s" /I"%s" /Fo"%s\\\\" %s\r\n' % (
                vv, tmp, u5.SRC, os.path.join(tmp, 'inc'), tmp, ' '.join('"%s"' % s for s in srcs)))
            r = subprocess.run(['cmd', '/c', bat], capture_output=True, text=True)
        else:
            cc = shutil.which('cc') or 'gcc'
            r = subprocess.run([cc, '-c', '-I', u5.SRC, '-I', os.path.join(tmp, 'inc')] + srcs, cwd=tmp,
                               capture_output=True, text=True)
        out = r.stdout + r.stderr
        errors = [l for l in out.splitlines() if ' error ' in l or ': error' in l]
        for l in errors:
            print(l)
        print('%d files, %s' % (len(files), 'compile errors: %d' % len(errors) if errors else 'all compile'))
        if errors:
            sys.exit(1)


if __name__ == '__main__':
    main()
