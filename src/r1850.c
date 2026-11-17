// Resident code, CS:1850: a module of its own in the original, between assembly modules,
// compiled with MSC 5.0 (its word-wrap loop is 5.0's code; it has no strings to show it).

#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "grap_drv.h"
#include "common/lzw.h"
#include "key/key.h"
#include "time/time.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

s16 D_5402[5] = { 1, 10, 100, 1000, 10000 };

// CHECKED
// The block of comment lines after the declarations is needed for the match. Why:
// MSC 5.0's pass 2 (C2.EXE) does not optimize a function as a whole. It reads the statements
// into a tree pool and, when the pool is full, optimizes and emits what it holds and goes on
// with an empty pool (its output file grows while the function is still being read). A loop
// gets registers only if the pool has room left when it comes. Compiled in one piece this
// function fills about 505 of the pool's 612 units (a unit is about a tree node), and its
// last two loops (1a03: local_14_am in CX; 1a14: local_e_am in SI, the bound in DI) are not
// optimized: 64 bytes differ. The original has them optimized, so there the pool was emptied
// inside the function, before 19ca.
// Pass 1 writes a line record for every source line inside a function that is not blank
// (a comment-only line too; a comment over several lines is one; blank lines, #if'd-out
// lines and lines outside functions give none), and each record takes one unit. So extra
// lines before 19ca move a cut into the function: 0-107 lines change nothing, 108-122
// optimize 1a03 only (46 bytes), 123-563 match, 564-605 give other code, 606-713 are as
// none again, and so on with period 612 (measured by inserting records in pass 1's output
// and running C2 on it). Only the number of records counts: not the line numbers, the text,
// /Zd or /Fc; lines after 19ca, in another function or before this one do nothing.
// Less memory gives the same cut with no extra line: with 236.75 to 248.5 KB of DOS memory
// free for C2.EXE the pool is about 280 units, and this file compiles EXACT without the
// padding (research/c2mem.py r1850.c ULTIMA_1850_PrintString 15152 15920; above that the
// pool has its full size, below it the code is different again). So the original function
// either had 123 or more further lines (comments, lines compiled out), or was compiled with
// about 240 KB left for pass 2. Which, cannot be told from the code; the build cannot limit
// pass 2's memory, so the comment lines stand in for either.
// 0000:1850
void ULTIMA_1850_PrintString(char* param_1)
{
    int local_44_aj;
    char local_42_al[40];
    uint local_1a_ap;
    int local_18_ai;
    int local_16_ak;
    int local_14_am;
    TextWindow* local_12;
    int local_10_aa;
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_aa;

    // Padding: each of these lines is one more unit in pass 2's pool (see above).
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //

    // debug("GRAP_PrintString(%s)", param_1);

    local_8 = 0;
    local_a = 0;
    local_4_aa = 0;
    if (*param_1 != '\0')
    {
        // 1872
        local_12 = &D_535e_textWindows[D_5386_current_text_window_idx];
        local_1a_ap = local_12->flags & 2;
        local_44_aj = (uint)local_12->right - (uint)local_12->left; // text_window_width = r - l;

        // 189a
        while (1)
        {
            if (local_4_aa != 0)
            {
                ULTIMA_16ba_PrintChar('\n');
            }

            // 18a7
            local_c = local_4_aa = local_6 = local_14_am = 0;
            local_10_aa =
                local_44_aj - ULTIMA_1f12_GetCurrentTextX(); // remaining_length = text_window_length - column

            for (; param_1[local_8] != '\n' && param_1[local_8] != '\r' && param_1[local_8] != '\0' &&
                              local_c <= local_10_aa;
                 local_8++, local_c++)
            {
                local_42_al[local_c] = param_1[local_8];
            }

            // 18f4 (after loop)
            if (local_c == 0)
            {
                if (param_1[local_8] != '\0')
                {
                    ULTIMA_16ba_PrintChar((byte)param_1[local_8++]);
                }
                else
                {
                    local_a++;
                }
            }
            else
            {
                // 191c
                if (local_c > local_10_aa)
                {
                    // 1927
                    local_18_ai = local_c;
                    local_16_ak = local_8;

                    // 1939 (MSC 5.0 recomputes param_1 + local_8 for each test; 5.1 would not)
                    for (; param_1[local_8] != ' ' && local_c != 0 && param_1[local_8] != '\n' &&
                           param_1[local_8] != '\0' && param_1[local_8] != '\r';
                         local_c--, local_8--)
                    {
                    }

                    // 1968
                    if (local_c == 0)
                    {
                        local_c = local_18_ai;
                        local_8 = local_16_ak;

                        if (local_12->currentX != 0)
                        {
                            ULTIMA_16ba_PrintChar('\n');
                            local_6 = 1;
                        }
                    }
                    else
                    {
                        // 1996: local_c in CX, stored after the loop
                        do
                        {
                            local_c--;
                        } while (local_42_al[local_c] == ' ' || local_42_al[local_c] == '\r' || local_42_al[local_c] == '\n');

                        if (param_1[local_8] != '\0')
                            local_8++;

                        // 19bf
                        if (local_c < local_10_aa)
                            local_4_aa++;
                    }

                    // 19ca
                    if (local_c > local_10_aa)
                        local_c--;
                }
                else
                {
                    local_c--;
                }

                // 19d5
                if (local_1a_ap != 0)
                {
                    if (local_c > local_44_aj)
                    {
                        ULTIMA_16ba_PrintChar('\n');
                    }

                    // 19ea
                    ULTIMA_1bf2_SetTextPosition((local_10_aa - local_c) / 2, ULTIMA_1cee_GetCurrentTextY());
                }

                // 19fd; 1a03: local_14_am in CX
                if (local_6 != 0)
                {
                    // 1a03
                    for (; local_42_al[local_14_am] == ' '; local_14_am++)
                    {
                    }
                }

                // 1a14: local_e_am in SI, the bound local_c in DI; the counter is stored in
                // local_e_am ([bp-0Ch]) after this loop, so the loop is over local_e_am.
                for (local_e_am = local_14_am; local_e_am <= local_c; local_e_am++)
                {
                    // 1a1c
                    ULTIMA_16ba_PrintChar((byte)local_42_al[local_e_am]);
                }
            }
            if (local_a != 0)
                break;
        }
    }
    return;
}

// CHECKED
// print_integer(val, min_len, filler)
// param_3 is a char: as an int, MSC 5.0 turns the fill loop into "rep stosb" and needs a
// temporary (frame 0x34); the original's loop stores AL each time.
void ULTIMA_1a3e_PrintNumber(int param_1, int param_2, char param_3)
{
    int local_32_am;
    int local_30; // is_negative
    int local_2e; // space
    int local_2c; // required_length
    char local_2a_aa[40];

    local_2c = 1;
    local_30 = 0;

    if (param_2 > 39)
    {
        param_2 = 39;
    }

    // 1a5b
    if (param_1 < 0) // is negative?
    {
        local_30++; // is_negative = true
        param_1 = -param_1;
    }

    // 1a6c: MSC walks D_5402 with a pointer (the extra stack slot) and sets local_32 to 5
    for (local_32_am = 1; local_32_am < 5; local_32_am++)
    {
        if (D_5402[local_32_am] <= param_1)
        {
            local_2c++;
        }
    }

    if (local_30)
    {
        local_2c++;
    }

    // 1a94
    local_2e = param_2 - local_2c;
    if (local_2e <= 0)
    {
        local_2e = 0;
    }
    else
    {
        // 1aa8
        for (local_32_am = 0; local_32_am != local_2e; local_32_am++)
        {
            local_2a_aa[local_32_am] = param_3;
        }
    }

    // 1ac0
    if (local_30)
    {
        local_2a_aa[local_2e++] = '-';
        local_2c--;
    }

    // 1ad3: the quotient assigned inside the digit expression (local_30 read once less)
    // lets MSC keep local_32 in SI and local_2e in DI, with local_30 in memory.
    do
    {
        local_32_am = D_5402[local_2c - 1];
        local_2a_aa[local_2e++] = (char)(local_30 = param_1 / local_32_am) + '0';
        param_1 -= local_32_am * local_30;
    } while (--local_2c != 0);

    // 1afb
    local_2a_aa[local_2e] = '\0';

    ULTIMA_1850_PrintString(local_2a_aa);
}
