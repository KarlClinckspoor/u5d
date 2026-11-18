import os, sys, random
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rk
from q2_comdef import comdef_order
from q2_predict import rnd_name
r = random.Random(11)
names = []; seen = set()
while len(names) < 150:
    n = rnd_name(r, 0)
    if n.upper() not in seen and n not in ('do', 'if', 'for', 'int', 'char'):
        seen.add(n.upper()); names.append(n)
S = {'R0': '\n'.join('int %s;' % n for n in names) + '\n'}
caps = rk.capture(S, sys.argv[1] if len(sys.argv) > 1 else 'msc51')
got = comdef_order(caps['R0']['OBJ'])
idx = {n.upper(): i for i, n in enumerate(names)}
for g in got:
    i = idx[g]
    print('%3d slot %2d len %2d sum %4d %s' % (i, (i + 9) % 64, len(g), sum(map(ord, g)), names[i]))
