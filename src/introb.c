// The introduction's tables are const in the original: MSC 5.1 then keeps values read from
// them in registers across calls (INTRO_014e). vars.h declares them without const, so its
// declarations are renamed out of the way here.
// TODO: declare them const in vars.h and drop this.
#define D_306c D_306c_vars_h
#define D_30c4 D_30c4_vars_h
#define D_30da D_30da_vars_h
#include "audio/audio.h"
#include "common/common.h"
#include "funcs.h"
#include "grap_drv.h"
#include "savegame.h"
#include "vars.h"
#include "macros.h"
#undef D_306c
#undef D_30c4
#undef D_30da

#include <stdlib.h>

#include "font.h"
#include "intro.h"

#include <string.h>

#include "time/time.h"

// The part of INTRO.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").

u16 D_3656[8] =
{
    0x0002, 0x0005, 0x0006, 0x0008, 0x0009, 0x000b, 0x000c, 0x000d
};

char* D_3666[8] =
{
    _STATIC_TEXT(0x6cf8, "    Name:"),
    _STATIC_TEXT(0x6d02, "  Sex:"),
    _STATIC_TEXT(0x6d0a, "Class:"),
    _STATIC_TEXT(0x6d12, "  Exp:"),
    _STATIC_TEXT(0x6d1a, "Level:"),
    _STATIC_TEXT(0x6d22, "  STR:"),
    _STATIC_TEXT(0x6d2a, "  DEX:"),
    _STATIC_TEXT(0x6d32, "  INT:")
};

// INTRO_1f26's box ends are initialised arrays (MSC 5.0 would put literals in CONST)
char D_3676[4] = "\x7b\x7f\x7f";

char D_367a[3] = "\x7f\x7f";

void INTRO_1e22(int param_1)
{
    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(3, D_3656[param_1]);
    ULTIMA_1850_PrintString(D_3666[param_1]);
    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(3, D_3656[param_1]);
    ULTIMA_1850_PrintString(D_3666[param_1]);
}

static void INTRO_1e62(void)
{
    int local_4;

    ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);

    ULTIMA_1bf2_SetTextPosition(0, 0x13);
    ULTIMA_16ba_PrintChar(0x7b);
    for (local_4 = 0; local_4 < 0x26; local_4++)
    {
        ULTIMA_16ba_PrintChar(0x7f);
    }
    ULTIMA_16ba_PrintChar(0x7c);
    for (local_4 = 0; local_4 < 3; local_4++)
    {
        ULTIMA_16ba_PrintChar(0x7f);
        ULTIMA_1bf2_SetTextPosition(0x27, ULTIMA_1cee_GetCurrentTextY());
        ULTIMA_16ba_PrintChar(0x7f);
    }
    ULTIMA_16ba_PrintChar(0x7d);
    for (local_4 = 0; local_4 < 0x26; local_4++)
    {
        ULTIMA_16ba_PrintChar(0x7f);
    }
    ULTIMA_16ba_PrintChar(0x7e);

    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(7, 0x9f, 0x138, 0x9f);
    ULTIMA_0f90_GRAP_Pen(0x138, 0xb8);
    ULTIMA_0f90_GRAP_Pen(7, 0xb8);
    ULTIMA_0f90_GRAP_Pen(7, 0x9f);
    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
}

static void INTRO_1f26(int param_1)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(param_1, 0, param_1 + 6, 0x8f);
    ULTIMA_0aa6_GRAP_3f_FillRect(param_1 + 0x8f, 0, param_1 + 0x97, 0x89);
    ULTIMA_0aa6_GRAP_3f_FillRect(param_1 + 7, 0x89, param_1 + 0x96, 0x8f);
    ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);
    ULTIMA_1bf2_SetTextPosition(0, 0);
    ULTIMA_1850_PrintString(D_3676);
    ULTIMA_4c2a();
    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
    ULTIMA_1850_PrintString(_TEXT(0x6d3a, " Ultima IV "));
    ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);
    ULTIMA_4cce();
    ULTIMA_1850_PrintString(D_367a);
    ULTIMA_16ba_PrintChar(0x7c);
    ULTIMA_1bf2_SetTextPosition(0, 0x11);
    ULTIMA_16ba_PrintChar(0x7d);
    ULTIMA_1bf2_SetTextPosition(0x12, 0x11);
    ULTIMA_16ba_PrintChar(0x7e);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(param_1 + 0x18, 7, param_1 + 7, 7);
    ULTIMA_0f90_GRAP_Pen(param_1 + 7, 0x88);
    ULTIMA_0f90_GRAP_Pen(param_1 + 0x8f, 0x88);
    ULTIMA_0f90_GRAP_Pen(param_1 + 0x8f, 7);
    ULTIMA_0f90_GRAP_Pen(param_1 + 0x80, 7);
    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
}

void INTRO_2024(void)
{
    INTRO_1e62();
    ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x13, 0x12);
    ULTIMA_1c22_SetTextWindowSize(1, 0x15, 0, 0x27, 0x12);
    ULTIMA_1c22_SetTextWindowSize(2, 3, 0x15, 0x25, 0x15);
    ULTIMA_1b94_SelectTextWindow(0);
    INTRO_1f26(0);
    ULTIMA_1b94_SelectTextWindow(1);
    INTRO_1f26(0xa8);
    ULTIMA_1bf2_SetTextPosition(0xc, 0);
    ULTIMA_16ba_PrintChar(' ');
}

// ASM
// animate "warriors of destiny"
void INTRO_2090_AnimateWD(void)
{
    ULTIMA_20fa_WaitTicks(1);
    DRV_69(0, 0); // ax: undefined, cf: 0
}

// ASM
// show "warriors of destiny" (a26e)
void INTRO_20ae_ShowWD(byte* ptr)
{
    DRV_69(ptr, 1); // ax: a, cf: 1
}
