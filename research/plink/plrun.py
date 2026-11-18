"""Run PLINK86plus 2.24 (../tools/plink86plus-2.24, not in the repository) in DOSBox-X:
    plrun.py WORKDIR [LNKFILE]
WORKDIR holds the objects, libraries and the command file (default U5.LNK); PLINK86.EXE and
OVERLAY.LIB are copied in unless present. Every prompt is answered with '.'. Writes LINK.LOG."""
import os, shutil, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
PL = os.path.normpath(os.path.join(ROOT, '..', 'tools', 'plink86plus-2.24'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import u5  # noqa: E402


def run(work, lnk='U5.LNK', timeout=600):
    for f in ('PLINK86.EXE', 'OVERLAY.LIB'):
        if not os.path.exists(os.path.join(work, f)):
            shutil.copy(os.path.join(PL, f), work)
    open(os.path.join(work, 'DOT.TXT'), 'w', newline='\r\n').write('.\n' * 8)
    open(os.path.join(work, 'B.BAT'), 'w', newline='\r\n').write(
        '@echo off\nPLINK86 @%s < DOT.TXT > LINK.LOG\n' % lnk)
    conf = os.path.join(work, 'c.conf')
    open(conf, 'w').write('[sdl]\noutput=surface\n[dosbox]\nmemsize=16\n[cpu]\ncycles=max\n[log]\n'
                          'logfile=\n[autoexec]\nmount c "%s"\nc:\ncall B.BAT\nexit\n' % work)
    try:
        subprocess.run([u5.find_dosbox(), '-conf', conf, '-nogui', '-exit', '-silent'],
                       env=dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy'),
                       capture_output=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
    p = os.path.join(work, 'LINK.LOG')
    return open(p, 'rb').read().decode('cp437', 'replace') if os.path.exists(p) else 'no log'


if __name__ == '__main__':
    print(run(sys.argv[1], *(sys.argv[2:3])))
