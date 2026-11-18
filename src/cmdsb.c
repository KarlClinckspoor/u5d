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

// The part of CMDS.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").

// select reagents
static int CMDS_18be(void)
{
    int local_18_ai;
    int local_16_ak;
    int local_14_am;
    byte local_12[8];
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_16_ak = 0;
    for (local_8 = 0; local_8 < 8; local_8++)
    {
        if (D_5850_reagents[local_8] != 0)
        {
            local_12[local_16_ak] = local_8;
            local_16_ak++;
        }
    }

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    ULTIMA_4efc_OpenStatsDivider();
    ULTIMA_4e50_ShowStatsTitle(_TEXT(0x8f64, "Reagents:"));

    for (local_8 = 0; local_8 != local_16_ak; local_8++)
    {
        ULTIMA_16ba_PrintChar('\n');
        ULTIMA_16ba_PrintChar(' ');
        ULTIMA_1a3e_PrintNumber(D_5850_reagents[local_12[local_8]], 2, 0x30);
        ULTIMA_16ba_PrintChar(' ');
        ULTIMA_1850_PrintString(D_19d2_reagentNames[local_12[local_8]]);
    }

    // d8f2
    local_6 = local_18_ai = 0;

    ULTIMA_2a28_InvertPlayerRow(1);
    local_4_ab = 0;
    do
    {
        ULTIMA_1b94_SelectTextWindow(2);
        local_a = ULTIMA_266c_GetChar();
        switch (local_a)
        {
        case U5_KEY_LEFT:
        case U5_KEY_UP:
            // 19b2
            if (local_6 > 0)
            {
                ULTIMA_2a28_InvertPlayerRow(local_6 + 1);
                local_6--;
                ULTIMA_2a28_InvertPlayerRow(local_6 + 1);
            }
            break;

        case U5_KEY_RIGHT:
        case U5_KEY_DOWN:
            // 19d0
            if (local_6 < local_16_ak + -1)
            {
                ULTIMA_2a28_InvertPlayerRow(local_6 + 1);
                local_6++;
                ULTIMA_2a28_InvertPlayerRow(local_6 + 1);
            }
            break;

        case 0x4d:
            // 19e0
            local_4_ab = 1;
            ULTIMA_1850_PrintString(_TEXT(0x8f6e, "\n\n"));
            break;

        case U5_KEY_ENTER:
        case U5_KEY_SPACE:
            // 19ee
            local_14_am = 0x80 >> local_12[local_6];
            local_18_ai ^= local_14_am;
            ULTIMA_1b94_SelectTextWindow(1);
            ULTIMA_1bf2_SetTextPosition(3, local_6 + 1);
            ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            if ((local_14_am & local_18_ai) != 0)
            {
                ULTIMA_16ba_PrintChar(0xf);
            }
            else
            {
                ULTIMA_16ba_PrintChar(' ');
            }

            ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            break;

        case U5_KEY_ESC:
            // 1a2e
            local_18_ai = -1;
            local_4_ab = 1;

            ULTIMA_16ba_PrintChar('\n');
            break;
        }
    } while (local_4_ab == 0);

    ULTIMA_2a28_InvertPlayerRow(local_6 + 1);
    ULTIMA_1b94_SelectTextWindow(2);
    return local_18_ai;
}

// select mix amount
static int CMDS_1a70(int param_1)
{
    bool local_6;
    int local_4_ab;
    int local_8;
    int local_a;

    do
    {
        local_6 = 1;
        ULTIMA_1850_PrintString(_TEXT(0x8f72, "How much? "));
        local_4_ab = ULTIMA_3b9e_GetNumber(2);
        if (local_4_ab != 0)
        {
            local_a = 0x80;
            for (local_8 = 0; local_8 < 8; local_8++)
            {
                if ((param_1 & local_a) != 0 && D_5850_reagents[local_8] < local_4_ab)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x8f7e, "Insufficient reagents!\n\n"));
                    local_6 = 0;
                    break;
                }
                local_a >>= 1;
            }
        }
    } while (!local_6);

    return local_4_ab;
}

void CMDS_1ad8_MixCmd(void)
{
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_8 = 0;
    for (local_a = 0; local_a < 8; local_a++)
    {
        local_8 += D_5850_reagents[local_a];
    }

    if (local_8 == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8f98, "No reagents owned!\n"));
        return;
    }

    ULTIMA_1850_PrintString(_TEXT(0x8fac, "For what spell?\n:"));
    local_6 = CAST2_00de_SelectSpell();
    if (local_6 == -1)
    {
        ULTIMA_1850_PrintString(_TEXT(0x8fbe, "\nNone!\n"));
    }
    else
    {
        ULTIMA_16ba_PrintChar('\n');
        ULTIMA_16ba_PrintChar(0x1b); // <-
        ULTIMA_16ba_PrintChar(',');
        ULTIMA_16ba_PrintChar(0x1a); // ->
        ULTIMA_16ba_PrintChar(',');
        ULTIMA_16ba_PrintChar(0x18); // ^
        ULTIMA_16ba_PrintChar(',');
        ULTIMA_16ba_PrintChar(0x19); // v
        ULTIMA_1850_PrintString(_TEXT(0x8fc6, " to move,\nRETURN selects.\nType M to mix:"));

        local_4_ab = CMDS_18be();
        if (local_4_ab >= 0)
        {
            local_8 = CMDS_1a70(local_4_ab);
            if (local_8 > 0)
            {
                if (local_4_ab != 0)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x8ff0, "Mixing...\n"));
                    if (D_5893_mapId > 0x20)
                    {
                        ULTIMA_20fa_WaitTicks(10);
                    }
                    else
                    {
                        ULTIMA_3ae6_WaitFrames(10);
                    }

                    local_c = 0x80;
                    for (local_a = 0; local_a < 8; local_a++)
                    {
                        if ((local_4_ab & local_c) != 0)
                        {
                            D_5850_reagents[local_a] -= local_8;
                        }
                        local_c >>= 1;
                    }

                    if (local_6 >= 0 && D_1cc0[local_6] == local_4_ab)
                    {
                        ULTIMA_1850_PrintString(_TEXT(0x8ffc, "\nDone!\n"));
                        D_57f0_mixedSpells[local_6] += local_8;
                        if (D_57f0_mixedSpells[local_6] > 99)
                        {
                            D_57f0_mixedSpells[local_6] = 99;
                        }
                    }
                    else
                    {
                        ULTIMA_16ba_PrintChar('\n');
                        ULTIMA_39fc_GetFirstActivePartyMember();
                        ULTIMA_2fd0_SpringTrap(D_5876_tempX);
                    }
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9004, "\nNothing to mix!\n"));
                }
            }
        }
    }

    ULTIMA_4e20_ClearStatsTitle();
    ULTIMA_4f3c_DrawStatsDivider();
    ULTIMA_2900_UpdateVitalsDisplay();
}

void CMDS_1c20_KlimbCmd(void)
{
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    if (D_57af_grapple == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9016, "With what?\n"));
        return;
    }

    if (D_587c_partyTile != TILE_ACTOR_AVATAR)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9022, "On foot!\n"));
        return;
    }

    if (ULTIMA_35ec_SelectDirection() == 0)
    {
        return;
    }

    local_6 = D_5876_tempX;
    local_a = D_5878_tempY;

    local_8 = D_5896_mapX + D_5876_tempX;
    local_c = D_5897_mapY + D_5878_tempY;

    local_e_am = *ULTIMA_4402_GetTileAddr(local_8, local_c);
    if (local_e_am == 13)
    {
        ULTIMA_1850_PrintString(_TEXT(0x902c, "Impassable!\n"));
    }
    else if (local_e_am != 12)
    {
        ULTIMA_1850_PrintString(_TEXT(0x903a, "Not climbable!\n"));
    }
    else
    {
        for (local_4_ab = 0; local_4_ab < D_585b_partyCount; local_4_ab++)
        {
            if (D_55a8_party[local_4_ab].status != STATUS_DEAD && D_55a8_party[local_4_ab].dex < ULTIMA_2092_RandomRange(1, 0x1e))
            {
                ULTIMA_1850_PrintString(_TEXT(0x904a, "Fell!\n"));
                ULTIMA_2a52_DamagePlayer(local_4_ab, ULTIMA_2092_RandomRange(1, 5));
            }
        }

        MAINOUT_0354_MoveParty(local_6, local_a);
    }
}
