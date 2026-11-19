#!/usr/bin/env python3
"""Assemble the assembly modules (src/asm/*.ASM) with MASM 4.0 in DOSBox-X, and check them.

    python tools/asmbuild.py [NAME.ASM ...] [--target T]    assemble (default: all) and compare

Each module's first line says which range of the original it is: "ULTIMA.EXE CS:xxxx-yyyy" for
the resident image, or "NAME.OVL xxxx-yyyy" (offsets in the .OVL file, as in u5d's names) for an assembly part of
an overlay. Its _TEXT is compared with those bytes, fixups masked, in the target's game files
(--target, else $U5_TARGET, else gog). The target's macros are defined (MASM /DRELEASE_1988
for 1988), and the objects go to the target's folder (build/obj, build/obj-1988, or $U5_OBJ).
For the gog target, the modules that differ by target (IFDEF RELEASE_1988) are also assembled
for 1988, into build/obj-1988: tools/u5link.py takes the EXE checksum from that link.
MASM 4.0 (MASM.EXE, from the sibling U1 project or your own copy) is expected in build/masm.
"""
import argparse, os, re, shutil, subprocess, sys, tempfile

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
        lines.append('D:\\MASM /Mx%s %s.ASM,%s.OBJ,%s.LST; > %s.LOG' % (u5.target_defines(), n, n, n, n))
    open(os.path.join(d, 'B.BAT'), 'w', newline='').write('\r\n'.join(lines) + '\r\n')
    conf = os.path.join(d, 'c.conf')
    open(conf, 'w').write('[sdl]\noutput=surface\n[cpu]\ncycles=max\n[log]\nlogfile=\n[autoexec]\n'
                          'mount c "%s"\nmount d "%s"\nc:\ncall B.BAT\nexit\n' % (d, MASM_DIR))
    subprocess.run([u5.find_dosbox(dosbox), '-conf', conf, '-nogui', '-exit', '-silent'],
                   env=dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy'),
                   capture_output=True, timeout=600)
    out = {}
    objdir = u5.OBJ
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


def module_place(path):
    """(overlay name or None, start, end) from the module's first line."""
    line = open(path, encoding='latin-1').readline()
    m = re.search(r'CS:([0-9a-f]{4})-([0-9a-f]{4})', line)
    if m:
        return None, int(m.group(1), 16), int(m.group(2), 16)
    m = re.search(r'\b([A-Z0-9]+)\.OVL ([0-9a-f]{4})-([0-9a-f]{4})', line)
    if m:
        return m.group(1), int(m.group(2), 16), int(m.group(3), 16)
    return None


def module_range(path):
    """The resident range (CS offsets) of a resident module, else None."""
    p = module_place(path)
    return (p[1], p[2]) if p and p[0] is None else None


def original_bytes(gamedata, place):
    ovl, a, b = place
    if ovl is None:
        return u5.exe_image(gamedata)[a:b]
    return u5.read_file(gamedata, ovl + '.OVL')[a:b]     # file offsets, as u5d's names


def code_image(obj):
    """(bytes, offsets under fixups): the module's code segments (class CODE, in order, each on
    its alignment, as the link places them: PLMGR's OVDATA after its code) from the first."""
    data, masked = bytearray(), set()
    for s in obj.segs[1:]:
        if s.cls != 'CODE' or not s.length:
            continue
        if data:
            a = {3: 16, 2: 2, 4: 256}.get(s.align, 1)
            data += bytes(-len(data) % a)
        masked |= {len(data) + k for fx in s.fixups for k in range(fx.loc, fx.loc + fx.size)}
        data += s.data[:s.length]
    return bytes(data), masked


def assemble_patch_base(paths, dosbox=None):
    """For a target that is another's link patched afterwards (u5.PATCH_OF: gog), assemble
    the modules of the list that differ by target for that other target too, into its folder
    (u5.patch_base_obj(): build/obj-1988 beside build/obj), where tools/u5link.py takes them
    for the EXE checksum. Returns {path: (Object or None, log)}."""
    base = u5.PATCH_OF.get(u5.TARGET)
    todo = [p for p in paths if base and u5.target_dependent(p) and p in u5.asm_modules(base)]
    if not todo:
        return {}
    with u5.building(base, u5.patch_base_obj()):
        return assemble_many(todo, dosbox)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('files', nargs='*')
    u5.add_target_argument(ap)
    a = ap.parse_args()
    u5.set_target(a.target)
    paths = [p if os.path.exists(p) else os.path.join(u5.SRC, 'asm', p) for p in a.files] or u5.asm_modules()
    gd = u5.find_gamedata()
    bad = 0
    for p, (obj, log) in assemble_many(paths).items():
        r = module_place(p)
        if obj is None:
            print('%-14s MASM failed:\n%s' % (os.path.basename(p), log))
            bad += 1
            continue
        data, masked = code_image(obj)
        orig = original_bytes(gd, r)
        diffs = [k for k in range(min(len(orig), len(data))) if k not in masked and data[k] != orig[k]]
        same = not diffs and len(data) == len(orig)
        where = 'CS:%04x-%04x' % r[1:] if r[0] is None else '%s.OVL %04x-%04x' % r
        print('%-14s %-22s %5d bytes  %s' % (os.path.basename(p), where, len(orig),
                                                    'identical' if same else '%d differ, length %d' % (len(diffs), len(data))))
        bad += not same
    extra = assemble_patch_base(paths)
    if extra:
        failed = [os.path.basename(p) for p, (o, log) in extra.items() if o is None]
        print('also assembled for %s (the EXE checksum, tools/u5link.py) into %s: %s%s' % (
            u5.TARGET_TITLE[u5.PATCH_OF[u5.TARGET]], os.path.relpath(u5.patch_base_obj(), u5.ROOT),
            ' '.join(sorted(os.path.basename(p) for p in extra)),
            '; MASM failed: ' + ' '.join(failed) if failed else ''))
        bad += len(failed)
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
