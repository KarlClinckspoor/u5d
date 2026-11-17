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
// _DATA is cast.c's alone. CAST_1bb0 compiles the same either way, so the object may start
// there instead; this file takes the latest possible start.

// defined in cast.c
int CAST_0000(int param_1);
int CAST_1bb0(int x, int y, int penColor);

// CHECKED
// MSC 5.0: the inverted CAST_1bb0 test, the switches' cases in the order UP, DOWN, RIGHT,
// LEFT (which gives the original's pointer temporaries and case layout), and local_5a
// renamed for 5.0's slot rule (local_5a and local_30_ac had swapped places).
static int CAST_1c36(s16* param_1, s16* param_2, int param_3, int param_4, int param_5)
{
    int local_90_ad;
    int local_8e;

    s16 local_8c[21];
    int local_62_ab;
    int local_60_ad;
    int local_5e;
    int local_5c;
    s16 local_5a_aa[21];

    s16 local_30_ac[21];
    int local_6;
    int local_4_ab;

    local_90_ad = 0;

    local_60_ad = D_ba14[param_4].x * 0x10 + 8;
    local_62_ab = D_ba14[param_4].y * 0x10 + 8;

    switch (param_5)
    {
    case DIR_UP:
        // dc04
        local_60_ad += 8;
        break;

    case DIR_RIGHT:
        // dc5a
        local_60_ad += 0x10;
        local_62_ab += 8;
        break;

    case DIR_LEFT:
        // dc5e
        local_62_ab += 8;
        break;

    case DIR_DOWN:
        // dc64
        local_60_ad += 8;
        local_62_ab += 0x10;
        break;
    }

    // dc08
    param_3 += 8;
    for (local_5c = 0; local_5c < 0x15; local_5c++)
    {
        local_8c[local_5c] = 0; // dc1d
        D_a9d0[local_5c] = D_1cf0[local_5c]; // dc2a

        local_30_ac[local_5c] = local_60_ad; // dc44
        local_5a_aa[local_5c] = local_62_ab; // dc49
    }

    // dc6e
    local_4_ab = 0;
    local_90_ad = 0;

    // dc7f
    do
    {
        local_4_ab = 0;

        for (local_5c = 0; local_5c < 0x15; local_5c++)
        {
            // dca7
            if (local_8c[local_5c] == 0)
            {
                // dcb3
                local_6 = ULTIMA_3aae_Random(0xf);

#if defined(ENABLE_SPEED_FIX)
                TIME_SleepMs(3);
#endif

                for (local_5e = 0; local_5e < local_6; local_5e++)
                {
                    if (CAST_1bb0(local_30_ac[local_5c], local_5a_aa[local_5c], param_3) != 0)
                    {
                        if (local_90_ad < 0x3f && D_5876 > -1 && D_5878 > -1 && D_5876 < 0xb && D_5878 < 0xb &&
                            GetMapViewport(D_5876, D_5878) == 0)
                        {
                            local_90_ad++;

                            param_1[local_90_ad] = D_5876;
                            param_2[local_90_ad] = D_5878;

                            GetMapViewport(D_5876, D_5878) = 0xff;
                        }
                    }
                    else
                    {
                        local_8c[local_5c] = 1;
                    }

                    if (local_8c[local_5c] == 0)
                    {
                        D_a9d0[local_5c] -= 10;

                        switch (param_5)
                        {
                        case DIR_UP:
                            local_5a_aa[local_5c]--;
                            break;

                        case DIR_DOWN:
                            local_5a_aa[local_5c]++;
                            break;

                        case DIR_RIGHT:
                            local_30_ac[local_5c]++;
                            break;

                        case DIR_LEFT:
                            local_30_ac[local_5c]--;
                            break;
                        }

                        if ((s16)D_a9d0[local_5c] < 1)
                        {
                            if (local_5c < 10)
                            {
                                local_8e = -1;
                            }
                            else
                            {
                                local_8e = 1;
                            }

                            switch (param_5)
                            {
                            case DIR_UP:
                                local_30_ac[local_5c] += local_8e;
                                break;

                            case DIR_DOWN:
                                local_30_ac[local_5c] -= local_8e;
                                break;

                            case DIR_RIGHT:
                                local_5a_aa[local_5c] += local_8e;
                                break;

                            case DIR_LEFT:
                                local_5a_aa[local_5c] -= local_8e;
                                break;
                            }

                            D_a9d0[local_5c] += D_1cf0[local_5c];
                        }
                    }

                    if (local_8c[local_5c] != 0)
                    {
                        break;
                    }
                }
            }
            else
            {
                local_4_ab++;
            }
        }
    } while (local_4_ab < 0x15);

    ULTIMA_230e_PcspkOff();
    return local_90_ad;
}

// CHECKED
// MSC 5.0: the first switch's cases ordered 1, default, 2; "ULTIMA_3abe() >= COMBAT_13e2()"
// (MSC calls the right-hand side first); the last loop's "> -1". The original's 10 bytes
// after the ret are the zero padding at the end of CAST.OVL.
void CAST_1f60(int param_1, int param_2, int param_3)
{
    S_55a8* local_104c;
    CombatEntity* local_106b;
    int local_108a;
    int local_10ai;
    int local_10ch;
    s16 local_102d[64];
    s16 local_82l[64];
    int local_10eg;

    local_10ch = 1;
    local_104c = &D_55a8_party[D_ba14[param_1].entityIdx];
#if !defined(MATCHING_BUILD)
    local_10eg = CAST2_0306();
#else
    local_10eg = CAST2_0306(param_1); // NOTE: original code error?
#endif
#if !defined(MATCHING_BUILD) // added in FMT
    if (local_10eg != 0)
#endif
    {
        switch (param_2)
        {
        case 1:
            local_10ai = 16000;
            break;
        default:
            local_10ai = 0x5140;
            break;
        case 2:
            local_10ai = 0x4b00;
            break;
        }

        // df30
        ULTIMA_223c_AudioWhiteNoise(800, local_10ai, 700);

        local_10eg = CAST_1c36(local_82l, local_102d, param_3, param_1, local_10eg);
        local_10eg++;

        for (local_10ch = 1; local_10ch < local_10eg; local_10ch++)
        {
            for (local_108a = 0x1f; local_108a > -1; local_108a--)
            {
                // dfac?
                local_106b = &D_ba14[local_108a]; // local_114
                if (local_106b->x == local_82l[local_10ch] && local_106b->y == local_102d[local_10ch] &&
                    (local_106b->turnTimer & 0x80) == 0 && (local_106b->flags & COMBAT_FLAGS_DEAD) == 0 && local_106b->flags != 0)
                {
                    local_106b->turnTimer |= 0x80;
                    switch (param_2)
                    {
                    case 1:
                        // e012
                        if (COMSUBS_0000(param_1, local_108a, 0) == 0 && CAST_0000(local_108a) == 0)
                        {
                            ULTIMA_3564(local_108a);
                            ULTIMA_68ae(local_108a);
                            COMSUBS_0312(local_108a, param_1); // -> e034 -> e0b2
                        }

                        break;

                    case 2:
                        // e03c
                        if (ULTIMA_3abe() >= COMBAT_13e2(local_108a, QUERY_STAT_DEX))
                        {
                            ULTIMA_3564(local_108a);
                            COMBAT_18ba(local_108a, param_1);
                            COMSUBS_0312(local_108a, param_1); // -> e034 -> e0b2
                        }

                        break;

                    case 3:
                        // e05c
                        ULTIMA_3564(local_108a);

                        ULTIMA_3f14_IncreaseInt(&local_104c->exp, COMBAT_1574(local_108a, ULTIMA_3aae_Random(0x1e)), 9999);
                        COMSUBS_0312(local_108a, param_1); // -> e034 -> e0b2
                        break;

                    case 4:
                        // e07e
                        if (COMSUBS_0000(param_1, local_108a, 0) == 0 && CAST_0000(local_108a) == 0)
                        {
                            ULTIMA_3564(local_108a);

                            ULTIMA_3f14_IncreaseInt(&local_104c->exp, COMBAT_1574(local_108a, 99), 9999); // -> e070
                            COMSUBS_0312(local_108a, param_1); // -> e034 -> e0b2
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
        for (local_108a = 0x1f; local_108a > -1; local_108a--)
        {
            // e0d1
            D_ba14[local_108a].turnTimer &= 0x7f;
        }
    }
}
