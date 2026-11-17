#!/usr/bin/env python3
"""The gate: build all 25 files from source and compare them with the GOG release.

    python tools/check.py [--keep] [--no-build] [--gamedata DIR]

1. Compiles every C file of the DOS build (tools/dosbuild.py: MSC 5.1 or 5.0 per file) and
   assembles every module of src/asm (tools/asmbuild.py: MASM 4.0) into a folder of its own,
   build/check/obj, starting empty: nothing from an earlier build is used.
2. Links them (tools/u5link.py, as the link script src/u5d.lnk says) into build/check/out:
   ULTIMA.EXE, the 23 code overlays, FLAMES.OVL and DATA.OVL.
3. Compares the SHA-1 of each with the GOG release's (the table u5.u5.GOG_SHA1).

Building needs the repository, the compilers, MASM and DOSBox-X; steps 1 to 3 open no game
file. When the game files are found (--gamedata, $U5_GAMEDATA, gamedata/, ../gamedata) two
more checks are made, and the last line says so:

4. the game files are the GOG release's (their SHA-1s are the table's);
5. the link script is what the original files give (tools/linkscript.py --check: every
   placement measured again from the game files and the objects just built).

Without them the comparison is with the SHA-1 table alone, which is the same test of the
25 files; 4 and 5 are skipped.

It passes (exit status 0) only when all 25 are identical, every object was built, and the
link resolved no reference by the address in its name and left none unresolved (and, with
the game files, 4 and 5 hold).

--no-build links and compares what build/check/obj already holds. --keep leaves
build/check/obj in place afterwards (build/check/out always stays).
"""
import argparse, glob, hashlib, os, shutil, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402

CHECK = os.path.join(u5.BUILD, 'check')


def sha1(data):
    return hashlib.sha1(data).hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--gamedata')
    ap.add_argument('--dosbox')
    ap.add_argument('--no-build', action='store_true', help='link and compare the objects already in build/check/obj')
    ap.add_argument('--keep', action='store_true', help='keep build/check/obj afterwards')
    a = ap.parse_args()
    if a.gamedata:
        os.environ['U5_GAMEDATA'] = os.path.abspath(a.gamedata)
    gd = u5.find_gamedata(a.gamedata, required=False)     # only to check, not to build
    obj, out = os.path.join(CHECK, 'obj'), os.path.join(CHECK, 'out')
    problems = []

    if not a.no_build:
        shutil.rmtree(obj, ignore_errors=True)
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(obj, exist_ok=True)
    u5.set_obj(obj)
    import asmbuild, dosbuild, u5link          # after set_obj: they read u5.OBJ
    cfiles = sorted(u5.MODULES)
    asms = sorted(glob.glob(os.path.join(u5.SRC, 'asm', '*.ASM')))
    if not a.no_build:
        print('compiling %d C files (MSC 5.1: %d, MSC 5.0: %d) ...' % (
            len(cfiles), sum(1 for f in cfiles if u5.COMPILER_BY_FILE.get(f, 'msc51') == 'msc51'),
            sum(1 for f in cfiles if u5.COMPILER_BY_FILE.get(f) == 'msc50')), flush=True)
        res = dosbuild.compile_files(cfiles, a.dosbox, quiet=True)
        for f in cfiles:
            if res.get(f):
                problems.append('%s: %s' % (f, res[f][0]))
        print('assembling %d modules (MASM 4.0) ...' % len(asms), flush=True)
        for p, (o, log) in asmbuild.assemble_many(asms, a.dosbox).items():
            if o is None:
                problems.append('%s: MASM failed' % os.path.basename(p))
    for f in cfiles + [os.path.basename(p) for p in asms]:
        if not os.path.exists(os.path.join(obj, os.path.splitext(f)[0].upper() + '.OBJ')):
            problems.append('%s: no object' % f)

    print('linking ...', flush=True)
    lk, files = u5link.link(out)
    same = 0
    for name in sorted(u5.GOG_SHA1, key=lambda n: (n != 'ULTIMA.EXE', n)):
        ours = files.get(name)
        h = sha1(ours) if ours is not None else '(not built)'
        ok = h == u5.GOG_SHA1[name]
        same += ok
        print('%-13s %s  %s' % (name, h, 'identical' if ok else 'DIFFERENT (GOG: %s)' % u5.GOG_SHA1[name]))
    for name in sorted(set(files) - set(u5.GOG_SHA1)):
        problems.append('%s: built, but not one of the 25 files' % name)
    if lk.by_convention:
        problems.append('%d names resolved only by the address in their name: %s' % (
            len(lk.by_convention), ' '.join(sorted(lk.by_convention)[:12])))
    if lk.unresolved:
        problems.append('unresolved: %s' % ' '.join(sorted(lk.unresolved)[:12]))
    problems += ['link: ' + n for n in lk.notes]
    if gd:
        for name, want in u5.GOG_SHA1.items():
            if sha1(u5.read_file(gd, name)) != want:
                problems.append('%s in %s is not the GOG release\'s file' % (name, gd))
        import linkscript
        measured, notes = linkscript.text(gd)
        problems += ['link script: ' + n for n in notes]
        if measured != open(u5link.SCRIPT, encoding='ascii').read():
            problems.append('src/u5d.lnk is not what tools/linkscript.py measures from the game files')
    for p in problems:
        print('problem: ' + p)
    if not a.keep and not a.no_build:
        shutil.rmtree(obj, ignore_errors=True)
    ok = same == len(u5.GOG_SHA1) and not problems
    print('%d of %d files identical to the GOG release: %s' % (same, len(u5.GOG_SHA1), 'PASS' if ok else 'FAIL'))
    print('built without the game files; ' + (
        'the game files in %s are the GOG release\'s and give the link script' % gd if gd and ok else
        'game files found in %s' % gd if gd else
        'no game files found: compared with the SHA-1 table only, the link script not measured again'))
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
