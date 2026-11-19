#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "npc.h"
#include "town.h"

static int SearchPath(int a, int b, int c, int d, int e, int f);
static int NpcDistance(int fromX, int fromY, int toX, int toY);
static int CanEnter(int x, int y, int z, int npc, int slot);
static int NpcCanStepTo(int x, int y, int npc, int slot);
u8 npcBlockedTiles[0x20] = {
    0x70, 0x0c, 0xff, 0xf8, 0x01, 0xf3, 0xff, 0xff,
    0x72, 0x3f, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff,
    0xfe, 0xff, 0x0f, 0xff, 0xff, 0xdf, 0xff, 0x5f,
    0xf0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

// Load the NPC file of the current town (towne.npc, dwelling.npc, castle.npc, keep.npc, 8 maps each): the schedules (0x200 bytes), the NPC types (0x20) and the dialog numbers (0x20, into NpcFmt.dialogNumber).
void LoadNpcFile(void)
{
    int fileOffset;
    int mapIndex;
    byte dialogNumbers_d[32];
    char* fileName;
    int kind_j;

    D_5893_mapId--;

    kind_j = (uint)D_5893_mapId >> 3;
    switch (kind_j)
    {
    case 0:
        fileName = /*0x6d46*/ "towne.npc";
        break;
    case 1:
        fileName = /*0x6d50*/ "dwelling.npc";
        break;
    case 2:
        fileName = /*0x6d5e*/ "castle.npc";
        break;
    case 3:
        fileName = /*0x6d6a*/ "keep.npc";
        break;
    }

    mapIndex = (uint)D_5893_mapId - (kind_j << 3);

    fileOffset = mapIndex * 0x240;
    ULTIMA_256e_ReadFileFromDisk(fileName, D_5d5e_npcSchedules, 0x200, fileOffset);
    ULTIMA_256e_ReadFileFromDisk(fileName, D_659e_npcTypes, 0x20, fileOffset + 0x200);
    ULTIMA_256e_ReadFileFromDisk(fileName, dialogNumbers_d, 0x20, fileOffset + 0x220);

    for (kind_j = 0; kind_j < 0x20; kind_j++)
    {
        D_5f5e_npcs[kind_j].dialogNumber = (u16)dialogNumbers_d[kind_j];
    }

    D_5893_mapId++;
}

// On entering a town: every NPC with a type appears at the place of the schedule entry in force at the hour, mode 1, with an empty move list.
void NPC_00d6_PlaceNpcs(int hour)
{
    int slot;
    int npc;

    for (npc = 1; npc < 0x20; npc++)
    {
        if (D_659e_npcTypes[npc] == 0)
        {
            D_5f5e_npcs[npc].mode = 0;
        }
        else
        {
            slot = ScheduleSlot(npc, hour);
            D_5f5e_npcs[npc].mode = 1;

            D_5f5e_npcs[npc].x = D_5d5e_npcSchedules[npc].x[slot];
            D_5f5e_npcs[npc].y = D_5d5e_npcSchedules[npc].y[slot];
            D_5f5e_npcs[npc].z = D_5d5e_npcSchedules[npc].z[slot];

            D_5f5e_npcs[npc].npcType = D_659e_npcTypes[npc];
            D_5f5e_npcs[npc].scheduleIdx = slot;
            D_5f5e_npcs[npc].actorIdx = 0;

            moveListPos[npc] = -1;
            moveLists[npc].data[0] = 0;
            npcStuckCount[npc] = 0;
        }
    }
}

// Search for the nearest ladder (up when wantUp, else down) from (x, y) for an NPC; the ladder's tile ends in D_5876/D_5878. Returns 1 when found.
static int FindLadder(int x, int y, int wantUp, int npc)
{
    int mode;

    if (wantUp != 0)
    {
        mode = -1;
    }
    else
    {
        mode = -2;
    }

    return SearchPath(x, y, 0, 0, mode, npc);
}

// Build the search grid searchGrid for NPC_032c: 0 for a free tile, 0x90 blocked (also near other actors), 5 at the goal (or at a ladder when mode < 0), 0x46 at the start.
// CHECKED (loop)
static void BuildSearchGrid(int startX, int startY, int goalX, int goalY, int mode, int npc)
{
    byte ladderTile;
    ActorFmt* actor;
    int slot;
    int x_i;
    int y_h;
    NpcFmt* npcRec_e;

    if (mode == -1)
    {
        ladderTile = 0xc8;
    }

    if (mode == -2)
    {
        ladderTile = 0xc9;
    }

    slot = ScheduleSlot(npc, D_587f_hour);
    for (y_h = 0; y_h < 0x20; y_h = y_h + 1)
    {
        for (x_i = 0; x_i < 0x20; x_i = x_i + 1)
        {
            if (CanEnter(x_i, y_h, D_5895_mapLevel, npc, slot) != 0)
            {
                searchGrid[x_i + y_h * 0x20] = 0;
            }
            else
            {
                searchGrid[x_i + y_h * 0x20] = 0x90;
            }

            if (mode < 0 && GetMap(x_i, y_h) == ladderTile)
            {
                searchGrid[x_i + y_h * 0x20] = 5;
            }
        }
    }

    // 0272
    npcRec_e = &D_5f5e_npcs[npc];

    for (y_h = 0x1f; y_h > 0; --y_h)
    {
        actor = &D_5c5a_actors[y_h];
        if (actor->tile > TILE_ACTOR_WIZARD - 1)
        {
            if (NpcDistance(actor->x, actor->y, npcRec_e->x, npcRec_e->y) < 4)
            {
                searchGrid[actor->y * 0x20 + actor->x] = 0x90;
            }
        }
    }

    actor = &D_5c5a_actors[0];
    if (NpcDistance(actor->x, actor->y, npcRec_e->x, npcRec_e->y) < 4)
    {
        searchGrid[actor->y * 0x20 + actor->x] = 0x90;
    }

    if (mode > -1)
    {
        searchGrid[goalY * 0x20 + goalX] = 5;
    }

    searchGrid[startY * 0x20 + startX] = 0x46;
}

// Breadth-first path search over the grid searchGrid from (startX, startY), marking the direction taken in the high nibble of each tile. Returns 1 when the goal is reached.
// CHECKED
static int SearchPath(int startX, int startY, int goalX, int goalY, int mode, int npc)
{
    byte queueY[32];
    int dir;
    byte queueX[32];
    int state;
    int head;
    int x_i;
    int y_h;
    int turn_g;
    int tail_f;

    state = 0;
    head = 0;
    tail_f = 1;

    BuildSearchGrid(startX, startY, goalX, goalY, mode, npc);

    queueX[0] = startX;
    queueY[0] = startY;

    for (;;)
    {
        if (state != 0)
            goto L_brk;

        x_i = queueX[head];
        y_h = queueY[head];
        dir = searchGrid[y_h * 0x20 + x_i] >> 4;

        for (turn_g = 0; turn_g < 4; turn_g++)
        {
            switch (dir)
            {
            case 1:
                x_i--;
                if (x_i < 0)
                {
                    state = 2;
                }
                break;

            case 2:
                y_h++;
                if (y_h > 0x20)
                {
                    state = 2;
                }
                break;

            case 3:
                x_i++;
                if (x_i > 0x20)
                {
                    state = 2;
                }
                break;

            case 4:
                y_h--;
                if (y_h < 0)
                {
                    state = 2;
                }
                break;

            default:
                break;
            }

            if (state != 2)
            {
                if (searchGrid[y_h * 0x20 + x_i] < 0x10)
                {
                    state = searchGrid[y_h * 0x20 + x_i] & 0xf;
                    searchGrid[y_h * 0x20 + x_i] = (byte)(dir << 4);
                    if (state == 5)
                    {
                        state = 1;
                        D_5876_tempX = x_i;
                        D_5878_tempY = y_h;
                        break;
                    }

                    if (tail_f != head)
                    {
                        queueX[tail_f] = x_i;
                        queueY[tail_f] = y_h;
                        tail_f++;
                    }

                    if (tail_f >= 0x20)
                    {
                        tail_f = 0;
                    }
                }
            }

            state = 0;
            dir = (dir & 3) + 1;
            x_i = queueX[head];
            y_h = queueY[head];
        }

        if (state == 1)
        {
        L_brk:
            break;
        }

        head++;
        if (head == 0x20)
        {
            head = 0;
        }
        if (head != tail_f)
            continue;
        break;
    }

    // The exit stores land at the first break of a for loop, and the code after the loop
    // follows them there: the original's are at the "== 1" break, so the first test
    // jumps to it with a goto and the loop ends with continue/break (not do-while).
    if (state != 0)
        return 1;
    return state;
}

// Turn the direction marks of the search into the NPC's move list: runs of (steps, direction) walked back from the goal, then reversed in place. Returns the length.
// CHECKED
static int BuildMoveList(int npc, int mode, int x, int y)
{
    int runDir;
    int length;
    int cellState;
    int run_k;
    uint dir_a;

    run_k = 0;
    length = 0;

    moveListPos[npc] = 0;

    cellState = searchGrid[y * 0x20 + x] & 0xf;
    dir_a = searchGrid[y * 0x20 + x] >> 4;

    runDir = dir_a;

    do
    {
        switch (dir_a)
        {
        case 1:
            x++;
            break;
        case 2:
            y--;
            break;
        case 3:
            x--;
            break;
        case 4:
            y++;
            break;
        }

        if (runDir == dir_a && cellState != 6)
        {
            run_k++;
        }

        if (runDir != dir_a || cellState == 6)
        {
            moveLists[npc].data[length] = run_k;
            length++;
            moveLists[npc].data[length] = runDir;
            length++;

            if (cellState == 6)
                break;

            runDir = dir_a;
            run_k = 1;
        }

        dir_a = searchGrid[y * 0x20 + x] >> 4;
        cellState = searchGrid[y * 0x20 + x] & 0xf;
    } while (length < 0x20);

    run_k = 0;
    cellState = length - 2;

    if (cellState >= 0)
    {
        do
        {
            // swap
            runDir = moveLists[npc].data[run_k];
            moveLists[npc].data[run_k] = moveLists[npc].data[cellState];
            moveLists[npc].data[cellState] = runDir;

            cellState++;
            run_k++;

            // swap*
            runDir = (moveLists[npc].data[run_k] + 1 & 3) + 1;
            moveLists[npc].data[run_k] = (moveLists[npc].data[cellState] + 1 & 3) + 1;
            moveLists[npc].data[cellState] = runDir;

            run_k++;
            cellState -= 3;
        } while (cellState >= run_k);
    }

    return length;
}

// Move (D_5876, D_5878) one tile in a direction: 1 east, 2 north, 3 west, 4 south.
// set walk direction
static void StepTemp(int dir)
{
    switch (dir)
    {
    case 1:
        D_5876_tempX++;
        if (D_5878_tempY > 0x20)
        {
            D_5876_tempX = 0x20;
        }
        break;

    case 2:
        D_5878_tempY--;
        if (D_5876_tempX < 0)
        {
            D_5878_tempY = 0;
        }
        break;

    case 3:
        D_5876_tempX--;
        if (D_5878_tempY < 0)
        {
            D_5876_tempX = 0;
        }
        break;

    case 4:
        D_5878_tempY++;
        if (D_5876_tempX > 0x20)
        {
            D_5878_tempY = 0x20;
        }
        break;
    }
}

// Manhattan distance of two tiles.
// Calculate manhattan distance
static int NpcDistance(int fromX, int fromY, int toX, int toY)
{
    int dy;
    int dx;

    dx = fromX - toX;
    dy = fromY - toY;

    if (dx < 0)
    {
        dx = -dx;
    }

    if (dy < 0)
    {
        dy = -dy;
    }

    dx += dy;
    return dx;
}

// Move an NPC one step closer to the Avatar (away when its schedule type is 3, with a random side step for types 5 and 7); next to him a type 4/5 NPC with something to say asks to talk (npcApproach = 't'), the others attack (npcApproach = 'a').
// CHECKED
// Both early branches end in "return" and the move code follows the if (no else): the
// 0x61 branch's tail-merged copy ("npcApproach = 0x61; jmp") then lands inside the first move
// loop, after the then branch's jump, as in the original (as an if/else with the move code
// in the else, it stays after the 4/5 branch).
static void PursueOrFlee(int npc, int slot)
{
    int alternative_n;
    int best;
    ActorFmt* avatar_d;
    int dir_d;
    int newY;
    int newX;
    int distances[4];
    NpcFmt* npcRec_e;
    int distance_e;
    int type_n;


    avatar_d = &D_5c5a_actors[0];
    npcRec_e = &D_5f5e_npcs[npc];
    type_n = D_5d5e_npcSchedules[npc].type[slot];
    distance_e = NpcDistance(D_5c5a_actors[0].x, D_5c5a_actors[0].y, npcRec_e->x, npcRec_e->y);
    if (distance_e == 1 && type_n > 3)
    {
        if (type_n == 4 || type_n == 5)
        {
            if (npcRec_e->dialogNumber != 0)
            {
                npcApproach = 0x74;
                npcApproaching = npc;
                type_n = 1;
                return;
            }
        }
        else
        {
            npcApproach = 0x61;
            npcApproaching = npc;
            type_n = 1;
            return;
        }
    }
    // a9ea
    for (dir_d = 1; dir_d < 5; dir_d++)
    {
        // aa09
        D_5876_tempX = npcRec_e->x;
        D_5878_tempY = npcRec_e->y;
        StepTemp(dir_d);
        newX = D_5876_tempX;
        newY = D_5878_tempY;
        if (NpcCanStepTo(D_5876_tempX, D_5878_tempY, npc, -1) != 0)
        {
            distances[dir_d - 1] = NpcDistance(avatar_d->x, avatar_d->y, newX, newY);
        }
        else
        {
            // aa56 (OK)
            distances[dir_d - 1] = 99;
        }
    }

    // aa6a (OK)
    best = -1; // di
    distance_e = NpcDistance(avatar_d->x, avatar_d->y, npcRec_e->x, npcRec_e->y);
    // aa8c
    for (dir_d = 1; dir_d < 8; dir_d++)
    {
        // aaa7
        if (dir_d < 5 && distances[dir_d - 1] != 99)
        {
            if (type_n == 3)
            {
                if (distance_e < distances[dir_d - 1])
                {
                    if (best == -1 || ULTIMA_2092_RandomRange(0, 1) != 0)
                    {
                        best = dir_d;
                    }

                    if (dir_d == 4)
                        break;
                }
            }
            else
            {
                // ab14
                if (distances[dir_d - 1] < distance_e)
                {
                    best = dir_d;
                    break;
                }
            }
        }

        // aae4 (OK)
        if (dir_d > 4 && distances[dir_d - 1] != 99) // BUG? (OOB, invalid index)
        {
            if (distances[dir_d - 5] == distance_e)
            {
                best = dir_d;
                break;
            }
        }
    }

    // ab22 (OK below)
    if ((type_n == 5 || type_n == 7) && ULTIMA_2092_RandomRange(0, 0x3f) < 0x10)
    {
        // ab3d
        alternative_n = best;
        for (dir_d = 1; dir_d < 5; dir_d++)
        {
            // ab4b
            if (dir_d != best && distances[dir_d - 1] != 99 &&
                (alternative_n == best || ULTIMA_2092_RandomRange(0, 0x3f) < 0x10))
            {
                alternative_n = dir_d;
            }
        }
        best = alternative_n;
    }

    if (best > -1)
    {
        D_5876_tempX = npcRec_e->x;
        D_5878_tempY = npcRec_e->y;
        StepTemp(best);

        D_5c5a_actors[npcRec_e->actorIdx].x = npcRec_e->x = D_5876_tempX;
        D_5c5a_actors[npcRec_e->actorIdx].y = npcRec_e->y = D_5878_tempY;
        D_24e6_viewDirty |= 2;
    }
}

// An NPC's schedule time has come: choose its mode (1 there, 2 walk, 4/5 and 6/7 change level, 8 elsewhere); returns nonzero when it has to move.
// CHECKED
static int ScheduleDue(int npc, int hour)
{
    NpcFmt* npcRec;
    int slot_k;
    int i;
    NpcScheduleFmt* schedule_b;

    slot_k = -1;
    npcRec = &D_5f5e_npcs[npc];
    schedule_b = &D_5d5e_npcSchedules[npc];

    for (i = 0; i < 4; i++)
    {
        if (schedule_b->time[i] == hour)
        {
            slot_k = ScheduleSlot(npc, hour);
            if (slot_k == (int)npcRec->scheduleIdx)
            {
                npcRec->mode = 1;
                break;
            }

            if (npcRec->z != D_5895_mapLevel && schedule_b->z[slot_k] != D_5895_mapLevel)
            {
                npcRec->mode = 8;
            }
            else
            {
                if (npcRec->z == D_5895_mapLevel)
                {
                    if (schedule_b->z[slot_k] == D_5895_mapLevel)
                    {
                        npcRec->mode = 2;
                    }
                    else if ((char)D_5895_mapLevel < (char)schedule_b->z[slot_k])
                    {
                        npcRec->mode = 6;
                    }
                    else
                    {
                        npcRec->mode = 7;
                    }
                }
                else if ((char)D_5895_mapLevel < (char)npcRec->z)
                {
                    npcRec->mode = 4;
                }
                else
                {
                    npcRec->mode = 5;
                }
            }
            break;
        }
    }

    // ac1b
    if (-1 < slot_k && npcRec->x == (uint)schedule_b->x[slot_k] &&
        npcRec->y == (uint)schedule_b->y[slot_k] &&
        npcRec->z == (uint)schedule_b->z[slot_k])
    {
        slot_k = 0;
        npcRec->mode = 1;
    }

    if (slot_k == -1)
    {
        slot_k = 0;
    }
    else
    {
        slot_k++;
    }

    return slot_k;
}

// 1 when the NPC stands on the ladder or stairs it needs to reach the level of its schedule entry.
static int StandsOnStairs(int npc, int slot)
{
    byte tile;
    int onStairs;
    NpcFmt* npcRec_e;

    npcRec_e = &D_5f5e_npcs[npc];

    // 0a4a
    tile = *ULTIMA_4402_GetTileAddr(D_5c5a_actors[npcRec_e->actorIdx].x, D_5c5a_actors[npcRec_e->actorIdx].y);
    if ((char)D_5d5e_npcSchedules[npc].z[slot] < (char)D_5895_mapLevel)
    {
        if (tile != TILE_MAP_LADDER_DOWN)
        {
            // 0a92
            if ((tile & 0xf4) == TILE_MAP_STAIR)
            {
                onStairs = 1;
            }
            else
            {
                onStairs = 0;
            }
        }
        else
        {
            // 0a9b
            onStairs = 1;
        }
    }
    else
    {
        // 0aa2
        tile = *ULTIMA_4402_GetTileAddr(D_5c5a_actors[npcRec_e->actorIdx].x, D_5c5a_actors[npcRec_e->actorIdx].y);
        if (tile == TILE_MAP_LADDER_UP)
        {
            // -> 0a9b
            onStairs = 1;
        }
        else
        {
            if ((tile & 0xf4) == TILE_MAP_STAIR)
            {
                onStairs = 1;
            }
            else
            {
                onStairs = 0;
            }
        }
    }

    return onStairs;
}

// The tile (x, y, z) for NPC npc: 2 when it is the schedule entry's place, 1 when free by D_367e (or a ladder in mode 3), else 0.
static int CanEnter(int x, int y, int z, int npc, int slot)
{
    byte tile;
    int result;
    NpcScheduleFmt* schedule_b;

    result = 0;
    schedule_b = &D_5d5e_npcSchedules[npc];

    if (slot > -1 && schedule_b->x[slot] == x && schedule_b->y[slot] == y && schedule_b->z[slot] == z)
    {
        result = 2;
    }
    else
    {
        if (x < 0 || y < 0 || 0x1f < x || 0x1f < y)
        {
            tile = dummyCell;
        }
        else
        {
            tile = GetMap(x, y);
        }

        if (D_5f5e_npcs[npc].mode == 3 && (tile == TILE_MAP_LADDER_UP || tile == TILE_MAP_LADDER_DOWN))
        {
            result = 1;
        }
        else
        {
            result = ((uint)npcBlockedTiles[tile >> 3] & 0x80 >> (tile & 7)) == 0;
        }
    }

    return result;
}

// 1 (or 2 at the goal) when NPC npc may step on (x, y): on the map, free, no actor there; chairs only when sitting, type 0xfc anywhere.
static int NpcCanStepTo(int x, int y, int npc, int slot)
{
    int tile;
    int actorHere_b;
    int result;

    actorHere_b = 0;
    if (x < 0 || x > 0x1f || y < 0 || y > 0x1f)
    {
        return 0;
    }

    tile = *ULTIMA_4402_GetTileAddr(x, y) & 0xfc;
    if (D_659e_npcTypes[npc] == 0xfc)
    {
        result = 1;
    }
    else if (tile == TILE_MAP_30)
    {
        result = 1;
    }
    else if (tile == TILE_MAP_CHAIR_90 && D_5f5e_npcs[npc].mode != 2)
    {
        result = 0;
    }
    else
    {
        result = CanEnter(x, y, D_5895_mapLevel, npc, slot);
    }

    actorHere_b = ULTIMA_3702_FindLastActorTileAt(x, y, D_5895_mapLevel);
    if (result == 0 || actorHere_b != 0)
    {
        result = 0;
    }

    return result;
}

// One random step of an NPC (one time in two), kept within maxDist of its schedule entry's place when maxDist is not 0.
// move npc randomly
static void Wander(NpcFmt* npcRec, int maxDist, int slot, int npc, NpcScheduleFmt* schedule)
{
    int y;
    int x;
    int dir_a;

    if ((ULTIMA_2092_RandomRange(0, 0xff) & 8) != 0)
    {
        dir_a = Random(0x40);
        dir_a = (dir_a & 3) + 1;
        D_5876_tempX = npcRec->x;
        D_5878_tempY = npcRec->y;
        StepTemp(dir_a);
        x = D_5876_tempX;
        y = D_5878_tempY;

        if ((maxDist == 0 || NpcDistance(schedule->x[slot], schedule->y[slot], D_5876_tempX, D_5878_tempY) <= maxDist) &&
            NpcCanStepTo(D_5876_tempX, D_5878_tempY, npc, slot) != 0)
        {
            D_5c5a_actors[npcRec->actorIdx].x = npcRec->x = x;
            D_5c5a_actors[npcRec->actorIdx].y = npcRec->y = y;

            D_24e6_viewDirty |= 2;
        }
    }
}

// Act by the type of the schedule entry: stand still, wander near or freely, follow, pursue or flee the Avatar.
// CHECKED (switch order)
static void ActBySchedule(int npc, int slot)
{
    NpcScheduleFmt* schedule;
    NpcFmt* npcRec;

    npcRec = &D_5f5e_npcs[npc];
    schedule = &D_5d5e_npcSchedules[npc];

    switch (schedule->type[slot])
    {
    case 0:
        break;

    case 2:
        Wander(npcRec, 0, slot, npc, schedule);
        break;

    case 4:
        if (NpcDistance(D_5896_mapX, D_5897_mapY, schedule->x[slot], schedule->y[slot]) >= 4)
        {
            Wander(npcRec, 3, slot, npc, schedule);
        }
        else
        {
            PursueOrFlee(npc, slot);
        }
        break;

    case 1:
        Wander(npcRec, 3, slot, npc, schedule);
        break;

    case 3:
    case 6:
        if (NpcDistance(D_5896_mapX, D_5897_mapY, npcRec->x, npcRec->y) < 4)
        {
            PursueOrFlee(npc, slot);
        }
        break;

    case 5:
    case 7:
        PursueOrFlee(npc, slot);
        break;
    }
}

// Move every NPC of the town one turn: schedule changes, level changes by ladder or stairs, walking along the move list, wandering.
// CHECKED
// move npc
// The loop body tests "mode > 1 || ScheduleDue" first, with the 4/5 and 6/7 cases ahead of the
// <= 3 case and the "mode <= 1" case last (the original's layout follows this order). The
// original compares mode signed: *(s16*)& gives the CSE in SI for > 3, == 4, == 5, where an
// (int) cast compares through AX. The SearchPath success path (119e) is an if/else, not "npcStuckCount = 0; continue;":
// a continue there pulls the loop increment after it and the 10d6 store's tail merge goes
// the wrong way.
void MoveNpcs(int hour)
{
    int pathSearched;
    byte tile;
    int result_j;
    int slot_g;
    int y;
    int x;
    int mode;
    int npc;
    NpcFmt* npcRec_k;

    pathSearched = 0;
    npcApproach = 0;
    npcApproaching = 0;

    for (npc = 1; npc < 0x20; npc++)
    {
        // 126d
        if (D_659e_npcTypes[npc] == 0)
            continue;

        slot_g = ScheduleSlot(npc, hour);
        npcRec_k = &D_5f5e_npcs[npc];

        // 128e
        if ((int)npcRec_k->mode > 1 || ScheduleDue(npc, hour) != 0)
        {
            // 12a2
            if (*(s16*)&npcRec_k->mode > 3)
            {
                if (*(s16*)&npcRec_k->mode == 4 || *(s16*)&npcRec_k->mode == 5)
                {
                    // 12bc
                    if (pathSearched != 1)
                    {
                        pathSearched = 1;
                        if (npcRec_k->mode == 4)
                        {
                            mode = 3;
                        }
                        else
                        {
                            // 0dd4
                            mode = 4;
                        }

                        // 0dd9
                        result_j =
                            FindLadder(D_5d5e_npcSchedules[npc].x[slot_g], D_5d5e_npcSchedules[npc].y[slot_g], mode == 3, npc);
                        if (result_j != 0)
                        {
                            x = D_5876_tempX;
                            y = D_5878_tempY;
                            result_j = SearchPath(x, y, D_5d5e_npcSchedules[npc].x[slot_g],
                                                D_5d5e_npcSchedules[npc].y[slot_g], mode, npc);
                        }

                        // 0e32
                        if (result_j != 0)
                        {
                            BuildMoveList(npc, mode, D_5d5e_npcSchedules[npc].x[slot_g], D_5d5e_npcSchedules[npc].y[slot_g]);

                            tile = GetMap(x, y);
                            if ((mode == 3 && tile == TILE_MAP_LADDER_UP) ||
                                (mode == 4 && tile == TILE_MAP_LADDER_DOWN) ||
                                (tile & 0xfc) == TILE_MAP_STAIR)
                            {
                                PlaceNpcActor(npc, x, y, D_5895_mapLevel);
                            }

                            npcRec_k->mode = 2;
                        }
                    }
                }
                // 0ea6
                else if ((npcRec_k->mode != 6 && npcRec_k->mode != 7) || StandsOnStairs(npc, slot_g) != 0)
                {
                    // 0ebf
                    PlaceNpcActor(npc, D_5d5e_npcSchedules[npc].x[slot_g], D_5d5e_npcSchedules[npc].y[slot_g],
                              D_5d5e_npcSchedules[npc].z[slot_g]);

                    npcRec_k->scheduleIdx = slot_g;
                    moveListPos[npc] = -1;
                    npcRec_k->mode = 1;
                }
                else
                {
                    // 0efe
                    if (npcRec_k->mode != 2 && pathSearched != 1)
                    {
                        pathSearched = 1;
                        if (npcRec_k->mode == 6)
                        {
                            mode = 1;
                        }
                        else
                        {
                            mode = 2;
                        }
                        // 0f2b
                        result_j = FindLadder(npcRec_k->x, npcRec_k->y, mode == 1, npc);
                        if (result_j != 0)
                        {
                            x = D_5876_tempX;
                            y = D_5878_tempY;
                            result_j = SearchPath(npcRec_k->x, npcRec_k->y, x, y, mode, npc);
                        }
                        // 0f71
                        if (result_j != 0)
                        {
                            BuildMoveList(npc, mode, x, y);
                            npcRec_k->mode = 3;
                        }
                    }
                }
            }
            else
            {
                // 0f94
                if (moveListPos[npc] > -1 && moveLists[npc].data[moveListPos[npc]] != 0)
                {
                    // 0fb8
                    D_5876_tempX = npcRec_k->x;
                    D_5878_tempY = npcRec_k->y;
                    StepTemp(moveLists[npc].data[moveListPos[npc] + 1]);
                    x = D_5876_tempX;
                    y = D_5878_tempY;
                    result_j = NpcCanStepTo(D_5876_tempX, y, npc, slot_g);
                    if (result_j != 0)
                    {
                        // 0ff9
                        npcRec_k->x = D_5c5a_actors[npcRec_k->actorIdx].x = x;
                        npcRec_k->y = D_5c5a_actors[npcRec_k->actorIdx].y = y;
                        D_24e6_viewDirty |= 2;

                        moveLists[npc].data[moveListPos[npc]]--;
                        npcStuckCount[npc] = 0;
                        if (moveLists[npc].data[moveListPos[npc]] == 0)
                        {
                            // 104a
                            moveListPos[npc]++;
                            moveLists[npc].data[moveListPos[npc]++] = 0;
                            if (moveListPos[npc] >= 0x20 || moveLists[npc].data[moveListPos[npc]] < 1)
                            {
                                moveListPos[npc] = -1;
                            }

                            if (result_j == 2)
                            {
                                // 1083
                                npcRec_k->scheduleIdx = slot_g;
                                npcRec_k->mode = 1;
                                moveListPos[npc] = -1;
                            }
                        }
                    }
                    else
                    {
                        npcStuckCount[npc]++;
                        Wander(&D_5f5e_npcs[npc], 0, slot_g, npc, &D_5d5e_npcSchedules[npc]);
                        if (npcStuckCount[npc] > 3)
                        {
                            // 10d0
                            moveListPos[npc] = -1;
                            // 10d6
                            npcStuckCount[npc] = 0;
                        }
                    }
                }
                else
                {
                    // 10e0
                    if (moveListPos[npc] == -1 && npcRec_k->mode == 3)
                    {
                        if ((s8)D_5895_mapLevel < (s8)D_5d5e_npcSchedules[npc].z[ScheduleSlot(npc, hour)])
                        {
                            npcRec_k->mode = 6;
                            return;
                        }
                        npcRec_k->mode = 7;
                        return;
                    }

                    // 1124
                    if (pathSearched < 1)
                    {
                        // 112d
                        if (npcRec_k->mode != 1)
                        {
                            // 1138
                            if (npcStuckCount[npc] < 200 && (npcStuckCount[npc] == 0 || ULTIMA_2092_RandomRange(0, 2) == 1))
                            {
                                // 1160
                                if (moveListPos[npc] == -1)
                                {
                                    pathSearched = pathSearched + 1;
                                    if ((result_j = SearchPath(npcRec_k->x, npcRec_k->y, D_5d5e_npcSchedules[npc].x[slot_g],
                                                                   D_5d5e_npcSchedules[npc].y[slot_g], 0, npc)) != 0)
                                    {
                                        result_j = BuildMoveList(npc, 0, D_5d5e_npcSchedules[npc].x[slot_g],
                                                                  D_5d5e_npcSchedules[npc].y[slot_g]);
                                        npcStuckCount[npc] = 0;
                                    }
                                    else
                                    {
                                        // 11ba
                                        npcStuckCount[npc] = 200;
                                        Wander(&D_5f5e_npcs[npc], 0, slot_g, npc, &D_5d5e_npcSchedules[npc]);
                                    }
                                }
                                else
                                {
                                    // 11c5
                                    Wander(&D_5f5e_npcs[npc], 0, slot_g, npc, &D_5d5e_npcSchedules[npc]);
                                }
                            }
                            else
                            {
                                // 11e8
                                if (npcStuckCount[npc] >= 200)
                                {
                                    npcStuckCount[npc]++;
                                }

                                // 11f9
                                if (npcStuckCount[npc] > 204)
                                {
                                    npcStuckCount[npc] = 0;
                                }
                            }
                        }
                    }
                    // 120e
                    else if (npcRec_k->z == (uint)D_5895_mapLevel)
                    {
                        if (slot_g == 1 || slot_g == 2)
                        {
                            result_j = 0;
                        }
                        else if (slot_g == 0)
                        {
                            result_j = 2;
                        }
                        else
                        {
                            result_j = slot_g - 1;
                        }
                        // 1243
                        ActBySchedule(npc, npcRec_k->scheduleIdx);
                    }
                }
            }
        }
        // 124e
        else if (npcRec_k->z == D_5895_mapLevel)
        {
            ActBySchedule(npc, slot_g);
        }
    }
}

