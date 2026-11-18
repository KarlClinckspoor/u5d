#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "grap_drv.h"
#include "sjog.h"

#include <string.h>

#if !defined(TARGET_DOS16) // the original's are A6F1E.ASM's data (ULTIMA_6ff0_DistanceFromCenter)
u16 D_6aa4 = 0;

u8 D_6aa8[0x24] =
{
    0x32, 0x29, 0x22, 0x1D, 0x1A, 0x19,
    0x29, 0x20, 0x19, 0x14, 0x11, 0x10,
    0x22, 0x19, 0x12, 0x0D, 0x0A, 0x09,
    0x1D, 0x14, 0x0D, 0x08, 0x05, 0x04,
    0x1A, 0x11, 0x0A, 0x05, 0x02, 0x01,
    0x19, 0x10, 0x09, 0x04, 0x01, 0x00
};
#endif

// In the original, ULTIMA_5f86 is the first function of this object (MSC 5.0): it is
// the right length only when compiled with 5.0, and the object before ends at 5f86.
// CHECKED
// param_1: special map type (flags)
// - 0: combat
// - 2: combat in dungeon?
// - 4: camping/hole up
// - 0x80: ?
void ULTIMA_5f86_SpecialMapHandler(int param_1, int param_2, int param_3)
{
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab; // active character

    D_58a1_combatFlags = param_1;
    local_8 = D_5896_mapX;
    local_c = D_5897_mapY;
    local_a = D_5895_mapLevel;

    D_5894_savedMapId = D_5893_mapId;
    local_4_ab = D_587b_activePlayer;
    D_5893_mapId = 0xff;

    // copy actors
    for (local_e_am = 0; local_e_am < 0x20; local_e_am++)
    {
        D_a9fc_savedActors[local_e_am] = D_5c5a_actors[local_e_am];
    }

    if (param_1 == 0)
    {
        if ((D_5c5a_actors[param_2].tile & 0xfc) == TILE_ACTOR_PIRATE)
        {
            local_6 = 0x1;
        }
        else
        {
            local_6 = ((D_5c5a_actors[param_2].tile - TILE_ACTOR_WIZARD) >> 2);
        }

        if (D_5c5a_actors[param_2].value > 0x7f)
        {
            local_6 += 0x100;
        }

        // 6021
        ULTIMA_6bc2_SetUpCombat(1, local_6); // ***CONFLICT***
    }
    else
    {
        if ((param_1 & 4) != 0) // 602e
        {
            // 6034
            // hole up
            if (CMDS_0000(param_1, param_2, param_3) != 0)
            {
                goto L_605c;
            }
            goto L_606c;
        }
        else if ((param_1 & 2) != 0) // 6046
        {
            local_6 = param_2;
            DNGLOOK_117e_PlaceCombatants(0, 2);
        }
        else
        {
            goto L_606c;
        }
    }

    // 605c
L_605c:
    D_589e_curCombatant = D_587b_activePlayer = 0xff;
    D_58a3_combatOver = 0;

    COMBAT_0b94_MainLoop(); // COMBAT.0b94

L_606c:
    // 606c
    if (D_bb16 != 0)
    {
        D_bb16 = 0;
        ULTIMA_6fbc_TilesetMode(1);
    }

    D_5896_mapX = local_8;
    D_5897_mapY = local_c;
    D_5895_mapLevel = local_a;
    D_5893_mapId = D_5894_savedMapId;
    D_24e6_viewDirty = 1;

#if defined(ENABLE_BGM)
    AUDIO_PlayBgmPerMap();
#endif

    ULTIMA_2900_UpdateVitalsDisplay();

    if (D_55a8_party[local_4_ab].status != STATUS_DEAD && D_55a8_party[local_4_ab].status != STATUS_SLEEP)
    {
        D_587b_activePlayer = local_4_ab;
    }
    else
    {
        D_587b_activePlayer = 0xff;
    }

    for (local_e_am = 0; local_e_am < 0x20; local_e_am++)
    {
        D_5c5a_actors[local_e_am] = D_a9fc_savedActors[local_e_am];
    }
}

// NOTE: combine arrays?
static void ULTIMA_60ec_LoadSpecialMap(int param_1)
{
    int local_4;

    ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa3f0, "BRIT.CBT"), D_ad14_combatMap, 0x160, param_1 * 0x160);

    for (local_4 = 0; local_4 < 6; local_4++)
    {
        D_1724_combatPartyX[local_4] = GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_SOUTH, local_4);
        D_172c_combatPartyY[local_4] = GetCombatMapPartyY(COMBAT_MAP_SPECIAL_PARTY_SOUTH, local_4);
    }

    for (local_4 = 0; local_4 < 0x10; local_4++)
    {
        D_1704_combatEnemyX[local_4] = GetCombatMapMonsterX(local_4);
        D_1714_combatEnemyY[local_4] = GetCombatMapMonsterY(local_4);
    }
}

void ULTIMA_6150_Combat(int param_1)
{
    int local_6;
    bool local_8;
    int local_a;
    int local_4_ab;

    local_a = D_5c5a_actors[param_1].tile & 0xfc;

    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    if (local_a < 0x40) // TILE_ACTOR_40
    {
        ULTIMA_1850_PrintString(_TEXT(0xa3fa, "PIRATES"));
    }
    else
    {
        ULTIMA_1850_PrintString(D_18b6_enemyNamesPlural[(local_a - 0x40) / 4]); // TILE_ACTOR_40
    }

    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1850_PrintString(_TEXT(0xa402, "\n\n"));

    local_6 = *ULTIMA_4402_GetTileAddr(D_5c5a_actors[param_1].x, D_5c5a_actors[param_1].y);
    if (local_6 < TILE_MAP_POISON || (local_6 & 0xfe) != TILE_MAP_6A && ((local_6 & 0xf0) == TILE_MAP_60))
    {
        local_8 = true;
    }
    else
    {
        local_8 = false;
    }

    if (local_a == 0xfc) // 61f3
    {
        local_4_ab = 10;
        if (D_57b5_sceptre != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0xa406, "The Sceptre is reclaimed!\n"));
            ULTIMA_2192_AudioPulse(0xfd2, 1, 65000, 1, 1);
            D_57b5_sceptre = 0;
        }
    }
    else
    {
        if (((byte)local_a & 0xf0) == 0x80) // 622c
        {
            local_8 = 1;
        }

        if ((D_587c_partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20) // 623a
        {
            if (local_a == TILE_ACTOR_PIRATE)
            {
                local_4_ab = 0xe;
            }
            else if (local_8)
            {
                local_4_ab = 0xb;
            }
            else
            {
                local_4_ab = 0xd;
            }
        }
        else if (local_a == TILE_ACTOR_PIRATE)
        {
            local_4_ab = 0xc;
        }
        else if (local_8)
        {
            local_4_ab = 0xf;
        }
        else
        {
            switch (local_6)
            {
            case TILE_MAP_WATER_1:
            case TILE_MAP_WATER_2:
            case TILE_MAP_WATER_3:
                local_4_ab = 0xf;
                break;
            case TILE_MAP_POISON:
                local_4_ab = 1;
                break;
            case TILE_MAP_GRASS:
                local_4_ab = 2;
                break;
            case TILE_MAP_6:
            case TILE_MAP_8:
                local_4_ab = 3;
                break;
            case TILE_MAP_7:
            case TILE_MAP_1E:
            case TILE_MAP_1F:
                local_4_ab = 4;
                break;
            case TILE_MAP_9:
            case TILE_MAP_A:
                local_4_ab = 5;
                break;
            case TILE_MAP_B:
            case TILE_MAP_C:
            case TILE_MAP_D:
            case TILE_MAP_E:
            case TILE_MAP_F:
                local_4_ab = 6;
                break;
            case TILE_MAP_1D:
            case TILE_MAP_48:
            case TILE_MAP_49:
            case TILE_MAP_6A:
            case TILE_MAP_6B:
                local_4_ab = 7;
                break;
            case TILE_MAP_44:
                local_4_ab = 8;
                break;
            default:
                if (D_5893_mapId != 0)
                {
                    local_4_ab = 8;
                }
                else
                {
                    local_4_ab = 2;
                }
                break;
            }
        }
    }

    ULTIMA_60ec_LoadSpecialMap(local_4_ab);
    ULTIMA_5f86_SpecialMapHandler(0, param_1, 0);

    SJOG_203e_EndCombat(param_1);
    ULTIMA_5e4a_BuildLightMap();
    ULTIMA_4f7c_AdvanceTime(0);
}

void ULTIMA_6360_Camping(int param_1, int param_2)
{
    ULTIMA_60ec_LoadSpecialMap(0);
    ULTIMA_5f86_SpecialMapHandler(4, param_1, param_2);
    ULTIMA_5e4a_BuildLightMap();
}

// draw frame
void ULTIMA_637e_DrawFrame(void)
{
    int local_4; // unused
 
    // black
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(0, 0, 319, 199);

    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0, 0, 319, 6);
    ULTIMA_0aa6_GRAP_3f_FillRect(0, 185, 191, 191);
    ULTIMA_0aa6_GRAP_3f_FillRect(0, 0, 6, 191);
    ULTIMA_0aa6_GRAP_3f_FillRect(185, 0, 191, 191);
    ULTIMA_0aa6_GRAP_3f_FillRect(313, 0, 319, 87);
    ULTIMA_0aa6_GRAP_3f_FillRect(192, 80, 312, 87);
    ULTIMA_0aa6_GRAP_3f_FillRect(192, 0x39, 312, 63);

    ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);
    ULTIMA_1bf2_SetTextPosition(0, 0);
    ULTIMA_16ba_PrintChar(0x7b);
    ULTIMA_1bf2_SetTextPosition(39, 0);
    ULTIMA_16ba_PrintChar(0x7c);
    ULTIMA_1bf2_SetTextPosition(0, 23);
    ULTIMA_16ba_PrintChar(0x7d);

    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(7, 7, 7, 184);
    ULTIMA_0f90_GRAP_LineTo(184, 184);
    ULTIMA_0f90_GRAP_LineTo(184, 7);
    ULTIMA_0f90_GRAP_LineTo(7, 7);
    ULTIMA_0b10_GRAP_Line(191, 191, 191, 87);
    ULTIMA_0f90_GRAP_LineTo(319, 87);
    ULTIMA_0b10_GRAP_Line(191, 7, 312, 7);
    ULTIMA_0f90_GRAP_LineTo(312, 56);
    ULTIMA_0f90_GRAP_LineTo(191, 56);
    ULTIMA_0f90_GRAP_LineTo(191, 7);
    ULTIMA_0b10_GRAP_Line(191, 63, 312, 63);
    ULTIMA_0f90_GRAP_LineTo(312, 80);
    ULTIMA_0f90_GRAP_LineTo(191, 80);
    ULTIMA_0f90_GRAP_LineTo(191, 63);

    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
}

// CHECKED
// place actor on combat map
// param_2: player / monster?
// return: placed actor idx
// Put a combatant on the combat map. kind 0: an enemy of type `entity`; 1: party member `entity`; 2: an object with tile `entity`. Returns its slot (for kind 2 the actor slot), -1 when full.
int ULTIMA_6506_PlaceCombatant(int param_1, int param_2, int param_3, int param_4, int param_5)
{
    int local_a;
    int local_8;
    int local_6;
    CombatEntity* local_4_aa;

#if !defined(MATCHING_BUILD)
    local_4_aa = NULL;
#endif

    local_a = 0;
    local_6 = 0;
    local_8 = -1;

    if (param_2 != 2)
    {
        if (param_2 == 0)
        {
            local_a = 6;
        }

        // 6531
        for (local_6 = local_a; local_6 < 0x20; local_6++)
        {
            // 653f
            // 6577
            if (D_ba14_combatants[local_6].flags == 0) // combat actor slot not occupied?
            {
                local_4_aa = &D_ba14_combatants[local_6];
                local_8 = local_6;

                // 6587
                if (param_2 == 1 && D_55a8_party[param_1].status != STATUS_DEAD) // player?
                {
                    // 65a0
                    local_4_aa->dex = D_55a8_party[param_1].dex;
                    local_4_aa->turnTimer = 36 - local_4_aa->dex;
                    local_4_aa->flags = COMBAT_FLAGS_PLAYER;
                    // 65b4
                    if (D_55a8_party[param_1].status != STATUS_GOOD && D_55a8_party[param_1].status != STATUS_POISONED)
                    {
                        local_4_aa->flags |= COMBAT_FLAGS_ASLEEP;
                    }
                }

                // 65c5
                if (param_2 == 0) // monster?
                {
                    local_4_aa->hp = D_13bc_enemyStats[param_1].maxHp;
                    // local_6 reused as a scratch variable (the loop ends with the break below):
                    // the original keeps Random(7) - 4 in DI, local_6's register, and the
                    // assignment also stops MSC from walking D_ba14_combatants with pointers.
                    local_6 = ULTIMA_3aae_Random(7) - 4;
                    local_4_aa->dex = local_6 + D_13bc_enemyStats[param_1].dex;
                    if (local_4_aa->dex > 0x1e)
                    {
                        local_4_aa->dex = D_13bc_enemyStats[param_1].dex;
                    }

                    local_4_aa->turnTimer = 36 - local_4_aa->dex;
                    local_4_aa->flags = COMBAT_FLAGS_MONSTER;
                    if (param_1 == 8 || param_1 == 9)
                    {
                        // 6609
                        local_4_aa->flags = COMBAT_FLAGS_DEAD;
                    }
                }

                // 660d
                local_4_aa->entityIdx = param_1;
                local_4_aa->x = param_3;
                local_4_aa->y = param_4;
                break;
            }
        }
    }

    // 6625
    if (local_8 > -1 || param_2 == 2)
    {
        // 6634
        for (local_6 = 0; local_6 < 0x20; local_6++)
        {
            if (D_5c5a_actors[local_6].tile == 0)
            {
                // 6685
                if (param_2 == 0)
                {
                    local_4_aa->actorIdx = local_6;
                    
                    D_5c5a_actors[local_6].animTile = D_5c5a_actors[local_6].tile = param_1 * 4 + TILE_ACTOR_WIZARD; // local_14
                }

                // 669a
                if (param_2 == 1)
                {
                    local_4_aa->actorIdx = local_6;
                }

                // 66a6
                if (param_2 == 2)
                {
                    D_5c5a_actors[local_6].animTile = D_5c5a_actors[local_6].tile = param_1;
                    local_8 = local_6;
                }

                // 66bb
                D_5c5a_actors[local_6].x = param_3;
                D_5c5a_actors[local_6].y = param_4;
                D_5c5a_actors[local_6].z = param_5;
                D_5c5a_actors[local_6].extra = 0xff;
                if (param_2 == 0)
                {
                    D_5c5a_actors[local_6].value = local_4_aa->hp;
                }
                else
                {
                    D_5c5a_actors[local_6].value = param_1;
                }

                // 66f8
                if (param_2 != 2)
                {
                    local_4_aa->actorIdx = local_6;
                }

                break;
            }
        }

        // 6706
        if (param_2 != 2 && local_6 == 0x20 && local_8 > -1)
        {
            local_8 = -1;
        }
    }
    else
    {
        local_8 = -1;
    }

    // 6751
    if (local_8 >= 0)
    {
        for (local_6 = 0; local_6 < 0x20; local_6++)
        {
            if (D_5c5a_actors[D_ba14_combatants[local_6].actorIdx].extra == local_8)
            {
                D_5c5a_actors[D_ba14_combatants[local_6].actorIdx].extra = 0xff;
            }
        }
    }

    return local_8;
}

// A ring worn into combat takes effect: invisibility (0x2a), regeneration (0x2c).
void ULTIMA_6794_ApplyRing(int param_1)
{
    CombatEntity* local_4 = &D_ba14_combatants[param_1];
    if ((local_4->flags & COMBAT_FLAGS_PLAYER) != 0 && (local_4->flags & (COMBAT_FLAGS_DEAD | COMBAT_FLAGS_ASLEEP)) == 0)
    {
        if (D_55a8_party[local_4->entityIdx].equips[4] == 42)
        {
            D_5c5a_actors[local_4->actorIdx].animTile = TILE_ACTOR_1D;
            local_4->flags |= COMBAT_FLAGS_INVISIBLE;
        }
        else if (D_55a8_party[local_4->entityIdx].equips[4] == 44)
        {
            ULTIMA_400c_RingRegeneration(local_4->entityIdx); // sic
        }
    }
}

// Wake a sleeping combatant.
void ULTIMA_6800_WakeCombatant(int param_1)
{
    CombatEntity* local_4 = &D_ba14_combatants[param_1];
    if ((local_4->flags & COMBAT_FLAGS_ASLEEP) != 0)
    {
        if ((local_4->flags & COMBAT_FLAGS_PLAYER) != 0)
        {
            D_55a8_party[local_4->entityIdx].status = STATUS_GOOD;
            if ((local_4->flags & COMBAT_FLAGS_INVISIBLE) != 0)
            {
                D_5c5a_actors[local_4->actorIdx].animTile = TILE_ACTOR_1D;
            }
            else
            {
                D_5c5a_actors[local_4->actorIdx].animTile = D_5c5a_actors[local_4->actorIdx].tile;
            }
        }
        else
        {
            D_5c5a_actors[local_4->actorIdx].animState = 0;
        }
        local_4->flags &= ~COMBAT_FLAGS_ASLEEP;
    }

    ULTIMA_2900_UpdateVitalsDisplay();
}

// Put a combatant to sleep, unless a poisoned party member.
void ULTIMA_6880_SleepIfNotPoisoned(int param_1)
{
    if ((D_ba14_combatants[param_1].flags & COMBAT_FLAGS_PLAYER) == 0 || D_55a8_party[D_ba14_combatants[param_1].entityIdx].status != STATUS_POISONED)
    {
        ULTIMA_68ae_PutToSleep(param_1);
    }
}

// Put a combatant to sleep.
void ULTIMA_68ae_PutToSleep(int param_1)
{
    CombatEntity* local_4 = &D_ba14_combatants[param_1];
    if ((local_4->flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        if (D_55a8_party[local_4->entityIdx].status != STATUS_DEAD)
        {
            D_55a8_party[local_4->entityIdx].status = STATUS_SLEEP;
            local_4->flags |= COMBAT_FLAGS_ASLEEP;
            D_5c5a_actors[local_4->actorIdx].animTile = TILE_ACTOR_SLEEP;

            if (param_1 == D_587b_activePlayer)
            {
                D_587b_activePlayer = 0xff;
            }

            D_58a2 = 4;

            ULTIMA_2900_UpdateVitalsDisplay();

            if ((D_58a1_combatFlags & 4) == 0)
            {
                ULTIMA_5910_UpdateFrame();
            }
        }
    }
    else
    {
        local_4->flags |= COMBAT_FLAGS_ASLEEP;
        D_58a2 = 4;
        D_5c5a_actors[local_4->actorIdx].animState = 0xff;

        ULTIMA_5910_UpdateFrame();
    }
}

// CHECKED
// initialize combat entities
// Clear the actors and the combatants and place the party on the combat map.
void ULTIMA_6936_InitCombatants(void)
{
    byte local_10_aa;
    byte local_e_am;
    byte local_c;
    int local_a;
    byte local_8;
    int local_6;
    CombatEntity* local_4_aa;

    local_10_aa = 0;

    // clear
    for (local_6 = 0; local_6 < 0x20; local_6++)
    {
        local_4_aa = &D_ba14_combatants[local_6];

        D_5c5a_actors[local_6].tile = D_5c5a_actors[local_6].animTile = D_5c5a_actors[local_6].x = D_5c5a_actors[local_6].animState = 0;
        D_5c5a_actors[local_6].y = D_5c5a_actors[local_6].z = D_5c5a_actors[local_6].value = 0;

        local_4_aa->hp = local_4_aa->dex = local_4_aa->flags = local_4_aa->entityIdx = 0;

        local_4_aa->actorIdx = local_4_aa->turnTimer = local_4_aa->x = local_4_aa->y = 0;
    }

    // place player party
    for (local_6 = 0; local_6 < D_585b_partyCount; local_6++)
    {
        if (D_55a8_party[local_6].status != STATUS_DEAD)
        {
            if (D_55a8_party[local_6].equips[4] == 42)
            {
                local_10_aa = 42;
            }

            if (D_55a8_party[local_6].equips[4] == 44)
            {
                local_10_aa = 44;
            }

            if (local_10_aa != 0)
            {
                local_a = ULTIMA_2092_RandomRange(0, 0xf);
                if (local_a == 0xb)
                {
                    ULTIMA_1850_PrintString(_TEXT(0xa422, "A ring has vanished!\n"));
                    ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
                    ULTIMA_6e60_Unequip(local_6, local_10_aa);
                }
                local_10_aa = 0;
            }

            local_8 = D_1724_combatPartyX[local_6];
            local_e_am = D_172c_combatPartyY[local_6];
            local_a = ULTIMA_6506_PlaceCombatant(local_6, 1, local_8, local_e_am, D_5895_mapLevel);

            D_5c5a_actors[D_ba14_combatants[local_a].actorIdx].extra = 0xff;

            // setup tile (according to class)
            switch (D_55a8_party[local_6].cls)
            {
            case CLASS_AVATAR:
                D_5c5a_actors[D_ba14_combatants[local_a].actorIdx].tile = TILE_ACTOR_4C;
                break;
            case CLASS_FIGHTER:
            case CLASS_PALADIN:
            case CLASS_RANGER:
                D_5c5a_actors[D_ba14_combatants[local_a].actorIdx].tile = TILE_ACTOR_FIGHTER;
                break;
            case CLASS_DRUID:
            case CLASS_MAGE:
                D_5c5a_actors[D_ba14_combatants[local_a].actorIdx].tile = TILE_ACTOR_WIZARD;
                break;
            case CLASS_BARD:
            case CLASS_SHEPHERD:
            case CLASS_TINKER:
                D_5c5a_actors[D_ba14_combatants[local_a].actorIdx].tile = TILE_ACTOR_BARD_44;
                break;
            }

            D_5c5a_actors[D_ba14_combatants[local_a].actorIdx].animTile = D_5c5a_actors[D_ba14_combatants[local_a].actorIdx].tile;

            if (D_55a8_party[local_6].status == STATUS_SLEEP)
            {
                ULTIMA_68ae_PutToSleep(local_a);
            }
            else
            {
                ULTIMA_6794_ApplyRing(local_a);
            }
        }
    }

    if (GetCombatMap(5, 5) == 0xdc)
    {
        local_c = ULTIMA_6506_PlaceCombatant(1, 2, 5, 5, D_5895_mapLevel);
        D_5c5a_actors[local_c].value = D_5895_mapLevel * 3 + 7;
        GetCombatMap(5, 5) = D_bb15_floorTile;
    }
}

// CHECKED
// prepare combat
// Set up a combat: the party (unless flag 4, camping), then the enemies of type `enemy`, as many as its statistics allow.
void ULTIMA_6bc2_SetUpCombat(int param_1, int param_2)
{
    byte local_30_a;
    int local_2e_ak;
    byte local_2c_am;
    int local_2a;
    int local_28[16];
    int local_8;
    int local_6;
    int local_4_ab;

    if (param_2 >= 0x100)
    {
        param_2 -= 0x100;
        local_8 = 0;
    }
    else
    {
        local_8 = 1;
    }

    D_58a0_combatExitDir = 0;
    if ((param_1 & 4) == 0)
    {
        ULTIMA_6936_InitCombatants();
    }

    for (local_2a = 0; local_2a < 0x10; local_2a++)
    {
        local_28[local_2a] = local_2a;
    }

    if ((param_1 & 4) != 0)
    {
        // shuffle
        for (local_2a = 0; local_2a < 0xf; local_2a++)
        {
            local_2e_ak = ULTIMA_2092_RandomRange(0, 0xf);
            // swap
            local_6 = local_28[local_2a];
            local_28[local_2a] = local_28[local_2e_ak];
            local_28[local_2e_ak] = local_6;
        }
    }

    if (param_1 != 0x80 || param_1 != 0x4) // BUG: expression always true
    {
        ULTIMA_1850_PrintString(_TEXT(0xa438, "*** CONFLICT ***\n"));
    }

    if (D_5894_savedMapId != 0 && D_5894_savedMapId < 0x21 && param_2 != 0xc && local_8 != 0)
    {
        local_6 = 1;
    }
    else
    {
        local_6 = D_13bc_enemyStats[param_2].maxEncounterCount;
    }

    if (local_6 != 8 && local_6 != 0x10 && local_6 != 1)
    {
        local_6 = ULTIMA_2092_RandomRange(1, local_6);
        if (D_5959 != 0)
        {
            local_6 = ULTIMA_2092_RandomRange(1, local_6);
        }
        ULTIMA_5910_UpdateFrame();
        if (local_6 > 0x1f - 6)
        {
            local_6 = 0x1a;
        }
    }

    local_2c_am = D_1704_combatEnemyX[local_28[0]];
    local_30_a = D_1714_combatEnemyY[local_28[0]];
    ULTIMA_6506_PlaceCombatant(param_2, 0, local_2c_am, local_30_a, D_5895_mapLevel);

    local_2e_ak = local_6 / 4 + 1;
    for (local_2a = 1; local_2a < local_6; local_2a++)
    {
        local_4_ab = param_2;
        if (local_2a < local_2e_ak && ULTIMA_3aae_Random(8) == 0)
        {
            local_4_ab = D_16d4_enemyCompanion[param_2];
        }
        local_2c_am = D_1704_combatEnemyX[local_28[local_2a]];
        local_30_a = D_1714_combatEnemyY[local_28[local_2a]];
        ULTIMA_6506_PlaceCombatant(local_4_ab, 0, local_2c_am, local_30_a, D_5895_mapLevel);
    }
}

// Whether (x, y) is inside the 11x11 view.
int ULTIMA_6d82_IsInView(int param_1, int param_2)
{
    if (param_1 > -1 && param_1 < 11 && param_2 > -1 && param_2 < 11)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// calc player def
// A party member's defence: the sum over the six pieces of equipment (D_1634_equipDefense), 3 more under the protection spell.
int ULTIMA_6da8_PlayerDefense(int param_1)
{
    int local_6_an;
    S_55a8* local_4;

    local_6_an = 0;
    local_4 = &D_55a8_party[param_1];

    if (D_5894_savedMapId == 40 && local_4->equips[1] != 0x0f)
    {
        return 0;
    }
    else
    {
#if defined(MATCHING_BUILD)
        // BUG: the comparison is always false on DOS16, and always true on FMT
#define CMP(a) ((a) > -1)
#else
#define CMP(a) (((s8)(a)) > -1)
#endif
        if (CMP(local_4->equips[0]))
        {
            // 6dd6
            local_6_an += D_1634_equipDefense[local_4->equips[0]];
        }

        if (CMP(local_4->equips[1]))
        {
            // 6dec
            local_6_an += D_1634_equipDefense[local_4->equips[1]];
        }

        if (CMP(local_4->equips[2]))
        {
            // 6dff
            local_6_an += D_1634_equipDefense[local_4->equips[2]];
        }

        if (CMP(local_4->equips[3]))
        {
            // 6e12
            local_6_an += D_1634_equipDefense[local_4->equips[3]];
        }

        if (CMP(local_4->equips[4]))
        {
            // 6e25
            local_6_an += D_1634_equipDefense[local_4->equips[4]];
        }

        if (CMP(local_4->equips[5]))
        {
            // 6e38
            local_6_an += D_1634_equipDefense[local_4->equips[5]];
        }

        // 6e4b
        if (D_587a_statusIcon == 80)
        {
            local_6_an += 3;
        }
    }

    return local_6_an;
}

// unequip
// Take a piece of equipment off a party member; 0 when not worn.
int ULTIMA_6e60_Unequip(int param_1, int param_2)
{
    int local_4;

    local_4 = 1;

    if (param_2 == D_55a8_party[param_1].equips[0])
    {
        D_55a8_party[param_1].equips[0] = 0xff;
    }
    else if (param_2 == D_55a8_party[param_1].equips[1])
    {
        D_55a8_party[param_1].equips[1] = 0xff;
    }
    else if (param_2 == D_55a8_party[param_1].equips[2])
    {
        D_55a8_party[param_1].equips[2] = 0xff;
    }
    else if (param_2 == D_55a8_party[param_1].equips[3])
    {
        D_55a8_party[param_1].equips[3] = 0xff;
    }
    else if (param_2 == D_55a8_party[param_1].equips[4])
    {
        D_55a8_party[param_1].equips[4] = 0xff;

        if (param_2 == 0x2a && D_5893_mapId > 0x7f && D_589e_curCombatant < 0x20)
        {
            D_ba14_combatants[D_589e_curCombatant].flags &= ~COMBAT_FLAGS_INVISIBLE;
        }
    }
    else if (param_2 == D_55a8_party[param_1].equips[5])
    {
        D_55a8_party[param_1].equips[5] = 0xff;
    }
    else
    {
        local_4 = 0;
    }

    // BUG: equip def is recalculated but discarded.
    ULTIMA_6da8_PlayerDefense(param_1);

    return local_4;
}

// from FMT
u8 DAT_000738d8[] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28, 0x28, 0x28, 0x28, 0x28, 0x20, 0x20,
                     0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                     0x88, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                     0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                     0x10, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x10, 0x10, 0x10, 0x10, 0x10,
                     0x10, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                     0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x10, 0x10, 0x10, 0x10, 0x20};

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// from FMT (27c98)
// Compare a word of a table (up to 9 characters, taken as upper case) with the start of `text`: 0 equal, -1 not.
int ULTIMA_6f1e_CompareWord(byte* param_1, char* param_2)
{
    byte bVar1;
    int iVar2;
    int iVar3;
    int local_18;
    char local_14[10];

    local_18 = -1;
    memset(local_14, 0, 10);

    for (iVar2 = 0; iVar2 < 9 && (iVar3 = iVar2, param_1[iVar2] != 0); iVar2++)
    {
        bVar1 = param_1[iVar2] & 0x7f;
        if ((DAT_000738d8[bVar1] & 2) != 0)
        {
            bVar1 = (param_1[iVar2] & 0x7fU) - 0x20;
        }
        local_14[iVar2] = (char)bVar1;
    }

    iVar2 = strncmp(local_14, param_2, iVar3);
    if (iVar2 == 0)
    {
        local_18 = 0;
    }

    return local_18;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static int ULTIMA_6f90(char* di)
{
    // repne scasb
    return strlen(di);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// Driver function 6c with mode + 3 (DNGLOOK, ENDGAME); what the modes do is the driver's business.
void ULTIMA_6f9e_TilesetModePlus3(int param_1)
{
    // bx: not initialized
    byte bl = 0;
    byte bh = 0;
    int ax = param_1 + 3;
    DRV_6c(ax, bl, bh);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// Driver function 6c with the mode (0, 1); what the modes do is the driver's business.
void ULTIMA_6fbc_TilesetMode(int param_1)
{
    // bx: not initialized
    byte bl = 0;
    byte bh = 0;
    DRV_6c(param_1, bl, bh);
}
#endif

void DRV_60_CF0(void* ax);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_6fd6_GRAP_AnimateTiles(void)
{
    // CLC
    // ax: D_539c
    DRV_60_CF0(D_539c);
    //debug("ULTIMA_6fd6");
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// The squared distance of view cell (x, y) from the centre (5, 5) of the 11x11 view (table D_6aa8); 0 outside.
int ULTIMA_6ff0_DistanceFromCenter(register int param_1, register int param_2)
{
    if ((param_1 < 0xb) && (param_2 < 0xb))
    {
        if ((param_1 > 5) || (param_2 > 5))
        {
            if (param_1 > 5)
            {
                param_1 = 10 - param_1;
            }
            if (param_2 > 5)
            {
                param_2 = 10 - param_2;
            }
        }

        return D_6aa8[param_1 + param_2 * 6];
    }

    return 0;
}
#endif
