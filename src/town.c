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


char* D_2652[4] =
{
    /*0x2626*/ "TOWNE.DAT",
    /*0x2630*/ "DWELLING.DAT",
    /*0x263d*/ "CASTLE.DAT",
    /*0x2648*/ "KEEP.DAT"
};

// check npc killed flag
int TOWN_0000_CheckNpcKilled(int param_1)
{
    if (D_659e[param_1] == TILE_ACTOR_SANDALWOOD_BOX || D_659e[param_1] >= TILE_ACTOR_WIZARD)
    {
        return ((*(u32*)&D_5b5a[(D_5893_map_id - 1) * 4]) & ((u32)1 << (byte)param_1)) != 0;
    }

    return 0;
}

// 0052: set killed
void TOWN_0052_SetNpcKilled(int param_1)
{
    int local_4;

    if (param_1 < 0 || param_1 > 0x1f)
        return;

    local_4 = D_659e[param_1] & 0xfc;

    if ((local_4 != TILE_ACTOR_GUARD && local_4 < TILE_ACTOR_80) || local_4 == TILE_ACTOR_SHARD)
    {
        // 0084
        (*(u32*)&D_5b5a[(D_5893_map_id - 1) * 4]) |= ((u32)1 << (byte)param_1);
    }
}

// despawn npc
void TOWN_00b0_DespawnNpc(int param_1)
{
    ActorFmt* actor = &D_5c5a[D_5f5e[param_1]._c];

    actor->_0_tile = actor->_1_animTile = actor->_2_x = actor->_3_y = actor->_4_z = actor->_6 = actor->_7 = D_5f5e[param_1]._0 =
        D_5f5e[param_1]._2_x = D_5f5e[param_1]._4_y = D_5f5e[param_1]._6_z = D_5f5e[param_1]._a = 0;
    D_5d5e[param_1].type[0] = D_5d5e[param_1].type[1] = D_5d5e[param_1].type[2] = 0;
    D_659e[param_1] = 0;
    D_24e6 |= 2;
}

int TOWN_011e(int param_1)
{
    int local_4;
    int local_2 = -1;

    for (local_4 = 0; local_4 < 0x20; local_4++)
    {
        if ((int)D_5f5e[local_4]._c == param_1 && D_659e[local_4] != 0 && D_5f5e[local_4]._0 != 0)
        {
            local_2 = local_4;
            break;
        }
    }

    return local_2;
}

void TOWN_0170(void)
{
    int local_6_an;
    int local_4;

    for (local_4 = 0; local_4 < 0x20; local_4++)
    {
        for (local_6_an = 0; local_6_an < 0x20; local_6_an++)
        {
            if (*ULTIMA_4402_GetTileAddr(local_4, local_6_an) == TILE_MAP_87)
            {
                *ULTIMA_4402_GetTileAddr(local_4, local_6_an + 1) ^= TILE_MAP_DD;
            }
        }
    }

    if ((*ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y) & 0xfe) != TILE_MAP_48)
    {
        for (local_4 = 0; local_4 < D_594e; local_4++)
        {
            if (D_587f == 5)
            {
                *ULTIMA_4402_GetTileAddr(D_58ee[local_4], D_590e[local_4]) = D_592e[local_4];
            }
            else
            {
                *ULTIMA_4402_GetTileAddr(D_58ee[local_4], D_590e[local_4]) = TILE_MAP_WATER_3;
            }
        }
    }

    D_24e6 |= 2;
}

static void TOWN_0212(void)
{
    int local_6_an;
    int local_4;

    if (D_5958 != 0xff)
    {
        ULTIMA_207e_srand(D_587e);

        for (local_6_an = 0; local_6_an < 0x20; local_6_an++)
        {
            for (local_4 = 0; local_4 < 0x20; local_4++)
            {
                switch (GetMap(local_4, local_6_an))
                {
                case TILE_MAP_TREE:
                    if (ULTIMA_2092_RandomRange(0, 7) != 0)
                    {
                        GetMap(local_4, local_6_an) = TILE_MAP_STUMP;
                    }
                    break;
                case TILE_MAP_CROPS:
                    if (ULTIMA_2092_RandomRange(0, 7) != 0)
                    {
                        GetMap(local_4, local_6_an) = TILE_MAP_CROPS_PICKED;
                    }
                    break;
                }
            }
        }

        D_24e6 |= 2;

        ULTIMA_207e_srand(ULTIMA_2056_GetTime());
    }
}

// setup shadowlord
void TOWN_02ae(void)
{
    int local_a;
    int local_8;
    ActorFmt* local_6;
    int local_4_ab;

    D_5958 = 0xff;
    if (D_5897_map_y != 4)
    {
        for (local_8 = 0; local_8 < 3; local_8++)
        {
            if (D_58c8[local_8] == D_5893_map_id)
            {
                D_5958 = local_8;
                break;
            }
        }
    }

    if (D_5958 != 0xff)
    {
        // Setup shadow lord
        TOWN_0212();
        for (local_8 = 0; local_8 < 0x20; local_8++)
        {
            if (D_5c5a[local_8]._0_tile == TILE_ACTOR_SHADOWLORD)
            {
                return;
            }
        }

        local_a = ULTIMA_38e4();

        local_6 = &D_5c5a[local_a];

        local_4_ab = 0x1f;
        for (local_8 = 0x1f; local_8 >= 0; local_8--)
        {
            if (D_659e[local_8] == 0)
            {
                local_4_ab = local_8;
                break;
            }
        }

        ASSERT(D_5893_map_id > 0 && D_5893_map_id <= 8);

        D_5f5e[local_4_ab]._0 = 1;
        D_5f5e[local_4_ab]._c = local_a;
        
        D_5f5e[local_4_ab]._2_x = local_6->_2_x = 0xf;
        D_5f5e[local_4_ab]._4_y = local_6->_3_y = D_13a6[D_5893_map_id - 1];
        D_5f5e[local_4_ab]._6_z = local_6->_6 = local_6->_5 = local_6->_4_z = local_6->_7 = 0;
        local_6->_0_tile = local_6->_1_animTile = TILE_ACTOR_SHADOWLORD;

        for (local_8 = 0; local_8 < 4; local_8++)
        {
            // npc time
            D_5d5e[local_4_ab].time[local_8] = 0;
        }

        for (local_8 = 0; local_8 < 3; local_8++)
        {
            // setup npc schedules
            D_5d5e[local_4_ab].type[local_8] = 6;
            D_5d5e[local_4_ab].x[local_8] = 0xf;
            D_5d5e[local_4_ab].y[local_8] = local_6->_3_y;
            D_5d5e[local_4_ab].z[local_8] = 0;
        }

        D_659e[local_4_ab] = 0xfc;
    }
}

void TOWN_0408(int param_1)
{
    int local_8_al;
    int local_6_an;
    int local_4;
    char* local_2;

    ULTIMA_4be8();
    ULTIMA_4a84();
    ULTIMA_2e96_SetWindDirection(-1);

    D_594f = 0;
    D_589b = D_589c = 0;

    local_2 = D_2652[(D_5893_map_id - 1) >> 3];
    local_6_an = (uint)D_1e1a[D_5893_map_id - 1] + (uint)D_5895_map_level;
    if (D_5895_map_level > 0x7f)
    {
        local_6_an -= 0x100;
    }

    ULTIMA_256e_ReadFileFromDisk(local_2, D_6608, 0x400, local_6_an << 10);

    D_594e = 0;
    D_217e = D_2180 = D_2182 = D_2184 = -1;

    for (local_4 = 0; local_4 < 0x20; local_4++)
    {
        for (local_8_al = 0; local_8_al < 0x20; local_8_al++)
        {
            if ((*ULTIMA_4402_GetTileAddr(local_4, local_8_al) & 0xfe) == TILE_MAP_48)
            {
                D_58ee[D_594e] = (char)local_4;
                D_590e[D_594e] = (char)local_8_al;
                D_592e[D_594e] = *ULTIMA_4402_GetTileAddr(local_4, local_8_al);
                D_594e++;
            }
            if (*ULTIMA_4402_GetTileAddr(local_4, local_8_al) == TILE_MAP_2A)
            {
                if (D_217e == -1)
                {
                    D_217e = local_4;
                    D_2180 = local_8_al;
                }
                else
                {
                    D_2182 = local_4;
                    D_2184 = local_8_al;
                }
            }
        }
    }

    if (D_587f < 5 || D_587f > 0x13)
    {
        TOWN_0170();
    }

    ULTIMA_5e4a();
    ULTIMA_4f7c(0);
    TOWN_0212();

    if (param_1 != 0)
    {
        TOWN_1694();
    }

    D_24e6 = 1;
}

static void TOWN_052e(int param_1, int param_2)
{
    if ((param_2 & 0xfc) == TILE_MAP_STAIR)
    {
        ULTIMA_5910_UpdateFrame();
        if (param_2 - TILE_MAP_STAIR == param_1)
        {
            D_5895_map_level++;
            ULTIMA_1850_PrintString(_TEXT(0x265a, "Up!\n"));
        }
        else if (param_2 - TILE_MAP_STAIR == (param_1 ^ 2))
        {
            D_5895_map_level--;
            ULTIMA_1850_PrintString(_TEXT(0x265f, "Down!\n"));
        }
        else
        {
            return;
        }

        TOWN_0408(1);
    }
}

// select party icon
// 0: up, 1: right, 2: down, 3: left
static void TOWN_057c(int param_1)
{
    switch (D_587c_partyTile & 0xfc)
    {
    case TILE_ACTOR_HORSE:
        ULTIMA_1850_PrintString(_TEXT(0x2666, "Ride "));
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
        ULTIMA_1850_PrintString(_TEXT(0x266c, "Fly "));
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
        ULTIMA_1850_PrintString(_TEXT(0x2671, "Row "));
        // fall-through

    case TILE_ACTOR_FRIGATE_20:
    case TILE_ACTOR_FRIGATE_24:
        D_587c_partyTile = (char)param_1 + (D_587c_partyTile & 0xfc);
        break;
    }
}

// CHECKED
// move
// return: exit from the map?
bool TOWN_0600(int param_1)
{
    int local_10_ad;
    int local_e;
    int local_c;
    int local_a;
    bool local_8; // reached the end of the map?
    bool local_6;
    int local_4_ab;

    if (D_587c_partyTile == TILE_ACTOR_AVATAR || (D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
    {
        ULTIMA_433e_AudioFootstep();
    }

    local_c = local_e = local_8 = 0;

    switch (param_1)
    {
    case DIR_UP:
        // 0648
        local_e--;
        if (D_5897_map_y < 1)
            local_8 = 1;
        // 0657
        param_1 = 0;
        TOWN_057c(0);
        ULTIMA_1850_PrintString(_TEXT(0x2676, "North\n"));
        break;

    case DIR_DOWN:
        // 06f8
        local_e++;
        if (D_5897_map_y > 0x1e)
            local_8 = 1;
        param_1 = 2;
        TOWN_057c(2);
        ULTIMA_1850_PrintString(_TEXT(0x267d, "South\n"));
        break;

    case DIR_RIGHT:
        // 071a
        local_c++;
        if (0x1e < D_5896_map_x)
            local_8 = 1;
        param_1 = 1;
        TOWN_057c(1);
        ULTIMA_1850_PrintString(_TEXT(0x2684, "East\n"));
        break;

    case DIR_LEFT:
        // 073c
        local_c--;
        if (D_5896_map_x < 1)
            local_8 = 1;
        param_1 = 3;
        TOWN_057c(3);
        ULTIMA_1850_PrintString(_TEXT(0x268a, "West\n"));
        break;
    }

    // 0669
    local_6 = 1;
    local_10_ad = GetMapViewport(local_c + 5, local_e + 5); // 0xaba7
    local_a = ULTIMA_368e_FindActorTileAtPos(local_c + (uint)D_5896_map_x, local_e + (uint)D_5897_map_y, D_5895_map_level);
    if (local_a != 0)
    {
        // 06a9
        local_6 = 0;
        if (D_587c_partyTile >= TILE_ACTOR_30 || D_587c_partyTile < TILE_ACTOR_FRIGATE_20)
        {
            // 06bf
            if (local_a >= TILE_ACTOR_FRIGATE_24 && local_a < TILE_ACTOR_PIRATE || local_a == TILE_ACTOR_CARPET || ((byte)local_a & 0xfe) == TILE_ACTOR_HORSE ||
                local_a == TILE_ACTOR_SLEEP || local_a == TILE_ACTOR_DEAD)
            {
                // -> 0771
                local_6 = 1;
            }
        }
        else
        {
            // 075e
            if (D_587c_partyTile >= TILE_ACTOR_SKIFF && local_a >= TILE_ACTOR_FRIGATE_24 && local_a < TILE_ACTOR_SKIFF)
            {
                // -> 0771
                local_6 = 1;
            }
        }
    }

    // 0776
    if (local_6 && ULTIMA_2c4c_IsWalkableTile(D_587c_partyTile, local_10_ad) != 0)
    {
        // 0792
        if (local_8 != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2690, "\nDost thou wish to leave? "));

            // 079f NOT MATCHING
            do
            {
                local_4_ab = ULTIMA_266c_GetChar();
            } while ((uint)local_4_ab != 'Y' && (uint)local_4_ab != 'N' && local_4_ab != U5_KEY_ESC);

            // 07b6
            if (local_4_ab == 'Y')
            {
                ULTIMA_1850_PrintString(_TEXT(0x26ab, "Yes\n\nExit to\n"));
                if (D_5893_map_id == 0x19)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x26b9, "Underworld!\n"));
                    D_5895_map_level = 0xff;
                }
                else
                {
                    // 07da
                    ULTIMA_1850_PrintString(_TEXT(0x26c6, "Britannia!\n"));
                    D_5895_map_level = 0;
                }

                // 07e6
                D_594f = 0;
                D_5896_map_x = D_1e8a[D_5893_map_id - 1];
                D_5897_map_y = D_1eb2[D_5893_map_id - 1];
                D_5893_map_id = 0;
            }
            else
            {
                // 0806
                ULTIMA_1850_PrintString(_TEXT(0x26d2, "No\n"));
                local_8 = 0;
            }
        }
        else
        {
            // 0810
            D_5896_map_x += local_c;
            D_5897_map_y += local_e;
            D_24e6 = 1;
            if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
            {
                ULTIMA_433e_AudioFootstep();
            }

            TOWN_052e(param_1, local_10_ad);
        }
    }
    else
    {
        // 083a
        ULTIMA_1850_PrintString(_TEXT(0x26d6, "Blocked!\n"));
        ULTIMA_22c0_AudioTone(0xa5, 200);
        ULTIMA_1b16_ClearKbdBuffer();
        local_8 = 0;
    }

    // 0854
    return local_8;
}

void TOWN_085e(int param_1)
{
    int i_ae;
    int local_4;

    if (D_659e[param_1] >= 0x2f)
    {
        local_4 = 7;
    }
    else
    {
        local_4 = 6;
    }

    for (i_ae = 0; i_ae < 4; i_ae++)
    {
        D_5d5e[param_1].time[i_ae] = 0;
    }

    for (i_ae = 0; i_ae < 3; i_ae++)
    {
        D_5d5e[param_1].type[i_ae] = local_4;
    }
}

void TOWN_08d4(int param_1)
{
    int local_6;
    int local_8;
    int unused_m; // unused, bucket 0: takes [bp-2]

    local_6 = 0;

    for (local_8 = 0; local_8 < 4; local_8++)
    {
        if (D_5d5e[param_1].time[local_8] != 0)
        {
            local_6 = 1;
        }
    }

    if (D_659e[param_1] < 0x74 && D_659e[param_1] >= 0x40 && (D_5f5e[param_1]._a == 0xfe || local_6))
    {
        D_5f5e[param_1]._a = 0xfd;

        for (local_8 = 0; local_8 < 3; local_8++)
        {
            D_5d5e[param_1].type[local_8] = 3;
        }
    }
}

// call guards?
void TOWN_0958(void)
{
    int local_6_an;
    int local_4;

    for (local_4 = 0; local_4 < 0x20; local_4++)
    {
        if (D_5f5e[local_4]._0 != 0)
        {
            local_6_an = D_659e[local_4];
            if (local_6_an == TILE_ACTOR_SHADOWLORD || local_6_an == TILE_ACTOR_D8 || local_6_an == TILE_ACTOR_GUARD)
            {
                TOWN_085e(local_4);
            }
            else
            {
                if (ULTIMA_2092_RandomRange(0, 0xff) < 0x80)
                {
                    TOWN_08d4(local_4);
                }
            }
        }
    }
}

// attack npc
void TOWN_09bc(int param_1)
{
    TOWN_0052_SetNpcKilled(param_1);
    ULTIMA_6150_Combat(D_5f5e[param_1]._c);
    TOWN_00b0_DespawnNpc(param_1);
    TOWN_0408(0);
    TOWN_02ae();
}

int TOWN_09e6_AttackCmd(void)
{
    int local_12_ab;
    int local_10_ad;
    int local_e;
    int local_c;
    int local_a;
    bool local_8;
    int local_6 = 1;
    int local_4_ab;

    ULTIMA_1850_PrintString(_TEXT(0x26e0, "Attack-"));

    if (*ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y) < TILE_MAP_POISON && D_587c_partyTile != TILE_ACTOR_AVATAR)
    {
        ULTIMA_1850_PrintString(_TEXT(0x26e8, "On foot!\n"));
        local_6 = 0;
    }
    else if (ULTIMA_35ec_SelectDirection() != 0)
    {
        local_a = (uint)D_5896_map_x + D_5876;
        local_c = (uint)D_5897_map_y + D_5878;
        if (*ULTIMA_4402_GetTileAddr(local_a, local_c) == TILE_MAP_MIRROR)
        {
            *ULTIMA_4402_GetTileAddr(local_a, local_c) = TILE_MAP_MIRROR_BROKEN;
            ULTIMA_1850_PrintString(_TEXT(0x26f2, "Broken!\n"));

            for (local_e = 2000; local_e < 20000; local_e += 1000)
            {
                ULTIMA_223c_AudioWhiteNoise(0x28, 0x78, local_e);
            }

            D_24e6 |= 2;
        }
        else
        {
            local_12_ab = ULTIMA_368e_FindActorTileAtPos(local_a, local_c, D_5895_map_level);
            local_8 = 1;
            if (local_12_ab != 0)
            {
                local_10_ad = D_5876;
                local_4_ab = TOWN_011e(local_10_ad);
                if (local_12_ab >= TILE_ACTOR_WIZARD && (local_12_ab < TILE_ACTOR_E8 || local_12_ab >= TILE_ACTOR_F0) && (local_12_ab & 0xfc) != TILE_ACTOR_SHARD)
                {
                    local_8 = 0;
                }
            }

            // 0ad8
            if (local_8)
            {
                // 0ae1
                ULTIMA_1850_PrintString(_TEXT(0x26fb, "Nothing to attack!\n"));
            }
            else
            {
                if (local_12_ab < 0x80)
                {
                    ULTIMA_3f36_DecreaseByte(&D_5888, 5);
                    TOWN_0958();
                }
                else if ((local_12_ab & 0xfc) == TILE_ACTOR_D8)
                {
                    TOWN_0958();
                }

                switch (*ULTIMA_4402_GetTileAddr(local_a, local_c))
                {
                case TILE_MAP_84:
                case TILE_MAP_85:
                case TILE_MAP_MIRROR_BROKEN:
                case TILE_MAP_BED:
                    if (local_12_ab == TILE_ACTOR_BLACKTHORN)
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x270f, "Missed!\n"));
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x2718, "Murdered!\n"));
                        ULTIMA_3f36_DecreaseByte(&D_5888, 5);
                        ULTIMA_3522(local_a, local_c);

                        if (local_4_ab >= 0)
                        {
                            TOWN_0052_SetNpcKilled(local_4_ab);
                            TOWN_00b0_DespawnNpc(local_4_ab);
                        }
                    }
                    break;

                default:
                    if (local_4_ab >= 0)
                    {
                        TOWN_0052_SetNpcKilled(local_4_ab);
                        TOWN_09bc(local_4_ab);
                    }
                    break;
                }

            }
        }
    }

    return local_6;
}

// CHECKED
int TOWN_0b82_KlimbCmd(void)
{
    int local_6_an;
    int local_4;

    local_4 = 0;
    ULTIMA_1850_PrintString(_TEXT(0x2723, "Klimb-"));

    if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
    {
        // 0ba0..
        ULTIMA_1850_PrintString(_TEXT(0x272a, "-On foot!\n"));
    }
    else
    {
        // 0ba8
        switch (*ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y))
        {
        case TILE_MAP_LADDER_UP:
            // 0bce..0bd0
            TOWN_052e(0, TILE_MAP_STAIR);
            local_4 = 1;
            break;

        case TILE_MAP_86:
        case TILE_MAP_LADDER_DOWN:
            // 0c32..0bd0
            TOWN_052e(2, TILE_MAP_STAIR);
            local_4 = 1;
            break;
        }

        // 0bdd
        if (local_4 == 0)
        {
            if (ULTIMA_35ec_SelectDirection() != 0)
            {
                local_6_an = *ULTIMA_4402_GetTileAddr(D_5896_map_x + D_5876, D_5897_map_y + D_5878);
                if (local_6_an == TILE_MAP_4C || local_6_an == TILE_MAP_CA || local_6_an == TILE_MAP_CB)
                {
                    // 0c19
                    D_5896_map_x += D_5876;
                    D_5897_map_y += D_5878;
                    D_24e6 = 1;
                    ULTIMA_5910_UpdateFrame();
                    local_4 = 1;
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
                local_4 = 1;
            }
        }
    }

    return local_4;
}

static int TOWN_0c4a(int param_1, int param_2)
{
    if (*ULTIMA_4402_GetTileAddr(param_1, param_2) == TILE_MAP_A2 ||
        *ULTIMA_4402_GetTileAddr(param_1, param_2) == TILE_MAP_43)
    {
        return 1;
    }

    return 0;
}

// CHECKED (loop, stack)
void TOWN_0c78(void)
{
    int local_6; // x
    ActorFmt* local_4_c;
    int local_8; // y
    int local_a; // i
    int local_c;
    int local_e;

    for (local_a = 0; local_a < 0x20; local_a++)
    {
        local_4_c = &D_5c5a[local_a];

        local_e = local_4_c->_0_tile;
        if (((byte)local_e & 0xfe) != TILE_ACTOR_HORSE || local_4_c->_4_z != D_5895_map_level ||
            ULTIMA_2092_RandomRange(0, 1) != 0)
            continue;
        {
            local_6 = local_4_c->_2_x;
            local_8 = local_4_c->_3_y;

            if (TOWN_0c4a(local_6, local_8 + 1) == 0 &&
                TOWN_0c4a(local_6 + 1, local_8) == 0 &&
                TOWN_0c4a(local_6, local_8 - 1) == 0 &&
                TOWN_0c4a(local_6 - 1, local_8) == 0)
            {
                // 0d12
                if (ULTIMA_2092_RandomRange(0, 1) != 0)
                {
                    local_c = ULTIMA_2092_RandomRange(0, 1) * 2 - 1;
                    local_6 += local_c;
                    if (local_c > 0)
                    {
                        local_e = TILE_ACTOR_HORSE;
                        // nop
                    }
                    else
                    {
                        local_e = TILE_ACTOR_HORSE + 1;
                    }
                }
                else
                {
                    local_8 += ULTIMA_2092_RandomRange(0, 1) * 2 - 1;
                }

                if (local_6 <= 0x1f && local_8 <= 0x1f && 0 <= local_6 && 0 <= local_8)
                {
                    if ((ULTIMA_2c4c_IsWalkableTile(TILE_ACTOR_HORSE, *ULTIMA_4402_GetTileAddr(local_6, local_8)) != 0) &&
                        (ULTIMA_3702(local_6, local_8, D_5895_map_level) == 0))
                    {
                        local_4_c->_0_tile = local_4_c->_1_animTile = local_e;
                        local_4_c->_2_x = (byte)local_6;
                        local_4_c->_3_y = (byte)local_8;
                        D_24e6 |= 2;
                    }
                }
            }
        }
    }
}

u8 D_2742[4] = { 0x03, 0x04, 0x02, 0x01 };

u16 D_2746[0xa] = {
    0x1eab, 0x0c2c, 0x0da9, 0x0f56,
    0x103f, 0x123c, 0x1478, 0x16fa,
    0x1857, 0x1b53
};

u8 D_275a[0xd] = {
    0x06, 0x07, 0x08, 0x09, 0x08, 0x07, 0x08, 0x07,
    0x06, 0x07, 0x06, 0x05, 0x03
};

u8 D_2767 = 0;





uint TOWN_0dc4(int param_1)
{
    uint local_4;

    if (param_1 != 0)
    {
        ULTIMA_5910_UpdateFrame(); // update_map?
        if (D_a9fa != 0)
        {
            ULTIMA_2900_UpdateVitalsDisplay();
            D_a9fa = 0;
        }
        ULTIMA_16ba_PrintChar('\n');
        ULTIMA_4c2a();
    }

    local_4 = ULTIMA_266c_GetChar();

    if (D_5957 != 0)
    {
        if (ULTIMA_2092_RandomRange(0, 1) != 0)
        {
            TOWN_0958();
            D_5957--;
            ULTIMA_1850_PrintString(_TEXT(0x273c, "Hic!\n"));
            local_4 = D_2742[ULTIMA_2092_RandomRange(0, 3)];
        }
    }

    return local_4;
}

int TOWN_0e34(int param_1)
{
    int local_4;

    local_4 = 3;
    if (GetMapViewport(5, 6) == 0x8d) // abc7
    {
        param_1 -= 0x30;
        if (D_a9ce != 0)
        {
            ULTIMA_2192_AudioPulse(D_2746[param_1], 1, 4000, 20000, -4);
        }
        if (D_275a[D_2767] == param_1)
        {
            D_2767++;
            if (D_2767 == 0xd)
            {
                D_2767 = 0;
                if (D_5893_map_id == 0x11 && D_5895_map_level == 0x02)
                {
                    GetMap(0x11, 0xd) ^= 0xb; // 67b9
                    ULTIMA_3072_ShakeScreen();
                    D_24e6 = 1;
                }
            }
        }
        else if (D_2767 == 0xa && param_1 == 8)
        {
            D_2767 = 3;
        }
        else if (D_2767 == 0xb && param_1 == 7)
        {
            D_2767 = 2;
        }
        else if (param_1 == D_275a[0])
        {
            D_2767 = 1;
        }
        else
        {
            D_2767 = 0;
        }
    }
    else
    {
        local_4 = ULTIMA_4080(param_1);
    }

    return local_4;
}

// CLEAN ROOM (written from the disassembly). Party status effects of the tile stood on:
// sleepers wake, trapdoor, poison (swamp), fire.
// The original optimizes the first two party loops (pointers in SI/DI) but not the poison
// loop (index recomputed each pass as i * 32 in SI, the for's plain layout): MSC 5.1's
// pass 2 had run out of memory for loop optimization by then. Every global symbol a module
// references costs pass 2 memory (more for longer names), and the original's names are not
// known, so the DOS build makes up the difference with 266 dead references ("if (0)") to
// variables town.c does not otherwise use: pass 1 drops the code, the symbols stay. The
// window is exact (265 and 267 give other code). The memory stays used for the rest of the
// object, which is why the object ends here: the functions from TOWN_10da are town2.c.
void TOWN_0f02(void)
{
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

#if defined(TARGET_DOS16)
    // pass 2 memory fill: see the note above the function
    if (0)
    {
        D_13ae_redColor = 0;
        D_13b0_brightWhiteColor = 0;
        D_13b2_blueColor = 0;
        D_13b4_greenColor = 0;
        D_13b6_magentaColor = 0;
        D_13b8_brightYellowColor = 0;
        D_13ba_lightGrayColor = 0;
        D_153c_enemyFlags[0] = 0;
        D_159c[0] = 0;
        D_15cc[0] = 0;
        D_15fc[0] = 0;
        D_1634[0] = 0;
        D_1664[0] = 0;
        D_169c[0] = 0;
        D_16d4[0] = 0;
        D_1704[0] = 0;
        D_1714[0] = 0;
        D_1724[0] = 0;
        D_172c[0] = 0;
        D_1734[0] = 0;
        D_173c[0] = 0;
        D_1744[0] = 0;
        D_1a58[0] = 0;
        D_1a74[0] = 0;
        D_1a7e[0] = 0;
        D_1aae[0] = 0;
        D_1ade[0] = 0;
        D_1ae8[0] = 0;
        D_1b18[0] = 0;
        D_1c90[0] = 0;
        D_1cc0[0] = 0;
        D_1cf0[0] = 0;
        D_1d1a[0] = 0;
        D_1eda[0] = 0;
        D_1f12[0] = 0;
        D_1f42[0] = 0;
        D_1f48[0] = 0;
        D_1f6e[0] = 0;
        D_1f76[0] = 0;
        D_2186 = 0;
        D_2188[0] = 0;
        D_21ba[0] = 0;
        D_244a[0] = 0;
        D_2452[0] = 0;
        D_2458[0] = 0;
        D_245e[0] = 0;
        D_2464[0] = 0;
        D_246a[0] = 0;
        D_2470[0] = 0;
        D_2476[0] = 0;
        D_2486[0] = 0;
        D_2496[0] = 0;
        D_24a6[0] = 0;
        D_24b6[0] = 0;
        D_24c6[0] = 0;
        D_24d6[0] = 0;
        D_24de[0] = 0;
        D_24e7 = 0;
        D_29f6[0] = 0;
        D_29fa[0] = 0;
        D_2bc0[0] = 0;
        D_2bcc[0] = 0;
        D_2bd4[0] = 0;
        D_2bda[0] = 0;
        D_2bdc[0] = 0;
        D_2be8[0] = 0;
        D_2bf0[0] = 0;
        D_2bf6[0] = 0;
        D_2c18[0] = 0;
        D_2c55 = 0;
        D_2c57 = 0;
        D_2c76[0] = 0;
        D_2de8[0] = 0;
        D_2df0[0] = 0;
        D_2df8[0] = 0;
        D_2e04[0] = 0;
        D_2e26[0] = 0;
        D_2e2a[0] = 0;
        D_2e42[0] = 0;
        D_2e4a[0] = 0;
        D_2e52[0] = 0;
        D_2e5a[0] = 0;
        D_2e72[0] = 0;
        D_2e8a[0] = 0;
        D_2e90[0] = 0;
        D_2e9a[0] = 0;
        D_2f16[0] = 0;
        D_2f1e[0] = 0;
        D_2f26 = 0;
        D_2f28[0] = 0;
        D_2f2c[0] = 0;
        D_3016[0] = 0;
        D_3040[0] = 0;
        D_3056[0] = 0;
        D_306c[0] = 0;
        D_3082[0] = 0;
        D_3098[0] = 0;
        D_30ae[0] = 0;
        D_30c4[0] = 0;
        D_30da[0] = 0;
        D_30f0[0] = 0;
        D_3304 = 0;
        D_3656[0] = 0;
        D_367e[0] = 0;
        D_369e[0] = 0;
        D_36da[0] = 0;
        D_3702[0] = 0;
        D_370e[0] = 0;
        D_3716[0] = 0;
        D_3720[0] = 0;
        D_372c[0] = 0;
        D_3738[0] = 0;
        D_3744[0] = 0;
        D_3750[0] = 0;
        D_3758[0] = 0;
        D_3760[0] = 0;
        D_3822[0] = 0;
        D_3832[0] = 0;
        D_383a[0] = 0;
        D_3840 = 0;
        D_3842[0] = 0;
        D_3850[0] = 0;
        D_385e[0] = 0;
        D_3866[0] = 0;
        D_386e[0] = 0;
        D_3876[0] = 0;
        D_3a06[0] = 0;
        D_3a0a[0] = 0;
        D_3a0e[0] = 0;
        D_3a26[0] = 0;
        D_3a82[0] = 0;
        D_3c0a[0] = 0;
        D_3c10[0] = 0;
        D_3c30[0] = 0;
        D_3c38[0] = 0;
        D_3c40[0] = 0;
        D_3c48[0] = 0;
        D_3cbe[0] = 0;
        D_3d5a[0] = 0;
        D_3d86[0] = 0;
        D_3d8e[0] = 0;
        D_3d96[0] = 0;
        D_3dca[0] = 0;
        D_3dd6[0] = 0;
        D_3ddc[0] = 0;
        D_3de2[0] = 0;
        D_3de8[0] = 0;
        D_3dee[0] = 0;
        D_3df4[0] = 0;
        D_3dfa[0] = 0;
        D_3e00[0] = 0;
        D_3e06[0] = 0;
        D_3e5a[0] = 0;
        D_3e60[0] = 0;
        D_3e66[0] = 0;
        D_3e6a[0] = 0;
        D_3e6e[0] = 0;
        D_3e78[0] = 0;
        D_3eea[0] = 0;
        D_3f5c[0] = 0;
        D_3fce[0] = 0;
        D_4040[0] = 0;
        D_40b2[0] = 0;
        D_4124[0] = 0;
        D_412c[0] = 0;
        D_4134[0] = 0;
        D_413c[0] = 0;
        D_416c[0] = 0;
        D_41bc[0] = 0;
        D_41c4[0] = 0;
        D_41cc[0] = 0;
        D_4512[0] = 0;
        D_4592[0] = 0;
        D_4596[0] = 0;
        D_4882[0] = 0;
        D_4886[0] = 0;
        D_488a[0] = 0;
        D_488e[0] = 0;
        D_4892[0] = 0;
        D_4aee = 0;
        D_4aef = 0;
        D_4af0 = 0;
        D_4af1 = 0;
        D_4af2 = 0;
        D_4af3 = 0;
        D_4af4 = 0;
        D_4af5 = 0;
        D_4af6[0] = 0;
        D_4b08[0] = 0;
        D_4b1a[0] = 0;
        D_4b2c[0] = 0;
        D_4b5e[0] = 0;
        D_4b6e[0] = 0;
        D_4b7e[0] = 0;
        D_4b86[0] = 0;
        D_4b8e[0] = 0;
        D_4b96[0] = 0;
        D_4b9e[0] = 0;
        D_4bc2[0] = 0;
        D_4be6[0] = 0;
        D_4bf4[0] = 0;
        D_4c02[0] = 0;
        D_4c10[0] = 0;
        D_4c1e[0] = 0;
        D_4c24[0] = 0;
        D_4c2a[0] = 0;
        D_4c30[0] = 0;
        D_4c36[0] = 0;
        D_4c48[0] = 0;
        D_4c54[0] = 0;
        D_4c66[0] = 0;
        D_4cdc[0] = 0;
        D_4d10[0] = 0;
        D_4d44[0] = 0;
        D_4d4c[0] = 0;
        D_4d56[0] = 0;
        D_4d5e[0] = 0;
        D_4d66[0] = 0;
        D_4d6e[0] = 0;
        D_4d76[0] = 0;
        D_4d7a[0] = 0;
        D_4d7e[0] = 0;
        D_4dc4[0] = 0;
        D_4e6e[0] = 0;
        D_4e7a[0] = 0;
        D_4e80[0] = 0;
        D_50ea[0] = 0;
        D_5146[0] = 0;
        D_514c[0] = 0;
        D_5150 = 0;
        D_5152 = 0;
        D_5154 = 0;
        D_5156 = 0;
        D_5158 = 0;
        D_515a = 0;
        D_5164[0] = 0;
        D_516c[0] = 0;
        D_5174[0] = 0;
        D_51fc[0] = 0;
        D_5204[0] = 0;
        D_520c = 0;
        D_52c4 = 0;
        D_52c8 = 0;
        D_52da_pen_color = 0;
        D_52ef_forceEga = 0;
        D_52f1_forceTandy = 0;
        D_52f3_forceHerc = 0;
        D_52f6 = 0;
        D_52f8 = 0;
        D_52fa = 0;
        D_52fc = 0;
        D_52fe = 0;
        D_5300 = 0;
        D_5304 = 0;
        D_5306[0] = 0;
        D_5314[0] = 0;
        D_5354 = 0;
        D_5356 = 0;
        D_5358 = 0;
        D_535a = 0;
        D_535c = 0;
        D_5386_current_text_window_idx = 0;
        D_5388_current_charset_idx = 0;
        D_538a = 0;
        D_538c = 0;
        D_538e = 0;
    }
#endif
    for (local_6 = 0; local_6 < (uint)D_585b; local_6++)
    {
        if (D_55a8_party[local_6].status == 'S' && ULTIMA_2092_RandomRange(0, 0xf) == 0xf)
        {
            D_55a8_party[local_6].status = 'G';
        }
    }

    do
    {
        local_4_ab = 0;
        local_a = *ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y);
        if (local_a == 0x8c && (D_587c_partyTile & 0xfe) != 0x14)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2768, "A TRAPDOOR!\n"));
            local_8 = D_587c_partyTile;
            D_587c_partyTile = 0;
            ULTIMA_5910_UpdateFrame();
            ULTIMA_2aa8();
            D_587c_partyTile = local_8;
            if (D_5893_map_id == 0x1d)
            {
                ULTIMA_0a70_GRAP_2d_SetPenColor(0);
                ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
                for (local_6 = 1000; local_6 > 250; local_6--)
                {
                    ULTIMA_22e2_PcspkOn(local_6);
                    ULTIMA_20c8_SomeDelay(1, 0x28);
                }
                ULTIMA_230e_PcspkOff();
                memset(D_6608, 0x8f, 0x400);
                D_24e6 = 1;
                memset(D_5c5a, 0, 0x100);
                for (local_6 = 0; local_6 < (uint)D_585b; local_6++)
                {
                    D_55a8_party[local_6].hp = 0;
                    D_55a8_party[local_6].status = 'D';
                    ULTIMA_223c_AudioWhiteNoise(0x28, 0xbb8, 0x1f4);
                    ULTIMA_2900_UpdateVitalsDisplay();
                }
            }
            else
            {
                D_5895_map_level--;
                TOWN_0408(1);
                local_4_ab = 1;
            }
        }
        else if (local_a == 4 && D_587c_partyTile == 0x1c)
        {
            for (local_6 = 0; local_6 < (uint)D_585b; local_6++)
            {
                if (D_55a8_party[local_6].status != 'D' && D_55a8_party[local_6].status != 'P' &&
                    D_55a8_party[local_6].dex < (uint)ULTIMA_2092_RandomRange(0, 0x1d))
                {
                    ULTIMA_1850_PrintString(_TEXT(0x2775, "Poisoned!\n"));
                    D_55a8_party[local_6].status = 'P';
                    ULTIMA_2900_UpdateVitalsDisplay();
                }
            }
        }
        else if (local_a == 0xbc || local_a == 0x8f)
        {
            ULTIMA_5910_UpdateFrame();
            ULTIMA_1850_PrintString(_TEXT(0x2780, "Burning!\n"));
            ULTIMA_2aa8();
        }
    } while (local_4_ab != 0);

    ULTIMA_2ae8();
}
