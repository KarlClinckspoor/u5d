"""Shared facts and paths for the matching tools.

Every number here was measured from the GOG release of Ultima V (see docs/MATCHING.md,
"The layout of the binary"). tools/layout.py --check re-derives them from the game files.
Where each module's code and data go is in the link script (src/u5d.lnk), which
tools/linkscript.py measures and tools/u5link.py links from.
"""
import os
import re
import shutil
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
BUILD = os.path.join(ROOT, 'build')
MSC51 = os.path.join(BUILD, 'msc51')
# Where the sources are read, and where the objects (NAME.OBJ, NAME.COD) are written and read.
# $U5_SRC and $U5_OBJ (or --obj DIR of dosbuild.py and match.py) name private folders, so
# that several workers can compile and compare at the same time (docs/MATCHING.md, "Working
# in parallel"). The tools read u5.SRC and u5.OBJ when they use them, not when they start.
DEFAULT_OBJ = os.path.join(BUILD, 'obj')
SRC = os.path.abspath(os.environ.get('U5_SRC') or os.path.join(ROOT, 'src'))
OBJ = os.path.abspath(os.environ.get('U5_OBJ') or DEFAULT_OBJ)


def set_obj(path):
    """Use path as the object folder, in this process and the ones it starts."""
    global OBJ
    if path:
        OBJ = os.path.abspath(path)
        os.environ['U5_OBJ'] = OBJ


def status_path():
    """match.py's status file: build/match/status.tsv for the shared object folder, and
    status.tsv inside a private object folder (a private run never writes the shared one)."""
    if os.path.normcase(OBJ) == os.path.normcase(DEFAULT_OBJ):
        return os.path.join(BUILD, 'match', 'status.tsv')
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
# them, tools/linkscript.py places them in their overlay by their functions' names.
del MODULES['flames.c']
MODULES.update({'%d000.c' % i: ('root', None) for i in range(8)})
MODULES['ultima.c'] = ('root', None)
# the small C modules between the resident assembly modules (named by their CS address)
MODULES.update({n + '.c': ('root', None) for n in ('r0d72', 'r1850', 'r1b38', 'r1dda', 'r1fa0', 'r216c',
                                                   'r2320', 'r251e', 'r256e')})
# 5000.c (MSC 5.0, ULTIMA_51a0 to 5910) is followed by an MSC 5.1 object from 5a28 to 5f86.
MODULES['r5a28.c'] = ('root', None)
MODULES['vars.c'] = ('data', None)
# INTRO.OVL's MSC 5.1 code is three objects: intro0.c (INTRO_0010, INTRO_0050), intro.c (to
# INTRO_094e) and intro2.c (from INTRO_0986_Main), linked in that order before introb.c
# (docs/MATCHING.md).
MODULES['intro0.c'] = ('ovl', 'INTRO')
MODULES['intro2.c'] = ('ovl', 'INTRO')
# Likewise CMDS.OVL: cmds.c (to CMDS_1030), cmds2.c (from CMDS_1202), cmdsb.c.
MODULES['cmds2.c'] = ('ovl', 'CMDS')
# TOWN.OVL: town.c (to TOWN_0f02) and town2.c (from TOWN_10da).
MODULES['town2.c'] = ('ovl', 'TOWN')
# SHOPPES2.OVL is two MSC 5.0 objects: shoppes2.c (to SHOPPES2_066c_Pub) and shoppe2b.c (from
# SHOPPES2_07e2_CanAfford, the shipwright).
MODULES['shoppe2b.c'] = ('ovl', 'SHOPPES2')

# The original was built with two compilers. Modules last compiled with MSC 5.0 put their
# string literals in a CONST segment, each on an even address, and encode small logical
# immediates in the long form (and si,7 as 81 e6 07 00); MSC 5.1 modules keep literals in
# _DATA, packed. Each module is compiled with the one the original used (docs/MATCHING.md).
COMPILER_BY_FILE = {f: 'msc50' for f in (
    '3000.c', '4000.c', '6000.c', 'blckthrn.c', 'cast2.c', 'combat.c', 'comsubs.c', 'dnglook.c',
    'endgame.c', 'font.c', 'lookobj.c', 'npc.c', 'r1850.c', 'r256e.c', 'shoppes.c', 'shoppes2.c', 'sjog.c',
    'talk.c', 'zstats.c', '5000.c')}
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
OBJECT_PARTS = [('R2320', 'A2322', 'R251E'), ('INTRO2', 'AINTRO2'), ('AFONT0', 'FONT')]

# A second name for a place that another name defines, as a linker's DEFINE gives. The
# original reaches the overworld map through a name of its own for DS:6608 (vars.h); which
# object defined it is not known.
LINK_DEFINES = {'D_6608_OVERWORLD': 'D_6608_MAP'}
# cmds.c knows 21 globals by their short names (the note at its top: CMDS_0000's pass-2 memory)
LINK_DEFINES.update({
    'D_5876': 'D_5876_TEMPX',
    'D_5878': 'D_5878_TEMPY',
    'D_587A': 'D_587A_STATUSICON',
    'D_587D': 'D_587D_MONTH',
    'D_588C': 'D_588C_CAMPHEALWAIT',
    'D_588E': 'D_588E_PROTECTTURNS',
    'D_5894': 'D_5894_SAVEDMAPID',
    'D_589E': 'D_589E_CURCOMBATANT',
    'D_595A': 'D_595A_DUNGEONMAP',
    'D_5C5A': 'D_5C5A_ACTORS',
    'D_A9CE': 'D_A9CE_SOUNDON',
    'D_BA14': 'D_BA14_COMBATANTS',
    'ULTIMA_3AE6': 'ULTIMA_3AE6_WAITFRAMES',
    'ULTIMA_400C': 'ULTIMA_400C_RINGREGENERATION',
    'ULTIMA_4A84': 'ULTIMA_4A84_DRAWSUNANDMOONS',
    'ULTIMA_4F7C': 'ULTIMA_4F7C_ADVANCETIME',
    'ULTIMA_6800': 'ULTIMA_6800_WAKECOMBATANT',
    'ULTIMA_6880': 'ULTIMA_6880_SLEEPIFNOTPOISONED',
    'ULTIMA_6936': 'ULTIMA_6936_INITCOMBATANTS',
    'ULTIMA_6BC2': 'ULTIMA_6BC2_SETUPCOMBAT',
    'ULTIMA_6D82': 'ULTIMA_6D82_ISINVIEW',
})

# The uninitialised (communal) variables, in the order PLINK86 allocated them from the end of
# the BSS segments (DS:a9bc) to _end (DS:bd40); tools/u5link.py allocates them by their
# sizes in this order and reports any that does not land on the address in its name.
# vars.c defines them. The order is a fact: the shared ones first, then those of INTRO,
# ENDGAME, TALK, CAST2, SHOPPES2 and FONT in link order, so PLINK86 allocated a communal when
# it first met it; within an object that is MSC's symbol-table order, which depends on names
# the original's source had and u5d does not (docs/MATCHING.md, "The resident data").
COMMUNALS = [n.upper() for n in (
    'D_a9bc D_a9bd_curDisk D_a9be D_a9c0 D_a9c2 D_a9c4 D_a9c6 D_a9c8_diskDrives D_a9ce_soundOn D_a9d0 D_a9fa_vitalsDirty D_a9fb D_a9fc_savedActors D_aafc_shopName D_aafe_shopOwner D_ab00_itemName D_ab02_viewTiles D_ac62_placeName D_ac64_viewActors D_ad14_combatMap '
    'D_b114_shopIndex D_b116_shopKind D_b118_shopPrice D_b11a D_b11c D_b11e_tileFrames D_b21e_scratch D_b9ee D_ba14_combatants D_bb14 D_bb15 D_bb16 D_bb17 '
    'D_bb18 D_bb1a D_bb1c D_bbd2 D_bc88 D_bcb0 '
    'D_bcb2 D_bcda '
    'D_bcdc_talkNpcIndex D_bcde_talkPtr D_bce0_talkCodeArgs D_bce4_talkWord D_bcf4_talkLabel D_bcf6_talkKeywordIndex D_bcf8_inputText '
    'D_bd08 D_bd14 D_bd15 '
    'D_bd16_pubKind D_bd18_boughtSomething D_bd1a_dinersFed D_bd1c_dinerCount D_bd1e_tableTile D_bd20_drinksBought D_bd22_thrownOut D_bd24_shipSold '
    'D_bd26 D_bd27 D_bd28 D_bd29 D_bd2a D_bd32 D_bd3a D_bd3b D_bd3c D_bd3d D_bd3e '
).split()]

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


def find_gamedata(given=None, required=True):
    """The folder of the GOG release's files: the one given, else $U5_GAMEDATA, else
    gamedata/ here or beside the checkout. A folder that is named (given or $U5_GAMEDATA) is
    the only one looked in. The files are needed to compare with and to measure from
    (tools/match.py, tools/linkscript.py, ...), not to build: with required=False the answer
    is None when they are not found."""
    named = given or os.environ.get('U5_GAMEDATA')
    for p in ([named] if named else [os.path.join(ROOT, 'gamedata'), os.path.join(ROOT, '..', 'gamedata')]):
        if os.path.exists(os.path.join(p, 'ULTIMA.EXE')):
            return os.path.abspath(p)
    if not required:
        return None
    raise SystemExit('the game files were not found%s: pass --gamedata DIR or set U5_GAMEDATA '
                     '(a folder holding ULTIMA.EXE and the .OVL files of the GOG release)'
                     % (' in %s' % named if named else ''))


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
