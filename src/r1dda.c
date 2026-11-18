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


int ULTIMA_1dda_WaitForKeystroke(int maxPolls)
{
    int key_c;
    int savedCalibration;
    int polls;

    polls = -1;
    if (maxPolls != 0)
    {
        polls++;
    }

    if (maxPolls > 1)
    {
        savedCalibration = D_5356_timerCalibration;
        D_5356_timerCalibration = 500;
    }

    do
    {
        key_c = ULTIMA_1b38_PollKeyWithCursor();
        if (maxPolls != 0)
        {
            polls++;
        }
    } while (polls < maxPolls && key_c == 0);

    if (maxPolls > 1)
    {
        D_5356_timerCalibration = savedCalibration;
    }

    return key_c;
}


void ULTIMA_1e38_IntroGetString(char* buffer, int maxLength)
{
    int key;
    int length;

    length = 0;
    do
    {
        key = ULTIMA_1dda_WaitForKeystroke(0);
        if ((key == U5_KEY_BACKSPACE || key == U5_KEY_LEFT) && length != 0)
        {
            ULTIMA_1fa0_Backspace(1);
            length--;
        }
        else if (key == U5_KEY_ESC && length != 0)
        {
            ULTIMA_1fa0_Backspace(length);
            length = 0;
        }
        else if (key > 0x1f && key < 0x80 && length < maxLength)
        {
            buffer[length] = key;
            length++;
            ULTIMA_16ba_PrintChar(key);
        }
    } while (key != U5_KEY_ENTER);

    buffer[length] = 0;
}
