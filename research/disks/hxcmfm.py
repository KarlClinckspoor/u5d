"""Decode an HxC .mfm floppy image (IBM MFM) into a raw sector image.
    hxcmfm.py MFMFILE OUT.IMA [SECTORS_PER_TRACK]
Prints, per track, the sectors found and any odd ones (sizes, bad CRCs, extra sectors)."""
import struct, sys
from collections import defaultdict


def crc16(data, crc=0xffff):
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xffff if crc & 0x8000 else (crc << 1) & 0xffff
    return crc


def bits_of(raw):
    out = bytearray(len(raw) * 8)
    for i, b in enumerate(raw):
        for k in range(8):
            out[i * 8 + k] = (b >> (7 - k)) & 1
    return out


def decode(bits, start, nbytes):
    out = bytearray(nbytes)
    p = start
    for i in range(nbytes):
        v = 0
        for k in range(8):
            v = (v << 1) | bits[p + 1]
            p += 2
        out[i] = v
    return bytes(out)


SYNC = [int(c) for c in format(0x4489, '016b')]


def find_syncs(bits):
    s = ''.join('1' if x else '0' for x in bits)
    pat = format(0x4489, '016b') * 3
    i = s.find(pat)
    while i >= 0:
        yield i + len(pat)
        i = s.find(pat, i + 16)


def track_sectors(raw):
    bits = bits_of(raw) * 2  # wrap around once
    n = len(bits) // 2
    secs = {}
    last_id = None
    for p in find_syncs(bits):
        if p >= n + 2000:
            break
        mark = decode(bits, p, 1)[0]
        if mark == 0xfe:
            c, h, r, sz = decode(bits, p + 16, 4)
            crc = decode(bits, p + 80, 2)
            ok = crc16(bytes([0xa1, 0xa1, 0xa1, 0xfe, c, h, r, sz])) == struct.unpack('>H', crc)[0]
            last_id = (c, h, r, sz, ok, p)
        elif mark in (0xfb, 0xf8) and last_id:
            c, h, r, sz, ok, ip = last_id
            size = 128 << (sz & 7)
            data = decode(bits, p + 16, size)
            dcrc = decode(bits, p + 16 + size * 16, 2)
            dok = crc16(bytes([0xa1, 0xa1, 0xa1, mark]) + data) == struct.unpack('>H', dcrc)[0]
            key = r
            if key not in secs or (dok and not secs[key][2]):
                secs[key] = (c, h, data, dok, ok, size)
            last_id = None
    return secs


def main():
    b = open(sys.argv[1], 'rb').read()
    assert b[:6] == b'HXCMFM', b[:8]
    ntr, nside, rpm, rate, iftype, tloff = struct.unpack('<HBHHBI', b[7:19])
    spt = int(sys.argv[3]) if len(sys.argv) > 3 else 9
    img = bytearray(ntr * nside * spt * 512)
    print('tracks', ntr, 'sides', nside, 'rpm', rpm, 'rate', rate)
    odd = 0
    for k in range(ntr * nside):
        tn, sn, tsize, toff = struct.unpack('<HBII', b[tloff + 11 * k:tloff + 11 * k + 11])
        secs = track_sectors(b[toff:toff + tsize])
        notes = []
        for r in sorted(secs):
            c, h, data, dok, idok, size = secs[r]
            if 1 <= r <= spt and size == 512:
                o = ((tn * nside + sn) * spt + r - 1) * 512
                img[o:o + 512] = data
            if size != 512 or not dok or not (1 <= r <= spt) or c != tn or h != sn:
                notes.append('r%d c%d h%d size %d crc %s' % (r, c, h, size, 'ok' if dok else 'BAD'))
        missing = [r for r in range(1, spt + 1) if r not in secs]
        if notes or missing:
            odd += 1
            print('track %2d side %d: %d sectors%s%s' % (tn, sn, len(secs), ' missing %s' % missing if missing else '',
                                                      ' odd: ' + '; '.join(notes) if notes else ''))
    open(sys.argv[2], 'wb').write(img)
    print('tracks with something odd:', odd)


main()
