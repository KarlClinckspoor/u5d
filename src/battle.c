#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "sjog.h"

#include <string.h>

// In the original, ULTIMA_5f86 is the first function of this object (MSC 5.0): it is
// the right length only when compiled with 5.0, and the object before ends at 5f86.
// CHECKED
// mapFlags: special map type (flags)
// - 0: combat
// - 2: combat in dungeon?
// - 4: camping/hole up
// - 0x80: ?
void SpecialMapHandler(int mapFlags, int actorOrGuard, int hours)
{
    int i_e;
    int savedY;
    int savedLevel;
    int savedX;
    int enemyType;
    int savedPlayer; // active character

    combatFlags = mapFlags;
    savedX = D_5896_mapX;
    savedY = D_5897_mapY;
    savedLevel = D_5895_mapLevel;

    D_5894_savedMapId = mapId;
    savedPlayer = activePlayer;
    mapId = 0xff;

    // copy actors
    for (i_e = 0; i_e < 0x20; i_e++)
    {
        savedActors[i_e] = actors[i_e];
    }

    if (mapFlags == 0)
    {
        if ((actors[actorOrGuard].tile & 0xfc) == TILE_ACTOR_PIRATE)
        {
            enemyType = 0x1;
        }
        else
        {
            enemyType = ((actors[actorOrGuard].tile - TILE_ACTOR_WIZARD) >> 2);
        }

        if (actors[actorOrGuard].value > 0x7f)
        {
            enemyType += 0x100;
        }

        // 6021
        SetUpCombat(1, enemyType); // ***CONFLICT***
    }
    else
    {
        if ((mapFlags & 4) != 0) // 602e
        {
            // 6034
            // hole up
            if (HoleUpRest(mapFlags, actorOrGuard, hours) != 0)
            {
                goto L_605c;
            }
            goto L_606c;
        }
        else if ((mapFlags & 2) != 0) // 6046
        {
            enemyType = actorOrGuard;
            PlaceCombatants(0, 2);
        }
        else
        {
            goto L_606c;
        }
    }

    // 605c
L_605c:
    curCombatant = activePlayer = 0xff;
    combatOver = 0;

    CombatMainLoop(); // COMBAT.0b94

L_606c:
    // 606c
    if (altTileset != 0)
    {
        altTileset = 0;
        TilesetMode(1);
    }

    D_5896_mapX = savedX;
    D_5897_mapY = savedY;
    D_5895_mapLevel = savedLevel;
    mapId = D_5894_savedMapId;
    viewDirty = 1;

    UpdateVitalsDisplay();

    if (party[savedPlayer].status != STATUS_DEAD && party[savedPlayer].status != STATUS_SLEEP)
    {
        activePlayer = savedPlayer;
    }
    else
    {
        activePlayer = 0xff;
    }

    for (i_e = 0; i_e < 0x20; i_e++)
    {
        actors[i_e] = savedActors[i_e];
    }
}

// NOTE: combine arrays?
static void LoadSpecialMap(int mapIndex)
{
    int i;

    ReadFileFromDisk(/*0xa3f0*/ "BRIT.CBT", combatMap, 0x160, mapIndex * 0x160);

    for (i = 0; i < 6; i++)
    {
        combatPartyX[i] = GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_SOUTH, i);
        combatPartyY[i] = GetCombatMapPartyY(COMBAT_MAP_SPECIAL_PARTY_SOUTH, i);
    }

    for (i = 0; i < 0x10; i++)
    {
        combatEnemyX[i] = GetCombatMapMonsterX(i);
        combatEnemyY[i] = GetCombatMapMonsterY(i);
    }
}

void Combat(int actorIndex)
{
    int tileUnder_j;
    bool atSea_j;
    int actorTile;
    int mapIndex;

    actorTile = actors[actorIndex].tile & 0xfc;

    PrintChar('\n');
    PrintChar(CTRL_CHAR_CENTER_TEXT);
    if (actorTile < 0x40) // TILE_ACTOR_40
    {
        PrintString(/*0xa3fa*/ "PIRATES");
    }
    else
    {
        PrintString(enemyNamesPlural[(actorTile - 0x40) / 4]); // TILE_ACTOR_40
    }

    PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    PrintString(/*0xa402*/ "\n\n");

    tileUnder_j = *GetTileAddr(actors[actorIndex].x, actors[actorIndex].y);
    if (tileUnder_j < TILE_MAP_POISON || (tileUnder_j & 0xfe) != TILE_MAP_6A && ((tileUnder_j & 0xf0) == TILE_MAP_60))
    {
        atSea_j = true;
    }
    else
    {
        atSea_j = false;
    }

    if (actorTile == 0xfc) // 61f3
    {
        mapIndex = 10;
        if (sceptre != 0)
        {
            PrintString(/*0xa406*/ "The Sceptre is reclaimed!\n");
            AudioPulse(0xfd2, 1, 65000, 1, 1);
            sceptre = 0;
        }
    }
    else
    {
        if (((byte)actorTile & 0xf0) == 0x80) // 622c
        {
            atSea_j = 1;
        }

        if ((partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20) // 623a
        {
            if (actorTile == TILE_ACTOR_PIRATE)
            {
                mapIndex = 0xe;
            }
            else if (atSea_j)
            {
                mapIndex = 0xb;
            }
            else
            {
                mapIndex = 0xd;
            }
        }
        else if (actorTile == TILE_ACTOR_PIRATE)
        {
            mapIndex = 0xc;
        }
        else if (atSea_j)
        {
            mapIndex = 0xf;
        }
        else
        {
            switch (tileUnder_j)
            {
            case TILE_MAP_WATER_1:
            case TILE_MAP_WATER_2:
            case TILE_MAP_WATER_3:
                mapIndex = 0xf;
                break;
            case TILE_MAP_POISON:
                mapIndex = 1;
                break;
            case TILE_MAP_GRASS:
                mapIndex = 2;
                break;
            case TILE_MAP_6:
            case TILE_MAP_8:
                mapIndex = 3;
                break;
            case TILE_MAP_7:
            case TILE_MAP_1E:
            case TILE_MAP_1F:
                mapIndex = 4;
                break;
            case TILE_MAP_9:
            case TILE_MAP_A:
                mapIndex = 5;
                break;
            case TILE_MAP_B:
            case TILE_MAP_C:
            case TILE_MAP_D:
            case TILE_MAP_E:
            case TILE_MAP_F:
                mapIndex = 6;
                break;
            case TILE_MAP_1D:
            case TILE_MAP_48:
            case TILE_MAP_49:
            case TILE_MAP_6A:
            case TILE_MAP_6B:
                mapIndex = 7;
                break;
            case TILE_MAP_44:
                mapIndex = 8;
                break;
            default:
                if (mapId != 0)
                {
                    mapIndex = 8;
                }
                else
                {
                    mapIndex = 2;
                }
                break;
            }
        }
    }

    LoadSpecialMap(mapIndex);
    SpecialMapHandler(0, actorIndex, 0);

    EndCombat(actorIndex);
    BuildLightMap();
    AdvanceTime(0);
}

void Camping(int guard, int hours)
{
    LoadSpecialMap(0);
    SpecialMapHandler(4, guard, hours);
    BuildLightMap();
}

// draw frame
void DrawFrame(void)
{
    int unused; // unused
 
    // black
    GRAP_2d_SetPenColor(0);
    GRAP_3f_FillRect(0, 0, 319, 199);

    GRAP_2d_SetPenColor(blueColor);
    GRAP_3f_FillRect(0, 0, 319, 6);
    GRAP_3f_FillRect(0, 185, 191, 191);
    GRAP_3f_FillRect(0, 0, 6, 191);
    GRAP_3f_FillRect(185, 0, 191, 191);
    GRAP_3f_FillRect(313, 0, 319, 87);
    GRAP_3f_FillRect(192, 80, 312, 87);
    GRAP_3f_FillRect(192, 0x39, 312, 63);

    SetTextForegroundColor(blueColor);
    SetTextPosition(0, 0);
    PrintChar(0x7b);
    SetTextPosition(39, 0);
    PrintChar(0x7c);
    SetTextPosition(0, 23);
    PrintChar(0x7d);

    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(7, 7, 7, 184);
    GRAP_LineTo(184, 184);
    GRAP_LineTo(184, 7);
    GRAP_LineTo(7, 7);
    GRAP_Line(191, 191, 191, 87);
    GRAP_LineTo(319, 87);
    GRAP_Line(191, 7, 312, 7);
    GRAP_LineTo(312, 56);
    GRAP_LineTo(191, 56);
    GRAP_LineTo(191, 7);
    GRAP_Line(191, 63, 312, 63);
    GRAP_LineTo(312, 80);
    GRAP_LineTo(191, 80);
    GRAP_LineTo(191, 63);

    SetTextForegroundColor(brightWhiteColor);
}

// CHECKED
// place actor on combat map
// kind: player / monster?
// return: placed actor idx
// Put a combatant on the combat map. kind 0: an enemy of type `entity`; 1: party member `entity`; 2: an object with tile `entity`. Returns its slot (for kind 2 the actor slot), -1 when full.
int PlaceCombatant(int entity, int kind, int cellX, int cellY, int mapLevel)
{
    int firstSlot;
    int result;
    int slot_n;
    CombatEntity* combatant_f;

    firstSlot = 0;
    slot_n = 0;
    result = -1;

    if (kind != 2)
    {
        if (kind == 0)
        {
            firstSlot = 6;
        }

        // 6531
        for (slot_n = firstSlot; slot_n < 0x20; slot_n++)
        {
            // 653f
            // 6577
            if (combatants[slot_n].flags == 0) // combat actor slot not occupied?
            {
                combatant_f = &combatants[slot_n];
                result = slot_n;

                // 6587
                if (kind == 1 && party[entity].status != STATUS_DEAD) // player?
                {
                    // 65a0
                    combatant_f->dex = party[entity].dex;
                    combatant_f->turnTimer = 36 - combatant_f->dex;
                    combatant_f->flags = COMBAT_FLAGS_PLAYER;
                    // 65b4
                    if (party[entity].status != STATUS_GOOD && party[entity].status != STATUS_POISONED)
                    {
                        combatant_f->flags |= COMBAT_FLAGS_ASLEEP;
                    }
                }

                // 65c5
                if (kind == 0) // monster?
                {
                    combatant_f->hp = D_13bc_enemyStats[entity].maxHp;
                    // slot_n reused as a scratch variable (the loop ends with the break below):
                    // the original keeps Random(7) - 4 in DI, slot_n's register, and the
                    // assignment also stops MSC from walking combatants with pointers.
                    slot_n = Random(7) - 4;
                    combatant_f->dex = slot_n + D_13bc_enemyStats[entity].dex;
                    if (combatant_f->dex > 0x1e)
                    {
                        combatant_f->dex = D_13bc_enemyStats[entity].dex;
                    }

                    combatant_f->turnTimer = 36 - combatant_f->dex;
                    combatant_f->flags = COMBAT_FLAGS_MONSTER;
                    if (entity == 8 || entity == 9)
                    {
                        // 6609
                        combatant_f->flags = COMBAT_FLAGS_DEAD;
                    }
                }

                // 660d
                combatant_f->entityIdx = entity;
                combatant_f->x = cellX;
                combatant_f->y = cellY;
                break;
            }
        }
    }

    // 6625
    if (result > -1 || kind == 2)
    {
        // 6634
        for (slot_n = 0; slot_n < 0x20; slot_n++)
        {
            if (actors[slot_n].tile == 0)
            {
                // 6685
                if (kind == 0)
                {
                    combatant_f->actorIdx = slot_n;
                    
                    actors[slot_n].animTile = actors[slot_n].tile = entity * 4 + TILE_ACTOR_WIZARD; // local_14
                }

                // 669a
                if (kind == 1)
                {
                    combatant_f->actorIdx = slot_n;
                }

                // 66a6
                if (kind == 2)
                {
                    actors[slot_n].animTile = actors[slot_n].tile = entity;
                    result = slot_n;
                }

                // 66bb
                actors[slot_n].x = cellX;
                actors[slot_n].y = cellY;
                actors[slot_n].z = mapLevel;
                actors[slot_n].extra = 0xff;
                if (kind == 0)
                {
                    actors[slot_n].value = combatant_f->hp;
                }
                else
                {
                    actors[slot_n].value = entity;
                }

                // 66f8
                if (kind != 2)
                {
                    combatant_f->actorIdx = slot_n;
                }

                break;
            }
        }

        // 6706
        if (kind != 2 && slot_n == 0x20 && result > -1)
        {
            result = -1;
        }
    }
    else
    {
        result = -1;
    }

    // 6751
    if (result >= 0)
    {
        for (slot_n = 0; slot_n < 0x20; slot_n++)
        {
            if (actors[combatants[slot_n].actorIdx].extra == result)
            {
                actors[combatants[slot_n].actorIdx].extra = 0xff;
            }
        }
    }

    return result;
}

// A ring worn into combat takes effect: invisibility (0x2a), regeneration (0x2c).
void ApplyRing(int combatantIndex)
{
    CombatEntity* combatant = &combatants[combatantIndex];
    if ((combatant->flags & COMBAT_FLAGS_PLAYER) != 0 && (combatant->flags & (COMBAT_FLAGS_DEAD | COMBAT_FLAGS_ASLEEP)) == 0)
    {
        if (party[combatant->entityIdx].equips[4] == 42)
        {
            actors[combatant->actorIdx].animTile = TILE_ACTOR_1D;
            combatant->flags |= COMBAT_FLAGS_INVISIBLE;
        }
        else if (party[combatant->entityIdx].equips[4] == 44)
        {
            RingRegeneration(combatant->entityIdx); // sic
        }
    }
}

// Wake a sleeping combatant.
void WakeCombatant(int combatantIndex)
{
    CombatEntity* combatant = &combatants[combatantIndex];
    if ((combatant->flags & COMBAT_FLAGS_ASLEEP) != 0)
    {
        if ((combatant->flags & COMBAT_FLAGS_PLAYER) != 0)
        {
            party[combatant->entityIdx].status = STATUS_GOOD;
            if ((combatant->flags & COMBAT_FLAGS_INVISIBLE) != 0)
            {
                actors[combatant->actorIdx].animTile = TILE_ACTOR_1D;
            }
            else
            {
                actors[combatant->actorIdx].animTile = actors[combatant->actorIdx].tile;
            }
        }
        else
        {
            actors[combatant->actorIdx].animState = 0;
        }
        combatant->flags &= ~COMBAT_FLAGS_ASLEEP;
    }

    UpdateVitalsDisplay();
}

// Put a combatant to sleep, unless a poisoned party member.
void SleepIfNotPoisoned(int combatantIndex)
{
    if ((combatants[combatantIndex].flags & COMBAT_FLAGS_PLAYER) == 0 || party[combatants[combatantIndex].entityIdx].status != STATUS_POISONED)
    {
        PutToSleep(combatantIndex);
    }
}

// Put a combatant to sleep.
void PutToSleep(int combatantIndex)
{
    CombatEntity* combatant = &combatants[combatantIndex];
    if ((combatant->flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        if (party[combatant->entityIdx].status != STATUS_DEAD)
        {
            party[combatant->entityIdx].status = STATUS_SLEEP;
            combatant->flags |= COMBAT_FLAGS_ASLEEP;
            actors[combatant->actorIdx].animTile = TILE_ACTOR_SLEEP;

            if (combatantIndex == activePlayer)
            {
                activePlayer = 0xff;
            }

            combatEvents = 4;

            UpdateVitalsDisplay();

            if ((combatFlags & 4) == 0)
            {
                UpdateFrame();
            }
        }
    }
    else
    {
        combatant->flags |= COMBAT_FLAGS_ASLEEP;
        combatEvents = 4;
        actors[combatant->actorIdx].animState = 0xff;

        UpdateFrame();
    }
}

// CHECKED
// initialize combat entities
// Clear the actors and the combatants and place the party on the combat map.
void InitCombatants(void)
{
    byte ringItem_i;
    byte startY;
    byte objectSlot_i;
    int combatantIndex;
    byte startX_m;
    int i_k;
    CombatEntity* entry;

    ringItem_i = 0;

    // clear
    for (i_k = 0; i_k < 0x20; i_k++)
    {
        entry = &combatants[i_k];

        actors[i_k].tile = actors[i_k].animTile = actors[i_k].x = actors[i_k].animState = 0;
        actors[i_k].y = actors[i_k].z = actors[i_k].value = 0;

        entry->hp = entry->dex = entry->flags = entry->entityIdx = 0;

        entry->actorIdx = entry->turnTimer = entry->x = entry->y = 0;
    }

    // place player party
    for (i_k = 0; i_k < partyCount; i_k++)
    {
        if (party[i_k].status != STATUS_DEAD)
        {
            if (party[i_k].equips[4] == 42)
            {
                ringItem_i = 42;
            }

            if (party[i_k].equips[4] == 44)
            {
                ringItem_i = 44;
            }

            if (ringItem_i != 0)
            {
                combatantIndex = RandomRange(0, 0xf);
                if (combatantIndex == 0xb)
                {
                    PrintString(/*0xa422*/ "A ring has vanished!\n");
                    AudioSweepTone(0x4b0, 2000, 1, 0x28);
                    Unequip(i_k, ringItem_i);
                }
                ringItem_i = 0;
            }

            startX_m = combatPartyX[i_k];
            startY = combatPartyY[i_k];
            combatantIndex = PlaceCombatant(i_k, 1, startX_m, startY, D_5895_mapLevel);

            actors[combatants[combatantIndex].actorIdx].extra = 0xff;

            // setup tile (according to class)
            switch (party[i_k].cls)
            {
            case CLASS_AVATAR:
                actors[combatants[combatantIndex].actorIdx].tile = TILE_ACTOR_4C;
                break;
            case CLASS_FIGHTER:
            case CLASS_PALADIN:
            case CLASS_RANGER:
                actors[combatants[combatantIndex].actorIdx].tile = TILE_ACTOR_FIGHTER;
                break;
            case CLASS_DRUID:
            case CLASS_MAGE:
                actors[combatants[combatantIndex].actorIdx].tile = TILE_ACTOR_WIZARD;
                break;
            case CLASS_BARD:
            case CLASS_SHEPHERD:
            case CLASS_TINKER:
                actors[combatants[combatantIndex].actorIdx].tile = TILE_ACTOR_BARD_44;
                break;
            }

            actors[combatants[combatantIndex].actorIdx].animTile = actors[combatants[combatantIndex].actorIdx].tile;

            if (party[i_k].status == STATUS_SLEEP)
            {
                PutToSleep(combatantIndex);
            }
            else
            {
                ApplyRing(combatantIndex);
            }
        }
    }

    if (GetCombatMap(5, 5) == 0xdc)
    {
        objectSlot_i = PlaceCombatant(1, 2, 5, 5, D_5895_mapLevel);
        actors[objectSlot_i].value = D_5895_mapLevel * 3 + 7;
        GetCombatMap(5, 5) = D_bb15_floorTile;
    }
}

// CHECKED
// prepare combat
// Set up a combat: the party (unless flag 4, camping), then the enemies of type `enemy`, as many as its statistics allow.
void SetUpCombat(int flags, int enemy)
{
    byte enemyY;
    int pick;
    byte enemyX;
    int i_n;
    int order_j[16];
    int fromWorld_i;
    int count_m;
    int kind;

    if (enemy >= 0x100)
    {
        enemy -= 0x100;
        fromWorld_i = 0;
    }
    else
    {
        fromWorld_i = 1;
    }

    combatExitDir = 0;
    if ((flags & 4) == 0)
    {
        InitCombatants();
    }

    for (i_n = 0; i_n < 0x10; i_n++)
    {
        order_j[i_n] = i_n;
    }

    if ((flags & 4) != 0)
    {
        // shuffle
        for (i_n = 0; i_n < 0xf; i_n++)
        {
            pick = RandomRange(0, 0xf);
            // swap
            count_m = order_j[i_n];
            order_j[i_n] = order_j[pick];
            order_j[pick] = count_m;
        }
    }

    if (flags != 0x80 || flags != 0x4) // BUG: expression always true
    {
        PrintString(/*0xa438*/ "*** CONFLICT ***\n");
    }

    if (D_5894_savedMapId != 0 && D_5894_savedMapId < 0x21 && enemy != 0xc && fromWorld_i != 0)
    {
        count_m = 1;
    }
    else
    {
        count_m = D_13bc_enemyStats[enemy].maxEncounterCount;
    }

    if (count_m != 8 && count_m != 0x10 && count_m != 1)
    {
        count_m = RandomRange(1, count_m);
        if (fewerEnemies != 0)
        {
            count_m = RandomRange(1, count_m);
        }
        UpdateFrame();
        if (count_m > 0x1f - 6)
        {
            count_m = 0x1a;
        }
    }

    enemyX = combatEnemyX[order_j[0]];
    enemyY = combatEnemyY[order_j[0]];
    PlaceCombatant(enemy, 0, enemyX, enemyY, D_5895_mapLevel);

    pick = count_m / 4 + 1;
    for (i_n = 1; i_n < count_m; i_n++)
    {
        kind = enemy;
        if (i_n < pick && Random(8) == 0)
        {
            kind = enemyCompanion[enemy];
        }
        enemyX = combatEnemyX[order_j[i_n]];
        enemyY = combatEnemyY[order_j[i_n]];
        PlaceCombatant(kind, 0, enemyX, enemyY, D_5895_mapLevel);
    }
}

// Whether (x, y) is inside the 11x11 view.
int IsInView(int viewX, int viewY)
{
    if (viewX > -1 && viewX < 11 && viewY > -1 && viewY < 11)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// calc player def
// A party member's defence: the sum over the six pieces of equipment (equipDefense), 3 more under the protection spell.
int PlayerDefense(int playerIndex)
{
    int defense;
    S_55a8* player_c;

    defense = 0;
    player_c = &party[playerIndex];

    if (D_5894_savedMapId == 40 && player_c->equips[1] != 0x0f)
    {
        return 0;
    }
    else
    {
        // BUG: the comparison is always false on DOS16, and always true on FMT
#define CMP(a) ((a) > -1)
        if (CMP(player_c->equips[0]))
        {
            // 6dd6
            defense += equipDefense[player_c->equips[0]];
        }

        if (CMP(player_c->equips[1]))
        {
            // 6dec
            defense += equipDefense[player_c->equips[1]];
        }

        if (CMP(player_c->equips[2]))
        {
            // 6dff
            defense += equipDefense[player_c->equips[2]];
        }

        if (CMP(player_c->equips[3]))
        {
            // 6e12
            defense += equipDefense[player_c->equips[3]];
        }

        if (CMP(player_c->equips[4]))
        {
            // 6e25
            defense += equipDefense[player_c->equips[4]];
        }

        if (CMP(player_c->equips[5]))
        {
            // 6e38
            defense += equipDefense[player_c->equips[5]];
        }

        // 6e4b
        if (statusIcon == 80)
        {
            defense += 3;
        }
    }

    return defense;
}

// unequip
// Take a piece of equipment off a party member; 0 when not worn.
int Unequip(int playerIndex, int item)
{
    int result;

    result = 1;

    if (item == party[playerIndex].equips[0])
    {
        party[playerIndex].equips[0] = 0xff;
    }
    else if (item == party[playerIndex].equips[1])
    {
        party[playerIndex].equips[1] = 0xff;
    }
    else if (item == party[playerIndex].equips[2])
    {
        party[playerIndex].equips[2] = 0xff;
    }
    else if (item == party[playerIndex].equips[3])
    {
        party[playerIndex].equips[3] = 0xff;
    }
    else if (item == party[playerIndex].equips[4])
    {
        party[playerIndex].equips[4] = 0xff;

        if (item == 0x2a && mapId > 0x7f && curCombatant < 0x20)
        {
            combatants[curCombatant].flags &= ~COMBAT_FLAGS_INVISIBLE;
        }
    }
    else if (item == party[playerIndex].equips[5])
    {
        party[playerIndex].equips[5] = 0xff;
    }
    else
    {
        result = 0;
    }

    // BUG: equip def is recalculated but discarded.
    PlayerDefense(playerIndex);

    return result;
}

void DRV_60_CF0(void* ax);

