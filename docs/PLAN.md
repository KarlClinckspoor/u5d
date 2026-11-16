# Plan: u5d byte-identical to the GOG release

**Finish line.** One command builds all 25 files from `src/` with Microsoft C 5.1 in DOSBox,
plus the assembly, the runtime library and the link. It compares them with the GOG files and
passes only when they are identical.

**Rule.** No percentages: `tools/match.py` says, per function, whether it matches.
`docs/MATCHING.md` is the how-to.

## State

| Date | EXACT | SLOTS | FIXUP | SIZE | CODE | Functions |
|---|---|---|---|---|---|---|
| 2026-11-15, first baseline | 185 | 198 | 70 | 199 | 41 | 693 |
| 2026-11-15, stack-slot rule applied | 285 | 35 | 140 | 197 | 36 | 693 |
| 2026-11-15, data moved into modules, string arrays | 343 | 33 | 82 | 197 | 38 | 693 |
| 2026-11-16, foreign strings, table sizes (last committed C) | 408 | 33 | 13 | 200 | 37 | 691 |
| 2026-11-16, two compilers (5.1 + 5.0); assembly versions not counted | 487 | 18 | 5 | 75 | 20 | 605 |

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

7. **Link.**
   - PLINK86 is not available, and MS LINK's overlays are a different scheme. So:
   - `tools/u5link.py`, a small OMF linker, places every module at its measured address
     (resident code, the four overlay areas, DGROUP).
   - It writes the overlay stubs and an MZ header like the original's, with relocations in
     the original order.
   - It produces `ULTIMA.EXE`, the 23 raw overlays, `FLAMES.OVL` and `DATA.OVL`.

8. **The gate.** `tools/check.py` (and a Makefile target) builds everything and compares the
   SHA-1 of all 25 files with the GOG release.

## Where it stands (2026-11-16)

Done, each checked by a tool:

- **Assembly:** all of the resident image's hand-written assembly (`src/asm/A*.ASM`, nine
  modules) and PLINK86's parts (`PLMGR`, `PLSTART`, `PLTAB`) assemble to the original bytes
  with MASM 4.0 (`tools/asmbuild.py`). Every relocated word has its segment fixup.
- **Runtime:** CS:017e-0878 is 17 modules of SLIBCR.LIB and LIBH.LIB (`tools/libmatch.py`).
- **Link:** `tools/u5link.py` links and compares. SHOPPES3.OVL is byte-identical.
- **Not committed:** the overlay C files have uncommitted changes from six matching agents
  that were stopped part-way. Review them with `tools/match.py --compile` and
  `tools/nativecheck.py`; keep what improves, revert what doesn't.

Next, in order:

1. **Finish the overlay C functions** (about 150 left; `build/match/status.tsv`).
2. **Split the resident C into its original modules.** The runs between the assembly
   modules are 0d72, 1850-1b16, 1b38, 1dda-1eac, 1fa0-2056, 216c, and 2320-3072 / 3072-400c /
   400c-51a0 / 51a0-60ec / 60ec-6f1e (each with its own data in DGROUP), plus 7040-71aa.
   - Exclude the C rewrites of assembly functions from the DOS build.
   - Match the remaining resident C.
3. **Resident data.**
   - Move vars.c's tables into their modules (or into data-only modules where no owner is
     known, saying so).
   - Place the runtime's data segments (_DATA, CDATA, XI*/XC* tables, NMSG) in DGROUP.
   - Use `_XTEXT` for ULTIMA_25ca's string at DS:a0e0.
4. **Finish the linker.**
   - EXE header: PLINK's fields as facts; the checksum 0x7e39 is not the standard MZ one.
   - Relocation table in the original order (seven increasing runs: module order, then
     fixup order).
   - DATA.OVL header: three seg:off entries for far pointers in DGROUP, 0f68:<DS-0x40>.
   - Accept no reference resolved only by the address in its name.
5. **The gate.** `tools/check.py` builds everything and compares the SHA-1 of all 25 files.
