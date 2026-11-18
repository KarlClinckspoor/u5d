"""A reader for the Intel OMF object files Microsoft C 5.1 writes.

Only what the matching tools need: segments and their data, public and local public
names, external names, communal variables with their sizes, and fixups with their targets
resolved to names or segments.
"""
import struct

THEADR, COMENT, MODEND, EXTDEF, PUBDEF, LINNUM, LNAMES, SEGDEF, GRPDEF, FIXUPP, LEDATA, \
    LIDATA, COMDEF, LEXTDEF, LPUBDEF, LCOMDEF = (0x80, 0x88, 0x8A, 0x8C, 0x90, 0x94, 0x96, 0x98,
                                                0x9A, 0x9C, 0xA0, 0xA2, 0xB0, 0xB4, 0xB6, 0xB8)

# Fixup location kinds (the "loc" field of a FIXUP subrecord).
LOC_LOBYTE, LOC_OFFSET, LOC_BASE, LOC_POINTER, LOC_HIBYTE = 0, 1, 2, 3, 4
LOC_SIZE = {LOC_LOBYTE: 1, LOC_OFFSET: 2, LOC_BASE: 2, LOC_POINTER: 4, LOC_HIBYTE: 1, 5: 2}


class Segment:
    def __init__(self, name, cls, length, align=1, combine=2):
        self.name, self.cls, self.length = name, cls, length
        self.align, self.combine = align, combine    # SEGDEF: 0 absolute, 1 byte, 2 word, 3 paragraph, 4 page;
                                                     # combine 2/4/7 public, 5 stack, 6 common
        self.data = bytearray(length)
        self.fixups = []      # Fixup objects, .loc is the offset in this segment
        self.publics = {}     # name -> offset (public and local public)


class Fixup:
    """One fixup. self_rel: True for a relative (call/jmp) fixup. target: ('ext', name),
    ('seg', segname) or ('grp', grpname). disp: target displacement (from the record, or
    for frames without one, from the bytes at loc as MSC writes them)."""
    def __init__(self, loc, kind, self_rel, target, disp, frame):
        self.loc, self.kind, self.self_rel, self.target, self.disp, self.frame = \
            loc, kind, self_rel, target, disp, frame

    @property
    def size(self):
        return LOC_SIZE.get(self.kind, 2)

    def __repr__(self):
        return 'Fixup(%04x %s%s %r+%d)' % (self.loc, 'rel ' if self.self_rel else '', self.kind,
                                          self.target, self.disp)


class Object:
    def __init__(self, path, data=None):
        self.path = path
        self.lnames = [None]
        self.segs = [None]
        self.grps = [None]
        self.groups = {}         # group name -> its segments' names
        self.exts = [None]
        self.comdefs = {}        # communal variables: name -> size in bytes
        self.absolutes = {}      # absolute publics: name -> value
        self.module = None
        self.comments = []
        self._read(data if data is not None else open(path, 'rb').read())

    def seg(self, name):
        for s in self.segs[1:]:
            if s.name == name:
                return s
        return None

    # -- reading
    @staticmethod
    def _idx(r, j):
        if r[j] & 0x80:
            return ((r[j] & 0x7f) << 8) | r[j + 1], j + 2
        return r[j], j + 1

    @staticmethod
    def _name(r, j):
        n = r[j]
        return r[j + 1:j + 1 + n].decode('latin-1'), j + 1 + n

    def _read(self, d):
        i = 0
        last = None              # (segment, offset) of the last LEDATA, for FIXUPP
        threads = {}             # ('t'|'f', n) -> (method, index)
        while i < len(d):
            t = d[i]
            length = struct.unpack_from('<H', d, i + 1)[0]
            r = d[i + 3:i + 3 + length - 1]
            i += 3 + length
            base = t & ~1        # 32-bit variants are not used by MSC 5.1
            if base == THEADR:
                self.module = self._name(r, 0)[0]
            elif base == COMENT:
                self.comments.append((r[1], bytes(r[2:])))
            elif base == LNAMES:
                j = 0
                while j < len(r):
                    s, j = self._name(r, j)
                    self.lnames.append(s)
            elif base == SEGDEF:
                acbp = r[0]
                j = 1
                if (acbp >> 5) == 0:
                    j += 3
                ln = struct.unpack_from('<H', r, j)[0]
                if acbp & 2:
                    ln = 0x10000
                nm, j = self._idx(r, j + 2)
                cl, j = self._idx(r, j)
                self.segs.append(Segment(self.lnames[nm], self.lnames[cl], ln, acbp >> 5, (acbp >> 2) & 7))
            elif base == GRPDEF:
                nm, j = self._idx(r, 0)
                self.grps.append(self.lnames[nm])
                members = self.groups.setdefault(self.lnames[nm], [])
                while j < len(r):
                    si, j = self._idx(r, j + 1)          # 0xff: a segment index follows
                    members.append(self.segs[si].name)
            elif base in (EXTDEF, LEXTDEF):
                j = 0
                while j < len(r):
                    s, j = self._name(r, j)
                    _, j = self._idx(r, j)
                    self.exts.append(s)
            elif base == COMDEF or base == LCOMDEF:
                j = 0
                while j < len(r):
                    s, j = self._name(r, j)
                    _, j = self._idx(r, j)
                    kind = r[j]; j += 1
                    size = 1
                    for _ in range(2 if kind == 0x61 else 1):   # far: count, element size
                        b = r[j]; j += 1
                        if b == 0x81: b = struct.unpack_from('<H', r, j)[0]; j += 2
                        elif b == 0x84: b = int.from_bytes(r[j:j + 3], 'little'); j += 3
                        elif b == 0x88: b = struct.unpack_from('<I', r, j)[0]; j += 4
                        size *= b
                    self.exts.append(s)
                    self.comdefs[s] = size
            elif base in (PUBDEF, LPUBDEF):
                j = 0
                _, j = self._idx(r, j)
                si, j = self._idx(r, j)
                if si == 0:
                    j += 2
                while j < len(r):
                    s, j = self._name(r, j)
                    off = struct.unpack_from('<H', r, j)[0]
                    j += 2
                    _, j = self._idx(r, j)
                    if si:
                        self.segs[si].publics[s] = off
                    else:
                        self.absolutes[s] = off          # an absolute public (crt0's __acrtused)
            elif base == MODEND:
                break
            elif base == LEDATA:
                si, j = self._idx(r, 0)
                off = struct.unpack_from('<H', r, j)[0]
                data = r[j + 2:]
                seg = self.segs[si]
                seg.data[off:off + len(data)] = data
                last = (seg, off)
            elif base == LIDATA:
                si, j = self._idx(r, 0)
                off = struct.unpack_from('<H', r, j)[0]
                data, _ = self._iter(r, j + 2)
                seg = self.segs[si]
                seg.data[off:off + len(data)] = data
                last = None      # fixups after LIDATA are not used by MSC for code
            elif base == FIXUPP:
                self._fixupp(r, last, threads)

    def _iter(self, r, j):
        rep = struct.unpack_from('<H', r, j)[0]
        blocks = struct.unpack_from('<H', r, j + 2)[0]
        j += 4
        if blocks == 0:
            n = r[j]
            out = bytes(r[j + 1:j + 1 + n])
            j += 1 + n
        else:
            out = b''
            for _ in range(blocks):
                part, j = self._iter(r, j)
                out += part
        return out * rep, j

    def _target(self, method, index):
        m = method & 3
        if m == 0:
            return ('seg', self.segs[index].name)
        if m == 1:
            return ('grp', self.grps[index])
        if m == 2:
            return ('ext', self.exts[index])
        return ('abs', index)

    def _fixupp(self, r, last, threads):
        j = 0
        while j < len(r):
            b = r[j]
            if not b & 0x80:                       # THREAD
                d, method, n = (b >> 6) & 1, (b >> 2) & 7, b & 3
                j += 1
                index = 0
                if d == 0 or method < 3:           # target threads always carry one
                    index, j = self._idx(r, j)
                threads[('f' if d else 't', n)] = (method, index)
                continue
            locat = (b << 8) | r[j + 1]
            j += 2
            self_rel = not (locat & 0x4000)
            kind = (locat >> 10) & 0xf
            dro = locat & 0x3ff
            fd = r[j]
            j += 1
            if fd & 0x80:
                frame = threads[('f', (fd >> 4) & 3)]
            else:
                fm = (fd >> 4) & 7
                fi = 0
                if fm < 3:
                    fi, j = self._idx(r, j)
                frame = (fm, fi)
            if fd & 0x08:
                tm, ti = threads[('t', fd & 3)]
                tm = (tm & 3) | (fd & 4)
            else:
                tm = fd & 7
                ti, j = self._idx(r, j)
            disp = 0
            if not (tm & 4):
                disp = struct.unpack_from('<H', r, j)[0]
                j += 2
            if last is None:
                continue
            seg, off = last
            seg.fixups.append(Fixup(off + dro, kind, self_rel, self._target(tm, ti), disp, frame))


def library_modules(path):
    """The object modules of an OMF library (.LIB): [(name, Object)]."""
    d = open(path, 'rb').read()
    if d[0] != 0xF0:
        raise ValueError('%s is not an OMF library' % path)
    page = struct.unpack_from('<H', d, 1)[0] + 3
    out = []
    i = page
    while i < len(d) and d[i] == THEADR:
        start = i
        while True:
            t = d[i]
            n = struct.unpack_from('<H', d, i + 1)[0]
            i += 3 + n
            if t in (MODEND, MODEND + 1):
                break
        obj = Object('%s(%d)' % (path, start), d[start:i])
        out.append((obj.module, obj))
        i = (i + page - 1) // page * page
        if i < len(d) and d[i] == 0xF1:       # end of the modules: the dictionary follows
            break
    return out
