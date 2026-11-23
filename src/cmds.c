#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "cast2.h"
#include "cmds.h"
#include "combat.h"
#include "comsubs.h"
#include "dnglook.h"
#include "npc.h"
#include "outsubs.h"
#include "town.h"

void MainoutMoveParty(int stepX, int stepY);
int IsMovingActor(int tile);

// The rest itself of Hole up and camp: sleeps the party for the hours (a bard on watch plays and a watcher
// stays awake), moving the sun and moons; in 1 of 64 hours an ambush starts a combat (returns 1), else the
// party heals (once per 14 hours, with a chance of an apparition outside dungeons).
// HoleUpRest sits at a limit of pass 2's memory: its loop optimization runs out of
// memory unless the module's global symbols take about 85 * (22 + 8) bytes more than u5d's
// readable names give (MATCHING.md, "What pass 2 counts, measured"). The original had
// something there that u5d does not; what it was is not known. The stand-in: 85 dead
// references (never compiled to code, never defined; the object lists them as externals
// without a fixup, as TOWN_0f02's fill) on one line in CMDS_0000. Exact for 75 to 95 of
// them (70 to 90 before the resident names town.c uses lost their prefixes: 140 characters
// fewer in CMDS_0000, 5 references): names cmds.c uses may grow or shrink by about 300
// characters before the window moves; if CMDS_0000 breaks after a rename, change the count
// (one reference is about 30 bytes).
extern int Q0000000, Q0000001, Q0000002, Q0000003, Q0000004, Q0000005, Q0000006, Q0000007, Q0000008, Q0000009, Q0000010, Q0000011, Q0000012, Q0000013, Q0000014, Q0000015, Q0000016, Q0000017, Q0000018, Q0000019, Q0000020, Q0000021, Q0000022, Q0000023, Q0000024, Q0000025, Q0000026, Q0000027, Q0000028, Q0000029, Q0000030, Q0000031, Q0000032, Q0000033, Q0000034, Q0000035, Q0000036, Q0000037, Q0000038, Q0000039, Q0000040, Q0000041, Q0000042, Q0000043, Q0000044, Q0000045, Q0000046, Q0000047, Q0000048, Q0000049, Q0000050, Q0000051, Q0000052, Q0000053, Q0000054, Q0000055, Q0000056, Q0000057, Q0000058, Q0000059, Q0000060, Q0000061, Q0000062, Q0000063, Q0000064, Q0000065, Q0000066, Q0000067, Q0000068, Q0000069, Q0000070, Q0000071, Q0000072, Q0000073, Q0000074, Q0000075, Q0000076, Q0000077, Q0000078, Q0000079, Q0000080, Q0000081, Q0000082, Q0000083, Q0000084;
// CHECKED
int HoleUpRest(int mode, int watchIdx, int hours)
{
    byte oldAnim_k;
    int wakeHour_i;
    byte oldTile_b;
    int slot_m;
    int idx_j;
    int member_g;
    int lastHour_m;
    int ambush_o;
    byte oldActive_d;
    CombatEntity* watcher;
    byte watchTile_g;
    int poisoned_k[8];
    int bardSlot;
    ActorFmt* bardActor; // not read
    if (0) { Q0000000 = 0; Q0000001 = 0; Q0000002 = 0; Q0000003 = 0; Q0000004 = 0; Q0000005 = 0; Q0000006 = 0; Q0000007 = 0; Q0000008 = 0; Q0000009 = 0; Q0000010 = 0; Q0000011 = 0; Q0000012 = 0; Q0000013 = 0; Q0000014 = 0; Q0000015 = 0; Q0000016 = 0; Q0000017 = 0; Q0000018 = 0; Q0000019 = 0; Q0000020 = 0; Q0000021 = 0; Q0000022 = 0; Q0000023 = 0; Q0000024 = 0; Q0000025 = 0; Q0000026 = 0; Q0000027 = 0; Q0000028 = 0; Q0000029 = 0; Q0000030 = 0; Q0000031 = 0; Q0000032 = 0; Q0000033 = 0; Q0000034 = 0; Q0000035 = 0; Q0000036 = 0; Q0000037 = 0; Q0000038 = 0; Q0000039 = 0; Q0000040 = 0; Q0000041 = 0; Q0000042 = 0; Q0000043 = 0; Q0000044 = 0; Q0000045 = 0; Q0000046 = 0; Q0000047 = 0; Q0000048 = 0; Q0000049 = 0; Q0000050 = 0; Q0000051 = 0; Q0000052 = 0; Q0000053 = 0; Q0000054 = 0; Q0000055 = 0; Q0000056 = 0; Q0000057 = 0; Q0000058 = 0; Q0000059 = 0; Q0000060 = 0; Q0000061 = 0; Q0000062 = 0; Q0000063 = 0; Q0000064 = 0; Q0000065 = 0; Q0000066 = 0; Q0000067 = 0; Q0000068 = 0; Q0000069 = 0; Q0000070 = 0; Q0000071 = 0; Q0000072 = 0; Q0000073 = 0; Q0000074 = 0; Q0000075 = 0; Q0000076 = 0; Q0000077 = 0; Q0000078 = 0; Q0000079 = 0; Q0000080 = 0; Q0000081 = 0; Q0000082 = 0; Q0000083 = 0; Q0000084 = 0; }

    lastHour_m = D_587f_hour;
    bardSlot = -1;
    ambush_o = -1;
    statusIcon = protectTurns = 0;
    oldActive_d = activePlayer;
    activePlayer = 0xff;
    if ((mode & 2) != 0)
    {
        PlaceCombatants(GetDungeonMap((uint)D_5896_mapX, (uint)D_5897_mapY, D_5895_mapLevel & 0xff), 0);
    }
    else
    {
        InitCombatants();
    }

    curCombatant = 0xff;
    wakeHour_i = hours + (uint)D_587f_hour;
    if (wakeHour_i > 0x17)
    {
        wakeHour_i -= 0x18;
    }

    if (5 < watchIdx || watchIdx < -1)
    {
        watchIdx = -1;
    }

    for (member_g = 0; member_g < 8; member_g++)
    {
        poisoned_k[member_g] = 0;
    }

    for (member_g = 0; member_g < partyCount; member_g++)
    {
        for (slot_m = 0; slot_m < 0x20; slot_m++)
        {
            if (member_g == combatants[slot_m].entityIdx)
            {
                watcher = &combatants[slot_m];
                break;
            }
        }

        switch (party[member_g].status)
        {
        case STATUS_POISONED:
            poisoned_k[member_g]++;
            // fallthrough
        case STATUS_GOOD:
        case STATUS_SLEEP:
            if (member_g != watchIdx)
            {
                SleepIfNotPoisoned(slot_m);
            }

            if (party[member_g].cls == CLASS_BARD && member_g == watchIdx && soundOn != 0)
            {
                bardSlot = slot_m;
            }
            break;
        }
    }

    UpdateFrame();

    if (bardSlot != -1)
    {
        bardActor = &actors[combatants[bardSlot].actorIdx];
        oldTile_b = bardActor->tile;
        oldAnim_k = bardActor->animTile;
        
        bardActor->tile = bardActor->animTile = TILE_ACTOR_5F;
        songPos = 1;
        WaitFrames(0x34);
        bardActor->tile = oldTile_b;
        bardActor->animTile = oldAnim_k;
        if (combatants[bardSlot].entityIdx != watchIdx)
        {
            SleepIfNotPoisoned(bardSlot);
        }

        UpdateFrame();
    }

    PrintString(/*0x41d4*/ "Zzzzzz...\n\n");

    if (watchIdx != -1)
    {
        for (slot_m = 0; slot_m < 0x20; slot_m++)
        {
            if (combatants[slot_m].entityIdx == watchIdx)
            {
                watcher = &combatants[slot_m];
                break;
            }
        }

        watchTile_g = actors[watcher->actorIdx].tile;
    }

    while (wakeHour_i != D_587f_hour)
    {
        if (D_587f_hour >= 0x18)
        {
            D_587f_hour -= 0x18;
        }

        UpdateFrame();
        RingRegeneration();
        UpdateVitalsDisplay();
        if (lastHour_m != D_587f_hour)
        {
            if (RandomRange(0, 0x3f) == 0)
            {
                SeedRandom(GetTime());
                ambush_o = (uint)ambushEnemies[RandomRange(0, 7)];
                PrintString(/*0x41e0*/ "Ambushed!\n\n");

                if (watchIdx > -1)
                {
                    // 0254..026e..027a..027d
                    for (idx_j = 0; idx_j < 6; idx_j++)
                    {
                        // 0283
                        if ((member_g = combatants[idx_j].entityIdx) >= 0 && member_g < partyCount)
                        {
                            WakeCombatant(idx_j);
                            if (poisoned_k[member_g] != 0)
                            {
                                party[member_g].status = STATUS_POISONED;
                            }
                            else
                            {
                                party[member_g].status = STATUS_GOOD;
                            }
                        }
                    }
                }

                UpdateVitalsDisplay();
                if ((mode & 2) != 0)
                {
                    PlaceCombatants(GetDungeonMap((uint)D_5896_mapX, (uint)D_5897_mapY, D_5895_mapLevel & 0xff), 1);
                }
                else
                {
                    SetUpCombat(mode, ambush_o);
                }

                break;
            }
        }

        lastHour_m = D_587f_hour;
        AdvanceTime(5);
        WaitTicks(1);
        if (D_5894_savedMapId < 0x21)
        {
            mapId = D_5894_savedMapId;
            DrawSunAndMoons();
            mapId = 0xff;
        }

        if (watchIdx != -1 && RandomRange(0, 3) == 2)
        {
            tempX = (uint)watcher->x;
            tempY = (uint)watcher->y;

            switch (RandomRange(0, 3))
            {
            case 0:
                tempY--;
                break;
            case 1:
                tempY++;
                break;
            case 2:
                tempX++;
                break;
            case 3:
                tempX--;
                break;
            }

            if (IsInView(tempX, tempY) != 0 && IsCellFree(watchTile_g, tempX, tempY) != 0)
            {
                watcher->x = actors[watcher->actorIdx].x = tempX;
                watcher->y = actors[watcher->actorIdx].y = tempY;
                UpdateFrame();
            }
        }
    }

    // c27d
    if (ambush_o > -1)
    {
        // conflict

        return 1;
    }

    if (campHealWait < 1 && hours > 5)
    {
        PrintString(/*0x41ec*/ "Party rested!\n");

        for (member_g = 0; member_g < partyCount; member_g++)
        {
            if (poisoned_k[member_g] == 0 && campHealWait < 1 && hours > 5 && party[member_g].status != STATUS_DEAD && member_g != watchIdx)
            {
                party[member_g].hp += RandomRange(1, 0x3f);
                if (party[member_g].hp > party[member_g].maxHp)
                {
                    party[member_g].hp = party[member_g].maxHp;
                }

                if (member_g != watchIdx)
                {
                    switch (party[member_g].cls)
                    {
                    case CLASS_AVATAR:
                    case CLASS_MAGE:
                        party[member_g].mag = party[member_g].intel;
                        break;
                    case CLASS_BARD:
                        party[member_g].mag = party[member_g].intel >> 1;
                        break;
                    }
                }
            }
        }

        if ((mode & 0x82) == 0 && RandomRange(0, 99) < 0x19)
        {
            apparitionMonth = D_587d_month;
            Apparition();
        }

        campHealWait = 0xe;
    }
    else
    {
        PrintString(/*0x41fb*/ "No effect...\n");
    }

    for (member_g = 0; member_g < partyCount; member_g++)
    {
        if (party[member_g].status == STATUS_SLEEP)
        {
            party[member_g].status = STATUS_GOOD;
        }
    }

    activePlayer = oldActive_d;
    UpdateVitalsDisplay();

    return 0;
}

// The Hole up command: asks for the hours (a digit), lets the town's NPCs move, puts the party to sleep and
// passes the time (swapping the day and night tiles at hours 20 and 5); an actor on the party's cell throws
// it out of bed.
void CmdsHoleUpCmd(void)
{
    int lastHour_h;
    int i;
    int wakeHour;
    byte key_h;
    int savedActive_b;

    PrintString(/*0x4209*/ "For how many hours? ");

    while ((key_h = GetChar()) != U5_KEY_SPACE && (key_h < 0x30 || key_h > 0x39))
    {
    }

    if (key_h != U5_KEY_SPACE && key_h != 0x30)
    {
        PrintChar(key_h);
        PrintChar('\n');

        wakeHour = (int)D_587f_hour + (int)key_h - 0x30;
        if (wakeHour > 0x17)
        {
            wakeHour -= 0x17;
        }

        for (i = 0; i < 0x10; i++)
        {
            MoveNpcs(D_587f_hour);
            UpdateFrame();
            if (npcApproach == 0x61)
            {
                return;
            }
        }

        savedActive_b = activePlayer;

        for (i = 0; i < partyCount; i++)
        {
            if (party[i].status == STATUS_GOOD)
            {
                party[i].status = STATUS_SLEEP;
            }
        }

        UpdateVitalsDisplay();
        PrintString(/*0x421e*/ "Zzzzzzz...\n");

        GRAP_2d_SetPenColor(0);
        GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);

        lastHour_h = D_587f_hour;
        while (wakeHour != D_587f_hour)
        {
            AdvanceTime(10);
            if (lastHour_h != D_587f_hour && (D_587f_hour == 0x14 || D_587f_hour == 0x05))
            {
                SwapDayNightTiles();
            }

            lastHour_h = D_587f_hour;
            DrawSunAndMoons();
            EndTurn();
            UpdateVitalsDisplay();
            PlaceActors();

            if (FindActorTileAtPos(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) != 0)
            {
                wakeHour = -1;
                break;
            }
            WaitTicks(1);
        }

        if (wakeHour == -1)
        {
            PrintString(/*0x422a*/ "Thrown out of bed!\n");
        }

        for (i = 0; i < partyCount; i++)
        {
            if (party[i].status == STATUS_SLEEP)
            {
                party[i].status = STATUS_GOOD;
            }
        }

        activePlayer = savedActive_b;
        D_5896_mapX++;
        viewDirty = 1;
        actors[0].x++;

        UpdateVitalsDisplay();
        UpdateFrame();
    }
}

// 1 when the party is on foot (the Avatar tile); else prints "On foot" and returns 0.
static int RequireOnFoot(void)
{
    if (partyTile != TILE_ACTOR_AVATAR && partyTile != TILE_ACTOR_1D)
    {
        PrintString(/*0x423e*/ "\nOn foot\n");
        return 0;
    }
    else
    {
        return 1;
    }
}

// 1 when the party is on foot, on a carpet or on a skiff; else prints "On foot" and returns 0.
static int CanBoardShip(void)
{
    switch (partyTile)
    {
    case TILE_ACTOR_FLYING_CARPET:
    case TILE_ACTOR_FLYING_CARPET + 1:
    case TILE_ACTOR_AVATAR:
    case TILE_ACTOR_1D:
    case TILE_ACTOR_SKIFF:
    case TILE_ACTOR_SKIFF + 1:
    case TILE_ACTOR_SKIFF + 2:
    case TILE_ACTOR_SKIFF + 3:
        return 1;

    default:
        PrintString(/*0x4248*/ "\nOn foot\n");
        return 0;
    }
}

static int IsLandAt(int viewY, int viewX);

// 1 when one of the four cells around the party is land.
static int IsLandNearby(void)
{
    if (IsLandAt(4, 5) == 1 || IsLandAt(6, 5) == 1 || IsLandAt(5, 6) == 1 || IsLandAt(5, 4) == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// 1 when the cell of the view is land: a walkable tile, or an actor tile of the ground kinds.
static int IsLandAt(int viewY, int viewX)
{
    int tile;

    tile = GetMapViewport(viewX, viewY);
    if (tile != 0)
    {
        return IsWalkableTile(0x1c, tile);
    }

    tile = GetActorMap(viewX, viewY);
    if (tile == 0x1b)
    {
        return 1;
    }

    tile &= 0xfc;
    if (tile == 0x1c || tile == 0x24 || tile == 0x10 || tile == 0x28)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// The Board command: boards the horse, carpet, skiff or frigate that stands on the party's cell (a ship's
// hull and skiffs are taken over; "Nay!" for a horse that belongs to someone), then the actor is removed; 1
// when the command was understood.
int BoardCmd(void)
{
    byte tile;
    int hull;
    int skiffs_l;
    int actorSlot_g;
    int npc;

    if (mapId > 0x20 && mapId < 0x29)
    {
        PrintString(/*0x4252*/ "\nNot here!\n");

        return 1;
    }

    tile = FindActorTileAtPos(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    actorSlot_g = tempX;
    if ((tile & 0xfe) == TILE_ACTOR_HORSE)
    {
        if (mapId != 0)
        {
            npc = FindNpcByActor(actorSlot_g);
            if (npc != -1 && npcs[npc].dialogNumber != 0)
            {
                PrintString(/*0x425e*/ "\"Nay!\"\n");
                return 1;
            }
        }

        if (RequireOnFoot() == 0)
        {
            return 1;
        }

        PrintString(/*0x4266*/ "horse\n");

        partyTile = tile + 2;
    }
    else if (tile == TILE_ACTOR_CARPET)
    {
        if (RequireOnFoot() == 0)
        {
            return 1;
        }

        PrintString(/*0x426d*/ "carpet\n");

        partyTile = TILE_ACTOR_FLYING_CARPET;
    }
    else if ((tile & 0xfc) == TILE_ACTOR_SKIFF)
    {
        if (RequireOnFoot() == 0)
        {
            return 1;
        }

        PrintString(/*0x4275*/ "skiff\n");

        partyTile = tile;
    }
    else if ((tile & 0xfc) == TILE_ACTOR_FRIGATE_24)
    {
        if (CanBoardShip() == 0)
        {
            return 1;
        }

        PrintString(/*0x427c*/ "Ship\n");

        hull = actors[actorSlot_g].value;
        if (hull < 10)
        {
            PrintString(/*0x4282*/ "\nDANGER: SHIP BADLY DAMAGED!\n");
        }

        actors[0].value = hull;
        skiffs_l = actors[actorSlot_g].extra;

        if ((partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET)
        {
            carpets++;
        }

        if ((partyTile & 0xfc) == TILE_ACTOR_SKIFF)
        {
            skiffs_l++;
        }

        if (skiffs_l == 0)
        {
            PrintString(/*0x42a0*/ "\nWARNING: NO SKIFFS ON BOARD!\n");
        }

        partyTile = tile;
        actors[0].extra = skiffs_l;
        vitalsDirty = 1;
    }
    else
    {
        PrintString(/*0x42bf*/ "What?\n");
        return 0;
    }

    SetActor(0, 0, 0, 0, 0, 0, actorSlot_g);
    viewDirty |= 2;

    return 1;
}

// Fire on the overworld: a frigate fires to either side (not ahead or astern), asking the direction; the
// first moving actor within three cells is hit for 1 to 20 and may be destroyed.
static void FireBroadsides(void)
{
    int y;
    int dirY;
    int x;
    int dirX;
    int step_e;
    int tile_c;
    int targetY;
    int targetX_b;
    int actorSlot_f;
    int hit_l;

    if (partyTile < TILE_ACTOR_FRIGATE_20 || partyTile > TILE_ACTOR_FRIGATE_20_END)
    {
        PrintString(/*0x42c6*/ "What?\n");
    }
    else
    {
        if (SelectDirection() == 0)
        {
            return;
        }

        dirX = tempX;
        dirY = tempY;

        if (dirX == 0 && (partyTile & 1) == 0 || dirX != 0 && (partyTile & 1) != 0)
        {
            // 09b2
            PrintString(/*0x42cd*/ "Fire broadsides only!\n");
        }
        else
        {
            // 09b8
            x = D_5896_mapX;
            y = D_5897_mapY;
            AudioSweepTone(1000, 200, 5, 300);

            // 09d8
            for (step_e = 0; step_e < 3; step_e++)
            {
                x += dirX;
                y += dirY;

                tile_c = FindActorTileAtPos(x, y, D_5895_mapLevel);

                if (IsMovingActor(tile_c) != 0 && (tile_c & 0xfc) != 0xec)
                {
                    // 0a10
                    actorSlot_f = tempX;

                    targetX_b = actors[actorSlot_f].x - D_5896_mapX + 5;
                    targetY = actors[actorSlot_f].y - D_5897_mapY + 5;
                    hit_l = FlyMissile(5, 5, targetX_b, targetY, 1);
                    if (hit_l != 0)
                    {
                        UpdateFrame();
                        HitEffectAt(x, y);

                        actors[actorSlot_f].value -= RandomRange(1, 0x14);
                        if (actors[actorSlot_f].value > 0x7f)
                        {
                            SetActor(0, 0, 0, 0, 0, 0, actorSlot_f);
                            viewDirty |= 2;
                        }
                    }

                    return;
                }
            }

            targetX_b = dirX * 3 + 5;
            targetY = dirY * 3 + 5;
            FlyMissile(5, 5, targetX_b, targetY, 1);
        }
    }
}

// The Fire command: on the overworld the frigate's broadsides; in a town or keep a cannon beside the party
// fires four cells along its facing, destroying a door (and the tile 0x44 left) or killing an actor, which
// costs karma 5 and kills its NPC; or hurts the party.
void FireCmd(void)
{
    uint cannonTile_e;
    int dirY_h;
    int y_g;
    int hitActor_b;
    int range_c;
    int dirX_i;
    int x_h;
    int endY_p;
    int tileAt_m;
    int endX;
    int startY;
    int target;
    int startX;
    int npc_e;
    int hitWall;

    if (0x20 < mapId && mapId < 0x29)
    {
        PrintString(/*0x42e4*/ "What?\n");
        return;
    }

    if (mapId == 0)
    {
        FireBroadsides();
        return;
    }

    SetTile(openDoorTile, openDoorX, openDoorY);

    if (((cannonTile_e = GetMapViewport(5, 4)) & 0xfc) == 0xb4)
    {
        dirX_i = 0;
        dirY_h = -1;
    }
    else if (((cannonTile_e = GetMapViewport(6, 5)) & 0xfc) == 0xb4)
    {
        dirX_i = 1;
        dirY_h = 0;
    }
    else if (((cannonTile_e = GetMapViewport(5, 6)) & 0xfc) == 0xb4)
    {
        dirX_i = 0;
        dirY_h = 1;
    }
    else if (((cannonTile_e = GetMapViewport(4, 5)) & 0xfc) == 0xb4)
    {
        dirX_i = -1;
        dirY_h = 0;
    }
    else
    {
        PrintString(/*0x42eb*/ "What?\n");
        return;
    }

    x_h = dirX_i + (uint)D_5896_mapX;
    y_g = dirY_h + (uint)D_5897_mapY;

    startX = endX = dirX_i + 5;
    startY = endY_p = dirY_h + 5;

    switch (cannonTile_e & 3)
    {
    case 0:
        dirX_i = 0;
        dirY_h = -1;
        break;
    case 1:
        dirX_i = 1;
        dirY_h = 0;
        break;
    case 2:
        dirX_i = 0;
        dirY_h = 1;
        break;
    case 3:
        dirX_i = -1;
        dirY_h = 0;
        break;
    }

    PrintString(/*0x42f2*/ "BOOOM!\n");
    AudioSweepTone(1000, 200, 5, 300);
    RaiseAlarm();

    hitWall = 0;
    hitActor_b = 0;
    range_c = 5;

    // 0c1a
    while (!hitWall && !hitActor_b && --range_c > 0)
    {
        x_h += dirX_i;
        y_g += dirY_h;

        endX += dirX_i;
        endY_p += dirY_h;

        tileAt_m = FindLastActorTileAt(x_h, y_g, D_5895_mapLevel);
        if (tileAt_m == 0)
        {
            switch (*GetTileAddr(x_h, y_g))
            {
            case TILE_MAP_97:
            case TILE_MAP_98:
            case TILE_MAP_99:
            case TILE_MAP_DOOR_B8:
            case TILE_MAP_DOOR_B9:
            case TILE_MAP_DOOR_BA:
            case TILE_MAP_DOOR_BB:
                hitWall = 1;
                break;
            }
        }
        // 0cb8
        else if ((tileAt_m >= 0x1c && (tileAt_m & 0xf8) != 0x78 && (tileAt_m & 0xfc) != 0x2f) // BUG: expression is always true
                 || tileAt_m == 0x10 || tileAt_m == 0x11)
        {
            hitActor_b = 1;
            target = tempX;
        }
    }

    FlyMissile(startX, startY, endX, endY_p, 1);
    if (hitWall || hitActor_b)
    {
        HitEffectAt(x_h, y_g);
    }

    if (hitWall)
    {
        PrintString(/*0x42fa*/ "Door destroyed!\n");
        *GetTileAddr(x_h, y_g) = TILE_MAP_44;
        viewDirty = 1;
        openDoorTile = 0;
    }

    if (hitActor_b && target != 0)
    {
        SetActor(0, 0, 0, 0, 0, 0, target);

        viewDirty |= 2;
        if (karma > 5)
        {
            karma -= 5;
        }
        else
        {
            karma = 0;
        }

        if ((npc_e = FindNpcByActor(target)) == -1)
        {
            return;
        }

        SetNpcKilled(npc_e);
        DespawnNpc(npc_e);
    }

    if (hitActor_b && target == 0)
    {
        DamageParty();
    }
}

// The Ignite torch command: uses a torch; the light lasts 240 turns (a random 112 to 127 more in a dungeon).
void IgniteTorchCmd(void)
{
    if (torches == 0)
    {
        PrintString(/*0x430b*/ "None owned!\n");
    }
    else
    {
        torches--;
        if (mapId > 0x20 && mapId < 0x29)
        {
            IncreaseByte(&torchTurns, RandomRange(0, 0xf) + 0x70, 0xff);
        }
        else
        {
            torchTurns = 0xf0;
        }
    }
}

// The New order command: swaps two party members (the leader cannot be moved).
void NewOrderCmd(void)
{
    int first;
    int second;
    S_55a8 swap_f;

    PrintString(/*0x4318*/ "\n\nSwap ");
    if ((first = SelectPlayer()) == -1)
    {
        PrintString(/*0x4320*/ "nobody!\n");
        return;
    }

    PrintString(party[first].name);
    if (first == 0)
    {
        PrintString(/*0x4329*/ "\n\n");
        PrintString(party[0].name);
        PrintString(/*0x432c*/ " must lead!\n");
        return;
    }

    PrintString(/*0x4339*/ "\nwith ");

    if ((second = SelectPlayer()) == -1)
    {
        PrintString(/*0x4340*/ "nobody!\n");
        return;
    }

    PrintString(party[second].name);
    if (second == 0)
    {
        PrintString(/*0x4349*/ "\n\n");
        PrintString(party[0].name);
        PrintString(/*0x434c*/ " must lead!\n");
        return;
    }

    PrintString(/*0x4359*/ "!\n");

    swap_f = party[first];
    party[first] = party[second];
    party[second] = swap_f;

    vitalsDirty = 1;
}

// The Xit command: leaves the horse, carpet, skiff or ship (onto land only; a ship hands out a skiff or a
// carpet) and puts the vehicle back as an actor.
// CHECKED
void XitCmd(void)
{
    byte skiffs;
    byte tileUnder_g;
    int slot;
    byte newTile_i;

    if (mapId < 0x20 && mapId > 0x29) // TODO: sic; BUG
    {
        // ce4c
        PrintString(/*0x435c*/ "\nNot here!\n");
        return; // -> cfaa
    }

    tileUnder_g = *GetTileAddr(D_5896_mapX, D_5897_mapY);

    switch (partyTile & 0xfc)
    {
    case TILE_ACTOR_AVATAR:
        PrintString(/*0x4368*/ "what?\n");
        return;

    case TILE_ACTOR_FRIGATE_20:
        PrintString(/*0x436f*/ "\nUnder sail!\n");
        return;

    case TILE_ACTOR_FLYING_CARPET:
        if (IsLandNearby() != 0 || IsWalkableTile(0x1c, tileUnder_g) != 0)
        {
            PrintString(/*0x437d*/ "carpet!\n");
            newTile_i = 0x1b;
            partyTile = TILE_ACTOR_AVATAR;
            break;
        }

        if (IsLandNearby() == 0)
        {
            PrintString(/*0x4386*/ "\nNo land nearby!\n");
            return;
        }

        PrintString(/*0x4398*/ "\nNot here!\n");
        return;

    case TILE_ACTOR_HORSE:
        PrintString(/*0x43a4*/ "horse!\n");
        newTile_i = partyTile - 2;
        partyTile = TILE_ACTOR_AVATAR;
        break;

    case TILE_ACTOR_SKIFF:
        // cef2
        if (IsLandNearby() == 0)
        {
            PrintString(/*0x43ac*/ "\nNo land nearby!\n");
            return;
        }

        if ((tileUnder_g & 0xfe) == TILE_MAP_6A)
        {
            PrintString(/*0x43be*/ "\nNot here!\n");
            return;
        }

        PrintString(/*0x43ca*/ "skiff!\n");
        newTile_i = partyTile;
        partyTile = TILE_ACTOR_AVATAR;
        break;

    case TILE_ACTOR_FRIGATE_24:
        // cf1c
        PrintString(/*0x43d2*/ "ship!\n");
        if (IsLandNearby() != 0)
        {
            newTile_i = partyTile;
            partyTile = TILE_ACTOR_AVATAR;

            skiffs = actors[0].extra;
        }
        else if (actors[0].extra != 0)
        {
            newTile_i = partyTile;
            partyTile += 4;
            skiffs = actors[0].extra - 1;
        }
        else if (carpets != 0)
        {
            carpets--;
            newTile_i = partyTile;
            partyTile = TILE_ACTOR_FLYING_CARPET;

            skiffs = actors[0].extra;
        }
        else
        {
            PrintString(/*0x43d9*/ "\nNo skiffs on board!\n");
            return;
        }

        break;
    }

    slot = FindActorSlot();
    SetActor(newTile_i, newTile_i, D_5896_mapX, D_5897_mapY, D_5895_mapLevel, actors[0].value, slot);
    actors[slot].extra = skiffs; // BUG: skiffs NOT INITIALIZED when (D_587c & 0xfc) != 0x24
    UpdateVitalsDisplay();
}

char* shadowlordNames[3] =
{
    /*0x43ef*/ "FAULINEI",
    /*0x43f8*/ "ASTAROTH",
    /*0x4401*/ "NOSFENTOR"
};

// A yelled shadowlord's name (FAULINEI, ASTAROTH, NOSFENTOR) on maps 0x1e to 0x20 calls that shadowlord up
// as an NPC two cells north (not when one is there, or when it is not allowed); returns 1 when nothing
// happened, else 0.
// CHECKED
int YellShadowlord(char* word)
{
    int slot;
    int which_o;
    int j_h;
    int result_c;
    int npc;

    result_c = 1;

    if (mapId == 0x1e || mapId == 0x1f || mapId == 0x20)
    {
        for (which_o = 0; which_o < 3; which_o++)
        {
            if (CompareWord((byte*)shadowlordNames[which_o], word) > -1)
                break;
        }

        if (which_o == 3 || D_5897_mapY < 2 || shadowlordTowns[which_o] == 0xff)
        {
            // d002
            PrintString(/*0x440b*/ "\nNo effect!\n");
            return /* AX: result_c */;
        }
        else
        {
            for (slot = 0; slot < 0x20; slot++)
            {
                if (actors[slot].tile == TILE_ACTOR_SHADOWLORD)
                {
                    PrintString(/*0x4418*/ "\nNo effect!\n");
                    return /* AX: result_c */;
                }
            }

            summonedLord = which_o;
            slot = FindActorSlot();
            SetActor(TILE_ACTOR_SHADOWLORD, TILE_ACTOR_SHADOWLORD, D_5896_mapX, D_5897_mapY - 2, D_5895_mapLevel, 0, slot);

            npc = 0x1f;
            for (j_h = 0x1f; j_h >= 0; j_h--)
            {
                if (npcTypes[j_h] == 0)
                {
                    npc = j_h;
                    break;
                }
            }

            npcs[npc].mode = 1;
            npcs[npc].actorIdx = slot;
            npcs[npc].x = D_5896_mapX;
            npcs[npc].y = D_5897_mapY - 2;
            npcs[npc].z = D_5895_mapLevel;

            for (j_h = 0; j_h < 4; j_h++)
            {
                npcSchedules[npc].time[j_h] = 0;
            }

            for (j_h = 0; j_h < 3; j_h++)
            {
                npcSchedules[npc].type[j_h] = 6;
                npcSchedules[npc].x[j_h] = D_5896_mapX;
                npcSchedules[npc].y[j_h] = D_5897_mapY - 2;
                npcSchedules[npc].z[j_h] = D_5895_mapLevel;
            }

            npcTypes[npc] = 0xfc;

            PrintString(/*0x4425*/ "\nA shadowlord appears\n");
            AudioPulse(0x28a0, 1, 30000, 2000, 2);

            actors[slot].tile = actors[slot].animTile = TILE_ACTOR_CIRCLE;
            ULTIMA_1068_RevealTile(0x1fc, 5, 3);
            actors[slot].tile = actors[slot].animTile = TILE_ACTOR_SHADOWLORD;

            /* FMT
                if (DAT_0003e08a_587a != 0x54) { // Icon != 0x54
                    return 0;
                }
                DAT_0003e08a_587a = 0; // Icon = 0
                FUN_00029d64_ULTIMA_5910_update_map();
                DAT_0003e08a_587a = 0x54; // Icon = 0x54
                return 0;
             */

            result_c = 0;
        L_d1e4:
            return result_c;
        }
    }
    else
    {
        PrintString(/*0x443c*/ "\nNo effect!\n");
        return /* AX: result_c */;
    }
}
