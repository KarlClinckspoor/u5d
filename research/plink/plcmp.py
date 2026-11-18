"""Compare a PLINK86plus link of u5d with the original: plcmp.py WORKDIR"""
import difflib, os, struct, sys
from collections import Counter
W = sys.argv[1]
G = 'C:/Users/karl9/Downloads/Claude/u5-decompiled/gamedata/'
e = open(os.path.join(W, 'ULTIMA.EXE'), 'rb').read()
o = open(G + 'ULTIMA.EXE', 'rb').read()
he = struct.unpack('<H', e[8:10])[0] * 16
ho = struct.unpack('<H', o[8:10])[0] * 16
A = e[he:he + 0x72e0]
B = o[ho:ho + 0x72e0]
sm = difflib.SequenceMatcher(None, B, A, autojunk=False)
blocks = [b for b in sm.get_matching_blocks() if b.size >= 16]
sh = Counter()
for b in blocks:
    sh[b.b - b.a] += b.size
print('root 0000-72e0: identical bytes %d of %d at the same offset; in blocks>=16 by shift: %s' % (
    sum(1 for i in range(len(B)) if A[i] == B[i]), len(B), [(k, v) for k, v in sh.most_common(5)]))
prev = None
for b in blocks:
    s = b.b - b.a
    if s != prev:
        print('  orig %04x ours %04x shift %+d' % (b.a, b.b, s))
        prev = s
print('header ours', ' '.join('%04x' % x for x in struct.unpack('<14H', e[:28])))
print('header orig', ' '.join('%04x' % x for x in struct.unpack('<14H', o[:28])))
