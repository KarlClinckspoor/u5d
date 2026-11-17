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
// MSC 5.0 registers every loop (1996 local_c in CX, 1a03 local_14_am in CX, 1a14 local_e_am
// in SI with the bound local_c in DI) only when the function has enough source lines before
// them: every line inside the function that holds a token or a comment counts, also an empty
// `;` or a local extern declaration (blank lines, one multi-line comment, #if 0 blocks, `{ }`
// lines or comment lines before the function do not; nor does where they are within the
// declarations). Without any comment lines in the body: 14 more lines register 1996 (64 bytes
// still differ), about 120 more give 46, about 140 more match. The padding block after the
// declarations stands for the lines the original had; it matches from about 125 of them.
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

    // Padding: these comment lines count as source lines for MSC 5.0 (see above).
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
