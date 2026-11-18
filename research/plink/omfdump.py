"""Dump the records of an OMF object or library: omfdump.py FILE [MODULE]
SEGDEF alignment/combine, GRPDEF, EXTDEF/PUBDEF/COMDEF order, COMENTs."""
import struct, sys

AL = {0: 'ABS', 1: 'BYTE', 2: 'WORD', 3: 'PARA', 4: 'PAGE', 5: 'DWORD'}
CB = {0: 'PRIVATE', 2: 'PUBLIC', 4: 'PUBLIC', 5: 'STACK', 6: 'COMMON', 7: 'PUBLIC'}


def idx(r, j):
    if r[j] & 0x80:
        return ((r[j] & 0x7f) << 8) | r[j + 1], j + 2
    return r[j], j + 1


def name(r, j):
    n = r[j]
    return r[j + 1:j + 1 + n].decode('latin-1'), j + 1 + n


def modules(d):
    if d[0] == 0xF0:
        page = struct.unpack_from('<H', d, 1)[0] + 3
        i = page
        while i < len(d) and d[i] == 0x80:
            s = i
            while True:
                t = d[i]; n = struct.unpack_from('<H', d, i + 1)[0]; i += 3 + n
                if t in (0x8a, 0x8b):
                    break
            yield d[s:i]
            i = (i + page - 1) // page * page
    else:
        yield d


def dump(m, out=print):
    i = 0; ln = [None]; segs = [None]; exts = [None]; grps = [None]
    while i < len(m):
        t = m[i]; n = struct.unpack_from('<H', m, i + 1)[0]; r = m[i + 3:i + 2 + n]; i += 3 + n
        if t == 0x80:
            out('THEADR %s' % name(r, 0)[0])
        elif t == 0x88:
            out('COMENT %02x %02x %r' % (r[0], r[1], bytes(r[2:])))
        elif t == 0x96:
            j = 0
            while j < len(r):
                s, j = name(r, j); ln.append(s)
        elif t == 0x98:
            a = r[0]; j = 1
            if a >> 5 == 0:
                j += 3
            l = struct.unpack_from('<H', r, j)[0]; j += 2
            if a & 2: l = 0x10000
            nm, j = idx(r, j); cl, j = idx(r, j); ov, j = idx(r, j)
            segs.append(ln[nm])
            out('SEGDEF %d %s %s %s %s len=%x' % (len(segs) - 1, ln[nm], ln[cl], AL.get(a >> 5), CB.get((a >> 2) & 7), l))
        elif t == 0x9a:
            nm, j = idx(r, 0); mem = []
            while j < len(r):
                j += 1; si, j = idx(r, j); mem.append(segs[si])
            grps.append(ln[nm]); out('GRPDEF %s %s' % (ln[nm], mem))
        elif t in (0x8c, 0xb4):
            j = 0; l = []
            while j < len(r):
                s, j = name(r, j); _, j = idx(r, j); l.append(s); exts.append(s)
            out('EXTDEF %s' % ' '.join(l))
        elif t in (0xb0, 0xb8):
            j = 0; l = []
            while j < len(r):
                s, j = name(r, j); _, j = idx(r, j); k = r[j]; j += 1
                for _ in range(2 if k == 0x61 else 1):
                    b = r[j]; j += 1
                    if b == 0x81: j += 2
                    elif b == 0x84: j += 3
                    elif b == 0x88: j += 4
                l.append(s); exts.append(s)
            out('COMDEF %s' % ' '.join(l))
        elif t in (0x90, 0xb6):
            g, j = idx(r, 0); si, j = idx(r, j)
            if si == 0: j += 2
            l = []
            while j < len(r):
                s, j = name(r, j); off = struct.unpack_from('<H', r, j)[0]; j += 2; _, j = idx(r, j)
                l.append('%s=%x' % (s, off))
            out('%s %s %s' % ('PUBDEF' if t == 0x90 else 'LPUBDEF', segs[si] if si else 'abs', ' '.join(l)))


if __name__ == '__main__':
    d = open(sys.argv[1], 'rb').read()
    for m in modules(d):
        if len(sys.argv) > 2:
            nm = name(m, 3)[0]
            if nm.upper() != sys.argv[2].upper():
                continue
        dump(m)
