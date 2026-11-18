#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include "shoppes.h"
#include "shoppes3.h"
#include "town.h"

u8 D_4d7e_innRates[6] = { 0x02, 0x03, 0x02, 0x03, 0x02, 0x03 };


// The inn. How many of the roster are staying here (their innMapId is this map).
static int SHOPPES3_0000_CountGuests(void)
{
    int member;
    int count_h;

    count_h = 0;

    for (member = 0; member < 0x10; member++)
    {
        if (D_55a8_party[member].innMapId == D_5893_mapId)
        {
            count_h++;
        }
    }

    return count_h;
}


u8 D_4dc4_innRooms[6] = {0x03, 0x04, 0x03, 0x02, 0x02, 0x02};

// inn - can rest?
// 1 when the inn has a free room (D_4dc4_innRooms); else the innkeeper is sorry.
// gender is the party member's gender character: 0xc is addressed as "milady".
static int SHOPPES3_002c_HasRoom(int gender)
{
    ULTIMA_1850_PrintString(_TEXT(0x4d84, "\n\n"));

    if (SHOPPES3_0000_CountGuests() < D_4dc4_innRooms[D_b114_shopIndex])
    {
        return 1;
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x4d87, "\"I am sorry,\n"));
        if (gender == 0xc)
        {
            ULTIMA_1850_PrintString(_TEXT(0x4d95, "milady"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x4d9c, "sir"));
        }

        ULTIMA_1850_PrintString(_TEXT(0x4da0, ", but we\nhave no room\navailable.\"\n\n"));
        return 0;
    }
}


u16 D_4e6e_innOfferTexts[6] = {0x25d0, 0x2600, 0x2641, 0x2641, 0x2667, 0x26a7};

u8 D_4e7a_innBedX[6] = {0x15, 0x0f, 0x19, 0x14, 0x1b, 0x07};

u8 D_4e80_innBedY[6] = {0x0a, 0x07, 0x09, 0x01, 0x06, 0x1a};

// inn - rest
// Rest for the night. The price is the rate times the party size, times
// (200 - 3 * intelligence) / 100: the talker's intelligence lowers it.
// The party sleeps in the inn's bed until 6 o'clock; hit points and magic are restored,
// the poisoned die. Returns the outcome for SHOPPES_0202_Farewell (-2: no room).
static int SHOPPES3_0072_Rest(int intelligence, int gender, int outcome)
{
    int member;
    byte answer;
    S_55a8* player_d;
    byte activePlayer_h;

    ULTIMA_16ba_PrintChar('R');
    if (SHOPPES3_002c_HasRoom(gender) != 0)
    {
        ULTIMA_16ba_PrintChar('\"');

        D_b118_shopPrice = D_4d7e_innRates[D_b114_shopIndex] * D_585b_partyCount;
        D_b118_shopPrice += D_b118_shopPrice * -((s32)(intelligence * 3) - 100) / 100;

        SHOPPES_017a_PrintShopFileText(D_4e6e_innOfferTexts[D_b114_shopIndex]);
        ULTIMA_1850_PrintString(_TEXT(0x4dca, "\nWilt thou take\nit?\" "));
        answer = SHOPPES_0280_AskYesNo();
        ULTIMA_1850_PrintString(_TEXT(0x4de0, "\n\n"));
        if (answer == 'Y')
        {
            if (D_57aa_gold < D_b118_shopPrice)
            {
                ULTIMA_1850_PrintString(_TEXT(0x4de3, "\"Highwaymen!\nCheap, at that!\nOUT!\" "));
                SHOPPES_0026_PrintShopText(_TEXT(0x4e07, "screams\n$.\n"));
            }
            else
            {
                D_57aa_gold -= D_b118_shopPrice;
                SHOPPES_019a_FalsehoodSurcharge();
                ULTIMA_2900_UpdateVitalsDisplay();

                ULTIMA_1850_PrintString(_TEXT(0x4e13, "\"Have a pleasant\nnight, "));
                if (gender == 0xc)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x4e2c, "milady"));
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x4e33, "sir"));
                }

                SHOPPES_0026_PrintShopText(_TEXT(0x4e37, "!\"\nsays $.\n\n"));

                ULTIMA_3ae6_WaitFrames(10);
                ULTIMA_39cc_SetTile(D_594f_openDoorTile, D_5950_openDoorX, D_5951_openDoorY);

                D_5896_mapX = D_4e7a_innBedX[D_b114_shopIndex];
                D_5897_mapY = D_4e80_innBedY[D_b114_shopIndex];
                D_24e6_viewDirty = 1;

                activePlayer_h = D_587b_activePlayer;

                for (member = 0; member < D_585b_partyCount; member++)
                {
                    if (D_55a8_party[member].status == STATUS_GOOD)
                    {
                        D_55a8_party[member].status = STATUS_SLEEP;
                    }
                }

                ULTIMA_2900_UpdateVitalsDisplay();
                ULTIMA_5910_UpdateFrame();
                ULTIMA_1850_PrintString(_TEXT(0x4e44, "Zzzzzz....\n\n"));

                for (member = 0; member < 0xc; member++)
                {
                    ULTIMA_4f7c_AdvanceTime(5);
                }

                while (D_587f_hour != 6)
                {
                    ULTIMA_3ae6_WaitFrames(1);
                    ULTIMA_400c_RingRegeneration();
                    ULTIMA_2900_UpdateVitalsDisplay();
                    ULTIMA_4f7c_AdvanceTime(9);
                    if (D_587f_hour == 0x14 || D_587f_hour == 0x05)
                    {
                        TOWN_0170();
                    }
                }

                ULTIMA_1850_PrintString(_TEXT(0x4e51, "Morning!\n"));
                TOWN_1694();

                // e3e0
                for (member = 0; member < D_585b_partyCount; member++)
                {
                    player_d = &D_55a8_party[member];

                    if (player_d->status != STATUS_DEAD)
                    {
                        player_d->hp = player_d->maxHp;

                        switch (player_d->cls)
                        {
                        case CLASS_AVATAR:
                        case CLASS_MAGE:
                            player_d->mag = player_d->intel;
                            break;

                        case CLASS_BARD:
                            player_d->mag = player_d->intel >> 1;
                            break;
                        }

                        if (player_d->status == STATUS_POISONED)
                        {
                            player_d->status = STATUS_DEAD;
                            player_d->hp = 0;

                            ULTIMA_16ba_PrintChar('\n');
                            ULTIMA_1850_PrintString(player_d->name);
                            ULTIMA_1850_PrintString(_TEXT(0x4e5b, " has\npassed away.\n"));
                        }
                        else if (player_d->status == STATUS_SLEEP)
                        {
                            player_d->status = STATUS_GOOD;
                        }
                    }
                }

                D_587b_activePlayer = activePlayer_h;
                D_5896_mapX++;
                D_24e6_viewDirty = 1;
                ULTIMA_2900_UpdateVitalsDisplay();

                outcome = -1;
            }
        }
    }
    else
    {
        outcome = -2;
    }

    return outcome;
}

// Leave a companion at the inn: taken out of the party to the end of the roster, with
// this map as innMapId and a month count of 0 (S_55a8.monthsAtInn, probably: the bill multiplies by it).
static int SHOPPES3_02ae_LeaveCompanion(int intelligence, int gender, int outcome)
{
    bool asked_i;
    byte answer_b;
    int slot;
    int member;
    S_55a8 saved_n;

    ULTIMA_16ba_PrintChar('L');

    if (SHOPPES3_002c_HasRoom(gender) == 0)
    {
        return outcome;
    }

    if (D_585b_partyCount == 1)
    {
        SHOPPES_017a_PrintShopFileText(0x26e8);
        outcome = -1;
    }
    else
    {
        do
        {
            asked_i = 1;
            SHOPPES_0026_PrintShopText(_TEXT(0x4e86, "$ asks,\n\"Who will\nstay?\" "));
            member = ULTIMA_2e8e_SelectPlayer();
            if (member == -1)
            {
                ULTIMA_1850_PrintString(_TEXT(0x4ea0, "Nobody\n\n"));
                outcome = -2;
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x4ea9, "\n\n"));
                if (member == 0)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x4eac, "Thy friend"));
                    if (D_585b_partyCount > 2)
                    {
                        ULTIMA_16ba_PrintChar('s');
                    }
                    ULTIMA_1850_PrintString(_TEXT(0x4eb7, " will not leave thee!\n\n"));
                    asked_i = 0;
                }
                else if (D_55a8_party[member].status == STATUS_DEAD)
                {
                    SHOPPES_017a_PrintShopFileText(0x2723);
                    outcome = -1;
                }
                else
                {
                    D_b118_shopPrice = (uint)D_4d7e_innRates[D_b114_shopIndex] * 10;
                    D_b118_shopPrice += D_b118_shopPrice * -((s32)(intelligence * 3) - 100) / 100;

                    ULTIMA_1850_PrintString(_TEXT(0x4ecf, "\"The rate for\nour most comfortable room will be "));
                    SHOPPES_0026_PrintShopText(_TEXT(0x4f00, "% gold per month, due at check-out."));
                    ULTIMA_1850_PrintString(_TEXT(0x4f24, "\nWilt thou take\nit?\" "));
                    answer_b = SHOPPES_0280_AskYesNo();
                    ULTIMA_1850_PrintString(_TEXT(0x4f3a, "\n\n"));
                    if (answer_b == 'Y')
                    {
                        outcome = 1;
                        if (D_587b_activePlayer == member)
                        {
                            D_587b_activePlayer = 0xff;
                        }
                        else if (member < D_587b_activePlayer && D_587b_activePlayer != 0xff)
                        {
                            D_587b_activePlayer--;
                        }

                        D_55a8_party[member].innMapId = D_5893_mapId;
                        D_55a8_party[member].monthsAtInn = 0;

                        saved_n = D_55a8_party[member];

                        for (slot = member; slot < 0xf; slot++)
                        {
                            D_55a8_party[slot] = D_55a8_party[slot + 1];
                        }

                        D_55a8_party[0x10 - 1] = saved_n;

                        D_585b_partyCount--;
                        ULTIMA_2900_UpdateVitalsDisplay();
                        SHOPPES_0026_PrintShopText(_TEXT(0x4f3d, "\"I thank thee.\"\nsays $.\n\n"));
                    }
                }
            }
        } while (!asked_i);
    }

    return outcome;
}

// The guest before / after roster place `slot`; 0 when there is none.
static int SHOPPES3_0494_PrevGuest(int slot)
{
    while (--slot)
    {
        if (D_55a8_party[slot].innMapId == D_5893_mapId)
        {
            break;
        }
    }

    return slot;
}

static int SHOPPES3_04b6_NextGuest(int slot)
{
    while (++slot < 0x10)
    {
        if (D_55a8_party[slot].innMapId == D_5893_mapId)
        {
            break;
        }
    }

    if (slot == 0x10)
    {
        return 0;
    }

    return slot;
}

// Pick up a companion: choose one from the guest register (when more than one stays),
// pay the months stayed, and take the companion back into the party.
static int SHOPPES3_04e6_PickUpCompanion(int intelligence, int outcome)
{
    int guest_b;
    byte key;
    int barY_l;
    int i;
    uint months;
    bool chosen_d;
    int moved_i;
    S_55a8* player_g;
    S_55a8 saved;
    int guestCount;

    guestCount = SHOPPES3_0000_CountGuests();

    if (D_585b_partyCount == 6)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4f57, "\n\nOne must first be left behind!\n\n"));
    }
    else if (guestCount == 0)
    {
        SHOPPES_0026_PrintShopText(_TEXT(0x4f7a, "\n\n\"No one here is from thy party!\"\nsays $.\n\n"));
    }
    else
    {
        if (guestCount > 1)
        {
            // e6ff
            ULTIMA_1850_PrintString(_TEXT(0x4fa7, "\n\n\"Who will\ncheck out?\" "));
            ULTIMA_1b94_SelectTextWindow(1);
            ULTIMA_4efc_OpenStatsDivider();
            ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
            ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
            ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
            ULTIMA_16ba_PrintChar(0x10);

            for (i = 0; i < 0xd; i++)
            {
                ULTIMA_16ba_PrintChar(0x11);
            }

            ULTIMA_16ba_PrintChar(0x13);

            for (i = 1; i < 8; i++)
            {
                ULTIMA_1bf2_SetTextPosition(0, i);
                ULTIMA_16ba_PrintChar(0x17);
                ULTIMA_1bf2_SetTextPosition(0xe, i);
                ULTIMA_16ba_PrintChar(0x17);
            }

            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_16ba_PrintChar(0x14);

            for (i = 0; i < 0xd; i++)
            {
                ULTIMA_16ba_PrintChar(0x15);
            }

            ULTIMA_16ba_PrintChar(0x16);
            ULTIMA_1bf2_SetTextPosition(1, 1);
            ULTIMA_1850_PrintString(_TEXT(0x4fc0, "    GUEST"));
            ULTIMA_1bf2_SetTextPosition(1, 2);
            ULTIMA_1850_PrintString(_TEXT(0x4fca, "  REGISTER:\n\n"));

            // e7d6
            for (guest_b = 1; guest_b < 0x10; guest_b++)
            {
                if (D_55a8_party[guest_b].innMapId == D_5893_mapId)
                {
                    ULTIMA_1bf2_SetTextPosition(4, ULTIMA_1cee_GetCurrentTextY());

                    ULTIMA_1850_PrintString(D_55a8_party[guest_b].name);
                    ULTIMA_16ba_PrintChar('\n');
                }
            }

            ULTIMA_1b94_SelectTextWindow(2);
            chosen_d = 0;
            guest_b = SHOPPES3_04b6_NextGuest(0);
            barY_l = 0x28;
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);

            while (!chosen_d)
            {
                ULTIMA_0b86_GRAP_XorFillRect(0xc6, barY_l, 0x131, barY_l + 7);

                // si = moved_i

                do
                {
                    key = ULTIMA_266c_GetChar();
                    switch (key)
                    {
                    case U5_KEY_LEFT:
                    case U5_KEY_UP:
                        // e85c
                        moved_i = SHOPPES3_0494_PrevGuest(guest_b);
                        if (moved_i != 0)
                        {
                            ULTIMA_0b86_GRAP_XorFillRect(0xc6, barY_l, 0x131, barY_l + 7);
                            guest_b = moved_i;
                            barY_l -= 8;
                        }
                        // -> e87e
                        break;

                    case U5_KEY_RIGHT:
                    case U5_KEY_DOWN:
                        // e884
                        moved_i = SHOPPES3_04b6_NextGuest(guest_b);
                        if (moved_i != 0)
                        {
                            ULTIMA_0b86_GRAP_XorFillRect(0xc6, barY_l, 0x131, barY_l + 7);
                            guest_b = moved_i;
                            barY_l += 8;
                        }
                        break;

                    case U5_KEY_ESC:
                        // e8a8
                        ULTIMA_1850_PrintString(_TEXT(0x4fd8, "No one\n\n"));
                        guest_b = 0;

                        moved_i = 1;
                        chosen_d = 1;
                        break;

                    case U5_KEY_SPACE:
                    case U5_KEY_ENTER:
                        // e8b4
                        moved_i = 1;
                        chosen_d = 1;
                        break;

                    default:
                        // e8d0
                        moved_i = 0;
                        break;
                    }

                    // e87e
                } while (moved_i == 0);
            }

            // e8e0
            ULTIMA_4f3c_DrawStatsDivider();
            ULTIMA_2900_UpdateVitalsDisplay();
        }
        else
        {
            guest_b = SHOPPES3_04b6_NextGuest(0);
        }

        if (guest_b != 0)
        {
            months = (uint)D_55a8_party[guest_b].monthsAtInn;
            if (months == 0)
            {
                months = 1;
            }

            D_b118_shopPrice = (uint)D_4d7e_innRates[D_b114_shopIndex] * 10;
            D_b118_shopPrice += D_b118_shopPrice * -((s32)(intelligence * 3) - 100) / 100;

            D_b118_shopPrice = months * D_b118_shopPrice;

            SHOPPES_0026_PrintShopText(_TEXT(0x4fe1, "\n\n\"That will be % gold, please.\"\n\n\""));
            if (D_57aa_gold < D_b118_shopPrice)
            {
                SHOPPES_017a_PrintShopFileText(0x275a);
                TOWN_0958();
                outcome = -1;
            }
            else
            {
                D_57aa_gold -= D_b118_shopPrice;

                SHOPPES_019a_FalsehoodSurcharge();
                ULTIMA_2900_UpdateVitalsDisplay();
                outcome = 1;

                saved = D_55a8_party[guest_b];

                for (i = guest_b; i > (int)D_585b_partyCount; i--)
                {
                    D_55a8_party[i] = D_55a8_party[i - 1];
                }

                // e9ff
                D_55a8_party[D_585b_partyCount] = saved;

                player_g = &D_55a8_party[D_585b_partyCount++];

                player_g->innMapId = 0;
                if (player_g->status == STATUS_POISONED)
                {
                    player_g->status = STATUS_DEAD;
                    player_g->hp = 0;

                    ULTIMA_1850_PrintString(_TEXT(0x5005, "Thy friend has died, by the way.\"\n"));
                }
                else
                {
                    player_g->hp = player_g->maxHp;

                    switch (player_g->cls)
                    {
                    case CLASS_AVATAR:
                    case CLASS_MAGE:
                        player_g->mag = player_g->intel;
                        break;
                    case CLASS_BARD:
                        player_g->mag = player_g->intel >> 1;
                        break;
                    }

                    ULTIMA_1850_PrintString(_TEXT(0x5028, "I hope thou hast found thy stay enjoyable,\"\n"));
                }

                SHOPPES_0026_PrintShopText(_TEXT(0x5055, "says $.\n\n"));
                ULTIMA_2900_UpdateVitalsDisplay();
            }
        }
    }

    return outcome;
}

// The inn (shop kind 7). playerIndex is the party member who talks.
void SHOPPES3_08b4_Inn(int playerIndex)
{
    int intelligence;
    int gender;
    bool done_k;
    int result_f;
    byte key;
    int outcome_j;

    done_k = outcome_j = 0;

    intelligence = D_55a8_party[playerIndex].intel;
    gender = D_55a8_party[playerIndex].gender;

    D_587a_statusIcon = D_588e_protectTurns = 0;

    ULTIMA_2900_UpdateVitalsDisplay();

    SHOPPES_01b6_Greet();

    while (!done_k)
    {
        do
        {
            key = ULTIMA_266c_GetChar();
            if (key == U5_KEY_SPACE)
            {
                key = 'N';
            }

            if (key == 'N')
            {
                ULTIMA_1850_PrintString(_TEXT(0x505f, "No"));
            }
            else if (key == 'Y')
            {
                ULTIMA_1850_PrintString(_TEXT(0x5062, "Yes"));
            }
        } while (key != 'N' && key != 'Y');

        if (key == 'N')
            break;

        SHOPPES_0026_PrintShopText(_TEXT(0x5066, "\n\n$ asks,\n\"Art thou here\nto Pick up or\n"));
        ULTIMA_1850_PrintString(_TEXT(0x508e, "Leave a\ncompanion, or\nto Rest for the\nnight?\" "));

        do
        {
            key = ULTIMA_266c_GetChar();

            switch (key)
            {
            case 0x52:
                // eb2c
                result_f = SHOPPES3_0072_Rest(intelligence, gender, outcome_j);
                if (result_f > -2)
                {
                    outcome_j = result_f;
                    // eb3f
                    done_k = 1;
                }
                // -> eb44
                break;

            case U5_KEY_SPACE:
                // eb3f
                done_k = 1;
                // -> eb44
                break;

            case 0x4c:
                // eb4c
                result_f = SHOPPES3_02ae_LeaveCompanion(intelligence, gender, outcome_j);
                if (result_f > -2)
                {
                    outcome_j = result_f;
                }

                // eb5f
                if (outcome_j == -1)
                {
                    // -> eb3f
                    done_k = 1;
                }
                break;

            case 0x50:
                // eb66
                outcome_j = SHOPPES3_04e6_PickUpCompanion(intelligence, outcome_j);
                // -> eb5f
                if (outcome_j == -1)
                {
                    // -> eb3f
                    done_k = 1;
                }
                break;

            default:
                key = 0;
            }

            // eb44
        } while (key == 0);

        if (!done_k)
        {
            ULTIMA_1850_PrintString(_TEXT(0x50bd, "\"Is there\nanything more\nI can do for\nthee?\" "));
        }
    }

    SHOPPES_0202_Farewell(outcome_j);
}
