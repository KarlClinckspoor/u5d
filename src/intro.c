// D_306c_pageTextX, D_30c4_pageImageX and D_30da_pageImageY are const tables (see INTRO_014e), and vars.h's declarations
// are renamed out of the way.
#define D_306c_pageTextX D_306c_vars_h
#define D_30c4_pageImageX D_30c4_vars_h
#define D_30da_pageImageY D_30da_vars_h
#include "audio/audio.h"
#include "common/common.h"
#include "funcs.h"
#include "grap_drv.h"
#include "savegame.h"
#include "vars.h"
#include "macros.h"
#undef D_306c_pageTextX
#undef D_30c4_pageImageX
#undef D_30da_pageImageY

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
void INTRO_0010_PutBackdropStrips(void);
int INTRO_0050_LordBritishAnim(int x, int y);

// common.h defines const away in the DOS build; these three tables are really const there
// too. MSC 5.1 still puts them in _DATA, in their place.
#undef const

Pair D_2f98_textLeft[0x15] =
{
    0xb4, 0x00, 0x00, 0xac, 0x00, 0x00, 0x00, 0xd2,
    0x00, 0x00, 0xb0, 0x00, 0x00, 0x00, 0xbc, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xb8, 0x00, 0x00, 0x00,
    0x00, 0x94, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00,
    0x00, 0x9c
};

Pair2 D_2fc2_textRight[0x15] =
{
    0x0140, 0x0140, 0x0140, 0x0140, 0x0084, 0x0140, 0x0140, 0x0140,
    0x0140, 0x0094, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140,
    0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140,
    0x0140, 0x0140, 0x00aa, 0x0140, 0x0140, 0x0140, 0x00aa, 0x0140,
    0x0140, 0x0140, 0x0140, 0x00aa, 0x0140, 0x0140, 0x0140, 0x00aa,
    0x0140, 0x0140
};

u16 D_3016_storyOffset[0x15] =
{
    0x0000, 0x0111, 0x03cb, 0x0590, 0x0803, 0x0ac4, 0x0000, 0x0d6d,
    0x0e23, 0x0fb6, 0x1173, 0x132a, 0x14be, 0x162e, 0x192b, 0x1b9a,
    0x1e7b, 0x2172, 0x244b, 0x2780, 0x2a97
};

u8 D_3040_regionTop[0x16] =
{
    0xb4, 0x46, 0x83, 0x20, 0x46, 0x85, 0xc8, 0xa8,
    0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0x72, 0x72, 0x60,
    0x21, 0x46, 0x60, 0x33, 0x4f, 0x00
};

u8 D_3056_regionBottom[0x16] =
{
    0xc8, 0xc8, 0xc8, 0xa0, 0xc8, 0xc8, 0xc8, 0xc8,
    0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8,
    0x89, 0xc8, 0xc8, 0x92, 0xc8, 0x00
};

const u8 D_306c_pageTextX[0x16] =
{
    0xb4, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x20, 0xbc,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb8, 0x00,
    0x00, 0x00, 0xae, 0x00, 0x00, 0x00
};

u8 D_3082_startY[0x16] =
{
    0x80, 0x00, 0x28, 0x00, 0x09, 0x00, 0x09, 0x88,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00,
    0x00, 0x00, 0x00, 0x09, 0x00, 0x00
};

u8 D_3098_imageIndex[0x16] =
{
    0x00, 0x01, 0x00, 0x01, 0x02, 0x02, 0x02, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x02,
    0x06, 0x04, 0x02, 0x06, 0x04, 0x00
};

u8 D_30ae_imageFile[0x16] =
{
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02,
    0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x00
};

const u8 D_30c4_pageImageX[0x16] =
{
    0x00, 0x00, 0x88, 0x00, 0x98, 0x00, 0x48, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0xb0,
    0x00, 0xb0, 0x00, 0xb0, 0x00, 0x00
};

const u8 D_30da_pageImageY[0x16] =
{
    0x00, 0x4a, 0x00, 0x26, 0x4c, 0x00, 0x26, 0x00,
    0x52, 0x52, 0x52, 0x52, 0x52, 0x00, 0x00, 0x00,
    0x2e, 0x4e, 0x00, 0x37, 0x57, 0x00
};

u8 D_30f0_pageKind[0x15] =
{
    0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04,
    0x05, 0x06, 0x05, 0x06, 0x04
};

// The introduction story ("Ultima V Introduction" in the menu): 21 pages, each a picture from the END/STORY
// picture files with its text from STORY.DAT in the justified font; a key turns the page.
// CHECKED
// Exact with two things: INTRO_0010_PutBackdropStrips/0050 in an object of their own (intro0.c; after them
// the do-while that reloads local_6 is not optimized: pass 2's capacity), and D_30da_pageImageY,
// D_30c4_pageImageX and D_306c_pageTextX const. The original keeps their values in SI/DI across the calls
// without any register variable (frame 0Ch): MSC knows a call cannot change a const table.
// Register variables for them take stack slots (frame 12h), even at function level.
void INTRO_014e_Introduction(void)
{
    byte* fontImage;
    int page_m;
    int loadedFile;
    byte* pageImage;
    byte* bannerImage_h;

    if (D_5893_mapId != 64)
    {
        ULTIMA_102e_UnloadTileset();
    }

    D_5893_mapId = 0x40;
    do
    {
        fontImage = ULTIMA_0fae_LoadResourceFile(D_25ea_fileNames[0]);
    } while (fontImage == 0);

    FONT_0000_DrawJustifiedText(fontImage, _TEXT(0x2f30, ""));
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(0, 10);

    do
    {
        bannerImage_h = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[0x11]);
    } while (bannerImage_h == 0);

    // 01b2
    do
    {
        pageImage = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[0x12]);
    } while (pageImage == 0);

    loadedFile = 0;
    for (page_m = 0; page_m < 0x15; page_m++) // 0384
    {
        // 038a
        if (loadedFile != D_30ae_imageFile[page_m])
        {
            ULTIMA_0be4_FreeImage(pageImage);
            loadedFile = D_30ae_imageFile[page_m];
            do
            {
                pageImage = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[loadedFile + 0x12]);
            } while (pageImage == 0);
        }

        D_a9be_promptMode = 2;
        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        if (D_30f0_pageKind[page_m] == 1)
        {
            switch (page_m)
            {
            case 0:
                ULTIMA_0d4c_GRAP_4b_PutImage(bannerImage_h, 0, 0xe0, 0x1e, 0);
                ULTIMA_0d4c_GRAP_4b_PutImage(bannerImage_h, 1, 0xa8, 0x3a, 0);
                break;
            case 7:
                ULTIMA_0d4c_GRAP_4b_PutImage(bannerImage_h, 0, 0xe8, 0x1a, 0);
                ULTIMA_0d4c_GRAP_4b_PutImage(bannerImage_h, 2, 200, 0x36, 0);
                break;
            case 0xe:
                ULTIMA_0d4c_GRAP_4b_PutImage(bannerImage_h, 0, 0xb8, 0, 0);
                ULTIMA_0d4c_GRAP_4b_PutImage(bannerImage_h, 3, 0xf8, 0, 0);
                break;
            }
        }

        ULTIMA_0d4c_GRAP_4b_PutImage(pageImage, D_3098_imageIndex[page_m], D_30c4_pageImageX[page_m], D_30da_pageImageY[page_m], 0);
        if (D_30f0_pageKind[page_m] >= 4)
        {
            ULTIMA_0d4c_GRAP_4b_PutImage(pageImage, D_30f0_pageKind[page_m] * 2 - 5, D_30c4_pageImageX[page_m], D_30da_pageImageY[page_m] + 0x37, 0);
        }

        D_5146_textLeftX[0] = D_2f98_textLeft[page_m].x;
        D_5146_textLeftX[1] = D_2f98_textLeft[page_m].y;

        D_514c_textRightX[0] = D_2fc2_textRight[page_m].x;
        D_514c_textRightX[1] = D_2fc2_textRight[page_m].y;

        D_5150_regionTop = D_3040_regionTop[page_m];
        D_5152_regionBottom = D_3056_regionBottom[page_m];
        D_5156_penX = D_306c_pageTextX[page_m];
        D_5158_penY = D_3082_startY[page_m];

        if (D_30f0_pageKind[page_m] == 3)
        {
            ULTIMA_0d4c_GRAP_4b_PutImage(pageImage, 3, 0x60, 0x27, 0);
            FONT_0000_DrawJustifiedText(fontImage, _TEXT(0x2f31, "Instantly, a shimmering blue door springs up!"));
            D_5156_penX = D_306c_pageTextX[page_m];
            D_5158_penY = 0xb4;
            FONT_0000_DrawJustifiedText(fontImage, _TEXT(0x2f5f, "With heart beating rapidly, you step into it."));
        }
        else
        {
            ULTIMA_256e_ReadFileFromDisk(/*0x2f8d*/ "STORY.DAT", D_b21e_scratch, 2000, D_3016_storyOffset[page_m]);
            FONT_0000_DrawJustifiedText(fontImage, D_b21e_scratch);
        }

        if (page_m != 0)
        {
            ULTIMA_1b16_ClearKbdBuffer();
            while (ULTIMA_1d5e_PollKey() == 0)
                ;
        }

        ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);

        if (D_30f0_pageKind[page_m] == 2)
        {
            ULTIMA_0c22_GRAP_0f_SelectPage(1);
            ULTIMA_0d4c_GRAP_4b_PutImage(pageImage, 2, 0x28, 0x56, 0);
            ULTIMA_0f46_GRAP_66_Reveal(0x28, 0x56, 0x4b, 0x78);
        }
    }

    ULTIMA_0be4_FreeImage(pageImage);
    ULTIMA_0be4_FreeImage(bannerImage_h);
    ULTIMA_0fdc_FreeBitImage(fontImage);

    ULTIMA_1b16_ClearKbdBuffer();
    while (ULTIMA_1d5e_PollKey() == 0)
        ;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    D_a9be_promptMode = 0;
    ULTIMA_251e_SwitchDisks(0);
}

#if defined(TARGET_DOS16)
#define const
#endif

// Draws a bar at the bottom of the strip and prints the text centred in it between two caps (the title of a
// story scene, the copyright line).
void INTRO_043e_PrintCaptionBar(char* text)
{
    int right;
    int left;

    left = -(ULTIMA_216c_strlen(text) / 2 - 0x12);
    right = left + ULTIMA_216c_strlen(text) + 2;
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 0xc1, left * 8, 199);
    ULTIMA_0aa6_GRAP_3f_FillRect(right * 8, 0xc1, 0x137, 199);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(8, 0xc0, left * 8, 0xc0);
    ULTIMA_0b10_GRAP_Line(right * 8, 0xc0, 0x137, 0xc0);
    ULTIMA_1bf2_SetTextPosition(left, 0x18);
    ULTIMA_4c2a_PrintLeftCap();
    ULTIMA_1850_PrintString(text);
    ULTIMA_4cce_PrintRightCap();
}

// Draws the frame of the main menu area: the box of blue border characters and the white outline.
static void INTRO_04e0_DrawMenuBorders(void)
{
    int i;

    ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);
    ULTIMA_1bf2_SetTextPosition(0, 0xf);
    ULTIMA_16ba_PrintChar(0x7b);

    for (i = 0; i < 0x26; i++)
    {
        ULTIMA_16ba_PrintChar(0x7f);
    }

    ULTIMA_16ba_PrintChar(0x7c);

    for (i = 0; i < 8; i++)
    {
        ULTIMA_16ba_PrintChar(0x7f);
        ULTIMA_1bf2_SetTextPosition(0x27, ULTIMA_1cee_GetCurrentTextY());
        ULTIMA_16ba_PrintChar(0x7f);
    }

    ULTIMA_16ba_PrintChar(0x7d);

    for (i = 0; i < 0x26; i++)
    {
        ULTIMA_16ba_PrintChar(0x7f);
    }

    D_538e_cursorAdvance = 0;
    ULTIMA_16ba_PrintChar(0x7e);
    D_538e_cursorAdvance = 1;
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(7, 127, 312, 127);
    ULTIMA_0f90_GRAP_LineTo(312, 192);
    ULTIMA_0f90_GRAP_LineTo(7, 192);
    ULTIMA_0f90_GRAP_LineTo(7, 127);
    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
}

// The title screen: the ULTIMA picture (revealed with its sound unless withSound is 0, which shows it at
// once), the backdrop strips, "Warriors of Destiny" from WD.BIT and its animation, then the menu frame.
void INTRO_05b0_DisplayTitle(int withSound)
{
    byte* wdImage;
    int local_6_p; // unused

    D_a9be_promptMode = 2;
    if (D_5893_mapId != 0x40)
    {
        ULTIMA_102e_UnloadTileset();
        D_5893_mapId = 0x40;
    }
    do
    {
        D_bb1a_titleImage = ULTIMA_0bae_LoadImageFile(D_25f0[23]); // "ULTIMA.16"
    } while (D_bb1a_titleImage == 0);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0d4c_GRAP_4b_PutImage(D_bb1a_titleImage, 0, 0, 0, 0);
    if (withSound != 0)
    {
#if !defined(TARGET_DOS16)
        // "ULTIMA" with sound
        AUDIO_PlayTitle1Sfx();
#endif
        ULTIMA_0f46_GRAP_66_Reveal(0, 0, 319, 100);
        withSound = ULTIMA_1d5e_PollKey() == 0;
#if !defined(TARGET_DOS16)
        AUDIO_StopSfx();
#endif
    }
    if (withSound == 0)
    {
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0, 0, 319, 100);
    }
    INTRO_0010_PutBackdropStrips();
    ULTIMA_0be4_FreeImage(D_bb1a_titleImage);
    if (withSound != 0)
    {
        do
        {
            wdImage = ULTIMA_0fae_LoadResourceFile(/*0x3105*/ "WD.BIT");
        } while (wdImage == 0);
#if !defined(TARGET_DOS16)
        // "warriors of destiny" with sound
        AUDIO_PlayTitle2Sfx();
#endif
        INTRO_20ae_ShowWD(wdImage);
#if !defined(TARGET_DOS16)
        AUDIO_StopSfx();
#endif
        ULTIMA_0fdc_FreeBitImage(wdImage);
    }
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    INTRO_2090_AnimateWD();
    D_a9be_promptMode = 0;
    INTRO_04e0_DrawMenuBorders();
}

// Prints one menu line at its column and row, inverted when it is the selected one.
static void INTRO_0676_WriteMenuOption(int selected, int row, int col, char* label)
{
    if (row == selected)
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    }

    ULTIMA_1bf2_SetTextPosition(col, row + 0x11);
    ULTIMA_16ba_PrintChar(' ');
    ULTIMA_1850_PrintString(label);
    ULTIMA_16ba_PrintChar(' ');

    if (row == selected)
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    }
}

// Prints the six menu lines with one selected (0 Journey Onward ... 5 Return to the View).
void INTRO_06bc_BuildMainMenu(int selected)
{
    INTRO_0676_WriteMenuOption(selected, 0, 0xc, _TEXT(0x310c, "Journey Onward"));
    INTRO_0676_WriteMenuOption(selected, 1, 9, _TEXT(0x311b, "Create New Character"));
    INTRO_0676_WriteMenuOption(selected, 2, 8, _TEXT(0x3130, "Transfer from Ultima IV"));
    INTRO_0676_WriteMenuOption(selected, 3, 9, _TEXT(0x3148, "Ultima V Introduction"));
    INTRO_0676_WriteMenuOption(selected, 4, 0xb, _TEXT(0x315e, "Acknowledgements"));
    INTRO_0676_WriteMenuOption(selected, 5, 10, _TEXT(0x316f, "Return to the View"));
}

// The acknowledgements: the STARTSC.16 picture scrolls in, waits for a key and scrolls out, then the menu
// frame returns.
void INTRO_072e_Acknowledgements(void)
{
    int pos;
    void* scrollImage;

    do
    {
        scrollImage = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[27]); // "STARTSC.16"
    } while (scrollImage == 0);

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0d4c_GRAP_4b_PutImage(scrollImage, 1, 0x10, 0x3f, 0);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);

    for (pos = 199; pos >= 0x3f; pos--)
    {
        ULTIMA_0d4c_GRAP_4b_PutImage(scrollImage, 0, 0x90, pos, 0); // (image_buffer, image_idx, x, y, ?)
        ULTIMA_0d4c_GRAP_4b_PutImage(scrollImage, 2, 0xa0, pos, 0);

#if defined(ENABLE_SPEED_FIX)
        TIME_SleepMs((199 - pos) / 16);
#endif
    }

    for (pos = 0; pos < 0x90; pos += 8)
    {
        ULTIMA_0d4c_GRAP_4b_PutImage(scrollImage, 0, 0x88 - pos, 0x3f, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x98 - pos, 0x3f, 0x9f - pos, 199);
        ULTIMA_0d4c_GRAP_4b_PutImage(scrollImage, 2, pos + 0xa8, 0x3f, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, pos + 0xa0, 0x3f, pos + 0xa7, 199);
        ULTIMA_20fa_WaitTicks(1);
    }

    do
    {
        D_bb1a_titleImage = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[26]); // "ULTIMA.16"
    } while (D_bb1a_titleImage == 0);

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0d4c_GRAP_4b_PutImage(D_bb1a_titleImage, 1, 0x10, 0x41, 0);
    ULTIMA_0be4_FreeImage(D_bb1a_titleImage);
    INTRO_04e0_DrawMenuBorders();
    INTRO_06bc_BuildMainMenu(4);

    while (ULTIMA_1d5e_PollKey() == 0)
    {
    }

    ULTIMA_0c22_GRAP_0f_SelectPage(0);

    for (pos = 0x88; pos >= 0; pos -= 8)
    {
        ULTIMA_0d4c_GRAP_4b_PutImage(scrollImage, 0, 0x90 - pos, 0x3f, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x88 - pos, 0x3f, 0x8f - pos, 199);
        ULTIMA_0d4c_GRAP_4b_PutImage(scrollImage, 2, pos + 0xa0, 0x3f, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, pos + 0xb0, 0x3f, pos + 0xb7, 199);
        ULTIMA_20fa_WaitTicks(1);
    }

    ULTIMA_0be4_FreeImage(scrollImage);

    for (pos = 0x3f; pos < 199; pos++)
    {
        ULTIMA_0d4c_GRAP_4b_PutImage(scrollImage, 0, 0x90, pos + 1, 0);
        ULTIMA_0d4c_GRAP_4b_PutImage(scrollImage, 2, 0xa0, pos + 1, 0);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x90, pos, 0xaf, pos);

#if defined(ENABLE_SPEED_FIX)
        TIME_SleepMs((199 - pos) / 16);
#endif
    }

    ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x90, 199, 0xaf, 199);
    INTRO_0010_PutBackdropStrips();
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_1b16_ClearKbdBuffer();
}

// Waits that many ticks, or less when a key is pressed; returns the key (0 when none).
int INTRO_094e_Pause(int ticks)
{
    int i;
    int key;

    key = 0;

    for (i = 0; i < ticks; i++)
    {
        if ((key = ULTIMA_1d5e_PollKey()) != 0)
            break;

        ULTIMA_20fa_WaitTicks(1);
    }

    return key;
}
