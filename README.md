# u5d - Ultima V Decompilation Project

## Introduction

A decompilation of Ultima V: Warriors of Destiny (MS-DOS) into C and assembly that rebuilds
the original game files **byte for byte** with the original toolchain.

Inspired by [u4-decompiled](https://github.com/ergonomy-joe/u4-decompiled), the project was
started by wonst719 as a personal challenge to understand the game and its code; this fork
carries it to an exact rebuild.

## Current Status

- **Byte-identical rebuild of two releases**, chosen by a target switch:
  - `gog`: the GOG release (a January 1992 patch of v1.16), the default;
  - `1988`: the original October 1988 release, v1.16 for 720K disks.
- All 25 files of each are identical: `ULTIMA.EXE`, the 23 code overlays, `FLAMES.OVL` and
  `DATA.OVL` (`tools/check.py`: "targets gog PASS, 1988 PASS").
- Every C function compiles to the original's bytes (594 in the GOG build, 596 in the 1988
  build) with Microsoft C 5.1 or 5.0, the compiler the original used for each file, except
  FONT_0000, which is built from assembly in both. The routines that were hand-written
  assembly in the original are assembly here too.
- The code has readable names throughout; the measured rules behind them (a local's name
  decides its stack slot, name lengths use the optimizer's memory) are in `docs/MATCHING.md`.
- The original was linked with Phoenix PLINK86plus 2.24. u5d links with its own small OMF
  linker (`tools/u5link.py`) from a link script (`src/u5d.lnk`), and computes the EXE
  checksum by PLINK's rule.

`docs/MATCHING.md` explains the tools, the method and what was learned about the binary;
`docs/PLAN.md` has the state and what is left.

## Building the original files

You need the game files to compare with (they are never committed), Microsoft C 5.1 and 5.0
(disk images or the `.7z` archives holding them), MASM 4.0 (`build/masm/MASM.EXE`),
DOSBox-X and Python 3.11 or later. `docs/MATCHING.md`, "What you need", has the details.

```sh
python -m venv .venv
.venv/Scripts/pip install -r tools/requirements.txt          # Linux/macOS: .venv/bin/pip
python tools/setup_msc51.py "path/to/Microsoft C 5.1 Optimizing Compiler (5.25).7z"   # -> build/msc51
python tools/setup_msc51.py "path/to/Microsoft C Compiler 5.0 (1987) (5.25-360k).7z"   # -> build/msc50
.venv/Scripts/python tools/check.py          # build both releases and compare (make check)
```

Game files are looked for in `gamedata/` (GOG) and `gamedata-1988/` (1988), in this
repository or beside it, or where `--gamedata`, `$U5_GAMEDATA_GOG`, `$U5_GAMEDATA_1988` or
`$U5_GAMEDATA` say. Every tool takes `--target gog|1988` (or `$U5_TARGET`).

## Credits

- wonst719, for the decompilation this fork builds on
- [Ultima Codex Wiki](https://wiki.ultimacodex.com/wiki/Ultima_V_internal_formats) - for game data and save-file formats
- [Exodus Project](https://github.com/mcmagi/ultima-exodus) - for the annotated disassembly
- The original Ultima V was created by Origin Systems and Lord British.
