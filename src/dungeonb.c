#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include <string.h>

#include "blckthrn.h"
#include "combat.h"
#include "dnglook.h"
#include "dungeon.h"
#include "endgame.h"

// The part of DUNGEON.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").
// Where this part starts is not certain: StepSound uses no strings and compiles the same
// with both compilers, so the original boundary may be 1020 or 104c.

void SetUpMonster(int respawn);
void PrintWalkDir(void);
void StepSound(void);

// The tables of one function are static locals in it (all but ShowPlaceName's, which
// translate.c patches). MSC knows that a call cannot change a static local whose address
// is not taken, so it keeps values read from them in registers across calls and hoists
// them out of loops (DrawMonster, 127e, 1786, 1952), as a const table does; they stay
// in _DATA, in source order.

u8 placeNameBase[8] = {0x00, 0x01, 0x02, 0x02, 0x03, 0x07, 0x0a, 0x0b};
u8 placeNameCount[8] = {0x01, 0x01, 0x00, 0x01, 0x04, 0x03, 0x01, 0x00};
u8 placeNameX[12] = {0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x09, 0x09, 0x07, 0x00};
u8 placeNameY[12] = {0x0a, 0x09, 0x0a, 0x0b, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0a, 0x00};
char* dungeonPlaceNames[11] =
{
    /*0x6bb0*/ "BOTTOMLESS\n   PIT    ",
    /*0x6bc6*/ "[E MAZE\n OF LO_\n SOULS ",
    /*0x6bde*/ "[E PRISON\n   WRO]  ",
    /*0x6bf2*/ "[E CRYPT",
    /*0x6bfc*/ "UPPER \nCRYPTS",
    /*0x6c0a*/ "LOWER \nCRYPTS",
    /*0x6c18*/ "DEBTERS\n ALLY  ",
    /*0x6c28*/ "D\\P",
    /*0x6c2c*/ "D\\PER",
    /*0x6c32*/ "D\\PE_",
    /*0x6c38*/ "MO[ER LODE\n   MAZE   ",
};

// CHECKED
// An early return, not if { ... }: MSC then keeps the map id in BX, where the if
// form makes it a register temporary in SI.
// the name of a special place of this dungeon (dungeonPlaceNames) in a small window on the view
static void ShowPlaceName(int number)
{
    int nameIndex;

    if (number > placeNameCount[D_5893_mapId - 0x21])
    {
        return;
    }

    // 1063 (ok)
    nameIndex = placeNameBase[D_5893_mapId - 0x21] + number - 1;
    SetTextWindowSize(0, placeNameX[nameIndex], placeNameY[nameIndex], 0x27, 0x18);
    SelectTextWindow(0);
    SelectCharset(1);
    SetTextPosition(0, 0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(dungeonPlaceNames[nameIndex]);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    SelectCharset(0);
    SelectTextWindow(2);
    SetTextWindowSize(0, 0, 0, 0x27, 0x18);
}

// the cell of this level at (x, y) wrapped to 8x8; bit 3 (visited) cleared below 0x90
static int CellAt(int x, int y)
{
    int cell;

    cell = GetDungeonMap(x & 7, y & 7, D_5895_mapLevel);
    if (cell < 0x90)
    {
        cell &= 0xf7;
    }

    return cell;
}

// CHECKED (static tables)
// the wandering monster at a distance of 1-3: its frames (size, mirroring, animation by its state) as pictures
static void DrawMonster(int dist)
{
    static u8 monsterTurnMirror[4] = {0x01, 0x03, 0x02, 0x03};
    static s16 monsterX[4] = {0x0028, 0x0048, 0x0050, 0x0058};
    static s16 monsterY[2][4] = {0x0056, 0x0056, 0x0060, 0x0062, 0x0010, 0x0028, 0x0046, 0x0055};
    int mirrorB;
    int big_e;
    int flags90;
    int frame_i;
    int mirrorA_h;
    int flags60;

    if (statusIcon == 'T')
    {
        D_5c5a_actors[1].animState = monsterAnimStates[D_5c5a_actors[1].tile];
    }

    // 113a
    big_e = D_5c5a_actors[1].extra == 0xff;
    // 1148
    flags90 = (D_5c5a_actors[1].animState & 0x90);
    flags60 = (D_5c5a_actors[1].animState & 0x60);
    frame_i = D_5c5a_actors[1].animState & 0xf;

    if (statusIcon == 'T')
    {
        
        mirrorA_h = mirrorB = 1;
    }
    else
    {
        mirrorA_h = ULTIMA_2092_RandomRange(0, 100) < 0x32;
        mirrorB = ULTIMA_2092_RandomRange(0, 100) < 0x32;
        switch (flags60)
        {
        case 0x20:
            // 11c2
            mirrorB = mirrorA_h = frame_i = frame_i == 0;
            break;
        case 0x40:
            break;
        case 0x60:
            // 11e6
            frame_i--;
            if (frame_i < 0)
            {
                frame_i = 3;
            }
            mirrorA_h = monsterTurnMirror[frame_i] >> 1;
            mirrorB = monsterTurnMirror[frame_i] & 1;
            break;
        }
    }

    // 11d3
    if (flags90 != 0)
    {
        mirrorB = (flags90 == 0x90) ^ mirrorA_h;
    }

    if (monsterImage == 0)
    {
        // debug message
        ULTIMA_1850_PrintString(/*0x6c4e*/ "Hey!! What's going on here???");
    }
    else
    {
        // 1220
        GRAP_4b_PutImage(monsterImage, mirrorA_h * 3 + dist - 1, monsterX[dist], monsterY[big_e][dist], big_e);
        GRAP_63_PutMirrored(monsterImage, mirrorB * 3 + dist - 1, 0x60, monsterY[big_e][dist], big_e);
        D_5c5a_actors[1].animState = flags60 + flags90 + frame_i;
    }
}

// CHECKED (static tables)
// a field at that distance: random short lines in the field's colour (sleep magenta, poison green, fire red, energy blue)
static void DrawField(int dist, int fieldKind)
{
    static s16 fieldTop[4] = {0x0010, 0x0038, 0x0050, 0x005c};
    static s16 fieldBottom[4] = {0x00a7, 0x0087, 0x006f, 0x0063};
    static s16 fieldCount[4] = {0x012c, 0x0064, 0x0032, 0x000f};
    static s16 fieldWidth[4] = {0x0007, 0x0007, 0x0005, 0x0002};
    int lineY;
    int lineNo_l;
    int lineX;
    int savedClip_d;

    savedClip_d = D_52c4_clipEnabled;
    D_52c4_clipEnabled = 0;

    switch (fieldKind)
    {
    case 0:
        // 12b4
        ULTIMA_0a70_GRAP_2d_SetPenColor(magentaColor + 8);
        break;
    case 1:
        // 1326
        ULTIMA_0a70_GRAP_2d_SetPenColor(greenColor + 8);
        break;
    case 2:
        // 132c
        ULTIMA_0a70_GRAP_2d_SetPenColor(redColor + 8);
        break;
    case 3:
        // 1332
        ULTIMA_0a70_GRAP_2d_SetPenColor(blueColor + 8);
        break;
    }

    // 12be: the four tables are static locals, so MSC hoists their values out of the loop
    // although it makes calls (temporaries at bp-0c..-14, the count in DI, counted down).
    for (lineNo_l = 0; lineNo_l < fieldCount[dist]; lineNo_l++)
    {
        lineX = ULTIMA_2092_RandomRange(fieldTop[dist], fieldBottom[dist] - fieldWidth[dist]);
        lineY = ULTIMA_2092_RandomRange(fieldTop[dist], fieldBottom[dist]);
        GRAP_39_HorizLine(lineX, lineY, fieldWidth[dist] + lineX);
    }

    D_52c4_clipEnabled = savedClip_d;
}

// one piece of the view (a wall part from DNGn.16, or from ITEMS.16 above 0x1f), plain or mirrored, at the place of its distance
static void DrawPiece(int piece, int fromItems, int mirrored)
{
    static s16 D_2e62_pieceX[2][4] = {0x0010, 0x0028, 0x0048, 0x0058, 0x0098, 0x0078, 0x0068, 0x0060};
    static s16 itemPieceX[4] = {0x0038, 0x0048, 0x0050, 0x0058};
    static s16 itemPieceY[2][4] = {0x0098, 0x0078, 0x0068, 0x0060, 0x000f, 0x0027, 0x0047, 0x0057};
    int pieceY_p;
    int pieceX;
    int column;
    int frame;

    if (piece < 0x1f)
    {
        column = (piece & 3);
        pieceX = D_2e62_pieceX[mirrored][column];
        if (mirrored != 0 && (((byte)piece & 0xf8) == 8 || ((byte)piece & 0xfc) == 0x18))
        {
            pieceX = 0x60;
        }

        pieceY_p = 0xe;
        if (mirrored != 0)
        {
            GRAP_63_PutMirrored(wallImage, piece, pieceX, pieceY_p, 0);
        }
        else
        {
            GRAP_4b_PutImage(wallImage, piece, pieceX, pieceY_p, 0);
        }
    }
    else
    {
        frame = ((piece + 1) >> 1) - 0x10;
        column = ((piece + 1) & 6) >> 1;
        if (mirrored != 0)
        {
            pieceX = 0x60;
        }
        else
        {
            pieceX = itemPieceX[column];
        }

        pieceY_p = (frame < 8) ? (fromItems ? itemPieceY[1][column] : 0x60) : itemPieceY[fromItems][column];

        if (mirrored != 0)
        {
            GRAP_63_PutMirrored(itemsImage, frame, pieceX, pieceY_p, fromItems);
        }
        else
        {
            GRAP_4b_PutImage(itemsImage, frame, pieceX, pieceY_p, fromItems);
        }
    }
}

// the drip of a stalactite in its phases (0-5); phase 5 is the sound; the next phase is returned
static int DrawDrip(int dropX, int dropY, int phase, int dist)
{
    if (phase == 5)
    {
        phase = 0;
        AudioSweepTone(0xc80, 0xdac, 1, -(dist * 8 - 0x14));
    }
    else
    {
        if (phase == 4)
        {
            if (D_52c8_videoDriver == 0 || D_52c8_videoDriver == 3)
            {
                ULTIMA_0a70_GRAP_2d_SetPenColor(3);
            }
            else
            {
                ULTIMA_0a70_GRAP_2d_SetPenColor(0xb);
            }
        }
        else
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(blueColor);
        }

        GRAP_39_HorizLine(dropX - 1, dropY, dropX + 1);
        GRAP_3c_VertLine(dropX, dropY - 1, dropY + 1);

        if (phase < 4)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(blueColor + 8);
            GRAP_30_Pset(dropX, dropY);
        }

        if (phase != 0 || (int)ULTIMA_2092_RandomRange(0, 0x40) < 4)
        {
            phase++;
        }
    }

    return phase;
}

// CHECKED (nop)
// the cell straight ahead at that distance: its pieces, drips, the place's name; 1 when it is open and the view goes on
static int DrawAhead(int x, int y, int dist)
{
    static u8 aheadPieceOffset[6] = {0x0c, 0x08, 0x18, 0x08, 0x0c, 0x0c};
    // NOTE: Original size 10 causes OOB. FMT uses 2 bytes of padding.
    static u8 aheadDripY[10] = {0x28, 0x2f, 0x42, 0x64, 0x92, 0x2e, 0x32, 0x3e, 0x52, 0x6d};
    int phase;
    int cell;
    int piece_j;

    cell = CellAt(x, y);
    if (cell < 0xa0)
    {
        return 1;
    }

    piece_j = aheadPieceOffset[(cell >> 4) - 0xa] + dist;
    if (dist == 0)
    {
        piece_j = 0xc;
    }

    DrawPiece(piece_j, 0, 0);
    DrawPiece(piece_j, 0, 1);

    if ((dist == 1 || dist == 2) && dungeonStyle == 1 && (cell & 0xf0) == 0xc0)
    {
        phase = cell & 7;

        phase = DrawDrip(0x5f, aheadDripY[phase + (dist - 1) * 5] + 0xe, phase, dist);

        GetDungeonMap(x & 7, y & 7, D_5895_mapLevel) &= 0xf8;
        GetDungeonMap(x & 7, y & 7, D_5895_mapLevel) += phase;
    }

    if (dist == 1)
    {
        if ((cell & 0xf0) == 0xb0 && (cell & 0xf) != 0)
        {
            ShowPlaceName(cell & 0xf);
            // NOT MATCHING: nop
        }
        else if (dungeonStyle == 3 && (cell & 0xf0) == 0xc0 && (int)ULTIMA_2092_RandomRange(0, 0x40) < 4)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(redColor + 8);
            GRAP_39_HorizLine(0x5c, 0x57, 0x5d);
            GRAP_39_HorizLine(0x5b, 0x58, 0x5d);
            GRAP_39_HorizLine(0x61, 0x57, 0x62);
            GRAP_39_HorizLine(0x61, 0x58, 99);
        }
    }

    if ((cell & 0xf0) == 0xe0 && dist == 0)
    {
        D_52c4_clipEnabled = 1;

        return 1;
    }

    return 0;
}

// CHECKED (nop)
// a cell beside the corridor (side 0 left, 1 right) at that distance
static void DrawSideCell(int x, int y, int side, int dist)
{
    // NOTE: Original size 10 causes OOB. FMT uses 2 bytes of padding.
    static u8 sideDripY[10] = {0x0e, 0x17, 0x32, 0x62, 0x9f, 0x28, 0x2d, 0x3c, 0x54, 0x77};
    int phase;
    int cell;
    int dropX_c;
    int piece_j;

    cell = CellAt(x, y);
    if (cell >= 0xa0)
    {
        switch (cell & 0xf0)
        {
        case 0xc0:
            // 16c4
            piece_j = dist + 0x14;
            DrawPiece(piece_j, 0, side);
            if (dist >= 2)
            {
                break;
            }

            if (dungeonStyle != 1)
            {
                break;
            }

            // 16ea
            phase = cell & 7;
            dropX_c = (dist != 0) ? 0x43 : 0x21;

            if (side != 0)
            {
                dropX_c = 0xbe - dropX_c;
            }

            phase = DrawDrip(dropX_c, sideDripY[phase + dist * 5] + 0xe, phase, dist);

            GetDungeonMap(x & 7, y & 7, D_5895_mapLevel) &= 0xf8;
            GetDungeonMap(x & 7, y & 7, D_5895_mapLevel) += phase;
            // NOT MATCHING (nop)
            break;

        case 0xa0:
        case 0xe0:
        case 0xf0:
            // 1766
            DrawPiece(dist + 4, 0, side);
            break;

        default:
            DrawPiece(dist, 0, side);
            break;
        }
    }
    else
    {
        DrawPiece(dist + 0x10, 0, side);
    }
}

// CHECKED
// the dots of a ladder's rungs (or a hole) at that distance, one set per frame
static void DrawWallDots(int dist, int frame)
{
    static Pair dotsDist0[3][8] =
    {
        0x5a, 0x73, 0x52, 0x77, 0x5f, 0x7b, 0x57, 0x7d,
        0x5b, 0x83, 0x50, 0x85, 0x50, 0x85, 0x50, 0x85,
        0x57, 0x73, 0x5e, 0x77, 0x51, 0x7a, 0x5d, 0x7d,
        0x5f, 0x81, 0x55, 0x82, 0x59, 0x87, 0x50, 0x8b,
        0x54, 0x75, 0x5a, 0x75, 0x5a, 0x7c, 0x50, 0x7e,
        0x55, 0x88, 0x59, 0x8a, 0x59, 0x8a, 0x59, 0x8a
    };
    static Pair dotsDist1[3][5] =
    {
        0x16, 0x07, 0x13, 0x0a, 0x0d, 0x0b, 0x17, 0x14,
        0x0d, 0x1b, 0x12, 0x06, 0x11, 0x0f, 0x17, 0x0f,
        0x0c, 0x10, 0x0f, 0x1a, 0x0f, 0x08, 0x16, 0x0b,
        0x10, 0x13, 0x0b, 0x15, 0x12, 0x1c
    };
    static Pair dotsDist2[3][4] =
    {
        0x0b, 0x01, 0x0f, 0x04, 0x09, 0x06, 0x0b, 0x08,
        0x0a, 0x02, 0x0e, 0x02, 0x0c, 0x0a, 0x0c, 0x0a,
        0x0d, 0x01, 0x09, 0x04, 0x09, 0x09, 0x09, 0x09
    };
    static Pair dotsDist3[3] =
    {
        0x06, 0x00, 0x05, 0x01, 0x07, 0x02
    };
    static Pair dotsDist0Extra[3] =
    {
        0x59, 0x91, 0x50, 0x90, 0x55, 0x8f
    };
    int dotNo;

    if (D_52c8_videoDriver == 0 || D_52c8_videoDriver == 3)
    {
        ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    }
    else
    {
        ULTIMA_0a70_GRAP_2d_SetPenColor(blueColor);
    }

    // 17a9
    switch (dist)
    {
    case 0:
        // 17cc
        if (D_52c8_videoDriver != 0 && D_52c8_videoDriver != 3)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(blueColor + 8);
        }

        // 17e4
        for (dotNo = 0; dotNo < 8; dotNo++)
        {
            GRAP_30_Pset(dotsDist0[frame][dotNo].x, dotsDist0[frame][dotNo].y);
            GRAP_30_Pset(0xbe - dotsDist0[frame][dotNo].x, dotsDist0[frame][dotNo].y);
        }

        GRAP_30_Pset(dotsDist0Extra[frame].x, dotsDist0Extra[frame].y);
        GRAP_30_Pset(0xbe - dotsDist0Extra[frame].x, dotsDist0Extra[frame].y);
        break;

    case 1:
        // 185e
        for (dotNo = 0; dotNo < 5; dotNo++)
        {
            GRAP_30_Pset(dotsDist1[frame][dotNo].x + 0x48, dotsDist1[frame][dotNo].y + 0x60);
            GRAP_30_Pset(0x76 - dotsDist1[frame][dotNo].x, dotsDist1[frame][dotNo].y + 0x60);
        }
        break;

    case 2:
        // 18c4
        for (dotNo = 0; dotNo < 4; dotNo++)
        {
            GRAP_30_Pset(dotsDist2[frame][dotNo].x + 0x50, dotsDist2[frame][dotNo].y + 0x60);
            GRAP_30_Pset(0x6e - dotsDist2[frame][dotNo].x, dotsDist2[frame][dotNo].y + 0x60);
        }
        break;

    case 3:
        // 1920
        GRAP_30_Pset(dotsDist3[frame].x + 0x58, dotsDist3[frame].y + 0x60);
        GRAP_30_Pset(0x66 - dotsDist3[frame].x, dotsDist3[frame].y + 0x60);
        break;
    }
}

/// CLEAN ROOM (written from the disassembly). param_1, param_2: map x, y; param_3:
// distance. The original builds each shape number in SI in two steps (first operand
// through AX into SI, second operand added to SI) and has no stack slot for it (frame 4).
// MSC 5.0 computes a common subexpression in AX/CX and moves it, and a pointer-typed one
// directly in SI (mov si,[bp+4]; shl si,1); only a register pointer assigned and then
// incremented gives the original's instructions. A register variable takes a stack slot,
// so the DOS build shares it with the tile's: the tile is local to an "if (1)" body and
// the register variable to the bare block after it (the same device as FindHiddenItem). That
// needs the wall branch written with gotos: its block ends in a jump, so MSC places it
// right after the test, as the then-branch of the plain if/else below.
// The source the original was compiled from is not known; this is a spelling that gives
// its bytes.
// a cell at a distance, from the far end back: wall pieces, ladders, fields, the visited mark, the monster there
static void DrawCell(int cellX, int cellY, int dist)
{
    static u8 mirroredPiece[8] = {0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00};
    static u8 plainPiece[8] = {0x00, 0x00, 0x1f, 0x1f, 0x37, 0x27, 0x2f, 0x3f};
    static u8 dotFrame = 0;
    int kind_j;

    if (1)
    {
        int cell_b;

        cell_b = CellAt(cellX, cellY);
        kind_j = cell_b >> 4;
        if (kind_j > 0 && kind_j < 8 && (kind_j != 6 || (cell_b & 7) == 0))
        {
            goto L_wall;
        }
        if (kind_j == 8)
        {
            DrawField(dist, cell_b & 7);
        }
    }
    {
        register byte* piece;

L_field:
        if (kind_j < 9 && (GetDungeonMap(cellX & 7, cellY & 7, D_5895_mapLevel) & 8) != 0)
        {
            piece = (byte*)(dist * 2);
            piece += plainPiece[6];
            DrawPiece((int)piece, 1, 0);
            DrawPiece((int)piece, 1, 1);
        }

        if (dist != 0 && (u8)(cellX & 7) == D_5c5a_actors[1].x && (u8)(cellY & 7) == D_5c5a_actors[1].y)
        {
            DrawMonster(dist);
        }

        if (dist == 0)
        {
            dotFrame++;
            if (dotFrame > 2)
            {
                dotFrame = 0;
            }
        }
        return;

L_wall:
        if (mirroredPiece[kind_j] != 0)
        {
            piece = (byte*)mirroredPiece[kind_j];
            piece += dist * 2;
            DrawPiece((int)piece, 1, 0);
            DrawPiece((int)piece, 1, 1);
        }

        if (plainPiece[kind_j] != 0)
        {
            piece = (byte*)plainPiece[kind_j];
            piece += dist * 2;
            DrawPiece((int)piece, 0, 0);
            DrawPiece((int)piece, 0, 1);
        }

        if (kind_j == 5)
        {
            DrawWallDots(dist, dotFrame);
        }
        goto L_field;
    }
}

// renders the first-person view into page 1: the cells in front (up to four, in light), then back to front; dark without a torch or light
void DrawView(int fullView)
{
    static s8 sideDx[4] = {0xff, 0x00, 0x01, 0x00};
    static s8 sideDy[4] = {0x00, 0xff, 0x00, 0x01};
    int stepX_f;
    int stepY_f;
    int dist;
    int cellX;
    int cellY;

    lastDisk = 0xff;
    GRAP_0f_SelectPage(1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x60, 0xe, 0xaf, 0xb2);
    SetClipWindow(0x28, 0x2a, 0x98, 0xb7);
    D_52c4_clipEnabled = 0;

    if (lightSpellTurns != 0 || torchTurns != 0)
    {
        cellX = D_5896_mapX;
        cellY = D_5897_mapY;
        stepX_f = facingDx[dungeonFacing];
        stepY_f = facingDy[dungeonFacing];

        for (dist = 0; dist < 4; dist++)
        {
            // 1b0e
            if (DrawAhead(cellX, cellY, dist) != 0)
            {
                if (dist != 0 || (CellAt(D_5896_mapX, D_5897_mapY) != 0xe0 && fullView != 0))
                {
                    // 1b36
                    DrawSideCell((u8)sideDx[dungeonFacing] + cellX, (u8)sideDy[dungeonFacing] + cellY, 0, dist);
                    DrawSideCell(cellX - (u8)sideDx[dungeonFacing], cellY - (u8)sideDy[dungeonFacing], 1, dist);
                }

                cellX += stepX_f;
                cellY += stepY_f;
            }
            else
            {
                break;
            }
        }

        // 1b92
        for (--dist; dist >= 0; dist--)
        {
            cellX -= stepX_f;
            cellY -= stepY_f;
            DrawCell(cellX, cellY, dist);
        }
    }
    else
    {
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    }

    GRAP_0f_SelectPage(0);
    D_52c4_clipEnabled = 0;
}

// vitals, the view to the screen, step sound, level and direction
void Redraw(void)
{
    ULTIMA_2900_UpdateVitalsDisplay();
    DrawView(1);
    GRAP_18_TransferArea(1, 0, 0x10, 0xe, 0xaf, 0xb2);
    StepSound();
    PrintWalkDir();
}

// whether the cell under the party on that level is free (the logic is probably not what was meant)
static int CanLandOn(int level, int mustBeEmpty)
{
    int kind;
    int free;

    free = 1;
    kind = GetDungeonMap(D_5896_mapX, D_5897_mapY, level) & 0xf0;
    if (mustBeEmpty != 0 &&
        (kind != 0 || kind == 0xe0 || kind == 0xd0 || kind == 0xc0 || kind == 0xb0)) // NOTE: logic bug?
    {
        free = 0;
    }

    return free;
}

// Klimb one level (-1 up, 1 down): blocked cells fail, from level 0 up or level 7 down it leaves; 1 when it leaves
int Klimb(int dir, int mustBeEmpty)
{
    int leaves;

    leaves = 0;

    if (dir > 0)
    {
        ULTIMA_1850_PrintString(/*0x6c6c*/ "Down!\n");
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x6c74*/ "Up!\n");
    }

    if (dir > 0 && D_5895_mapLevel == 7 || dir < 0 && D_5895_mapLevel == 0)
    {
        leaves = 1;
    }
    else
    {
        if (CanLandOn(dir + (uint)D_5895_mapLevel, mustBeEmpty) != 0)
        {
            D_5895_mapLevel += dir;
            if (dir > 0)
            {
                dungeonMoveDir = 5;
            }
            else
            {
                dungeonMoveDir = 4;
            }

            SetUpMonster(1);
            Redraw();
        }
        else
        {
            ULTIMA_1850_PrintString(/*0x6c7a*/ "Failed!\n");
            AudioSweepTone(800, 2000, 1, 0x32);
        }
    }

    return leaves;
}

// leaves the dungeon to the surface or the underworld, at the dungeon's place on the map
void DUNGEON_1d08_Exit(void)
{
    D_5896_mapX = D_1e8a_placeX[D_5893_mapId - 1];
    D_5897_mapY = D_1eb2_placeY[D_5893_mapId - 1];

    ULTIMA_1850_PrintString(/*0x6c84*/ "\nExit to ");
    if (D_5895_mapLevel != 0)
    {
        D_5895_mapLevel = 0xff;
        ULTIMA_1850_PrintString(/*0x6c8e*/ "Underworld!\n\n");
    }
    else
    {
        D_5895_mapLevel = 0;
        ULTIMA_1850_PrintString(/*0x6c9c*/ "Britannia!\n\n");
    }

    D_5893_mapId = 0;
}

// the Attack command: fights the monster when it is in the cell ahead
int DungeonAttackCmd(void)
{
    int targetY;
    int targetX;
    int result;

    result = 0;

    ULTIMA_1850_PrintString(/*0x6caa*/ "Attack\n");

    targetX = (facingDx[dungeonFacing] + D_5896_mapX) & 7;
    targetY = (facingDy[dungeonFacing] + D_5897_mapY) & 7;
    if (targetX == D_5c5a_actors[1].x && targetY == D_5c5a_actors[1].y)
    {
        combatFlags = 2;
        FreeDungeonImages();
        BuildWallRing();
        SetUpDungeonFight();
        SpecialMapHandler(2, D_5c5a_actors[1].value, 0);

        if (combatExitDir == 5)
        {
            if (D_5895_mapLevel != 0)
            {
                D_5895_mapLevel--;
            }
            else
            {
                DUNGEON_1d08_Exit();
            }
        }
        else if (combatExitDir == 6)
        {
            if (D_5895_mapLevel < 7)
            {
                D_5895_mapLevel++;
            }
            else
            {
                DUNGEON_1d08_Exit();
            }
        }

        LoadDungeonImages(1);

        if (D_5893_mapId != 0)
        {
            SetUpMonster(1);
            Redraw();
        }
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x6cb2*/ "What?\n");
    }

    return result;
}

// the Klimb command: a ladder or the grapple allows up, down or both (then asks U/D)
int DungeonKlimbCmd(void)
{
    int kind_c;
    int visited;
    int canDown_m;
    int canUp;

    canUp = 0;
    canDown_m = 0;
    kind_c = CellAt(D_5896_mapX, D_5897_mapY) & 0xf0;
    visited = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) & 8;
    if (kind_c == 0x10 || kind_c == 0x30 || (visited != 0 && grapple != 0))
    {
        canUp = 1;
    }

    if (kind_c == 0x20 || kind_c == 0x30 || kind_c == 0x60)
    {
        canDown_m = 1;
    }

    if (canUp && canDown_m)
    {
        ULTIMA_1850_PrintString(/*0x6cba*/ "Klimb-U/D-");
        while (canUp && canDown_m)
        {
            switch (ULTIMA_266c_GetChar())
            {
            case U5_KEY_UP:
            case 0x55:
                canDown_m = 0;
                break;

            case U5_KEY_DOWN:
            case 0x44:
                canUp = 0;
                break;

            case U5_KEY_SPACE:
                ULTIMA_1850_PrintString(/*0x6cc6*/ "Pass\n\n");
                canUp = canDown_m = 0;
                break;
            }
        }
    }
    else if (canUp || canDown_m) // 1eee
    {
        // 1efa
        ULTIMA_1850_PrintString(/*0x6cce*/ "Klimb-");
    }
    else if (visited != 0)
    {
        // 1f08
        ULTIMA_1850_PrintString(/*0x6cd6*/ "Klimb-\nWith What?\n");

        return 0;
    }
    else
    {
        // 1f16
        ULTIMA_1850_PrintString(/*0x6cea*/ "Klimb-what?\n");

        return 0;
    }

    // 1f1c
    if ((canUp && Klimb(-1, 0) != 0) || (canDown_m && Klimb(1, 0) != 0))
    {
        DUNGEON_1d08_Exit();
    }

    return 1;
}
