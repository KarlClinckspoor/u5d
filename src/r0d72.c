// Resident code, CS:0d72: a module of its own in the original, between assembly modules.

#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "grap_drv.h"
#include <stdio.h>
#include "macros.h"
int D_5306[7] = {0x0018, 0x0028, 0x0048, 0x0070, 0x0098, 0x00d8, 0x0118};

int D_5314[7] = {0x0003, 0x0007, 0x000b, 0x0014, 0x0020, 0x002d, 0x003d};


int ULTIMA_0d72_AnimateOriginLogo(byte* image)
{
    int iVar1;
    int iStack_4;

    iStack_4 = 0;
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(0, 0, 319, 199);

    for (iVar1 = 0; iVar1 < 7; iVar1++)
    {
        ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(image, iVar1, (320 - D_5306[iVar1]) >> 1, iStack_4);
        iStack_4 += D_5314[iVar1];
    }

    ULTIMA_0c22_GRAP_0f_SelectPage(0);

    return ULTIMA_1140_GRAP_6f_AnimateOriginLogo();
}
