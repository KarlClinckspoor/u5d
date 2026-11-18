#!/usr/bin/env python3
"""The gate: build all 25 files from source and compare them with the release.

    python tools/check.py [--target gog|1988|both] [--keep] [--no-build] [--gamedata DIR]

u5d builds two releases (tools/u5.py, TARGETS): the GOG release (gog) and the October 1988
release (1988). The gate checks both by default (--target both); --target gog or 1988 checks
one. For each target:

1. Compiles every C file of the DOS build (tools/dosbuild.py: MSC 5.1 or 5.0 per file) and
   assembles every module of src/asm the target has (tools/asmbuild.py: MASM 4.0) into a
   folder of its own, build/check/obj (gog) or build/check/obj-1988, starting empty: nothing
   from an earlier build is used. The 1988 target's sources are compiled and assembled with
   RELEASE_1988 defined.
2. Links them (tools/u5link.py, from the link command file src/u5d.lnk) into build/check/out or
   build/check/out-1988: ULTIMA.EXE, the 23 code overlays, FLAMES.OVL and DATA.OVL.
3. Compares the SHA-1 of each with the release's (u5.release_sha1()).

The GOG release is the 1988 link patched in 1992, and its EXE checksum is the 1988 link's
(tools/u5link.py): the gog build also builds the 1988 form of the sources that differ by
target (u5.target_dependent: those that use RELEASE_1988), into build/check/obj-1988. With
--target both, the 1988 target then takes every other object from the gog build: a source
that does not use RELEASE_1988 compiles to the same object for both. (--target 1988 alone
builds all of its objects itself.)

Building needs the repository, the compilers, MASM and DOSBox-X; steps 1 to 3 open no game
file. When a target's game files are found (gamedata/ or ../gamedata, gamedata-1988/ or
../gamedata-1988, $U5_GAMEDATA_GOG, $U5_GAMEDATA_1988, or $U5_GAMEDATA and --gamedata for a
single target) two more checks are made for it, and its last line says so:

4. the game files are the release's (their SHA-1s are the table's);
5. every placement u5link computed for the target is the original's, and the link command
   file's facts are the files' (tools/linkscript.py: measured from the game files and the
   objects just built).

Without them the comparison is with the SHA-1 table alone, which is the same test of the
25 files; 4 and 5 are skipped.

It passes (exit status 0) only when, for every target checked, all 25 are identical, every
object was built, and the link resolved no reference by the address in its name and left
none unresolved (and, with the game files, 4 and 5 hold).

--no-build links and compares what build/check/obj (and obj-1988) already hold. --keep
leaves them in place afterwards (build/check/out always stays).
"""
import argparse, hashlib, os, shutil, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402

CHECK = os.path.join(u5.BUILD, 'check')


def sha1(data):
    return hashlib.sha1(data).hexdigest()


def folders(target):
    suffix = '' if target == u5.DEFAULT_TARGET else '-' + target
    return os.path.join(CHECK, 'obj' + suffix), os.path.join(CHECK, 'out' + suffix)


def object_name(path):
    return os.path.splitext(os.path.basename(path))[0].upper()


def build(target, cfiles, asms, dosbox, problems):
    """Compile cfiles and assemble asms (paths) for target, into its folder (u5.OBJ)."""
    import asmbuild, dosbuild
    with u5.building(target, folders(target)[0]):
        if cfiles:
            res = dosbuild.compile_files(cfiles, dosbox, quiet=True)
            problems += ['%s (%s): %s' % (f, target, res[f][0]) for f in cfiles if res.get(f)]
        if asms:
            for p, (o, log) in asmbuild.assemble_many(asms, dosbox).items():
                if o is None:
                    problems.append('%s (%s): MASM failed' % (os.path.basename(p), target))


def check_target(target, gd, problems):
    """Link target's objects, compare the 25 files; with gd, the game-file checks. Returns
    the number of identical files."""
    import u5link
    obj, out = folders(target)
    with u5.building(target, obj):
        for f in sorted(u5.MODULES) + [os.path.basename(p) for p in u5.asm_modules()]:
            if not os.path.exists(os.path.join(obj, object_name(f) + '.OBJ')):
                problems.append('%s: no object' % f)
        print('linking %s ...' % u5.TARGET_TITLE[target], flush=True)
        shutil.rmtree(out, ignore_errors=True)
        lk, files = u5link.link(out)
        table = u5.release_sha1()
        same = 0
        for name in sorted(table, key=lambda n: (n != 'ULTIMA.EXE', n)):
            ours = files.get(name)
            h = sha1(ours) if ours is not None else '(not built)'
            ok = h == table[name]
            same += ok
            print('%-13s %s  %s' % (name, h, 'identical' if ok else 'DIFFERENT (wanted: %s)' % table[name]))
        print('EXE checksum %04x (%s)' % (lk.checksum_value, 'the %s link\'s' % u5.PATCH_OF[target]
                                           if target in u5.PATCH_OF else 'this link\'s'))
        for name in sorted(set(files) - set(table)):
            problems.append('%s: built, but not one of the 25 files' % name)
        if lk.by_convention:
            problems.append('%d names resolved only by the address in their name: %s' % (
                len(lk.by_convention), ' '.join(sorted(lk.by_convention)[:12])))
        if lk.unresolved:
            problems.append('unresolved: %s' % ' '.join(sorted(lk.unresolved)[:12]))
        problems += ['link: ' + n for n in lk.notes]
        if gd:
            for name, want in table.items():
                if sha1(u5.read_file(gd, name)) != want:
                    problems.append('%s in %s is not %s\'s file' % (name, gd, u5.TARGET_TITLE[target]))
            import linkscript
            lines, found = linkscript.check(gd)
            problems += ['link placement (tools/linkscript.py): ' + n for n in found]
    return same


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--target', choices=u5.TARGETS + ('both',), default='both',
                    help='the release(s) to build and compare (default: both)')
    ap.add_argument('--gamedata', help='the game files of the one target checked')
    ap.add_argument('--dosbox')
    ap.add_argument('--no-build', action='store_true', help='link and compare the objects already in build/check')
    ap.add_argument('--keep', action='store_true', help='keep build/check/obj (and obj-1988) afterwards')
    a = ap.parse_args()
    targets = list(u5.TARGETS) if a.target == 'both' else [a.target]
    if a.gamedata and len(targets) > 1:
        ap.error('--gamedata names the files of one target: use --target, or $U5_GAMEDATA_GOG and $U5_GAMEDATA_1988')
    if a.gamedata:
        os.environ['U5_GAMEDATA'] = os.path.abspath(a.gamedata)
    # the targets' builds: each target's own, and the patched-from target's form of the
    # sources that differ (the EXE checksum)
    needed = set(targets) | {u5.PATCH_OF[t] for t in targets if t in u5.PATCH_OF}
    for t in needed:
        os.makedirs(folders(t)[0], exist_ok=True)
        if not a.no_build:
            shutil.rmtree(folders(t)[0], ignore_errors=True)
            os.makedirs(folders(t)[0])
    problems = {t: [] for t in needed}
    if not a.no_build:
        cfiles = sorted(u5.MODULES)
        first = None        # the target built in full; the others take what does not differ from it
        for t in [t for t in u5.TARGETS if t in targets]:
            dep = u5.target_dependent_objects(t)
            if first:
                cs = [f for f in cfiles if object_name(f) in dep]
                asms = [p for p in u5.asm_modules(t) if object_name(p) in dep]
                rest = [object_name(f) for f in cfiles + u5.asm_modules(t) if object_name(f) not in dep]
                print('%s: building the %d objects that differ by target (%s); the other %d as for %s ...' % (
                    t, len(cs) + len(asms), ' '.join(sorted(object_name(f) for f in cs + asms)), len(rest), first),
                    flush=True)
                for n in rest:
                    p = os.path.join(folders(first)[0], n + '.OBJ')
                    if os.path.exists(p):
                        shutil.copy(p, os.path.join(folders(t)[0], n + '.OBJ'))
            else:
                first = t
                cs, asms = cfiles, u5.asm_modules(t)
                print('%s: compiling %d C files (MSC 5.1: %d, MSC 5.0: %d), assembling %d modules (MASM 4.0)%s ...' % (
                    t, len(cs), sum(1 for f in cs if u5.COMPILER_BY_FILE.get(f, 'msc51') == 'msc51'),
                    sum(1 for f in cs if u5.COMPILER_BY_FILE.get(f) == 'msc50'), len(asms),
                    ' with' + u5.target_defines(t) if u5.target_defines(t) else ''), flush=True)
            build(t, cs, asms, a.dosbox, problems[t])
            # the patched-from target's form of what differs, for this target's EXE checksum
            base = u5.PATCH_OF.get(t)
            if base and base not in targets:
                dep = u5.target_dependent_objects(base)
                cs = [f for f in cfiles if object_name(f) in dep]
                asms = [p for p in u5.asm_modules(base) if object_name(p) in dep]
                print('%s: building the %s form of %s (the EXE checksum) ...' % (
                    t, base, ' '.join(sorted(object_name(f) for f in cs + asms))), flush=True)
                build(base, cs, asms, a.dosbox, problems[t])
    result = {}
    for t in targets:
        print('== %s (target %s) ==' % (u5.TARGET_TITLE[t], t))
        gd = u5.find_gamedata(a.gamedata, required=False, target=t)     # only to check, not to build
        same = check_target(t, gd, problems[t])
        for p in problems[t]:
            print('problem: ' + p)
        n = len(u5.release_sha1(t))
        ok = same == n and not problems[t]
        result[t] = ok
        print('%d of %d files identical to %s: %s' % (same, n, u5.TARGET_TITLE[t], 'PASS' if ok else 'FAIL'))
        print('built without the game files; ' + (
            'the game files in %s are %s\'s and give u5link\'s placement' % (gd, u5.TARGET_TITLE[t]) if gd and ok else
            'game files found in %s' % gd if gd else
            'no game files found: compared with the SHA-1 table only, the placement not measured'))
    if not a.keep and not a.no_build:
        for t in needed:
            shutil.rmtree(folders(t)[0], ignore_errors=True)
    if len(targets) > 1:
        print('targets %s: %s' % (', '.join('%s %s' % (t, 'PASS' if result[t] else 'FAIL') for t in targets),
                                  'PASS' if all(result.values()) else 'FAIL'))
    sys.exit(0 if all(result.values()) else 1)


if __name__ == '__main__':
    main()
