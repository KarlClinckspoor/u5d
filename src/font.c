#include "common/common.h"
#include "savegame.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include <string.h>

#include "audio/audio.h"

#include "font.h"
#include "intro.h"
#include "tiles.h"

// glyph widths from ' ' (never read: spaces are D_5154 wide) to 'z'
u8 D_50ea[0x5b] =
{
    0x00,
    0x02, 0x06, 0x07, 0x07, 0x07, 0x07, 0x03, 0x04,
    0x04, 0x05, 0x06, 0x03, 0x03, 0x02, 0x07, 0x06,
    0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x02, 0x03, 0x05, 0x05, 0x05, 0x06, 0x07,
    0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x04, 0x07, 0x07, 0x06, 0x07, 0x06, 0x06, 0x07,
    0x06, 0x07, 0x06, 0x06, 0x06, 0x06, 0x07, 0x06,
    0x06, 0x06, 0x03, 0x07, 0x03, 0x07, 0x08, 0x03,
    0x05, 0x05, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x02, 0x03, 0x05, 0x02, 0x07, 0x06, 0x05, 0x05,
    0x05, 0x04, 0x04, 0x04, 0x05, 0x05, 0x07, 0x05,
    0x05, 0x04
};

u16 D_5146[2] = {0, 0};
u16 D_514a = 0; // no code refers to it

u16 D_514c[2] = {0x0140, 0x0140};

u16 D_5150 = 0x00c8;

u16 D_5152 = 0x00c8;

u16 D_5154 = 5;

u16 D_5156 = 0;

u16 D_5158 = 0x003c;
u16 D_515a = 0;

char* D_515c[4] =
{
    _STATIC_TEXT(0xa010, "The Summoning"),
    _STATIC_TEXT(0xa01e, "The Journey"),
    _STATIC_TEXT(0xa02a, "The Arrival"),
    _STATIC_TEXT(0xa036, "The Welcoming")
};

u8 D_5164[8] = {0x02, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00};

u8 D_516c[8] = {0x00, 0x02, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00};

u8 D_5174[8] = {0x00, 0x00, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00};

u16 D_517c[8][8] =
{
    0x0000, 0x06c2, 0x07a7, 0x088c, 0x0960, 0x0a47, 0x0b2d, 0x0c11,
    0x06c2, 0x0000, 0x0cda, 0x0d8f, 0x0e77, 0x0f56, 0x1039, 0x111b,
    0x07a7, 0x0cda, 0x0000, 0x11ff, 0x12db, 0x1374, 0x1456, 0x14ed,
    0x088c, 0x0d8f, 0x11ff, 0x0000, 0x15d1, 0x16af, 0x1794, 0x185b,
    0x0960, 0x0e77, 0x12db, 0x15d1, 0x0000, 0x192f, 0x1a18, 0x1b00,
    0x0a47, 0x0f56, 0x1374, 0x16af, 0x192f, 0x0000, 0x1be3, 0x1cbb,
    0x0b2d, 0x1039, 0x1456, 0x1794, 0x1a18, 0x1be3, 0x0000, 0x1d87,
    0x0c11, 0x111b, 0x14ed, 0x185b, 0x1b00, 0x1cbb, 0x1d87, 0x0000
};

u8 D_51fc[8] = {0x28, 0x30, 0x30, 0x28, 0x28, 0x30, 0x28, 0x30};

u8 D_5204[8] = {0x05, 0x07, 0x04, 0x0a, 0x08, 0x00, 0x05, 0x06};

// CHECKED
// NOT MATCHING (SIZE 698/674, 544 bytes): the first inner loop differs from its start. The
// original keeps local_4_ab, local_a, local_10_ad_am and param_2 in SI, DI, CX and DX
// (stored when the loop ends), rereads param_2[i] through "mov bx,cx; add bx,dx" at every
// test, compares the characters unsigned ("ja"/"jbe": byte, not char) and places the
// "{"/"_"/glyph branch at the loop's top, the "<= 0x20" branch after the merged test. Ours
// registers nothing there and keeps param_2[i] in a stack temporary (as in CMDS_12c8).
// param_2 is byte* (544 -> 536 bytes, same shape). Also tried, no better: the inner loop as
// a for with the two assignments in its init; "if (c > 0x20) {...} else" (548); *(param_2 + i)
// in the test, the body or both; all of the file's data defined after FONT_0000, only its own
// data, or none (capacity: no change, so not an object boundary).
// param_1: image, param_2: text
void FONT_0000(byte* param_1, byte* param_2)
{
    int local_12_ab_am;
    int local_10_ad_am;
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    local_12_ab_am = ((int)D_5158 > (int)D_5150 && (int)D_5158 < (int)D_5152) ? 1 : 0; // TODO: D_5158, D_5150, D_5152 -> int?

    local_c = D_514c[local_12_ab_am] - D_5146[local_12_ab_am];

    local_e_am = local_6 = 0;
    local_4_ab = D_5156 - D_5146[local_12_ab_am];

    while (param_2[local_e_am] != '\0')
    {
        local_a = 0;
        local_10_ad_am = local_e_am;
        while (param_2[local_10_ad_am] != '\0' && local_4_ab < local_c && param_2[local_10_ad_am] != '\n')
        {
            if (param_2[local_10_ad_am] <= 0x20)
            {
                local_4_ab += D_5154;
                local_a += 1;
            }
            else if (param_2[local_10_ad_am] == '{')
            {
                local_4_ab += 0xf;
            }
            else if (param_2[local_10_ad_am] != '_')
            {
                local_4_ab += D_50ea[param_2[local_10_ad_am] - 0x20] + 1;
            }

            local_10_ad_am++;
        }

        while (1)
        {
            if ((param_2[local_10_ad_am] == '\0' && local_4_ab < local_c))
                break;
            if (param_2[local_10_ad_am] == '\n')
                break;
            if (--local_10_ad_am <= local_e_am)
                break;

            if (param_2[local_10_ad_am] == ' ')
            {
                local_4_ab -= D_5154;
                local_a--;
                break;
            }

            if (param_2[local_10_ad_am] == '_')
            {
                if (local_4_ab + D_50f7 + 1 < local_c)
                {
                    local_4_ab += D_50f7 + 1;
                    break;
                }
            }

            if (param_2[local_10_ad_am] != '_' && param_2[local_10_ad_am] != '{')
            {
                local_4_ab -= D_50ea[param_2[local_10_ad_am] - 0x20] + 1;
            }
        }

        local_8 = local_c - local_4_ab;
        local_6 = local_10_ad_am;
        local_10_ad_am = local_e_am;

        for (; local_10_ad_am < local_6; local_10_ad_am++)
        {
            if (param_2[local_10_ad_am] <= 0x20)
            {
                D_5156 += D_5154;
                if (local_a != 0 && param_2[local_6] != '\0' && param_2[local_6] != '\n')
                {
                    D_5156 += local_8 / local_a;
                    local_8 -= local_8 / local_a;
                    local_a--;
                }
            }
            else if (param_2[local_10_ad_am] == '{')
            {
                D_5156 += 0xf;
            }
            else if (param_2[local_10_ad_am] != '_')
            {
                if (D_5158 < 0xc0)
                {
                    // NOTE: "or" mix
                    ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(param_1, param_2[local_10_ad_am] - 0x20, D_5156, D_5158);
                }
                D_5156 += D_50ea[param_2[local_10_ad_am] - 0x20] + 1;
            }
        }

        local_e_am = local_6;
        if (param_2[local_e_am] != '\0')
        {
            if (param_2[local_e_am] == '_' && D_5158 < 0xc0)
            {
                ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(param_1, 0xd, D_5156, D_5158);
            }

            local_4_ab = 0;
            D_5158 += 9;
            local_12_ab_am = ((int)D_5158 > (int)D_5150 && (int)D_5158 < (int)D_5152) ? 1 : 0;

            D_5156 = D_5146[local_12_ab_am];
            local_c = D_514c[local_12_ab_am] - D_5156;
            local_e_am++;
        }
    }

    ULTIMA_0c22_GRAP_0f_SelectPage(0);
}

static void FONT_02a2(int param_1, int param_2)
{
    byte local_4_ab;

    local_4_ab = GetMap(param_1, param_2);
    if (local_4_ab != 0)
    {
        ULTIMA_10e0_GRAP_51_PutTile(D_b11e[local_4_ab], param_1, param_2 + 7);
        return;
    }
    else
    {
        local_4_ab = D_6608[param_1 + param_2 * 0x20 + 0x80];
        if (local_4_ab == 0x16)
        {
            return;
        }

        ULTIMA_10e0_GRAP_51_PutTile(0x100 + local_4_ab, param_1, param_2 + 7);
    }
}

// CHECKED
// The decompiler's local_c (x + y * 0x20, stored at bp-0Ah and never read) is MSC's common
// subexpression temporary: the source indexes D_6608 with the whole expression twice. As a
// local it took bp-8, a slot of its own before the loops' reserved temporaries.
int FONT_02fc(int param_1)
{
    int local_8;
    int local_6;
    ActorFmt* local_4_ab;

    do
    {
        ULTIMA_4552_AnimateActors();
        INTRO_2090_AnimateWD();

        for (local_6 = 0; local_6 < 32; local_6++)
        {
            local_4_ab = &D_5c5a[local_6];

            if (local_4_ab->_0_tile != 0)
            {
                D_6608[local_4_ab->_2_x + local_4_ab->_3_y * 0x20] = 0;
                D_6608[local_4_ab->_2_x + local_4_ab->_3_y * 0x20 + 0x80] = local_4_ab->_1_animTile;
            }
        }

        for (local_6 = D_bd26; local_6 <= D_bd27; local_6++)
        {
            for (local_8 = 0; local_8 < 4; local_8++)
            {
                if (GetMap(local_6, local_8) != 0xfe)
                {
                    FONT_02a2(local_6, local_8);
                }
            }
        }

        if (D_bd26 != 0 && D_bd28 != 0)
        {
            D_bd26 -= 1;
            D_bd27 += 1;
        }

        D_bd28 = !D_bd28;

        if (ULTIMA_1d5e_PollKey() != 0)
        {
            return 1;
        }

        if (param_1 != 0)
        {
            ULTIMA_20fa_WaitTicks(1);
        }

        if (D_bd29 == 2)
        {
            ULTIMA_223c_AudioWhiteNoise(0x14, 0x3c, 10000);
        }
        else if (D_bd29 == 3)
        {
            D_515a++;
            if ((int)D_515a > 7)
            {
                D_515a = 0;
            }

            if (D_515a == 0)
            {
                ULTIMA_22c0_AudioTone(3000, 3);
            }
            else if (D_515a == 4)
            {
                ULTIMA_22c0_AudioTone(2000, 3);
            }
        }
    } while (--param_1 != 0);

    return 0;
}

// CHECKED
static void FONT_0418(int param_1)
{
    int local_6_an;
    int local_4_ab;

    ASSERT(param_1 < 4);

    INTRO_043e(D_515c[param_1]);

    for (local_4_ab = 0; local_4_ab < 0x13; local_4_ab++)
    {
        for (local_6_an = 0; local_6_an < 4; local_6_an++)
        {
            GetCopiedMap(local_4_ab, local_6_an) = GetMap(local_4_ab, local_6_an) = D_b21e[param_1 * 0x80 + local_4_ab + local_6_an * 0x20];
        }
    }

    D_bd26 = D_bd27 = 9;
    D_bd28 = 0;
    D_bd29 = param_1;
}

// assembly in the original (src/asm/AFONT.ASM): save and restore the video page buffer
void FONT_0e52(void);
void FONT_0e7b(void);

// CHECKED
// Case 0xb's loop is a while: as a for (same code), MSC frees its temporaries (bp-20h,
// bp-22h) and case 0xc's pointer walk reuses them, giving a frame of 22h instead of the
// original's 28h (0xc at bp-24h..28h, the middle one in memory). The source steps
// local_12/local_14 with += in case 0xb (the decompiler's local_10 * 9 / * 3 gave MSC's
// closed form instead); += in a for header moves the counter's increment first.
// "view"
void FONT_04a4(void)
{
    int local_18_ai;
    int local_16_ak;
    int local_14_am;
    int local_12;
    int local_10_aa;
    int local_e_am;
    ActorFmt* local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_aa;

    D_52ba_vdp._52be_tileYOffset = 0x10;
    D_bd26 = D_bd27 = 9;
    D_bd28 = 0;

    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x78, 0x78, 199, 0x7e);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(0x78, 0x7f, 199, 0x7f);

    if (D_5893_map_id != 0x42)
    {
        ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa044, "MISCMAPS.DAT"), D_b21e, 2000, 0x2c0);
        if (D_5893_map_id == 0x40)
        {
            FONT_0e52();
            while (ULTIMA_0ff4_LoadTileset(*D_25f0) == 0) {}
            FONT_0e7b();
        }
    }

    D_5893_map_id = 0x42;

    for (local_e_am = 0x200; local_e_am < 2000; local_e_am++)
    {
        switch (D_b21e[local_e_am])
        {
        case 0:
            local_c = &D_5c5a[D_b21e[++local_e_am]];
            local_c->_0_tile = local_c->_1_animTile = D_b21e[++local_e_am];
            local_c->_2_x = D_b21e[++local_e_am];
            local_c->_3_y = D_b21e[++local_e_am];
            local_c->_6 = 0;
            break;

        case 1:
            local_c = &D_5c5a[D_b21e[++local_e_am]];
            local_c->_0_tile = local_c->_1_animTile = local_c->_6 = 0;

            GetMap(local_c->_2_x, local_c->_3_y) = GetCopiedMap(local_c->_2_x, local_c->_3_y);
            break;

        case 2:
            local_c = &D_5c5a[D_b21e[++local_e_am]];
            local_18_ai = D_b21e[++local_e_am];
            GetMap(local_c->_2_x, local_c->_3_y) = GetCopiedMap(local_c->_2_x, local_c->_3_y);
            local_c->_2_x += D_24d6[local_18_ai];
            local_c->_3_y += D_24de[local_18_ai];
            break;

        case 3:
            if (FONT_02fc(D_b21e[++local_e_am]) != 0)
            {
                return;
            }
            break;

        case 4:
            local_6 = D_b21e[++local_e_am];
            local_8 = D_b21e[++local_e_am];
            GetMap(local_6, local_8) = GetCopiedMap(local_6, local_8) = TILE_MAP_FE;

            for (local_10_aa = 1; local_10_aa < 0x10; local_10_aa++)
            {
                ULTIMA_1112_GRAP_60(local_10_aa, local_6, local_8 + 7);
                if (FONT_02fc(1) != 0)
                {
                    return;
                }
            }

            GetMap(local_6, local_8) = GetCopiedMap(local_6, local_8) = TILE_MAP_MOONGATE;
            if (FONT_02fc(2) != 0)
            {
                return;
            }
            break;

        case 5:
            GetMap(local_6, local_8) = GetCopiedMap(local_6, local_8) = TILE_MAP_FE;

            local_10_aa = 0xf;
            do
            {
                ULTIMA_1112_GRAP_60(local_10_aa, local_6, local_8 + 7);
                if (FONT_02fc(1) != 0)
                {
                    return;
                }
                local_10_aa--;
            } while (local_10_aa > 0);

            GetMap(local_6, local_8) = GetCopiedMap(local_6, local_8) = TILE_MAP_GRASS;
            if (FONT_02fc(2) != 0)
            {
                return;
            }
            break;

        case 6:
            FONT_0418(D_b21e[++local_e_am]);
            break;

        case 7:
            local_c = &D_5c5a[D_b21e[++local_e_am]];
            local_a = local_c->_0_tile;

            local_c->_0_tile = local_c->_1_animTile = TILE_ACTOR_CIRCLE;

            if (ULTIMA_1068(0x100 + local_a, local_c->_2_x, local_c->_3_y + 7) != 0)
            {
                return;
            }
            local_c->_0_tile = local_c->_1_animTile = local_a;
            break;

        case 8:
            local_c = &D_5c5a[D_b21e[++local_e_am]];
            local_12 = local_c->_2_x;
            local_14_am = local_c->_3_y;
            local_a = local_c->_0_tile;

            local_c->_0_tile = local_c->_1_animTile = TILE_ACTOR_CIRCLE;

            if (ULTIMA_1068(GetCopiedMap(local_12, local_14_am), local_12, local_14_am + 7) != 0)
            {
                return;
            }

            local_c->_0_tile = local_c->_1_animTile = local_a;
            break;

        case 9:
            local_e_am = 0x1ff;
            break;

        case 10:
            local_a = D_b21e[++local_e_am];
            local_12 = D_b21e[++local_e_am];
            local_14_am = D_b21e[++local_e_am];
            GetCopiedMap(local_12, local_14_am) = local_a;
            GetMap(local_12, local_14_am) = local_a;
            break;

        case 0xb:
            // ...
            local_12 = local_14_am = 0;
            local_10_aa = 0;
            while (local_10_aa < 5)
            {
                if (FONT_02fc(1) != 0)
                {
                    return;
                }
                ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
                ULTIMA_0b10_GRAP_Line(local_12 + 0x80, local_14_am + 0x98, local_12 + 0x89, local_14_am + 0x9b);
                ULTIMA_0b10_GRAP_Line(local_12 + 0x80, local_14_am + 0x99, local_12 + 0x89, local_14_am + 0x9c);
                local_12 += 9;
                local_14_am += 3;
                local_10_aa++;
            }

            local_e_am++;
            local_e_am++;
            local_c = &D_5c5a[D_b21e[local_e_am]];
            ULTIMA_10e0_GRAP_51_PutTile(0, local_c->_2_x, local_c->_3_y + 7);
            ULTIMA_223c_AudioWhiteNoise(1, 0x4b0, 4000);
            if (FONT_02fc(3) != 0)
            {
                return;
            }
            break;

        case 0xc:
            for (local_10_aa = 0; local_10_aa < 0x20; local_10_aa++)
            {
                D_5c5a[local_10_aa]._1_animTile = D_5c5a[local_10_aa]._0_tile = D_5c5a[local_10_aa]._6 = 0;
            }
            break;

        case 0xd:
            local_c = &D_5c5a[D_b21e[++local_e_am]];
            local_18_ai = D_b21e[++local_e_am];
            GetMap(local_c->_2_x, local_c->_3_y) = GetCopiedMap(local_c->_2_x, local_c->_3_y);
            local_c->_2_x += D_24d6[local_18_ai];
            local_c->_3_y += D_24de[local_18_ai];
            if (FONT_02fc(7) != 0)
            {
                return;
            }
            break;

        case 0xe:
            local_16_ak = D_b21e[++local_e_am];
            local_4_aa = local_e_am;
            break;

        case 0xf:
            local_16_ak--;
            if (local_16_ak != 0)
            {
                local_e_am = local_4_aa;
            }
            break;
        }

        if (ULTIMA_1d5e_PollKey() != 0)
        {
            return;
        }
    }
}

// 0998 ~ 0b0a: character creation

static int FONT_0998(void)
{
    int local_4_ab;

    do
    {
        local_4_ab = ULTIMA_2092_RandomRange(0, 7);
    } while (D_bd2a[local_4_ab] != 0 || D_bd32[local_4_ab] != 0);

    D_bd2a[local_4_ab] = 1;

    return local_4_ab;
}

static void FONT_09c8(byte* param_1, byte* param_2)
{
    int local_10_ad_am;
    int local_e_am;
    int local_a;
    byte local_c;
    int local_8;
    int local_6;
    byte local_4_ab;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0d4c_GRAP_4b_PutImage(param_2, 1, 0x10, 0, 0);
    ULTIMA_0d4c_GRAP_4b_PutImage(param_2, 1, 200, 0, 0);

    D_5156 = 0;
    D_5158 = 0x98;

    local_6 = FONT_0998();
    local_8 = FONT_0998();
    if (local_6 > local_8)
    {
        local_e_am = local_8;
        local_10_ad_am = local_6;
        local_c = 'B';
    }
    else
    {
        local_e_am = local_6;
        local_10_ad_am = local_8;
        local_c = 'A';
    }

    ULTIMA_0d4c_GRAP_4b_PutImage(param_2, local_e_am + 2, D_51fc[local_e_am], D_5204[local_e_am], 0);
    ULTIMA_0d4c_GRAP_4b_PutImage(param_2, local_10_ad_am + 2, D_51fc[local_10_ad_am] + 0xb8, D_5204[local_10_ad_am], 0);
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa052, "QUESTION.DAT"), D_b21e, 2000, D_517c[local_6][local_8]);
    FONT_0000(param_1, (char*)D_b21e);
    ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);

    do
    {
        local_4_ab = ULTIMA_2032_ToUpper(ULTIMA_1d5e_PollKey());
        if (local_4_ab == 'A')
            break;
    } while (local_4_ab != 'B');

    if (local_4_ab != local_c)
    {
        local_a = local_6;
        local_6 = local_8;
        local_8 = local_a;
    }

    D_bd3c += D_5164[local_6];
    D_bd3d += D_516c[local_6];
    D_bd3e += D_5174[local_6];
    D_bd32[local_8] = 1;
}

// CHECKED
void FONT_0b0a(void)
{
    byte* local_a;
    byte local_8;
    byte* local_6;
    byte local_4_ab;

    if (D_5893_map_id != 64)
    {
        ULTIMA_102e_UnloadTileset();
    }

    D_a9cb = 0xff;

    do
    {
        local_a = ULTIMA_0fae_LoadResourceFile(D_25ea[0]);
    } while (local_a == 0);

    do
    {
        local_6 = ULTIMA_0bae_LoadImageFile(D_25ea[0x10]);
    } while (local_6 == 0);

#if !defined(TARGET_DOS16)
    FILE_ReadSavegameFile(_TEXT(0xa060, "INIT.GAM"));
#else
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa060, "INIT.GAM"), &D_55a6, (int)&D_6606 - (int)&D_55a6, 0);
#endif
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x78, 0x78, 200, 0x7e);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(0x78, 0x7f, 200, 0x7f);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 0x80, 0x137, 0xbf);
    ULTIMA_1bf2_SetTextPosition(3, 0x11);

    ULTIMA_1850_PrintString(_TEXT(0xa06a, "By what name shalt thou be known?"));
    ULTIMA_1bf2_SetTextPosition(0xe, 0x13);
    ULTIMA_16ba_PrintChar(':');
    ULTIMA_1e38_IntroGetString(D_55a8_party[0].name, 8);

    if (D_55a8_party[0].name[0] != 0)
    {
        ULTIMA_1bf2_SetTextPosition(8, 0x15);
        ULTIMA_1850_PrintString(_TEXT(0xa08c, "Art thou Male or Female? "));

        do
        {
            local_4_ab = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0));
            if (local_4_ab == 'M')
                break;
        } while (local_4_ab != 'F');

        ULTIMA_16ba_PrintChar(local_4_ab);
        if (local_4_ab == 'M')
        {
            D_55a8_party[0].gender = 0xb;
        }
        else
        {
            D_55a8_party[0].gender = 0xc;
        }

        D_5156 = 0;
        D_5158 = 9;
        D_5146[0] = 0;
        D_514c[0] = 0x140;
        D_5146[1] = 0xaf;
        D_514c[1] = 0x140;
        D_5150 = 0x59;
        D_5152 = 200;

#if defined(ENABLE_BGM)
        AUDIO_StopBgm();
#endif

        ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa0a6, "QUESTION.DAT"), D_b21e, 2000, 0);

        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        D_a9bd[1] = 2;
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_0d4c_GRAP_4b_PutImage(local_6, 0, 0, 0x60, 0);
        FONT_0000(local_a, D_b21e);
        ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);

        D_5150 = 200;

        while (ULTIMA_1d5e_PollKey() == 0) {}

        D_bd3c = D_55a8_party[0].intel;
        D_bd3d = D_55a8_party[0].dex;
        D_bd3e = D_55a8_party[0].str;

        for (local_8 = 0; local_8 < 8; local_8++)
        {
            D_bd2a[local_8] = D_bd32[local_8] = 0;
        }

        for (local_8 = 0; local_8 < 4; local_8++)
        {
            FONT_09c8(local_a, local_6);
        }

        for (local_8 = 0; local_8 < 8; local_8++)
        {
            D_bd2a[local_8] = 0;
        }

        for (local_8 = 0; local_8 < 2; local_8++)
        {
            FONT_09c8(local_a, local_6);
        }

        for (local_8 = 0; local_8 < 8; local_8++)
        {
            D_bd2a[local_8] = 0;
        }

        FONT_09c8(local_a, local_6);

        D_5156 = D_5158 = 0;
        D_5150 = 0x5a;
        D_5146[1] = 0;
        D_514c[1] = 0xa6;
        D_5154 = 4;

        ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa0b4, "QUESTION.DAT"), D_b21e, 2000, 0x322);
        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_0d4c_GRAP_4b_PutImage(local_6, 10, 0xa8, 100, 0);

        FONT_0000(local_a, D_b21e);
        D_5154 = 5;
        ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);

        while (ULTIMA_1d5e_PollKey() == 0)
        {}

        ULTIMA_0be4_FreeImage(local_6);
        ULTIMA_0fdc_FreeBitImage(local_a);

        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_0c22_GRAP_0f_SelectPage(0);

        D_a9bd[1] = 0;
        
        D_55a8_party[0].mag = D_55a8_party[0].intel = D_bd3c;
        D_55a8_party[0].dex = D_bd3d;
        //D_55a8_party[0]._c = (D_bd3e - 0x14 & ~-(D_bd3e < 0x14)) + 0x14;
        D_55a8_party[0].str = D_bd3e <= 0x14 ? 0x14 : D_bd3e;
        ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa0c2, "INIT.OOL"), D_b31e, 0x100, 0);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_1bf2_SetTextPosition(0, 10);
        ULTIMA_251e_SwitchDisks(3);

        // NOT MATCHING: sub al, al vs sub ax, ax
        memset(D_b21e, 0, 0x100);

        ULTIMA_25d8_WriteFileToDisk(_TEXT(0xa0cc, "SAVED.OOL"), D_b21e, 0x200);
#if !defined(TARGET_DOS16)
        FILE_WriteSavegameFile(_TEXT(0xa0d6, "SAVED.GAM"));
#else
        ULTIMA_25d8_WriteFileToDisk(_TEXT(0xa0d6, "SAVED.GAM"), &D_55a6, (int)&D_6606 - (int)&D_55a6);
#endif
    }

    ULTIMA_251e_SwitchDisks(0);
    D_5893_map_id = 0x40;
}

#if !defined(TARGET_DOS16) // assembly in the original: src/asm/AFONT.ASM
// The original saves the first 0x3000 bytes of the video page buffer (segment D_5354) into
// overlay area 2 before the tile set is changed, and restores them after; nothing to do here.
void FONT_0e52(void) {}

void FONT_0e7b(void) {}
#endif
