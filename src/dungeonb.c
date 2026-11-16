#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include <string.h>

#include "blckthrn.h"
#include "combat.h"
#include "dnglook.h"
#include "dungeon.h"
#include "endgame.h"
#include "audio/aud_mus.h"

// The part of DUNGEON.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").
// Where this part starts is not certain: DUNGEON_1020 uses no strings and compiles the same
// with both compilers, so the original boundary may be 1020 or 104c.

void DUNGEON_0134(int param_1);
void DUNGEON_01d2_PrintWalkDir(void);
void DUNGEON_1020(void);

u8 D_2de8[8] = {0x00, 0x01, 0x02, 0x02, 0x03, 0x07, 0x0a, 0x0b};

u8 D_2df0[8] = {0x01, 0x01, 0x00, 0x01, 0x04, 0x03, 0x01, 0x00};

u8 D_2df8[12] = {0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x09, 0x09, 0x07, 0x00};

u8 D_2e04[12] = {0x0a, 0x09, 0x0a, 0x0b, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0a, 0x00};

char* D_2e10[11] =
{
    _STATIC_TEXT(0x6bb0, "BOTTOMLESS\n   PIT    "),
    _STATIC_TEXT(0x6bc6, "[E MAZE\n OF LO_\n SOULS "),
    _STATIC_TEXT(0x6bde, "[E PRISON\n   WRO]  "),
    _STATIC_TEXT(0x6bf2, "[E CRYPT"),
    _STATIC_TEXT(0x6bfc, "UPPER \nCRYPTS"),
    _STATIC_TEXT(0x6c0a, "LOWER \nCRYPTS"),
    _STATIC_TEXT(0x6c18, "DEBTERS\n ALLY  "),
    _STATIC_TEXT(0x6c28, "D\\P"),
    _STATIC_TEXT(0x6c2c, "D\\PER"),
    _STATIC_TEXT(0x6c32, "D\\PE_"),
    _STATIC_TEXT(0x6c38, "MO[ER LODE\n   MAZE   "),
};

u8 D_2e26[4] = {0x01, 0x03, 0x02, 0x03};

s16 D_2e2a[4] = {0x0028, 0x0048, 0x0050, 0x0058};

s16 D_2e32[2][4] = {0x0056, 0x0056, 0x0060, 0x0062, 0x0010, 0x0028, 0x0046, 0x0055};

s16 D_2e42[4] = {0x0010, 0x0038, 0x0050, 0x005c};

s16 D_2e4a[4] = {0x00a7, 0x0087, 0x006f, 0x0063};

s16 D_2e52[4] = {0x012c, 0x0064, 0x0032, 0x000f};

s16 D_2e5a[4] = {0x0007, 0x0007, 0x0005, 0x0002};

s16 D_2e62[2][4] = {0x0010, 0x0028, 0x0048, 0x0058, 0x0098, 0x0078, 0x0068, 0x0060};

s16 D_2e72[4] = {0x0038, 0x0048, 0x0050, 0x0058};

s16 D_2e7a[2][4] = {0x0098, 0x0078, 0x0068, 0x0060, 0x000f, 0x0027, 0x0047, 0x0057};

u8 D_2e8a[6] = {0x0c, 0x08, 0x18, 0x08, 0x0c, 0x0c};

// NOTE: Original size 10 causes OOB. FMT uses 2 bytes of padding.
u8 D_2e90[10] = {0x28, 0x2f, 0x42, 0x64, 0x92, 0x2e, 0x32, 0x3e, 0x52, 0x6d};

// NOTE: Original size 10 causes OOB. FMT uses 2 bytes of padding.
u8 D_2e9a[10] = {0x0e, 0x17, 0x32, 0x62, 0x9f, 0x28, 0x2d, 0x3c, 0x54, 0x77};

Pair D_2ea4[3][8] =
{
    0x5a, 0x73, 0x52, 0x77, 0x5f, 0x7b, 0x57, 0x7d,
    0x5b, 0x83, 0x50, 0x85, 0x50, 0x85, 0x50, 0x85,
    0x57, 0x73, 0x5e, 0x77, 0x51, 0x7a, 0x5d, 0x7d,
    0x5f, 0x81, 0x55, 0x82, 0x59, 0x87, 0x50, 0x8b,
    0x54, 0x75, 0x5a, 0x75, 0x5a, 0x7c, 0x50, 0x7e,
    0x55, 0x88, 0x59, 0x8a, 0x59, 0x8a, 0x59, 0x8a
};

Pair D_2ed4[3][5] =
{
    0x16, 0x07, 0x13, 0x0a, 0x0d, 0x0b, 0x17, 0x14,
    0x0d, 0x1b, 0x12, 0x06, 0x11, 0x0f, 0x17, 0x0f,
    0x0c, 0x10, 0x0f, 0x1a, 0x0f, 0x08, 0x16, 0x0b,
    0x10, 0x13, 0x0b, 0x15, 0x12, 0x1c
};

Pair D_2ef2[3][4] =
{
    0x0b, 0x01, 0x0f, 0x04, 0x09, 0x06, 0x0b, 0x08,
    0x0a, 0x02, 0x0e, 0x02, 0x0c, 0x0a, 0x0c, 0x0a,
    0x0d, 0x01, 0x09, 0x04, 0x09, 0x09, 0x09, 0x09
};

Pair D_2f0a[3] =
{
    0x06, 0x00, 0x05, 0x01, 0x07, 0x02
};

Pair D_2f10[3] =
{
    0x59, 0x91, 0x50, 0x90, 0x55, 0x8f
};

u8 D_2f16[8] = {0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00};

u8 D_2f1e[8] = {0x00, 0x00, 0x1f, 0x1f, 0x37, 0x27, 0x2f, 0x3f};

u8 D_2f26 = 0;

s8 D_2f28[4] = {0xff, 0x00, 0x01, 0x00};

s8 D_2f2c[4] = {0x00, 0xff, 0x00, 0x01};

// CHECKED (register)
static void DUNGEON_104c(int param_1)
{
    int local_4;

    if (param_1 <= D_2df0[D_5893_map_id - 0x21])
    {
        // 1063 (ok)
        local_4 = D_2de8[D_5893_map_id - 0x21] + param_1 - 1;
        ULTIMA_1c22_SetTextWindowSize(0, D_2df8[local_4], D_2e04[local_4], 0x27, 0x18);
        ULTIMA_1b94_SelectTextWindow(0);
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_1bf2_SetTextPosition(0, 0);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
        ULTIMA_1850_PrintString(D_2e10[local_4]);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
        ULTIMA_1c9e_SelectCharset(0);
        ULTIMA_1b94_SelectTextWindow(2);
        ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);
    }
}

static int DUNGEON_10dc(int param_1, int param_2)
{
    int local_4;

    local_4 = GetDungeonMap(param_1 & 7, param_2 & 7, D_5895_map_level);
    if (local_4 < 0x90)
    {
        local_4 &= 0xf7;
    }

    return local_4;
}

// CHECKED (register)
static void DUNGEON_111e(int param_1)
{
    int local_e;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;
    register int uVar2;

    if (D_587a == 'T')
    {
        D_5c5a[1]._6 = D_1744[D_5c5a[1]._0_tile];
    }

    // 113a
    local_c = D_5c5a[1]._7 == 0xff;
    // 1148
    local_a = (D_5c5a[1]._6 & 0x90);
    local_4_ab = (D_5c5a[1]._6 & 0x60);
    local_8 = D_5c5a[1]._6 & 0xf;

    if (D_587a == 'T')
    {
        
        local_6 = local_e = 1;
    }
    else
    {
        local_6 = ULTIMA_2092_RandomRange(0, 100) < 0x32;
        local_e = ULTIMA_2092_RandomRange(0, 100) < 0x32;
        switch (local_4_ab)
        {
        case 0x20:
            // 11c2
            local_e = local_6 = local_8 = local_8 == 0;
            break;
        case 0x40:
            break;
        case 0x60:
            // 11e6
            local_8--;
            if (local_8 < 0)
            {
                local_8 = 3;
            }
            local_6 = D_2e26[local_8] >> 1;
            local_e = D_2e26[local_8] & 1;
            break;
        }
    }

    // 11d3
    if (local_a != 0)
    {
        local_e = (local_a == 0x90) ^ local_6;
    }

    if (D_a9c6 == 0)
    {
        // debug message
        ULTIMA_1850_PrintString(_TEXT(0x6c4e, "Hey!! What's going on here???"));
    }
    else
    {
        // 1220
        uVar2 = D_2e32[local_c][param_1];
        ULTIMA_0d4c_GRAP_4b_PutImage(D_a9c6, local_6 * 3 + param_1 - 1, D_2e2a[param_1], uVar2, local_c);
        ULTIMA_0bfc_GRAP_63(D_a9c6, local_e * 3 + param_1 - 1, 0x60, uVar2, local_c);
        D_5c5a[1]._6 = local_4_ab + local_a + local_8;
    }
}

// CHECKED (optimization)
static void DUNGEON_127e(int param_1, int param_2)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_4_ab = D_52ba_vdp._52c4;
    D_52ba_vdp._52c4 = 0;

    switch (param_2)
    {
    case 0:
        // 12b4
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b6_magentaColor + 8);
        break;
    case 1:
        // 1326
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b4_greenColor + 8);
        break;
    case 2:
        // 132c
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor + 8);
        break;
    case 3:
        // 1332
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
        break;
    }

    // 12be NOT MATCHING
    for (local_8 = 0; local_8 < D_2e52[param_1]; local_8++)
    {
        local_6 = ULTIMA_2092_RandomRange(D_2e42[param_1], D_2e4a[param_1] - D_2e5a[param_1]);
        ULTIMA_0c9c_GRAP_39_HorizLine(local_6, ULTIMA_2092_RandomRange(D_2e42[param_1], D_2e4a[param_1]), D_2e5a[param_1] + local_6);
    }

    D_52ba_vdp._52c4 = local_4_ab;
}

static void DUNGEON_134a(int param_1, int param_2, int param_3)
{
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    if (param_1 < 0x1f)
    {
        local_6 = (param_1 & 3);
        local_8 = D_2e62[param_3][local_6];
        if (param_3 != 0 && (((byte)param_1 & 0xf8) == 8 || ((byte)param_1 & 0xfc) == 0x18))
        {
            local_8 = 0x60;
        }

        local_a = 0xe;
        if (param_3 != 0)
        {
            ULTIMA_0bfc_GRAP_63(D_a9c0, param_1, local_8, local_a, 0);
        }
        else
        {
            ULTIMA_0d4c_GRAP_4b_PutImage(D_a9c0, param_1, local_8, local_a, 0);
        }
    }
    else
    {
        local_4_ab = ((param_1 + 1) >> 1) - 0x10;
        local_6 = ((param_1 + 1) & 6) >> 1;
        if (param_3 != 0)
        {
            local_8 = 0x60;
        }
        else
        {
            local_8 = D_2e72[local_6];
        }

        local_a = (local_4_ab < 8) ? (param_2 ? D_2e7a[1][local_6] : 0x60) : D_2e7a[param_2][local_6];

        if (param_3 != 0)
        {
            ULTIMA_0bfc_GRAP_63(D_a9c4, local_4_ab, local_8, local_a, param_2);
        }
        else
        {
            ULTIMA_0d4c_GRAP_4b_PutImage(D_a9c4, local_4_ab, local_8, local_a, param_2);
        }
    }
}

static int DUNGEON_145c(int param_1, int param_2, int param_3, int param_4)
{
    if (param_3 == 5)
    {
        param_3 = 0;
        ULTIMA_43ae_AudioSweepTone(0xc80, 0xdac, 1, -(param_4 * 8 - 0x14));
    }
    else
    {
        if (param_3 == 4)
        {
            if (D_52c8 == 0 || D_52c8 == 3)
            {
                ULTIMA_0a70_GRAP_2d_SetPenColor(3);
            }
            else
            {
                ULTIMA_0a70_GRAP_2d_SetPenColor(0xb);
            }
        }
        else
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
        }

        ULTIMA_0c9c_GRAP_39_HorizLine(param_1 - 1, param_2, param_1 + 1);
        ULTIMA_0cf2_GRAP_3c_VertLine(param_1, param_2 - 1, param_2 + 1);

        if (param_3 < 4)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
            ULTIMA_0c64_GRAP_30_Pset(param_1, param_2);
        }

        if (param_3 != 0 || (int)ULTIMA_2092_RandomRange(0, 0x40) < 4)
        {
            param_3++;
        }
    }

    return param_3;
}

// CHECKED (nop)
static int DUNGEON_150a(int param_1, int param_2, int param_3)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_6 = DUNGEON_10dc(param_1, param_2);
    if (local_6 < 0xa0)
    {
        return 1;
    }

    local_4_ab = D_2e8a[(local_6 >> 4) - 0xa] + param_3;
    if (param_3 == 0)
    {
        local_4_ab = 0xc;
    }

    DUNGEON_134a(local_4_ab, 0, 0);
    DUNGEON_134a(local_4_ab, 0, 1);

    if ((param_3 == 1 || param_3 == 2) && D_6604 == 1 && (local_6 & 0xf0) == 0xc0)
    {
        local_8 = local_6 & 7;

#if !defined(TARGET_DOS16)
        if (local_8 >= 5)
        {
            debug("WARNING: local_8 >= 5");
        }
#endif

        local_8 = DUNGEON_145c(0x5f, D_2e90[local_8 + (param_3 - 1) * 5] + 0xe, local_8, param_3);

        GetDungeonMap(param_1 & 7, param_2 & 7, D_5895_map_level) &= 0xf8;
        GetDungeonMap(param_1 & 7, param_2 & 7, D_5895_map_level) += local_8;
    }

    if (param_3 == 1)
    {
        if ((local_6 & 0xf0) == 0xb0 && (local_6 & 0xf) != 0)
        {
            DUNGEON_104c(local_6 & 0xf);
            // NOT MATCHING: nop
        }
        else if (D_6604 == 3 && (local_6 & 0xf0) == 0xc0 && (int)ULTIMA_2092_RandomRange(0, 0x40) < 4)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor + 8);
            ULTIMA_0c9c_GRAP_39_HorizLine(0x5c, 0x57, 0x5d);
            ULTIMA_0c9c_GRAP_39_HorizLine(0x5b, 0x58, 0x5d);
            ULTIMA_0c9c_GRAP_39_HorizLine(0x61, 0x57, 0x62);
            ULTIMA_0c9c_GRAP_39_HorizLine(0x61, 0x58, 99);
        }
    }

    if ((local_6 & 0xf0) == 0xe0 && param_3 == 0)
    {
        D_52ba_vdp._52c4 = 1;

        return 1;
    }

    return 0;
}

// CHECKED (nop)
static void DUNGEON_1682(int param_1, int param_2, int param_3, int param_4)
{
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_8 = DUNGEON_10dc(param_1, param_2);
    if (local_8 >= 0xa0)
    {
        switch (local_8 & 0xf0)
        {
        case 0xc0:
            // 16c4
            local_4_ab = param_4 + 0x14;
            DUNGEON_134a(local_4_ab, 0, param_3);
            if (param_4 >= 2)
            {
                break;
            }

            if (D_6604 != 1)
            {
                break;
            }

            // 16ea
            local_a = local_8 & 7;
            local_6 = (param_4 != 0) ? 0x43 : 0x21;

            if (param_3 != 0)
            {
                local_6 = 0xbe - local_6;
            }

#if !defined(TARGET_DOS16)
            if (local_a >= 5)
            {
                debug("WARNING: local_a >= 5");
            }
#endif

            local_a = DUNGEON_145c(local_6, D_2e9a[local_a + param_4 * 5] + 0xe, local_a, param_4);

            GetDungeonMap(param_1 & 7, param_2 & 7, D_5895_map_level) &= 0xf8;
            GetDungeonMap(param_1 & 7, param_2 & 7, D_5895_map_level) += local_a;
            // NOT MATCHING (nop)
            break;

        case 0xa0:
        case 0xe0:
        case 0xf0:
            // 1766
            DUNGEON_134a(param_4 + 4, 0, param_3);
            break;

        default:
            DUNGEON_134a(param_4, 0, param_3);
            break;
        }
    }
    else
    {
        DUNGEON_134a(param_4 + 0x10, 0, param_3);
    }
}

// CHECKED
static void DUNGEON_1786(int param_1, int param_2)
{
    int local_4;

    if (D_52c8 == 0 || D_52c8 == 3)
    {
        ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    }
    else
    {
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    }

    // 17a9
    switch (param_1)
    {
    case 0:
        // 17cc
        if (D_52c8 != 0 && D_52c8 != 3)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
        }

        // 17e4
        for (local_4 = 0; local_4 < 8; local_4++)
        {
            ULTIMA_0c64_GRAP_30_Pset(D_2ea4[param_2][local_4].x, D_2ea4[param_2][local_4].y);
            ULTIMA_0c64_GRAP_30_Pset(0xbe - D_2ea4[param_2][local_4].x, D_2ea4[param_2][local_4].y);
        }

        ULTIMA_0c64_GRAP_30_Pset(D_2f10[param_2].x, D_2f10[param_2].y);
        ULTIMA_0c64_GRAP_30_Pset(0xbe - D_2f10[param_2].x, D_2f10[param_2].y);
        break;

    case 1:
        // 185e
        for (local_4 = 0; local_4 < 5; local_4++)
        {
            ULTIMA_0c64_GRAP_30_Pset(D_2ed4[param_2][local_4].x + 0x48, D_2ed4[param_2][local_4].y + 0x60);
            ULTIMA_0c64_GRAP_30_Pset(0x76 - D_2ed4[param_2][local_4].x, D_2ed4[param_2][local_4].y + 0x60);
        }
        break;

    case 2:
        // 18c4
        for (local_4 = 0; local_4 < 4; local_4++)
        {
            ULTIMA_0c64_GRAP_30_Pset(D_2ef2[param_2][local_4].x + 0x50, D_2ef2[param_2][local_4].y + 0x60);
            ULTIMA_0c64_GRAP_30_Pset(0x6e - D_2ef2[param_2][local_4].x, D_2ef2[param_2][local_4].y + 0x60);
        }
        break;

    case 3:
        // 1920
        ULTIMA_0c64_GRAP_30_Pset(D_2f0a[param_2].x + 0x58, D_2f0a[param_2].y + 0x60);
        ULTIMA_0c64_GRAP_30_Pset(0x66 - D_2f0a[param_2].x, D_2f0a[param_2].y + 0x60);
        break;
    }
}

// CHECKED
static void DUNGEON_1952(int param_1, int param_2, int param_3)
{
    int local_6_an;
    int local_4;

    local_6_an = DUNGEON_10dc(param_1, param_2);
    local_4 = local_6_an >> 4;
    if (local_4 > 0 && local_4 < 8 && (local_4 != 6 || (local_6_an & 7) == 0))
    {
        if (D_2f16[local_4] != 0)
        {
            DUNGEON_134a(D_2f16[local_4] + param_3 * 2, 1, 0);
            DUNGEON_134a(D_2f16[local_4] + param_3 * 2, 1, 1);
        }

        if (D_2f1e[local_4] != 0)
        {
            DUNGEON_134a(D_2f1e[local_4] + param_3 * 2, 0, 0);
            DUNGEON_134a(D_2f1e[local_4] + param_3 * 2, 0, 1);
        }

        if (local_4 == 5)
        {
            DUNGEON_1786(param_3, D_2f26);
        }
    }
    else if (local_4 == 8)
    {
        DUNGEON_127e(param_3, local_6_an & 7);
    }

    // 1a03
    if (local_4 < 9 && (GetDungeonMap(param_1 & 7, param_2 & 7, D_5895_map_level) & 8) != 0)
    {
        // 1a2d
        DUNGEON_134a(D_2f24 + param_3 * 2, 1, 0);
        DUNGEON_134a(D_2f24 + param_3 * 2, 1, 1);
    }

    // 1a4f (OK)
    if (param_3 != 0 && (param_1 & 7) == D_5c5a[1]._2_x && (param_2 & 7) == D_5c5a[1]._3_y)
    {
        DUNGEON_111e(param_3);
    }

    if (param_3 == 0 && ++D_2f26 > 2)
    {
        D_2f26 = 0;
    }
}

void DUNGEON_1a90(int param_1)
{
    int local_a;
    int local_c;
    int local_4_ab;
    int local_6_x;
    int local_8_y;

    D_545e = 0xff;
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x60, 0xe, 0xaf, 0xb2);
    ULTIMA_0c3c(0x28, 0x2a, 0x98, 0xb7);
    D_52ba_vdp._52c4 = 0;

    if (D_58a6 != 0 || D_58a7 != 0)
    {
        local_6_x = D_5896_map_x;
        local_8_y = D_5897_map_y;
        local_a = D_24d6[D_6603];
        local_c = D_24de[D_6603];

        for (local_4_ab = 0; local_4_ab < 4; local_4_ab++)
        {
            // 1b0e
            if (DUNGEON_150a(local_6_x, local_8_y, local_4_ab) != 0)
            {
                if (local_4_ab != 0 || (DUNGEON_10dc(D_5896_map_x, D_5897_map_y) != 0xe0 && param_1 != 0))
                {
                    // 1b36
                    DUNGEON_1682((u8)D_2f28[D_6603] + local_6_x, (u8)D_2f2c[D_6603] + local_8_y, 0, local_4_ab);
                    DUNGEON_1682(local_6_x - (u8)D_2f28[D_6603], local_8_y - (u8)D_2f2c[D_6603], 1, local_4_ab);
                }

                local_6_x += local_a;
                local_8_y += local_c;
            }
            else
            {
                break;
            }
        }

        // 1b92
        for (--local_4_ab; local_4_ab >= 0; local_4_ab--)
        {
            local_6_x -= local_a;
            local_8_y -= local_c;
            DUNGEON_1952(local_6_x, local_8_y, local_4_ab);
        }
    }
    else
    {
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    }

    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    D_52ba_vdp._52c4 = 0;
}

void DUNGEON_1be0(void)
{
    ULTIMA_2900_UpdateVitalsDisplay();
    DUNGEON_1a90(1);
    ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x10, 0xe, 0xaf, 0xb2);
    DUNGEON_1020();
    DUNGEON_01d2_PrintWalkDir();
}

static int DUNGEON_1c0c(int param_1, int param_2)
{
    int local_6_an;
    int local_4;

    local_4 = 1;
    local_6_an = GetDungeonMap(D_5896_map_x, D_5897_map_y, param_1) & 0xf0;
    if (param_2 != 0 &&
        (local_6_an != 0 || local_6_an == 0xe0 || local_6_an == 0xd0 || local_6_an == 0xc0 || local_6_an == 0xb0)) // NOTE: logic bug?
    {
        local_4 = 0;
    }

    return local_4;
}

int DUNGEON_1c6a_Klimb(int param_1, int param_2)
{
    int local_4;

    local_4 = 0;

    if (param_1 > 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x6c6c, "Down!\n"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x6c74, "Up!\n"));
    }

    if (param_1 > 0 && D_5895_map_level == 7 || param_1 < 0 && D_5895_map_level == 0)
    {
        local_4 = 1;
    }
    else
    {
        if (DUNGEON_1c0c(param_1 + (uint)D_5895_map_level, param_2) != 0)
        {
            D_5895_map_level += param_1;
            if (param_1 > 0)
            {
                D_6602 = 5;
            }
            else
            {
                D_6602 = 4;
            }

            DUNGEON_0134(1);
            DUNGEON_1be0();
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x6c7a, "Failed!\n"));
            ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
        }
    }

    return local_4;
}

void DUNGEON_1d08_Exit(void)
{
    D_5896_map_x = D_1e8a[D_5893_map_id - 1];
    D_5897_map_y = D_1eb2[D_5893_map_id - 1];

    ULTIMA_1850_PrintString(_TEXT(0x6c84, "\nExit to "));
    if (D_5895_map_level != 0)
    {
        D_5895_map_level = 0xff;
        ULTIMA_1850_PrintString(_TEXT(0x6c8e, "Underworld!\n\n"));
    }
    else
    {
        D_5895_map_level = 0;
        ULTIMA_1850_PrintString(_TEXT(0x6c9c, "Britannia!\n\n"));
    }

    D_5893_map_id = 0;
}

int DUNGEON_1d4a_AttackCmd(void)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_4_ab = 0;

    ULTIMA_1850_PrintString(_TEXT(0x6caa, "Attack\n"));

    local_6 = (D_24d6[D_6603] + D_5896_map_x) & 7;
    local_8 = (D_24de[D_6603] + D_5897_map_y) & 7;
    if (local_6 == D_5c5a[1]._2_x && local_8 == D_5c5a[1]._3_y)
    {
        D_58a1 = 2;
        DNGLOOK_1130();
        DNGLOOK_0c6c();
        DNGLOOK_0d3e();
        ULTIMA_5f86_SpecialMapHandler(2, D_5c5a[1]._5, 0);

        if (D_58a0 == 5)
        {
            if (D_5895_map_level != 0)
            {
                D_5895_map_level--;
            }
            else
            {
                DUNGEON_1d08_Exit();
            }
        }
        else if (D_58a0 == 6)
        {
            if (D_5895_map_level < 7)
            {
                D_5895_map_level++;
            }
            else
            {
                DUNGEON_1d08_Exit();
            }
        }

        DNGLOOK_109e(1);

        if (D_5893_map_id != 0)
        {
            DUNGEON_0134(1);
            DUNGEON_1be0();
        }
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x6cb2, "What?\n"));
    }

    return local_4_ab;
}

int DUNGEON_1e10_KlimbCmd(void)
{
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_4_ab = 0;
    local_6 = 0;
    local_a = DUNGEON_10dc(D_5896_map_x, D_5897_map_y) & 0xf0;
    local_8 = GetDungeonMap(D_5896_map_x, D_5897_map_y, D_5895_map_level) & 8;
    if (local_a == 0x10 || local_a == 0x30 || (local_8 != 0 && D_57af != 0))
    {
        local_4_ab = 1;
    }

    if (local_a == 0x20 || local_a == 0x30 || local_a == 0x60)
    {
        local_6 = 1;
    }

    if (local_4_ab && local_6)
    {
        ULTIMA_1850_PrintString(_TEXT(0x6cba, "Klimb-U/D-"));
        while (local_4_ab && local_6)
        {
            switch (ULTIMA_266c_GetChar())
            {
            case U5_KEY_UP:
            case 0x55:
                local_6 = 0;
                break;

            case U5_KEY_DOWN:
            case 0x44:
                local_4_ab = 0;
                break;

            case U5_KEY_SPACE:
                ULTIMA_1850_PrintString(_TEXT(0x6cc6, "Pass\n\n"));
                local_4_ab = local_6 = 0;
                break;
            }
        }
    }
    else if (local_4_ab || local_6) // 1eee
    {
        // 1efa
        ULTIMA_1850_PrintString(_TEXT(0x6cce, "Klimb-"));
    }
    else if (local_8 != 0)
    {
        // 1f08
        ULTIMA_1850_PrintString(_TEXT(0x6cd6, "Klimb-\nWith What?\n"));

        return 0;
    }
    else
    {
        // 1f16
        ULTIMA_1850_PrintString(_TEXT(0x6cea, "Klimb-what?\n"));

        return 0;
    }

    // 1f1c
    if ((local_4_ab && DUNGEON_1c6a_Klimb(-1, 0) != 0) || (local_6 && DUNGEON_1c6a_Klimb(1, 0) != 0))
    {
        DUNGEON_1d08_Exit();
    }

    return 1;
}
