// Resident code, CS:1b38: a module of its own in the original, between assembly modules.

#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u16 cursorFrame = 0; // this module's data (DS:540c)

u16 PollKeyWithCursor(void)
{
    int key;
    int savedFlag_d;

    savedFlag_d = cursorAdvance;
    cursorAdvance = 0;

    ULTIMA_16ba_PrintChar(cursorFirstChar + cursorFrame++); // animate cursor

    key = PollKey();
    if ((int)cursorFrame >= (int)cursorFrameCount)
    {
        cursorFrame = 0;
    }

    if (key != 0)
    {
        ULTIMA_16ba_PrintChar(' ');
    }
    else
    {
        WaitTicks(1);
    }

    cursorAdvance = savedFlag_d;
    return key;
}
