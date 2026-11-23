// The introduction's tables are const in the original: MSC 5.1 then keeps values read from
// them in registers across calls (INTRO_014e). vars.h declares them without const, so its
// declarations are renamed out of the way here.
// TODO: declare them const in vars.h and drop this.
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

// The part of INTRO.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").

u16 labelRow[8] =
{
    0x0002, 0x0005, 0x0006, 0x0008, 0x0009, 0x000b, 0x000c, 0x000d
};

char* fieldLabels[8] =
{
    /*0x6cf8*/ "    Name:",
    /*0x6d02*/ "  Sex:",
    /*0x6d0a*/ "Class:",
    /*0x6d12*/ "  Exp:",
    /*0x6d1a*/ "Level:",
    /*0x6d22*/ "  STR:",
    /*0x6d2a*/ "  DEX:",
    /*0x6d32*/ "  INT:"
};

// DrawCharacterBox's box ends are initialised arrays (MSC 5.0 would put literals in CONST)
char boxTopLeft[4] = "\x7b\x7f\x7f";

char boxTopRight[3] = "\x7f\x7f";

// Prints the label of field 0 to 7 ("Name:" to "INT:") at its row in windows 0 and 1 of the transfer screen.
void PrintFieldLabel(int field)
{
    SelectTextWindow(0);
    SetTextPosition(3, labelRow[field]);
    PrintString(fieldLabels[field]);
    SelectTextWindow(1);
    SetTextPosition(3, labelRow[field]);
    PrintString(fieldLabels[field]);
}

// Draws the framed box at the bottom of the transfer screen (rows 19 to 22).
static void DrawPromptBox(void)
{
    int i;

    SetTextForegroundColor(blueColor);

    SetTextPosition(0, 0x13);
    PrintChar(0x7b);
    for (i = 0; i < 0x26; i++)
    {
        PrintChar(0x7f);
    }
    PrintChar(0x7c);
    for (i = 0; i < 3; i++)
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
    PrintChar(0x7e);

    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(7, 0x9f, 0x138, 0x9f);
    GRAP_LineTo(0x138, 0xb8);
    GRAP_LineTo(7, 0xb8);
    GRAP_LineTo(7, 0x9f);
    SetTextForegroundColor(brightWhiteColor);
}

// Draws the framed " Ultima IV " box of one of the transfer screen's two characters at an x offset.
static void DrawCharacterBox(int left)
{
    GRAP_2d_SetPenColor(blueColor);
    GRAP_3f_FillRect(left, 0, left + 6, 0x8f);
    GRAP_3f_FillRect(left + 0x8f, 0, left + 0x97, 0x89);
    GRAP_3f_FillRect(left + 7, 0x89, left + 0x96, 0x8f);
    SetTextForegroundColor(blueColor);
    SetTextPosition(0, 0);
    PrintString(boxTopLeft);
    PrintLeftCap();
    SetTextForegroundColor(brightWhiteColor);
    PrintString(/*0x6d3a*/ " Ultima IV ");
    SetTextForegroundColor(blueColor);
    PrintRightCap();
    PrintString(boxTopRight);
    PrintChar(0x7c);
    SetTextPosition(0, 0x11);
    PrintChar(0x7d);
    SetTextPosition(0x12, 0x11);
    PrintChar(0x7e);
    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(left + 0x18, 7, left + 7, 7);
    GRAP_LineTo(left + 7, 0x88);
    GRAP_LineTo(left + 0x8f, 0x88);
    GRAP_LineTo(left + 0x8f, 7);
    GRAP_LineTo(left + 0x80, 7);
    SetTextForegroundColor(brightWhiteColor);
}

// Sets the transfer screen up: the prompt box, three text windows, and the character boxes at 0 and 0xa8.
void SetUpTransferScreen(void)
{
    DrawPromptBox();
    SetTextWindowSize(0, 0, 0, 0x13, 0x12);
    SetTextWindowSize(1, 0x15, 0, 0x27, 0x12);
    SetTextWindowSize(2, 3, 0x15, 0x25, 0x15);
    SelectTextWindow(0);
    DrawCharacterBox(0);
    SelectTextWindow(1);
    DrawCharacterBox(0xa8);
    SetTextPosition(0xc, 0);
    PrintChar(' ');
}

