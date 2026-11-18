#include "common/common.h"

#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include <stdlib.h>

#include "audio/aud_mus.h"

#include "blckthrn.h"
#include "cast2.h"
#include "comsubs.h"
#include "mainout.h"
#include "outsubs.h"

static u16 D_a524; // this object's _BSS (DS:a524)

// defined in mainoutb.c, the part of MAINOUT.OVL built with MSC 5.0
extern u8 D_2bc0[];
extern u8 D_2bcc[];
extern u8 D_2bd4[];
extern u8 D_2bda[];
extern u8 D_2bdc[];
extern u8 D_2be8[];
extern u8 D_2bf0[];
extern u8 D_2bf6[];
int MAINOUT_1a60(void);
void MAINOUT_109e(void);
void MAINOUT_1be8(void);

void MAINOUT_0000(void)
{
    ULTIMA_4be8_ClearSky();
    ULTIMA_4f7c_AdvanceTime(0);

    D_58a4_viewOn = D_5956_newPrompt = D_24e6_viewDirty = 1;
    D_5955_sailDir = 0;
    D_589b_mapOriginX = D_5896_mapX & 0xf0;
    if ((D_5896_mapX & 0xf) < 8)
    {
        D_589b_mapOriginX = (D_589b_mapOriginX - 0x10) & 0xf0;
    }

    // 0034
    D_589c_mapOriginY = D_5897_mapY & 0xf0;
    if ((D_5897_mapY & 0xf) < 8)
    {
        D_589c_mapOriginY = (D_589c_mapOriginY - 0x10) & 0xf0;
    }

    // 004f
    OUTSUBS_01b4(0, 1);
    OUTSUBS_01b4(0, -1);
    ULTIMA_4f7c_AdvanceTime(0);
    ULTIMA_5e4a_BuildLightMap();
    ULTIMA_2e96_SetWindDirection(-1);
    ULTIMA_4a84_DrawSunAndMoons();
    OUTSUBS_0566();
}

void MAINOUT_007a(void)
{
    int local_8;
    int local_a;

    int local_6;
    ActorFmt* local_4_ab;

    for (local_6 = 1; local_6 < 32; local_6++)
    {
        local_4_ab = &D_5c5a_actors[local_6];

        local_8 = abs(D_5896_mapX - local_4_ab->x);
        local_a = abs(D_5897_mapY - local_4_ab->y);

        if (local_4_ab->tile != 0 && local_4_ab->z == D_5895_mapLevel && local_8 < 6 && local_a < 6)
        {
            ULTIMA_3ae6_WaitFrames(1);
        }

        break;
    }
}

static int MAINOUT_00da(int param_1)
{
    int local1_6;
    int local2_4 = 0;

    switch (D_587c_partyTile & 0xfc)
    {
    case TILE_ACTOR_HORSE:
        // 010a
        ULTIMA_1850_PrintString(_TEXT(0x2946, "Ride "));
        if (param_1 == 1)
        {
            D_587c_partyTile = TILE_ACTOR_RIDING_HORSE;
        }
        else if (param_1 == 3)
        {
            D_587c_partyTile = TILE_ACTOR_RIDING_HORSE + 1;
        }
        break;

    case TILE_ACTOR_FLYING_CARPET:
        // 0130
        ULTIMA_1850_PrintString(_TEXT(0x294c, "Fly "));
        if (param_1 == 1)
        {
            D_587c_partyTile = TILE_ACTOR_FLYING_CARPET;
        }
        else if (param_1 == 3)
        {
            D_587c_partyTile = TILE_ACTOR_FLYING_CARPET + 1;
        }
        break;

    case TILE_ACTOR_SKIFF:
        // 0152
        ULTIMA_1850_PrintString(_TEXT(0x2951, "Row "));
        D_587c_partyTile = (char)param_1 + (D_587c_partyTile & 0xfc);
        break;

    case TILE_ACTOR_FRIGATE_20:
    case TILE_ACTOR_FRIGATE_24:
        // 016a
        local1_6 = D_587c_partyTile;
        D_587c_partyTile = (char)param_1 + (D_587c_partyTile & 0xfc);
        if (local1_6 != D_587c_partyTile)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2956, "Head "));

            switch (param_1)
            {
            case 0:
                ULTIMA_1850_PrintString(_TEXT(0x295c, "North\n"));
                break;
            case 2:
                ULTIMA_1850_PrintString(_TEXT(0x2963, "South\n"));
                break;
            case 1:
                ULTIMA_1850_PrintString(_TEXT(0x296a, "East\n"));
                break;
            case 3:
                ULTIMA_1850_PrintString(_TEXT(0x2970, "West\n"));
                break;
            }

            // 01ad
            D_5956_newPrompt = local2_4 = 1;
            if (D_5c5a_actors[0].value < 0x32)
            {
                ULTIMA_1850_PrintString(_TEXT(0x2976, "Hull weak!\n"));
            }
        }
        // 01dc
        else if (D_587c_partyTile < TILE_ACTOR_FRIGATE_24 && D_5892_windDir == 0)
        {
            local2_4 = 1;
        }

        break;
    }

    // 0129
    return local2_4;
}

// Check before Walk/Move
static int MAINOUT_01fe(int param_2, int param_1)
{
    int local_8;
    int local_6;
    bool local_4_ab;

    if ((D_587c_partyTile & 0xfc) == TILE_ACTOR_FRIGATE_24)
    {
        ULTIMA_1850_PrintString(_TEXT(0x2982, "Rowing!\n"));
    }

    local_4_ab = 1;
    local_6 = ULTIMA_368e_FindActorTileAtPos(param_2 + (uint)D_5896_mapX, param_1 + (uint)D_5897_mapY, D_5895_mapLevel);
    if (local_6 != 0)
    {
        local_4_ab = 0;
        if (D_587c_partyTile >= TILE_ACTOR_30 || D_587c_partyTile < TILE_ACTOR_FRIGATE_20)
        {
            if ((local_6 >= TILE_ACTOR_FRIGATE_24 && local_6 < TILE_ACTOR_PIRATE) || local_6 == TILE_ACTOR_CARPET || (local_6 & 0xfe) == TILE_ACTOR_HORSE)
            {
                local_4_ab = 1;
            }
        }
        else
        {
            if (D_587c_partyTile >= TILE_ACTOR_SKIFF && local_6 >= TILE_ACTOR_FRIGATE_24 && local_6 < TILE_ACTOR_SKIFF)
            {
                local_4_ab = 1;
            }
        }
    }

    local_8 = GetMapViewport(param_2 + 5, param_1 + 5);

    local_4_ab = local_4_ab && ULTIMA_2c4c_IsWalkableTile(D_587c_partyTile, local_8) != 0 ? 1 : 0;
    if (local_4_ab == 0)
    {
        if (D_5955_sailDir != 0)
        {
            if (local_8 == TILE_MAP_WATER_3)
            {
                ULTIMA_1850_PrintString(_TEXT(0x298b, "BREAKING UP!\n"));
            }
            else if (local_8 != TILE_MAP_47)
            {
                ULTIMA_1850_PrintString(_TEXT(0x2999, "COLLISION!\n"));
            }

            if (local_8 == TILE_MAP_47)
            {
                ULTIMA_1850_PrintString(_TEXT(0x29a5, "Docked!\n"));
                D_587c_partyTile += 4;
            }
            else
            {
                ULTIMA_223c_AudioWhiteNoise(100, 2000, 300);
                MAINOUT_109e();
            }

            D_5955_sailDir = 0;
            D_5956_newPrompt = 1;
        }
        else if (D_587c_partyTile < TILE_ACTOR_FRIGATE_20 || (local_6 & 0xfc) != TILE_ACTOR_WHIRLPOOL)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29ae, "Blocked!\n"));
            if (local_8 == TILE_MAP_2F)
            {
                ULTIMA_1850_PrintString(_TEXT(0x29b8, "OUCH!\n"));
                ULTIMA_2aa8_DamageParty();
            }
            else
            {
                ULTIMA_22c0_AudioTone(0xa5, 200);
            }
            ULTIMA_1b16_ClearKbdBuffer();
        }
    }

    return local_4_ab;
}

// Move
void MAINOUT_0354(int param_1, int param_2)
{
    int local_6_an;
    int local_4;

    D_5896_mapX += param_1;
    D_5897_mapY += param_2;
    D_24e6_viewDirty = 1;
    local_4 = D_5896_mapX - D_589b_mapOriginX & 0x1f;
    local_6_an = D_5897_mapY - D_589c_mapOriginY & 0x1f;

    if (local_4 < 5 || local_4 > 0x1a || local_6_an < 5 || local_6_an > 0x1a)
    {
        OUTSUBS_02c8(param_1, param_2); // 7bd2
        D_589b_mapOriginX = (char)param_1 * 16 + D_589b_mapOriginX & 0xf0; // wrap x?
        D_589c_mapOriginY = (char)param_2 * 16 + D_589c_mapOriginY & 0xf0; // wrap y?
        OUTSUBS_01b4(param_1, param_2); // 7b8a
        ULTIMA_5e4a_BuildLightMap();
    }
}

static void MAINOUT_03e0(void)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_4_ab = 0;

    local_8 = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    if (local_8 == TILE_MAP_GRASS)
    {
        // 0406
        local_6 = 0;
    }
    else if (local_8 == TILE_MAP_1E || local_8 == TILE_MAP_1F)
    {
        // 041a
        local_6 = 1;
    }
    else if (local_8 < TILE_MAP_POISON || local_8 >= TILE_MAP_HUT) // 042e
    {
        // -> 0406
        local_6 = 0;
    }
    else if (local_8 >= TILE_MAP_9)
    {
        local_6 = 2;
    }
    else
    {
        local_6 = 1;
    }

    // 041f
    switch (local_6)
    {
    case 1:
        // 0448
        local_4_ab += MAINOUT_1a60();
        MAINOUT_007a();

        if (local_4_ab == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29bf, "Slow progress!\n"));
        }

        ULTIMA_4f7c_AdvanceTime(2);
        break;

    case 2:
        // 0468
        local_4_ab += MAINOUT_1a60();
        MAINOUT_007a();

        local_4_ab += MAINOUT_1a60();
        MAINOUT_007a();

        if (local_4_ab == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29cf, "Very slow!\n"));
        }

        ULTIMA_4f7c_AdvanceTime(4);
        break;
    }
}

u8 D_29f6[4] = { 0x00, 0x00, 0xFF, 0x01 };

u8 D_29fa[4] = { 0x01, 0xFF, 0x00, 0x00 };

static int MAINOUT_0490(int param_1, int param_2)
{
    int local_8;
    int local_6;
    int local_4_ab;

    if ((D_587c_partyTile & 0xfc) == TILE_ACTOR_FRIGATE_20)
    {
        if (param_1 != D_5955_sailDir)
        {
            D_5955_sailDir = param_1;
            D_5883 = 0;
        }
        D_5956_newPrompt = 0;
    }

    // 04b8
    if (D_587c_partyTile == TILE_ACTOR_AVATAR || (D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
    {
        ULTIMA_433e_AudioFootstep();
    }

    // 04cb
    local_6 = local_8 = 0;

    switch (param_1)
    {
    case DIR_UP:
        // 04f0
        local_8--;
#if !defined(MATCHING_BUILD)
        local_4_ab = MAINOUT_00da(0);
        if (local_4_ab != 0)
        {
            return local_4_ab;
        }
#else
        if (MAINOUT_00da(0))
        {
            return;
        }
#endif

        // 0500
        if (D_5955_sailDir == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29db, "North\n"));
        }
        break;

    case DIR_DOWN:
        // 054e
        local_8++;
#if !defined(MATCHING_BUILD)
        local_4_ab = MAINOUT_00da(2);
        if (local_4_ab != 0)
        {
            return local_4_ab;
        }
#else
        if (MAINOUT_00da(2))
        {
            return;
        }
#endif
        if (D_5955_sailDir == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29e2, "South\n"));
        }
        break;

    case DIR_RIGHT:
        // 055c
        local_6++;
#if !defined(MATCHING_BUILD)
        local_4_ab = MAINOUT_00da(1);
        if (local_4_ab != 0)
        {
            return local_4_ab;
        }
#else
        if (MAINOUT_00da(1))
        {
            return;
        }
#endif
        if (D_5955_sailDir == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29e9, "East\n"));
        }
        break;

    case DIR_LEFT:
        // 0576
        local_6--;
#if !defined(MATCHING_BUILD)
        local_4_ab = MAINOUT_00da(3);
        if (local_4_ab != 0)
        {
            return local_4_ab;
        }
#else
        if (MAINOUT_00da(3))
        {
            return;
        }
#endif
        if (D_5955_sailDir == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29ef, "West\n"));
        }
        break;
    }

    // 050e
    local_4_ab = MAINOUT_01fe(local_6, local_8);
    if (param_2 == 0 && local_4_ab != 0)
    {
        if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
        {
            ULTIMA_433e_AudioFootstep();
        }
        // 0530
        MAINOUT_0354(local_6, local_8);
        MAINOUT_03e0();
    }
    return local_4_ab;
}

// TODO
// u8 D_29f5[4]; // wind-releated
// u8 D_29f9[4]; // wind-releated

static int MAINOUT_0598(void)
{
    int local1_a;
    int local2_8;
    int local3_6;
    int local4_4;

    ULTIMA_5910_UpdateFrame();
    if (D_a9fa_vitalsDirty != 0)
    {
        ULTIMA_2900_UpdateVitalsDisplay();
        D_a9fa_vitalsDirty = 0;
    }

    // 05b2
    if ((GetMapViewport(5, 6) & 0xfc) == 0xd4)
    {
        OUTSUBS_0458_Falls();
        local3_6 = 0;
    }
    else
    {
        // 05c6
        if (D_5956_newPrompt != 0)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a_PrintLeftCap();
        }

        // 05d7
        D_5956_newPrompt = 1;
        while (D_5955_sailDir != 0)
        {
            local3_6 = ULTIMA_2032_ToUpper(ULTIMA_1b38_PollKeyWithCursor());
            if (local3_6 != 0 && local3_6 != D_5955_sailDir)
                break;

            // 05e6
            local3_6 = (uint)D_5955_sailDir;
            if (D_5892_windDir != 0)
            {
                local2_8 = local1_a = 0;

                switch (local3_6)
                {
                case DIR_UP:
                    // 0614
                    local1_a = (byte)-1;
                    break;
                case DIR_DOWN:
                    // 0658
                    local1_a = 1;
                    break;
                case DIR_RIGHT:
                    // 0660
                    local2_8 = 1;
                    break;
                case DIR_LEFT:
                    // 0668
                    local2_8 = (byte)-1;
                    break;
                }

                // 0619
                local4_4 = 1;
                if (D_29f6[D_5892_windDir - 1] != local2_8)
                {
                    local4_4++;
                }
                // 062e
                if (D_29fa[D_5892_windDir - 1] != local1_a)
                {
                    local4_4++;
                }
                // 0640
                if (local4_4 % 3 <= (uint)D_5883)
                {
                    D_5883 = 0;
                    break;
                }
            }

            // 0670
            if (D_57bb_hmsCapePlans > 0x7f)
            {
                D_a524 = (uint)(D_a524 == 0);
                ULTIMA_4f7c_AdvanceTime(1);
            }
            else
            {
                ULTIMA_4f7c_AdvanceTime(2);
            }

            if (D_a524 == 0 || D_57bb_hmsCapePlans < 0x80)
            {
                MAINOUT_1a60();
            }

            ULTIMA_3ae6_WaitFrames(1);
            D_5883 = D_5883 + 1;
        }

        if (D_5955_sailDir == 0)
        {
            local3_6 = ULTIMA_266c_GetChar();
        }
    }

    return local3_6;
}

int MAINOUT_06ec_AttackCmd(void)
{
    int local_6;
    int local_8;
    int local_4_ab = 0;
    int local_a;

    ULTIMA_1850_PrintString(_TEXT(0x29fe, "Attack-"));
    if ((*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) < TILE_MAP_POISON) &&
        ((D_587c_partyTile & 0xfc) == TILE_ACTOR_SKIFF || (D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET))
    {
        ULTIMA_1850_PrintString(_TEXT(0x2a06, "On foot!\n"));
        local_4_ab = 0;
    }
    else
    {
        if (ULTIMA_35ec_SelectDirection() != 0)
        {
            local_6 = (uint)D_5896_mapX + D_5876_tempX;
            local_8 = (uint)D_5897_mapY + D_5878_tempY;
            local_a = ULTIMA_368e_FindActorTileAtPos(local_6, local_8, D_5895_mapLevel) & 0xfc;
            if (local_a == TILE_ACTOR_PIRATE || (local_a != TILE_ACTOR_SHARD && local_a != TILE_ACTOR_E8 && local_a >= TILE_ACTOR_WIZARD))
            {
                ULTIMA_6150_Combat(D_5876_tempX);
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x2a10, "Nothing to attack!\n"));
            }
        }
    }
    return local_4_ab;
}

// Load dungeon
static int MAINOUT_0790_LoadDungeon(char* param_1)
{
    int local_6_an;
    int local_4;

    ULTIMA_1850_PrintString(param_1);
    for (local_4 = 0x20; local_4 < 0x28; local_4++)
    {
        if (D_1e8a_placeX[local_4] == D_5896_mapX && D_1eb2_placeY[local_4] == D_5897_mapY)
            break;
    }

    if (local_4 < 0x28)
    {
        if (D_587c_partyTile != TILE_ACTOR_AVATAR)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2a24, "\nOn foot!\n"));
#if !defined(MATCHING_BUILD)
            return 0; // TODO: validate return value
#else
            return; // sic
#endif
        }

        if (local_4 == 0x27)
        {
            if ((D_58c8_shadowlordTowns[0] & D_58c8_shadowlordTowns[1] & D_58c8_shadowlordTowns[2]) < 0x80)
            {
                ULTIMA_1850_PrintString(_TEXT(0x2a2f, "\nAttacked at entrance!\n"));
                local_6_an = ULTIMA_38e4_FindActorSlot();
                D_5c5a_actors[local_6_an].tile = TILE_ACTOR_SHADOWLORD;
                ULTIMA_6150_Combat(local_6_an);
#if !defined(MATCHING_BUILD)
                return 0; // TODO: validate return value
#else
                return; // sic
#endif
            }
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x2a47, "\n\n"));
            ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
            ULTIMA_1850_PrintString(D_1e3a_mapNames[local_4]);
            ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
        }

        ULTIMA_16ba_PrintChar('\n');
        if (D_a9bd_curDisk[0] != 1)
        {
            ULTIMA_251e_SwitchDisks(1);
            while (ULTIMA_1674_TestOpenFile(/*0x2a4a*/ "BRIT.DAT") == 0) {}
        }

        ULTIMA_25d8_WriteFileToDisk(OUTSUBS_0368_GetWorldSavefile(), D_5c5a_actors, 0x100);
        ULTIMA_251e_SwitchDisks(2);
        ULTIMA_256e_ReadFileFromDisk(/*0x2a53*/ "DUNGEON.DAT", D_595a_dungeonMap, 0x200, local_4 * 0x200 + -0x4000);
        D_5893_mapId = local_4 + 1;
        if (D_5895_mapLevel != 0 && (byte)(local_4 + 1) != 0x28)
        {
            D_5895_mapLevel = 7;
            D_6603_dungeonFacing = 3;
            D_6602 = 4;
            D_5896_mapX = D_5897_mapY = 7;
        }
        else
        {
            D_5895_mapLevel = 0;
            D_5896_mapX = D_5897_mapY = 1;
            D_6603_dungeonFacing = 1;
            D_6602 = 5;
        }
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x2a5f, "\nWhat dungeon?\n"));
    }

    return 1;
}

int MAINOUT_08de_EnterCmd(void)
{
    int local_6;
    int local_4_ab;
    int local_8;

    local_4_ab = 1;

    ULTIMA_1850_PrintString(_TEXT(0x2a6f, "Enter "));

    local_8 = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    switch (local_8)
    {
    case TILE_MAP_SHRINE:
        ULTIMA_1850_PrintString(_TEXT(0x2a76, "the shrine of\n"));
        for (local_6 = 0; local_6 < 8; local_6++)
        {
            if (D_1f6e[local_6] == D_5896_mapX && D_1f76[local_6] == D_5897_mapY)
                break;
        }

        ULTIMA_1850_PrintString(D_1f4e_virtueNames[local_6]);
        ULTIMA_16ba_PrintChar('\n');
        CAST2_0e76_Shrine();
        break;

    case TILE_MAP_HUT:
        local_4_ab = OUTSUBS_0388_EnterTown(_TEXT(0x2a85, "hut"));
        break;

    case TILE_MAP_CODEX:
        ULTIMA_1850_PrintString(_TEXT(0x2a89, "the Shrine of the Codex!\n"));
        CAST2_0e76_Shrine();
        break;

    case TILE_MAP_KEEP:
        local_4_ab = OUTSUBS_0388_EnterTown(_TEXT(0x2aa3, "keep"));
        break;

    case TILE_MAP_VILLAGE:
        local_4_ab = OUTSUBS_0388_EnterTown(_TEXT(0x2aa8, "village"));
        break;

    case TILE_MAP_TOWNE:
        local_4_ab = OUTSUBS_0388_EnterTown(_TEXT(0x2ab0, "towne"));
        break;

    case TILE_MAP_CASTLE:
        local_4_ab = OUTSUBS_0388_EnterTown(_TEXT(0x2ab6, "castle"));
        break;

    case TILE_MAP_CAVE:
        local_4_ab = MAINOUT_0790_LoadDungeon(_TEXT(0x2abd, "cave"));
        break;

    case TILE_MAP_MINE:
        local_4_ab = MAINOUT_0790_LoadDungeon(_TEXT(0x2ac2, "mine"));
        break;

    case TILE_MAP_DUNGEON:
        local_4_ab = MAINOUT_0790_LoadDungeon(_TEXT(0x2ac7, "dungeon"));
        break;

    case TILE_MAP_RUINS:
        ULTIMA_1850_PrintString(_TEXT(0x2acf, "ruins"));
        break;

    case TILE_MAP_LIGHTHOUSE:
        local_4_ab = OUTSUBS_0388_EnterTown(_TEXT(0x2ad5, "lighthouse"));
        break;

    case TILE_MAP_PALACEBT:
        local_4_ab = OUTSUBS_0388_EnterTown(_TEXT(0x2ae0, "the palace of Blackthorn!"));
        break;

    case TILE_MAP_CASTLELB:
        local_4_ab = OUTSUBS_0388_EnterTown(_TEXT(0x2afa, "the Castle of Lord British!"));
        break;

    default:
        ULTIMA_1850_PrintString(_TEXT(0x2b16, "What?\n"));
        local_4_ab = 0;
        break;
    }

    return local_4_ab;
}

static int MAINOUT_0a1a(int param_1)
{
    if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) == 0xff && D_587a_statusIcon != 0xe)
    {
        D_58a5_lightLevel = 0;
        if (param_1 == 0)
        {
            ULTIMA_5910_UpdateFrame();
            param_1 = 1;
        }
    }
    else
    {
        param_1 = 0;
        ULTIMA_4f7c_AdvanceTime(0);
    }

    return param_1;
}

// check earthquake
static void MAINOUT_0a60(void)
{
    if (D_5895_mapLevel != 0)
    {
        if (ULTIMA_2092_RandomRange(0, 0xff) == 0x69)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2b1d, "EARTHQUAKE!\n"));
            ULTIMA_3072_ShakeScreen();
            ULTIMA_2aa8_DamageParty();
        }
    }
}

static void MAINOUT_0a84_MainLoop(void)
{
    int local_12_ab;
    int local_10_ad; // not used
    int local_e; // not used
    bool local_c;
    int local_a;
    int local_8;
    byte local_6;
    int local_4_ab = 0;
    do
    {
        // 0a8f
        local_c = 0;
        local_a = 1;
        local_4_ab = MAINOUT_0a1a(local_4_ab);
        local_8 = ULTIMA_39fc_GetFirstActivePartyMember();
        if (local_8 == 1)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a_PrintLeftCap();
            // 0aba
            ULTIMA_1850_PrintString(_TEXT(0x2b2a, "Zzzzzz...\n"));
        }
        else if (local_8 == -1) // 0ac2
        {
            if (D_a9bd_curDisk[0] != 1)
            {
                ULTIMA_251e_SwitchDisks(1);
                // 0ad6
                while (ULTIMA_1674_TestOpenFile(/*0x2b35*/ "BRIT.DAT") == 0) {}
            }
            // 0ae1
            ULTIMA_25d8_WriteFileToDisk(OUTSUBS_0368_GetWorldSavefile(), D_5c5a_actors, 0x100);
            BLCKTHRN_0910_Death();
            local_c = 1;
            local_a = 0;
        }
        else
        {
            // 0b00
            if (ULTIMA_48a8_EnterMoongate() != 0)
            {
                CAST2_0e76_Shrine();
            }
            // 0b0a
            if (D_5893_mapId != 0)
            {
                return;
            }
            // 0b14
            local_6 = MAINOUT_0598();
            if (local_6 < 0x20)
            {
                switch (local_6 & 0xff)
                {
                case U5_KEY_CTRL_K: // 0b34
                    ULTIMA_1a3e_PrintNumber(D_5888_karma, 1, 0x20);
                    // -> 0b75
                    ULTIMA_16ba_PrintChar('\n');
                    // -> 0af8
                    local_a = 0;
                    break; // -> 0c12

                case U5_KEY_CTRL_E: // 0b48
                    ULTIMA_1850_PrintString(_TEXT(0x2b3e, "Exit to DOS? "));
                    local_6 = ULTIMA_266c_GetChar();
                    if (local_6 == 'Y')
                    {
                        ULTIMA_0878_RestoreVideoMode();
                        exit(0);
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x2b4c, "N\n"));
                    }
                    break;

                case U5_KEY_CTRL_V: // 0b6e
                    ULTIMA_1850_PrintString(_TEXT(0x2b4f, U5_VERSION));
                    // 0b75
                    ULTIMA_16ba_PrintChar('\n');
                    local_a = 0;
                    break;

                case U5_KEY_CTRL_S: // 0b80
                    ULTIMA_1850_PrintString(_TEXT(0x2b54, "Sound "));
                    if (D_a9ce_soundOn != 0)
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x2b5b, "Off\n"));
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x2b60, "On\n"));
                    }
                    // ?
                    D_a9ce_soundOn = !D_a9ce_soundOn;
                    break;

                case U5_KEY_LEFT: // 0baa
                case U5_KEY_RIGHT:
                case U5_KEY_UP:
                case U5_KEY_DOWN:
                    local_a = MAINOUT_0490(local_6, local_4_ab);
                    break;

                case 0: // 0af8
                    local_a = 0;
                    break;

                default: // 0bb8
                    ULTIMA_1850_PrintString(_TEXT(0x2b64, "What?\n"));
                    break;
                }
            }
            else
            {
                // 0bee
                if (local_6 < 0x30 || local_6 > 0x39)
                {
                    // 0bfa
                    local_a = ULTIMA_3178_ProcessCommand(local_6 & 0xff);
                }
                else
                {
                    // 0c06
                    local_a = ULTIMA_4080_SetActivePlayer(local_6);
                }
            }
        }
        // 0c12
        if ((D_587c_partyTile & 0xfc) != TILE_ACTOR_FRIGATE_20)
        {
            D_5955_sailDir = 0;
        }
        // 0c20
        if (D_5893_mapId != 0)
        {
            local_c = 1;
        }
        else
        {
            if (local_a != 0)
            {
                // 0c39
                ULTIMA_4f7c_AdvanceTime(2);
                local_12_ab = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
                if ((local_12_ab & 0xfe) == TILE_MAP_6A)
                {
                    MAINOUT_1be8();
                }
                // 0c64
                else if (local_12_ab == TILE_MAP_POISON && D_587c_partyTile == TILE_ACTOR_AVATAR)
                {
                    OUTSUBS_05fc_StepOnPoisonTile();
                    ULTIMA_3ae6_WaitFrames(1);
                }
                else if (local_12_ab == TILE_MAP_LAVA)
                {
                    OUTSUBS_05ee_StepOnBurningTile();
                }
                else if (D_5896_mapX == 0xe9 && D_5897_mapY == 0xeb && D_5895_mapLevel == 0 && D_5893_mapId == 0)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x2b6b, "\n\""));
                    if ((byte)D_58cc_questsOrdained != 0)
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x2b6e, "Pass, Seeker!\"\n"));
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x2b7e, "Thou art not upon a Sacred Quest!\n"));
                        ULTIMA_1850_PrintString(_TEXT(0x2ba1, "Passage denied!\"\n"));
                        D_5897_mapY++;
                    }
                }

                // 0cd0
                MAINOUT_0a60();
                ULTIMA_2ae8_EndTurn();
                if (local_12_ab == 1 && ((D_587c_partyTile & 0xfc) == TILE_ACTOR_SKIFF || (D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET))
                {
                    ULTIMA_1850_PrintString(_TEXT(0x2bb3, "Rough seas!\n"));
                    ULTIMA_3522_HitEffectAt(D_5896_mapX, D_5897_mapY);
                    MAINOUT_109e();
                }
                if ((local_12_ab & 0xfc) == TILE_MAP_WATERFALL)
                {
                    OUTSUBS_0458_Falls();
                }
                MAINOUT_1a60();
            }
        }
    } while (!local_c);
}

void MAINOUT_0d22_Entry(void)
{
    int local_8;
    int local_6;
    ActorFmt* local_4_ab;

#if defined(ENABLE_BGM)
    AUDIO_StopBgm();
#endif

    MAINOUT_0000();

    if (D_6605_boughtShip >= 0x40)
    {
        local_6 = ULTIMA_38e4_FindActorSlot();
        local_4_ab = &D_5c5a_actors[local_6]; // get some actor
        local_4_ab->extra = D_6605_boughtShip & 0x3f;
        local_4_ab->x = D_5953_boughtShipX; // ship_x
        local_4_ab->y = D_5954_boughtShipY; // ship_y
        local_4_ab->z = 0;

        if (D_6605_boughtShip > 0x7f)
        {
            local_8 = TILE_ACTOR_FRIGATE_25;
        }
        else
        {
            local_8 = TILE_ACTOR_SKIFF_29;
        }

        local_4_ab->tile = local_4_ab->animTile = local_8;
        local_4_ab->value = 99;
        D_6605_boughtShip = 0;
    }

#if defined(ENABLE_BGM)
    AUDIO_PlayBgmPerMap();
#endif

    MAINOUT_0a84_MainLoop();
}

int MAINOUT_0d8c(void)
{
    int local_6_an;
    int local_4;

    if (D_5895_mapLevel > 0x7f)
    {
        return 3;
    }

    local_6_an = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    if (local_6_an >= TILE_MAP_20 && local_6_an <= TILE_MAP_26)
    {
        local_4 = 0;
    }
    else
    {
        if (local_6_an == TILE_MAP_POISON || (local_6_an >= TILE_MAP_9 && local_6_an <= TILE_MAP_F))
        {
            local_4 = 2;
        }
        else
        {
            local_4 = 1;
        }
    }

    if (D_587f_hour >= 0x20 || D_587f_hour < 5)
    {
        return local_4 + 3;
    }

    return local_4;
}

// The original adds the index and the pointer as integers (mov bx,cx / add bx,si),
// not with the [bx][si] addressing mode that param_1[i] gives.
#if defined(TARGET_DOS16)
#define MAINOUT_0E04_AT(i) (*(byte*)((uint)(i) + (uint)param_1))
#else
#define MAINOUT_0E04_AT(i) (param_1[i])
#endif

static int MAINOUT_0e04(byte* param_1)
{
    int local_6_an;
    int local_4;

    local_4 = ULTIMA_2092_RandomRange(0, 0xff);

    for (local_6_an = 0; MAINOUT_0E04_AT(local_6_an) <= local_4; local_6_an++)
    {
        local_4 -= MAINOUT_0E04_AT(local_6_an);
    }
    return local_6_an;
}

static byte MAINOUT_0e4e(int param_1)
{
    int unused;

    if (param_1 < TILE_MAP_POISON || (param_1 >= TILE_MAP_60 && param_1 <= TILE_MAP_6F) || (param_1 >= TILE_MAP_WATERFALL && param_1 <= TILE_MAP_WATERFALL + 3) ||
        (param_1 >= TILE_MAP_WATER_E4 && param_1 <= TILE_MAP_WATER_E7))
    {
        if (ULTIMA_2092_RandomRange(0, 0x40) < 0x10)
        {
            if (D_5895_mapLevel < 0x80)
            {
                if (param_1 == 1 && ULTIMA_2092_RandomRange(0, 7) == 7)
                {
                    return TILE_ACTOR_WHIRLPOOL;
                }
                else
                {
                    return D_2bd4[MAINOUT_0e04(D_2bf0)];
                }
            }
            else
            {
                return D_2bda[MAINOUT_0e04(D_2bf6)];
            }
        }
    }
    else if (param_1 == TILE_MAP_7)
    {
        if (ULTIMA_2092_RandomRange(0, 3) == 0)
        {
            return TILE_ACTOR_E0;
        }
    }
    else
    {
        if (param_1 == TILE_MAP_POISON && D_5895_mapLevel == 0xff)
        {
            return TILE_ACTOR_F8;
        }

        if (param_1 == TILE_MAP_C || param_1 == TILE_MAP_D)
        {
            return 0;
        }

        if (param_1 < TILE_MAP_HUT || ((byte)param_1 & 0xfc) == TILE_MAP_30)
        {
            if (D_5895_mapLevel < 0x80)
            {
                return D_2bc0[MAINOUT_0e04(D_2bdc)];
            }

            return D_2bcc[MAINOUT_0e04(D_2be8)];
        }
    }

    return 0;
}

static void MAINOUT_0f4e(void)
{
    do
    {
        D_5876_tempX = (byte)(ULTIMA_2092_RandomRange(0, 0x1f) + D_589b_mapOriginX);
        D_5878_tempY = (byte)(ULTIMA_2092_RandomRange(0, 0x1f) + D_589c_mapOriginY);
    } while (abs(D_5876_tempX - D_5896_mapX) <= 6 || abs(D_5878_tempY - D_5897_mapY) <= 6 ||
        abs(D_5876_tempX - D_5896_mapX) >= 0xfa || abs(D_5878_tempY - D_5897_mapY) >= 0xfa);
}

void MAINOUT_0fc4(void)
{
    int local_a;
    int local_8;
    int local_6;
    byte local_4_ab;

    for (local_6 = 0; local_6 < 0x80; local_6++)
    {
        MAINOUT_0f4e();

        local_8 = D_5876_tempX;
        local_a = D_5878_tempY;

        local_4_ab = MAINOUT_0e4e(*ULTIMA_4402_GetTileAddr(local_8, local_a));
        if (local_4_ab != 0)
        {
            if (local_4_ab != TILE_ACTOR_PIRATE || (*ULTIMA_4402_GetTileAddr(local_8, local_a) & 0xf0) != TILE_MAP_60)
            {
                break;
            }
        }
    }

    if (local_6 != 0x80)
    {
        local_6 = ULTIMA_38e4_FindActorSlot();
        ULTIMA_3a74_SetActor(local_4_ab, local_4_ab, local_8, local_a, D_5895_mapLevel, 0, local_6);
        if (local_4_ab == TILE_ACTOR_PIRATE)
        {
            D_5c5a_actors[local_6].value = 100;
        }
    }
}

int MAINOUT_105c(int param_1)
{
    if (param_1 >= 0x2c && param_1 <= 0x2f)
    {
        return 1;
    }
    
    if (param_1 < 0x80)
    {
        return 0;
    }

    if (param_1 >= 0xb4 && param_1 <= 0xb7 || param_1 >= 0xe8 && param_1 <= 0xeb)
    {
        return 0;
    }

    return 1;
}
