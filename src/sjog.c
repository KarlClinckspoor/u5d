#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "combat.h"
#include "comsubs.h"
#include "dungeon.h"
#include "endgame.h"
#include "sjog.h"

#include "town.h"

void ULTIMA_6794_ApplyRing(int combatantIndex);

// the last free actor slot (tile 0), counting down from 31; 0 when none
static int SJOG_0000_FindFreeActor(void)
{
    int slot = 0x20;

    while (--slot >= 0)
    {
        if (D_5c5a_actors[slot].tile == 0)
            return slot;
    }

    return 0;
}

// the cell beside (x, y) in a direction (0 north, 1 east, 2 south, 3 west), into D_5876/D_5878
static void SJOG_002a_StepFromCell(int dir, int fromX, int fromY)
{
    D_5876_tempX = fromX;
    D_5878_tempY = fromY;

    switch (dir)
    {
    case 0:
        D_5878_tempY--;
        break;
    case 1:
        D_5876_tempX++;
        break;
    case 2:
        D_5878_tempY++;
        break;
    case 3:
        D_5876_tempX--;
        break;
    }
}

// "Dir-": a key picks the cell ahead, here, right or left of the facing, into D_5876/D_5878; 0 for Pass
int SJOG_006c_AskDungeonCell(int facing)
{
    byte key;
    int newDir_g;

    ULTIMA_1850_PrintString(_TEXT(0x84e6, "Dir-"));

    while ((key = ULTIMA_266c_GetChar()) != U5_KEY_SPACE &&
           key != U5_KEY_UP && key != U5_KEY_DOWN &&
           key != U5_KEY_RIGHT && key != U5_KEY_LEFT)
    {
    }

    switch (key)
    {
    case U5_KEY_SPACE:
        ULTIMA_1850_PrintString(_TEXT(0x84ec, "Pass\n"));
        return 0;

    case U5_KEY_UP:
        ULTIMA_1850_PrintString(_TEXT(0x84f2, "Ahead\n"));
        SJOG_002a_StepFromCell(facing, D_5896_mapX, D_5897_mapY);
        break;

    case U5_KEY_DOWN:
        ULTIMA_1850_PrintString(_TEXT(0x84fa, "Here\n"));
        D_5876_tempX = (uint)D_5896_mapX;
        D_5878_tempY = (uint)D_5897_mapY;
        break;

    case U5_KEY_RIGHT:
        ULTIMA_1850_PrintString(_TEXT(0x8500, "Right\n"));
        newDir_g = (facing + 1) % 4;

        SJOG_002a_StepFromCell(newDir_g, D_5896_mapX, D_5897_mapY);
        break;

    case U5_KEY_LEFT:
        ULTIMA_1850_PrintString(_TEXT(0x8508, "Left\n"));
        newDir_g = (facing + 3) % 4;

        SJOG_002a_StepFromCell(newDir_g, D_5896_mapX, D_5897_mapY);
        break;
    }

    return 1;
}

// "a chest!", "a sack of gold!" ...: what an actor tile is
static void SJOG_012a_PrintItemName(int tile)
{
    switch (tile)
    {
    case TILE_ACTOR_CHEST:
        ULTIMA_1850_PrintString(_TEXT(0x850e, "a chest!\n"));
        break;
    case TILE_ACTOR_GOLD:
        ULTIMA_1850_PrintString(_TEXT(0x8518, "a sack of gold!\n"));
        break;
    case TILE_ACTOR_POTION:
        ULTIMA_1850_PrintString(_TEXT(0x852a, "a potion!\n"));
        break;
    case TILE_ACTOR_SCROLL:
        ULTIMA_1850_PrintString(_TEXT(0x8536, "a scroll!\n"));
        break;
    case TILE_ACTOR_WEAPON:
        ULTIMA_1850_PrintString(_TEXT(0x8542, "a weapon!\n"));
        break;
    case TILE_ACTOR_SHIELD:
        ULTIMA_1850_PrintString(_TEXT(0x854e, "a shield!\n"));
        break;
    case TILE_ACTOR_KEY:
        ULTIMA_1850_PrintString(_TEXT(0x855a, "a ring of keys!\n"));
        break;
    case TILE_ACTOR_GEM:
        ULTIMA_1850_PrintString(_TEXT(0x856c, "a gem!\n"));
        break;
    case TILE_ACTOR_HELM:
        ULTIMA_1850_PrintString(_TEXT(0x8574, "a helm!\n"));
        break;
    case TILE_ACTOR_RING:
        ULTIMA_1850_PrintString(_TEXT(0x857e, "a ring!\n"));
        break;
    case TILE_ACTOR_ARMOUR:
        ULTIMA_1850_PrintString(_TEXT(0x8588, "some armour!\n"));
        break;
    case TILE_ACTOR_AMULET:
        ULTIMA_1850_PrintString(_TEXT(0x8596, "an amulet!\n"));
        break;
    case TILE_ACTOR_TORCH:
        ULTIMA_1850_PrintString(_TEXT(0x85a2, "some torches!\n"));
        break;
    case TILE_ACTOR_FOOD:
        ULTIMA_1850_PrintString(_TEXT(0x85b2, "some food!\n"));
        break;
    case TILE_ACTOR_MOONSTONE:
        ULTIMA_1850_PrintString(_TEXT(0x85be, "a strange rock!\n"));
        break;
    case TILE_ACTOR_SLEEP:
        ULTIMA_1850_PrintString(_TEXT(0x85d0, "a rotting body!\n"));
        break;
    case TILE_ACTOR_DEAD:
        ULTIMA_1850_PrintString(_TEXT(0x85e2, "a moldy corpse!\n"));
        break;
    default:
        ULTIMA_1850_PrintString(_TEXT(0x85f4, "Nothing of note.\n"));
        break;
    }
}

// searching a body: usually it crumbles to nothing (7 in 8), 1 in 32 a plague poisons the member, else nothing, worms, guts or a bloody pulp; or food or gold under it
static void SJOG_01f2_SearchBody(int actor, int member)
{
    if (ULTIMA_2092_RandomRange(0, 7) != 0)
    {
        ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, actor);
        if (ULTIMA_2092_RandomRange(0, 0x1f) == 0x13)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8606, "Plague!\n"));
            ULTIMA_223c_AudioWhiteNoise(0x28, 3000, 500);
            D_55a8_party[member].status = STATUS_POISONED;
            D_a9fa_vitalsDirty = 1;
        }
        else
        {
            switch (ULTIMA_2092_RandomRange(0, ULTIMA_2092_RandomRange(0, 3)))
            {
            case 0:
                ULTIMA_1850_PrintString(_TEXT(0x8610, "nothing!\n"));
                break;

            case 1:
                ULTIMA_1850_PrintString(_TEXT(0x861a, "worms!\n"));
                break;

            case 2:
                ULTIMA_1850_PrintString(_TEXT(0x8622, "guts!\n"));
                break;

            case 3:
                ULTIMA_1850_PrintString(_TEXT(0x862a, "a bloody pulp!\n"));
                break;
            }
        }
    }
    else
    {
        if (ULTIMA_2092_RandomRange(0, 3) == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x863a, "food!\n"));
            D_5c5a_actors[actor].tile = D_5c5a_actors[actor].animTile = TILE_ACTOR_FOOD;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8642, "gold!\n"));
            D_5c5a_actors[actor].tile = D_5c5a_actors[actor].animTile = TILE_ACTOR_GOLD;
        }
        
        D_5c5a_actors[actor].value = ULTIMA_2092_RandomRange(1, 3);
        D_24e6_viewDirty |= 2;
        ULTIMA_5910_UpdateFrame();
    }
}

// searching a chest: the member's intelligence against the chest's trap value tells "no trap", "a simple trap", "a complex trap" or just "a trap"
static void SJOG_02ea_SearchChestTrap(int chest, int member)
{
    int chance;
    int sensed;
    int chestValue_k;

    chestValue_k = D_5c5a_actors[chest].value;
    if ((chestValue_k & 0x80) == 0)
    {
        chance = (-(D_55a8_party[member].intel - 0x1e)) >> 1;
    }
    else
    {
        chance = ((chestValue_k & 0x7f) - (uint)D_55a8_party[member].intel + 0x1e) >> 1;
    }

    if (chance <= ULTIMA_2092_RandomRange(1, 0x1e))
        sensed = 1;
    else
        sensed = 0;

    if ((sensed && (chestValue_k & 0x80) == 0) || (!sensed && (chestValue_k & 0x80) != 0))
    {
        ULTIMA_1850_PrintString(_TEXT(0x864a, "no trap!\n"));
    }
    else
    {
        chance = chestValue_k & 0x7f;
        if (sensed && chance < 10)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8654, "a simple trap!\n"));
        }
        else if (sensed && 0x14 < chance)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8664, "a complex trap!\n"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8676, "a trap!\n"));
        }
    }
}

// a buried moonstone (the tables D_5830..D_5848) at this place and map becomes an actor; 1 when found
static int SJOG_03a8_SearchMoonstone(int x, int y, int level)
{
    int stone;
    int slot_i;
    int present;
    int newSlot;

    stone = 8;

    // 03b5
    while (--stone >= 0)
    {
        // 03bd
        if (D_5830_moonstoneX[stone] == x && D_5838_moonstoneY[stone] == y &&
            D_5848_moonstoneZ[stone] == level && D_5840_moonstoneMap[stone] == D_5893_mapId)
        {
            // 03e6
            present = 0;
            slot_i = 0x20;
            while (--slot_i >= 0)
            {
                // 03ee
                if (ULTIMA_368e_FindActorTileAtPos(x, y, level) == TILE_ACTOR_MOONSTONE && D_5c5a_actors[D_5876_tempX].value == stone)
                {
                    present = 1;
                }
                // 0415
            }

            // 041e
            if (present != 0)
            {
                continue;
            }

            // 0422
            newSlot = ULTIMA_38e4_FindActorSlot();
            ULTIMA_3a74_SetActor(TILE_ACTOR_MOONSTONE, TILE_ACTOR_MOONSTONE, x, y, level, stone, newSlot);
            ULTIMA_1850_PrintString(_TEXT(0x8680, "a strange rock!\n"));
            D_24e6_viewDirty |= 2;
            return 1;
        }
    }

    // 0450
    return 0;
}
u8 D_3e66_harvestX[4] = {0xb6, 0x61, 0x2c, 0x00};

u8 D_3e6a_harvestY[4] = {0x36, 0xa5, 0x89, 0x00};

u8 D_3e6e_harvestReagent[4] = {0x07, 0x07, 0x06, 0x00};

char* D_3e72_harvestNames[3] =
{
    _STATIC_TEXT(0x8692, "mandrake root!"),
    _STATIC_TEXT(0x86a2, "mandrake root!"),
    _STATIC_TEXT(0x86b2, "nightshade!")
};

// at the three places D_3e66/D_3e6a, at hour 0, once a day: 2 to 15 sprigs of mandrake root or nightshade; 1 when it did
static int SJOG_045a_HarvestReagent(int x, int y)
{
    int reagent_n;
    int spot;
    int amount;
    char** nameList;

    spot = 0;
    nameList = D_3e72_harvestNames;
    do
    {
        if (D_3e66_harvestX[spot] == x && D_3e6a_harvestY[spot] == y && D_587f_hour == 0 && D_5858_harvestDay[spot] != D_587e_day)
        {
            D_5858_harvestDay[spot] = D_587e_day;

            reagent_n = D_3e6e_harvestReagent[spot];
            amount = ULTIMA_2092_RandomRange(2, 0xf);
            D_5850_reagents[reagent_n] += amount;
            if (D_5850_reagents[reagent_n] > 99)
            {
                D_5850_reagents[reagent_n] = 99;
            }

            // 04ba
            if (amount < 10)
            {
                ULTIMA_1a3e_PrintNumber(amount, 1, 0x20);
            }
            else
            {
                ULTIMA_1a3e_PrintNumber(amount, 2, 0x20);
            }

            // 04d4
            ULTIMA_1850_PrintString(_TEXT(0x86be, " sprigs of\n"));
            ULTIMA_1850_PrintString(*nameList);
            ULTIMA_1850_PrintString(_TEXT(0x86ca, "\n"));

            // 050b
            return 1;
        }

        // 04f6
        nameList++;
        spot++;
    } while (spot < 3);

    // 0503
    return 0;
}

// CHECKED
// The original keeps D_3e78_hiddenTile[local_4_ab] in SI with no stack slot of its own: si shares
// local_a's slot (bp-8). Under MSC 5.0 a register variable takes a slot, and a block's
// slots are given back for a sibling block only when the block is the body of an if or a
// for (if (1) here), not a bare { }. local_a is declared in it, so it comes after the
// function's own locals. (An "extern const" table would also keep the value across the
// call, but 5.0 then writes fixups to segment index 0.)
// looks the place up in the table of hidden items (D_3f5c..D_40b2: map, level, x, y, tile, value); a find that is not taken yet becomes an actor, else "nothing of note"
static void SJOG_0514_FindHiddenItem(int x, int y)
{
    int slot;
    int entry_n;
    int bit_d;

    if (1)
    {
        bool found;

        found = 0;

        for (entry_n = 0; entry_n < 0x71; entry_n++)
        {
            if (D_3f5c_hiddenMap[entry_n] == D_5893_mapId && D_3fce_hiddenLevel[entry_n] == D_5895_mapLevel && D_4040_hiddenX[entry_n] == x && D_40b2_hiddenY[entry_n] == y)
            {
                // 0558
                if ((entry_n == 0xd && D_57ac_keys == 0 && ULTIMA_368e_FindActorTileAtPos(x, y, D_5895_mapLevel) == 0) ||
                    (entry_n == 0xe && D_587e_day != D_57b2_skullKeysDay) ||
                    (entry_n == 0xf && D_57c0_equipment[0x27] == 0 && ULTIMA_368e_FindActorTileAtPos(x, y, D_5895_mapLevel) == 0))
                {
                    found = 1;
                    break;
                }

                // 05f6 (OK)
                bit_d = 1 << (entry_n & 7);
                if ((D_585c_takenItemFlags[(entry_n >> 3)] & bit_d) == 0 && (entry_n < 0xd || entry_n > 0xf))
                {
                    // 061a
                    D_585c_takenItemFlags[(entry_n >> 3)] |= bit_d;

                    // -> 05a1
                    found = 1;
                    break;
                }
            }
        }

        if (entry_n == 0xe)
        {
            D_57b2_skullKeysDay = D_587e_day;
        }

        if (!found)
        {
            goto L_nothing;
        }
    }
    {
        register int itemTile_g;
        slot = ULTIMA_38e4_FindActorSlot();
        itemTile_g = D_3e78_hiddenTile[entry_n];
        ULTIMA_3a74_SetActor(itemTile_g, itemTile_g, D_4040_hiddenX[entry_n], D_40b2_hiddenY[entry_n], D_3fce_hiddenLevel[entry_n], D_3eea_hiddenValue[entry_n], slot);
        D_24e6_viewDirty |= 2;
        SJOG_012a_PrintItemName(itemTile_g);
        return;
    }
L_nothing:
    ULTIMA_1850_PrintString(_TEXT(0x86cc, "nothing of note.\n"));
}

// CHECKED
// Search in a dungeon: what the cell ahead hides: traps (by level and dexterity), pits, bombs, hidden doors, fields, treasure
static void SJOG_0646_SearchInDungeon(void)
{
    int member_g;
    int cell_o;
    int lookY;
    int lookX;
    int trapLevel;
    int skill;
    int dex_o;

    if ((member_g = ULTIMA_4988_GetActingPlayer()) + 1 == 0)
    {
        return;
    }

    if (D_58a7_torchTurns == 0 && D_58a6_lightSpellTurns == 0)
    {
        // 066b
        ULTIMA_1850_PrintString(_TEXT(0x86de, "\nYou find:\ndarkness.\n"));
        return;
    }

    if (SJOG_006c_AskDungeonCell(D_6603_dungeonFacing) == 0)
    {
        return;
    }

    lookX = D_5876_tempX;
    lookY = D_5878_tempY;
    cell_o = GetDungeonMap(lookX & 7, lookY & 7, (uint)D_5895_mapLevel);
    dex_o = D_55a8_party[member_g].dex;
    skill = (D_5895_mapLevel * 2 - dex_o + 0x1e) >> 1;
    ULTIMA_1850_PrintString(_TEXT(0x86f4, "You find:\n"));

    switch (cell_o & 0xf0)
    {
    case 0x0:
        // 0704
        ULTIMA_1850_PrintString(_TEXT(0x8700, "Nothing of note.\n"));
        break;

    case 0x10:
    case 0x20:
    case 0x30:
        // 070a
        ULTIMA_1850_PrintString(_TEXT(0x8712, "Nothing hidden on the ladder.\n"));
        break;

    case 0x40:
        // 0710
        if (ULTIMA_2092_RandomRange(1, 0x1e) > skill)
        {
            if (cell_o == 0x40)
            {
                ULTIMA_1850_PrintString(_TEXT(0x8732, "No trap\n"));
                break;
            }

            trapLevel = D_5895_mapLevel;
        }
        else
        {
            trapLevel = ULTIMA_2092_RandomRange(1, 8);
        }

        if (trapLevel < 4)
        {
            ULTIMA_1850_PrintString(_TEXT(0x873c, "A simple trap\n"));
            break;
        }
        else if (trapLevel >= 7)
        {
            ULTIMA_1850_PrintString(_TEXT(0x874c, "A complex trap\n"));
            break;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x875c, "A trap\n"));
            break;
        }
        break;

    case 0x50:
        // 0760
        ULTIMA_1850_PrintString(_TEXT(0x8764, "Nothing hidden on the fountain.\n"));
        break;

    case 0x60:
        // 0766
        switch (cell_o)
        {
        case 0x60:
            // 077c
            ULTIMA_1850_PrintString(_TEXT(0x8786, "Nothing hidden\nin the pit.\n"));
            break;

        case 0x61:
            // 0782
            ULTIMA_1850_PrintString(_TEXT(0x87a2, "A pit!\n"));
            GetDungeonMap(lookX & 7, lookY & 7, D_5895_mapLevel) = (cell_o & 0x8) + 0x60;
            if (D_5895_mapLevel < 7)
            {
                GetDungeonMap(lookX & 7, lookY & 7, D_5895_mapLevel + 1) |= 8;
            }

            DUNGEON_1a90_DrawView(1);
            ULTIMA_0f46_GRAP_66_Reveal(8, 8, 0xb7, 0xb7);
            break;

        case 0x62:
            // 07c6
            if (skill < ULTIMA_2092_RandomRange(1, 0x1e))
            {
                ULTIMA_1850_PrintString(_TEXT(0x87aa, "A bomb trap!\n"));
                GetDungeonMap(lookX & 7, lookY & 7, D_5895_mapLevel) &= 8;
            }
            else
            {
                // 0802
                ULTIMA_1850_PrintString(_TEXT(0x87b8, "Nothing of note.\n"));
            }

            // nop?
            break;
        }

        break;

    case 0x70:
        // 0808
        ULTIMA_1850_PrintString(_TEXT(0x87ca, "Treasure!\n"));
        break;

    case 0x80:
        // 080e
        switch (cell_o)
        {
        case 0x80:
            ULTIMA_1850_PrintString(_TEXT(0x87d6, "A sleep field.\n"));
            break;
        case 0x81:
            ULTIMA_1850_PrintString(_TEXT(0x87e6, "A poison gas field.\n"));
            break;
        case 0x82:
            ULTIMA_1850_PrintString(_TEXT(0x87fc, "A wall of fire.\n"));
            break;
        case 0x83:
            ULTIMA_1850_PrintString(_TEXT(0x880e, "An electric field.\n"));
            break;
        default:
            ULTIMA_1850_PrintString(_TEXT(0x8822, "An energy field.\n"));
            break;
        }
        break;

    case 0x90:
        // 0844
        ULTIMA_1850_PrintString(_TEXT(0x8834, "This tile is impossible.\n"));
        break;

    case 0xa0:
        // 084a
        ULTIMA_1850_PrintString(_TEXT(0x884e, "Nothing hidden on the door.\n"));
        break;

    case 0xb0:
        // 0850
        ULTIMA_1850_PrintString(_TEXT(0x886c, "Nothing hidden on the wall.\n"));
        break;

    case 0xc0:
        // 0856
        switch (D_6604_dungeonStyle & 0xf)
        {
        case 2:
            // 08b6
            ULTIMA_1850_PrintString(_TEXT(0x888a, "Nothing in the caved in passage.\n"));
            break;

        case 1:
            // 08bc
            ULTIMA_1850_PrintString(_TEXT(0x88ac, "Nothing on the stalactite.\n"));
            break;

        default:
            // 0868
            ULTIMA_1850_PrintString(_TEXT(0x88c8, "Nothing hidden on the skeleton.\n"));
            ULTIMA_1850_PrintString(_TEXT(0x88ea, "It crumbles away.\n"));

            GetDungeonMap(lookX & 7, lookY & 7, D_5895_mapLevel) = (cell_o & 8) - 0x50;
            DUNGEON_1a90_DrawView(1);
            ULTIMA_0f46_GRAP_66_Reveal(8, 8, 0xb7, 0xb7);
            break;
        }
        break;

    case 0xd0:
        // 08c2
        ULTIMA_1850_PrintString(_TEXT(0x88fe, "A hidden door!\n"));

        GetDungeonMap(lookX & 7, lookY & 7, D_5895_mapLevel) = (cell_o & 8) - 0x20;
        DUNGEON_1a90_DrawView(1);
        ULTIMA_0f46_GRAP_66_Reveal(8, 8, 0xb7, 0xb7);
        break;

    case 0xe0:
    case 0xf0:
        // 08f0
        ULTIMA_1850_PrintString(_TEXT(0x890e, "Nothing hidden on the door.\n"));
        break;
    }
}

// CHECKED
// the Search command: in a dungeon its own; else a direction, then a chest's trap, a body, a hidden door, a moonstone, reagents, or an item of the hidden table
void SJOG_095c_SearchCmd(void)
{
    int tile_a;
    int member_g;
    int lookX;
    int lookY;
    int index_h;
    int found_d;
    int actorTile;

    found_d = 0;

    if (D_5893_mapId > 0x20 && D_5893_mapId < 0x29)
    {
        SJOG_0646_SearchInDungeon();
        return;
    }

    if (ULTIMA_35ec_SelectDirection() == 0)
    {
        return;
    }

    lookX = D_5896_mapX + D_5876_tempX;
    lookY = D_5897_mapY + D_5878_tempY;
    if ((member_g = ULTIMA_4988_GetActingPlayer()) == -1)
    {
        return;
    }

    tile_a = *ULTIMA_4402_GetTileAddr(lookX, lookY);

    for (index_h = 1; index_h < 0x20; index_h++)
    {
        if (D_5c5a_actors[index_h].x == lookX &&
            D_5c5a_actors[index_h].y == lookY &&
            (D_5893_mapId > 0x7f || D_5c5a_actors[index_h].z == D_5895_mapLevel))
        {
            if ((actorTile = D_5c5a_actors[index_h].tile) == TILE_ACTOR_CHEST)
                break;
        }
    }

    if (index_h < 0x20)
    {
        ULTIMA_1850_PrintString(_TEXT(0x892c, "\nThou dost find\n"));
        SJOG_02ea_SearchChestTrap(index_h, member_g);
        return;
    }

    actorTile = ULTIMA_3702_FindLastActorTileAt(lookX, lookY, D_5895_mapLevel);
    if (actorTile == 0x1f)
    {
        ULTIMA_1850_PrintString(_TEXT(0x893e, "\nThou dost find\n"));
        SJOG_01f2_SearchBody(D_5876_tempX, member_g);
        return;
    }

    switch (tile_a)
    {
    case TILE_MAP_STUMP:
        ULTIMA_1850_PrintString(_TEXT(0x8950, "\nIn the stump\nt"));
        break;

    case TILE_MAP_SHELF:
        ULTIMA_1850_PrintString(_TEXT(0x8960, "\nOn the shelf\nt"));
        break;

    case TILE_MAP_BOOKSHELF:
    case TILE_MAP_BOOKSHELF + 1:
        ULTIMA_1850_PrintString(_TEXT(0x8970, "\nIn the bookshelf\nt"));
        break;

    case TILE_MAP_WELL:
        ULTIMA_1850_PrintString(_TEXT(0x8984, "\nNear the well\nt"));
        break;

    case TILE_MAP_DESK:
        ULTIMA_1850_PrintString(_TEXT(0x8996, "\nIn the desk\nt"));
        break;

    case TILE_MAP_BARREL:
        ULTIMA_1850_PrintString(_TEXT(0x89a6, "\nIn the barrel\nt"));
        break;

    case TILE_MAP_VANITY:
        ULTIMA_1850_PrintString(_TEXT(0x89b8, "\nIn the vanity\nt"));
        break;

    case TILE_MAP_BED:
    case TILE_MAP_BED + 1:
        ULTIMA_1850_PrintString(_TEXT(0x89ca, "\nUnder the bed\nt"));
        break;

    case TILE_MAP_DRESSER:
        ULTIMA_1850_PrintString(_TEXT(0x89dc, "\nIn the dresser\nt"));
        break;

    case TILE_MAP_TRUNK:
        ULTIMA_1850_PrintString(_TEXT(0x89ee, "\nIn the trunk\nt"));
        break;

    case TILE_MAP_FIREPLACE:
        ULTIMA_1850_PrintString(_TEXT(0x89fe, "\nIn the fireplace\nt"));
        break;

    case TILE_MAP_BRAZIER:
        ULTIMA_1850_PrintString(_TEXT(0x8a12, "\nIn the brazier\nt"));
        break;

    case TILE_MAP_WALL:
        ULTIMA_1850_PrintString(_TEXT(0x8a24, "\nIn the wall\nt"));
        break;

    default:
        ULTIMA_1850_PrintString(_TEXT(0x8a34, "\nT"));
        break;
    }

    ULTIMA_1850_PrintString(_TEXT(0x8a38, "hou dost find\n"));
    if (tile_a == TILE_MAP_HIDDEN_DOOR)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8a48, "a hidden door!\n"));
        if (D_5895_mapLevel < 0x80)
        {
            *ULTIMA_4402_GetTileAddr(lookX, lookY) = TILE_MAP_DOOR_B9;
        }
        else
        {
            *ULTIMA_4402_GetTileAddr(lookX, lookY) = TILE_MAP_DOOR_B8;
        }
        D_24e6_viewDirty |= 2;
    }
    else if ((tile_a == TILE_MAP_MOONGATE || (found_d = SJOG_03a8_SearchMoonstone(lookX, lookY, D_5895_mapLevel)) == 0) && (found_d = SJOG_045a_HarvestReagent(lookX, lookY)) == 0)
    {
        SJOG_0514_FindHiddenItem(lookX, lookY);
    }
}

// picking a chest's lock with a key: a locked one (value >= 0x80) opens by dexterity, else the key is used up
static void SJOG_0baa_JimmyChest(int chestX, int chestY, int chestLevel, int chest)
{
    int member;
    byte skill;
    byte chestValue_k;

    if ((member = ULTIMA_4988_GetActingPlayer()) != -1)
    {
        chestValue_k = D_5c5a_actors[chest].value;
        if (chestValue_k < 0x80)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8a58, "Key broke!\n"));
        }
        else
        {
            skill = (((chestValue_k & 0x7f) - D_55a8_party[member].dex + 0x1e) >> 1) & 0xff;
            if (skill < ULTIMA_2092_RandomRange(1, 0x1e))
            {
                ULTIMA_1850_PrintString(_TEXT(0x8a64, "Success!\n"));
                D_5c5a_actors[chest].value &= 0x7f;
                return;
            }
            ULTIMA_1850_PrintString(_TEXT(0x8a6e, "Key broke!\n"));
        }

        ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
        D_57ac_keys--;
    }
}

// CHECKED
// Jimmy in a dungeon: a locked chest (0x4?) opens or loses a key, by dexterity and level
static void SJOG_0c3e_JimmyInDungeon(void)
{
    int cell_o;
    int member_g;
    int lookY;
    int lookX;
    int skill;
    int dex_o;

    ULTIMA_1850_PrintString(_TEXT(0x8a7a, "\n"));
    if ((member_g = ULTIMA_4988_GetActingPlayer()) == -1)
    {
        return;
    }

    lookX = D_5896_mapX;
    lookY = D_5897_mapY;
    cell_o = GetDungeonMap(lookX & 7, lookY & 7, (uint)D_5895_mapLevel);
    dex_o = D_55a8_party[member_g].dex;
    skill = (D_5895_mapLevel * 2 - dex_o + 0x1e) >> 1;

    if ((cell_o & 0xf7) == 0x40)
    {
        if (D_57ac_keys == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8a7c, "No keys!\n"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8a86, "Key broke!\n"));
            D_57ac_keys--;
        }
    }
    else if ((cell_o & 0xf0) == 0x40)
    {
        if (D_57ac_keys == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8a92, "No keys!\n"));
        }
        else
        {
            if (skill < ULTIMA_2092_RandomRange(1, 0x1e))
            {
                ULTIMA_1850_PrintString(_TEXT(0x8a9c, "Chest unlocked\n"));
                GetDungeonMap(lookX & 7, lookY & 7, D_5895_mapLevel) = (cell_o & 8) + 0x40;
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x8aac, "Key broke!\n"));
                D_57ac_keys--;
            }
        }
    }
    else if ((cell_o & 0xf0) == 0x70)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8ab8, "Already open!\n"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x8ac8, "What?\n"));
    }
}

// CHECKED
// the Jimmy command: a key opens a locked door (dexterity), frees a prisoner behind bars (84, 85: karma + 2, the NPC changed), or picks a chest
void SJOG_0d4a_JimmyCmd(void)
{
    int tile_b;
    int npcIndex_f;
    int member;
    int targetX_a;
    int targetY_a;

    if (D_5893_mapId > 0x20 && D_5893_mapId < 0x29)
    {
        SJOG_0c3e_JimmyInDungeon();
        return;
    }

    if (D_57ac_keys == 0)
    {
        // cced, ccf0
        ULTIMA_1850_PrintString(_TEXT(0x8ad0, "No Keys!\n"));
        return;
    }

    // ccf8
    if (ULTIMA_35ec_SelectDirection() == 0)
    {
        return;
    }

    // cd02
    targetX_a = D_5896_mapX + D_5876_tempX;
    targetY_a = D_5897_mapY + D_5878_tempY;
    tile_b = *ULTIMA_4402_GetTileAddr(targetX_a, targetY_a);
    switch (tile_b)
    {
    case TILE_MAP_DOOR_B9:
    case TILE_MAP_DOOR_BB:
        // cd48
        if ((member = ULTIMA_4988_GetActingPlayer()) == -1)
        {
            break;
        }

        if (D_55a8_party[member].dex <= ULTIMA_2092_RandomRange(0, 0x1d))
        {
            ULTIMA_1850_PrintString(_TEXT(0x8ada, "Key broke!\n"));
            D_57ac_keys--;
            break;
        }

        *ULTIMA_4402_GetTileAddr(targetX_a, targetY_a) = tile_b - 1;
        D_24e6_viewDirty |= 2;
        ULTIMA_1850_PrintString(_TEXT(0x8ae6, "Unlocked!\n"));
        break;

    case TILE_MAP_97:
    case TILE_MAP_98:
        // cd9c -> cd72
        ULTIMA_1850_PrintString(_TEXT(0x8af2, "Key broke!\n"));
        D_57ac_keys--;
        break;

    case TILE_MAP_84:
    case TILE_MAP_85:
        // cda2
        if (D_5893_mapId < 0x80 && ULTIMA_368e_FindActorTileAtPos(targetX_a, targetY_a, D_5895_mapLevel) == 0)
        {
            // -> ccf0
            ULTIMA_1850_PrintString(_TEXT(0x8afe, "No one is there!\n"));
            break;
        }

        // cdc2
        npcIndex_f = D_5876_tempX;
        if ((member = ULTIMA_4988_GetActingPlayer()) == -1)
        {
            break;
        }

        if (D_55a8_party[member].dex <= ULTIMA_2092_RandomRange(0, 0x1d))
        {
            // cdef -> cd72
            ULTIMA_1850_PrintString(_TEXT(0x8b10, "Key broke!\n"));
            D_57ac_keys--;
            break;
        }

        // cdf6
        if (D_5893_mapId < 0x7f)
        {
            if ((npcIndex_f = TOWN_011e_FindNpcByActor(npcIndex_f)) == -1)
            {
                // -> ccf0
                ULTIMA_1850_PrintString(_TEXT(0x8b1c, "Couldn't find this npc\n\n"));
                break;
            }

            if (D_5f5e_npcs[npcIndex_f].dialogNumber != 0)
            {
                D_5f5e_npcs[npcIndex_f].dialogNumber = 0;
            }

            // ce24
            if (TOWN_0000_CheckNpcKilled(npcIndex_f) == 0)
            {
                D_5d5e_npcSchedules[npcIndex_f].type[0] = 5;
                D_5d5e_npcSchedules[npcIndex_f].type[1] = 5;
                D_5d5e_npcSchedules[npcIndex_f].type[2] = 5;
                ULTIMA_1850_PrintString(_TEXT(0x8b36, "\n\"I thank thee!\"\n"));
                ULTIMA_3ef0_IncreaseByte(&D_5888_karma, 2, 99);
            }

            // ce5a
            TOWN_0052_SetNpcKilled(npcIndex_f);
            break;
        }

        // ce64
        *ULTIMA_4402_GetTileAddr(targetX_a, targetY_a) = TILE_MAP_44;
        D_24e6_viewDirty |= 2;
        ULTIMA_1850_PrintString(_TEXT(0x8b48, "Unlocked\n"));
        break;

    default:
        // ceac (the counter is tile_b, reused)
        for (tile_b = 1; tile_b < 0x20; tile_b++)
        {
            if (D_5c5a_actors[tile_b].x == targetX_a && D_5c5a_actors[tile_b].y == targetY_a &&
                (D_5893_mapId > 0x7f || D_5c5a_actors[tile_b].z == D_5895_mapLevel) &&
                D_5c5a_actors[tile_b].tile == TILE_ACTOR_CHEST)
            {
                // exit from the loop
                break;
            }
        }

        // cee9
        if (tile_b < 0x20)
        {
            SJOG_0baa_JimmyChest(targetX_a, targetY_a, D_5895_mapLevel, tile_b);
            break;
        }

        ULTIMA_1850_PrintString(_TEXT(0x8b52, "No lock!\n"));
        break;
    }
}

// puts one item actor of a chest's contents at the place: the amount by kind and level (kinds 1 and 2 random), and what it is is printed
static void SJOG_0f88_DropItem(int kind, int amount, int x, int y, int z, int level, int* dropped)
{
    int slot;
    int value_c;

    if (kind == 3 || kind == 4)
    {
        amount--;
    }

    if (kind == 1)
    {
        value_c = ULTIMA_2092_RandomRange(1, level);
    }
    else if (kind == 2)
    {
        value_c = ULTIMA_2092_RandomRange(1, level * 3);
    }
    else
    {
        value_c = amount;
    }

    slot = SJOG_0000_FindFreeActor();
    if (slot != 0)
    {
        ULTIMA_3a74_SetActor(kind, kind, x, y, z, value_c, slot);
        if (D_5893_mapId < 0x80)
        {
            D_5c5a_actors[slot].extra = 0;
            D_5c5a_actors[slot].z = z;
        }
        else
        {
            D_5c5a_actors[slot].extra = 0x20;
        }
        D_24e6_viewDirty = D_24e6_viewDirty | 2;
        ULTIMA_5910_UpdateFrame();
        if (*dropped == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8b5c, "Found:\n"));
            *dropped = 1;
        }
        SJOG_012a_PrintItemName(kind);
    }
}

// the chest's treasure: each of the eight kinds D_4124 (gold, food, torches, gems, keys...) that the chest's level allows, by a roll
static void SJOG_1040_DropTreasure(int level, int x, int y, int z, int* dropped)
{
    int index;
    int amount;

    index = 8;
    while (--index >= 0)
    {
        if (D_412c_treasureMinLevel[index] <= level)
        {
            if (D_412c_treasureMinLevel[index] <= ULTIMA_2092_RandomRange(1, 0x1e))
            {
                if (D_4134_treasureMaxAmount[index] == 1)
                {
                    amount = 1;
                }
                else
                {
                    amount = ULTIMA_2092_RandomRange(1, D_4134_treasureMaxAmount[index]);
                }

                SJOG_0f88_DropItem(D_4124_treasureKind[index], amount, x, y, z, level, dropped);
            }
        }
    }
}

// the chest's gear: half its level in tries on the 48 entries of D_413c (weapons, armour, scrolls, potions...) by their level thresholds D_416c
static void SJOG_10b8_DropGear(int level, int x, int y, int z, int* dropped)
{
    int pick;
    int tries_i;

    tries_i = level / 2;
    while (tries_i-- >= 0)
    {
        pick = ULTIMA_2092_RandomRange(0, 0x2f);

        if (D_416c_gearMinLevel[pick] <= level)
        {
            if (D_416c_gearMinLevel[pick] <= ULTIMA_2092_RandomRange(1, 0x1e))
            {
                SJOG_0f88_DropItem(D_413c_gearKind[pick], pick, x, y, z, level, dropped);
            }
        }
    }
}

// CHECKED
// opens a chest: the trap (value 0x80 up) springs, the treasure and the gear drop; karma falls by 2 in a town
static void SJOG_112c_OpenChest(int x, int y, int z)
{
    int member;
    int dropped_g;
    byte chestValue;
    int index_h;
    CombatEntity* combatant_f;

    dropped_g = 0;
    for (index_h = 1; index_h < 0x20; index_h++)
    {
        if (D_5c5a_actors[index_h].x == x && D_5c5a_actors[index_h].y == y &&
            (D_5893_mapId > 0x7f || (D_5893_mapId < 0x80 && D_5c5a_actors[index_h].z == z)))
        {
            if (D_5c5a_actors[index_h].tile == TILE_ACTOR_CHEST)
                break;

            if (D_5c5a_actors[index_h].tile == TILE_ACTOR_SANDALWOOD_BOX)
            {
                ULTIMA_1850_PrintString(_TEXT(0x8b64, "Can't!\n"));
                return;
            }
        }
    }

    if (index_h == 0x20)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8b6c, "Nothing to open!\n"));
        return;
    }

    if ((member = ULTIMA_4988_GetActingPlayer()) == -1)
    {
        return;
    }

    chestValue = D_5c5a_actors[index_h].value;
    ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, index_h);
    D_24e6_viewDirty = D_24e6_viewDirty | 2;
    if (D_5893_mapId >= 1 && D_5893_mapId <= 0x20)
    {
        if (D_5888_karma > 2)
        {
            D_5888_karma -= 2;
        }
        else
        {
            D_5888_karma = 0;
        }
    }

    if (chestValue > 0x7f)
    {
        chestValue &= 0x7f;
        ULTIMA_1850_PrintString(_TEXT(0x8b7e, "Trapped!\n"));
        ULTIMA_2fd0_SpringTrap(member);
        if (D_5893_mapId > 0x7f && D_55a8_party[member].status == STATUS_DEAD)
        {
            for (index_h = 0; index_h < 32; index_h++)
            {
                combatant_f = &D_ba14_combatants[index_h];
                if ((combatant_f->flags & COMBAT_FLAGS_PLAYER) != 0 && combatant_f->entityIdx == member)
                {
                    combatant_f->flags |= COMBAT_FLAGS_DEAD;

                    D_5c5a_actors[combatant_f->actorIdx].tile = D_5c5a_actors[combatant_f->actorIdx].animTile = TILE_ACTOR_SLEEP;
                    break;
                }
            }

            if (member == D_587b_activePlayer)
            {
                D_587b_activePlayer = 0xff;
            }
            ULTIMA_5910_UpdateFrame();
        }
    }

    SJOG_1040_DropTreasure(chestValue, x, y, z, &dropped_g);
    SJOG_10b8_DropGear(chestValue, x, y, z, &dropped_g);
    if (dropped_g != 0)
    {
        return;
    }

    ULTIMA_1850_PrintString(_TEXT(0x8b88, "Chest empty!\n"));
}

// Open in a dungeon: a chest (0x4?) springs its trap and becomes an open one (0x7?)
static void SJOG_12d4_OpenInDungeon(void)
{
    int cell;
    int member_h;

    cell = GetDungeonMap2(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    if ((cell & 0xf0) == 0x40)
    {
        if ((member_h = ULTIMA_4988_GetActingPlayer()) == -1)
        {
            return;
        }

        if ((cell & 7) != 0)
        {
            ULTIMA_2fd0_SpringTrap(member_h);
        }

        GetDungeonMap2(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) = (cell & 8) + 0x70;
        ULTIMA_1850_PrintString(_TEXT(0x8b96, "\nChest opened\n"));
    }
    else if ((cell & 0xf0) == 0x70)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8ba6, "Already Open!\n"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x8bb6, "What?\n"));
    }
}

// the Open command: in a dungeon its own; else a direction, then a door (the open door shuts again after four turns), a trunk, or a chest
void SJOG_1374_OpenCmd(void)
{
    int targetY;
    int targetX;
    int tile_b;

    if (D_5893_mapId > 0x20 && D_5893_mapId < 0x29)
    {
        SJOG_12d4_OpenInDungeon();
        return;
    }

    ULTIMA_39cc_SetTile(D_594f_openDoorTile, D_5950_openDoorX, D_5951_openDoorY);
    if (ULTIMA_35ec_SelectDirection() == 0)
    {
        return;
    }

    targetX = D_5896_mapX + D_5876_tempX;
    targetY = D_5897_mapY + D_5878_tempY;
    tile_b = *ULTIMA_4402_GetTileAddr(targetX, targetY);
    switch (tile_b)
    {
    case TILE_MAP_TRUNK:
        // 13ea
        ULTIMA_1850_PrintString(_TEXT(0x8bbe, "It's open!\n"));
        break;

    case TILE_MAP_99:
        // 13f4
        ULTIMA_1850_PrintString(_TEXT(0x8bca, "Too heavy!\n"));
        break;

    case TILE_MAP_97:
    case TILE_MAP_98:
    case TILE_MAP_DOOR_B9:
    case TILE_MAP_DOOR_BB:
        // 13fa
        ULTIMA_1850_PrintString(_TEXT(0x8bd6, "Locked!\n"));
        break;

    case TILE_MAP_DOOR_B8:
    case TILE_MAP_DOOR_BA:
        // 1400
        D_594f_openDoorTile = tile_b;
        D_5952_openDoorTurns = 4;
        D_5950_openDoorX = (u8)targetX;
        D_5951_openDoorY = (u8)targetY;
        *ULTIMA_4402_GetTileAddr(targetX, targetY) = TILE_MAP_44;
        D_24e6_viewDirty = 1;
        ULTIMA_1850_PrintString(_TEXT(0x8be0, "Opened!\n"));
        break;

    default:
        // 1444
        SJOG_112c_OpenChest(targetX, targetY, D_5895_mapLevel);
        break;
    }
}

u8 D_3e78_hiddenTile[0x72] =
{
    0x0b, 0x05, 0x0b, 0x05, 0x0b, 0x05, 0x0b, 0x05,
    0x0b, 0x05, 0x0b, 0x05, 0x04, 0x07, 0x07, 0x05,
    0x05, 0x05, 0x08, 0x0b, 0x0b, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x0f, 0x0d, 0x0a, 0x0f, 0x05, 0x0a,
    0x04, 0x0f, 0x0f, 0x05, 0x0d, 0x05, 0x03, 0x04,
    0x1f, 0x1f, 0x1e, 0x1e, 0x1e, 0x05, 0x05, 0x07,
    0x02, 0x04, 0x04, 0x03, 0x03, 0x04, 0x08, 0x1e,
    0x03, 0x03, 0x07, 0x0d, 0x03, 0x08, 0x02, 0x03,
    0x04, 0x0c, 0x04, 0x03, 0x0b, 0x0b, 0x0b, 0x0b,
    0x05, 0x07, 0x0a, 0x03, 0x04, 0x04, 0x04, 0x04,
    0x0a, 0x1e, 0x1e, 0x1e, 0x1e, 0x04, 0x0a, 0x08,
    0x04, 0x04, 0x04, 0x0a, 0x07, 0x0c, 0x07, 0x03,
    0x04, 0x0c, 0x03, 0x03, 0x04, 0x0a, 0x0c, 0x03,
    0x05, 0x0c, 0x03, 0x0f, 0x03, 0x0f, 0x0f, 0x03,
    0x04, 0x00
};

u8 D_3eea_hiddenValue[0x72] =
{
    0x0f, 0x29, 0x0f, 0x29, 0x0f, 0x29, 0x0f, 0x29,
    0x0f, 0x29, 0x0f, 0x29, 0xff, 0x09, 0x85, 0x27,
    0x23, 0x28, 0x01, 0x0a, 0x0a, 0x05, 0x06, 0x07,
    0x05, 0x07, 0x0a, 0x03, 0x2a, 0x05, 0x14, 0x2b,
    0x01, 0x05, 0x0a, 0x26, 0x04, 0x12, 0x03, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x25, 0x05,
    0x63, 0x01, 0x01, 0x01, 0x02, 0x06, 0x04, 0x00,
    0x01, 0x03, 0x07, 0x09, 0x00, 0x05, 0x32, 0x07,
    0x05, 0x2d, 0x07, 0x04, 0x0a, 0x0b, 0x09, 0x0c,
    0x1e, 0x07, 0x2c, 0x03, 0x05, 0x05, 0x02, 0x07,
    0x2b, 0x00, 0x00, 0x00, 0x00, 0x06, 0x2c, 0x03,
    0x01, 0x01, 0x01, 0x2a, 0x05, 0x2f, 0x01, 0x03,
    0x01, 0x2e, 0x05, 0x04, 0x05, 0x2a, 0x2e, 0x07,
    0x12, 0x2e, 0x01, 0x14, 0x07, 0x10, 0x0d, 0x06,
    0x07, 0x00
};

u8 D_3f5c_hiddenMap[0x72] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x15, 0x12, 0x05, 0x00,
    0x12, 0x12, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x07,
    0x07, 0x08, 0x08, 0x09, 0x0b, 0x0b, 0x0b, 0x0d,
    0x0e, 0x0e, 0x0e, 0x0e, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x14,
    0x15, 0x15, 0x15, 0x16, 0x16, 0x17, 0x18, 0x18,
    0x19, 0x1a, 0x1b, 0x1b, 0x1c, 0x1e, 0x1e, 0x1e,
    0x1e, 0x1e, 0x1f, 0x1f, 0x20, 0x01, 0x01, 0x11,
    0x11, 0x00
};

u8 D_3fce_hiddenLevel[0x72] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xff, 0xff, 0x01, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02,
    0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02,
    0x02, 0x00
};

u8 D_4040_hiddenX[0x72] =
{
    0xe9, 0xe9, 0xe9, 0xe9, 0xe9, 0xe9, 0xe9, 0xe9,
    0xe9, 0xe9, 0xe9, 0xe9, 0x0f, 0x08, 0x02, 0x40,
    0x07, 0x17, 0x08, 0x19, 0x19, 0x17, 0x0d, 0x0e,
    0x10, 0x11, 0x18, 0x1b, 0x1b, 0x02, 0x06, 0x18,
    0x15, 0x14, 0x1d, 0x1e, 0x01, 0x1d, 0x16, 0x01,
    0x0d, 0x0e, 0x0e, 0x10, 0x12, 0x10, 0x1b, 0x14,
    0x1b, 0x0d, 0x0c, 0x08, 0x08, 0x07, 0x18, 0x05,
    0x06, 0x15, 0x19, 0x0a, 0x15, 0x0e, 0x10, 0x18,
    0x10, 0x0e, 0x11, 0x0e, 0x14, 0x15, 0x16, 0x17,
    0x15, 0x07, 0x14, 0x11, 0x0d, 0x0e, 0x10, 0x11,
    0x13, 0x03, 0x05, 0x07, 0x07, 0x05, 0x03, 0x12,
    0x0d, 0x0e, 0x10, 0x0d, 0x03, 0x0f, 0x13, 0x13,
    0x19, 0x0b, 0x0b, 0x0a, 0x13, 0x17, 0x17, 0x14,
    0x16, 0x16, 0x06, 0x13, 0x08, 0x13, 0x14, 0x0c,
    0x0c, 0x00
};

u8 D_40b2_hiddenY[0x72] =
{
    0xe9, 0xe9, 0xe9, 0xe9, 0xe9, 0xe9, 0xe9, 0xe9,
    0xe9, 0xe9, 0xe9, 0xe9, 0x02, 0x06, 0x02, 0x50,
    0x06, 0x06, 0x05, 0x06, 0x08, 0x05, 0x0d, 0x0d,
    0x0d, 0x0d, 0x13, 0x03, 0x1d, 0x01, 0x1a, 0x06,
    0x10, 0x0a, 0x01, 0x17, 0x1d, 0x0b, 0x1a, 0x1a,
    0x02, 0x02, 0x04, 0x03, 0x02, 0x03, 0x16, 0x16,
    0x08, 0x0b, 0x0b, 0x15, 0x17, 0x17, 0x06, 0x02,
    0x07, 0x12, 0x15, 0x0c, 0x0f, 0x09, 0x0c, 0x02,
    0x0c, 0x10, 0x0c, 0x0c, 0x07, 0x07, 0x07, 0x07,
    0x0d, 0x12, 0x17, 0x12, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x02, 0x07, 0x07, 0x02, 0x07, 0x02, 0x19,
    0x02, 0x02, 0x02, 0x0d, 0x0c, 0x01, 0x16, 0x16,
    0x10, 0x04, 0x11, 0x11, 0x16, 0x09, 0x07, 0x07,
    0x13, 0x11, 0x03, 0x07, 0x15, 0x18, 0x18, 0x0c,
    0x0c, 0x00
};

u8 D_4124_treasureKind[8] = { 0x01, 0x02, 0x03, 0x04, 0x07, 0x08, 0x0d, 0x0f };

u8 D_412c_treasureMinLevel[8] = { 0x19, 0x03, 0x11, 0x11, 0x09, 0x0f, 0x07, 0x07 };

u8 D_4134_treasureMaxAmount[8] = { 0x0a, 0x5a, 0x08, 0x08, 0x02, 0x02, 0x02, 0x02 };

u8 D_413c_gearKind[0x30] =
{
    0x09, 0x09, 0x09, 0x09, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x0a, 0x0a, 0x0a, 0x0c, 0x0c, 0x0c
};

u8 D_416c_gearMinLevel[0x30] =
{
    0x0a, 0x0a, 0x0f, 0x14, 0x0a, 0x0f, 0x14, 0x1c,
    0xff, 0x0f, 0x0f, 0x14, 0x14, 0x14, 0x18, 0xff,
    0x05, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
    0x0f, 0x0f, 0x0f, 0x0a, 0x0f, 0x0a, 0x14, 0x14,
    0x14, 0x14, 0x14, 0xff, 0x17, 0x17, 0x17, 0xff,
    0xff, 0xff, 0x17, 0x17, 0x17, 0x17, 0x0f, 0xff
};

char* D_419c_potionColors[8] =
{
    _STATIC_TEXT(0x8bea, "blue"),
    _STATIC_TEXT(0x8bf0, "yellow"),
    _STATIC_TEXT(0x8bf8, "red"),
    _STATIC_TEXT(0x8bfc, "green"),
    _STATIC_TEXT(0x8c02, "orange"),
    _STATIC_TEXT(0x8c0a, "purple"),
    _STATIC_TEXT(0x8c12, "black"),
    _STATIC_TEXT(0x8c18, "white")
};

char* D_41ac_scrollRunes[8] =
{
    _STATIC_TEXT(0x8c1e, "VL"),
    _STATIC_TEXT(0x8c22, "RH"),
    _STATIC_TEXT(0x8c26, "IS"),
    _STATIC_TEXT(0x8c2a, "IA"),
    _STATIC_TEXT(0x8c2e, "IQW"),
    _STATIC_TEXT(0x8c32, "KXC"),
    _STATIC_TEXT(0x8c36, "IMC"),
    _STATIC_TEXT(0x8c3a, "AT")
};

u8 D_41bc_dungeonLootOdds[8] = {0x02, 0x04, 0x05, 0x0a, 0x14, 0x19, 0x19, 0x00};

u8 D_41c4_dungeonLootMax[8] = {0x1f, 0x00, 0x03, 0x03, 0x03, 0x07, 0x07, 0x00};

u8 D_41cc_dungeonLootKind[8] = {0x0f, 0x02, 0x07, 0x08, 0x0d, 0xff, 0xff, 0x00};

// CHECKED (nop)
// takes an object (tile kind, amount, actor slot) into the party's goods: food, gold, torches, gems, keys, scrolls, potions, gear, the shards, crown, sceptre, amulet, a carpet, the sandalwood box; the actor is removed
static void SJOG_1458_GetObject(int kind, int amount, int slot)
{
    switch (kind)
    {
    case TILE_ACTOR_CHEST:
        ULTIMA_1850_PrintString(_TEXT(0x8c3e, "Open it first!\n"));
        return;

    case TILE_ACTOR_MOONSTONE:
        ULTIMA_1850_PrintString(_TEXT(0x8c4e, "A moonstone!\n"));
        D_5840_moonstoneMap[amount] = 0xff;
        break;

    case TILE_ACTOR_CARPET:
        ULTIMA_1850_PrintString(_TEXT(0x8c5c, "A magic carpet!\n"));
        D_57b0_carpets++;
        if (D_57b0_carpets == 100)
        {
            D_57b0_carpets = 99;
        }

        if (D_5893_mapId != 0x11)
            break;

        TOWN_00b0_DespawnNpc(0x16);
        break;

    case TILE_ACTOR_FOOD:
        ULTIMA_1a3e_PrintNumber(amount, 1, 0x20);
        ULTIMA_1850_PrintString(_TEXT(0x8c6e, " food!\n"));
        ULTIMA_3f14_IncreaseInt(&D_57a8_food, amount, 9999);
        break;

    case TILE_ACTOR_SANDALWOOD_BOX:
        ULTIMA_1850_PrintString(_TEXT(0x8c76, "A sandalwood box!\n"));
        D_57bf_sandalwoodBox = 0xff;
        D_5b5a_npcKilled[0x43] |= 0x80;
        break;

    case TILE_ACTOR_TORCH:
        ULTIMA_1a3e_PrintNumber(amount, 1, 0x20);
        ULTIMA_1850_PrintString(_TEXT(0x8c8a, " torch"));
        if (amount == 1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8c92, "!\n"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8c96, "es!\n"));
        }
        ULTIMA_3ef0_IncreaseByte(&D_57ae_torches, amount, 99);
        break;

    case TILE_ACTOR_GEM:
        ULTIMA_1a3e_PrintNumber(amount, 1, 0x20);
        ULTIMA_1850_PrintString(_TEXT(0x8c9c, " gem"));
        if (amount == 1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8ca2, "!\n"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8ca6, "s!\n"));
        }

        ULTIMA_3ef0_IncreaseByte(&D_57ad_gems, amount, 99);
        break;

    case TILE_ACTOR_KEY:
        if (amount > 0x7f)
        {
            amount &= 0x7f;
            ULTIMA_1a3e_PrintNumber(amount, 1, 0x20);
            ULTIMA_1850_PrintString(_TEXT(0x8caa, " odd key"));
            ULTIMA_3ef0_IncreaseByte(&D_57b1_skullKeys, amount, 99);
        }
        else
        {
            ULTIMA_1a3e_PrintNumber(amount, 1, 0x20);
            ULTIMA_1850_PrintString(_TEXT(0x8cb4, " key"));
            ULTIMA_3ef0_IncreaseByte(&D_57ac_keys, amount, 99);
        }

        if (amount == 1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8cba, "!\n"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8cbe, "s!\n"));
        }
        break;

    case TILE_ACTOR_SCROLL:
        if (amount == 0xff)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8cc2, "The plans for the HMS Cape!\n"));
            D_57bb_hmsCapePlans = 0xff;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8ce0, "A scroll: "));
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1850_PrintString(D_41ac_scrollRunes[amount & 7]);
            ULTIMA_1c9e_SelectCharset(0);
            ULTIMA_1850_PrintString(_TEXT(0x8cec, "!\n"));
            D_5820_scrolls[amount]++;
            if (D_5820_scrolls[amount] == 100)
            {
                D_5820_scrolls[amount] = 99;
            }
        }
        break;

    case TILE_ACTOR_GOLD:
        ULTIMA_1a3e_PrintNumber(amount, 1, 0x20);
        ULTIMA_1850_PrintString(_TEXT(0x8cf0, " gold!\n"));
        ULTIMA_3f14_IncreaseInt(&D_57aa_gold, amount, 9999);
        break;

    case TILE_ACTOR_POTION:
        ULTIMA_1850_PrintString(_TEXT(0x8cf8, "A "));
        ULTIMA_1850_PrintString(D_419c_potionColors[amount]);
        ULTIMA_1850_PrintString(_TEXT(0x8cfc, " potion!\n"));
        D_5828_potions[amount]++;
        if (D_5828_potions[amount] == 100)
        {
            D_5828_potions[amount] = 99;
        }
        break;

    case TILE_ACTOR_WEAPON:
    case TILE_ACTOR_SHIELD:
    case TILE_ACTOR_HELM:
    case TILE_ACTOR_RING:
    case TILE_ACTOR_ARMOUR:
    case TILE_ACTOR_AMULET:
        if (amount == 0x1b || amount == 0x1d)
        {
            ULTIMA_3ef0_IncreaseByte(&D_57c0_equipment[amount], 5, 99);
        }
        else
        {
            D_57c0_equipment[amount]++;
            if (D_57c0_equipment[amount] == 100)
            {
                D_57c0_equipment[amount] = 99;
            }
        }

        ULTIMA_1850_PrintString(D_17f6_equipNames[amount]);
        ULTIMA_1850_PrintString(_TEXT(0x8d06, "!\n"));
        break;

    case TILE_ACTOR_SHARD:
        D_57b6_shards[amount & 3] = 0xff;
        ULTIMA_1850_PrintString(_TEXT(0x8d0a, "The Shard of\n"));
        switch (amount & 3)
        {
        case 0:
            ULTIMA_1850_PrintString(_TEXT(0x8d18, "Falsehood!\n"));
            break;
        case 1:
            ULTIMA_1850_PrintString(_TEXT(0x8d24, "Hatred!\n"));
            break;
        default:
            ULTIMA_1850_PrintString(_TEXT(0x8d2e, "Cowardice!\n"));
            break;
        }
        break;

    case TILE_ACTOR_CROWN:
        D_57b4_crown = 0xff;
        ULTIMA_1850_PrintString(_TEXT(0x8d3a, "The Crown of Lord British!\n"));
        TOWN_0052_SetNpcKilled(TOWN_011e_FindNpcByActor(slot));
        TOWN_00b0_DespawnNpc(TOWN_011e_FindNpcByActor(slot));
        break;

    case TILE_ACTOR_SCEPTRE:
        D_57b5_sceptre = 0xff;
        ULTIMA_1850_PrintString(_TEXT(0x8d56, "The Sceptre of Lord British!\n"));
        break;

    case TILE_ACTOR_AMULET_LB:
        D_57b3_amulet = 0xff;
        ULTIMA_1850_PrintString(_TEXT(0x8d74, "The Amulet of Lord British!\n"));
        break;

    default:
        ULTIMA_1850_PrintString(_TEXT(0x8d92, "Nothing to get!\n"));
        return;
    }

    if (slot < 0x20)
    {
        ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, slot);
    }

    D_24e6_viewDirty |= 2;
    D_a9fa_vitalsDirty = 1;
}

// CHECKED (nop)
// Get in a dungeon: an open chest (0x7?) gives its treasure, by level
static void SJOG_179e_GetInDungeon(void)
{
    int cell_i;
    int index;
    int gearType;
    int amount_l;

    ULTIMA_1850_PrintString(_TEXT(0x8da4, "Get\n"));

    cell_i = GetDungeonMap2(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    if ((cell_i & 0xf0) == 0x40)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8daa, "Must open first!\n"));
        return;
    }

    if ((cell_i & 0xf0) == 0x70)
    {
        GetDungeonMap2(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 8;

        ULTIMA_1850_PrintString(_TEXT(0x8dbc, "contents\nof chest\nYou find:\n"));

        for (index = 0; index < 7; index++)
        {
            if (D_41bc_dungeonLootOdds[index] <= ULTIMA_2092_RandomRange(1, D_5895_mapLevel * 4 + 4))
            {
                if (index == 5)
                {
                    gearType = ULTIMA_2092_RandomRange(0, 7);
                    SJOG_1458_GetObject(3, gearType, 0x20);
                }
                else if (index == 6)
                {
                    gearType = ULTIMA_2092_RandomRange(0, 7);
                    SJOG_1458_GetObject(4, gearType, 0x20);
                }
                else
                {
                    if (index == 1)
                    {
                        amount_l = ULTIMA_2092_RandomRange(1, D_5895_mapLevel << 3);
                    }
                    else
                    {
                        amount_l = ULTIMA_2092_RandomRange(1, D_41c4_dungeonLootMax[index]);
                    }

                    SJOG_1458_GetObject(D_41cc_dungeonLootKind[index], amount_l, 0x20);
                }
            }
        }

        return;
    }

    ULTIMA_1850_PrintString(_TEXT(0x8dda, "Not here!\n"));
}

// CHECKED (loop)
// the Get command: in a dungeon its own; else a direction, then an object there, a torch from a wall, or food from crops and tables
void SJOG_18ce_GetCmd(void)
{
    int stepY;
    int targetY;
    int stepX_l;
    int targetX;
    int tile_b;
    int index_h;
    int value_c;

    if (D_5893_mapId > 0x20 && D_5893_mapId < 0x29)
    {
        SJOG_179e_GetInDungeon();
        return;
    }

    if (ULTIMA_35ec_SelectDirection() == 0)
    {
        return;
    }

    stepX_l = D_5876_tempX;
    stepY = D_5878_tempY;
    targetX = stepX_l + D_5896_mapX;
    targetY = stepY + D_5897_mapY;
    ULTIMA_1850_PrintString(_TEXT(0x8de6, "\n"));

    // 1921 (NOT MATCHING: loop)
    for (index_h = 1; index_h < 0x20; index_h++)
    {
        if (D_5c5a_actors[index_h].x == targetX && D_5c5a_actors[index_h].y == targetY &&
            (D_5893_mapId > 0x7f || (D_5893_mapId < 0x80 && D_5c5a_actors[index_h].z == D_5895_mapLevel)))
        {
            tile_b = D_5c5a_actors[index_h].tile;
            if (tile_b < TILE_ACTOR_HORSE || tile_b == TILE_ACTOR_MOONSTONE || tile_b == TILE_ACTOR_CARPET || (tile_b & 0xfc) == TILE_ACTOR_SHARD)
                break;
        }
    }

    // 197f (NOT MATCHING)
    if (index_h < 0x20)
    {
        value_c = D_5c5a_actors[index_h].value;
        SJOG_1458_GetObject(tile_b, value_c, index_h);
        return;
    }

    // 19c0
    tile_b = *ULTIMA_4402_GetTileAddr(targetX, targetY);
    switch (tile_b)
    {
    case TILE_MAP_B0:
    case TILE_MAP_B1:
        // 19e8
        *ULTIMA_4402_GetTileAddr(targetX, targetY) = TILE_MAP_44;
        D_24e6_viewDirty = 1;
        if (D_5893_mapId < 0x80)
        {
            ULTIMA_5e4a_BuildLightMap();
        }
        D_58a7_torchTurns = 100;
        ULTIMA_1850_PrintString(_TEXT(0x8de8, "Borrowed!\n"));
        ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
        ULTIMA_5910_UpdateFrame();
        break;

    case TILE_MAP_CROPS:
        // 1a2a
        *ULTIMA_4402_GetTileAddr(targetX, targetY) = TILE_MAP_CROPS_PICKED;
        D_24e6_viewDirty |= 2;
        ULTIMA_1850_PrintString(_TEXT(0x8df4, "Crops picked!\n"));

        ULTIMA_3f14_IncreaseInt(&D_57a8_food, 1, 9999);

        D_a9fa_vitalsDirty = 1;

        if (D_5888_karma != 0)
        {
            D_5888_karma--;
        }
        break;

    case TILE_MAP_TABLE_9A:
        // 1a6a
        if (stepY == 1)
        {
            *ULTIMA_4402_GetTileAddr(targetX, targetY) = TILE_MAP_TABLE_95;
            D_24e6_viewDirty |= 2;
            ULTIMA_1850_PrintString(_TEXT(0x8e04, "Mmmmm...!\n"));

            ULTIMA_3f14_IncreaseInt(&D_57a8_food, 1, 9999);

            D_a9fa_vitalsDirty = 1;

            if (D_5888_karma != 0)
            {
                D_5888_karma--;
            }
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8e10, "Can't reach plate!\n"));
        }

        break;

    case TILE_MAP_TABLE_9B:
        // 1a92
        if (stepY == -1)
        {
            *ULTIMA_4402_GetTileAddr(targetX, targetY) = TILE_MAP_TABLE_95;
            D_24e6_viewDirty |= 2;
            ULTIMA_1850_PrintString(_TEXT(0x8e24, "Mmmmm...!\n"));

            ULTIMA_3f14_IncreaseInt(&D_57a8_food, 1, 9999);
            if (D_5888_karma != 0)
            {
                D_5888_karma--;
            }
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8e30, "Can't reach plate!\n"));
        }

        break;

    case TILE_MAP_TABLE_9C:
        // 1aca
        if (stepX_l == 1 || stepX_l == -1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8e44, "Can't reach plate!\n"));
            break;
        }

        if (stepY == 1)
        {
            *ULTIMA_4402_GetTileAddr(targetX, targetY) = TILE_MAP_TABLE_9B;
        }

        if (stepY == -1)
        {
            *ULTIMA_4402_GetTileAddr(targetX, targetY) = TILE_MAP_TABLE_9A;
        }

        D_24e6_viewDirty |= 2;
        ULTIMA_1850_PrintString(_TEXT(0x8e58, "Mmmmm...!\n"));

        ULTIMA_3f14_IncreaseInt(&D_57a8_food, 1, 9999);
        if (D_5888_karma != 0)
        {
            D_5888_karma--;
        }

        break;

    default:
        // 1b28
        ULTIMA_1850_PrintString(_TEXT(0x8e64, "Nothing to get!\n"));
        break;
    }
}

// takes the item off every party member who has it equipped; how many did
int SJOG_1b34_UnequipAll(int item)
{
    int count;
    int member;

    count = 0;

    for (member = 0; member < D_585b_partyCount; member++)
    {
        if (ULTIMA_6e60_Unequip(member, item) != 0)
        {
            count++;
        }
    }

    return count;
}

// counts the living combatants: the enemies into D_5876, the party's side into D_5878
void SJOG_1b6c_CountCombatants(void)
{
    int isEnemy;
    int index;

    D_5876_tempX = D_5878_tempY = 0;

    for (index = 0; index < 0x20; index++)
    {
        if (D_ba14_combatants[index].flags != 0 && (D_ba14_combatants[index].flags & COMBAT_FLAGS_DEAD) == 0)
        {
            isEnemy = ULTIMA_5646_IsEnemy(index);
            if (isEnemy != 0)
            {
                D_5876_tempX++;
            }
            else
            {
                D_5878_tempY++;
            }
        }
    }
}

// a combatant leaves the field (not on a ship): the first player's direction fixes where the party leaves; 1 when it left
static int SJOG_1bb2_CombatExit(int combatant, int dir)
{
    if ((D_587c_partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8e76, "\nStay with ship!\n"));
        return 0;
    }

    if ((D_ba14_combatants[combatant].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        if (D_58a0_combatExitDir == 0)
        {
            D_58a0_combatExitDir = dir;
        }
        else if (dir != D_58a0_combatExitDir && (D_58a1_combatFlags & 0x80) != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x8e88, "\nAll must use the same exit!\n"));
            ULTIMA_22c0_AudioTone(0xa5, 200);
            return 0;
        }
    }

    D_589f_focusBlink = 1;
    SJOG_1b6c_CountCombatants();
    if (D_5876_tempX == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8ea6, "Leave!\n"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x8eae, "Escape!\n"));
    }

    ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
    D_587b_activePlayer = 0xff;
    COMBAT_1236_RemoveEntity(-combatant - 1);
    ULTIMA_2900_UpdateVitalsDisplay();
    ULTIMA_5910_UpdateFrame();

    return 1;
}

// move player in combat
// param_1: combat entity idx, param_2: direction
// moves a combatant of the party one cell in a direction: off the map it leaves; a blocked cell sounds a tone; 1 when it moved
int SJOG_1c56_CombatMovePlayer(int combatant, int dir)
{
    int actorIndex;
    int stepY;
    int stepX;
    int destY_g;
    int destX_h;

    stepX = stepY = 0;
    actorIndex = D_ba14_combatants[combatant].actorIdx;

    switch (dir)
    {
    case DIR_UP:
        stepY = -1;
        ULTIMA_1850_PrintString(_TEXT(0x8eb8, "North\n"));
        break;
    case DIR_DOWN:
        stepY = 1;
        ULTIMA_1850_PrintString(_TEXT(0x8ec0, "South\n"));
        break;
    case DIR_RIGHT:
        stepX = 1;
        ULTIMA_1850_PrintString(_TEXT(0x8ec8, "East\n"));
        break;
    case DIR_LEFT:
        stepX = -1;
        ULTIMA_1850_PrintString(_TEXT(0x8ece, "West\n"));
        break;
    }

    destX_h = D_ba14_combatants[combatant].x + stepX;
    destY_g = D_ba14_combatants[combatant].y + stepY;
    if (destX_h > 0xa || destY_g > 0xa || destX_h < 0 || destY_g < 0)
    {
        return SJOG_1bb2_CombatExit(combatant, dir);
    }

    if (COMBAT_0000_IsCellFree(D_5c5a_actors[actorIndex].tile, destX_h, destY_g) != 0)
    {
        D_5c5a_actors[actorIndex].x = D_ba14_combatants[combatant].x = destX_h;
        D_5c5a_actors[actorIndex].y = D_ba14_combatants[combatant].y = destY_g;

        ULTIMA_433e_AudioFootstep();
        if ((D_58a1_combatFlags & 0x82) != 0)
        {
            COMBAT_111a_FireMapTrigger(destX_h, destY_g);
        }

        return 1;
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x8ed4, "Blocked!\n"));
        ULTIMA_22c0_AudioTone(0xa5, 200);
        ULTIMA_1b16_ClearKbdBuffer();
        return 0;
    }
}

// CHECKED
// klimb in combat
// Klimb in combat: a ladder up or down leaves the field (up 5, down 6); or one step across a 0x4c tile
int SJOG_1d6a_CombatKlimb(void)
{
    int key;
    int targetY;
    int targetX;
    ActorFmt* actor_g;
    int tile_b;

    ULTIMA_1850_PrintString(_TEXT(0x8ede, "Klimb-"));

    tile_b = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);

    if ((D_58a1_combatFlags & 2) != 0 && tile_b == TILE_MAP_LADDER_UP && D_bb16_altTileset != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8ee6, "U/D-"));
        do
        {
            key = ULTIMA_266c_GetChar();
            switch (key)
            {
            case U5_KEY_UP:
            case 0x55:
                // -> dd4b
                break;

            default:
                // dd44
                key = 0;
                break;

            case U5_KEY_DOWN:
            case 0x44:
                // dd48
                tile_b = TILE_MAP_LADDER_DOWN;
                break;
            }
            // dd4b
        } while (key == 0);
    }

    if (tile_b == TILE_MAP_LADDER_UP)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8eec, "Up!\n"));

        return SJOG_1bb2_CombatExit(D_589e_curCombatant, 5);
    }

    if ((tile_b == TILE_MAP_86 && (D_58a1_combatFlags & 0x80) != 0) || tile_b == TILE_MAP_LADDER_DOWN)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8ef2, "Down!\n"));

        return SJOG_1bb2_CombatExit(D_589e_curCombatant, 6);
    }

    if (ULTIMA_35ec_SelectDirection() != 0)
    {
        targetX = D_5896_mapX + D_5876_tempX;
        targetY = D_5897_mapY + D_5878_tempY;
        if (*ULTIMA_4402_GetTileAddr(targetX, targetY) == TILE_MAP_4C && ULTIMA_3702_FindLastActorTileAt(targetX, targetY, 0) == 0)
        {
            actor_g = &D_5c5a_actors[D_ba14_combatants[D_589e_curCombatant].actorIdx];
            actor_g->x = D_ba14_combatants[D_589e_curCombatant].x = targetX;
            actor_g->y = D_ba14_combatants[D_589e_curCombatant].y = targetY;

            COMBAT_111a_FireMapTrigger(targetX, targetY);
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x8efa, "What?\n"));
            return 0;
        }
    }

    return 1;
}

// a combatant who steps on the row-2 mirror of the endgame room leaves the field (exit code 0x4d)
void SJOG_1ea4_CheckMirror(void)
{
    CombatEntity* combatant;

    combatant = &D_ba14_combatants[D_589e_curCombatant];

    if (combatant->flags != 0 && (combatant->flags & COMBAT_FLAGS_DEAD) == 0 && combatant->y == 2 &&
        (D_ac64_viewActors[combatant->x + 0x10] & 0xfc) == TILE_ACTOR_MIRROR)
    {
        D_58a0_combatExitDir = 0x4d;
        ULTIMA_16ba_PrintChar('\n');
        COMSUBS_0094_PrintName(D_589e_curCombatant);
        ULTIMA_1850_PrintString(_TEXT(0x8f02, " is absorbed!\n"));
        ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
        D_587b_activePlayer = 0xff;
        ULTIMA_2900_UpdateVitalsDisplay();
        COMBAT_1236_RemoveEntity(-D_589e_curCombatant - 1);
        ULTIMA_5910_UpdateFrame();
    }
}

// a command the world has and combat refuses: prints its word, a message, two tones; always 1
int SJOG_1f26_CombatMiscCmd(char* word, int kind)
{
    ULTIMA_1850_PrintString(word);

    switch (kind)
    {
    case 1:
        ULTIMA_1850_PrintString(_TEXT(0x8f12, " what?"));
        break;
    case 2:
        ULTIMA_1850_PrintString(_TEXT(0x8f1a, "-Not here"));
        break;
    case 3:
        ULTIMA_1850_PrintString(_TEXT(0x8f24, "-Funny, no response!"));
        break;
    }

    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_22c0_AudioTone(0xdc, 0x96);
    ULTIMA_22c0_AudioTone(0x96, 0x96);

    return 1;
}

// a number key in combat selects that party member as the active player when he or she can act; 1 when it did
int SJOG_1f7a_CombatSetActivePlayer(int member)
{
    int canAct;
    int index;
    int flags_c;

    canAct = 0;
    ULTIMA_1850_PrintString(_TEXT(0x8f3a, "Set active plr:\n"));
    
    for (index = 0; index < 0x20; index++)
    {
        flags_c = D_ba14_combatants[index].flags;

        if ((flags_c & COMBAT_FLAGS_PLAYER) != 0)
        {
            if (ULTIMA_5646_IsEnemy(index) == 0 && D_ba14_combatants[index].entityIdx == member)
            {
                if ((flags_c & (COMBAT_FLAGS_DEAD | COMBAT_FLAGS_ASLEEP | COMBAT_FLAGS_4)) != 0)
                {
                    canAct = 0;
                }
                else
                {
                    canAct = 1;
                }

                break;
            }
        }
    }

    if (canAct != 0)
    {
        D_587b_activePlayer = member;
        COMSUBS_0094_PrintName(index);
        ULTIMA_16ba_PrintChar('\n');
        ULTIMA_2900_UpdateVitalsDisplay();
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x8f4c, "Invalid!\n"));
    }

    return canAct;
}

// at a combatant's turn: the ring, fading fields, the protection spell's turns count down
void SJOG_2012_TurnUpkeep(void)
{
    ULTIMA_6794_ApplyRing(D_589e_curCombatant);
    COMSUBS_0056_FadeFields();

    if (D_588e_protectTurns != 0 && D_588e_protectTurns != 0xff)
    {
        if (--D_588e_protectTurns == 0)
        {
            D_587a_statusIcon = 0;
            D_a9fa_vitalsDirty = 1;
        }
    }
}

// CHECKED
// the combat is over: the actors are put back; the monster's actor is removed (a pirate ship sinks to a wreck); the map id is restored
void SJOG_203e_EndCombat(int actor)
{
    int index;

    ActorFmt* monster;

    if (D_58a0_combatExitDir == 0x4d)
    {
        ENDGAME_0648_EndgameMain();
    }

    for (index = 0; index < 0x20; index++)
    {
        D_5c5a_actors[index] = D_a9fc_savedActors[index];
    }

    if ((D_58a1_combatFlags & 0x82) == 0)
    {
        if (actor < 0x20)
        {
            monster = &D_5c5a_actors[actor];

            if (D_58a3_combatOver != 0 && (monster->tile & 0xfc) == TILE_ACTOR_PIRATE)
            {
                monster->tile -= 8;
                monster->animTile -= 8;
                monster->value = 99;
                monster->extra = 2;
            }
            else
            {
                monster->tile = monster->animTile = monster->x = monster->y = monster->z = 0;
            }
        }

        ULTIMA_16ba_PrintChar('\n');
        D_5893_mapId = D_5894_savedMapId;
    }
}

// 1 when the cell (x, y) is not free for the combatant (0 off the map when it may leave)
int SJOG_20d8_IsCellBlocked(int x, int y, int combatant)
{
    int blocked;
    CombatEntity* entity_c;

    blocked = 1;
    entity_c = &D_ba14_combatants[combatant];

    if (x > -1 && x < 11 && y > -1 && y < 11)
    {
        blocked = COMBAT_0000_IsCellFree(D_5c5a_actors[entity_c->actorIdx].tile, x, y);
        blocked = 1 - blocked;
    }
    else
    {
        if ((entity_c->flags & COMBAT_FLAGS_2) != 0)
        {
            blocked = 0;
        }
    }

    return blocked;
}

// CHECKED
// 1 when all four neighbouring cells of the combatant are blocked (the probing walk is as in the original)
int SJOG_2148_IsSurrounded(int combatant)
{
    int dir;
    int y;
    int x;
    int blockedCount_b;
    CombatEntity* entity_c;

    blockedCount_b = 0;
    entity_c = &D_ba14_combatants[combatant];

    x = entity_c->x;
    y = entity_c->y;

    dir = 0;
    do
    {
        switch (dir)
        {
        case 0:
            y++;
            break;
        case 2:
            // bug?
            x--;
            x--;
            break;
        case 1:
        case 3:
            y--;
            x++;
            break;
        }

        if (SJOG_20d8_IsCellBlocked(x, y, combatant) != 0)
        {
            blockedCount_b++;
        }
    } while (++dir < 4);

    return blockedCount_b == 4;
}

// CHECKED
// the first charmed party member among the first six is freed: sound, the item 0x23 taken off, put to sleep; the index or -1
int SJOG_21ce_BreakCharm(void)
{
    int index;

    for (index = 0; index < 6; index++)
    {
        if ((D_ba14_combatants[index].flags & COMBAT_FLAGS_CHARMED) != 0 && (D_ba14_combatants[index].flags & COMBAT_FLAGS_PLAYER) != 0)
        {
            D_ba14_combatants[index].flags &= ~COMBAT_FLAGS_CHARMED;

            ULTIMA_1850_PrintString(D_55a8_party[D_ba14_combatants[index].entityIdx].name);
            ULTIMA_1850_PrintString(_TEXT(0x8f56, " passes out!"));
            ULTIMA_2192_AudioPulse(0xc1c, 1, 30000, 1000, 2);
            ULTIMA_6e60_Unequip(D_ba14_combatants[index].entityIdx, 0x23);
            ULTIMA_68ae_PutToSleep(index);
            break;
        }
    }

    if (index == 6)
        index = -1;

    return index;
}
