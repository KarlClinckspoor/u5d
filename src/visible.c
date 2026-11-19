// Resident code from CS:5a28 to 5f86: an object of its own, compiled with MSC 5.1, after the
// MSC 5.0 object view.c (RandomFrame to ULTIMA_5910) and before battle.c. Its data is the two
// tables at DS:6a86.

#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "cmds.h"
#include "combat.h"
#include "dnglook.h"
u8 opaqueTiles[0x13] = {
    0x09, 0x0A, 0x0C, 0x0D, 0x4D, 0x4E, 0x4F, 0x5A,
    0x97, 0xB8, 0xB9, 0xBC, 0xD0, 0xD1, 0xD2, 0xD3,
    0xF8, 0xFE, 0xFF
};

u8 lightTiles[10] = {
    0xDC, 0xBD, 0xBE, 0xB2, 0xDE, 0xBF, 0xB0, 0xB1,
    0xB3, 0xBC
};

static bool IsTransparentTile(byte tile, int distance);

// CHECKED
// The locals' suffixes (_aj, _ab ...) keep the original's slot order (rename.py).
// Two unused labels (L_s before the GetMapViewport store at 5b83, L_c before the 5dfe test)
// stop MSC keeping an address or originY + nextY in DI across them, which gives the
// original's [bx+di+table] addressing; the gotos to L_5c93 put that block before 5c9c.
// The queue is 256 ints, not 257: `register byte* cell` (SI) at function level reserves the
// unused slot at -4 between tail and the queue (cell hashes to bucket 0, declared before
// tail). As a block variable its slot came after the locals and moved the temporary.
// generate visibility map (BFS)
static void BuildVisibilityMap(int lightLevel, int originY, int originX, int rowOffset, int colOffset, uint stride, byte* map)
{
    int dir; // -0x216
    byte tile;
    int nextY; // -0x212; y
    int nextX; // -0x210; x
    int combatMode_h; // -0x20e
    int head; // -0x20c
    int valid; // -0x20a
    int fromY_d; // -0x208
    int fromX_e; // -0x206
    int queue_l[256]; // -0x204
    register byte* cell;
    int tail_g;

    head = 0;
    tail_g = 0;
    dir = 7;
    valid = 1;
    combatMode_h = 0;

    if (lightLevel > 0)
    {
        // 5a57
        if (rowOffset == -0x6f)
        {
            rowOffset = 0;
            combatMode_h++;
        }

        // 5a66 (si vs di)
        lightLevel++;
        queue_l[tail_g] = 5;
        queue_l[++tail_g] = 5;
        ++tail_g;
        map[rowOffset * 0x20 + colOffset + 0xa5] =
            *ULTIMA_4402_GetTileAddr(originX + (uint)D_589b_mapOriginX + 5, originY + (uint)D_589c_mapOriginY + 5);

        while (head != tail_g)
        {
            // 5abc
            fromY_d = nextY = queue_l[head];
            fromX_e = nextX = queue_l[++head];
            ++head;

            while (dir > -1) // 5ce5
            {
                // 5aec
                switch (dir)
                {
                case 0:
                case 1:
                case 7:
                    nextX--;
                    break;
                case 4:
                case 5:
                    nextX++;
                    break;
                case 2:
                case 3:
                    nextY--;
                    break;
                case 6:
                    nextY++;
                    break;
                }

                if (nextX < 0 || 10 < nextX || nextY < 0 || 10 < nextY)
                {
                    // 5b42
                    valid = 0;
                }

                // 5b48
                cell = map + (rowOffset + nextY) * 0x20 + (colOffset + nextX);

                // 5b5f
                if (combatMode_h == 0)
                {
                    // 5b66
                    if ((GetMapViewport(nextX, nextY) != 0) && (0x1f >= rowOffset + nextY))
                    {
                    L_s:
                        GetMapViewport(nextX, nextY) = 0;

                        // -> 5b9b
                    }
                    else
                    {
                        // 5b95
                        valid = 0;
                    }
                }
                else
                {
                    // 5b90
                    if (*cell != 0xff)
                    {
                        // 5b95
                        valid = 0;
                    }
                }

                // 5b9b
                if (valid != 0)
                {
                    // 5ba5
                    tile = *ULTIMA_4402_GetTileAddr(originX + nextX + (uint)D_589b_mapOriginX, originY + nextY + (uint)D_589c_mapOriginY);
                    if (DistanceFromCenter(nextX, nextY) >= lightLevel)
                    {
                        if (combatMode_h)
                        {
                        L_c:
                            if (IsTransparentTile(tile, DistanceFromCenter(nextX, nextY)) == 0)
                            {
                                // 5c05
                                if (
                                    map[fromY_d * 0x20 + fromX_e] == 0 ||
                                    combatMap[(originY + fromY_d) * 0x20 + originX + fromX_e] == 0 ||
                                    combatMap[(originY + nextY) * 0x20 + originX + nextX] == 0)
                                {
                                    tile = *cell = 0xff;
                                }
                                else
                                    goto L_5c93;
                            }
                            else
                            {
                                if (originY + nextY < 0 || originX + nextX < 0 ||
                                    originX + nextX > 0x1f || originY + nextY > 0x1f)
                                {
                                    *cell = 0;
                                }
                                else if (combatMap[(originY + nextY) * 0x20 + originX + nextX] != 0)
                                {
                                L_5c93:
                                    *cell = tile;
                                }
                                else
                                {
                                    *cell = 0;
                                }
                            }
                        }
                        else
                        {
                            tile = 0xff;
                        }
                    }
                    else
                        goto L_5c93;

                    // 5ca1
                    if (IsTransparentTile(tile, DistanceFromCenter(nextX, nextY)) != 0)
                    {
                        queue_l[tail_g] = nextY;
                        tail_g++;
                        queue_l[tail_g] = nextX;
                        tail_g++;
                    }
                }

                // 5cdb
                dir--;
                valid = 1;
            }

            // 5cef
            dir = 7;
        }
    }

    // 5d01
}

// update map data?
// light: light intensity
// Build the 11x11 view around map cell (x, y) for the light level: what the party can see.
void BuildView(int light, int mapX, int mapY, int size)
{
    int originX;
    int originY;
    byte* cell_c;
    int col_f;
    int row;

    for (col_f = 0; col_f < 0xb; col_f++)
    {
        memset(&GetMapViewport(0, col_f), 0xff, 0xb);
    }

    col_f = mapX - 5;
    row = mapY - 5;

    if (light > 0)
    {
        BuildVisibilityMap(light, row, col_f, -111, 0, 0x20, viewTiles);
        cell_c = viewTiles;

        for (row = 0; row < 0xb; row++)
        {
            for (col_f = 0; col_f < 0xb; col_f++)
            {
                if (*cell_c == 0)
                {
                    (*cell_c)--;
                }

                cell_c++;
            }

            cell_c += 0x15;
        }
    }

    if (light < 0)
    {
        originX = mapX - 5;
        originY = mapY - 5;

        for (row = 0; row < 0xb; row++)
        {
            for (col_f = 0; col_f < 0xb; col_f++)
            {
                GetMapViewport(col_f, row) = *ULTIMA_4402_GetTileAddr(
                    originX + col_f + (uint)D_589b_mapOriginX,
                    originY + row + (uint)D_589c_mapOriginY);
            }
        }
    }
}

// Probably: the tile does not block sight (not in opaqueTiles); the door-like tiles only when distance is 1.
static bool IsTransparentTile(byte tile, int distance)
{
    if (tile == TILE_MAP_4B || tile == TILE_MAP_4A || tile == TILE_MAP_DOOR_BA || tile == TILE_MAP_DOOR_BB || tile == TILE_MAP_98)
    {
        if (distance == 1)
            return 1;
        else
            return 0;
    }
    else
    {
        if (!memchr(opaqueTiles, tile, 0x13))
            return 1;
        else
            return 0;
    }
}

// Build the 32x32 light map: the cells lit by the lamps, fires and moongates of the map buffer.
void ULTIMA_5e4a_BuildLightMap(void)
{
    byte tile;
    int count;
    int lights_g[64];
    int lightY;
    int lightX;
    int row_i;
    int scanY_c;
    int scanX_d;
    u8* cell;

    count = 0;
    cell = combatMap;

    memset(combatMap, 0xff, 0x400);

    // 5e72
    for (scanY_c = 0; scanY_c < 0x20; scanY_c++)
    {
        for (scanX_d = 0; scanX_d < 0x20; scanX_d++)
        {
            // 5e78
            tile = *ULTIMA_4402_GetTileAddr(D_589b_mapOriginX + scanX_d, D_589c_mapOriginY + scanY_c);
            if (memchr(lightTiles, tile, 10) != 0)
            {
                // 5eb0
                lights_g[count++] = scanY_c;
                lights_g[count++] = scanX_d;
                *cell = tile;
            }

            // 5ed2
            cell++;
        }
    }

    scanX_d = count;
    scanY_c = 0;

    // 5ef5
    if (count > 0)
    {
        do
        {
            // 5efe
            lightY = lights_g[scanY_c++] - 5;
            lightX = lights_g[scanY_c] - 5;

            // 5f23
            for (row_i = 0; row_i < 0xb; row_i++)
            {
                // 5f29
                memset(&GetMapViewport(0, row_i), 0xff, 0xb);
            }

            // 5f3f
            BuildVisibilityMap(10, lightY, lightX, lightY, lightX, 0x20, combatMap);
            scanY_c++;
        } while (scanY_c < scanX_d);
    }

    // 5f65
    cell = combatMap;

    for (scanX_d = 0; scanX_d < 0x400; scanX_d++)
    {
        if (*cell == 0xff)
        {
            (*cell)++;
        }
        cell++;
    }
}

void TilesetMode(int mode);

void SetUpCombat(int flags, int enemy);
