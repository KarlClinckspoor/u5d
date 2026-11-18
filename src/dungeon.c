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

// defined in dungeonb.c, the part of DUNGEON.OVL built with MSC 5.0
void DUNGEON_1a90_DrawView(int fullView);
void DUNGEON_1be0_Redraw(void);

void DNGLOOK_109e_LoadDungeonImages(int clearEncounter);

u8 D_2c76_facingOfDir[6] = {0x00, 0x01, 0x02, 0x03, 0x00, 0x02};

// "Entering room...": a cell of kind 0xa? / 0xf? opens its room: the combat map is read from dungeon.cbt, the party fights; a won room is marked cleared
static void DUNGEON_0000_EnterRoom(byte cell)
{
    int level_f;
    int room;
    ActorFmt savedMonster2;

    ActorFmt savedMonster1;
    int fileOffset_f;

    int savedY;
    int savedX;
    int savedPlayer;

    ULTIMA_1850_PrintString(_TEXT(0x2c58, "Entering room...\n"));

    D_6603_dungeonFacing = D_2c76_facingOfDir[D_6602_dungeonMoveDir];

    ULTIMA_1b16_ClearKbdBuffer();

    room = cell & 0xf;
    savedPlayer = D_587b_activePlayer;
    D_589e_curCombatant = D_587b_activePlayer = 0xff;

    level_f = D_5893_mapId - 0x21;
    if (level_f >= 1)
    {
        level_f--;
    }

    fileOffset_f = 0x1600 * level_f + room * 0x160;

    memset(D_ad14_combatMap, 0, 0x160);

    ULTIMA_256e_ReadFileFromDisk(/*0x2c6a*/ "dungeon.cbt", D_ad14_combatMap, 0x160, fileOffset_f);

    D_5894_savedMapId = D_5893_mapId;
    savedX = D_5896_mapX;
    savedY = D_5897_mapY;

    savedMonster1 = D_5c5a_actors[1];
    savedMonster2 = D_5c5a_actors[2];

    D_5893_mapId = 0xff;
    D_58a0_combatExitDir = 0;

    DNGLOOK_117e_PlaceCombatants(cell, 3);
    ULTIMA_5910_UpdateFrame();

    D_58a1_combatFlags = 0x82;

    if (COMBAT_0b94_MainLoop() == 0)
    {
        if (D_58a0_combatExitDir == 0x4d)
        {
            ENDGAME_0648_EndgameMain();
        }

        D_5893_mapId = D_5894_savedMapId;
        DNGLOOK_0844_MarkRoomCleared(room);
        GetDungeonMap((uint)savedX, (uint)savedY, (uint)D_5895_mapLevel) &= 0xaf;
    }

    D_5897_mapY = savedY;
    D_5896_mapX = savedX;
    D_5893_mapId = D_5894_savedMapId;

    DNGLOOK_0fda_StepAfterCombat();

    D_5c5a_actors[1] = savedMonster1;
    D_5c5a_actors[2] = savedMonster2;
    D_587b_activePlayer = savedPlayer;
}

static int DUNGEON_0252_PlaceMonster(void);

// the level's wandering monster (actor 1): with the flag, a new one (type of D_173c by a roll of 0-7, placed at random) and its picture loaded
void DUNGEON_0134_SetUpMonster(int respawn)
{
    int type_k;
    int oldValue;
    ActorFmt* monster;

    monster = &D_5c5a_actors[1];
    oldValue = D_5c5a_actors[1].value;
    if (respawn != 0)
    {
        type_k = ULTIMA_2092_RandomRange(0, 7);
        D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = type_k;
        D_5c5a_actors[1].extra = 0;
        D_5c5a_actors[1].animState = D_1744_monsterAnimStates[type_k];
        D_5c5a_actors[1].value = D_173c_monsterTypes[type_k];
        D_5c5a_actors[1].z = D_5895_mapLevel;
        if (oldValue != 0xff)
        {
            ULTIMA_0be4_FreeImage(D_a9c6_monsterImage);
        }
        D_a9c6_monsterImage = 0;
        if (DUNGEON_0252_PlaceMonster() == 0)
        {
            monster->tile = monster->animTile = 0;
            monster->value = 0xff;
        }
    }

    if (monster->value != 0xff)
    {
        do
        {
            D_a9c6_monsterImage = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[monster->tile + 8]);
        } while (D_a9c6_monsterImage == 0);
    }
}

// prints the level number and the facing (North, East, South, West) in the view's bars
void DUNGEON_01d2_PrintWalkDir(void)
{
    int level;
    int savedWindow_f;

    savedWindow_f = D_5386_textWindowIndex;
    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(0xc, 0);
    level = D_5895_mapLevel + 1;
    ULTIMA_1a3e_PrintNumber(level, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(0xc, 0x17);

    switch (D_6603_dungeonFacing)
    {
    case 0:
        ULTIMA_1850_PrintString(_TEXT(0x2c7c, "North"));
        break;
    case 1:
        ULTIMA_1850_PrintString(_TEXT(0x2c82, " East"));
        break;
    case 2:
        ULTIMA_1850_PrintString(_TEXT(0x2c88, "South"));
        break;
    case 3:
        ULTIMA_1850_PrintString(_TEXT(0x2c8e, " West"));
        break;
    default:
        ULTIMA_1850_PrintString(_TEXT(0x2c94, " ????"));
        break;
    }

    ULTIMA_1b94_SelectTextWindow(savedWindow_f);
}

// puts actors 1 and 2 on a random open cell (eight tries, not on the party's row and column); 1 when it found one
static int DUNGEON_0252_PlaceMonster(void)
{
    byte kind_b;
    int offset;
    int tries;
    byte* cellPtr_c;
    int cellY_h;
    int cellX_i;

    for (tries = 0; tries < 8; tries++)
    {
        // 0262
        cellPtr_c = &GetDungeonMap(0, 0, D_5895_mapLevel);
        offset = ULTIMA_2092_RandomRange(0, 0x3f);
        cellPtr_c += offset;
        kind_b = *cellPtr_c & 0xf0;
        if (kind_b < 0x60 || kind_b == 0x70)
        {
            // 0292
            cellY_h = offset / 8;
            cellX_i = offset % 8;
            if (cellX_i != D_5896_mapX && cellY_h != D_5897_mapY)
            {
                D_5c5a_actors[1].x = D_5c5a_actors[2].x = cellX_i;
                D_5c5a_actors[1].y = D_5c5a_actors[2].y = cellY_h;

                if ((D_5c5a_actors[1].value == 0x16 || D_5c5a_actors[1].value == 0x18) && (int)ULTIMA_2092_RandomRange(0, 99) > 0x30)
                {
                    D_5c5a_actors[1].extra = 0xff;
                }

                offset = 1;
                break;
            }
        }
    }

    if (tries == 8)
    {
        D_5c5a_actors[1].x = D_5c5a_actors[2].x = D_5c5a_actors[1].y = D_5c5a_actors[2].y = 0xff;
        offset = 0;
    }

    return offset;
}

// the view's top and bottom bars and the labels "L " (level) and "Dir:"
static void DUNGEON_0332_DrawFrame(void)
{
    int savedWindow;

    savedWindow = D_5386_textWindowIndex;
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x28, 0, 0x98, 7);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x30, 0xb9, 0x98, 0xbf);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(0x28, 7, 0x98, 7);
    ULTIMA_0b10_GRAP_Line(0x30, 0xb8, 0x98, 0xb8);
    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(10, 0);
    ULTIMA_4c2a_PrintLeftCap();
    ULTIMA_1850_PrintString(_TEXT(0x2c9a, "L "));
    ULTIMA_4cce_PrintRightCap();
    ULTIMA_1bf2_SetTextPosition(6, 0x17);
    ULTIMA_4c2a_PrintLeftCap();
    ULTIMA_1850_PrintString(_TEXT(0x2c9d, "Dir:      "));
    ULTIMA_4cce_PrintRightCap();
    ULTIMA_1b94_SelectTextWindow(savedWindow);
}

void DUNGEON_1020_StepSound(void);

// the prompt; while no key is there the view is redrawn (light flicker); returns the key (arrow keys as 0xfa+)
static int DUNGEON_03d6_WaitForKey(void)
{
    int key;
    int fullView_c;

    if (D_a9fa_vitalsDirty != 0)
    {
        ULTIMA_2900_UpdateVitalsDisplay();
        D_a9fa_vitalsDirty = 0;
    }

    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_4c2a_PrintLeftCap();

    fullView_c = 1;
    do
    {
        key = ULTIMA_2032_ToUpper(ULTIMA_1b38_PollKeyWithCursor());
        if (key != 0 && key < 5 && D_538a_numpadKeys == 0)
        {
            key += 0xfa;
        }

        if (key == 0)
        {
            DUNGEON_1a90_DrawView(fullView_c);
            if (fullView_c != 0)
            {
                ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x10, 0xe, 0xaf, 0xb2);
            }
            else
            {
                ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x28, 0xe, 0x96, 0xb2);
            }

            if (D_6604_dungeonStyle != 1)
            {
                fullView_c = 0;
            }

            DUNGEON_1020_StepSound();
        }
    } while (key == 0);

    return key;
}

// the party walks into an electric field: "Ouch!", flash, thrown back a step, party hurt
static void DUNGEON_0470_ElectricField(int direction)
{
    DUNGEON_1be0_Redraw();
    ULTIMA_1850_PrintString(_TEXT(0x2ca8, "Ouch!\n"));
    ULTIMA_1850_PrintString(_TEXT(0x2caf, "Electric field!\n"));
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_20fa_WaitTicks(1);
    ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_223c_AudioWhiteNoise(1, 500, 20000);
    if (direction > 0)
    {
        D_5896_mapX -= D_24d6_facingDx[D_6603_dungeonFacing];
        D_5897_mapY -= D_24de_facingDy[D_6603_dungeonFacing];
    }
    else
    {
        D_5896_mapX += D_24d6_facingDx[D_6603_dungeonFacing];
        D_5897_mapY += D_24de_facingDy[D_6603_dungeonFacing];
    }
    ULTIMA_2aa8_DamageParty();
    DUNGEON_1be0_Redraw();
}

// Advance / Turn right / Back up / Turn left / Turn around: blocked by walls and doors, an electric field (0x83) hurts; 1 when it moved
static int DUNGEON_0502_WalkCmd(int key, byte cell)
{
    byte destCell_j;
    int destY;
    int destX;
    int stepDir_f;
    int moved_f;

    stepDir_f = 0;
    moved_f = 0;

    switch (key)
    {
    case DIR_UP:
        // 0542
        ULTIMA_1850_PrintString(_TEXT(0x2cc0, "Advance\n"));
        stepDir_f = 1;
        break;

    case DIR_RIGHT:
        // 0628
        if ((cell & 0xf0) == 0xe0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2cc9, "Not in doorway!\n")); // -> 0666
        }
        else
        {
            // 0636
            ULTIMA_1850_PrintString(_TEXT(0x2cda, "Turn right\n"));
            D_6603_dungeonFacing = (D_6603_dungeonFacing + 1) & 3;
        }
        break;

    case DIR_DOWN:
        // 064a
        ULTIMA_1850_PrintString(_TEXT(0x2ce6, "Back up\n"));
        stepDir_f = -1;
        break;

    case DIR_LEFT:
        // 065a
        if ((cell & 0xf0) == 0xe0)
        {
            // 0666
            ULTIMA_1850_PrintString(_TEXT(0x2cef, "Not in doorway!\n"));
        }
        else
        {
            // 066e
            ULTIMA_1850_PrintString(_TEXT(0x2d00, "Turn left\n"));
            D_6603_dungeonFacing = (D_6603_dungeonFacing + 3) & 3;
        }
        break;

    default:
        // 0533
        ULTIMA_1850_PrintString(_TEXT(0x2d0b, "Turn around.\n"));
        D_6603_dungeonFacing = (D_6603_dungeonFacing + 2) & 3; // -> 0642 -> 054e
        break;
    }

    D_6602_dungeonMoveDir = D_6603_dungeonFacing;

    if (stepDir_f != 0)
    {
        moved_f = 1;
        destX = D_24d6_facingDx[D_6603_dungeonFacing] * stepDir_f + D_5896_mapX;
        if (destX < 0)
        {
            destX = 7;
        }

        if (destX > 7)
        {
            destX = 0;
        }

        destY = D_24de_facingDy[D_6603_dungeonFacing] * stepDir_f + D_5897_mapY;
        if (destY < 0)
        {
            destY = 7;
        }

        if (destY > 7)
        {
            destY = 0;
        }

        // 05bc
        destCell_j = GetDungeonMap(destX, destY, D_5895_mapLevel);
        if (destCell_j == 0x83)
        {
            D_5896_mapX = destX;
            D_5897_mapY = destY;
            DUNGEON_0470_ElectricField(stepDir_f);
            destX = D_5896_mapX;
            destY = D_5897_mapY;
        }

        // 05fb
        destCell_j &= 0xf0;
        if (destCell_j > 0xa0 && destCell_j < 0xe0 || stepDir_f == -1 && (destCell_j == 0xa0 || destCell_j == 0xf0))
        {
            // 061d
            ULTIMA_1850_PrintString(_TEXT(0x2d19, "Blocked!\n"));
        }
        else
        {
            // 067c~: OK
            if (destX == D_5c5a_actors[1].x && destY == D_5c5a_actors[1].y)
            {
                ULTIMA_1850_PrintString(_TEXT(0x2d23, "Blocked!\n"));
            }
            else
            {
                // 0694
                D_a9fb_stepVolume = 0xf;
                D_24e7_stepToggle = 0;
                DUNGEON_1020_StepSound();
                D_24e7_stepToggle = 0;
                D_5896_mapX = (byte)destX & 7;
                D_5897_mapY = (byte)destY & 7;
            }
        }
    }

    // 06b6
    DUNGEON_1be0_Redraw();
    return moved_f;
}

// one key of the dungeon: Ctrl-K karma, E exit, V version, S sound, movement, a number selects a player, else the common commands
static int DUNGEON_06c4_ProcessCommand(int key)
{
    byte cell;
    int result_b;

    result_b = 1;
    switch (key)
    {
    case U5_KEY_CTRL_K:
        // 06f2
        ULTIMA_1a3e_PrintNumber(D_5888_karma, 1, 0x20);
        ULTIMA_16ba_PrintChar('\n');
        break;

    case U5_KEY_CTRL_E:
        // 0710
        ULTIMA_1850_PrintString(_TEXT(0x2d2d, "Exit to DOS? "));
        key = ULTIMA_266c_GetChar();
        if (key == 0x59)
        {
            ULTIMA_0878_RestoreVideoMode();
            exit(0);
            // nop?
            break;
        }
        ULTIMA_1850_PrintString(_TEXT(0x2d3b, "N\n"));
        break;

    case U5_KEY_CTRL_V:
        // 073a
        ULTIMA_1850_PrintString(_TEXT(0x2d3e, U5_VERSION));
        ULTIMA_16ba_PrintChar('\n');
        break;

    case U5_KEY_LEFT:
    case U5_KEY_RIGHT:
    case U5_KEY_UP:
    case U5_KEY_DOWN:
    case U5_KEY_ENTER:
    case U5_KEY_PERIOD:
        // 0744
        cell = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        result_b = DUNGEON_0502_WalkCmd(key, cell);
        // ?
        break;

    case U5_KEY_CTRL_S:
        // 0776
        ULTIMA_1850_PrintString(_TEXT(0x2d43, "Sound "));
        if (D_a9ce_soundOn != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2d4a, "Off\n"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x2d4f, "On\n"));
        }
        D_a9ce_soundOn = !D_a9ce_soundOn;
        break;

    default:
        if ((uint)key >= 0x30 && (uint)key <= 0x39)
        {
            result_b = ULTIMA_4080_SetActivePlayer(key);
            result_b = 0;
        }
        else
        {
            result_b = ULTIMA_3178_ProcessCommand(key);
        }
        break;
    }

    return result_b;
}

// the wandering monster takes a random step (not into pits, fields or walls); 1 when it reaches the party
static int DUNGEON_07e2_MoveMonster(void)
{
    ActorFmt* previous;
    byte kind_g;
    int newY_j;
    int newX_k;
    int reached;
    int moved;
    int tries;
    int dir_i;
    int oldY_p;
    int oldX;
    ActorFmt* monster_i;

    tries = 0;
    reached = 0;
    moved = 999;
    monster_i = &D_5c5a_actors[1];
    previous = &D_5c5a_actors[2];

    if (D_5c5a_actors[1].value != 0xff)
    {
        if (monster_i->value != 0x1b)
        {
            // 0819
            for (tries = 0; tries < 8; tries++)
            {
                // 083e
                dir_i = ULTIMA_2092_RandomRange(0, 3);
                newX_k = D_24d6_facingDx[dir_i] + monster_i->x;
                if (newX_k > 7)
                {
                    newX_k = 0;
                }
                if (newX_k < 0)
                {
                    newX_k = 7;
                }

                // 0866
                newY_j = D_24de_facingDy[dir_i] + monster_i->y;
                if (newY_j > 7)
                {
                    newY_j = 0;
                }
                if (newY_j < 0)
                {
                    newY_j = 7;
                }

                // 0882
                kind_g = GetDungeonMap(newX_k, newY_j, D_5895_mapLevel);
                kind_g &= 0xf0;

                if (kind_g != 0x60 && kind_g != 0x80 && kind_g < 0xa0)
                {
                    oldX = previous->x;
                    oldY_p = previous->y;
                    if (newX_k != oldX || newY_j != oldY_p || ULTIMA_2092_RandomRange(0, 7) == 1)
                    {
                        moved = 900;
                        break;
                    }
                }
            }

            // 08d6
            if (moved == 900)
            {
                previous->x = monster_i->x;
                previous->y = monster_i->y;
                monster_i->x = (byte)newX_k;
                monster_i->y = (byte)newY_j;
            }
        }

        // 0917
        if (monster_i->x == D_5896_mapX && monster_i->y == D_5897_mapY)
        {
            reached = 1;
            monster_i->x = previous->x;
            monster_i->y = previous->y;
        }
    }
    return reached;
}

// a sleep field: each member may fall asleep (dexterity against a roll), the field is used up
static void DUNGEON_0948_SleepSpell(void)
{
    int member;

    ULTIMA_1850_PrintString(_TEXT(0x2d53, "Sleep spell!\n"));

    for (member = 0; member < D_585b_partyCount; member++)
    {
        if (D_55a8_party[member].dex <= ULTIMA_2092_RandomRange(1, 0x1e) && D_55a8_party[member].status != STATUS_DEAD)
        {
            D_55a8_party[member].status = STATUS_SLEEP;
            ULTIMA_2a28_InvertPlayerRow(member);
            ULTIMA_223c_AudioWhiteNoise(1, 0x32, 0xdac);
            D_a9fa_vitalsDirty = 1;
        }
    }

    GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 8;
}

// a poison field: each member may be poisoned
static void DUNGEON_09e6_Poison(void)
{
    int member;

    ULTIMA_1850_PrintString(_TEXT(0x2d61, "Poison!\n"));

    for (member = 0; member < D_585b_partyCount; member++)
    {
        if (D_55a8_party[member].dex <= ULTIMA_2092_RandomRange(1, 0x1e) && D_55a8_party[member].status != STATUS_DEAD)
        {
            D_55a8_party[member].status = STATUS_POISONED;
            ULTIMA_223c_AudioWhiteNoise(1, 0x32, 0xdac);
        }
    }
}

// a pit trap: falls a level at a time while the cell below is a pit too, "splat" damage; a room cell on arrival is entered
static void DUNGEON_0a4c_FallThroughPit(byte cell)
{
    D_6602_dungeonMoveDir = 5;

    while ((cell == 0x61 || cell == 0x69) && D_5895_mapLevel < 8)
    {
        ULTIMA_1850_PrintString(_TEXT(0x2d6a, "Pit Trap!\n"));
        ULTIMA_1850_PrintString(_TEXT(0x2d75, "Falling...\n"));

        GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 0xf8;

        D_5895_mapLevel++;

        cell = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        if (cell < 0x90)
        {
            cell = cell | 8;
            GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) = cell;
        }

        DUNGEON_1be0_Redraw();
        ULTIMA_1850_PrintString(_TEXT(0x2d81, "      ...splat!\n"));
        ULTIMA_2aa8_DamageParty();
    }

    if (D_5895_mapLevel == 8)
    {
        D_5893_mapId = 0;
    }

    if (D_5893_mapId != 0)
    {
        cell = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        cell &= 0xf0;
        if (cell == 0xa0 || cell == 0xf0)
        {
            DNGLOOK_1130_FreeDungeonImages();
            DUNGEON_0000_EnterRoom(GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel));
            if (D_5893_mapId == 0)
            {
                return;
            }

            DNGLOOK_109e_LoadDungeonImages(1);
        }

        DUNGEON_0134_SetUpMonster(1);
    }
}

// "Attacked from the ...!": the party turns to face the monster, then a fight
static void DUNGEON_0b7e_MonsterAttacks(void)
{
    ActorFmt* monster;

    ULTIMA_1850_PrintString(_TEXT(0x2d92, "Attacked"));

    monster = &D_5c5a_actors[2];

    if (((monster->x - 1) & 7) == D_5896_mapX)
    {
        D_6602_dungeonMoveDir = 1;
    }
    else if (((monster->x + 1) & 7) == D_5896_mapX)
    {
        D_6602_dungeonMoveDir = 3;
    }
    else if (((monster->y - 1) & 7) == D_5897_mapY)
    {
        D_6602_dungeonMoveDir = 2;
    }
    else
    {
        D_6602_dungeonMoveDir = 0;
    }

    if (D_6602_dungeonMoveDir != D_6603_dungeonFacing)
    {
        ULTIMA_1850_PrintString(_TEXT(0x2d9b, " from the "));

        switch (D_6602_dungeonMoveDir)
        {
        case 0:
            ULTIMA_1850_PrintString(_TEXT(0x2da6, "north"));
            break;
        case 1:
            ULTIMA_1850_PrintString(_TEXT(0x2dac, "east"));
            break;
        case 2:
            ULTIMA_1850_PrintString(_TEXT(0x2db1, "south"));
            break;
        case 3:
            ULTIMA_1850_PrintString(_TEXT(0x2db7, "west"));
            break;
        }

        D_6603_dungeonFacing = D_6602_dungeonMoveDir;
    }

    ULTIMA_1850_PrintString(_TEXT(0x2dbc, "!\n"));
    DUNGEON_1be0_Redraw();
    ULTIMA_1b16_ClearKbdBuffer();
    ULTIMA_20fa_WaitTicks(2);
    DNGLOOK_1130_FreeDungeonImages();
    D_58a0_combatExitDir = 0;
    D_58a1_combatFlags = 2;
    DNGLOOK_0d3e_SetUpDungeonFight();
    ULTIMA_5f86_SpecialMapHandler(2, D_5c5a_actors[1].value, 0);
    DNGLOOK_0fda_StepAfterCombat();
    DNGLOOK_109e_LoadDungeonImages(1);
    DUNGEON_0134_SetUpMonster(1);
    if (D_5893_mapId != 0)
    {
        DUNGEON_1be0_Redraw();
    }
}

// after a command that moved: sleepers may wake, the monster moves, a room is entered, a trap or field acts, the turn ends
void DUNGEON_0c76_AfterStep(byte cell, int monsterMoves)
{
    int attacked_h;
    byte woken_e;
    int member;
    byte kind;

    attacked_h = 0;
    kind = cell & 0xf0;
    woken_e = 0;

    for (member = 0; member < D_585b_partyCount; member++)
    {
        if (D_55a8_party[member].status == STATUS_SLEEP && ULTIMA_2092_RandomRange(0, 0x3f) < 4)
        {
            D_55a8_party[member].status = STATUS_GOOD;
            woken_e++;
        }
    }

    if (woken_e != 0)
    {
        ULTIMA_2900_UpdateVitalsDisplay();
    }

    if (monsterMoves != 0)
    {
        attacked_h = DUNGEON_07e2_MoveMonster();
    }

    if (attacked_h != 0)
    {
        DUNGEON_0b7e_MonsterAttacks();
        cell = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        kind = cell & 0xf0;
    }

    // 0d17
    if (kind == 0xf0 || kind == 0xa0)
    {
        // 0d23
        DNGLOOK_1130_FreeDungeonImages();

        // 0d26
        while ((kind == 0xf0 || kind == 0xa0))
        {
            // 0d32
            if (ULTIMA_39fc_GetFirstActivePartyMember() > -1)
            {
                DUNGEON_0000_EnterRoom(cell);
                if (D_5893_mapId == 0)
                {
                    break;
                }

                // 0d62
                cell = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
                kind = cell & 0xf0;
            }
            else
            {
                break;
            }
        }

        // 0d4a
        DNGLOOK_109e_LoadDungeonImages(1);

        if (D_5893_mapId > 0x20)
        {
            // 0d5b
            DUNGEON_1be0_Redraw();
        }
        // -> 0e22
    }
    else
    {
        switch (cell)
        {
        case 0x80:
        case 0x88:
            DUNGEON_0948_SleepSpell();
            break;

        case 0x81:
        case 0x89:
            DUNGEON_09e6_Poison();
            break;

        case 0x82:
        case 0x8a:
            ULTIMA_1850_PrintString(_TEXT(0x2dbf, "Fire!!\n"));
            ULTIMA_2aa8_DamageParty();
            break;

        case 0x62:
        case 0x6a:
            ULTIMA_1850_PrintString(_TEXT(0x2dc7, "Bomb Trap!\n"));
            ULTIMA_1850_PrintString(_TEXT(0x2dd3, "KABOOM!!\n"));
            GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 8;
            ULTIMA_2aa8_DamageParty();
            break;

        case 0x61:
        case 0x69:
            DUNGEON_0a4c_FallThroughPit(cell);
            break;
        }

        ULTIMA_2900_UpdateVitalsDisplay();
    }

    ULTIMA_2ae8_EndTurn();
}

// the dungeon's overlay entry and command loop: sets the dungeon's wall and floor tiles by its number, loads the pictures, runs until the party leaves or dies
void DUNGEON_0e2e_MainLoop(int fresh)
{
    int awake;
    int running_i;
    int turnTaken_n;
    int key;
    u8 kind;
    int quickToggle_b;
    u8 cell;

    running_i = 1;
    awake = 0;
    quickToggle_b = 0;
    D_bb17_loadFlags = 2;
    DNGLOOK_093a_ApplyClearedRooms();
    kind = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    kind &= 0xf0;
    DUNGEON_0332_DrawFrame();
    DUNGEON_01d2_PrintWalkDir();
    key = D_5893_mapId - 0x20;
    if (key == 1 || key == 4 || key == 5)
    {
        // 0e95
        D_bb14_wallTile = 0x4f;
        D_bb15_floorTile = 0x45;
        D_6604_dungeonStyle = 3;
    }
    else
    {
        // 0ea6
        D_bb14_wallTile = 0x4d;
        D_bb15_floorTile = 5;
        if (key == 6 || key == 7)
        {
            // 0ebc
            D_6604_dungeonStyle = 2;
        }
        else
        {
            // 0ec4
            D_6604_dungeonStyle = 1;
        }
    }

    // 0ec9
    if (kind == 0xa0 || kind == 0xf0)
    {
        // 0ed5
        DNGLOOK_1130_FreeDungeonImages();
        DUNGEON_0000_EnterRoom(GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel));
    }

    if (D_5893_mapId > 0x20)
    {
        DNGLOOK_109e_LoadDungeonImages(fresh);
        DUNGEON_0134_SetUpMonster(fresh);
        DUNGEON_1be0_Redraw();
    }

    // 0f15
    while (running_i != 0 && D_5893_mapId > 0x20) // 0f93
    {
        running_i = 1;
        turnTaken_n = 1;
        awake = ULTIMA_39fc_GetFirstActivePartyMember();
        if (awake == turnTaken_n)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a_PrintLeftCap();
            ULTIMA_1850_PrintString(_TEXT(0x2ddd, "Zzzzzz...\n"));
        }

        // 0fc3
        if (awake < 0)
        {
            running_i = 0;
            turnTaken_n = 0;
        }

        // 0fd0
        if (awake != 0)
            continue;

        if (D_587a_statusIcon != 'T' && D_587a_statusIcon != 'Q')
        {
            // 0fea -> 0f2e
            quickToggle_b = 1;
            ULTIMA_4f7c_AdvanceTime(1);
        }
        else
        {
            if (D_587a_statusIcon == 'Q')
            {
                quickToggle_b ^= 1;
                if (quickToggle_b != 0)
                {
                    ULTIMA_4f7c_AdvanceTime(1);
                }
            }
            else
            {
                quickToggle_b = 0;
            }
        }

        // 0f36
        key = DUNGEON_03d6_WaitForKey();
        if (key > -1)
        {
            turnTaken_n = DUNGEON_06c4_ProcessCommand(key);
        }

        if (D_5893_mapId < 0x21)
        {
            turnTaken_n = 0;
            running_i = turnTaken_n;
        }

        // 0f53
        cell = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel & 0xff);
        kind = cell & 0xf0;
        if (turnTaken_n != 0)
        {
            DUNGEON_0c76_AfterStep(cell, quickToggle_b);
        }

        if (D_5893_mapId >= 0x21)
        {
            continue;
        }

        turnTaken_n = 0;
        running_i = turnTaken_n;
    }

    // 0ff2
    if ((D_bb17_loadFlags & 2) != 0)
    {
        D_bb17_loadFlags &= 0xfb;
    }
    else
    {
        D_bb17_loadFlags |= 4;
    }
    // 100b
    DNGLOOK_1130_FreeDungeonImages();

    if (awake < 0)
    {
        BLCKTHRN_0910_Death();
    }
}

// every other step a noise of volume D_a9fb that dies down by 4
void DUNGEON_1020_StepSound(void)
{
    D_24e7_stepToggle ^= 1;
    if (D_24e7_stepToggle != 0 && D_a9fb_stepVolume != 0)
    {
        ULTIMA_223c_AudioWhiteNoise(1, D_a9fb_stepVolume, 20000);
        ULTIMA_3f36_DecreaseByte(&D_a9fb_stepVolume, 4);
    }
}
