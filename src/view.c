// Resident code from CS:51a0 to 5a28: an object of its own, compiled with MSC 5.0 (PutActorsInView
// matches only under 5.0; 5a28, 5d0a and 5e4a only under 5.1, in visible.c).

#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "cmds.h"
#include "combat.h"
#include "dnglook.h"

// A random animation phase 0-3 (always 0 when the status icon is T).
static int RandomFrame(void)
{
    if (statusIcon == 'T')
    {
        return 0;
    }
    else
    {
        return RandomRange(0, 3);
    }
}

// Put actor shape on view cell (viewX,viewY), changed by the furniture under it (chairs, beds, ladders, mirrors); the view tile is cleared.
static void PutViewActor(int viewX, int viewY, int mapX, int mapY, int shape)
{
    int tileUnder;

    if (shape == 0x1c || shape >= 0x12 && shape < 0x16 || shape >= 0x40 || shape >= 0x28 && shape < 0x2c)
    {
        // 51e9
        tileUnder = *GetTileAddr(mapX, mapY);
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
                    shape = RandomFrame() + 0x60; // -> 5300
                    break;

                case TILE_MAP_85: // 133
                    // 5280
                    shape = RandomFrame() + 0x64; // -> 5300
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
                    shape = RandomFrame() + 0x3c;
                    break;

                case TILE_MAP_CHAIR_92: // 146
                    // 52aa
                    if ((*GetTileAddr(mapX, mapY + 1) == TILE_MAP_TABLE_9A) ||
                        (*GetTileAddr(mapX, mapY + 1) == TILE_MAP_TABLE_9C))
                    {
                        shape = RandomFrame() + 0x34;
                    }
                    else
                    {
                        shape = 0x32;
                    }
                    break;

                case TILE_MAP_CHAIR_90: // 144
                    // 52da
                    if ((*GetTileAddr(mapX, mapY - 1) == TILE_MAP_TABLE_9B) ||
                        (*GetTileAddr(mapX, mapY - 1) == TILE_MAP_TABLE_9C))
                    {
                        shape = RandomFrame() + 0x38;
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
                    if ((*GetTileAddr(mapX, mapY - 1) == TILE_MAP_MIRROR) && (viewY != 0))
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
// MSC 5.0 (this object is 5.0, visible.c after it 5.1): only 5.0 replaces the loop test with a
// pointer compare (cmp [bp-26h],5C5Bh; jb) while keeping the dec of index; 5.1 gives
// dec/jns. The loop must be a for: a do-while is never strength-reduced (13 pointer temporaries).
// Put the party and the actors on their cells of the view (sitting, sleeping and climbing shapes by the tile under them).
void PutActorsInView(void)
{
    int row;
    int index_m;
    byte viewY;
    byte viewX;

    byte mapY_i;
    byte mapX_j;

    if (mapId < 0x80)
    {
        actors[0].x = D_5896_mapX;
        actors[0].y = D_5897_mapY;
        actors[0].z = D_5895_mapLevel;
        actors[0].animTile = actors[0].tile = partyTile;
        
        for (row = 0; row < 0xb; row++)
        {
            for (index_m = 0; index_m < 0xb; index_m++)
            {
                if (GetMapViewport(index_m, row) == TILE_MAP_DD &&
                    DistanceFromCenter(index_m, row) > 5)
                {
                    GetMapViewport(index_m, row) = TILE_MAP_1C;
                }
                else if (GetMapViewport(index_m, row) == TILE_MAP_1C &&
                    DistanceFromCenter(index_m, row) <= 5)
                {
                    GetMapViewport(index_m, row) = TILE_MAP_DD;
                }
            }
        }
    }

    // 542a
    for (index_m = 31; index_m >= 0; index_m--)
    {
        if (actors[index_m].tile != 0)
        {
            viewX = mapX_j = actors[index_m].x;
            viewY = mapY_i = actors[index_m].y;

            if (mapId < 0x80)
            {
                viewX -= D_5896_mapX - 5;
                viewY -= D_5897_mapY - 5;
                if (actors[index_m].z != D_5895_mapLevel || viewX > 10 || viewY > 10)
                    continue;
            }

            if (actors[index_m].animTile != 0 &&
                GetMapViewport(viewX, viewY) != 0xff &&
                GetMapViewport(viewX, viewY) != TILE_MAP_87)
            {
                if ((actors[index_m].tile & 0xfc) == TILE_ACTOR_E8 || actors[index_m].tile == TILE_ACTOR_SLEEP || actors[index_m].tile == TILE_ACTOR_DEAD)
                {
                    // 5503
                    if (GetMapViewport(viewX, viewY) != 0)
                    {
                        GetActorMap(viewX, viewY) = actors[index_m].animTile;
                        GetMapViewport(viewX, viewY) = 0;
                    }
                }
                else if (actors[index_m].animTile == TILE_ACTOR_1D || actors[index_m].animTile == TILE_ACTOR_SLEEP)
                {
                    GetActorMap(viewX, viewY) = actors[index_m].animTile;
                    GetMapViewport(viewX, viewY) = 0;
                }
                else if (actors[index_m].tile == TILE_ACTOR_BARD)
                {
                    if (GetMapViewport(viewX, viewY) == TILE_MAP_CHAIR_92)
                    {
                        GetActorMap(viewX, viewY) = actors[index_m].animTile;
                        GetMapViewport(viewX, viewY) = 0;
                    }
                    else
                    {
                        PutViewActor(viewX, viewY, mapX_j, mapY_i, actors[index_m].animTile - 8);
                    }
                }
                else
                {
                    PutViewActor(viewX, viewY, mapX_j, mapY_i, actors[index_m].animTile);
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

    flags = combatants[combatantIndex].flags;
    entityIndex = combatants[combatantIndex].entityIdx;

    if ((flags & COMBAT_FLAGS_DEAD) != 0)
    {
        return 0;
    }

    // 566e
    if ((flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        if (entityIndex != 0 && party[entityIndex].name[4] == 'j')
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

void DrawMap(void)
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
                    GRAP_51_PutTile(0x100 + GetActorMap(col, row_f), col, row_f);
                }
            }
            else
            {
                if (GetMapViewport(col, row_f) == TILE_MAP_MOONGATE && moongateHeight != 0 && moongateHeight < 0x10)
                {
                    // moongate?
                    GRAP_60_Moongate(moongateHeight, col, row_f);
                }
                else
                {
                    // draw map tile
                    GRAP_51_PutTile(tileFrames[GetMapViewport(col, row_f)], col, row_f);
                }
            }
        }
    }

    if (0x7f < mapId && (focusBlink = !focusBlink) != 0 && curCombatant != 0xff && ULTIMA_5646_IsEnemy(curCombatant) == 0)
    {
        pixelX = (uint)combatants[curCombatant].x * 0x10 + 8;
        pixelY = (uint)combatants[curCombatant].y * 0x10 + 8;

        GRAP_2d_SetPenColor(brightWhiteColor);

        for (col = 0; col < 2; col++)
        {
            for (row_f = 0; row_f < 0xf; row_f += 0xe)
            {
                GRAP_39_HorizLine(pixelX, pixelY + col + row_f, pixelX + 0xf);
                GRAP_3c_VertLine(pixelX + col + row_f, pixelY, pixelY + 0xf);
            }
        }

        if (crosshairOn != 0)
        {
            // 5813
            // draw crosshair / focus
            pixelX = (uint)crosshairX * 0x10 + 8; // cross hair x
            pixelY = (uint)crosshairY * 0x10 + 8; // cross hair y

            // 586a
            for (col = 0; col < 2; col++)
            {
                for (row_f = 0; row_f < 2; row_f++)
                {
                    // 5887
                    GRAP_2d_SetPenColor(brightWhiteColor);
                    GRAP_Line(row_f * 0xb + pixelX + 2, col * 3 + pixelY + 6, row_f * 3 + pixelX + 6, col * 3 + pixelY + 6);
                    GRAP_LineTo(row_f * 3 + pixelX + 6, col * 0xb + pixelY + 2);
                    GRAP_2d_SetPenColor(0);
                    GRAP_Line(row_f * 0xb + pixelX + 2, col * 5 + pixelY + 5, row_f * 5 + pixelX + 5, col * 5 + pixelY + 5);
                    GRAP_LineTo(row_f * 5 + pixelX + 5, col * 0xb + pixelY + 2);
                    GRAP_Line(row_f * 0xb + pixelX + 2, pixelY + col + 7, row_f * 3 + pixelX + 6, pixelY + col + 7);

                    GRAP_Line(pixelX + row_f + 7, col * 3 + pixelY + 6, pixelX + row_f + 7, col * 0xb + pixelY + 2);
                }
            }
        }
    }
}

// Animate / Update
void UpdateFrame(void)
{
    int col_b;
    int row;

    lastDisk = 0xff;
    if (statusIcon == 'T')
    {
        animate = 0;
    }
    if (viewOn != 0)
    {
        if (animate != 0) // animate flag
        {
            if (animate != 0xff)
            {
                AnimateActors();
            }
            UpdateWindDirectionRandomly();
            if (mapId < 0x80)
            {
                UpdateMoongates();
                AnimateLighthouse();
            }
        }

        if (mapId < 0x80)
        {
            if (viewDirty != 0)
            {
                BuildView(D_58a5_lightLevel, (uint)D_5896_mapX - (uint)mapOriginX, (uint)D_5897_mapY - (uint)mapOriginY, 0xb);
                viewDirty = 0;
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
                                *GetTileAddr((byte)col_b + (uint)D_5896_mapX - 5, (byte)row + (uint)D_5897_mapY - 5);
                        }
                    }
                }
            }
        }
        else // map_id >= 0x80; combat?
        {
            memcpy(viewTiles, combatMap, 0x160);
        }

        PutActorsInView();
        DrawMap();
        if (animate != 0)
        {
            AudioPlayAmbientSfx();
        }
    }

    animate = 1;
}
