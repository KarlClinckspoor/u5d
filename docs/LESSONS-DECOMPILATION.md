# Lessons learned: matching u5d byte for byte

u5d is a decompilation of Ultima V that builds for modern systems. The `matching` branch makes
the same source compile and link to the GOG release's bytes, with the original toolchain:
Microsoft C 5.1 and 5.0, MASM 4.0, and a small linker of our own in place of PLINK86. How-to
details are in `docs/MATCHING.md` and the state is in `docs/PLAN.md`. This file records what
the work taught: about the compilers, the binary, and how to run a long matching effort.

## 1. Make the original toolchain the referee, per function

`tools/match.py` compiles a module in DOSBox-X and compares every function with the original,
byte for byte, and checks every fixup against the address its name carries. It gives a
verdict per function (EXACT, SLOTS, FIXUP, SIZE, CODE) instead of a percentage. One module
recompiles in about 5 seconds, so a variant of a function costs almost nothing to try.

**Lesson:** a fast, per-function verdict is what makes trial and error feasible. Count matched
functions, never "percent similar": a function is right or it isn't.

## 2. Two compilers built one program

The original was built with MSC 5.1 and MSC 5.0 side by side; five overlays even contain one
object of each. The data told them apart before the code did:
- MSC 5.1 packs a module's string literals into `_DATA`;
- MSC 5.0 puts them in CONST, word-aligned, and aligns `char` arrays with a pad byte.

Each compiler also has its own stack-slot hash (16 buckets on the sum of a local's name; 5.0
with a word-sum rule). So a local's *name* decides its stack slot, and `tools/slots.py`
renames locals to give the original's slots.

**Lesson:** when most functions match and a group refuses to, question the toolchain before
the source. The data layout is a cheap witness to which compiler built an object.

## 3. Data has an owner, and its order is evidence

The original's data segment (DGROUP) is the modules' data laid end to end in link order.
`vars.c` held all of it; it now lives in the modules that own it, in the order each compiler
emits it (MSC 5.1: literals at once, a definition's variables after the next function). The
linked DGROUP is now identical to the original's `DATA.OVL`.

The order of strings is a record of the original source. ZSTATS' last two swapped strings
showed that ZSTATS_0c5c tested "Thou art not strong enough!" *before* the code that prints
"Ring vanishes!". Reordering the source fixed DGROUP even though the function still differs.

**Lesson:** compare the data, not only the code. Where things sit tells you where they were
defined and in what order the source said them.

## 4. The compiler's choices answer to the source's shape

Most unmatched functions differ not in what they compute but in the form MSC chose. Each of
these was found on one function and then fixed others:
- **A cast compares through AX.** `(uint)x == 'N'` with `int x` in SI gives
  `mov ax,si; cmp ax,4Eh`, where `x == 'N'` gives `cmp si,4Eh`. A scan of every unmatched
  listing for the original's `cmp ax` found four more functions in minutes.
- **If/else or early returns decide which copy of merged code survives.** MSC merges identical
  code tails and keeps one copy. A copy that falls through is kept; an if/else's then-branch
  wins over a plain `break`. NPC_0d00 needed the if/else, CMDS_0EB4 the early returns.
- **A label stops constant propagation;** the original reloads a variable just set to 0.
- **A bare `return;`** in a function that returns a value leaves AX as it is (CMDS_1030);
  `RETURN_AX(value)` in `common.h` keeps the native build correct.
- **Code after a loop can be inside it** (`if (x == 0) continue;` and the rest), which keeps
  the loop's register variables from being stored when it ends.

**Lesson:** read the listings for the *pattern* of the difference, write the smallest source
change that reproduces it, then scan every unmatched function for the same pattern.

## 5. Rule out the obvious causes before inventing a theory

Several hours went to theories that a two-minute check would have ended:
- The original kept the earlier copy of merged code where we kept the later. It looked like a
  different compiler build, until a scan showed the original merging both ways, including in
  functions we already matched. It was the source after all.
- `const` seemed to do nothing, until it turned out that `common.h` defines `const` away in
  the DOS build. A real `const` table does change the code, but it moves the table out of
  `_DATA`, where the original's isn't.
- MSC 5.1 behaves like 5.0 in the cases tried, so "the other compiler" was not the answer.

**Lesson:** before blaming the compiler, check what the build does to the source (macros,
switches) and whether the original shows the "impossible" behaviour elsewhere.

## 6. Write down what was tried: `NOT MATCHING` notes

A function left unmatched gets a `NOT MATCHING` note in the source: what still differs, every
form tried and what it did, and the closest version. `docs/MATCHING.md` ("Working on a
function") gives the format.

This matters because the work outlives any one session. A long conversation is compacted into
a summary, and a new session starts from nothing; both lose the dead ends. A summary line
such as "NPC_032c: block layout, unsolved" drops the four loop forms already tried, and the
next attempt repeats them. A note next to the function survives, and it is read exactly when
it is needed: when someone opens that function again.

**Lesson:** keep findings in files, not in anyone's memory. Compaction then helps instead of
hurting: it clears old listings out of the way while the facts stay in the repository.

## 7. Small habits that saved time

- Write helper scripts to a file and run them; don't paste them through a shell heredoc. Git
  Bash's quoting mangled backslash escapes (`\n`, `\b`, `\t`) in regexes and C strings
  several times, each time silently.
- In Git Bash, `MSYS_NO_PATHCONV=1` stops `/Ox`-style compiler switches being turned into
  paths.
- Restore a file from a copy or from git before trying the next variant, so experiments don't
  pile on top of each other.
- Commit each improvement on its own with the reason in the message, and confirm the native
  build still compiles (`tools/nativecheck.py`) before committing.
