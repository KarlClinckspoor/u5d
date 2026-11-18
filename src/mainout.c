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

static u16 D_a524_capeToggle; // this object's _BSS (DS:a524)

// defined in mainoutb.c, the part of MAINOUT.OVL built with MSC 5.0
extern u8 D_2bc0_landMonsters[];
extern u8 D_2bcc_deepLandMonsters[];
extern u8 D_2bd4_seaMonsters[];
extern u8 D_2bda_deepSeaMonsters[];
extern u8 D_2bdc_landWeights[];
extern u8 D_2be8_deepLandWeights[];
extern u8 D_2bf0_seaWeights[];
extern u8 D_2bf6_deepSeaWeights[];
int MAINOUT_1a60_MonstersTurn(void);
void MAINOUT_109e_DamageVehicle(void);
void MAINOUT_1be8_BridgeTrolls(void);

// enters the overworld: clears the sky, sets the view origin around the party, loads the map chunks, the light map, wind, sun and moons, the underworld's items
void MAINOUT_0000_SetUpOverworld(void)
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
    OUTSUBS_01b4_LoadViewChunks(0, 1);
    OUTSUBS_01b4_LoadViewChunks(0, -1);
    ULTIMA_4f7c_AdvanceTime(0);
    ULTIMA_5e4a_BuildLightMap();
    ULTIMA_2e96_SetWindDirection(-1);
    ULTIMA_4a84_DrawSunAndMoons();
    OUTSUBS_0566_PlaceAmuletShards();
}

// one frame's wait when an actor of the table is within five cells (only actor 1 is looked at: the loop breaks at once)
void MAINOUT_007a_WaitIfActorNear(void)
{
    int distX;
    int distY;

    int index;
    ActorFmt* actor_h;

    for (index = 1; index < 32; index++)
    {
        actor_h = &D_5c5a_actors[index];

        distX = abs(D_5896_mapX - actor_h->x);
        distY = abs(D_5897_mapY - actor_h->y);

        if (actor_h->tile != 0 && actor_h->z == D_5895_mapLevel && distX < 6 && distY < 6)
        {
            ULTIMA_3ae6_WaitFrames(1);
        }

        break;
    }
}

// "Ride/Fly/Row/Head": turns the party's tile to the direction; 1 when the party only turned (no step), also a frigate under no wind
static int MAINOUT_00da_TurnVehicle(int dir)
{
    int oldTile;
    int turned = 0;

    switch (D_587c_partyTile & 0xfc)
    {
    case TILE_ACTOR_HORSE:
        // 010a
        ULTIMA_1850_PrintString(_TEXT(0x2946, "Ride "));
        if (dir == 1)
        {
            D_587c_partyTile = TILE_ACTOR_RIDING_HORSE;
        }
        else if (dir == 3)
        {
            D_587c_partyTile = TILE_ACTOR_RIDING_HORSE + 1;
        }
        break;

    case TILE_ACTOR_FLYING_CARPET:
        // 0130
        ULTIMA_1850_PrintString(_TEXT(0x294c, "Fly "));
        if (dir == 1)
        {
            D_587c_partyTile = TILE_ACTOR_FLYING_CARPET;
        }
        else if (dir == 3)
        {
            D_587c_partyTile = TILE_ACTOR_FLYING_CARPET + 1;
        }
        break;

    case TILE_ACTOR_SKIFF:
        // 0152
        ULTIMA_1850_PrintString(_TEXT(0x2951, "Row "));
        D_587c_partyTile = (char)dir + (D_587c_partyTile & 0xfc);
        break;

    case TILE_ACTOR_FRIGATE_20:
    case TILE_ACTOR_FRIGATE_24:
        // 016a
        oldTile = D_587c_partyTile;
        D_587c_partyTile = (char)dir + (D_587c_partyTile & 0xfc);
        if (oldTile != D_587c_partyTile)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2956, "Head "));

            switch (dir)
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
            D_5956_newPrompt = turned = 1;
            if (D_5c5a_actors[0].value < 0x32)
            {
                ULTIMA_1850_PrintString(_TEXT(0x2976, "Hull weak!\n"));
            }
        }
        // 01dc
        else if (D_587c_partyTile < TILE_ACTOR_FRIGATE_24 && D_5892_windDir == 0)
        {
            turned = 1;
        }

        break;
    }

    // 0129
    return turned;
}

// whether the party may step by (x, y): the actor there and the walkable tile decide; prints Blocked!, OUCH!, COLLISION!, BREAKING UP!, Docked!
static int MAINOUT_01fe_CanStepTo(int stepX, int stepY)
{
    int destTile;
    int blockerTile;
    bool canStep_c;

    if ((D_587c_partyTile & 0xfc) == TILE_ACTOR_FRIGATE_24)
    {
        ULTIMA_1850_PrintString(_TEXT(0x2982, "Rowing!\n"));
    }

    canStep_c = 1;
    blockerTile = ULTIMA_368e_FindActorTileAtPos(stepX + (uint)D_5896_mapX, stepY + (uint)D_5897_mapY, D_5895_mapLevel);
    if (blockerTile != 0)
    {
        canStep_c = 0;
        if (D_587c_partyTile >= TILE_ACTOR_30 || D_587c_partyTile < TILE_ACTOR_FRIGATE_20)
        {
            if ((blockerTile >= TILE_ACTOR_FRIGATE_24 && blockerTile < TILE_ACTOR_PIRATE) || blockerTile == TILE_ACTOR_CARPET || (blockerTile & 0xfe) == TILE_ACTOR_HORSE)
            {
                canStep_c = 1;
            }
        }
        else
        {
            if (D_587c_partyTile >= TILE_ACTOR_SKIFF && blockerTile >= TILE_ACTOR_FRIGATE_24 && blockerTile < TILE_ACTOR_SKIFF)
            {
                canStep_c = 1;
            }
        }
    }

    destTile = GetMapViewport(stepX + 5, stepY + 5);

    canStep_c = canStep_c && ULTIMA_2c4c_IsWalkableTile(D_587c_partyTile, destTile) != 0 ? 1 : 0;
    if (canStep_c == 0)
    {
        if (D_5955_sailDir != 0)
        {
            if (destTile == TILE_MAP_WATER_3)
            {
                ULTIMA_1850_PrintString(_TEXT(0x298b, "BREAKING UP!\n"));
            }
            else if (destTile != TILE_MAP_47)
            {
                ULTIMA_1850_PrintString(_TEXT(0x2999, "COLLISION!\n"));
            }

            if (destTile == TILE_MAP_47)
            {
                ULTIMA_1850_PrintString(_TEXT(0x29a5, "Docked!\n"));
                D_587c_partyTile += 4;
            }
            else
            {
                ULTIMA_223c_AudioWhiteNoise(100, 2000, 300);
                MAINOUT_109e_DamageVehicle();
            }

            D_5955_sailDir = 0;
            D_5956_newPrompt = 1;
        }
        else if (D_587c_partyTile < TILE_ACTOR_FRIGATE_20 || (blockerTile & 0xfc) != TILE_ACTOR_WHIRLPOOL)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29ae, "Blocked!\n"));
            if (destTile == TILE_MAP_2F)
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

    return canStep_c;
}

// the party moves by (x, y); near the edge of the 32x32 view the chunks are shifted and loaded
void MAINOUT_0354_MoveParty(int stepX, int stepY)
{
    int relY;
    int relX;

    D_5896_mapX += stepX;
    D_5897_mapY += stepY;
    D_24e6_viewDirty = 1;
    relX = D_5896_mapX - D_589b_mapOriginX & 0x1f;
    relY = D_5897_mapY - D_589c_mapOriginY & 0x1f;

    if (relX < 5 || relX > 0x1a || relY < 5 || relY > 0x1a)
    {
        OUTSUBS_02c8_ShiftChunks(stepX, stepY); // 7bd2
        D_589b_mapOriginX = (char)stepX * 16 + D_589b_mapOriginX & 0xf0; // wrap x?
        D_589c_mapOriginY = (char)stepY * 16 + D_589c_mapOriginY & 0xf0; // wrap y?
        OUTSUBS_01b4_LoadViewChunks(stepX, stepY); // 7b8a
        ULTIMA_5e4a_BuildLightMap();
    }
}

// after a step: on rough terrain (forest, hills, mountains) the monsters move once or twice more, "Slow progress!" / "Very slow!"
static void MAINOUT_03e0_SlowTerrain(void)
{
    int tile;
    int terrainClass;
    int moved;

    moved = 0;

    tile = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    if (tile == TILE_MAP_GRASS)
    {
        // 0406
        terrainClass = 0;
    }
    else if (tile == TILE_MAP_1E || tile == TILE_MAP_1F)
    {
        // 041a
        terrainClass = 1;
    }
    else if (tile < TILE_MAP_POISON || tile >= TILE_MAP_HUT) // 042e
    {
        // -> 0406
        terrainClass = 0;
    }
    else if (tile >= TILE_MAP_9)
    {
        terrainClass = 2;
    }
    else
    {
        terrainClass = 1;
    }

    // 041f
    switch (terrainClass)
    {
    case 1:
        // 0448
        moved += MAINOUT_1a60_MonstersTurn();
        MAINOUT_007a_WaitIfActorNear();

        if (moved == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29bf, "Slow progress!\n"));
        }

        ULTIMA_4f7c_AdvanceTime(2);
        break;

    case 2:
        // 0468
        moved += MAINOUT_1a60_MonstersTurn();
        MAINOUT_007a_WaitIfActorNear();

        moved += MAINOUT_1a60_MonstersTurn();
        MAINOUT_007a_WaitIfActorNear();

        if (moved == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x29cf, "Very slow!\n"));
        }

        ULTIMA_4f7c_AdvanceTime(4);
        break;
    }
}

u8 D_29f6_windDx[4] = { 0x00, 0x00, 0xFF, 0x01 };

u8 D_29fa_windDy[4] = { 0x01, 0xFF, 0x00, 0x00 };

// a direction key: prints the direction, turns the vehicle, steps when allowed (and the frigate sets sail); 1 when it stepped
static int MAINOUT_0490_WalkInDirection(int dir, int noMove)
{
    int stepY;
    int stepX;
    int result_b;

    if ((D_587c_partyTile & 0xfc) == TILE_ACTOR_FRIGATE_20)
    {
        if (dir != D_5955_sailDir)
        {
            D_5955_sailDir = dir;
            D_5883_sailCounter = 0;
        }
        D_5956_newPrompt = 0;
    }

    // 04b8
    if (D_587c_partyTile == TILE_ACTOR_AVATAR || (D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
    {
        ULTIMA_433e_AudioFootstep();
    }

    // 04cb
    stepX = stepY = 0;

    switch (dir)
    {
    case DIR_UP:
        // 04f0
        stepY--;
#if !defined(MATCHING_BUILD)
        result_b = MAINOUT_00da_TurnVehicle(0);
        if (result_b != 0)
        {
            return result_b;
        }
#else
        if (MAINOUT_00da_TurnVehicle(0))
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
        stepY++;
#if !defined(MATCHING_BUILD)
        result_b = MAINOUT_00da_TurnVehicle(2);
        if (result_b != 0)
        {
            return result_b;
        }
#else
        if (MAINOUT_00da_TurnVehicle(2))
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
        stepX++;
#if !defined(MATCHING_BUILD)
        result_b = MAINOUT_00da_TurnVehicle(1);
        if (result_b != 0)
        {
            return result_b;
        }
#else
        if (MAINOUT_00da_TurnVehicle(1))
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
        stepX--;
#if !defined(MATCHING_BUILD)
        result_b = MAINOUT_00da_TurnVehicle(3);
        if (result_b != 0)
        {
            return result_b;
        }
#else
        if (MAINOUT_00da_TurnVehicle(3))
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
    result_b = MAINOUT_01fe_CanStepTo(stepX, stepY);
    if (noMove == 0 && result_b != 0)
    {
        if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
        {
            ULTIMA_433e_AudioFootstep();
        }
        // 0530
        MAINOUT_0354_MoveParty(stepX, stepY);
        MAINOUT_03e0_SlowTerrain();
    }
    return result_b;
}

// TODO
// u8 D_29f5[4]; // wind-releated
// u8 D_29f9[4]; // wind-releated

// redraws, handles the falling tile, prints the prompt; a frigate under sail moves with the wind until a key; returns the key
static int MAINOUT_0598_WaitForKey(void)
{
    int windStepY;
    int windStepX;
    int key_h;
    int windScore_c;

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
        key_h = 0;
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
            key_h = ULTIMA_2032_ToUpper(ULTIMA_1b38_PollKeyWithCursor());
            if (key_h != 0 && key_h != D_5955_sailDir)
                break;

            // 05e6
            key_h = (uint)D_5955_sailDir;
            if (D_5892_windDir != 0)
            {
                windStepX = windStepY = 0;

                switch (key_h)
                {
                case DIR_UP:
                    // 0614
                    windStepY = (byte)-1;
                    break;
                case DIR_DOWN:
                    // 0658
                    windStepY = 1;
                    break;
                case DIR_RIGHT:
                    // 0660
                    windStepX = 1;
                    break;
                case DIR_LEFT:
                    // 0668
                    windStepX = (byte)-1;
                    break;
                }

                // 0619
                windScore_c = 1;
                if (D_29f6_windDx[D_5892_windDir - 1] != windStepX)
                {
                    windScore_c++;
                }
                // 062e
                if (D_29fa_windDy[D_5892_windDir - 1] != windStepY)
                {
                    windScore_c++;
                }
                // 0640
                if (windScore_c % 3 <= (uint)D_5883_sailCounter)
                {
                    D_5883_sailCounter = 0;
                    break;
                }
            }

            // 0670
            if (D_57bb_hmsCapePlans > 0x7f)
            {
                D_a524_capeToggle = (uint)(D_a524_capeToggle == 0);
                ULTIMA_4f7c_AdvanceTime(1);
            }
            else
            {
                ULTIMA_4f7c_AdvanceTime(2);
            }

            if (D_a524_capeToggle == 0 || D_57bb_hmsCapePlans < 0x80)
            {
                MAINOUT_1a60_MonstersTurn();
            }

            ULTIMA_3ae6_WaitFrames(1);
            D_5883_sailCounter = D_5883_sailCounter + 1;
        }

        if (D_5955_sailDir == 0)
        {
            key_h = ULTIMA_266c_GetChar();
        }
    }

    return key_h;
}

// the Attack command: a direction, then combat with what is there, if it can be fought (not on a skiff or carpet over water)
int MAINOUT_06ec_AttackCmd(void)
{
    int targetX_b;
    int targetY_b;
    int result_b = 0;
    int targetTile;

    ULTIMA_1850_PrintString(_TEXT(0x29fe, "Attack-"));
    if ((*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) < TILE_MAP_POISON) &&
        ((D_587c_partyTile & 0xfc) == TILE_ACTOR_SKIFF || (D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET))
    {
        ULTIMA_1850_PrintString(_TEXT(0x2a06, "On foot!\n"));
        result_b = 0;
    }
    else
    {
        if (ULTIMA_35ec_SelectDirection() != 0)
        {
            targetX_b = (uint)D_5896_mapX + D_5876_tempX;
            targetY_b = (uint)D_5897_mapY + D_5878_tempY;
            targetTile = ULTIMA_368e_FindActorTileAtPos(targetX_b, targetY_b, D_5895_mapLevel) & 0xfc;
            if (targetTile == TILE_ACTOR_PIRATE || (targetTile != TILE_ACTOR_SHARD && targetTile != TILE_ACTOR_E8 && targetTile >= TILE_ACTOR_WIZARD))
            {
                ULTIMA_6150_Combat(D_5876_tempX);
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x2a10, "Nothing to attack!\n"));
            }
        }
    }
    return result_b;
}

// prints what is entered, finds the dungeon at the party's position (D_1e8a/D_1eb2, entries 0x20-0x27), saves the actors and reads its DUNGEON.DAT level; Doom's entrance may be guarded
static int MAINOUT_0790_LoadDungeon(char* verb)
{
    int slot;
    int dungeonIndex_i;

    ULTIMA_1850_PrintString(verb);
    for (dungeonIndex_i = 0x20; dungeonIndex_i < 0x28; dungeonIndex_i++)
    {
        if (D_1e8a_placeX[dungeonIndex_i] == D_5896_mapX && D_1eb2_placeY[dungeonIndex_i] == D_5897_mapY)
            break;
    }

    if (dungeonIndex_i < 0x28)
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

        if (dungeonIndex_i == 0x27)
        {
            if ((D_58c8_shadowlordTowns[0] & D_58c8_shadowlordTowns[1] & D_58c8_shadowlordTowns[2]) < 0x80)
            {
                ULTIMA_1850_PrintString(_TEXT(0x2a2f, "\nAttacked at entrance!\n"));
                slot = ULTIMA_38e4_FindActorSlot();
                D_5c5a_actors[slot].tile = TILE_ACTOR_SHADOWLORD;
                ULTIMA_6150_Combat(slot);
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
            ULTIMA_1850_PrintString(D_1e3a_mapNames[dungeonIndex_i]);
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
        ULTIMA_256e_ReadFileFromDisk(/*0x2a53*/ "DUNGEON.DAT", D_595a_dungeonMap, 0x200, dungeonIndex_i * 0x200 + -0x4000);
        D_5893_mapId = dungeonIndex_i + 1;
        if (D_5895_mapLevel != 0 && (byte)(dungeonIndex_i + 1) != 0x28)
        {
            D_5895_mapLevel = 7;
            D_6603_dungeonFacing = 3;
            D_6602_dungeonMoveDir = 4;
            D_5896_mapX = D_5897_mapY = 7;
        }
        else
        {
            D_5895_mapLevel = 0;
            D_5896_mapX = D_5897_mapY = 1;
            D_6603_dungeonFacing = 1;
            D_6602_dungeonMoveDir = 5;
        }
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x2a5f, "\nWhat dungeon?\n"));
    }

    return 1;
}

// the Enter command: by the tile under the party, a shrine, a town, castle, keep, hut, lighthouse, dungeon, cave, mine or the palace of Blackthorn; 1 when it went in
int MAINOUT_08de_EnterCmd(void)
{
    int shrineIndex;
    int result_b;
    int tile;

    result_b = 1;

    ULTIMA_1850_PrintString(_TEXT(0x2a6f, "Enter "));

    tile = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    switch (tile)
    {
    case TILE_MAP_SHRINE:
        ULTIMA_1850_PrintString(_TEXT(0x2a76, "the shrine of\n"));
        for (shrineIndex = 0; shrineIndex < 8; shrineIndex++)
        {
            if (D_1f6e_shrineX[shrineIndex] == D_5896_mapX && D_1f76_shrineY[shrineIndex] == D_5897_mapY)
                break;
        }

        ULTIMA_1850_PrintString(D_1f4e_virtueNames[shrineIndex]);
        ULTIMA_16ba_PrintChar('\n');
        CAST2_0e76_Shrine();
        break;

    case TILE_MAP_HUT:
        result_b = OUTSUBS_0388_EnterTown(_TEXT(0x2a85, "hut"));
        break;

    case TILE_MAP_CODEX:
        ULTIMA_1850_PrintString(_TEXT(0x2a89, "the Shrine of the Codex!\n"));
        CAST2_0e76_Shrine();
        break;

    case TILE_MAP_KEEP:
        result_b = OUTSUBS_0388_EnterTown(_TEXT(0x2aa3, "keep"));
        break;

    case TILE_MAP_VILLAGE:
        result_b = OUTSUBS_0388_EnterTown(_TEXT(0x2aa8, "village"));
        break;

    case TILE_MAP_TOWNE:
        result_b = OUTSUBS_0388_EnterTown(_TEXT(0x2ab0, "towne"));
        break;

    case TILE_MAP_CASTLE:
        result_b = OUTSUBS_0388_EnterTown(_TEXT(0x2ab6, "castle"));
        break;

    case TILE_MAP_CAVE:
        result_b = MAINOUT_0790_LoadDungeon(_TEXT(0x2abd, "cave"));
        break;

    case TILE_MAP_MINE:
        result_b = MAINOUT_0790_LoadDungeon(_TEXT(0x2ac2, "mine"));
        break;

    case TILE_MAP_DUNGEON:
        result_b = MAINOUT_0790_LoadDungeon(_TEXT(0x2ac7, "dungeon"));
        break;

    case TILE_MAP_RUINS:
        ULTIMA_1850_PrintString(_TEXT(0x2acf, "ruins"));
        break;

    case TILE_MAP_LIGHTHOUSE:
        result_b = OUTSUBS_0388_EnterTown(_TEXT(0x2ad5, "lighthouse"));
        break;

    case TILE_MAP_PALACEBT:
        result_b = OUTSUBS_0388_EnterTown(_TEXT(0x2ae0, "the palace of Blackthorn!"));
        break;

    case TILE_MAP_CASTLELB:
        result_b = OUTSUBS_0388_EnterTown(_TEXT(0x2afa, "the Castle of Lord British!"));
        break;

    default:
        ULTIMA_1850_PrintString(_TEXT(0x2b16, "What?\n"));
        result_b = 0;
        break;
    }

    return result_b;
}

// on a blank (0xff) tile without the status 0xe the light level is 0, the view is redrawn once
static int MAINOUT_0a1a_UpdateDarkness(int wasDark)
{
    if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) == 0xff && D_587a_statusIcon != 0xe)
    {
        D_58a5_lightLevel = 0;
        if (wasDark == 0)
        {
            ULTIMA_5910_UpdateFrame();
            wasDark = 1;
        }
    }
    else
    {
        wasDark = 0;
        ULTIMA_4f7c_AdvanceTime(0);
    }

    return wasDark;
}

// underground: one in 256 turns "EARTHQUAKE!" damages the party
static void MAINOUT_0a60_CheckEarthquake(void)
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

// the overworld's command loop: sleeping, death, moongates, a key (Ctrl-K karma, E exit, V version, S sound, direction, commands), then the tile's effects and the monsters' turn
static void MAINOUT_0a84_MainLoop(void)
{
    int tileUnder;
    int unused1_h; // not used
    int unused2_g; // not used
    bool done_g;
    int turnTaken;
    int awake;
    byte key;
    int dark = 0;
    do
    {
        // 0a8f
        done_g = 0;
        turnTaken = 1;
        dark = MAINOUT_0a1a_UpdateDarkness(dark);
        awake = ULTIMA_39fc_GetFirstActivePartyMember();
        if (awake == 1)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a_PrintLeftCap();
            // 0aba
            ULTIMA_1850_PrintString(_TEXT(0x2b2a, "Zzzzzz...\n"));
        }
        else if (awake == -1) // 0ac2
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
            done_g = 1;
            turnTaken = 0;
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
            key = MAINOUT_0598_WaitForKey();
            if (key < 0x20)
            {
                switch (key & 0xff)
                {
                case U5_KEY_CTRL_K: // 0b34
                    ULTIMA_1a3e_PrintNumber(D_5888_karma, 1, 0x20);
                    // -> 0b75
                    ULTIMA_16ba_PrintChar('\n');
                    // -> 0af8
                    turnTaken = 0;
                    break; // -> 0c12

                case U5_KEY_CTRL_E: // 0b48
                    ULTIMA_1850_PrintString(_TEXT(0x2b3e, "Exit to DOS? "));
                    key = ULTIMA_266c_GetChar();
                    if (key == 'Y')
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
                    turnTaken = 0;
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
                    turnTaken = MAINOUT_0490_WalkInDirection(key, dark);
                    break;

                case 0: // 0af8
                    turnTaken = 0;
                    break;

                default: // 0bb8
                    ULTIMA_1850_PrintString(_TEXT(0x2b64, "What?\n"));
                    break;
                }
            }
            else
            {
                // 0bee
                if (key < 0x30 || key > 0x39)
                {
                    // 0bfa
                    turnTaken = ULTIMA_3178_ProcessCommand(key & 0xff);
                }
                else
                {
                    // 0c06
                    turnTaken = ULTIMA_4080_SetActivePlayer(key);
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
            done_g = 1;
        }
        else
        {
            if (turnTaken != 0)
            {
                // 0c39
                ULTIMA_4f7c_AdvanceTime(2);
                tileUnder = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
                if ((tileUnder & 0xfe) == TILE_MAP_6A)
                {
                    MAINOUT_1be8_BridgeTrolls();
                }
                // 0c64
                else if (tileUnder == TILE_MAP_POISON && D_587c_partyTile == TILE_ACTOR_AVATAR)
                {
                    OUTSUBS_05fc_StepOnPoisonTile();
                    ULTIMA_3ae6_WaitFrames(1);
                }
                else if (tileUnder == TILE_MAP_LAVA)
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
                MAINOUT_0a60_CheckEarthquake();
                ULTIMA_2ae8_EndTurn();
                if (tileUnder == 1 && ((D_587c_partyTile & 0xfc) == TILE_ACTOR_SKIFF || (D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET))
                {
                    ULTIMA_1850_PrintString(_TEXT(0x2bb3, "Rough seas!\n"));
                    ULTIMA_3522_HitEffectAt(D_5896_mapX, D_5897_mapY);
                    MAINOUT_109e_DamageVehicle();
                }
                if ((tileUnder & 0xfc) == TILE_MAP_WATERFALL)
                {
                    OUTSUBS_0458_Falls();
                }
                MAINOUT_1a60_MonstersTurn();
            }
        }
    } while (!done_g);
}

// the overlay's entry: sets up the overworld, puts a bought ship at its place, runs the main loop
void MAINOUT_0d22_Entry(void)
{
    int shipTile;
    int slot;
    ActorFmt* ship_m;

#if defined(ENABLE_BGM)
    AUDIO_StopBgm();
#endif

    MAINOUT_0000_SetUpOverworld();

    if (D_6605_boughtShip >= 0x40)
    {
        slot = ULTIMA_38e4_FindActorSlot();
        ship_m = &D_5c5a_actors[slot]; // get some actor
        ship_m->extra = D_6605_boughtShip & 0x3f;
        ship_m->x = D_5953_boughtShipX; // ship_x
        ship_m->y = D_5954_boughtShipY; // ship_y
        ship_m->z = 0;

        if (D_6605_boughtShip > 0x7f)
        {
            shipTile = TILE_ACTOR_FRIGATE_25;
        }
        else
        {
            shipTile = TILE_ACTOR_SKIFF_29;
        }

        ship_m->tile = ship_m->animTile = shipTile;
        ship_m->value = 99;
        D_6605_boughtShip = 0;
    }

#if defined(ENABLE_BGM)
    AUDIO_PlayBgmPerMap();
#endif

    MAINOUT_0a84_MainLoop();
}

// how likely a monster spawn is, 0 (towns, castles) to 5: by the tile under the party, + 3 at night; 3 in the underworld
int MAINOUT_0d8c_SpawnChance(void)
{
    int tile;
    int chance;

    if (D_5895_mapLevel > 0x7f)
    {
        return 3;
    }

    tile = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    if (tile >= TILE_MAP_20 && tile <= TILE_MAP_26)
    {
        chance = 0;
    }
    else
    {
        if (tile == TILE_MAP_POISON || (tile >= TILE_MAP_9 && tile <= TILE_MAP_F))
        {
            chance = 2;
        }
        else
        {
            chance = 1;
        }
    }

    if (D_587f_hour >= 0x20 || D_587f_hour < 5)
    {
        return chance + 3;
    }

    return chance;
}

// The original adds the index and the pointer as integers (mov bx,cx / add bx,si),
// not with the [bx][si] addressing mode that param_1[i] gives.
#if defined(TARGET_DOS16)
#define MAINOUT_0E04_AT(i) (*(byte*)((uint)(i) + (uint)weights))
#else
#define MAINOUT_0E04_AT(i) (weights[i])
#endif

// an index picked by weights that add up to 256 (a roll of 0-255 minus each weight)
static int MAINOUT_0e04_PickWeighted(byte* weights)
{
    int index;
    int roll_h;

    roll_h = ULTIMA_2092_RandomRange(0, 0xff);

    for (index = 0; MAINOUT_0E04_AT(index) <= roll_h; index++)
    {
        roll_h -= MAINOUT_0E04_AT(index);
    }
    return index;
}

// the monster (actor tile) that may appear on that terrain: sea, land or underworld tables, whirlpools in deep water; 0 for none
static byte MAINOUT_0e4e_MonsterForTile(int tile)
{
    int unused;

    if (tile < TILE_MAP_POISON || (tile >= TILE_MAP_60 && tile <= TILE_MAP_6F) || (tile >= TILE_MAP_WATERFALL && tile <= TILE_MAP_WATERFALL + 3) ||
        (tile >= TILE_MAP_WATER_E4 && tile <= TILE_MAP_WATER_E7))
    {
        if (ULTIMA_2092_RandomRange(0, 0x40) < 0x10)
        {
            if (D_5895_mapLevel < 0x80)
            {
                if (tile == 1 && ULTIMA_2092_RandomRange(0, 7) == 7)
                {
                    return TILE_ACTOR_WHIRLPOOL;
                }
                else
                {
                    return D_2bd4_seaMonsters[MAINOUT_0e04_PickWeighted(D_2bf0_seaWeights)];
                }
            }
            else
            {
                return D_2bda_deepSeaMonsters[MAINOUT_0e04_PickWeighted(D_2bf6_deepSeaWeights)];
            }
        }
    }
    else if (tile == TILE_MAP_7)
    {
        if (ULTIMA_2092_RandomRange(0, 3) == 0)
        {
            return TILE_ACTOR_E0;
        }
    }
    else
    {
        if (tile == TILE_MAP_POISON && D_5895_mapLevel == 0xff)
        {
            return TILE_ACTOR_F8;
        }

        if (tile == TILE_MAP_C || tile == TILE_MAP_D)
        {
            return 0;
        }

        if (tile < TILE_MAP_HUT || ((byte)tile & 0xfc) == TILE_MAP_30)
        {
            if (D_5895_mapLevel < 0x80)
            {
                return D_2bc0_landMonsters[MAINOUT_0e04_PickWeighted(D_2bdc_landWeights)];
            }

            return D_2bcc_deepLandMonsters[MAINOUT_0e04_PickWeighted(D_2be8_deepLandWeights)];
        }
    }

    return 0;
}

// a random cell of the 32x32 view more than six away from the party, into D_5876/D_5878
static void MAINOUT_0f4e_PickSpawnCell(void)
{
    do
    {
        D_5876_tempX = (byte)(ULTIMA_2092_RandomRange(0, 0x1f) + D_589b_mapOriginX);
        D_5878_tempY = (byte)(ULTIMA_2092_RandomRange(0, 0x1f) + D_589c_mapOriginY);
    } while (abs(D_5876_tempX - D_5896_mapX) <= 6 || abs(D_5878_tempY - D_5897_mapY) <= 6 ||
        abs(D_5876_tempX - D_5896_mapX) >= 0xfa || abs(D_5878_tempY - D_5897_mapY) >= 0xfa);
}

// looks up to 128 times for a cell and a monster that fit, and puts it there (pirates get hull value 100)
void MAINOUT_0fc4_SpawnMonster(void)
{
    int spawnY;
    int spawnX;
    int tries_j;
    byte monsterTile_k;

    for (tries_j = 0; tries_j < 0x80; tries_j++)
    {
        MAINOUT_0f4e_PickSpawnCell();

        spawnX = D_5876_tempX;
        spawnY = D_5878_tempY;

        monsterTile_k = MAINOUT_0e4e_MonsterForTile(*ULTIMA_4402_GetTileAddr(spawnX, spawnY));
        if (monsterTile_k != 0)
        {
            if (monsterTile_k != TILE_ACTOR_PIRATE || (*ULTIMA_4402_GetTileAddr(spawnX, spawnY) & 0xf0) != TILE_MAP_60)
            {
                break;
            }
        }
    }

    if (tries_j != 0x80)
    {
        tries_j = ULTIMA_38e4_FindActorSlot();
        ULTIMA_3a74_SetActor(monsterTile_k, monsterTile_k, spawnX, spawnY, D_5895_mapLevel, 0, tries_j);
        if (monsterTile_k == TILE_ACTOR_PIRATE)
        {
            D_5c5a_actors[tries_j].value = 100;
        }
    }
}

// whether an actor tile is one that moves by itself: 0x2c-0x2f and 0x80 up except 0xb4-0xb7 and 0xe8-0xeb
int MAINOUT_105c_IsMovingActor(int tile)
{
    if (tile >= 0x2c && tile <= 0x2f)
    {
        return 1;
    }
    
    if (tile < 0x80)
    {
        return 0;
    }

    if (tile >= 0xb4 && tile <= 0xb7 || tile >= 0xe8 && tile <= 0xeb)
    {
        return 0;
    }

    return 1;
}
