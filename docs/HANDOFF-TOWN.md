# Handoff: TOWN_0f02 with the natural names

**Status (2026-11-23): done.** The 96 names town.c uses are renamed in every file; TOWN_0f02
matches with 276 fill references (window 274-277), CMDS_0000 with 85 (window 75-95); the gate
passes for both targets. The measurements are in docs/MATCHING.md, "TOWN_0f02 with the natural
names". The text below is the brief as it was.

## Where things stand

- Worktree `C:\Users\karl9\Downloads\Claude\u5-decompiled\u5d-w24-natural`, branch
  `natural-names`, local only (never pushed). Two commits on `matching` (8c12fe9):
  - 82d293b: `tools/names.tsv` (1,339 names with their addresses; `tools/names.py --check`).
  - a6e3732: 1,124 renames (603 functions, 521 globals), e.g.
    `BLCKTHRN_0000_Footsteps` -> `Footsteps`, `D_57aa_gold` -> `gold`.
- The branch passes the whole gate for both targets: 594/594 and 596/596 EXACT,
  `check.py` "targets gog PASS, 1988 PASS: PASS", `linkscript.py --check` OK,
  `names.py --check` OK.
- **What is missing:** `src/town.c` is the same text as on `main`. The 97 names town.c uses
  (41 distinct resident functions plus globals) keep their prefixes **in every file**, and they
  are the most used names in the program: `ULTIMA_1850_PrintString` (1,325 uses),
  `PrintChar`, `RandomRange`, `SetPenColor`, `GetTileAddr`, ... 215 of the 1,339 names in
  `names.tsv` are still prefixed (67 functions, 148 globals): those 97, names refused because
  a local has the same name, link-time clashes (`Main`, `Exit`, `Delay`, ...), and the 31 bare
  `D_xxxx` globals.
- The scheduled Opus run (Mon 2026-11-23, session "u5d: Opus worker on TOWN_0f02 (natural
  names)") started a worker and went idle after 20 seconds; it left no file or commit in the
  worktree. Treat the problem as untouched.

## The problem

With those 97 names shortened, TOWN_0f02 (in `src/town.c`, MSC 5.1) compiles to **476 bytes**
against the original's **472**. The Haiku worker that made the branch reports that no count
of the dead-reference fill from 10 to 285 changes that (the fill is 245 `F_xxxx` references,
four to a line, in an `if (0)` block near `src/town.c:1243-1260`).

That contradicts what was measured before (docs/MATCHING.md, "What the shared vocabulary
cost" and "What pass 2 counts, measured"):

- MSC pass 2 has a memory limit. A global costs about 22+L bytes (L = name length), a literal
  about 27, a local about 25+L. Lines dropped by `#if` or inside `if (0)` cost 0 in code but
  their names still cost memory.
- TOWN_0f02 sits at such a limit, and the fill was the knob: when the names town.c uses grew by
  373 characters, taking out the last 15 references made it match again (12-15 worked, 11
  and 16 did not), about one reference per 30 characters.
- Shortening names frees memory, so the expectation is that **adding** references brings it
  back: the 41 renames free roughly 41 x 12 = ~500 characters, so about 15-20 more
  references than 245, i.e. around 260-265, which is inside the range the worker says it
  tried.

So either that range was not really tested, or something besides the one memory window
changes when the names get shorter.

## What to check first

1. **Was the fill experiment real?** Reproduce it before believing it. Rename only the 41/97
   town.c names (from `names.tsv`, with `tools/rename.py` as the branch did), then
   `tools/dosbuild.py town.c` and `tools/match.py town.c -v` at fill counts 245, 250, 255, 260,
   265, 270. Confirm each count really changed the object (the fill names are `extern`, never
   defined, so they appear as externals in the OBJ: count them). The worker may have
   recompiled a stale file, edited the wrong block, or renamed something inside the fill.
2. **Look at the difference.** `tools/match.py town.c --dis TOWN_0f02` side by side: which
   4 bytes? A lost loop optimisation or register variable (as in CMDS_0000, where the frame
   goes 3Ch -> 42h) points to the memory limit; a different instruction choice or literal
   order points to something else (symbol order, hash buckets).
3. **Is it town.c's names or other files'?** The renames touch every file that uses those
   names, but only town.c's compile matters for TOWN_0f02. Shorten them in town.c alone
   (keep the old names via a `#define` or by renaming only there) to separate the cases.
4. **Shorter, not fewer.** If the fill cannot compensate, try making the fill names longer
   (`F_xxxx` -> longer names of the same count) to give back exactly the characters taken
   out, then shift by single characters to find the window's edges. That isolates
   "characters of names" from "number of symbols".
5. **Other ideas, not tested:** symbol-table hash buckets or chunk boundaries (the length of a
   name decides where it lands, not just how much it costs); the order of declarations in
   `funcs.h`; whether the fill must sit before a certain point in the file.

Record every measurement in docs/MATCHING.md, as the earlier sections do.

## Done means

- TOWN_0f02 EXACT with the 97 names shortened to their `names.tsv` natural names, and the
  rename applied to every file; or, if it cannot be done, the cause measured and written
  down.
- Gate for both targets before each commit:
  - all functions EXACT (594 gog / 596 1988);
  - `python tools/check.py` -> "targets gog PASS, 1988 PASS: PASS";
  - `tools/asmbuild` for both targets and `python tools/dosbuild.py vars.c`, then
    `python tools/linkscript.py --check`;
  - `python tools/names.py --check` OK.
  A full gate is about 6 minutes; one module compiles in about 1.5 minutes.
- Fix the stale notes on this branch: the town.c note (MATCHING.md around the "Natural names,
  measured" bullet, and the comment at the fill in `src/town.c`) and the cmds.c window note.

## Rules

- Commit with `git -c user.email=30571394+KarlClinckspoor@users.noreply.github.com commit`;
  no other email anywhere. End messages with the worker's own `Co-Authored-By:` line.
- Never push; never touch upstream (wonst719/u5d). The user confirms each push.
- No game files, toolchains or Phoenix/Microsoft binaries in commits.
- The native/Windows build was removed and stays removed. Only the gog and 1988 v1.16 720K
  targets matter.
- CMDS_0000 (cmds.c, 72 dead references) and ULTIMA_1850 (print.c, comment-line padding) are
  also at limits; if a rename moves them, they need the same treatment.
