"""Assemble research/plink/*.ASM (or those named) with MASM 4.0 (build/masm) into
%TEMP%/w20-plink/lib, add the Nantucket COMENT, and build OVU5.LIB there: OVERLAY.LIB with
OVUSERM replaced by OVUSER (MSC's LIB). pl.py copies that folder into each link.
    plasm.py [FILE.ASM ...]"""
import glob, os, shutil, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import pl  # noqa: E402

OUT = os.path.join(pl.TEMP, 'lib')
files = sorted(glob.glob(os.path.join(HERE, '*.ASM'))) + sys.argv[1:]
d = os.path.join(pl.TEMP, 'asm')
shutil.rmtree(d, ignore_errors=True)
os.makedirs(d)
os.makedirs(OUT, exist_ok=True)
shutil.copy(os.path.join(ROOT, 'build', 'masm', 'MASM.EXE'), d)
shutil.copy(os.path.join(ROOT, 'build', 'msc51', 'BIN', 'LIB.EXE'), d)
shutil.copy(os.path.join(pl.plrun.PL, 'OVERLAY.LIB'), os.path.join(d, 'OVU5.LIB'))
bat = ''
for f in files:
    n = os.path.splitext(os.path.basename(f))[0].upper()
    shutil.copy(f, os.path.join(d, n + '.ASM'))
    bat += 'MASM /Mx %s.ASM,%s.OBJ,%s.LST; > %s.LOG\n' % (n, n, n, n)
bat += 'LIB OVU5.LIB -OVUSERM +OVUSER.OBJ; > LIB.LOG\n'
pl.dosbox(d, bat)
for f in files:
    n = os.path.splitext(os.path.basename(f))[0].upper()
    print(open(os.path.join(d, n + '.LOG'), encoding='latin-1').read().strip().splitlines()[-1:])
    o = os.path.join(d, n + '.OBJ')
    if os.path.exists(o):
        open(os.path.join(OUT, n + '.OBJ'), 'wb').write(pl.tag(open(o, 'rb').read()))
print(open(os.path.join(d, 'LIB.LOG'), encoding='latin-1').read())
if os.path.exists(os.path.join(d, 'OVU5.LIB')):
    shutil.copy(os.path.join(d, 'OVU5.LIB'), OUT)
print(sorted(os.listdir(OUT)))
