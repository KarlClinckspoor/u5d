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

// CHECKED (NOT MATCHING: register, loop optimization)
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
        do
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
                        // 1996 NOT MATCHING: the original keeps local_c in CX through this loop
                        // (and stores it after); ours decrements [bp-0Ah] in place.
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

                // 19fd (NOT MATCHING: loop optimization)
                if (local_6 != 0)
                {
                    // 1a03
                    for (; local_42_al[local_14_am] == ' '; local_14_am++)
                    {
                    }
                }

                // 1a14 (NOT MATCHING: loop optimization)
                for (; local_14_am <= local_c; local_14_am++)
                {
                    // 1a1c
                    ULTIMA_16ba_PrintChar((byte)local_42_al[local_14_am]);
                }
            }
        } while (local_a == 0);
    }
    return;
}

// CHECKED (NOT MATCHING: optimization)
// print_integer(val, min_len, filler)
void ULTIMA_1a3e_PrintNumber(int param_1, int param_2, int param_3)
{
    int l_2a = 1; // required_length
    int l_2c;     // space
    int l_2e = 0; // is_negative
    int l_30;
    char local_28[40];

    register s16 *si;

    if (param_2 > 39)
    {
        param_2 = 39;
    }

    // 1a5b
    if (param_1 < 0) // is negative?
    {
        l_2e++; // is_negative = true
        param_1 = -param_1;
    }

    // 1a6c
    l_30 = 5;

    si = &D_5402[1];
    do
    {
        // 1a7a
        if (*si <= param_1) // cmp order?
        {
            l_2a++;
        }
        si++;
    } while (si < &D_5402[5]); // 1a7f

    if (l_2e)
    {
        l_2a++;
    }

    // 1a94
    l_2c = param_2 - l_2a;
    if (l_2c <= 0)
    {
        l_2c = 0;
    }
    else
    {
        // 1aa8: NOT MATCHING (loop optimization)
        for (l_30 = 0; l_30 != l_2c; l_30++)
        {
            local_28[l_30] = param_3;
        }
    }

    // 1ac0
    if (l_2e)
    {
        local_28[l_2c++] = '-';
        l_2a--;
    }

    // 1ad3: NOT MATCHING
    do
    {
        l_30 = D_5402[l_2a - 1];
        l_2e = param_1 / l_30;
        local_28[l_2c++] = (char)l_2e + '0';

        param_1 -= l_30 * l_2e;
    } while (--l_2a != 0);

    // 1afb
    local_28[l_2c] = '\0';

    ULTIMA_1850_PrintString(local_28);
}
