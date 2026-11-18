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
        D_5891_animate = 0xff;
    }

    do
    {
        key = ULTIMA_2032_ToUpper(ULTIMA_1b38_PollKeyWithCursor());
        if ((key == 0) && (D_5893_mapId < 0x21 || 0x7f < D_5893_mapId))
        {
            ULTIMA_5910_UpdateFrame();
        }
    } while (key == 0);

    if (D_538a_numpadKeys != 0)
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
static void ULTIMA_2726_PrintPartyRow(int rowIndex)
{
    int     col;
    CombatEntity* combatant;

    ULTIMA_1bf2_SetTextPosition(0, rowIndex);

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
    combatant = &D_ba14_combatants[D_589e_curCombatant];
    if (D_5893_mapId > 0x7f && D_589e_curCombatant != 0xff && (combatant->flags & COMBAT_FLAGS_PLAYER) != 0 && combatant->entityIdx == rowIndex)
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    }

    // 278e
    ULTIMA_1850_PrintString(D_55a8_party[rowIndex].name);
    for (col = ULTIMA_216c_strlen(D_55a8_party[rowIndex].name); col < 9; col++)
    {
        // 27b8
        ULTIMA_16ba_PrintChar(' ');
    }

    // 27c2
    if (rowIndex == D_587b_activePlayer)
    {
        if (D_55a8_party[rowIndex].status != STATUS_DEAD && D_55a8_party[rowIndex].status != STATUS_SLEEP)
        {
            ULTIMA_16ba_PrintChar(0x1a);
        }
        else
        {
            ULTIMA_16ba_PrintChar(' ');
            D_587b_activePlayer = 0xff;
        }
    }
    else
    {
        // 27f6
        ULTIMA_16ba_PrintChar(' ');
    }

    // 27fd
    ULTIMA_1a3e_PrintNumber(D_55a8_party[rowIndex].hp, 4, 0x20);

    if (D_5893_mapId > 0x7f)
    {
        if ((D_ba14_combatants[rowIndex].flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER | COMBAT_FLAGS_DEAD | COMBAT_FLAGS_CHARMED)) ==
            (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_CHARMED) &&
            D_ba14_combatants[rowIndex].entityIdx == rowIndex)
        {
            // -> 2849
            ULTIMA_16ba_PrintChar('C');
        }
        else
        {
            // 283c -> 2849
            ULTIMA_16ba_PrintChar(D_55a8_party[rowIndex].status);
        }

        if (D_589e_curCombatant != 0xff && (combatant->flags & COMBAT_FLAGS_PLAYER) != 0)
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

void ULTIMA_2884_UpdateGoldDisplay(void)
{
    int savedWindow;

    savedWindow = D_5386_textWindowIndex;
    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(8, 7);
    if (D_57aa_gold < 1000)
    {
        ULTIMA_16ba_PrintChar(' ');
    }
    if (D_57aa_gold < 100)
    {
        ULTIMA_16ba_PrintChar(' ');
    }
    if (D_57aa_gold < 10)
    {
        ULTIMA_16ba_PrintChar(' ');
    }
    ULTIMA_1850_PrintString(/*0x54b6*/ " G:");
    ULTIMA_1a3e_PrintNumber(D_57aa_gold, 1, ' ');
    while (0xf > ULTIMA_1f12_GetCurrentTextX())
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    ULTIMA_1b94_SelectTextWindow(savedWindow);
}

void ULTIMA_2900_UpdateVitalsDisplay(void)
{
    int row;

    ULTIMA_1b94_SelectTextWindow(1);

    row = 0;
    do
    {
        ULTIMA_2726_PrintPartyRow(row);
        row++;
    } while (row < 6);

    ULTIMA_1bf2_SetTextPosition(0, 7);

    // food
    ULTIMA_1850_PrintString(/*0x54ba*/ "F:");
    ULTIMA_1a3e_PrintNumber(D_57a8_food, 1, ' ');

    while (ULTIMA_1f12_GetCurrentTextX() < 8)
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    if ((D_5893_mapId < 0x80) && ((D_587c_partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20))
    {
        // ship
        ULTIMA_1850_PrintString(/*0x54bd*/ "Ship:");
        ULTIMA_1a3e_PrintNumber(D_5c5a_actors[0].value, 1, ' ');
        if (D_5c5a_actors[0].value < 10)
        {
            ULTIMA_16ba_PrintChar(' ');
        }
    }
    else
    {
        ULTIMA_2884_UpdateGoldDisplay();
    }

    ULTIMA_1850_PrintString(/*0x54c3*/ "\n   ");

    if ((D_587d_month < 10) && (D_587e_day < 10))
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    // date (m/d/y)
    ULTIMA_1a3e_PrintNumber(D_587d_month, 1, ' ');
    ULTIMA_16ba_PrintChar('-');
    ULTIMA_1a3e_PrintNumber(D_587e_day, 1, ' ');
    ULTIMA_16ba_PrintChar('-');
    ULTIMA_1a3e_PrintNumber(D_5874_year, 3, '0');

    while (ULTIMA_1f12_GetCurrentTextX() < 0xf)
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    ULTIMA_1bf2_SetTextPosition(6, 6);

    // icon?
    if (D_587a_statusIcon != 0)
    {
        ULTIMA_4c2a_PrintLeftCap();
        ULTIMA_16ba_PrintChar(D_587a_statusIcon);
        ULTIMA_4cce_PrintRightCap();
    }
    else
    {
        ULTIMA_4f3c_DrawStatsDivider();
    }

    ULTIMA_1b94_SelectTextWindow(2);
}

// Invert the party member's row in the stats window (the selection bar, the flash of a hit).
void ULTIMA_2a28_InvertPlayerRow(int playerIndex)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b86_GRAP_XorFillRect(0xc0, playerIndex * 8 + 8, 0x137, playerIndex * 8 + 0xf);
}

// process damage (show fx, reduce hp)
// Hurt a party member: flash and noise, hit points down by `damage`, dead at 0.
void ULTIMA_2a52_DamagePlayer(int playerIndex, uint damage)
{
    ULTIMA_2a28_InvertPlayerRow(playerIndex);
    ULTIMA_223c_AudioWhiteNoise(10, 0x640, 2000);
    ULTIMA_2a28_InvertPlayerRow(playerIndex);

    D_55a8_party[playerIndex].hp -= damage;
    if (D_55a8_party[playerIndex].hp <= 0)
    {
        D_55a8_party[playerIndex].hp = 0;
        D_55a8_party[playerIndex].status = STATUS_DEAD;
        if (playerIndex == D_587b_activePlayer)
        {
            D_587b_activePlayer = 0xff;
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
            ULTIMA_2a52_DamagePlayer(i, ULTIMA_2092_RandomRange(1, 8));
        }
    }
}

void ULTIMA_3ef0_IncreaseByte(byte* value, int amount, int limit);

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
        if ((uint)status == STATUS_DEAD && playerIndex_l == D_587b_activePlayer)
        {
            D_587b_activePlayer = 0xff;
        }

        if ((uint)status != STATUS_DEAD && (uint)status != STATUS_SLEEP)
        {
            if ((uint)status == STATUS_POISONED)
            {
                ULTIMA_2a52_DamagePlayer(playerIndex_l, 1);
            }

            ableCount_d++;
        }
    }

    if (D_587f_hour != D_5880_lastHour)
    {
        if (D_57a8_food == 0)
        {
            ULTIMA_1850_PrintString(/*0x54c8*/ "Starving!\n");
            ULTIMA_2aa8_DamageParty();
        }
        else if (D_587f_hour == 6 || D_587f_hour == 12 || D_587f_hour == 18)
        {
            ULTIMA_3f54_DecreaseInt(&D_57a8_food, ableCount_d);
        }
        D_5880_lastHour = D_587f_hour;
    }

    ULTIMA_3ef0_IncreaseByte(&D_588b_turnCount, 1, 0xff);
    if (D_588e_protectTurns != 0 && D_588e_protectTurns != 0xff)
    {
        D_588e_protectTurns--;
        if (D_588e_protectTurns == 0)
        {
            D_587a_statusIcon = 0;
            ULTIMA_2900_UpdateVitalsDisplay();
        }
    }

    ULTIMA_400c_RingRegeneration();
}

// IsWalkableTile's tables, in this module's _DATA between its strings (DS:54d4)
u8 D_54d4_blockedTileBits[32] =
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
u8 D_54f4_moveType[64] =
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

u8 D_5534_skiffWaterBits[16] = { 0x0a, 0x05, 0x03, 0x09, 0x0c, 0x06, 0x0b, 0x0d, 0x0e, 0x07, 0x0a, 0x05, 0x08, 0x04, 0x02, 0x01 };

u8 D_5544_skiffTileBits[4] = { 0x06, 0x03, 0x09, 0x0c };

// DS:5548: 12 bytes of zeros that no code refers to
u8 D_5548[12] = { 0 };

// walkable for normal walk type?
static bool ULTIMA_2bd4_IsWalkableNormal(byte actorTile, int targetTile)
{
    bool walkable;

    if ((D_54d4_blockedTileBits[targetTile >> 3] & (0x80 >> (targetTile & 7))) != 0)
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
static int ULTIMA_2c2e_IsWaterTile(int tile)
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
    switch (D_54f4_moveType[actorTile >> 2])
    {
    case 0: // normal
        return ULTIMA_2bd4_IsWalkableNormal(actorTile, targetTile);
    case 1: // sea monsters
        return ULTIMA_2c2e_IsWaterTile(targetTile);
    case 2: // flying
        if (((byte)targetTile & 0xf0) == TILE_MAP_60 || ULTIMA_2c2e_IsWaterTile(targetTile) != 0 || ULTIMA_2bd4_IsWalkableNormal(actorTile, targetTile) != 0)
            goto L_ret1;
        return 0;
    default:
        return 0;
    case 3: // horse
        if (ULTIMA_2bd4_IsWalkableNormal(actorTile, targetTile) != 0 && targetTile != TILE_MAP_LAVA && targetTile != 4)
            goto L_ret1;
        return 0;
    case 4: // ghost / shadowlord
        return ULTIMA_2c2e_IsWaterTile(targetTile) == 0;
    case 5: // skiff
        if (((byte)targetTile & 0xfc) == TILE_MAP_34)
        {
            return (8 >> (actorTile & 3) & (uint)D_5544_skiffTileBits[targetTile - TILE_MAP_34]) != 0;
        }
        if (ULTIMA_2c2e_IsWaterTile(targetTile) == 0)
        {
            return 0;
        }
        if (targetTile < TILE_MAP_60)
            goto L_ret1;
        if (!((uint)D_5534_skiffWaterBits[targetTile - TILE_MAP_60] & (8 >> (actorTile & 3))))
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
int ULTIMA_2d7a_SelectPlayerOrNone(int allowNone)
{
    bool done;
    int key;
    int shownRow_d;
    s16 selected_h;

    selected_h = 0;
    shownRow_d = -1;
    done = 0;
    if (D_5893_mapId > 0x7f && (D_ba14_combatants[D_589e_curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        ULTIMA_2a28_InvertPlayerRow(D_ba14_combatants[D_589e_curCombatant].entityIdx);
    }

    // 2db2
    ULTIMA_4e50_ShowStatsTitle(/*0x5554*/ "Select:");
    // ^OK

    do
    {
        // 2dbc
        if (shownRow_d != selected_h)
        {
            ULTIMA_2a28_InvertPlayerRow(selected_h);
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
                ULTIMA_3f54_DecreaseInt(&selected_h, 1);
                break;

            case U5_KEY_RIGHT:
            case U5_KEY_DOWN:
                ULTIMA_3f14_IncreaseInt(&selected_h, 1, D_585b_partyCount - 1);
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
            ULTIMA_2a28_InvertPlayerRow(shownRow_d);
        }
    } while (!done);

    // 2e6e
    if (-1 < selected_h)
    {
        ULTIMA_2a28_InvertPlayerRow(selected_h);
    }

    ULTIMA_4e20_ClearStatsTitle();
    return selected_h;
}

// Ask for a party member; -1 for Esc.
int ULTIMA_2e8e_SelectPlayer(void)
{
    return ULTIMA_2d7a_SelectPlayerOrNone(0);
}

void ULTIMA_2e96_SetWindDirection(int direction)
{
    if (direction != -1)
    {
        D_5892_windDir = (char)direction;
        D_5883_sailCounter = 0;
    }

    if (D_5893_mapId >= 0x21 || D_5893_mapId == 0x19)
        return;

    if (D_5895_mapLevel < 0x80) // s8 level >= 0?
    {
        ULTIMA_1b94_SelectTextWindow(0);
        ULTIMA_1bf2_SetTextPosition(6, 0x17);
        ULTIMA_4c2a_PrintLeftCap();

        switch (D_5892_windDir)
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
        ULTIMA_4cce_PrintRightCap();
        ULTIMA_1b94_SelectTextWindow(2);
    }
    else
    {
        // level < 0
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
        ULTIMA_0b10_GRAP_Line(0x30, 0xb8, 0x98, 0xb8);
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
        ULTIMA_0aa6_GRAP_3f_FillRect(0x30, 0xb9, 0x98, 0xbf);
    }
}

// CHECKED
void ULTIMA_2f62_UpdateWindDirectionRandomly(void)
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
static void ULTIMA_2fa6_PoisonPlayer(int playerIndex)
{
    if (playerIndex < D_585b_partyCount && D_55a8_party[playerIndex].status != STATUS_DEAD)
    {
        D_55a8_party[playerIndex].status = STATUS_POISONED;
        ULTIMA_2900_UpdateVitalsDisplay();
    }
}

// A trap goes off on a party member: acid, poison, a bomb or gas.
void ULTIMA_2fd0_SpringTrap(int playerIndex)
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
        trapKind = (uint)D_559e_trapKinds[ULTIMA_2092_RandomRange(0, 7)];
    }

    switch (trapKind)
    {
    case 0:
        ULTIMA_1850_PrintString(/*0x5581*/ "ACID!\n");
        ULTIMA_2a52_DamagePlayer(playerIndex, ULTIMA_3abe_Random1To30());
        break;
    case 1:
        ULTIMA_1850_PrintString(/*0x5588*/ "POISON!\n");
        ULTIMA_2fa6_PoisonPlayer(playerIndex);
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
            ULTIMA_2fa6_PoisonPlayer(playerIndex_j);
        } while (++playerIndex_j < 6);
        break;
    }
}

// after the last string (DS:559e), used by ULTIMA_2fd0_SpringTrap above
u8 D_559e_trapKinds[8] = { 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03 };
