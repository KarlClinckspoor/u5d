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

Most `SIZE` verdicts in `0000.c` and `1000.c` are routines that were hand-written assembly in
the original and are C in u5d (step 5).

## Steps

1. **Tools** *(done)*:
   - `setup_msc51.py` installs the compiler;
   - `dosbuild.py` compiles in DOSBox-X;
   - `match.py` compares per function and checks every fixup;
   - the layout facts are in `tools/u5.py`.

2. **Stack-slot order** (most `SLOTS`). Find MSC 5.1's rule for which local gets which
   `[bp-n]`.
   - *Ruled out:* name hash (MSC 4.0), declaration order, file-scope declaration count.
   - *Next:* minimal test files compiled in one DOSBox run, varying order of first use, use
     count, `register`, unused locals, types, nested blocks, and the switches `/Ol` and `/Oe`.
   - *If that stalls:* read the slot allocator in C2.EXE directly.
   - The rule goes into MATCHING.md, with a checker if it can be computed.

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
