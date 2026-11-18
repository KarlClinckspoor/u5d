#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include <stdlib.h>
#include <string.h>

#include "cast2.h"
#include "shoppes.h"
#include "zstats.h"

// Copy source to dest; returns the end of the copy (its terminator).
static char* SHOPPES_0000_AppendString(char* dest, char* source)
{
    while (*dest = *source++)
    {
        dest++;
    }

    return dest;
}

// Print a shop text. Bytes from 0x80 are words of the word table (D_24ea_wordTable);
// # the shop's name, $ the owner's, % the price (D_b118_shopPrice), ^ D_b11a_shopQuantity (a
// quantity), & D_ab00_itemName (an item), * D_ac62_placeName (a place): the placeholders of SHOPPE.DAT on the
// Ultima Codex, "Ultima V internal formats". @ is the time of day here
// (morning, afternoon, evening).
void SHOPPES_0026_PrintShopText(char* text)
{
    byte ch_c;
    char number_d[6];
    char* scan;
    char* out_h;

    scan = text;
    out_h = D_b7fa_shopTextOut;

    while (*scan != 0)
    {
        // 0068
        ch_c = *scan;
        if (ch_c == 0x23 || ch_c == 0x24 || ch_c == 0x40 || ch_c == 0x25 || ch_c == 0x5e ||
            ch_c == 0x26 || ch_c == 0x2a || ch_c >= 0x80)
        {
            // 008d
            *scan = 0;
            out_h = SHOPPES_0000_AppendString(out_h, text);
            *scan = ch_c;
            text = scan + 1;
        }

        // 00a4
        scan++;

        if (ch_c >= 0x80)
        {
            // 00ab
            *out_h++ = 0x20;
            out_h = SHOPPES_0000_AppendString(out_h, D_24ea_wordTable[ch_c - 0x80]);
            if (*scan != 0 && (byte)*scan < 0x80)
            {
                *out_h++ = 0x20;
            }
        }
        else
        {
            // 0036
            switch (ch_c)
            {
            case 0x23: // '#'
                // 0056
                out_h = SHOPPES_0000_AppendString(out_h, D_aafc_shopName); // 3ef24
                break;

            case 0x24: // '$'
                // 00d0
                out_h = SHOPPES_0000_AppendString(out_h, D_aafe_shopOwner); // 3ef28
                continue;

            case 0x40: // '('
                // 00d8
                if (D_587f_hour < 0xc)
                {
                    out_h = SHOPPES_0000_AppendString(out_h, /*0x7826*/ "morning");
                }
                else if (D_587f_hour < 0x12)
                {
                    out_h = SHOPPES_0000_AppendString(out_h, /*0x782e*/ "afternoon");
                }
                else
                {
                    out_h = SHOPPES_0000_AppendString(out_h, /*0x7838*/ "evening");
                }
                break;

            case 0x25: // '%'
                // 00fc
                itoa(D_b118_shopPrice, number_d, 10);
                out_h = SHOPPES_0000_AppendString(out_h, number_d);
                break;

            case 0x5e: // '^'
                // 0114
                itoa(D_b11a_shopQuantity, number_d, 10);
                out_h = SHOPPES_0000_AppendString(out_h, number_d);
                break;

            case 0x26: // '&'
                // 0122
                out_h = SHOPPES_0000_AppendString(out_h, D_ab00_itemName);
                break;

            case 0x2a: // '*'
                // 012a
                out_h = SHOPPES_0000_AppendString(out_h, D_ac62_placeName);
                break;
            }
        }
    }

    // 014a
    strcpy(out_h, text);

    ULTIMA_1850_PrintString(D_b7fa_shopTextOut);
}

// Print the text at fileOffset of SHOPPE.DAT.
void SHOPPES_017a_PrintShopFileText(int fileOffset)
{
    ULTIMA_256e_ReadFileFromDisk(/*0x7840*/ "SHOPPE.DAT", D_b21e_scratch, 1500, fileOffset);
    SHOPPES_0026_PrintShopText(D_b21e_scratch);
}

// After a payment: with shadowlord 0 in town (see TALK_1180_FalsehoodSteals) the party
// loses up to 64 gold more.
void SHOPPES_019a_FalsehoodSurcharge(void)
{
    if (D_5958_townShadowlord == 0)
    {
        // FMT: int local_8 = D_57aa_gold;
        // 3f54(&local_8)
        // D_57aa_gold = local_8
        ULTIMA_3f54_DecreaseInt(&D_57aa_gold, ULTIMA_2092_RandomRange(1, 0x40));
    }
}

// One of the shop kind's four greetings (D_3b2a_greetTexts), then the prompt.
void SHOPPES_01b6_Greet(void)
{
    ULTIMA_16ba_PrintChar('\"');
    SHOPPES_017a_PrintShopFileText(D_3b2a_greetTexts[D_b116_shopKind][ULTIMA_2092_RandomRange(0, 3)]);

    if (ULTIMA_1f12_GetCurrentTextX() > 0xb)
    {
        ULTIMA_1850_PrintString(/*0x784c*/ "\n\n:");
    }
    else if (ULTIMA_1f12_GetCurrentTextX() == 0)
    {
        ULTIMA_1850_PrintString(/*0x7850*/ "\n:");
    }
    else
    {
        ULTIMA_16ba_PrintChar(' ');
    }
}

// One of the shop kind's four farewells: outcome 0 (D_3b6a_farewellTexts) or 1 (D_3baa_farewellTexts1); nothing for others.
void SHOPPES_0202_Farewell(int outcome)
{
    if (outcome == 0)
    {
        ULTIMA_1850_PrintString(/*0x7854*/ "\n\n\"");
        SHOPPES_017a_PrintShopFileText(D_3b6a_farewellTexts[D_b116_shopKind][ULTIMA_2092_RandomRange(0, 3)]);
    }
    else if (outcome == 1)
    {
        ULTIMA_1850_PrintString(/*0x7858*/ "\n\n\"");
        SHOPPES_017a_PrintShopFileText(D_3baa_farewellTexts1[D_b116_shopKind][ULTIMA_2092_RandomRange(0, 3)]);
    }

    if (outcome == 0 || outcome == 1)
    {
        if (ULTIMA_1f12_GetCurrentTextX() != 0)
        {
            ULTIMA_16ba_PrintChar('\n');
        }

        SHOPPES_0026_PrintShopText(/*0x785c*/ "says $.\n");
    }
}

// CHECKED
// ask y/n
int SHOPPES_0280_AskYesNo(void)
{
    int key;

    do
    {
        key = ULTIMA_266c_GetChar();
        if ((uint)key == 0x4e)
        {
            ULTIMA_1850_PrintString(/*0x7866*/ "No");
        }
        else if ((uint)key == 0x59)
        {
            ULTIMA_1850_PrintString(/*0x786a*/ "Yes");
        }
    } while ((uint)key != 0x4e && (uint)key != 0x59);

    return key;
}

// Provisioner: price (by the talker's intelligence), sales talk and "Interested?" for keys (item 0), gems (1) or torches (2). Returns 1 when sold, -1 when the party cannot pay, else the outcome so far.
static int SHOPPES_02ba_OfferProvision(int member, int item, int outcome)
{
    char key;


    D_b118_shopPrice = D_3bea_provisionPrices[D_b114_shopIndex][item];
    D_b118_shopPrice += (D_b118_shopPrice * -((s32)(D_55a8_party[member].intel * 3) - 100) / (u32)100);

    ULTIMA_1850_PrintString(/*0x786e*/ "\n\n\"");
    SHOPPES_017a_PrintShopFileText(D_3c0a_provisionTexts[item]);
    ULTIMA_1850_PrintString(/*0x7872*/ "\n\nInterested?\" ");

    do
    {
        key = ULTIMA_266c_GetChar();
        if (key == 'N')
        {
            ULTIMA_1850_PrintString(/*0x7882*/ "No\n\n\"What else, then?\n\n");
        }
        else if (key == 'Y')
        {
            ULTIMA_1850_PrintString(/*0x789a*/ "Yes\n");
            if (D_57aa_gold < D_b118_shopPrice)
            {
                SHOPPES_017a_PrintShopFileText(0x21e6);
                outcome = -1;
            }
            else
            {
                D_57aa_gold -= D_b118_shopPrice;
                SHOPPES_019a_FalsehoodSurcharge();
                ULTIMA_2900_UpdateVitalsDisplay();
                switch (item)
                {
                case 0:
                    ULTIMA_3ef0_IncreaseByte(&D_57ac_keys, 3, 99);
                    break;
                case 1:
                    ULTIMA_3ef0_IncreaseByte(&D_57ad_gems, 4, 99);
                    break;
                case 2:
                    ULTIMA_3ef0_IncreaseByte(&D_57ae_torches, 5, 99);
                    break;
                }

                outcome = 1;
                SHOPPES_0026_PrintShopText(/*0x78a0*/ "\n\"Sold!\"\nsays $.\n\n\"What else, \n");
                if (D_55a8_party[member].gender == 12)
                {
                    ULTIMA_1850_PrintString(/*0x78c0*/ "m'lady");
                }
                else
                {
                    ULTIMA_1850_PrintString(/*0x78c8*/ "m'lord");
                }

                ULTIMA_1850_PrintString(/*0x78d0*/ "?\n\n");
            }
        }
    } while (key != 'N' && key != 'Y');

    return outcome;
}

// Provisioner: the list a Keys, b Gems, c Torches; loops until the party leaves. Returns the outcome for the farewell.
static int SHOPPES_03f6_ProvisionMenu(int member)
{
    bool done;
    int outcome;
    byte key_g;

    outcome = done = 0;

    while (!done)
    {
        ULTIMA_1850_PrintString(/*0x78d4*/ "a.........Keys\n");
        ULTIMA_1850_PrintString(/*0x78e4*/ "b.........Gems\n");
        ULTIMA_1850_PrintString(/*0x78f4*/ "c......Torches\n\n");
        ULTIMA_1850_PrintString(/*0x7906*/ "Thy concern?\" ");

        do
        {
            key_g = ULTIMA_266c_GetChar();
            switch (key_g)
            {
            case 0x41:
            case 0x42:
            case 0x43:
                ULTIMA_16ba_PrintChar(key_g | 0x20);
                outcome = SHOPPES_02ba_OfferProvision(member, key_g - 0x41, outcome);
                if (outcome == -1)
                    done = 1;
                break;

            case U5_KEY_ESC:
            case U5_KEY_SPACE:
                done = 1;
                break;

            case 0x44:
                SHOPPES_017a_PrintShopFileText(8725); // offset: 0x2215
                break;

            default:
                key_g = 0;
                break;
            }
        } while (key_g == 0 && !done);
    }

    return outcome;
}

// The provisioner (keys, gems, torches): greeting, the list, farewell.
void SHOPPES_04a2_Provisioner(int member)
{
    int done;
    byte key_g;
    int outcome;

    outcome = done = 0;

    SHOPPES_01b6_Greet();

    do
    {
        key_g = ULTIMA_266c_GetChar();
        switch (key_g)
        {
        case 0x59:
            ULTIMA_1850_PrintString(/*0x7916*/ "Yes\n\n\"We sell:\n\n");
            outcome = SHOPPES_03f6_ProvisionMenu(member);
            done = 1;
            break;

        case U5_KEY_SPACE:
        case 0x4e:
            ULTIMA_1850_PrintString(/*0x7928*/ "No");
            done = 1;
            break;
        }
    } while (!done);

    SHOPPES_0202_Farewell(outcome);
}

// Reagent shop: the choice-th reagent this shop sells (D_3a32 price, D_3a5a lot size), sales talk and sale. Returns -2 for a letter with no reagent, -1 when the party cannot pay.
// CHECKED
static int SHOPPES_0502_OfferReagent(int member, int choice, int outcome)
{
    int reagent;
    int listed;
    char key_g;

    listed = 0;
    for (reagent = 0; reagent < 8; reagent++)
    {
        if (D_3a32_reagentPrices[D_b114_shopIndex][reagent] != 0)
        {
            if (listed == choice)
                break;

            listed++;
        }
    }

    if (reagent == 8)
    {
        return -2;
    }

    if (D_5850_reagents[reagent] == 99)
    {
        ULTIMA_1850_PrintString(/*0x792c*/ "\n\n\"Thou canst not carry any more!\"\n\n");
        ULTIMA_266c_GetChar();
        return outcome;
    }
    else
    {
        D_b118_shopPrice = D_3a32_reagentPrices[D_b114_shopIndex][reagent];
        D_b118_shopPrice += D_b118_shopPrice * -((s32)(D_55a8_party[member].intel * 3) - 100) / (u32)100;
        D_b11a_shopQuantity = D_3a5a_reagentLots[D_b114_shopIndex][reagent];

        ULTIMA_1850_PrintString(/*0x7952*/ "\n\n\"");
        SHOPPES_0026_PrintShopText((char*)&D_b21e_scratch[D_3c10_reagentTexts[reagent] - 0x1a67]); // 0x1a67: file load offset
        ULTIMA_1850_PrintString(/*0x7956*/ " Is this thy need?\" ");
        if (ULTIMA_1f12_GetCurrentTextX() > 0xc)
        {
            ULTIMA_1850_PrintString(/*0x796c*/ "\n:");
        }

        do
        {
            key_g = ULTIMA_266c_GetChar();
            if (key_g == 'N')
            {
                ULTIMA_1850_PrintString(/*0x7970*/ "No\n\n\"What else?\n\n");
            }
            else if (key_g == 'Y')
            {
                ULTIMA_1850_PrintString(/*0x7982*/ "Yes\n");
                if (D_57aa_gold < D_b118_shopPrice)
                {
                    SHOPPES_0026_PrintShopText((char*)&D_b21e_scratch[0x4c4]);
                    outcome = -1;
                }
                else
                {
                    D_57aa_gold -= D_b118_shopPrice;
                    SHOPPES_019a_FalsehoodSurcharge();
                    ULTIMA_2900_UpdateVitalsDisplay();
                    ULTIMA_3ef0_IncreaseByte(&D_5850_reagents[reagent], D_b11a_shopQuantity, 99);
                    outcome = 1;
                    SHOPPES_0026_PrintShopText(/*0x7988*/ "\n\"I thank thee!\"\nsays $.\n");
                    ULTIMA_1850_PrintString(/*0x79a2*/ "\"Anything else?\n\n");
                }
            }
        } while (key_g != 'N' && key_g != 'Y');
    }

    // a8e7
    return outcome;
}
u8 D_3a32_reagentPrices[5][8] =
{
    0x00, 0x14, 0x12, 0x0c, 0x00, 0x00, 0x0c, 0x0d,
    0x0c, 0x10, 0x10, 0x08, 0x14, 0x00, 0x00, 0x00,
    0x0e, 0x10, 0x00, 0x00, 0x1e, 0x12, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x06, 0x08, 0x08, 0x0a, 0x0f,
    0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x1e, 0x28    
};

u8 D_3a5a_reagentLots[5][8] =
{
    0x00, 0x0a, 0x06, 0x04, 0x00, 0x00, 0x01, 0x01,
    0x0c, 0x08, 0x08, 0x02, 0x04, 0x00, 0x00, 0x00,
    0x0e, 0x08, 0x00, 0x00, 0x06, 0x06, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x01
};

u16 D_3a82_itemBasePrices[0x30] =
{
    0x000f, 0x0032, 0x0078, 0x0096, 0x0028, 0x0046, 0x0078, 0x07d0,
    0x0000, 0x0014, 0x0032, 0x0064, 0x0096, 0x012c, 0x02bc, 0x0000,
    0x0001, 0x000a, 0x0005, 0x0005, 0x000f, 0x0007, 0x0003, 0x0028,
    0x0032, 0x003c, 0x004b, 0x000a, 0x0096, 0x000f, 0x0046, 0x0055,
    0x0096, 0x00c8, 0x00fa, 0x0000, 0x0320, 0x00fa, 0x03e8, 0x0000,
    0x0000, 0x0000, 0x01c2, 0x01f4, 0x00c8, 0x0384, 0x00f0, 0x0000
};

u8 D_3ae2_armsStock[9][8] =
{
    0x10, 0x11, 0x1a, 0x1b, 0x1c, 0x1d, 0x24, 0xff,
    0x13, 0x18, 0x2e, 0x16, 0x03, 0x06, 0x19, 0xff,
    0x00, 0x09, 0x0a, 0x12, 0x15, 0x25, 0x26, 0xff,
    0x02, 0x04, 0x0b, 0x17, 0x1e, 0x18, 0x1f, 0xff,
    0x20, 0x21, 0x22, 0x02, 0x05, 0x0c, 0x0e, 0xff,
    0x01, 0x07, 0x0d, 0x0e, 0x1e, 0x25, 0x2b, 0xff,
    0x00, 0x0a, 0x10, 0x14, 0x17, 0x13, 0x2a, 0xff,
    0x07, 0x20, 0x24, 0x1b, 0x1f, 0x2c, 0x2d, 0xff,
    0x01, 0x0d, 0x1c, 0x1d, 0x22, 0x16, 0x19, 0xff
};

u16 D_3b2a_greetTexts[8][4] =
{
    0x0000, 0x0000, 0x0000, 0x0000, 0x0d6c, 0x0da6, 0x0dd9, 0x0e14,
    0x1491, 0x14e1, 0x1513, 0x1557, 0x1697, 0x16d8, 0x1710, 0x174d,
    0x1a67, 0x1aaa, 0x1ae5, 0x1b25, 0x1f62, 0x1f92, 0x1fd1, 0x2026,
    0x223c, 0x2273, 0x22ab, 0x22df, 0x23e4, 0x241a, 0x245c, 0x2495
};

u16 D_3b6a_farewellTexts[8][4] =
{
    0x0000, 0x0011, 0x0038, 0x0043, 0x0e44, 0x0e69, 0x0e85, 0x0ea1,
    0x159a, 0x15b4, 0x15be, 0x15c9, 0x1788, 0x179b, 0x17c7, 0x17d2,
    0x1b61, 0x1b73, 0x1b90, 0x1ba8, 0x205f, 0x2078, 0x2086, 0x209b,
    0x231e, 0x233c, 0x235e, 0x2381, 0x24f2, 0x250c, 0x2520, 0x2539
};

u16 D_3baa_farewellTexts1[8][4] =
{
    0x0057, 0x0064, 0x0076, 0x0083, 0x0ebc, 0x0ece, 0x0ee3, 0x0ef4,
    0x15ea, 0x160b, 0x161b, 0x1637, 0x17e6, 0x1807, 0x181d, 0x1832,
    0x1bbf, 0x1bda, 0x1bf5, 0x1c0f, 0x20bb, 0x20da, 0x20ee, 0x210e,
    0x231e, 0x233c, 0x235e, 0x2381, 0x2569, 0x2584, 0x25a2, 0x25be
};

u16 D_3bea_provisionPrices[4][4] =
{
    0x00be, 0x00ff, 0x000c, 0x0118, 0x00a0, 0x00c8, 0x000b, 0x00af,
    0x00b9, 0x00e1, 0x0019, 0x0109, 0x00aa, 0x0096, 0x0014, 0x012c
};

u16 D_3c0a_provisionTexts[3] = { 0x212d, 0x216c, 0x21b9 };

u16 D_3c10_reagentTexts[8] = { 0x1c1c, 0x1c7c, 0x1cda, 0x1d3b, 0x1d8d, 0x1dee, 0x1e4e, 0x1ebd };

char* D_3c20_reagentNames[8] =
{
    /*0x79b4*/ "Sulfur Ash",
    /*0x79c0*/ "Ginseng",
    /*0x79c8*/ "Garlic",
    /*0x79d0*/ "Spider Silk",
    /*0x79dc*/ "Blood Moss",
    /*0x79e8*/ "Black Pearl",
    /*0x79f4*/ "Nightshade",
    /*0x7a00*/ "Mandrake"
};

// Reagent shop: the lettered list of the reagents on sale ("Thy interest?") and the purchases.
static int SHOPPES_0666_ReagentMenu(int member)
{
    bool done;
    int result_h;
    int outcome;
    byte letter_h;
    int reagent;
    byte key_g;

    outcome = done = 0;

    ULTIMA_256e_ReadFileFromDisk(/*0x7a0a*/ "SHOPPE.DAT", D_b21e_scratch, 1500, 6759); //offset: 0x1a67

    while (!done)
    {
        letter_h = 'A';
        reagent = 0;
        do
        {
            if (D_3a32_reagentPrices[D_b114_shopIndex][reagent] != 0) // size = max_b114 * 8 + 8
            {
                ULTIMA_16ba_PrintChar(letter_h++);
                ULTIMA_1850_PrintString(/*0x7a16*/ "...");
                ULTIMA_1850_PrintString(D_3c20_reagentNames[reagent]);
                ULTIMA_16ba_PrintChar('\n');
            }
            reagent++;
        } while (reagent < 8);

        ULTIMA_1850_PrintString(/*0x7a1a*/ "\nThy interest?\" ");

        do
        {
            key_g = ULTIMA_266c_GetChar();

            switch (key_g)
            {
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
                result_h = SHOPPES_0502_OfferReagent(member, key_g - 0x41, outcome);
                if (result_h == -1)
                {
                    outcome = -1;
                    done = 1;
                }
                else if (result_h == -2)
                {
                    key_g = 0;
                }
                else
                {
                    outcome = result_h;
                }
                break;

            case U5_KEY_ENTER:
            case U5_KEY_ESC:
            case U5_KEY_SPACE:
                done = 1;
                break;

            default:
                key_g = 0;
                break;
            }

        } while (key_g == 0 && done == 0);
    }

    return outcome;
}

// The reagent shop: greeting, the list, farewell.
void SHOPPES_075e_Reagents(int member)
{
    int done;
    byte key_g;
    int outcome;
    
    outcome = done = 0;

    SHOPPES_01b6_Greet();

    do
    {
        key_g = ULTIMA_266c_GetChar();
        switch (key_g)
        {
        case 0x59:
            ULTIMA_1850_PrintString(/*0x7a2c*/ "Yes\n\n\"Fine! We sell:\n\n");
            outcome = SHOPPES_0666_ReagentMenu(member);
            done = 1;
            break;

        case U5_KEY_SPACE:
        case 0x4e:
            ULTIMA_1850_PrintString(/*0x7a44*/ "No");
            done = 1;
            break;
        }
    } while (!done);

    SHOPPES_0202_Farewell(outcome);
}

// The stables: sells a horse for D_3c30 (adjusted by intelligence), placed on a free tile next to the party. Closed when there is none.
// CHECKED (loop)
void SHOPPES_07be_Stables(int member)
{
    int tile;
    int outcome;
    int freeActor;
    int y;
    int i;
    int x;
    bool done;
    ActorFmt* horse;
    byte key_g;

    for (i = 0; i < 32; i++)
    {
        if (D_5c5a_actors[i].tile == 0)
            break;
    }
    freeActor = i;

    for (i = 0; i < 4; i++)
    {
        x = D_3c38_stableDx[i] + D_5896_mapX;
        y = D_3c40_stableDy[i] + D_5897_mapY;
        tile = *ULTIMA_4402_GetTileAddr(x, y);

        if (ULTIMA_368e_FindActorTileAtPos(x, y, D_5895_mapLevel) == 0)
        {
            if (tile == TILE_MAP_44 || tile == TILE_MAP_45 || tile == TILE_MAP_GRASS)
                break;
        }
    }

    // aabf
    if (i == 4 || freeActor == 0x20)
    {
        ULTIMA_1850_PrintString(/*0x7a48*/ "The stables are closed.\n");
    }
    else
    {
        outcome = done = 0;

        D_b118_shopPrice = D_3c30_horsePrices[D_b114_shopIndex];
        D_b118_shopPrice += (D_b118_shopPrice * -((s32)(D_55a8_party[member].intel * 3) - 100) / (u32)100);

        SHOPPES_01b6_Greet();
        do
        {
            key_g = ULTIMA_266c_GetChar();
            switch (key_g)
            {
            case 0x59:
                ULTIMA_1850_PrintString(/*0x7a62*/ "Yes\n\n\"");
                SHOPPES_017a_PrintShopFileText(5699); // 0x1643
                ULTIMA_1850_PrintString(/*0x7a6a*/ "\n\nDeal?\" ");

                do
                {
                    key_g = ULTIMA_266c_GetChar();
                    if (key_g == 'Y')
                        break;
                } while (key_g != 'N');

                if (key_g == 'N')
                {
                    ULTIMA_1850_PrintString(/*0x7a74*/ "No");
                    done = 1;
                }
                else
                {
                    ULTIMA_1850_PrintString(/*0x7a78*/ "Yes!");
                    if (D_57aa_gold < D_b118_shopPrice)
                    {
                        ULTIMA_1850_PrintString(/*0x7a7e*/ "\n\n\"Thou couldst not afford to ");
                        SHOPPES_0026_PrintShopText(/*0x7a9e*/ "feed it!\"\nyells $.\n");
                        outcome = -1;
                    }
                    else
                    {
                        // abda
                        D_57aa_gold -= D_b118_shopPrice;
                        SHOPPES_019a_FalsehoodSurcharge();

                        horse = &D_5c5a_actors[freeActor];
                        horse->animState = horse->extra = horse->value = 0;
                        horse->tile = horse->animTile = TILE_ACTOR_HORSE;
                        horse->x = x;
                        horse->y = y;
                        horse->z = D_5895_mapLevel;

                        ULTIMA_5910_UpdateFrame();
                        ULTIMA_2900_UpdateVitalsDisplay();
                        outcome = 1;
                    }

                    done = 1;
                }
                break;

            case U5_KEY_SPACE:
            case 0x4e:
                ULTIMA_1850_PrintString(/*0x7ab2*/ "No");
                done = 1;
                break;
            }
        } while (!done);

        SHOPPES_0202_Farewell(outcome);
    }
}
u16 D_3c30_horsePrices[4] = {0x0064, 0x0082, 0x00a0, 0x00be};

s16 D_3c38_stableDx[4] = {0x0000, 0x0000, 0x0001, 0xffff};

s16 D_3c40_stableDy[4] = {0x0001, 0xffff, 0x0000, 0x0000};

s16 D_3c48_armsTexts[0x2f] =
{
    0x0095, 0x00d0, 0x011f, 0x0152, 0x0192, 0x01cb, 0x0209, 0x024d,
    0x0000, 0x02b2, 0x030a, 0x0342, 0x0376, 0x03c6, 0x0410, 0x0000,
    0x0465, 0x04a1, 0x04de, 0x0521, 0x0560, 0x05ab, 0x05f6, 0x0638,
    0x066e, 0x06b0, 0x06ee, 0x0721, 0x0742, 0x077c, 0x07a4, 0x07e7,
    0x082f, 0x0872, 0x08bb, 0x0000, 0x090e, 0x0959, 0x09a0, 0x0000,
    0x0000, 0x0000, 0x09f7, 0x0a60, 0x0ac2, 0x0b19, 0x0b78
};

char* D_3ca6_buyOffers[4] =
{
    /*0x7ab6*/ "Wouldst thou buy one?",
    /*0x7acc*/ "Wilt thou take it?",
    /*0x7ae0*/ "Wish ye it?",
    /*0x7aec*/ "May I get one for thee?"
};

char* D_3cae_cantPayInsults[4] =
{
    /*0x7b04*/ "Can't pay?! Out with ye, orc-face!",
    /*0x7b28*/ "What be ye trying to pull? OUT!",
    /*0x7b48*/ "OUT, SLIME!",
    /*0x7b54*/ "BEAT IT!",
};

// Arms shop: price of an item (D_3a82), its sales talk, and the sale (at most 99 of an item, ammunition is set to 99). Returns 1 sold, -1 when the party cannot pay.
static int SHOPPES_09ac_OfferArm(int member, int item, int outcome)
{
    byte key;

    D_b118_shopPrice = D_3a82_itemBasePrices[item];
    D_b118_shopPrice += (D_b118_shopPrice * -((s32)(D_55a8_party[member].intel * 3) - 100) / (u32)100);

    ULTIMA_1850_PrintString(/*0x7b5e*/ "\n\n\"");
    SHOPPES_017a_PrintShopFileText(D_3c48_armsTexts[item]);
    ULTIMA_1850_PrintString(/*0x7b62*/ "\n\n");
    ULTIMA_1850_PrintString(D_3ca6_buyOffers[ULTIMA_2092_RandomRange(0, 3)]);
    ULTIMA_1850_PrintString(/*0x7b66*/ "\" ");

    do
    {
        key = ULTIMA_266c_GetChar();
        if (key == 'N')
        {
            ULTIMA_1850_PrintString(/*0x7b6a*/ "No\n\n");
        }
        else if (key == 'Y')
        {
            ULTIMA_1850_PrintString(/*0x7b70*/ "Yes\n");
            if (D_57c0_equipment[item] == 99)
            {
                ULTIMA_1850_PrintString(/*0x7b76*/ "\n\"Thou canst not carry any more!\"\n");
                SHOPPES_0026_PrintShopText(/*0x7b9a*/ "says $.\n\n");
                ULTIMA_266c_GetChar();
            }
            else if (D_57aa_gold < D_b118_shopPrice)
            {
                ULTIMA_1850_PrintString(/*0x7ba4*/ "\n\"");
                ULTIMA_1850_PrintString(D_3cae_cantPayInsults[ULTIMA_2092_RandomRange(0, 3)]);
                SHOPPES_0026_PrintShopText(/*0x7ba8*/ "\"\nyells $.\n");
                outcome = -1;
            }
            else
            {
                D_57aa_gold -= D_b118_shopPrice;
                SHOPPES_019a_FalsehoodSurcharge();
                ULTIMA_2900_UpdateVitalsDisplay();
                if (item == 0x1b || item == 0x1d)
                {
                    D_57c0_equipment[item] = 99;
                }
                else
                {
                    ULTIMA_3ef0_IncreaseByte(item + D_57c0_equipment, 1, 99);
                }

                SHOPPES_0026_PrintShopText(/*0x7bb4*/ "\nSold!\n");
                outcome = 1;
            }
        }
    } while (key != 'N' && key != 'Y');

    if (outcome != -1)
    {
        ULTIMA_1850_PrintString(/*0x7bbc*/ "\"Anything else,\n");
        if (outcome != 0)
        {
            if (D_55a8_party[member].gender == 12)
            {
                ULTIMA_1850_PrintString(/*0x7bce*/ "milady?");
            }
            else
            {
                ULTIMA_1850_PrintString(/*0x7bd6*/ "sir?");
            }
        }
        else
        {
            ULTIMA_1850_PrintString(/*0x7bdc*/ "then?");
        }
    }

    return outcome;
}

char* D_3cb6_showPrompts[4] =
{
    /*0x7be2*/ "What may I show thee?",
    /*0x7bf8*/ "Which wouldst thou like to see?",
    /*0x7c18*/ "What is thine interest?",
    /*0x7c30*/ "Which would ye see?",
};

// Arms shop: the lettered list of what this shop stocks (D_3ae2); the purchases. Returns the outcome for the farewell.
// CHECKED
static int SHOPPES_0b30_BuyMenu(int member)
{
    bool done;
    int item;
    int rows;
    int outcome;
    byte key_g;

    outcome = done = 0;

    do
    {
        ULTIMA_1850_PrintString(/*0x7c44*/ "\n\n");

        for (rows = 0; rows < 8; rows++)
        {
            item = D_3ae2_armsStock[D_b114_shopIndex][rows];
            if (item == 0xff)
                break;

            // ae5c
            ULTIMA_16ba_PrintChar((uint)rows + 0x61);
            ULTIMA_1850_PrintString(/*0x7c48*/ "...");

            if (ULTIMA_216c_strlen(D_17f6_equipNames[item]) < 0xd)
            {
                ULTIMA_1850_PrintString(D_17f6_equipNames[item]);
            }
            else
            {
                ULTIMA_1850_PrintString(D_1962_equipShortNames[item]);
            }

            if (ULTIMA_1f12_GetCurrentTextX() != 0)
            {
                ULTIMA_16ba_PrintChar('\n');
            }
        }

        ULTIMA_16ba_PrintChar('\n');
        ULTIMA_1850_PrintString(D_3cb6_showPrompts[ULTIMA_2092_RandomRange(0, 3)]);
        ULTIMA_1850_PrintString(/*0x7c4c*/ "\" ");

        do
        {
            // ae1b
            key_g = ULTIMA_266c_GetChar();
            item = key_g - 0x41; // - 'A'
            if (item < rows && item >= 0)
            {
                ULTIMA_16ba_PrintChar((uint)item + 0x61);
                outcome = SHOPPES_09ac_OfferArm(member, D_3ae2_armsStock[D_b114_shopIndex][item], outcome);
                if (outcome == -1)
                {
                    done = 1;
                }
            }
            else
            {
                if (key_g == U5_KEY_ESC || key_g == U5_KEY_SPACE)
                {
                    done = 1;
                }
                else
                {
                    key_g = 0;
                }
            }
        } while (key_g == 0 && !done);
    } while (!done);

    return outcome;
}

// 1 when the party owns any equipment.
static int SHOPPES_0c58_HasEquipment(void)
{
    int item;

    for (item = 0; item < 0x30; item++)
    {
        if (D_57c0_equipment[item] != 0)
        {
            return 1;
        }
    }

    return 0;
}

// Arms shop, selling: scroll the party's equipment list so the cursor item is visible and draw it (cursor inverted, arrows at the bottom). Returns the cursor's row.
static int SHOPPES_0c80_DrawSellList(int* first, int* cursor)
{
    int row;
    int arrows;
    int item;
    int cursorRow;

    if (D_57c0_equipment[*cursor] == 0)
    {
        if (*first == *cursor)
        {
            *cursor = ZSTATS_056c_PrevItem(*cursor, 0x30, D_57c0_equipment, 0xff);
            if (*cursor == -1)
            {
                *cursor = ZSTATS_05a4_NextItem(-1, 0x30, D_57c0_equipment, 0xff);
            }

            *first = *cursor;
        }
        else
        {
            *cursor = ZSTATS_05a4_NextItem(*cursor, 0x30, D_57c0_equipment, 0xff);
            if (*cursor == -1)
            {
                *cursor = ZSTATS_056c_PrevItem(0x30, 0x30, D_57c0_equipment, 0xff);
                if (ZSTATS_056c_PrevItem(*first, 0x30, D_57c0_equipment, 0xff) > -1)
                {
                    *first = ZSTATS_056c_PrevItem(*first, 0x30, D_57c0_equipment, 0xff);
                }
            }
        }
    }

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(1, 1);

    item = *first;
    while (item != -1)
    {
        if (*cursor == item)
        {
            ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            cursorRow = ULTIMA_1cee_GetCurrentTextY();
        }

        ZSTATS_05e2_PrintItemRow(item, D_57c0_equipment, D_1962_equipShortNames, 0x2d);

        if (*cursor == item)
        {
            ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
        }

        ULTIMA_1bf2_SetTextPosition(1, ULTIMA_1cee_GetCurrentTextY());
        if (ULTIMA_1cee_GetCurrentTextY() == 5)
            break;
        
        item = ZSTATS_05a4_NextItem(item, 0x30, D_57c0_equipment, 0xff);
    }

    for (row = ULTIMA_1cee_GetCurrentTextY(); row != 5; row = row + 1)
    {
        ULTIMA_1bf2_SetTextPosition(1, row);
        ULTIMA_1850_PrintString(/*0x7c50*/ "             ");
    }

    arrows = 0;
    if (ZSTATS_056c_PrevItem(*first, 0x30, D_57c0_equipment, 0xff) != -1)
    {
        arrows = 2;
    }

    if (item != -1 && ZSTATS_05a4_NextItem(item, 0x30, D_57c0_equipment, 0xff) != -1)
    {
        arrows++;
    }

    if (arrows != 0)
    {
        ULTIMA_1bf2_SetTextPosition(6, 6);
        ULTIMA_4c2a_PrintLeftCap();
    }

    switch (arrows)
    {
    case 0:
        ULTIMA_4f3c_DrawStatsDivider();
        break;
    case 1:
        ULTIMA_16ba_PrintChar(0x19);
        ULTIMA_4cce_PrintRightCap();
        break;
    case 2:
        ULTIMA_16ba_PrintChar(0x18);
        ULTIMA_4cce_PrintRightCap();
        break;
    case 3:
        ULTIMA_16ba_PrintChar(0x12);
        ULTIMA_4cce_PrintRightCap();
        break;
    }

    ULTIMA_1b94_SelectTextWindow(2);

    return cursorRow;
}
u16 D_3cbe_buybackTexts[8] = {0x0bc0, 0x0bff, 0x0c2e, 0x0c55, 0x0ca1, 0x0ce8, 0x0d12, 0x0d3d};

char* D_3cce_shopItemNames[0x30] =
{
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /*0x7c5e*/ "Cloth suit",
    /*0x7c6a*/ "Leather suit",
    /*0x7c78*/ "Ring Mail suit",
    /*0x7c88*/ "Scale suit",
    /*0x7c94*/ "Chain suit",
    /*0x7ca0*/ "Plate suit",
    /*0x7cac*/ "Mystic suits",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /*0x7cba*/ "Two-Handed Hammer",
    /*0x7ccc*/ "Two-Handed Axe",
    /*0x7cdc*/ "Two-Handed Sword",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /*0x7cee*/ "Invisibility Ring",
    /*0x7d00*/ "Protection Ring",
    /*0x7d10*/ "Regeneration Ring",
    /*0x7d22*/ "Turning Amulet",
    0,
    0
};

// Arms shop, selling: the shop buys one item for 3% of its base price per point of the talker's intelligence; arrows and bolts are refused. Returns 1 for ammunition, else 0.
static int SHOPPES_0e76_SellItem(int member, int item)
{
    byte key;

    if (item == 0x1b || item == 0x1d)
    {
        SHOPPES_0026_PrintShopText(/*0x7d32*/ "\n\n\"We don't deal in used ammunition!\"\ngrowls $.\n");
        return 1;
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x7d64*/ "\n\n\"");
        if (D_3a82_itemBasePrices[item] != 0)
        {
            D_b118_shopPrice = (D_55a8_party[member].intel * D_3a82_itemBasePrices[item] * (u32)3) / (u32)100 + 1;

            D_ab00_itemName = D_3cce_shopItemNames[item];
            if (D_3cce_shopItemNames[item] == 0)
            {
                D_ab00_itemName = D_17f6_equipNames[item];
            }

            //SHOPPES_0026_PrintShopText(D_3cbe_buybackTexts[iVar5] + 0xa65e);
            SHOPPES_0026_PrintShopText((char*)&D_b21e_scratch[D_3cbe_buybackTexts[ULTIMA_2092_RandomRange(0, 7)] + 0xa65e - 0xb21e]);
            ULTIMA_1850_PrintString(/*0x7d68*/ "\n\nDeal?\" ");
            do
            {
                key = ULTIMA_266c_GetChar();
            } while (key != 'N' && key != 'Y');

            if (key == 'N')
            {
                ULTIMA_1850_PrintString(/*0x7d72*/ "No");
            }
            else
            {
                SHOPPES_0026_PrintShopText(/*0x7d76*/ "Yes\n\n\"Done!\"\nsays $.");
                ULTIMA_3f14_IncreaseInt(&D_57aa_gold, D_b118_shopPrice, 9999);
                ULTIMA_3f36_DecreaseByte(&D_57c0_equipment[item], 1);
            }
            ULTIMA_2884_UpdateGoldDisplay();
        }
        else
        {
            SHOPPES_0026_PrintShopText(/*0x7d8c*/ "That, I cannot buy from thee.\"\nsays $.");
        }

        return 0;
    }
}

char* D_3d2e_sellPrompts[4] =
{
    /*0x7db4*/ "Which item wouldst thou like to sell?",
    /*0x7dda*/ "What dost thou wish to sell?",
    /*0x7df8*/ "Show me what ye got...",
    /*0x7e10*/ "What dost thou have for me to buy?"
};

char* D_3d36_farewells[4] =
{
    /*0x7e34*/ "Good-bye...",
    /*0x7e40*/ "Mayhap another time...",
    /*0x7e58*/ "Godspeed...",
    /*0x7e64*/ "Fare thee well..."
};

char* D_3d3e_sellMorePrompts[4] =
{
    /*0x7e76*/ "What else can ye offer me?",
    /*0x7e92*/ "What else hath ye to sell?",
    /*0x7eae*/ "What else doth thou wish to sell?",
    /*0x7ed0*/ "What other arms wilt thou sell?"
};

// Arms shop, selling: the cursor list of the party's equipment (arrows, Enter, Esc) and the sales.
// CHECKED (di)
static void SHOPPES_0f64_SellMenu(int member)
{
    int neighbor;
    int sold_m;
    int cursor;
    int i;
    int cursorRow;
    int done;
    byte key_g;
    int first_h;
    int steps;

    if (SHOPPES_0c58_HasEquipment() != 0)
    {
        ULTIMA_1850_PrintString(D_3d2e_sellPrompts[ULTIMA_2092_RandomRange(0, 3)]);
        ULTIMA_1850_PrintString(/*0x7ef0*/ "\" ");
        ULTIMA_4e50_ShowStatsTitle(/*0x7ef4*/ "Arms");
        ULTIMA_1b94_SelectTextWindow(1);
        ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 6);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
        ULTIMA_16ba_PrintChar(0x10);

        for (i = 0; i < 0xd; i++)
        {
            ULTIMA_16ba_PrintChar(0x11);
        }

        ULTIMA_16ba_PrintChar(0x13);

        for (i = 1; i < 5; i++)
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

        cursor = first_h = ZSTATS_05a4_NextItem(-1, 0x30, D_57c0_equipment, 0xff);
        cursorRow = SHOPPES_0c80_DrawSellList(&first_h, &cursor);
        done = sold_m = 0;

        ULTIMA_256e_ReadFileFromDisk(/*0x7efa*/ "SHOPPE.DAT", D_b21e_scratch, 1500, 3008); // offset: 0xbc0

        // b317
        while (done == 0)
        {
            // b320
            key_g = ULTIMA_266c_GetChar();

            switch (key_g)
            {
            case U5_KEY_LEFT:
            case U5_KEY_UP:
            case U5_KEY_PGUP:
                // b34c
                if (key_g == U5_KEY_PGUP)
                {
                    steps = 4;
                }
                else
                {
                    steps = 1;
                }

                for (i = 0; i < steps; i++)
                {
                    // b376
                    neighbor = ZSTATS_056c_PrevItem(cursor, 0x30, D_57c0_equipment, 0xff);

                    if (cursor == first_h)
                    {
                        if (neighbor > -1)
                        {
                            first_h = neighbor;
                            // b395
                            cursor = neighbor;
                        }
                    }
                    else
                    {
                        // b395
                        cursor = neighbor;
                    }
                }

                // b3a2
                cursorRow = SHOPPES_0c80_DrawSellList(&first_h, &cursor);
                break;

            case U5_KEY_RIGHT:
            case U5_KEY_DOWN:
            case U5_KEY_PGDN:
                // b3b4
                if (key_g == U5_KEY_PGDN)
                {
                    steps = 4;
                }
                else
                {
                    steps = 1;
                }

                for (i = 0; i < steps; i++)
                {
                    // b3da
                    neighbor = ZSTATS_05a4_NextItem(cursor, 0x30, D_57c0_equipment, 0xff);
                    if (neighbor > -1)
                    {
                        cursor = neighbor;
                        if (++cursorRow > 4)
                        {
                            first_h = ZSTATS_05a4_NextItem(first_h, 0x30, D_57c0_equipment, 0xff);
                        }
                    }
                }

                cursorRow = SHOPPES_0c80_DrawSellList(&first_h, &cursor);
                break;

            case U5_KEY_HOME:
                // b41a
                cursor = first_h = ZSTATS_05a4_NextItem(-1, 0x30, D_57c0_equipment, 0xff);
                cursorRow = SHOPPES_0c80_DrawSellList(&first_h, &cursor);
                break;

            case U5_KEY_END:
                // b436
                cursor = first_h = ZSTATS_056c_PrevItem(0x30, 0x30, D_57c0_equipment, 0xff);
                for (i = 1; i < 4; i++)
                {
                    neighbor = ZSTATS_056c_PrevItem(first_h, 0x30, D_57c0_equipment, 0xff);
                    if (neighbor == -1)
                        break;

                    first_h = neighbor;
                }

                cursorRow = SHOPPES_0c80_DrawSellList(&first_h, &cursor);
                break;

            case U5_KEY_ENTER:
            case U5_KEY_SPACE:
                // b47a
                sold_m = SHOPPES_0e76_SellItem(member, cursor);
                if (sold_m == 0 && SHOPPES_0c58_HasEquipment() != 0)
                {
                    cursorRow = SHOPPES_0c80_DrawSellList(&first_h, &cursor);
                    ULTIMA_1850_PrintString(/*0x7f06*/ "\n\n\"");
                    ULTIMA_1850_PrintString(D_3d3e_sellMorePrompts[ULTIMA_2092_RandomRange(0, 3)]);
                    ULTIMA_1850_PrintString(/*0x7f0a*/ "\" ");
                }
                else
                {
                    done = 1;
                }
                break;

            case U5_KEY_ESC:
                // b4c6
                done = 1;
                break;
            }
        }

        // b4f6 (OK)
        ULTIMA_2900_UpdateVitalsDisplay();
        ULTIMA_4e20_ClearStatsTitle();

        if (sold_m != 0)
        {
            return;
        }

        ULTIMA_1850_PrintString(/*0x7f0e*/ "\n\n\"");
        ULTIMA_1850_PrintString(D_3d36_farewells[ULTIMA_2092_RandomRange(0, 3)]);
        ULTIMA_1850_PrintString(/*0x7f12*/ "\"\n");

        if (SHOPPES_0c58_HasEquipment() == 0)
        {
            return;
        }

        SHOPPES_0026_PrintShopText(/*0x7f16*/ "says $.\n");
    }
    else
    {
        SHOPPES_0026_PrintShopText(/*0x7f20*/ "Thou hast nothing to sell!\"\ngrowls $.\n");
    }
}
char* D_3d46_buyOrSellGreetings[2] =
{
    /*0x7f48*/ "Hail, friend! Wouldst thou Buy or Sell?",
    /*0x7f70*/ "Greetings, traveller! Wish ye to Buy, or hast thou wares to Sell?",
};

char* D_3d4a_goodReplies[4] =
{
    /*0x7fb2*/ "Very good!\n",
    /*0x7fbe*/ "Excellent!\n",
    /*0x7fca*/ "Fine, fine!\n",
    /*0x7fd8*/ "But of course!\n"
};

char* D_3d52_stockIntros[4] =
{
    /*0x7fe8*/ "We have:",
    /*0x7ff2*/ "We stock:",
    /*0x7ffc*/ "Thou canst buy:",
    /*0x800c*/ "We've got:"
};

// the reply to buying a spell already known: an array, so in _DATA (DS:3d5a)
char D_3d5a_noNeedOfArt[] = "\n\n\"Thou hast no need of this art!\"\nsays $.";

u8 D_3d86_healPrices[8] = {0x23, 0x28, 0x2d, 0x32, 0x37, 0x3c, 0x41, 0x46};

u8 D_3d8e_curePrices[8] = {0x14, 0x19, 0x1e, 0x23, 0x28, 0x0f, 0x0a, 0x01};

u16 D_3d96_raisePrices[8] = {0x00c8, 0x00d7, 0x00e1, 0x00ed, 0x00f7, 0x00f9, 0x0106, 0x010e};

// The shop of arms and armour: greeting, then Buy or Sell, farewell.
void SHOPPES_12b2_Armourer(int member)
{
    int outcome;
    byte key;

    outcome = 0;

    SHOPPES_0026_PrintShopText(/*0x8018*/ "\"Good @, and welcome to #!\"\n");
    ULTIMA_266c_GetChar();
    SHOPPES_0026_PrintShopText(/*0x8036*/ "\n$ says,\n\"");

    ULTIMA_1850_PrintString(D_3d46_buyOrSellGreetings[ULTIMA_2092_RandomRange(0, 1)]);
    ULTIMA_1850_PrintString(/*0x8042*/ "\" ");

    do
    {
        key = ULTIMA_266c_GetChar();
        switch (key)
        {
        case 0x42:
            ULTIMA_1850_PrintString(/*0x8046*/ "Buy\n\n\"");
            ULTIMA_1850_PrintString(D_3d4a_goodReplies[ULTIMA_2092_RandomRange(0, 3)]);
            ULTIMA_1850_PrintString(D_3d52_stockIntros[ULTIMA_2092_RandomRange(0, 3)]);
            outcome = SHOPPES_0b30_BuyMenu(member);
            break;
        case 0x53:
            ULTIMA_1850_PrintString(/*0x804e*/ "Sell\n\n\"");
            SHOPPES_0f64_SellMenu(member);
            outcome = -1;
            break;
        case U5_KEY_SPACE:
            ULTIMA_1850_PrintString(/*0x8056*/ "No");
            break;
        }
    } while (key != 'B' && key != 'S' && key != U5_KEY_SPACE);

    SHOPPES_0202_Farewell(outcome);
}

// Healer: "Who needs my aid?"; the one member of a party of one, -1 for no one.
static int SHOPPES_137c_ChoosePatient(void)
{
    int patient;

    if (D_585b_partyCount == 1)
    {
        patient = 0;
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x805a*/ "\n\n\"Who needs my aid?\" ");
        patient = ULTIMA_2e8e_SelectPlayer();
        if (patient == -1)
        {
            ULTIMA_1850_PrintString(/*0x8072*/ "No one");
        }
    }

    return patient;
}

// Healer: the screen flashes (red, white) with sounds as the spell is cast.
static void SHOPPES_13b0_HealingFlash(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor);
    ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_2192_AudioPulse(0x100e, 1, 0x57e4, 5000, 1);
    ULTIMA_2192_AudioPulse(0x100e, 1, 0x57e4, 0x6b6c, -1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_2192_AudioPulse(0x11b2, 1, 40000, 1, 1);
    ULTIMA_2192_AudioPulse(0x11b2, 1, 40000, 40000, -1);
    ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_2192_AudioPulse(0x8fc, 1, 18000, 1, 2);
    ULTIMA_2192_AudioPulse(0x8fc, 1, 18000, 36000, -2);
}

// Healer: "for N gold. Wilt thou pay?"; takes the gold. Returns 1 when refused or when the party is too poor.
// CHECKED (si)
static int SHOPPES_146a_AskPayment(void)
{
    bool refused;
    int key;

    SHOPPES_0026_PrintShopText(/*0x807a*/ "for % gold.\n\nWilt thou\npay?\" ");
    do
    {
        // b708
        key = ULTIMA_266c_GetChar();
        if ((uint)key == 'Y')
        {
            ULTIMA_1850_PrintString(/*0x8098*/ "Yes");
        }
        else if ((uint)key == 'N')
        {
            ULTIMA_1850_PrintString(/*0x809c*/ "No");
        }

        // b726
    } while ((uint)key != 'Y' && (uint)key != 'N');

    // b732
    refused = key == 'N';

    // b742
    if (refused == 0 && D_57aa_gold < D_b118_shopPrice && (D_b118_shopPrice > 100 || D_5893_mapId != 7))
    {
        // b75e
        SHOPPES_017a_PrintShopFileText(9131); // 0x23ab
        refused = 1;
    }

    // b76a
    if (refused == 0 && D_57aa_gold >= D_b118_shopPrice)
    {
        D_57aa_gold -= D_b118_shopPrice;
        SHOPPES_019a_FalsehoodSurcharge();
    }

    // b780
    return refused;
}

// The healer: Cure, Heal or Resurrect a party member for a price (free in map 5: "Receive now the Light!"), with the flash and sound.
// CHECKED
// The original places the "Is there any other way" tail (ending in a jump back to the loop
// test) right after case C's local_8 == -1 test. MSC moves it there only when nothing falls
// into it: the switch ends in goto L_b838 (and default: goto L_b838, so the dispatch's
// default jump goes there too), and case C jumps to it with a goto. Written inside case
// C instead, the code is the same but its strings come first in CONST. The gotos to L_b8a1
// and L_b914 make the then-branches jump forward into the later copy as the original does,
// and the label L_b9c1 keeps MSC from holding local_8 * 32 in SI across the calls.
void SHOPPES_14f8_Healer(int unused)
{
    int done_j;
    byte key_g;
    int patient;

    SHOPPES_01b6_Greet();

    done_j = 0;
    while (!done_j)
    {
        do
        {
            key_g = ULTIMA_266c_GetChar();
            if (key_g == 'N')
            {
                ULTIMA_1850_PrintString(/*0x80a0*/ "No");
            }
            else if (key_g == 'Y')
            {
                ULTIMA_1850_PrintString(/*0x80a4*/ "Yes\n\n");
            }
        } while (key_g != 'N' && key_g != 'Y');

        if (key_g == 'N')
            break;

        ULTIMA_1850_PrintString(/*0x80aa*/ "\"We have powers to Cure, Heal, or Resurrect.\"\n");
        SHOPPES_0026_PrintShopText(/*0x80da*/ "says $.\n\n\"What is the nature of thy need?\" ");

        do
        {
            key_g = ULTIMA_266c_GetChar();
        } while (key_g != 'C' && key_g != 'H' && key_g != 'R' &&
                 key_g != U5_KEY_SPACE && key_g != U5_KEY_ENTER);

        switch (key_g)
        {
        case 0x43:
            // b826
            ULTIMA_1850_PrintString(/*0x8106*/ "Curing");
            patient = SHOPPES_137c_ChoosePatient();
            if (patient == -1)
                goto L_b838;
            {
                // b856
                if (D_55a8_party[patient].status != STATUS_POISONED)
                {
                    // b864
                    SHOPPES_0026_PrintShopText(D_3d5a_noNeedOfArt);
                }
                else
                {
                    // b86e
                    ULTIMA_1850_PrintString(/*0x810e*/ "\n\n\"");
                    if (D_5893_mapId == 5)
                    {
                        ULTIMA_1850_PrintString(/*0x8112*/ "Receive now the Light!\"");
                        // -> b8a1
                        goto L_b8a1;
                    }
                    else
                    {
                        // b886
                        ULTIMA_1850_PrintString(/*0x812a*/ "I can cure thy poisoned body ");
                        D_b118_shopPrice = D_3d8e_curePrices[D_b114_shopIndex];
                        if (SHOPPES_146a_AskPayment() == 0)
                        {
                            // b8a1
L_b8a1:
                            SHOPPES_13b0_HealingFlash();
                            D_55a8_party[patient].status = STATUS_GOOD;
                        }
                    }
                }
            }
            break;
        case 0x48:
            // b8b2
            ULTIMA_1850_PrintString(/*0x8148*/ "Healing");
            patient = SHOPPES_137c_ChoosePatient();
            if (patient != -1)
            {
                // b8c7
                if (D_55a8_party[patient].status != STATUS_DEAD && D_55a8_party[patient].hp != D_55a8_party[patient].maxHp)
                {
                    ULTIMA_1850_PrintString(/*0x8150*/ "\n\n\"");
                    if (D_5893_mapId == 5)
                    {
                        ULTIMA_1850_PrintString(/*0x8154*/ "Receive now the Light!\"");
                        // -> b914
                        goto L_b914;
                    }
                    else
                    {
                        // b8f6
                        ULTIMA_1850_PrintString(/*0x816c*/ "I can heal thee ");
                        D_b118_shopPrice = D_3d86_healPrices[D_b114_shopIndex];
                        if (SHOPPES_146a_AskPayment() == 0)
                        {
                            // b914
L_b914:
                            SHOPPES_13b0_HealingFlash();
                            D_55a8_party[patient].hp = D_55a8_party[patient].maxHp;
                        }
                    }
                }
                else
                {
                    // -> b864
                    SHOPPES_0026_PrintShopText(D_3d5a_noNeedOfArt);
                }
            }
            break;
        case 0x52:
            // b92a
            ULTIMA_1850_PrintString(/*0x817e*/ "Resurrect");
            patient = SHOPPES_137c_ChoosePatient();
            if (patient != -1)
            {
                if (D_55a8_party[patient].status == STATUS_DEAD)
                {
                    ULTIMA_1850_PrintString(/*0x8188*/ "\n\n\"");
                    ULTIMA_1850_PrintString(/*0x818c*/ "I can raise this unfortunate person from ");
                    ULTIMA_1850_PrintString(/*0x81b6*/ "the dead ");

                    D_b118_shopPrice = D_3d96_raisePrices[D_b114_shopIndex];

                    if (SHOPPES_146a_AskPayment() == 0)
                    {
L_b9c1:
                        SHOPPES_13b0_HealingFlash();
                        CAST2_05e0_Resurrect(patient, 0xff);
                        D_55a8_party[patient].hp = D_55a8_party[patient].maxHp;
                        ULTIMA_2900_UpdateVitalsDisplay();
                    }
                }
                else
                {
                    SHOPPES_0026_PrintShopText(D_3d5a_noNeedOfArt);
                }
            }
            break;
        case U5_KEY_ENTER:
        case U5_KEY_SPACE:
            // b99e
            ULTIMA_1850_PrintString(/*0x81c0*/ "Nothing");
            done_j = 1;
            break;
        default:
            goto L_b838;
        }
        goto L_b838;

L_b838:
        if (!done_j)
        {
            ULTIMA_2900_UpdateVitalsDisplay();
            ULTIMA_1850_PrintString(/*0x81c8*/ "\n\n\"Is there any other way in which I may\n");
            ULTIMA_1850_PrintString(/*0x81f2*/ "aid thee?\" ");
        }
    }

    SHOPPES_0202_Farewell(1);
}
