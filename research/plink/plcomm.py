"""The communal variables as PLINK allocated them (MAP ... M, the C_COMMON listing) against
the link command file's communals (the original's order): plcomm.py WORKDIR"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(HERE)), 'tools'))
import u5link  # noqa: E402

text = open(os.path.join(sys.argv[1], 'ULTIMA.MAP'), encoding='latin-1').read()
blk = text[text.index('C_COMMON'):]
blk = blk[:blk.index('\n\n')] if '\n\n' in blk else blk
got = {}
for m in re.finditer(r'(D_[0-9A-F]{4})\w*\s+[0-9a-f]{4}:([0-9a-f]{4})', blk):
    got[m.group(1)] = int(m.group(2), 16)
want = [u5link.exported(n) for n in u5link.Script().communals]
order = sorted(got, key=got.get)
print('%d communals; at their original address: %d' % (len(got), sum(1 for k, v in got.items() if int(k[2:], 16) == v)))
print('PLINK order :', ' '.join(order))
print('original    :', ' '.join(n[:6] for n in want))
