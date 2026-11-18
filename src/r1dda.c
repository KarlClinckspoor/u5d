// Resident code, CS:1dda: a module of its own in the original, between assembly modules.

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


int ULTIMA_1dda_WaitForKeystroke(int param_1)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_4_ab = -1;
    if (param_1 != 0)
    {
        local_4_ab++;
    }

    if (param_1 > 1)
    {
        local_6 = D_5356_timerCalibration;
        D_5356_timerCalibration = 500;
    }

    do
    {
        local_8 = ULTIMA_1b38_PollKeyWithCursor();
        if (param_1 != 0)
        {
            local_4_ab++;
        }
    } while (local_4_ab < param_1 && local_8 == 0);

    if (param_1 > 1)
    {
        D_5356_timerCalibration = local_6;
    }

    return local_8;
}


void ULTIMA_1e38_IntroGetString(char* param_1, int param_2)
{
    int local_6_an;
    int local_4;

    local_4 = 0;
    do
    {
        local_6_an = ULTIMA_1dda_WaitForKeystroke(0);
        if ((local_6_an == U5_KEY_BACKSPACE || local_6_an == U5_KEY_LEFT) && local_4 != 0)
        {
            ULTIMA_1fa0_Backspace(1);
            local_4--;
        }
        else if (local_6_an == U5_KEY_ESC && local_4 != 0)
        {
            ULTIMA_1fa0_Backspace(local_4);
            local_4 = 0;
        }
        else if (local_6_an > 0x1f && local_6_an < 0x80 && local_4 < param_2)
        {
            param_1[local_4] = local_6_an;
            local_4++;
            ULTIMA_16ba_PrintChar(local_6_an);
        }
    } while (local_6_an != U5_KEY_ENTER);

    param_1[local_4] = 0;
}
