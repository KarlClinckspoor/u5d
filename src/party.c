#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <stdlib.h>
#include <time.h>

int ULTIMA_266c_GetChar(void)
{
    int key;

    if (D_52c8_videoDriver == 2)
    {
        animate = 0xff;
    }

    do
    {
        key = ToUpperChar(PollKeyWithCursor());
        if ((key == 0) && (D_5893_mapId < 0x21 || 0x7f < D_5893_mapId))
        {
            ULTIMA_5910_UpdateFrame();
        }
    } while (key == 0);

    if (numpadKeys != 0)
    {
        // numpad
        switch (key)
        {
        case 0x31:
            key = U5_KEY_END;
            break;
        case 0x32:
            key = U5_KEY_DOWN;
            break;
        case 0x33:
            key = U5_KEY_PGDN;
            break;
        case 0x34:
            key = U5_KEY_LEFT;
            break;
        case 0x36:
            key = U5_KEY_RIGHT;
            break;
        case 0x37:
            key = U5_KEY_HOME;
            break;
        case 0x38:
            key = U5_KEY_UP;
            break;
        case 0x39:
            key = U5_KEY_PGUP;
        }
    }
    else
    {
        if (key < 5)
            key += 0xfa;
    }

    return key;
}

// Print row rowIndex of the party list: name, active-player marker, hit points (and the combat status).
static void PrintPartyRow(int rowIndex)
{
    int     col;
    CombatEntity* combatant;

    SetTextPosition(0, rowIndex);

    if (D_585b_partyCount <= rowIndex)
    {
        for (col = 0; col < 0xf; col++)
        {
            // 2748
            ULTIMA_16ba_PrintChar(' ');
        }

        return;
    }

    // 2758
    combatant = &combatants[curCombatant];
    if (D_5893_mapId > 0x7f && curCombatant != 0xff && (combatant->flags & COMBAT_FLAGS_PLAYER) != 0 && combatant->entityIdx == rowIndex)
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    }

    // 278e
    ULTIMA_1850_PrintString(D_55a8_party[rowIndex].name);
    for (col = StringLength(D_55a8_party[rowIndex].name); col < 9; col++)
    {
        // 27b8
        ULTIMA_16ba_PrintChar(' ');
    }

    // 27c2
    if (rowIndex == activePlayer)
    {
        if (D_55a8_party[rowIndex].status != STATUS_DEAD && D_55a8_party[rowIndex].status != STATUS_SLEEP)
        {
            ULTIMA_16ba_PrintChar(0x1a);
        }
        else
        {
            ULTIMA_16ba_PrintChar(' ');
            activePlayer = 0xff;
        }
    }
    else
    {
        // 27f6
        ULTIMA_16ba_PrintChar(' ');
    }

    // 27fd
    PrintNumber(D_55a8_party[rowIndex].hp, 4, 0x20);

    if (D_5893_mapId > 0x7f)
    {
        if ((combatants[rowIndex].flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER | COMBAT_FLAGS_DEAD | COMBAT_FLAGS_CHARMED)) ==
            (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_CHARMED) &&
            combatants[rowIndex].entityIdx == rowIndex)
        {
            // -> 2849
            ULTIMA_16ba_PrintChar('C');
        }
        else
        {
            // 283c -> 2849
            ULTIMA_16ba_PrintChar(D_55a8_party[rowIndex].status);
        }

        if (curCombatant != 0xff && (combatant->flags & COMBAT_FLAGS_PLAYER) != 0)
        {
            if (combatant->entityIdx == rowIndex)
            {
                ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            }
        }

        // 287d
        return;
    }

    // 286c
    ULTIMA_16ba_PrintChar(D_55a8_party[rowIndex].status);
}

void UpdateGoldDisplay(void)
{
    int savedWindow;

    savedWindow = textWindowIndex;
    SelectTextWindow(1);
    SetTextPosition(8, 7);
    if (gold < 1000)
    {
        ULTIMA_16ba_PrintChar(' ');
    }
    if (gold < 100)
    {
        ULTIMA_16ba_PrintChar(' ');
    }
    if (gold < 10)
    {
        ULTIMA_16ba_PrintChar(' ');
    }
    ULTIMA_1850_PrintString(/*0x54b6*/ " G:");
    PrintNumber(gold, 1, ' ');
    while (0xf > GetCurrentTextX())
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    SelectTextWindow(savedWindow);
}

void ULTIMA_2900_UpdateVitalsDisplay(void)
{
    int row;

    SelectTextWindow(1);

    row = 0;
    do
    {
        PrintPartyRow(row);
        row++;
    } while (row < 6);

    SetTextPosition(0, 7);

    // food
    ULTIMA_1850_PrintString(/*0x54ba*/ "F:");
    PrintNumber(food, 1, ' ');

    while (GetCurrentTextX() < 8)
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    if ((D_5893_mapId < 0x80) && ((D_587c_partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20))
    {
        // ship
        ULTIMA_1850_PrintString(/*0x54bd*/ "Ship:");
        PrintNumber(D_5c5a_actors[0].value, 1, ' ');
        if (D_5c5a_actors[0].value < 10)
        {
            ULTIMA_16ba_PrintChar(' ');
        }
    }
    else
    {
        UpdateGoldDisplay();
    }

    ULTIMA_1850_PrintString(/*0x54c3*/ "\n   ");

    if ((D_587d_month < 10) && (D_587e_day < 10))
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    // date (m/d/y)
    PrintNumber(D_587d_month, 1, ' ');
    ULTIMA_16ba_PrintChar('-');
    PrintNumber(D_587e_day, 1, ' ');
    ULTIMA_16ba_PrintChar('-');
    PrintNumber(year, 3, '0');

    while (GetCurrentTextX() < 0xf)
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    SetTextPosition(6, 6);

    // icon?
    if (statusIcon != 0)
    {
        ULTIMA_4c2a_PrintLeftCap();
        ULTIMA_16ba_PrintChar(statusIcon);
        PrintRightCap();
    }
    else
    {
        DrawStatsDivider();
    }

    SelectTextWindow(2);
}

// Invert the party member's row in the stats window (the selection bar, the flash of a hit).
void InvertPlayerRow(int playerIndex)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_XorFillRect(0xc0, playerIndex * 8 + 8, 0x137, playerIndex * 8 + 0xf);
}

// process damage (show fx, reduce hp)
// Hurt a party member: flash and noise, hit points down by `damage`, dead at 0.
void DamagePlayer(int playerIndex, uint damage)
{
    InvertPlayerRow(playerIndex);
    ULTIMA_223c_AudioWhiteNoise(10, 0x640, 2000);
    InvertPlayerRow(playerIndex);

    D_55a8_party[playerIndex].hp -= damage;
    if (D_55a8_party[playerIndex].hp <= 0)
    {
        D_55a8_party[playerIndex].hp = 0;
        D_55a8_party[playerIndex].status = STATUS_DEAD;
        if (playerIndex == activePlayer)
        {
            activePlayer = 0xff;
        }
    }

    ULTIMA_2900_UpdateVitalsDisplay();
}

// process party damage
// Hurt every living party member by 1-8 points.
void ULTIMA_2aa8_DamageParty(void)
{
    int i;
    for (i = 0; i < 6; i++)
    {
        if (i < D_585b_partyCount && D_55a8_party[i].status != STATUS_DEAD)
        {
            DamagePlayer(i, ULTIMA_2092_RandomRange(1, 8));
        }
    }
}

void IncreaseByte(byte* value, int amount, int limit);

// CHECKED
// The end of a turn: poison, food (three meals a day, starving without), the turn count, the protection spell, the regeneration rings.
void ULTIMA_2ae8_EndTurn(void)
{
    int playerIndex_l;
    int status;
    int ableCount_d;

    ableCount_d = 0;

    for (playerIndex_l = 0; playerIndex_l < D_585b_partyCount; playerIndex_l++)
    {
        status = D_55a8_party[playerIndex_l].status;
        if ((uint)status == STATUS_DEAD && playerIndex_l == activePlayer)
        {
            activePlayer = 0xff;
        }

        if ((uint)status != STATUS_DEAD && (uint)status != STATUS_SLEEP)
        {
            if ((uint)status == STATUS_POISONED)
            {
                DamagePlayer(playerIndex_l, 1);
            }

            ableCount_d++;
        }
    }

    if (D_587f_hour != lastHour)
    {
        if (food == 0)
        {
            ULTIMA_1850_PrintString(/*0x54c8*/ "Starving!\n");
            ULTIMA_2aa8_DamageParty();
        }
        else if (D_587f_hour == 6 || D_587f_hour == 12 || D_587f_hour == 18)
        {
            DecreaseInt(&food, ableCount_d);
        }
        lastHour = D_587f_hour;
    }

    IncreaseByte(&turnCount, 1, 0xff);
    if (protectTurns != 0 && protectTurns != 0xff)
    {
        protectTurns--;
        if (protectTurns == 0)
        {
            statusIcon = 0;
            ULTIMA_2900_UpdateVitalsDisplay();
        }
    }

    RingRegeneration();
}

// IsWalkableTile's tables, in this module's _DATA between its strings (DS:54d4)
u8 blockedTileBits[32] =
{
    0x70, 0x0c, 0x00, 0x28, 0x01, 0xf3, 0x00, 0xbd,
    0x72, 0x3f, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff,
    0xfc, 0xf6, 0x0f, 0xff, 0xff, 0xc7, 0xff, 0xf7,
    0xf0, 0x3f, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xbe
};

// 0: normal
// 1: sea monsters
// 2: flying (magic carpet, lb, ...)
// 3: horse
// 4: ghost / shadowlord
// 5: skiff (boat)
// 6: frigate
// 7: rotworm
// 8: corpser
// 9: whirlpool
// 10: sand trap
// 0xff / -1: invalid
u8 moveType[64] =
{
    0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x00, 0x00,
    0x06, 0x06, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02,
    0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x00, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02,
    0x0a, 0x00, 0xff, 0x09, 0x02, 0x08, 0x07, 0x04
};

u8 skiffWaterBits[16] = { 0x0a, 0x05, 0x03, 0x09, 0x0c, 0x06, 0x0b, 0x0d, 0x0e, 0x07, 0x0a, 0x05, 0x08, 0x04, 0x02, 0x01 };

u8 skiffTileBits[4] = { 0x06, 0x03, 0x09, 0x0c };

// DS:5548: 12 bytes of zeros that no code refers to
u8 D_5548[12] = { 0 };

// walkable for normal walk type?
static bool IsWalkableNormal(byte actorTile, int targetTile)
{
    bool walkable;

    if ((blockedTileBits[targetTile >> 3] & (0x80 >> (targetTile & 7))) != 0)
        walkable = 0;
    else
        walkable = 1;

    if ((((actorTile & 0xfe) != TILE_ACTOR_AVATAR) && ((actorTile & 0xf0) != TILE_ACTOR_WIZARD)) && (((byte)targetTile & 0xfc) == TILE_MAP_CHAIR_90))
    {
        walkable = 0;
    }

    return walkable;
}

// is walkable water tile? (walkable/passable for sea monster / flying / ghost)
static int IsWaterTile(int tile)
{
    return tile < 4 || ((byte)tile & 0xf0) == TILE_MAP_60;
}

// CHECKED
// The gotos give the original's tail merging: one "return 1" block (L_ret1, in case 6)
// that cases 2, 3 and 5 jump to; case 3 written with && (its "je ret0" tests then end in
// their own "jmp L_ret1" instead of joining case 2's tail); cases 7-10 as "if (t == X)
// return 1; return 0;", so that case 7's own "return 1" becomes a jmp to L_ret1 and cases
// 8-10 join case 7's tail.
int ULTIMA_2c4c_IsWalkableTile(int actorTile, int targetTile)
{
    switch (moveType[actorTile >> 2])
    {
    case 0: // normal
        return IsWalkableNormal(actorTile, targetTile);
    case 1: // sea monsters
        return IsWaterTile(targetTile);
    case 2: // flying
        if (((byte)targetTile & 0xf0) == TILE_MAP_60 || IsWaterTile(targetTile) != 0 || IsWalkableNormal(actorTile, targetTile) != 0)
            goto L_ret1;
        return 0;
    default:
        return 0;
    case 3: // horse
        if (IsWalkableNormal(actorTile, targetTile) != 0 && targetTile != TILE_MAP_LAVA && targetTile != 4)
            goto L_ret1;
        return 0;
    case 4: // ghost / shadowlord
        return IsWaterTile(targetTile) == 0;
    case 5: // skiff
        if (((byte)targetTile & 0xfc) == TILE_MAP_34)
        {
            return (8 >> (actorTile & 3) & (uint)skiffTileBits[targetTile - TILE_MAP_34]) != 0;
        }
        if (IsWaterTile(targetTile) == 0)
        {
            return 0;
        }
        if (targetTile < TILE_MAP_60)
            goto L_ret1;
        if (!((uint)skiffWaterBits[targetTile - TILE_MAP_60] & (8 >> (actorTile & 3))))
            return 0;
        goto L_ret1;
    case 6: // frigate
        if (targetTile > 2)
            return 0;
    L_ret1:
        return 1;
    case 7: // rotworm
        if (targetTile == TILE_MAP_POISON)
            return 1;
        return 0;
    case 8: // corpser
        if (targetTile == TILE_MAP_GRASS)
            return 1;
        return 0;
    case 9: // whirlpool
        if (targetTile == TILE_MAP_WATER_1)
            return 1;
        return 0;
    case 10: // sand trap
        if (targetTile == TILE_MAP_7)
            return 1;
        return 0;
    }
}

// CHECKED
// select player (zstats, ...)
// The gotos to one "done = 1;" keep done to three references, so MSC puts shownRow in
// DI and leaves done in memory, as the original does (written out in each case, MSC
// picks done instead).
// Ask for a party member with the bar in the stats window (1-6, the arrows, Enter). Returns the index, -1 for Esc, or -2 for 0 where `allowNone` is set.
int SelectPlayerOrNone(int allowNone)
{
    bool done;
    int key;
    int shownRow_d;
    s16 selected_h;

    selected_h = 0;
    shownRow_d = -1;
    done = 0;
    if (D_5893_mapId > 0x7f && (combatants[curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        InvertPlayerRow(combatants[curCombatant].entityIdx);
    }

    // 2db2
    ShowStatsTitle(/*0x5554*/ "Select:");
    // ^OK

    do
    {
        // 2dbc
        if (shownRow_d != selected_h)
        {
            InvertPlayerRow(selected_h);
            shownRow_d = selected_h;
        }

        do
        {
            key = ULTIMA_266c_GetChar();
        } while ((uint)key > '7');

        if ((uint)key > '0' && (uint)key < '7' && (uint)key - '1' < D_585b_partyCount)
        {
            selected_h = (uint)key - '1';
        }
        else
        {
            switch (key)
            {
            case U5_KEY_LEFT:
            case U5_KEY_UP:
                DecreaseInt(&selected_h, 1);
                break;

            case U5_KEY_RIGHT:
            case U5_KEY_DOWN:
                IncreaseInt(&selected_h, 1, D_585b_partyCount - 1);
                break;

            case 0x30:
                if (allowNone == 0)
                    break;

                selected_h = -2;
            L_done:
                done = 1;
                break;

            case U5_KEY_ESC:
                selected_h = -1;
                goto L_done;

            case U5_KEY_ENTER:
            case U5_KEY_SPACE:
                goto L_done;
            }
        }

        if (shownRow_d != selected_h)
        {
            InvertPlayerRow(shownRow_d);
        }
    } while (!done);

    // 2e6e
    if (-1 < selected_h)
    {
        InvertPlayerRow(selected_h);
    }

    ClearStatsTitle();
    return selected_h;
}

// Ask for a party member; -1 for Esc.
int SelectPlayer(void)
{
    return SelectPlayerOrNone(0);
}

void ULTIMA_2e96_SetWindDirection(int direction)
{
    if (direction != -1)
    {
        windDir = (char)direction;
        sailCounter = 0;
    }

    if (D_5893_mapId >= 0x21 || D_5893_mapId == 0x19)
        return;

    if (D_5895_mapLevel < 0x80) // s8 level >= 0?
    {
        SelectTextWindow(0);
        SetTextPosition(6, 0x17);
        ULTIMA_4c2a_PrintLeftCap();

        switch (windDir)
        {
        case 0:
            ULTIMA_1850_PrintString(/*0x555c*/ "Calm ");
            break;
        case 1:
            ULTIMA_1850_PrintString(/*0x5562*/ "North");
            break;
        case 2:
            ULTIMA_1850_PrintString(/*0x5568*/ "South");
            break;
        case 3:
            ULTIMA_1850_PrintString(/*0x556e*/ "East ");
            break;
        case 4:
            ULTIMA_1850_PrintString(/*0x5574*/ "West ");
            break;
        }

        ULTIMA_1850_PrintString(/*0x557a*/ " Winds");
        PrintRightCap();
        SelectTextWindow(2);
    }
    else
    {
        // level < 0
        ULTIMA_0a70_GRAP_2d_SetPenColor(brightWhiteColor);
        GRAP_Line(0x30, 0xb8, 0x98, 0xb8);
        ULTIMA_0a70_GRAP_2d_SetPenColor(blueColor);
        ULTIMA_0aa6_GRAP_3f_FillRect(0x30, 0xb9, 0x98, 0xbf);
    }
}

// CHECKED
void UpdateWindDirectionRandomly(void)
{
    int newDirection;
    if (ULTIMA_2092_RandomRange(0, 0x3f) == 0)
    {
        newDirection = ULTIMA_2092_RandomRange(0, 4);
        while (newDirection == 0 && ULTIMA_2092_RandomRange(0, 0xff) < 0xc0)
        {
            newDirection = ULTIMA_2092_RandomRange(0, 4);
        }

        ULTIMA_2e96_SetWindDirection(newDirection);
    }
}

// Poison a living party member (the trap effect).
static void PoisonPlayer(int playerIndex)
{
    if (playerIndex < D_585b_partyCount && D_55a8_party[playerIndex].status != STATUS_DEAD)
    {
        D_55a8_party[playerIndex].status = STATUS_POISONED;
        ULTIMA_2900_UpdateVitalsDisplay();
    }
}

// A trap goes off on a party member: acid, poison, a bomb or gas.
void SpringTrap(int playerIndex)
{
    int trapKind;
    int playerIndex_j;

    ULTIMA_223c_AudioWhiteNoise(0x28, 3000, 500);

    if (D_5893_mapId > 0x7f)
    {
        trapKind = ULTIMA_2092_RandomRange(0, 1);
    }
    else
    {
        trapKind = (uint)trapKinds[ULTIMA_2092_RandomRange(0, 7)];
    }

    switch (trapKind)
    {
    case 0:
        ULTIMA_1850_PrintString(/*0x5581*/ "ACID!\n");
        DamagePlayer(playerIndex, Random1To30());
        break;
    case 1:
        ULTIMA_1850_PrintString(/*0x5588*/ "POISON!\n");
        PoisonPlayer(playerIndex);
        break;
    case 2:
        ULTIMA_1850_PrintString(/*0x5591*/ "BOMB!\n");
        ULTIMA_2aa8_DamageParty();
        break;
    case 3:
        ULTIMA_1850_PrintString(/*0x5598*/ "GAS!\n");
        playerIndex_j = 0;
        do
        {
            PoisonPlayer(playerIndex_j);
        } while (++playerIndex_j < 6);
        break;
    }
}

// after the last string (DS:559e), used by SpringTrap above
u8 trapKinds[8] = { 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03 };
