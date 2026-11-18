// Resident code, CS:1b38: a module of its own in the original, between assembly modules.

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


u16 D_540c_cursorFrame = 0; // this module's data (DS:540c)

u16 ULTIMA_1b38_PollKeyWithCursor(void)
{
    int key;
    int savedFlag_d;

    savedFlag_d = D_538e_cursorAdvance;
    D_538e_cursorAdvance = 0;

    ULTIMA_16ba_PrintChar(D_5390_cursorFirstChar + D_540c_cursorFrame++); // animate cursor

    key = ULTIMA_1d5e_PollKey();
    if ((int)D_540c_cursorFrame >= (int)D_5392_cursorFrameCount)
    {
        D_540c_cursorFrame = 0;
    }

    if (key != 0)
    {
        ULTIMA_16ba_PrintChar(' ');
    }
    else
    {
        ULTIMA_20fa_WaitTicks(1);
    }

    D_538e_cursorAdvance = savedFlag_d;
    return key;
}
