"""Shared facts and paths for the matching tools.

Every number here was measured from the GOG release of Ultima V (see docs/MATCHING.md,
"The layout of the binary"). tools/layout.py --check re-derives them from the game files.
Where each module's code and data go, tools/u5link.py computes from the link command file
(src/u5d.lnk) by PLINK's rules; it checks that the overlay bases, header lengths, numbers
and DGROUP's paragraph here (which the names' addresses rest on) are what it computes, and
tools/linkscript.py compares its placement with the original files.

u5d builds two releases, chosen by the target (TARGETS below): the GOG release (the default)
and the October 1988 release it was patched from.
"""
import contextlib
import glob
import os
import re
import shutil
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
BUILD = os.path.join(ROOT, 'build')
MSC51 = os.path.join(BUILD, 'msc51')

# The releases u5d builds, chosen by the target: a tool's --target, else $U5_TARGET, else gog.
#   gog    the GOG release: Origin's October 1988 link of v1.16 as patched in January 1992
#          (ULTIMA.EXE: the entry of PLSTART, ULTIMA_2322 and one byte of A1EAC; INTRO.OVL:
#          INTRO_132a). The default.
#   1988   the October 1988 release, v1.16 720K (the files are kept as ../gamedata-1988).
# The two differ only in ULTIMA.EXE and INTRO.OVL. A target's sources are compiled and
# assembled with its macros defined (CL and MASM /DRELEASE_1988 for 1988, none for gog): the
# parts that differ are `#if defined(RELEASE_1988)` in C (ULTIMA_2322 in diskswap.c, INTRO_132a
# in intro2.c: 1.16's C, compiler output in 1988), `IFDEF RELEASE_1988` in assembly (PLSTART's
# first 35 bytes, one byte of A1EAC).
TARGETS = ('gog', '1988')
DEFAULT_TARGET = 'gog'
TARGET_TITLE = {'gog': 'the GOG release', '1988': 'the 1988 release'}
TARGET_DEFINES = {'gog': (), '1988': ('RELEASE_1988',)}
# the folder of a target's game files, in this repository or beside it (find_gamedata)
GAMEDATA_DIR = {'gog': 'gamedata', '1988': 'gamedata-1988'}
# A target whose files are another target's link patched afterwards. The patch did not write
# the EXE checksum again, so the checksum is the other target's link's (tools/u5link.py): the
# GOG target's build also builds the 1988 form of the modules that differ, for it.
PATCH_OF = {'gog': '1988'}
# Assembly modules a target leaves out, because C stands for them there: the 1988 target
# compiles ULTIMA_2322 (with D_545e_lastDisk and diskswap.c's strings) from src/diskswap.c and INTRO_132a
# (with its strings) from src/intro2.c; A2322.ASM and AINTRO2.ASM are the 1992 patch's bytes.
# The link command file's lines for them carry target=gog (src/u5d.lnk).
ASM_EXCLUDED = {'gog': (), '1988': ('A2322', 'AINTRO2')}


def _target(t):
    t = t or DEFAULT_TARGET
    if t not in TARGETS:
        raise SystemExit('unknown target %r (U5_TARGET or --target): one of %s' % (t, ', '.join(TARGETS)))
    return t


TARGET = _target(os.environ.get('U5_TARGET'))

# Where the sources are read, and where the objects (NAME.OBJ, NAME.COD) are written and read.
# $U5_SRC and $U5_OBJ (or --obj DIR of dosbuild.py and match.py) name private folders, so
# that several workers can compile and compare at the same time (docs/MATCHING.md, "Working
# in parallel"). The tools read u5.SRC and u5.OBJ when they use them, not when they start.
# Each target has its own object folder: build/obj for gog, build/obj-1988 for 1988.
DEFAULT_OBJ = os.path.join(BUILD, 'obj')
SRC = os.path.abspath(os.environ.get('U5_SRC') or os.path.join(ROOT, 'src'))


def default_obj(target=None):
    """The shared object folder of a target: build/obj (gog), build/obj-1988 (1988)."""
    target = target or TARGET
    return DEFAULT_OBJ if target == DEFAULT_TARGET else DEFAULT_OBJ + '-' + target


OBJ = os.path.abspath(os.environ.get('U5_OBJ') or default_obj())


def set_obj(path):
    """Use path as the object folder, in this process and the ones it starts."""
    global OBJ
    if path:
        OBJ = os.path.abspath(path)
        os.environ['U5_OBJ'] = OBJ


def set_target(target):
    """Build and compare target, in this process and the ones it starts. The object folder
    becomes the target's own, unless one was named ($U5_OBJ, set_obj)."""
    global TARGET, OBJ
    if target:
        TARGET = _target(target)
        os.environ['U5_TARGET'] = TARGET
        if not os.environ.get('U5_OBJ'):
            OBJ = os.path.abspath(default_obj())


@contextlib.contextmanager
def building(target, obj):
    """Within the block, the tools in this process build target into the folder obj (the
    variables only: processes started inside inherit the outer ones)."""
    global TARGET, OBJ
    old = TARGET, OBJ
    TARGET, OBJ = _target(target), os.path.abspath(obj)
    try:
        yield
    finally:
        TARGET, OBJ = old


def add_target_argument(ap):
    """--target for a tool's argparse parser (apply it with set_target(a.target))."""
    ap.add_argument('--target', choices=TARGETS,
                    help='the release to build and compare: gog (the default, else $U5_TARGET) or 1988')


def target_defines(target=None):
    """The compiler and assembler switches of a target: ' /DRELEASE_1988' or ''."""
    return ''.join(' /D' + d for d in TARGET_DEFINES[target or TARGET])


def patch_base_obj():
    """For a target that is another's link patched afterwards (PATCH_OF): the folder of that
    other target's objects, where the build puts the other form of the modules that differ:
    build/obj-1988 beside build/obj, DIR-1988 beside a private DIR. None for other targets."""
    base = PATCH_OF.get(TARGET)
    if not base:
        return None
    if os.path.normcase(OBJ) == os.path.normcase(os.path.abspath(default_obj())):
        return os.path.abspath(default_obj(base))
    return OBJ + '-' + base


def mentions_target_macro(path):
    """Whether a source file uses a target's macro (RELEASE_1988): in a preprocessor line of
    C (#if, #elif, ...), in assembly outside a comment (IFDEF ...). Comments do not count."""
    lines = open(path, encoding='latin-1').read().splitlines()
    if path.upper().endswith('.ASM'):
        lines = [l.split(';')[0] for l in lines]
    else:
        lines = [l for l in lines if l.lstrip().startswith('#')]
    macros = [d for t in TARGETS for d in TARGET_DEFINES[t]]
    return any(re.search(r'\b%s\b' % d, l) for l in lines for d in macros)


def target_dependent(path):
    """Whether a source builds differently by target: it uses a target's macro, or (a C file)
    some header in src/ does. Every other source gives the same object for every target."""
    if mentions_target_macro(path):
        return True
    if path.lower().endswith('.c'):
        return any(mentions_target_macro(h) for h in glob.glob(os.path.join(SRC, '**', '*.h'), recursive=True))
    return False


def asm_modules(target=None):
    """The assembly modules of a target's build (src/asm/*.ASM, less ASM_EXCLUDED)."""
    out = sorted(glob.glob(os.path.join(SRC, 'asm', '*.ASM')))
    return [p for p in out if os.path.splitext(os.path.basename(p))[0].upper() not in ASM_EXCLUDED[target or TARGET]]


def target_dependent_objects(target=None):
    """The names of the objects (C and assembly) that a target builds differently."""
    out = {os.path.splitext(os.path.basename(p))[0].upper() for p in asm_modules(target) if target_dependent(p)}
    out |= {os.path.splitext(f)[0].upper() for f in MODULES if target_dependent(os.path.join(SRC, f))}
    return out


def status_path():
    """match.py's status file: build/match/status.tsv for the shared object folder (gog),
    build/match/status-1988.tsv for the 1988 target's, and status.tsv inside a private
    object folder (a private run never writes the shared one)."""
    if os.path.normcase(OBJ) == os.path.normcase(os.path.abspath(default_obj())):
        return os.path.join(BUILD, 'match', 'status.tsv' if TARGET == DEFAULT_TARGET else 'status-%s.tsv' % TARGET)
    return os.path.join(OBJ, 'status.tsv')


def replace_file(tmp, dst):
    """Move tmp over dst in one step, so that a reader never sees half a file. On Windows
    this fails while another process has dst open: try again for a short while."""
    import time
    for attempt in range(50):
        try:
            os.replace(tmp, dst)
            return
        except PermissionError:
            if attempt == 49:
                raise
            time.sleep(0.1)

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
# tools/u5link.py computes the areas (PLINK's rule) and notes any base that is not this.
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
# INTRO.OVL's 16-byte header is PLINK86's relocation list for the overlay (the only overlay
# with relocated words): a dword (CS offset, segment 0) for each word that holds the code
# segment, the later word first, then zeros. Its two entries are the segment halves of the far
# pointers to ULTIMA_2320 and ULTIMA_2322 that INTRO_132a stores in D_5394_diskSwapFn.
OVERLAY_HEADER = {'INTRO': 0x10}


def overlay_relocations(gamedata, ovl):
    """The CS addresses of the relocated words of overlay ovl, from its header."""
    d = read_file(gamedata, ovl + '.OVL')
    out = []
    for k in range(0, OVERLAY_HEADER.get(ovl, 0) - 3, 4):
        off, seg = struct.unpack_from('<HH', d, k)
        if not off and not seg:
            break
        out.append(seg * 16 + off)
    return out

# The C files of the DOS build, and what each one is compared with.
#   ('ovl', NAME)   the whole _TEXT segment is NAME.OVL
#   ('root', None)  functions are compared at their CS addresses in ULTIMA.EXE
#   ('data', None)  initialised data, compared with DATA.OVL (DGROUP)
MODULES = {o.lower() + '.c': ('ovl', o) for o in OVERLAYS}
# FLAMES.OVL is all assembly (src/asm/AFLAMES.ASM), as are parts of other overlays (AFONT0,
# AFONT, AINTRO2, AINTRO, ANPC: first line "NAME.OVL xxxx-yyyy"): tools/asmbuild.py assembles
# them, src/u5d.lnk names them in their overlay's section, and tools/linkscript.py measures
# where the original has them by their functions' names.
del MODULES['flames.c']
# The resident C objects (docs/MATCHING.md, "The resident C files"): ultima.c (CS:0000), the
# small modules between the resident assembly modules (logo.c 0d72 to fileio.c 256e), then
# party.c (266c), commands.c (3072), world.c (400c), view.c (51a0), visible.c (5a28),
# battle.c (5f86) and lighthse.c (7040).
MODULES.update({n + '.c': ('root', None) for n in ('party', 'commands', 'world', 'view', 'battle', 'lighthse')})
MODULES['ultima.c'] = ('root', None)
MODULES.update({n + '.c': ('root', None) for n in ('logo', 'print', 'pollkey', 'waitkey', 'backspc', 'ustrlen',
                                                   'diskswap', 'swdisks', 'fileio')})
# view.c (MSC 5.0, ULTIMA_51a0_RandomFrame to 5910) is followed by an MSC 5.1 object from 5a28 to 5f86.
MODULES['visible.c'] = ('root', None)
MODULES['vars.c'] = ('data', None)
# INTRO.OVL's MSC 5.1 code is three objects: intro0.c (INTRO_0010_PutBackdropStrips, INTRO_0050), intro.c (to
# INTRO_094e) and intro2.c (from INTRO_0986_Main), linked in that order before introb.c
# (docs/MATCHING.md).
MODULES['intro0.c'] = ('ovl', 'INTRO')
MODULES['intro2.c'] = ('ovl', 'INTRO')
# Likewise CMDS.OVL: cmds.c (to CMDS_1030_YellShadowlord), cmds2.c (from CMDS_1202_RestoreShrine), cmdsb.c.
MODULES['cmds2.c'] = ('ovl', 'CMDS')
# TOWN.OVL: town.c (to TOWN_0f02_TileEffects) and town2.c (from TOWN_10da_ShoutBegone).
MODULES['town2.c'] = ('ovl', 'TOWN')
# SHOPPES2.OVL is two MSC 5.0 objects: shoppes2.c (to SHOPPES2_066c_Pub) and shoppe2b.c (from
# SHOPPES2_07e2_CanAfford, the shipwright).
MODULES['shoppe2b.c'] = ('ovl', 'SHOPPES2')

# The original was built with two compilers. Modules last compiled with MSC 5.0 put their
# string literals in a CONST segment, each on an even address, and encode small logical
# immediates in the long form (and si,7 as 81 e6 07 00); MSC 5.1 modules keep literals in
# _DATA, packed. Each module is compiled with the one the original used (docs/MATCHING.md).
COMPILER_BY_FILE = {f: 'msc50' for f in (
    'commands.c', 'world.c', 'battle.c', 'blckthrn.c', 'cast2.c', 'combat.c', 'comsubs.c', 'dnglook.c',
    'endgame.c', 'font.c', 'lookobj.c', 'npc.c', 'print.c', 'fileio.c', 'shoppes.c', 'shoppes2.c', 'sjog.c',
    'talk.c', 'zstats.c', 'view.c')}
COMPILER_BY_FILE['shoppe2b.c'] = 'msc50'
# Overlays built from two objects: NAME.c (MSC 5.1) and NAMEb.c (MSC 5.0), linked in that
# order. The 5.0 part holds the functions that use CONST strings; its _DATA follows the 5.1
# part's in DGROUP (docs/MATCHING.md, "Overlays of two objects").
for _o in ('DUNGEON', 'MAINOUT', 'INTRO', 'OUTSUBS', 'CMDS', 'CAST'):
    MODULES[_o.lower() + 'b.c'] = ('ovl', _o)
    COMPILER_BY_FILE[_o.lower() + 'b.c'] = 'msc50'

# Objects that are the parts of one object of the original: a C file and the assembly module
# that stands for one of its functions (docs/MATCHING.md, "Assembly instead of unmatched or
# patched C"). MSC wrote such an object's fixups in one record, last to first, and the
# relocation tables have its words in that order (tools/u5link.py, relocation_order).
OBJECT_PARTS = [('DISKSWAP', 'A2322', 'SWDISKS'), ('INTRO2', 'AINTRO2'), ('AFONT0', 'FONT')]


def object_parts(target=None):
    """OBJECT_PARTS for a target: without the assembly modules it leaves out (ASM_EXCLUDED)."""
    ex = ASM_EXCLUDED[target or TARGET]
    return [tuple(n for n in g if n not in ex) for g in OBJECT_PARTS]

EXE_HEADER = 0x800  # 0x80 paragraphs
DATA_OVL_HEADER = 0x10  # DATA.OVL is a 16-byte header, then DGROUP from DS:0000
# DGROUP's paragraph, relative to the load segment: right after overlay area 4's largest
# member (0xe1e0 + 0x1460 = 0xf640). Relocated words with this value are 'DGROUP'.
# (tools/u5link.py computes it and notes when it is not this.)
DGROUP_SEG = 0x0f64

# Overlay numbers, as the stubs pass them to the overlay manager: the position (from 1) of
# the file name in the table in ULTIMA.EXE.
OVERLAY_NUMBER = {n: i + 1 for i, n in enumerate(
    ['TOWN', 'MAINOUT', 'DUNGEON', 'INTRO', 'FLAMES', 'NPC', 'COMBAT', 'BLCKTHRN', 'LOOKOBJ',
     'DNGLOOK', 'OUTSUBS', 'SHOPPES', 'ENDGAME', 'SJOG', 'CMDS', 'CAST', 'TALK', 'CAST2',
     'ZSTATS', 'COMSUBS', 'SHOPPES2', 'SHOPPES3', 'FONT', 'DATA'])}


# SHA-1 of the GOG release's files (Ultima 4+5+6 pack, Ultima V 1.16 as patched by GOG).
GOG_SHA1 = {
    'ULTIMA.EXE': '9971bbeb6d00fe8c43a98d5ad2d7b82be2d0ca8b',
    'DATA.OVL': '9b5b640d4b6a014c1983a7f0527bbba9bfb4204e',
    'FLAMES.OVL': '942370a93a77019ee38a6cccc2050267c4b01b03',
    'BLCKTHRN.OVL': '002a9b427e85a3a5bf5518fd0feacf8ac38da1e0',
    'CAST.OVL': '517573779ddb41b40f305a5e20d0d4f95fd751ea',
    'CAST2.OVL': '03d9e239f8c347825e39780c5e0299306064700b',
    'CMDS.OVL': 'e5ae97bcb3d6d0f531787789f6ee02872c370a17',
    'COMBAT.OVL': '48b1a3e38bf9e95458458b2fbbe8f8c4c5b6ee2b',
    'COMSUBS.OVL': '13c9479fc26c851fc9a4eb6ba1843109f09e0b16',
    'DNGLOOK.OVL': '1677a1625db99021ff54d5378761219480e41b94',
    'DUNGEON.OVL': 'c1c407a48915517dc161e28a464c8df65d0a69d8',
    'ENDGAME.OVL': 'e801051bebfdf167ff35be6035688f44983ab57a',
    'FONT.OVL': '90303cf773e225ef4bbd462ab34598c29b1dea94',
    'INTRO.OVL': '4195043891ee60f82c65fe739d77f2ac3e98168c',
    'LOOKOBJ.OVL': 'fe25e3dbabbd3057ab5dd60c18bcf402d1952f80',
    'MAINOUT.OVL': 'd027edaee9b2317f649182a12610f6b60410d649',
    'NPC.OVL': '3c48134a8a99093c3c21b5d7f0253428cc1b5848',
    'OUTSUBS.OVL': '1710166896b91198630794223445f483afd04f78',
    'SHOPPES.OVL': '8c51e5fae50fc0e7524e01d040abd6177e85cd3d',
    'SHOPPES2.OVL': '1644c2ed9d9ad4a1e1dc8ae944eb4040e12c3c8d',
    'SHOPPES3.OVL': '82c5fdb37a965f3976b2d7b63207b032cce56c3e',
    'SJOG.OVL': '188d9e684199f899d33532f6913485786f337962',
    'TALK.OVL': '17ff6bbde9d9781874ba74f9ab5feb3b9808c612',
    'TOWN.OVL': 'fdfed52705b7aa6efc2a89a5b2628f3537292372',
    'ZSTATS.OVL': '94a03813bcb563ccde005d1a09cb67b5e75ea684',
}
# SHA-1 of the 1988 release's files (v1.16 720K, October 1988): GOG's but for the two files
# the 1992 patch changed.
SHA1_1988 = dict(GOG_SHA1, **{
    'ULTIMA.EXE': '0633077be971894b28e320917e6a48b2d40a8b87',
    'INTRO.OVL': '9007ce6ed8377570ec9f22eeecf5a7ee6bcd325f',
})
RELEASE_SHA1 = {'gog': GOG_SHA1, '1988': SHA1_1988}


def release_sha1(target=None):
    """{file name: SHA-1} of a target's 25 files."""
    return RELEASE_SHA1[target or TARGET]


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


def find_gamedata(given=None, required=True, target=None):
    """The folder of the target's game files: the one given, else $U5_GAMEDATA_GOG or
    $U5_GAMEDATA_1988 (the target's own), else $U5_GAMEDATA, else gamedata/ (gog) or
    gamedata-1988/ (1988) here or beside the checkout. A folder that is named (given or by
    a variable) is the only one looked in. The files are needed to compare with and to measure
    from (tools/match.py, tools/linkscript.py, ...), not to build: with required=False the
    answer is None when they are not found."""
    target = target or TARGET
    named = given or os.environ.get('U5_GAMEDATA_' + target.upper()) or os.environ.get('U5_GAMEDATA')
    d = GAMEDATA_DIR[target]
    for p in ([named] if named else [os.path.join(ROOT, d), os.path.join(ROOT, '..', d)]):
        if os.path.exists(os.path.join(p, 'ULTIMA.EXE')):
            return os.path.abspath(p)
    if not required:
        return None
    raise SystemExit('the game files were not found%s: pass --gamedata DIR or set U5_GAMEDATA '
                     '(a folder holding ULTIMA.EXE and the .OVL files of %s)'
                     % (' in %s' % named if named else '', TARGET_TITLE[target]))


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
# offset 0x52 in TOWN.OVL, D_5893_mapId is DS:5893. With /Gc (Pascal names) the object
# files hold them in upper case.
NAME_RE = re.compile(r'^_?([A-Z][A-Z0-9]*)_([0-9A-Fa-f]{4})(?:_|$)')


NAMES_TSV = os.path.join(HERE, 'names.tsv')
_NAMES = None


def _table():
    """{first 31 characters, upper case: (name, kind, module, address)} from tools/names.tsv,
    the names that carry an address (tools/names.py): the readable names carry none."""
    global _NAMES
    if _NAMES is None:
        _NAMES = {}
        if os.path.exists(NAMES_TSV):
            for line in open(NAMES_TSV, encoding='utf-8').read().split('\n')[1:]:
                f = line.rstrip('\r').split('\t')
                if len(f) == 4:
                    _NAMES[f[0][:31].upper()] = (f[0], f[1], f[2], int(f[3], 16))
    return _NAMES


def name_info(name):
    """(module, kind, address) for a name that has an address, else None. The module is
    ULTIMA, an overlay's name or D; kind is 'code' or 'data'; the address is the offset (in
    the overlay, for an overlay's function). The name is in names.tsv (a readable name, or
    one with its prefix) or carries its address as a prefix (MODULE_addr...)."""
    row = _table().get(name[:31].upper())
    if row:
        return row[2], row[1], row[3]
    m = NAME_RE.match(name)
    if m:
        pre, off = m.group(1).upper(), int(m.group(2), 16)
        if pre == 'ULTIMA':
            return 'ULTIMA', 'code', off
        if pre == 'D':
            return 'D', 'data', off
        if pre in OVERLAY_BASE:
            return pre, 'code', off
    return None


def name_address(name):
    """('code', cs_offset) or ('data', ds_offset) for a name that carries one, else None."""
    s = re.match(r'^_?S_0[xX]([0-9A-Fa-f]{1,4})$', name)   # a string array, S_0x6ada
    if s:
        return ('data', int(s.group(1), 16))
    info = name_info(name)
    if not info:
        return None
    mod, kind, off = info
    if kind == 'data':
        return ('data', off)
    return ('code', off if mod == 'ULTIMA' else OVERLAY_BASE[mod] + off)


def module_owner(name):
    """'ULTIMA', an overlay name, 'D' or None: the module of a name that has an address."""
    info = name_info(name)
    return info[0] if info else None


