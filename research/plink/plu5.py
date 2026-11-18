"""Link u5d's objects with the real PLINK86plus 2.24 (../tools/plink86plus-2.24, not in the repo):
    plu5.py WORKDIR VARIANT
VARIANT: a (LIBRARY SLIBCR, LIBH after ULTIMA.OBJ), s, s2 (SEARCH), f (libraries in the FILE list),
ax (a, with every root object but ULTIMA's code segment renamed _TEXT -> XTEXT), cNx / cNsx (a merged
SLIBCE.LIB built with MSC's LIB in WORKDIR/../libs-out/CEN.LIB, plus the rename). Undefined names are
taken from WORKDIR/../plu5-a/undef.txt (a first run with variant a writes LINK.LOG; extract the
"Symbol X" lines) and DEFINEd: CMDS_0000's short link names to their long names, STUB_X to X, the
names u5link resolves by address to that address. See docs/MATCHING.md, "Linking with PLINK86plus 2.24".
The first experiment, superseded by pl.py; it reads the link script's format of before phase B
(overlay and object lines), which src/u5d.lnk no longer has.
"""
import os, re, shutil, struct, subprocess, sys

ROOT = r'C:\Users\karl9\Downloads\Claude\u5-decompiled\u5d'
PL = r'C:\Users\karl9\Downloads\Claude\u5-decompiled\tools\plink86plus-2.24'
work = sys.argv[1]
variant = sys.argv[2] if len(sys.argv) > 2 else 'a'

lnk = open(os.path.join(ROOT, 'src', 'u5d.lnk')).read().splitlines()
root, ovl, order = [], {}, []
for l in lnk:
    f = l.split()
    if not f or f[0].startswith('#'):
        continue
    if f[0] == 'overlay':
        order.append(f[1]); ovl[f[1]] = []
    elif f[0] == 'object':
        name = f[1]
        if name in ('PLMGR', 'PLTAB', 'PLSTART'):
            continue
        code = next((x for x in f if x.startswith('code=')), None)
        if code and ':' in code:
            ovl[code[5:].split(':')[0]].append(name)
        else:
            root.append(name)
areas, cur, base = [], None, None
for o in order:
    b = next(x for x in lnk if x.startswith('overlay') and x.split()[1] == o).split()[2]
    b = int(b.split('=')[1], 16)
    if base is None or (b != base and not (o == 'INTRO')):
        if not (o == 'INTRO'):
            cur = []; areas.append(cur); base = b
    cur.append(o)

if os.path.exists(work):
    shutil.rmtree(work)
os.makedirs(work)


def rec(t, body):
    r = bytes([t]) + struct.pack('<H', len(body) + 1) + body
    return r + bytes([(-sum(r)) & 0xff])


TAG = rec(0x88, b'\xc0\xc8\x00\x5e\xa1\x07')


SEGRENAME = set()


def tag(src, dst):
    b = open(src, 'rb').read()
    if os.path.basename(src).upper()[:-4] in SEGRENAME:
        b = renseg(b)
    n = struct.unpack('<H', b[1:3])[0]
    assert b[0] == 0x80
    open(dst, 'wb').write(b[:3 + n] + TAG + b[3 + n:])


def renseg(b):
    out = bytearray(); i = 0
    while i < len(b):
        t = b[i]; n = struct.unpack('<H', b[i + 1:i + 3])[0]; body = bytearray(b[i + 3:i + 2 + n])
        if t == 0x96:
            j = 0
            while j < len(body):
                l = body[j]
                if bytes(body[j + 1:j + 1 + l]) == b'_TEXT':
                    body[j + 1:j + 1 + l] = b'XTEXT'
                j += 1 + l
        out += rec(t, bytes(body)); i += 3 + n
    return bytes(out)


def fn(name):  # DOS file names: a leading digit is fine with the extension given
    return name + '.OBJ'


if variant.endswith('x'):
    SEGRENAME.update(n for n in root[1:] if n != 'VARS')
for name in root + [n for o in order for n in ovl[o]]:
    tag(os.path.join(ROOT, 'build', 'obj', fn(name)), os.path.join(work, fn(name)))
for lib in ('SLIBCR.LIB', 'LIBH.LIB'):
    shutil.copy(os.path.join(ROOT, 'build', 'msc51', 'LIB', lib), work)
shutil.copy(os.path.join(ROOT, 'build', 'msc51', 'LIB', 'SLIBCR.LIB'), os.path.join(work, 'SLIBCE.LIB'))
if variant.startswith('c'):
    shutil.copy(os.path.join(os.path.dirname(work), 'libs-out', 'CE%s.LIB' % variant[1]), os.path.join(work, 'SLIBCE.LIB'))
for f in ('PLINK86.EXE', 'OVERLAY.LIB'):
    shutil.copy(os.path.join(PL, f), work)

L = ['OUTPUT ULTIMA.EXE', 'MAP = ULTIMA.MAP']
first, rest = root[0], root[1:]
if variant.startswith('c') and 's' in variant:
    L += ['FILE %s' % fn(first), 'SEARCH SLIBCE.LIB']
    L += ['FILE %s' % fn(n) for n in rest if n != 'VARS']
elif variant.startswith('c'):
    L += ['FILE %s' % fn(first), 'LIBRARY SLIBCE.LIB']
    L += ['FILE %s' % fn(n) for n in rest if n != 'VARS']
elif variant in ('a', 'ax'):      # libraries searched right after the first object
    L += ['FILE %s' % fn(first), 'LIBRARY SLIBCR.LIB, LIBH.LIB']
    L += ['FILE %s' % fn(n) for n in rest if n != 'VARS']
elif variant == 's':    # SEARCH: search the libraries at this point
    L += ['FILE %s' % fn(first), 'SEARCH SLIBCR.LIB, LIBH.LIB']
    L += ['FILE %s' % fn(n) for n in rest if n != 'VARS']
elif variant == 'f':    # libraries in the FILE list after the first object
    L += ['FILE %s, SLIBCR.LIB, LIBH.LIB' % fn(first)]
    L += ['FILE %s' % fn(n) for n in rest if n != 'VARS']
elif variant == 's2':
    L += ['FILE %s' % fn(first), 'SEARCH SLIBCR.LIB', 'SEARCH LIBH.LIB']
    L += ['FILE %s' % fn(n) for n in rest if n != 'VARS']
else:
    L += ['FILE %s' % fn(n) for n in root if n != 'VARS']
for a in areas:
    L.append('BEGIN')
    for o in a:
        L.append('  SECTION INTO %s FILE %s' % (o, ', '.join(fn(n) for n in ovl[o])))
    L.append('END')
if 'VARS' in root:
    L.append('FILE VARS.OBJ')
if variant not in ('a', 'ax', 's', 's2', 'f') and not variant.startswith('c'):
    L.append('LIBRARY SLIBCR.LIB, LIBH.LIB')
L.append('LIBRARY OVERLAY.LIB')
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import u5
und = os.path.join(os.path.dirname(work), 'plu5-a', 'undef.txt')
alias = {k.upper(): v.upper() for k, v in u5.LINK_DEFINES.items()}
for sname in (open(und).read().split() if os.path.exists(und) else []):
    if sname in alias:
        L.append('DEFINE %s = %s' % (sname, alias[sname]))
    elif sname.startswith('STUB_'):
        L.append('DEFINE %s = %s' % (sname, sname[5:]))
    elif sname == 'ULTIMA_0ACE_GRAP_18_TRANSFERAREA':
        L.append('DEFINE %s = ULTIMA_0ACE_GRAP_18_TRANSFERARE' % sname)
    else:
        m = re.match(r'[A-Z0-9]*_?([0-9A-F]{4})', sname.split('_', 1)[1] if '_' in sname else sname)
        v = int(sname.split('_')[1][:4], 16) if re.match(r'^[A-Z0-9]+_[0-9A-F]{4}', sname) else 0
        L.append('DEFINE %s = 0%XH' % (sname, v))
open(os.path.join(work, 'U5.LNK'), 'w', newline='\r\n').write('\n'.join(L) + '\n')
open(os.path.join(work, 'DOT.TXT'), 'w', newline='\r\n').write('.\n.\n.\n')
open(os.path.join(work, 'B.BAT'), 'w', newline='\r\n').write('@echo off\nPLINK86 @U5.LNK < DOT.TXT > LINK.LOG\n')
conf = os.path.join(work, 'c.conf')
open(conf, 'w').write('[sdl]\noutput=surface\n[dosbox]\nmemsize=16\n[cpu]\ncycles=max\n[log]\nlogfile=\n[autoexec]\n'
                      'mount c "%s"\nc:\ncall B.BAT\nexit\n' % work)
try:
    subprocess.run([r'C:\Users\karl9\Downloads\Claude\u6-decompiled\tools\Dosbox-X\dosbox-x.exe', '-conf', conf,
                    '-nogui', '-exit', '-silent'], env=dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy'),
                   capture_output=True, timeout=300)
except subprocess.TimeoutExpired:
    print('TIMEOUT')
print(open(os.path.join(work, 'U5.LNK')).read())
p = os.path.join(work, 'LINK.LOG')
print(open(p, 'rb').read().decode('cp437', 'replace')[:3000] if os.path.exists(p) else 'no log')
print(sorted(x for x in os.listdir(work) if not x.endswith('.OBJ')))
