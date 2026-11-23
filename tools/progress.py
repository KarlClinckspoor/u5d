#!/usr/bin/env python3
"""progress.py: progress card (docs/progress.svg) and numbers (docs/progress.json).

u5d rebuilds both releases byte for byte (tools/check.py), and tools/match.py is what says, per
function, whether it is EXACT; that needs the game files and the compiler. This script counts
from the sources alone, for one target (gog by default, --target 1988), one tile per function:

  - a C function of src/*.c (the `#if defined(RELEASE_1988)` blocks are in the 1988 build only),
    and
  - an assembly routine of src/asm/*.ASM (a label; local L_xxxx labels are not functions;
    ASM_EXCLUDED of tools/u5.py leaves A2322 and AINTRO2 out of the 1988 build),

when it carries an address: either in its name (MODULE_ADDR or MODULE_ADDR_Name, as before the
natural names) or through tools/names.tsv, which gives the module and address of every natural
name (kind "code"; MASM's labels are upper case, so the lookup ignores case). One tile per
module and address, ULTIMA (the resident code) first, then the overlays, each by address. Its
status is:

  matched  the function is in src/ with a descriptive name (a natural name, or MODULE_ADDR_Name)
  attempt  reserved (a function being worked on)
  open     the function is named by its address only (MODULE_ADDR)
  aside    reserved for code that is not Ultima V's own: none (the C runtime library is linked from
           the compiler's files and is not in the repository)

The percentage is matched / (matched + attempt + open). It counts sources; EXACT verdicts come from
tools/match.py and tools/check.py, which this script does not run. (docs/PLAN.md's rule "no
percentages" is about match verdicts; this is only a count of what is in src/.)

    python3 tools/progress.py [--target gog|1988]   # writes docs/progress.svg and docs/progress.json
    python3 tools/progress.py --check               # exit 1 if the committed files are stale
"""
import argparse, glob, json, os, re, sys
from html import escape

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')
NAME = 'u5d'
ASM_EXCLUDED = {'gog': (), '1988': ('A2322', 'AINTRO2')}      # as tools/u5.py
COLORS = dict(matched='#2a8c3c', attempt='#b8860b', open='#64748b', aside='#3b5b8a')
HEADLINE = 'functions matched'
LABELS = dict(matched='Matched', attempt='Attempt', open='Open', aside='Aside')
NAMED = re.compile(r'^([A-Za-z][A-Za-z0-9]*)_([0-9A-Fa-f]{3,4})(_\w+)?$')
C_DEF = re.compile(
    r'^(?!(?:extern|typedef|return|else|if|while|for|switch)\b)'
    r'(?:[A-Za-z_]\w*[ \t*]+)+\**([A-Za-z_]\w*)[ \t]*\(([^;{}]*)\)[ \t]*\n?[ \t]*\{', re.M)
ASM_LABEL = re.compile(r'^([A-Za-z_]\w*):', re.M)
NAMES_TSV = os.path.join(ROOT, 'tools', 'names.tsv')


def read(path):
    with open(path, encoding='latin1', newline='') as f:
        return f.read().replace('\r\n', '\n')


def preprocess(text, target):
    """Drop comments, and the #if defined(RELEASE_1988) blocks when the target is not 1988."""
    text = re.sub(r'/\*.*?\*/', lambda m: re.sub(r'[^\n]', ' ', m.group(0)), text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    if target != '1988':
        text = re.sub(r'^#if defined\(RELEASE_1988\)[^\n]*\n.*?^#endif[^\n]*\n',
                      lambda m: re.sub(r'[^\n]', ' ', m.group(0)), text, flags=re.S | re.M)
    return text


def functions(target):
    rows = []
    for path in sorted(glob.glob(os.path.join(SRC, '*.c'))):
        for m in C_DEF.finditer(preprocess(read(path), target)):
            rows.append((m.group(1), 'C', os.path.basename(path)))
    for path in sorted(glob.glob(os.path.join(SRC, 'asm', '*.ASM'))):
        stem = os.path.splitext(os.path.basename(path))[0].upper()
        if stem in ASM_EXCLUDED[target]:
            continue
        for m in ASM_LABEL.finditer(read(path)):
            if not m.group(1).startswith('L_'):
                rows.append((m.group(1), 'ASM', os.path.basename(path)))
    return rows


def name_table():
    """{NAME (upper case): (MODULE, address)} of the code rows of tools/names.tsv (none before it existed)."""
    table = {}
    if os.path.exists(NAMES_TSV):
        for line in read(NAMES_TSV).splitlines()[1:]:
            name, kind, module, addr = line.split('\t')
            if kind == 'code':
                table[name.upper()] = (module.upper(), int(addr, 16))
    return table


def classify(target):
    table = name_table()
    flat, seen = [], set()
    for name, kind, file in functions(target):
        m = NAMED.match(name)
        if m:
            where, status = (m.group(1).upper(), int(m.group(2), 16)), 'matched' if m.group(3) else 'open'
        elif name.upper() in table:
            where, status = table[name.upper()], 'matched'
        else:
            continue                          # no address: a helper, a data label, a local label
        if where in seen:                     # a C function and its assembly twin, or a repeat
            continue
        seen.add(where)
        flat.append(dict(module=where[0], addr=where[1], name=name, kind=kind, file=file, status=status))
    flat.sort(key=lambda r: (r['module'] != 'ULTIMA', r['module'], r['addr']))
    return flat


def summary(rows):
    c = {k: 0 for k in COLORS}
    for r in rows:
        c[r['status']] += 1
    total = c['matched'] + c['attempt'] + c['open']
    return dict(c, total=total, percent=round(100.0 * c['matched'] / total, 1) if total else 100.0)


def svg(flat, target):
    s = summary(flat)
    n = len(flat)
    W, PAD = 840, 20
    inner = W - 2 * PAD
    cols = 1
    for cols_try in range(20, 200):      # square tiles of at least 11 px
        if inner / cols_try < 11:
            break
        cols = cols_try
    cell = inner / cols
    top = 160
    H = int(top + -(-n // cols) * cell + 22)
    caption = ('Function map: 1 function per tile (C and assembly), ULTIMA first, then the overlays, '
               'by address; target %s' % target)
    out = []
    a = out.append
    a('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d" '
      'font-family="Roboto, Helvetica, Arial, sans-serif" role="img" aria-label="%s">'
      % (W, H, W, H, escape('Decompilation progress: %d of %d %s' % (s['matched'], s['total'], HEADLINE))))
    a('<rect width="%d" height="%d" rx="14" fill="#0f172a"/>' % (W, H))
    a('<text x="%d" y="38" font-size="21" fill="#ffffff">Decompilation progress</text>' % PAD)
    a('<text x="%d" y="38" font-size="15" fill="#ffffff" text-anchor="end">%s</text>' % (W - PAD, escape(NAME)))
    a('<text x="%d" y="62" font-size="15" fill="#e5e7eb">%s / %s %s (%.1f%%)</text>'
      % (PAD, format(s['matched'], ','), format(s['total'], ','), HEADLINE, s['percent']))
    a('<rect x="%d" y="74" width="%d" height="18" rx="5" fill="#334155"/>' % (PAD, inner))
    if s['total']:
        a('<rect x="%d" y="74" width="%.2f" height="18" rx="5" fill="%s"/>'
          % (PAD, inner * s['matched'] / s['total'], COLORS['matched']))
    for x, k in zip([PAD, 232, 430, 628], ['matched', 'attempt', 'open', 'aside']):
        a('<rect x="%d" y="104" width="10" height="10" fill="%s"/>' % (x, COLORS[k]))
        a('<text x="%d" y="113" font-size="12" fill="#e5e7eb">%s: %s</text>' % (x + 14, LABELS[k], format(s[k], ',')))
    a('<text x="%d" y="140" font-size="11" fill="#e5e7eb">%s</text>' % (PAD, escape(caption)))
    for i, r in enumerate(flat):
        x, y = PAD + (i % cols) * cell, top + (i // cols) * cell
        a('<rect x="%.2f" y="%.2f" width="%.2f" height="%.2f" fill="%s" stroke="#0f172a" stroke-width="1">'
          '<title>%s (%s, %s)</title></rect>' % (x, y, cell, cell, COLORS[r['status']], escape(r['name']), r['kind'], r['status']))
    a('</svg>')
    return '\n'.join(out) + '\n'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--target', default='gog', choices=['gog', '1988'])
    args = ap.parse_args()
    flat = classify(args.target)
    result = {'target': args.target, 'all': summary(flat)}
    result['by_module'] = {}
    for r in flat:
        result['by_module'].setdefault(r['module'], []).append(r)
    result['by_module'] = {m: summary(rs) for m, rs in result['by_module'].items()}
    other = '1988' if args.target == 'gog' else 'gog'
    result['other_target'] = {other: summary(classify(other))}
    js = json.dumps(result, indent=1) + '\n'
    paths = {os.path.join(ROOT, 'docs', 'progress.json'): js, os.path.join(ROOT, 'docs', 'progress.svg'): svg(flat, args.target)}
    if args.check:
        stale = [p for p, t in paths.items() if not os.path.exists(p) or read(p) != t]
        if stale:
            print('stale:', *stale)
            sys.exit(1)
        return
    for p, t in paths.items():
        with open(p, 'w', encoding='utf-8', newline='\n') as f:
            f.write(t)
    r = result['all']
    print('%s: matched %d  open %d  -> %.1f%% of %d  (C %d, asm %d)' % (
        args.target, r['matched'], r['open'], r['percent'], r['total'],
        sum(1 for x in flat if x['kind'] == 'C'), sum(1 for x in flat if x['kind'] == 'ASM')))
    print(json.dumps(result['other_target']))
    for x in flat:
        if x['status'] == 'open':
            print('open:', x['name'], x['file'])


if __name__ == '__main__':
    main()
