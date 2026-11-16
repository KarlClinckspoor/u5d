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
  assembly and code bytes.
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
3. Recompile and compare (`match.py FILE.c --compile -v`).
4. A change for the native (SDL) build that would alter the DOS code goes under
   `#if !defined(TARGET_DOS16)`, as u5d already does. The DOS build is `MATCHING_BUILD` plus
   the GOG differences (`GOG_BUILD`; see below).
5. What you learn about the compiler goes into **"What MSC 5.1 does"** below, with the
   smallest example that shows it.

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
DS:5548 are unused. DGROUP now differs from DATA.OVL only in ZSTATS's two swapped strings.
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
`_DATA` and CONST, and for every 5.0 overlay module both are the same as the original's,
with one exception:
- **ZSTATS:** two strings are swapped. ZSTATS_0c5c (not yet matched) prints "Thou art not
  strong enough!" before "Ring vanishes!" in the original's source.

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
    around the rest of the function is not enough) and probably ULTIMA_4402 and 4552.
- **A label stops constant propagation.** SHOPPES2_00ac sets `local_4 = 0` and then
  indexes with it. The original reloads local_4 from the stack, where MSC would fold the 0.
  A label between the two statements gives the original's code.
- **A loop's register locals are stored when it ends.** The original's TALK_0c5c has
  no such store, because its code after the loop is inside it (`if (local_6 == 0)
  continue;` and the rest, instead of `do ... while (local_6 == 0);`). Moving SHOPPES2_0508's
  code into its inner loop the same way lost every register local, so that one is still
  open.
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
