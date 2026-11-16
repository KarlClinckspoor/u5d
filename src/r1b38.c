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


u16 D_540c = 0; // this module's data (DS:540c)

u16 ULTIMA_1b38_PollKeyWithCursor(void)
{
    int local_6_an;
    int local_4;

    local_4 = D_538e;
    D_538e = 0;

    ULTIMA_16ba_PrintChar(D_5390 + D_540c++); // animate cursor

    local_6_an = ULTIMA_1d5e_PollKey();
    if ((int)D_540c >= (int)D_5392)
    {
        D_540c = 0;
    }

    if (local_6_an != 0)
    {
        ULTIMA_16ba_PrintChar(' ');
    }
    else
    {
#if defined(TARGET_DOS32)
        // Temporary
        ULTIMA_20c8_SomeDelay(1, 21);
#else
        ULTIMA_20fa_WaitTicks(1);
#endif
    }

    D_538e = local_4;
    return local_6_an;
}
