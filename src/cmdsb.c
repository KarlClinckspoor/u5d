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

// The reagent list with a cursor for mixing: arrows move, return toggles a reagent, M accepts, Esc aborts;
// returns the bit mask of the chosen reagents (-1 when aborted).
static int CMDS_18be_SelectReagents(void)
{
    int mask;
    int count;
    int bit_j;
    byte owned_l[8];
    int key;
    int i;
    int cursor_h;
    int done;

    count = 0;
    for (i = 0; i < 8; i++)
    {
        if (D_5850_reagents[i] != 0)
        {
            owned_l[count] = i;
            count++;
        }
    }

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    ULTIMA_4efc_OpenStatsDivider();
    ULTIMA_4e50_ShowStatsTitle(/*0x8f64*/ "Reagents:");

    for (i = 0; i != count; i++)
    {
        ULTIMA_16ba_PrintChar('\n');
        ULTIMA_16ba_PrintChar(' ');
        ULTIMA_1a3e_PrintNumber(D_5850_reagents[owned_l[i]], 2, 0x30);
        ULTIMA_16ba_PrintChar(' ');
        ULTIMA_1850_PrintString(D_19d2_reagentNames[owned_l[i]]);
    }

    // d8f2
    cursor_h = mask = 0;

    ULTIMA_2a28_InvertPlayerRow(1);
    done = 0;
    do
    {
        ULTIMA_1b94_SelectTextWindow(2);
        key = ULTIMA_266c_GetChar();
        switch (key)
        {
        case U5_KEY_LEFT:
        case U5_KEY_UP:
            // 19b2
            if (cursor_h > 0)
            {
                ULTIMA_2a28_InvertPlayerRow(cursor_h + 1);
                cursor_h--;
                ULTIMA_2a28_InvertPlayerRow(cursor_h + 1);
            }
            break;

        case U5_KEY_RIGHT:
        case U5_KEY_DOWN:
            // 19d0
            if (cursor_h < count + -1)
            {
                ULTIMA_2a28_InvertPlayerRow(cursor_h + 1);
                cursor_h++;
                ULTIMA_2a28_InvertPlayerRow(cursor_h + 1);
            }
            break;

        case 0x4d:
            // 19e0
            done = 1;
            ULTIMA_1850_PrintString(/*0x8f6e*/ "\n\n");
            break;

        case U5_KEY_ENTER:
        case U5_KEY_SPACE:
            // 19ee
            bit_j = 0x80 >> owned_l[cursor_h];
            mask ^= bit_j;
            ULTIMA_1b94_SelectTextWindow(1);
            ULTIMA_1bf2_SetTextPosition(3, cursor_h + 1);
            ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            if ((bit_j & mask) != 0)
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
            mask = -1;
            done = 1;

            ULTIMA_16ba_PrintChar('\n');
            break;
        }
    } while (done == 0);

    ULTIMA_2a28_InvertPlayerRow(cursor_h + 1);
    ULTIMA_1b94_SelectTextWindow(2);
    return mask;
}

// Asks "How much?" until every chosen reagent covers the amount; returns it.
static int CMDS_1a70_AskMixAmount(int mask)
{
    bool enough;
    int amount;
    int i;
    int bit;

    do
    {
        enough = 1;
        ULTIMA_1850_PrintString(/*0x8f72*/ "How much? ");
        amount = ULTIMA_3b9e_GetNumber(2);
        if (amount != 0)
        {
            bit = 0x80;
            for (i = 0; i < 8; i++)
            {
                if ((mask & bit) != 0 && D_5850_reagents[i] < amount)
                {
                    ULTIMA_1850_PrintString(/*0x8f7e*/ "Insufficient reagents!\n\n");
                    enough = 0;
                    break;
                }
                bit >>= 1;
            }
        }
    } while (!enough);

    return amount;
}

// The Mix command: asks the spell and the reagents and the amount; a right recipe adds that many mixed
// spells (up to 99), a wrong one springs a trap on the first active member.
void CMDS_1ad8_MixCmd(void)
{
    int bit;
    int i;
    int amount;
    int spell;
    int mask_d;

    amount = 0;
    for (i = 0; i < 8; i++)
    {
        amount += D_5850_reagents[i];
    }

    if (amount == 0)
    {
        ULTIMA_1850_PrintString(/*0x8f98*/ "No reagents owned!\n");
        return;
    }

    ULTIMA_1850_PrintString(/*0x8fac*/ "For what spell?\n:");
    spell = CAST2_00de_SelectSpell();
    if (spell == -1)
    {
        ULTIMA_1850_PrintString(/*0x8fbe*/ "\nNone!\n");
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
        ULTIMA_1850_PrintString(/*0x8fc6*/ " to move,\nRETURN selects.\nType M to mix:");

        mask_d = CMDS_18be_SelectReagents();
        if (mask_d >= 0)
        {
            amount = CMDS_1a70_AskMixAmount(mask_d);
            if (amount > 0)
            {
                if (mask_d != 0)
                {
                    ULTIMA_1850_PrintString(/*0x8ff0*/ "Mixing...\n");
                    if (D_5893_mapId > 0x20)
                    {
                        ULTIMA_20fa_WaitTicks(10);
                    }
                    else
                    {
                        ULTIMA_3ae6_WaitFrames(10);
                    }

                    bit = 0x80;
                    for (i = 0; i < 8; i++)
                    {
                        if ((mask_d & bit) != 0)
                        {
                            D_5850_reagents[i] -= amount;
                        }
                        bit >>= 1;
                    }

                    if (spell >= 0 && D_1cc0_spellReagents[spell] == mask_d)
                    {
                        ULTIMA_1850_PrintString(/*0x8ffc*/ "\nDone!\n");
                        D_57f0_mixedSpells[spell] += amount;
                        if (D_57f0_mixedSpells[spell] > 99)
                        {
                            D_57f0_mixedSpells[spell] = 99;
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
                    ULTIMA_1850_PrintString(/*0x9004*/ "\nNothing to mix!\n");
                }
            }
        }
    }

    ULTIMA_4e20_ClearStatsTitle();
    ULTIMA_4f3c_DrawStatsDivider();
    ULTIMA_2900_UpdateVitalsDisplay();
}

// The Klimb command: with the grappling hook, on foot, up a mountain tile (12) in the chosen direction;
// members with dexterity below a roll fall and are hurt; the party moves there.
void CMDS_1c20_KlimbCmd(void)
{
    int tile;
    int y_e;
    int dy;
    int x_e;
    int dx;
    int member;

    if (D_57af_grapple == 0)
    {
        ULTIMA_1850_PrintString(/*0x9016*/ "With what?\n");
        return;
    }

    if (D_587c_partyTile != TILE_ACTOR_AVATAR)
    {
        ULTIMA_1850_PrintString(/*0x9022*/ "On foot!\n");
        return;
    }

    if (ULTIMA_35ec_SelectDirection() == 0)
    {
        return;
    }

    dx = D_5876_tempX;
    dy = D_5878_tempY;

    x_e = D_5896_mapX + D_5876_tempX;
    y_e = D_5897_mapY + D_5878_tempY;

    tile = *ULTIMA_4402_GetTileAddr(x_e, y_e);
    if (tile == 13)
    {
        ULTIMA_1850_PrintString(/*0x902c*/ "Impassable!\n");
    }
    else if (tile != 12)
    {
        ULTIMA_1850_PrintString(/*0x903a*/ "Not climbable!\n");
    }
    else
    {
        for (member = 0; member < D_585b_partyCount; member++)
        {
            if (D_55a8_party[member].status != STATUS_DEAD && D_55a8_party[member].dex < ULTIMA_2092_RandomRange(1, 0x1e))
            {
                ULTIMA_1850_PrintString(/*0x904a*/ "Fell!\n");
                ULTIMA_2a52_DamagePlayer(member, ULTIMA_2092_RandomRange(1, 5));
            }
        }

        MAINOUT_0354_MoveParty(dx, dy);
    }
}
