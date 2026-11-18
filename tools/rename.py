#!/usr/bin/env python3
"""Rename symbols in src/ without changing the compiled code: every rename is checked.

    python tools/rename.py OLD NEW                       a function, global, macro or type
    python tools/rename.py FILE.c:FUNCTION OLD NEW       a local or a parameter of FUNCTION
    python tools/rename.py --field OLD NEW               a struct field (.OLD, ->OLD, its declaration)
    python tools/rename.py --batch FILE                  many, one per line (the same three forms,
                                                         a field as "field OLD NEW"; '#' comments)
    python tools/rename.py --hash FILE.c:FUNCTION        the function's locals, buckets and slot order
    python tools/rename.py --suggest FILE.c:FUNCTION OLD BASE    the name a rename would use

    --match      recompile the modules whose text changed and compare them (tools/match.py
                 --compile; tools/asmbuild.py when an .ASM changed). When a function is no
                 longer EXACT every file is put back as it was, unless --keep is given.
    --bisect     with --batch and --match: when the batch fails, find the entries that break
                 the match (halving), leave those out, apply the rest and name them.
    --gate       run the gate at the end (tools/check.py); with a failure, put the files back.
    --exact      for a local: use NEW as given even when it changes the slot order (reported).
    --docs       also rename in docs/*.md and README*.md.
    --force      allow a name that drops the address prefix, is longer than 31, or hides a global.
    -n           dry run: say what would change.

FUNCTION may be the address prefix alone: talk.c:TALK_0054 finds TALK_0054_CanTalkAcross.

Names change the code here (docs/MATCHING.md, "Readable names"), so the tool knows three rules.

1. Stack slots. MSC orders a function's locals by a hash of their names (16 buckets, the
   bucket rising away from BP; within a bucket the last declared first; tools/slots.py has
   both compilers' hashes). The tool solves all the locals renamed in one function together:
   as many as possible get the name as given, the others a suffix `_x` (then `_xy`) that
   leaves the slot order of the function's locals as it was, as the existing local_4_ab
   names do (index -> index_c). It says which. Parameters have fixed places and take any name.
2. 31 significant characters. MSC cuts identifiers at 31 (warning C4011), and two names
   that agree that far are one symbol; public names reach the assembly and the linker in
   upper case. A new name longer than 31 is refused unless --force, and one that agrees
   with another identifier in its first 31 characters (ignoring case for globals) always.
3. The address stays. MODULE_addr and D_addr tie the source to the binary and the tools
   read them (tools/u5.py, name_address): ULTIMA_2c4c -> ULTIMA_2c4c_IsWalkableTile, not
   IsWalkableTile.

What cannot be predicted is pass 2's memory (the tree pool): it depends on the length of
the global names a module references, so a longer global name can change a function that
sits near a limit (TOWN_0f02, ULTIMA_1850). --match finds that out; the report gives the
change in name length per module so that it can be made up for.

A global rename is whole-word in src/ (C, headers, the native build's folders), in upper
case and cut at 31 in src/asm/*.ASM, and both ways in src/u5d.lnk and tools/*.py (the
tables in tools/u5.py). String literals are left alone; comments are renamed too. Line
endings are kept.
"""
import argparse, glob, itertools, os, re, subprocess, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import slots  # noqa: E402
import u5  # noqa: E402

SIGNIFICANT = 31
STUB_NAME = 26        # STUB_<26 characters>: the overlay call stubs of PLTAB.ASM
TOKEN = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'|[A-Za-z_]\w*', re.S)
ADDRESSED = re.compile(r'^([A-Z][A-Z0-9]*_[0-9A-Fa-f]{4})(?:_|$)')
KEYWORDS = set('''auto break case char const continue default do double else enum extern float for
goto if int long register return short signed sizeof static struct switch typedef union unsigned
void volatile while far near huge pascal cdecl fortran interrupt'''.split())
FUNC = re.compile(r'^(?:static\s+)?[\w \t\*]*?\b(\w+)[ \t]*\(([^;{)]*)\)\s*\n\{', re.M)
DECLTYPE = re.compile(r'^\s*(' + slots.TYPE + r')(?=[\w\*])')
PROTO_SKIP = re.compile(r'^\s*(return|else|goto|case|do)\b')


def read(path):
    return open(path, encoding='utf-8', errors='surrogateescape', newline='').read()


def write(path, text):
    open(path, 'w', encoding='utf-8', errors='surrogateescape', newline='').write(text)


def source_files(docs=False):
    """{path: kind}: 'c' (exact case), 'asm' (upper case, 31 characters), 'both', 'text'."""
    out = {}
    for root, _dirs, files in os.walk(u5.SRC):
        for f in files:
            ext = os.path.splitext(f)[1].lower()
            p = os.path.join(root, f)
            if ext in ('.c', '.h', '.inc', '.cpp'):
                out[p] = 'c'
            elif ext == '.asm':
                out[p] = 'asm'
            elif ext in ('.lnk', '.rsp'):
                out[p] = 'both'
    for p in glob.glob(os.path.join(u5.HERE, '*.py')):
        if os.path.basename(p) != 'rename.py':
            out[p] = 'both'
    if docs:
        for p in glob.glob(os.path.join(u5.ROOT, 'docs', '*.md')) + glob.glob(os.path.join(u5.ROOT, 'README*.md')):
            out[p] = 'text'
    return out


def sub_c(text, old, new=None, field=False):
    """Whole-word OLD -> NEW in C text (or a dict {old: new}, all at once): not in string
    literals, in comments too. A plain rename leaves .OLD and ->OLD alone (a field of the
    same name); a field rename takes those and the declaration, a name followed by ; [ , or
    : on a line of its own type."""
    n = 0
    mp = old if isinstance(old, dict) else {old: new}
    word = re.compile(r'(?<![\w])(%s)(?![\w])' % '|'.join(re.escape(o) for o in sorted(mp, key=len, reverse=True)))

    def member(s, i):
        j = i - 1
        while j >= 0 and s[j] in ' \t':
            j -= 1
        return j >= 0 and (s[j] == '.' or s[j - 1:j + 1] == '->')

    def tok(m):
        nonlocal n
        t = m.group(0)
        if t[0] in '"\'':
            return t
        if t.startswith('/'):
            t2, k = word.subn(lambda w: mp[w.group(1)], t)
            n += k
            return t2
        if t not in mp:
            return t
        is_member = member(m.string, m.start())
        if field:
            after = m.string[m.end():m.end() + 40].lstrip(' \t')
            line = m.string[m.string.rfind('\n', 0, m.start()) + 1:m.start()]
            declared = bool(after[:1] in (';', '[', ',', ':') and re.match(r'^\s*[\w\s\*]+$', line) and line.strip()
                            and not re.match(r'^\s*(return|goto|case)\b', line))
            if not (is_member or declared):
                return t
        elif is_member:
            return t
        n += 1
        return mp[t]
    return TOKEN.sub(tok, text), n


def sub_word(text, old, new, flags=0):
    return re.subn(r'(?<![\w])%s(?![\w])' % re.escape(old), new, text, flags=flags)


def identifiers(text, members=True):
    """The identifiers in C text; members=False leaves out the ones after . or -> (fields)."""
    out = set()
    for m in TOKEN.finditer(text):
        if m.group(0)[0] in '"\'/':
            continue
        if not members and re.search(r'(\.|->)\s*$', text[max(0, m.start() - 8):m.start()]):
            continue
        out.add(m.group(0))
    return out


_ALL = None
_RESERVED = None


def all_identifiers():
    """Every identifier in the C sources and headers of src/ (not the assembly)."""
    global _ALL
    if _ALL is None:
        _ALL = set()
        for p, kind in source_files().items():
            if kind == 'c':
                _ALL |= identifiers(read(p))
    return _ALL


def reserved_names():
    """Names a local must not take: every macro of src/ and every identifier of vars.h."""
    global _RESERVED
    if _RESERVED is None:
        _RESERVED = set()
        for p, kind in source_files().items():
            if kind == 'c':
                t = read(p)
                _RESERVED |= set(re.findall(r'^[ \t]*#[ \t]*define[ \t]+(\w+)', t, re.M))
                if os.path.basename(p) == 'vars.h':
                    _RESERVED |= identifiers(t)
    return _RESERVED


def check_name(old, new, others, force, ignore_case):
    """Problems with NEW as a name beside the identifiers in others (a set without OLD)."""
    probs = []
    if not re.match(r'^[A-Za-z_]\w*$', new) or new in KEYWORDS:
        return ['%s is not an identifier' % new]
    key = (lambda s: s[:SIGNIFICANT].upper()) if ignore_case else (lambda s: s[:SIGNIFICANT])
    if new in others:
        probs.append('%s is already used' % new)
    else:
        clash = sorted(o for o in others if key(o) == key(new) and o != old)
        if clash:
            probs.append('%s is the same symbol as %s (%d significant characters%s)' % (
                new, clash[0], SIGNIFICANT, ', upper case at link' if ignore_case else ''))
    if len(new) > SIGNIFICANT and not force:
        probs.append('%s has %d characters; MSC keeps %d (C4011): %s' % (new, len(new), SIGNIFICANT, new[:SIGNIFICANT]))
    m = ADDRESSED.match(old)
    if m and not force and not re.match(r'^%s(?:_|$)' % re.escape(m.group(1)), new):
        probs.append('%s must keep the address prefix %s' % (new, m.group(1)))
    return probs


# ---- functions and their locals

def find_functions(text, name):
    """[(full name, start, body start, end)] of the definitions of name (more than one when
    a function is written twice, under #if); name may be the address prefix alone."""
    hits = []
    for m in FUNC.finditer(text):
        n = m.group(1)
        if n == name or (n.startswith(name + '_') and ADDRESSED.match(name)):
            depth, i = 1, m.end()
            while depth and i < len(text):
                depth += {'{': 1, '}': -1}.get(text[i], 0)
                i += 1
            hits.append((n, m.start(), m.end(), i))
    hits = [h for h in hits if h[0] == name] or hits
    if not hits:
        raise SystemExit('no function %s' % name)
    return hits


def split_declarators(s):
    out, depth, cur = [], 0, ''
    for c in s:
        if c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
        if c == ',' and depth == 0:
            out.append(cur)
            cur = ''
        else:
            cur += c
    out.append(cur)
    return out


def locals_of(body):
    """The stack locals a function body declares, in order, at any block depth: names only.
    static and extern declarations have no slot."""
    names = []
    code = re.sub(r'//[^\n]*|/\*.*?\*/', lambda m: re.sub(r'[^\n]', ' ', m.group(0)), body, flags=re.S)
    for line in code.split('\n'):
        m = DECLTYPE.match(line)
        if not m or not line.rstrip().endswith(';'):
            continue
        if re.search(r'\b(static|extern|typedef)\b', m.group(1)) or line.split()[0] in ('return', 'goto', 'case', 'else'):
            continue
        rest = line[m.end():].rstrip()[:-1]
        if '(' in rest.split('=')[0]:
            continue                                  # a prototype or a call
        found = []
        for d in split_declarators(rest):
            n = re.match(r'^[\s\*]*([A-Za-z_]\w*)\s*(\[[^\]]*\]\s*)*$', d.split('=')[0])
            if not n:
                found = []
                break
            found.append(n.group(1))
        names += found
    return names


def params_of(header_params):
    out = []
    for p in split_declarators(header_params):
        m = re.search(r'([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*$', p.strip())
        if m and p.strip() != 'void':
            out.append(m.group(1))
    return out


def suffixes():
    return ['_' + c for c in slots.SUFFIX_CHARS] + ['_' + c + d for c in slots.SUFFIX_CHARS for d in slots.SUFFIX_CHARS]


def choose_local_names(names, want, taken, exact=False):
    """names: the function's locals in declaration order; want {old: base}. Returns
    {old: (name, note)}: as many base names as possible as given, the others with a suffix
    (_x, then _xy) chosen so that the compiler's slot order (slots.alloc_order) stays what
    it is. Only the final order counts, so a function's locals are solved together."""
    target = slots.alloc_order(names)
    idx = {n: i for i, n in enumerate(names)}
    olds = [n for n in target if n in want]
    if exact:
        out = {o: want[o] for o in olds}
        moved = slots.alloc_order([out.get(n, n) for n in names]) != [out.get(n, n) for n in target]
        return {o: (want[o], 'THE SLOT ORDER CHANGES' if moved else '') for o in olds}

    def fits(b, i, pb, pi):
        return b > pb or (b == pb and i < pi)        # within a bucket the last declared first

    def solve(suffixed):
        out, used = {}, set(taken)
        pb, pi = -1, 1 << 30
        for t in target:
            if t in suffixed:
                got = None
                for b in range(max(pb, 0), slots.BUCKETS):
                    if not fits(b, idx[t], pb, pi):
                        continue
                    for sfx in suffixes():
                        x = want[t] + sfx
                        if slots.bucket(x) == b and x not in used and len(x) <= SIGNIFICANT:
                            got = (x, sfx, b)
                            break
                    if got:
                        break
                if not got:
                    return None
                x, sfx, b = got
                out[t] = (x, 'suffix %s keeps the slot order (bucket %d; %s is bucket %d)' % (
                    sfx, b, want[t], slots.bucket(want[t])))
            else:
                x = want.get(t, t)
                b = slots.bucket(x)
                if not fits(b, idx[t], pb, pi) or (t in want and x in used):
                    return None
                if t in want:
                    out[t] = (x, '')
            used.add(x)
            pb, pi = b, idx[t]
        final = {o: v[0] for o, v in out.items()}
        if slots.alloc_order([final.get(n, n) for n in names]) != [final.get(n, n) for n in target]:
            return None
        return out

    if len(olds) <= 12:
        for k in range(len(olds) + 1):               # the fewest suffixes first
            for sub in itertools.combinations(olds, k):
                r = solve(set(sub))
                if r is not None:
                    return r
    else:                                            # many locals: plain where it fits, in turn
        sub = set()
        for i, o in enumerate(olds):
            if not solve(sub | set(olds[i + 1:])):
                sub.add(o)
        r = solve(sub)
        if r is not None:
            return r
    raise SystemExit('no names found that keep the slot order: %s' % ', '.join(want.values()))


# ---- the renames

class Change:
    def __init__(self):
        self.files = {}          # path -> new text
        self.before = {}         # path -> old text
        self.log = []
        self.stats = {'global': 0, 'field': 0, 'param': 0, 'local plain': 0, 'local suffixed': 0,
                      'local moved': 0}
        self.length = {}         # module file -> change in the length of the global names it uses

    def text(self, path):
        if path not in self.files:
            self.before[path] = self.files[path] = read(path)
        return self.files[path]

    def put(self, path, text):
        self.text(path)
        self.files[path] = text

    def changed(self):
        return sorted(p for p in self.files if self.files[p] != self.before[p])

    def write(self):
        for p in self.changed():
            write(p, self.files[p])

    def restore(self):
        for p in self.changed():
            write(p, self.before[p])


def rename_global(ch, old, new, a):
    global _ALL
    others = all_identifiers() - {old}
    probs = check_name(old, new, others, a.force, ignore_case=True)
    if probs:
        return probs
    total = 0
    up_old, up_new = old.upper()[:SIGNIFICANT], new.upper()[:SIGNIFICANT]
    for p, kind in source_files(a.docs).items():
        t = ch.text(p)
        n = 0
        if kind == 'c':
            t, n = sub_c(t, old, new)
            if n and os.path.basename(p) in u5.MODULES and os.path.dirname(p) == u5.SRC:
                ch.length[os.path.basename(p)] = ch.length.get(os.path.basename(p), 0) + len(new) - len(old)
        elif kind == 'asm':
            t, n = sub_word(t, up_old, up_new, re.I)
            # an overlay function's call stub is STUB_ + 26 characters of its name
            # (src/asm/PLTAB.ASM; tools/u5link.py looks it up by that name)
            t, k = sub_word(t, 'STUB_' + old.upper()[:STUB_NAME], 'STUB_' + new.upper()[:STUB_NAME], re.I)
            n += k
        else:
            t, n = sub_word(t, old, new)
            for o, w in {(old.upper(), new.upper()), (up_old, up_new)}:
                if o != old:
                    t, k = sub_word(t, o, w)
                    n += k
        if n:
            ch.put(p, t)
            total += n
    if not total:
        return ['%s does not occur' % old]
    _ALL = (others | {new})
    ch.stats['global'] += 1
    ch.log.append('%-34s -> %-34s %d places' % (old, new, total))
    return []


def rename_field(ch, old, new, a):
    probs = check_name(old, new, set(), a.force, ignore_case=False)
    if probs:
        return probs
    total = 0
    for p, kind in source_files().items():
        if kind != 'c':
            continue
        t, n = sub_c(ch.text(p), old, new, field=True)
        if n:
            ch.put(p, t)
            total += n
    if not total:
        return ['no field %s' % old]
    ch.stats['field'] += 1
    ch.log.append('field %-28s -> %-34s %d places' % (old, new, total))
    return []


def module_path(cfile):
    p = cfile if os.path.exists(cfile) else os.path.join(u5.SRC, cfile)
    if not os.path.exists(p):
        raise SystemExit('no file %s' % cfile)
    return p


def rename_locals(ch, where, pairs, a):
    """pairs [(old, base)]: locals and parameters of one function, solved together."""
    cfile, func = where.split(':', 1)
    path = module_path(cfile)
    slots.RULE = u5.COMPILER_BY_FILE.get(os.path.basename(path), 'msc51')
    count = len(find_functions(ch.text(path), func))
    probs, done = [], set()
    for k in range(count):
        text = ch.text(path)
        fname, start, body, end = find_functions(text, func)[k]
        span = text[start:end]
        params = params_of(FUNC.match(span).group(2))
        names = locals_of(text[body:end])
        want_p = {o: n for o, n in pairs if o in params}
        want_l = {o: n for o, n in pairs if o in names and o not in params}
        if not want_p and not want_l:
            continue
        taken = identifiers(span, members=False) - set(want_p) - set(want_l)
        chosen = {o: (n, '') for o, n in want_p.items()}
        chosen.update(choose_local_names(names, want_l, taken | set(want_p.values()), a.exact))
        finals = [v[0] for v in chosen.values()]
        p = []
        for o, (name, note) in chosen.items():
            others = taken | set(x for x in finals if x != name)
            if finals.count(name) > 1:
                p.append('%s is given to two names' % name)
            p += check_name(o, name, others, a.force, ignore_case=False)
            if name in reserved_names() and not a.force:
                # a macro or a global of that name: the local would hide it
                p.append('%s is also a macro or a global (vars.h); --force to hide it' % name)
        if p:
            probs += p
            continue
        new_span, n = sub_c(span, {o: v[0] for o, v in chosen.items()})
        ch.put(path, text[:start] + new_span + text[end:])
        if want_p:
            # the prototypes of the function, here and in the headers
            proto = re.compile(r'^([ \t]*[A-Za-z_][\w \t\*]*?[ \t\*]%s[ \t]*\()([^;{)]*)(\)[ \t]*;)' % re.escape(fname), re.M)
            mp = {o: chosen[o][0] for o in want_p}

            def fix(m):
                if PROTO_SKIP.match(m.group(1)):
                    return m.group(0)
                return m.group(1) + sub_c(m.group(2), mp)[0] + m.group(3)
            for q, qk in source_files().items():
                if qk != 'c':
                    continue
                t = ch.text(q)
                t2 = proto.sub(fix, t)
                if t2 != t:
                    ch.put(q, t2)
        for o, n in pairs:
            if o not in chosen or o in done:
                continue
            name, note = chosen[o]
            kind = 'param' if o in want_p else ('local moved' if note.startswith('THE SLOT') else
                                                'local suffixed' if note else 'local plain')
            ch.stats[kind] += 1
            done.add(o)
            ch.log.append('%-34s %-14s -> %-20s %s' % (fname, o, name, note))
    for o, n in pairs:
        if o not in done and not probs:
            probs.append('%s has no local or parameter %s' % (func, o))
    return probs


def parse_batch(path):
    out = []
    for i, line in enumerate(open(path, encoding='utf-8').read().split('\n'), 1):
        w = line.split('#')[0].split()
        if not w:
            continue
        if len(w) == 2:
            out.append((w[0], w[1]))
        elif len(w) == 3:
            out.append((w[0], w[1], w[2]))
        else:
            raise SystemExit('%s line %d: OLD NEW, FILE.c:FUNCTION OLD NEW or field OLD NEW' % (path, i))
    return out


# ---- checking

def run(tool, *args):
    r = subprocess.run([sys.executable, os.path.join(u5.HERE, tool)] + list(args), capture_output=True, text=True)
    return r.returncode, r.stdout + r.stderr


def affected(ch):
    mods = [os.path.basename(p) for p in ch.changed()
            if os.path.dirname(p) == u5.SRC and os.path.basename(p) in u5.MODULES and u5.MODULES[os.path.basename(p)][0] != 'data']
    asm = any(p.lower().endswith('.asm') for p in ch.changed())
    return sorted(mods), asm


def match(ch):
    """Recompile and compare the changed modules: (ok, report)."""
    mods, asm = affected(ch)
    report = []
    ok = True
    if mods:
        rc, out = run('match.py', '--compile', '-v', *mods)
        lines = out.strip().split('\n')
        bad = [l for l in lines if re.search(r'\b(SLOTS|FIXUP|SIZE|CODE)\b', l) and not l.startswith('total')
               and not re.search(r'\bEXACT \d', l)]
        report += [l for l in lines if l.startswith('total')] or lines[-1:]
        if rc:
            ok = False
            report += bad[:25] if bad else lines[-25:]
    if asm:
        rc, out = run('asmbuild.py')
        report += out.strip().split('\n')[-1:]
        if rc:
            ok = False
            report += out.strip().split('\n')[-15:]
    return ok, report


def attempt(entries, a):
    """Apply entries to the files as they are now; with --match, compare. Returns
    (change, problems, ok, report). The files are written only when there is no problem."""
    global _ALL, _RESERVED
    _ALL = _RESERVED = None
    ch = Change()
    probs = []
    groups = {}                                  # a function's locals are solved together
    for e in entries:
        if ':' in e[0]:
            if len(e) != 3:
                raise SystemExit('%s: FILE.c:FUNCTION OLD NEW' % ' '.join(e))
            groups.setdefault(e[0], []).append((e[1], e[2]))
    for e in entries:
        if ':' in e[0]:
            if e[0] in groups:
                probs += ['%s: %s' % (e[0], x) for x in rename_locals(ch, e[0], groups.pop(e[0]), a)]
        elif e[0] == 'field' and len(e) == 3:
            probs += ['%s: %s' % (' '.join(e), x) for x in rename_field(ch, e[1], e[2], a)]
        elif len(e) == 2:
            probs += ['%s: %s' % (' '.join(e), x) for x in rename_global(ch, e[0], e[1], a)]
        else:
            raise SystemExit('%s: OLD NEW, FILE.c:FUNCTION OLD NEW or field OLD NEW' % ' '.join(e))
    if probs or a.dry_run:
        return ch, probs, not probs, []
    ch.write()
    ok, report = (True, [])
    if a.match:
        ok, report = match(ch)
        if not ok and not a.keep:
            ch.restore()
    return ch, probs, ok, report


def bisect(entries, a, depth=0):
    """The entries that can be applied together; returns (applied, failed). Each half is
    tried on the files as they are (nothing stays applied between tries)."""
    ch, probs, ok, report = attempt(entries, a)
    if ok and not probs:
        ch.restore()
        print('%s%d entries match' % ('  ' * depth, len(entries)), flush=True)
        return list(entries), []
    if len(entries) == 1:
        print('%sFAILS: %s' % ('  ' * depth, ' '.join(entries[0])), flush=True)
        for l in (probs or report)[:6]:
            print('%s    %s' % ('  ' * depth, l))
        return [], list(entries)
    half = len(entries) // 2
    a1, f1 = bisect(entries[:half], a, depth + 1)
    a2, f2 = bisect(entries[half:], a, depth + 1)
    return a1 + a2, f1 + f2


def show_hash(where):
    cfile, func = where.split(':', 1)
    path = module_path(cfile)
    slots.RULE = u5.COMPILER_BY_FILE.get(os.path.basename(path), 'msc51')
    text = read(path)
    fname, start, body, end = find_functions(text, func)[0]
    names = locals_of(text[body:end])
    print('%s (%s, %s): parameters %s' % (fname, os.path.basename(path), slots.RULE,
                                          ', '.join(params_of(FUNC.match(text[start:end]).group(2))) or 'none'))
    for n in slots.alloc_order(names):
        print('  bucket %2d  %s' % (slots.bucket(n), n))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('args', nargs='*')
    ap.add_argument('--field', action='store_true')
    ap.add_argument('--batch', metavar='FILE')
    ap.add_argument('--hash', metavar='FILE.c:FUNCTION')
    ap.add_argument('--suggest', metavar='FILE.c:FUNCTION')
    ap.add_argument('--match', action='store_true')
    ap.add_argument('--bisect', action='store_true')
    ap.add_argument('--gate', action='store_true')
    ap.add_argument('--keep', action='store_true', help='keep the renames when the match or the gate fails')
    ap.add_argument('--exact', action='store_true')
    ap.add_argument('--docs', action='store_true')
    ap.add_argument('--force', action='store_true')
    ap.add_argument('-n', '--dry-run', action='store_true')
    a = ap.parse_args()

    if a.hash:
        show_hash(a.hash)
        return
    if a.suggest:
        if len(a.args) != 2:
            ap.error('--suggest FILE.c:FUNCTION OLD BASE')
        cfile, func = a.suggest.split(':', 1)
        path = module_path(cfile)
        slots.RULE = u5.COMPILER_BY_FILE.get(os.path.basename(path), 'msc51')
        text = read(path)
        fname, start, body, end = find_functions(text, func)[0]
        r = choose_local_names(locals_of(text[body:end]), {a.args[0]: a.args[1]},
                               identifiers(text[start:end], members=False) - {a.args[0]})
        print('%s %s' % r[a.args[0]])
        return

    if a.batch:
        entries = parse_batch(a.batch)
    elif a.field and len(a.args) == 2:
        entries = [('field', a.args[0], a.args[1])]
    elif len(a.args) in (2, 3):
        entries = [tuple(a.args)]
    else:
        ap.error('OLD NEW, or FILE.c:FUNCTION OLD NEW, or --batch FILE')

    ch, probs, ok, report = attempt(entries, a)
    failed = []
    if not ok and not probs and a.bisect and a.match and not a.dry_run:
        if a.keep:
            ch.restore()
        print('the batch does not match as a whole; halving ...', flush=True)
        for l in report[:8]:
            print('    ' + l)
        applied, failed = bisect(entries, a)
        ch, probs, ok, report = attempt(applied, a)
    for l in ch.log:
        print(l)
    for p in probs:
        print('problem: ' + p)
    if probs:
        sys.exit('nothing changed')
    s = ch.stats
    print('%d global, %d field, %d parameter; locals: %d as given, %d with a suffix, %d changing the slot order'
          % (s['global'], s['field'], s['param'], s['local plain'], s['local suffixed'], s['local moved']))
    if any(ch.length.values()):
        print('length of the global names used, per module: ' + ', '.join(
            '%s %+d' % kv for kv in sorted(ch.length.items()) if kv[1]))
    if a.dry_run:
        print('dry run: %d files would change' % len(ch.changed()))
        return
    for l in report:
        print(l)
    if a.match and not ok:
        print('MATCH FAILS: %s' % ('kept (--keep)' if a.keep else 'every file put back'))
        sys.exit(1)
    if failed:
        print('%d entries left out (they break the match):' % len(failed))
        for e in failed:
            print('  ' + ' '.join(e))
    if a.gate:
        print('the gate (tools/check.py) ...', flush=True)
        rc, out = run('check.py')
        print('\n'.join(out.strip().split('\n')[-2:]))
        if rc:
            if not a.keep:
                ch.restore()
            print('GATE FAILS: %s' % ('kept (--keep)' if a.keep else 'every file put back'))
            sys.exit(1)
    print('%d files changed%s' % (len(ch.changed()), '' if a.match or a.gate else
                                  '; not compiled: run tools/match.py --compile and tools/check.py'))
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
