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
  | 1 | 0x81d0 | INTRO (base 0x81c0: its file starts with a 16-byte header), TOWN, MAINOUT, DUNGEON |
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
the resident `r1850.c` and `r251e.c`. (r1850 has no strings; its word-wrap loop recomputes an
address for each test as only 5.0 does, and its locals fall in place only by 5.0's slot rule.)

**The resident 2000.c range is three objects.** CONST holds one string between FONT's and
3000.c's: DS:a0e0, ULTIMA_25ca's "Your disk may be write-protected". So the original had a
5.0 module in the middle of what u5d had as 2000.c. ULTIMA_256e and ULTIMA_25d8 call
ULTIMA_2322 with a far call, which MSC makes only for a far function of another object, so
that module starts after 2322. They are now:
- `r2320.c`, CS:2320-251e, MSC 5.1: 2320 and the GOG-patched 2322; its strings at DS:545f;
- `r251e.c`, CS:251e-266c, MSC 5.0: 251e, 256e, 25ca, 25d8; its string at DS:a0e0;
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
| 6acc-... | runtime CDATA, then CONST | |

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
Left: the uninitialised data above DS:a524 (all zero in DATA.OVL) is communal, and the
link still finds those 72 names only by the addresses in them. Communal records carry sizes,
not types, yet the layout has type alignment and unnamed gaps, so the allocation rule is
still to be found.

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
  and `cmds2.c` (from CMDS_1202), linked in that order before the `b` part. They were found
  by the optimizer capacity (below): INTRO_0986 and CMDS_12c8 match only without the
  functions before them. A new object's `_DATA` starts at an even address, so CMDS2 starts
  at CMDS_1202's first string (DS:4450), right after the D_444a table, not at D_444a's
  strings (DS:43ef). Each new file is registered in `tools/u5.py` (`MODULES`) and the build
  lists (CMakeLists.txt, Makefile, src/build.bat, u5win), and functions now called across
  objects lost `static`.

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
    function remove its loop optimization. What does not: unreferenced declarations, long
    names, statement count, DOS memory (400K and 608K free give the same code), the OS/2
    C2.EXE bound for DOS, the header size.
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
  - Near the limit, comment lines inside a function count against it (blank lines do not);
    far from it, 200 comment lines change nothing. Recompile after editing notes in such a
    function (TOWN_0f02).
  - The other way round: TOWN_0f02 matches only with about 270 more global symbols before
    it (unreferenced tentative definitions, or 120 small functions each setting its own
    extern; 80 extern references or 120 empty functions do nothing), so the original had
    more there. Tentative definitions become communals at link, so they are no fix. Open.
  - Split by it so far: intro0.c / intro2.c (INTRO_014e, INTRO_0986), cmds2.c (CMDS_12c8),
    shoppe2b.c (SHOPPES2_08a8).
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
