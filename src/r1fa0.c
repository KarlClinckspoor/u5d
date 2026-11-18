// Resident code, CS:1fa0: a module of its own in the original, between assembly modules.

#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ULTIMA_1fa0_Backspace(int char_count)
{
    int textX;
    int charIndex;
    TextWindow* window_i;
    u16 savedFlag_d;

    if ((ULTIMA_1f12_GetCurrentTextX() != 0 || ULTIMA_1cee_GetCurrentTextY() != 0) && char_count != 0)
    {
        savedFlag_d = D_538e_cursorAdvance;
        window_i = &D_535e_textWindows[D_5386_textWindowIndex];
        D_538e_cursorAdvance = 0;

        for (charIndex = 0; charIndex < char_count; charIndex++)
        {
            textX = ULTIMA_1f12_GetCurrentTextX();
            ULTIMA_16ba_PrintChar(' ');
            if (textX != 0)
            {
                ULTIMA_1bf2_SetTextPosition(textX - 1, ULTIMA_1cee_GetCurrentTextY());
            }
            else
            {
                ULTIMA_1bf2_SetTextPosition(window_i->right - window_i->left, ULTIMA_1cee_GetCurrentTextY() - 1);
            }
        }

        D_538e_cursorAdvance = savedFlag_d;
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
