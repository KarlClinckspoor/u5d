"""List and extract a FAT12 floppy image: fat12.py IMAGE OUTDIR  (or IMAGE from a zip: fat12.py ZIP MEMBER OUTDIR)"""
import os, struct, sys, zipfile


def load(args):
    if args[0].lower().endswith('.zip'):
        return zipfile.ZipFile(args[0]).read(args[1]), args[2]
    return open(args[0], 'rb').read(), args[1]


def files(img):
    bps, spc, rsv, nfat, nroot, tot, media, spf = struct.unpack('<HBHBHHBH', img[11:24])
    if bps not in (512,) or nfat not in (1, 2):
        # boot sector without a BPB (DOS 1.x disks): guess by size
        size = len(img)
        bps, spc, rsv, nfat, spf = 512, 2, 1, 2, {368640: 2, 737280: 3, 1474560: 9}.get(size, 2)
        nroot = {368640: 112, 737280: 112, 1474560: 224}.get(size, 112)
        if size == 1474560:
            spc = 1
    fat = img[rsv * bps:(rsv + spf) * bps]
    rootoff = (rsv + nfat * spf) * bps
    dataoff = rootoff + nroot * 32

    def nxt(c):
        o = c * 3 // 2
        v = fat[o] | fat[o + 1] << 8
        return (v >> 4) if c & 1 else (v & 0xfff)

    def chain(c, size):
        out = bytearray()
        while 2 <= c < 0xff0 and len(out) < size + bps * spc:
            o = dataoff + (c - 2) * spc * bps
            out += img[o:o + spc * bps]
            c = nxt(c)
        return bytes(out[:size])

    def walk(off, n, path):
        for k in range(n):
            e = img[off + 32 * k:off + 32 * k + 32]
            if not e or e[0] == 0:
                break
            if e[0] == 0xe5 or e[11] == 0x0f or e[11] & 8:
                continue
            name = e[:8].decode('latin-1').rstrip()
            ext = e[8:11].decode('latin-1').rstrip()
            full = name + ('.' + ext if ext else '')
            t, d, clus, size = struct.unpack('<HHHI', e[22:32])
            date = '%04d-%02d-%02d %02d:%02d' % (1980 + (d >> 9), (d >> 5) & 15, d & 31, t >> 11, (t >> 5) & 63)
            if e[11] & 0x10:
                if full in ('.', '..'):
                    continue
                data = chain(clus, 0x10000)
                yield from walk_buf(data, path + full + '/')
            else:
                yield path + full, date, chain(clus, size)

    def walk_buf(buf, path):
        for k in range(len(buf) // 32):
            e = buf[32 * k:32 * k + 32]
            if e[0] == 0:
                break
            if e[0] == 0xe5 or e[11] == 0x0f or e[11] & 8:
                continue
            name = e[:8].decode('latin-1').rstrip(); ext = e[8:11].decode('latin-1').rstrip()
            full = name + ('.' + ext if ext else '')
            t, d, clus, size = struct.unpack('<HHHI', e[22:32])
            date = '%04d-%02d-%02d %02d:%02d' % (1980 + (d >> 9), (d >> 5) & 15, d & 31, t >> 11, (t >> 5) & 63)
            if e[11] & 0x10:
                if full not in ('.', '..'):
                    yield from walk_buf(chain(clus, 0x10000), path + full + '/')
            else:
                yield path + full, date, chain(clus, size)

    yield from walk(rootoff, nroot, '')


if __name__ == '__main__':
    img, out = load(sys.argv[1:])
    os.makedirs(out, exist_ok=True)
    for name, date, data in files(img):
        p = os.path.join(out, name.replace('/', os.sep))
        os.makedirs(os.path.dirname(p) or out, exist_ok=True)
        open(p, 'wb').write(data)
        print('%-14s %7d %s' % (name, len(data), date))
