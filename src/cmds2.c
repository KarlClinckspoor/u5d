#include "common/common.h"
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

void MAINOUT_0354(int param_1, int param_2);
int MAINOUT_105c(int param_1);

// CMDS.OVL's MSC 5.1 code is two objects: cmds.c (CMDS_0000-CMDS_1030) and this one, from
// CMDS_1202 (its _DATA starts at DS:4450, an even address, after D_444a). Compiled after
// the first part's functions, MSC 5.1 drops loop optimizations in CMDS_12c8 (its loop
// registers); as an object of its own it gives the original's code.

// defined in cmds.c
int CMDS_1030(char* param_1);

// meditate
static void CMDS_1202(int param_1, int param_2, int param_3)
{
    bool local_16_an;
    int local_14;
    char local_12[16];

    local_16_an = 1;

    ULTIMA_1850_PrintString(_TEXT(0x4450, "\nUpon what virtue\ndost thou\nmeditate?\n\n:"));
    ULTIMA_3b1c_GetString(local_12, 0xf);

    if (ULTIMA_6f1e((byte*)D_1f4e[param_1], local_12) == -1)
    {
        local_16_an = 0;
    }

    for (local_14 = 0; local_14 < 3; local_14++)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4479, "\nMantra:"));
        ULTIMA_3b1c_GetString(local_12, 0xf);

        if (ULTIMA_6f1e((byte*)D_1f5e[param_1], local_12) == -1)
        {
            local_16_an = 0;
        }
    }

    if (local_16_an && D_1f6e[param_1] == param_2 && D_1f76[param_1] == param_3)
    {
        D_58d8[param_1] &= 0x7f;
        ULTIMA_1850_PrintString(_TEXT(0x4482, "\n\nThe Shrine is\nrestored!\n"));
        ULTIMA_3072_ShakeScreen();
        *ULTIMA_4402_GetTileAddr(param_2, param_3) = TILE_MAP_SHRINE;
        D_24e6 |= 2;
    }
    else
    {
        ULTIMA_16ba_PrintChar('\n');
    }
}

char* D_4502[8] =
{
    _STATIC_TEXT(0x449d, "FALLAX"),
    _STATIC_TEXT(0x44a4, "VILIS"),
    _STATIC_TEXT(0x44aa, "INOPIA"),
    _STATIC_TEXT(0x44b1, "MALUM"),
    _STATIC_TEXT(0x44b7, "AVIDUS"),
    _STATIC_TEXT(0x44be, "INFAMA"),
    _STATIC_TEXT(0x44c5, "IGNAVUS"),
    _STATIC_TEXT(0x44cd, "VERAMOCOR")
};

u8 D_4512[8] = {0x18, 0x16, 0x16, 0x18, 0x18, 0x17, 0x17, 0x16};

// CHECKED
// local_4_ab and local_a are stack locals that the loop optimizer keeps in SI and DI.
static void CMDS_12c8(char* param_1)
{
    char** local_e;
    byte local_c;
    int local_a; // <- di
    bool local_8;
    int local_6;
    int local_4_ab; // <- si

    local_8 = 0;

    for (local_4_ab = 0, local_e = D_4502; local_4_ab < 8; local_4_ab++)
    {
        if (ULTIMA_6f1e((byte*)*local_e, param_1) > -1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x44d7, "\nA word of power is uttered\n"));
            ULTIMA_3072_ShakeScreen();

            local_c = GetMapViewport(4, 5);
            if (D_4512[local_4_ab] == local_c || local_c == TILE_MAP_DF || local_c == TILE_MAP_RUINS)
            {
                local_6 = -1;
                local_a = 0;
            }
            else
            {
                local_c = GetMapViewport(5, 6);
                if (D_4512[local_4_ab] == local_c || local_c == TILE_MAP_DF || local_c == TILE_MAP_RUINS)
                {
                    local_6 = 0;
                    local_a = 1;
                }
                else
                {
                    local_c = GetMapViewport(6, 5);
                    if (D_4512[local_4_ab] == local_c || local_c == TILE_MAP_DF || local_c == TILE_MAP_RUINS)
                    {
                        local_6 = 1;
                        local_a = 0;
                    }
                    else
                    {
                        local_c = GetMapViewport(5, 4);
                        if (D_4512[local_4_ab] == local_c || local_c == TILE_MAP_DF || local_c == TILE_MAP_RUINS)
                        {
                            local_6 = 0;
                            local_a = -1;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            if (local_c == TILE_MAP_RUINS)
            {
                CMDS_1202(local_4_ab, local_6 + D_5896_map_x, local_a + D_5897_map_y);
                return;
            }

            if (local_6 + D_5896_map_x == D_1e8a[local_4_ab + 0x20] &&
                local_a + D_5897_map_y == D_1eb2[local_4_ab + 0x20])
            {
                D_58d0[local_4_ab] ^= 0x80;
                *ULTIMA_4402_GetTileAddr(local_6 + D_5896_map_x, local_a + D_5897_map_y) ^= D_4512[local_4_ab] ^ TILE_MAP_DF;
                D_24e6 |= 2;
            }

            local_8 = 1;
            break;
        }
        local_e++;
    }

    if (!local_8)
    {
        ULTIMA_1850_PrintString(_TEXT(0x44f4, "\nNo effect!\n"));
    }
}

int CMDS_1418_YellCmd(void)
{
    int local_24_an;
    char local_22[32];

    local_24_an = 1;

    if ((D_587c_partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20 && D_5893_map_id < 0x80)
    {
        if ((D_587c_partyTile & 0xfc) == TILE_ACTOR_FRIGATE_20)
        {
            ULTIMA_1850_PrintString(_TEXT(0x451a, "FURL!\n"));
            D_587c_partyTile += 4;
#if !defined(MATCHING_BUILD)
            return local_24_an; // TODO: validate return value
#else
            return; // sic
#endif
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x4521, "HOIST!\n"));
            D_587c_partyTile -= 4;
#if !defined(MATCHING_BUILD)
            return local_24_an; // TODO: validate return value
#else
            return; // sic
#endif
        }
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x4529, "what?\n:"));
        ULTIMA_3b1c_GetString(local_22, 0x1e);
        if (local_22[0] == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x4531, "Nothing\n"));
#if !defined(MATCHING_BUILD)
            return local_24_an; // TODO: validate return value
#else
            return; // sic
#endif
        }
        else
        {
            ULTIMA_16ba_PrintChar('\n');
            if (D_5893_map_id >= 1 && D_5893_map_id <= 0x20)
            {
                local_24_an = CMDS_1030(local_22);
            }
            else if (D_5893_map_id == 0)
            {
                CMDS_12c8(local_22);
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x453a, "\nNo effect!\n"));
            }
        }
    }

    return local_24_an;
}

static int CMDS_14ba_Pushable(byte param_1)
{
    switch (param_1)
    {
    case TILE_MAP_5B:
    case TILE_MAP_CHAIR_90:
    case TILE_MAP_CHAIR_91:
    case TILE_MAP_CHAIR_92:
    case TILE_MAP_CHAIR_93:
    case TILE_MAP_DESK:
    case TILE_MAP_BARREL:
    case TILE_MAP_VANITY:
    case TILE_MAP_A9:
    case TILE_MAP_DRESSER:
    case TILE_MAP_AE:
    case TILE_MAP_TRUNK:
    case TILE_MAP_CANNON_B4:
    case TILE_MAP_CANNON_B5:
    case TILE_MAP_CANNON_B6:
    case TILE_MAP_CANNON_B7:
        return 1;
    }

    return 0;
}

static int CMDS_1504(int param_1, int param_2, int param_3, int param_4)
{
    if (param_1 == 1 && param_2 == 0)
    {
        param_3++;
    }
    if (param_1 == 0 && param_2 == 1)
    {
        param_3 += 2;
    }
    if (param_1 == -1 && param_2 == 0)
    {
        param_3 += 3;
    }
    if (param_4 != 0)
    {
        param_3 ^= 2;
    }

    return param_3;
}

static void CMDS_1548_Pushed(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6, int param_7,
                 int param_8)
{
    ULTIMA_1850_PrintString(_TEXT(0x4547, "Pushed!\n"));
    *ULTIMA_4402_GetTileAddr(param_5, param_6) = param_1;
    *ULTIMA_4402_GetTileAddr(param_3, param_4) = param_2;
    param_1 &= 0xfc;
    if (param_1 == TILE_MAP_CHAIR_90 || param_1 == TILE_MAP_CANNON_B4)
    {
        *ULTIMA_4402_GetTileAddr(param_5, param_6) = CMDS_1504(param_7, param_8, param_1, 0);
    }
}

static void CMDS_15b0_Pulled(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6, int param_7,
                 int param_8)
{
    ULTIMA_1850_PrintString(_TEXT(0x4550, "Pulled!\n"));
    *ULTIMA_4402_GetTileAddr(param_3, param_4) = param_2;
    *ULTIMA_4402_GetTileAddr(param_5, param_6) = param_1;
    param_2 &= 0xfc;
    if (param_2 == TILE_MAP_CHAIR_90 || param_2 == TILE_MAP_CANNON_B4)
    {
        *ULTIMA_4402_GetTileAddr(param_3, param_4) = CMDS_1504(param_7, param_8, param_2, 1);
    }
}

void CMDS_161a_PushCmd(void)
{
    int local_1c_aa;
    int local_1a_ac;
    int local_18_al;
    int local_16_an;
    int local_14;
    int local_12_ab;
    int local_10_ad;
    int local_e;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    ULTIMA_39cc_SetTile(D_594f, D_5950, D_5951);
    if (ULTIMA_35ec_SelectDirection() == 0)
    {
        return;
    }

    local_12_ab = D_5876;
    local_18_al = D_5878;

    if (D_5893_map_id > 0x7f)
    {
        local_c = D_5896_map_x;
        local_e = D_5897_map_y;
        D_5896_map_x = D_ba14[D_589e].x;
        D_5897_map_y = D_ba14[D_589e].y;
    }

    local_10_ad = local_12_ab + (uint)D_5896_map_x;
    local_16_an = local_18_al + (uint)D_5897_map_y;

    if (D_5893_map_id > 0x7f && COMBAT_111a(local_10_ad, local_16_an) != 0)
    {
        return;
    }

    local_4_ab = *ULTIMA_4402_GetTileAddr(local_10_ad, local_16_an);
    if (ULTIMA_368e_FindActorTileAtPos(local_10_ad, local_16_an, D_5895_map_level) != 0 || CMDS_14ba_Pushable(local_4_ab) == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4559, "Won't budge!\n"));
    }
    else
    {
        local_a = (local_4_ab & 0xfc) == TILE_MAP_CANNON_B4 ? TILE_MAP_45 : TILE_MAP_44;
        local_14 = local_10_ad + local_12_ab;
        local_1a_ac = local_16_an + local_18_al;
        local_6 = *ULTIMA_4402_GetTileAddr(local_14, local_1a_ac);
        local_1c_aa = *ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y);
        if (ULTIMA_368e_FindActorTileAtPos(local_14, local_1a_ac, D_5895_map_level) == 0 && local_6 == local_a)
        {
            CMDS_1548_Pushed(local_4_ab, local_6, local_10_ad, local_16_an, local_14, local_1a_ac, local_12_ab, local_18_al);
        }
        else
        {
            if (local_1c_aa == local_a)
            {
                CMDS_15b0_Pulled(local_1c_aa, local_4_ab, D_5896_map_x, D_5897_map_y, local_10_ad, local_16_an, local_12_ab, local_18_al);
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x4567, "Won't budge\n"));
                return;
            }
        }

        D_5896_map_x += local_12_ab;
        D_5897_map_y += local_18_al;
        D_24e6 = 1;

        if (D_5893_map_id > 0x7f)
        {
            local_8 = D_ba14[D_589e].entityIdx;

            D_ba14[D_589e].x += local_12_ab;
            D_ba14[D_589e].y += local_18_al;

            D_5c5a[local_8].x += local_12_ab;
            D_5c5a[local_8].y += local_18_al;

            D_5896_map_x = local_c;
            D_5897_map_y = local_e;

            ULTIMA_5910_UpdateFrame();
        }
    }
}

// escape
int CMDS_17ec_Escape(void)
{
    int local_8;
    int local_6;
    bool local_4_ab;

    ULTIMA_1850_PrintString(_TEXT(0x4574, "Escape"));

    local_4_ab = local_6 = 0;
    for (local_8 = 0; local_8 < 0x20; local_8++)
    {
        if ((D_ba14[local_8].flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_DEAD)) == COMBAT_FLAGS_PLAYER)
        {
            local_4_ab = 1;
            break;
        }
    }

    if (local_4_ab)
    {
        if ((D_58a1 & 0x80) != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x457b, "-Not here!\n"));
            local_6 = 1;
        }
        else if (D_58a3 == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x4587, "-Not yet!\n"));
            local_6 = 1;
        }
    }

    if (local_6 == 0)
    {
        ULTIMA_16ba_PrintChar('!');

        for (local_8 = 0; local_8 < 0x20; local_8++)
        {
            if (D_ba14[local_8].flags != 0)
            {
                COMBAT_1236(-local_8 - 1);
                ULTIMA_5910_UpdateFrame();
            }
        }

        for (local_8 = 0; local_8 < 0x20; local_8++)
        {
            if (D_5c5a[local_8].tile != 0)
            {
                COMBAT_1236(local_8 + 1);
                ULTIMA_5910_UpdateFrame();
            }
        }

        ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
    }

    D_a9fa = 1;
    return local_6;
}

// CMDS_MIX.C?
