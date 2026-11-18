#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include <stdlib.h>

#include "audio/aud_mus.h"

#include "endgame.h"
#include "font.h"
#include "tiles.h"

// CHECKED (nop)
static void ENDGAME_0000(void)
{
    byte* local_c;
    uint local_a;
    int local_8;
    byte* local_6;
    byte* local_4_ab;

    ULTIMA_251e_SwitchDisks(2);

    do
    {
        local_c = ULTIMA_0fae_LoadResourceFile(D_25ea_fileNames[0]); // PROPORT.PCS
    } while (local_c == 0);

    do
    {
        local_4_ab = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[0x11]); // TEXT.16
    } while (local_4_ab == 0);

    do
    {
        local_6 = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[0x18]); // END1.16
    } while (local_6 == 0);

    ULTIMA_0f46_GRAP_66_Reveal(0, 0, 0x13f, 199);
    ULTIMA_1b94_SelectTextWindow(0);

    local_a = 0x31;
    for (local_8 = 0; local_8 < 6; local_8++)
    {
        if (D_3df4[local_8] != local_a)
        {
            ULTIMA_0be4_FreeImage(local_6);
            local_a = (uint)D_3df4[local_8];
            do
            {
                local_6 = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[0x18 + local_a]);
            } while (local_6 == 0);
        }

        // a32d
        ULTIMA_0c22_GRAP_0f_SelectPage(1);

        D_a9be = 2;
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

        if (D_3e06[local_8] == 1)
        {
            switch (local_8)
            {
            case 0:
                // a366
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 0, 0xd8, 0, 0);
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 4, 0x98, 0x1c, 0); // a385
                break;

            case 3:
                // a454
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 5, 0xe0, 0, 0);
                ULTIMA_0d4c_GRAP_4b_PutImage(local_4_ab, 0, 0xb0, 0, 0);
                // -> a385
                break;

            default:
                // a363: NOT MATCHING nop
                ;
            }
        }

        // a38c
        ULTIMA_0d4c_GRAP_4b_PutImage(local_6, D_3dee[local_8], D_3dfa[local_8], D_3e00[local_8], 0);

        D_5146[0] = D_3da6[local_8].x;
        D_5146[1] = D_3da6[local_8].y;
        D_514c[0] = D_3db2[local_8].x;
        D_514c[1] = D_3db2[local_8].y;
        D_5150 = D_3dd6[local_8];
        D_5152 = D_3ddc[local_8];
        D_5156 = D_3de2[local_8];
        D_5158 = D_3de8[local_8];

        ULTIMA_256e_ReadFileFromDisk(_TEXT(0x81fe, "END.DAT"), D_b21e_scratch, 2000, D_3dca[local_8]);
        FONT_0000(local_c, D_b21e_scratch); // 7ce6 -> 1778:0566

        if (local_8 != 0)
        {
            // a40e
            while (ULTIMA_1d5e_PollKey() == 0)
                ;
        }

        // a415
        ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
    }

    // a43f
    ULTIMA_0be4_FreeImage(local_6);
    ULTIMA_0be4_FreeImage(local_4_ab);
    ULTIMA_0fdc_FreeBitImage(local_c);

    // a476
    while (ULTIMA_1d5e_PollKey() == 0)
        ;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

    // a48b
    do
    {
        local_6 = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[0x1c]); // ENDSC.16
    } while (local_6 == 0);

    ULTIMA_0d4c_GRAP_4b_PutImage(local_6, 0, 0x28, 0, 0);
    ULTIMA_0be4_FreeImage(local_6);
    ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    D_a9be = 0;
}

static void ENDGAME_023a(char* param_1)
{
    int notused;

    for (; *param_1 != '\0' && D_bcda < 39; D_bcda++)
    {
        D_bcb2[D_bcda] = *param_1++;
    }

    D_bcb2[D_bcda] = 0;

    if (D_bcb2[D_bcda - 1] == '\n')
    {
        ULTIMA_1850_PrintString(D_bcb2);
        D_bcda = 0;
    }
}
Pair D_3da6[6] = {0xac, 0x00, 0x00, 0x00, 0x00, 0xc4, 0xb3, 0x00, 0x00, 0xa1, 0x00, 0x00};

Pair2 D_3db2[6] = {0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x009a, 0x0140};

u16 D_3dca[6] = {0x0000, 0x01a8, 0x03bc, 0x05fa, 0x08e8, 0x0b74};

u8 D_3dd6[6] = {0x7e, 0x7e, 0x2a, 0x64, 0x52, 0x70};

u8 D_3ddc[6] = {0xc8, 0xc8, 0x94, 0xc8, 0xc8, 0xc8};

u8 D_3de2[6] = {0xac, 0x00, 0x00, 0xb3, 0x00, 0x00};

u8 D_3de8[6] = {0x42, 0x5c, 0x09, 0x26, 0x09, 0x00};

u8 D_3dee[6] = {0x00, 0x01, 0x02, 0x00, 0x01, 0x02};

u8 D_3df4[6] = {0x00, 0x00, 0x00, 0x01, 0x01, 0x01};

u8 D_3dfa[6] = {0x00, 0x40, 0x00, 0x00, 0x00, 0xa0};

u8 D_3e00[6] = {0x00, 0x00, 0x34, 0x00, 0x5c, 0x00};

u8 D_3e06[6] = {0x01, 0x00, 0x00, 0x01, 0x00, 0x00};

char* D_3e0c[19] =
{
    _STATIC_TEXT(0x8206, "One"),
    _STATIC_TEXT(0x820a, "Two"),
    _STATIC_TEXT(0x820e, "Three"),
    _STATIC_TEXT(0x8214, "Four"),
    _STATIC_TEXT(0x821a, "Five"),
    _STATIC_TEXT(0x8220, "Six"),
    _STATIC_TEXT(0x8224, "Seven"),
    _STATIC_TEXT(0x822a, "Eight"),
    _STATIC_TEXT(0x8230, "Nine"),
    _STATIC_TEXT(0x8236, "Ten"),
    _STATIC_TEXT(0x823a, "Eleven"),
    _STATIC_TEXT(0x8242, "Twelve"),
    _STATIC_TEXT(0x824a, "Thirteen"),
    _STATIC_TEXT(0x8254, "Fourteen"),
    _STATIC_TEXT(0x825e, "Fifteen"),
    _STATIC_TEXT(0x8266, "Sixteen"),
    _STATIC_TEXT(0x826e, "Seventeen"),
    _STATIC_TEXT(0x8278, "Eighteen"),
    _STATIC_TEXT(0x8282, "Nineteen")
};

char* D_3e32[8] =
{
    _STATIC_TEXT(0x828c, "Twenty"),
    _STATIC_TEXT(0x8294, "Thirty"),
    _STATIC_TEXT(0x829c, "Forty"),
    _STATIC_TEXT(0x82a2, "Fifty"),
    _STATIC_TEXT(0x82a8, "Sixty"),
    _STATIC_TEXT(0x82ae, "Seventy"),
    _STATIC_TEXT(0x82b6, "Eighty"),
    _STATIC_TEXT(0x82be, "Ninety")
};

static void ENDGAME_028c(int param_1)
{
    if (param_1 < 0x15)
    {
        ENDGAME_023a(D_3e0c[param_1 - 1]);
    }
    else
    {
        ENDGAME_023a(D_3e32[param_1 / 10 - 2]);

        if (param_1 % 10)
        {
            ENDGAME_023a(_TEXT(0x82c6, "-"));
            ENDGAME_023a(D_3e0c[(param_1 % 10) - 1]);
        }
    }
}
char* D_3e42[12] =
{
    _STATIC_TEXT(0x82c8, "First"),
    _STATIC_TEXT(0x82ce, "Second"),
    _STATIC_TEXT(0x82d6, "Third"),
    _STATIC_TEXT(0x82dc, "Fourth"),
    _STATIC_TEXT(0x82e4, "Fifth"),
    _STATIC_TEXT(0x82ea, "Sixth"),
    _STATIC_TEXT(0x82f0, "Seventh"),
    _STATIC_TEXT(0x82f8, "Eighth"),
    _STATIC_TEXT(0x8300, "Ninth"),
    _STATIC_TEXT(0x8306, "Tenth"),
    _STATIC_TEXT(0x830c, "Eleventh"),
    _STATIC_TEXT(0x8316, "Twelfth")
};

u8 D_3e5a[6] = {0x05, 0x04, 0x06, 0x03, 0x05, 0x07};

u8 D_3e60[6] = {0x05, 0x06, 0x06, 0x07, 0x07, 0x07};

static void ENDGAME_02d6(int param_1)
{
    if (param_1 < 13)
    {
        ENDGAME_023a(D_3e42[param_1 - 1]);
    }
    else if (param_1 < 20)
    {
        ENDGAME_028c(param_1);
        ENDGAME_023a(_TEXT(0x831e, "th"));
    }
    else
    {
        ENDGAME_023a(_TEXT(0x8322, "Twent"));
        if (param_1 == 20)
        {
            ENDGAME_023a(_TEXT(0x8328, "ieth"));
        }
        else
        {
            ENDGAME_023a(_TEXT(0x832e, "y-"));
            ENDGAME_023a(D_3e42[param_1 - 0x15]);
        }
    }
}

static void ENDGAME_0326(void)
{
    char local_10[8];
    int local_8;
    int local_6;
    int local_4_ab;

    ULTIMA_1bf2_SetTextPosition(0, 1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(_TEXT(0x8332, "Be it known that on\n"));
    D_bcda = 0;
    ENDGAME_023a(_TEXT(0x8348, "the "));
    ENDGAME_02d6(D_587e_day);
    ENDGAME_023a(_TEXT(0x834e, " Day of\n"));
    ENDGAME_023a(_TEXT(0x8358, "the "));
    ENDGAME_02d6(D_587d_month);
    ENDGAME_023a(_TEXT(0x835e, " Month\n"));
    ULTIMA_1850_PrintString(_TEXT(0x8366, "of the Year\n"));
    ENDGAME_028c((s16)D_5874_year / 100);
    ENDGAME_023a(_TEXT(0x8374, " Hundred\n"));
    ENDGAME_028c((s16)D_5874_year % 100);
    ENDGAME_023a(_TEXT(0x837e, "\n\n"));
    ENDGAME_023a(D_55a8_party[0].name);
    ENDGAME_023a(_TEXT(0x8382, " the Avatar\n\n"));
    ULTIMA_1850_PrintString(_TEXT(0x8390, "saved the life\n"));
    ULTIMA_1850_PrintString(_TEXT(0x83a0, "of our sovereign\n"));
    ULTIMA_1850_PrintString(_TEXT(0x83b2, "Lord British, thereby\n"));
    ULTIMA_1850_PrintString(_TEXT(0x83ca, "saving our people\n"));
    ULTIMA_1850_PrintString(_TEXT(0x83de, "and our land.\n\n"));
    ULTIMA_1c9e_SelectCharset(1);
    ULTIMA_1850_PrintString(_TEXT(0x83ee, "[E@QUE_@OF@[E@AVATAR\n"));
    ULTIMA_1850_PrintString(_TEXT(0x8404, "IS@FOREVER\n\n\n\n"));
    ULTIMA_1c9e_SelectCharset(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x8414, "Report now, thy Quest compleat in\n"));

    // a697
    local_6 = D_5874_year - 0x8b;
    local_8 = D_587d_month - 4;
    local_4_ab = D_587e_day - 5;
    if (local_4_ab < 0)
    {
        // a6ba
        local_4_ab += 0x1c;
        local_8--;
    }

    // a6c1
    if (local_8 < 0)
    {
        local_8 += 0xd;
        local_6--;
    }

    // a6ce
    if (local_6 != 0)
    {
        ENDGAME_023a(u5_itoa(local_6, local_10, 10));
        ENDGAME_023a(_TEXT(0x8438, " year"));
        if (local_6 > 1)
        {
            ENDGAME_023a(_TEXT(0x843e, "s"));
        }

        if (local_8 != 0 || local_4_ab != 0)
        {
            ENDGAME_023a(_TEXT(0x8440, ", "));
        }
    }

    if (local_8 != 0)
    {
        ENDGAME_023a(u5_itoa(local_8, local_10, 10));
        ENDGAME_023a(_TEXT(0x8444, " month"));
        if (local_8 > 1)
        {
            ENDGAME_023a(_TEXT(0x844c, "s"));
        }

        if (local_4_ab != 0)
        {
            ENDGAME_023a(_TEXT(0x844e, ", "));
        }
    }

    if (local_4_ab != 0)
    {
        ENDGAME_023a(u5_itoa(local_4_ab, local_10, 10));
        ENDGAME_023a(_TEXT(0x8452, " day"));
        if (local_4_ab > 1)
        {
            ENDGAME_023a(_TEXT(0x8458, "s"));
        }
    }

    ENDGAME_023a(_TEXT(0x845a, "\n"));
    ULTIMA_1850_PrintString(_TEXT(0x845c, "to Lord British at Origin Systems!"));

    do
    {
        // infinite loop
#if !defined(TARGET_DOS16)
        ULTIMA_20fa_WaitTicks(1);
#endif
        local_4_ab++;
    } while (1);
}

static void ENDGAME_04fe(void)
{
    ULTIMA_3ae6_WaitFrames(2);
    ULTIMA_433e_AudioFootstep();
    ULTIMA_3ae6_WaitFrames(3);
}

// CHECKED (optimization)
// actor_idx, x, y
static int ENDGAME_0510(int param_1, int param_2, int param_3)
{
    int local_6;
    ActorFmt* local_4_c;
    int local_8;
    int local_a;

    local_8 = 1;
    local_4_c = &D_5c5a_actors[param_1];

    if (local_4_c->tile == 0)
    {
        // a7c0
        local_8 = 0;
    }
    else if (local_4_c->x == param_2 && local_4_c->y == param_3) // a7c8
    {
        local_8 = 0;
    }
    else
    {
        // a7da
        local_6 = abs(local_4_c->x - param_2);
        local_a = abs(local_4_c->y - param_3);
        if (local_6 < local_a)
        {
            if (local_4_c->y > param_3)
            {
                local_4_c->y--;
            }
            else
            {
                // a80c
                local_4_c->y++;
            }
        }
        else if (local_4_c->x > param_2) // a812
        {
            local_4_c->x--;
        }
        else
        {
            // a822
            local_4_c->x++;
        }

        ENDGAME_04fe();

    }

    return local_8;
}

static void ENDGAME_05a2(int param_1)
{
    int local_8;
    int local_6;
    ActorFmt* local_4_c;
    int local_a;
    local_4_c = &D_5c5a_actors[param_1];

    if (local_4_c->tile != 0 && ULTIMA_2092_RandomRange(0, 1) != 0)
    {
        for (local_a = 0; local_a < 8; local_a++)
        {
            local_6 = local_4_c->x;
            local_8 = local_4_c->y;

            switch (ULTIMA_2092_RandomRange(0, 3))
            {
            case 0:
                local_6++;
                break;
            case 1:
                local_6--;
                break;
            case 2:
                local_8++;
                break;
            case 3:
                local_8--;
                break;
            }

            if (GetMapViewport(local_6, local_8) == TILE_MAP_44)
            {
                local_4_c->x = local_6;
                local_4_c->y = local_8;
                break;
            }
        }
    }

    ULTIMA_3ae6_WaitFrames(1);
}

// CHECKED
void ENDGAME_0648_EndgameMain(void)
{
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    ActorFmt* local_6;
    int local_4_ab;

    D_589e_curCombatant = 0xff;
    ULTIMA_2900_UpdateVitalsDisplay();
    ULTIMA_6f9e_TilesetModePlus3(1);
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0x8480, "MISCMAPS.DAT"), D_ac64_viewActors, 0xb0, 0x210);
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0x848e, "ENDMSG.DAT"), D_b21e_scratch, 1000, 0);

    for (local_c = 0; local_c < 0xb; local_c++)
    {
        for (local_e_am = 0; local_e_am < 0xb; local_e_am++)
        {
            GetCombatMap(local_e_am, local_c) = D_ac64_viewActors[local_c * 0x10 + local_e_am];
        }
    }

    // a949
    for (local_c = 0; local_c < 0x20; local_c++)
    {
        D_5c5a_actors[local_c].tile = D_5c5a_actors[local_c].animTile = 0;
    }

    // a966
    local_6 = &D_5c5a_actors[31];
    local_6->tile = local_6->animTile = TILE_ACTOR_7C;
    local_6->x = 5;
    local_6->y = 8;
    local_6->animState = 0;
    ULTIMA_3ae6_WaitFrames(0x28);

    while (ENDGAME_0510(0x1f, 5, 3) != 0)
        ;

    // a9ad
    // the party loop
    for (local_c = 0; local_c < D_585b_partyCount; local_c++)
    {
        if (D_55a8_party[local_c].status == STATUS_DEAD)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_1850_PrintString(D_55a8_party[local_c].name);
            ULTIMA_1850_PrintString(_TEXT(0x849a, " lives!\n"));
            D_55a8_party[local_c].status = STATUS_GOOD;
            D_55a8_party[local_c].hp = D_55a8_party[local_c].maxHp;
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
            ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
            ULTIMA_2192_AudioPulse(0x2260, 1, 40000, 5000, 1);
            ULTIMA_2900_UpdateVitalsDisplay();
        }

        // aa25
        local_8 = ULTIMA_4d76_FindIndex(_TEXT(0x84a4, CLASS_LIST), D_55a8_party[local_c].cls);

        // aa37
        local_6 = &D_5c5a_actors[local_c];
        
        local_6->tile = local_6->animTile = D_1ade[local_8];
        local_6->x = 5;
        local_6->y = 9;
        local_6->animState = 0;

        // aa57
        ULTIMA_3ae6_WaitFrames(1);

        // aa5e: *(D_3e5a + local_c), not D_3e5a[local_c]: only then does MSC 5.0 rotate
        // this while and hoist both bytes into SI/DI, as the original does
        while (ENDGAME_0510(local_c, *(D_3e5a + local_c), *(D_3e60 + local_c)) != 0)
            ;
    }

    // aabc
    ULTIMA_3ae6_WaitFrames(0x28);
    ULTIMA_1850_PrintString((char*)D_b21e_scratch);
    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_1850_PrintString(_TEXT(0x84ae, "!\"\n\n"));
    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x21]);

    do
    {
        local_4_ab = ULTIMA_266c_GetChar();
        switch (local_4_ab)
        {
        case 0x59:
            ULTIMA_1850_PrintString(_TEXT(0x84b4, "Yes\n\n"));
            break;
        case 0x4e:
            ULTIMA_1850_PrintString(_TEXT(0x84ba, "No\n\n"));
            break;
        }
    } while ((uint)local_4_ab != 0x59 && (uint)local_4_ab != 0x4e); // aaf8

    // ab0c
    if (local_4_ab == 0x4e)
    {
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x49]);

        do
        {
            local_4_ab = ULTIMA_266c_GetChar();
            switch (local_4_ab)
            {
            case 0x59:
                ULTIMA_1850_PrintString(_TEXT(0x84c0, "Yes\n\n"));
                break;
            case 0x4e:
                ULTIMA_1850_PrintString(_TEXT(0x84c6, "No\n\n"));
                break;
            }
        } while ((uint)local_4_ab != 0x59 && (uint)local_4_ab != 0x4e);
    }

    // ab49
    if (local_4_ab == 0x59 && D_57bf_sandalwoodBox != 0)
    {
#if defined(ENABLE_BGM)
        AUDIO_PlayBgmForEnding();
#endif

        ULTIMA_3ae6_WaitFrames(8);

        while (ENDGAME_0510(0, 5, 4) != 0)
            ;

        while (ENDGAME_0510(0, 5, 5) != 0)
            ;

        ULTIMA_3ae6_WaitFrames(4);

        local_6 = &D_5c5a_actors[6];
        
        local_6->tile = local_6->animTile = TILE_ACTOR_SANDALWOOD_BOX;
        local_6->x = 5;
        local_6->y = 4;
        local_6->animState = 0;

        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0xab]);
        ULTIMA_3ae6_WaitFrames(0x28);
        ULTIMA_1850_PrintString(_TEXT(0x84cc, "\n\nHe says:\n\n"));
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0xd3]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x128]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x167]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x1c9]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x211]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x24b]);

        D_5c5a_actors[6].animTile = D_5c5a_actors[6].tile = TILE_ACTOR_GEM;

        ULTIMA_266c_GetChar();
        ULTIMA_2192_AudioPulse(0x1450, 1, 50000, 10000, 1);

        D_5c5a_actors[6].animTile = D_5c5a_actors[6].tile = 0;
        GetCombatMap(5, 4) = 0xdc;

        for (D_5887_moongateHeight = 1; D_5887_moongateHeight < 0x10; D_5887_moongateHeight++)
        {
            ULTIMA_3ae6_WaitFrames(1);
        }

        ULTIMA_3ae6_WaitFrames(4);

        while (ENDGAME_0510(0x1f, 5, 4) != 0)
            ;

        D_5c5a_actors[31].tile = D_5c5a_actors[31].animTile = 0;

        // ac64
        ULTIMA_3ae6_WaitFrames(1);

        for (local_c = 0; local_c < D_585b_partyCount; local_c++)
        {
            while (ENDGAME_0510(local_c, 5, 4) != 0)
                ;

            D_5c5a_actors[local_c].tile = D_5c5a_actors[local_c].animTile = 0;
            ULTIMA_3ae6_WaitFrames(1);
        }

        for (D_5887_moongateHeight = 0xf; D_5887_moongateHeight > 0; D_5887_moongateHeight--)
        {
            ULTIMA_3ae6_WaitFrames(1);
        }

        ULTIMA_10e0_GRAP_51_PutTile(0x44, 5, 4);
        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_0a70_GRAP_2d_SetPenColor(0);
        ULTIMA_102e_UnloadTileset();
        ULTIMA_0aa6_GRAP_3f_FillRect(0, 0, 0x13f, 199);
        ULTIMA_0c22_GRAP_0f_SelectPage(0);
        ENDGAME_0000();
        ENDGAME_0326();
        return;
    }

    // bad endgame
    ULTIMA_1850_PrintString(_TEXT(0x84da, "\"I see...\n"));
    ULTIMA_3ae6_WaitFrames(0x28);
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x2d5]);
    D_5c5a_actors[0].y--;
    ENDGAME_04fe();

    do
    {
        local_a = ENDGAME_0510(2, 8, 6);
        local_a += ENDGAME_0510(0x1f, 4, 1);
        local_a += ENDGAME_0510(0, 8, 4);
    } while (local_a != 0);

    do
    {
        ENDGAME_05a2(1);
        ENDGAME_05a2(3);
        ENDGAME_05a2(4);
        ENDGAME_05a2(5);
    } while (1);
}
