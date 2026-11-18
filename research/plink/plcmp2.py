"""Compare a PLINK86plus link of u5d (pl.py's WORKDIR) with a target's files.

    plcmp2.py WORKDIR [--target gog|1988] [-v]

ULTIMA.EXE: PLINK's image against the original's image up to CS:81d0 (PLSTART, Origin's wrapper,
is not the linker's), by ranges (root code 0000-72e0, the loader 72e0-7780, the overlay table and
stubs 7780-81d0); each .OVL file and DATA.OVL whole. Prints the differing runs with -v."""
import argparse, os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import u5  # noqa: E402


def runs(a, b, base=0):
    out, i, n = [], 0, min(len(a), len(b))
    while i < n:
        if a[i] != b[i]:
            j = i
            while j < n and (a[j] != b[j] or any(a[k] != b[k] for k in range(j, min(j + 4, n)))):
                j += 1
            out.append((base + i, j - i))
            i = j
        else:
            i += 1
    return out


def exe_parts(d):
    hdr = struct.unpack_from('<H', d, 8)[0] * 16
    pages, last = struct.unpack_from('<HH', d, 2)
    size = pages * 512 - (512 - last if last else 0)
    return d[:hdr], d[hdr:size]


def compare(work, target, verbose=False, out=print):
    gd = u5.find_gamedata(target=target)
    res = {}
    e = os.path.join(work, 'ULTIMA.EXE')
    if not os.path.exists(e):
        out('no ULTIMA.EXE')
        return res
    h, img = exe_parts(open(e, 'rb').read())
    oh, oimg = exe_parts(open(os.path.join(gd, 'ULTIMA.EXE'), 'rb').read())
    out('ULTIMA.EXE  image %x (original up to PLSTART 81d0), header %x' % (len(img), len(h)))
    hw = struct.unpack('<14H', h[:28])
    out('  header ours ' + ' '.join('%04x' % x for x in hw))
    out('  header orig ' + ' '.join('%04x' % x for x in struct.unpack('<14H', oh[:28])))
    for name, a, b in (('root', 0, 0x72e0), ('loader', 0x72e0, 0x7780), ('table', 0x7780, 0x81d0)):
        A, B = img[a:b], oimg[a:b]
        same = sum(1 for i in range(min(len(A), len(B))) if A[i] == B[i])
        r = runs(A, B, a)
        res[name] = (same, b - a)
        out('  %-6s %04x-%04x: %5d of %5d identical, %d runs differ%s' % (
            name, a, b, same, b - a, len(r), (': ' + ' '.join('%04x+%x' % x for x in r[:40])) if verbose else ''))
    if len(img) > 0x81d0:
        out('  image beyond 81d0: %x bytes (DGROUP or overlays kept in the EXE?)' % (len(img) - 0x81d0))
    for f in sorted(os.listdir(gd)):
        if not f.upper().endswith('.OVL'):
            continue
        o = open(os.path.join(gd, f), 'rb').read()
        p = os.path.join(work, f.upper())
        if not os.path.exists(p):
            out('%-12s missing' % f)
            res[f] = None
            continue
        d = open(p, 'rb').read()
        r = runs(d, o)
        nd = sum(1 for i in range(min(len(d), len(o))) if d[i] != o[i])
        res[f] = nd if len(d) == len(o) else -1
        out('%-12s %s len %5x/%5x  %s%s' % (
            f, 'IDENTICAL' if d == o else 'differs  ', len(d), len(o),
            '' if d == o else '%d bytes in %d runs' % (nd, len(r)),
            (': ' + ' '.join('%04x+%x' % x for x in r[:30])) if verbose and d != o else ''))
    return res


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('work')
    ap.add_argument('--target', default='gog')
    ap.add_argument('-v', action='store_true')
    a = ap.parse_args()
    compare(a.work, a.target, a.v)
