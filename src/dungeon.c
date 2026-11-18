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

// defined in dungeonb.c, the part of DUNGEON.OVL built with MSC 5.0
void DUNGEON_1a90(int param_1);
void DUNGEON_1be0(void);

void DNGLOOK_109e(int param_1);

u8 D_2c76[6] = {0x00, 0x01, 0x02, 0x03, 0x00, 0x02};

static void DUNGEON_0000(byte param_1)
{
    int local_1e_an;
    int local_1c;
    ActorFmt local_1a_aa;

    ActorFmt local_12;
    int local_a;

    int local_8;
    int local_6;
    int local_4_ab;

    ULTIMA_1850_PrintString(_TEXT(0x2c58, "Entering room...\n"));

    D_6603_dungeonFacing = D_2c76[D_6602];

    ULTIMA_1b16_ClearKbdBuffer();

    local_1c = param_1 & 0xf;
    local_4_ab = D_587b_activePlayer;
    D_589e_curCombatant = D_587b_activePlayer = 0xff;

    local_1e_an = D_5893_mapId - 0x21;
    if (local_1e_an >= 1)
    {
        local_1e_an--;
    }

    local_a = 0x1600 * local_1e_an + local_1c * 0x160;

    memset(D_ad14_combatMap, 0, 0x160);

    ULTIMA_256e_ReadFileFromDisk(/*0x2c6a*/ "dungeon.cbt", D_ad14_combatMap, 0x160, local_a);

    D_5894_savedMapId = D_5893_mapId;
    local_6 = D_5896_mapX;
    local_8 = D_5897_mapY;

    local_12 = D_5c5a_actors[1];
    local_1a_aa = D_5c5a_actors[2];

    D_5893_mapId = 0xff;
    D_58a0_combatExitDir = 0;

    DNGLOOK_117e(param_1, 3);
    ULTIMA_5910_UpdateFrame();

    D_58a1_combatFlags = 0x82;

    if (COMBAT_0b94_MainLoop() == 0)
    {
        if (D_58a0_combatExitDir == 0x4d)
        {
            ENDGAME_0648_EndgameMain();
        }

        D_5893_mapId = D_5894_savedMapId;
        DNGLOOK_0844(local_1c);
        GetDungeonMap((uint)local_6, (uint)local_8, (uint)D_5895_mapLevel) &= 0xaf;
    }

    D_5897_mapY = local_8;
    D_5896_mapX = local_6;
    D_5893_mapId = D_5894_savedMapId;

    DNGLOOK_0fda();

    D_5c5a_actors[1] = local_12;
    D_5c5a_actors[2] = local_1a_aa;
    D_587b_activePlayer = local_4_ab;
}

static int DUNGEON_0252(void);

void DUNGEON_0134(int param_1)
{
    int local_8;
    int local_6;
    ActorFmt* local_4_ab;

    local_4_ab = &D_5c5a_actors[1];
    local_6 = D_5c5a_actors[1].value;
    if (param_1 != 0)
    {
        local_8 = ULTIMA_2092_RandomRange(0, 7);
        D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = local_8;
        D_5c5a_actors[1].extra = 0;
        D_5c5a_actors[1].animState = D_1744[local_8];
        D_5c5a_actors[1].value = D_173c[local_8];
        D_5c5a_actors[1].z = D_5895_mapLevel;
        if (local_6 != 0xff)
        {
            ULTIMA_0be4_FreeImage(D_a9c6);
        }
        D_a9c6 = 0;
        if (DUNGEON_0252() == 0)
        {
            local_4_ab->tile = local_4_ab->animTile = 0;
            local_4_ab->value = 0xff;
        }
    }

    if (local_4_ab->value != 0xff)
    {
        do
        {
            D_a9c6 = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[local_4_ab->tile + 8]);
        } while (D_a9c6 == 0);
    }
}

void DUNGEON_01d2_PrintWalkDir(void)
{
    int local_6_an;
    int local_4;

    local_4 = D_5386_textWindowIndex;
    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(0xc, 0);
    local_6_an = D_5895_mapLevel + 1;
    ULTIMA_1a3e_PrintNumber(local_6_an, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(0xc, 0x17);

    switch (D_6603_dungeonFacing)
    {
    case 0:
        ULTIMA_1850_PrintString(_TEXT(0x2c7c, "North"));
        break;
    case 1:
        ULTIMA_1850_PrintString(_TEXT(0x2c82, " East"));
        break;
    case 2:
        ULTIMA_1850_PrintString(_TEXT(0x2c88, "South"));
        break;
    case 3:
        ULTIMA_1850_PrintString(_TEXT(0x2c8e, " West"));
        break;
    default:
        ULTIMA_1850_PrintString(_TEXT(0x2c94, " ????"));
        break;
    }

    ULTIMA_1b94_SelectTextWindow(local_4);
}

static int DUNGEON_0252(void)
{
    byte local_c;
    int local_a;
    int local_8;
    byte* local_e;
    int local_6;
    int local_4_ab;

    for (local_8 = 0; local_8 < 8; local_8++)
    {
        // 0262
        local_e = &GetDungeonMap(0, 0, D_5895_mapLevel);
        local_a = ULTIMA_2092_RandomRange(0, 0x3f);
        local_e += local_a;
        local_c = *local_e & 0xf0;
        if (local_c < 0x60 || local_c == 0x70)
        {
            // 0292
            local_6 = local_a / 8;
            local_4_ab = local_a % 8;
            if (local_4_ab != D_5896_mapX && local_6 != D_5897_mapY)
            {
                D_5c5a_actors[1].x = D_5c5a_actors[2].x = local_4_ab;
                D_5c5a_actors[1].y = D_5c5a_actors[2].y = local_6;

                if ((D_5c5a_actors[1].value == 0x16 || D_5c5a_actors[1].value == 0x18) && (int)ULTIMA_2092_RandomRange(0, 99) > 0x30)
                {
                    D_5c5a_actors[1].extra = 0xff;
                }

                local_a = 1;
                break;
            }
        }
    }

    if (local_8 == 8)
    {
        D_5c5a_actors[1].x = D_5c5a_actors[2].x = D_5c5a_actors[1].y = D_5c5a_actors[2].y = 0xff;
        local_a = 0;
    }

    return local_a;
}

static void DUNGEON_0332(void)
{
    int local_4;

    local_4 = D_5386_textWindowIndex;
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x28, 0, 0x98, 7);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x30, 0xb9, 0x98, 0xbf);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(0x28, 7, 0x98, 7);
    ULTIMA_0b10_GRAP_Line(0x30, 0xb8, 0x98, 0xb8);
    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(10, 0);
    ULTIMA_4c2a_PrintLeftCap();
    ULTIMA_1850_PrintString(_TEXT(0x2c9a, "L "));
    ULTIMA_4cce_PrintRightCap();
    ULTIMA_1bf2_SetTextPosition(6, 0x17);
    ULTIMA_4c2a_PrintLeftCap();
    ULTIMA_1850_PrintString(_TEXT(0x2c9d, "Dir:      "));
    ULTIMA_4cce_PrintRightCap();
    ULTIMA_1b94_SelectTextWindow(local_4);
}

void DUNGEON_1020(void);

static int DUNGEON_03d6(void)
{
    int local_6_an;
    int local_4;

    if (D_a9fa_vitalsDirty != 0)
    {
        ULTIMA_2900_UpdateVitalsDisplay();
        D_a9fa_vitalsDirty = 0;
    }

    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_4c2a_PrintLeftCap();

    local_4 = 1;
    do
    {
        local_6_an = ULTIMA_2032_ToUpper(ULTIMA_1b38_PollKeyWithCursor());
        if (local_6_an != 0 && local_6_an < 5 && D_538a == 0)
        {
            local_6_an += 0xfa;
        }

        if (local_6_an == 0)
        {
            DUNGEON_1a90(local_4);
            if (local_4 != 0)
            {
                ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x10, 0xe, 0xaf, 0xb2);
            }
            else
            {
                ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0x28, 0xe, 0x96, 0xb2);
            }

            if (D_6604_dungeonStyle != 1)
            {
                local_4 = 0;
            }

            DUNGEON_1020();
        }
    } while (local_6_an == 0);

    return local_6_an;
}

static void DUNGEON_0470_ElectricField(int param_1)
{
    DUNGEON_1be0();
    ULTIMA_1850_PrintString(_TEXT(0x2ca8, "Ouch!\n"));
    ULTIMA_1850_PrintString(_TEXT(0x2caf, "Electric field!\n"));
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_20fa_WaitTicks(1);
    ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_223c_AudioWhiteNoise(1, 500, 20000);
    if (param_1 > 0)
    {
        D_5896_mapX -= D_24d6_facingDx[D_6603_dungeonFacing];
        D_5897_mapY -= D_24de_facingDy[D_6603_dungeonFacing];
    }
    else
    {
        D_5896_mapX += D_24d6_facingDx[D_6603_dungeonFacing];
        D_5897_mapY += D_24de_facingDy[D_6603_dungeonFacing];
    }
    ULTIMA_2aa8_DamageParty();
    DUNGEON_1be0();
}

static int DUNGEON_0502(int param_1, byte param_2)
{
    byte local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_6 = 0;
    local_4_ab = 0;

    switch (param_1)
    {
    case DIR_UP:
        // 0542
        ULTIMA_1850_PrintString(_TEXT(0x2cc0, "Advance\n"));
        local_6 = 1;
        break;

    case DIR_RIGHT:
        // 0628
        if ((param_2 & 0xf0) == 0xe0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2cc9, "Not in doorway!\n")); // -> 0666
        }
        else
        {
            // 0636
            ULTIMA_1850_PrintString(_TEXT(0x2cda, "Turn right\n"));
            D_6603_dungeonFacing = (D_6603_dungeonFacing + 1) & 3;
        }
        break;

    case DIR_DOWN:
        // 064a
        ULTIMA_1850_PrintString(_TEXT(0x2ce6, "Back up\n"));
        local_6 = -1;
        break;

    case DIR_LEFT:
        // 065a
        if ((param_2 & 0xf0) == 0xe0)
        {
            // 0666
            ULTIMA_1850_PrintString(_TEXT(0x2cef, "Not in doorway!\n"));
        }
        else
        {
            // 066e
            ULTIMA_1850_PrintString(_TEXT(0x2d00, "Turn left\n"));
            D_6603_dungeonFacing = (D_6603_dungeonFacing + 3) & 3;
        }
        break;

    default:
        // 0533
        ULTIMA_1850_PrintString(_TEXT(0x2d0b, "Turn around.\n"));
        D_6603_dungeonFacing = (D_6603_dungeonFacing + 2) & 3; // -> 0642 -> 054e
        break;
    }

    D_6602 = D_6603_dungeonFacing;

    if (local_6 != 0)
    {
        local_4_ab = 1;
        local_8 = D_24d6_facingDx[D_6603_dungeonFacing] * local_6 + D_5896_mapX;
        if (local_8 < 0)
        {
            local_8 = 7;
        }

        if (local_8 > 7)
        {
            local_8 = 0;
        }

        local_a = D_24de_facingDy[D_6603_dungeonFacing] * local_6 + D_5897_mapY;
        if (local_a < 0)
        {
            local_a = 7;
        }

        if (local_a > 7)
        {
            local_a = 0;
        }

        // 05bc
        local_c = GetDungeonMap(local_8, local_a, D_5895_mapLevel);
        if (local_c == 0x83)
        {
            D_5896_mapX = local_8;
            D_5897_mapY = local_a;
            DUNGEON_0470_ElectricField(local_6);
            local_8 = D_5896_mapX;
            local_a = D_5897_mapY;
        }

        // 05fb
        local_c &= 0xf0;
        if (local_c > 0xa0 && local_c < 0xe0 || local_6 == -1 && (local_c == 0xa0 || local_c == 0xf0))
        {
            // 061d
            ULTIMA_1850_PrintString(_TEXT(0x2d19, "Blocked!\n"));
        }
        else
        {
            // 067c~: OK
            if (local_8 == D_5c5a_actors[1].x && local_a == D_5c5a_actors[1].y)
            {
                ULTIMA_1850_PrintString(_TEXT(0x2d23, "Blocked!\n"));
            }
            else
            {
                // 0694
                D_a9fb = 0xf;
                D_24e7 = 0;
                DUNGEON_1020();
                D_24e7 = 0;
                D_5896_mapX = (byte)local_8 & 7;
                D_5897_mapY = (byte)local_a & 7;
            }
        }
    }

    // 06b6
    DUNGEON_1be0();
    return local_4_ab;
}

static int DUNGEON_06c4_ProcessCommand(int param_1)
{
    byte local_6_an;
    int local_4;

    local_4 = 1;
    switch (param_1)
    {
    case U5_KEY_CTRL_K:
        // 06f2
        ULTIMA_1a3e_PrintNumber(D_5888_karma, 1, 0x20);
        ULTIMA_16ba_PrintChar('\n');
        break;

    case U5_KEY_CTRL_E:
        // 0710
        ULTIMA_1850_PrintString(_TEXT(0x2d2d, "Exit to DOS? "));
        param_1 = ULTIMA_266c_GetChar();
        if (param_1 == 0x59)
        {
            ULTIMA_0878_RestoreVideoMode();
            exit(0);
            // nop?
            break;
        }
        ULTIMA_1850_PrintString(_TEXT(0x2d3b, "N\n"));
        break;

    case U5_KEY_CTRL_V:
        // 073a
        ULTIMA_1850_PrintString(_TEXT(0x2d3e, U5_VERSION));
        ULTIMA_16ba_PrintChar('\n');
        break;

    case U5_KEY_LEFT:
    case U5_KEY_RIGHT:
    case U5_KEY_UP:
    case U5_KEY_DOWN:
    case U5_KEY_ENTER:
    case U5_KEY_PERIOD:
        // 0744
        local_6_an = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        local_4 = DUNGEON_0502(param_1, local_6_an);
        // ?
        break;

    case U5_KEY_CTRL_S:
        // 0776
        ULTIMA_1850_PrintString(_TEXT(0x2d43, "Sound "));
        if (D_a9ce_soundOn != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2d4a, "Off\n"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x2d4f, "On\n"));
        }
        D_a9ce_soundOn = !D_a9ce_soundOn;
        break;

    default:
        if ((uint)param_1 >= 0x30 && (uint)param_1 <= 0x39)
        {
            local_4 = ULTIMA_4080_SetActivePlayer(param_1);
            local_4 = 0;
        }
        else
        {
            local_4 = ULTIMA_3178_ProcessCommand(param_1);
        }
        break;
    }

    return local_4;
}

static int DUNGEON_07e2(void)
{
    ActorFmt* local_18_al;
    byte local_16_an;
    int local_14;
    int local_12_ab;
    int local_10_ad;
    int local_e;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    ActorFmt* local_4_ab;

    local_c = 0;
    local_10_ad = 0;
    local_e = 999;
    local_4_ab = &D_5c5a_actors[1];
    local_18_al = &D_5c5a_actors[2];

    if (D_5c5a_actors[1].value != 0xff)
    {
        if (local_4_ab->value != 0x1b)
        {
            // 0819
            for (local_c = 0; local_c < 8; local_c++)
            {
                // 083e
                local_a = ULTIMA_2092_RandomRange(0, 3);
                local_12_ab = D_24d6_facingDx[local_a] + local_4_ab->x;
                if (local_12_ab > 7)
                {
                    local_12_ab = 0;
                }
                if (local_12_ab < 0)
                {
                    local_12_ab = 7;
                }

                // 0866
                local_14 = D_24de_facingDy[local_a] + local_4_ab->y;
                if (local_14 > 7)
                {
                    local_14 = 0;
                }
                if (local_14 < 0)
                {
                    local_14 = 7;
                }

                // 0882
                local_16_an = GetDungeonMap(local_12_ab, local_14, D_5895_mapLevel);
                local_16_an &= 0xf0;

                if (local_16_an != 0x60 && local_16_an != 0x80 && local_16_an < 0xa0)
                {
                    local_6 = local_18_al->x;
                    local_8 = local_18_al->y;
                    if (local_12_ab != local_6 || local_14 != local_8 || ULTIMA_2092_RandomRange(0, 7) == 1)
                    {
                        local_e = 900;
                        break;
                    }
                }
            }

            // 08d6
            if (local_e == 900)
            {
                local_18_al->x = local_4_ab->x;
                local_18_al->y = local_4_ab->y;
                local_4_ab->x = (byte)local_12_ab;
                local_4_ab->y = (byte)local_14;
            }
        }

        // 0917
        if (local_4_ab->x == D_5896_mapX && local_4_ab->y == D_5897_mapY)
        {
            local_10_ad = 1;
            local_4_ab->x = local_18_al->x;
            local_4_ab->y = local_18_al->y;
        }
    }
    return local_10_ad;
}

// sleep spell
static void DUNGEON_0948_SleepSpell(void)
{
    int local_4;

    ULTIMA_1850_PrintString(_TEXT(0x2d53, "Sleep spell!\n"));

    for (local_4 = 0; local_4 < D_585b_partyCount; local_4++)
    {
        if (D_55a8_party[local_4].dex <= ULTIMA_2092_RandomRange(1, 0x1e) && D_55a8_party[local_4].status != STATUS_DEAD)
        {
            D_55a8_party[local_4].status = STATUS_SLEEP;
            ULTIMA_2a28_InvertPlayerRow(local_4);
            ULTIMA_223c_AudioWhiteNoise(1, 0x32, 0xdac);
            D_a9fa_vitalsDirty = 1;
        }
    }

    GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 8;
}

// poison
static void DUNGEON_09e6_Poison(void)
{
    int local_4;

    ULTIMA_1850_PrintString(_TEXT(0x2d61, "Poison!\n"));

    for (local_4 = 0; local_4 < D_585b_partyCount; local_4++)
    {
        if (D_55a8_party[local_4].dex <= ULTIMA_2092_RandomRange(1, 0x1e) && D_55a8_party[local_4].status != STATUS_DEAD)
        {
            D_55a8_party[local_4].status = STATUS_POISONED;
            ULTIMA_223c_AudioWhiteNoise(1, 0x32, 0xdac);
        }
    }
}

static void DUNGEON_0a4c(byte param_1)
{
    D_6602 = 5;

    while ((param_1 == 0x61 || param_1 == 0x69) && D_5895_mapLevel < 8)
    {
        ULTIMA_1850_PrintString(_TEXT(0x2d6a, "Pit Trap!\n"));
        ULTIMA_1850_PrintString(_TEXT(0x2d75, "Falling...\n"));

        GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 0xf8;

        D_5895_mapLevel++;

        param_1 = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        if (param_1 < 0x90)
        {
            param_1 = param_1 | 8;
            GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) = param_1;
        }

        DUNGEON_1be0();
        ULTIMA_1850_PrintString(_TEXT(0x2d81, "      ...splat!\n"));
        ULTIMA_2aa8_DamageParty();
    }

    if (D_5895_mapLevel == 8)
    {
        D_5893_mapId = 0;
    }

    if (D_5893_mapId != 0)
    {
        param_1 = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        param_1 &= 0xf0;
        if (param_1 == 0xa0 || param_1 == 0xf0)
        {
            DNGLOOK_1130();
            DUNGEON_0000(GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel));
            if (D_5893_mapId == 0)
            {
                return;
            }

            DNGLOOK_109e(1);
        }

        DUNGEON_0134(1);
    }
}

// attacked
static void DUNGEON_0b7e(void)
{
    ActorFmt* local_4;

    ULTIMA_1850_PrintString(_TEXT(0x2d92, "Attacked"));

    local_4 = &D_5c5a_actors[2];

    if (((local_4->x - 1) & 7) == D_5896_mapX)
    {
        D_6602 = 1;
    }
    else if (((local_4->x + 1) & 7) == D_5896_mapX)
    {
        D_6602 = 3;
    }
    else if (((local_4->y - 1) & 7) == D_5897_mapY)
    {
        D_6602 = 2;
    }
    else
    {
        D_6602 = 0;
    }

    if (D_6602 != D_6603_dungeonFacing)
    {
        ULTIMA_1850_PrintString(_TEXT(0x2d9b, " from the "));

        switch (D_6602)
        {
        case 0:
            ULTIMA_1850_PrintString(_TEXT(0x2da6, "north"));
            break;
        case 1:
            ULTIMA_1850_PrintString(_TEXT(0x2dac, "east"));
            break;
        case 2:
            ULTIMA_1850_PrintString(_TEXT(0x2db1, "south"));
            break;
        case 3:
            ULTIMA_1850_PrintString(_TEXT(0x2db7, "west"));
            break;
        }

        D_6603_dungeonFacing = D_6602;
    }

    ULTIMA_1850_PrintString(_TEXT(0x2dbc, "!\n"));
    DUNGEON_1be0();
    ULTIMA_1b16_ClearKbdBuffer();
    ULTIMA_20fa_WaitTicks(2);
    DNGLOOK_1130();
    D_58a0_combatExitDir = 0;
    D_58a1_combatFlags = 2;
    DNGLOOK_0d3e();
    ULTIMA_5f86_SpecialMapHandler(2, D_5c5a_actors[1].value, 0);
    DNGLOOK_0fda();
    DNGLOOK_109e(1);
    DUNGEON_0134(1);
    if (D_5893_mapId != 0)
    {
        DUNGEON_1be0();
    }
}

void DUNGEON_0c76(byte param_1, int param_2)
{
    int local_a;
    byte local_8;
    int local_6;
    byte local_4_ab;

    local_a = 0;
    local_4_ab = param_1 & 0xf0;
    local_8 = 0;

    for (local_6 = 0; local_6 < D_585b_partyCount; local_6++)
    {
        if (D_55a8_party[local_6].status == STATUS_SLEEP && ULTIMA_2092_RandomRange(0, 0x3f) < 4)
        {
            D_55a8_party[local_6].status = STATUS_GOOD;
            local_8++;
        }
    }

    if (local_8 != 0)
    {
        ULTIMA_2900_UpdateVitalsDisplay();
    }

    if (param_2 != 0)
    {
        local_a = DUNGEON_07e2();
    }

    if (local_a != 0)
    {
        DUNGEON_0b7e();
        param_1 = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        local_4_ab = param_1 & 0xf0;
    }

    // 0d17
    if (local_4_ab == 0xf0 || local_4_ab == 0xa0)
    {
        // 0d23
        DNGLOOK_1130();

        // 0d26
        while ((local_4_ab == 0xf0 || local_4_ab == 0xa0))
        {
            // 0d32
            if (ULTIMA_39fc_GetFirstActivePartyMember() > -1)
            {
                DUNGEON_0000(param_1);
                if (D_5893_mapId == 0)
                {
                    break;
                }

                // 0d62
                param_1 = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
                local_4_ab = param_1 & 0xf0;
            }
            else
            {
                break;
            }
        }

        // 0d4a
        DNGLOOK_109e(1);

        if (D_5893_mapId > 0x20)
        {
            // 0d5b
            DUNGEON_1be0();
        }
        // -> 0e22
    }
    else
    {
        switch (param_1)
        {
        case 0x80:
        case 0x88:
            DUNGEON_0948_SleepSpell();
            break;

        case 0x81:
        case 0x89:
            DUNGEON_09e6_Poison();
            break;

        case 0x82:
        case 0x8a:
            ULTIMA_1850_PrintString(_TEXT(0x2dbf, "Fire!!\n"));
            ULTIMA_2aa8_DamageParty();
            break;

        case 0x62:
        case 0x6a:
            ULTIMA_1850_PrintString(_TEXT(0x2dc7, "Bomb Trap!\n"));
            ULTIMA_1850_PrintString(_TEXT(0x2dd3, "KABOOM!!\n"));
            GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel) &= 8;
            ULTIMA_2aa8_DamageParty();
            break;

        case 0x61:
        case 0x69:
            DUNGEON_0a4c(param_1);
            break;
        }

        ULTIMA_2900_UpdateVitalsDisplay();
    }

    ULTIMA_2ae8_EndTurn();
}

void DUNGEON_0e2e_MainLoop(int param_1)
{
    int local_10_ad;
    int local_e;
    int local_c;
    int local_a;
    u8 local_8;
    int local_6;
    u8 local_4_ab;

#if !defined(MATCHING_BUILD)
    local_c = 0;
#endif

    local_e = 1;
    local_10_ad = 0;
    local_6 = 0;
    D_bb17 = 2;
    DNGLOOK_093a();
    local_8 = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    local_8 &= 0xf0;
    DUNGEON_0332();
    DUNGEON_01d2_PrintWalkDir();
    local_a = D_5893_mapId - 0x20;
    if (local_a == 1 || local_a == 4 || local_a == 5)
    {
        // 0e95
        D_bb14 = 0x4f;
        D_bb15 = 0x45;
        D_6604_dungeonStyle = 3;
    }
    else
    {
        // 0ea6
        D_bb14 = 0x4d;
        D_bb15 = 5;
        if (local_a == 6 || local_a == 7)
        {
            // 0ebc
            D_6604_dungeonStyle = 2;
        }
        else
        {
            // 0ec4
            D_6604_dungeonStyle = 1;
        }
    }

    // 0ec9
    if (local_8 == 0xa0 || local_8 == 0xf0)
    {
        // 0ed5
        DNGLOOK_1130();
        DUNGEON_0000(GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel));
    }

    if (D_5893_mapId > 0x20)
    {
        DNGLOOK_109e(param_1);
        DUNGEON_0134(param_1);
        DUNGEON_1be0();
    }

#if defined(ENABLE_BGM)
    AUDIO_PlayBgmPerMap();
#endif

    // 0f15
    while (local_e != 0 && D_5893_mapId > 0x20) // 0f93
    {
        local_e = 1;
        local_c = 1;
        local_10_ad = ULTIMA_39fc_GetFirstActivePartyMember();
        if (local_10_ad == local_c)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a_PrintLeftCap();
            ULTIMA_1850_PrintString(_TEXT(0x2ddd, "Zzzzzz...\n"));
        }

        // 0fc3
        if (local_10_ad < 0)
        {
            local_e = 0;
            local_c = 0;
        }

        // 0fd0
        if (local_10_ad != 0)
            continue;

        if (D_587a_statusIcon != 'T' && D_587a_statusIcon != 'Q')
        {
            // 0fea -> 0f2e
            local_6 = 1;
            ULTIMA_4f7c_AdvanceTime(1);
        }
        else
        {
            if (D_587a_statusIcon == 'Q')
            {
                local_6 ^= 1;
                if (local_6 != 0)
                {
                    ULTIMA_4f7c_AdvanceTime(1);
                }
            }
            else
            {
                local_6 = 0;
            }
        }

        // 0f36
        local_a = DUNGEON_03d6();
        if (local_a > -1)
        {
            local_c = DUNGEON_06c4_ProcessCommand(local_a);
        }

        if (D_5893_mapId < 0x21)
        {
            local_c = 0;
            local_e = local_c;
        }

        // 0f53
        local_4_ab = GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel & 0xff);
        local_8 = local_4_ab & 0xf0;
        if (local_c != 0)
        {
            DUNGEON_0c76(local_4_ab, local_6);
        }

        if (D_5893_mapId >= 0x21)
        {
            continue;
        }

        local_c = 0;
        local_e = local_c;
    }

    // 0ff2
    if ((D_bb17 & 2) != 0)
    {
        D_bb17 &= 0xfb;
    }
    else
    {
        D_bb17 |= 4;
    }
    // 100b
    DNGLOOK_1130();

    if (local_10_ad < 0)
    {
        BLCKTHRN_0910_Death();
    }
}

void DUNGEON_1020(void)
{
    D_24e7 ^= 1;
    if (D_24e7 != 0 && D_a9fb != 0)
    {
        ULTIMA_223c_AudioWhiteNoise(1, D_a9fb, 20000);
        ULTIMA_3f36_DecreaseByte(&D_a9fb, 4);
    }
}
