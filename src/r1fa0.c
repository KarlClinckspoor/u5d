// Resident code, CS:1fa0: a module of its own in the original, between assembly modules.

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


void ULTIMA_1fa0_Backspace(int char_count)
{
    int local_a;
    int local_8;
    TextWindow* local_6;
    u16 local_4_ab;

    if ((ULTIMA_1f12_GetCurrentTextX() != 0 || ULTIMA_1cee_GetCurrentTextY() != 0) && char_count != 0)
    {
        local_4_ab = D_538e;
        local_6 = &D_535e_textWindows[D_5386_current_text_window_idx];
        D_538e = 0;

        for (local_8 = 0; local_8 < char_count; local_8++)
        {
            local_a = ULTIMA_1f12_GetCurrentTextX();
            ULTIMA_16ba_PrintChar(' ');
            if (local_a != 0)
            {
                ULTIMA_1bf2_SetTextPosition(local_a - 1, ULTIMA_1cee_GetCurrentTextY());
            }
            else
            {
                ULTIMA_1bf2_SetTextPosition(local_6->right - local_6->left, ULTIMA_1cee_GetCurrentTextY() - 1);
            }
        }

        D_538e = local_4_ab;
    }
}


u16 ULTIMA_2032_ToUpper(u8 x)
{
    if (x >= 0x61 && x <= 0x7a)
    {
        return x - 0x20;
    }

    return x;
}
