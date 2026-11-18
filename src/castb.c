#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "cast.h"
#include "cast2.h"
#include "combat.h"
#include "comsubs.h"
#include "dnglook.h"
#include "dungeon.h"
#include "lookobj.h"
#include "town.h"
#include "zstats.h"
#include "time/time.h"

// CAST.OVL's last two functions are an object of their own, built with MSC 5.0, after
// cast.c's (MSC 5.1). They use no strings, so no string shows the compiler: compiled after
// cast.c's functions their loops were not optimized (pass 2's capacity), alone under MSC 5.1
// still not all of them; under MSC 5.0 both are exact. That object has no data, and CAST's
// _DATA is cast.c's alone. CAST_1bb0_PlotSprayPixel compiles the same either way, so the object may start
// there instead; this file takes the latest possible start.

// defined in cast.c
int CAST_0000_IsImmuneToSpells(int combatant);
int CAST_1bb0_PlotSprayPixel(int x, int y, int penColor);

// The spray of a cone spell: 21 rays of pixels leave the caster's side in a direction, each wavering by its own slope (D_1cf0_raySlopes),
// a random number of pixels a turn; the cells they cross are collected (once each, up to 63) into the two lists. Returns how many.
// CHECKED
// MSC 5.0: the inverted CAST_1bb0_PlotSprayPixel test, the switches' cases in the order UP, DOWN, RIGHT,
// LEFT (which gives the original's pointer temporaries and case layout), and local_5a
// renamed for 5.0's slot rule (local_5a and local_30_ac had swapped places).
static int CAST_1c36_SprayRays(s16* outXs, s16* outYs, int color, int caster, int direction)
{
    int cellCount;
    int side_c;

    s16 ended_h[21];
    int startY;
    int startX;
    int pixelStep_f;
    int ray_i;
    s16 rayY[21];

    s16 rayX[21];
    int pixelsThisTurn_h;
    int endedCount_f;

    cellCount = 0;

    startX = D_ba14_combatants[caster].x * 0x10 + 8;
    startY = D_ba14_combatants[caster].y * 0x10 + 8;

    switch (direction)
    {
    case DIR_UP:
        // dc04
        startX += 8;
        break;

    case DIR_RIGHT:
        // dc5a
        startX += 0x10;
        startY += 8;
        break;

    case DIR_LEFT:
        // dc5e
        startY += 8;
        break;

    case DIR_DOWN:
        // dc64
        startX += 8;
        startY += 0x10;
        break;
    }

    // dc08
    color += 8;
    for (ray_i = 0; ray_i < 0x15; ray_i++)
    {
        ended_h[ray_i] = 0; // dc1d
        D_a9d0_rayBalance[ray_i] = D_1cf0_raySlopes[ray_i]; // dc2a

        rayX[ray_i] = startX; // dc44
        rayY[ray_i] = startY; // dc49
    }

    // dc6e
    endedCount_f = 0;
    cellCount = 0;

    // dc7f
    do
    {
        endedCount_f = 0;

        for (ray_i = 0; ray_i < 0x15; ray_i++)
        {
            // dca7
            if (ended_h[ray_i] == 0)
            {
                // dcb3
                pixelsThisTurn_h = ULTIMA_3aae_Random(0xf);

#if defined(ENABLE_SPEED_FIX)
                TIME_SleepMs(3);
#endif

                for (pixelStep_f = 0; pixelStep_f < pixelsThisTurn_h; pixelStep_f++)
                {
                    if (CAST_1bb0_PlotSprayPixel(rayX[ray_i], rayY[ray_i], color) != 0)
                    {
                        if (cellCount < 0x3f && D_5876_tempX > -1 && D_5878_tempY > -1 && D_5876_tempX < 0xb && D_5878_tempY < 0xb &&
                            GetMapViewport(D_5876_tempX, D_5878_tempY) == 0)
                        {
                            cellCount++;

                            outXs[cellCount] = D_5876_tempX;
                            outYs[cellCount] = D_5878_tempY;

                            GetMapViewport(D_5876_tempX, D_5878_tempY) = 0xff;
                        }
                    }
                    else
                    {
                        ended_h[ray_i] = 1;
                    }

                    if (ended_h[ray_i] == 0)
                    {
                        D_a9d0_rayBalance[ray_i] -= 10;

                        switch (direction)
                        {
                        case DIR_UP:
                            rayY[ray_i]--;
                            break;

                        case DIR_DOWN:
                            rayY[ray_i]++;
                            break;

                        case DIR_RIGHT:
                            rayX[ray_i]++;
                            break;

                        case DIR_LEFT:
                            rayX[ray_i]--;
                            break;
                        }

                        if ((s16)D_a9d0_rayBalance[ray_i] < 1)
                        {
                            if (ray_i < 10)
                            {
                                side_c = -1;
                            }
                            else
                            {
                                side_c = 1;
                            }

                            switch (direction)
                            {
                            case DIR_UP:
                                rayX[ray_i] += side_c;
                                break;

                            case DIR_DOWN:
                                rayX[ray_i] -= side_c;
                                break;

                            case DIR_RIGHT:
                                rayY[ray_i] += side_c;
                                break;

                            case DIR_LEFT:
                                rayY[ray_i] -= side_c;
                                break;
                            }

                            D_a9d0_rayBalance[ray_i] += D_1cf0_raySlopes[ray_i];
                        }
                    }

                    if (ended_h[ray_i] != 0)
                    {
                        break;
                    }
                }
            }
            else
            {
                endedCount_f++;
            }
        }
    } while (endedCount_f < 0x15);

    ULTIMA_230e_PcspkOff();
    return cellCount;
}

// The four cone spells (kind 1 In Zu sleeps, 2 In Nox Hur poisons, 3 In Flam Hur burns, 4 In Vas Grav Corp kills outright; each
// in a color of its own): asks a direction, sprays, then every creature on a cell reached suffers its effect once (it may resist).
// CHECKED
// MSC 5.0: the first switch's cases ordered 1, default, 2; "ULTIMA_3abe_Random1To30() >= COMBAT_13e2_QueryStat()"
// (MSC calls the right-hand side first); the last loop's "> -1". The original's 10 bytes
// after the ret are the zero padding at the end of CAST.OVL.
void CAST_1f60_ConeSpell(int caster, int kind, int color)
{
    S_55a8* member_j;
    CombatEntity* target_l;
    int victim_h;
    int noisePitch_n;
    int cellIdx;
    s16 hitYs[64];
    s16 hitXs[64];
    int dirOrCount;

    cellIdx = 1;
    member_j = &D_55a8_party[D_ba14_combatants[caster].entityIdx];
#if !defined(MATCHING_BUILD)
    dirOrCount = CAST2_0306_AskDirection();
#else
    dirOrCount = CAST2_0306_AskDirection(caster); // NOTE: original code error?
#endif
#if !defined(MATCHING_BUILD) // added in FMT
    if (dirOrCount != 0)
#endif
    {
        switch (kind)
        {
        case 1:
            noisePitch_n = 16000;
            break;
        default:
            noisePitch_n = 0x5140;
            break;
        case 2:
            noisePitch_n = 0x4b00;
            break;
        }

        // df30
        ULTIMA_223c_AudioWhiteNoise(800, noisePitch_n, 700);

        dirOrCount = CAST_1c36_SprayRays(hitXs, hitYs, color, caster, dirOrCount);
        dirOrCount++;

        for (cellIdx = 1; cellIdx < dirOrCount; cellIdx++)
        {
            for (victim_h = 0x1f; victim_h > -1; victim_h--)
            {
                // dfac?
                target_l = &D_ba14_combatants[victim_h]; // local_114
                if (target_l->x == hitXs[cellIdx] && target_l->y == hitYs[cellIdx] &&
                    (target_l->turnTimer & 0x80) == 0 && (target_l->flags & COMBAT_FLAGS_DEAD) == 0 && target_l->flags != 0)
                {
                    target_l->turnTimer |= 0x80;
                    switch (kind)
                    {
                    case 1:
                        // e012
                        if (COMSUBS_0000_TargetResists(caster, victim_h, 0) == 0 && CAST_0000_IsImmuneToSpells(victim_h) == 0)
                        {
                            ULTIMA_3564_HitEffectOnActor(victim_h);
                            ULTIMA_68ae_PutToSleep(victim_h);
                            COMSUBS_0312_ReportHit(victim_h, caster); // -> e034 -> e0b2
                        }

                        break;

                    case 2:
                        // e03c
                        if (ULTIMA_3abe_Random1To30() >= COMBAT_13e2_QueryStat(victim_h, QUERY_STAT_DEX))
                        {
                            ULTIMA_3564_HitEffectOnActor(victim_h);
                            COMBAT_18ba_Poison(victim_h, caster);
                            COMSUBS_0312_ReportHit(victim_h, caster); // -> e034 -> e0b2
                        }

                        break;

                    case 3:
                        // e05c
                        ULTIMA_3564_HitEffectOnActor(victim_h);

                        ULTIMA_3f14_IncreaseInt(&member_j->exp, COMBAT_1574_ApplyDamage(victim_h, ULTIMA_3aae_Random(0x1e)), 9999);
                        COMSUBS_0312_ReportHit(victim_h, caster); // -> e034 -> e0b2
                        break;

                    case 4:
                        // e07e
                        if (COMSUBS_0000_TargetResists(caster, victim_h, 0) == 0 && CAST_0000_IsImmuneToSpells(victim_h) == 0)
                        {
                            ULTIMA_3564_HitEffectOnActor(victim_h);

                            ULTIMA_3f14_IncreaseInt(&member_j->exp, COMBAT_1574_ApplyDamage(victim_h, 99), 9999); // -> e070
                            COMSUBS_0312_ReportHit(victim_h, caster); // -> e034 -> e0b2
                        }
                        break;
                    }

                    break;
                }
                // e0a4
            }
            // e0b2
        }

        // e0c8
        for (victim_h = 0x1f; victim_h > -1; victim_h--)
        {
            // e0d1
            D_ba14_combatants[victim_h].turnTimer &= 0x7f;
        }
    }
}
