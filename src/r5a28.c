// Resident code from CS:5a28 to 5f86: an object of its own, compiled with MSC 5.1, after the
// MSC 5.0 object 5000.c (ULTIMA_51a0_RandomFrame to ULTIMA_5910) and before 6000.c. Its data is the two
// tables at DS:6a86.

#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "audio/aud_mus.h"

#include "cmds.h"
#include "combat.h"
#include "dnglook.h"
u8 D_6a86[0x13] = {
    0x09, 0x0A, 0x0C, 0x0D, 0x4D, 0x4E, 0x4F, 0x5A,
    0x97, 0xB8, 0xB9, 0xBC, 0xD0, 0xD1, 0xD2, 0xD3,
    0xF8, 0xFE, 0xFF
};

u8 D_6a9a[10] = {
    0xDC, 0xBD, 0xBE, 0xB2, 0xDE, 0xBF, 0xB0, 0xB1,
    0xB3, 0xBC
};

static bool ULTIMA_5dfe_IsTransparentTile(byte param_1, int param_2);

// CHECKED
// The locals are named for the original's slots (slots.py, after renaming them local_N).
// Two unused labels (L_s before the GetMapViewport store at 5b83, L_c before the 5dfe test)
// stop MSC keeping an address or param_2_y + local_214_an in DI across them, which gives the
// original's [bx+di+table] addressing; the gotos to L_5c93 put that block before 5c9c.
// The queue is 256 ints, not 257: `register byte* cell` (SI) at function level reserves the
// unused slot at -4 between local_4_ab and the queue (cell hashes to bucket 0, declared before
// local_4_ab). As a block variable its slot came after the locals and moved the temporary.
// generate visibility map (BFS)
static void ULTIMA_5a28_BuildVisibilityMap(int param_1, int param_2_y, int param_3_x, int param_4, int param_5, uint param_6, byte* param_7_map)
{
    int local_218_aj; // -0x216
    byte local_216_al;
    int local_214_an; // -0x212; y
    int local_212; // -0x210; x
    int local_210_ab; // -0x20e
    int local_20e_an; // -0x20c
    int local_20c; // -0x20a
    int local_20a; // -0x208
    int local_208; // -0x206
    int local_206[256]; // -0x204
    register byte* cell;
    int local_4_ab;

    local_20e_an = 0;
    local_4_ab = 0;
    local_218_aj = 7;
    local_20c = 1;
    local_210_ab = 0;

    if (param_1 > 0)
    {
        // 5a57
        if (param_4 == -0x6f)
        {
            param_4 = 0;
            local_210_ab++;
        }

        // 5a66 (si vs di)
        param_1++;
        local_206[local_4_ab] = 5;
        local_206[++local_4_ab] = 5;
        ++local_4_ab;
        param_7_map[param_4 * 0x20 + param_5 + 0xa5] =
            *ULTIMA_4402_GetTileAddr(param_3_x + (uint)D_589b_mapOriginX + 5, param_2_y + (uint)D_589c_mapOriginY + 5);

        while (local_20e_an != local_4_ab)
        {
            // 5abc
            local_20a = local_214_an = local_206[local_20e_an];
            local_208 = local_212 = local_206[++local_20e_an];
            ++local_20e_an;

            while (local_218_aj > -1) // 5ce5
            {
                // 5aec
                switch (local_218_aj)
                {
                case 0:
                case 1:
                case 7:
                    local_212--;
                    break;
                case 4:
                case 5:
                    local_212++;
                    break;
                case 2:
                case 3:
                    local_214_an--;
                    break;
                case 6:
                    local_214_an++;
                    break;
                }

                if (local_212 < 0 || 10 < local_212 || local_214_an < 0 || 10 < local_214_an)
                {
                    // 5b42
                    local_20c = 0;
                }

#if !defined(MATCHING_BUILD)
                // TODO: temporary!
                if (param_7_map == D_ad14_combatMap)
                {
                    if (param_4 + local_214_an < 0 || param_5 + local_212 < 0 || param_4 + local_214_an >= 0x20 || param_5 + local_212 >= 0x20)
                    {
                        local_218_aj--;
                        local_20c = 1;
                        continue;
                    }
                }
                else if (param_7_map == D_ab02_viewTiles)
                {
                    if (param_4 + local_214_an < 0 || param_5 + local_212 < 0 || param_4 + local_214_an >= 0xb || param_5 + local_212 >= 0x20)
                    {
                        local_218_aj--;
                        local_20c = 1;
                        continue;
                    }
                }
                else
                {
                    ASSERT(0);
                }
#endif

                // 5b48
                cell = param_7_map + (param_4 + local_214_an) * 0x20 + (param_5 + local_212);

                // 5b5f
                if (local_210_ab == 0)
                {
                    // 5b66
                    if ((GetMapViewport(local_212, local_214_an) != 0) && (0x1f >= param_4 + local_214_an))
                    {
                    L_s:
                        GetMapViewport(local_212, local_214_an) = 0;

                        // -> 5b9b
                    }
                    else
                    {
                        // 5b95
                        local_20c = 0;
                    }
                }
                else
#if !defined(MATCHING_BUILD)
                    if (local_20c != 0)
#endif
                {
                    // 5b90
                    if (*cell != 0xff)
                    {
                        // 5b95
                        local_20c = 0;
                    }
                }

                // 5b9b
                if (local_20c != 0)
                {
                    // 5ba5
                    local_216_al = *ULTIMA_4402_GetTileAddr(param_3_x + local_212 + (uint)D_589b_mapOriginX, param_2_y + local_214_an + (uint)D_589c_mapOriginY);
                    if (ULTIMA_6ff0_DistanceFromCenter(local_212, local_214_an) >= param_1)
                    {
                        if (local_210_ab)
                        {
                        L_c:
                            if (ULTIMA_5dfe_IsTransparentTile(local_216_al, ULTIMA_6ff0_DistanceFromCenter(local_212, local_214_an)) == 0)
                            {
                                // 5c05
                                if (
#if !defined(MATCHING_BUILD)
                                    param_2_y + local_20a >= 0x20 || param_3_x + local_208 >= 0x20 ||
                                    param_2_y + local_214_an < 0 || param_3_x + local_208 < 0 ||
#endif
                                    param_7_map[local_20a * 0x20 + local_208] == 0 ||
                                    D_ad14_combatMap[(param_2_y + local_20a) * 0x20 + param_3_x + local_208] == 0 ||
                                    D_ad14_combatMap[(param_2_y + local_214_an) * 0x20 + param_3_x + local_212] == 0)
                                {
                                    local_216_al = *cell = 0xff;
                                }
                                else
                                    goto L_5c93;
                            }
                            else
                            {
                                if (param_2_y + local_214_an < 0 || param_3_x + local_212 < 0 ||
                                    param_3_x + local_212 > 0x1f || param_2_y + local_214_an > 0x1f)
                                {
                                    *cell = 0;
                                }
                                else if (D_ad14_combatMap[(param_2_y + local_214_an) * 0x20 + param_3_x + local_212] != 0)
                                {
                                L_5c93:
                                    *cell = local_216_al;
                                }
                                else
                                {
                                    *cell = 0;
                                }
                            }
                        }
                        else
                        {
                            local_216_al = 0xff;
                        }
                    }
                    else
                        goto L_5c93;

                    // 5ca1
                    if (ULTIMA_5dfe_IsTransparentTile(local_216_al, ULTIMA_6ff0_DistanceFromCenter(local_212, local_214_an)) != 0)
                    {
                        local_206[local_4_ab] = local_214_an;
                        local_4_ab++;
                        local_206[local_4_ab] = local_212;
                        local_4_ab++;
                    }
                }

                // 5cdb
                local_218_aj--;
                local_20c = 1;
            }

            // 5cef
            local_218_aj = 7;
        }
    }

    // 5d01
}

// update map data?
// param_1: light intensity
// Build the 11x11 view around map cell (x, y) for the light level: what the party can see.
void ULTIMA_5d0a_BuildView(int param_1_light, int param_2_x, int param_3_y, int param_4)
{
    int local5_4;
    int local4_6;
    byte* local3_8;
    int local2_a;
    int local1_c;

    for (local2_a = 0; local2_a < 0xb; local2_a++)
    {
        memset(&GetMapViewport(0, local2_a), 0xff, 0xb);
    }

    local2_a = param_2_x - 5;
    local1_c = param_3_y - 5;

    if (param_1_light > 0)
    {
        ULTIMA_5a28_BuildVisibilityMap(param_1_light, local1_c, local2_a, -111, 0, 0x20, D_ab02_viewTiles);
        local3_8 = D_ab02_viewTiles;

        for (local1_c = 0; local1_c < 0xb; local1_c++)
        {
            for (local2_a = 0; local2_a < 0xb; local2_a++)
            {
                if (*local3_8 == 0)
                {
                    (*local3_8)--;
                }

                local3_8++;
            }

            local3_8 += 0x15;
        }
    }

    if (param_1_light < 0)
    {
        local5_4 = param_2_x - 5;
        local4_6 = param_3_y - 5;

        for (local1_c = 0; local1_c < 0xb; local1_c++)
        {
            for (local2_a = 0; local2_a < 0xb; local2_a++)
            {
                GetMapViewport(local2_a, local1_c) = *ULTIMA_4402_GetTileAddr(
                    local5_4 + local2_a + (uint)D_589b_mapOriginX,
                    local4_6 + local1_c + (uint)D_589c_mapOriginY);
            }
        }
    }
}

// Probably: the tile does not block sight (not in D_6a86); the door-like tiles only when param_2 is 1.
static bool ULTIMA_5dfe_IsTransparentTile(byte param_1, int param_2)
{
    if (param_1 == TILE_MAP_4B || param_1 == TILE_MAP_4A || param_1 == TILE_MAP_DOOR_BA || param_1 == TILE_MAP_DOOR_BB || param_1 == TILE_MAP_98)
    {
        if (param_2 == 1)
            return 1;
        else
            return 0;
    }
    else
    {
        if (!memchr(D_6a86, param_1, 0x13))
            return 1;
        else
            return 0;
    }
}

// Build the 32x32 light map: the cells lit by the lamps, fires and moongates of the map buffer.
void ULTIMA_5e4a_BuildLightMap(void)
{
    byte local_92_aj;
    int local_90_al;
    int local_8e_ah[64];
    int local_e;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    u8* local_4_ab;

    local_90_al = 0;
    local_4_ab = D_ad14_combatMap;

    memset(D_ad14_combatMap, 0xff, 0x400);

    // 5e72
    for (local_8 = 0; local_8 < 0x20; local_8++)
    {
        for (local_6 = 0; local_6 < 0x20; local_6++)
        {
            // 5e78
            local_92_aj = *ULTIMA_4402_GetTileAddr(D_589b_mapOriginX + local_6, D_589c_mapOriginY + local_8);
            if (memchr(D_6a9a, local_92_aj, 10) != 0)
            {
                // 5eb0
                local_8e_ah[local_90_al++] = local_8;
                local_8e_ah[local_90_al++] = local_6;
                *local_4_ab = local_92_aj;
            }

            // 5ed2
            local_4_ab++;
        }
    }

    local_6 = local_90_al;
    local_8 = 0;

    // 5ef5
    if (local_90_al > 0)
    {
        do
        {
            // 5efe
            local_e = local_8e_ah[local_8++] - 5;
            local_c = local_8e_ah[local_8] - 5;

            // 5f23
            for (local_a = 0; local_a < 0xb; local_a++)
            {
                // 5f29
                memset(&GetMapViewport(0, local_a), 0xff, 0xb);
            }

            // 5f3f
            ULTIMA_5a28_BuildVisibilityMap(10, local_e, local_c, local_e, local_c, 0x20, D_ad14_combatMap);
            local_8++;
        } while (local_8 < local_6);
    }

    // 5f65
    local_4_ab = D_ad14_combatMap;

    for (local_6 = 0; local_6 < 0x400; local_6++)
    {
        if (*local_4_ab == 0xff)
        {
            (*local_4_ab)++;
        }
        local_4_ab++;
    }
}

void ULTIMA_6fbc_TilesetMode(int param_1);

void ULTIMA_6bc2_SetUpCombat(int param_1, int param_2);
