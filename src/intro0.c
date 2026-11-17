// INTRO.OVL's MSC 5.1 code is three objects: this one (INTRO_0010, INTRO_0050), intro.c
// (INTRO_014e-INTRO_094e) and intro2.c (from INTRO_0986_Main), linked in that order before
// introb.c. Compiled after these two functions, INTRO_014e's inner do-while is not optimized
// (pass 2's capacity, docs/MATCHING.md); as the first function of its own object it is
// exact. These two use no strings and no data of their own, so this object's _DATA is empty
// and intro.c's starts at DS:2f30 as before.
#include "audio/audio.h"
#include "common/common.h"
#include "funcs.h"
#include "grap_drv.h"
#include "savegame.h"
#include "vars.h"
#include "macros.h"

#include <stdlib.h>

#include "font.h"
#include "intro.h"

#include <string.h>

#include "time/time.h"

void INTRO_0010(void)
{
    int local_4;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

    for (local_4 = 0; local_4 < 4; local_4++)
    {
        ULTIMA_0d4c_GRAP_4b_PutImage(D_bb1a, local_4 + 1, 0x10, local_4 * 0x32, 0);
    }
}

// lord british animation
int INTRO_0050_LordBritishAnim(int param_1, int param_2)
{
    int local_e;
    int local_c_yMove;
    int local_a_xMove;
    int local_8;
    byte* local_6;
    int local_4_ab;

#if !defined(MATCHING_BUILD)
    local_6 = D_b21e;
    ASSERT(SCRATCH_SIZE >= 2783); // BRITISH.PTH file size
#define MEM_SIZE SCRATCH_SIZE
#else
    // DOS: uses savegame area
    local_6 = &D_55a6;
#define MEM_SIZE ((byte*)&D_6606 - (byte*)&D_55a6) /*0x1060*/
#endif

    local_e = D_13b0_brightWhiteColor;

    ULTIMA_0a70_GRAP_2d_SetPenColor(local_e);

    local_4_ab = D_5356;
    D_5356 = 0x113;

    for (local_8 = D_bb18; local_8 < MEM_SIZE; local_8++)
    {
        if (local_e != 0)
        {
            ULTIMA_0c64_GRAP_30_Pset(param_1, param_2);
        }

        local_e = D_13b0_brightWhiteColor;

        local_c_yMove = local_6[local_8] & 7;
        if (local_c_yMove > 2)
        {
            local_e = 0;
        }

        if ((local_6[local_8] & 8) != 0)
        {
            local_c_yMove = -local_c_yMove;
        }

        local_a_xMove = (local_6[local_8] >> 4) & 0x7;
        if (local_a_xMove > 2)
        {
            local_e = 0;
        }

        if ((local_6[local_8] & 0x80) != 0)
        {
            local_a_xMove = -local_a_xMove;
        }

        param_1 += local_a_xMove;
        param_2 += local_c_yMove;

        if (ULTIMA_1d5e_PollKey() != 0)
        {
            D_5356 = local_4_ab;
            return 0;
        }

        if ((local_8 & 0x1f) == 0)
        {
            ULTIMA_20fa_WaitTicks(1);
        }

        if (local_6[local_8] == 0)
            break;
    }

    D_bb18 = local_8 + 1;
    D_5356 = local_4_ab;

    return D_bb18;
#undef MEM_SIZE
}
