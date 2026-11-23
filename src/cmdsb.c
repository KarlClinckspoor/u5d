#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

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
static int SelectReagents(void)
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

    SelectTextWindow(1);
    SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    OpenStatsDivider();
    ShowStatsTitle(/*0x8f64*/ "Reagents:");

    for (i = 0; i != count; i++)
    {
        PrintChar('\n');
        PrintChar(' ');
        PrintNumber(D_5850_reagents[owned_l[i]], 2, 0x30);
        PrintChar(' ');
        PrintString(D_19d2_reagentNames[owned_l[i]]);
    }

    // d8f2
    cursor_h = mask = 0;

    InvertPlayerRow(1);
    done = 0;
    do
    {
        SelectTextWindow(2);
        key = GetChar();
        switch (key)
        {
        case U5_KEY_LEFT:
        case U5_KEY_UP:
            // 19b2
            if (cursor_h > 0)
            {
                InvertPlayerRow(cursor_h + 1);
                cursor_h--;
                InvertPlayerRow(cursor_h + 1);
            }
            break;

        case U5_KEY_RIGHT:
        case U5_KEY_DOWN:
            // 19d0
            if (cursor_h < count + -1)
            {
                InvertPlayerRow(cursor_h + 1);
                cursor_h++;
                InvertPlayerRow(cursor_h + 1);
            }
            break;

        case 0x4d:
            // 19e0
            done = 1;
            PrintString(/*0x8f6e*/ "\n\n");
            break;

        case U5_KEY_ENTER:
        case U5_KEY_SPACE:
            // 19ee
            bit_j = 0x80 >> owned_l[cursor_h];
            mask ^= bit_j;
            SelectTextWindow(1);
            SetTextPosition(3, cursor_h + 1);
            PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            if ((bit_j & mask) != 0)
            {
                PrintChar(0xf);
            }
            else
            {
                PrintChar(' ');
            }

            PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            break;

        case U5_KEY_ESC:
            // 1a2e
            mask = -1;
            done = 1;

            PrintChar('\n');
            break;
        }
    } while (done == 0);

    InvertPlayerRow(cursor_h + 1);
    SelectTextWindow(2);
    return mask;
}

// Asks "How much?" until every chosen reagent covers the amount; returns it.
static int AskMixAmount(int mask)
{
    bool enough;
    int amount;
    int i;
    int bit;

    do
    {
        enough = 1;
        PrintString(/*0x8f72*/ "How much? ");
        amount = GetNumber(2);
        if (amount != 0)
        {
            bit = 0x80;
            for (i = 0; i < 8; i++)
            {
                if ((mask & bit) != 0 && D_5850_reagents[i] < amount)
                {
                    PrintString(/*0x8f7e*/ "Insufficient reagents!\n\n");
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
void MixCmd(void)
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
        PrintString(/*0x8f98*/ "No reagents owned!\n");
        return;
    }

    PrintString(/*0x8fac*/ "For what spell?\n:");
    spell = SelectSpell();
    if (spell == -1)
    {
        PrintString(/*0x8fbe*/ "\nNone!\n");
    }
    else
    {
        PrintChar('\n');
        PrintChar(0x1b); // <-
        PrintChar(',');
        PrintChar(0x1a); // ->
        PrintChar(',');
        PrintChar(0x18); // ^
        PrintChar(',');
        PrintChar(0x19); // v
        PrintString(/*0x8fc6*/ " to move,\nRETURN selects.\nType M to mix:");

        mask_d = SelectReagents();
        if (mask_d >= 0)
        {
            amount = AskMixAmount(mask_d);
            if (amount > 0)
            {
                if (mask_d != 0)
                {
                    PrintString(/*0x8ff0*/ "Mixing...\n");
                    if (mapId > 0x20)
                    {
                        WaitTicks(10);
                    }
                    else
                    {
                        WaitFrames(10);
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

                    if (spell >= 0 && spellReagents[spell] == mask_d)
                    {
                        PrintString(/*0x8ffc*/ "\nDone!\n");
                        mixedSpells[spell] += amount;
                        if (mixedSpells[spell] > 99)
                        {
                            mixedSpells[spell] = 99;
                        }
                    }
                    else
                    {
                        PrintChar('\n');
                        GetFirstActivePartyMember();
                        SpringTrap(tempX);
                    }
                }
                else
                {
                    PrintString(/*0x9004*/ "\nNothing to mix!\n");
                }
            }
        }
    }

    ClearStatsTitle();
    DrawStatsDivider();
    UpdateVitalsDisplay();
}

// The Klimb command: with the grappling hook, on foot, up a mountain tile (12) in the chosen direction;
// members with dexterity below a roll fall and are hurt; the party moves there.
void CmdsKlimbCmd(void)
{
    int tile;
    int y_e;
    int dy;
    int x_e;
    int dx;
    int member;

    if (grapple == 0)
    {
        PrintString(/*0x9016*/ "With what?\n");
        return;
    }

    if (partyTile != TILE_ACTOR_AVATAR)
    {
        PrintString(/*0x9022*/ "On foot!\n");
        return;
    }

    if (SelectDirection() == 0)
    {
        return;
    }

    dx = tempX;
    dy = tempY;

    x_e = D_5896_mapX + tempX;
    y_e = D_5897_mapY + tempY;

    tile = *GetTileAddr(x_e, y_e);
    if (tile == 13)
    {
        PrintString(/*0x902c*/ "Impassable!\n");
    }
    else if (tile != 12)
    {
        PrintString(/*0x903a*/ "Not climbable!\n");
    }
    else
    {
        for (member = 0; member < partyCount; member++)
        {
            if (party[member].status != STATUS_DEAD && party[member].dex < RandomRange(1, 0x1e))
            {
                PrintString(/*0x904a*/ "Fell!\n");
                DamagePlayer(member, RandomRange(1, 5));
            }
        }

        MainoutMoveParty(dx, dy);
    }
}
