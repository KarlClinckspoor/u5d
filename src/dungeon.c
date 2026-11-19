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

// defined in dungeonb.c, the part of DUNGEON.OVL built with MSC 5.0
void DrawView(int fullView);
void Redraw(void);

void LoadDungeonImages(int clearEncounter);

u8 facingOfDir[6] = {0x00, 0x01, 0x02, 0x03, 0x00, 0x02};

// "Entering room...": a cell of kind 0xa? / 0xf? opens its room: the combat map is read from dungeon.cbt, the party fights; a won room is marked cleared
static void EnterRoom(byte cell)
{
    int level_f;
    int room;
    ActorFmt savedMonster2;

    ActorFmt savedMonster1;
    int fileOffset_f;

    int savedY;
    int savedX;
    int savedPlayer;

    ULTIMA_1850_PrintString(/*0x2c58*/ "Entering room...\n");

    dungeonFacing = facingOfDir[dungeonMoveDir];

    ULTIMA_1b16_ClearKbdBuffer();

    room = cell & 0xf;
    savedPlayer = activePlayer;
    curCombatant = activePlayer = 0xff;

    level_f = D_5893_mapId - 0x21;
    if (level_f >= 1)
    {
        level_f--;
    }

    fileOffset_f = 0x1600 * level_f + room * 0x160;

    memset(combatMap, 0, 0x160);

    ULTIMA_256e_ReadFileFromDisk(/*0x2c6a*/ "dungeon.cbt", combatMap, 0x160, fileOffset_f);

    D_5894_savedMapId = D_5893_mapId;
    savedX = D_5896_mapX;
    savedY = D_5897_mapY;

    savedMonster1 = D_5c5a_actors[1];
    savedMonster2 = D_5c5a_actors[2];

    D_5893_mapId = 0xff;
    combatExitDir = 0;

    PlaceCombatants(cell, 3);
    ULTIMA_5910_UpdateFrame();

    combatFlags = 0x82;

    if (CombatMainLoop() == 0)
    {
        if (combatExitDir == 0x4d)
        {
            EndgameMain();
        }

        D_5893_mapId = D_5894_savedMapId;
        MarkRoomCleared(room);
        GetDungeonMap((uint)savedX, (uint)savedY, (uint)D_5895_mapLevel) &= 0xaf;
    }

    D_5897_mapY = savedY;
    D_5896_mapX = savedX;
    D_5893_mapId = D_5894_savedMapId;

    StepAfterCombat();

    D_5c5a_actors[1] = savedMonster1;
    D_5c5a_actors[2] = savedMonster2;
    activePlayer = savedPlayer;
}

static int PlaceMonster(void);

// the level's wandering monster (actor 1): with the flag, a new one (type of D_173c by a roll of 0-7, placed at random) and its picture loaded
void SetUpMonster(int respawn)
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
        D_5c5a_actors[1].animState = monsterAnimStates[type_k];
        D_5c5a_actors[1].value = monsterTypes[type_k];
        D_5c5a_actors[1].z = D_5895_mapLevel;
        if (oldValue != 0xff)
        {
            FreeImage(monsterImage);
        }
        monsterImage = 0;
        if (PlaceMonster() == 0)
        {
            monster->tile = monster->animTile = 0;
            monster->value = 0xff;
        }
    }

    if (monster->value != 0xff)
    {
        do
        {
            monsterImage = LoadImageFile(fileNames[monster->tile + 8]);
        } while (monsterImage == 0);
    }
}

// prints the level number and the facing (North, East, South, West) in the view's bars
void PrintWalkDir(void)
{
    int level;
    int savedWindow_f;

    savedWindow_f = textWindowIndex;
    SelectTextWindow(0);
    SetTextPosition(0xc, 0);
    level = D_5895_mapLevel + 1;
    PrintNumber(level, 1, 0x20);
    SetTextPosition(0xc, 0x17);

    switch (dungeonFacing)
    {
    case 0:
        ULTIMA_1850_PrintString(/*0x2c7c*/ "North");
        break;
    case 1:
        ULTIMA_1850_PrintString(/*0x2c82*/ " East");
        break;
    case 2:
        ULTIMA_1850_PrintString(/*0x2c88*/ "South");
        break;
    case 3:
        ULTIMA_1850_PrintString(/*0x2c8e*/ " West");
        break;
    default:
        ULTIMA_1850_PrintString(/*0x2c94*/ " ????");
        break;
    }

    SelectTextWindow(savedWindow_f);
}

// puts actors 1 and 2 on a random open cell (eight tries, not on the party's row and column); 1 when it found one
static int PlaceMonster(void)
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
static void DungeonDrawFrame(void)
{
    int savedWindow;

    savedWindow = textWindowIndex;
    ULTIMA_0a70_GRAP_2d_SetPenColor(blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x28, 0, 0x98, 7);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x30, 0xb9, 0x98, 0xbf);
    ULTIMA_0a70_GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(0x28, 7, 0x98, 7);
    GRAP_Line(0x30, 0xb8, 0x98, 0xb8);
    SelectTextWindow(0);
    SetTextPosition(10, 0);
    ULTIMA_4c2a_PrintLeftCap();
    ULTIMA_1850_PrintString(/*0x2c9a*/ "L ");
    PrintRightCap();
    SetTextPosition(6, 0x17);
    ULTIMA_4c2a_PrintLeftCap();
    ULTIMA_1850_PrintString(/*0x2c9d*/ "Dir:      ");
    PrintRightCap();
    SelectTextWindow(savedWindow);
}

void StepSound(void);

// the prompt; while no key is there the view is redrawn (light flicker); returns the key (arrow keys as 0xfa+)
static int DungeonWaitForKey(void)
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
        key = ToUpperChar(PollKeyWithCursor());
        if (key != 0 && key < 5 && numpadKeys == 0)
        {
            key += 0xfa;
        }

        if (key == 0)
        {
            DrawView(fullView_c);
            if (fullView_c != 0)
            {
                GRAP_18_TransferArea(1, 0, 0x10, 0xe, 0xaf, 0xb2);
            }
            else
            {
                GRAP_18_TransferArea(1, 0, 0x28, 0xe, 0x96, 0xb2);
            }

            if (dungeonStyle != 1)
            {
                fullView_c = 0;
            }

            StepSound();
        }
    } while (key == 0);

    return key;
}

// the party walks into an electric field: "Ouch!", flash, thrown back a step, party hurt
static void ElectricField(int direction)
{
    Redraw();
    ULTIMA_1850_PrintString(/*0x2ca8*/ "Ouch!\n");
    ULTIMA_1850_PrintString(/*0x2caf*/ "Electric field!\n");
    ULTIMA_0a70_GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    WaitTicks(1);
    GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_223c_AudioWhiteNoise(1, 500, 20000);
    if (direction > 0)
    {
        D_5896_mapX -= facingDx[dungeonFacing];
        D_5897_mapY -= facingDy[dungeonFacing];
    }
    else
    {
        D_5896_mapX += facingDx[dungeonFacing];
        D_5897_mapY += facingDy[dungeonFacing];
    }
    ULTIMA_2aa8_DamageParty();
    Redraw();
}

// Advance / Turn right / Back up / Turn left / Turn around: blocked by walls and doors, an electric field (0x83) hurts; 1 when it moved
static int WalkCmd(int key, byte cell)
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
        ULTIMA_1850_PrintString(/*0x2cc0*/ "Advance\n");
        stepDir_f = 1;
        break;

    case DIR_RIGHT:
        // 0628
        if ((cell & 0xf0) == 0xe0)
        {
            ULTIMA_1850_PrintString(/*0x2cc9*/ "Not in doorway!\n"); // -> 0666
        }
        else
        {
            // 0636
            ULTIMA_1850_PrintString(/*0x2cda*/ "Turn right\n");
            dungeonFacing = (dungeonFacing + 1) & 3;
        }
        break;

    case DIR_DOWN:
        // 064a
        ULTIMA_1850_PrintString(/*0x2ce6*/ "Back up\n");
        stepDir_f = -1;
        break;

    case DIR_LEFT:
        // 065a
        if ((cell & 0xf0) == 0xe0)
        {
            // 0666
            ULTIMA_1850_PrintString(/*0x2cef*/ "Not in doorway!\n");
        }
        else
        {
            // 066e
            ULTIMA_1850_PrintString(/*0x2d00*/ "Turn left\n");
            dungeonFacing = (dungeonFacing + 3) & 3;
        }
        break;

    default:
        // 0533
        ULTIMA_1850_PrintString(/*0x2d0b*/ "Turn around.\n");
        dungeonFacing = (dungeonFacing + 2) & 3; // -> 0642 -> 054e
        break;
    }

    dungeonMoveDir = dungeonFacing;

    if (stepDir_f != 0)
    {
        moved_f = 1;
        destX = facingDx[dungeonFacing] * stepDir_f + D_5896_mapX;
        if (destX < 0)
        {
            destX = 7;
        }

        if (destX > 7)
        {
            destX = 0;
        }

        destY = facingDy[dungeonFacing] * stepDir_f + D_5897_mapY;
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
            ElectricField(stepDir_f);
            destX = D_5896_mapX;
            destY = D_5897_mapY;
        }

        // 05fb
        destCell_j &= 0xf0;
        if (destCell_j > 0xa0 && destCell_j < 0xe0 || stepDir_f == -1 && (destCell_j == 0xa0 || destCell_j == 0xf0))
        {
            // 061d
            ULTIMA_1850_PrintString(/*0x2d19*/ "Blocked!\n");
        }
        else
        {
            // 067c~: OK
            if (destX == D_5c5a_actors[1].x && destY == D_5c5a_actors[1].y)
            {
                ULTIMA_1850_PrintString(/*0x2d23*/ "Blocked!\n");
            }
            else
            {
                // 0694
                stepVolume = 0xf;
                stepToggle = 0;
                StepSound();
                stepToggle = 0;
                D_5896_mapX = (byte)destX & 7;
                D_5897_mapY = (byte)destY & 7;
            }
        }
    }

    // 06b6
    Redraw();
    return moved_f;
}

// one key of the dungeon: Ctrl-K karma, E exit, V version, S sound, movement, a number selects a player, else the common commands
static int DungeonProcessCommand(int key)
{
    byte cell;
    int result_b;

    result_b = 1;
    switch (key)
    {
    case U5_KEY_CTRL_K:
        // 06f2
        PrintNumber(D_5888_karma, 1, 0x20);
        ULTIMA_16ba_PrintChar('\n');
        break;

    case U5_KEY_CTRL_E:
        // 0710
        ULTIMA_1850_PrintString(/*0x2d2d*/ "Exit to DOS? ");
        key = ULTIMA_266c_GetChar();
        if (key == 0x59)
        {
            RestoreVideoMode();
            exit(0);
            // nop?
            break;
        }
        ULTIMA_1850_PrintString(/*0x2d3b*/ "N\n");
        break;

    case U5_KEY_CTRL_V:
        // 073a
        ULTIMA_1850_PrintString(/*0x2d3e*/ "1.16");
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
        result_b = WalkCmd(key, cell);
        // ?
        break;

    case U5_KEY_CTRL_S:
        // 0776
        ULTIMA_1850_PrintString(/*0x2d43*/ "Sound ");
        if (D_a9ce_soundOn != 0)
        {
            ULTIMA_1850_PrintString(/*0x2d4a*/ "Off\n");
        }
        else
        {
            ULTIMA_1850_PrintString(/*0x2d4f*/ "On\n");
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
            result_b = ProcessCommand(key);
        }
        break;
    }

    return result_b;
}

// the wandering monster takes a random step (not into pits, fields or walls); 1 when it reaches the party
static int MoveMonster(void)
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
                newX_k = facingDx[dir_i] + monster_i->x;
                if (newX_k > 7)
                {
                    newX_k = 0;
                }
                if (newX_k < 0)
                {
                    newX_k = 7;
                }

                // 0866
                newY_j = facingDy[dir_i] + monster_i->y;
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
static void SleepSpell(void)
{
    int member;

    ULTIMA_1850_PrintString(/*0x2d53*/ "Sleep spell!\n");

    for (member = 0; member < D_585b_partyCount; member++)
    {
        if (D_55a8_party[member].dex <= ULTIMA_2092_RandomRange(1, 0x1e) && D_55a8_party[member].status != STATUS_DEAD)
        {
            D_55a8_party[member].status = STATUS_SLEEP;
            InvertPlayerRow(member);
            ULTIMA_223c_AudioWhiteNoise(1, 0x32, 0xdac);
            D_a9fa_vitalsDirty = 1;
        }
    }

    GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 8;
}

// a poison field: each member may be poisoned
static void DungeonPoison(void)
{
    int member;

    ULTIMA_1850_PrintString(/*0x2d61*/ "Poison!\n");

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
static void FallThroughPit(byte cell)
{
    dungeonMoveDir = 5;

    while ((cell == 0x61 || cell == 0x69) && D_5895_mapLevel < 8)
    {
        ULTIMA_1850_PrintString(/*0x2d6a*/ "Pit Trap!\n");
        ULTIMA_1850_PrintString(/*0x2d75*/ "Falling...\n");

        GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 0xf8;

        D_5895_mapLevel++;

        cell = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        if (cell < 0x90)
        {
            cell = cell | 8;
            GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) = cell;
        }

        Redraw();
        ULTIMA_1850_PrintString(/*0x2d81*/ "      ...splat!\n");
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
            FreeDungeonImages();
            EnterRoom(GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel));
            if (D_5893_mapId == 0)
            {
                return;
            }

            LoadDungeonImages(1);
        }

        SetUpMonster(1);
    }
}

// "Attacked from the ...!": the party turns to face the monster, then a fight
static void MonsterAttacks(void)
{
    ActorFmt* monster;

    ULTIMA_1850_PrintString(/*0x2d92*/ "Attacked");

    monster = &D_5c5a_actors[2];

    if (((monster->x - 1) & 7) == D_5896_mapX)
    {
        dungeonMoveDir = 1;
    }
    else if (((monster->x + 1) & 7) == D_5896_mapX)
    {
        dungeonMoveDir = 3;
    }
    else if (((monster->y - 1) & 7) == D_5897_mapY)
    {
        dungeonMoveDir = 2;
    }
    else
    {
        dungeonMoveDir = 0;
    }

    if (dungeonMoveDir != dungeonFacing)
    {
        ULTIMA_1850_PrintString(/*0x2d9b*/ " from the ");

        switch (dungeonMoveDir)
        {
        case 0:
            ULTIMA_1850_PrintString(/*0x2da6*/ "north");
            break;
        case 1:
            ULTIMA_1850_PrintString(/*0x2dac*/ "east");
            break;
        case 2:
            ULTIMA_1850_PrintString(/*0x2db1*/ "south");
            break;
        case 3:
            ULTIMA_1850_PrintString(/*0x2db7*/ "west");
            break;
        }

        dungeonFacing = dungeonMoveDir;
    }

    ULTIMA_1850_PrintString(/*0x2dbc*/ "!\n");
    Redraw();
    ULTIMA_1b16_ClearKbdBuffer();
    WaitTicks(2);
    FreeDungeonImages();
    combatExitDir = 0;
    combatFlags = 2;
    SetUpDungeonFight();
    SpecialMapHandler(2, D_5c5a_actors[1].value, 0);
    StepAfterCombat();
    LoadDungeonImages(1);
    SetUpMonster(1);
    if (D_5893_mapId != 0)
    {
        Redraw();
    }
}

// after a command that moved: sleepers may wake, the monster moves, a room is entered, a trap or field acts, the turn ends
void AfterStep(byte cell, int monsterMoves)
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
        attacked_h = MoveMonster();
    }

    if (attacked_h != 0)
    {
        MonsterAttacks();
        cell = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        kind = cell & 0xf0;
    }

    // 0d17
    if (kind == 0xf0 || kind == 0xa0)
    {
        // 0d23
        FreeDungeonImages();

        // 0d26
        while ((kind == 0xf0 || kind == 0xa0))
        {
            // 0d32
            if (GetFirstActivePartyMember() > -1)
            {
                EnterRoom(cell);
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
        LoadDungeonImages(1);

        if (D_5893_mapId > 0x20)
        {
            // 0d5b
            Redraw();
        }
        // -> 0e22
    }
    else
    {
        switch (cell)
        {
        case 0x80:
        case 0x88:
            SleepSpell();
            break;

        case 0x81:
        case 0x89:
            DungeonPoison();
            break;

        case 0x82:
        case 0x8a:
            ULTIMA_1850_PrintString(/*0x2dbf*/ "Fire!!\n");
            ULTIMA_2aa8_DamageParty();
            break;

        case 0x62:
        case 0x6a:
            ULTIMA_1850_PrintString(/*0x2dc7*/ "Bomb Trap!\n");
            ULTIMA_1850_PrintString(/*0x2dd3*/ "KABOOM!!\n");
            GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 8;
            ULTIMA_2aa8_DamageParty();
            break;

        case 0x61:
        case 0x69:
            FallThroughPit(cell);
            break;
        }

        ULTIMA_2900_UpdateVitalsDisplay();
    }

    ULTIMA_2ae8_EndTurn();
}

// the dungeon's overlay entry and command loop: sets the dungeon's wall and floor tiles by its number, loads the pictures, runs until the party leaves or dies
void DungeonMainLoop(int fresh)
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
    loadFlags = 2;
    ApplyClearedRooms();
    kind = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    kind &= 0xf0;
    DungeonDrawFrame();
    PrintWalkDir();
    key = D_5893_mapId - 0x20;
    if (key == 1 || key == 4 || key == 5)
    {
        // 0e95
        wallTile = 0x4f;
        D_bb15_floorTile = 0x45;
        dungeonStyle = 3;
    }
    else
    {
        // 0ea6
        wallTile = 0x4d;
        D_bb15_floorTile = 5;
        if (key == 6 || key == 7)
        {
            // 0ebc
            dungeonStyle = 2;
        }
        else
        {
            // 0ec4
            dungeonStyle = 1;
        }
    }

    // 0ec9
    if (kind == 0xa0 || kind == 0xf0)
    {
        // 0ed5
        FreeDungeonImages();
        EnterRoom(GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel));
    }

    if (D_5893_mapId > 0x20)
    {
        LoadDungeonImages(fresh);
        SetUpMonster(fresh);
        Redraw();
    }

    // 0f15
    while (running_i != 0 && D_5893_mapId > 0x20) // 0f93
    {
        running_i = 1;
        turnTaken_n = 1;
        awake = GetFirstActivePartyMember();
        if (awake == turnTaken_n)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a_PrintLeftCap();
            ULTIMA_1850_PrintString(/*0x2ddd*/ "Zzzzzz...\n");
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

        if (statusIcon != 'T' && statusIcon != 'Q')
        {
            // 0fea -> 0f2e
            quickToggle_b = 1;
            ULTIMA_4f7c_AdvanceTime(1);
        }
        else
        {
            if (statusIcon == 'Q')
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
        key = DungeonWaitForKey();
        if (key > -1)
        {
            turnTaken_n = DungeonProcessCommand(key);
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
            AfterStep(cell, quickToggle_b);
        }

        if (D_5893_mapId >= 0x21)
        {
            continue;
        }

        turnTaken_n = 0;
        running_i = turnTaken_n;
    }

    // 0ff2
    if ((loadFlags & 2) != 0)
    {
        loadFlags &= 0xfb;
    }
    else
    {
        loadFlags |= 4;
    }
    // 100b
    FreeDungeonImages();

    if (awake < 0)
    {
        Death();
    }
}

// every other step a noise of volume D_a9fb that dies down by 4
void StepSound(void)
{
    stepToggle ^= 1;
    if (stepToggle != 0 && stepVolume != 0)
    {
        ULTIMA_223c_AudioWhiteNoise(1, stepVolume, 20000);
        ULTIMA_3f36_DecreaseByte(&stepVolume, 4);
    }
}
