#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "npc.h"
#include "town.h"

static int NPC_032c(int a, int b, int c, int d, int e, int f);
static int NPC_06a0(int param_1, int param_2, int param_3, int param_4);
static int NPC_0adc(int param_1, int param_2, int param_3, int param_4, int param_5);
static int NPC_0b9e(int param_1, int param_2, int param_3, int param_4);
u8 D_367e[0x20] = {
    0x70, 0x0c, 0xff, 0xf8, 0x01, 0xf3, 0xff, 0xff,
    0x72, 0x3f, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff,
    0xfe, 0xff, 0x0f, 0xff, 0xff, 0xdf, 0xff, 0x5f,
    0xf0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void NPC_0000_LoadNpcFile(void)
{
    int local_2a;
    int local_28;
    byte local_26[32];
    char* local_6;
    int local_4_ab;

#if !defined(MATCHING_BUILD)
    local_6 = NULL;
#endif

    D_5893_map_id--;

    local_4_ab = (uint)D_5893_map_id >> 3;
    switch (local_4_ab)
    {
    case 0:
        local_6 = _TEXT(0x6d46, "towne.npc");
        break;
    case 1:
        local_6 = _TEXT(0x6d50, "dwelling.npc");
        break;
    case 2:
        local_6 = _TEXT(0x6d5e, "castle.npc");
        break;
    case 3:
        local_6 = _TEXT(0x6d6a, "keep.npc");
        break;
    }

    local_28 = (uint)D_5893_map_id - (local_4_ab << 3);

    local_2a = local_28 * 0x240;
    ULTIMA_256e_ReadFileFromDisk(local_6, D_5d5e, 0x200, local_2a);
    ULTIMA_256e_ReadFileFromDisk(local_6, D_659e, 0x20, local_2a + 0x200);
    ULTIMA_256e_ReadFileFromDisk(local_6, local_26, 0x20, local_2a + 0x220);

    for (local_4_ab = 0; local_4_ab < 0x20; local_4_ab++)
    {
        D_5f5e[local_4_ab]._a = (u16)local_26[local_4_ab];
    }

    D_5893_map_id++;
}

void NPC_00d6(int param_1)
{
    int local_6_an;
    int local_4;

    for (local_4 = 1; local_4 < 0x20; local_4++)
    {
        if (D_659e[local_4] == 0)
        {
            D_5f5e[local_4]._0 = 0;
        }
        else
        {
            local_6_an = NPC_12e0(local_4, param_1);
            D_5f5e[local_4]._0 = 1;

            D_5f5e[local_4]._2_x = D_5d5e[local_4].x[local_6_an];
            D_5f5e[local_4]._4_y = D_5d5e[local_4].y[local_6_an];
            D_5f5e[local_4]._6_z = D_5d5e[local_4].z[local_6_an];

            D_5f5e[local_4]._8 = D_659e[local_4];
            D_5f5e[local_4]._e = local_6_an;
            D_5f5e[local_4]._c = 0;

            D_655e[local_4] = -1;
            D_615e[local_4].data[0] = 0;
            D_65c2[local_4] = 0;
        }
    }
}

static int NPC_01a0(int param_1, int param_2, int param_3, int param_4)
{
    int local_4;

    if (param_3 != 0)
    {
        local_4 = -1;
    }
    else
    {
        local_4 = -2;
    }

    return NPC_032c(param_1, param_2, 0, 0, local_4, param_4);
}

// CHECKED (loop)
static void NPC_01d2(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6)
{
    byte local_e_ak;
    ActorFmt* local_c_n;
    int local_a;
    int local_8;
    int local_6;
    NpcFmt* local_4_ab;

    if (param_5 == -1)
    {
        local_e_ak = 0xc8;
    }

    if (param_5 == -2)
    {
        local_e_ak = 0xc9;
    }

    local_a = NPC_12e0(param_6, D_587f);
    for (local_6 = 0; local_6 < 0x20; local_6 = local_6 + 1)
    {
        for (local_8 = 0; local_8 < 0x20; local_8 = local_8 + 1)
        {
            if (NPC_0adc(local_8, local_6, D_5895_map_level, param_6, local_a) != 0)
            {
                D_b11c[local_8 + local_6 * 0x20] = 0;
            }
            else
            {
                D_b11c[local_8 + local_6 * 0x20] = 0x90;
            }

            if (param_5 < 0 && GetMap(local_8, local_6) == local_e_ak)
            {
                D_b11c[local_8 + local_6 * 0x20] = 5;
            }
        }
    }

    // 0272
    local_4_ab = &D_5f5e[param_6];

    for (local_6 = 0x1f; local_6 > 0; --local_6)
    {
        local_c_n = &D_5c5a[local_6];
        if (local_c_n->_0_tile > TILE_ACTOR_WIZARD - 1)
        {
            if (NPC_06a0(local_c_n->_2_x, local_c_n->_3_y, local_4_ab->_2_x, local_4_ab->_4_y) < 4)
            {
                D_b11c[local_c_n->_3_y * 0x20 + local_c_n->_2_x] = 0x90;
            }
        }
    }

    local_c_n = &D_5c5a[0];
    if (NPC_06a0(local_c_n->_2_x, local_c_n->_3_y, local_4_ab->_2_x, local_4_ab->_4_y) < 4)
    {
        D_b11c[local_c_n->_3_y * 0x20 + local_c_n->_2_x] = 0x90;
    }

    if (param_5 > -1)
    {
        D_b11c[param_4 * 0x20 + param_3] = 5;
    }

    D_b11c[param_2 * 0x20 + param_1] = 0x46;
}

// CHECKED
static int NPC_032c(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6)
{
    byte local_50_am[32];
    int local_30_ab_am;
    byte local_2e_an_am[32];
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_e_am = 0;
    local_c = 0;
    local_4_ab = 1;

    NPC_01d2(param_1, param_2, param_3, param_4, param_5, param_6);

    local_2e_an_am[0] = param_1;
    local_50_am[0] = param_2;

    do
    {
        if (local_e_am != 0)
            break;

        local_a = local_2e_an_am[local_c];
        local_8 = local_50_am[local_c];
        local_30_ab_am = D_b11c[local_8 * 0x20 + local_a] >> 4;

        for (local_6 = 0; local_6 < 4; local_6++)
        {
            switch (local_30_ab_am)
            {
            case 1:
                local_a--;
                if (local_a < 0)
                {
                    local_e_am = 2;
                }
                break;

            case 2:
                local_8++;
                if (local_8 > 0x20)
                {
                    local_e_am = 2;
                }
                break;

            case 3:
                local_a++;
                if (local_a > 0x20)
                {
                    local_e_am = 2;
                }
                break;

            case 4:
                local_8--;
                if (local_8 < 0)
                {
                    local_e_am = 2;
                }
                break;

            default:
                break;
            }

#if !defined(MATCHING_BUILD)
            if (local_a >= 0x20 || local_8 >= 0x20)
            {
                local_e_am = 2;
            }
#endif

            if (local_e_am != 2)
            {
                if (D_b11c[local_8 * 0x20 + local_a] < 0x10)
                {
                    local_e_am = D_b11c[local_8 * 0x20 + local_a] & 0xf;
                    D_b11c[local_8 * 0x20 + local_a] = (byte)(local_30_ab_am << 4);
                    if (local_e_am == 5)
                    {
                        local_e_am = 1;
                        D_5876 = local_a;
                        D_5878 = local_8;
                        break;
                    }

                    if (local_4_ab != local_c)
                    {
                        local_2e_an_am[local_4_ab] = local_a;
                        local_50_am[local_4_ab] = local_8;
                        local_4_ab++;
                    }

                    if (local_4_ab >= 0x20)
                    {
                        local_4_ab = 0;
                    }
                }
            }

            local_e_am = 0;
            local_30_ab_am = (local_30_ab_am & 3) + 1;
            local_a = local_2e_an_am[local_c];
            local_8 = local_50_am[local_c];
        }

        if (local_e_am == 1)
            break;

        local_c++;
        if (local_c == 0x20)
        {
            local_c = 0;
        }
    } while (local_c != local_4_ab);

    // NOT MATCHING by its block order only: the original places this exit block (store
    // local_e_am, "return 1") right after the "== 1" test, ahead of local_c++.
    if (local_e_am != 0)
        return 1;
    return local_e_am;
}

// CHECKED
static int NPC_04ac(int param_1, int param_2, int param_3, int param_4)
{
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    uint local_4_ab;

    local_6 = 0;
    local_a = 0;

    D_655e[param_1] = 0;

    local_8 = D_b11c[param_4 * 0x20 + param_3] & 0xf;
    local_4_ab = D_b11c[param_4 * 0x20 + param_3] >> 4;

    local_c = local_4_ab;

    do
    {
        switch (local_4_ab)
        {
        case 1:
            param_3++;
            break;
        case 2:
            param_4--;
            break;
        case 3:
            param_3--;
            break;
        case 4:
            param_4++;
            break;
        }

        if (local_c == local_4_ab && local_8 != 6)
        {
            local_6++;
        }

        if (local_c != local_4_ab || local_8 == 6)
        {
            D_615e[param_1].data[local_a] = local_6;
            local_a++;
            D_615e[param_1].data[local_a] = local_c;
            local_a++;

            if (local_8 == 6)
                break;

            local_c = local_4_ab;
            local_6 = 1;
        }

        local_4_ab = D_b11c[param_4 * 0x20 + param_3] >> 4;
        local_8 = D_b11c[param_4 * 0x20 + param_3] & 0xf;
    } while (local_a < 0x20);

    local_6 = 0;
    local_8 = local_a - 2;

    if (local_8 >= 0)
    {
        do
        {
            // swap
            local_c = D_615e[param_1].data[local_6];
            D_615e[param_1].data[local_6] = D_615e[param_1].data[local_8];
            D_615e[param_1].data[local_8] = local_c;

            local_8++;
            local_6++;

            // swap*
            local_c = (D_615e[param_1].data[local_6] + 1 & 3) + 1;
            D_615e[param_1].data[local_6] = (D_615e[param_1].data[local_8] + 1 & 3) + 1;
            D_615e[param_1].data[local_8] = local_c;

            local_6++;
            local_8 -= 3;
        } while (local_8 >= local_6);
    }

    return local_a;
}

// set walk direction
static void NPC_0632(int param_1)
{
    switch (param_1)
    {
    case 1:
        D_5876++;
        if (D_5878 > 0x20)
        {
            D_5876 = 0x20;
        }
        break;

    case 2:
        D_5878--;
        if (D_5876 < 0)
        {
            D_5878 = 0;
        }
        break;

    case 3:
        D_5876--;
        if (D_5878 < 0)
        {
            D_5876 = 0;
        }
        break;

    case 4:
        D_5878++;
        if (D_5876 > 0x20)
        {
            D_5878 = 0x20;
        }
        break;
    }
}

// Calculate manhattan distance
static int NPC_06a0(int param_1, int param_2, int param_3, int param_4)
{
    int local_6_an;
    int local_4;

    local_4 = param_1 - param_3;
    local_6_an = param_2 - param_4;

    if (local_4 < 0)
    {
        local_4 = -local_4;
    }

    if (local_6_an < 0)
    {
        local_6_an = -local_6_an;
    }

    local_4 += local_6_an;
    return local_4;
}

// CHECKED
static void NPC_06e4(int param_1, int param_2)
{
    int local_1c_aa;
    int local_1a_ac;
    ActorFmt* local_18_al;
    int local_16_an;
    int local_14;
    int local_12;
    int local_10[4];
    NpcFmt* local_8;
    int local_6_an_ab;
    int local_4_ab;

    CHECK_PARAM_RANGE(param_1, 0, 32);
    CHECK_PARAM_RANGE(param_2, 0, 3);

    local_18_al = &D_5c5a[0];
    local_8 = &D_5f5e[param_1];
    local_4_ab = D_5d5e[param_1].type[param_2];
    local_6_an_ab = NPC_06a0(D_5c5a[0]._2_x, D_5c5a[0]._3_y, local_8->_2_x, local_8->_4_y);
    if (local_6_an_ab == 1 && local_4_ab > 3)
    {
        if (local_4_ab == 4 || local_4_ab == 5)
        {
            // a9cd
            if (local_8->_a == 0)
            {
                goto move;
            }
            // a9d6
            D_65be = 0x74;
            // a9db
            D_65bf = param_1;
            local_4_ab = 1;
        }
        else
        {
            // aa4e: -> a9db
            D_65be = 0x61;
            D_65bf = param_1;
            local_4_ab = 1;
        }
    }
    else
    {
    move:
        // a9ea
        for (local_16_an = 1; local_16_an < 5; local_16_an++)
        {
            // aa09
            D_5876 = local_8->_2_x;
            D_5878 = local_8->_4_y;
            NPC_0632(local_16_an);
            local_12 = D_5876;
            local_14 = D_5878;
            if (NPC_0b9e(D_5876, D_5878, param_1, -1) != 0)
            {
                local_10[local_16_an - 1] = NPC_06a0(local_18_al->_2_x, local_18_al->_3_y, local_12, local_14);
            }
            else
            {
                // aa56 (OK)
                local_10[local_16_an - 1] = 99;
            }
        }

        // aa6a (OK)
        local_1a_ac = -1; // di
        local_6_an_ab = NPC_06a0(local_18_al->_2_x, local_18_al->_3_y, local_8->_2_x, local_8->_4_y);
        // aa8c
        for (local_16_an = 1; local_16_an < 8; local_16_an++)
        {
            // aaa7
            if (local_16_an < 5 && local_10[local_16_an - 1] != 99)
            {
                if (local_4_ab == 3)
                {
                    if (local_6_an_ab < local_10[local_16_an - 1])
                    {
                        if (local_1a_ac == -1 || ULTIMA_2092_RandomRange(0, 1) != 0)
                        {
                            local_1a_ac = local_16_an;
                        }

                        if (local_16_an == 4)
                            break;
                    }
                }
                else
                {
                    // ab14
                    if (local_10[local_16_an - 1] < local_6_an_ab)
                    {
                        local_1a_ac = local_16_an;
                        break;
                    }
                }
            }

#if defined(MATCHING_BUILD)
            // aae4 (OK)
            if (local_16_an > 4 && local_10[local_16_an - 1] != 99) // BUG? (OOB, invalid index)
#else
            if (local_16_an > 4 && local_10[local_16_an - 5] != 99) // BUGFIX?
#endif
            {
                if (local_10[local_16_an - 5] == local_6_an_ab)
                {
                    local_1a_ac = local_16_an;
                    break;
                }
            }
        }

        // ab22 (OK below)
        if ((local_4_ab == 5 || local_4_ab == 7) && ULTIMA_2092_RandomRange(0, 0x3f) < 0x10)
        {
            // ab3d
            local_1c_aa = local_1a_ac;
            for (local_16_an = 1; local_16_an < 5; local_16_an++)
            {
                // ab4b
                if (local_1a_ac != local_16_an && local_10[local_16_an - 1] != 99 &&
                    (local_1c_aa == local_1a_ac || ULTIMA_2092_RandomRange(0, 0x3f) < 0x10))
                {
                    local_1c_aa = local_16_an;
                }
            }
            local_1a_ac = local_1c_aa;
        }

        if (local_1a_ac > -1)
        {
            D_5876 = local_8->_2_x;
            D_5878 = local_8->_4_y;
            NPC_0632(local_1a_ac);

            D_5c5a[local_8->_c]._2_x = local_8->_2_x = D_5876;
            D_5c5a[local_8->_c]._3_y = local_8->_4_y = D_5878;
            D_24e6 |= 2;
        }
    }
}

// CHECKED
static int NPC_0938(int param_1, int param_2)
{
    NpcFmt* local_8;
    int local_a;
    int local_6;
    NpcScheduleFmt* local_4_c;

    local_a = -1;
    local_8 = &D_5f5e[param_1];
    local_4_c = &D_5d5e[param_1];

    for (local_6 = 0; local_6 < 4; local_6++)
    {
        if (local_4_c->time[local_6] == param_2)
        {
            local_a = NPC_12e0(param_1, param_2);
            if (local_a == (int)local_8->_e)
            {
                local_8->_0 = 1;
                break;
            }

            if (local_8->_6_z != D_5895_map_level && local_4_c->z[local_a] != D_5895_map_level)
            {
                local_8->_0 = 8;
            }
            else
            {
                if (local_8->_6_z == D_5895_map_level)
                {
                    if (local_4_c->z[local_a] == D_5895_map_level)
                    {
                        local_8->_0 = 2;
                    }
                    else if ((char)D_5895_map_level < (char)local_4_c->z[local_a])
                    {
                        local_8->_0 = 6;
                    }
                    else
                    {
                        local_8->_0 = 7;
                    }
                }
                else if ((char)D_5895_map_level < (char)local_8->_6_z)
                {
                    local_8->_0 = 4;
                }
                else
                {
                    local_8->_0 = 5;
                }
            }
            break;
        }
    }

    // ac1b
    if (-1 < local_a && local_8->_2_x == (uint)local_4_c->x[local_a] &&
        local_8->_4_y == (uint)local_4_c->y[local_a] &&
        local_8->_6_z == (uint)local_4_c->z[local_a])
    {
        local_a = 0;
        local_8->_0 = 1;
    }

    if (local_a == -1)
    {
        local_a = 0;
    }
    else
    {
        local_a++;
    }

    return local_a;
}

static int NPC_0a4a(int param_1, int param_2)
{
    byte local_8;
    int local_6;
    NpcFmt* local_4_c;

    local_4_c = &D_5f5e[param_1];

    // 0a4a
    local_8 = *ULTIMA_4402_GetTileAddr(D_5c5a[local_4_c->_c]._2_x, D_5c5a[local_4_c->_c]._3_y);
    if ((char)D_5d5e[param_1].z[param_2] < (char)D_5895_map_level)
    {
        if (local_8 != TILE_MAP_LADDER_DOWN)
        {
            // 0a92
            if ((local_8 & 0xf4) == TILE_MAP_STAIR)
            {
                local_6 = 1;
            }
            else
            {
                local_6 = 0;
            }
        }
        else
        {
            // 0a9b
            local_6 = 1;
        }
    }
    else
    {
        // 0aa2
        local_8 = *ULTIMA_4402_GetTileAddr(D_5c5a[local_4_c->_c]._2_x, D_5c5a[local_4_c->_c]._3_y);
        if (local_8 == TILE_MAP_LADDER_UP)
        {
            // -> 0a9b
            local_6 = 1;
        }
        else
        {
            if ((local_8 & 0xf4) == TILE_MAP_STAIR)
            {
                local_6 = 1;
            }
            else
            {
                local_6 = 0;
            }
        }
    }

    return local_6;
}

static int NPC_0adc(int param_1, int param_2, int param_3, int param_4, int param_5)
{
    byte local_8;
    int local_6;
    NpcScheduleFmt* local_4_c;

    local_6 = 0;
    local_4_c = &D_5d5e[param_4];

    if (param_5 > -1 && local_4_c->x[param_5] == param_1 && local_4_c->y[param_5] == param_2 && local_4_c->z[param_5] == param_3)
    {
        local_6 = 2;
    }
    else
    {
        if (param_1 < 0 || param_2 < 0 || 0x1f < param_1 || 0x1f < param_2)
        {
            local_8 = D_6a07;
        }
        else
        {
            local_8 = GetMap(param_1, param_2);
        }

        if (D_5f5e[param_4]._0 == 3 && (local_8 == TILE_MAP_LADDER_UP || local_8 == TILE_MAP_LADDER_DOWN))
        {
            local_6 = 1;
        }
        else
        {
            local_6 = ((uint)D_367e[local_8 >> 3] & 0x80 >> (local_8 & 7)) == 0;
        }
    }

    return local_6;
}

static int NPC_0b9e(int param_1, int param_2, int param_3, int param_4)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_6 = 0;
    if (param_1 < 0 || param_1 > 0x1f || param_2 < 0 || param_2 > 0x1f)
    {
        return 0;
    }

    local_8 = *ULTIMA_4402_GetTileAddr(param_1, param_2) & 0xfc;
    if (D_659e[param_3] == 0xfc)
    {
        local_4_ab = 1;
    }
    else if (local_8 == TILE_MAP_30)
    {
        local_4_ab = 1;
    }
    else if (local_8 == TILE_MAP_CHAIR_90 && D_5f5e[param_3]._0 != 2)
    {
        local_4_ab = 0;
    }
    else
    {
        local_4_ab = NPC_0adc(param_1, param_2, D_5895_map_level, param_3, param_4);
    }

    local_6 = ULTIMA_3702(param_1, param_2, D_5895_map_level);
    if (local_4_ab == 0 || local_6 != 0)
    {
        local_4_ab = 0;
    }

    return local_4_ab;
}

// move npc randomly
static void NPC_0c50(NpcFmt* param_1, int param_2, int param_3, int param_4, NpcScheduleFmt* param_5)
{
    int local_8;
    int local_6;
    int local_4_ab;

    if ((ULTIMA_2092_RandomRange(0, 0xff) & 8) != 0)
    {
        local_4_ab = ULTIMA_3aae_Random(0x40);
        local_4_ab = (local_4_ab & 3) + 1;
        D_5876 = param_1->_2_x;
        D_5878 = param_1->_4_y;
        NPC_0632(local_4_ab);
        local_6 = D_5876;
        local_8 = D_5878;

        if ((param_2 == 0 || NPC_06a0(param_5->x[param_3], param_5->y[param_3], D_5876, D_5878) <= param_2) &&
            NPC_0b9e(D_5876, D_5878, param_4, param_3) != 0)
        {
            D_5c5a[param_1->_c]._2_x = param_1->_2_x = local_6;
            D_5c5a[param_1->_c]._3_y = param_1->_4_y = local_8;

            D_24e6 |= 2;
        }
    }
}

// CHECKED (switch order)
static void NPC_0d00(int param_1, int param_2)
{
    NpcScheduleFmt* local_6;
    NpcFmt* local_4_c;

    local_4_c = &D_5f5e[param_1];
    local_6 = &D_5d5e[param_1];

    switch (local_6->type[param_2])
    {
    case 0:
        break;

    case 2:
        NPC_0c50(local_4_c, 0, param_2, param_1, local_6);
        break;

    case 4:
        if (NPC_06a0(D_5896_map_x, D_5897_map_y, local_6->x[param_2], local_6->y[param_2]) >= 4)
        {
            NPC_0c50(local_4_c, 3, param_2, param_1, local_6);
            break;
        }
        NPC_06e4(param_1, param_2);
        break;

    case 1:
        NPC_0c50(local_4_c, 3, param_2, param_1, local_6);
        break;

    case 3:
    case 6:
        if (NPC_06a0(D_5896_map_x, D_5897_map_y, local_4_c->_2_x, local_4_c->_4_y) < 4)
        {
            NPC_06e4(param_1, param_2);
        }
        break;

    case 5:
    case 7:
        NPC_06e4(param_1, param_2);
        break;
    }
}

// NOT MATCHING (checked)
// move npc
void NPC_0db4(int param_1)
{
    int local_14_am;
    byte local_12_ab_am;
    int local_10_ad_am;
    int local_e_am;
    int local_c;
    int local_a;
    int local_6;
    int local_4_ab;
    NpcFmt* local_8;

    local_14_am = 0;
    D_65be = 0;
    D_65bf = 0;

    // 0dd9
    for (local_4_ab = 1; /*1267*/ local_4_ab < 0x20; /*1264*/ local_4_ab++)
    {
        // 126d
        if (D_659e[local_4_ab] == 0)
            continue;

        // 1277
        local_e_am = NPC_12e0(local_4_ab, param_1);
        // 1281
        local_8 = &D_5f5e[local_4_ab];

        // 128e
        if (local_8->_0 <= 1 && NPC_0938(local_4_ab, param_1) == 0)
        {
            // 124e
            if (local_8->_6_z == D_5895_map_level)
            {
                // 1261
                NPC_0d00(local_4_ab, local_e_am);
            }
        }
        // 12a2
        else if (local_8->_0 > 3)
        {
            if (local_8->_0 == 4 || local_8->_0 == 5)
            {
                // 12bc
                if (local_14_am != 1)
                {
                    local_14_am = 1;
                    if (local_8->_0 == 4)
                    {
                        // 12cf
                        local_6 = 3;
                        // -> 0dd9
                    }
                    else
                    {
                        // 0dd4
                        local_6 = 4;
                    }

                    // 0dd9 (ok)
                    local_10_ad_am =
                        NPC_01a0(D_5d5e[local_4_ab].x[local_e_am], D_5d5e[local_4_ab].y[local_e_am], local_6 == 3, local_4_ab);
                    if (local_10_ad_am != 0)
                    {
                        local_a = D_5876;
                        local_c = D_5878;
                        local_10_ad_am = NPC_032c(local_a, local_c, D_5d5e[local_4_ab].x[local_e_am],
                                            D_5d5e[local_4_ab].y[local_e_am], local_6, local_4_ab);
                    }

                    // 0e32 (ok)
                    if (local_10_ad_am != 0)
                    {
                        // 0e3b (ok)
                        NPC_04ac(local_4_ab, local_6, D_5d5e[local_4_ab].x[local_e_am], D_5d5e[local_4_ab].y[local_e_am]);

                        // 0e5a (ok)
                        local_12_ab_am = GetMap(local_a, local_c);
                        if (/*0e6b*/ (local_6 == 3 && local_12_ab_am == TILE_MAP_LADDER_UP) ||
                            /*0e75*/ (local_6 == 4 && local_12_ab_am == TILE_MAP_LADDER_DOWN) ||
                            /*0e81*/ (local_12_ab_am & 0xfc) == TILE_MAP_STAIR)
                        {
                            // 0e8a (ok)
                            TOWN_1726(local_4_ab, local_a, local_c, D_5895_map_level);
                        }

                        // 0e9c (ok)
                        local_8->_0 = 2;
                    }
                }
            }
            else if (local_8->_0 == 6 || local_8->_0 == 7)
            {
                // 0eb2
                if (NPC_0a4a(local_4_ab, local_e_am) != 0)
                {
                    // 0ebf
                    TOWN_1726(local_4_ab, D_5d5e[local_4_ab].x[local_e_am], D_5d5e[local_4_ab].y[local_e_am],
                              D_5d5e[local_4_ab].z[local_e_am]);

                    local_8->_e = local_e_am;
                    D_655e[local_4_ab] = -1;
                    local_8->_0 = 1;
                }
                else
                {
                    // 0efe
                    if (local_8->_0 != 2 && local_14_am != 1)
                    {
                        local_14_am = 1;
                        if (local_8->_0 == 6)
                        {
                            local_6 = 1;
                        }
                        else
                        {
                            local_6 = 2;
                        }
                        // 0f2b
                        local_10_ad_am = NPC_01a0(local_8->_2_x, local_8->_4_y, local_6 == 1, local_4_ab);
                        if (local_10_ad_am != 0)
                        {
                            local_a = D_5876;
                            local_c = D_5878;
                            local_10_ad_am = NPC_032c(local_8->_2_x, local_8->_4_y, local_a, local_c, local_6, local_4_ab);
                        }
                        // 0f71
                        if (local_10_ad_am != 0)
                        {
                            // 0f7a
                            NPC_04ac(local_4_ab, local_6, local_a, local_c);
                            local_8->_0 = 3;
                        }
                    }
                }
            }
        }
        else
        {
            // 0f94
            if (D_655e[local_4_ab] > -1 && D_615e[local_4_ab].data[D_655e[local_4_ab]] != 0)
            {
                // 0fb8
                D_5876 = local_8->_2_x;
                D_5878 = local_8->_4_y;
                NPC_0632(D_615e[local_4_ab].data[D_655e[local_4_ab] + 1]);
                local_a = D_5876;
                local_c = D_5878;
                local_10_ad_am = NPC_0b9e(D_5876, local_c, local_4_ab, local_e_am);
                if (local_10_ad_am != 0)
                {
                    // 0ff9
                    D_5c5a[local_8->_c]._2_x = (u8)local_a;
                    local_8->_2_x = local_a & 0xff;
                    D_5c5a[local_8->_c]._3_y = (u8)local_c;
                    local_8->_4_y = local_c & 0xff;
                    D_24e6 |= 2;

                    D_615e[local_4_ab].data[D_655e[local_4_ab]]--;
                    D_65c2[local_4_ab] = 0;
                    if (D_615e[local_4_ab].data[D_655e[local_4_ab]] == 0)
                    {
                        // 104a
                        D_655e[local_4_ab]++;
                        D_615e[local_4_ab].data[D_655e[local_4_ab]++] = 0;
                        if (D_655e[local_4_ab] >= 0x20 || D_615e[local_4_ab].data[D_655e[local_4_ab]] < 1)
                        {
                            D_655e[local_4_ab] = -1;
                        }

                        if (local_10_ad_am == 2)
                        {
                            // 1083
                            local_8->_e = local_e_am;
                            local_8->_0 = 1;
                            D_655e[local_4_ab] = -1;
                        }
                    }
                }
                else
                {
                    D_65c2[local_4_ab]++;
                    NPC_0c50(&D_5f5e[local_4_ab], 0, local_e_am, local_4_ab, &D_5d5e[local_4_ab]);
                    if (D_65c2[local_4_ab] > 3)
                    {
                        // 10d0
                        D_655e[local_4_ab] = -1;
                        // 10d6
                        D_65c2[local_4_ab] = 0;
                    }
                }
            }
            else
            {
                // 10e0
                if (D_655e[local_4_ab] == -1 && local_8->_0 == 3)
                {
                    local_e_am = NPC_12e0(local_4_ab, param_1);
                    if (D_5895_map_level < D_5d5e[local_4_ab].z[local_e_am])
                    {
                        local_8->_0 = 6;
                        return; // 12d8
                    }
                    local_8->_0 = 7;
                    return; // 12d8
                }

                // 1124
                if (local_14_am < 1)
                {
                    // 112d
                    if (local_8->_0 != 1)
                    {
                        // 1138
                        if (D_65c2[local_4_ab] < 200 && (D_65c2[local_4_ab] == 0 || ULTIMA_2092_RandomRange(0, 2) == 1))
                        {
                            // 1160
                            if (D_655e[local_4_ab] == -1)
                            {
                                local_14_am = local_14_am + 1;
                                if (NPC_032c(local_8->_2_x, local_8->_4_y, D_5d5e[local_4_ab].x[local_e_am],
                                             D_5d5e[local_4_ab].y[local_e_am], 0, local_4_ab) != 0)
                                {
                                    NPC_04ac(local_4_ab, 0, D_5d5e[local_4_ab].x[local_e_am],
                                             D_5d5e[local_4_ab].y[local_e_am]);

                                    D_65c2[local_4_ab] = 0;
                                    continue;
                                }
                                // 11ba
                                D_65c2[local_4_ab] = 200;
                            }

                            // 11c5
                            NPC_0c50(local_8, 0, local_e_am, local_4_ab, &D_5d5e[local_4_ab]);
                        }
                        else
                        {
                            // 11e8
                            if (D_65c2[local_4_ab] >= 200)
                            {
                                D_65c2[local_4_ab]++;
                            }

                            // 11f9
                            if (D_65c2[local_4_ab] > 204)
                            {
                                D_65c2[local_4_ab] = 0;
                            }
                        }
                    }
                }
                // 120e
                else if (local_8->_6_z == (uint)D_5895_map_level)
                {
                    // 1243 -> 1261
                    NPC_0d00(local_4_ab, local_8->_e);
                }
                else if (local_e_am == 1 || local_e_am == 2)
                {
                    // 1227
                    local_10_ad_am = 0;
                    // -> 1243
                    NPC_0d00(local_4_ab, local_8->_e);
                }
                else if (local_e_am == 0)
                {
                    local_10_ad_am = 2;
                    // -> 1243
                    NPC_0d00(local_4_ab, local_8->_e);
                }
                else
                {
                    // 123c
                    local_10_ad_am = local_e_am - 1;
                    // 1243
                    NPC_0d00(local_4_ab, local_8->_e);
                }
                // 1264
            }
        }
    }

    // 12d8
}

#if !defined(TARGET_DOS16) // assembly in the original: src/asm/ANPC.ASM
int NPC_12e0(int param_1, char param_2)
{
    int i;
    int ret;
    byte local_8[4];

    for (i = 0; i < 4; i++)
    {
        local_8[i] = param_2 - D_5d5e[param_1].time[i];
    }

    ret = 0;
    if (local_8[1] < local_8[0])
    {
        local_8[0] = local_8[1];
        ret = 1;
    }

    if (local_8[2] < local_8[0])
    {
        local_8[0] = local_8[2];
        ret = 2;
    }

    if (local_8[3] < local_8[0])
    {
        ret = 1;
    }

    return ret;
}
#endif
