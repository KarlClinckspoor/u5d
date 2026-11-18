"""Q2: test the within-slot key: sum of (c & 0xDF) mod 256, ascending, ties newest first."""
import os, sys, pickle
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk
data = pickle.load(open(os.path.join(rk.WORK, 'q2fit.pkl'), 'rb'))

def key(n, mask=0xDF, mod=256):
    return sum(ord(c) & mask for c in n) % mod

def predict(names, c0=9, mask=0xDF, mod=256):
    return [names[i].upper() for i in sorted(range(len(names)), key=lambda i: ((i + c0) % 64, key(names[i], mask, mod), -i))]

for mask in (0xDF, 0xFF, 0x5F, 0x1F, 0x7F):
    for mod in (256, 128, 512, 65536):
        ok = sum(predict(names, mask=mask, mod=mod) == got for names, got in data.values())
        if ok:
            print('mask %02x mod %d: %d of %d tests predicted exactly' % (mask, mod, ok, len(data)))
