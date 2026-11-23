// INTRO.OVL's MSC 5.1 code is three objects: this one (PutBackdropStrips, INTRO_0050), intro.c
// (INTRO_014e-INTRO_094e) and intro2.c (from INTRO_0986_Main), linked in that order before
// introb.c. Compiled after these two functions, INTRO_014e's inner do-while is not optimized
// (pass 2's capacity, docs/MATCHING.md); as the first function of its own object it is
// exact. These two use no strings and no data of their own, so this object's _DATA is empty
// and intro.c's starts at DS:2f30 as before.
#include "common/common.h"

// this module's communal variables (vars.h, DS:a9bc)
u16 pathPos; // "lord british" index
void* D_bb1a_titleImage; // pointer

#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include <stdlib.h>

#include "font.h"
#include "intro.h"

#include <string.h>

// Clears page 1 and puts the four 50-line pictures 1 to 4 of ULTIMA.16 one under the other at x 0x10
// (probably the backdrop of the menu and story pages).
void PutBackdropStrips(void)
{
    int band;

    GRAP_0f_SelectPage(1);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);

    for (band = 0; band < 4; band++)
    {
        GRAP_4b_PutImage(D_bb1a_titleImage, band + 1, 0x10, band * 0x32, 0);
    }
}

// Draws Lord British by the path in BRITISH.PTH from its current position (pathPos) up to the next
// pen-up, one pen step per byte (3-bit steps in y and x, signs in bits 3 and 7); returns the new position,
// or 0 when a key was pressed.
int LordBritishAnim(int x, int y)
{
    int penColor_n;
    int yMove_p;
    int xMove;
    int pos_l;
    byte* path;
    int savedCalibration;

    // DOS: uses savegame area
    path = &saveStart;
#define MEM_SIZE ((byte*)&saveEnd - (byte*)&saveStart) /*0x1060*/

    penColor_n = brightWhiteColor;

    GRAP_2d_SetPenColor(penColor_n);

    savedCalibration = timerCalibration;
    timerCalibration = 0x113;

    for (pos_l = pathPos; pos_l < MEM_SIZE; pos_l++)
    {
        if (penColor_n != 0)
        {
            GRAP_30_Pset(x, y);
        }

        penColor_n = brightWhiteColor;

        yMove_p = path[pos_l] & 7;
        if (yMove_p > 2)
        {
            penColor_n = 0;
        }

        if ((path[pos_l] & 8) != 0)
        {
            yMove_p = -yMove_p;
        }

        xMove = (path[pos_l] >> 4) & 0x7;
        if (xMove > 2)
        {
            penColor_n = 0;
        }

        if ((path[pos_l] & 0x80) != 0)
        {
            xMove = -xMove;
        }

        x += xMove;
        y += yMove_p;

        if (PollKey() != 0)
        {
            timerCalibration = savedCalibration;
            return 0;
        }

        if ((pos_l & 0x1f) == 0)
        {
            WaitTicks(1);
        }

        if (path[pos_l] == 0)
            break;
    }

    pathPos = pos_l + 1;
    timerCalibration = savedCalibration;

    return pathPos;
#undef MEM_SIZE
}
