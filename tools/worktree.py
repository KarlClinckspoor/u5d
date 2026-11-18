#!/usr/bin/env python3
"""Make (or remove) a git worktree that is ready to build, one per worker.

    python tools/worktree.py NAME                 create ../u5d-NAME on a new branch work/NAME
    python tools/worktree.py NAME --remove        remove it again (the branch is kept)
    python tools/worktree.py --list

A worktree is a second checkout of the same repository, in its own folder and on its own
branch: its src/, its build/obj and its build/match/status.tsv are its own, so workers in
different worktrees cannot disturb each other. The new one is put beside the main checkout
and its branch starts at the commit checked out here (or --from REF).

build/ is not in git, so a new worktree would have no compilers. This copies into it:

  build/msc51, build/msc50, build/masm    the compilers and the assembler
  build/obj, build/match                  the objects and the status file as they are here

and then compiles and compares one small module (swdisks.c) inside the new worktree, to show
that it works. The game files and DOSBox-X are looked up as usual (tools/u5.py): ../gamedata
and the U6 project next door are found from a worktree too; a gamedata/ folder inside this
checkout is not, so set U5_GAMEDATA then. The Python environment (.venv) is not copied:
run the tools of the worktree with the Python of the main checkout.

--remove refuses when the worktree has changes that are not committed (--force removes it
anyway). The branch work/NAME stays unless --delete-branch is given; git refuses to delete
a branch that is not merged, unless --force is given too.
"""
import argparse, os, re, shutil, subprocess, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402

COPIED = ('msc51', 'msc50', 'masm', 'obj', 'match')
CHECK = 'swdisks.c'


def git(*args, cwd=None, check=True):
    r = subprocess.run(('git',) + args, cwd=cwd or u5.ROOT, capture_output=True, text=True)
    if check and r.returncode:
        sys.exit('git %s failed:\n%s%s' % (' '.join(args), r.stdout, r.stderr))
    return r


def main_checkout():
    """The folder of the main checkout (the one that holds .git), also when this runs in a worktree."""
    common = git('rev-parse', '--path-format=absolute', '--git-common-dir').stdout.strip()
    return os.path.dirname(os.path.normpath(common))


def place(name):
    main = main_checkout()
    return os.path.join(os.path.dirname(main), '%s-%s' % (os.path.basename(main), name))


def create(name, start, check):
    path, branch = place(name), 'work/' + name
    if os.path.exists(path):
        sys.exit('%s exists already' % path)
    if git('rev-parse', '--verify', '--quiet', 'refs/heads/' + branch, check=False).returncode == 0:
        if start:
            sys.exit('the branch %s exists already: leave out --from to check it out' % branch)
        git('worktree', 'add', path, branch)
        print('%s: the existing branch %s' % (path, branch))
    else:
        git('worktree', 'add', path, '-b', branch, start or 'HEAD')
        print('%s: new branch %s from %s' % (path, branch, start or git('rev-parse', '--short', 'HEAD').stdout.strip()))
    for d in COPIED:
        s = os.path.join(u5.BUILD, d)
        if os.path.isdir(s):
            shutil.copytree(s, os.path.join(path, 'build', d), ignore=shutil.ignore_patterns('*.tmp'))
            print('  copied build/%s' % d)
        elif d in ('msc51', 'msc50'):
            print('  build/%s is missing here: run tools/setup_msc51.py' % d)
    if git('status', '--porcelain', '--', 'src').stdout.strip() or start:
        print('  src/ here differs from the new worktree (changes not committed, or --from): the copied '
              'objects and status are not those of its sources. Run tools/match.py --compile there.')
    if not check:
        return
    # compile and compare one module there, with the worktree's own tools and build folder
    stem = os.path.splitext(CHECK)[0].upper()
    obj = os.path.join(path, 'build', 'obj', stem + '.OBJ')
    if os.path.exists(obj):
        os.remove(obj)
    env = {k: v for k, v in os.environ.items() if k not in ('U5_OBJ', 'U5_SRC')}
    r = subprocess.run([sys.executable, os.path.join(path, 'tools', 'match.py'), CHECK, '--compile'],
                       cwd=path, env=env, capture_output=True, text=True)
    print('  check: match.py %s --compile, in the worktree:' % CHECK)
    for line in (r.stdout + r.stderr).splitlines():
        print('    ' + line)
    if r.returncode or not os.path.exists(obj):
        sys.exit('the check failed: the worktree is left as it is at %s' % path)
    print('ready. Work there with this Python:\n  cd %s\n  %s tools/match.py ...' % (path, sys.executable))


def remove(name, force, delete_branch):
    path, branch = place(name), 'work/' + name
    r = git('worktree', 'remove', *(['--force'] if force else []), path, check=False)
    if r.returncode:
        sys.exit('git worktree remove failed (nothing was removed by this tool):\n%s%s' % (r.stdout, r.stderr))
    print('removed %s' % path)
    if delete_branch:
        r = git('branch', '-D' if force else '-d', branch, check=False)
        print((r.stdout + r.stderr).strip())
        if r.returncode:
            sys.exit(1)
    else:
        print('the branch %s is kept' % branch)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('name', nargs='?', help='letters, digits, - and _')
    ap.add_argument('--from', dest='start', metavar='REF', help='start the new branch here (default: the commit checked out)')
    ap.add_argument('--no-check', action='store_true', help='do not compile and compare %s in the new worktree' % CHECK)
    ap.add_argument('--remove', action='store_true')
    ap.add_argument('--force', action='store_true', help='with --remove: also when it has changes that are not committed')
    ap.add_argument('--delete-branch', action='store_true', help='with --remove: delete work/NAME too')
    ap.add_argument('--list', action='store_true')
    a = ap.parse_args()
    if a.list:
        print(git('worktree', 'list').stdout, end='')
        return
    if not a.name or not re.fullmatch(r'[A-Za-z0-9_-]+', a.name):
        ap.error('give a NAME of letters, digits, - and _')
    if a.remove:
        remove(a.name, a.force, a.delete_branch)
    else:
        create(a.name, a.start, not a.no_check)


if __name__ == '__main__':
    main()
