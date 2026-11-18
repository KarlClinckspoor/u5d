#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "audio/aud_mus.h"

#include "outsubs.h"

u8 D_3866[8] = {0x4F, 0x45, 0xA4, 0x17, 0x19, 0x63, 0xFE, 0x88};

u8 D_386e[8] = {0x4E, 0x58, 0xE2, 0x04, 0x2C, 0xC5, 0x00, 0xDE};

u8 D_3876[0x100] = {
    0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0xFF, 0x0C,
    0xFF, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0xFF,
    0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22,
    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0xFF,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41,
    0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51,
    0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61,
    0x62, 0x63, 0x64, 0xFF, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0xFF,
    0xFF, 0xFF, 0x70, 0x71, 0xFF, 0x72, 0x73, 0x74,
    0xFF, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xFF,
    0xFF, 0xFF, 0x7B, 0x7C, 0xFF, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0xFF,
    0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x88, 0x89, 0xFF,
    0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0xFF,
    0x91, 0xFF, 0xFF, 0x92, 0x93, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x9A, 0x9B,
    0xFF, 0x9C, 0x9D, 0xFF, 0x9E, 0x9F, 0xA0, 0xA1,
    0xFF, 0xFF, 0xA2, 0xA3, 0xFF, 0xA4, 0xA5, 0xA6,
    0xFF, 0xA7, 0xA8, 0xFF, 0xA9, 0xAA, 0xAB, 0xAC,
    0xFF, 0xFF, 0xAD, 0xAE, 0xFF, 0xAF, 0xB0, 0xB1,
    0xFF, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8,
    0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0,
    0xFF, 0xFF, 0xFF, 0xC1, 0xC2, 0xFF, 0xC3, 0xC4,
    0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC
};

static int OUTSUBS_0000(uint param_1)
{
    int local2_4;
    int local1_6 = 1;

    param_1 = (byte)(param_1 >> 8);

    for (local2_4 = 0; local2_4 < 8; local2_4++)
    {
        if (D_3866[local2_4] == param_1)
        {
            local1_6 = D_58d0_dungeonOpen[local2_4] == 0;
            break;
        }
    }

    return local1_6;
}

static int OUTSUBS_004a(int param_1)
{
    int local2_4;
    int local1_6 = 0;

    param_1 >>= 8;

    for (local2_4 = 0; local2_4 < 8; local2_4++)
    {
        // 0063
        if (D_386e[local2_4] == param_1)
        {
            local1_6 = 0x7f < D_58d8_shrineDestroyed[local2_4];
            break;
        }
    }

    return local1_6;
}

static void OUTSUBS_0098(char* param_1/*file_name*/, int param_2/*layer?*/, int param_3)
{
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    if (*param_1 == 'B') // "BRIT.DAT"
    {
        local_4_ab = D_3876[(byte)(param_3 >> 8)];
        if (local_4_ab == 0xff)
        {
            memset(&D_6608[param_2 * 0x100], 1, 0x100);
            return; // -> 01ac
        }

        // 00d4
        local_4_ab <<= 8;
    }
    else
    {
        // 00dc
        local_4_ab = param_3;
    }

    // 00e2
    if (D_5895_mapLevel != 0)
    {
        ULTIMA_251e_SwitchDisks(5);
    }

    // 00f0
    ULTIMA_256e_ReadFileFromDisk(param_1, D_6608 + param_2 * 0x100, 0x100, local_4_ab);
    local_6 = param_3 >> 4 & 0xf0;
    local_8 = param_3 >> 8 & 0xf0;

    for (local_c = 0; local_c < 0x10; local_c++)
    {
        // 0125
        for (local_a = 0; local_a < 0x10; local_a++)
        {
            switch (GetMapOverworld(local_a, local_c, param_2))
            {
            case 0x16:
            case 0x17:
            case 0x18:
                // 012c
                if (OUTSUBS_0000(param_3) != 0)
                {
                    *ULTIMA_4402_GetTileAddr(local_a + local_6, local_c + local_8) = TILE_MAP_DF;
                }
                break;
            case 0x19:
                // 017d
                if (OUTSUBS_004a(param_3) != 0)
                {
                    *ULTIMA_4402_GetTileAddr(local_a + local_6, local_c + local_8) = TILE_MAP_RUINS;
                }
                break;
            }
        }
    }
}

void OUTSUBS_01b4(int param_1, int param_2)
{
    int local_c;
    int local_a;
    byte* local_8;
    int local_6;
    char* local_4_ab;

    local_a = D_589b_mapOriginX * 0x10 + D_589c_mapOriginY * 0x100;

    if (D_589b_mapOriginX == 0xf0)
    {
        local_6 = -0xf00; // 0xf100?
    }
    else
    {
        // 01dc
        local_6 = 0x100;
    }

    // 01e1
    if (D_5895_mapLevel > 0x7f)
    {
        local_4_ab = /*0x3976*/ "UNDER.DAT";
    }
    else
    {
        local_4_ab = /*0x3980*/ "BRIT.DAT";
    }

    // 01f5
    if (param_1 == -1 || param_2 == -1)
    {
        OUTSUBS_0098(local_4_ab, 0, local_a);
    }
    // 020d
    if (param_1 == 1 || param_2 == -1)
    {
        OUTSUBS_0098(local_4_ab, 1, local_a + local_6);
    }
    // 022a
    if (param_1 == -1 || param_2 == 1)
    {
        OUTSUBS_0098(local_4_ab, 2, local_a + 0x1000);
    }
    // 0247
    if (param_1 == 1 || param_2 == 1)
    {
        OUTSUBS_0098(local_4_ab, 3, (local_a + local_6) + 0x1000);
    }

    // 0267
    local_8 = memchr(D_6608, 0x1b, 0x400);
    if (local_8 == 0)
    {
        D_217e_beaconX = D_2180_beaconY = -1;
    }
    else
    {
        // 028c
        local_c = (int)(local_8 - D_6608);
        D_217e_beaconX = local_c & 0xf;
        D_2180_beaconY = ((byte)local_c & 0xf0) >> 4;
        if ((local_c & 0x100) != 0)
        {
            D_217e_beaconX += 0x10;
        }
        if ((local_c & 0x200) != 0)
        {
            D_2180_beaconY += 0x10;
        }
    }
}

// Load chunk?
void OUTSUBS_02c8(int param_1, int param_2)
{
    int local_a;
    int local_8;

    int local_6;
    int local_4_ab;

    local_4_ab = param_1 < 0 || param_2 < 0 ? 3 : 0;
    local_8 = param_1 < 0 || param_2 > 0 ? 2 : 1;

    local_6 = local_4_ab + param_2 + param_1 * 2;
    local_a = local_8 + param_2 + param_1 * 2;

    memcpy(local_4_ab * 0x100 + D_6608, local_8 * 0x100 + D_6608, 0x100);
    memcpy(local_6 * 0x100 + D_6608, local_a * 0x100 + D_6608, 0x100);
}

char* OUTSUBS_0368_GetWorldSavefile(void)
{
    char* local_4;
    if (D_5895_mapLevel == 0)
    {
        local_4 = /*0x3989*/ "BRIT.OOL";
    }
    else
    {
        local_4 = /*0x3992*/ "UNDER.OOL";
    }

    return local_4;
}

// enter town
int OUTSUBS_0388_EnterTown(char* param_1)
{
    int local_6_an;
    int local_4;

    local_4 = 1;
    ULTIMA_1850_PrintString(param_1);

    for (local_6_an = 0; local_6_an < 0x20; local_6_an++)
    {
        if (D_1e8a_placeX[local_6_an] == D_5896_mapX && (D_1eb2_placeY[local_6_an] == D_5897_mapY))
            break;
    }

    if (local_6_an < 0x20)
    {
        if (local_6_an < 0xd || local_6_an > 0x11)
        {
            ULTIMA_1850_PrintString(_TEXT(0x399c, "\n\n"));
            ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
            ULTIMA_1850_PrintString(D_1e3a_mapNames[local_6_an]);
            ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
        }

        ULTIMA_16ba_PrintChar('\n');
        if (D_a9bd_curDisk[0] != 1)
        {
            ULTIMA_251e_SwitchDisks(1);

            while (ULTIMA_1674_TestOpenFile(/*0x399f*/ "BRIT.DAT") == 0)
                ;
        }

        ULTIMA_25d8_WriteFileToDisk(OUTSUBS_0368_GetWorldSavefile(), D_5c5a_actors, 0x100);
        D_5893_mapId = local_6_an + 1;
        D_5895_mapLevel = 0;
        D_5896_mapX = 0xf;
        D_5897_mapY = 0x1e;
        local_4 = 0;
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x39a8, "\nWhat town?\n"));
    }

    return local_4;
}

void MAINOUT_0000(void);
void MAINOUT_0354(int param_1, int param_2);

u8 D_3a06[4] = { 0xC0, 0x82, 0xB0 };

u8 D_3a0a[4] = { 0x50, 0x41, 0xB8 };

u8 D_3a0e[3] = { 0xF0, 0xF1, 0xF2 };

// f-a-l-l-s
void OUTSUBS_0458_Falls(void)
{
    int local_6_an;
    int local_4;

    ULTIMA_1850_PrintString(_TEXT(0x39b5, "F-A-L-L-S!!!\n"));
    MAINOUT_0354(0, 1);
    ULTIMA_3ae6_WaitFrames(1);
    MAINOUT_0354(0, 1);
    ULTIMA_43ae_AudioSweepTone(0x9c4, 800, 1, 300);

    local_6_an = D_587c_partyTile;
    D_587c_partyTile = 0;

    ULTIMA_3ae6_WaitFrames(1);

    for (local_4 = 0; local_4 < D_585b_partyCount; local_4++)
    {
        if (D_55a8_party[local_4].status != STATUS_DEAD && D_55a8_party[local_4].dex <= ULTIMA_3abe_Random1To30())
        {
            ULTIMA_2a52_DamagePlayer(local_4, 1);
        }
    }

    ULTIMA_3ae6_WaitFrames(2);
    D_587c_partyTile = local_6_an;

    if (D_5896_mapX == 0x36 && D_5897_mapY == 0x8a)
    {
        ULTIMA_1850_PrintString(_TEXT(0x39c3, "Falling into underworld!!\n"));

#if defined(ENABLE_BGM)
        AUDIO_StopBgm();
#endif

        D_5895_mapLevel = 0xff;

        ULTIMA_25d8_WriteFileToDisk(/*0x39de*/ "BRIT.OOL", D_5c5a_actors, 0x100);
        ULTIMA_256e_ReadFileFromDisk(/*0x39e7*/ "UNDER.OOL", D_5c5a_actors, 0x100, 0);
        ULTIMA_251e_SwitchDisks(5);

        while (ULTIMA_1674_TestOpenFile(/*0x39f1*/ "UNDER.DAT") == 0)
            ;

        ULTIMA_25d8_WriteFileToDisk(/*0x39fb*/ "UNDER.OOL", D_5c5a_actors, 0x100);
        MAINOUT_0000(); // THUNK 7b7e

#if defined(ENABLE_BGM)
        AUDIO_PlayBgmPerMap();
#endif
    }
}

void OUTSUBS_0566(void)
{
    int local1_6;
    ActorFmt* local2_4;

    if (D_5895_mapLevel != 0)
    {
        if (D_57b3_amulet == 0)
        {
            // D_5c5a_actors[28] == 5d3a
            local2_4 = &D_5c5a_actors[28];
            local2_4->tile = local2_4->animTile = TILE_ACTOR_AMULET_LB;
            local2_4->x = 0x69;
            local2_4->y = 0xe1;
            local2_4->z = 0xff;
            local2_4->value = 0xf3;
        }

        // 059d
        for (local1_6 = 0; local1_6 < 3; local1_6++)
        {
            if (D_57b6_shards[local1_6] == 0 && D_58c8_shadowlordTowns[local1_6] < 0x80)
            {
                // D_5c5a_actors[29] == 5d42
                local2_4 = &D_5c5a_actors[local1_6 + 29];
                local2_4->tile = local2_4->animTile = TILE_ACTOR_SHARD;
                local2_4->x = D_3a06[local1_6];
                local2_4->y = D_3a0a[local1_6];
                local2_4->z = 0xff;
                local2_4->value = D_3a0e[local1_6];
            }
        }
    }
}

// step on burning tile
void OUTSUBS_05ee_StepOnBurningTile(void)
{
    ULTIMA_5910_UpdateFrame();
    ULTIMA_1850_PrintString(_TEXT(0x3a11, "Burning!\n"));
    ULTIMA_2aa8_DamageParty();
}

// step on poison tile
void OUTSUBS_05fc_StepOnPoisonTile(void)
{
    int local_4;

    for (local_4 = 0; local_4 < D_585b_partyCount; local_4++)
    {
        if (D_55a8_party[local_4].status != STATUS_DEAD && D_55a8_party[local_4].status != STATUS_POISONED)
        {
            if (D_55a8_party[local_4].dex < ULTIMA_2092_RandomRange(1, 0x1e))
            {
                D_55a8_party[local_4].status = STATUS_POISONED;
                ULTIMA_1850_PrintString(_TEXT(0x3a1b, "Poisoned!\n"));
            }
        }
    }
}
