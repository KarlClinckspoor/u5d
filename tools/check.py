#!/usr/bin/env python3
"""The gate: build all 25 files from source and compare them with the GOG release.

    python tools/check.py [--keep] [--no-build] [--gamedata DIR]

1. Compiles every C file of the DOS build (tools/dosbuild.py: MSC 5.1 or 5.0 per file) and
   assembles every module of src/asm (tools/asmbuild.py: MASM 4.0) into a folder of its own,
   build/check/obj, starting empty: nothing from an earlier build is used.
2. Links them (tools/u5link.py) into build/check/out: ULTIMA.EXE, the 23 code overlays,
   FLAMES.OVL and DATA.OVL.
3. Compares the SHA-1 of each with the GOG release's (the table below).

It passes (exit status 0) only when all 25 are identical, every object was built, and the
link resolved no reference by the address in its name and left none unresolved.

--no-build links and compares what build/check/obj already holds. --keep leaves
build/check/obj in place afterwards (build/check/out always stays).

What the gate does not show: the linker places modules where the original has them and
reads those places from the original files (tools/u5link.py, "facts"), so the game files
are needed to build, not only to compare.
"""
import argparse, glob, hashlib, os, shutil, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402

# SHA-1 of the GOG release's files (Ultima 4+5+6 pack, Ultima V 1.16 as patched by GOG).
GOG_SHA1 = {
    'ULTIMA.EXE': '9971bbeb6d00fe8c43a98d5ad2d7b82be2d0ca8b',
    'DATA.OVL': '9b5b640d4b6a014c1983a7f0527bbba9bfb4204e',
    'FLAMES.OVL': '942370a93a77019ee38a6cccc2050267c4b01b03',
    'BLCKTHRN.OVL': '002a9b427e85a3a5bf5518fd0feacf8ac38da1e0',
    'CAST.OVL': '517573779ddb41b40f305a5e20d0d4f95fd751ea',
    'CAST2.OVL': '03d9e239f8c347825e39780c5e0299306064700b',
    'CMDS.OVL': 'e5ae97bcb3d6d0f531787789f6ee02872c370a17',
    'COMBAT.OVL': '48b1a3e38bf9e95458458b2fbbe8f8c4c5b6ee2b',
    'COMSUBS.OVL': '13c9479fc26c851fc9a4eb6ba1843109f09e0b16',
    'DNGLOOK.OVL': '1677a1625db99021ff54d5378761219480e41b94',
    'DUNGEON.OVL': 'c1c407a48915517dc161e28a464c8df65d0a69d8',
    'ENDGAME.OVL': 'e801051bebfdf167ff35be6035688f44983ab57a',
    'FONT.OVL': '90303cf773e225ef4bbd462ab34598c29b1dea94',
    'INTRO.OVL': '4195043891ee60f82c65fe739d77f2ac3e98168c',
    'LOOKOBJ.OVL': 'fe25e3dbabbd3057ab5dd60c18bcf402d1952f80',
    'MAINOUT.OVL': 'd027edaee9b2317f649182a12610f6b60410d649',
    'NPC.OVL': '3c48134a8a99093c3c21b5d7f0253428cc1b5848',
    'OUTSUBS.OVL': '1710166896b91198630794223445f483afd04f78',
    'SHOPPES.OVL': '8c51e5fae50fc0e7524e01d040abd6177e85cd3d',
    'SHOPPES2.OVL': '1644c2ed9d9ad4a1e1dc8ae944eb4040e12c3c8d',
    'SHOPPES3.OVL': '82c5fdb37a965f3976b2d7b63207b032cce56c3e',
    'SJOG.OVL': '188d9e684199f899d33532f6913485786f337962',
    'TALK.OVL': '17ff6bbde9d9781874ba74f9ab5feb3b9808c612',
    'TOWN.OVL': 'fdfed52705b7aa6efc2a89a5b2628f3537292372',
    'ZSTATS.OVL': '94a03813bcb563ccde005d1a09cb67b5e75ea684',
}
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
    gd = u5.find_gamedata(a.gamedata)
    obj, out = os.path.join(CHECK, 'obj'), os.path.join(CHECK, 'out')
    problems = []

    # the original, which the linker measures its placements from, must be the GOG release
    for name, want in GOG_SHA1.items():
        if sha1(u5.read_file(gd, name)) != want:
            problems.append('%s in %s is not the GOG release\'s file' % (name, gd))

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
    for name in sorted(GOG_SHA1, key=lambda n: (n != 'ULTIMA.EXE', n)):
        ours = files.get(name, (None,))[0]
        h = sha1(ours) if ours is not None else '(not built)'
        ok = h == GOG_SHA1[name]
        same += ok
        print('%-13s %s  %s' % (name, h, 'identical' if ok else 'DIFFERENT (GOG: %s)' % GOG_SHA1[name]))
    for name in sorted(set(files) - set(GOG_SHA1)):
        problems.append('%s: built, but not one of the 25 files' % name)
    if lk.by_convention:
        problems.append('%d names resolved only by the address in their name: %s' % (
            len(lk.by_convention), ' '.join(sorted(lk.by_convention)[:12])))
    if lk.unresolved:
        problems.append('unresolved: %s' % ' '.join(sorted(lk.unresolved)[:12]))
    problems += ['link: ' + n for n in lk.notes if 'measured' not in n]
    for p in problems:
        print('problem: ' + p)
    if not a.keep and not a.no_build:
        shutil.rmtree(obj, ignore_errors=True)
    ok = same == len(GOG_SHA1) and not problems
    print('%d of %d files identical to the GOG release: %s' % (same, len(GOG_SHA1), 'PASS' if ok else 'FAIL'))
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
