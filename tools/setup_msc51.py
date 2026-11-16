#!/usr/bin/env python3
"""Install Microsoft C 5.1 into build/msc51 from its floppy images.

    python tools/setup_msc51.py ARCHIVE_OR_DIR

ARCHIVE_OR_DIR is either the folder holding the 5.25" disk images
("Setup (01521).img", "Compiler 1 (01522).img", ...) or the .7z archive they came in
(7-Zip's 7z must then be on PATH or in its usual Windows folder). The images are FAT12
floppies; this script reads them itself, so mtools is not needed.

It copies the compiler passes, the linker and librarian, the small/medium-model libraries
and the include files, then checks the executables against known MD5s. Nothing from
Microsoft is ever committed: build/ is ignored by git.
"""
import argparse, hashlib, os, shutil, struct, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import u5  # noqa: E402

# disk label prefix -> what to take from it ('*.EXE' style patterns), and where it goes
DISKS = [
    ('Setup', ['CL.EXE', 'CL.ERR', 'CL.HLP', 'C1.EXE', 'C23.ERR'], 'BIN'),
    ('Compiler 1', ['C1L.EXE', 'C3.EXE', 'C1.ERR'], 'BIN'),
    ('Compiler 2', ['C2.EXE'], 'BIN'),
    ('Utilities 1', ['LINK.EXE', 'LIB.EXE', 'EXEHDR.EXE', 'EXEMOD.EXE', 'EXEPACK.EXE', 'MAKE.EXE'], 'BIN'),
    ('Libraries (Small and Medium Models)', ['*'], 'LIB'),
    ('Floating-Point Libraries', ['EM.LIB', '87.LIB', 'SLIBFP.LIB', 'MLIBFP.LIB', 'LIBH.LIB'], 'LIB'),
    ('Include Files and Startup Source Code', ['INCLUDE/*', 'INCLUDE/SYS/*'], ''),
    ('Include Files and Startup Source Code', ['STARTUP/*', 'STARTUP/DOS/*'], ''),
]

# MD5s of the executables of the copy this project was matched with.
MD5 = {
    'BIN/CL.EXE': 'c8cad528f30804623d2ed6bace171460',
    'BIN/C1.EXE': '3597c4c7108ba99f8cc731804795b9ce',
    'BIN/C1L.EXE': '2fe5c93772ec24060817d0911febe921',
    'BIN/C2.EXE': '37ed663823aea0ec1d39d2c98f2bd924',
    'BIN/C3.EXE': '3f09668d138e5480698860c4db0da4b1',
    'BIN/LINK.EXE': '07639b628b60f8f02c9d65bfd67895c8',
}


class Fat12:
    """A FAT12 floppy image: list and read files (8.3 names, subdirectories)."""

    def __init__(self, path):
        self.d = open(path, 'rb').read()
        b = self.d
        self.bps = struct.unpack_from('<H', b, 11)[0]
        self.spc = b[13]
        res = struct.unpack_from('<H', b, 14)[0]
        nfat = b[16]
        self.nroot = struct.unpack_from('<H', b, 17)[0]
        spf = struct.unpack_from('<H', b, 22)[0]
        self.fat = res * self.bps
        self.root = (res + nfat * spf) * self.bps
        self.data = self.root + self.nroot * 32

    def _next(self, c):
        o = self.fat + c * 3 // 2
        v = struct.unpack_from('<H', self.d, o)[0]
        return (v >> 4) if c & 1 else (v & 0xfff)

    def _chain(self, c):
        out = bytearray()
        size = self.spc * self.bps
        while 2 <= c < 0xff8:
            o = self.data + (c - 2) * size
            out += self.d[o:o + size]
            c = self._next(c)
        return bytes(out)

    def _entries(self, raw):
        for i in range(0, len(raw), 32):
            e = raw[i:i + 32]
            if e[0] == 0:
                break
            if e[0] == 0xe5 or e[11] == 0x0f or e[11] & 0x08:
                continue
            name = e[0:8].decode('latin-1').rstrip()
            ext = e[8:11].decode('latin-1').rstrip()
            if name in ('.', '..'):
                continue
            full = name + ('.' + ext if ext else '')
            yield full, bool(e[11] & 0x10), struct.unpack_from('<H', e, 26)[0], \
                struct.unpack_from('<I', e, 28)[0]

    def walk(self, raw=None, prefix=''):
        if raw is None:
            raw = self.d[self.root:self.root + self.nroot * 32]
        for name, is_dir, cl, size in self._entries(raw):
            if is_dir:
                yield from self.walk(self._chain(cl), prefix + name + '/')
            else:
                yield prefix + name, self._chain(cl)[:size]


def match(pattern, path):
    pd, pf = os.path.split(pattern)
    d, f = os.path.split(path)
    return pd.upper() == d.upper() and (pf == '*' or pf.upper() == f.upper())


def find_images(src, tmp):
    if os.path.isdir(src):
        top = src
    else:
        z = shutil.which('7z') or next((p for p in (r'C:\Program Files\7-Zip\7z.exe',
                                                     r'C:\Program Files (x86)\7-Zip\7z.exe')
                                        if os.path.exists(p)), None)
        if not z:
            sys.exit('7-Zip is needed to open %s (or extract it yourself and pass the folder)' % src)
        subprocess.run([z, 'x', '-y', '-o' + tmp, src], check=True, stdout=subprocess.DEVNULL)
        top = tmp
    imgs = {}
    for dp, _, fs in os.walk(top):
        for f in fs:
            if f.lower().endswith('.img'):
                imgs[f] = os.path.join(dp, f)
    return imgs


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('source', help='the MSC 5.1 .7z archive or the folder of its .img files')
    ap.add_argument('--dest', default=u5.MSC51)
    a = ap.parse_args()
    with tempfile.TemporaryDirectory() as tmp:
        imgs = find_images(a.source, tmp)
        if not imgs:
            sys.exit('no .img files found in %s' % a.source)
        if os.path.exists(a.dest):
            shutil.rmtree(a.dest)
        for label, patterns, sub in DISKS:
            img = next((p for n, p in imgs.items() if n.startswith(label + ' (')), None)
            if not img:
                sys.exit('disk image "%s (...).img" is missing' % label)
            for path, data in Fat12(img).walk():
                if any(match(p, path) for p in patterns):
                    out = os.path.join(a.dest, sub, path)
                    os.makedirs(os.path.dirname(out), exist_ok=True)
                    open(out, 'wb').write(data)
    for d in ('TMP',):
        os.makedirs(os.path.join(a.dest, d), exist_ok=True)
    bad = 0
    for rel, want in MD5.items():
        p = os.path.join(a.dest, rel)
        got = hashlib.md5(open(p, 'rb').read()).hexdigest() if os.path.exists(p) else 'missing'
        if got != want:
            print('%s: md5 %s, expected %s' % (rel, got, want))
            bad += 1
    n = sum(len(fs) for _, _, fs in os.walk(a.dest))
    print('installed %d files in %s' % (n, a.dest))
    if bad:
        sys.exit('%d executables differ from the copy the project was matched with' % bad)
    print('compiler executables match the known MD5s')


if __name__ == '__main__':
    main()
