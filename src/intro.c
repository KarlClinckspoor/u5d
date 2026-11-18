// D_306c, D_30c4 and D_30da are const tables (see INTRO_014e), and vars.h's declarations
// are renamed out of the way.
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

// defined in introb.c, the part of INTRO.OVL built with MSC 5.0
void INTRO_2090_AnimateWD(void);

// NOTE: Addresses are shifted by 0x10 (there appears to be a header)

void INTRO_20ae_ShowWD(byte* ptr);

// defined in intro0.c
void INTRO_0010(void);
int INTRO_0050_LordBritishAnim(int param_1, int param_2);

// common.h defines const away in the DOS build; these three tables are really const there
// too. MSC 5.1 still puts them in _DATA, in their place.
#undef const

Pair D_2f98[0x15] =
{
    0xb4, 0x00, 0x00, 0xac, 0x00, 0x00, 0x00, 0xd2,
    0x00, 0x00, 0xb0, 0x00, 0x00, 0x00, 0xbc, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xb8, 0x00, 0x00, 0x00,
    0x00, 0x94, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00,
    0x00, 0x9c
};

Pair2 D_2fc2[0x15] =
{
    0x0140, 0x0140, 0x0140, 0x0140, 0x0084, 0x0140, 0x0140, 0x0140,
    0x0140, 0x0094, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140,
    0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140,
    0x0140, 0x0140, 0x00aa, 0x0140, 0x0140, 0x0140, 0x00aa, 0x0140,
    0x0140, 0x0140, 0x0140, 0x00aa, 0x0140, 0x0140, 0x0140, 0x00aa,
    0x0140, 0x0140
};

u16 D_3016[0x15] =
{
    0x0000, 0x0111, 0x03cb, 0x0590, 0x0803, 0x0ac4, 0x0000, 0x0d6d,
    0x0e23, 0x0fb6, 0x1173, 0x132a, 0x14be, 0x162e, 0x192b, 0x1b9a,
    0x1e7b, 0x2172, 0x244b, 0x2780, 0x2a97
};

u8 D_3040[0x16] =
{
    0xb4, 0x46, 0x83, 0x20, 0x46, 0x85, 0xc8, 0xa8,
    0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0x72, 0x72, 0x60,
    0x21, 0x46, 0x60, 0x33, 0x4f, 0x00
};

u8 D_3056[0x16] =
{
    0xc8, 0xc8, 0xc8, 0xa0, 0xc8, 0xc8, 0xc8, 0xc8,
    0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8,
    0x89, 0xc8, 0xc8, 0x92, 0xc8, 0x00
};

const u8 D_306c[0x16] =
{
    0xb4, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x20, 0xbc,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb8, 0x00,
    0x00, 0x00, 0xae, 0x00, 0x00, 0x00
};

u8 D_3082[0x16] =
{
    0x80, 0x00, 0x28, 0x00, 0x09, 0x00, 0x09, 0x88,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00,
    0x00, 0x00, 0x00, 0x09, 0x00, 0x00
};

u8 D_3098[0x16] =
{
    0x00, 0x01, 0x00, 0x01, 0x02, 0x02, 0x02, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x02,
    0x06, 0x04, 0x02, 0x06, 0x04, 0x00
};

u8 D_30ae[0x16] =
{
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02,
    0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x00
};

const u8 D_30c4[0x16] =
{
    0x00, 0x00, 0x88, 0x00, 0x98, 0x00, 0x48, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0xb0,
    0x00, 0xb0, 0x00, 0xb0, 0x00, 0x00
};

const u8 D_30da[0x16] =
{
    0x00, 0x4a, 0x00, 0x26, 0x4c, 0x00, 0x26, 0x00,
    0x52, 0x52, 0x52, 0x52, 0x52, 0x00, 0x00, 0x00,
    0x2e, 0x4e, 0x00, 0x37, 0x57, 0x00
};

u8 D_30f0[0x15] =
{
    0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04,
    0x05, 0x06, 0x05, 0x06, 0x04
};

// CHECKED
// introduction
// Exact with two things: INTRO_0010/0050 in an object of their own (intro0.c; after them
// the do-while that reloads local_6 is not optimized: pass 2's capacity), and D_30da,
// D_30c4 and D_306c const. The original keeps their values in SI/DI across the calls
// without any register variable (frame 0Ch): MSC knows a call cannot change a const table.
// Register variables for them take stack slots (frame 12h), even at function level.
void INTRO_014e_Introduction(void)
{
    byte* local_c;
    int local_a;
    int local_8;
    byte* local_6;
    byte* local_4_ab;

    if (D_5893_mapId != 64)
    {
        ULTIMA_102e_UnloadTileset();
    }

    D_5893_mapId = 0x40;
    do
    {
        local_c = ULTIMA_0fae_LoadResourceFile(D_25ea_fileNames[0]);
    } while (local_c == 0);

    FONT_0000(local_c, _TEXT(0x2f30, ""));
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(0, 10);

    do
    {
        local_4_ab = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[0x11]);
    } while (local_4_ab == 0);

    // 01b2
    do
    {
        local_6 = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[0x12]);
    } while (local_6 == 0);

    local_8 = 0;
    for (local_a = 0; local_a < 0x15; local_a++) // 0384
    {
        // 038a
        if (local_8 != D_30ae[local_a])
        {
            ULTIMA_0be4_FreeImage(local_6);
            local_8 = D_30ae[local_a];
            do
            {
                local_6 = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[local_8 + 0x12]);
            } while (local_6 == 0);
        }

        D_a9be = 2;
        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        if (D_30f0[local_a] == 1)
        {
            switch (local_a)
            {
            case 0:
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 0, 0xe0, 0x1e, 0);
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 1, 0xa8, 0x3a, 0);
                break;
            case 7:
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 0, 0xe8, 0x1a, 0);
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 2, 200, 0x36, 0);
                break;
            case 0xe:
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 0, 0xb8, 0, 0);
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 3, 0xf8, 0, 0);
                break;
            }
        }

        ULTIMA_0d4c_GRAP_4b_PutImage(local_6, D_3098[local_a], D_30c4[local_a], D_30da[local_a], 0);
        if (D_30f0[local_a] >= 4)
        {
            ULTIMA_0d4c_GRAP_4b_PutImage(local_6, D_30f0[local_a] * 2 - 5, D_30c4[local_a], D_30da[local_a] + 0x37, 0);
        }

        D_5146[0] = D_2f98[local_a].x;
        D_5146[1] = D_2f98[local_a].y;

        D_514c[0] = D_2fc2[local_a].x;
        D_514c[1] = D_2fc2[local_a].y;

        D_5150 = D_3040[local_a];
        D_5152 = D_3056[local_a];
        D_5156 = D_306c[local_a];
        D_5158 = D_3082[local_a];

        if (D_30f0[local_a] == 3)
        {
            ULTIMA_0d4c_GRAP_4b_PutImage(local_6, 3, 0x60, 0x27, 0);
            FONT_0000(local_c, _TEXT(0x2f31, "Instantly, a shimmering blue door springs up!"));
            D_5156 = D_306c[local_a];
            D_5158 = 0xb4;
            FONT_0000(local_c, _TEXT(0x2f5f, "With heart beating rapidly, you step into it."));
        }
        else
        {
            ULTIMA_256e_ReadFileFromDisk(/*0x2f8d*/ "STORY.DAT", D_b21e_scratch, 2000, D_3016[local_a]);
            FONT_0000(local_c, D_b21e_scratch);
        }

        if (local_a != 0)
        {
            ULTIMA_1b16_ClearKbdBuffer();
            while (ULTIMA_1d5e_PollKey() == 0)
                ;
        }

        ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);

        if (D_30f0[local_a] == 2)
        {
            ULTIMA_0c22_GRAP_0f_SelectPage(1);
            ULTIMA_0d4c_GRAP_4b_PutImage(local_6, 2, 0x28, 0x56, 0);
            ULTIMA_0f46_GRAP_66_Reveal(0x28, 0x56, 0x4b, 0x78);
        }
    }

    ULTIMA_0be4_FreeImage(local_6);
    ULTIMA_0be4_FreeImage(local_4_ab);
    ULTIMA_0fdc_FreeBitImage(local_c);

    ULTIMA_1b16_ClearKbdBuffer();
    while (ULTIMA_1d5e_PollKey() == 0)
        ;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    D_a9be = 0;
    ULTIMA_251e_SwitchDisks(0);
}

#if defined(TARGET_DOS16)
#define const
#endif

void INTRO_043e(char* param_1)
{
    int local_6_an;
    int local_4;

    local_4 = -(ULTIMA_216c_strlen(param_1) / 2 - 0x12);
    local_6_an = local_4 + ULTIMA_216c_strlen(param_1) + 2;
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 0xc1, local_4 * 8, 199);
    ULTIMA_0aa6_GRAP_3f_FillRect(local_6_an * 8, 0xc1, 0x137, 199);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(8, 0xc0, local_4 * 8, 0xc0);
    ULTIMA_0b10_GRAP_Line(local_6_an * 8, 0xc0, 0x137, 0xc0);
    ULTIMA_1bf2_SetTextPosition(local_4, 0x18);
    ULTIMA_4c2a_PrintLeftCap();
    ULTIMA_1850_PrintString(param_1);
    ULTIMA_4cce_PrintRightCap();
}

static void INTRO_04e0_DrawMenuBorders(void)
{
    int local_4;

    ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);
    ULTIMA_1bf2_SetTextPosition(0, 0xf);
    ULTIMA_16ba_PrintChar(0x7b);

    for (local_4 = 0; local_4 < 0x26; local_4++)
    {
        ULTIMA_16ba_PrintChar(0x7f);
    }

    ULTIMA_16ba_PrintChar(0x7c);

    for (local_4 = 0; local_4 < 8; local_4++)
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

    D_538e = 0;
    ULTIMA_16ba_PrintChar(0x7e);
    D_538e = 1;
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(7, 127, 312, 127);
    ULTIMA_0f90_GRAP_LineTo(312, 192);
    ULTIMA_0f90_GRAP_LineTo(7, 192);
    ULTIMA_0f90_GRAP_LineTo(7, 127);
    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
}

void INTRO_05b0_DisplayTitle(int param_1) // (0 for fast display)
{
    byte* local_4;
    int local_6_p; // unused

    D_a9be = 2;
    if (D_5893_mapId != 0x40)
    {
        ULTIMA_102e_UnloadTileset();
        D_5893_mapId = 0x40;
    }
    do
    {
        D_bb1a = ULTIMA_0bae_LoadImageFile(D_25f0[23]); // "ULTIMA.16"
    } while (D_bb1a == 0);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0d4c_GRAP_4b_PutImage(D_bb1a, 0, 0, 0, 0);
    if (param_1 != 0)
    {
#if !defined(TARGET_DOS16)
        // "ULTIMA" with sound
        AUDIO_PlayTitle1Sfx();
#endif
        ULTIMA_0f46_GRAP_66_Reveal(0, 0, 319, 100);
        param_1 = ULTIMA_1d5e_PollKey() == 0;
#if !defined(TARGET_DOS16)
        AUDIO_StopSfx();
#endif
    }
    if (param_1 == 0)
    {
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0, 0, 319, 100);
    }
    INTRO_0010();
    ULTIMA_0be4_FreeImage(D_bb1a);
    if (param_1 != 0)
    {
        do
        {
            local_4 = ULTIMA_0fae_LoadResourceFile(/*0x3105*/ "WD.BIT");
        } while (local_4 == 0);
#if !defined(TARGET_DOS16)
        // "warriors of destiny" with sound
        AUDIO_PlayTitle2Sfx();
#endif
        INTRO_20ae_ShowWD(local_4);
#if !defined(TARGET_DOS16)
        AUDIO_StopSfx();
#endif
        ULTIMA_0fdc_FreeBitImage(local_4);
    }
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    INTRO_2090_AnimateWD();
    D_a9be = 0;
    INTRO_04e0_DrawMenuBorders();
}

static void INTRO_0676_WriteMenuOption(int param_4, int param_3, int param_2, char* param_1)
{
    if (param_3 == param_4)
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    }

    ULTIMA_1bf2_SetTextPosition(param_2, param_3 + 0x11);
    ULTIMA_16ba_PrintChar(' ');
    ULTIMA_1850_PrintString(param_1);
    ULTIMA_16ba_PrintChar(' ');

    if (param_3 == param_4)
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    }
}

void INTRO_06bc_BuildMainMenu(int param_1)
{
    INTRO_0676_WriteMenuOption(param_1, 0, 0xc, _TEXT(0x310c, "Journey Onward"));
    INTRO_0676_WriteMenuOption(param_1, 1, 9, _TEXT(0x311b, "Create New Character"));
    INTRO_0676_WriteMenuOption(param_1, 2, 8, _TEXT(0x3130, "Transfer from Ultima IV"));
    INTRO_0676_WriteMenuOption(param_1, 3, 9, _TEXT(0x3148, "Ultima V Introduction"));
    INTRO_0676_WriteMenuOption(param_1, 4, 0xb, _TEXT(0x315e, "Acknowledgements"));
    INTRO_0676_WriteMenuOption(param_1, 5, 10, _TEXT(0x316f, "Return to the View"));
}

void INTRO_072e_Acknowledgements(void)
{
    int local_6_an;
    void* local_4;

    do
    {
        local_4 = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[27]); // "STARTSC.16"
    } while (local_4 == 0);

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0d4c_GRAP_4b_PutImage(local_4, 1, 0x10, 0x3f, 0);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);

    for (local_6_an = 199; local_6_an >= 0x3f; local_6_an--)
    {
        ULTIMA_0d4c_GRAP_4b_PutImage(local_4, 0, 0x90, local_6_an, 0); // (image_buffer, image_idx, x, y, ?)
        ULTIMA_0d4c_GRAP_4b_PutImage(local_4, 2, 0xa0, local_6_an, 0);

#if defined(ENABLE_SPEED_FIX)
        TIME_SleepMs((199 - local_6_an) / 16);
#endif
    }

    for (local_6_an = 0; local_6_an < 0x90; local_6_an += 8)
    {
        ULTIMA_0d4c_GRAP_4b_PutImage(local_4, 0, 0x88 - local_6_an, 0x3f, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x98 - local_6_an, 0x3f, 0x9f - local_6_an, 199);
        ULTIMA_0d4c_GRAP_4b_PutImage(local_4, 2, local_6_an + 0xa8, 0x3f, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, local_6_an + 0xa0, 0x3f, local_6_an + 0xa7, 199);
        ULTIMA_20fa_WaitTicks(1);
    }

    do
    {
        D_bb1a = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[26]); // "ULTIMA.16"
    } while (D_bb1a == 0);

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0d4c_GRAP_4b_PutImage(D_bb1a, 1, 0x10, 0x41, 0);
    ULTIMA_0be4_FreeImage(D_bb1a);
    INTRO_04e0_DrawMenuBorders();
    INTRO_06bc_BuildMainMenu(4);

    while (ULTIMA_1d5e_PollKey() == 0)
    {
    }

    ULTIMA_0c22_GRAP_0f_SelectPage(0);

    for (local_6_an = 0x88; local_6_an >= 0; local_6_an -= 8)
    {
        ULTIMA_0d4c_GRAP_4b_PutImage(local_4, 0, 0x90 - local_6_an, 0x3f, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x88 - local_6_an, 0x3f, 0x8f - local_6_an, 199);
        ULTIMA_0d4c_GRAP_4b_PutImage(local_4, 2, local_6_an + 0xa0, 0x3f, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, local_6_an + 0xb0, 0x3f, local_6_an + 0xb7, 199);
        ULTIMA_20fa_WaitTicks(1);
    }

    ULTIMA_0be4_FreeImage(local_4);

    for (local_6_an = 0x3f; local_6_an < 199; local_6_an++)
    {
        ULTIMA_0d4c_GRAP_4b_PutImage(local_4, 0, 0x90, local_6_an + 1, 0);
        ULTIMA_0d4c_GRAP_4b_PutImage(local_4, 2, 0xa0, local_6_an + 1, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x90, local_6_an, 0xaf, local_6_an);

#if defined(ENABLE_SPEED_FIX)
        TIME_SleepMs((199 - local_6_an) / 16);
#endif
    }

    ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x90, 199, 0xaf, 199);
    INTRO_0010();
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_1b16_ClearKbdBuffer();
}

// pause(wait time) (8b0e)
int INTRO_094e_Pause(int param_1)
{
    int local_6_an;
    int local_4;

    local_4 = 0;

    for (local_6_an = 0; local_6_an < param_1; local_6_an++)
    {
        if ((local_4 = ULTIMA_1d5e_PollKey()) != 0)
            break;

        ULTIMA_20fa_WaitTicks(1);
    }

    return local_4;
}
