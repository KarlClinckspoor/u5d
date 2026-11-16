"""Shared facts and paths for the matching tools.

Every number here was measured from the GOG release of Ultima V (see docs/MATCHING.md,
"The layout of the binary"). tools/layout.py --check re-derives them from the game files.
"""
import os
import re
import shutil
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SRC = os.path.join(ROOT, 'src')
BUILD = os.path.join(ROOT, 'build')
MSC51 = os.path.join(BUILD, 'msc51')

# Compiler switches, from src/build.bat. ultima.c is built without loop optimisation.
# vars.c is too big for the normal first pass (C1042 "out of heap space"); /B1 selects
# C1L.EXE, the large-model first pass, which emits the same code.
CFLAGS = '/Ox /Or /Gs /Gc /Zp'
CFLAGS_BY_FILE = {'ultima.c': '/Oaitr /Or /Gs /Gc /Zp',
                  'vars.c': '/B1 D:\\BIN\\C1L.EXE /Ox /Or /Gs /Gc /Zp'}

# ULTIMA.EXE: the resident code is CS:0000 upwards; the overlays are loaded into the same
# code segment at one of four area bases. A name such as TOWN_0052 is the offset in the
# .OVL file, so its CS address is base + 0x52. (INTRO.OVL starts with a 16-byte header, so
# its file offsets are 0x10 more than its code offsets and its base is 0x10 lower.)
OVERLAY_BASE = {
    'INTRO': 0x81c0, 'TOWN': 0x81d0, 'MAINOUT': 0x81d0, 'DUNGEON': 0x81d0,
    'COMBAT': 0xa290, 'SHOPPES': 0xa290, 'NPC': 0xa290, 'LOOKOBJ': 0xa290,
    'DNGLOOK': 0xa290, 'BLCKTHRN': 0xa290, 'ENDGAME': 0xa290, 'OUTSUBS': 0xa290,
    'SJOG': 0xbf80, 'CAST': 0xbf80, 'CMDS': 0xbf80, 'TALK': 0xbf80,
    'COMSUBS': 0xe1e0, 'ZSTATS': 0xe1e0, 'CAST2': 0xe1e0, 'FONT': 0xe1e0,
    'SHOPPES2': 0xe1e0, 'SHOPPES3': 0xe1e0,
    'FLAMES': 0xa290,         # a 32-byte delay routine, loaded into area 2
}
OVERLAYS = list(OVERLAY_BASE)

# The C files of the DOS build, and what each one is compared with.
#   ('ovl', NAME)   the whole _TEXT segment is NAME.OVL
#   ('root', None)  functions are compared at their CS addresses in ULTIMA.EXE
#   ('data', None)  initialised data, compared with DATA.OVL (DGROUP)
MODULES = {o.lower() + '.c': ('ovl', o) for o in OVERLAYS}
MODULES['flames.c'] = ('ovl', 'FLAMES')
MODULES.update({'%d000.c' % i: ('root', None) for i in range(8)})
MODULES['ultima.c'] = ('root', None)
# the small C modules between the resident assembly modules (named by their CS address)
MODULES.update({n + '.c': ('root', None) for n in ('r0d72', 'r1850', 'r1b38', 'r1dda', 'r1fa0', 'r216c')})
MODULES['vars.c'] = ('data', None)

# The original was built with two compilers. Modules last compiled with MSC 5.0 put their
# string literals in a CONST segment, each on an even address, and encode small logical
# immediates in the long form (and si,7 as 81 e6 07 00); MSC 5.1 modules keep literals in
# _DATA, packed. Each module is compiled with the one the original used (docs/MATCHING.md).
COMPILER_BY_FILE = {f: 'msc50' for f in (
    '3000.c', '4000.c', '6000.c', 'blckthrn.c', 'cast2.c', 'combat.c', 'comsubs.c', 'dnglook.c',
    'endgame.c', 'font.c', 'lookobj.c', 'npc.c', 'shoppes.c', 'shoppes2.c', 'sjog.c', 'talk.c',
    'zstats.c')}
# overlays built from two objects: NAME.c (MSC 5.1) and NAMEb.c (MSC 5.0)
for _o in ('DUNGEON', 'MAINOUT', 'INTRO', 'OUTSUBS', 'CMDS'):
    if os.path.exists(os.path.join(SRC, _o.lower() + 'b.c')):
        MODULES[_o.lower() + 'b.c'] = ('ovl', _o)
        COMPILER_BY_FILE[_o.lower() + 'b.c'] = 'msc50'

EXE_HEADER = 0x800  # 0x80 paragraphs
DATA_OVL_HEADER = 0x10  # DATA.OVL is a 16-byte header, then DGROUP from DS:0000
# DGROUP's paragraph, relative to the load segment: right after overlay area 4's largest
# member (0xe1e0 + 0x1460 = 0xf640). Relocated words with this value are 'DGROUP'.
DGROUP_SEG = 0x0f64

# Overlay numbers, as the stubs pass them to the overlay manager: the position (from 1) of
# the file name in the table in ULTIMA.EXE.
OVERLAY_NUMBER = {n: i + 1 for i, n in enumerate(
    ['TOWN', 'MAINOUT', 'DUNGEON', 'INTRO', 'FLAMES', 'NPC', 'COMBAT', 'BLCKTHRN', 'LOOKOBJ',
     'DNGLOOK', 'OUTSUBS', 'SHOPPES', 'ENDGAME', 'SJOG', 'CMDS', 'CAST', 'TALK', 'CAST2',
     'ZSTATS', 'COMSUBS', 'SHOPPES2', 'SHOPPES3', 'FONT', 'DATA'])}


def ds_image(gamedata):
    return read_file(gamedata, 'DATA.OVL')[DATA_OVL_HEADER:]


def overlay_stubs(img):
    """The overlay call stubs in the resident code: {stub CS address: (overlay number,
    target CS address)}. Each is 12 bytes: call far 072e:02ec (the overlay manager);
    dw overlay number; jmp far 0000:target."""
    out = {}
    for m in re.finditer(rb'\x9a\xec\x02\x2e\x07(..)\xea(..)\x00\x00', img, re.S):
        out[m.start()] = (struct.unpack('<H', m.group(1))[0], struct.unpack('<H', m.group(2))[0])
    return out


def find_gamedata(given=None):
    for p in (given, os.environ.get('U5_GAMEDATA'), os.path.join(ROOT, 'gamedata'),
              os.path.join(ROOT, '..', 'gamedata')):
        if p and os.path.exists(os.path.join(p, 'ULTIMA.EXE')):
            return os.path.abspath(p)
    raise SystemExit('the game files were not found: pass --gamedata DIR or set U5_GAMEDATA '
                     '(a folder holding ULTIMA.EXE and the .OVL files of the GOG release)')


def find_dosbox(given=None):
    cands = [given, os.environ.get('U5_DOSBOX'), shutil.which('dosbox-x'),
             os.path.join(ROOT, '..', '..', 'u6-decompiled', 'tools', 'Dosbox-X', 'dosbox-x.exe')]
    for p in cands:
        if p and os.path.exists(p):
            return os.path.abspath(p)
    raise SystemExit('DOSBox-X was not found: pass --dosbox PATH or set U5_DOSBOX')


def read_file(gamedata, name):
    return open(os.path.join(gamedata, name), 'rb').read()


def exe_image(gamedata):
    d = read_file(gamedata, 'ULTIMA.EXE')
    hdr = struct.unpack_from('<H', d, 8)[0] * 16
    assert hdr == EXE_HEADER, hdr
    return d[hdr:]


# u5d's names carry their addresses: ULTIMA_1850_PrintString is CS:1850, TOWN_0052 is
# offset 0x52 in TOWN.OVL, D_5893_map_id is DS:5893. With /Gc (Pascal names) the object
# files hold them in upper case.
NAME_RE = re.compile(r'^_?([A-Z][A-Z0-9]*)_([0-9A-Fa-f]{4})(?:_|$)')


def name_address(name):
    """('code', cs_offset) or ('data', ds_offset) for a name that carries one, else None."""
    s = re.match(r'^_?S_0[xX]([0-9A-Fa-f]{1,4})$', name)   # a string array, S_0x6ada
    if s:
        return ('data', int(s.group(1), 16))
    m = NAME_RE.match(name)
    if not m:
        return None
    pre, off = m.group(1).upper(), int(m.group(2), 16)
    if pre == 'ULTIMA':
        return ('code', off)
    if pre == 'D':
        return ('data', off)
    if pre in OVERLAY_BASE:
        return ('code', OVERLAY_BASE[pre] + off)
    return None


def module_owner(name):
    """'ULTIMA', an overlay name, 'D' or None, from a symbol's prefix."""
    m = NAME_RE.match(name)
    return m.group(1).upper() if m else None
