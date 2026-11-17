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

Most `SIZE` verdicts in `0000.c` and `1000.c` are routines that were hand-written assembly in
the original and are C in u5d (step 5).

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
   - The C versions stay for the native build.

6. **Runtime library.** Identify which modules of `SLIBCR.LIB` (and which build) make up
   the resident runtime, by matching them against the image.

7. **Link** *(done)*.
   - PLINK86 is not available, and MS LINK's overlays are a different scheme. So:
   - `tools/u5link.py`, a small OMF linker, places every module at its measured address
     (resident code, the four overlay areas, DGROUP).
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
   ULTIMA_2322 and INTRO_132a are assembly in the DOS build, their C native-only;
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
   - The C rewrites of assembly functions are native-only (`#if !defined(TARGET_DOS16)`);
     `0000.c` and `1000.c` give the DOS build no code.
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

## What is left

The 25 files are identical. What the build still takes from the original rather than from a
rule or from source, each named where it lives:

- **Placement is measured at link time.** `u5link.py` reads the original files to place each
  object's data (where its references point) and the library modules (where their bytes
  are). The game files are therefore needed to build. Recording these facts in a link
  script (one line per object: code address, `_DATA`, `CONST`, `_BSS`) would make the link
  independent of them.
- **The order of the communal variables** (`u5.COMMUNALS`, 70 names) is a fact. The evidence
  says PLINK86 allocated each when it first met it (shared ones first, then INTRO's,
  ENDGAME's, TALK's, CAST2's, SHOPPES2's, FONT's, in link order); within an object that is
  MSC's symbol-table order, which depends on the original's names. Five of them (D_bbd2,
  D_bcb0, D_bd14, D_bd3a, D_bd3b) are referenced by no code and declared only to fill the
  layout.
- **`D_6608_overworld`** is a linker definition; which object defined the second name is
  not known.
- **The EXE checksum** (0x7e39) is copied; what PLINK86 summed was not found (not the file,
  the image, the header or the overlays, as words or bytes).
- **Stand-in devices** in the source, each explained in its note: ULTIMA_1850's comment-line
  padding, TOWN_0f02's dead references (both pass 2's pool), DUNGEON_1952's register slot
  sharing, AINTRO2's BYTE-aligned `_DATA`, A2322 holding D_545e and r2320's strings.
  - ULTIMA_1850: `r1850.c` is an object of its own, and it matches without the padding
    when pass 2 has 236.75-248.5 KB (MATCHING.md). A per-file memory limit for C2 in
    `dosbuild.py` (`research/c2mem.py` has the wrapper) would replace the padding with a
    statement about the build machine; neither can be told from the other by the code.
  - A2322: ULTIMA_251e, now known to be part of the same object, refers to D_545e, so
    D_545e could be defined in `r251e.c` again; A2322's `_DATA` (the strings from DS:545f)
    would then need BYTE alignment, as AINTRO2's has. Not applied: it trades one device for
    another.
- **FONT_0000, ULTIMA_2322, INTRO_132a** are assembly in the DOS build (the first is
  unmatched C, the other two GOG's patches).
