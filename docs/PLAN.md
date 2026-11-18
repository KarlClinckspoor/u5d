# Plan: u5d byte-identical to the GOG release

**Finish line.** One command builds all 25 files from `src/` with Microsoft C 5.1 in DOSBox,
plus the assembly, the runtime library and the link. It compares them with the GOG files and
passes only when they are identical.

**Rule.** No percentages: `tools/match.py` says, per function, whether it matches.
`docs/MATCHING.md` is the how-to; `docs/LESSONS-DECOMPILATION.md` records what the work taught.

## State

| Date | EXACT | SLOTS | FIXUP | SIZE | CODE | Functions |
|---|---|---|---|---|---|---|
| 2026-11-15, first baseline | 185 | 198 | 70 | 199 | 41 | 693 |
| 2026-11-15, stack-slot rule applied | 285 | 35 | 140 | 197 | 36 | 693 |
| 2026-11-15, data moved into modules, string arrays | 343 | 33 | 82 | 197 | 38 | 693 |
| 2026-11-16, foreign strings, table sizes (last committed C) | 408 | 33 | 13 | 200 | 37 | 691 |
| 2026-11-16, two compilers (5.1 + 5.0); assembly versions not counted | 487 | 18 | 5 | 75 | 20 | 605 |
| 2026-11-16, MSC 5.0 slot hash; five overlays split into 5.1 + 5.0 objects | 500 | 14 | 5 | 71 | 15 | 605 |
| 2026-11-16, 5.0 modules own their data; slot renames by observation | 511 | 4 | 5 | 71 | 14 | 605 |
| 2026-11-16, 2000.c split in three; overlay assembly (FLAMES, FONT, INTRO, NPC) | 529 | 1 | 0 | 57 | 9 | 596 |
| 2026-11-16, boundary audit: FONT/TOWN data, 5f86 into 6000.c, r1850 is 5.0 | 530 | 1 | 0 | 57 | 9 | 597 |
| 2026-11-16, vars.c into its modules: DGROUP = DATA.OVL but 44 bytes (ZSTATS) | 530 | 1 | 0 | 57 | 9 | 597 |
| 2026-11-16, casts through AX, tail-merge structure, bare returns, D_52c8 global; DGROUP identical | 539 | 1 | 0 | 49 | 8 | 597 |
| 2026-11-16, common subexpressions by source shape, D_6608_overworld | 542 | 1 | 0 | 47 | 7 | 597 |
| 2026-11-16, shapes the decompiler rewrote (closed-form loops, inc ax tests, early returns) | 549 | 1 | 0 | 40 | 7 | 597 |
| 2026-11-16, gotos and duplicated tails for merge direction, joined conditions, loop kinds | 554 | 1 | 0 | 36 | 6 | 597 |
| 2026-11-17, four parallel workers; INTRO and CMDS split into a third object (optimizer capacity) | 572 | 1 | 0 | 20 | 4 | 597 |
| 2026-11-17, batch 2: static/const tables, intro0.c, castb.c (5.0), shoppe2b.c | 583 | 1 | 0 | 7 | 6 | 597 |
| 2026-11-17, batch 3: 5000.c split (5.0 + r5a28.c 5.1), for/while temporaries, goto-placed blocks | 591 | 0 | 0 | 4 | 2 | 597 |
| 2026-11-17, clean-room rewrites from the disassembly; town.c split (town2.c); 22 overlays and DGROUP link identical | 594 | 0 | 0 | 3 | 0 | 597 |
| 2026-11-17, FONT_0000, ULTIMA_2322, INTRO_132a as assembly (AFONT0, A2322, AINTRO2); 23 overlays, resident image and DGROUP link identical | 594 | 0 | 0 | 0 | 0 | 594 |
| 2026-11-17, the gate: EXE header and relocation table, DATA.OVL's header, communal variables; all 25 files identical (`tools/check.py`) | 594 | 0 | 0 | 0 | 0 | 594 |
| 2026-11-18, two targets: GOG (1992) and the 1988 release v1.16 720K, 25/25 each; ULTIMA_2322 and INTRO_132a compiler output in 1988; the EXE checksum by rule | 596 | 0 | 0 | 0 | 0 | 596 |

The early `SIZE` verdicts in `0000.c` and `1000.c` were routines that were hand-written
assembly in the original and C in u5d (step 5); both files went with the native port.

## Steps

1. **Tools** *(done)*:
   - `setup_msc51.py` installs the compiler;
   - `dosbuild.py` compiles in DOSBox-X;
   - `match.py` compares per function and checks every fixup;
   - the layout facts are in `tools/u5.py`.

2. **Stack-slot order** *(rule found)*. Locals are hashed into 16 buckets on the sum of their
   characters; `tools/slots.py` renames them to the original order. The remaining `SLOTS`
   functions need a closer look: locals in inner blocks, register choice, or compiler
   temporaries.

3. **DGROUP: data and strings back into their modules** (most `FIXUP`, and the `_DATA`
   differences).
   - Every module's strings and static tables must be defined in that module, in the
     original order, so that its `_DATA` lands at its DS offset.
   - `vars.c` keeps only what belongs to the resident code's data, split into files C1 can
     compile.
   - A data comparison of the whole of DGROUP against `DATA.OVL`.

4. **The remaining `SIZE`/`CODE` functions in C**, module by module, smallest first.

5. **Assembly.**
   - The routines that were hand-written assembly (u5d's `// ASM` and `// STUB`), the
     start-up and the PLINK overlay manager and stubs become `.ASM` sources that assemble
     to the original bytes.
   - MASM 3.0/4.0 are in the sibling projects. Instructions they encode differently are
     written as `db`.
   - The native port's C versions went with the port.

6. **Runtime library.** Identify which modules of `SLIBCR.LIB` (and which build) make up
   the resident runtime, by matching them against the image.

7. **Link** *(done)*.
   - PLINK86 is not available, and MS LINK's overlays are a different scheme. So:
   - `tools/u5link.py`, a small OMF linker, places every module at its measured address
     (resident code, the four overlay areas, DGROUP), which it reads from the link script.
   - It writes the overlay stubs and an MZ header like the original's, with relocations in
     the original order.
   - It produces `ULTIMA.EXE`, the 23 raw overlays, `FLAMES.OVL` and `DATA.OVL`.

8. **The gate** *(done)*. `tools/check.py` (and `make check`) builds everything and compares
   the SHA-1 of all 25 files with the GOG release.

## Where it stands (2026-11-17)

Done, each checked by a tool:

- **Assembly:** all of the resident image's hand-written assembly (`src/asm/A*.ASM`, nine
  modules) and PLINK86's parts (`PLMGR`, `PLSTART`, `PLTAB`) assemble to the original bytes
  with MASM 4.0 (`tools/asmbuild.py`). Every relocated word has its segment fixup.
  Four overlays hold assembly too: all of FLAMES.OVL (`AFLAMES.ASM`), NPC.OVL's last routine
  (`ANPC.ASM`), FONT.OVL's first function and last two routines (`AFONT0.ASM`, `AFONT.ASM`),
  and INTRO.OVL's GOG-patched U4 transfer and last two routines (`AINTRO2.ASM`, `AINTRO.ASM`,
  the video driver calls u5d had as `DRV_69`). The resident image adds `A2322.ASM` (the
  GOG-patched ULTIMA_2322). Their first line names the overlay range (file offsets)
  ("FONT.OVL 0e52-0e97"); asmbuild compares them with the .OVL file and u5link places them by
  their functions' names. FLAMES.OVL links byte-identical.
- **Runtime:** CS:017e-0878 is 17 modules of SLIBCR.LIB and LIBH.LIB (`tools/libmatch.py`).
- **Link:** `tools/u5link.py` links and compares the 25 files as whole files: ULTIMA.EXE
  (header, relocation table, resident image), the 23 code overlays (INTRO.OVL with its
  header), FLAMES.OVL and DATA.OVL (header and DGROUP). All are byte-identical, and every
  reference is resolved by a definition (none by the address in its name).
- **The gate:** `tools/check.py` (`make check`) compiles and assembles everything into an
  empty folder, links, and compares the SHA-1 of the 25 files with the GOG release's. It
  passes.
- **Objects:** INTRO has four objects (`intro0.c`, `intro.c`, `intro2.c`, `introb.c`), CMDS
  three (`cmds.c`, `cmds2.c`, `cmdsb.c`), SHOPPES2 two 5.0 objects (`shoppes2.c`,
  `shoppe2b.c`), found by the optimizer capacity; CAST is a 5.1 + 5.0 pair (`cast.c`,
  `castb.c` from CAST_1c36), and the resident 5000.c a 5.0 + 5.1 pair (`5000.c`,
  `r5a28.c` from ULTIMA_5a28). See MATCHING.md.

Next, in order:

1. **Finish the C functions** (done: every C function of the DOS build is exact; FONT_0000,
   ULTIMA_2322 and INTRO_132a are assembly in the gog build and C in the 1988 build;
   `build/match/status.tsv`). Every
   module now compiles with the compiler the original used, with 5.0's own slot hash, and
   DUNGEON, MAINOUT, INTRO, OUTSUBS and CMDS are split into their 5.1 and 5.0 objects.
   - *(done)* The 5.0 modules have plain literals and their own tables (`tools/own50.py`).
     `_DATA` and CONST match the original for every 5.0 overlay module, and the linked
     DGROUP is identical to DATA.OVL's.
   - Remaining hard cases, per the last worker:
     - *(104c done: an early return.)* DUNGEON_111e/127e/1786/1952 hold a table value in a
       register across calls with no source variable (a compiler CSE temp). A `register`
       local gives the code, but 5.0 then reserves an extra stack slot; really const
       tables give it too, but move the tables to CONST.
     - MAINOUT_17d4 stores constants to a parameter through AX.
     - *(done)* LOOKOBJ_0cf4: D_52c8 is a plain global in the DOS build (A0878.ASM's name).
     - *(done)* COMBAT_0B94: `int local_6` and the inner loop as a `for`.
     - *(done, by splitting objects)* CMDS_12c8, INTRO_0986.
     - *(done in batch 2)* INTRO_014e, CAST_1c36/1f60, SHOPPES2_08a8, TALK_01e2, SJOG_0514,
       NPC_06e4/0DB4, DUNGEON_111e/127e/1786.
     - *(done in batch 3)* ULTIMA_5394 (5000.c split), ULTIMA_5a28, ULTIMA_1850 (with a
       block of comment lines standing for the original's source lines), FONT_04A4,
       FONT_02FC, ZSTATS_0F2E, ZSTATS_06e8, MAINOUT_17d4.
     - *(done by a clean-room rewrite)* ZSTATS_0c5c, DUNGEON_1952 (a register pointer sharing a
       slot, DOS build only), TOWN_0f02 (266 dead references in an `if (0)` block, DOS build
       only, and TOWN.OVL split into town.c and town2.c).
     - FONT_0000: built from AFONT0.ASM until the C matches (56 bytes, 670/674; the outer
       for's increment is unknown, and the BX temporary for `local_12 * 2` and one operand
       order differ). Its note says what is known.
     - Devices that stand for something unknown in the original (each explained in its
       note): ULTIMA_1850's comment-line padding and TOWN_0f02's dead references (both pass
       2's pool), DUNGEON_1952's register slot sharing.
     - ULTIMA_1850's padding stands for a smaller pass-2 pool (more lines or less memory in
       the original). To drop it, give `tools/dosbuild.py` a per-file memory limit for C2
       (`research/c2mem.py` has the wrapper).
     - 2000.c: ULTIMA_256e and 25d8 call ULTIMA_2322 with `call far 0:2322`, where MSC emits
       `push cs; call near` for a far function defined earlier in the same module. So 2322
       (the GOG-patched function) was probably in another object; it is now A2322.ASM.
2. **Split the resident C into its original modules** *(done)*. The runs between the
   assembly modules are objects of their own: `r0d72.c`, `r1850.c`, `r1b38.c`, `r1dda.c`,
   `r1fa0.c`, `r216c.c`, `r2320.c` + `A2322.ASM` + `r251e.c` (one object in the original,
   CS:2320-256e), `r256e.c` (256e-266c), `2000.c` (from 266c), `3000.c`, `4000.c`, `5000.c`,
   `r5a28.c`, `6000.c` (from 5f86), `7000.c` (7040-71aa).
   - The native port's C rewrites of the assembly functions (`0000.c`, `1000.c` and the
     blocks under `#if !defined(TARGET_DOS16)`) went with the port.
   - The boundary at 256e (not 251e) comes from the relocation table: see MATCHING.md,
     "The resident 2000.c range".
3. **Resident data** *(done)*.
   - Every initialised variable is defined by the module that owns it; `vars.c` gives the DOS
     build only the communal (uninitialised) variables.
   - The runtime's data segments (`_DATA`, `CDATA`, the empty `XI*`/`XC*`/`XO*` tables, the
     message segments) are the library modules' own, placed in DGROUP.
   - ULTIMA_25ca's string at DS:a0e0 is `r256e.c`'s own literal in CONST (no `_XTEXT`
     needed once the module was split off).
   - Uninitialised data: DS:a524-a9bc is the `_BSS` of four objects (their `static`
     variables), DS:a9bc-bd40 the communal variables, which the link allocates.
4. **Finish the linker** *(done)*.
   - EXE header: sizes, relocation count, header length and entry point by rule; PLINK86's
     own fields as facts (the word 1 at 0x1c, SS:SP, the checksum 0x7e39, which is not the
     MZ checksum; minimum allocation explained but measured).
   - Relocation table: modules in link order, each module's words in the order of its
     fixup records (MSC: last to first; MASM: first to last). That gives the original's
     seven increasing runs once 2320-256e is one object.
   - DATA.OVL's header: the three far pointers in DGROUP, as 0f68:<DS-0x40> (the paragraph
     where `_DATA` starts).
   - No reference is resolved by the address in its name: the communal variables are
     allocated by size in PLINK86's order (`u5.COMMUNALS`), the statics are in `_BSS`, and
     `D_6608_overworld` is a linker definition (`u5.LINK_DEFINES`).
5. **The gate** *(done)*. `tools/check.py`; `make check`.
6. **The link script** *(done)*. `src/u5d.lnk` holds every placement (each module's code
   address and data segment offsets, the 17 library modules, the overlays' sizes, DGROUP's
   size, `_edata`/`_end`, PLINK86's EXE header fields); `tools/linkscript.py` measures it
   from the game files and `--check`s it. `u5link.py` links from the script and the objects
   alone, and `check.py` builds without the game files (MATCHING.md, "The link script").

7. **Readable names** *(done: every module, the shared vocabulary and the resident code)*. Names change the compiled code here (a local's name its
   stack slot, the length of global names pass 2's memory), so renames go through
   `tools/rename.py`, which keeps the slot order (a suffix where needed), the 31 significant
   characters and the address prefixes, recompiles and compares, and puts the files back
   when a function stops matching. MATCHING.md, "Readable names", has the convention, the
   tool, the measured cost and the plan.
   - Done (pilot): TALK and SHOPPES3 whole (47 functions: names, parameters, locals, a
     comment above each), the SHOPPES helpers they call, the shops' and the conversation's
     globals, and six widely used savegame globals (gold, food, party size, karma, hour,
     active player): 37 functions, 39 globals, 79 locals, 36 parameters. No pool window
     moved; 31 locals carry a slot suffix.
   - TOWN_0f02's fill no longer names other modules' globals (`F_xxxx` in town.c), so naming
     a global elsewhere cannot move its window.
   - Done (shared vocabulary; batches in `docs/names/vocab-*.txt`): 28 struct fields (the
     actors' tile/animTile/x/y/z/value/animState/extra, the NPC state, the party record's
     monthsAtInn and innMapId, the video parameters; the U4 party's virtues), 91 globals of
     the saved game with the Codex's SAVED.GAM offsets (`D_5c5a_actors`, `D_5893_mapId`,
     `D_57ac_keys`, `D_5876_tempX`...; the ten snake_case names are lowerCamel now), 60
     globals of the state, the tables and the text and video variables (`D_ba14_combatants`,
     `D_ab02_viewTiles`, `D_24e6_viewDirty`, `D_17f6_equipNames`...), and 60 resident
     functions with a comment above each (`ULTIMA_4f7c_AdvanceTime`,
     `ULTIMA_2a52_DamagePlayer`, `ULTIMA_6506_PlaceCombatant`...). Two functions moved:
     TOWN_0f02 (15 fill references taken out) and CMDS_0000 (cmds.c keeps 21 short link
     names); MATCHING.md, "What the shared vocabulary cost".
   - Left of the shared names, for whoever meets them: the savegame bytes whose use is
     one module group's (`D_5882`, `D_5883`, `D_5889`, `D_588a`, `D_588d`, `D_588f`,
     `D_5890`, `D_58a2`, `D_58a8`, `D_58cb`, `D_5959`, `D_5d5a`, `D_5d5c`, `D_65be`-`D_65c2`,
     `D_6602`), the fields `_18`, `_8`, `_a` and `_52c0`-`_52c6`, the parameters of the
     resident functions (free: any name), the static resident functions and the assembly's
     private labels (the resident code comes last), and the tables only one group uses.
   - Done (module groups; one batch per module in `docs/names/`): every overlay module.
     Shops and town (ZSTATS, SHOPPES2, SHOPPES, NPC, TOWN), magic and combat (COMSUBS,
     CAST2, CAST, COMBAT), world (OUTSUBS, BLCKTHRN, LOOKOBJ, DNGLOOK, MAINOUT, DUNGEON,
     SJOG), commands and the rest (ENDGAME, FONT, INTRO, CMDS): about 360 functions with a
     comment above each, 1,700 locals and parameters, 260 globals of one group. TOWN_0f02
     lost 6 more fill references (245 left); cmds.c's short link names are 24 (COMBAT_0000,
     OUTSUBS_0658 and DNGLOOK_117e, which CMDS_0000 calls). MATCHING.md, "What the module
     groups cost".
   - To continue: the resident code (2000.c-7000.c, the `r*.c` files, ultima.c, the
     assembly's private names), with the shared globals the groups left (used by the
     assembly or several groups). Small batches: ULTIMA_1850, town.c and cmds.c are at limits.
   - Done (resident code; `docs/names/resident-*.txt`): the last 18 functions, about 540
     locals and parameters, 160 globals, the struct fields; then the stand-in step
     (CMDS_0000's dead references, TOWN_0f02's fill four to a line).
8. **Re-derive the link facts** *(planned)*. `src/u5d.lnk` still states where everything
   goes (71 `object` and 17 `library` lines with addresses, 23 `overlay` lines, the `exe`,
   `dgroup` and `define` lines), `u5.COMMUNALS` the order of the communals, `u5.LINK_DEFINES`
   D_6608's second name, and PLMGR/PLTAB are written from the original's bytes. The
   original linker, PLINK86plus 2.24, is at hand (`../tools/plink86plus-2.24`, not in the
   repository), and linking u5d's objects with it already gives every overlay file the
   original's length and 26,397 of the root's 29,408 bytes in place (MATCHING.md, "Linking
   with PLINK86plus 2.24"). Two phases:
   - **A. Reproduce the original link with the original linker** (PLINK86plus as the
     oracle; `research/plink/`): find Origin's link command file.
     1. The loader: which OVERLAY.LIB variant (`DEFINE $OVLYVD=...`) and segment order give
        PLMGR (0x4a0 bytes, CS:72e0) before the overlay table (PLTAB, CS:7780) and the
        overlay areas at 81d0; then the areas' bases follow (INTRO's 81c0 with its header).
     2. DGROUP as DATA.OVL: the section and header that write it to a file of its own (its
        header: three far pointers).
     3. The library order: why memchr comes after uldiv in the original (the order the
        first objects reference names in, or link order), with the merged SLIBCE.LIB.
     4. The segment names: the root objects' code segment is not `_TEXT` (only then does
        the library code follow ULTIMA.OBJ); find the natural form (MSC `/NT`, the
        assembly's `SEGMENT` names) and check it changes no byte.
     5. The communals: PLINK allocates them in COMDEF record order (research/RULES.md);
        with step 4's objects, check the order it gives against `u5.COMMUNALS`.
     6. Then byte for byte: the linked EXE (with Origin's wrapper appended as a separate
        step, as Origin did) and all overlays and DATA.OVL against both targets.
   - **B. Make u5d's own build follow the rules** (no Phoenix file needed to build):
     1. u5link takes PLINK's rules instead of addresses: segment order by first
        appearance, breadth-first library search, overlay areas as large as their largest
        section, alignment, communals in record order, the overlay table and stubs.
     2. `src/u5d.lnk` becomes a PLINK-style command file (file order, sections, areas,
        libraries, DEFINEs) plus whatever stays a fact; `linkscript.py --check` compares
        the addresses u5link computes with the original's.
     3. `u5.COMMUNALS` becomes a check (the order follows from the declarations, as
        research/RULES.md describes), D_6608's second name a DEFINE in the command file.
     4. PLTAB generated by u5link from the sections (as PLINK does); PLMGR stays assembly
        (it is OVERLAY.LIB's code, as SLIBCR's modules are the library's), PLSTART stays
        (Origin's wrapper, not the linker's).
   Gate: `check.py` for both targets throughout; A changes no source byte, B none either.

## What is left

The 25 files are identical, and building them needs only the repository, the compilers,
MASM and DOSBox-X: the game files are needed to compare (or the SHA-1 table is used) and to
measure the link script again. What the build still takes from the original rather than
from a rule or from source, each named where it lives:

- **Placement** is the link script's (`src/u5d.lnk`): 154 lines of facts, measured by
  `tools/linkscript.py`. Much of it could become rule: a module's code follows the one
  before it (with the alignment of its segment), and the data segments follow in link order
  per class; the script would then shrink to the link order. Not tried.
- **The order of the communal variables** (`u5.COMMUNALS`, 70 names) is a fact, and no rule
  was found (MATCHING.md, "The uninitialised data"). At the level of objects the part from
  DS:bb18 is first-seen in link order (INTRO0, INTRO2, ENDGAME, TALK, CAST2, SHOPPES2,
  SHOPPE2B, FONT); inside an object the order is neither the order of first reference nor
  u5d's EXTDEF order, and the 33 shared ones are in the symbol-table order of one object
  that declared them all. Both depend on the original's names. Five (D_bbd2, D_bcb0,
  D_bd14, D_bd3a, D_bd3b) are referenced by no code and declared only to fill the layout.
- **`D_6608_overworld`** is a linker definition; which object defined the second name is
  not known.
- **The EXE checksum** is computed (u5link: PLINK86's MZ checksum of the program without
  PLSTART, Origin's copy-protection wrapper; for gog, of the 1988 link, since the 1992
  patch left the field alone).
- **The 1988 release** is a second target (`--target 1988`, gate 25/25; MATCHING.md, "Two
  targets"). Its ULTIMA_2322 and INTRO_132a are compiler output; only the wrapper's entry
  (PLSTART) and A1EAC's drive-letter byte are conditional assembly.
- **Stand-in devices**, final (each explained in its note; MATCHING.md, "What pass 2
  counts, measured"):
  - ULTIMA_1850's comment lines (r1850.c): kept. Its limit is the pool's cut, the widest
    window of the three (123-563 lines); a C2 memory limit for r1850.c would do the same
    and is no more certain.
  - TOWN_0f02's 245 dead references (town.c): kept, four to a line. The original had about
    7.5 KB more pass-2 memory in use between TOWN_0c78 and TOWN_0f02; no uniform cause fits.
  - CMDS_0000's 72 dead references (cmds.c): replaced the 27 short link names; window 64-80.
  - AINTRO2's BYTE-aligned `_DATA` and A2322 holding D_545e and r2320's strings: the gog
    target only, the price of building 1992's patched functions from assembly; the 1988
    target has neither.
  - DUNGEON_1952's and SJOG_0514's register slot sharing is not a stand-in: the original
    must have shared that slot too.
- **FONT_0000** is assembly in both targets (unmatched C, 670/674 bytes; MATCHING.md has what
  is known). ULTIMA_2322 and INTRO_132a are assembly in the gog target only (1992's patch).
