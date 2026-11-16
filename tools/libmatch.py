#!/usr/bin/env python3
"""Find Microsoft C library modules in ULTIMA.EXE's resident image.

    python tools/libmatch.py [LIB ...]

For every module of the libraries (default: build/msc51/LIB/SLIBCR.LIB, EM.LIB, SLIBFP.LIB,
LIBH.LIB), its code segment (_TEXT, or any segment of class CODE) is searched in the image
with the bytes under fixups masked. Each match is listed with its address; a module whose
code occurs exactly once at a place the map (tools/rootmap.py) calls CRT is the library
the original was linked with.
"""
import argparse, glob, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omf  # noqa: E402
import u5  # noqa: E402


def code_segments(obj):
    for s in obj.segs[1:]:
        if s.length and (s.name.endswith('_TEXT') or s.cls == 'CODE'):
            yield s


def find(img, seg):
    """Addresses where seg's bytes occur in img, fixup bytes masked."""
    masked = {k for fx in seg.fixups for k in range(fx.loc, fx.loc + fx.size)}
    data = bytes(seg.data)
    # anchor on the longest unmasked run
    best, run = (0, 0), None
    i = 0
    while i < len(data):
        if i in masked:
            i += 1
            continue
        j = i
        while j < len(data) and j not in masked:
            j += 1
        if j - i > best[1] - best[0]:
            best = (i, j)
        i = j
    a, b = best
    if b - a < 4:
        return []
    out = []
    start = 0
    needle = data[a:b]
    while True:
        k = img.find(needle, start)
        if k < 0:
            break
        base = k - a
        if base >= 0 and base + len(data) <= len(img) and all(
                img[base + x] == data[x] for x in range(len(data)) if x not in masked):
            out.append(base)
        start = k + 1
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('libs', nargs='*')
    a = ap.parse_args()
    libs = a.libs or [os.path.join(u5.MSC51, 'LIB', n) for n in ('SLIBCR.LIB', 'EM.LIB', 'SLIBFP.LIB', 'LIBH.LIB')]
    img = u5.exe_image(u5.find_gamedata())
    hits = []
    for lib in libs:
        for name, obj in omf.library_modules(lib):
            for seg in code_segments(obj):
                for at in find(img, seg):
                    hits.append((at, at + seg.length, os.path.basename(lib), name, seg.name))
    for at, end, lib, name, segname in sorted(hits):
        print('%04x-%04x %5d  %-11s %-22s %s' % (at, end, end - at, lib, name, segname))


if __name__ == '__main__':
    main()
