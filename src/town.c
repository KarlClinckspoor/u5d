#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <memory.h>

#include "audio/aud_mus.h"

#include "blckthrn.h"
#include "cast2.h"
#include "npc.h"
#include "talk.h"
#include "town.h"


char* D_2652_townFileNames[4] =
{
    /*0x2626*/ "TOWNE.DAT",
    /*0x2630*/ "DWELLING.DAT",
    /*0x263d*/ "CASTLE.DAT",
    /*0x2648*/ "KEEP.DAT"
};

// 1 when the NPC has been killed in this map (bit npc of the 4 bytes of D_5b5a_npcKilled for the map); only asked for the sandalwood box and the NPC types from TILE_ACTOR_WIZARD up.
// check npc killed flag
int TOWN_0000_CheckNpcKilled(int npc)
{
    if (D_659e_npcTypes[npc] == TILE_ACTOR_SANDALWOOD_BOX || D_659e_npcTypes[npc] >= TILE_ACTOR_WIZARD)
    {
        return ((*(u32*)&D_5b5a_npcKilled[(D_5893_mapId - 1) * 4]) & ((u32)1 << (byte)npc)) != 0;
    }

    return 0;
}

// Mark the NPC as killed in this map's bits (not guards, nor types 0x80 and up other than shards).
// 0052: set killed
void TOWN_0052_SetNpcKilled(int npc)
{
    int type;

    if (npc < 0 || npc > 0x1f)
        return;

    type = D_659e_npcTypes[npc] & 0xfc;

    if ((type != TILE_ACTOR_GUARD && type < TILE_ACTOR_80) || type == TILE_ACTOR_SHARD)
    {
        // 0084
        (*(u32*)&D_5b5a_npcKilled[(D_5893_mapId - 1) * 4]) |= ((u32)1 << (byte)npc);
    }
}

// Remove the NPC and its actor from the town: everything cleared, schedule types 0.
// despawn npc
void TOWN_00b0_DespawnNpc(int npc)
{
    ActorFmt* actor = &D_5c5a_actors[D_5f5e_npcs[npc].actorIdx];

    actor->tile = actor->animTile = actor->x = actor->y = actor->z = actor->animState = actor->extra = D_5f5e_npcs[npc].mode =
        D_5f5e_npcs[npc].x = D_5f5e_npcs[npc].y = D_5f5e_npcs[npc].z = D_5f5e_npcs[npc].dialogNumber = 0;
    D_5d5e_npcSchedules[npc].type[0] = D_5d5e_npcSchedules[npc].type[1] = D_5d5e_npcSchedules[npc].type[2] = 0;
    D_659e_npcTypes[npc] = 0;
    D_24e6_viewDirty |= 2;
}

// The NPC slot whose actor is actorIdx (alive and with a type); -1 if none.
int TOWN_011e_FindNpcByActor(int actorIdx)
{
    int npc;
    int found_e = -1;

    for (npc = 0; npc < 0x20; npc++)
    {
        if ((int)D_5f5e_npcs[npc].actorIdx == actorIdx && D_659e_npcTypes[npc] != 0 && D_5f5e_npcs[npc].mode != 0)
        {
            found_e = npc;
            break;
        }
    }

    return found_e;
}

// At night (and back at 5 o'clock): toggle the tile below each lamp (tile 0x87) and show the annotated tiles (0x48/0x49) only at 5, else water.
void TOWN_0170_SwapDayNightTiles(void)
{
    int y;
    int i;

    for (i = 0; i < 0x20; i++)
    {
        for (y = 0; y < 0x20; y++)
        {
            if (*ULTIMA_4402_GetTileAddr(i, y) == TILE_MAP_87)
            {
                *ULTIMA_4402_GetTileAddr(i, y + 1) ^= TILE_MAP_DD;
            }
        }
    }

    if ((*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) & 0xfe) != TILE_MAP_48)
    {
        for (i = 0; i < D_594e_annotCount; i++)
        {
            if (D_587f_hour == 5)
            {
                *ULTIMA_4402_GetTileAddr(D_58ee_annotX[i], D_590e_annotY[i]) = D_592e_annotTile[i];
            }
            else
            {
                *ULTIMA_4402_GetTileAddr(D_58ee_annotX[i], D_590e_annotY[i]) = TILE_MAP_WATER_3;
            }
        }
    }

    D_24e6_viewDirty |= 2;
}

// With a shadowlord in the town most trees become stumps and most crops are picked (fixed by the day through srand), then srand goes back to the time.
static void TOWN_0212_BlightLand(void)
{
    int y;
    int x;

    if (D_5958_townShadowlord != 0xff)
    {
        ULTIMA_207e_srand(D_587e_day);

        for (y = 0; y < 0x20; y++)
        {
            for (x = 0; x < 0x20; x++)
            {
                switch (GetMap(x, y))
                {
                case TILE_MAP_TREE:
                    if (ULTIMA_2092_RandomRange(0, 7) != 0)
                    {
                        GetMap(x, y) = TILE_MAP_STUMP;
                    }
                    break;
                case TILE_MAP_CROPS:
                    if (ULTIMA_2092_RandomRange(0, 7) != 0)
                    {
                        GetMap(x, y) = TILE_MAP_CROPS_PICKED;
                    }
                    break;
                }
            }
        }

        D_24e6_viewDirty |= 2;

        ULTIMA_207e_srand(ULTIMA_2056_GetTime());
    }
}

// Find which shadowlord (D_58c8_shadowlordTowns) haunts this map, blight the land, and create his actor and an NPC for him (schedule type 6) if not there.
// setup shadowlord
void TOWN_02ae_SetupShadowlord(void)
{
    int actorSlot;
    int i;
    ActorFmt* actor;
    int npcSlot;

    D_5958_townShadowlord = 0xff;
    if (D_5897_mapY != 4)
    {
        for (i = 0; i < 3; i++)
        {
            if (D_58c8_shadowlordTowns[i] == D_5893_mapId)
            {
                D_5958_townShadowlord = i;
                break;
            }
        }
    }

    if (D_5958_townShadowlord != 0xff)
    {
        // Setup shadow lord
        TOWN_0212_BlightLand();
        for (i = 0; i < 0x20; i++)
        {
            if (D_5c5a_actors[i].tile == TILE_ACTOR_SHADOWLORD)
            {
                return;
            }
        }

        actorSlot = ULTIMA_38e4_FindActorSlot();

        actor = &D_5c5a_actors[actorSlot];

        npcSlot = 0x1f;
        for (i = 0x1f; i >= 0; i--)
        {
            if (D_659e_npcTypes[i] == 0)
            {
                npcSlot = i;
                break;
            }
        }

        ASSERT(D_5893_mapId > 0 && D_5893_mapId <= 8);

        D_5f5e_npcs[npcSlot].mode = 1;
        D_5f5e_npcs[npcSlot].actorIdx = actorSlot;
        
        D_5f5e_npcs[npcSlot].x = actor->x = 0xf;
        D_5f5e_npcs[npcSlot].y = actor->y = D_13a6_shadowlordStartY[D_5893_mapId - 1];
        D_5f5e_npcs[npcSlot].z = actor->animState = actor->value = actor->z = actor->extra = 0;
        actor->tile = actor->animTile = TILE_ACTOR_SHADOWLORD;

        for (i = 0; i < 4; i++)
        {
            // npc time
            D_5d5e_npcSchedules[npcSlot].time[i] = 0;
        }

        for (i = 0; i < 3; i++)
        {
            // setup npc schedules
            D_5d5e_npcSchedules[npcSlot].type[i] = 6;
            D_5d5e_npcSchedules[npcSlot].x[i] = 0xf;
            D_5d5e_npcSchedules[npcSlot].y[i] = actor->y;
            D_5d5e_npcSchedules[npcSlot].z[i] = 0;
        }

        D_659e_npcTypes[npcSlot] = 0xfc;
    }
}

// Load the town map of this level from TOWNE/DWELLING/CASTLE/KEEP.DAT, list the annotated tiles and lighthouse beacons, apply the night tiles, build the light map; place the NPC actors when placeNpcs.
void TOWN_0408_LoadTownMap(int placeNpcs)
{
    int y;
    int mapRecord_l;
    int x;
    char* fileName;

    ULTIMA_4be8_ClearSky();
    ULTIMA_4a84_DrawSunAndMoons();
    ULTIMA_2e96_SetWindDirection(-1);

    D_594f_openDoorTile = 0;
    D_589b_mapOriginX = D_589c_mapOriginY = 0;

    fileName = D_2652_townFileNames[(D_5893_mapId - 1) >> 3];
    mapRecord_l = (uint)D_1e1a_townMapOffset[D_5893_mapId - 1] + (uint)D_5895_mapLevel;
    if (D_5895_mapLevel > 0x7f)
    {
        mapRecord_l -= 0x100;
    }

    ULTIMA_256e_ReadFileFromDisk(fileName, D_6608, 0x400, mapRecord_l << 10);

    D_594e_annotCount = 0;
    D_217e_beaconX = D_2180_beaconY = D_2182_beacon2X = D_2184_beacon2Y = -1;

    for (x = 0; x < 0x20; x++)
    {
        for (y = 0; y < 0x20; y++)
        {
            if ((*ULTIMA_4402_GetTileAddr(x, y) & 0xfe) == TILE_MAP_48)
            {
                D_58ee_annotX[D_594e_annotCount] = (char)x;
                D_590e_annotY[D_594e_annotCount] = (char)y;
                D_592e_annotTile[D_594e_annotCount] = *ULTIMA_4402_GetTileAddr(x, y);
                D_594e_annotCount++;
            }
            if (*ULTIMA_4402_GetTileAddr(x, y) == TILE_MAP_2A)
            {
                if (D_217e_beaconX == -1)
                {
                    D_217e_beaconX = x;
                    D_2180_beaconY = y;
                }
                else
                {
                    D_2182_beacon2X = x;
                    D_2184_beacon2Y = y;
                }
            }
        }
    }

    if (D_587f_hour < 5 || D_587f_hour > 0x13)
    {
        TOWN_0170_SwapDayNightTiles();
    }

    ULTIMA_5e4a_BuildLightMap();
    ULTIMA_4f7c_AdvanceTime(0);
    TOWN_0212_BlightLand();

    if (placeNpcs != 0)
    {
        TOWN_1694_PlaceActors();
    }

    D_24e6_viewDirty = 1;
}

// Stairs just stepped on: go up one level if the stairs face the way of the step, down if they face the other way, and reload the map.
static void TOWN_052e_TakeStairs(int facing, int tile)
{
    if ((tile & 0xfc) == TILE_MAP_STAIR)
    {
        ULTIMA_5910_UpdateFrame();
        if (tile - TILE_MAP_STAIR == facing)
        {
            D_5895_mapLevel++;
            ULTIMA_1850_PrintString(_TEXT(0x265a, "Up!\n"));
        }
        else if (tile - TILE_MAP_STAIR == (facing ^ 2))
        {
            D_5895_mapLevel--;
            ULTIMA_1850_PrintString(_TEXT(0x265f, "Down!\n"));
        }
        else
        {
            return;
        }

        TOWN_0408_LoadTownMap(1);
    }
}

// Choose the party's tile for the way it faces (0 up, 1 right, 2 down, 3 left), printing Ride/Fly/Row.
// select party icon
// 0: up, 1: right, 2: down, 3: left
static void TOWN_057c_SetPartyIcon(int facing)
{
    switch (D_587c_partyTile & 0xfc)
    {
    case TILE_ACTOR_HORSE:
        ULTIMA_1850_PrintString(_TEXT(0x2666, "Ride "));
        if (facing == 1)
        {
            D_587c_partyTile = TILE_ACTOR_RIDING_HORSE;
        }
        else if (facing == 3)
        {
            D_587c_partyTile = TILE_ACTOR_RIDING_HORSE + 1;
        }
        break;

    case TILE_ACTOR_FLYING_CARPET:
        ULTIMA_1850_PrintString(_TEXT(0x266c, "Fly "));
        if (facing == 1)
        {
            D_587c_partyTile = TILE_ACTOR_FLYING_CARPET;
        }
        else if (facing == 3)
        {
            D_587c_partyTile = TILE_ACTOR_FLYING_CARPET + 1;
        }
        break;

    case TILE_ACTOR_SKIFF:
        ULTIMA_1850_PrintString(_TEXT(0x2671, "Row "));
        // fall-through

    case TILE_ACTOR_FRIGATE_20:
    case TILE_ACTOR_FRIGATE_24:
        D_587c_partyTile = (char)facing + (D_587c_partyTile & 0xfc);
        break;
    }
}

// Move the party one step in a direction in a town: blocked, onto stairs, or off the map ("Dost thou wish to leave?"). Returns 1 when the party left the map.
// CHECKED
// move
// return: exit from the map?
bool TOWN_0600_MoveParty(int direction)
{
    int tile;
    int dy;
    int dx;
    int actorThere_j;
    bool atEdge; // reached the end of the map?
    bool free;
    int key_h;

    if (D_587c_partyTile == TILE_ACTOR_AVATAR || (D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
    {
        ULTIMA_433e_AudioFootstep();
    }

    dx = dy = atEdge = 0;

    switch (direction)
    {
    case DIR_UP:
        // 0648
        dy--;
        if (D_5897_mapY < 1)
            atEdge = 1;
        // 0657
        direction = 0;
        TOWN_057c_SetPartyIcon(0);
        ULTIMA_1850_PrintString(_TEXT(0x2676, "North\n"));
        break;

    case DIR_DOWN:
        // 06f8
        dy++;
        if (D_5897_mapY > 0x1e)
            atEdge = 1;
        direction = 2;
        TOWN_057c_SetPartyIcon(2);
        ULTIMA_1850_PrintString(_TEXT(0x267d, "South\n"));
        break;

    case DIR_RIGHT:
        // 071a
        dx++;
        if (0x1e < D_5896_mapX)
            atEdge = 1;
        direction = 1;
        TOWN_057c_SetPartyIcon(1);
        ULTIMA_1850_PrintString(_TEXT(0x2684, "East\n"));
        break;

    case DIR_LEFT:
        // 073c
        dx--;
        if (D_5896_mapX < 1)
            atEdge = 1;
        direction = 3;
        TOWN_057c_SetPartyIcon(3);
        ULTIMA_1850_PrintString(_TEXT(0x268a, "West\n"));
        break;
    }

    // 0669
    free = 1;
    tile = GetMapViewport(dx + 5, dy + 5); // 0xaba7
    actorThere_j = ULTIMA_368e_FindActorTileAtPos(dx + (uint)D_5896_mapX, dy + (uint)D_5897_mapY, D_5895_mapLevel);
    if (actorThere_j != 0)
    {
        // 06a9
        free = 0;
        if (D_587c_partyTile >= TILE_ACTOR_30 || D_587c_partyTile < TILE_ACTOR_FRIGATE_20)
        {
            // 06bf
            if (actorThere_j >= TILE_ACTOR_FRIGATE_24 && actorThere_j < TILE_ACTOR_PIRATE || actorThere_j == TILE_ACTOR_CARPET || ((byte)actorThere_j & 0xfe) == TILE_ACTOR_HORSE ||
                actorThere_j == TILE_ACTOR_SLEEP || actorThere_j == TILE_ACTOR_DEAD)
            {
                // -> 0771
                free = 1;
            }
        }
        else
        {
            // 075e
            if (D_587c_partyTile >= TILE_ACTOR_SKIFF && actorThere_j >= TILE_ACTOR_FRIGATE_24 && actorThere_j < TILE_ACTOR_SKIFF)
            {
                // -> 0771
                free = 1;
            }
        }
    }

    // 0776
    if (free && ULTIMA_2c4c_IsWalkableTile(D_587c_partyTile, tile) != 0)
    {
        // 0792
        if (atEdge != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2690, "\nDost thou wish to leave? "));

            // 079f NOT MATCHING
            do
            {
                key_h = ULTIMA_266c_GetChar();
            } while ((uint)key_h != 'Y' && (uint)key_h != 'N' && key_h != U5_KEY_ESC);

            // 07b6
            if (key_h == 'Y')
            {
                ULTIMA_1850_PrintString(_TEXT(0x26ab, "Yes\n\nExit to\n"));
                if (D_5893_mapId == 0x19)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x26b9, "Underworld!\n"));
                    D_5895_mapLevel = 0xff;
                }
                else
                {
                    // 07da
                    ULTIMA_1850_PrintString(_TEXT(0x26c6, "Britannia!\n"));
                    D_5895_mapLevel = 0;
                }

                // 07e6
                D_594f_openDoorTile = 0;
                D_5896_mapX = D_1e8a_placeX[D_5893_mapId - 1];
                D_5897_mapY = D_1eb2_placeY[D_5893_mapId - 1];
                D_5893_mapId = 0;
            }
            else
            {
                // 0806
                ULTIMA_1850_PrintString(_TEXT(0x26d2, "No\n"));
                atEdge = 0;
            }
        }
        else
        {
            // 0810
            D_5896_mapX += dx;
            D_5897_mapY += dy;
            D_24e6_viewDirty = 1;
            if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
            {
                ULTIMA_433e_AudioFootstep();
            }

            TOWN_052e_TakeStairs(direction, tile);
        }
    }
    else
    {
        // 083a
        ULTIMA_1850_PrintString(_TEXT(0x26d6, "Blocked!\n"));
        ULTIMA_22c0_AudioTone(0xa5, 200);
        ULTIMA_1b16_ClearKbdBuffer();
        atEdge = 0;
    }

    // 0854
    return atEdge;
}

// Make the NPC pursue the Avatar: schedule times 0, type 7 (from NPC type 0x2f up) or 6.
void TOWN_085e_SetPursue(int npc)
{
    int i;
    int type;

    if (D_659e_npcTypes[npc] >= 0x2f)
    {
        type = 7;
    }
    else
    {
        type = 6;
    }

    for (i = 0; i < 4; i++)
    {
        D_5d5e_npcSchedules[npc].time[i] = 0;
    }

    for (i = 0; i < 3; i++)
    {
        D_5d5e_npcSchedules[npc].type[i] = type;
    }
}

// Make a villager (NPC type 0x40-0x73) flee: schedule type 3 and dialog number 0xfd, when it keeps times or was set to 0xfe.
void TOWN_08d4_SetFlee(int npc)
{
    int hasTimes_c;
    int i;
    int unused_m; // unused, bucket 0: takes [bp-2]

    hasTimes_c = 0;

    for (i = 0; i < 4; i++)
    {
        if (D_5d5e_npcSchedules[npc].time[i] != 0)
        {
            hasTimes_c = 1;
        }
    }

    if (D_659e_npcTypes[npc] < 0x74 && D_659e_npcTypes[npc] >= 0x40 && (D_5f5e_npcs[npc].dialogNumber == 0xfe || hasTimes_c))
    {
        D_5f5e_npcs[npc].dialogNumber = 0xfd;

        for (i = 0; i < 3; i++)
        {
            D_5d5e_npcSchedules[npc].type[i] = 3;
        }
    }
}

// Raise the alarm: guards, shadowlords and the like pursue; other NPCs flee half the time.
// call guards?
void TOWN_0958_RaiseAlarm(void)
{
    int type;
    int npc;

    for (npc = 0; npc < 0x20; npc++)
    {
        if (D_5f5e_npcs[npc].mode != 0)
        {
            type = D_659e_npcTypes[npc];
            if (type == TILE_ACTOR_SHADOWLORD || type == TILE_ACTOR_D8 || type == TILE_ACTOR_GUARD)
            {
                TOWN_085e_SetPursue(npc);
            }
            else
            {
                if (ULTIMA_2092_RandomRange(0, 0xff) < 0x80)
                {
                    TOWN_08d4_SetFlee(npc);
                }
            }
        }
    }
}

// Fight an NPC: mark it killed, run the combat with its actor, remove it, rebuild the town.
// attack npc
void TOWN_09bc_FightNpc(int npc)
{
    TOWN_0052_SetNpcKilled(npc);
    ULTIMA_6150_Combat(D_5f5e_npcs[npc].actorIdx);
    TOWN_00b0_DespawnNpc(npc);
    TOWN_0408_LoadTownMap(0);
    TOWN_02ae_SetupShadowlord();
}

// A command: attack in a direction; breaks mirrors, strikes NPCs (karma -5, alarm), "Murdered!" in a bed; returns 1 when a turn passed.
int TOWN_09e6_AttackCmd(void)
{
    int actorTile_h;
    int actorIdx;
    int pitch;
    int y_p;
    int x;
    bool nothing;
    int turnUsed_h = 1;
    int npc;

    ULTIMA_1850_PrintString(_TEXT(0x26e0, "Attack-"));

    if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) < TILE_MAP_POISON && D_587c_partyTile != TILE_ACTOR_AVATAR)
    {
        ULTIMA_1850_PrintString(_TEXT(0x26e8, "On foot!\n"));
        turnUsed_h = 0;
    }
    else if (ULTIMA_35ec_SelectDirection() != 0)
    {
        x = (uint)D_5896_mapX + D_5876_tempX;
        y_p = (uint)D_5897_mapY + D_5878_tempY;
        if (*ULTIMA_4402_GetTileAddr(x, y_p) == TILE_MAP_MIRROR)
        {
            *ULTIMA_4402_GetTileAddr(x, y_p) = TILE_MAP_MIRROR_BROKEN;
            ULTIMA_1850_PrintString(_TEXT(0x26f2, "Broken!\n"));

            for (pitch = 2000; pitch < 20000; pitch += 1000)
            {
                ULTIMA_223c_AudioWhiteNoise(0x28, 0x78, pitch);
            }

            D_24e6_viewDirty |= 2;
        }
        else
        {
            actorTile_h = ULTIMA_368e_FindActorTileAtPos(x, y_p, D_5895_mapLevel);
            nothing = 1;
            if (actorTile_h != 0)
            {
                actorIdx = D_5876_tempX;
                npc = TOWN_011e_FindNpcByActor(actorIdx);
                if (actorTile_h >= TILE_ACTOR_WIZARD && (actorTile_h < TILE_ACTOR_E8 || actorTile_h >= TILE_ACTOR_F0) && (actorTile_h & 0xfc) != TILE_ACTOR_SHARD)
                {
                    nothing = 0;
                }
            }

            // 0ad8
            if (nothing)
            {
                // 0ae1
                ULTIMA_1850_PrintString(_TEXT(0x26fb, "Nothing to attack!\n"));
            }
            else
            {
                if (actorTile_h < 0x80)
                {
                    ULTIMA_3f36_DecreaseByte(&D_5888_karma, 5);
                    TOWN_0958_RaiseAlarm();
                }
                else if ((actorTile_h & 0xfc) == TILE_ACTOR_D8)
                {
                    TOWN_0958_RaiseAlarm();
                }

                switch (*ULTIMA_4402_GetTileAddr(x, y_p))
                {
                case TILE_MAP_84:
                case TILE_MAP_85:
                case TILE_MAP_MIRROR_BROKEN:
                case TILE_MAP_BED:
                    if (actorTile_h == TILE_ACTOR_BLACKTHORN)
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x270f, "Missed!\n"));
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x2718, "Murdered!\n"));
                        ULTIMA_3f36_DecreaseByte(&D_5888_karma, 5);
                        ULTIMA_3522_HitEffectAt(x, y_p);

                        if (npc >= 0)
                        {
                            TOWN_0052_SetNpcKilled(npc);
                            TOWN_00b0_DespawnNpc(npc);
                        }
                    }
                    break;

                default:
                    if (npc >= 0)
                    {
                        TOWN_0052_SetNpcKilled(npc);
                        TOWN_09bc_FightNpc(npc);
                    }
                    break;
                }

            }
        }
    }

    return turnUsed_h;
}

// K command: climb ladders and stairs, or the tiles 0x4c, 0xca, 0xcb a step away. Returns 1 when something happened.
// CHECKED
int TOWN_0b82_KlimbCmd(void)
{
    int tile;
    int done;

    done = 0;
    ULTIMA_1850_PrintString(_TEXT(0x2723, "Klimb-"));

    if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
    {
        // 0ba0..
        ULTIMA_1850_PrintString(_TEXT(0x272a, "-On foot!\n"));
    }
    else
    {
        // 0ba8
        switch (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY))
        {
        case TILE_MAP_LADDER_UP:
            // 0bce..0bd0
            TOWN_052e_TakeStairs(0, TILE_MAP_STAIR);
            done = 1;
            break;

        case TILE_MAP_86:
        case TILE_MAP_LADDER_DOWN:
            // 0c32..0bd0
            TOWN_052e_TakeStairs(2, TILE_MAP_STAIR);
            done = 1;
            break;
        }

        // 0bdd
        if (done == 0)
        {
            if (ULTIMA_35ec_SelectDirection() != 0)
            {
                tile = *ULTIMA_4402_GetTileAddr(D_5896_mapX + D_5876_tempX, D_5897_mapY + D_5878_tempY);
                if (tile == TILE_MAP_4C || tile == TILE_MAP_CA || tile == TILE_MAP_CB)
                {
                    // 0c19
                    D_5896_mapX += D_5876_tempX;
                    D_5897_mapY += D_5878_tempY;
                    D_24e6_viewDirty = 1;
                    ULTIMA_5910_UpdateFrame();
                    done = 1;
                }
                else
                {
                    // 0c38..0ba0
                    ULTIMA_1850_PrintString(_TEXT(0x2735, "What?\n"));
                }
            }
            else
            {
                // 0c3e: no direction given
                done = 1;
            }
        }
    }

    return done;
}

// 1 when the tile is 0xa2 or 0x43 (a loose horse does not wander next to them).
static int TOWN_0c4a_IsTileA2Or43(int x, int y)
{
    if (*ULTIMA_4402_GetTileAddr(x, y) == TILE_MAP_A2 ||
        *ULTIMA_4402_GetTileAddr(x, y) == TILE_MAP_43)
    {
        return 1;
    }

    return 0;
}

// The loose horses of the town (tile 0x10/0x11) move one step, half the time, unless next to tile 0xa2/0x43.
// CHECKED (loop, stack)
void TOWN_0c78_WanderHorses(void)
{
    int x; // x
    ActorFmt* actor_h;
    int y; // y
    int i_b; // i
    int step;
    int tile;

    for (i_b = 0; i_b < 0x20; i_b++)
    {
        actor_h = &D_5c5a_actors[i_b];

        tile = actor_h->tile;
        if (((byte)tile & 0xfe) != TILE_ACTOR_HORSE || actor_h->z != D_5895_mapLevel ||
            ULTIMA_2092_RandomRange(0, 1) != 0)
            continue;
        {
            x = actor_h->x;
            y = actor_h->y;

            if (TOWN_0c4a_IsTileA2Or43(x, y + 1) == 0 &&
                TOWN_0c4a_IsTileA2Or43(x + 1, y) == 0 &&
                TOWN_0c4a_IsTileA2Or43(x, y - 1) == 0 &&
                TOWN_0c4a_IsTileA2Or43(x - 1, y) == 0)
            {
                // 0d12
                if (ULTIMA_2092_RandomRange(0, 1) != 0)
                {
                    step = ULTIMA_2092_RandomRange(0, 1) * 2 - 1;
                    x += step;
                    if (step > 0)
                    {
                        tile = TILE_ACTOR_HORSE;
                        // nop
                    }
                    else
                    {
                        tile = TILE_ACTOR_HORSE + 1;
                    }
                }
                else
                {
                    y += ULTIMA_2092_RandomRange(0, 1) * 2 - 1;
                }

                if (x <= 0x1f && y <= 0x1f && 0 <= x && 0 <= y)
                {
                    if ((ULTIMA_2c4c_IsWalkableTile(TILE_ACTOR_HORSE, *ULTIMA_4402_GetTileAddr(x, y)) != 0) &&
                        (ULTIMA_3702_FindLastActorTileAt(x, y, D_5895_mapLevel) == 0))
                    {
                        actor_h->tile = actor_h->animTile = tile;
                        actor_h->x = (byte)x;
                        actor_h->y = (byte)y;
                        D_24e6_viewDirty |= 2;
                    }
                }
            }
        }
    }
}

u8 D_2742_drunkDirs[4] = { 0x03, 0x04, 0x02, 0x01 };

u16 D_2746_harpsichordFreq[0xa] = {
    0x1eab, 0x0c2c, 0x0da9, 0x0f56,
    0x103f, 0x123c, 0x1478, 0x16fa,
    0x1857, 0x1b53
};

u8 D_275a_secretTune[0xd] = {
    0x06, 0x07, 0x08, 0x09, 0x08, 0x07, 0x08, 0x07,
    0x06, 0x07, 0x06, 0x05, 0x03
};

u8 D_2767_tuneStep = 0;





// Read a key in a town (with the screen update and prompt when asked); a drunk party staggers: "Hic!" and a random direction key.
uint TOWN_0dc4_GetTownKey(int prompt)
{
    uint key;

    if (prompt != 0)
    {
        ULTIMA_5910_UpdateFrame(); // update_map?
        if (D_a9fa_vitalsDirty != 0)
        {
            ULTIMA_2900_UpdateVitalsDisplay();
            D_a9fa_vitalsDirty = 0;
        }
        ULTIMA_16ba_PrintChar('\n');
        ULTIMA_4c2a_PrintLeftCap();
    }

    key = ULTIMA_266c_GetChar();

    if (D_5957_drunkTurns != 0)
    {
        if (ULTIMA_2092_RandomRange(0, 1) != 0)
        {
            TOWN_0958_RaiseAlarm();
            D_5957_drunkTurns--;
            ULTIMA_1850_PrintString(_TEXT(0x273c, "Hic!\n"));
            key = D_2742_drunkDirs[ULTIMA_2092_RandomRange(0, 3)];
        }
    }

    return key;
}

// A digit key: next to the instrument tile (0x8d in front) it plays note 0-9; the tune D_275a_secretTune on map 0x11, level 2, opens the way (tile at 0x11,0xd) and shakes the screen. Else it selects the active player.
int TOWN_0e34_NumberKey(int key)
{
    int result;

    result = 3;
    if (GetMapViewport(5, 6) == 0x8d) // abc7
    {
        key -= 0x30;
        if (D_a9ce_soundOn != 0)
        {
            ULTIMA_2192_AudioPulse(D_2746_harpsichordFreq[key], 1, 4000, 20000, -4);
        }
        if (D_275a_secretTune[D_2767_tuneStep] == key)
        {
            D_2767_tuneStep++;
            if (D_2767_tuneStep == 0xd)
            {
                D_2767_tuneStep = 0;
                if (D_5893_mapId == 0x11 && D_5895_mapLevel == 0x02)
                {
                    GetMap(0x11, 0xd) ^= 0xb; // 67b9
                    ULTIMA_3072_ShakeScreen();
                    D_24e6_viewDirty = 1;
                }
            }
        }
        else if (D_2767_tuneStep == 0xa && key == 8)
        {
            D_2767_tuneStep = 3;
        }
        else if (D_2767_tuneStep == 0xb && key == 7)
        {
            D_2767_tuneStep = 2;
        }
        else if (key == D_275a_secretTune[0])
        {
            D_2767_tuneStep = 1;
        }
        else
        {
            D_2767_tuneStep = 0;
        }
    }
    else
    {
        result = ULTIMA_4080_SetActivePlayer(key);
    }

    return result;
}

// TOWN_0f02_TileEffects's pass-2 memory fill (see its note) has names of its own: F_xxxx, each with the
// type and the name length of the D_xxxx it stood for when the window was measured. They are
// declared here and nowhere defined: the object lists them as externals that nothing refers
// to (no fixup), so the link needs no definition. Renaming a global therefore does not move
// the window, unless town.c itself uses that global: then the length of the names town.c
// references changes (tools/rename.py reports it per module), and the same number of
// characters must come off (or go onto) these names. Do not rename them otherwise.
#if defined(TARGET_DOS16)
extern u16 F_13ae_redColor;
extern u16 F_13b0_brightWhiteColor;
extern u16 F_13b2_blueColor;
extern u16 F_13b4_greenColor;
extern u16 F_13b6_magentaColor;
extern u16 F_13b8_brightYellowColor;
extern u16 F_13ba_lightGrayColor;
extern u16 F_153c_enemyFlags[0x30];
extern u8 F_159c[0x30];
extern u8 F_15cc[0x30];
extern u8 F_15fc[0x38];
extern u8 F_1634[0x30];
extern u8 F_1664[0x38];
extern u8 F_169c[0x38];
extern u8 F_16d4[0x30];
extern u8 F_1704[0x10];
extern u8 F_1714[0x10];
extern u8 F_1724[8];
extern u8 F_172c[8];
extern u8 F_1734[8];
extern u8 F_173c[8];
extern u8 F_1744[8];
extern s16 F_1a58[9];
extern u16 F_1a74[5];
extern u8 F_1a7e[0x30];
extern u8 F_1aae[0x30];
extern u8 F_1ade[9];
extern u8 F_1ae8[0x30];
extern u8 F_1b18[0xb0 + 0x33];
extern u8 F_1c90[0x30];
extern u8 F_1cc0[0x30];
extern u16 F_1cf0[0x15];
extern u8 F_1d1a[0x100];
extern u8 F_1eda[28 * 2];
extern u8 F_1f12[0x30];
extern u8 F_1f42[6];
extern u8 F_1f48[6];
extern u8 F_1f6e[8];
extern u8 F_1f76[8];
extern u8 F_2186;
extern s8 F_2188[0x32];
extern u8 F_21ba[0x10];
extern u8 F_244a[8];
extern u8 F_2452[6];
extern u8 F_2458[6];
extern u8 F_245e[6];
extern u8 F_2464[6];
extern u8 F_246a[6];
extern u8 F_2470[6];
extern u8 F_2476[0x10];
extern u8 F_2486[0x10];
extern u8 F_2496[0x10];
extern u8 F_24a6[0x10];
extern u8 F_24b6[0x10];
extern u8 F_24c6[0x10];
extern s16 F_24d6[4];
extern s16 F_24de[4];
extern u8 F_24e7;
extern u8 F_29f6[4];
extern u8 F_29fa[4];
extern u8 F_2bc0[0xc];
extern u8 F_2bcc[0x8];
extern u8 F_2bd4[0x6];
extern u8 F_2bda[0x2];
extern u8 F_2bdc[0xc];
extern u8 F_2be8[0x8];
extern u8 F_2bf0[0x6];
extern u8 F_2bf6[0x2];
extern u8 F_2c18[0x3d];
extern u8 F_2c55;
extern u8 F_2c57;
extern u8 F_2c76[6];
extern u8 F_2de8[8];
extern u8 F_2df0[8];
extern u8 F_2df8[12];
extern u8 F_2e04[12];
extern u8 F_2e26[4];
extern s16 F_2e2a[4];
extern s16 F_2e42[4];
extern s16 F_2e4a[4];
extern s16 F_2e52[4];
extern s16 F_2e5a[4];
extern s16 F_2e72[4];
extern u8 F_2e8a[6];
extern u8 F_2e90[10];
extern u8 F_2e9a[10];
extern u8 F_2f16[8];
extern u8 F_2f1e[8];
extern u8 F_2f26;
extern s8 F_2f28[4];
extern s8 F_2f2c[4];
extern u16 F_3016[0x15];
extern u8 F_3040[0x16];
extern u8 F_3056[0x16];
extern u8 F_306c[0x16];
extern u8 F_3082[0x16];
extern u8 F_3098[0x16];
extern u8 F_30ae[0x16];
extern u8 F_30c4[0x16];
extern u8 F_30da[0x16];
extern u8 F_30f0[0x15];
extern u16 F_3304;
extern u16 F_3656[8];
extern u8 F_367e[0x20];
extern u8 F_369e[0x3c];
extern u8 F_36da[0x27];
extern u8 F_3702[12];
extern u8 F_370e[8];
extern u8 F_3716[10];
extern u16 F_3720[6];
extern u16 F_372c[6];
extern u16 F_3738[6];
extern s16 F_3744[6];
extern u8 F_3750[8];
extern u8 F_3758[8];
extern u8 F_3760[0xb0];
extern u8 F_3822[16];
extern u8 F_3832[8];
extern u8 F_383a[6];
extern u8 F_3840;
extern u8 F_3842[0xd];
extern u8 F_3850[0xd];
extern u8 F_385e[8];
extern u8 F_3866[8];
extern u8 F_386e[8];
extern u8 F_3876[0x100];
extern u8 F_3a06[4];
extern u8 F_3a0a[4];
extern u8 F_3a0e[3];
extern u16 F_3a26[6];
extern u16 F_3a82[0x30];
extern u16 F_3c0a[3];
extern u16 F_3c10[8];
extern u16 F_3c30[4];
extern s16 F_3c38[4];
extern s16 F_3c40[4];
extern s16 F_3c48[0x2f];
extern u16 F_3cbe[8];
extern char F_3d5a[];
extern u8 F_3d86[8];
extern u8 F_3d8e[8];
extern u16 F_3d96[8];
extern u16 F_3dca[6];
extern u8 F_3dd6[6];
extern u8 F_3ddc[6];
extern u8 F_3de2[6];
extern u8 F_3de8[6];
extern u8 F_3dee[6];
extern u8 F_3df4[6];
extern u8 F_3dfa[6];
extern u8 F_3e00[6];
extern u8 F_3e06[6];
extern u8 F_3e5a[6];
extern u8 F_3e60[6];
extern u8 F_3e66[4];
extern u8 F_3e6a[4];
extern u8 F_3e6e[4];
extern u8 F_3e78[0x72];
extern u8 F_3eea[0x72];
extern u8 F_3f5c[0x72];
extern u8 F_3fce[0x72];
extern u8 F_4040[0x72];
extern u8 F_40b2[0x72];
extern u8 F_4124[8];
extern u8 F_412c[8];
extern u8 F_4134[8];
extern u8 F_413c[0x30];
extern u8 F_416c[0x30];
extern u8 F_41bc[8];
extern u8 F_41c4[8];
extern u8 F_41cc[8];
extern u8 F_4512[8];
extern u8 F_4592[4];
extern u8 F_4596[4];
extern u8 F_4882[4];
extern u8 F_4886[4];
extern u8 F_488a[4];
extern u8 F_488e[4];
extern u8 F_4892[3];
extern u8 F_4aee;
extern u8 F_4aef;
extern u8 F_4af0;
extern u8 F_4af1;
extern u8 F_4af2;
extern u8 F_4af3;
extern u8 F_4af4;
extern u8 F_4af5;
extern u16 F_4af6[9];
extern u16 F_4b08[9];
extern u16 F_4b1a[9];
extern u16 F_4b2c[9];
extern u16 F_4b5e[8];
extern u16 F_4b6e[8];
extern u8 F_4b7e[8];
extern u8 F_4b86[8];
extern u8 F_4b8e[8];
extern u8 F_4b96[8];
extern u8 F_4b9e[0x24];
extern u8 F_4bc2[0x24];
extern u16 F_4be6[7];
extern u16 F_4bf4[7];
extern u16 F_4c02[7];
extern u16 F_4c10[7];
extern char F_4c1e[5];
extern char F_4c24[5];
extern char F_4c2a[5];
extern char F_4c30[5];
extern u16 F_4c36[9];
extern u16 F_4c48[6];
extern u16 F_4c54[9];
extern u16 F_4c66[7];
extern u8  F_4cdc[26];
extern u16 F_4d10[0x1a];
extern u16 F_4d44[4];
extern u8  F_4d4c[0xa];
extern u16 F_4d56[4];
extern u16 F_4d5e[4];
extern u16 F_4d66[4];
extern u16 F_4d6e[4];
extern u8  F_4d76[4];
extern u8  F_4d7a[4];
extern u8  F_4d7e[6];
extern u8 F_4dc4[6];
extern u16 F_4e6e[6];
extern u8 F_4e7a[6];
extern u8 F_4e80[6];
extern u8 F_50ea[0x5b];
extern u16 F_5146[2];
extern u16 F_514c[2];
extern u16 F_5150;
extern u16 F_5152;
extern u16 F_5154;
extern u16 F_5156;
extern u16 F_5158;
extern u16 F_515a;
extern u8 F_5164[8];
extern u8 F_516c[8];
extern u8 F_5174[8];
extern u8 F_51fc[8];
extern u8 F_5204[8];
extern u16 F_520c;
extern int F_52c4;
extern int F_52c8;
extern int F_52da_pen_color;
extern int F_52ef_forceEga;
extern int F_52f1_forceTandy;
extern int F_52f3_forceHerc;
extern int F_52f6;
extern int F_52f8;
extern int F_5306[7];
extern int F_5314[7];
#endif

// Effects of the tile the party stands on each turn: sleepers may wake, a trapdoor (0x8c) drops the party a level (killing it on map 0x1d), swamp poison, fire (0xbc, 0x8f); then the turn ends.
// CLEAN ROOM (written from the disassembly). Party status effects of the tile stood on:
// sleepers wake, trapdoor, poison (swamp), fire.
// The original optimizes the first two party loops (pointers in SI/DI) but not the poison
// loop (index recomputed each pass as i * 32 in SI, the for's plain layout): MSC 5.1's
// pass 2 had run out of memory for loop optimization by then. Every global symbol a module
// references costs pass 2 memory (more for longer names), and the original's names are not
// known, so the DOS build makes up the difference with 245 dead references ("if (0)") to
// names town.c does not otherwise use (the F_xxxx above): pass 1 drops the code, the symbols stay. The
// window is narrow, and moves with the names town.c uses: it was 266 exactly before the
// shared globals got their readable names, and is 248 to 251 with them (MATCHING.md); with TOWN's readable names 245 (the last 6 of the 251 left out: 5 to 7 match). The memory stays used for the rest of the
// object, which is why the object ends here: the functions from TOWN_10da_ShoutBegone are town2.c.
// Dead lines cost nothing (pass 1 drops their line records), only the symbols do, so the fill is
// written four to a line. The original had about 7.5 KB more pass-2 memory in use between
// TOWN_0c78 and here than u5d (a uniform surplus for the whole module breaks TOWN_0C78 first);
// what it was is not known (MATCHING.md, "What pass 2 counts, measured").
void TOWN_0f02_TileEffects(void)
{
    int tile;
    int savedIcon;
    int i;
    int again;

#if defined(TARGET_DOS16)
    // pass 2 memory fill: see the note above the function
    if (0)
    {
        F_13ae_redColor = 0; F_13b0_brightWhiteColor = 0; F_13b2_blueColor = 0; F_13b4_greenColor = 0;
        F_13b6_magentaColor = 0; F_13b8_brightYellowColor = 0; F_13ba_lightGrayColor = 0; F_153c_enemyFlags[0] = 0;
        F_159c[0] = 0; F_15cc[0] = 0; F_15fc[0] = 0; F_1634[0] = 0;
        F_1664[0] = 0; F_169c[0] = 0; F_16d4[0] = 0; F_1704[0] = 0;
        F_1714[0] = 0; F_1724[0] = 0; F_172c[0] = 0; F_1734[0] = 0;
        F_173c[0] = 0; F_1744[0] = 0; F_1a58[0] = 0; F_1a74[0] = 0;
        F_1a7e[0] = 0; F_1aae[0] = 0; F_1ade[0] = 0; F_1ae8[0] = 0;
        F_1b18[0] = 0; F_1c90[0] = 0; F_1cc0[0] = 0; F_1cf0[0] = 0;
        F_1d1a[0] = 0; F_1eda[0] = 0; F_1f12[0] = 0; F_1f42[0] = 0;
        F_1f48[0] = 0; F_1f6e[0] = 0; F_1f76[0] = 0; F_2186 = 0;
        F_2188[0] = 0; F_21ba[0] = 0; F_244a[0] = 0; F_2452[0] = 0;
        F_2458[0] = 0; F_245e[0] = 0; F_2464[0] = 0; F_246a[0] = 0;
        F_2470[0] = 0; F_2476[0] = 0; F_2486[0] = 0; F_2496[0] = 0;
        F_24a6[0] = 0; F_24b6[0] = 0; F_24c6[0] = 0; F_24d6[0] = 0;
        F_24de[0] = 0; F_24e7 = 0; F_29f6[0] = 0; F_29fa[0] = 0;
        F_2bc0[0] = 0; F_2bcc[0] = 0; F_2bd4[0] = 0; F_2bda[0] = 0;
        F_2bdc[0] = 0; F_2be8[0] = 0; F_2bf0[0] = 0; F_2bf6[0] = 0;
        F_2c18[0] = 0; F_2c55 = 0; F_2c57 = 0; F_2c76[0] = 0;
        F_2de8[0] = 0; F_2df0[0] = 0; F_2df8[0] = 0; F_2e04[0] = 0;
        F_2e26[0] = 0; F_2e2a[0] = 0; F_2e42[0] = 0; F_2e4a[0] = 0;
        F_2e52[0] = 0; F_2e5a[0] = 0; F_2e72[0] = 0; F_2e8a[0] = 0;
        F_2e90[0] = 0; F_2e9a[0] = 0; F_2f16[0] = 0; F_2f1e[0] = 0;
        F_2f26 = 0; F_2f28[0] = 0; F_2f2c[0] = 0; F_3016[0] = 0;
        F_3040[0] = 0; F_3056[0] = 0; F_306c[0] = 0; F_3082[0] = 0;
        F_3098[0] = 0; F_30ae[0] = 0; F_30c4[0] = 0; F_30da[0] = 0;
        F_30f0[0] = 0; F_3304 = 0; F_3656[0] = 0; F_367e[0] = 0;
        F_369e[0] = 0; F_36da[0] = 0; F_3702[0] = 0; F_370e[0] = 0;
        F_3716[0] = 0; F_3720[0] = 0; F_372c[0] = 0; F_3738[0] = 0;
        F_3744[0] = 0; F_3750[0] = 0; F_3758[0] = 0; F_3760[0] = 0;
        F_3822[0] = 0; F_3832[0] = 0; F_383a[0] = 0; F_3840 = 0;
        F_3842[0] = 0; F_3850[0] = 0; F_385e[0] = 0; F_3866[0] = 0;
        F_386e[0] = 0; F_3876[0] = 0; F_3a06[0] = 0; F_3a0a[0] = 0;
        F_3a0e[0] = 0; F_3a26[0] = 0; F_3a82[0] = 0; F_3c0a[0] = 0;
        F_3c10[0] = 0; F_3c30[0] = 0; F_3c38[0] = 0; F_3c40[0] = 0;
        F_3c48[0] = 0; F_3cbe[0] = 0; F_3d5a[0] = 0; F_3d86[0] = 0;
        F_3d8e[0] = 0; F_3d96[0] = 0; F_3dca[0] = 0; F_3dd6[0] = 0;
        F_3ddc[0] = 0; F_3de2[0] = 0; F_3de8[0] = 0; F_3dee[0] = 0;
        F_3df4[0] = 0; F_3dfa[0] = 0; F_3e00[0] = 0; F_3e06[0] = 0;
        F_3e5a[0] = 0; F_3e60[0] = 0; F_3e66[0] = 0; F_3e6a[0] = 0;
        F_3e6e[0] = 0; F_3e78[0] = 0; F_3eea[0] = 0; F_3f5c[0] = 0;
        F_3fce[0] = 0; F_4040[0] = 0; F_40b2[0] = 0; F_4124[0] = 0;
        F_412c[0] = 0; F_4134[0] = 0; F_413c[0] = 0; F_416c[0] = 0;
        F_41bc[0] = 0; F_41c4[0] = 0; F_41cc[0] = 0; F_4512[0] = 0;
        F_4592[0] = 0; F_4596[0] = 0; F_4882[0] = 0; F_4886[0] = 0;
        F_488a[0] = 0; F_488e[0] = 0; F_4892[0] = 0; F_4aee = 0;
        F_4aef = 0; F_4af0 = 0; F_4af1 = 0; F_4af2 = 0;
        F_4af3 = 0; F_4af4 = 0; F_4af5 = 0; F_4af6[0] = 0;
        F_4b08[0] = 0; F_4b1a[0] = 0; F_4b2c[0] = 0; F_4b5e[0] = 0;
        F_4b6e[0] = 0; F_4b7e[0] = 0; F_4b86[0] = 0; F_4b8e[0] = 0;
        F_4b96[0] = 0; F_4b9e[0] = 0; F_4bc2[0] = 0; F_4be6[0] = 0;
        F_4bf4[0] = 0; F_4c02[0] = 0; F_4c10[0] = 0; F_4c1e[0] = 0;
        F_4c24[0] = 0; F_4c2a[0] = 0; F_4c30[0] = 0; F_4c36[0] = 0;
        F_4c48[0] = 0; F_4c54[0] = 0; F_4c66[0] = 0; F_4cdc[0] = 0;
        F_4d10[0] = 0; F_4d44[0] = 0; F_4d4c[0] = 0; F_4d56[0] = 0;
        F_4d5e[0] = 0; F_4d66[0] = 0; F_4d6e[0] = 0; F_4d76[0] = 0;
        F_4d7a[0] = 0; F_4d7e[0] = 0; F_4dc4[0] = 0; F_4e6e[0] = 0;
        F_4e7a[0] = 0; F_4e80[0] = 0; F_50ea[0] = 0; F_5146[0] = 0;
        F_514c[0] = 0; F_5150 = 0; F_5152 = 0; F_5154 = 0;
        F_5156 = 0; F_5158 = 0; F_515a = 0; F_5164[0] = 0;
        F_516c[0] = 0; F_5174[0] = 0; F_51fc[0] = 0; F_5204[0] = 0;
        F_520c = 0; F_52c4 = 0; F_52c8 = 0; F_52da_pen_color = 0;
        F_52ef_forceEga = 0;
    }
#endif
    for (i = 0; i < (uint)D_585b_partyCount; i++)
    {
        if (D_55a8_party[i].status == 'S' && ULTIMA_2092_RandomRange(0, 0xf) == 0xf)
        {
            D_55a8_party[i].status = 'G';
        }
    }

    do
    {
        again = 0;
        tile = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
        if (tile == 0x8c && (D_587c_partyTile & 0xfe) != 0x14)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2768, "A TRAPDOOR!\n"));
            savedIcon = D_587c_partyTile;
            D_587c_partyTile = 0;
            ULTIMA_5910_UpdateFrame();
            ULTIMA_2aa8_DamageParty();
            D_587c_partyTile = savedIcon;
            if (D_5893_mapId == 0x1d)
            {
                ULTIMA_0a70_GRAP_2d_SetPenColor(0);
                ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
                for (i = 1000; i > 250; i--)
                {
                    ULTIMA_22e2_PcspkOn(i);
                    ULTIMA_20c8_Delay(1, 0x28);
                }
                ULTIMA_230e_PcspkOff();
                memset(D_6608, 0x8f, 0x400);
                D_24e6_viewDirty = 1;
                memset(D_5c5a_actors, 0, 0x100);
                for (i = 0; i < (uint)D_585b_partyCount; i++)
                {
                    D_55a8_party[i].hp = 0;
                    D_55a8_party[i].status = 'D';
                    ULTIMA_223c_AudioWhiteNoise(0x28, 0xbb8, 0x1f4);
                    ULTIMA_2900_UpdateVitalsDisplay();
                }
            }
            else
            {
                D_5895_mapLevel--;
                TOWN_0408_LoadTownMap(1);
                again = 1;
            }
        }
        else if (tile == 4 && D_587c_partyTile == 0x1c)
        {
            for (i = 0; i < (uint)D_585b_partyCount; i++)
            {
                if (D_55a8_party[i].status != 'D' && D_55a8_party[i].status != 'P' &&
                    D_55a8_party[i].dex < (uint)ULTIMA_2092_RandomRange(0, 0x1d))
                {
                    ULTIMA_1850_PrintString(_TEXT(0x2775, "Poisoned!\n"));
                    D_55a8_party[i].status = 'P';
                    ULTIMA_2900_UpdateVitalsDisplay();
                }
            }
        }
        else if (tile == 0xbc || tile == 0x8f)
        {
            ULTIMA_5910_UpdateFrame();
            ULTIMA_1850_PrintString(_TEXT(0x2780, "Burning!\n"));
            ULTIMA_2aa8_DamageParty();
        }
    } while (again != 0);

    ULTIMA_2ae8_EndTurn();
}
