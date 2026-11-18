#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include "audio/aud_mus.h"

#include "blckthrn.h"
#include "cast2.h"
#include "town.h"
u8 D_369e[0x3c] =
{
    0x13, 0x06, 0x44, 0x00, 0x04, 0x07, 0x11, 0x11,
    0x02, 0x02, 0x01, 0x13, 0x1f, 0x12, 0x11, 0x11,
    0x10, 0x01, 0x13, 0x1f, 0x01, 0x13, 0x1c, 0x01,
    0x10, 0x1c, 0x1c, 0x12, 0x06, 0xbb, 0x00, 0x04,
    0x08, 0x19, 0x02, 0x04, 0x01, 0x1a, 0x12, 0x1a,
    0x11, 0x09, 0x08, 0x02, 0x03, 0x11, 0x12, 0x09,
    0x06, 0x02, 0x03, 0x17, 0x16, 0x09, 0x07, 0x02,
    0x06, 0x08, 0x00, 0x00
};

u8 D_36da[0x27] =
{
    0x05, 0x16, 0x12, 0x02, 0x07, 0x11, 0x22, 0x02,
    0x02, 0x01, 0x13, 0x23, 0x22, 0x05, 0x03, 0x06,
    0x82, 0x05, 0x07, 0x09, 0x01, 0x07, 0x02, 0x05,
    0x13, 0x10, 0x05, 0x0c, 0x02, 0x03, 0x17, 0x06,
    0xe9, 0x05, 0x09, 0x02, 0x03, 0x15, 0x00//, 0x00 (gap 1?)
};

u8 D_3702[12] = {0x08, 0x01, 0x10, 0x14, 0x02, 0x03, 0x01, 0x13, 0x15, 0x05, 0x08, 0x00};

u8 D_370e[8] = {0x05, 0x0b, 0x13, 0x02, 0x04, 0x10, 0x11, 0x00};

u8 D_3716[10] = {0x05, 0x04, 0x19, 0x02, 0x05, 0x1a, 0x09, 0x08, 0x07, 0x00};

u16 D_3720[6] = { 0x1130, 0x101d, 0x0e53, 0x0b75, 0x0ce4, 0x0ce4 };

u16 D_372c[6] = { 0xc350, 0xc350, 0xc350, 0x7530, 0x9c40, 0x9c40 };

u16 D_3738[6] = { 0x0bb8, 0x0bb8, 0x0bb8, 0x03e8, 0x0064, 0x9ca4 };

s16 D_3744[6] = { 1, 1, 1, 1, 1, -1 };

static void BLCKTHRN_0000(int param_1)
{
    int local_4;

    for (local_4 = 0; local_4 < param_1; local_4++)
    {
        ULTIMA_433e_AudioFootstep();
        ULTIMA_3ae6_WaitFrames(2);
    }
}

static int BLCKTHRN_002e(int param_1)
{
    int local_4;

    SET_UNINITIALIZED_16(local_4);

    switch (param_1 & 0xfc)
    {
    case 0x10:
        local_4 = 6;
        break;
    case 0x14:
        local_4 = 7;
        break;
    case 0x18:
        local_4 = 8;
        break;
    case 0x1c:
        local_4 = 0;
        break;
    case 0x20:
        local_4 = 1;
        break;
    }

    D_5876_tempX = D_5878_tempY = 0;

    switch (param_1 & 3)
    {
    case 0:
        D_5878_tempY--;
        break;
    case 2:
        D_5878_tempY++;
        break;
    case 1:
        D_5876_tempX++;
        break;
    case 3:
        D_5876_tempX--;
        break;
    }

    CHECK_UNINITIALIZED_16(local_4);

    return local_4;
}

// CHECKED
static void BLCKTHRN_00be(byte* param_1)
{
    int local_18_al;
    int local_16_an;
    int local_14_ap;
    int local_12_ab;
    int local_10_ad;
    int local_e_a;
    bool local_c_ac; // di
    bool local_a_ae;
    int local_8_an;
    int local_6;
    int local_4_ab;

    local_c_ac = 1;
    local_6 = 1; // a35c
    local_a_ae = 0; // -> a4ce

    while ((local_4_ab = *param_1++) != 0)
    {
        if (local_4_ab < 0x10)
        {
            // a4ec
            switch (local_4_ab)
            {
            case 1:
                // a360
                local_a_ae = 1;
                break;

            case 2:
                // a368
                local_6 = *param_1;
                param_1++;
                break;

            case 3:
                // a378
                local_c_ac = 1;
                break;

            case 4:
                // a37e
                local_c_ac = 0;
                break;

            case 5:
                // a384
                ULTIMA_3ae6_WaitFrames(*param_1++);
                break;

            case 6:
                // a396
                local_e_a = *param_1++;
                local_12_ab = *param_1++;
                local_16_an = *param_1++;
                GetCombatMap(local_12_ab, local_16_an) = local_e_a;
                //param_1 += 4;
                break;

            case 7:
                // a3cc
                ULTIMA_5910_UpdateFrame();
                break;

            case 8:
                // a3d2
                BLCKTHRN_0000(local_6);
                local_6 = 1;
                break;

            case 9:
                // a3e0
                local_e_a = *param_1++;
                D_5c5a_actors[local_e_a].tile = D_5c5a_actors[local_e_a].animTile = 0;
                break;
            }

            continue;
        }

        // a41c
        local_e_a = BLCKTHRN_002e(local_4_ab);
        local_12_ab = D_5876_tempX;
        local_16_an = D_5878_tempY;
        if (local_a_ae)
        {
            local_10_ad = BLCKTHRN_002e(*param_1++);
            local_14_ap = D_5876_tempX;
            local_18_al = D_5878_tempY;
        }

        // a454 ->
        for (local_8_an = 0; local_8_an < local_6; local_8_an++)
        {
            // a495
            D_5c5a_actors[local_e_a].x += local_12_ab;
            D_5c5a_actors[local_e_a].y += local_16_an;
            if (local_a_ae)
            {
                D_5c5a_actors[local_10_ad].x += local_14_ap;
                D_5c5a_actors[local_10_ad].y += local_18_al;
            }

            if (local_c_ac)
            {
                BLCKTHRN_0000(1);
            }
        }

        local_6 = 1; // a4c9
        local_a_ae = 0; // a4ce
    }
}

static void BLCKTHRN_0278(int param_1, int param_2)
{
    switch (param_1)
    {
    case 0:
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x0]);
        ULTIMA_1850_PrintString(D_1f4e_virtueNames[param_2]);
        ULTIMA_1850_PrintString(_TEXT(0x6f6e, "?\""));
        break;
    case 1:
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x2d]);
        ULTIMA_1850_PrintString(D_1f4e_virtueNames[param_2]);
        ULTIMA_1850_PrintString(_TEXT(0x6f72, "?\""));
        break;
    case 2:
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x52]);
        ULTIMA_1850_PrintString(D_1f4e_virtueNames[param_2]);
        ULTIMA_1850_PrintString(_TEXT(0x6f76, "?\""));
        break;
    case 3:
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0xac]);
        break;
    }
}

static int BLCKTHRN_02ea(int param_1)
{
    int local_1c;
    int local_1a;
    int local_18;
    int local_16;
    char local_14_aa[18];

    ULTIMA_1850_PrintString(_TEXT(0x6f7a, "\n\nYour response?\n:"));
    ULTIMA_3b1c_GetString(local_14_aa, 0xe);
    ULTIMA_1850_PrintString(_TEXT(0x6f8e, "\n\n"));
    local_1c = ULTIMA_216c_strlen(D_1f5e_mantras[param_1]);
    local_16 = 0;

    for (local_18 = 0; local_18 <= ULTIMA_216c_strlen(local_14_aa) - local_1c; local_18++)
    {
        local_16 = 1;

        for (local_1a = 0; local_1a < local_1c; local_1a++)
        {
            if (ULTIMA_2032_ToUpper((byte)local_14_aa[local_18 + local_1a]) != ULTIMA_2032_ToUpper((byte)D_1f5e_mantras[param_1][local_1a]))
            {
                local_16 = 0;
                break;
            }
        }

        if (local_16 != 0)
        {
            break;
        }
    }

    return local_16;
}

// CHECKED
static void BLCKTHRN_03ae(int param_1)
{
    int local_28;
    int local_26;
    int local_24_aa;
    S_55a8 local_22_ab;

    if (param_1 != 0)
    {
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0xf1]);
    }
    else
    {
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x12d]);
    }

    ULTIMA_3ae6_WaitFrames(10);

    for (local_24_aa = 2000; local_24_aa < 25000; local_24_aa += 0x32)
    {
        ULTIMA_2192_AudioPulse(0xa50, 1, 200, local_24_aa, 0);
    }

    for (local_24_aa = 25000; 2000 < local_24_aa; local_24_aa -= 0x32)
    {
        ULTIMA_2192_AudioPulse(0xa50, 1, 200, local_24_aa, 0);
    }

    ULTIMA_3522_HitEffectAt(D_5c5a_actors[1].x, D_5c5a_actors[1].y);

    D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = 0;
    GetCombatMap(5, 7) = 0x80;

    local_26 = 0;
    for (local_28 = 0; local_28 < D_585b_partyCount; local_28++)
    {
        if (D_55a8_party[local_28].status != STATUS_DEAD)
        {
            local_26++;
        }
        if (local_26 == 2)
            break;
    }

    // a6fc
    local_22_ab = D_55a8_party[local_28];

    // NOT MATCHING
    for (local_24_aa = local_28; local_24_aa < 0xf; local_24_aa++)
    {
        // a73b
        D_55a8_party[local_24_aa] = D_55a8_party[local_24_aa + 1];
    }

    // a752
    D_55a8_party[0xf] = local_22_ab;
    D_55a8_party[0xf].innMapId = 0x7f;

    D_585b_partyCount--;
    ULTIMA_2900_UpdateVitalsDisplay();
    if (param_1 != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x6f92, "\n\n"));
        ULTIMA_1850_PrintString(D_55a8_party[0xf].name);
        ULTIMA_1850_PrintString(_TEXT(0x6f96, " is sliced in half! "));
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x1a0]);
    }

    ULTIMA_16ba_PrintChar('\n');
}

static void BLCKTHRN_0510(void)
{
    ULTIMA_266c_GetChar();
    BLCKTHRN_00be(D_369e);
}

static void BLCKTHRN_051c(void)
{
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x229]);
    BLCKTHRN_00be(D_36da);
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x25f]);
    ULTIMA_1850_PrintString(D_55a8_party[1].name);
    ULTIMA_1850_PrintString(_TEXT(0x6fac, " die!\" "));
    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString(_TEXT(0x6fb4, "\n\n"));
}

static void BLCKTHRN_054a(int param_1, int param_2)
{
    bool local_6_an;
    int local_4;

    local_6_an = 0;
    ULTIMA_1850_PrintString(_TEXT(0x6fb8, "\n\n"));

    for (local_4 = 0; local_4 < 4; local_4++)
    {
        BLCKTHRN_0278(local_4, param_1);
        if (BLCKTHRN_02ea(param_1) != 0)
        {
            *(D_58d8_shrineDestroyed + param_1) = 0xff;
            ULTIMA_3f36_DecreaseByte(&D_5888_karma, 5);
            if (param_2 > 1)
            {
                BLCKTHRN_03ae(0);
                BLCKTHRN_0510();
            }
            else
            {
                ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x29c]);
                BLCKTHRN_0510();
            }
            break;
        }

        if (param_2 < 2)
        {
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x2dc]);
            BLCKTHRN_0510();
            break;
        }

        if (local_6_an)
        {
            ULTIMA_4f7c_AdvanceTime(2);
            ULTIMA_2900_UpdateVitalsDisplay();

            switch (local_4)
            {
            case 0:
                GetCombatMap(5, 9) = 0xea;
                break;
            case 1:
                GetCombatMap(5, 9) = 0xeb;
                break;
            case 2:
                GetCombatMap(5, 9) = 0xe8;
                break;
            case 3:
                BLCKTHRN_03ae(1);
                break;
            }
        }
        else
        {
            local_6_an = 1;
            BLCKTHRN_051c();
        }
    }
}

void BLCKTHRN_060e_Capture(void)
{
    int local_10_a;
    int local_e_ae;
    int local_c_h;
    int local_a_i;
    int local_8;

    int local_6_b;
    ActorFmt* local_4_c;

    local_a_i = 0;
    for (local_8 = 0; local_8 != D_585b_partyCount; local_8++)
    {
        if (D_55a8_party[local_8].status != STATUS_DEAD)
        {
            local_a_i = local_a_i + 1;
        }
    }

    D_587c_partyTile = TILE_ACTOR_AVATAR;
    ULTIMA_5910_UpdateFrame();
    ULTIMA_1850_PrintString(_TEXT(0x6fbc, "\nThou art subdued and blindfolded!"));

    for (local_c_h = 0; local_c_h < 8; local_c_h++)
    {
        if (D_58d8_shrineDestroyed[local_c_h] == 0)
            break;
    }

    if (local_c_h < 8)
    {
        local_10_a = 0;

        ULTIMA_3ae6_WaitFrames(2);
        ULTIMA_0a70_GRAP_2d_SetPenColor(0);
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
        D_5895_mapLevel = 0xff;
        ULTIMA_4a84_DrawSunAndMoons();
        ULTIMA_2e96_SetWindDirection(-1);

        for (local_8 = 0; local_8 < 5; local_8++)
        {
            ULTIMA_20fa_WaitTicks(5);
            ULTIMA_433e_AudioFootstep();
        }

        ULTIMA_1850_PrintString(_TEXT(0x6fe0, "\n\nStrong guards drag thee away!"));

        for (local_8 = 0; local_8 < (D_a9ce_soundOn != 0 ? 0x12 : 3); local_8++)
        {
            ULTIMA_20fa_WaitTicks(5);
            ULTIMA_433e_AudioFootstep();
        }

        for (local_8 = 0; local_8 < 0x20; local_8++)
        {
            ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, local_8);
        }

        D_5893_mapId = 0xff;
        D_589e_curCombatant = 0xff;

        ULTIMA_256e_ReadFileFromDisk(_TEXT(0x7000, "MISCMAPS.DAT"), D_ac64_viewActors, 0xb0, 0);
        ULTIMA_256e_ReadFileFromDisk(_TEXT(0x700e, "MISCMSG.DAT"), D_b21e_scratch, 1000, 0);

        for (local_8 = 0; local_8 < 0xb; local_8++)
        {
            for (local_e_ae = 0; local_e_ae < 0xb; local_e_ae++)
            {
                GetCombatMap(local_e_ae, local_8) = D_ac64_viewActors[local_8 * 0x10 + local_e_ae];
            }
        }

        // a9ef
        for (local_8 = 0; local_8 < local_a_i; local_8++)
        {
            local_4_c = &D_5c5a_actors[local_8];
            local_4_c->x = D_1f42[D_1f12[(local_a_i - 1) * 8 + local_8]];
            local_4_c->y = D_1f48[D_1f12[(local_a_i - 1) * 8 + local_8]];

            local_6_b = ULTIMA_4d76_FindIndex(_TEXT(0x701a, CLASS_LIST), D_55a8_party[local_8].cls);
            local_4_c->tile = local_4_c->animTile = D_1ade[local_6_b];
        }

        // aa61
        ULTIMA_3ae6_WaitFrames(0x10);
        ULTIMA_1850_PrintString(_TEXT(0x7024, "\n\nThou hast been chained and manacled!"));
        ULTIMA_3ae6_WaitFrames(0x32);
        ULTIMA_1850_PrintString(_TEXT(0x704c, "\n\nFootsteps!"));
        BLCKTHRN_0000(8);
        ULTIMA_3a74_SetActor(TILE_ACTOR_GUARD, TILE_ACTOR_GUARD, 4, 10, 0, 0, 6);
        ULTIMA_3a74_SetActor(TILE_ACTOR_GUARD, TILE_ACTOR_GUARD, 6, 10, 0, 0, 7);
        BLCKTHRN_00be(D_3702);
        ULTIMA_2192_AudioPulse(0xaf0, 1, 13000, 100, 5);
        ULTIMA_3a74_SetActor(TILE_ACTOR_CIRCLE, TILE_ACTOR_CIRCLE, 5, 5, 0, 0, 8);
        ULTIMA_1068_RevealTile(0x178, 5, 5);
        ULTIMA_3a74_SetActor(TILE_ACTOR_BLACKTHORN, TILE_ACTOR_BLACKTHORN, 5, 5, 0, 0, 8);
        ULTIMA_3ae6_WaitFrames(8);
        ULTIMA_1850_PrintString(_TEXT(0x705a, "\n\nBlackthorn says:\n\n\"Ah, "));
        ULTIMA_1850_PrintString(D_55a8_party[0].name);
        ULTIMA_1850_PrintString(_TEXT(0x7074, "!\n'Tis indeed an honour to meet thee at last! "));
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString(_TEXT(0x70a4, "\n\nGUARD! Release this good"));
        if (D_55a8_party[0].gender == 12)
        {
            ULTIMA_1850_PrintString(_TEXT(0x70c0, " lady "));
        }
        else if (D_55a8_party[0].gender == 11)
        {
            ULTIMA_1850_PrintString(_TEXT(0x70c8, "man "));
        }
        ULTIMA_1850_PrintString(_TEXT(0x70ce, "at once!\""));
        BLCKTHRN_00be(D_370e);
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x32c]);
        ULTIMA_266c_GetChar();
        BLCKTHRN_054a(local_c_h, local_a_i);
        if (D_5c5a_actors[8].tile != 0)
        {
            BLCKTHRN_00be(D_3716);
        }
    }

    D_5895_mapLevel = 0xff;
    ULTIMA_4a84_DrawSunAndMoons();
    ULTIMA_2e96_SetWindDirection(-1);
    D_5896_mapX = 10;
    D_5897_mapY = 7;
    D_57ac_keys = 0;
    D_5893_mapId = 0x12;
}

// CHECKED
void BLCKTHRN_0910_Death(void)
{
    int local_c;
    int local_6;
    uint local_8;
    int local_a;
    int local_4_ab;

#if defined(ENABLE_BGM)
    AUDIO_StopBgm();
#endif

    ULTIMA_251e_SwitchDisks(1);

    while (ULTIMA_1674_TestOpenFile(_TEXT(0x70d8, "BRIT.DAT")) == 0) {}

    CAST2_08ea(D_58a6_lightSpellTurns);
    D_587c_partyTile = TILE_ACTOR_SLEEP;

    if (D_5893_mapId < 0x21)
    {
        ULTIMA_5910_UpdateFrame();
    }

    ULTIMA_20fa_WaitTicks(10);
    local_c = D_5893_mapId;
    D_5893_mapId = 0xff;
    D_589e_curCombatant = 0xff;
    ULTIMA_1850_PrintString(_TEXT(0x70e2, "\nAn unending darkness engulfs thee..."));
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_0f46_GRAP_66_Reveal(8, 8, 0xb7, 0xb7);

    for (local_8 = 0; local_8 < 0xb; local_8++)
    {
        for (local_a = 0; local_a < 0xb; local_a++)
        {
            GetCombatMap(local_a, local_8) = 0xff;
        }
    }

    for (local_8 = 0; local_8 < 0x20; local_8++)
    {
        ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, local_8);
    }

    ULTIMA_5910_UpdateFrame();

    ULTIMA_1850_PrintString(_TEXT(0x7108, "\n\nThou hast found refuge."));
    ULTIMA_20fa_WaitTicks(0xe);
    ULTIMA_1850_PrintString(_TEXT(0x7122, "\n\nNo evil lives here, only peace and darkness."));
    ULTIMA_20fa_WaitTicks(0x1c);
    ULTIMA_1850_PrintString(_TEXT(0x7152, "\n\nBut thy slumber is disturbed!"));

    GetCombatMap(5, 5) = 0; // 0xadb9

    D_5c5a_actors[0].tile = D_5c5a_actors[0].animTile = TILE_ACTOR_AVATAR;
    D_5c5a_actors[0].x = D_5c5a_actors[0].y = 5;

    ULTIMA_5910_UpdateFrame();

    // ac9d: NOT MATCHING
    for (local_8 = 0; local_8 < 6; local_8++)
    {
        ULTIMA_2192_AudioPulse(D_3720[local_8], 1, D_372c[local_8], D_3738[local_8], D_3744[local_8]);
    }

    ULTIMA_1850_PrintString(_TEXT(0x7172, "\n\nSomeone shouts\n\n\"FORTIS FORTUNA\nAVENTARI\""));
    ULTIMA_20fa_WaitTicks(6);

    D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = TILE_ACTOR_CIRCLE;
    D_5c5a_actors[1].x = 2;
    D_5c5a_actors[1].y = 7;
    GetCombatMap(2, 7) = 0;
    ULTIMA_1068_RevealTile(0x5e, 2, 7);
    GetCombatMap(2, 7) = TILE_MAP_5E;
    D_5c5a_actors[1].tile = 0;
    ULTIMA_5910_UpdateFrame();

    ULTIMA_20fa_WaitTicks(4);
    D_5c5a_actors[1].tile = TILE_ACTOR_CIRCLE;
    D_5c5a_actors[1].x = 8;
    GetCombatMap(8, 7) = 0;
    ULTIMA_1068_RevealTile(0x5f, 8, 7);
    GetCombatMap(8, 7) = TILE_MAP_5F;
    D_5c5a_actors[1].tile = 0;
    ULTIMA_5910_UpdateFrame();

    ULTIMA_20fa_WaitTicks(4);
    ULTIMA_1850_PrintString(_TEXT(0x719e, "\n\nThere is a peal of thunder!\n"));
    ULTIMA_3072_ShakeScreen();
    ULTIMA_3072_ShakeScreen();
    GetCombatMap(5, 2) = 0;
    D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = TILE_ACTOR_CIRCLE;
    D_5c5a_actors[1].x = 5;
    D_5c5a_actors[1].y = 2;
    ULTIMA_1068_RevealTile(0x174, 5, 2);
    D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = TILE_ACTOR_74;
    ULTIMA_5910_UpdateFrame();

    // ad93
    local_4_ab = D_5888_karma / 0x14;
    ULTIMA_1850_PrintString(_TEXT(0x71be, "\n\""));
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0x71c2, "KARMA.DAT"), D_b21e_scratch, 2000, D_1a74[local_4_ab]);
    ULTIMA_1850_PrintString((char*)D_b21e_scratch);
    ULTIMA_16ba_PrintChar('\"');
    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString(_TEXT(0x71cc, "\n\nStrange words are intoned."));
    ULTIMA_20fa_WaitTicks(4);

    for (local_8 = 0; local_8 < D_585b_partyCount; local_8++)
    {
        local_6 = (u32)36400 / (local_8 + 7);
        ULTIMA_2192_AudioPulse(local_6, 1, 30000, 2000, 2);
        CAST2_05e0_Resurrect(local_8, 0xff);
        D_55a8_party[local_8].hp = D_55a8_party[local_8].maxHp;
        ULTIMA_2900_UpdateVitalsDisplay();
    }

    ULTIMA_1850_PrintString(_TEXT(0x71ea, "\n\nVertigo...\n"));
    ULTIMA_20fa_WaitTicks(4);
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_10e0_GRAP_51_PutTile(0x11c, 5, 5);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_0f46_GRAP_66_Reveal(8, 8, 0xb7, 0xb7);

    if (D_5888_karma < 0x4b)
    {
        D_5888_karma = 0x4b;
    }

    D_5893_mapId = 0x11;
    D_5895_mapLevel = 1;
    D_587c_partyTile = TILE_ACTOR_AVATAR;

    D_5896_mapX = D_5897_mapY = 10;
    D_587a_statusIcon = D_588e_protectTurns = 0;

    while (D_587f_hour != 0x6)
    {
        ULTIMA_4f7c_AdvanceTime(9);
    }

    D_58a7_torchTurns = D_58a6_lightSpellTurns = 0;

    if (D_57a8_food == 0)
    {
        D_57a8_food = 0x3f;
    }

    if (local_c != 0 && local_c < 0x21)
    {
        TOWN_11f0_Entry(1);
    }
}
