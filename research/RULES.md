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

QUESTION1

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

### Could u5.COMMUNALS be derived?

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
