// Resident code from CS:51a0 to 5a28: an object of its own, compiled with MSC 5.0 (ULTIMA_5394_PutActorsInView
// matches only under 5.0; 5a28, 5d0a and 5e4a only under 5.1, in r5a28.c).

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

// A random animation phase 0-3 (always 0 when the status icon is T).
static int ULTIMA_51a0_RandomFrame(void)
{
    if (D_587a_statusIcon == 'T')
    {
        return 0;
    }
    else
    {
        return ULTIMA_2092_RandomRange(0, 3);
    }
}

// Put actor shape param_5 on view cell (param_1,param_2), changed by the furniture under it (chairs, beds, ladders, mirrors); the view tile is cleared.
static void ULTIMA_51b8_PutViewActor(int param_1, int param_2, int param_3, int param_4, int param_5)
{
    int local_4;

    ASSERT(param_1 >= 0 && param_1 < 32 && param_2 >= 0 && param_2 < 0xb);

    if (param_5 == 0x1c || param_5 >= 0x12 && param_5 < 0x16 || param_5 >= 0x40 || param_5 >= 0x28 && param_5 < 0x2c)
    {
        // 51e9
        local_4 = *ULTIMA_4402_GetTileAddr(param_3, param_4);
        if (local_4 == TILE_MAP_EC || local_4 == TILE_MAP_A)
            return;

        // 520b?
        if (local_4 == TILE_MAP_57) // 87
        {
            // 520b
            GetMapViewport(param_1, param_2) = TILE_MAP_38;
            return;
        }

        if (local_4 == TILE_MAP_6A || local_4 == TILE_MAP_6B) // 106, 107
        {
            // 522e
            if ((param_5 & 0xf0) == 0x80)
            {
                return;
            }
            if ((param_5 & 0xfc) == 0x28)
            {
                return;
            }
        }
        else
        {
            // 524a
            if (param_5 < 0x80)
            {
                // 5254
                switch (local_4)
                {
                case TILE_MAP_84: // 132
                    // 5276
                    param_5 = ULTIMA_51a0_RandomFrame() + 0x60; // -> 5300
                    break;

                case TILE_MAP_85: // 133
                    // 5280
                    param_5 = ULTIMA_51a0_RandomFrame() + 0x64; // -> 5300
                    break;

                case TILE_MAP_LADDER_UP: // 200
                    // 528a
                    param_5 = 0x17;
                    break;

                case TILE_MAP_LADDER_DOWN: // 201
                    // 5292
                    param_5 = 0x18;
                    break;

                case TILE_MAP_BED: // 171
                    // 529a
                    param_5 = 0x1a;
                    break;

                // 93..9e
                case TILE_MAP_MIRROR: // 157
                case TILE_MAP_MIRROR_9E: // 158 (531a)
                    // 52a2
                    param_5 = ULTIMA_51a0_RandomFrame() + 0x3c;
                    break;

                case TILE_MAP_CHAIR_92: // 146
                    // 52aa
                    if ((*ULTIMA_4402_GetTileAddr(param_3, param_4 + 1) == TILE_MAP_TABLE_9A) ||
                        (*ULTIMA_4402_GetTileAddr(param_3, param_4 + 1) == TILE_MAP_TABLE_9C))
                    {
                        param_5 = ULTIMA_51a0_RandomFrame() + 0x34;
                    }
                    else
                    {
                        param_5 = 0x32;
                    }
                    break;

                case TILE_MAP_CHAIR_90: // 144
                    // 52da
                    if ((*ULTIMA_4402_GetTileAddr(param_3, param_4 - 1) == TILE_MAP_TABLE_9B) ||
                        (*ULTIMA_4402_GetTileAddr(param_3, param_4 - 1) == TILE_MAP_TABLE_9C))
                    {
                        param_5 = ULTIMA_51a0_RandomFrame() + 0x38;
                    }
                    else
                    {
                        param_5 = 0x30;
                    }
                    break;

                case TILE_MAP_CHAIR_91: // 145, 532a
                case TILE_MAP_CHAIR_93: // 147
                    // -> 530e -> 5300
                    param_5 = (local_4 & 3) + 0x30;
                    break;

                default:
                    // 532c
                    if ((*ULTIMA_4402_GetTileAddr(param_3, param_4 - 1) == TILE_MAP_MIRROR) && (param_2 != 0))
                    {
                        GetMapViewport(param_1, param_2 - 1) = TILE_MAP_MIRROR_9E;
                    }
                    break;
                }
            }
        }
    }

    GetActorMap(param_1, param_2) = (byte)param_5;
    GetMapViewport(param_1, param_2) = 0;

    // 538d
}

// CHECKED
// MSC 5.0 (this object is 5.0, r5a28.c after it 5.1): only 5.0 replaces the loop test with a
// pointer compare (cmp [bp-26h],5C5Bh; jb) while keeping the dec of local_c; 5.1 gives
// dec/jns. The loop must be a for: a do-while is never strength-reduced (13 pointer temporaries).
// Put the party and the actors on their cells of the view (sitting, sleeping and climbing shapes by the tile under them).
void ULTIMA_5394_PutActorsInView(void)
{
    int local_e_am;
    int local_c;
    byte local_a;
    byte local_8;

    byte local_6;
    byte local_4_ab;

    if (D_5893_mapId < 0x80)
    {
        D_5c5a_actors[0].x = D_5896_mapX;
        D_5c5a_actors[0].y = D_5897_mapY;
        D_5c5a_actors[0].z = D_5895_mapLevel;
        D_5c5a_actors[0].animTile = D_5c5a_actors[0].tile = D_587c_partyTile;
        
        for (local_e_am = 0; local_e_am < 0xb; local_e_am++)
        {
            for (local_c = 0; local_c < 0xb; local_c++)
            {
                if (GetMapViewport(local_c, local_e_am) == TILE_MAP_DD &&
                    ULTIMA_6ff0_DistanceFromCenter(local_c, local_e_am) > 5)
                {
                    GetMapViewport(local_c, local_e_am) = TILE_MAP_1C;
                }
                else if (GetMapViewport(local_c, local_e_am) == TILE_MAP_1C &&
                    ULTIMA_6ff0_DistanceFromCenter(local_c, local_e_am) <= 5)
                {
                    GetMapViewport(local_c, local_e_am) = TILE_MAP_DD;
                }
            }
        }
    }

    // 542a
    for (local_c = 31; local_c >= 0; local_c--)
    {
        if (D_5c5a_actors[local_c].tile != 0)
        {
            local_8 = local_4_ab = D_5c5a_actors[local_c].x;
            local_a = local_6 = D_5c5a_actors[local_c].y;

            if (D_5893_mapId < 0x80)
            {
                local_8 -= D_5896_mapX - 5;
                local_a -= D_5897_mapY - 5;
                if (D_5c5a_actors[local_c].z != D_5895_mapLevel || local_8 > 10 || local_a > 10)
                    continue;
            }

            if (D_5c5a_actors[local_c].animTile != 0 &&
                GetMapViewport(local_8, local_a) != 0xff &&
                GetMapViewport(local_8, local_a) != TILE_MAP_87)
            {
                if ((D_5c5a_actors[local_c].tile & 0xfc) == TILE_ACTOR_E8 || D_5c5a_actors[local_c].tile == TILE_ACTOR_SLEEP || D_5c5a_actors[local_c].tile == TILE_ACTOR_DEAD)
                {
                    // 5503
                    if (GetMapViewport(local_8, local_a) != 0)
                    {
                        GetActorMap(local_8, local_a) = D_5c5a_actors[local_c].animTile;
                        GetMapViewport(local_8, local_a) = 0;
                    }
                }
                else if (D_5c5a_actors[local_c].animTile == TILE_ACTOR_1D || D_5c5a_actors[local_c].animTile == TILE_ACTOR_SLEEP)
                {
                    GetActorMap(local_8, local_a) = D_5c5a_actors[local_c].animTile;
                    GetMapViewport(local_8, local_a) = 0;
                }
                else if (D_5c5a_actors[local_c].tile == TILE_ACTOR_BARD)
                {
                    if (GetMapViewport(local_8, local_a) == TILE_MAP_CHAIR_92)
                    {
                        GetActorMap(local_8, local_a) = D_5c5a_actors[local_c].animTile;
                        GetMapViewport(local_8, local_a) = 0;
                    }
                    else
                    {
                        ULTIMA_51b8_PutViewActor(local_8, local_a, local_4_ab, local_6, D_5c5a_actors[local_c].animTile - 8);
                    }
                }
                else
                {
                    ULTIMA_51b8_PutViewActor(local_8, local_a, local_4_ab, local_6, D_5c5a_actors[local_c].animTile);
                }
            }
        }
    }
}

// Whether a combatant fights against the party: a monster that is not charmed, a party member who is. 0 for the dead.
int ULTIMA_5646_IsEnemy(int param_1)
{
    int local1_6;
    int local2_4;

    local1_6 = D_ba14_combatants[param_1].flags;
    local2_4 = D_ba14_combatants[param_1].entityIdx;

    if ((local1_6 & COMBAT_FLAGS_DEAD) != 0)
    {
        return 0;
    }

    // 566e
    if ((local1_6 & COMBAT_FLAGS_PLAYER) != 0)
    {
        if (local2_4 != 0 && D_55a8_party[local2_4].name[4] == 'j')
        {
            return 1;
        }
        else
        {
            // 568e
            return (local1_6 & COMBAT_FLAGS_CHARMED);
        }
    }
    else
    {
        // 5696
        return !(local1_6 & COMBAT_FLAGS_CHARMED);
    }

    // 56a4
}

void ULTIMA_56ac_DrawMap(void)
{
    int local_8_al;
    int local_6_an;
    int local_4;
    int local_2;

    for (local_8_al = 0; local_8_al < 11; local_8_al++)
    {
        for (local_6_an = 0; local_6_an < 0xb; local_6_an++)
        {
            if (GetMapViewport(local_6_an, local_8_al) == 0)
            {
                if (GetActorMap(local_6_an, local_8_al) != 0x16)
                {
                    // draw actor
                    ULTIMA_10e0_GRAP_51_PutTile(0x100 + GetActorMap(local_6_an, local_8_al), local_6_an, local_8_al);
                }
            }
            else
            {
                if (GetMapViewport(local_6_an, local_8_al) == TILE_MAP_MOONGATE && D_5887_moongateHeight != 0 && D_5887_moongateHeight < 0x10)
                {
                    // moongate?
                    ULTIMA_1112_GRAP_60_Moongate(D_5887_moongateHeight, local_6_an, local_8_al);
                }
                else
                {
                    // draw map tile
                    ULTIMA_10e0_GRAP_51_PutTile(D_b11e_tileFrames[GetMapViewport(local_6_an, local_8_al)], local_6_an, local_8_al);
                }
            }
        }
    }

    if (0x7f < D_5893_mapId && (D_589f_focusBlink = !D_589f_focusBlink) != 0 && D_589e_curCombatant != 0xff && ULTIMA_5646_IsEnemy(D_589e_curCombatant) == 0)
    {
        local_2 = (uint)D_ba14_combatants[D_589e_curCombatant].x * 0x10 + 8;
        local_4 = (uint)D_ba14_combatants[D_589e_curCombatant].y * 0x10 + 8;

        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);

        for (local_6_an = 0; local_6_an < 2; local_6_an++)
        {
            for (local_8_al = 0; local_8_al < 0xf; local_8_al += 0xe)
            {
                ULTIMA_0c9c_GRAP_39_HorizLine(local_2, local_4 + local_6_an + local_8_al, local_2 + 0xf);
                ULTIMA_0cf2_GRAP_3c_VertLine(local_2 + local_6_an + local_8_al, local_4, local_4 + 0xf);
            }
        }

        if (D_5898_crosshairOn != 0)
        {
            // 5813
            // draw crosshair / focus
            local_2 = (uint)D_5899_crosshairX * 0x10 + 8; // cross hair x
            local_4 = (uint)D_589a_crosshairY * 0x10 + 8; // cross hair y

            // 586a
            for (local_6_an = 0; local_6_an < 2; local_6_an++)
            {
                for (local_8_al = 0; local_8_al < 2; local_8_al++)
                {
                    // 5887
                    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
                    ULTIMA_0b10_GRAP_Line(local_8_al * 0xb + local_2 + 2, local_6_an * 3 + local_4 + 6, local_8_al * 3 + local_2 + 6, local_6_an * 3 + local_4 + 6);
                    ULTIMA_0f90_GRAP_LineTo(local_8_al * 3 + local_2 + 6, local_6_an * 0xb + local_4 + 2);
                    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
                    ULTIMA_0b10_GRAP_Line(local_8_al * 0xb + local_2 + 2, local_6_an * 5 + local_4 + 5, local_8_al * 5 + local_2 + 5, local_6_an * 5 + local_4 + 5);
                    ULTIMA_0f90_GRAP_LineTo(local_8_al * 5 + local_2 + 5, local_6_an * 0xb + local_4 + 2);
                    ULTIMA_0b10_GRAP_Line(local_8_al * 0xb + local_2 + 2, local_4 + local_6_an + 7, local_8_al * 3 + local_2 + 6, local_4 + local_6_an + 7);

                    ULTIMA_0b10_GRAP_Line(local_2 + local_8_al + 7, local_6_an * 3 + local_4 + 6, local_2 + local_8_al + 7, local_6_an * 0xb + local_4 + 2);
                }
            }
        }
    }
}

// Animate / Update
void ULTIMA_5910_UpdateFrame(void)
{
    int local_2;
    int local_4;

    D_545e = 0xff;
    if (D_587a_statusIcon == 'T')
    {
        D_5891_animate = 0;
    }
    if (D_58a4_viewOn != 0)
    {
        if (D_5891_animate != 0) // animate flag
        {
            if (D_5891_animate != 0xff)
            {
                ULTIMA_4552_AnimateActors();
            }
            ULTIMA_2f62_UpdateWindDirectionRandomly();
            if (D_5893_mapId < 0x80)
            {
                ULTIMA_475a_UpdateMoongates();
                ULTIMA_70a6_AnimateLighthouse();
            }
        }

        if (D_5893_mapId < 0x80)
        {
            if (D_24e6_viewDirty != 0)
            {
                ULTIMA_5d0a_BuildView(D_58a5_lightLevel, (uint)D_5896_mapX - (uint)D_589b_mapOriginX, (uint)D_5897_mapY - (uint)D_589c_mapOriginY, 0xb);
                D_24e6_viewDirty = 0;
            }
            else
            {
                for (local_4 = 0; local_4 < 11; local_4++)
                {
                    for (local_2 = 0; local_2 < 11; local_2++)
                    {
                        if (GetMapViewport(local_2, local_4) == 0)
                        {
                            GetMapViewport(local_2, local_4) =
                                *ULTIMA_4402_GetTileAddr((byte)local_2 + (uint)D_5896_mapX - 5, (byte)local_4 + (uint)D_5897_mapY - 5);
                        }
                    }
                }
            }
        }
        else // map_id >= 0x80; combat?
        {
            memcpy(D_ab02_viewTiles, D_ad14_combatMap, 0x160);
        }

        ULTIMA_5394_PutActorsInView();
        ULTIMA_56ac_DrawMap();
        if (D_5891_animate != 0)
        {
            ULTIMA_4102_AudioPlayAmbientSfx();
        }
    }

    D_5891_animate = 1;
}
