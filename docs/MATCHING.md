# Matching u5d against the GOG release

The aim of the `matching` branch is source that Microsoft C 5.1 turns back into the game
files of the GOG release, byte for byte. Those files are `ULTIMA.EXE`, the 23 code
overlays, `FLAMES.OVL` and `DATA.OVL`. This page explains:

- how to set up the tools;
- how to compile and compare;
- how to work on a function until it matches;
- what has been learned about the binary.

`docs/PLAN.md` says what is left and in what order. `build/match/status.tsv`, written by the
tools, is the current state of every function.

## What you need

- **The game.** The GOG release of *Ultima V* (in the *Ultima 4+5+6* pack). The tools look
  for its files, at least `ULTIMA.EXE`, `*.OVL` and `DATA.OVL`, in:
  - `--gamedata DIR`
  - `$U5_GAMEDATA`
  - `gamedata/` in this repository
  - `../gamedata`
- **Microsoft C 5.1 and 5.0.** The original was built with both (see "Two compilers" below).
  You need the disk images of each, or the `.7z` archives holding them.
  Nothing from Microsoft is committed: `build/` is ignored.
- **DOSBox-X**, to run the compiler. Give it with `--dosbox PATH` or `$U5_DOSBOX`, or put
  `dosbox-x` on `PATH`. The tools also look in the U6 project next door
  (`../../u6-decompiled/tools/Dosbox-X/dosbox-x.exe`).
- **Python 3.11 or later**, with `iced-x86` for decoding instructions.
- **7-Zip**, only if you give the setup the `.7z` archive rather than the extracted images.

## Setting up (once)

```sh
python -m venv .venv
.venv/Scripts/pip install -r tools/requirements.txt          # Linux/macOS: .venv/bin/pip
python tools/setup_msc51.py "path/to/Microsoft C 5.1 Optimizing Compiler (5.25).7z"   # -> build/msc51
python tools/setup_msc51.py "path/to/Microsoft C Compiler 5.0 (1987) (5.25-360k).7z"   # -> build/msc50
```

`setup_msc51.py` reads the FAT12 disk images itself, so mtools is not needed. It copies the
compiler passes (CL, C1, C1L, C2, C3), LINK, LIB, the small and medium model libraries and the
headers into `build/msc51/`, then checks the executables against the MD5s of the copy this
project is matched with.

The examples below use `.venv/Scripts/python` (Windows); use `.venv/bin/python` elsewhere.

## The loop

```sh
.venv/Scripts/python tools/dosbuild.py town.c                       # compile one file
.venv/Scripts/python tools/match.py town.c -v                       # compare it, function by function
.venv/Scripts/python tools/match.py town.c --dis TOWN_0c78          # one function, side by side
.venv/Scripts/python tools/match.py --compile                       # everything, from scratch
```

### `dosbuild.py`

- Copies `src/` to a scratch drive and mounts the compiler as `D:`.
- Compiles in one headless DOSBox-X session, with the switches of `src/build.bat` (in
  `tools/u5.py`: `/Ox /Or /Gs /Gc /Zp`, with `/Oaitr` for `ultima.c`).
- Writes `build/obj/NAME.OBJ` and the listing `build/obj/NAME.COD`, which interleaves source,
  assembly and code bytes. `--obj DIR` writes them elsewhere (see "Working in parallel").
- Takes about 1–5 seconds per file.

### Data tools

- `tools/dgroup.py` maps DGROUP. It finds each module's strings by their `_TEXT(addr, ...)`
  (or `/*0xaddr*/` comment) addresses, and each `vars.c` variable by the address in its
  name, with the modules that use it.
- `tools/movevar.py FILE.c --plan/--apply` moves a module's tables out of `vars.c` into the
  module, at the place that gives the original order, fixing sizes that ran into the next
  item.
- `tools/strings.py FILE.c --apply` writes the string arrays of an *array* module (see
  "Alignment" below).
- `match.py FILE.c --data` shows a module's `_DATA` against DGROUP where they differ.

### `match.py`

Compares every function of the object with the original:

- `TOWN_0052` against offset 0x52 of `TOWN.OVL`;
- `ULTIMA_1850` against CS:1850 of `ULTIMA.EXE`.

Bytes under a fixup are not compared as bytes. Every fixup is checked instead, using the
addresses that u5d's names carry:

| Reference | It must be |
|---|---|
| call to `ULTIMA_xxxx` | a call that reaches CS:xxxx |
| call to a function of another overlay | a call to that function's overlay stub in the resident code, whose overlay number and target are checked |
| offset of `D_xxxx`+n | xxxx+n |
| offset into the module's own data (strings, statics) | one consistent base for the whole module |
| relative call or jump to another function of the same module (no fixup) | the original's target |

Names with no address (runtime library) come from the `// 0442: aNlmul` comments in
`0000.c`. Anything the tool cannot check is counted as *unverified* and never passed silently.

The verdicts, worst first:

| Verdict | Meaning |
|---|---|
| `SIZE` | the function has a different length |
| `CODE` | different instructions |
| `SLOTS` | identical except for the stack offsets of locals (`[bp-2]` and `[bp-4]` swapped, etc.) |
| `FIXUP` | identical bytes, but a reference points elsewhere. Usually module data that isn't laid out as in the original yet; the notes give the drift in bytes |
| `EXACT` | everything equal and every reference verified |

The module line also compares the module's initialised `_DATA` with `DATA.OVL` at the base
its references imply.

The exit status is 0 only when every function given is `EXACT`. Results accumulate in
`build/match/status.tsv`.

### The link: `u5link.py`

`tools/u5link.py [--out DIR] [-v]` links what `build/obj` holds into the 25 files
(`build/out`) and compares each with the original. Exit status 0: all identical and every
name defined.

- **Placement** is measured: an object's code goes where its functions' names say, its
  data segments (`_DATA`, `CONST`, `_BSS`) where its references point in the original, the
  library modules where their bytes are.
- **Resolution** is by rule: a name is the public that defines it (in its first 31
  characters: MSC 5.1 and MASM cut names there, MSC 5.0 writes 32); a call into another
  overlay goes to the function's stub; communal variables are allocated (see "The
  uninitialised data"); `u5.LINK_DEFINES` gives a second name to a defined place
  (`D_6608_overworld`). A name defined nowhere is still resolved by the address it carries,
  counted, and fails the link.
- **Relocations.** Every word that holds a segment is listed, as (offset, paragraph of its
  segment): 0000 for `_TEXT`, 072e and 0778 for PLINK86's segments, and for DGROUP 0f68, the
  paragraph where `_DATA` starts (DS:0042, after the runtime's `NULL` segment), so a word at
  DS:5216 is 0f68:51d6.
  - *Order:* module by module in link order, each module's words in the order of its fixup
    records. MSC writes a code record's fixups from the last to the first, MASM from the
    first to the last, so the table falls through a C object and rises through an assembly
    module: ULTIMA.EXE's 409 entries are seven rising runs.
  - Objects that are parts of one original object (`u5.OBJECT_PARTS`: a C file and the
    assembly standing for one of its functions) are taken together, last word first.
  - The resident words are ULTIMA.EXE's relocation table; an overlay's are its header
    (INTRO.OVL's two); DGROUP's are DATA.OVL's header (three far pointers: crt0's, crt0dat's
    and D_5394_fn, which A0DE0.ASM writes as `dw OFFSET L_1232, _TEXT`). A header is the
    entries, a zero dword, and zeros to the paragraph.
- **ULTIMA.EXE's header.** By rule: file size, relocation count, the table at 0x1e, the
  header padded to 512 bytes, the entry point (PLSTART's segment, 081d:0000). The minimum
  allocation is the end of the stack that follows DGROUP (`_end` + crt0's 0x800-byte
  `STACK`, paragraph 1bb8) plus the start-up segment's 0x52 paragraphs, less the image.
  PLINK86's own, as facts: the word 1 at 0x1c, SS:SP (0x100 paragraphs past the image,
  0x64), and the checksum 0x7e39, which is not the MZ checksum.

### The gate: `check.py`

`tools/check.py` (or `make check PYTHON=.venv/Scripts/python`) compiles every C file and
assembles every module into an empty folder (`build/check/obj`), links them into
`build/check/out`, and compares the SHA-1 of the 25 files with the GOG release's (the table in
the tool). It passes only when all 25 are identical, every object was built and the link
resolved every name by a definition. It takes about ten minutes; `--no-build` links the
objects of the last run again, `--keep` keeps them.

### `nativecheck.py`

`tools/nativecheck.py [FILE.c ...]` compiles the files with the host compiler as the native
(SDL) build would. The native build must keep compiling from the same sources, so run it
after changing C.

## Working on a function

1. `match.py FILE.c --dis FUNCTION` shows both disassemblies side by side. The listing
   `build/obj/NAME.COD` shows which source line produced which instructions.
2. Change the C. Keep the meaning, and change the form the compiler sees: expression
   shape, temporaries, declaration order, `register`, loop form, signedness.
3. Recompile and compare (`match.py FILE.c --compile -v`). To try several forms at once
   without editing the file, use `variants.py` (see "Working in parallel").
4. A change for the native (SDL) build that would alter the DOS code goes under
   `#if !defined(TARGET_DOS16)`, as u5d already does. The DOS build is `MATCHING_BUILD` plus
   the GOG differences (`GOG_BUILD`; see below).
5. What you learn about the compiler goes into **"What MSC 5.1 does"** below, with the
   smallest example that shows it.
6. A function you leave unmatched gets a `NOT MATCHING` note just above it or at the place
   it differs. The note says:
   - what still differs, in terms of the listing ("the original keeps local_6 in SI without
     storing it at e7b3");
   - every form you tried that did not fix it, and what each did ("an if/else around the
     rest of the function is not enough"; "register in an inner block takes a slot");
   - the closest version, if it is not the one in the file.

   The next attempt, in a new session or after the conversation has been compacted, starts
   from the note instead of trying the same forms again. Notes older than this rule often
   say only `NOT MATCHING`; add what you learn when you touch them.

**When a function is stuck, rewrite it from the disassembly.** The last four functions resisted three rounds of workers who refined the decompiler-derived C; a clean-room worker who
saw only stubs, the disassembly, the tools and this file matched three of them and took the fourth from 513 to 56 bytes in one session. Stub the function (keep its signature and static
data), leave out its notes and history, and write it as its 1988 author would have; read the old notes only to avoid repeating a dead end.

## Working in parallel

Several workers can match at the same time, if each keeps to these two rules:

- **one worktree per worker**, for everything that is edited, compiled and committed;
- **`variants.py` for trying things out**: it never writes to `src/`.

Without them, workers in one checkout overwrite each other's edits in `src/`, compare
against objects that somebody else compiled last (`build/obj` holds one object per module),
and mix their commits and their lines in `build/match/status.tsv`.

Each compile is a DOSBox-X run, and they share the machine: with four workers (up to seven
DOSBox-X instances) a compile took one to three minutes instead of a few seconds. Four was
about the useful limit; give workers disjoint modules, and expect a split that adds a file
(`intro2.c`) to touch shared lists (`tools/u5.py`, the build files).

### A worktree per worker: `worktree.py`

```sh
.venv/Scripts/python tools/worktree.py NAME             # ../u5d-NAME, on a new branch work/NAME
.venv/Scripts/python tools/worktree.py --list
.venv/Scripts/python tools/worktree.py NAME --remove    # the branch work/NAME is kept
```

- A worktree is a second checkout of the same repository in its own folder, on its own
  branch. Its `src/`, `build/obj` and `build/match/status.tsv` are its own.
- `build/` is not in git, so the tool copies the compilers and the assembler
  (`build/msc51`, `build/msc50`, `build/masm`) into the new worktree, and the objects and
  the status file as they are here. Then it compiles and compares `r251e.c` there, to show
  that the worktree builds.
- The game files and DOSBox-X are found from a worktree as from the main checkout
  (`../gamedata`, the U6 project next door). A `gamedata/` folder inside the main checkout
  is not: set `U5_GAMEDATA` then.
- `.venv` is not copied. In the worktree, run the tools with the main checkout's Python:
  `../u5d/.venv/Scripts/python tools/match.py town.c --compile -v`.
- Commit on `work/NAME` in the worktree. To bring the work back, merge or cherry-pick the
  branch in the main checkout. Two workers who change the same file get an ordinary git
  conflict there, instead of losing one of the changes without notice.
- `--remove` refuses when the worktree has changes that are not committed (`--force`
  removes it anyway). `--delete-branch` also deletes `work/NAME`.

### Trying variants: `variants.py`

```sh
.venv/Scripts/python tools/variants.py src/town.c spec.py TOWN_0c78             # try them all
.venv/Scripts/python tools/variants.py src/town.c spec.py TOWN_0c78 --keep 2    # write variant 2 into src/
```

`spec.py` lists the variants, each a name and the texts to replace. Every old text must
occur exactly once in the file:

```python
VARIANTS = [
    ('as it is', []),
    ('unsigned i', [('int i;', 'unsigned i;')]),
    ('two changes', [('a = b + c;', 'a = c + b;'), ('if (a != 0)', 'if (a)')]),
]
```

- Each variant is compiled and compared, and the function's line is printed as
  `match.py -v` prints it (`--notes` adds the notes; `--only 1 3` tries some of them):

  ```
  0 as it is | TOWN_0C78                          8e48 CODE  len  210/210  diffs   4 first @0031
  1 unsigned i | TOWN_0C78                          8e48 EXACT len  210/210  diffs   0
  ```

- The tool works in a folder of its own under `build/variants` (a copy of `src/` with the
  variant in it, and a private object folder) and removes it at the end. `src/`,
  `build/obj` and `status.tsv` are not touched, so any number of runs can go on at the
  same time, on the same file too.
- `--keep N` is the only thing that writes to `src/`. It applies variant N to the file as
  it is at that moment, so a variant that somebody else kept meanwhile stays. Recompile
  afterwards (`match.py FILE.c --compile`): `build/obj` still holds the old object.

### A private object folder

`dosbuild.py` and `match.py` take `--obj DIR` (or `$U5_OBJ`): the objects and listings are
written to and read from DIR instead of `build/obj`, and `match.py` writes `DIR/status.tsv`
instead of the shared status file (`--no-status`: none). `$U5_SRC` names another source
folder in the same way. `variants.py` is built on these; you rarely need them yourself.
The other tools that use objects (`slots.py`, `permute.py`, `asmbuild.py`, `u5link.py`) follow
`$U5_OBJ` too.

What is still shared inside one checkout: `src/` itself and git. So edit and commit only in
your own worktree.

## The layout of the binary

These are measured from the GOG files. `tools/u5.py` holds them, and the tools check them on
every run: a wrong base shows up as fixups that do not verify.

- **ULTIMA.EXE:**
  - MZ header of 0x80 paragraphs; 409 relocations.
  - Entry point at 0x81d:0000: PLINK86's start-up code. The EXE contains
    "Copyright (C) 1984, 1985, 1986 by Phoenix Software Associates Ltd.".
  - The runtime is Microsoft's: start-up at CS:017e–0877, and "MS Run-Time Library -
    Copyright (c) 1988, Microsoft Corp" at DS:0008.
- **Memory model:** in effect *small*, with one code segment and one data segment. Code
  calls are near. `/Gc` makes every public name Pascal-style: upper case, no underscore.
  So `D_5893_map_id` is `D_5893_MAP_ID` in the objects.
- **Overlays** are raw code images (no header, no relocations), loaded into the same code
  segment at one of four area bases:

  | Area | Base | Overlays |
  |---|---|---|
  | 1 | 0x81d0 | INTRO (base 0x81c0: its file starts with a 16-byte header, the overlay's relocation list), TOWN, MAINOUT, DUNGEON |
  | 2 | 0xa290 | COMBAT, SHOPPES, NPC, LOOKOBJ, DNGLOOK, BLCKTHRN, ENDGAME, OUTSUBS |
  | 3 | 0xbf80 | SJOG, CAST, CMDS, TALK |
  | 4 | 0xe1e0 | COMSUBS, ZSTATS, CAST2, FONT, SHOPPES2, SHOPPES3 |

  Each area starts where the previous area's largest overlay ends, so area 1 overwrites the
  PLINK start-up code after it has run. An overlay's names are offsets in its file:
  `TOWN_0052` is at CS:81d0+52.
- **Overlay stubs.** A call into another overlay goes to a 12-byte stub in the resident code:
  `call far 072e:02ec` (the overlay manager), `dw` overlay number, `jmp far 0000:target`.
  Overlay numbers count from 1 in the order of the file-name table in `ULTIMA.EXE`: TOWN 1,
  MAINOUT 2, …, SHOPPES 12, …, FONT 23, DATA 24.
- **DATA.OVL** is a 16-byte header followed by DGROUP from DS:0000. All string literals are
  in it. Each module's strings and static data sit at a fixed DS offset, in link order.
- **GOG differences.** The GOG files are 1.16 with a few patched ranges. u5d marks them with
  `GOG_BUILD`: the disk-swap prompt in `2000.c` (around CS:2405–24aa), and the start of the
  U4 import in `intro.c` (around INTRO 1332–1402). The DOS build must produce the GOG bytes
  there.

## Two compilers

The original's modules were built with **two compilers**: MSC 5.1, and MSC 5.0 for the
modules that had not been recompiled since an upgrade (a makefile rebuilds only what
changed). They are told apart by:

| | MSC 5.1 | MSC 5.0 |
|---|---|---|
| string literals | in `_DATA`, packed byte by byte | in `CONST`, each on an even address |
| `and si,7` | `83 e6 07` | `81 e6 07 00` (long immediate) |
| stack slot of a local | bucket = sum of the name's characters % 16 | bucket = (low byte + high byte) of the name added up as 16-bit words, % 16 |

Across the whole original, small logical immediates on registers other than AX are written
29 times in the long form and 4 times in the short one. The short ones are all in packed-
string modules, the long ones all in the others. MSC 5.0 also prefers to end loops with a
pointer compare.

**Stack slots under 5.0.** The slot order has the same shape in both compilers (16 buckets,
bucket 0 nearest BP, the last declared first within a bucket), but 5.0 hashes names
differently: the name is read as little-endian 16-bit words (an odd last byte is a word of
its own), the words are added modulo 0x10000, and the bucket is (low byte + high byte) % 16.
Each carry out of the low byte therefore adds one to 5.1's bucket: names of up to four
characters fall in the same bucket under both, `local_6` is bucket 0 under 5.1 and 1 under
5.0. This was measured, not guessed: 80 names of every length from 1 to 31 placed against
one-letter names of each bucket, and 150 two-local functions, all compiled with both
compilers; the rule fits every case.
`tools/slots.py` picks the rule from the file's compiler; `--hash NAME` prints both buckets.
Renaming COMBAT's, COMSUBS's and LOOKOBJ's locals by it is what let them move to 5.0.

A local declared inside an inner block is placed after all of the function's own locals
and after the slots of the compiler's temporaries made so far. So an unused slot just above
a local, while the frame has no slot to spare, says that local was declared in a block:
DNGLOOK_0c6c's `local_c` (slots -2, -4, -6, unused -8, `local_c` at -0a) matches only
when it is declared inside the `if` that uses it. Under 5.0 a `register` variable always
takes a stack slot too, wherever it is declared (SJOG_0514's `si` gives a frame two bytes
larger than the original's).

Two blocks side by side (not nested) can share slots, but only in one case measured so far:
SJOG_0514 with `int local_8` in the `for` body and `register int si` in the later `if`
block had the frame of four locals, not five. Two plain `int`s in two sibling blocks did
not share (FONT_02fc: `-6` and `-8`), and neither did the compiler's temporaries of two
loops each put in its own block. MSC also reserves a temporary slot for a loop that it
turns into a pointer walk, even when the code never uses it (FONT_02fc: frame 0Ch for four
locals).

Because the linker gathers segments by class, DGROUP holds all `_DATA` (5.1 modules' strings
and tables, 5.0 modules' tables), then the runtime's tables, then all `CONST` (5.0 modules'
strings, from DS:6ada).

`tools/u5.py` `COMPILER_BY_FILE` says which compiler builds which file. `dosbuild.py`
follows it, and for 5.0 rewrites `//` comments as `/* */` in the staged copies (5.0 knows
only the latter). MSC 5.0 modules so far: 3000, 4000, 6000, BLCKTHRN, CAST2, DNGLOOK,
COMBAT, COMSUBS, ENDGAME, FONT, LOOKOBJ, NPC, SHOPPES, SHOPPES2, SJOG, TALK, ZSTATS, and
the resident `r1850.c` and `r256e.c`. (r1850 has no strings; its word-wrap loop recomputes an
address for each test as only 5.0 does, and its locals fall in place only by 5.0's slot rule.)

**The resident 2000.c range is three objects.** CONST holds one string between FONT's and
3000.c's: DS:a0e0, ULTIMA_25ca's "Your disk may be write-protected". So the original had a
5.0 module in the middle of what u5d had as 2000.c. ULTIMA_256e and ULTIMA_25d8 call
ULTIMA_2322 with a far call, which MSC makes only for a far function of another object, so
that module starts after 2322. ULTIMA.EXE's relocation table says where: its words for
CS:2320-266c are 2567, 252b, 2516, 2333, then 2640, 2626, 25fd, 2592. MSC writes the fixups
of a code record from the last to the first, so each falling run is one object: 2320, 2322
and 251e (which sets D_545e, that object's variable), then 256e, 25ca and 25d8. They are now:
- `r2320.c` (2320), `A2322.ASM` (the GOG-patched 2322) and `r251e.c` (251e), CS:2320-256e,
  MSC 5.1: three files for one object of the original, because the assembly lies between
  the C functions (`tools/u5.py`, `OBJECT_PARTS`); its strings at DS:545f. ULTIMA_251e
  compiles the same under both compilers;
- `r256e.c`, CS:256e-266c, MSC 5.0: 256e, 25ca, 25d8; its string at DS:a0e0;
- `2000.c`, from CS:266c, MSC 5.1. Its `_DATA` holds the walkable-tile tables between its
  strings (D_54d4-D_5547, then 12 bytes of zeros no code reads, D_5548), and D_559e after
  the last string. They moved there from `vars.c`.

256e, 25ca, 2e96 and 2fd0 became exact, and 2000.c's `_DATA` is the original's.

**Checks for object boundaries.** Three checks, run over the whole build, look for the
remaining places where u5d's files do not follow the original's objects:

- *DGROUP coverage.* Every byte of DGROUP from the overlays' `_DATA` (DS:2626) to the end
  of CONST should belong to some module's data. Three ranges did not: D_28c6 (TOWN's, at
  the end of its `_DATA`), DS:50ea-520c (FONT's tables, with the four chapter titles that
  start FONT's CONST at a010) and D_520c (AFONT.ASM's variable). Now only one-byte
  alignment gaps are left there. The large ranges still unclaimed (DS:0042-1393,
  13a6-2626, 52ba-545f, 55a6-6acc) are the resident data in `vars.c`.
- *Far calls.* MSC calls a far function with `call far` from another object and with
  `push cs; call near` from its own. The only far calls left in the resident code are
  256e's and 25d8's to 2322, which the split of 2000.c explains.
- *Both compilers.* Compiling every module with both compilers and comparing per function
  found ULTIMA_5f86: right length and 19 bytes off under 5.0, 251 under 5.1. It is the
  last function before 6000.c (a 5.0 module) and uses no strings, so it is the first
  function of 6000.c's object; it is now exact there. INTRO_0986 compiles exactly under
  5.0 too, but INTRO's strings are packed at odd addresses, which 5.0 never does (it
  word-aligns every char array, with `ORG $+1`), so that object is 5.1 and the source
  has yet to be found.

**The resident data.** `vars.c` held all of the resident data. It belongs to modules, in
link order, like everything else:

| DS | Owner | What |
|---|---|---|
| 0000-0042 | runtime (chksum) | |
| 0042-2626 | `ultima.c` | the global tables, defined before `main` |
| 2626-520e | the overlays | |
| 520e-52ba | runtime | |
| 52ba-5306 | A0878.ASM | video driver parameters |
| 5306-5322 | `r0d72.c` | logo frame sizes |
| 5322-5402 | A0DE0.ASM | driver names and pointer, text windows, file state |
| 5402, 540c | `r1850.c`, `r1b38.c` | powers of ten, cursor index |
| 540e-545e | A1B94, A1EAC, A2056, A2192 (.ASM) | key tables, drive, timer, sound |
| 545e-55a6 | `r2320.c`, `2000.c` | |
| 55a6-6a0a | `3000.c` | saved game and game state (all zero) |
| 6a0a-6aa4 | `3000.c`, `4000.c`, `5000.c` | their tables |
| 6aa4-6acc | A6F1E.ASM | |
| 6acc-a524 | runtime CDATA, then CONST, then the runtime's messages | |
| a524-bd40 | uninitialised: `_BSS`, then the communal variables (below) | |

How ultima.c's data comes out in that order: MSC 5.1 emits a string literal as soon as it
meets it, but holds a definition's variables back until the end of the next function. So
the tables' initialiser literals (the item and monster names, 0042), then `main`'s two
strings (1393), then the tables themselves (13a6) are one object's `_DATA`. Both compilers
put zero-initialised data in `_DATA` and uninitialised data in communal variables.

An assembly module's data is written by `tools/asmdata.py NAME.ASM LO HI --apply`: a
`_DATA` segment holding the original bytes, a PUBLIC label at each named variable and at
each address the code uses by number, and those numeric operands turned into the labels
(`ds:[544ch]` becomes `D_544C`; MASM encodes it the same, now with a fixup). asmbuild
must still report the module identical.

Things the original's bytes corrected: D_1a58 (class-name padding) was reversed;
D_1724/D_172c and D_6a48 are two bytes longer than they had been; the weapon names at
DS:174c-17f0 are word-aligned char arrays, not literals; D_24e8, D_514a and the bytes at
DS:5548 are unused. The linked DGROUP is identical to DATA.OVL's (48,448 bytes).

**The uninitialised data** (DS:a524 to `_end`, DS:bd40; all zero in DATA.OVL) is the segments
of class BSS, in this order:

| DS | What | Defined by |
|---|---|---|
| a524-a9bc | `_BSS`: the objects' `static` variables, in link order | `mainout.c` (D_a524), `mainoutb.c` (D_a526, D_a527), `dnglook.c` (D_a528, D_a628), `comsubs.c` (D_a728, D_a872) |
| a9bc | the runtime's `XOB`/`XO`/`XOE` (empty) | crt0dat |
| a9bc-bd40 | `c_common`: the communal variables | `vars.c` (vars.h, `BSS`) |

- *Statics.* MSC puts an uninitialised `static` in the object's `_BSS`; where the runtime's
  code names `XOB` (DS:a9bc) shows where `_BSS` ends. The seven names below it are each
  used by one overlay, in link order (MAINOUT 2, DNGLOOK 10, COMSUBS 20), so they are those
  objects' statics. MSC lays statics out as it does locals: by the hash bucket of the name,
  the last declared first within a bucket (D_a728 and D_a872 share bucket 5 under 5.0's
  hash, so comsubs.c declares D_a872 first; the other pairs are in different buckets).
  COMSUBS's two lists are 0x14a bytes each: the original's test for the end of the first
  names the second (`D_a872 < p`).
- *Communals.* An uninitialised variable without `static` is a COMDEF record, a name and a
  size, in every object that declares it without `extern`; the linker gives it a place. u5link
  allocates them one after the other from the end of the BSS segments, a variable of two
  bytes or more on an even address, in the order of `u5.COMMUNALS`, and reports any that
  does not land on the address in its name or does not end at `_end`. All 70 land.
- *The order is a fact.* The shared variables come first (DS:a9bc-bb18), then those only
  INTRO uses, ENDGAME, TALK, CAST2, SHOPPES2, FONT: link order, so PLINK86 allocated a
  communal when it first met it. Within an object MSC writes COMDEF records in symbol-table
  order (`VARS.OBJ`: runs 64 declarations apart, interleaved), which depends on the
  original's names; that is why related variables are not adjacent (the shop's pointers at
  aafc-ab00, ac62 and b114-b11a, between the map buffers).
- *What the sizes showed.* D_a9bd is one byte and D_a9be another (two bytes at an odd
  address would have been aligned). Five places hold variables no code names: D_bbd2 (0xb6
  bytes, the size of D_bb1c), D_bcb0 (2), D_bd14 (1), D_bd3a and D_bd3b (1 each). They are
  declared so that the rest lands.
- `vars.c` defines the communal variables in the DOS build (`#define BSS`), and nothing
  else; in the native build it defines all of vars.h as before.

**A 5.0 module's own data.** u5d first had these modules' strings as generated char arrays
(`tools/strings.py`, a stand-in for 5.0's aligned literals) and their tables in `vars.c`.
`tools/own50.py FILE.c FIRST LAST --apply` gives a 5.0 module its own data:
- it moves the `vars.c` tables from FIRST to LAST into the module;
- it turns the strings back into plain literals;
- it places each table that holds strings so that CONST comes out in the original order.

Each module's `_DATA` range follows the link order. Every table between one module's
tables and the next is used by that module alone (endgame D_3da6-D_3e60, sjog
D_3e66-D_41cc, and so on), so the ranges were not guessed. `match.py` then reports both
`_DATA` and CONST, and for every 5.0 overlay module both are the same as the original's.
ZSTATS was the last one: its strings come out in source order, and ZSTATS_0c5c tests
"Thou art not strong enough!" before the code that prints "Ring vanishes!".

Three items turned out not to be what u5d had:
- **D_3d5a (SHOPPES):** a `char` array holding its text, not a pointer to a string.
- **D_4a9a (TALK):** "IMPE", an array in TALK's `_DATA`.
- **D_4b4e (CAST2):** eight words no code reads.

### Overlays of two objects

Five overlays hold functions of both kinds: some use packed `_DATA` strings (5.1), later ones
use CONST strings (5.0). Each was linked from two objects, a 5.1 part and then a 5.0 part,
and u5d now has them as `NAME.c` and `NAMEb.c`:

| Overlay | 5.0 part from | Its `_DATA` | Evidence |
|---|---|---|---|
| DUNGEON | `DUNGEON_104c` | DS:2de8, 328 bytes | D_2e10's strings start CONST at 6bb0 |
| MAINOUT | `MAINOUT_109e` | DS:2bc0, 152 bytes | first function with CONST strings |
| INTRO | `INTRO_1e22` | DS:3656, 39 bytes | D_3666's strings are in CONST |
| OUTSUBS | `OUTSUBS_0658` | DS:3a26, 12 bytes | the only function with CONST strings |
| CMDS | `CMDS_18be` | none | CMDS_17ec still uses packed strings |
| CAST | `CAST_1c36` | none | 1c36 and 1f60 have the right length only under 5.0 (no strings) |

The resident 5000.c is two objects as well, the other way round: `5000.c` (MSC 5.0,
ULTIMA_51a0-5910) and `r5a28.c` (MSC 5.1, 5a28-5e4a); see "Try both compilers" below.

SHOPPES2 is two MSC 5.0 objects, `shoppes2.c` (to SHOPPES2_066c) and `shoppe2b.c` (from
SHOPPES2_07e2, the shipwright, with the tables D_4d66-D_4d7a; not `shoppes2b`, which is
not an 8.3 name). Late functions that use no strings may be a 5.0 part even in a 5.1
module: try 5.0 on them before the capacity test, and rerun `slots.py`, since 5.0's hash
moves locals (CAST_1c36: 5.0's end-pointer loop compare was the tell).

- **Data.** The 5.0 part's `_DATA` follows the 5.1 part's, as the link order puts it. In
  every case the tables that follow the 5.1 part's last string are the 5.0 part's.
  `match.py` finds both blocks at those addresses with every byte the same.
- **INTRO's box ends.** INTRO_1f26 prints `"{"` and `""`, which sit packed in
  `_DATA` at 3676 and 367a, next to its CONST string " Ultima IV ". MSC 5.0 puts every
  literal in CONST (tested), so these two are initialised `char` arrays, `D_3676` and `D_367a`.
- **Uncertain boundaries.** Where no function in between uses a string and both compilers
  give the same code, the boundary cannot be seen. MAINOUT's could be anywhere from 0e4e to
  109e, DUNGEON's at 1020 or 104c. The files take the latest possible start and say so.
- **Tools.** `match.py` ends a function at the next function of either object. `u5link.py`
  places the second object where its first function's name says.
- **More than two objects.** The 5.1 parts of INTRO and CMDS are themselves several objects:
  `intro0.c` (INTRO_0010, INTRO_0050: no strings or data, so intro.c's `_DATA` still starts
  at DS:2f30), `intro.c` (to INTRO_094e) and `intro2.c` (from INTRO_0986_Main), `cmds.c` (to CMDS_1030)
  and `cmds2.c` (from CMDS_1202), linked in that order before the `b` part (TOWN, all 5.1,
  is `town.c` and `town2.c`; see "Dead references" below). They were found
  by the optimizer capacity (below): INTRO_0986 and CMDS_12c8 match only without the
  functions before them. A new object's `_DATA` starts at an even address, so CMDS2 starts
  at CMDS_1202's first string (DS:4450), right after the D_444a table, not at D_444a's
  strings (DS:43ef). Each new file is registered in `tools/u5.py` (`MODULES`) and the build
  lists (CMakeLists.txt, Makefile, src/build.bat, u5win), and functions now called across
  objects lost `static`.
- **Assembly instead of unmatched or patched C.** Three functions are built from assembly
  written from the original bytes, as the original's own assembly is, while their C stays
  for the native build only (`#if !defined(TARGET_DOS16)`, with a note pointing to the
  module): FONT_0000 (`AFONT0.ASM`, FONT.OVL 0000-02a2, MSC 5.0 output the C comes within 56
  bytes of), ULTIMA_2322 (`A2322.ASM`, CS:2322-251e) and INTRO_132a (`AINTRO2.ASM`, INTRO.OVL
  132a-1e22), the two functions GOG patched in place, whose bytes are no compiler's output.
  `tools/asmgen.py NAME START END --ovl OVERLAY` drafts an overlay part (offsets in the
  .OVL file; calls into the resident code and the overlay's own functions by name, labels by
  file offset). The data must not move: a function's literals leave the C object with it,
  so they go into the assembly module's `_DATA`, placed by u5link from the code's
  references. AINTRO2's strings follow intro2.c's packed strings at an odd address
  (DS:3345), so its `_DATA` is declared BYTE-aligned; A2322 holds all of the old object's
  data (D_545e and the strings, DS:545e-54b6), because with 2322 gone no code of r2320.c
  refers to D_545e and the link could not place it (ULTIMA_251e, the same object's last
  function, does: PLAN.md, "What is left"). Both are stand-ins for the original's
  single objects. Functions the assembly calls lose `static` (INTRO_1016, 1278, 12ea).
- **INTRO.OVL's header** is PLINK86's relocation list for the overlay, the only one with
  relocated words: a dword (CS offset, segment 0) per word that holds the code segment, the
  later word first (MSC's fixup order within an object; see "The link" below), then a zero
  dword and zeros to the paragraph. Its two
  entries are the segment halves of the far pointers to ULTIMA_2320 and 2322 that
  INTRO_132a stores in D_5394_fn. `tools/u5.py` `OVERLAY_HEADER` records its size; asmgen
  reads the relocations from it, and u5link writes it from the segment fixups of the
  overlay's objects. u5link also places MSC's far-pointer fixups (kind 3, `call far` to a far
  function of another object) and loader-resolved offsets (kind 5, the address of a far
  function), and its resident relocations are the EXE's 409.

## What MSC 5.1 does

Each entry gives the evidence. Add to this list as you learn.

- **Switches.** `/Ox /Or /Gs /Gc /Zp` (from u5d's `build.bat`) is right. For `shoppes3.c`, 11
  other combinations were all far worse: the best of them differed in 1,925 bytes against
  85.
- **Stack slots of locals: a name hash.** MSC 5.1 keeps a function's locals in a hash table
  of 16 buckets.
  - The bucket is the sum of the name's characters modulo 16.
  - Locals get their slots bucket 0 first (nearest BP).
  - Within a bucket, the last declared comes first.

  So *renaming a local can move it on the stack*. This held for every one of the 190
  functions with two or more stack locals in u5d's own listings (`research/slots.py`,
  `research/slots_rules.py`). It is MSC 4.0's rule from u4-decompiled with 16 buckets
  instead of 11. Declaration order matters only between locals of the same bucket.

  u5d's `local_N` names (from Ghidra) say where each local was in the original: [bp-(N-2)].
  `tools/slots.py FILE.c --apply` renames locals so the hash order gives the original
  order. It renames as few as possible, as `local_N_x`, and the underscore keeps N
  readable. `tools/slots.py --hash NAME` prints a name's bucket, for choosing real names
  later. This alone took the exact functions from 185 to 285.
- **Data order.** A module's `_DATA` is almost in source order:
  - string literals are written as each function is compiled;
  - a definition at file scope comes *after the literals of the function that follows it*.

  So a table that follows function F's strings in the original is defined just before F.
  Seen in `shoppes3.c`, where `tools/movevar.py` places tables this way.
- **Alignment.** Every data object (arrays, variables, `char` arrays too) starts at an even
  address, but string literals are packed byte by byte, under any switches
  (`build/exp/T1.C`). In 16 modules of the original every string starts at an even address,
  an odd-length one being followed by a pad byte. Their strings were data objects, so the
  DOS build defines them as `static char S_0x96b4[] = "...";` in DGROUP order
  (`tools/strings.py`, which generates a block after the `#include` lines and maps
  `_TEXT(0x96b4, ...)` to `S_0x96b4`).
  - *Array* modules: 3000, 4000, 6000, BLCKTHRN, CAST2, COMBAT, COMSUBS, DNGLOOK,
    ENDGAME, FONT, LOOKOBJ, SHOPPES, SHOPPES2, SJOG, TALK, ZSTATS.
  - *Literal* modules: 2000, CAST, CMDS, DUNGEON, INTRO, MAINOUT, OUTSUBS, SHOPPES3,
    TOWN.
- **A `return;` after a branch's last loop** changes the jump layout (`je end; jmp top`
  instead of `jne top; jmp end`). Found on MAINOUT_109e.
- **Struct fields and globals optimize differently.** Some u5d globals are `#define`s to
  struct fields (`D_52c8` is `D_52ba_vdp._52c8_...`). In a loop, MSC hoists a plain global
  into a register but not a struct field. The register it takes then decides the loop
  form: with CX taken, the counter is dropped and the loop ends with a pointer compare
  (`cmp si,bp; jb`), where we get `mov cx,n ... loop`. Seen on LOOKOBJ_0cf4, still
  unmatched.
- **Some loops the original optimizes, we do not, and the other way round.** In TOWN_0f02
  the original optimizes the first loop (pointer in SI, counter in DI, the bound re-read
  each pass) but not the third. We get the reverse for the first loop. Neither a separate
  index variable nor `#pragma loop_opt` explains it yet.
- **A cast compares through AX.** When a local lives in SI or DI, `x == 'N'` compiles to
  `cmp si,4Eh`. A cast that changes the type, `(uint)x == 'N'` with `int x`, goes through AX
  instead: `mov ax,si; cmp ax,4Eh`, and the `mov` is left out while AX still holds the
  value (right after `x = f();`, or between two compares). Both compilers do it.
  - Typical case: a `GetChar()` result tested against key codes in a loop.
  - Fixed ULTIMA_2AE8, ULTIMA_3B9E, SHOPPES_0280, SHOPPES_146a and ENDGAME's Y/N loops.
  - The tell in a listing: `cmp ax,imm` after `mov si,ax` or `mov ax,si`, where we compare
    the register itself.
  - Arithmetic too: `x + 0x61` with x in SI is `lea ax,[si+61h]`; `(uint)x + 0x61` is
    `mov ax,si; add ax,61h`, or just the `add` while AX holds x (SHOPPES_0b30).
- **Where an assignment goes.** `if ((x = f()) == K)` and `x = f(); if (x == K)` can choose
  registers differently. In SJOG_095C the first form gave the original's DX/CX for two loop
  variables; the second swapped them.
- **Which copy tail merging keeps.** When two blocks end in the same instructions and the
  same jump, MSC keeps one copy and the other block jumps into it. Usually the earlier copy
  is kept, but a copy that ends an if/else's then-branch wins.
  - The fix in NPC_0d00: `if (c) NPC_0c50(...); else NPC_06e4(...); break;` instead of
    `if (c) { NPC_0c50(...); break; } NPC_06e4(...); break;`. With that, case 2's identical
    call jumps forward into case 4's copy, as in the original.
  - A copy that falls through to the merge point is always kept.
  - A forward jump `mov ax,imm; jmp` into another block's `push ax` is the sign in a
    listing. The original has dozens of them, so look for an if/else when ours jumps back
    instead.
  - Still open: ZSTATS_0c5c (all its messages jump to the ammunition one; an if/else
    around the rest of the function is not enough). ULTIMA_4402 was a symbol (below), 4552
    was gotos (next item).
- **Gotos where the merge goes the wrong way.** When the original shares a block that MSC,
  given the code written out, would merge the other way round (or drop), the source had a
  goto to a label in that block. ULTIMA_4552: `case 0: goto L_restart;` with the label on
  case 6's `local_14 = 0; break;` leaves the original's dead `jmp` after the jump-table
  dispatch (the table goes straight to the label), and cases 1-4 ending in `goto L_done;`
  (the label on case 5's last `local_a = 1; break;`) jump forward as the original does.
  ULTIMA_2c4c gets most of the way with gotos to one `return 1;` (still open).
- **Write the tail in both branches.** TALK_0c5c's two branches end in the same test and
  the same PrintCr block, which the original shares. Written once after the if/else, MSC
  gives each branch its own test; written out in both branches, MSC merges the copies into
  the original's shared block, placed before the else.
- **Early returns or if/else decide the kept copy.** CMDS_0EB4 is the other way round
  from NPC_0d00. Its carpet and skiff cases were if/else chains, and the copy in the last
  else falls through and is kept. The original has `if (...) { print; return; }` for
  each, every copy ends in a jump, and the first one is kept.
- **`return;` in a function with a value.** CMDS_1030's "No effect!" paths jump straight to
  the epilogue without loading AX: the original had a bare `return;`. `RETURN_AX(value)`
  (common.h) is that `return;` in the DOS build and `return value;` elsewhere.
- **Table values kept in registers across calls**: MSC keeps a value read from a table in
  SI/DI across calls (and hoists it out of loops that call) only when it knows a call
  can't change the table. Two source forms give that and keep the table in `_DATA`:
  - MSC 5.0: the table is a `static` local of the one function that uses it (dungeonb.c:
    every table there has one user and the tables sit in function order). The data keeps
    source order, but making only some tables static while later ones stay at file scope
    breaks the layout (5.0 seems to emit file-scope data before static locals). DUNGEON_104c's
    tables stay global because translate.c patches D_2e10. (DUNGEON_111e, 127e, 1786.)
  - MSC 5.1: the table is really `const`; 5.1 leaves an initialised const table in `_DATA`
    in its place (5.0 moves it to CONST). common.h defines `const` away in the DOS build, so
    intro.c has `#undef const` around its tables (INTRO_014e).
  A `register` local in an inner block gives the code too, but takes a stack slot.
- **A label stops constant propagation.** SHOPPES2_00ac sets `local_4 = 0` and then
  indexes with it. The original reloads local_4 from the stack, where MSC would fold the 0.
  A label between the two statements gives the original's code. INTRO_1016 is the same:
  the original pushes local_10_ad (set to 8) from the stack instead of the constant.
- **Where a repeated value lives.** A value the source reads twice can become a common
  subexpression in a register, and the source's shape picks the register:
  - ULTIMA_25D8 keeps D_a9bd[0] in SI, besides storing it in local_6, only when the test
    says `D_a9c8[D_a9bd[0]]` again instead of `D_a9c8[local_6]`;
  - DUNGEON_104c indexes two tables with the map id. Written `if (x <= ...) { ... }`, 5.0
    keeps the id in SI (push si, pop si); written as an early return, `if (x > ...)
    return;`, it stays in BX, as in the original.
- **A cast can turn a loop into a pointer walk.** TALK_01e2's `for (i = 0; i < D_585b;
  i++)` over the party became the original's end-pointer loop (`cmp si,dx`) only with
  `(int)D_585b`; `!=` instead of `<` then gave its `je` for an empty party. The same cast
  did not help the other party loops (TOWN_0f02, ENDGAME_0648, CMDS_0000, DNGLOOK_117E).
- **Tail merging stops at a different symbol.** ULTIMA_4402 ends its overworld and town
  paths with the same `add ax,[bp+6]; add ax,6608h`; MSC merged both instructions, the
  original only the store after them. Naming the overworld map with a symbol of its own
  (D_6608_overworld, the same address) gives the original's code: the merge compares
  fixup targets by name.
- **Shapes the decompiler rewrote.** Ghidra's output often describes MSC's code rather than
  the source. Each of these was a whole function's difference:
  - `inc ax; jne` after a call is `if ((x = f()) + 1 == 0)`; `x = f(); if (x + 1 == 0)`
    compares with -1, and `++x == 0` also stores x (DNGLOOK_0000, SJOG_0646, where the
    decompiled `++` even read the wrong party member);
  - `x = 0; if (n > 0) { c = n; x += n; do { ... } while (--c != 0); }` is MSC's closed
    form of `for (x = 0; x < n; x++)`: the counter in DI, x set once (SHOPPES_0f64);
  - a value stored to two locals when a loop ends is a loop over one of them followed by
    `other = it;` (SHOPPES_07BE);
  - `n = 5;` followed by a loop over a pointer into a table is the closed form of
    `for (n = 1; n < 5; n++) ... table[n] ...`: MSC walks the table with a pointer of its
    own (which takes a stack slot) and sets n to its final value (ULTIMA_1a3e);
  - `mov ax,[bp+4]; jmp` to the epilogue from inside an if is an early `return param;`
    in that branch (SHOPPES_0502);
  - `y = x;` after a loop over x, with x and y in SI and DI and no store of x, is a loop over
    y itself: the decompiler split one variable in two registers (SHOPPES2_0508);
  - a nested `if (letter) { if (n < 4) {...} } else switch` whose failed inner test jumps
    to the switch is one condition, `if (letter && n < 4) {...} else switch` (CAST2_00de);
  - `x = 0; y = 0;` before an inner loop, where the original loads y's register before the
    outer loop (a dead load), is a chained `for (x = y = 0; ...)`, and an increment of y at
    the end of the body that the original does after x's is `for (...; x++, y++)`
    (CAST2_00de);
  - `x = 0; for (; n > 0; n--) { ...; x++; }` is the closed form of `for (x = 0; x < n;
    x++)` with n unchanged; written the decompiler's way, n takes SI for the whole function
    (BLCKTHRN_00be);
  - two registers incremented in a loop and stored at its exit (`CX/DX`) are two locals the
    body steps, not `i + offset` (DNGLOOK_117e);
  - `jmp test; body; incr; test: ...; jne body` with the exit falling through is a do-while
    entered at its test: `i = 0; goto T; do { body; i++; T:; } while (i < 6 && (c = f()) !=
    0xffff);` (`!= -1` gives `inc ax; je` instead) (ZSTATS_06e8, still not matching);
  - `mov al,0FFh; mov [g],al` for one store is a chained assignment, `g = g = 0xff`
    (SHOPPES2_00ac; MAINOUT_17d4 stores constants to a parameter the same way, through a
    `P2_SET` macro; DOS build only);
  - `local_12 = i * 9; local_14 = i * 3;` in a loop may be `local_12 += 9; local_14 += 3;`
    at the end of the body from `local_12 = local_14 = 0`: the original steps X and Y in
    DI/SI and keeps the counter in memory, compared with 5; the `i * K` form gives MSC's
    closed form instead. In the for header, the `+=` moves the counter's increment first
    (FONT_04A4);
  - the decompiler splits one stack slot into several variables across switch cases: map
    the original's `[bp-N]` per case and merge them (FONT_04A4: three variables were bp-0Eh;
    441 -> 365 bytes);
  - `x = init; while (x != -1) { ...; x = f(x); }` where the init and the increment share
    one store (`mov ax,init; jmp T; I: call f; T: mov [x],ax; cmp ax,-1`) is `for (x =
    init; x != -1; x = f(x))` (ZSTATS_0F2E);
  - a jump to a jump (`je L; jmp ret0; L: jmp ret1`) is an inline `return 1` that tail
    merging turned into a jmp: `if (t == X) return 1; return 0;`, where other cases `goto`
    one shared `return 1` (ULTIMA_2c4c). There `if (a && b && c) goto L_ret1; return 0;`
    keeps a case's own tail, where `if (!a || !b || !c) return 0;` merges it into the
    neighbour's;
  - a then-branch that jumps forward into an identical later copy is a `goto` to a label in
    that copy, and that label also stops MSC keeping `x * 32` in a register across calls
    (SHOPPES_14f8).
- **A loop's register locals are stored when it ends.** The original's TALK_0c5c has
  no such store, because its code after the loop is inside it (`if (local_6 == 0)
  continue;` and the rest, instead of `do ... while (local_6 == 0);`). SHOPPES2_0508 was a
  split variable instead (above).
- **The loop kind decides where those stores go.** COMBAT_0b94's inner loop, written
  `D_589e = 0; do { ... break; ... } while (++D_589e < 0x20);`, stores its register locals
  after the loop and the break jumps there. As `for (D_589e = 0; D_589e < 0x20; D_589e++)`
  the stores sit at the break and the loop's end jumps to them, as in the original. (It
  also needed `int local_6`: the original holds the flags byte in DI as an int.)
- **A char parameter.** ULTIMA_1a3e fills a buffer with its third parameter. As an `int`,
  MSC 5.0 turns the loop into `rep stosb` and needs a temporary; declared `char` (and the
  prototype with it), the loop stores AL each time as in the original, and the frame
  shrinks by the temporary. The callers' code does not change.
- **A module's earlier functions use up the optimizer's capacity.** In pass 2, every global
  symbol the module has referenced or defined so far counts against a fixed limit. Past it,
  a function's loops are not optimized: no pointer induction variables, no loop registers,
  extra temporaries (and common subexpressions that crowd out the loop registers, as in
  CMDS_12c8).
  - What counts: 40 extra `extern int` references, or 30 small functions, placed before a
    function remove its loop optimization. What does not: unreferenced declarations,
    statement count, DOS memory (400K and 608K free give the same code), the OS/2 C2.EXE
    bound for DOS, the header size.
  - It is pass-2 memory in bytes, not a symbol count (TOWN_0f02): each global symbol the
    module defines or references costs about 28 bytes plus its name's length, arrays more
    (a struct costs as an int; `static int X;` as much). So name length does count, and a
    count found by experiment holds only for the names used; windows near the limit are
    narrow and non-monotonic. Locals cost nothing past their function.
  - The test: in a `variants.py` spec, delete the functions before the target. If it then
    matches or gets close, the original probably had an object boundary there: split the
    file (INTRO_0986, CMDS_12c8; see "More than two objects").
  - Grouping variables into one symbol also relieves it, but changes alias analysis (a store
    through one array forces other variables to be re-read), so the original had separate
    symbols and a split is the fix.
  - Exceeding it is exactly `#pragma loop_opt(off)`: ZSTATS_06e8 with 600 extra references
    gives byte for byte the loop_opt(off) code. If deleting the earlier functions does not
    change a function, capacity is ruled out (ZSTATS_06e8, ULTIMA_5394, ULTIMA_5a28,
    DUNGEON_1952, FONT_0000).
  - Under 5.0 the cut-off is sharp (SHOPPES2_08a8): 8 referenced externs before the
    function still give full loop optimization, 10 give none. The function's own size
    counts too (removing its inner loops or long arithmetic brought the registers back);
    400 unreferenced externs, a minimal header or string literals do not. Split: shoppe2b.c.
  - **What the capacity is: pass 2 works in a tree pool** (measured black-box; C2.EXE is
    EXEPACKed and was not disassembled). C2 reads a function's statements into a pool and,
    when it is full, optimizes and emits what it holds and goes on with an empty pool
    (truncate pass 1's EX file mid-function and the first part's code is still written).
    A loop gets registers only if the pool has room when it comes: loops at the end of a
    function that nearly fills the pool are left unoptimized, and the same loops are
    optimized when a pool boundary falls before them. Under MSC 5.0 the pool is 612 units
    with plenty of memory (ULTIMA_1850 fills about 505).
  - **Every non-blank source line inside a function is one unit** (MSC 5.0). Pass 1 writes a
    line record for it (`4F 01 nn`, or `4F 01 80 lo hi` from line 128), also for a
    comment-only line; blank lines, `#if`'d-out lines and lines outside functions give none,
    and a comment over several lines is one. Only the number counts, not line numbers, text,
    `/Zd` or `/Fc`. So lines move the boundary, periodically: in ULTIMA_1850, 0-107 extra
    lines before 19ca change nothing, 108-122 optimize one more loop, 123-563 match, 564-605
    give other code, then again with period 612. Lines in other functions or before the
    function do nothing. ULTIMA_1850 keeps a block of empty `//` lines after its
    declarations for this (see its note). The "unused label" and `while (1)` effects below
    may be the same pool at work. **Recompile after editing comments inside a function.**
  - **Less DOS memory for C2.EXE shrinks the pool.** Under 5.0 the code is constant from about
    249 KB free upward; at 236.75-248.5 KB the pool is about 280 units (r1850.c matches
    there with no padding); below about 220 KB pass 2 runs out of heap. Under 5.1 the code
    changes from one paragraph to the next below about 254 KB (town.c). The original's code
    can therefore depend on how much memory the build machine had at that compile, and it
    was not the same for all modules (at 242 KB, 59 functions of the other 5.0 modules stop
    matching). So ULTIMA_1850's original had either 123-563 more non-blank lines or about
    240 KB for pass 2; the code cannot tell which. A memory limit through CL fails (pass 1
    runs out of heap at about 270 KB with u5d's headers); it must be applied to C2 alone.
  - **Tools.** `research/c2mem.py FILE.c FUNC FROM TO [STEP]` compiles with only N paragraphs
    free for C2; `research/c2lines.py FILE.c FUNC LINE [FROM TO STEP] [--kb N]` inserts line
    records after the function's entry in pass 1's output and runs C2/C3 on it (a full
    period is 612 under 5.0, about 390 for town.c under 5.1). Neither touches src/ or
    build/obj. If neither moves a stuck function toward the original, the pool is ruled
    out (ZSTATS_0c5c, DUNGEON_1952, FONT_0000). CL passes each pass its arguments in
    `MSC_CMD_FLAGS`; the intermediate files are `<base>EX` (trees and line records), `SY`,
    `IN`, `ST` in TMP.
  - TOWN_0f02 is this by memory: exact at 15782 or 15784-15793 paragraphs free for C2, but
    TOWN_0408, 0c78 and 1694 then differ (the same state as the extra definitions).
  - **Dead references use pass-2 memory too.** `if (0) { X = 0; ... }` naming globals the
    module does not otherwise reference generates no code, but each symbol costs pass-2
    memory like a live reference, and unlike tentative definitions it creates nothing at
    link. TOWN_0f02 matches with exactly 266 of them (265 and 267 give other code) at the
    top of the function, under `#if defined(TARGET_DOS16)`. The memory stays used for the
    rest of the object, so TOWN_0f02 must be the last function of its object: TOWN.OVL is
    `town.c` (to TOWN_0f02) and `town2.c` (from TOWN_10da, `_DATA` at DS:278a). The window
    depends on u5d's symbol names: renaming or adding globals that town.c references moves
    it. It stands for whatever held that memory in the original (unknown).
  - The other way round: TOWN_0f02 matches only with about 10 KB more pass-2 memory before
    it (330 short unreferenced `int X%d;` definitions exactly, not 325 or 335; or 120 small
    functions each setting its own extern; extern references, empty functions, defining
    vars.h's uninitialised variables, `/Zd` or `/Zi` do nothing), so the original had more
    there, depending on names and types u5d does not have. A boundary after 0f02 is
    consistent (TOWN_10da-1726 compile the same as their own object). Tentative definitions
    become communals at link, so they are no fix. Open.
  - Split by it so far: intro0.c / intro2.c (INTRO_014e, INTRO_0986), cmds2.c (CMDS_12c8),
    shoppe2b.c (SHOPPES2_08a8).
- **Try both compilers on every function before the capacity test.** 5000.c was 5.0 code
  (ULTIMA_51a0-5910, still `5000.c`) followed by 5.1 code (`r5a28.c`, 5a28-5e4a, with the
  tables at DS:6a86); compiling the whole file under the other compiler showed the boundary
  at once. The tell was ULTIMA_5394's loop exit: for a downward loop over a struct array
  (`for (i = 31; i >= 0; i--)`; a do-while is never strength-reduced) 5.0 compares the last
  pointer temporary (`cmp [bp-26h],5C5Bh; jb`) and keeps the counter's `dec`, where 5.1
  always emits `dec; jns`.
- **A `for` frees its temporaries, a `while` keeps them (5.0).** A `for` and the equivalent
  while, do-while or goto loop give the same code, but MSC 5.0 frees the `for` loop's
  temporaries at its end and a later loop's reuse their slots; with the other forms the
  later loop takes new slots and the frame grows. FONT_04A4: case 0xb's loop as a `while`
  gave the original's frame 28h, the `for` 22h.
- **A block with a declaration gives a loop its own trip-count temporary (5.0).** Two `for
  (i = 0; i < n; i++)` loops with the same bound share one trip-count temporary; in a block
  that declares a local (even unused) one gets a new slot. Braces alone do nothing
  (ZSTATS_0F2E).
- **A local stored once and never read is a CSE temporary** with a dead store (`mov
  [bp-N],ax; mov bx,ax`): write the whole expression at each use. Its slot comes after the
  loops' reserved temporaries, which a real local's does not (FONT_02FC).
- **Under 5.1 a register variable's reserved slot can fill a gap.** A function-level
  `register` local whose name hashes between two locals takes the slot between them, so a
  one-slot gap holds it with no frame change. ULTIMA_5a28's gap was its queue, decompiled
  one element too long (`int local_206[256]`, not 257).
- **`for (;;)` with a break gives an inner loop its registers.** An outer `for (;;) { if (!c)
  break; ... }` instead of `while (c)` gave FONT_0000's first inner loop register
  variables (a label before that loop's init does the same; outer loop as gotos loses them).
- **Two compares in one loop can need an if/do form**: an entry test `cmp [i],ax` (ax = n)
  with a bottom test `cmp [n],si; je` is `if (i < n) do ... while (n != ++i)`. A `for`
  gives `cmp si,[n]` at the bottom; the swapped `for` gives `cmp [n],ax` at the entry
  (FONT_0000).
- **5.0 builds a common subexpression directly in SI only for index uses** (`mov si,ax; ...;
  add si,ax`); a value CSE is computed in AX (with CX) and moved. A sum built in SI in two
  steps (`mov si,ax` after the first operand, the second in AX, `add si,ax`) comes only
  from a register pointer: `x = (byte*)a; x += b;` (DUNGEON_1952). An int register variable
  or a pointer-typed CSE is computed directly (`mov si,[bp+4]; shl si,1`).
- **A block reached only by a goto is placed after the first unconditional jump that
  follows its place in the source** (5.0, MAINOUT_17d4). To move a block out of line after
  a later jump, put it after that code as `if (0) { L_x: ...; goto L_next; }` and reach it
  with `else goto L_x;` (a label at the next test, `L_next:`). Written in place as an else-if
  it stays inline with a jump over it; at the function's end it lands right after the goto.
  A dead `goto L` after a `return` attracts nothing.
- **A while loop's floating test block after a later case** (ZSTATS_06e8): the test block
  floats behind the first case break that jumps to it. To put it after a later case, write
  the test inside the switch right after that case, followed by the loop head and `goto
  L_list`; enter with `goto L_test`, end every case and `default:` with `goto L_test`, and
  put `L_exit: return` after the switch (a `break` to the return instead makes the return
  float up behind the test).
- **Which tail-merge copy wins, continued.** An if/else chain instead of early returns
  keeps the later copy: `if (c1) f(a); else if (c2) f(b); else g();` keeps the second `f`
  (it ends a then-branch) and the first jumps into it; with `{ f(a); return; }` the first
  is kept (MAINOUT_17d4). The copy that falls through to the function's final `return`
  wins: ZSTATS_0c5c keeps its later ammunition copy only as an `else if` branch with no
  `return`, with the rest in the last `else`.
- **Code after an if/else floats behind the jump over the else** when the else's code ends
  in a jump: the code after it (up to its first unconditional jump) lands right after the
  then-branch's jump, ahead of the else block (ZSTATS_0c5c).
- **A message block after the last switch case is a goto past it.** `if (ok) goto L_go;
  msg(...); return 0; L_go: ...` after a switch places the message block after the last
  case (behind the code that follows the switch, up to its first unconditional jump) and
  keeps the strings in source order; `if (!ok) { msg; return 0; }` keeps it inline
  (ZSTATS_0c5c).
- **Slot sharing under 5.0, more precisely:** `if (1) { int t; ... }` followed by one bare
  block `{ register T x; ... }` shares t's slot. A register variable in a later if-body does
  not give its slot back to a block after it, so all of x's uses must sit in that one bare
  block (labels inside it may be goto targets). A then-branch can then be written at the
  end of the function as a block reached only by a goto and ending in one: MSC places it
  after the first jump to it, inverted, where a plain if/else would (DUNGEON_1952, which
  keeps the plain if/else for the native build under `#else`).
- **An outer `for` needs an increment expression for its inner loops to get registers
  (5.0).** `while (c)` and `for (; c; )` give none; `for (;;) { if (!c) break; ...` gives
  them but leaves the first inner loop's loads at the top of the body; `for (; c; expr)`,
  even with `expr` = `0`, gives registers, the test at the bottom and the loads after it
  (FONT_0000).
- **`if (c == 0) continue;` (or a goto to the end of the body) against `if (c != 0) {...}`:**
  the if-block keeps c's operand in AL across its first inner test; the jump form re-reads
  memory. With an increment, `continue` duplicates the increment and test at its site; a
  goto to a label at the end of the body does not (FONT_0000).
- **A loop's decrement in its while condition** (`&& start < --i`) makes the first `break` in
  the body the place where the code after the loop lands (FONT_0000).
- **Code after a loop goes to its first break.** MSC places a `for` loop's register stores
  and the code that follows the loop at the loop's first `break`; other exits jump to it.
  To put that block at a later break, make the earlier break a `goto` to a label on the
  later one: `for (;;) { if (x) goto L_brk; ... if (y == 1) { L_brk: break; } ... if (c !=
  d) continue; break; }` (NPC_032c). A do-while puts it after the loop. With the dispatch
  at the top, a switch does the same: the code after it goes to the first case's break, as
  a fall-through chain up to its first unconditional jump (ZSTATS_0c5c).
- **A block that nothing falls into follows the first jump to it.** A block that ends in an
  unconditional jump and is reached only by jumps (a while loop's test, a shared tail) is
  placed right after the first jump to it in code order, and a conditional jump to it is
  inverted. Swapping two cases swaps where it lands; `continue` or `break` makes no
  difference (ZSTATS_06e8). In SHOPPES_14f8 the tail after the switch stayed put while the
  last case fell into it; ending the switch with `goto L_tail;` and `default: goto
  L_tail;`, with case C doing `if (x == -1) goto L_tail;`, moved it into case C as in the
  original, its strings still in source order. Open: the original sometimes places such a
  block elsewhere (NPC_06e4's tail-merged else inside a later loop, ZSTATS_0c5c's "not
  strong" message after the last case).
- **`jmp near X; nop; nop; nop` is a removed dead jump**: the jump over an `else` after a
  then-branch that ends in `return` or `break`, sized first and then turned into nops
  (CAST_11de, DNGLOOK_06a8). In TALK_031e it showed which copy of a print tail the guard
  path fell into: `if (!guard) goto L_join;` instead of `else if`.
- **`*(A + i)` is not `A[i]` to the loop optimizer.** In ENDGAME_0648, `while (f(i, *(D_3e5a
  + i), *(D_3e60 + i)) != 0);` lets MSC 5.0 hoist both table bytes into SI/DI and rotate the
  loop (`call; or ax,ax; je; <hoist>; L: call; jne L`); the outer i stays in memory. With
  `D_3e5a[i]` nothing is hoisted and i takes SI. Ghidra's `if (f()) { while (f()); }` was
  this rotated while.
- **A constant-count byte copy becomes a block move.** In TALK_080a, `for (i = 0; i < 3;
  i++) dst[i] = src[i];` compiles to `mov cx,1; repne movsw; movsb` with the loop's closed
  form after it (`add [i],3`) and a reserved temporary; `memcpy(dst, src, 3)` gives `shr
  cx,1 / adc cx,cx`. With a `byte*` global as the source it took `((char*)D_bcde)[i]`.
  Tiny-count `rep movsw` elsewhere may be the same.
- **`x--; if (x == 0)` is not `if (--x == 0)`.** In TALK_080a's loop the separate decrement
  let MSC strength-reduce `x * 32` into SI (`sub si,20h; dec [x]; or si,si; jne`) and keep
  the inner counter in DI; with `--x` in the test x took DI and the product was recomputed.
- **Empty `while` or do-while.** `while ((uint)(c = GetChar()) < 0x30 || (uint)c > 0x39);`
  ends `jb top; cmp; jbe exit; jmp top`, an exit by jump; `do c = GetChar(); while (...);`
  ends `jb top; cmp; ja top` and falls through. Only the jumping exit let MSC move the code
  after the loop up before the outer loop's test. Likewise a `for` ending a then-branch gave
  `jb skip; jmp end; skip: jmp top`, and `i = 0; do { ...; i++; } while (i < 7);` the
  original's `jb top; jmp end` (CAST2_0966).
- **Fewer references move a register to another variable.** In ULTIMA_1a3e, `buf[i++] =
  (char)(q = n / d) + '0'; n -= d * q;` reads q once less than a separate `q = n / d;`, and
  MSC 5.0 then keeps d in SI and i in DI with q in memory, as the original. In ULTIMA_2d7a,
  three cases that each set `local_6 = 1` became gotos to one `local_6 = 1; break;`, so
  local_a got DI and local_6 stayed in memory.
- **A `continue` stops the closed form.** In ULTIMA_4102's inner for loop, `if (d >= best)
  continue;` instead of an if around the rest keeps the second counter's increment in the
  loop and the signed `jle` exit, as the original; without it MSC 5.0 adds the trip count
  to that counter before the loop and exits with `je`.
- **A loop index reused as scratch stops pointer walking.** ULTIMA_6506's original sets its
  loop index to `Random(7) - 4` just before `break`: the value lands in the index's DI, and
  because the index is assigned in the body MSC stops walking D_ba14 with a pointer.
- **A memset destination kept in a local**: `push di; ... rep stosb; pop di` inside a loop
  and a store of DI after it mean the destination was assigned to a local first, which the
  loop keeps in a register (`p = &GetCombatMap(0, i); memset(p, ...)`, DNGLOOK_0d3e).
- **Pointer plus offset is not `&p[...]`.** `ptr = p + (a + y) * 0x20 + (b + x);` gives
  `ax = b + x; add si,ax`; `&p[(a + y) * 0x20 + b + x]` is reassociated into a chain of
  adds (ULTIMA_5a28).
- **A `register` variable takes a stack slot under 5.1 too**, in an inner block or at
  function level: `register byte* ptr` gets SI but reserves an unused slot in its name's
  hash bucket (ULTIMA_5a28, 2 bytes of frame left); INTRO_014e's three register variables
  made its frame 12h against the original's 0Ch. `slots.py` skips a function with a
  non-`local_N` name ("nothing to change"); rename the locals first.
- **Under 5.0, unused block-scope locals take no slot** (an `int`, a `register int` or two
  in a case block left FONT_04A4's frame unchanged), and **a later block's register
  variable shares an earlier sibling block's slot only if that block is the body of an
  `if` or `for`**: a bare `{ }` or the else of `if (0)` does not share. SJOG_0514's last
  byte was `if (1) { bool local_a; ...; if (!local_a) goto L_nothing; } { register int si;
  ...; return; } L_nothing: ...`. (An `extern const` table also gave its code, but 5.0 then
  writes fixups to segment index 0, a broken object.)
- **An unsigned loop index adds a trip-count temporary**: `uint local_14` makes MSC 5.0
  store the trip count in one more temporary (bp-1Ch), as TALK_01e2's original does.
- **An unused label stops common subexpressions locally.** ULTIMA_5a28 kept a whole address,
  or `param_2_y + local_214_an`, in DI across calls; a label before the store (or before the
  later test) gave the original's `[bx+di+disp]` addressing with the row recomputed. In
  ULTIMA_1850 a label anywhere, or an outer `while (1) { ... if (x) break; }` instead of a
  do-while, acts on the whole function instead: one more loop gets registers (a label right
  before or after a loop does nothing; one is already there).
- **Branch order follows the original's layout.** NPC_0DB4's loop body sits after the loop
  test (`jmp test ... test: jge exit; head; jmp back into the branches`); it matched only
  with the if/else nesting in the original's order, the branch placed last being the final
  else. Related, from the same function:
  - `x = 0; continue;` makes MSC put a copy of the loop increment right after it; that copy
    falls through, so tail merging keeps it and the earlier identical copy jumps to it.
    Written as if/else, the earlier copy is kept, as in the original.
  - A goto into a tail-merged block makes MSC reload the register the merge relied on, so a
    jump into a merged copy that uses a live register is cross-jumping, not a source goto.
  - `(int)p->field` on a u16 compares through AX; `*(s16*)&p->field` gives the plain
    register compare and one common subexpression (`mov si,[bx]; cmp si,3; ... cmp si,4`).
- **Early returns decide where a tail-merged else goes.** In NPC_06e4, with both early
  branches ending in `return` and no `else` around the rest, the else's merged copy
  (`D_65be = 0x61; jmp`) went inside the first later loop, after a then-branch's jump; with
  the rest in an `else` it stayed after the then-branch. A goto into a loop body costs the
  loop its registers, even when the target only jumps out again.
- **MSC calls the right-hand side of a comparison first.** In `f() <= g()`, g is called
  first; for the original's call order write the comparison the other way round (CAST_1f60).
- **Case order numbers the per-case pointer temporaries.** In strength-reduced loops each
  case gets its own pointer temporary, numbered in the source order of the cases, which
  also sets the case layout (CAST_1c36). The case order in the source is the order of the
  case bodies in the code (ZSTATS_0F2E).
- **Pointer locals that are really hoisted temporaries.** SHOPPES2_08a8's decompiled `local_a
  = local_8 = &D_55a8_party[param_1].intel;` before the loop is two compiler temporaries:
  the source read `D_55a8_party[param_1].intel` at each use, and MSC 5.0 hoists only the
  invariant address, once per use (`mov [bp-6],ax; mov [bp-8],ax`), between the statements
  before the loop and the loop-register loads. With pointer locals it hoists the whole
  long factor, assuming nothing changes `*p`.
- **A `return` inside a loop with register variables stores them at its own site**
  (`mov [bp-2],si; jmp exit`). A single store at the loop exit means the returns were
  `goto L_end` past the loop, and `if (x) return; continue;` pieces were plain `break`s
  (SHOPPES2_08a8).
- **Same-length instructions with another encoding** (`and bx,0Fh` as `81 e3 0f 00`
  instead of `83 e3 0f`) are not produced by bitfields, `unsigned` or other mask
  spellings. Open.

### `permute.py`

`tools/permute.py FILE.c FUNCTION [--apply]` searches meaning-preserving variants of one
function: locals renamed into other hash buckets, `register` added or removed,
declarations swapped, comparison operands swapped. It compiles each and keeps the closest
to the original. It helps with slot and register-choice differences. It cannot find
structural changes (loop shapes, statement order).
- **Pass 1 heap.** `vars.c` makes C1 stop with C1002 "out of heap space", even with C1L.EXE.
  The data has to be split into the modules it belongs to anyway (see "DGROUP" in
  `docs/PLAN.md`).
- **Truncation.** Identifiers are cut to 31 characters (warning C4011). Two names that agree in
  their first 31 characters become one symbol.
- **No `NULL`.** MSC 5.1's `<time.h>` and `<stdlib.h>` don't define it.
- **Packing.** `#pragma pack(push, 1)` is not understood (C4109); `/Zp` packs instead.

## DOSBox-X notes

- `TMP` must name a directory that exists. Otherwise C1 stops with C1042 "cannot open compiler
  intermediate file".
- From Git Bash, MSYS rewrites arguments such as `/Ox` into Windows paths. Run the Python
  tools directly, or set `MSYS_NO_PATHCONV=1`.
