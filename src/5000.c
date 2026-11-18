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

// Put actor shape on view cell (viewX,viewY), changed by the furniture under it (chairs, beds, ladders, mirrors); the view tile is cleared.
static void ULTIMA_51b8_PutViewActor(int viewX, int viewY, int mapX, int mapY, int shape)
{
    int tileUnder;

    ASSERT(viewX >= 0 && viewX < 32 && viewY >= 0 && viewY < 0xb);

    if (shape == 0x1c || shape >= 0x12 && shape < 0x16 || shape >= 0x40 || shape >= 0x28 && shape < 0x2c)
    {
        // 51e9
        tileUnder = *ULTIMA_4402_GetTileAddr(mapX, mapY);
        if (tileUnder == TILE_MAP_EC || tileUnder == TILE_MAP_A)
            return;

        // 520b?
        if (tileUnder == TILE_MAP_57) // 87
        {
            // 520b
            GetMapViewport(viewX, viewY) = TILE_MAP_38;
            return;
        }

        if (tileUnder == TILE_MAP_6A || tileUnder == TILE_MAP_6B) // 106, 107
        {
            // 522e
            if ((shape & 0xf0) == 0x80)
            {
                return;
            }
            if ((shape & 0xfc) == 0x28)
            {
                return;
            }
        }
        else
        {
            // 524a
            if (shape < 0x80)
            {
                // 5254
                switch (tileUnder)
                {
                case TILE_MAP_84: // 132
                    // 5276
                    shape = ULTIMA_51a0_RandomFrame() + 0x60; // -> 5300
                    break;

                case TILE_MAP_85: // 133
                    // 5280
                    shape = ULTIMA_51a0_RandomFrame() + 0x64; // -> 5300
                    break;

                case TILE_MAP_LADDER_UP: // 200
                    // 528a
                    shape = 0x17;
                    break;

                case TILE_MAP_LADDER_DOWN: // 201
                    // 5292
                    shape = 0x18;
                    break;

                case TILE_MAP_BED: // 171
                    // 529a
                    shape = 0x1a;
                    break;

                // 93..9e
                case TILE_MAP_MIRROR: // 157
                case TILE_MAP_MIRROR_9E: // 158 (531a)
                    // 52a2
                    shape = ULTIMA_51a0_RandomFrame() + 0x3c;
                    break;

                case TILE_MAP_CHAIR_92: // 146
                    // 52aa
                    if ((*ULTIMA_4402_GetTileAddr(mapX, mapY + 1) == TILE_MAP_TABLE_9A) ||
                        (*ULTIMA_4402_GetTileAddr(mapX, mapY + 1) == TILE_MAP_TABLE_9C))
                    {
                        shape = ULTIMA_51a0_RandomFrame() + 0x34;
                    }
                    else
                    {
                        shape = 0x32;
                    }
                    break;

                case TILE_MAP_CHAIR_90: // 144
                    // 52da
                    if ((*ULTIMA_4402_GetTileAddr(mapX, mapY - 1) == TILE_MAP_TABLE_9B) ||
                        (*ULTIMA_4402_GetTileAddr(mapX, mapY - 1) == TILE_MAP_TABLE_9C))
                    {
                        shape = ULTIMA_51a0_RandomFrame() + 0x38;
                    }
                    else
                    {
                        shape = 0x30;
                    }
                    break;

                case TILE_MAP_CHAIR_91: // 145, 532a
                case TILE_MAP_CHAIR_93: // 147
                    // -> 530e -> 5300
                    shape = (tileUnder & 3) + 0x30;
                    break;

                default:
                    // 532c
                    if ((*ULTIMA_4402_GetTileAddr(mapX, mapY - 1) == TILE_MAP_MIRROR) && (viewY != 0))
                    {
                        GetMapViewport(viewX, viewY - 1) = TILE_MAP_MIRROR_9E;
                    }
                    break;
                }
            }
        }
    }

    GetActorMap(viewX, viewY) = (byte)shape;
    GetMapViewport(viewX, viewY) = 0;

    // 538d
}

// CHECKED
// MSC 5.0 (this object is 5.0, r5a28.c after it 5.1): only 5.0 replaces the loop test with a
// pointer compare (cmp [bp-26h],5C5Bh; jb) while keeping the dec of index; 5.1 gives
// dec/jns. The loop must be a for: a do-while is never strength-reduced (13 pointer temporaries).
// Put the party and the actors on their cells of the view (sitting, sleeping and climbing shapes by the tile under them).
void ULTIMA_5394_PutActorsInView(void)
{
    int row;
    int index_m;
    byte viewY;
    byte viewX;

    byte mapY_i;
    byte mapX_j;

    if (D_5893_mapId < 0x80)
    {
        D_5c5a_actors[0].x = D_5896_mapX;
        D_5c5a_actors[0].y = D_5897_mapY;
        D_5c5a_actors[0].z = D_5895_mapLevel;
        D_5c5a_actors[0].animTile = D_5c5a_actors[0].tile = D_587c_partyTile;
        
        for (row = 0; row < 0xb; row++)
        {
            for (index_m = 0; index_m < 0xb; index_m++)
            {
                if (GetMapViewport(index_m, row) == TILE_MAP_DD &&
                    ULTIMA_6ff0_DistanceFromCenter(index_m, row) > 5)
                {
                    GetMapViewport(index_m, row) = TILE_MAP_1C;
                }
                else if (GetMapViewport(index_m, row) == TILE_MAP_1C &&
                    ULTIMA_6ff0_DistanceFromCenter(index_m, row) <= 5)
                {
                    GetMapViewport(index_m, row) = TILE_MAP_DD;
                }
            }
        }
    }

    // 542a
    for (index_m = 31; index_m >= 0; index_m--)
    {
        if (D_5c5a_actors[index_m].tile != 0)
        {
            viewX = mapX_j = D_5c5a_actors[index_m].x;
            viewY = mapY_i = D_5c5a_actors[index_m].y;

            if (D_5893_mapId < 0x80)
            {
                viewX -= D_5896_mapX - 5;
                viewY -= D_5897_mapY - 5;
                if (D_5c5a_actors[index_m].z != D_5895_mapLevel || viewX > 10 || viewY > 10)
                    continue;
            }

            if (D_5c5a_actors[index_m].animTile != 0 &&
                GetMapViewport(viewX, viewY) != 0xff &&
                GetMapViewport(viewX, viewY) != TILE_MAP_87)
            {
                if ((D_5c5a_actors[index_m].tile & 0xfc) == TILE_ACTOR_E8 || D_5c5a_actors[index_m].tile == TILE_ACTOR_SLEEP || D_5c5a_actors[index_m].tile == TILE_ACTOR_DEAD)
                {
                    // 5503
                    if (GetMapViewport(viewX, viewY) != 0)
                    {
                        GetActorMap(viewX, viewY) = D_5c5a_actors[index_m].animTile;
                        GetMapViewport(viewX, viewY) = 0;
                    }
                }
                else if (D_5c5a_actors[index_m].animTile == TILE_ACTOR_1D || D_5c5a_actors[index_m].animTile == TILE_ACTOR_SLEEP)
                {
                    GetActorMap(viewX, viewY) = D_5c5a_actors[index_m].animTile;
                    GetMapViewport(viewX, viewY) = 0;
                }
                else if (D_5c5a_actors[index_m].tile == TILE_ACTOR_BARD)
                {
                    if (GetMapViewport(viewX, viewY) == TILE_MAP_CHAIR_92)
                    {
                        GetActorMap(viewX, viewY) = D_5c5a_actors[index_m].animTile;
                        GetMapViewport(viewX, viewY) = 0;
                    }
                    else
                    {
                        ULTIMA_51b8_PutViewActor(viewX, viewY, mapX_j, mapY_i, D_5c5a_actors[index_m].animTile - 8);
                    }
                }
                else
                {
                    ULTIMA_51b8_PutViewActor(viewX, viewY, mapX_j, mapY_i, D_5c5a_actors[index_m].animTile);
                }
            }
        }
    }
}

// Whether a combatant fights against the party: a monster that is not charmed, a party member who is. 0 for the dead.
int ULTIMA_5646_IsEnemy(int combatantIndex)
{
    int flags;
    int entityIndex;

    flags = D_ba14_combatants[combatantIndex].flags;
    entityIndex = D_ba14_combatants[combatantIndex].entityIdx;

    if ((flags & COMBAT_FLAGS_DEAD) != 0)
    {
        return 0;
    }

    // 566e
    if ((flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        if (entityIndex != 0 && D_55a8_party[entityIndex].name[4] == 'j')
        {
            return 1;
        }
        else
        {
            // 568e
            return (flags & COMBAT_FLAGS_CHARMED);
        }
    }
    else
    {
        // 5696
        return !(flags & COMBAT_FLAGS_CHARMED);
    }

    // 56a4
}

void ULTIMA_56ac_DrawMap(void)
{
    int row_f;
    int col;
    int pixelY;
    int pixelX;

    for (row_f = 0; row_f < 11; row_f++)
    {
        for (col = 0; col < 0xb; col++)
        {
            if (GetMapViewport(col, row_f) == 0)
            {
                if (GetActorMap(col, row_f) != 0x16)
                {
                    // draw actor
                    ULTIMA_10e0_GRAP_51_PutTile(0x100 + GetActorMap(col, row_f), col, row_f);
                }
            }
            else
            {
                if (GetMapViewport(col, row_f) == TILE_MAP_MOONGATE && D_5887_moongateHeight != 0 && D_5887_moongateHeight < 0x10)
                {
                    // moongate?
                    ULTIMA_1112_GRAP_60_Moongate(D_5887_moongateHeight, col, row_f);
                }
                else
                {
                    // draw map tile
                    ULTIMA_10e0_GRAP_51_PutTile(D_b11e_tileFrames[GetMapViewport(col, row_f)], col, row_f);
                }
            }
        }
    }

    if (0x7f < D_5893_mapId && (D_589f_focusBlink = !D_589f_focusBlink) != 0 && D_589e_curCombatant != 0xff && ULTIMA_5646_IsEnemy(D_589e_curCombatant) == 0)
    {
        pixelX = (uint)D_ba14_combatants[D_589e_curCombatant].x * 0x10 + 8;
        pixelY = (uint)D_ba14_combatants[D_589e_curCombatant].y * 0x10 + 8;

        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);

        for (col = 0; col < 2; col++)
        {
            for (row_f = 0; row_f < 0xf; row_f += 0xe)
            {
                ULTIMA_0c9c_GRAP_39_HorizLine(pixelX, pixelY + col + row_f, pixelX + 0xf);
                ULTIMA_0cf2_GRAP_3c_VertLine(pixelX + col + row_f, pixelY, pixelY + 0xf);
            }
        }

        if (D_5898_crosshairOn != 0)
        {
            // 5813
            // draw crosshair / focus
            pixelX = (uint)D_5899_crosshairX * 0x10 + 8; // cross hair x
            pixelY = (uint)D_589a_crosshairY * 0x10 + 8; // cross hair y

            // 586a
            for (col = 0; col < 2; col++)
            {
                for (row_f = 0; row_f < 2; row_f++)
                {
                    // 5887
                    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
                    ULTIMA_0b10_GRAP_Line(row_f * 0xb + pixelX + 2, col * 3 + pixelY + 6, row_f * 3 + pixelX + 6, col * 3 + pixelY + 6);
                    ULTIMA_0f90_GRAP_LineTo(row_f * 3 + pixelX + 6, col * 0xb + pixelY + 2);
                    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
                    ULTIMA_0b10_GRAP_Line(row_f * 0xb + pixelX + 2, col * 5 + pixelY + 5, row_f * 5 + pixelX + 5, col * 5 + pixelY + 5);
                    ULTIMA_0f90_GRAP_LineTo(row_f * 5 + pixelX + 5, col * 0xb + pixelY + 2);
                    ULTIMA_0b10_GRAP_Line(row_f * 0xb + pixelX + 2, pixelY + col + 7, row_f * 3 + pixelX + 6, pixelY + col + 7);

                    ULTIMA_0b10_GRAP_Line(pixelX + row_f + 7, col * 3 + pixelY + 6, pixelX + row_f + 7, col * 0xb + pixelY + 2);
                }
            }
        }
    }
}

// Animate / Update
void ULTIMA_5910_UpdateFrame(void)
{
    int col_b;
    int row;

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
                for (row = 0; row < 11; row++)
                {
                    for (col_b = 0; col_b < 11; col_b++)
                    {
                        if (GetMapViewport(col_b, row) == 0)
                        {
                            GetMapViewport(col_b, row) =
                                *ULTIMA_4402_GetTileAddr((byte)col_b + (uint)D_5896_mapX - 5, (byte)row + (uint)D_5897_mapY - 5);
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
