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
- **Microsoft C 5.1.** The fourteen 5.25" disk images, or the `.7z` archive holding them.
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
python tools/setup_msc51.py "path/to/Microsoft C 5.1 Optimizing Compiler (5.25).7z"
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
