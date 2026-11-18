"""Link u5d's objects with PLINK86plus 2.24 (../tools/plink86plus-2.24, not in the repository).

    pl.py WORKDIR LNKFILE [--target gog|1988] [--rename all|none|NAMES] [--libs DIR]

LNKFILE is a PLINK command file (research/plink/*.LNK) that may use these lines:
    @ROOT            FILE lines for the resident objects after ULTIMA.OBJ (link-script order)
    @AREAS           the four BEGIN ... END areas with a SECTION INTO per overlay
    @AREAn           one of them (n = 1..4)
    @DEFINES         DEFINEs for the names nothing defines (found by a first run)
The objects come from build/obj (build/obj-1988 for the 1988 target's own) with the Nantucket
COMENT added (PLINK86plus' Nantucket edition refuses others); --rename renames the code
segment _TEXT to XTEXT in the resident objects other than ULTIMA (all), in none, or in those
named (comma-separated). Libraries copied in: SLIBCE.LIB (SLIBCR+LIBH+EM+SLIBFP merged with
MSC's LIB, built once into --libs), SLIBCR.LIB, LIBH.LIB, OVERLAY.LIB, and every .OBJ/.LIB in
research/plink/lib/ (plasm.py's output: OVUSER.OBJ, ...). Writes WORKDIR/LINK.LOG, the EXE,
the .OVL files and the map; plcmp2.py compares them with a target's files."""
import argparse, os, re, shutil, struct, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
sys.path.insert(0, HERE)
import u5  # noqa: E402
import plrun  # noqa: E402

TEMP = os.path.join(os.environ.get('TEMP', '/tmp'), 'w20-plink')
SKIP = ('PLMGR', 'PLTAB', 'PLSTART')


def rec(t, body):
    r = bytes([t]) + struct.pack('<H', len(body) + 1) + body
    return r + bytes([(-sum(r)) & 0xff])


TAG = rec(0x88, b'\xc0\xc8\x00\x5e\xa1\x07')


def records(b):
    i = 0
    while i < len(b):
        t = b[i]
        n = struct.unpack('<H', b[i + 1:i + 3])[0]
        yield t, b[i + 3:i + 2 + n]
        i += 3 + n


def renseg(b, old=b'_TEXT', new=b'XTEXT'):
    out = bytearray()
    for t, body in records(b):
        body = bytearray(body)
        if t == 0x96:
            j = 0
            while j < len(body):
                n = body[j]
                if bytes(body[j + 1:j + 1 + n]) == old:
                    body[j + 1:j + 1 + n] = new
                j += 1 + n
        out += rec(t, bytes(body))
    return bytes(out)


def tag(b):
    n = struct.unpack('<H', b[1:3])[0]
    assert b[0] == 0x80
    if TAG in b[:3 + n + len(TAG)]:
        return b
    return b[:3 + n] + TAG + b[3 + n:]


def script(target):
    """(root objects in processing order, {overlay: [objects]}, overlay order, areas), from the
    link command file (src/u5d.lnk)."""
    import u5link
    sc = u5link.Script(target=target)
    files = [(n, sec) for k, n, sec in sc.order if k == 'file']
    root = [n for n, sec in files if sec is None and n not in SKIP]
    ovl = {o: [n for n, sec in files if sec == o] for o in sc.overlays()}
    return root, ovl, sc.overlays(), sc.areas


def objpath(name, target):
    if target != 'gog':
        p = os.path.join(ROOT, 'build', 'obj-' + target, name + '.OBJ')
        if os.path.exists(p):
            return p
    return os.path.join(ROOT, 'build', 'obj', name + '.OBJ')


def dosbox(d, bat):
    open(os.path.join(d, 'B.BAT'), 'w', newline='\r\n').write('@echo off\n' + bat)
    conf = os.path.join(d, 'c.conf')
    open(conf, 'w').write('[sdl]\noutput=surface\n[cpu]\ncycles=max\n[log]\nlogfile=\n[autoexec]\n'
                          'mount c "%s"\nc:\ncall B.BAT\nexit\n' % d)
    subprocess.run([u5.find_dosbox(), '-conf', conf, '-nogui', '-exit', '-silent'],
                   env=dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy'),
                   capture_output=True, timeout=300)


MERGE = os.environ.get('PL_MERGE', 'SLIBCR,LIBH').split(',')


def merged_lib(libs):
    """SLIBCE.LIB merged from $PL_MERGE (default SLIBCR,LIBH; EM and SLIBFP bring a FAR_BSS
    segment the original does not have)."""
    libs = libs + '-' + '-'.join(MERGE)
    out = os.path.join(libs, 'SLIBCE.LIB')
    if os.path.exists(out):
        return out
    os.makedirs(libs, exist_ok=True)
    L = os.path.join(ROOT, 'build', 'msc51', 'LIB')
    for f in MERGE:
        shutil.copy(os.path.join(L, f + '.LIB'), libs)
    shutil.copy(os.path.join(ROOT, 'build', 'msc51', 'BIN', 'LIB.EXE'), libs)
    dosbox(libs, 'LIB SLIBCE.LIB %s; > LIB.LOG\n' % ' '.join('+%s.LIB' % f for f in MERGE))
    assert os.path.exists(out), open(os.path.join(libs, 'LIB.LOG')).read()
    return out


def defines(names):
    import u5link
    alias = {u5link.exported(k): u5link.exported(v) for k, v in u5link.Script().defines.items()}
    L = []
    for s in names:
        if s in alias:
            L.append('DEFINE %s = %s' % (s, alias[s]))
        elif s.startswith('STUB_'):
            L.append('DEFINE %s = %s' % (s, s[5:]))
        elif s == 'ULTIMA_0ACE_GRAP_18_TRANSFERAREA':
            L.append('DEFINE %s = ULTIMA_0ACE_GRAP_18_TRANSFERARE' % s)
        else:
            m = re.match(r'^[A-Z0-9]+_([0-9A-F]{4})', s)
            L.append('DEFINE %s = 0%XH' % (s, int(m.group(1), 16) if m else 0))
    return L


def area_lines(a, ovl):
    L = ['BEGIN']
    for o in a:
        L.append('  SECTION INTO %s FILE %s' % (o, ', '.join(n + '.OBJ' for n in ovl[o])))
    return L + ['END']


def make_lib(work, name, objs):
    """Build WORKDIR/NAME from WORKDIR's OBJs with MSC's LIB (in the order given)."""
    shutil.copy(os.path.join(ROOT, 'build', 'msc51', 'BIN', 'LIB.EXE'), work)
    bat = ''
    for k in range(0, len(objs), 8):
        bat += 'LIB %s %s; >> MKLIB.LOG\n' % (name, ' '.join('+%s.OBJ' % o for o in objs[k:k + 8]))
    dosbox(work, bat)
    return open(os.path.join(work, 'MKLIB.LOG'), encoding='latin-1').read()


def prepare(work, lnk, target='gog', rename='all', libs=None, extra=()):
    root, ovl, order, areas = script(target)
    if os.path.exists(work):
        shutil.rmtree(work)
    os.makedirs(work)
    if rename == 'all':
        ren = set(n for n in root[1:] if n != 'VARS')
    elif rename == 'none':
        ren = set()
    else:
        ren = set(rename.split(','))
    for name in root + [n for o in order for n in ovl[o]]:
        b = open(objpath(name, target), 'rb').read()
        if name in ren:
            b = renseg(b)
        open(os.path.join(work, name + '.OBJ'), 'wb').write(tag(b))
    shutil.copy(merged_lib(libs or os.path.join(TEMP, 'libs')), work)
    for f in ('SLIBCR.LIB', 'LIBH.LIB'):
        shutil.copy(os.path.join(ROOT, 'build', 'msc51', 'LIB', f), work)
    lib = os.path.join(HERE, 'lib')
    for d in (lib, os.path.join(TEMP, 'lib')):
        if os.path.isdir(d):
            for f in os.listdir(d):
                if f.upper().endswith(('.OBJ', '.LIB')):
                    shutil.copy(os.path.join(d, f), os.path.join(work, f.upper()))
    for f in extra:
        shutil.copy(f, work)
    text = open(lnk).read() if os.path.exists(lnk) else lnk
    L = []
    for line in text.splitlines():
        s = line.strip().upper()
        if s == '@ROOT':
            L += ['FILE %s.OBJ' % n for n in root[1:] if n != 'VARS']
        elif s == '@AREAS':
            for a in areas:
                L += area_lines(a, ovl)
        elif re.match(r'@AREA\d$', s):
            L += area_lines(areas[int(s[5:]) - 1], ovl)
        elif s.startswith('@ROOTLIB '):      # @ROOTLIB NAME.LIB: the resident objects as a library
            print(make_lib(work, s.split()[1], [n for n in root[1:] if n != 'VARS'])[-300:])
        else:
            L.append(line)
    return L


def link(work, L):
    def write(lines):
        open(os.path.join(work, 'U5.LNK'), 'w', newline='\r\n').write('\n'.join(lines) + '\n')
    at = next((i for i, x in enumerate(L) if x.strip().upper() == '@DEFINES'), None)
    write([x for x in L if x.strip().upper() != '@DEFINES'])
    log = plrun.run(work)
    und = list(dict.fromkeys(re.findall(r'Symbol (\S+) was accessed', log)))
    if und and at is not None:
        write(L[:at] + defines(und) + L[at + 1:])
        log = plrun.run(work)
    return log


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('work')
    ap.add_argument('lnk')
    ap.add_argument('--target', default='gog')
    ap.add_argument('--rename', default='all')
    ap.add_argument('--libs')
    ap.add_argument('--overlay-lib', help='a library to use as OVERLAY.LIB (e.g. OVU5.LIB)')
    a = ap.parse_args()
    L = prepare(a.work, a.lnk, a.target, a.rename, a.libs)
    if a.overlay_lib:
        src = a.overlay_lib if os.path.exists(a.overlay_lib) else os.path.join(TEMP, 'lib', a.overlay_lib)
        shutil.copy(src, os.path.join(a.work, 'OVERLAY.LIB'))
    log = link(a.work, L)
    log = re.sub(r'(Pass \d for Module \S+ File \S+)', r'\1\n', log)
    print('\n'.join(x for x in log.splitlines() if not x.startswith('Pass ') and x.strip())[-3000:])

