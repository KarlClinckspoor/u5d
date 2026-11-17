#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"

#include <string.h>

#include "dnglook.h"
#include "dungeon.h"
#include "sjog.h"
#include "audio/audio.h"

u8 D_383a[6] = {0x50, 0x5b, 0x41, 0x46, 0x4b, 0x4c};

u8 D_3840 = 6;

u8 D_3842[0xd] =
{
    0x08, 0x08, 0x04, 0x03, 0x08, 0x08, 0x04, 0x03,
    0x06, 0x03, 0x08, 0x01, 0x08
};

u8 D_3850[0xd] =
{
    0x00, 0x00, 0x1e, 0x04, 0x01, 0x01, 0x00, 0x2a,
    0x09, 0x2d, 0x01, 0x01, 0x01
};

u8 D_385e[8] = {0x14, 0x15, 0x16, 0x22, 0x21, 0x18, 0x1f, 0x18};

// CHECKED (code structure)
void DNGLOOK_0000_LookCmdInDungeon(void)
{
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    byte local_6;
    int local_4_ab;

    if ((local_e_am = ULTIMA_4988()) + 1 == 0)
    {
        return;
    }

    if (D_58a7 == 0 && D_58a6 == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x752e, "You see:\ndarkness.\n"));
        return;
    }

    if (SJOG_006c(D_6603) == 0)
    {
        return;
    }

    local_8 = D_5876;
    local_a = D_5878;

    local_c = GetDungeonMap(local_8 & 7, local_a & 7, D_5895_map_level);

    ULTIMA_1850_PrintString(_TEXT(0x7542, "You see:\n"));

    if (local_c == 0x61)
    {
        local_c = 0;
    }

    // a30b
    if ((local_c & 0xf0) == 0x80)
    {
        switch (local_c)
        {
        case 0x80:
            ULTIMA_1850_PrintString(_TEXT(0x754c, "A sleep field.\n"));
            break;
        case 0x81:
            ULTIMA_1850_PrintString(_TEXT(0x755c, "A poison gas field.\n"));
            break;
        case 0x82:
            ULTIMA_1850_PrintString(_TEXT(0x7572, "A wall of fire.\n"));
            break;
        case 0x83:
            ULTIMA_1850_PrintString(_TEXT(0x7584, "An electric field.\n"));
            break;
        default:
            ULTIMA_1850_PrintString(_TEXT(0x7598, "An energy field.\n"));
            // nop
            break;
        }

        return;
    }

    if ((local_c & 0xf0) == 0xc0)
    {
        local_4_ab = D_6604 & 0xf;
        if (local_4_ab == 1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x75aa, "a dripping stalactite.\n"));
        }
        else if (local_4_ab == 2)
        {
            ULTIMA_1850_PrintString(_TEXT(0x75c2, "a caved in passage.\n"));
        }
        else
        {
            // a376
            if (ULTIMA_2092_RandomRange(1, 0xff) == 0xff)
            {
                ULTIMA_1850_PrintString(_TEXT(0x75d8, "an unfortunate software pirate.\n"));
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x75fa, "a less fortunate adventurer.\n"));
            }
        }

        return;
    }

    // a392
    switch (local_c & 0xf0)
    {
    case 0x00: // a3b8
        ULTIMA_1850_PrintString(_TEXT(0x7618, "a passage.\n"));
        break;
    case 0x10: // a3d6
        ULTIMA_1850_PrintString(_TEXT(0x7624, "an up ladder.\n"));
        break;
    case 0x20: // a3dc
        ULTIMA_1850_PrintString(_TEXT(0x7634, "a down ladder.\n"));
        break;
    case 0x30: // a3e2
        ULTIMA_1850_PrintString(_TEXT(0x7644, "a ladder.\n"));
        break;
    case 0x40:
        ULTIMA_1850_PrintString(_TEXT(0x7650, "a wooden chest.\n"));
        break;
    case 0x50:
        ULTIMA_1850_PrintString(_TEXT(0x7662, "a fountain.\n"));
        break;
    case 0x60:
        ULTIMA_1850_PrintString(_TEXT(0x7670, "a pit.\n"));
        break;
    case 0x70: // a3fa
        ULTIMA_1850_PrintString(_TEXT(0x7678, "an open chest.\n"));
        break;
    case 0x80:
        ULTIMA_1850_PrintString(_TEXT(0x7688, "an energy field.\n"));
        break;
    case 0x90:
        ULTIMA_1850_PrintString(_TEXT(0x769a, "nothing of note.\n"));
        break;
    case 0xa0:
        ULTIMA_1850_PrintString(_TEXT(0x76ac, "a heavy door.\n"));
        break;
    case 0xb0:
        ULTIMA_1850_PrintString(_TEXT(0x76bc, "a wall.\n"));
        break;
    case 0xc0:
        ULTIMA_1850_PrintString(_TEXT(0x76c6, "SPEC WALL ERR.\n"));
        break;
    case 0xd0:
        ULTIMA_1850_PrintString(_TEXT(0x76d6, "a wall.\n"));
        break;
    case 0xe0:
        ULTIMA_1850_PrintString(_TEXT(0x76e0, "a heavy door.\n"));
        break;
    case 0xf0: // a42a
        ULTIMA_1850_PrintString(_TEXT(0x76f0, "a heavy door.\n"));
        break;
    }

    // a3bf
    if ((local_c & 0xf0) == 0x50)
    {
        // a3cb
        ULTIMA_1850_PrintString(_TEXT(0x7700, "Will you drink?\n"));
        // -> a480

        // a480
        while ((local_6 = (byte)ULTIMA_266c_GetChar()) != 'Y' && local_6 != 'N')
        {
        }

        // a48a
        if (local_6 == 'N')
        {
            ULTIMA_1850_PrintString(_TEXT(0x7712, "No.\n"));
        }
        else
        {
            // a496~ (OK)
            ULTIMA_1850_PrintString(_TEXT(0x7718, "Yes.  Gulp!\n"));

            switch (local_c)
            {
            case 0x50:
                ULTIMA_1850_PrintString(_TEXT(0x7726, "Cured!\n"));
                D_55a8_party[local_e_am].status = STATUS_GOOD;
                break;

            case 0x51:
                ULTIMA_1850_PrintString(_TEXT(0x772e, "Healed!\n"));
                D_55a8_party[local_e_am].hp = D_55a8_party[local_e_am].maxHp;
                break;

            case 0x52:
                ULTIMA_1850_PrintString(_TEXT(0x7738, "Poisoned!\n"));
                D_55a8_party[local_e_am].status = STATUS_POISONED;
                break;

            default:
                ULTIMA_1850_PrintString(_TEXT(0x7744, "Bad taste.\n"));
                ULTIMA_2a52(local_e_am, ULTIMA_2092_RandomRange(0, 7));
                break;
            }
        }
    }
    // a50e
}

static void DNGLOOK_0284(int param_1, int param_2)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b6_magentaColor + 8);
    ULTIMA_0b10_GRAP_Line(param_1 + 1, param_2, param_1 + 6, param_2);
    ULTIMA_0b10_GRAP_Line(param_1 + 1, param_2 + 1, param_1 + 6, param_2 + 1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor + 8);
    ULTIMA_0b10_GRAP_Line(param_1 + 1, param_2 + 2, param_1 + 6, param_2 + 2);
    ULTIMA_0b10_GRAP_Line(param_1 + 1, param_2 + 3, param_1 + 6, param_2 + 3);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
    ULTIMA_0b10_GRAP_Line(param_1 + 1, param_2 + 4, param_1 + 6, param_2 + 4);
    ULTIMA_0b10_GRAP_Line(param_1 + 1, param_2 + 5, param_1 + 6, param_2 + 5);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b4_greenColor + 8);
    ULTIMA_0b10_GRAP_Line(param_1 + 1, param_2 + 6, param_1 + 6, param_2 + 6);
    ULTIMA_0b10_GRAP_Line(param_1 + 1, param_2 + 7, param_1 + 6, param_2 + 7);
}

// "view": put one tile (x, y)
static int DNGLOOK_0340(int param_1, int param_2)
{
    byte local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_8 = 0;
    if (param_1 < 0 || param_1 > 0x15 || param_2 < 0 || param_2 > 0x15)
    {
        return 0;
    }

    if (D_ad14[param_2 * 0x20 + param_1] != 0)
    {
        local_8 = 1;
        D_ad14[param_2 * 0x20 + param_1] = 0;

        local_4_ab = (param_1 + D_5896_map_x - 0xb) & 7;
        local_6 = (param_2 + D_5897_map_y - 0xb) & 7;
        local_a = GetDungeonMap(local_4_ab, local_6, D_5895_map_level);
        local_a >>= 4;

        ULTIMA_1bf2_SetTextPosition(param_1 + 1, param_2 + 1);

        // a66a
        param_1 = param_1 * 8 + 8;
        param_2 = param_2 * 8 + 8;
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);

        switch (local_a)
        {
        case 0:
            if ((GetDungeonMap(local_4_ab, local_6, D_5895_map_level) & 8) != 0)
            {
                ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
                ULTIMA_16ba_PrintChar(0x18);
            }
            break;

        case 1:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
            ULTIMA_16ba_PrintChar(0x2e); // ladder up (TODO: define?)
            break;

        case 2:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
            ULTIMA_16ba_PrintChar(0x2d); // ladder down
            break;

        case 3:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
            ULTIMA_16ba_PrintChar(0x2f); // ladder u/d
            break;

        case 4:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13b8_brightYellowColor);
            ULTIMA_16ba_PrintChar(0x70); // chest?
            break;

        case 5:
            ULTIMA_0b10_GRAP_Line(param_1 + 9 - 8, param_2 + 0xc - 8, param_1 + 0xe - 8, param_2 + 0xc - 8);
            ULTIMA_0b10_GRAP_Line(param_1 + 10 - 8, param_2 + 0xd - 8, param_1 + 0xd - 8, param_2 + 0xd - 8);
            ULTIMA_0b10_GRAP_Line(param_1 + 9 - 8, param_2 + 0xe - 8, param_1 + 10 - 8, param_2 + 0xe - 8);
            ULTIMA_0b10_GRAP_Line(param_1 + 0xd - 8, param_2 + 0xe - 8, param_1 + 0xe - 8, param_2 + 0xe - 8);
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
            ULTIMA_0c64_GRAP_30_Pset(param_1 + 9 - 8, param_2 + 10 - 8);
            ULTIMA_0c64_GRAP_30_Pset(param_1 + 10 - 8, param_2 + 9 - 8);
            ULTIMA_0b10_GRAP_Line(param_1 + 0xb - 8, param_2 + 10 - 8, param_1 + 0xc - 8, param_2 + 10 - 8);
            ULTIMA_0b10_GRAP_Line(param_1 + 0xb - 8, param_2 + 0xb - 8, param_1 + 0xc - 8, param_2 + 0xb - 8);
            ULTIMA_0c64_GRAP_30_Pset(param_1 + 0xd - 8, param_2 + 9 - 8);
            ULTIMA_0c64_GRAP_30_Pset(param_1 + 0xe - 8, param_2 + 10 - 8);
            break;

        case 6:
            // a7e4
            ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
            
            switch (GetDungeonMap(local_4_ab, local_6, (uint)D_5895_map_level))
            {
            case 96:
                ULTIMA_16ba_PrintChar(0x19);
                break;
            case 97:
            case 105:
                ULTIMA_1c9e_SelectCharset(1);
                ULTIMA_16ba_PrintChar(0x71); // pit trap
                break;
            case 104:
                ULTIMA_16ba_PrintChar(0x12);
                break;
            default:
                ULTIMA_1c9e_SelectCharset(1);
                ULTIMA_1cca_SetTextForegroundColor(D_13ae_redColor + 8);
                ULTIMA_16ba_PrintChar(0x72);
                break;
            }

            break;

        case 8:
            DNGLOOK_0284(param_1, param_2);
            break;

        case 10:
        case 0xf:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13b8_brightYellowColor);
            ULTIMA_16ba_PrintChar(0x73);
            break;

        case 0xb:
            if (GetDungeonMap(local_4_ab, local_6, (uint)D_5895_map_level) == 0xb0)
            {
                ULTIMA_16ba_PrintChar(0x7f);
            }
            else
            {
                ULTIMA_1c9e_SelectCharset(1);
                ULTIMA_16ba_PrintChar(0x74);
            }
            local_8 = 0;
            break;

        case 0xc:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1f26_SetTextBackgroundColor(D_13b2_blueColor);
            ULTIMA_16ba_PrintChar(0x75);
            ULTIMA_1f26_SetTextBackgroundColor(0);
            local_8 = 0;
            break;

        case 0xd:
            ULTIMA_1c9e_SelectCharset(1);
            if (D_52c8 == 0)
            {
                ULTIMA_1f26_SetTextBackgroundColor(D_13b2_blueColor);
            }
            else if (D_52c8 == 1 || D_52c8 == 2)
            {
                ULTIMA_1f26_SetTextBackgroundColor(D_13ba_lightGrayColor);
            }

            ULTIMA_16ba_PrintChar(0x76);
            ULTIMA_1f26_SetTextBackgroundColor(0);
            local_8 = 0;
            break;

        case 0xe:
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1cca_SetTextForegroundColor(D_13b8_brightYellowColor);
            ULTIMA_16ba_PrintChar(0x77);
            break;
        }
    }

    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
    ULTIMA_1c9e_SelectCharset(0);

    return local_8;
}

void DNGLOOK_06a8_ViewCmd(void)
{
    int local_16_ak;
    int local_14_am;
    int local_12_a;
    byte* local_10_aa;
    int local_e_am;
    byte* local_c;
    int local_a;
    byte* local_8;
    byte* local_6;
    byte* local_4_ab;

    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);

    local_4_ab = local_c = D_a528;
    local_6 = local_10_aa = D_a628;
    local_8 = &D_a528[0xff];

    memset(D_ad14, 0xff, 0x2e0);

    *local_4_ab = 0xb;
    local_4_ab++;
    *local_6 = 0xb;
    local_6++;
    local_12_a = 0x60;
    local_14_am = 0x60;
    D_ad14[0x16b] = 0;

    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(0xc, 0xc);
    ULTIMA_1c9e_SelectCharset(1);
    ULTIMA_1cca_SetTextForegroundColor(D_13b4_greenColor + 8);
    if (D_52c8 == 3)
    {
        ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
    }

    // a9ce
    ULTIMA_16ba_PrintChar(0x60);
    ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
    ULTIMA_1c9e_SelectCharset(0);
    // -> aa66

    while (local_4_ab != local_c) // aa66
    {
        for (local_16_ak = 0; local_16_ak < 8; local_16_ak++) // aa3f
        {
            local_a = local_12_a = *local_c;
            local_e_am = local_14_am = *local_10_aa;

            // ok
            switch (local_16_ak)
            {
            case 0:
                local_12_a--;
                local_14_am--;
                break;

            case 1:
                local_14_am--;
                break;

            case 2:
                local_12_a++;
                local_14_am--;
                break;

            case 3:
                local_12_a--;
                break;

            case 4:
                local_12_a++;
                break;

            case 5:
                local_12_a--;
                local_14_am++;
                break;

            case 6:
                local_14_am++;
                break;

            case 7:
                local_12_a++;
                local_14_am++;
                break;
            }

            // aa3f ok
            if (DNGLOOK_0340(local_12_a, local_14_am) != 0)
            {
                *local_4_ab++ = local_12_a;
                *local_6++ = local_14_am;

                if (local_4_ab > local_8)
                {
                    local_4_ab = D_a528;
                    local_6 = D_a628;
                }
            }
        }

        local_c++;
        local_10_aa++;
        if (local_c > local_8)
        {
            local_c = D_a528;
            local_10_aa = D_a628;
        }
    }

    // aaaa ok
    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_1dda_WaitForKeystroke(0);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    DUNGEON_1be0();
}

// CHECKED (nop)
void DNGLOOK_0844(int param_1)
{
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    for (local_6 = 0; local_6 < D_3840; local_6++)
    {
        if (D_383a[local_6] == (D_5893_map_id & 0xf) * 0x10 + param_1)
        {
            return; // nop NOT MATCHING
        }
    }

    local_4_ab = D_5893_map_id - 0x21;
    if (local_4_ab >= 1)
    {
        local_4_ab--;
    }

    local_a = param_1 + local_4_ab * 0x10;
    local_8 = local_a / 8;
    D_58e0[local_8] |= 1 << ((byte)local_a & 7);
}

static int DNGLOOK_08d4(int param_1)
{
    int local_6;
    int local_c;
    int local_a;
    u8 local_8;
    u8 local_4_ab;

    local_6 = (int)D_5893_map_id - 0x21;
    if (local_6 >= 1) {
        local_6--;
    }

    // 08ed
    local_c = local_6 * 0x10 + param_1;
    local_a = local_c / 8;
    local_4_ab = D_58e0[local_a];
    local_8 = 1 << (local_c & 7);

    if (local_8 & local_4_ab)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void DNGLOOK_093a(void)
{
    byte local_c;
    byte* local_a;
    int local_8;
    int local_6;
    byte local_4_ab;

    local_a = D_595a;

    for (local_6 = 0; local_6 < 0x200; local_6++)
    {
        local_c = *local_a;
        local_4_ab = local_c & 0xf0;
        if (local_4_ab == 0xf0)
        {
            local_8 = local_c & 0xf;
            if (DNGLOOK_08d4(local_8) != 0)
            {
                *local_a &= 0xaf;
            }
        }
        local_a++;
    }
}

static void DNGLOOK_097e(byte param_1, int param_2)
{
    int local_6;
    int local_4_ab;
    int local_8;
    int local_c;
    int local_a;

    local_a = 0;
    local_c = 0;
    local_4_ab = 0;
    local_6 = 0;

    switch (param_2)
    {
    case 0:
        local_a++;
        break;

    case 1:
        local_c++;
        local_4_ab = 10;
        break;

    case 2:
        local_a++;
        local_6 = 10;
        break;

    case 3:
        local_c++;
        break;
    }

    for (local_8 = 0; local_8 < 0xb; local_8++)
    {
        GetCombatMap(local_4_ab, local_6) = 0xff;
        local_4_ab += local_a;
        local_6 += local_c;
    }

    if (param_2 == 0 && param_1 == 0xe0)
    {
        GetCombatMap(5, 2) = GetCombatMap(5, 8) = D_bb14;
    }

    if (param_2 == 3 && param_1 == 0xe0)
    {
        GetCombatMap(2, 5) = GetCombatMap(8, 5) = D_bb14;
    }
}

static void DNGLOOK_0a48(int param_1)
{
    int local_8;
    int local_c;
    int local_a;
    int local_6;
    int local_4_ab;

    local_a = 0;
    local_c = 0;
    local_4_ab = 3;
    local_6 = 3;

    switch (param_1)
    {
    case 0:
        local_a++;
        local_6 = 1;
        break;

    case 1:
        local_c++;
        local_4_ab = 9;
        break;

    case 2:
        local_a++;
        local_6 = 9;
        break;

    case 3:
        local_c++;
        local_4_ab = 1;
        break;
    }

    for (local_8 = 0; local_8 < 5; local_8++)
    {
        GetCombatMap(local_4_ab, local_6) = D_bb15;
        local_4_ab += local_a;
        local_6 += local_c;
    }
}

static void DNGLOOK_0aee(int param_1)
{
    int local_6;
    int local_4_ab;
    int local_8;
    int local_c;
    int local_a;

    local_a = 0;
    local_c = 0;

    switch (param_1)
    {
    case 0:
        local_a++;
        local_4_ab = 2;
        local_6 = 1;
        break;

    case 1:
        local_c++;
        local_4_ab = 9;
        local_6 = 2;
        break;

    case 2:
        local_a++;
        local_4_ab = 2;
        local_6 = 9;
        break;

    case 3:
        local_c++;
        local_4_ab = 1;
        local_6 = 2;
        break;
    }

    for (local_8 = 0; local_8 < 7; local_8++)
    {
        GetCombatMap(local_4_ab, local_6) = D_bb15;
        local_4_ab += local_a;
        local_6 += local_c;
    }
}

// CHECKED (nop)
static void DNGLOOK_0b9e(int param_1)
{
    byte local_8;
    int local_6;
    int local_4_ab;

    local_4_ab = D_5896_map_x;
    local_6 = D_5897_map_y;
    switch (param_1)
    {
    case 0:
        local_6--;
        break;

    case 1:
        local_4_ab++;
        break;

    case 2:
        local_6++;
        break;

    case 3:
        local_4_ab--;
        break;
    }

    local_8 = GetDungeonMap(local_4_ab & 7, local_6 & 7, D_5895_map_level);
    local_8 &= 0xf0;
    if (local_8 < 0xa0)
    {
        DNGLOOK_0aee(param_1);
    }

    if (local_8 == 0xb0 || local_8 == 0xc0 || local_8 == 0xd0)
    {
        DNGLOOK_097e(GetDungeonMap(D_5896_map_x, D_5897_map_y, D_5895_map_level) & 0xf0, param_1);
        // nop
    }
    else
    {
        DNGLOOK_0a48(param_1);
    }
}

// CHECKED (loop)
void DNGLOOK_0c6c(void)
{
    byte local_8;
    int local_6;

    byte* local_4_aa;

    local_4_aa = &GetCombatMap(0, 1);
    memset(local_4_aa, D_bb14, 0xb);

    local_4_aa = &GetCombatMap(0, 9);
    memset(local_4_aa, D_bb14, 0xb);

    // af2e; NOT MATCHING
    for (local_6 = 0; local_6 < 0xb; local_6++)
    {
        GetCombatMap(1, local_6) = GetCombatMap(9, local_6) = D_bb14;
    }

    GetCombatMap(0, 0) = GetCombatMap(10, 0) = GetCombatMap(0, 10) = GetCombatMap(10, 10) = 0xff;

    local_8 = GetDungeonMap(D_5896_map_x, D_5897_map_y, D_5895_map_level);
    local_8 &= 0xf0;
    if (local_8 != 0 && local_8 < 0x80)
    {
        byte local_c;

        local_8 >>= 4;
        local_c = D_244a[local_8];
        if (local_c != 0)
        {
            GetCombatMap(5, 5) = local_c;
        }

        if (local_8 == 3)
        {
            D_bb16 = 1;
            ULTIMA_6fbc(0);
        }
        else
        {
            D_bb16 = 0;
        }
    }

    for (local_6 = 0; local_6 < 4; local_6++)
    {
        DNGLOOK_0b9e(local_6);
    }
}

// CHECKED (optimization)
void DNGLOOK_0d3e(void)
{
    byte* local_34_ak;
    int local_32_am;
    int local_30_a;
    int local_2e_ak[16];
    byte* local_e_am;
    byte* local_c;
    byte* local_a;
    byte* local_8;
    byte* local_6;
    int local_4_ab;

    for (local_30_a = 0; local_30_a < 0xb; local_30_a++)
    {
        local_34_ak = &GetCombatMap(0, local_30_a);
        memset(local_34_ak, D_bb15, 0xb);
    }

    if ((D_58a1 & 4) != 0)
    {
        GetCombatMap(5, 5) = 0xb3;
    }

    // b006
    DNGLOOK_0c6c();

    for (local_30_a = 0; local_30_a < 8; local_30_a++)
    {
        GetCombatMapTriggerX(local_30_a) = GetCombatMapTriggerY(local_30_a) = 0xff;
    }

    local_34_ak = &GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_SOUTH, 0);
    local_a = &GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_WEST, 0);
    local_6 = &GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_NORTH, 0);
    local_e_am = &GetCombatMapPartyX(COMBAT_MAP_SPECIAL_PARTY_EAST, 0);

    for (local_30_a = 0; local_30_a < 6; local_30_a++)
    {
        if ((D_58a1 & 4) != 0)
        {
            *local_34_ak = *local_a = *local_6 = *local_e_am = D_2452[local_30_a];
            // nop
        }
        else
        {
            *local_34_ak = D_2470[local_30_a];
            *local_a = D_246a[local_30_a];
            *local_6 = D_245e[local_30_a];
            *local_e_am = D_2464[local_30_a];
        }
        local_34_ak++;
        local_a++;
        local_6++;
        local_e_am++;
    }

    // OK
    for (local_30_a = 0; local_30_a < 6; local_30_a++)
    {
        if ((D_58a1 & 4) != 0)
        {
            *local_34_ak = *local_a = *local_6 = *local_e_am = D_2458[local_30_a];
        }
        else
        {
            *local_34_ak = D_2464[local_30_a];
            *local_a = D_245e[local_30_a];
            *local_6 = D_246a[local_30_a];
            *local_e_am = D_2470[local_30_a];
        }
        local_34_ak++;
        local_a++;
        local_6++;
        local_e_am++;
    }

    // b0e3
    local_8 = &GetCombatMapMonsterX(0);
    local_c = &GetCombatMapMonsterY(0);

    // OK
    for (local_30_a = 0; local_30_a < 0x10; local_30_a++)
    {
        // b0f8
        if ((D_58a1 & 4) != 0)
        {
            *local_c = D_24c6[local_30_a];
            *local_8 = D_24b6[local_30_a];
        }
        else
        {
            // b10e
            switch (D_6603)
            {
            case 0:
                *local_8 = D_2476[local_30_a];
                *local_c = D_2496[local_30_a];
                break;
            case 1:
                *local_8 = D_2486[local_30_a];
                *local_c = D_24a6[local_30_a];
                break;
            case 2:
                *local_8 = D_24a6[local_30_a];
                *local_c = D_2486[local_30_a];
                break;
            case 3:
                *local_8 = D_2496[local_30_a];
                *local_c = D_2476[local_30_a];
                break;
            }
        }

        local_8++;
        local_c++;
    }

    for (local_30_a = 0; local_30_a < 0x10; local_30_a++)
    {
        local_2e_ak[local_30_a] = local_30_a;
    }

    // b18d
    for (local_30_a = 0; local_30_a < 0x10; local_30_a++)
    {
        local_32_am = ULTIMA_2092_RandomRange(0, 0xf);

        local_4_ab = local_2e_ak[local_30_a];
        local_2e_ak[local_30_a] = local_2e_ak[local_32_am];
        local_2e_ak[local_32_am] = local_4_ab;

        GetCombatMapMonsterTile(local_30_a) = 0;
    }

    // b1cf OK
    if ((D_58a1 & 4) != 0)
    {
        local_32_am = D_173c[ULTIMA_2092_RandomRange(0, 7)];
    }
    else
    {
        local_32_am = D_5c5a[1]._5;
    }

    local_4_ab = ULTIMA_2092_RandomRange(1, D_13bc_enemyStats[local_32_am].maxEncounterCount);
    if (D_13bc_enemyStats[local_32_am].maxEncounterCount == 8 || D_13bc_enemyStats[local_32_am].maxEncounterCount == 0x10)
    {
        local_4_ab = D_13bc_enemyStats[local_32_am].maxEncounterCount;
    }

    // b223
    for (local_30_a = 0; local_30_a < local_4_ab; local_30_a++)
    {
        GetCombatMapMonsterTile(local_2e_ak[local_30_a]) = local_32_am * 4 + 64;
    }
}

void DNGLOOK_0fda(void)
{
    switch (D_58a0)
    {
    case DIR_UP:
        D_5897_map_y--;
        D_6602 = D_6603 = 0;
        if (7 < D_5897_map_y)
        {
            D_5897_map_y = 7;
        }
        break;

    case DIR_RIGHT:
        D_5896_map_x++;
        D_6602 = D_6603 = 1;
        if (7 < D_5896_map_x)
        {
            D_5896_map_x = 0;
        }
        break;

    case DIR_DOWN:
        D_5897_map_y++;
        D_6602 = D_6603 = 2;
        if (7 < D_5897_map_y)
        {
            D_5897_map_y = 0;
        }
        break;

    case DIR_LEFT:
        D_5896_map_x--;
        D_6602 = D_6603 = 3;
        if (7 < D_5896_map_x)
        {
            D_5896_map_x = 7;
        }
        break;

    case 5:
        if (D_5895_map_level == 0)
        {
            DUNGEON_1d08_Exit();
            D_5893_map_id = 0;
        }
        else
        {
            D_5895_map_level--;
            D_6602 = 4;
        }
        break;

    case 6:
        if (D_5895_map_level == 7)
        {
            DUNGEON_1d08_Exit();
            D_5893_map_id = 0;
        }
        else
        {
            D_5895_map_level++;
            D_6602 = 5;
        }
        break;
    }
}

void DNGLOOK_109e(int param_1)
{
    if (D_5893_map_id != 0)
    {
        if ((D_bb17 & 2) != 0)
        {
            ULTIMA_102e_UnloadTileset();
        }

        if ((D_bb17 & 1) == 0)
        {
            do
            {
                D_a9c0 = ULTIMA_0bae_LoadImageFile(D_25ea[4 + D_6604]); // DNGn.16
            } while (D_a9c0 == 0);

            do
            {
                D_a9c4 = ULTIMA_0bae_LoadImageFile(D_25ea[4]); // ITEMS.16
            } while (D_a9c4 == 0);
        }

        D_bb17 = 1;
        if (param_1 != 0)
        {
            D_5c5a[1]._0_tile = D_5c5a[1]._1_animTile = 0;
            D_5c5a[1]._5 = D_5c5a[1]._2_x = D_5c5a[1]._3_y = 0xff;
        }

        ULTIMA_0a70_GRAP_2d_SetPenColor(0);
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
        ULTIMA_0c22_GRAP_0f_SelectPage(0);
    }
}

// on leave (cleanup)
void DNGLOOK_1130(void)
{
#if defined(ENABLE_BGM)
    AUDIO_StopBgm();
#endif

    if ((D_bb17 & 1) != 0)
    {
        if (D_5c5a[1]._5 != 0xff)
        {
            ULTIMA_0be4_FreeImage(D_a9c6);
        }
        D_a9c6 = 0;
        ULTIMA_0be4_FreeImage(D_a9c4);
        ULTIMA_0be4_FreeImage(D_a9c0);
    }

    if ((D_bb17 & 2) == 0)
    {
        while (ULTIMA_0ff4_LoadTileset(*D_25f0) == 0) // TILES.16
            ;
    }

    if ((D_bb17 & 4) == 0)
    {
        ULTIMA_6f9e(0);
    }

    D_bb17 = 6;
}

// CHECKED (optimization)
// prepare combat (in dungeon)
void DNGLOOK_117e(int param_1, int param_2)
{
    int local_16_ak;
    int local_14_am;
    int local_12_a;
    int local_10_aa;
    int local_e_am;
    int local_c;
    byte local_a;
    byte local_8[4];
    int local_4_ab;

    local_c = COMBAT_MAP_PARTY_X_INDEX;
    local_e_am = COMBAT_MAP_PARTY_Y_INDEX;
    local_4_ab = COMBAT_MAP_SPECIAL_PARTY_NORTH;
    if (param_2 != 1)
    {
        // b42e
        switch (D_6603)
        {
        case 0:
        case 5:
            // b44c
            local_4_ab = COMBAT_MAP_SPECIAL_PARTY_SOUTH;
            break;
        case 1:
            // b4c2
            local_4_ab = COMBAT_MAP_SPECIAL_PARTY_WEST;
            break;
        case 3:
            // b4ca
            local_4_ab = COMBAT_MAP_SPECIAL_PARTY_EAST;
            break;
        }

        // b451
        ULTIMA_6936();

        for (local_10_aa = 0; local_10_aa < D_585b; local_10_aa++)
        {
            D_ba14[local_10_aa].x = D_5c5a[local_10_aa]._2_x = GetCombatMapSpecial(local_4_ab, local_c);
            D_ba14[local_10_aa].y = D_5c5a[local_10_aa]._3_y = GetCombatMapSpecial(local_4_ab, local_e_am);
            local_c++;
            local_e_am++;
        }
    }

    // b4d8
    if (param_2 > 0 && ((param_1 > 0xef && param_2 == 3) || param_2 < 3))
    {
        local_4_ab = local_e_am = local_c = COMBAT_MAP_SPECIAL_BASE;

        for (local_10_aa = 0; local_10_aa < 4; local_10_aa++)
        {
            local_8[local_10_aa] = D_385e[ULTIMA_2092_RandomRange(0, 7)];
        }

        // b521
        for (local_10_aa = 0; local_10_aa < 0x10; local_10_aa++)
        {
            local_a = GetCombatMapSpecial(COMBAT_MAP_SPECIAL_MONSTER_TILES, local_4_ab);
            if (local_a != 0)
            { 
                // b542
                if (local_a < 0x40 || (local_a & 0xfc) == 0xb4 || (local_a & 0xfc) == 0xe8)
                {
                    local_14_am = 2;
                    local_16_ak = local_a;
                    // nop
                }
                else
                {
                    local_14_am = 0;
                    local_16_ak = (local_a - 0x40) / 4;
                }

                // b575
                if ((local_a & 0xfc) == 0xec)
                {
                    local_16_ak = local_8[local_a & 3];
                }

                // b58e
                local_12_a = ULTIMA_6506(local_16_ak, local_14_am, GetCombatMapSpecial(COMBAT_MAP_SPECIAL_MONSTER_X, local_c),
                                       GetCombatMapSpecial(COMBAT_MAP_SPECIAL_MONSTER_Y, local_e_am), D_5895_map_level);
                if (local_14_am == 2)
                {
                    if (local_16_ak == 1)
                    {
                        D_5c5a[local_12_a]._5 = D_5895_map_level * 3 + 7;
                        // nop
                    }
                    else if (local_16_ak == 2)
                    {
                        D_5c5a[local_12_a]._5 = ULTIMA_2092_RandomRange(1, D_5895_map_level * 10 + 10);
                    }
                    else if (local_16_ak < 0x10)
                    {
                        ASSERT(local_16_ak != 0);
                        // TODO: offset
                        D_5c5a[local_12_a]._5 = D_3850[local_16_ak - 3] + ULTIMA_2092_RandomRange(0, D_3842[local_16_ak - 3] - 1);
                    }
                }
            }

            local_4_ab++;
            local_c++;
            local_e_am++;
        }
    }
}
