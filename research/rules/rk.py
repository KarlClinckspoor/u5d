#!/usr/bin/env python3
"""Rules kit: compile small test programs with MSC 5.0/5.1 in DOSBox-X, many at once.

Used by the experiments in research/rules/ (findings: research/RULES.md).

    capture(sources, comp)          -> {name: {'EX': bytes, 'SY':..., 'IN':..., 'ST':..., 'OBJ': bytes}}
        Runs CL on each source (one DOSBox-X session) with a spy in place of C1/C2/C3 that
        keeps pass 1's intermediate files; the normal object is kept too.
    passes(jobs, comp, name)        -> [obj bytes or None]
        Runs C2 and C3 on each job (a dict of pass-1 files, plus optional 'PARAS': paragraphs
        left free for C2), one session.
    funcs(obj)                      -> {NAME: bytes}   the code of each public function
    sig(code)                       -> short signature of a function's bytes
    insert_lines(ex, line, n)       -> EX with n line records after the function whose
                                       opening-brace line is `line` (as research/c2lines.py)

The C2/C3 command flags are the ones CL passes for u5d's switches (/Ox /Or /Gs /Gc /Zp),
recorded by the spy (FLAGS.TXT) and checked against research/c2lines.py.
"""
import hashlib, os, shutil, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import u5  # noqa: E402
import omf  # noqa: E402
import dosbuild  # noqa: E402

WORK = os.path.join(u5.BUILD, 'rules')
BS = chr(92)
CFLAGS = u5.CFLAGS

SPY_C = r'''#include <stdio.h>
#include <process.h>
#include <string.h>
#include <stdlib.h>
/* In place of C1/C2/C3.EXE: logs MSC_CMD_FLAGS, runs the real pass from C:\RB, copies C:\TMP to C:\KEEPn */
int main(int argc, char **argv)
{
    char real[80], cmd[80], *p, *q, *e;
    int r;
    FILE *f;
    p = strrchr(argv[0], '/');
    q = strrchr(argv[0], 92);
    if (q > p) p = q;
    p = p ? p + 1 : argv[0];
    e = getenv("MSC_CMD_FLAGS");
    f = fopen("C:/FLAGS.TXT", "a");
    if (f) { fprintf(f, "%s: %s\n", p, e ? e : ""); fclose(f); }
    strcpy(real, "C:/RB/");
    strcat(real, p);
    r = spawnv(P_WAIT, real, argv);
    strcpy(cmd, "copy C:\\TMP\\*.* C:\\KEEPx");
    cmd[23] = p[1];
    spawnl(P_WAIT, "Z:\\COMMAND.COM", "COMMAND", "/C", cmd, NULL);
    return r;
}
'''
HOG_C = r'''#include <process.h>
#include <stdlib.h>
#include <dos.h>
int main(int argc, char **argv)
{
    unsigned seg, max, want;
    want = (unsigned)atol(argv[1]);
    _dos_allocmem(0xffff, &max);
    if (max > want + 1)
        _dos_allocmem(max - want - 1, &seg);
    return spawnv(P_WAIT, argv[2], argv + 2);
}
'''

C2FLAGS = '-Oc -Ol -Oa -Ot -G0 -A snd -W 1'
C3FLAGS = '-T 0 -NM %s -FPi -Ot -Oa -Or -c C:\\%s.OBJ -A snd -CC 0sO'


def dosbox(cdir, comp, bat, timeout=3600):
    open(os.path.join(cdir, 'GO.BAT'), 'w', newline='').write('\r\n'.join(bat) + '\r\n')
    conf = os.path.join(cdir, 'dosbox.conf')
    open(conf, 'w').write('[sdl]\noutput=surface\n[dosbox]\nmemsize=16\n[cpu]\ncycles=max\n[log]\nlogfile=\n'
                          '[autoexec]\nmount c "%s"\nmount d "%s"\nc:\ncall %sGO.BAT\nexit\n'
                          % (cdir, dosbuild.compiler_dir(comp), BS))
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
    subprocess.run([u5.find_dosbox(None), '-conf', conf, '-nogui', '-exit', '-silent'], env=env,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=timeout)


def helper(name, source):
    exe = os.path.join(WORK, name + '.EXE')
    if not os.path.exists(exe):
        d = os.path.join(WORK, 'helper_' + name)
        shutil.rmtree(d, ignore_errors=True)
        os.makedirs(d)
        open(os.path.join(d, name + '.C'), 'w').write(source)
        dosbox(d, 'msc51', ['set PATH=D:%sBIN' % BS, 'set INCLUDE=D:%sINCLUDE' % BS, 'set LIB=D:%sLIB' % BS,
                            'set TMP=C:%s' % BS, 'CL /AS %s.C /link /NOD SLIBCR LIBH > OUT.TXT' % name])
        if not os.path.exists(os.path.join(d, name + '.EXE')):
            sys.exit('%s did not build: see %s' % (name, os.path.join(d, 'OUT.TXT')))
        shutil.copy(os.path.join(d, name + '.EXE'), exe)
    return exe


def newdir(tag):
    os.makedirs(WORK, exist_ok=True)
    return tempfile.mkdtemp(prefix=tag + '_', dir=WORK)


def capture(sources, comp='msc51', flags=CFLAGS, include_src=False, keep=False):
    """sources: {NAME: C text} (NAME up to 8 chars). Returns {NAME: files}, files['ERR'] = CL output.
    include_src: copy u5d's src/ so that test files can #include its headers."""
    d = newdir('cap')
    tools_dir = dosbuild.compiler_dir(comp)
    S = os.path.join(d, 'S')
    if include_src:
        shutil.copytree(u5.SRC, S, ignore=shutil.ignore_patterns('*.OBJ', '*.obj', '*.COD', '*.cod'))
        if comp == 'msc50':
            for dp, _, fs in os.walk(S):
                for fn in fs:
                    if fn.lower().endswith(('.c', '.h')):
                        p = os.path.join(dp, fn)
                        t = open(p, encoding='latin-1', newline='').read()
                        open(p, 'w', encoding='latin-1', newline='').write(dosbuild.c_comments(t))
    else:
        os.makedirs(S)
    for k in ('BIN', 'RB', 'KEEP1', 'KEEP2', 'KEEP3', 'TMP', 'CAP'):
        os.makedirs(os.path.join(d, k), exist_ok=True)
    spy = helper('SPYR', SPY_C)
    for fn in os.listdir(os.path.join(tools_dir, 'BIN')):
        shutil.copy(os.path.join(tools_dir, 'BIN', fn), os.path.join(d, 'RB', fn))
        shutil.copy(spy if fn in ('C1.EXE', 'C2.EXE', 'C3.EXE') else os.path.join(tools_dir, 'BIN', fn),
                    os.path.join(d, 'BIN', fn))
    bat = ['@echo off', 'set PATH=C:%sBIN' % BS, 'set INCLUDE=D:%sINCLUDE' % BS, 'set LIB=D:%sLIB' % BS,
           'set TMP=C:%sTMP' % BS, 'cd %sS' % BS]
    for name, text in sources.items():
        if comp == 'msc50':
            text = dosbuild.c_comments(text)
        open(os.path.join(S, name + '.C'), 'w', newline='').write(text.replace('\r\n', '\n').replace('\n', '\r\n'))
        bat += ['md C:%sCAP%s%s' % (BS, BS, name),
                'CL %s /c /Fc %s.C > C:%sCAP%s%s%sOUT.TXT' % (flags, name, BS, BS, name, BS),
                'copy C:%sKEEP1%s*.* C:%sCAP%s%s > NUL' % (BS, BS, BS, BS, name),
                'for %%%%f in (C:%sKEEP1%s*.*) do del %%%%f' % (BS, BS),
                'for %%%%f in (C:%sKEEP2%s*.*) do del %%%%f' % (BS, BS),
                'for %%%%f in (C:%sKEEP3%s*.*) do del %%%%f' % (BS, BS)]
    dosbox(d, comp, bat)
    out = {}
    for name in sources:
        c = os.path.join(d, 'CAP', name)
        files = {}
        if os.path.isdir(c):
            for fn in os.listdir(c):
                if fn.upper() == 'OUT.TXT':
                    files['ERR'] = open(os.path.join(c, fn), encoding='latin-1').read()
                elif len(fn) >= 2:
                    files[fn[-2:].upper()] = open(os.path.join(c, fn), 'rb').read()
        for ext in ('OBJ', 'COD'):
            p = os.path.join(S, name + '.' + ext)
            if os.path.exists(p):
                files[ext] = open(p, 'rb').read()
        out[name] = files
    fl = os.path.join(d, 'FLAGS.TXT')
    out['_flags'] = open(fl).read() if os.path.exists(fl) else ''
    if not keep:
        shutil.rmtree(d, ignore_errors=True)
    else:
        out['_dir'] = d
    return out


def passes(jobs, comp='msc51', name='T'):
    """jobs: list of dicts with 'EX','SY','IN','ST' (and optional 'PARAS'). Returns [obj or None]."""
    d = newdir('run')
    os.makedirs(os.path.join(d, 'TMP'))
    shutil.copy(helper('HOGR', HOG_C), os.path.join(d, 'HOG.EXE'))
    tmp = 'C:%sTMP%s' % (BS, BS)
    bat = ['@echo off', 'set PATH=D:%sBIN' % BS]
    for i, files in enumerate(jobs):
        b = 'J%04d' % i
        for k in ('EX', 'SY', 'IN', 'ST'):
            if k in files:
                open(os.path.join(d, 'TMP', b + k), 'wb').write(files[k])
        c2 = 'D:%sBIN%sC2.EXE' % (BS, BS)
        bat.append('set MSC_CMD_FLAGS=-il %s%s -ef D:%sBIN%sc23.err %s' % (tmp, b, BS, BS, C2FLAGS))
        paras = files.get('PARAS', 0)
        bat.append(('C:%sHOG.EXE %d %s > NUL' % (BS, paras, c2)) if paras else c2 + ' > NUL')
        bat.append('set MSC_CMD_FLAGS=-il %s%s -ef D:%sBIN%sc23.err %s' % (tmp, b, BS, BS, C3FLAGS % (name, b)))
        bat.append('D:%sBIN%sC3.EXE > NUL' % (BS, BS))
        bat.append('for %%%%f in (%s%s*.*) do del %%%%f' % (tmp, b))
    dosbox(d, comp, bat)
    res = []
    for i in range(len(jobs)):
        p = os.path.join(d, 'J%04d.OBJ' % i)
        res.append(open(p, 'rb').read() if os.path.exists(p) and os.path.getsize(p) else None)
    shutil.rmtree(d, ignore_errors=True)
    return res


def funcs(obj):
    """{NAME: code bytes} for the public functions of _TEXT (each to the next public or the end)."""
    if obj is None:
        return {}
    o = omf.Object('x', obj)
    t = o.seg('_TEXT')
    if t is None:
        return {}
    pubs = sorted(t.publics.items(), key=lambda kv: kv[1])
    out = {}
    for i, (n, a) in enumerate(pubs):
        b = pubs[i + 1][1] if i + 1 < len(pubs) else t.length
        out[n] = bytes(t.data[a:b])
    return out


def sig(code):
    if code is None:
        return 'NONE'
    return '%d:%s' % (len(code), hashlib.sha1(code).hexdigest()[:6])


def line_record(line):
    return bytes([0x4f, 1, 0x80, line & 255, line >> 8]) if line >= 128 else bytes([0x4f, 1, line])


def insert_lines(ex, line, n):
    pat = line_record(line)
    i = ex.find(pat)
    if i < 0:
        raise ValueError('no line record for line %d' % line)
    i = ex.index(bytes([0x4f, 1]), i + len(pat))
    return ex[:i] + bytes([0x4f, 1, 1]) * n + ex[i:]


def dis(code, ip=0):
    import iced_x86
    fmt = iced_x86.Formatter(iced_x86.FormatterSyntax.MASM)
    return ['%04x %-14s %s' % (ins.ip, code[ins.ip:ins.ip + ins.len].hex(), fmt.format(ins))
            for ins in iced_x86.Decoder(16, bytes(code), ip=ip)]


def brace_line(text, func):
    """1-based line number of the opening brace of function func in text."""
    lines = text.split('\n')
    for i, l in enumerate(lines):
        if func + '(' in l and not l.rstrip().endswith(';'):
            for j in range(i, len(lines)):
                if lines[j].strip().startswith('{'):
                    return j + 1
    raise ValueError(func)
