// pageTextX, pageImageX and pageImageY are const tables (see INTRO_014e), and vars.h's declarations
// are renamed out of the way.
#define pageTextX D_306c_vars_h
#define pageImageX D_30c4_vars_h
#define pageImageY D_30da_vars_h
#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#undef pageTextX
#undef pageImageX
#undef pageImageY

#include <stdlib.h>

#include "font.h"
#include "intro.h"

#include <string.h>

extern void* D_bb1a_titleImage;

// defined in introb.c, the part of INTRO.OVL built with MSC 5.0
void AnimateWD(void);

// NOTE: Addresses are shifted by 0x10 (there appears to be a header)

void ShowWD(byte* ptr);

// defined in intro0.c
void PutBackdropStrips(void);
int LordBritishAnim(int x, int y);

// The three tables are const; MSC 5.1 still puts them in _DATA, in their place.

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

u16 storyOffset[0x15] =
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

const u8 pageTextX[0x16] =
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

const u8 pageImageX[0x16] =
{
    0x00, 0x00, 0x88, 0x00, 0x98, 0x00, 0x48, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0xb0,
    0x00, 0xb0, 0x00, 0xb0, 0x00, 0x00
};

const u8 pageImageY[0x16] =
{
    0x00, 0x4a, 0x00, 0x26, 0x4c, 0x00, 0x26, 0x00,
    0x52, 0x52, 0x52, 0x52, 0x52, 0x00, 0x00, 0x00,
    0x2e, 0x4e, 0x00, 0x37, 0x57, 0x00
};

u8 pageKind[0x15] =
{
    0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04,
    0x05, 0x06, 0x05, 0x06, 0x04
};

// The introduction story ("Ultima V Introduction" in the menu): 21 pages, each a picture from the END/STORY
// picture files with its text from STORY.DAT in the justified font; a key turns the page.
// CHECKED
// Exact with two things: PutBackdropStrips/0050 in an object of their own (intro0.c; after them
// the do-while that reloads local_6 is not optimized: pass 2's capacity), and pageImageY,
// pageImageX and pageTextX const. The original keeps their values in SI/DI across the calls
// without any register variable (frame 0Ch): MSC knows a call cannot change a const table.
// Register variables for them take stack slots (frame 12h), even at function level.
void Introduction(void)
{
    byte* fontImage;
    int page_m;
    int loadedFile;
    byte* pageImage;
    byte* bannerImage_h;

    if (mapId != 64)
    {
        UnloadTileset();
    }

    mapId = 0x40;
    do
    {
        fontImage = LoadResourceFile(fileNames[0]);
    } while (fontImage == 0);

    DrawJustifiedText(fontImage, /*0x2f30*/ "");
    GRAP_0f_SelectPage(1);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    GRAP_0f_SelectPage(0);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    SetTextPosition(0, 10);

    do
    {
        bannerImage_h = LoadImageFile(fileNames[0x11]);
    } while (bannerImage_h == 0);

    // 01b2
    do
    {
        pageImage = LoadImageFile(fileNames[0x12]);
    } while (pageImage == 0);

    loadedFile = 0;
    for (page_m = 0; page_m < 0x15; page_m++) // 0384
    {
        // 038a
        if (loadedFile != D_30ae_imageFile[page_m])
        {
            FreeImage(pageImage);
            loadedFile = D_30ae_imageFile[page_m];
            do
            {
                pageImage = LoadImageFile(fileNames[loadedFile + 0x12]);
            } while (pageImage == 0);
        }

        promptMode = 2;
        GRAP_0f_SelectPage(1);
        PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        if (pageKind[page_m] == 1)
        {
            switch (page_m)
            {
            case 0:
                GRAP_4b_PutImage(bannerImage_h, 0, 0xe0, 0x1e, 0);
                GRAP_4b_PutImage(bannerImage_h, 1, 0xa8, 0x3a, 0);
                break;
            case 7:
                GRAP_4b_PutImage(bannerImage_h, 0, 0xe8, 0x1a, 0);
                GRAP_4b_PutImage(bannerImage_h, 2, 200, 0x36, 0);
                break;
            case 0xe:
                GRAP_4b_PutImage(bannerImage_h, 0, 0xb8, 0, 0);
                GRAP_4b_PutImage(bannerImage_h, 3, 0xf8, 0, 0);
                break;
            }
        }

        GRAP_4b_PutImage(pageImage, D_3098_imageIndex[page_m], pageImageX[page_m], pageImageY[page_m], 0);
        if (pageKind[page_m] >= 4)
        {
            GRAP_4b_PutImage(pageImage, pageKind[page_m] * 2 - 5, pageImageX[page_m], pageImageY[page_m] + 0x37, 0);
        }

        textLeftX[0] = D_2f98_textLeft[page_m].x;
        textLeftX[1] = D_2f98_textLeft[page_m].y;

        textRightX[0] = D_2fc2_textRight[page_m].x;
        textRightX[1] = D_2fc2_textRight[page_m].y;

        D_5150_regionTop = D_3040_regionTop[page_m];
        D_5152_regionBottom = D_3056_regionBottom[page_m];
        D_5156_penX = pageTextX[page_m];
        D_5158_penY = D_3082_startY[page_m];

        if (pageKind[page_m] == 3)
        {
            GRAP_4b_PutImage(pageImage, 3, 0x60, 0x27, 0);
            DrawJustifiedText(fontImage, /*0x2f31*/ "Instantly, a shimmering blue door springs up!");
            D_5156_penX = pageTextX[page_m];
            D_5158_penY = 0xb4;
            DrawJustifiedText(fontImage, /*0x2f5f*/ "With heart beating rapidly, you step into it.");
        }
        else
        {
            ReadFileFromDisk(/*0x2f8d*/ "STORY.DAT", scratch, 2000, storyOffset[page_m]);
            DrawJustifiedText(fontImage, scratch);
        }

        if (page_m != 0)
        {
            ClearKbdBuffer();
            while (PollKey() == 0)
                ;
        }

        GRAP_1b_TransferFullscreen(1, 0);

        if (pageKind[page_m] == 2)
        {
            GRAP_0f_SelectPage(1);
            GRAP_4b_PutImage(pageImage, 2, 0x28, 0x56, 0);
            GRAP_66_Reveal(0x28, 0x56, 0x4b, 0x78);
        }
    }

    FreeImage(pageImage);
    FreeImage(bannerImage_h);
    FreeBitImage(fontImage);

    ClearKbdBuffer();
    while (PollKey() == 0)
        ;

    GRAP_0f_SelectPage(1);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    GRAP_0f_SelectPage(0);
    promptMode = 0;
    SwitchDisks(0);
}

// Draws a bar at the bottom of the strip and prints the text centred in it between two caps (the title of a
// story scene, the copyright line).
void PrintCaptionBar(char* text)
{
    int right;
    int left;

    left = -(StringLength(text) / 2 - 0x12);
    right = left + StringLength(text) + 2;
    GRAP_2d_SetPenColor(blueColor);
    GRAP_3f_FillRect(8, 0xc1, left * 8, 199);
    GRAP_3f_FillRect(right * 8, 0xc1, 0x137, 199);
    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(8, 0xc0, left * 8, 0xc0);
    GRAP_Line(right * 8, 0xc0, 0x137, 0xc0);
    SetTextPosition(left, 0x18);
    PrintLeftCap();
    PrintString(text);
    PrintRightCap();
}

// Draws the frame of the main menu area: the box of blue border characters and the white outline.
static void DrawMenuBorders(void)
{
    int i;

    SetTextForegroundColor(blueColor);
    SetTextPosition(0, 0xf);
    PrintChar(0x7b);

    for (i = 0; i < 0x26; i++)
    {
        PrintChar(0x7f);
    }

    PrintChar(0x7c);

    for (i = 0; i < 8; i++)
    {
        PrintChar(0x7f);
        SetTextPosition(0x27, GetCurrentTextY());
        PrintChar(0x7f);
    }

    PrintChar(0x7d);

    for (i = 0; i < 0x26; i++)
    {
        PrintChar(0x7f);
    }

    cursorAdvance = 0;
    PrintChar(0x7e);
    cursorAdvance = 1;
    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(7, 127, 312, 127);
    GRAP_LineTo(312, 192);
    GRAP_LineTo(7, 192);
    GRAP_LineTo(7, 127);
    SetTextForegroundColor(brightWhiteColor);
}

// The title screen: the ULTIMA picture (revealed with its sound unless withSound is 0, which shows it at
// once), the backdrop strips, "Warriors of Destiny" from WD.BIT and its animation, then the menu frame.
void DisplayTitle(int withSound)
{
    byte* wdImage;
    int local_6_p; // unused

    promptMode = 2;
    if (mapId != 0x40)
    {
        UnloadTileset();
        mapId = 0x40;
    }
    do
    {
        D_bb1a_titleImage = LoadImageFile(tilesFileName[23]); // "ULTIMA.16"
    } while (D_bb1a_titleImage == 0);
    GRAP_0f_SelectPage(0);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    GRAP_0f_SelectPage(1);
    GRAP_4b_PutImage(D_bb1a_titleImage, 0, 0, 0, 0);
    if (withSound != 0)
    {
        GRAP_66_Reveal(0, 0, 319, 100);
        withSound = PollKey() == 0;
    }
    if (withSound == 0)
    {
        GRAP_18_TransferArea(1, 0, 0, 0, 319, 100);
    }
    PutBackdropStrips();
    FreeImage(D_bb1a_titleImage);
    if (withSound != 0)
    {
        do
        {
            wdImage = LoadResourceFile(/*0x3105*/ "WD.BIT");
        } while (wdImage == 0);
        ShowWD(wdImage);
        FreeBitImage(wdImage);
    }
    GRAP_0f_SelectPage(0);
    AnimateWD();
    promptMode = 0;
    DrawMenuBorders();
}

// Prints one menu line at its column and row, inverted when it is the selected one.
static void WriteMenuOption(int selected, int row, int col, char* label)
{
    if (row == selected)
    {
        PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    }

    SetTextPosition(col, row + 0x11);
    PrintChar(' ');
    PrintString(label);
    PrintChar(' ');

    if (row == selected)
    {
        PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    }
}

// Prints the six menu lines with one selected (0 Journey Onward ... 5 Return to the View).
void BuildMainMenu(int selected)
{
    WriteMenuOption(selected, 0, 0xc, /*0x310c*/ "Journey Onward");
    WriteMenuOption(selected, 1, 9, /*0x311b*/ "Create New Character");
    WriteMenuOption(selected, 2, 8, /*0x3130*/ "Transfer from Ultima IV");
    WriteMenuOption(selected, 3, 9, /*0x3148*/ "Ultima V Introduction");
    WriteMenuOption(selected, 4, 0xb, /*0x315e*/ "Acknowledgements");
    WriteMenuOption(selected, 5, 10, /*0x316f*/ "Return to the View");
}

// The acknowledgements: the STARTSC.16 picture scrolls in, waits for a key and scrolls out, then the menu
// frame returns.
void Acknowledgements(void)
{
    int pos;
    void* scrollImage;

    do
    {
        scrollImage = LoadImageFile(fileNames[27]); // "STARTSC.16"
    } while (scrollImage == 0);

    GRAP_0f_SelectPage(1);
    GRAP_4b_PutImage(scrollImage, 1, 0x10, 0x3f, 0);
    GRAP_0f_SelectPage(0);

    for (pos = 199; pos >= 0x3f; pos--)
    {
        GRAP_4b_PutImage(scrollImage, 0, 0x90, pos, 0); // (image_buffer, image_idx, x, y, ?)
        GRAP_4b_PutImage(scrollImage, 2, 0xa0, pos, 0);
    }

    for (pos = 0; pos < 0x90; pos += 8)
    {
        GRAP_4b_PutImage(scrollImage, 0, 0x88 - pos, 0x3f, 0);
        GRAP_18_TransferArea(1, 0, 0x98 - pos, 0x3f, 0x9f - pos, 199);
        GRAP_4b_PutImage(scrollImage, 2, pos + 0xa8, 0x3f, 0);
        GRAP_18_TransferArea(1, 0, pos + 0xa0, 0x3f, pos + 0xa7, 199);
        WaitTicks(1);
    }

    do
    {
        D_bb1a_titleImage = LoadImageFile(fileNames[26]); // "ULTIMA.16"
    } while (D_bb1a_titleImage == 0);

    GRAP_0f_SelectPage(1);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    GRAP_4b_PutImage(D_bb1a_titleImage, 1, 0x10, 0x41, 0);
    FreeImage(D_bb1a_titleImage);
    DrawMenuBorders();
    BuildMainMenu(4);

    while (PollKey() == 0)
    {
    }

    GRAP_0f_SelectPage(0);

    for (pos = 0x88; pos >= 0; pos -= 8)
    {
        GRAP_4b_PutImage(scrollImage, 0, 0x90 - pos, 0x3f, 0);
        GRAP_18_TransferArea(1, 0, 0x88 - pos, 0x3f, 0x8f - pos, 199);
        GRAP_4b_PutImage(scrollImage, 2, pos + 0xa0, 0x3f, 0);
        GRAP_18_TransferArea(1, 0, pos + 0xb0, 0x3f, pos + 0xb7, 199);
        WaitTicks(1);
    }

    FreeImage(scrollImage);

    for (pos = 0x3f; pos < 199; pos++)
    {
        GRAP_4b_PutImage(scrollImage, 0, 0x90, pos + 1, 0);
        GRAP_4b_PutImage(scrollImage, 2, 0xa0, pos + 1, 0);
        GRAP_18_TransferArea(1, 0, 0x90, pos, 0xaf, pos);
    }

    GRAP_18_TransferArea(1, 0, 0x90, 199, 0xaf, 199);
    PutBackdropStrips();
    GRAP_0f_SelectPage(0);
    ClearKbdBuffer();
}

// Waits that many ticks, or less when a key is pressed; returns the key (0 when none).
int Pause(int ticks)
{
    int i;
    int key;

    key = 0;

    for (i = 0; i < ticks; i++)
    {
        if ((key = PollKey()) != 0)
            break;

        WaitTicks(1);
    }

    return key;
}
