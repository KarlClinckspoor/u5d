#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"

#include <string.h>

#include "dnglook.h"
#include "dungeon.h"
#include "sjog.h"
#include "audio/audio.h"

static u8 D_a528_viewQueueX[0x100]; // this object's _BSS (DS:a528)
static u8 D_a628_viewQueueY[0x100];

u8 D_383a_roomsNeverCleared[6] = {0x50, 0x5b, 0x41, 0x46, 0x4b, 0x4c};

u8 D_3840_neverClearedCount = 6;

u8 D_3842_objectValueRange[0xd] =
{
    0x08, 0x08, 0x04, 0x03, 0x08, 0x08, 0x04, 0x03,
    0x06, 0x03, 0x08, 0x01, 0x08
};

u8 D_3850_objectValueBase[0xd] =
{
    0x00, 0x00, 0x1e, 0x04, 0x01, 0x01, 0x00, 0x2a,
    0x09, 0x2d, 0x01, 0x01, 0x01
};

u8 D_385e_randomMonsterTypes[8] = {0x14, 0x15, 0x16, 0x22, 0x21, 0x18, 0x1f, 0x18};

// CHECKED (code structure)
// the Look command in a dungeon: darkness without light, else what is in the cell ahead (fields, walls, doors, chests, ladders, a fountain to drink from)
void DNGLOOK_0000_LookCmdInDungeon(void)
{
    int member_g;
    int cell_o;
    int lookY;
    int lookX;
    byte answer;
    int style_o;

    if ((member_g = ULTIMA_4988_GetActingPlayer()) + 1 == 0)
    {
        return;
    }

    if (D_58a7_torchTurns == 0 && D_58a6_lightSpellTurns == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x752e, "You see:\ndarkness.\n"));
        return;
    }

    if (SJOG_006c(D_6603_dungeonFacing) == 0)
    {
        return;
    }

    lookX = D_5876_tempX;
    lookY = D_5878_tempY;

    cell_o = GetDungeonMap(lookX & 7, lookY & 7, D_5895_mapLevel);

    ULTIMA_1850_PrintString(_TEXT(0x7542, "You see:\n"));

    if (cell_o == 0x61)
    {
        cell_o = 0;
    }

    // a30b
    if ((cell_o & 0xf0) == 0x80)
    {
        switch (cell_o)
        {
        case 0x80:
            ULTIMA_1850_PrintString(_TEXT(0x754c, "A sleep field.\n"));
            break;
        case 0x81:
            ULTIMA_1850_PrintString(_TEXT(0x755c, "A poison gas field.\n"));
            break;
        case 0x82:
            ULTIMA_1850_PrintString(_TEXT(0x7572, "A wall of fire.\n"));
            break;
        case 0x83:
            ULTIMA_1850_PrintString(_TEXT(0x7584, "An electric field.\n"));
            break;
        default:
            ULTIMA_1850_PrintString(_TEXT(0x7598, "An energy field.\n"));
            // nop
            break;
        }

        return;
    }

    if ((cell_o & 0xf0) == 0xc0)
    {
        style_o = D_6604_dungeonStyle & 0xf;
        if (style_o == 1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x75aa, "a dripping stalactite.\n"));
        }
        else if (style_o == 2)
        {
            ULTIMA_1850_PrintString(_TEXT(0x75c2, "a caved in passage.\n"));
        }
        else
        {
            // a376
            if (ULTIMA_2092_RandomRange(1, 0xff) == 0xff)
            {
                ULTIMA_1850_PrintString(_TEXT(0x75d8, "an unfortunate software pirate.\n"));
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x75fa, "a less fortunate adventurer.\n"));
            }
        }

        return;
    }

    // a392
    switch (cell_o & 0xf0)
    {
    case 0x00: // a3b8
        ULTIMA_1850_PrintString(_TEXT(0x7618, "a passage.\n"));
        break;
    case 0x10: // a3d6
        ULTIMA_1850_PrintString(_TEXT(0x7624, "an up ladder.\n"));
        break;
    case 0x20: // a3dc
        ULTIMA_1850_PrintString(_TEXT(0x7634, "a down ladder.\n"));
        break;
    case 0x30: // a3e2
        ULTIMA_1850_PrintString(_TEXT(0x7644, "a ladder.\n"));
        break;
    case 0x40:
        ULTIMA_1850_PrintString(_TEXT(0x7650, "a wooden chest.\n"));
        break;
    case 0x50:
        ULTIMA_1850_PrintString(_TEXT(0x7662, "a fountain.\n"));
        break;
    case 0x60:
        ULTIMA_1850_PrintString(_TEXT(0x7670, "a pit.\n"));
        break;
    case 0x70: // a3fa
        ULTIMA_1850_PrintString(_TEXT(0x7678, "an open chest.\n"));
        break;
    case 0x80:
        ULTIMA_1850_PrintString(_TEXT(0x7688, "an energy field.\n"));
        break;
    case 0x90:
        ULTIMA_1850_PrintString(_TEXT(0x769a, "nothing of note.\n"));
        break;
    case 0xa0:
        ULTIMA_1850_PrintString(_TEXT(0x76ac, "a heavy door.\n"));
        break;
    case 0xb0:
        ULTIMA_1850_PrintString(_TEXT(0x76bc, "a wall.\n"));
        break;
    case 0xc0:
        ULTIMA_1850_PrintString(_TEXT(0x76c6, "SPEC WALL ERR.\n"));
        break;
    case 0xd0:
        ULTIMA_1850_PrintString(_TEXT(0x76d6, "a wall.\n"));
        break;
    case 0xe0:
        ULTIMA_1850_PrintString(_TEXT(0x76e0, "a heavy door.\n"));
        break;
    case 0xf0: // a42a
        ULTIMA_1850_PrintString(_TEXT(0x76f0, "a heavy door.\n"));
        break;
    }

    // a3bf
    if ((cell_o & 0xf0) == 0x50)
    {
        // a3cb
        ULTIMA_1850_PrintString(_TEXT(0x7700, "Will you drink?\n"));
        // -> a480

        // a480
        while ((answer = (byte)ULTIMA_266c_GetChar()) != 'Y' && answer != 'N')
        {
        }

        // a48a
        if (answer == 'N')
        {
            ULTIMA_1850_PrintString(_TEXT(0x7712, "No.\n"));
        }
        else
        {
            // a496~ (OK)
            ULTIMA_1850_PrintString(_TEXT(0x7718, "Yes.  Gulp!\n"));

            switch (cell_o)
            {
            case 0x50:
                ULTIMA_1850_PrintString(_TEXT(0x7726, "Cured!\n"));
                D_55a8_party[member_g].status = STATUS_GOOD;
                break;

            case 0x51:
                ULTIMA_1850_PrintString(_TEXT(0x772e, "Healed!\n"));
                D_55a8_party[member_g].hp = D_55a8_party[member_g].maxHp;
                break;

            case 0x52:
                ULTIMA_1850_PrintString(_TEXT(0x7738, "Poisoned!\n"));
                D_55a8_party[member_g].status = STATUS_POISONED;
                break;

            default:
                ULTIMA_1850_PrintString(_TEXT(0x7744, "Bad taste.\n"));
                ULTIMA_2a52_DamagePlayer(member_g, ULTIMA_2092_RandomRange(0, 7));
                break;
            }
        }
    }
    // a50e
}

// a field cell of the overview: stripes in magenta, red, blue and green
static void DNGLOOK_0284_DrawFieldStripes(int left, int top)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b6_magentaColor + 8);
    ULTIMA_0b10_GRAP_Line(left + 1, top, left + 6, top);
    ULTIMA_0b10_GRAP_Line(left + 1, top + 1, left + 6, top + 1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor + 8);
    ULTIMA_0b10_GRAP_Line(left + 1, top + 2, left + 6, top + 2);
    ULTIMA_0b10_GRAP_Line(left + 1, top + 3, left + 6, top + 3);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
    ULTIMA_0b10_GRAP_Line(left + 1, top + 4, left + 6, top + 4);
    ULTIMA_0b10_GRAP_Line(left + 1, top + 5, left + 6, top + 5);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b4_greenColor + 8);
    ULTIMA_0b10_GRAP_Line(left + 1, top + 6, left + 6, top + 6);
    ULTIMA_0b10_GRAP_Line(left + 1, top + 7, left + 6, top + 7);
}

// the View command: draws one cell (its glyph or picture) of the dungeon level at (col, row) of the 22x22 view; 1 when it is open ground to spread to
static int DNGLOOK_0340_DrawViewCell(int col, int row)
{
    byte kind;
    int open;
    int cellY_g;
    int cellX_h;

    open = 0;
    if (col < 0 || col > 0x15 || row < 0 || row > 0x15)
    {
        return 0;
    }

    if (D_ad14_combatMap[row * 0x20 + col] != 0)
    {
        open = 1;
        D_ad14_combatMap[row * 0x20 + col] = 0;

        cellX_h = (col + D_5896_mapX - 0xb) & 7;
        cellY_g = (row + D_5897_mapY - 0xb) & 7;
        kind = GetDungeonMap(cellX_h, cellY_g, D_5895_mapLevel);
        kind >>= 4;

        ULTIMA_1bf2_SetTextPosition(col + 1, row + 1);

        // a66a
        col = col * 8 + 8;
        row = row * 8 + 8;
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);

        switch (kind)
        {
        case 0:
            if ((GetDungeonMap(cellX_h, cellY_g, D_5895_mapLevel) & 8) != 0)
            {
                ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
                ULTIMA_16ba_PrintChar(0x18);
            }
            break;

        case 1:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
            ULTIMA_16ba_PrintChar(0x2e); // ladder up (TODO: define?)
            break;

        case 2:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
            ULTIMA_16ba_PrintChar(0x2d); // ladder down
            break;

        case 3:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
            ULTIMA_16ba_PrintChar(0x2f); // ladder u/d
            break;

        case 4:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13b8_brightYellowColor);
            ULTIMA_16ba_PrintChar(0x70); // chest?
            break;

        case 5:
            ULTIMA_0b10_GRAP_Line(col + 9 - 8, row + 0xc - 8, col + 0xe - 8, row + 0xc - 8);
            ULTIMA_0b10_GRAP_Line(col + 10 - 8, row + 0xd - 8, col + 0xd - 8, row + 0xd - 8);
            ULTIMA_0b10_GRAP_Line(col + 9 - 8, row + 0xe - 8, col + 10 - 8, row + 0xe - 8);
            ULTIMA_0b10_GRAP_Line(col + 0xd - 8, row + 0xe - 8, col + 0xe - 8, row + 0xe - 8);
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
            ULTIMA_0c64_GRAP_30_Pset(col + 9 - 8, row + 10 - 8);
            ULTIMA_0c64_GRAP_30_Pset(col + 10 - 8, row + 9 - 8);
            ULTIMA_0b10_GRAP_Line(col + 0xb - 8, row + 10 - 8, col + 0xc - 8, row + 10 - 8);
            ULTIMA_0b10_GRAP_Line(col + 0xb - 8, row + 0xb - 8, col + 0xc - 8, row + 0xb - 8);
            ULTIMA_0c64_GRAP_30_Pset(col + 0xd - 8, row + 9 - 8);
            ULTIMA_0c64_GRAP_30_Pset(col + 0xe - 8, row + 10 - 8);
            break;

        case 6:
            // a7e4
            ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
            
            switch (GetDungeonMap(cellX_h, cellY_g, (uint)D_5895_mapLevel))
            {
            case 96:
                ULTIMA_16ba_PrintChar(0x19);
                break;
            case 97:
            case 105:
                ULTIMA_1c9e_SelectCharset(1);
                ULTIMA_16ba_PrintChar(0x71); // pit trap
                break;
            case 104:
                ULTIMA_16ba_PrintChar(0x12);
                break;
            default:
                ULTIMA_1c9e_SelectCharset(1);
                ULTIMA_1cca_SetTextForegroundColor(D_13ae_redColor + 8);
                ULTIMA_16ba_PrintChar(0x72);
                break;
            }

            break;

        case 8:
            DNGLOOK_0284_DrawFieldStripes(col, row);
            break;

        case 10:
        case 0xf:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13b8_brightYellowColor);
            ULTIMA_16ba_PrintChar(0x73);
            break;

        case 0xb:
            if (GetDungeonMap(cellX_h, cellY_g, (uint)D_5895_mapLevel) == 0xb0)
            {
                ULTIMA_16ba_PrintChar(0x7f);
            }
            else
            {
                ULTIMA_1c9e_SelectCharset(1);
                ULTIMA_16ba_PrintChar(0x74);
            }
            open = 0;
            break;

        case 0xc:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1f26_SetTextBackgroundColor(D_13b2_blueColor);
            ULTIMA_16ba_PrintChar(0x75);
            ULTIMA_1f26_SetTextBackgroundColor(0);
            open = 0;
            break;

        case 0xd:
            ULTIMA_1c9e_SelectCharset(1);
            if (D_52c8_videoDriver == 0)
            {
                ULTIMA_1f26_SetTextBackgroundColor(D_13b2_blueColor);
            }
            else if (D_52c8_videoDriver == 1 || D_52c8_videoDriver == 2)
            {
                ULTIMA_1f26_SetTextBackgroundColor(D_13ba_lightGrayColor);
            }

            ULTIMA_16ba_PrintChar(0x76);
            ULTIMA_1f26_SetTextBackgroundColor(0);
            open = 0;
            break;

        case 0xe:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13b8_brightYellowColor);
            ULTIMA_16ba_PrintChar(0x77);
            break;
        }
    }

    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
    ULTIMA_1c9e_SelectCharset(0);

    return open;
}

// the View command: spreads from the party's cell over the open ground of the level, drawing each cell, until a key
void DNGLOOK_06a8_ViewCmd(void)
{
    int dir;
    int nextY_d;
    int nextX_e;
    byte* headY;
    int curY_h;
    byte* headX;
    int curX_i;
    byte* queueLast;
    byte* tailY;
    byte* tailX;

    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);

    tailX = headX = D_a528_viewQueueX;
    tailY = headY = D_a628_viewQueueY;
    queueLast = &D_a528_viewQueueX[0xff];

    memset(D_ad14_combatMap, 0xff, 0x2e0);

    *tailX = 0xb;
    tailX++;
    *tailY = 0xb;
    tailY++;
    nextX_e = 0x60;
    nextY_d = 0x60;
    D_ad14_combatMap[0x16b] = 0;

    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(0xc, 0xc);
    ULTIMA_1c9e_SelectCharset(1);
    ULTIMA_1cca_SetTextForegroundColor(D_13b4_greenColor + 8);
    if (D_52c8_videoDriver == 3)
    {
        ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
    }

    // a9ce
    ULTIMA_16ba_PrintChar(0x60);
    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
    ULTIMA_1c9e_SelectCharset(0);
    // -> aa66

    while (tailX != headX) // aa66
    {
        for (dir = 0; dir < 8; dir++) // aa3f
        {
            curX_i = nextX_e = *headX;
            curY_h = nextY_d = *headY;

            // ok
            switch (dir)
            {
            case 0:
                nextX_e--;
                nextY_d--;
                break;

            case 1:
                nextY_d--;
                break;

            case 2:
                nextX_e++;
                nextY_d--;
                break;

            case 3:
                nextX_e--;
                break;

            case 4:
                nextX_e++;
                break;

            case 5:
                nextX_e--;
                nextY_d++;
                break;

            case 6:
                nextY_d++;
                break;

            case 7:
                nextX_e++;
                nextY_d++;
                break;
            }

            // aa3f ok
            if (DNGLOOK_0340_DrawViewCell(nextX_e, nextY_d) != 0)
            {
                *tailX++ = nextX_e;
                *tailY++ = nextY_d;

                if (tailX > queueLast)
                {
                    tailX = D_a528_viewQueueX;
                    tailY = D_a628_viewQueueY;
                }
            }
        }

        headX++;
        headY++;
        if (headX > queueLast)
        {
            headX = D_a528_viewQueueX;
            headY = D_a628_viewQueueY;
        }
    }

    // aaaa ok
    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_1dda_WaitForKeystroke(0);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    DUNGEON_1be0();
}

// CHECKED (nop)
// sets the room's bit in D_58e0_roomsCleared (the level's 16 rooms), except for the rooms D_383a lists
void DNGLOOK_0844_MarkRoomCleared(int room)
{
    int roomBit;
    int byteIndex;
    int index;
    int level;

    for (index = 0; index < D_3840_neverClearedCount; index++)
    {
        if (D_383a_roomsNeverCleared[index] == (D_5893_mapId & 0xf) * 0x10 + room)
        {
            return; // nop NOT MATCHING
        }
    }

    level = D_5893_mapId - 0x21;
    if (level >= 1)
    {
        level--;
    }

    roomBit = room + level * 0x10;
    byteIndex = roomBit / 8;
    D_58e0_roomsCleared[byteIndex] |= 1 << ((byte)roomBit & 7);
}

// the room's bit of D_58e0_roomsCleared on this level
static int DNGLOOK_08d4_IsRoomCleared(int room)
{
    int level;
    int roomBit;
    int byteIndex;
    u8 mask;
    u8 flags_c;

    level = (int)D_5893_mapId - 0x21;
    if (level >= 1) {
        level--;
    }

    // 08ed
    roomBit = level * 0x10 + room;
    byteIndex = roomBit / 8;
    flags_c = D_58e0_roomsCleared[byteIndex];
    mask = 1 << (roomBit & 7);

    if (mask & flags_c)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// in the loaded level, a cell of kind 0xf? whose room is cleared becomes 0xa? (&= 0xaf: a door)
void DNGLOOK_093a_ApplyClearedRooms(void)
{
    byte cell_m;
    byte* cellPtr_g;
    int room;
    int index;
    byte kind;

    cellPtr_g = D_595a_dungeonMap;

    for (index = 0; index < 0x200; index++)
    {
        cell_m = *cellPtr_g;
        kind = cell_m & 0xf0;
        if (kind == 0xf0)
        {
            room = cell_m & 0xf;
            if (DNGLOOK_08d4_IsRoomCleared(room) != 0)
            {
                *cellPtr_g &= 0xaf;
            }
        }
        cellPtr_g++;
    }
}

// a combat map side (0 north, 1 east, 2 south, 3 west) all solid (0xff); a door cell (0xe0) puts two wall tiles in the middle
static void DNGLOOK_097e_FillWallSide(byte cellKind, int side)
{
    int y;
    int x;
    int count;
    int stepY_f;
    int stepX_g;

    stepX_g = 0;
    stepY_f = 0;
    x = 0;
    y = 0;

    switch (side)
    {
    case 0:
        stepX_g++;
        break;

    case 1:
        stepY_f++;
        x = 10;
        break;

    case 2:
        stepX_g++;
        y = 10;
        break;

    case 3:
        stepY_f++;
        break;
    }

    for (count = 0; count < 0xb; count++)
    {
        GetCombatMap(x, y) = 0xff;
        x += stepX_g;
        y += stepY_f;
    }

    if (side == 0 && cellKind == 0xe0)
    {
        GetCombatMap(5, 2) = GetCombatMap(5, 8) = D_bb14;
    }

    if (side == 3 && cellKind == 0xe0)
    {
        GetCombatMap(2, 5) = GetCombatMap(8, 5) = D_bb14;
    }
}

// five floor tiles of the wall ring on one side
static void DNGLOOK_0a48_OpenNarrowGap(int side)
{
    int count;
    int stepY_f;
    int stepX_g;
    int y;
    int x;

    stepX_g = 0;
    stepY_f = 0;
    x = 3;
    y = 3;

    switch (side)
    {
    case 0:
        stepX_g++;
        y = 1;
        break;

    case 1:
        stepY_f++;
        x = 9;
        break;

    case 2:
        stepX_g++;
        y = 9;
        break;

    case 3:
        stepY_f++;
        x = 1;
        break;
    }

    for (count = 0; count < 5; count++)
    {
        GetCombatMap(x, y) = D_bb15;
        x += stepX_g;
        y += stepY_f;
    }
}

// seven floor tiles of the wall ring on one side
static void DNGLOOK_0aee_OpenWideGap(int side)
{
    int y;
    int x;
    int count;
    int stepY_f;
    int stepX_g;

    stepX_g = 0;
    stepY_f = 0;

    switch (side)
    {
    case 0:
        stepX_g++;
        x = 2;
        y = 1;
        break;

    case 1:
        stepY_f++;
        x = 9;
        y = 2;
        break;

    case 2:
        stepX_g++;
        x = 2;
        y = 9;
        break;

    case 3:
        stepY_f++;
        x = 1;
        y = 2;
        break;
    }

    for (count = 0; count < 7; count++)
    {
        GetCombatMap(x, y) = D_bb15;
        x += stepX_g;
        y += stepY_f;
    }
}

// CHECKED (nop)
// the combat map's side toward the neighbouring cell: a gap when it is open, solid when it is a wall, else the narrow gap
static void DNGLOOK_0b9e_BuildSide(int side)
{
    byte neighbour_f;
    int cellY;
    int cellX;

    cellX = D_5896_mapX;
    cellY = D_5897_mapY;
    switch (side)
    {
    case 0:
        cellY--;
        break;

    case 1:
        cellX++;
        break;

    case 2:
        cellY++;
        break;

    case 3:
        cellX--;
        break;
    }

    neighbour_f = GetDungeonMap(cellX & 7, cellY & 7, D_5895_mapLevel);
    neighbour_f &= 0xf0;
    if (neighbour_f < 0xa0)
    {
        DNGLOOK_0aee_OpenWideGap(side);
    }

    if (neighbour_f == 0xb0 || neighbour_f == 0xc0 || neighbour_f == 0xd0)
    {
        DNGLOOK_097e_FillWallSide(GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) & 0xf0, side);
        // nop
    }
    else
    {
        DNGLOOK_0a48_OpenNarrowGap(side);
    }
}

// CHECKED (loop)
// the combat map's wall ring (rows and columns 1 and 9), the centre piece of the party's cell kind (D_244a) and the four sides
void DNGLOOK_0c6c_BuildWallRing(void)
{
    byte kind_j;
    int index_h;

    byte* linePtr_b;

    linePtr_b = &GetCombatMap(0, 1);
    memset(linePtr_b, D_bb14, 0xb);

    linePtr_b = &GetCombatMap(0, 9);
    memset(linePtr_b, D_bb14, 0xb);

    // af2e; NOT MATCHING
    for (index_h = 0; index_h < 0xb; index_h++)
    {
        GetCombatMap(1, index_h) = GetCombatMap(9, index_h) = D_bb14;
    }

    GetCombatMap(0, 0) = GetCombatMap(10, 0) = GetCombatMap(0, 10) = GetCombatMap(10, 10) = 0xff;

    kind_j = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    kind_j &= 0xf0;
    if (kind_j != 0 && kind_j < 0x80)
    {
        byte centreTile;

        kind_j >>= 4;
        centreTile = D_244a_centreTiles[kind_j];
        if (centreTile != 0)
        {
            GetCombatMap(5, 5) = centreTile;
        }

        if (kind_j == 3)
        {
            D_bb16 = 1;
            ULTIMA_6fbc_TilesetMode(0);
        }
        else
        {
            D_bb16 = 0;
        }
    }

    for (index_h = 0; index_h < 4; index_h++)
    {
        DNGLOOK_0b9e_BuildSide(index_h);
    }
}

// CHECKED (optimization)
// floor, ring, the party's start cells, the monsters' cells (turned by the facing), the monster kind and how many; fills the combat map
void DNGLOOK_0d3e_SetUpDungeonFight(void)
{
    byte* southPtr;
    int pickIdx_l;
    int index;
    int order_m[16];
    byte* eastPtr_u;
    byte* monsterYPtr;
    byte* westPtr_n;
    byte* monsterXPtr;
    byte* northPtr;
    int count_g;

    for (index = 0; index < 0xb; index++)
    {
        southPtr = &GetCombatMap(0, index);
        memset(southPtr, D_bb15, 0xb);
    }

    if ((D_58a1_combatFlags & 4) != 0)
    {
        GetCombatMap(5, 5) = 0xb3;
    }

    // b006
    DNGLOOK_0c6c_BuildWallRing();

    for (index = 0; index < 8; index++)
    {
        GetCombatMapTriggerX(index) = GetCombatMapTriggerY(index) = 0xff;
    }

    southPtr = &GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_SOUTH, 0);
    westPtr_n = &GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_WEST, 0);
    northPtr = &GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_NORTH, 0);
    eastPtr_u = &GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_EAST, 0);

    for (index = 0; index < 6; index++)
    {
        if ((D_58a1_combatFlags & 4) != 0)
        {
            *southPtr = *westPtr_n = *northPtr = *eastPtr_u = D_2452_roomPartyX[index];
            // nop
        }
        else
        {
            *southPtr = D_2470_partyLineD[index];
            *westPtr_n = D_246a_partyLineC[index];
            *northPtr = D_245e_partyLineA[index];
            *eastPtr_u = D_2464_partyLineB[index];
        }
        southPtr++;
        westPtr_n++;
        northPtr++;
        eastPtr_u++;
    }

    // OK
    for (index = 0; index < 6; index++)
    {
        if ((D_58a1_combatFlags & 4) != 0)
        {
            *southPtr = *westPtr_n = *northPtr = *eastPtr_u = D_2458_roomPartyY[index];
        }
        else
        {
            *southPtr = D_2464_partyLineB[index];
            *westPtr_n = D_245e_partyLineA[index];
            *northPtr = D_246a_partyLineC[index];
            *eastPtr_u = D_2470_partyLineD[index];
        }
        southPtr++;
        westPtr_n++;
        northPtr++;
        eastPtr_u++;
    }

    // b0e3
    monsterXPtr = &GetCombatMapMonsterX(0);
    monsterYPtr = &GetCombatMapMonsterY(0);

    // OK
    for (index = 0; index < 0x10; index++)
    {
        // b0f8
        if ((D_58a1_combatFlags & 4) != 0)
        {
            *monsterYPtr = D_24c6_roomMonsterY[index];
            *monsterXPtr = D_24b6_roomMonsterX[index];
        }
        else
        {
            // b10e
            switch (D_6603_dungeonFacing)
            {
            case 0:
                *monsterXPtr = D_2476_monsterLineA[index];
                *monsterYPtr = D_2496_monsterLineC[index];
                break;
            case 1:
                *monsterXPtr = D_2486_monsterLineB[index];
                *monsterYPtr = D_24a6_monsterLineD[index];
                break;
            case 2:
                *monsterXPtr = D_24a6_monsterLineD[index];
                *monsterYPtr = D_2486_monsterLineB[index];
                break;
            case 3:
                *monsterXPtr = D_2496_monsterLineC[index];
                *monsterYPtr = D_2476_monsterLineA[index];
                break;
            }
        }

        monsterXPtr++;
        monsterYPtr++;
    }

    for (index = 0; index < 0x10; index++)
    {
        order_m[index] = index;
    }

    // b18d
    for (index = 0; index < 0x10; index++)
    {
        pickIdx_l = ULTIMA_2092_RandomRange(0, 0xf);

        count_g = order_m[index];
        order_m[index] = order_m[pickIdx_l];
        order_m[pickIdx_l] = count_g;

        GetCombatMapMonsterTile(index) = 0;
    }

    // b1cf OK
    if ((D_58a1_combatFlags & 4) != 0)
    {
        pickIdx_l = D_173c[ULTIMA_2092_RandomRange(0, 7)];
    }
    else
    {
        pickIdx_l = D_5c5a_actors[1].value;
    }

    count_g = ULTIMA_2092_RandomRange(1, D_13bc_enemyStats[pickIdx_l].maxEncounterCount);
    if (D_13bc_enemyStats[pickIdx_l].maxEncounterCount == 8 || D_13bc_enemyStats[pickIdx_l].maxEncounterCount == 0x10)
    {
        count_g = D_13bc_enemyStats[pickIdx_l].maxEncounterCount;
    }

    // b223
    for (index = 0; index < count_g; index++)
    {
        GetCombatMapMonsterTile(order_m[index]) = pickIdx_l * 4 + 64;
    }
}

// after a fight the party steps in the direction it left by (wrapping), or takes the ladder: up (5) or down (6); out of the dungeon from level 0 or 7
void DNGLOOK_0fda_StepAfterCombat(void)
{
    switch (D_58a0_combatExitDir)
    {
    case DIR_UP:
        D_5897_mapY--;
        D_6602 = D_6603_dungeonFacing = 0;
        if (7 < D_5897_mapY)
        {
            D_5897_mapY = 7;
        }
        break;

    case DIR_RIGHT:
        D_5896_mapX++;
        D_6602 = D_6603_dungeonFacing = 1;
        if (7 < D_5896_mapX)
        {
            D_5896_mapX = 0;
        }
        break;

    case DIR_DOWN:
        D_5897_mapY++;
        D_6602 = D_6603_dungeonFacing = 2;
        if (7 < D_5897_mapY)
        {
            D_5897_mapY = 0;
        }
        break;

    case DIR_LEFT:
        D_5896_mapX--;
        D_6602 = D_6603_dungeonFacing = 3;
        if (7 < D_5896_mapX)
        {
            D_5896_mapX = 7;
        }
        break;

    case 5:
        if (D_5895_mapLevel == 0)
        {
            DUNGEON_1d08_Exit();
            D_5893_mapId = 0;
        }
        else
        {
            D_5895_mapLevel--;
            D_6602 = 4;
        }
        break;

    case 6:
        if (D_5895_mapLevel == 7)
        {
            DUNGEON_1d08_Exit();
            D_5893_mapId = 0;
        }
        else
        {
            D_5895_mapLevel++;
            D_6602 = 5;
        }
        break;
    }
}

// loads the dungeon's wall pictures (DNGn.16, n by the style) and ITEMS.16, clears the view; the encounter actor too when asked
void DNGLOOK_109e_LoadDungeonImages(int clearEncounter)
{
    if (D_5893_mapId != 0)
    {
        if ((D_bb17 & 2) != 0)
        {
            ULTIMA_102e_UnloadTileset();
        }

        if ((D_bb17 & 1) == 0)
        {
            do
            {
                D_a9c0 = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[4 + D_6604_dungeonStyle]); // DNGn.16
            } while (D_a9c0 == 0);

            do
            {
                D_a9c4 = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[4]); // ITEMS.16
            } while (D_a9c4 == 0);
        }

        D_bb17 = 1;
        if (clearEncounter != 0)
        {
            D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = 0;
            D_5c5a_actors[1].value = D_5c5a_actors[1].x = D_5c5a_actors[1].y = 0xff;
        }

        ULTIMA_0a70_GRAP_2d_SetPenColor(0);
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
        ULTIMA_0c22_GRAP_0f_SelectPage(0);
    }
}

// on leaving: frees the pictures, loads TILES.16 again and the tileset mode
void DNGLOOK_1130_FreeDungeonImages(void)
{
#if defined(ENABLE_BGM)
    AUDIO_StopBgm();
#endif

    if ((D_bb17 & 1) != 0)
    {
        if (D_5c5a_actors[1].value != 0xff)
        {
            ULTIMA_0be4_FreeImage(D_a9c6);
        }
        D_a9c6 = 0;
        ULTIMA_0be4_FreeImage(D_a9c4);
        ULTIMA_0be4_FreeImage(D_a9c0);
    }

    if ((D_bb17 & 2) == 0)
    {
        while (ULTIMA_0ff4_LoadTileset(*D_25f0) == 0) // TILES.16
            ;
    }

    if ((D_bb17 & 4) == 0)
    {
        ULTIMA_6f9e_TilesetModePlus3(0);
    }

    D_bb17 = 6;
}

// CHECKED (optimization)
// prepare combat (in dungeon)
// puts the party on the fight's start cells (by the facing) and the monsters of the combat map as actors, with their values
void DNGLOOK_117e_PlaceCombatants(int cellKind, int mode)
{
    int monsterType;
    int placeKind_f;
    int slot;
    int index_j;
    int yIndex;
    int xIndex;
    byte tile_b;
    byte randomTypes_i[4];
    int side_k;

    xIndex = COMBAT_MAP_PARTY_X_INDEX;
    yIndex = COMBAT_MAP_PARTY_Y_INDEX;
    side_k = COMBAT_MAP_SPECIAL_PARTY_NORTH;
    if (mode != 1)
    {
        // b42e
        switch (D_6603_dungeonFacing)
        {
        case 0:
        case 5:
            // b44c
            side_k = COMBAT_MAP_SPECIAL_PARTY_SOUTH;
            break;
        case 1:
            // b4c2
            side_k = COMBAT_MAP_SPECIAL_PARTY_WEST;
            break;
        case 3:
            // b4ca
            side_k = COMBAT_MAP_SPECIAL_PARTY_EAST;
            break;
        }

        // b451
        ULTIMA_6936_InitCombatants();

        for (index_j = 0; index_j < D_585b_partyCount; index_j++)
        {
            D_ba14_combatants[index_j].x = D_5c5a_actors[index_j].x = GetCombatMapSpecial(side_k, xIndex);
            D_ba14_combatants[index_j].y = D_5c5a_actors[index_j].y = GetCombatMapSpecial(side_k, yIndex);
            xIndex++;
            yIndex++;
        }
    }

    // b4d8
    if (mode > 0 && ((cellKind > 0xef && mode == 3) || mode < 3))
    {
        side_k = yIndex = xIndex = COMBAT_MAP_SPECIAL_BASE;

        for (index_j = 0; index_j < 4; index_j++)
        {
            randomTypes_i[index_j] = D_385e_randomMonsterTypes[ULTIMA_2092_RandomRange(0, 7)];
        }

        // b521
        for (index_j = 0; index_j < 0x10; index_j++)
        {
            tile_b = GetCombatMapSpecial(COMBAT_MAP_SPECIAL_MONSTER_TILES, side_k);
            if (tile_b != 0)
            { 
                // b542
                if (tile_b < 0x40 || (tile_b & 0xfc) == 0xb4 || (tile_b & 0xfc) == 0xe8)
                {
                    placeKind_f = 2;
                    monsterType = tile_b;
                    // nop
                }
                else
                {
                    placeKind_f = 0;
                    monsterType = (tile_b - 0x40) / 4;
                }

                // b575
                if ((tile_b & 0xfc) == 0xec)
                {
                    monsterType = randomTypes_i[tile_b & 3];
                }

                // b58e
                slot = ULTIMA_6506_PlaceCombatant(monsterType, placeKind_f, GetCombatMapSpecial(COMBAT_MAP_SPECIAL_MONSTER_X, xIndex),
                                       GetCombatMapSpecial(COMBAT_MAP_SPECIAL_MONSTER_Y, yIndex), D_5895_mapLevel);
                if (placeKind_f == 2)
                {
                    if (monsterType == 1)
                    {
                        D_5c5a_actors[slot].value = D_5895_mapLevel * 3 + 7;
                        // nop
                    }
                    else if (monsterType == 2)
                    {
                        D_5c5a_actors[slot].value = ULTIMA_2092_RandomRange(1, D_5895_mapLevel * 10 + 10);
                    }
                    else if (monsterType < 0x10)
                    {
                        ASSERT(monsterType != 0);
                        // TODO: offset
                        D_5c5a_actors[slot].value = D_3850_objectValueBase[monsterType - 3] + ULTIMA_2092_RandomRange(0, D_3842_objectValueRange[monsterType - 3] - 1);
                    }
                }
            }

            side_k++;
            xIndex++;
            yIndex++;
        }
    }
}
