# Rules behind u5d's stand-in devices (worker w16)

Small test programs written from scratch, compiled with MSC 5.0 and 5.1 in DOSBox-X, to find the
compiler's rules behind u5d's stand-in devices; then each rule checked on u5d itself. Scripts and
raw outputs are in `research/rules/` (`out_*.txt` are the outputs quoted below). The kit:

- `rk.py`: runs CL on many test files in one DOSBox-X session with a spy in place of C1/C2/C3
  that keeps pass 1's intermediate files (`capture`), then C2/C3 on any number of edited pass-1
  files in one session (`passes`, optionally with N paragraphs left free for C2); reads the
  functions' code out of the objects. About 0.5 s per C2/C3 run.
- `edge.py`: the pool edge of a probe function (below), for many test modules at once.
- `kscan.py`, `kall.py`, `nodev.py`: experiments on u5d's own sources (a copy; src/ untouched).

## 1. What pass 2's memory counts

### Method

A probe function (20 assignments, then two loops) in a module where everything is controlled.
Line records inserted after its entry (as `research/c2lines.py` does) fill the pool; at some
count n, "the edge", the cut reaches the loops and the code changes (n = 349 under 5.1, 457
under 5.0, for the bare probe). Anything added to the module moves the edge by its cost, in
pool units (one unit = one line record). Each table is one run of `edge.py` (coarse step 10,
then step 1); `out_q1_*.txt`.

### Costs (`q1_costs.py`, `q1_more.py`, `q1_infunc.py`, `q1_add.py`; K = 50 of each)

**MSC 5.0 and 5.1 give the same numbers** (`out_q1_costs_50.txt` = `_51.txt` shifted).

| added to the module (before the probe) | cost in units (K = 50) | per item |
|---|---|---|
| `extern int x;` declared, not used (L = 4, 12, 28) | 0, 0, 0 | 0 |
| prototype, not called (L = 4, 28) | 0, 0 | 0 |
| struct tag, typedef, `#define` (200), comment lines outside bodies | 0 | 0 |
| extern referenced in an earlier function (L = 4, 12, 28) | 44, 57, 84 | 0.75 + L/30 |
| function called in an earlier function (L = 4, 28) | 50, 90 | 0.87 + L/30 |
| `static int x;`, `int x;` or `int x = 1;`, never referenced (L = 4, 28) | 43, 83 | 0.73 + L/30 |
| the same statics, also referenced | 44, 84 | (referencing adds nothing) |
| referenced `extern int x[10]` (L = 12) | 64 | 0.89 + L/30 |
| referenced `extern struct S x` (L = 12) | 57 | as an int |
| dead reference `if (0) { x = 0; }` (earlier function or the probe) | as a live reference | 0.75 + L/30 |
| string literal in an earlier function (1, 10 or 60 chars; 50 times the same one) | 45 | 0.9, any length, no merging |
| an earlier function definition (with 50 statements, 50 comment lines) | 1 | its own name only |
| references in a function *after* the probe | 0 | (counted from first appearance on) |
| the probe's own locals: 10 unused (L = 2, 28) | 9, 18 | 0.85 + L/30 |
| the probe's locals/parameter renamed 1 -> 29 characters | 2, 1 | L/30 |
| inside the probe, before the cut: comment-only lines | 50 | 1 |
| inside the probe: 50 dead lines `if (0) { ... }` | 0 | **0** (pass 1 drops their line records) |
| inside the probe: 50 statements `a1 = n;` | 250 | 5 |
| inside the probe: 50 lines under `#if 0` | 0 | 0 |

- **Additive**: three groups that cost 57, 44 and 45 alone cost 146 together (`q1_add.py`).
- **A unit is 30 bytes** (`q1_mem.py`: below about 256 KB free for C2 under 5.1, 400 paragraphs
  less memory move the edge by 214 units: 6400 / 214 = 29.9 bytes). So the per-item costs are
  bytes: **a global symbol ≈ 22 + L bytes** (called function ≈ 26 + L, array ≈ 27 + L), a string
  literal ≈ 27, a local or parameter ≈ 25 + L, a line record 30.

**Formula.** The pool a function's statements get is
`P(compiler, free memory) - sum over the module's global symbols seen so far (defined anywhere
above, or referenced, live or dead, in this or an earlier function) of (22 + L) bytes - 27 per
string literal so far - sum over the function's locals and parameters of (25 + L)`, in 30-byte
units, and each non-blank, not compiled-out line of the body takes one unit, each statement
about four more. Declarations that nothing references cost nothing. P is constant above about
256 KB free (5.1; 249 KB under 5.0, MATCHING.md) and drops 16 bytes per paragraph below.

u5d checks:
- MATCHING.md's "`D_595a`, which cmds.c only declares" broke CMDS_0000: **refuted as stated** -
  CMDS_0000 references it, through `GetDungeonMap` (macros.h), twice. With the rule above,
  declarations alone never count.
- "28 bytes plus the name" (MATCHING.md, from TOWN_0f02) and "about one reference for 30
  characters": confirmed and refined (22 + L; 30 bytes = one unit).
- ULTIMA_1850 needs 123-563 more lines; with K extra defined names of 8 characters at the top of
  r1850.c it matches for **K = 140 to 520** (`kscan.py r1850.c`, `out_k_r1850.txt`; 120 and 540
  not): one 8-character symbol = 1.0 unit, as the table says. Confirmed.

### Two different limits, not one

- **ULTIMA_1850 (5.0) is the cut position**: lines, symbols and memory are interchangeable
  there, and the window is wide.
- **TOWN_0f02 and CMDS_0000 (5.1) are not**: line records inserted after the entry of the
  device-free functions (`nodev.py` + `research/c2lines.py`, 0-798 records) never give the
  original: TOWN_0f02 cycles through three other codes with a period of about 365, CMDS_0000 does not
  change at all. They react only to symbol bytes, in narrow windows: loop optimization running
  out of memory, not a cut.

### One explanation for all three devices? No.

A uniform cause (the original's headers defining more names in every module, or less DOS
memory for C2: both take the same bytes from every function of a module) was scanned on the
three modules with the devices removed and K extra defined 8-character names at the top
(`kscan.py`, `out_k_*.txt`):

| module | what matches | what breaks |
|---|---|---|
| r1850.c (5.0) | ULTIMA_1850 for K = 140..520 | ULTIMA_1A3E from about 580 |
| cmds.c (5.1) | CMDS_0000 for K = 70, 80, 120 (static, step 10; 1..59 never); with dead references in CMDS_0000: **K = 64..84 and 118..128** | CMDS_0962 from 160 |
| town.c (5.1) | TOWN_0f02 at K = 261 only (258, 264 not) | **TOWN_0C78 from K = 35**, TOWN_0408 from 255, TOWN_09E6 from 306 |

town.c alone rules it out: whatever the original had before TOWN_0C78's loops may exceed u5d's
by at most about 30 units (1 KB), but TOWN_0f02 needs about 260 units (7.5 KB) more than u5d at
its third loop. Shorter original names cannot help (savings only add up along the file), longer
ones would need about 7,000 more characters in TOWN_0c78, 0dc4, 0e34 and 0f02. Comment lines in
TOWN_0f02 are ruled out above. So the original's town.c had about 7.5 KB of pass-2 memory in
use between TOWN_0c78's loops and TOWN_0f02's poison loop that u5d's has not: in symbols that
is about 260 more defined or referenced names or string literals there (or that many locals in
TOWN_0f02). What they were is not known; a data definition or literal block placed in the
source between those functions would do it. (For 5.1, the memory route is MATCHING.md's
c2mem result again: exact at 15782-15793 paragraphs but TOWN_0408, 0c78 and 1694 differ.)
How sure: high for "no uniform cause"; the three scans are direct.

### Proposals

1. **CMDS_0000: dead references instead of short link names.** cmds.c with all 24 readable names
   (no `#define` list, no `LINK_DEFINES` aliases, no stub aliases) and **74 dead references**
   `if (0) { Q0000000 = 0; ... }` (8-character `extern int` names, one line, after the
   declarations of CMDS_0000) gives CMDS_0000 EXACT and every other cmds.c function EXACT; the
   window is 64..84 (`out_k_cmds_ref.txt`), so names in cmds.c can grow or shrink by about
   300 characters before it moves, where the short-name device had no room (6 characters broke
   it). The fill names must stay undefined (as TOWN's `F_xxxx`; the linker sees EXTDEFs without
   fixups). Not run through the gate here.
2. **TOWN_0f02: keep the device**; the dead lines cost nothing, only the 245 symbols (≈7.4 KB),
   so the fill may be written on fewer lines. Moving it to the top of the file breaks TOWN_0C78
   (K >= 35), which is why it must sit after TOWN_0e34 (in TOWN_0f02 itself, as now).
3. **ULTIMA_1850: keep the comment lines** (window 123-563 lines, the widest of the three).

## 2. The order of communal variables (COMDEF records)

### Rule (MSC 5.0 and 5.1 alike)

**C1 numbers every symbol-table entry it creates, in the order it creates them (serial s, from 9
at the top of a file). A communal is filed in slot `s mod 64`. COMDEF records are written slot by
slot, 0 to 63; within a slot by `(sum of the name's characters c & 0xDF) mod 256`, ascending; at
equal sums the later declared comes first.** The spelling of a name matters only between
communals that share a slot (serials 64 apart); otherwise the order is declaration order,
cut at slot 63 and continued from slot 0.

What takes a serial (`q2_entries.py`, both compilers the same): every declared name, of any
kind and scope, referenced or not: a variable, `extern` or not, a prototype (1, plus 1 per
*named* parameter: `void f(int a, int b);` is 3, `void f(int, int);` is 1), a function
definition (2, plus 1 per parameter and local), a string literal (1), a typedef (1), a struct
tag (1 plus 1 per member; an anonymous `typedef struct {...} T` is 2 plus members), an enum
(1 plus 1 per constant). `#define` takes none.

Evidence:
- `q2_comdef.py`, `q2_blocks.py`: 40 or 55 communals come out in declaration order; 56 to 64 as
  "55..63, 0..54" for any names (60 random names: the same rotation as 60 `v000` names); more
  interleave (100: `55..63, 0 64, 1 65, ...`). Other declarations interleaved shift the cut by
  their serials (externs, prototypes, statics, typedefs: 1 each; tags with a member: 2).
- `q2_slots.py`: with `slot = (s + 9) mod 64` no output of 64 to 300 communals ever goes back a
  slot.
- `q2_pairs.py` (two communals per slot, names chosen): same-length names in a slot go by their
  character sum; across lengths by the sum of upper-case codes mod 256; ties: later first.
- `q2_fit.py` / `q2_fit2.py`: 8 modules of 150 random names (letters, digits, `_`, lengths 2-13)
  predicted exactly with the key `sum(c & 0xDF) mod 256` (`& 0x5F` too); no other tried key
  (sum, xor, h*k+c for several k, length, first/last char, the name; several moduli) fits.
- **u5d check, confirmed** (`q2_vars.py`): u5d's own `VARS.OBJ` (70 COMDEF records, vars.h's
  BSS lines are consecutive declarations) is predicted exactly, with the first BSS variable in
  slot 49 (the "runs 64 declarations apart, interleaved" of MATCHING.md).

### Compiler or linker?

Both, one after the other. **PLINK86plus 2.24 (Ultima V's linker) allocates communals in the
order of the COMDEF records, object by object in link order; a name already allocated is
skipped; a communal of 2 bytes or more (a 3-byte char array too) starts on an even address, a
1-byte one anywhere.** Test (`q2_plinkplus.py`, MSC 5.1 objects tagged with the Nantucket COMENT,
the communals' addresses read from address tables in the linked EXE): object A's 70 communals
landed exactly in A's COMDEF order (`a55..a63, a00 a64, a01 a65, ...`, the slot walk above), B's
in B's record order with A's `a04`, `a03` skipped, then C's (declared `c05..c00`) in that order.
So u5link's rule (sizes, alignment, first seen) is PLINK86plus's, and the order within an object
is the compiler's slot walk. (PLINK86 1.28 does not know COMDEF records at all, `q2_plink.py`:
"Warning: unknown record type B0H".)

### Could u5.COMMUNALS be derived? (Done, docs/MATCHING.md "Phase C")

Not from the names: the names decide only collisions inside a slot. It is decided by how many
symbol-table entries precede each communal in the object that declares it first. For the
original that is the declaration order of the shared header (all 33 shared communals came from
one object; in address order means: consecutive or rising serials within one turn of 64 slots,
or an arrangement whose slot walk gives address order) and of each module's own communals. What
it would take in u5d:
1. u5link allocating communals in COMDEF-record order as met in link order (PLINK86's rule)
   instead of the list;
2. the 37 module-only communals declared (tentative definitions) in their own modules (intro0,
   intro2, endgame, talk, cast2, shoppes2, shoppe2b, font), the five unused ones included;
3. the 33 shared ones declared without `extern` in the header part that the first-linked C object
   sees, as consecutive declarations starting in a slot from 0 to 31 (pad with declarations, e.g.
   unused prototypes, to place the first one), so that the slot walk gives address order; each
   module's own group likewise (a group of fewer than 64 consecutive declarations that does not
   cross slot 63 comes out in declaration order).
Then `u5.COMMUNALS` becomes a check instead of a fact. The order would still be chosen to fit the
original, not derived from it; the gain is only that the list follows from the sources.

Done that way (MATCHING.md, "Phase C"), with no padding: each group is declared right after
common.h (ultima.c's 33 shared ones, then each overlay module's own), and every group falls
within one turn of the 64 slots, so its COMDEFs come out in declaration (address) order. The
command file's `communals` lines are gone; u5link allocates in record order. MSC writes EXTDEFs
in the same walk (100 prototypes, called in any order: the same EXTDEF order), for the names an
object references and the functions it defines.

## 3. C shapes for patterns u5d cannot spell naturally

### (a) A value built in SI in two steps, no stack slot (DUNGEON_1952, SJOG_0514; MSC 5.0)

`q3a_si.py`: 26 spellings in the same small function (frame of two int locals, then the shape,
then two calls with the value). Rules found:
- **Every `register` local takes a stack slot under MSC 5.0 and 5.1**, whatever its type (int,
  unsigned, char, byte*, char*, int*), at function level or in a nested block: frame 6 instead
  of 4 in every case. Only a `register` *parameter* has none (frame 4), but it is loaded into SI
  at entry (`mov si,[bp+4]`), which the originals do not do.
- **The two-step SI build (`mov al,[tbl+bx]; sub ah,ah; mov si,ax; mov ax,[bp+4]; shl ax,1;
  add si,ax`) comes only from two statements into a register variable** (`p = tbl[k]; p +=
  dist * 2;`), any of the register types above. One statement (`p = tbl[k] + dist * 2`, either
  order, pointer arithmetic, `&p[...]`, comma expression) computes `dist*2` first. Without a
  register variable (a CSE written twice, casts, unsigned, `<< 1`, `+ dist + dist`) the value is
  recomputed through AX with `dist*2` held in SI: frame 4, but other code.
- **A slot is given back to a later sibling block only if the earlier block is the body of an
  `if` (then-branch), `for`, `while` or `switch` (also a `case` block)**; not a bare `{ }`, not an
  `else` body, not a `do` body (`q3a_blocks.py`, 11 x 2 shapes, both compilers the same).

So the original must have had a register local sharing a slot, which needs an earlier sibling
block of one of those kinds; u5d's `if (1)` is one legal spelling, and `for (;;) { ... break; }`,
`while (x) {...}` or a real `if (cond)` body are the others. No spelling without a shared slot
exists under these compilers. u5d check: consistent (DUNGEON_1952 and SJOG_0514 use exactly
this); nothing to replace.

### (b) FONT_0000 (MSC 5.0)

The two open points, in isolation (`q3b_font.py`, `q3b_font2.py`, `q3b_cse.py`, `q3b_font0.py`,
`q3b_font0_pool.py`; `v_font0.py` for variants.py on the real file):
- **Operand order of `+` with a loop-hoisted invariant** (0173, `mov ax,[bp-14h]; add ax,si`):
  MSC 5.0 puts the register operand first for any source order, cast or comparison form (19
  conditions x 4 update forms, all "mov ax,REG; add ax,[temp]"). With a *user* variable instead
  of the hoisted temporary the source order is kept: `int w = D_50f7; ... (uint)(w + used + 1)`
  gives exactly the original's `mov ax,[bp-2]; add ax,si; inc ax; cmp ax,di; jae; add si,[bp-4]`
  in the small test. On the real function this does not transfer: a real local costs a slot
  and the frame and every offset move (variants: 664/674, 423 bytes differ, six names, int,
  uint, before loop B or as it was).
- **The region*2 CSE at the end of the outer loop** (025f, original `shl ax,1; mov [bp-1ch],ax;
  mov bx,ax`): in every reduced version it is the original's temp+BX: small tests with one,
  two or three inner loops, with or without the same CSE before the loop, `for`/`while`/an
  increment; and FONT_0000 itself with any one of its three inner loops removed. Only the whole
  function gives u5d's `mov si,ax; shl si,1`. Not the pool cut (line records 0-1300 inserted
  after the entry: the CSE stays in SI at every cut), not the local names' lengths (1 to 23
  characters). So some per-function count (not memory) is exceeded in u5d's version and was not
  in the original: the original had less of something in this function (temporaries, loops'
  candidates, expressions); not identified.
