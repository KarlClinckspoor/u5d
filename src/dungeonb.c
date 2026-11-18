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
#include "audio/aud_mus.h"

// The part of DUNGEON.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").
// Where this part starts is not certain: DUNGEON_1020_StepSound uses no strings and compiles the same
// with both compilers, so the original boundary may be 1020 or 104c.

void DUNGEON_0134_SetUpMonster(int respawn);
void DUNGEON_01d2_PrintWalkDir(void);
void DUNGEON_1020_StepSound(void);

// The tables of one function are static locals in it (all but DUNGEON_104c_ShowPlaceName's, which
// translate.c patches). MSC knows that a call cannot change a static local whose address
// is not taken, so it keeps values read from them in registers across calls and hoists
// them out of loops (DUNGEON_111e_DrawMonster, 127e, 1786, 1952), as a const table does; they stay
// in _DATA, in source order.

u8 D_2de8_placeNameBase[8] = {0x00, 0x01, 0x02, 0x02, 0x03, 0x07, 0x0a, 0x0b};
u8 D_2df0_placeNameCount[8] = {0x01, 0x01, 0x00, 0x01, 0x04, 0x03, 0x01, 0x00};
u8 D_2df8_placeNameX[12] = {0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x09, 0x09, 0x07, 0x00};
u8 D_2e04_placeNameY[12] = {0x0a, 0x09, 0x0a, 0x0b, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0a, 0x00};
char* D_2e10_dungeonPlaceNames[11] =
{
    _STATIC_TEXT(0x6bb0, "BOTTOMLESS\n   PIT    "),
    _STATIC_TEXT(0x6bc6, "[E MAZE\n OF LO_\n SOULS "),
    _STATIC_TEXT(0x6bde, "[E PRISON\n   WRO]  "),
    _STATIC_TEXT(0x6bf2, "[E CRYPT"),
    _STATIC_TEXT(0x6bfc, "UPPER \nCRYPTS"),
    _STATIC_TEXT(0x6c0a, "LOWER \nCRYPTS"),
    _STATIC_TEXT(0x6c18, "DEBTERS\n ALLY  "),
    _STATIC_TEXT(0x6c28, "D\\P"),
    _STATIC_TEXT(0x6c2c, "D\\PER"),
    _STATIC_TEXT(0x6c32, "D\\PE_"),
    _STATIC_TEXT(0x6c38, "MO[ER LODE\n   MAZE   "),
};

// CHECKED
// An early return, not if { ... }: MSC then keeps the map id in BX, where the if
// form makes it a register temporary in SI.
// the name of a special place of this dungeon (D_2e10_dungeonPlaceNames) in a small window on the view
static void DUNGEON_104c_ShowPlaceName(int number)
{
    int nameIndex;

    if (number > D_2df0_placeNameCount[D_5893_mapId - 0x21])
    {
        return;
    }

    // 1063 (ok)
    nameIndex = D_2de8_placeNameBase[D_5893_mapId - 0x21] + number - 1;
    ULTIMA_1c22_SetTextWindowSize(0, D_2df8_placeNameX[nameIndex], D_2e04_placeNameY[nameIndex], 0x27, 0x18);
    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1c9e_SelectCharset(1);
    ULTIMA_1bf2_SetTextPosition(0, 0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(D_2e10_dungeonPlaceNames[nameIndex]);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1c9e_SelectCharset(0);
    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);
}

// the cell of this level at (x, y) wrapped to 8x8; bit 3 (visited) cleared below 0x90
static int DUNGEON_10dc_CellAt(int x, int y)
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
static void DUNGEON_111e_DrawMonster(int dist)
{
    static u8 D_2e26_monsterTurnMirror[4] = {0x01, 0x03, 0x02, 0x03};
    static s16 D_2e2a_monsterX[4] = {0x0028, 0x0048, 0x0050, 0x0058};
    static s16 D_2e32_monsterY[2][4] = {0x0056, 0x0056, 0x0060, 0x0062, 0x0010, 0x0028, 0x0046, 0x0055};
    int mirrorB;
    int big_e;
    int flags90;
    int frame_i;
    int mirrorA_h;
    int flags60;

    if (D_587a_statusIcon == 'T')
    {
        D_5c5a_actors[1].animState = D_1744_monsterAnimStates[D_5c5a_actors[1].tile];
    }

    // 113a
    big_e = D_5c5a_actors[1].extra == 0xff;
    // 1148
    flags90 = (D_5c5a_actors[1].animState & 0x90);
    flags60 = (D_5c5a_actors[1].animState & 0x60);
    frame_i = D_5c5a_actors[1].animState & 0xf;

    if (D_587a_statusIcon == 'T')
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
            mirrorA_h = D_2e26_monsterTurnMirror[frame_i] >> 1;
            mirrorB = D_2e26_monsterTurnMirror[frame_i] & 1;
            break;
        }
    }

    // 11d3
    if (flags90 != 0)
    {
        mirrorB = (flags90 == 0x90) ^ mirrorA_h;
    }

    if (D_a9c6_monsterImage == 0)
    {
        // debug message
        ULTIMA_1850_PrintString(_TEXT(0x6c4e, "Hey!! What's going on here???"));
    }
    else
    {
        // 1220
        ULTIMA_0d4c_GRAP_4b_PutImage(D_a9c6_monsterImage, mirrorA_h * 3 + dist - 1, D_2e2a_monsterX[dist], D_2e32_monsterY[big_e][dist], big_e);
        ULTIMA_0bfc_GRAP_63_PutMirrored(D_a9c6_monsterImage, mirrorB * 3 + dist - 1, 0x60, D_2e32_monsterY[big_e][dist], big_e);
        D_5c5a_actors[1].animState = flags60 + flags90 + frame_i;
    }
}

// CHECKED (static tables)
// a field at that distance: random short lines in the field's colour (sleep magenta, poison green, fire red, energy blue)
static void DUNGEON_127e_DrawField(int dist, int fieldKind)
{
    static s16 D_2e42_fieldTop[4] = {0x0010, 0x0038, 0x0050, 0x005c};
    static s16 D_2e4a_fieldBottom[4] = {0x00a7, 0x0087, 0x006f, 0x0063};
    static s16 D_2e52_fieldCount[4] = {0x012c, 0x0064, 0x0032, 0x000f};
    static s16 D_2e5a_fieldWidth[4] = {0x0007, 0x0007, 0x0005, 0x0002};
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
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b6_magentaColor + 8);
        break;
    case 1:
        // 1326
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b4_greenColor + 8);
        break;
    case 2:
        // 132c
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor + 8);
        break;
    case 3:
        // 1332
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
        break;
    }

    // 12be: the four tables are static locals, so MSC hoists their values out of the loop
    // although it makes calls (temporaries at bp-0c..-14, the count in DI, counted down).
    for (lineNo_l = 0; lineNo_l < D_2e52_fieldCount[dist]; lineNo_l++)
    {
        lineX = ULTIMA_2092_RandomRange(D_2e42_fieldTop[dist], D_2e4a_fieldBottom[dist] - D_2e5a_fieldWidth[dist]);
        lineY = ULTIMA_2092_RandomRange(D_2e42_fieldTop[dist], D_2e4a_fieldBottom[dist]);
        ULTIMA_0c9c_GRAP_39_HorizLine(lineX, lineY, D_2e5a_fieldWidth[dist] + lineX);
    }

    D_52c4_clipEnabled = savedClip_d;
}

// one piece of the view (a wall part from DNGn.16, or from ITEMS.16 above 0x1f), plain or mirrored, at the place of its distance
static void DUNGEON_134a_DrawPiece(int piece, int fromItems, int mirrored)
{
    static s16 D_2e62_pieceX[2][4] = {0x0010, 0x0028, 0x0048, 0x0058, 0x0098, 0x0078, 0x0068, 0x0060};
    static s16 D_2e72_itemPieceX[4] = {0x0038, 0x0048, 0x0050, 0x0058};
    static s16 D_2e7a_itemPieceY[2][4] = {0x0098, 0x0078, 0x0068, 0x0060, 0x000f, 0x0027, 0x0047, 0x0057};
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
            ULTIMA_0bfc_GRAP_63_PutMirrored(D_a9c0_wallImage, piece, pieceX, pieceY_p, 0);
        }
        else
        {
            ULTIMA_0d4c_GRAP_4b_PutImage(D_a9c0_wallImage, piece, pieceX, pieceY_p, 0);
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
            pieceX = D_2e72_itemPieceX[column];
        }

        pieceY_p = (frame < 8) ? (fromItems ? D_2e7a_itemPieceY[1][column] : 0x60) : D_2e7a_itemPieceY[fromItems][column];

        if (mirrored != 0)
        {
            ULTIMA_0bfc_GRAP_63_PutMirrored(D_a9c4_itemsImage, frame, pieceX, pieceY_p, fromItems);
        }
        else
        {
            ULTIMA_0d4c_GRAP_4b_PutImage(D_a9c4_itemsImage, frame, pieceX, pieceY_p, fromItems);
        }
    }
}

// the drip of a stalactite in its phases (0-5); phase 5 is the sound; the next phase is returned
static int DUNGEON_145c_DrawDrip(int dropX, int dropY, int phase, int dist)
{
    if (phase == 5)
    {
        phase = 0;
        ULTIMA_43ae_AudioSweepTone(0xc80, 0xdac, 1, -(dist * 8 - 0x14));
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
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
        }

        ULTIMA_0c9c_GRAP_39_HorizLine(dropX - 1, dropY, dropX + 1);
        ULTIMA_0cf2_GRAP_3c_VertLine(dropX, dropY - 1, dropY + 1);

        if (phase < 4)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
            ULTIMA_0c64_GRAP_30_Pset(dropX, dropY);
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
static int DUNGEON_150a_DrawAhead(int x, int y, int dist)
{
    static u8 D_2e8a_aheadPieceOffset[6] = {0x0c, 0x08, 0x18, 0x08, 0x0c, 0x0c};
    // NOTE: Original size 10 causes OOB. FMT uses 2 bytes of padding.
    static u8 D_2e90_aheadDripY[10] = {0x28, 0x2f, 0x42, 0x64, 0x92, 0x2e, 0x32, 0x3e, 0x52, 0x6d};
    int phase;
    int cell;
    int piece_j;

    cell = DUNGEON_10dc_CellAt(x, y);
    if (cell < 0xa0)
    {
        return 1;
    }

    piece_j = D_2e8a_aheadPieceOffset[(cell >> 4) - 0xa] + dist;
    if (dist == 0)
    {
        piece_j = 0xc;
    }

    DUNGEON_134a_DrawPiece(piece_j, 0, 0);
    DUNGEON_134a_DrawPiece(piece_j, 0, 1);

    if ((dist == 1 || dist == 2) && D_6604_dungeonStyle == 1 && (cell & 0xf0) == 0xc0)
    {
        phase = cell & 7;

        phase = DUNGEON_145c_DrawDrip(0x5f, D_2e90_aheadDripY[phase + (dist - 1) * 5] + 0xe, phase, dist);

        GetDungeonMap(x & 7, y & 7, D_5895_mapLevel) &= 0xf8;
        GetDungeonMap(x & 7, y & 7, D_5895_mapLevel) += phase;
    }

    if (dist == 1)
    {
        if ((cell & 0xf0) == 0xb0 && (cell & 0xf) != 0)
        {
            DUNGEON_104c_ShowPlaceName(cell & 0xf);
            // NOT MATCHING: nop
        }
        else if (D_6604_dungeonStyle == 3 && (cell & 0xf0) == 0xc0 && (int)ULTIMA_2092_RandomRange(0, 0x40) < 4)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor + 8);
            ULTIMA_0c9c_GRAP_39_HorizLine(0x5c, 0x57, 0x5d);
            ULTIMA_0c9c_GRAP_39_HorizLine(0x5b, 0x58, 0x5d);
            ULTIMA_0c9c_GRAP_39_HorizLine(0x61, 0x57, 0x62);
            ULTIMA_0c9c_GRAP_39_HorizLine(0x61, 0x58, 99);
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
static void DUNGEON_1682_DrawSideCell(int x, int y, int side, int dist)
{
    // NOTE: Original size 10 causes OOB. FMT uses 2 bytes of padding.
    static u8 D_2e9a_sideDripY[10] = {0x0e, 0x17, 0x32, 0x62, 0x9f, 0x28, 0x2d, 0x3c, 0x54, 0x77};
    int phase;
    int cell;
    int dropX_c;
    int piece_j;

    cell = DUNGEON_10dc_CellAt(x, y);
    if (cell >= 0xa0)
    {
        switch (cell & 0xf0)
        {
        case 0xc0:
            // 16c4
            piece_j = dist + 0x14;
            DUNGEON_134a_DrawPiece(piece_j, 0, side);
            if (dist >= 2)
            {
                break;
            }

            if (D_6604_dungeonStyle != 1)
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

            phase = DUNGEON_145c_DrawDrip(dropX_c, D_2e9a_sideDripY[phase + dist * 5] + 0xe, phase, dist);

            GetDungeonMap(x & 7, y & 7, D_5895_mapLevel) &= 0xf8;
            GetDungeonMap(x & 7, y & 7, D_5895_mapLevel) += phase;
            // NOT MATCHING (nop)
            break;

        case 0xa0:
        case 0xe0:
        case 0xf0:
            // 1766
            DUNGEON_134a_DrawPiece(dist + 4, 0, side);
            break;

        default:
            DUNGEON_134a_DrawPiece(dist, 0, side);
            break;
        }
    }
    else
    {
        DUNGEON_134a_DrawPiece(dist + 0x10, 0, side);
    }
}

// CHECKED
// the dots of a ladder's rungs (or a hole) at that distance, one set per frame
static void DUNGEON_1786_DrawWallDots(int dist, int frame)
{
    static Pair D_2ea4_dotsDist0[3][8] =
    {
        0x5a, 0x73, 0x52, 0x77, 0x5f, 0x7b, 0x57, 0x7d,
        0x5b, 0x83, 0x50, 0x85, 0x50, 0x85, 0x50, 0x85,
        0x57, 0x73, 0x5e, 0x77, 0x51, 0x7a, 0x5d, 0x7d,
        0x5f, 0x81, 0x55, 0x82, 0x59, 0x87, 0x50, 0x8b,
        0x54, 0x75, 0x5a, 0x75, 0x5a, 0x7c, 0x50, 0x7e,
        0x55, 0x88, 0x59, 0x8a, 0x59, 0x8a, 0x59, 0x8a
    };
    static Pair D_2ed4_dotsDist1[3][5] =
    {
        0x16, 0x07, 0x13, 0x0a, 0x0d, 0x0b, 0x17, 0x14,
        0x0d, 0x1b, 0x12, 0x06, 0x11, 0x0f, 0x17, 0x0f,
        0x0c, 0x10, 0x0f, 0x1a, 0x0f, 0x08, 0x16, 0x0b,
        0x10, 0x13, 0x0b, 0x15, 0x12, 0x1c
    };
    static Pair D_2ef2_dotsDist2[3][4] =
    {
        0x0b, 0x01, 0x0f, 0x04, 0x09, 0x06, 0x0b, 0x08,
        0x0a, 0x02, 0x0e, 0x02, 0x0c, 0x0a, 0x0c, 0x0a,
        0x0d, 0x01, 0x09, 0x04, 0x09, 0x09, 0x09, 0x09
    };
    static Pair D_2f0a_dotsDist3[3] =
    {
        0x06, 0x00, 0x05, 0x01, 0x07, 0x02
    };
    static Pair D_2f10_dotsDist0Extra[3] =
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
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    }

    // 17a9
    switch (dist)
    {
    case 0:
        // 17cc
        if (D_52c8_videoDriver != 0 && D_52c8_videoDriver != 3)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
        }

        // 17e4
        for (dotNo = 0; dotNo < 8; dotNo++)
        {
            ULTIMA_0c64_GRAP_30_Pset(D_2ea4_dotsDist0[frame][dotNo].x, D_2ea4_dotsDist0[frame][dotNo].y);
            ULTIMA_0c64_GRAP_30_Pset(0xbe - D_2ea4_dotsDist0[frame][dotNo].x, D_2ea4_dotsDist0[frame][dotNo].y);
        }

        ULTIMA_0c64_GRAP_30_Pset(D_2f10_dotsDist0Extra[frame].x, D_2f10_dotsDist0Extra[frame].y);
        ULTIMA_0c64_GRAP_30_Pset(0xbe - D_2f10_dotsDist0Extra[frame].x, D_2f10_dotsDist0Extra[frame].y);
        break;

    case 1:
        // 185e
        for (dotNo = 0; dotNo < 5; dotNo++)
        {
            ULTIMA_0c64_GRAP_30_Pset(D_2ed4_dotsDist1[frame][dotNo].x + 0x48, D_2ed4_dotsDist1[frame][dotNo].y + 0x60);
            ULTIMA_0c64_GRAP_30_Pset(0x76 - D_2ed4_dotsDist1[frame][dotNo].x, D_2ed4_dotsDist1[frame][dotNo].y + 0x60);
        }
        break;

    case 2:
        // 18c4
        for (dotNo = 0; dotNo < 4; dotNo++)
        {
            ULTIMA_0c64_GRAP_30_Pset(D_2ef2_dotsDist2[frame][dotNo].x + 0x50, D_2ef2_dotsDist2[frame][dotNo].y + 0x60);
            ULTIMA_0c64_GRAP_30_Pset(0x6e - D_2ef2_dotsDist2[frame][dotNo].x, D_2ef2_dotsDist2[frame][dotNo].y + 0x60);
        }
        break;

    case 3:
        // 1920
        ULTIMA_0c64_GRAP_30_Pset(D_2f0a_dotsDist3[frame].x + 0x58, D_2f0a_dotsDist3[frame].y + 0x60);
        ULTIMA_0c64_GRAP_30_Pset(0x66 - D_2f0a_dotsDist3[frame].x, D_2f0a_dotsDist3[frame].y + 0x60);
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
// the register variable to the bare block after it (the same device as SJOG_0514_FindHiddenItem). That
// needs the wall branch written with gotos: its block ends in a jump, so MSC places it
// right after the test, as the then-branch of the plain if/else below.
// The source the original was compiled from is not known; this is a spelling that gives
// its bytes.
// a cell at a distance, from the far end back: wall pieces, ladders, fields, the visited mark, the monster there
static void DUNGEON_1952_DrawCell(int cellX, int cellY, int dist)
{
    static u8 D_2f16_mirroredPiece[8] = {0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00};
    static u8 D_2f1e_plainPiece[8] = {0x00, 0x00, 0x1f, 0x1f, 0x37, 0x27, 0x2f, 0x3f};
    static u8 D_2f26_dotFrame = 0;
    int kind_j;

    if (1)
    {
        int cell_b;

        cell_b = DUNGEON_10dc_CellAt(cellX, cellY);
        kind_j = cell_b >> 4;
        if (kind_j > 0 && kind_j < 8 && (kind_j != 6 || (cell_b & 7) == 0))
        {
            goto L_wall;
        }
        if (kind_j == 8)
        {
            DUNGEON_127e_DrawField(dist, cell_b & 7);
        }
    }
    {
        register byte* piece;

L_field:
        if (kind_j < 9 && (GetDungeonMap(cellX & 7, cellY & 7, D_5895_mapLevel) & 8) != 0)
        {
            piece = (byte*)(dist * 2);
            piece += D_2f1e_plainPiece[6];
            DUNGEON_134a_DrawPiece((int)piece, 1, 0);
            DUNGEON_134a_DrawPiece((int)piece, 1, 1);
        }

        if (dist != 0 && (u8)(cellX & 7) == D_5c5a_actors[1].x && (u8)(cellY & 7) == D_5c5a_actors[1].y)
        {
            DUNGEON_111e_DrawMonster(dist);
        }

        if (dist == 0)
        {
            D_2f26_dotFrame++;
            if (D_2f26_dotFrame > 2)
            {
                D_2f26_dotFrame = 0;
            }
        }
        return;

L_wall:
        if (D_2f16_mirroredPiece[kind_j] != 0)
        {
            piece = (byte*)D_2f16_mirroredPiece[kind_j];
            piece += dist * 2;
            DUNGEON_134a_DrawPiece((int)piece, 1, 0);
            DUNGEON_134a_DrawPiece((int)piece, 1, 1);
        }

        if (D_2f1e_plainPiece[kind_j] != 0)
        {
            piece = (byte*)D_2f1e_plainPiece[kind_j];
            piece += dist * 2;
            DUNGEON_134a_DrawPiece((int)piece, 0, 0);
            DUNGEON_134a_DrawPiece((int)piece, 0, 1);
        }

        if (kind_j == 5)
        {
            DUNGEON_1786_DrawWallDots(dist, D_2f26_dotFrame);
        }
        goto L_field;
    }
}

// renders the first-person view into page 1: the cells in front (up to four, in light), then back to front; dark without a torch or light
void DUNGEON_1a90_DrawView(int fullView)
{
    static s8 D_2f28_sideDx[4] = {0xff, 0x00, 0x01, 0x00};
    static s8 D_2f2c_sideDy[4] = {0x00, 0xff, 0x00, 0x01};
    int stepX_f;
    int stepY_f;
    int dist;
    int cellX;
    int cellY;

    D_545e_lastDisk = 0xff;
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x60, 0xe, 0xaf, 0xb2);
    ULTIMA_0c3c_SetClipWindow(0x28, 0x2a, 0x98, 0xb7);
    D_52c4_clipEnabled = 0;

    if (D_58a6_lightSpellTurns != 0 || D_58a7_torchTurns != 0)
    {
        cellX = D_5896_mapX;
        cellY = D_5897_mapY;
        stepX_f = D_24d6_facingDx[D_6603_dungeonFacing];
        stepY_f = D_24de_facingDy[D_6603_dungeonFacing];

        for (dist = 0; dist < 4; dist++)
        {
            // 1b0e
            if (DUNGEON_150a_DrawAhead(cellX, cellY, dist) != 0)
            {
                if (dist != 0 || (DUNGEON_10dc_CellAt(D_5896_mapX, D_5897_mapY) != 0xe0 && fullView != 0))
                {
                    // 1b36
                    DUNGEON_1682_DrawSideCell((u8)D_2f28_sideDx[D_6603_dungeonFacing] + cellX, (u8)D_2f2c_sideDy[D_6603_dungeonFacing] + cellY, 0, dist);
                    DUNGEON_1682_DrawSideCell(cellX - (u8)D_2f28_sideDx[D_6603_dungeonFacing], cellY - (u8)D_2f2c_sideDy[D_6603_dungeonFacing], 1, dist);
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
            DUNGEON_1952_DrawCell(cellX, cellY, dist);
        }
    }
    else
    {
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    }

    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    D_52c4_clipEnabled = 0;
}

// vitals, the view to the screen, step sound, level and direction
void DUNGEON_1be0_Redraw(void)
{
    ULTIMA_2900_UpdateVitalsDisplay();
    DUNGEON_1a90_DrawView(1);
    ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x10, 0xe, 0xaf, 0xb2);
    DUNGEON_1020_StepSound();
    DUNGEON_01d2_PrintWalkDir();
}

// whether the cell under the party on that level is free (the logic is probably not what was meant)
static int DUNGEON_1c0c_CanLandOn(int level, int mustBeEmpty)
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
int DUNGEON_1c6a_Klimb(int dir, int mustBeEmpty)
{
    int leaves;

    leaves = 0;

    if (dir > 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x6c6c, "Down!\n"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x6c74, "Up!\n"));
    }

    if (dir > 0 && D_5895_mapLevel == 7 || dir < 0 && D_5895_mapLevel == 0)
    {
        leaves = 1;
    }
    else
    {
        if (DUNGEON_1c0c_CanLandOn(dir + (uint)D_5895_mapLevel, mustBeEmpty) != 0)
        {
            D_5895_mapLevel += dir;
            if (dir > 0)
            {
                D_6602_dungeonMoveDir = 5;
            }
            else
            {
                D_6602_dungeonMoveDir = 4;
            }

            DUNGEON_0134_SetUpMonster(1);
            DUNGEON_1be0_Redraw();
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x6c7a, "Failed!\n"));
            ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
        }
    }

    return leaves;
}

// leaves the dungeon to the surface or the underworld, at the dungeon's place on the map
void DUNGEON_1d08_Exit(void)
{
    D_5896_mapX = D_1e8a_placeX[D_5893_mapId - 1];
    D_5897_mapY = D_1eb2_placeY[D_5893_mapId - 1];

    ULTIMA_1850_PrintString(_TEXT(0x6c84, "\nExit to "));
    if (D_5895_mapLevel != 0)
    {
        D_5895_mapLevel = 0xff;
        ULTIMA_1850_PrintString(_TEXT(0x6c8e, "Underworld!\n\n"));
    }
    else
    {
        D_5895_mapLevel = 0;
        ULTIMA_1850_PrintString(_TEXT(0x6c9c, "Britannia!\n\n"));
    }

    D_5893_mapId = 0;
}

// the Attack command: fights the monster when it is in the cell ahead
int DUNGEON_1d4a_AttackCmd(void)
{
    int targetY;
    int targetX;
    int result;

    result = 0;

    ULTIMA_1850_PrintString(_TEXT(0x6caa, "Attack\n"));

    targetX = (D_24d6_facingDx[D_6603_dungeonFacing] + D_5896_mapX) & 7;
    targetY = (D_24de_facingDy[D_6603_dungeonFacing] + D_5897_mapY) & 7;
    if (targetX == D_5c5a_actors[1].x && targetY == D_5c5a_actors[1].y)
    {
        D_58a1_combatFlags = 2;
        DNGLOOK_1130_FreeDungeonImages();
        DNGLOOK_0c6c_BuildWallRing();
        DNGLOOK_0d3e_SetUpDungeonFight();
        ULTIMA_5f86_SpecialMapHandler(2, D_5c5a_actors[1].value, 0);

        if (D_58a0_combatExitDir == 5)
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
        else if (D_58a0_combatExitDir == 6)
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

        DNGLOOK_109e_LoadDungeonImages(1);

        if (D_5893_mapId != 0)
        {
            DUNGEON_0134_SetUpMonster(1);
            DUNGEON_1be0_Redraw();
        }
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x6cb2, "What?\n"));
    }

    return result;
}

// the Klimb command: a ladder or the grapple allows up, down or both (then asks U/D)
int DUNGEON_1e10_KlimbCmd(void)
{
    int kind_c;
    int visited;
    int canDown_m;
    int canUp;

    canUp = 0;
    canDown_m = 0;
    kind_c = DUNGEON_10dc_CellAt(D_5896_mapX, D_5897_mapY) & 0xf0;
    visited = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) & 8;
    if (kind_c == 0x10 || kind_c == 0x30 || (visited != 0 && D_57af_grapple != 0))
    {
        canUp = 1;
    }

    if (kind_c == 0x20 || kind_c == 0x30 || kind_c == 0x60)
    {
        canDown_m = 1;
    }

    if (canUp && canDown_m)
    {
        ULTIMA_1850_PrintString(_TEXT(0x6cba, "Klimb-U/D-"));
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
                ULTIMA_1850_PrintString(_TEXT(0x6cc6, "Pass\n\n"));
                canUp = canDown_m = 0;
                break;
            }
        }
    }
    else if (canUp || canDown_m) // 1eee
    {
        // 1efa
        ULTIMA_1850_PrintString(_TEXT(0x6cce, "Klimb-"));
    }
    else if (visited != 0)
    {
        // 1f08
        ULTIMA_1850_PrintString(_TEXT(0x6cd6, "Klimb-\nWith What?\n"));

        return 0;
    }
    else
    {
        // 1f16
        ULTIMA_1850_PrintString(_TEXT(0x6cea, "Klimb-what?\n"));

        return 0;
    }

    // 1f1c
    if ((canUp && DUNGEON_1c6a_Klimb(-1, 0) != 0) || (canDown_m && DUNGEON_1c6a_Klimb(1, 0) != 0))
    {
        DUNGEON_1d08_Exit();
    }

    return 1;
}
