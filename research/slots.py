"""Dataset: for every compiled function, its locals (declaration order, type, first use,
use count) and the stack slots MSC 5.1 gave them (from the .COD listings)."""
import glob, os, re, sys, json
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC, OBJ = os.path.join(ROOT, 'src'), os.path.join(ROOT, 'build', 'obj')

def cod_slots():
    out = {}
    for p in glob.glob(os.path.join(OBJ, '*.COD')):
        cur = None
        for line in open(p, encoding='latin-1'):
            m = re.match(r'^(\w+)\tPROC NEAR', line)
            if m: cur = m.group(1); out[cur] = {}; continue
            m = re.match(r'^;\t(\w+) = (-?\d+)$', line.rstrip())
            if m and cur: out[cur][m.group(1)] = int(m.group(2))
            if line.startswith(';|***') and cur and out[cur]: pass
    return out

TYPES = r'(?:unsigned\s+|signed\s+|static\s+|register\s+)*(?:int|char|byte|u8|s8|u16|s16|u32|s32|uint|word|bool|long|short|void|S_\w+|Pair\w*|struct\s+\w+)\s*\**'
def src_funcs():
    out = {}
    for p in glob.glob(os.path.join(SRC, '*.c')):
        s = open(p, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'^(?:static\s+)?[\w\s\*]*?\b(\w+)\s*\(([^;{)]*)\)\s*\n\{', s, re.M):
            name, start = m.group(1), m.end()
            depth, i = 1, start
            while depth and i < len(s):
                depth += {'{': 1, '}': -1}.get(s[i], 0); i += 1
            body = s[start:i - 1]
            decls = []
            pos = 0
            for line in body.split('\n'):
                t = line.strip()
                if not t or t.startswith('//') or t.startswith('#'): pos += len(line) + 1; continue
                dm = re.match(r'^(' + TYPES + r')\s*(\w+)\s*(\[[^\]]*\])?\s*(=[^;]*)?;', t)
                if not dm: break
                decls.append((dm.group(2), dm.group(1).strip(), bool(dm.group(3)), bool(dm.group(4))))
                pos += len(line) + 1
            rest = body[pos:]
            toks = re.findall(r'\b\w+\b', rest)
            uses = {d[0]: toks.count(d[0]) for d in decls}
            first = {d[0]: (toks.index(d[0]) if d[0] in toks else 10**6) for d in decls}
            out[name.upper()] = dict(file=os.path.basename(p), name=name, decls=decls, uses=uses, first=first,
                                      params=[x.strip().split()[-1].strip('*') for x in m.group(2).split(',') if x.strip() and x.strip() != 'void'])
    return out

if __name__ == '__main__':
    slots, funcs = cod_slots(), src_funcs()
    data = []
    for k, sl in slots.items():
        f = funcs.get(k)
        if not f: continue
        locs = [d for d in f['decls'] if d[0] in sl and sl[d[0]] < 0]
        if len(locs) < 2: continue
        data.append(dict(func=k, file=f['file'], decls=locs, slots={d[0]: sl[d[0]] for d in locs},
                         uses=f['uses'], first=f['first'], all_decls=f['decls']))
    json.dump(data, open(os.path.join(ROOT, 'build', 'slots.json'), 'w'), indent=0)
    print(len(data), 'functions with 2+ locals in slots')
