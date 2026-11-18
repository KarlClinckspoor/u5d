#include "common/common.h"
// CMDS_0000_HoleUpRest sits at a limit of pass 2's memory, which the names of the globals this object
// declares and uses count against: with the readable names of the 27 below (found by taking
// the new names one at a time) it loses its loop optimization. So this object alone knows
// them by their short names; the source below uses the readable ones, and the link
// resolves the short ones by the address they carry. After common.h, which defines
// TARGET_DOS16 and declares none of them. If a later rename breaks CMDS_0000_HoleUpRest again, add
// its name here (docs/MATCHING.md, "Readable names").
#if defined(TARGET_DOS16)
#define D_5876_tempX D_5876
#define D_5878_tempY D_5878
#define D_587a_statusIcon D_587a
#define D_587d_month D_587d
#define D_588c_campHealWait D_588c
#define D_588e_protectTurns D_588e
#define D_5894_savedMapId D_5894
#define D_589e_curCombatant D_589e
#define D_595a_dungeonMap D_595a
#define D_1734_ambushEnemies D_1734
#define D_588d_apparitionMonth D_588d
#define D_6a08_songPos D_6a08
#define D_5c5a_actors D_5c5a
#define D_a9ce_soundOn D_a9ce
#define D_ba14_combatants D_ba14
#define COMBAT_0000_IsCellFree COMBAT_0000
#define DNGLOOK_117e_PlaceCombatants DNGLOOK_117e
#define OUTSUBS_0658_Apparition OUTSUBS_0658
#define ULTIMA_3ae6_WaitFrames ULTIMA_3ae6
#define ULTIMA_400c_RingRegeneration ULTIMA_400c
#define ULTIMA_4a84_DrawSunAndMoons ULTIMA_4a84
#define ULTIMA_4f7c_AdvanceTime ULTIMA_4f7c
#define ULTIMA_6800_WakeCombatant ULTIMA_6800
#define ULTIMA_6880_SleepIfNotPoisoned ULTIMA_6880
#define ULTIMA_6936_InitCombatants ULTIMA_6936
#define ULTIMA_6bc2_SetUpCombat ULTIMA_6bc2
#define ULTIMA_6d82_IsInView ULTIMA_6d82
#endif
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "audio/aud_mus.h"

#include "cast2.h"
#include "cmds.h"
#include "combat.h"
#include "comsubs.h"
#include "dnglook.h"
#include "npc.h"
#include "outsubs.h"
#include "town.h"

void MAINOUT_0354_MoveParty(int stepX, int stepY);
int MAINOUT_105c_IsMovingActor(int tile);

// The rest itself of Hole up and camp: sleeps the party for the hours (a bard on watch plays and a watcher
// stays awake), moving the sun and moons; in 1 of 64 hours an ambush starts a combat (returns 1), else the
// party heals (once per 14 hours, with a chance of an apparition outside dungeons).
// CHECKED
int CMDS_0000_HoleUpRest(int mode, int watchIdx, int hours)
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

#if !defined(MATCHING_BUILD)
    watcher = 0;
#endif

    lastHour_m = D_587f_hour;
    bardSlot = -1;
    ambush_o = -1;
    D_587a_statusIcon = D_588e_protectTurns = 0;
    oldActive_d = D_587b_activePlayer;
    D_587b_activePlayer = 0xff;
    if ((mode & 2) != 0)
    {
        DNGLOOK_117e_PlaceCombatants(GetDungeonMap((uint)D_5896_mapX, (uint)D_5897_mapY, D_5895_mapLevel & 0xff), 0);
    }
    else
    {
        ULTIMA_6936_InitCombatants();
    }

#if defined(ENABLE_BGM)
    AUDIO_PlayBgmForHoleUp();
#endif

    D_589e_curCombatant = 0xff;
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

    for (member_g = 0; member_g < D_585b_partyCount; member_g++)
    {
        for (slot_m = 0; slot_m < 0x20; slot_m++)
        {
            if (member_g == D_ba14_combatants[slot_m].entityIdx)
            {
                watcher = &D_ba14_combatants[slot_m];
                break;
            }
        }

        switch (D_55a8_party[member_g].status)
        {
        case STATUS_POISONED:
            poisoned_k[member_g]++;
            // fallthrough
        case STATUS_GOOD:
        case STATUS_SLEEP:
            if (member_g != watchIdx)
            {
                ULTIMA_6880_SleepIfNotPoisoned(slot_m);
            }

            if (D_55a8_party[member_g].cls == CLASS_BARD && member_g == watchIdx && D_a9ce_soundOn != 0)
            {
                bardSlot = slot_m;
            }
            break;
        }
    }

    ULTIMA_5910_UpdateFrame();

    if (bardSlot != -1)
    {
        bardActor = &D_5c5a_actors[D_ba14_combatants[bardSlot].actorIdx];
        oldTile_b = bardActor->tile;
        oldAnim_k = bardActor->animTile;
        
        bardActor->tile = bardActor->animTile = TILE_ACTOR_5F;
        D_6a08_songPos = 1;
        ULTIMA_3ae6_WaitFrames(0x34);
        bardActor->tile = oldTile_b;
        bardActor->animTile = oldAnim_k;
        if (D_ba14_combatants[bardSlot].entityIdx != watchIdx)
        {
            ULTIMA_6880_SleepIfNotPoisoned(bardSlot);
        }

        ULTIMA_5910_UpdateFrame();
    }

    ULTIMA_1850_PrintString(_TEXT(0x41d4, "Zzzzzz...\n\n"));

    if (watchIdx != -1)
    {
        for (slot_m = 0; slot_m < 0x20; slot_m++)
        {
            if (D_ba14_combatants[slot_m].entityIdx == watchIdx)
            {
                watcher = &D_ba14_combatants[slot_m];
                break;
            }
        }

        watchTile_g = D_5c5a_actors[watcher->actorIdx].tile;
    }

    while (wakeHour_i != D_587f_hour)
    {
        if (D_587f_hour >= 0x18)
        {
            D_587f_hour -= 0x18;
        }

        ULTIMA_5910_UpdateFrame();
        ULTIMA_400c_RingRegeneration();
        ULTIMA_2900_UpdateVitalsDisplay();
        if (lastHour_m != D_587f_hour)
        {
            if (ULTIMA_2092_RandomRange(0, 0x3f) == 0)
            {
                ULTIMA_207e_srand(ULTIMA_2056_GetTime());
                ambush_o = (uint)D_1734_ambushEnemies[ULTIMA_2092_RandomRange(0, 7)];
                ULTIMA_1850_PrintString(_TEXT(0x41e0, "Ambushed!\n\n"));

                if (watchIdx > -1)
                {
                    // 0254..026e..027a..027d
                    for (idx_j = 0; idx_j < 6; idx_j++)
                    {
                        // 0283
                        if ((member_g = D_ba14_combatants[idx_j].entityIdx) >= 0 && member_g < D_585b_partyCount)
                        {
                            ULTIMA_6800_WakeCombatant(idx_j);
                            if (poisoned_k[member_g] != 0)
                            {
                                D_55a8_party[member_g].status = STATUS_POISONED;
                            }
                            else
                            {
                                D_55a8_party[member_g].status = STATUS_GOOD;
                            }
                        }
                    }
                }

                ULTIMA_2900_UpdateVitalsDisplay();
                if ((mode & 2) != 0)
                {
                    DNGLOOK_117e_PlaceCombatants(GetDungeonMap((uint)D_5896_mapX, (uint)D_5897_mapY, D_5895_mapLevel & 0xff), 1);
                }
                else
                {
                    ULTIMA_6bc2_SetUpCombat(mode, ambush_o);
                }

                break;
            }
        }

        lastHour_m = D_587f_hour;
        ULTIMA_4f7c_AdvanceTime(5);
        ULTIMA_20fa_WaitTicks(1);
        if (D_5894_savedMapId < 0x21)
        {
            D_5893_mapId = D_5894_savedMapId;
            ULTIMA_4a84_DrawSunAndMoons();
            D_5893_mapId = 0xff;
        }

        if (watchIdx != -1 && ULTIMA_2092_RandomRange(0, 3) == 2)
        {
            D_5876_tempX = (uint)watcher->x;
            D_5878_tempY = (uint)watcher->y;

            switch (ULTIMA_2092_RandomRange(0, 3))
            {
            case 0:
                D_5878_tempY--;
                break;
            case 1:
                D_5878_tempY++;
                break;
            case 2:
                D_5876_tempX++;
                break;
            case 3:
                D_5876_tempX--;
                break;
            }

            if (ULTIMA_6d82_IsInView(D_5876_tempX, D_5878_tempY) != 0 && COMBAT_0000_IsCellFree(watchTile_g, D_5876_tempX, D_5878_tempY) != 0)
            {
                watcher->x = D_5c5a_actors[watcher->actorIdx].x = D_5876_tempX;
                watcher->y = D_5c5a_actors[watcher->actorIdx].y = D_5878_tempY;
                ULTIMA_5910_UpdateFrame();
            }
        }
    }

    // c27d
    if (ambush_o > -1)
    {
        // conflict
#if defined(ENABLE_BGM)
        AUDIO_StopBgm();
#endif

        return 1;
    }

    if (D_588c_campHealWait < 1 && hours > 5)
    {
        ULTIMA_1850_PrintString(_TEXT(0x41ec, "Party rested!\n"));

        for (member_g = 0; member_g < D_585b_partyCount; member_g++)
        {
            if (poisoned_k[member_g] == 0 && D_588c_campHealWait < 1 && hours > 5 && D_55a8_party[member_g].status != STATUS_DEAD && member_g != watchIdx)
            {
                D_55a8_party[member_g].hp += ULTIMA_2092_RandomRange(1, 0x3f);
                if (D_55a8_party[member_g].hp > D_55a8_party[member_g].maxHp)
                {
                    D_55a8_party[member_g].hp = D_55a8_party[member_g].maxHp;
                }

                if (member_g != watchIdx)
                {
                    switch (D_55a8_party[member_g].cls)
                    {
                    case CLASS_AVATAR:
                    case CLASS_MAGE:
                        D_55a8_party[member_g].mag = D_55a8_party[member_g].intel;
                        break;
                    case CLASS_BARD:
                        D_55a8_party[member_g].mag = D_55a8_party[member_g].intel >> 1;
                        break;
                    }
                }
            }
        }

        if ((mode & 0x82) == 0 && ULTIMA_2092_RandomRange(0, 99) < 0x19)
        {
            D_588d_apparitionMonth = D_587d_month;
            OUTSUBS_0658_Apparition();
        }

        D_588c_campHealWait = 0xe;
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x41fb, "No effect...\n"));
    }

    for (member_g = 0; member_g < D_585b_partyCount; member_g++)
    {
        if (D_55a8_party[member_g].status == STATUS_SLEEP)
        {
            D_55a8_party[member_g].status = STATUS_GOOD;
        }
    }

    D_587b_activePlayer = oldActive_d;
    ULTIMA_2900_UpdateVitalsDisplay();

#if defined(ENABLE_BGM)
    AUDIO_PlayBgmPerMap();
#endif

    return 0;
}

// The Hole up command: asks for the hours (a digit), lets the town's NPCs move, puts the party to sleep and
// passes the time (swapping the day and night tiles at hours 20 and 5); an actor on the party's cell throws
// it out of bed.
void CMDS_0552_HoleUpCmd(void)
{
    int lastHour_h;
    int i;
    int wakeHour;
    byte key_h;
    int savedActive_b;

    ULTIMA_1850_PrintString(_TEXT(0x4209, "For how many hours? "));

    while ((key_h = ULTIMA_266c_GetChar()) != U5_KEY_SPACE && (key_h < 0x30 || key_h > 0x39))
    {
    }

    if (key_h != U5_KEY_SPACE && key_h != 0x30)
    {
        ULTIMA_16ba_PrintChar(key_h);
        ULTIMA_16ba_PrintChar('\n');

        wakeHour = (int)D_587f_hour + (int)key_h - 0x30;
        if (wakeHour > 0x17)
        {
            wakeHour -= 0x17;
        }

        for (i = 0; i < 0x10; i++)
        {
            NPC_0db4_MoveNpcs(D_587f_hour);
            ULTIMA_5910_UpdateFrame();
            if (D_65be_npcApproach == 0x61)
            {
                return;
            }
        }

        savedActive_b = D_587b_activePlayer;

        for (i = 0; i < D_585b_partyCount; i++)
        {
            if (D_55a8_party[i].status == STATUS_GOOD)
            {
                D_55a8_party[i].status = STATUS_SLEEP;
            }
        }

        ULTIMA_2900_UpdateVitalsDisplay();
        ULTIMA_1850_PrintString(_TEXT(0x421e, "Zzzzzzz...\n"));

        ULTIMA_0a70_GRAP_2d_SetPenColor(0);
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);

        lastHour_h = D_587f_hour;
        while (wakeHour != D_587f_hour)
        {
            ULTIMA_4f7c_AdvanceTime(10);
            if (lastHour_h != D_587f_hour && (D_587f_hour == 0x14 || D_587f_hour == 0x05))
            {
                TOWN_0170_SwapDayNightTiles();
            }

            lastHour_h = D_587f_hour;
            ULTIMA_4a84_DrawSunAndMoons();
            ULTIMA_2ae8_EndTurn();
            ULTIMA_2900_UpdateVitalsDisplay();
            TOWN_1694_PlaceActors();

            if (ULTIMA_368e_FindActorTileAtPos(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) != 0)
            {
                wakeHour = -1;
                break;
            }
            ULTIMA_20fa_WaitTicks(1);
        }

        if (wakeHour == -1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x422a, "Thrown out of bed!\n"));
        }

        for (i = 0; i < D_585b_partyCount; i++)
        {
            if (D_55a8_party[i].status == STATUS_SLEEP)
            {
                D_55a8_party[i].status = STATUS_GOOD;
            }
        }

        D_587b_activePlayer = savedActive_b;
        D_5896_mapX++;
        D_24e6_viewDirty = 1;
        D_5c5a_actors[0].x++;

        ULTIMA_2900_UpdateVitalsDisplay();
        ULTIMA_5910_UpdateFrame();
    }
}

// 1 when the party is on foot (the Avatar tile); else prints "On foot" and returns 0.
static int CMDS_06ee_RequireOnFoot(void)
{
    if (D_587c_partyTile != TILE_ACTOR_AVATAR && D_587c_partyTile != TILE_ACTOR_1D)
    {
        ULTIMA_1850_PrintString(_TEXT(0x423e, "\nOn foot\n"));
        return 0;
    }
    else
    {
        return 1;
    }
}

// 1 when the party is on foot, on a carpet or on a skiff; else prints "On foot" and returns 0.
static int CMDS_070c_CanBoardShip(void)
{
    switch (D_587c_partyTile)
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
        ULTIMA_1850_PrintString(_TEXT(0x4248, "\nOn foot\n"));
        return 0;
    }
}

static int CMDS_0788_IsLandAt(int viewY, int viewX);

// 1 when one of the four cells around the party is land.
static int CMDS_073e_IsLandNearby(void)
{
    if (CMDS_0788_IsLandAt(4, 5) == 1 || CMDS_0788_IsLandAt(6, 5) == 1 || CMDS_0788_IsLandAt(5, 6) == 1 || CMDS_0788_IsLandAt(5, 4) == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// 1 when the cell of the view is land: a walkable tile, or an actor tile of the ground kinds.
static int CMDS_0788_IsLandAt(int viewY, int viewX)
{
    int tile;

    tile = GetMapViewport(viewX, viewY);
    if (tile != 0)
    {
        return ULTIMA_2c4c_IsWalkableTile(0x1c, tile);
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
int CMDS_07f6_BoardCmd(void)
{
    byte tile;
    int hull;
    int skiffs_l;
    int actorSlot_g;
    int npc;

    if (D_5893_mapId > 0x20 && D_5893_mapId < 0x29)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4252, "\nNot here!\n"));

        return 1;
    }

    tile = ULTIMA_368e_FindActorTileAtPos(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    actorSlot_g = D_5876_tempX;
    if ((tile & 0xfe) == TILE_ACTOR_HORSE)
    {
        if (D_5893_mapId != 0)
        {
            npc = TOWN_011e_FindNpcByActor(actorSlot_g);
            if (npc != -1 && D_5f5e_npcs[npc].dialogNumber != 0)
            {
                ULTIMA_1850_PrintString(_TEXT(0x425e, "\"Nay!\"\n"));
                return 1;
            }
        }

        if (CMDS_06ee_RequireOnFoot() == 0)
        {
            return 1;
        }

        ULTIMA_1850_PrintString(_TEXT(0x4266, "horse\n"));

        D_587c_partyTile = tile + 2;
    }
    else if (tile == TILE_ACTOR_CARPET)
    {
        if (CMDS_06ee_RequireOnFoot() == 0)
        {
            return 1;
        }

        ULTIMA_1850_PrintString(_TEXT(0x426d, "carpet\n"));

        D_587c_partyTile = TILE_ACTOR_FLYING_CARPET;
    }
    else if ((tile & 0xfc) == TILE_ACTOR_SKIFF)
    {
        if (CMDS_06ee_RequireOnFoot() == 0)
        {
            return 1;
        }

        ULTIMA_1850_PrintString(_TEXT(0x4275, "skiff\n"));

        D_587c_partyTile = tile;
    }
    else if ((tile & 0xfc) == TILE_ACTOR_FRIGATE_24)
    {
        if (CMDS_070c_CanBoardShip() == 0)
        {
            return 1;
        }

        ULTIMA_1850_PrintString(_TEXT(0x427c, "Ship\n"));

        hull = D_5c5a_actors[actorSlot_g].value;
        if (hull < 10)
        {
            ULTIMA_1850_PrintString(_TEXT(0x4282, "\nDANGER: SHIP BADLY DAMAGED!\n"));
        }

        D_5c5a_actors[0].value = hull;
        skiffs_l = D_5c5a_actors[actorSlot_g].extra;

        if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET)
        {
            D_57b0_carpets++;
        }

        if ((D_587c_partyTile & 0xfc) == TILE_ACTOR_SKIFF)
        {
            skiffs_l++;
        }

        if (skiffs_l == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x42a0, "\nWARNING: NO SKIFFS ON BOARD!\n"));
        }

        D_587c_partyTile = tile;
        D_5c5a_actors[0].extra = skiffs_l;
        D_a9fa_vitalsDirty = 1;
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x42bf, "What?\n"));
        return 0;
    }

    ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, actorSlot_g);
    D_24e6_viewDirty |= 2;

#if defined(ENABLE_BGM)
    AUDIO_PlayBgmPerMap();
#endif

    return 1;
}

// Fire on the overworld: a frigate fires to either side (not ahead or astern), asking the direction; the
// first moving actor within three cells is hit for 1 to 20 and may be destroyed.
static void CMDS_0962_FireBroadsides(void)
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

    if (D_587c_partyTile < TILE_ACTOR_FRIGATE_20 || D_587c_partyTile > TILE_ACTOR_FRIGATE_20_END)
    {
        ULTIMA_1850_PrintString(_TEXT(0x42c6, "What?\n"));
    }
    else
    {
        if (ULTIMA_35ec_SelectDirection() == 0)
        {
            return;
        }

        dirX = D_5876_tempX;
        dirY = D_5878_tempY;

        if (dirX == 0 && (D_587c_partyTile & 1) == 0 || dirX != 0 && (D_587c_partyTile & 1) != 0)
        {
            // 09b2
            ULTIMA_1850_PrintString(_TEXT(0x42cd, "Fire broadsides only!\n"));
        }
        else
        {
            // 09b8
            x = D_5896_mapX;
            y = D_5897_mapY;
            ULTIMA_43ae_AudioSweepTone(1000, 200, 5, 300);

            // 09d8
            for (step_e = 0; step_e < 3; step_e++)
            {
                x += dirX;
                y += dirY;

                tile_c = ULTIMA_368e_FindActorTileAtPos(x, y, D_5895_mapLevel);

                if (MAINOUT_105c_IsMovingActor(tile_c) != 0 && (tile_c & 0xfc) != 0xec)
                {
                    // 0a10
                    actorSlot_f = D_5876_tempX;

                    targetX_b = D_5c5a_actors[actorSlot_f].x - D_5896_mapX + 5;
                    targetY = D_5c5a_actors[actorSlot_f].y - D_5897_mapY + 5;
                    hit_l = COMSUBS_12de_FlyMissile(5, 5, targetX_b, targetY, 1);
                    if (hit_l != 0)
                    {
                        ULTIMA_5910_UpdateFrame();
                        ULTIMA_3522_HitEffectAt(x, y);

                        D_5c5a_actors[actorSlot_f].value -= ULTIMA_2092_RandomRange(1, 0x14);
                        if (D_5c5a_actors[actorSlot_f].value > 0x7f)
                        {
                            ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, actorSlot_f);
                            D_24e6_viewDirty |= 2;
                        }
                    }

                    return;
                }
            }

            targetX_b = dirX * 3 + 5;
            targetY = dirY * 3 + 5;
            COMSUBS_12de_FlyMissile(5, 5, targetX_b, targetY, 1);
        }
    }
}

// The Fire command: on the overworld the frigate's broadsides; in a town or keep a cannon beside the party
// fires four cells along its facing, destroying a door (and the tile 0x44 left) or killing an actor, which
// costs karma 5 and kills its NPC; or hurts the party.
void CMDS_0aea_FireCmd(void)
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

    if (0x20 < D_5893_mapId && D_5893_mapId < 0x29)
    {
        ULTIMA_1850_PrintString(_TEXT(0x42e4, "What?\n"));
        return;
    }

    if (D_5893_mapId == 0)
    {
        CMDS_0962_FireBroadsides();
        return;
    }

    ULTIMA_39cc_SetTile(D_594f_openDoorTile, D_5950_openDoorX, D_5951_openDoorY);

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
        ULTIMA_1850_PrintString(_TEXT(0x42eb, "What?\n"));
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

    ULTIMA_1850_PrintString(_TEXT(0x42f2, "BOOOM!\n"));
    ULTIMA_43ae_AudioSweepTone(1000, 200, 5, 300);
    TOWN_0958_RaiseAlarm();

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

        tileAt_m = ULTIMA_3702_FindLastActorTileAt(x_h, y_g, D_5895_mapLevel);
        if (tileAt_m == 0)
        {
            switch (*ULTIMA_4402_GetTileAddr(x_h, y_g))
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
            target = D_5876_tempX;
        }
    }

    COMSUBS_12de_FlyMissile(startX, startY, endX, endY_p, 1);
    if (hitWall || hitActor_b)
    {
        ULTIMA_3522_HitEffectAt(x_h, y_g);
    }

    if (hitWall)
    {
        ULTIMA_1850_PrintString(_TEXT(0x42fa, "Door destroyed!\n"));
        *ULTIMA_4402_GetTileAddr(x_h, y_g) = TILE_MAP_44;
        D_24e6_viewDirty = 1;
        D_594f_openDoorTile = 0;
    }

    if (hitActor_b && target != 0)
    {
        ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, target);

        D_24e6_viewDirty |= 2;
        if (D_5888_karma > 5)
        {
            D_5888_karma -= 5;
        }
        else
        {
            D_5888_karma = 0;
        }

        if ((npc_e = TOWN_011e_FindNpcByActor(target)) == -1)
        {
            return;
        }

        TOWN_0052_SetNpcKilled(npc_e);
        TOWN_00b0_DespawnNpc(npc_e);
    }

    if (hitActor_b && target == 0)
    {
        ULTIMA_2aa8_DamageParty();
    }
}

// The Ignite torch command: uses a torch; the light lasts 240 turns (a random 112 to 127 more in a dungeon).
void CMDS_0d98_IgniteTorchCmd(void)
{
    if (D_57ae_torches == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x430b, "None owned!\n"));
    }
    else
    {
        D_57ae_torches--;
        if (D_5893_mapId > 0x20 && D_5893_mapId < 0x29)
        {
            ULTIMA_3ef0_IncreaseByte(&D_58a7_torchTurns, ULTIMA_2092_RandomRange(0, 0xf) + 0x70, 0xff);
        }
        else
        {
            D_58a7_torchTurns = 0xf0;
        }
    }
}

// The New order command: swaps two party members (the leader cannot be moved).
void CMDS_0ddc_NewOrderCmd(void)
{
    int first;
    int second;
    S_55a8 swap_f;

    ULTIMA_1850_PrintString(_TEXT(0x4318, "\n\nSwap "));
    if ((first = ULTIMA_2e8e_SelectPlayer()) == -1)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4320, "nobody!\n"));
        return;
    }

    ULTIMA_1850_PrintString(D_55a8_party[first].name);
    if (first == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4329, "\n\n"));
        ULTIMA_1850_PrintString(D_55a8_party[0].name);
        ULTIMA_1850_PrintString(_TEXT(0x432c, " must lead!\n"));
        return;
    }

    ULTIMA_1850_PrintString(_TEXT(0x4339, "\nwith "));

    if ((second = ULTIMA_2e8e_SelectPlayer()) == -1)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4340, "nobody!\n"));
        return;
    }

    ULTIMA_1850_PrintString(D_55a8_party[second].name);
    if (second == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4349, "\n\n"));
        ULTIMA_1850_PrintString(D_55a8_party[0].name);
        ULTIMA_1850_PrintString(_TEXT(0x434c, " must lead!\n"));
        return;
    }

    ULTIMA_1850_PrintString(_TEXT(0x4359, "!\n"));

    swap_f = D_55a8_party[first];
    D_55a8_party[first] = D_55a8_party[second];
    D_55a8_party[second] = swap_f;

    D_a9fa_vitalsDirty = 1;
}

// The Xit command: leaves the horse, carpet, skiff or ship (onto land only; a ship hands out a skiff or a
// carpet) and puts the vehicle back as an actor.
// CHECKED
void CMDS_0eb4_XitCmd(void)
{
    byte skiffs;
    byte tileUnder_g;
    int slot;
    byte newTile_i;

#if !defined(MATCHING_BUILD)
    skiffs = 0;
#endif

    if (D_5893_mapId < 0x20 && D_5893_mapId > 0x29) // TODO: sic; BUG
    {
        // ce4c
        ULTIMA_1850_PrintString(_TEXT(0x435c, "\nNot here!\n"));
        return; // -> cfaa
    }

    tileUnder_g = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);

    switch (D_587c_partyTile & 0xfc)
    {
    case TILE_ACTOR_AVATAR:
        ULTIMA_1850_PrintString(_TEXT(0x4368, "what?\n"));
        return;

    case TILE_ACTOR_FRIGATE_20:
        ULTIMA_1850_PrintString(_TEXT(0x436f, "\nUnder sail!\n"));
        return;

    case TILE_ACTOR_FLYING_CARPET:
        if (CMDS_073e_IsLandNearby() != 0 || ULTIMA_2c4c_IsWalkableTile(0x1c, tileUnder_g) != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x437d, "carpet!\n"));
            newTile_i = 0x1b;
            D_587c_partyTile = TILE_ACTOR_AVATAR;
            break;
        }

        if (CMDS_073e_IsLandNearby() == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x4386, "\nNo land nearby!\n"));
            return;
        }

        ULTIMA_1850_PrintString(_TEXT(0x4398, "\nNot here!\n"));
        return;

    case TILE_ACTOR_HORSE:
        ULTIMA_1850_PrintString(_TEXT(0x43a4, "horse!\n"));
        newTile_i = D_587c_partyTile - 2;
        D_587c_partyTile = TILE_ACTOR_AVATAR;
        break;

    case TILE_ACTOR_SKIFF:
        // cef2
        if (CMDS_073e_IsLandNearby() == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x43ac, "\nNo land nearby!\n"));
            return;
        }

        if ((tileUnder_g & 0xfe) == TILE_MAP_6A)
        {
            ULTIMA_1850_PrintString(_TEXT(0x43be, "\nNot here!\n"));
            return;
        }

        ULTIMA_1850_PrintString(_TEXT(0x43ca, "skiff!\n"));
        newTile_i = D_587c_partyTile;
        D_587c_partyTile = TILE_ACTOR_AVATAR;
        break;

    case TILE_ACTOR_FRIGATE_24:
        // cf1c
        ULTIMA_1850_PrintString(_TEXT(0x43d2, "ship!\n"));
        if (CMDS_073e_IsLandNearby() != 0)
        {
            newTile_i = D_587c_partyTile;
            D_587c_partyTile = TILE_ACTOR_AVATAR;

            skiffs = D_5c5a_actors[0].extra;
        }
        else if (D_5c5a_actors[0].extra != 0)
        {
            newTile_i = D_587c_partyTile;
            D_587c_partyTile += 4;
            skiffs = D_5c5a_actors[0].extra - 1;
        }
        else if (D_57b0_carpets != 0)
        {
            D_57b0_carpets--;
            newTile_i = D_587c_partyTile;
            D_587c_partyTile = TILE_ACTOR_FLYING_CARPET;

            skiffs = D_5c5a_actors[0].extra;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x43d9, "\nNo skiffs on board!\n"));
            return;
        }

        break;
    }

    slot = ULTIMA_38e4_FindActorSlot();
    ULTIMA_3a74_SetActor(newTile_i, newTile_i, D_5896_mapX, D_5897_mapY, D_5895_mapLevel, D_5c5a_actors[0].value, slot);
    D_5c5a_actors[slot].extra = skiffs; // BUG: skiffs NOT INITIALIZED when (D_587c & 0xfc) != 0x24
    ULTIMA_2900_UpdateVitalsDisplay();

#if defined(ENABLE_BGM)
    AUDIO_PlayBgmPerMap();
#endif
}

char* D_444a_shadowlordNames[3] =
{
    _STATIC_TEXT(0x43ef, "FAULINEI"),
    _STATIC_TEXT(0x43f8, "ASTAROTH"),
    _STATIC_TEXT(0x4401, "NOSFENTOR")
};

// A yelled shadowlord's name (FAULINEI, ASTAROTH, NOSFENTOR) on maps 0x1e to 0x20 calls that shadowlord up
// as an NPC two cells north (not when one is there, or when it is not allowed); returns 1 when nothing
// happened, else 0.
// CHECKED
int CMDS_1030_YellShadowlord(char* word)
{
    int slot;
    int which_o;
    int j_h;
    int result_c;
    int npc;

    result_c = 1;

    if (D_5893_mapId == 0x1e || D_5893_mapId == 0x1f || D_5893_mapId == 0x20)
    {
        for (which_o = 0; which_o < 3; which_o++)
        {
            if (ULTIMA_6f1e_CompareWord((byte*)D_444a_shadowlordNames[which_o], word) > -1)
                break;
        }

        if (which_o == 3 || D_5897_mapY < 2 || D_58c8_shadowlordTowns[which_o] == 0xff)
        {
            // d002
            ULTIMA_1850_PrintString(_TEXT(0x440b, "\nNo effect!\n"));
            RETURN_AX(result_c);
        }
        else
        {
            for (slot = 0; slot < 0x20; slot++)
            {
                if (D_5c5a_actors[slot].tile == TILE_ACTOR_SHADOWLORD)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x4418, "\nNo effect!\n"));
                    RETURN_AX(result_c);
                }
            }

            D_58cb_summonedLord = which_o;
            slot = ULTIMA_38e4_FindActorSlot();
            ULTIMA_3a74_SetActor(TILE_ACTOR_SHADOWLORD, TILE_ACTOR_SHADOWLORD, D_5896_mapX, D_5897_mapY - 2, D_5895_mapLevel, 0, slot);

            npc = 0x1f;
            for (j_h = 0x1f; j_h >= 0; j_h--)
            {
                if (D_659e_npcTypes[j_h] == 0)
                {
                    npc = j_h;
                    break;
                }
            }

            D_5f5e_npcs[npc].mode = 1;
            D_5f5e_npcs[npc].actorIdx = slot;
            D_5f5e_npcs[npc].x = D_5896_mapX;
            D_5f5e_npcs[npc].y = D_5897_mapY - 2;
            D_5f5e_npcs[npc].z = D_5895_mapLevel;

            for (j_h = 0; j_h < 4; j_h++)
            {
                D_5d5e_npcSchedules[npc].time[j_h] = 0;
            }

            for (j_h = 0; j_h < 3; j_h++)
            {
                D_5d5e_npcSchedules[npc].type[j_h] = 6;
                D_5d5e_npcSchedules[npc].x[j_h] = D_5896_mapX;
                D_5d5e_npcSchedules[npc].y[j_h] = D_5897_mapY - 2;
                D_5d5e_npcSchedules[npc].z[j_h] = D_5895_mapLevel;
            }

            D_659e_npcTypes[npc] = 0xfc;

            ULTIMA_1850_PrintString(_TEXT(0x4425, "\nA shadowlord appears\n"));
            ULTIMA_2192_AudioPulse(0x28a0, 1, 30000, 2000, 2);

            D_5c5a_actors[slot].tile = D_5c5a_actors[slot].animTile = TILE_ACTOR_CIRCLE;
            ULTIMA_1068_RevealTile(0x1fc, 5, 3);
            D_5c5a_actors[slot].tile = D_5c5a_actors[slot].animTile = TILE_ACTOR_SHADOWLORD;

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
        ULTIMA_1850_PrintString(_TEXT(0x443c, "\nNo effect!\n"));
        RETURN_AX(result_c);
    }
}
