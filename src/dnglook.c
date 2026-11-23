#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"

#include <string.h>

#include "dnglook.h"
#include "dungeon.h"
#include "sjog.h"

static u8 viewQueueX[0x100]; // this object's _BSS (DS:a528)
static u8 viewQueueY[0x100];

u8 roomsNeverCleared[6] = {0x50, 0x5b, 0x41, 0x46, 0x4b, 0x4c};

u8 neverClearedCount = 6;

u8 objectValueRange[0xd] =
{
    0x08, 0x08, 0x04, 0x03, 0x08, 0x08, 0x04, 0x03,
    0x06, 0x03, 0x08, 0x01, 0x08
};

u8 objectValueBase[0xd] =
{
    0x00, 0x00, 0x1e, 0x04, 0x01, 0x01, 0x00, 0x2a,
    0x09, 0x2d, 0x01, 0x01, 0x01
};

u8 randomMonsterTypes[8] = {0x14, 0x15, 0x16, 0x22, 0x21, 0x18, 0x1f, 0x18};

// CHECKED (code structure)
// the Look command in a dungeon: darkness without light, else what is in the cell ahead (fields, walls, doors, chests, ladders, a fountain to drink from)
void LookCmdInDungeon(void)
{
    int member_g;
    int cell_o;
    int lookY;
    int lookX;
    byte answer;
    int style_o;

    if ((member_g = GetActingPlayer()) + 1 == 0)
    {
        return;
    }

    if (torchTurns == 0 && lightSpellTurns == 0)
    {
        PrintString(/*0x752e*/ "You see:\ndarkness.\n");
        return;
    }

    if (AskDungeonCell(dungeonFacing) == 0)
    {
        return;
    }

    lookX = tempX;
    lookY = tempY;

    cell_o = GetDungeonMap(lookX & 7, lookY & 7, D_5895_mapLevel);

    PrintString(/*0x7542*/ "You see:\n");

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
            PrintString(/*0x754c*/ "A sleep field.\n");
            break;
        case 0x81:
            PrintString(/*0x755c*/ "A poison gas field.\n");
            break;
        case 0x82:
            PrintString(/*0x7572*/ "A wall of fire.\n");
            break;
        case 0x83:
            PrintString(/*0x7584*/ "An electric field.\n");
            break;
        default:
            PrintString(/*0x7598*/ "An energy field.\n");
            // nop
            break;
        }

        return;
    }

    if ((cell_o & 0xf0) == 0xc0)
    {
        style_o = dungeonStyle & 0xf;
        if (style_o == 1)
        {
            PrintString(/*0x75aa*/ "a dripping stalactite.\n");
        }
        else if (style_o == 2)
        {
            PrintString(/*0x75c2*/ "a caved in passage.\n");
        }
        else
        {
            // a376
            if (RandomRange(1, 0xff) == 0xff)
            {
                PrintString(/*0x75d8*/ "an unfortunate software pirate.\n");
            }
            else
            {
                PrintString(/*0x75fa*/ "a less fortunate adventurer.\n");
            }
        }

        return;
    }

    // a392
    switch (cell_o & 0xf0)
    {
    case 0x00: // a3b8
        PrintString(/*0x7618*/ "a passage.\n");
        break;
    case 0x10: // a3d6
        PrintString(/*0x7624*/ "an up ladder.\n");
        break;
    case 0x20: // a3dc
        PrintString(/*0x7634*/ "a down ladder.\n");
        break;
    case 0x30: // a3e2
        PrintString(/*0x7644*/ "a ladder.\n");
        break;
    case 0x40:
        PrintString(/*0x7650*/ "a wooden chest.\n");
        break;
    case 0x50:
        PrintString(/*0x7662*/ "a fountain.\n");
        break;
    case 0x60:
        PrintString(/*0x7670*/ "a pit.\n");
        break;
    case 0x70: // a3fa
        PrintString(/*0x7678*/ "an open chest.\n");
        break;
    case 0x80:
        PrintString(/*0x7688*/ "an energy field.\n");
        break;
    case 0x90:
        PrintString(/*0x769a*/ "nothing of note.\n");
        break;
    case 0xa0:
        PrintString(/*0x76ac*/ "a heavy door.\n");
        break;
    case 0xb0:
        PrintString(/*0x76bc*/ "a wall.\n");
        break;
    case 0xc0:
        PrintString(/*0x76c6*/ "SPEC WALL ERR.\n");
        break;
    case 0xd0:
        PrintString(/*0x76d6*/ "a wall.\n");
        break;
    case 0xe0:
        PrintString(/*0x76e0*/ "a heavy door.\n");
        break;
    case 0xf0: // a42a
        PrintString(/*0x76f0*/ "a heavy door.\n");
        break;
    }

    // a3bf
    if ((cell_o & 0xf0) == 0x50)
    {
        // a3cb
        PrintString(/*0x7700*/ "Will you drink?\n");
        // -> a480

        // a480
        while ((answer = (byte)GetChar()) != 'Y' && answer != 'N')
        {
        }

        // a48a
        if (answer == 'N')
        {
            PrintString(/*0x7712*/ "No.\n");
        }
        else
        {
            // a496~ (OK)
            PrintString(/*0x7718*/ "Yes.  Gulp!\n");

            switch (cell_o)
            {
            case 0x50:
                PrintString(/*0x7726*/ "Cured!\n");
                party[member_g].status = STATUS_GOOD;
                break;

            case 0x51:
                PrintString(/*0x772e*/ "Healed!\n");
                party[member_g].hp = party[member_g].maxHp;
                break;

            case 0x52:
                PrintString(/*0x7738*/ "Poisoned!\n");
                party[member_g].status = STATUS_POISONED;
                break;

            default:
                PrintString(/*0x7744*/ "Bad taste.\n");
                DamagePlayer(member_g, RandomRange(0, 7));
                break;
            }
        }
    }
    // a50e
}

// a field cell of the overview: stripes in magenta, red, blue and green
static void DrawFieldStripes(int left, int top)
{
    GRAP_2d_SetPenColor(magentaColor + 8);
    GRAP_Line(left + 1, top, left + 6, top);
    GRAP_Line(left + 1, top + 1, left + 6, top + 1);
    GRAP_2d_SetPenColor(redColor + 8);
    GRAP_Line(left + 1, top + 2, left + 6, top + 2);
    GRAP_Line(left + 1, top + 3, left + 6, top + 3);
    GRAP_2d_SetPenColor(blueColor + 8);
    GRAP_Line(left + 1, top + 4, left + 6, top + 4);
    GRAP_Line(left + 1, top + 5, left + 6, top + 5);
    GRAP_2d_SetPenColor(greenColor + 8);
    GRAP_Line(left + 1, top + 6, left + 6, top + 6);
    GRAP_Line(left + 1, top + 7, left + 6, top + 7);
}

// the View command: draws one cell (its glyph or picture) of the dungeon level at (col, row) of the 22x22 view; 1 when it is open ground to spread to
static int DrawViewCell(int col, int row)
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

    if (combatMap[row * 0x20 + col] != 0)
    {
        open = 1;
        combatMap[row * 0x20 + col] = 0;

        cellX_h = (col + D_5896_mapX - 0xb) & 7;
        cellY_g = (row + D_5897_mapY - 0xb) & 7;
        kind = GetDungeonMap(cellX_h, cellY_g, D_5895_mapLevel);
        kind >>= 4;

        SetTextPosition(col + 1, row + 1);

        // a66a
        col = col * 8 + 8;
        row = row * 8 + 8;
        GRAP_2d_SetPenColor(brightWhiteColor);

        switch (kind)
        {
        case 0:
            if ((GetDungeonMap(cellX_h, cellY_g, D_5895_mapLevel) & 8) != 0)
            {
                SetTextForegroundColor(lightGrayColor);
                PrintChar(0x18);
            }
            break;

        case 1:
            SelectCharset(1);
            SetTextForegroundColor(lightGrayColor);
            PrintChar(0x2e); // ladder up (TODO: define?)
            break;

        case 2:
            SelectCharset(1);
            SetTextForegroundColor(lightGrayColor);
            PrintChar(0x2d); // ladder down
            break;

        case 3:
            SelectCharset(1);
            SetTextForegroundColor(lightGrayColor);
            PrintChar(0x2f); // ladder u/d
            break;

        case 4:
            SelectCharset(1);
            SetTextForegroundColor(brightYellowColor);
            PrintChar(0x70); // chest?
            break;

        case 5:
            GRAP_Line(col + 9 - 8, row + 0xc - 8, col + 0xe - 8, row + 0xc - 8);
            GRAP_Line(col + 10 - 8, row + 0xd - 8, col + 0xd - 8, row + 0xd - 8);
            GRAP_Line(col + 9 - 8, row + 0xe - 8, col + 10 - 8, row + 0xe - 8);
            GRAP_Line(col + 0xd - 8, row + 0xe - 8, col + 0xe - 8, row + 0xe - 8);
            GRAP_2d_SetPenColor(blueColor + 8);
            GRAP_30_Pset(col + 9 - 8, row + 10 - 8);
            GRAP_30_Pset(col + 10 - 8, row + 9 - 8);
            GRAP_Line(col + 0xb - 8, row + 10 - 8, col + 0xc - 8, row + 10 - 8);
            GRAP_Line(col + 0xb - 8, row + 0xb - 8, col + 0xc - 8, row + 0xb - 8);
            GRAP_30_Pset(col + 0xd - 8, row + 9 - 8);
            GRAP_30_Pset(col + 0xe - 8, row + 10 - 8);
            break;

        case 6:
            // a7e4
            SetTextForegroundColor(lightGrayColor);
            
            switch (GetDungeonMap(cellX_h, cellY_g, (uint)D_5895_mapLevel))
            {
            case 96:
                PrintChar(0x19);
                break;
            case 97:
            case 105:
                SelectCharset(1);
                PrintChar(0x71); // pit trap
                break;
            case 104:
                PrintChar(0x12);
                break;
            default:
                SelectCharset(1);
                SetTextForegroundColor(redColor + 8);
                PrintChar(0x72);
                break;
            }

            break;

        case 8:
            DrawFieldStripes(col, row);
            break;

        case 10:
        case 0xf:
            SelectCharset(1);
            SetTextForegroundColor(brightYellowColor);
            PrintChar(0x73);
            break;

        case 0xb:
            if (GetDungeonMap(cellX_h, cellY_g, (uint)D_5895_mapLevel) == 0xb0)
            {
                PrintChar(0x7f);
            }
            else
            {
                SelectCharset(1);
                PrintChar(0x74);
            }
            open = 0;
            break;

        case 0xc:
            SelectCharset(1);
            SetTextBackgroundColor(blueColor);
            PrintChar(0x75);
            SetTextBackgroundColor(0);
            open = 0;
            break;

        case 0xd:
            SelectCharset(1);
            if (D_52c8_videoDriver == 0)
            {
                SetTextBackgroundColor(blueColor);
            }
            else if (D_52c8_videoDriver == 1 || D_52c8_videoDriver == 2)
            {
                SetTextBackgroundColor(lightGrayColor);
            }

            PrintChar(0x76);
            SetTextBackgroundColor(0);
            open = 0;
            break;

        case 0xe:
            SelectCharset(1);
            SetTextForegroundColor(brightYellowColor);
            PrintChar(0x77);
            break;
        }
    }

    SetTextForegroundColor(brightWhiteColor);
    SelectCharset(0);

    return open;
}

// the View command: spreads from the party's cell over the open ground of the level, drawing each cell, until a key
void DnglookViewCmd(void)
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

    GRAP_2d_SetPenColor(0);
    GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);

    tailX = headX = viewQueueX;
    tailY = headY = viewQueueY;
    queueLast = &viewQueueX[0xff];

    memset(combatMap, 0xff, 0x2e0);

    *tailX = 0xb;
    tailX++;
    *tailY = 0xb;
    tailY++;
    nextX_e = 0x60;
    nextY_d = 0x60;
    combatMap[0x16b] = 0;

    GRAP_2d_SetPenColor(brightWhiteColor);
    SelectTextWindow(0);
    SetTextPosition(0xc, 0xc);
    SelectCharset(1);
    SetTextForegroundColor(greenColor + 8);
    if (D_52c8_videoDriver == 3)
    {
        SetTextForegroundColor(brightWhiteColor);
    }

    // a9ce
    PrintChar(0x60);
    SetTextForegroundColor(brightWhiteColor);
    SelectCharset(0);
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
            if (DrawViewCell(nextX_e, nextY_d) != 0)
            {
                *tailX++ = nextX_e;
                *tailY++ = nextY_d;

                if (tailX > queueLast)
                {
                    tailX = viewQueueX;
                    tailY = viewQueueY;
                }
            }
        }

        headX++;
        headY++;
        if (headX > queueLast)
        {
            headX = viewQueueX;
            headY = viewQueueY;
        }
    }

    // aaaa ok
    SelectTextWindow(2);
    WaitForKeystroke(0);
    GRAP_2d_SetPenColor(0);
    GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    Redraw();
}

// CHECKED (nop)
// sets the room's bit in roomsCleared (the level's 16 rooms), except for the rooms D_383a lists
void MarkRoomCleared(int room)
{
    int roomBit;
    int byteIndex;
    int index;
    int level;

    for (index = 0; index < neverClearedCount; index++)
    {
        if (roomsNeverCleared[index] == (mapId & 0xf) * 0x10 + room)
        {
            return; // nop NOT MATCHING
        }
    }

    level = mapId - 0x21;
    if (level >= 1)
    {
        level--;
    }

    roomBit = room + level * 0x10;
    byteIndex = roomBit / 8;
    roomsCleared[byteIndex] |= 1 << ((byte)roomBit & 7);
}

// the room's bit of roomsCleared on this level
static int IsRoomCleared(int room)
{
    int level;
    int roomBit;
    int byteIndex;
    u8 mask;
    u8 flags_c;

    level = (int)mapId - 0x21;
    if (level >= 1) {
        level--;
    }

    // 08ed
    roomBit = level * 0x10 + room;
    byteIndex = roomBit / 8;
    flags_c = roomsCleared[byteIndex];
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
void ApplyClearedRooms(void)
{
    byte cell_m;
    byte* cellPtr_g;
    int room;
    int index;
    byte kind;

    cellPtr_g = dungeonMap;

    for (index = 0; index < 0x200; index++)
    {
        cell_m = *cellPtr_g;
        kind = cell_m & 0xf0;
        if (kind == 0xf0)
        {
            room = cell_m & 0xf;
            if (IsRoomCleared(room) != 0)
            {
                *cellPtr_g &= 0xaf;
            }
        }
        cellPtr_g++;
    }
}

// a combat map side (0 north, 1 east, 2 south, 3 west) all solid (0xff); a door cell (0xe0) puts two wall tiles in the middle
static void FillWallSide(byte cellKind, int side)
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
        GetCombatMap(5, 2) = GetCombatMap(5, 8) = wallTile;
    }

    if (side == 3 && cellKind == 0xe0)
    {
        GetCombatMap(2, 5) = GetCombatMap(8, 5) = wallTile;
    }
}

// five floor tiles of the wall ring on one side
static void OpenNarrowGap(int side)
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
        GetCombatMap(x, y) = D_bb15_floorTile;
        x += stepX_g;
        y += stepY_f;
    }
}

// seven floor tiles of the wall ring on one side
static void OpenWideGap(int side)
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
        GetCombatMap(x, y) = D_bb15_floorTile;
        x += stepX_g;
        y += stepY_f;
    }
}

// CHECKED (nop)
// the combat map's side toward the neighbouring cell: a gap when it is open, solid when it is a wall, else the narrow gap
static void BuildSide(int side)
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
        OpenWideGap(side);
    }

    if (neighbour_f == 0xb0 || neighbour_f == 0xc0 || neighbour_f == 0xd0)
    {
        FillWallSide(GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) & 0xf0, side);
        // nop
    }
    else
    {
        OpenNarrowGap(side);
    }
}

// CHECKED (loop)
// the combat map's wall ring (rows and columns 1 and 9), the centre piece of the party's cell kind (D_244a) and the four sides
void BuildWallRing(void)
{
    byte kind_j;
    int index_h;

    byte* linePtr_b;

    linePtr_b = &GetCombatMap(0, 1);
    memset(linePtr_b, wallTile, 0xb);

    linePtr_b = &GetCombatMap(0, 9);
    memset(linePtr_b, wallTile, 0xb);

    // af2e; NOT MATCHING
    for (index_h = 0; index_h < 0xb; index_h++)
    {
        GetCombatMap(1, index_h) = GetCombatMap(9, index_h) = wallTile;
    }

    GetCombatMap(0, 0) = GetCombatMap(10, 0) = GetCombatMap(0, 10) = GetCombatMap(10, 10) = 0xff;

    kind_j = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    kind_j &= 0xf0;
    if (kind_j != 0 && kind_j < 0x80)
    {
        byte centreTile;

        kind_j >>= 4;
        centreTile = centreTiles[kind_j];
        if (centreTile != 0)
        {
            GetCombatMap(5, 5) = centreTile;
        }

        if (kind_j == 3)
        {
            altTileset = 1;
            TilesetMode(0);
        }
        else
        {
            altTileset = 0;
        }
    }

    for (index_h = 0; index_h < 4; index_h++)
    {
        BuildSide(index_h);
    }
}

// CHECKED (optimization)
// floor, ring, the party's start cells, the monsters' cells (turned by the facing), the monster kind and how many; fills the combat map
void SetUpDungeonFight(void)
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
        memset(southPtr, D_bb15_floorTile, 0xb);
    }

    if ((combatFlags & 4) != 0)
    {
        GetCombatMap(5, 5) = 0xb3;
    }

    // b006
    BuildWallRing();

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
        if ((combatFlags & 4) != 0)
        {
            *southPtr = *westPtr_n = *northPtr = *eastPtr_u = roomPartyX[index];
            // nop
        }
        else
        {
            *southPtr = partyLineD[index];
            *westPtr_n = partyLineC[index];
            *northPtr = partyLineA[index];
            *eastPtr_u = partyLineB[index];
        }
        southPtr++;
        westPtr_n++;
        northPtr++;
        eastPtr_u++;
    }

    // OK
    for (index = 0; index < 6; index++)
    {
        if ((combatFlags & 4) != 0)
        {
            *southPtr = *westPtr_n = *northPtr = *eastPtr_u = roomPartyY[index];
        }
        else
        {
            *southPtr = partyLineB[index];
            *westPtr_n = partyLineA[index];
            *northPtr = partyLineC[index];
            *eastPtr_u = partyLineD[index];
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
        if ((combatFlags & 4) != 0)
        {
            *monsterYPtr = roomMonsterY[index];
            *monsterXPtr = roomMonsterX[index];
        }
        else
        {
            // b10e
            switch (dungeonFacing)
            {
            case 0:
                *monsterXPtr = monsterLineA[index];
                *monsterYPtr = monsterLineC[index];
                break;
            case 1:
                *monsterXPtr = monsterLineB[index];
                *monsterYPtr = monsterLineD[index];
                break;
            case 2:
                *monsterXPtr = monsterLineD[index];
                *monsterYPtr = monsterLineB[index];
                break;
            case 3:
                *monsterXPtr = monsterLineC[index];
                *monsterYPtr = monsterLineA[index];
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
        pickIdx_l = RandomRange(0, 0xf);

        count_g = order_m[index];
        order_m[index] = order_m[pickIdx_l];
        order_m[pickIdx_l] = count_g;

        GetCombatMapMonsterTile(index) = 0;
    }

    // b1cf OK
    if ((combatFlags & 4) != 0)
    {
        pickIdx_l = monsterTypes[RandomRange(0, 7)];
    }
    else
    {
        pickIdx_l = actors[1].value;
    }

    count_g = RandomRange(1, D_13bc_enemyStats[pickIdx_l].maxEncounterCount);
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
void StepAfterCombat(void)
{
    switch (combatExitDir)
    {
    case DIR_UP:
        D_5897_mapY--;
        dungeonMoveDir = dungeonFacing = 0;
        if (7 < D_5897_mapY)
        {
            D_5897_mapY = 7;
        }
        break;

    case DIR_RIGHT:
        D_5896_mapX++;
        dungeonMoveDir = dungeonFacing = 1;
        if (7 < D_5896_mapX)
        {
            D_5896_mapX = 0;
        }
        break;

    case DIR_DOWN:
        D_5897_mapY++;
        dungeonMoveDir = dungeonFacing = 2;
        if (7 < D_5897_mapY)
        {
            D_5897_mapY = 0;
        }
        break;

    case DIR_LEFT:
        D_5896_mapX--;
        dungeonMoveDir = dungeonFacing = 3;
        if (7 < D_5896_mapX)
        {
            D_5896_mapX = 7;
        }
        break;

    case 5:
        if (D_5895_mapLevel == 0)
        {
            DUNGEON_1d08_Exit();
            mapId = 0;
        }
        else
        {
            D_5895_mapLevel--;
            dungeonMoveDir = 4;
        }
        break;

    case 6:
        if (D_5895_mapLevel == 7)
        {
            DUNGEON_1d08_Exit();
            mapId = 0;
        }
        else
        {
            D_5895_mapLevel++;
            dungeonMoveDir = 5;
        }
        break;
    }
}

// loads the dungeon's wall pictures (DNGn.16, n by the style) and ITEMS.16, clears the view; the encounter actor too when asked
void LoadDungeonImages(int clearEncounter)
{
    if (mapId != 0)
    {
        if ((loadFlags & 2) != 0)
        {
            UnloadTileset();
        }

        if ((loadFlags & 1) == 0)
        {
            do
            {
                wallImage = LoadImageFile(fileNames[4 + dungeonStyle]); // DNGn.16
            } while (wallImage == 0);

            do
            {
                itemsImage = LoadImageFile(fileNames[4]); // ITEMS.16
            } while (itemsImage == 0);
        }

        loadFlags = 1;
        if (clearEncounter != 0)
        {
            actors[1].tile = actors[1].animTile = 0;
            actors[1].value = actors[1].x = actors[1].y = 0xff;
        }

        GRAP_2d_SetPenColor(0);
        GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
        GRAP_0f_SelectPage(1);
        GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
        GRAP_0f_SelectPage(0);
    }
}

// on leaving: frees the pictures, loads TILES.16 again and the tileset mode
void FreeDungeonImages(void)
{
    if ((loadFlags & 1) != 0)
    {
        if (actors[1].value != 0xff)
        {
            FreeImage(monsterImage);
        }
        monsterImage = 0;
        FreeImage(itemsImage);
        FreeImage(wallImage);
    }

    if ((loadFlags & 2) == 0)
    {
        while (LoadTileset(*tilesFileName) == 0) // TILES.16
            ;
    }

    if ((loadFlags & 4) == 0)
    {
        TilesetModePlus3(0);
    }

    loadFlags = 6;
}

// CHECKED (optimization)
// prepare combat (in dungeon)
// puts the party on the fight's start cells (by the facing) and the monsters of the combat map as actors, with their values
void PlaceCombatants(int cellKind, int mode)
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
        switch (dungeonFacing)
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
        InitCombatants();

        for (index_j = 0; index_j < partyCount; index_j++)
        {
            combatants[index_j].x = actors[index_j].x = GetCombatMapSpecial(side_k, xIndex);
            combatants[index_j].y = actors[index_j].y = GetCombatMapSpecial(side_k, yIndex);
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
            randomTypes_i[index_j] = randomMonsterTypes[RandomRange(0, 7)];
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
                slot = PlaceCombatant(monsterType, placeKind_f, GetCombatMapSpecial(COMBAT_MAP_SPECIAL_MONSTER_X, xIndex),
                                       GetCombatMapSpecial(COMBAT_MAP_SPECIAL_MONSTER_Y, yIndex), D_5895_mapLevel);
                if (placeKind_f == 2)
                {
                    if (monsterType == 1)
                    {
                        actors[slot].value = D_5895_mapLevel * 3 + 7;
                        // nop
                    }
                    else if (monsterType == 2)
                    {
                        actors[slot].value = RandomRange(1, D_5895_mapLevel * 10 + 10);
                    }
                    else if (monsterType < 0x10)
                    {
                        // TODO: offset
                        actors[slot].value = objectValueBase[monsterType - 3] + RandomRange(0, objectValueRange[monsterType - 3] - 1);
                    }
                }
            }

            side_k++;
            xIndex++;
            yIndex++;
        }
    }
}
