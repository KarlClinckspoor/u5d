// Resident code, CS:1fa0: a module of its own in the original, between assembly modules.

#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Backspace(int char_count)
{
    int textX;
    int charIndex;
    TextWindow* window_i;
    u16 savedFlag_d;

    if ((GetCurrentTextX() != 0 || GetCurrentTextY() != 0) && char_count != 0)
    {
        savedFlag_d = cursorAdvance;
        window_i = &textWindows[textWindowIndex];
        cursorAdvance = 0;

        for (charIndex = 0; charIndex < char_count; charIndex++)
        {
            textX = GetCurrentTextX();
            ULTIMA_16ba_PrintChar(' ');
            if (textX != 0)
            {
                SetTextPosition(textX - 1, GetCurrentTextY());
            }
            else
            {
                SetTextPosition(window_i->right - window_i->left, GetCurrentTextY() - 1);
            }
        }

        cursorAdvance = savedFlag_d;
    }
}

u16 ToUpperChar(u8 x)
{
    if (x >= 0x61 && x <= 0x7a)
    {
        return x - 0x20;
    }

    return x;
}
