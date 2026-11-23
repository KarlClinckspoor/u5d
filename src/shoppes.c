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
static char* AppendString(char* dest, char* source)
{
    while (*dest = *source++)
    {
        dest++;
    }

    return dest;
}

// Print a shop text. Bytes from 0x80 are words of the word table (wordTable);
// # the shop's name, $ the owner's, % the price (shopPrice), ^ shopQuantity (a
// quantity), & itemName (an item), * placeName (a place): the placeholders of SHOPPE.DAT on the
// Ultima Codex, "Ultima V internal formats". @ is the time of day here
// (morning, afternoon, evening).
void PrintShopText(char* text)
{
    byte ch_c;
    char number_d[6];
    char* scan;
    char* out_h;

    scan = text;
    out_h = shopTextOut;

    while (*scan != 0)
    {
        // 0068
        ch_c = *scan;
        if (ch_c == 0x23 || ch_c == 0x24 || ch_c == 0x40 || ch_c == 0x25 || ch_c == 0x5e ||
            ch_c == 0x26 || ch_c == 0x2a || ch_c >= 0x80)
        {
            // 008d
            *scan = 0;
            out_h = AppendString(out_h, text);
            *scan = ch_c;
            text = scan + 1;
        }

        // 00a4
        scan++;

        if (ch_c >= 0x80)
        {
            // 00ab
            *out_h++ = 0x20;
            out_h = AppendString(out_h, wordTable[ch_c - 0x80]);
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
                out_h = AppendString(out_h, shopName); // 3ef24
                break;

            case 0x24: // '$'
                // 00d0
                out_h = AppendString(out_h, shopOwner); // 3ef28
                continue;

            case 0x40: // '('
                // 00d8
                if (D_587f_hour < 0xc)
                {
                    out_h = AppendString(out_h, /*0x7826*/ "morning");
                }
                else if (D_587f_hour < 0x12)
                {
                    out_h = AppendString(out_h, /*0x782e*/ "afternoon");
                }
                else
                {
                    out_h = AppendString(out_h, /*0x7838*/ "evening");
                }
                break;

            case 0x25: // '%'
                // 00fc
                itoa(shopPrice, number_d, 10);
                out_h = AppendString(out_h, number_d);
                break;

            case 0x5e: // '^'
                // 0114
                itoa(shopQuantity, number_d, 10);
                out_h = AppendString(out_h, number_d);
                break;

            case 0x26: // '&'
                // 0122
                out_h = AppendString(out_h, itemName);
                break;

            case 0x2a: // '*'
                // 012a
                out_h = AppendString(out_h, placeName);
                break;
            }
        }
    }

    // 014a
    strcpy(out_h, text);

    PrintString(shopTextOut);
}

// Print the text at fileOffset of SHOPPE.DAT.
void PrintShopFileText(int fileOffset)
{
    ReadFileFromDisk(/*0x7840*/ "SHOPPE.DAT", scratch, 1500, fileOffset);
    PrintShopText(scratch);
}

// After a payment: with shadowlord 0 in town (see FalsehoodSteals) the party
// loses up to 64 gold more.
void FalsehoodSurcharge(void)
{
    if (townShadowlord == 0)
    {
        // FMT: int local_8 = gold;
        // 3f54(&local_8)
        // gold = local_8
        DecreaseInt(&gold, RandomRange(1, 0x40));
    }
}

// One of the shop kind's four greetings (greetTexts), then the prompt.
void Greet(void)
{
    PrintChar('\"');
    PrintShopFileText(greetTexts[shopKind][RandomRange(0, 3)]);

    if (GetCurrentTextX() > 0xb)
    {
        PrintString(/*0x784c*/ "\n\n:");
    }
    else if (GetCurrentTextX() == 0)
    {
        PrintString(/*0x7850*/ "\n:");
    }
    else
    {
        PrintChar(' ');
    }
}

// One of the shop kind's four farewells: outcome 0 (farewellTexts) or 1 (farewellTexts1); nothing for others.
void ShoppesFarewell(int outcome)
{
    if (outcome == 0)
    {
        PrintString(/*0x7854*/ "\n\n\"");
        PrintShopFileText(farewellTexts[shopKind][RandomRange(0, 3)]);
    }
    else if (outcome == 1)
    {
        PrintString(/*0x7858*/ "\n\n\"");
        PrintShopFileText(farewellTexts1[shopKind][RandomRange(0, 3)]);
    }

    if (outcome == 0 || outcome == 1)
    {
        if (GetCurrentTextX() != 0)
        {
            PrintChar('\n');
        }

        PrintShopText(/*0x785c*/ "says $.\n");
    }
}

// CHECKED
// ask y/n
int AskYesNo(void)
{
    int key;

    do
    {
        key = GetChar();
        if ((uint)key == 0x4e)
        {
            PrintString(/*0x7866*/ "No");
        }
        else if ((uint)key == 0x59)
        {
            PrintString(/*0x786a*/ "Yes");
        }
    } while ((uint)key != 0x4e && (uint)key != 0x59);

    return key;
}

// Provisioner: price (by the talker's intelligence), sales talk and "Interested?" for keys (item 0), gems (1) or torches (2). Returns 1 when sold, -1 when the party cannot pay, else the outcome so far.
static int OfferProvision(int member, int item, int outcome)
{
    char key;


    shopPrice = provisionPrices[shopIndex][item];
    shopPrice += (shopPrice * -((s32)(party[member].intel * 3) - 100) / (u32)100);

    PrintString(/*0x786e*/ "\n\n\"");
    PrintShopFileText(provisionTexts[item]);
    PrintString(/*0x7872*/ "\n\nInterested?\" ");

    do
    {
        key = GetChar();
        if (key == 'N')
        {
            PrintString(/*0x7882*/ "No\n\n\"What else, then?\n\n");
        }
        else if (key == 'Y')
        {
            PrintString(/*0x789a*/ "Yes\n");
            if (gold < shopPrice)
            {
                PrintShopFileText(0x21e6);
                outcome = -1;
            }
            else
            {
                gold -= shopPrice;
                FalsehoodSurcharge();
                UpdateVitalsDisplay();
                switch (item)
                {
                case 0:
                    IncreaseByte(&keys, 3, 99);
                    break;
                case 1:
                    IncreaseByte(&gems, 4, 99);
                    break;
                case 2:
                    IncreaseByte(&torches, 5, 99);
                    break;
                }

                outcome = 1;
                PrintShopText(/*0x78a0*/ "\n\"Sold!\"\nsays $.\n\n\"What else, \n");
                if (party[member].gender == 12)
                {
                    PrintString(/*0x78c0*/ "m'lady");
                }
                else
                {
                    PrintString(/*0x78c8*/ "m'lord");
                }

                PrintString(/*0x78d0*/ "?\n\n");
            }
        }
    } while (key != 'N' && key != 'Y');

    return outcome;
}

// Provisioner: the list a Keys, b Gems, c Torches; loops until the party leaves. Returns the outcome for the farewell.
static int ProvisionMenu(int member)
{
    bool done;
    int outcome;
    byte key_g;

    outcome = done = 0;

    while (!done)
    {
        PrintString(/*0x78d4*/ "a.........Keys\n");
        PrintString(/*0x78e4*/ "b.........Gems\n");
        PrintString(/*0x78f4*/ "c......Torches\n\n");
        PrintString(/*0x7906*/ "Thy concern?\" ");

        do
        {
            key_g = GetChar();
            switch (key_g)
            {
            case 0x41:
            case 0x42:
            case 0x43:
                PrintChar(key_g | 0x20);
                outcome = OfferProvision(member, key_g - 0x41, outcome);
                if (outcome == -1)
                    done = 1;
                break;

            case U5_KEY_ESC:
            case U5_KEY_SPACE:
                done = 1;
                break;

            case 0x44:
                PrintShopFileText(8725); // offset: 0x2215
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
void Provisioner(int member)
{
    int done;
    byte key_g;
    int outcome;

    outcome = done = 0;

    Greet();

    do
    {
        key_g = GetChar();
        switch (key_g)
        {
        case 0x59:
            PrintString(/*0x7916*/ "Yes\n\n\"We sell:\n\n");
            outcome = ProvisionMenu(member);
            done = 1;
            break;

        case U5_KEY_SPACE:
        case 0x4e:
            PrintString(/*0x7928*/ "No");
            done = 1;
            break;
        }
    } while (!done);

    ShoppesFarewell(outcome);
}

// Reagent shop: the choice-th reagent this shop sells (D_3a32 price, D_3a5a lot size), sales talk and sale. Returns -2 for a letter with no reagent, -1 when the party cannot pay.
// CHECKED
static int OfferReagent(int member, int choice, int outcome)
{
    int reagent;
    int listed;
    char key_g;

    listed = 0;
    for (reagent = 0; reagent < 8; reagent++)
    {
        if (reagentPrices[shopIndex][reagent] != 0)
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
        PrintString(/*0x792c*/ "\n\n\"Thou canst not carry any more!\"\n\n");
        GetChar();
        return outcome;
    }
    else
    {
        shopPrice = reagentPrices[shopIndex][reagent];
        shopPrice += shopPrice * -((s32)(party[member].intel * 3) - 100) / (u32)100;
        shopQuantity = reagentLots[shopIndex][reagent];

        PrintString(/*0x7952*/ "\n\n\"");
        PrintShopText((char*)&scratch[reagentTexts[reagent] - 0x1a67]); // 0x1a67: file load offset
        PrintString(/*0x7956*/ " Is this thy need?\" ");
        if (GetCurrentTextX() > 0xc)
        {
            PrintString(/*0x796c*/ "\n:");
        }

        do
        {
            key_g = GetChar();
            if (key_g == 'N')
            {
                PrintString(/*0x7970*/ "No\n\n\"What else?\n\n");
            }
            else if (key_g == 'Y')
            {
                PrintString(/*0x7982*/ "Yes\n");
                if (gold < shopPrice)
                {
                    PrintShopText((char*)&scratch[0x4c4]);
                    outcome = -1;
                }
                else
                {
                    gold -= shopPrice;
                    FalsehoodSurcharge();
                    UpdateVitalsDisplay();
                    IncreaseByte(&D_5850_reagents[reagent], shopQuantity, 99);
                    outcome = 1;
                    PrintShopText(/*0x7988*/ "\n\"I thank thee!\"\nsays $.\n");
                    PrintString(/*0x79a2*/ "\"Anything else?\n\n");
                }
            }
        } while (key_g != 'N' && key_g != 'Y');
    }

    // a8e7
    return outcome;
}
u8 reagentPrices[5][8] =
{
    0x00, 0x14, 0x12, 0x0c, 0x00, 0x00, 0x0c, 0x0d,
    0x0c, 0x10, 0x10, 0x08, 0x14, 0x00, 0x00, 0x00,
    0x0e, 0x10, 0x00, 0x00, 0x1e, 0x12, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x06, 0x08, 0x08, 0x0a, 0x0f,
    0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x1e, 0x28    
};

u8 reagentLots[5][8] =
{
    0x00, 0x0a, 0x06, 0x04, 0x00, 0x00, 0x01, 0x01,
    0x0c, 0x08, 0x08, 0x02, 0x04, 0x00, 0x00, 0x00,
    0x0e, 0x08, 0x00, 0x00, 0x06, 0x06, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x01
};

u16 itemBasePrices[0x30] =
{
    0x000f, 0x0032, 0x0078, 0x0096, 0x0028, 0x0046, 0x0078, 0x07d0,
    0x0000, 0x0014, 0x0032, 0x0064, 0x0096, 0x012c, 0x02bc, 0x0000,
    0x0001, 0x000a, 0x0005, 0x0005, 0x000f, 0x0007, 0x0003, 0x0028,
    0x0032, 0x003c, 0x004b, 0x000a, 0x0096, 0x000f, 0x0046, 0x0055,
    0x0096, 0x00c8, 0x00fa, 0x0000, 0x0320, 0x00fa, 0x03e8, 0x0000,
    0x0000, 0x0000, 0x01c2, 0x01f4, 0x00c8, 0x0384, 0x00f0, 0x0000
};

u8 armsStock[9][8] =
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

u16 greetTexts[8][4] =
{
    0x0000, 0x0000, 0x0000, 0x0000, 0x0d6c, 0x0da6, 0x0dd9, 0x0e14,
    0x1491, 0x14e1, 0x1513, 0x1557, 0x1697, 0x16d8, 0x1710, 0x174d,
    0x1a67, 0x1aaa, 0x1ae5, 0x1b25, 0x1f62, 0x1f92, 0x1fd1, 0x2026,
    0x223c, 0x2273, 0x22ab, 0x22df, 0x23e4, 0x241a, 0x245c, 0x2495
};

u16 farewellTexts[8][4] =
{
    0x0000, 0x0011, 0x0038, 0x0043, 0x0e44, 0x0e69, 0x0e85, 0x0ea1,
    0x159a, 0x15b4, 0x15be, 0x15c9, 0x1788, 0x179b, 0x17c7, 0x17d2,
    0x1b61, 0x1b73, 0x1b90, 0x1ba8, 0x205f, 0x2078, 0x2086, 0x209b,
    0x231e, 0x233c, 0x235e, 0x2381, 0x24f2, 0x250c, 0x2520, 0x2539
};

u16 farewellTexts1[8][4] =
{
    0x0057, 0x0064, 0x0076, 0x0083, 0x0ebc, 0x0ece, 0x0ee3, 0x0ef4,
    0x15ea, 0x160b, 0x161b, 0x1637, 0x17e6, 0x1807, 0x181d, 0x1832,
    0x1bbf, 0x1bda, 0x1bf5, 0x1c0f, 0x20bb, 0x20da, 0x20ee, 0x210e,
    0x231e, 0x233c, 0x235e, 0x2381, 0x2569, 0x2584, 0x25a2, 0x25be
};

u16 provisionPrices[4][4] =
{
    0x00be, 0x00ff, 0x000c, 0x0118, 0x00a0, 0x00c8, 0x000b, 0x00af,
    0x00b9, 0x00e1, 0x0019, 0x0109, 0x00aa, 0x0096, 0x0014, 0x012c
};

u16 provisionTexts[3] = { 0x212d, 0x216c, 0x21b9 };

u16 reagentTexts[8] = { 0x1c1c, 0x1c7c, 0x1cda, 0x1d3b, 0x1d8d, 0x1dee, 0x1e4e, 0x1ebd };

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
static int ReagentMenu(int member)
{
    bool done;
    int result_h;
    int outcome;
    byte letter_h;
    int reagent;
    byte key_g;

    outcome = done = 0;

    ReadFileFromDisk(/*0x7a0a*/ "SHOPPE.DAT", scratch, 1500, 6759); //offset: 0x1a67

    while (!done)
    {
        letter_h = 'A';
        reagent = 0;
        do
        {
            if (reagentPrices[shopIndex][reagent] != 0) // size = max_b114 * 8 + 8
            {
                PrintChar(letter_h++);
                PrintString(/*0x7a16*/ "...");
                PrintString(D_3c20_reagentNames[reagent]);
                PrintChar('\n');
            }
            reagent++;
        } while (reagent < 8);

        PrintString(/*0x7a1a*/ "\nThy interest?\" ");

        do
        {
            key_g = GetChar();

            switch (key_g)
            {
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
                result_h = OfferReagent(member, key_g - 0x41, outcome);
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
void Reagents(int member)
{
    int done;
    byte key_g;
    int outcome;
    
    outcome = done = 0;

    Greet();

    do
    {
        key_g = GetChar();
        switch (key_g)
        {
        case 0x59:
            PrintString(/*0x7a2c*/ "Yes\n\n\"Fine! We sell:\n\n");
            outcome = ReagentMenu(member);
            done = 1;
            break;

        case U5_KEY_SPACE:
        case 0x4e:
            PrintString(/*0x7a44*/ "No");
            done = 1;
            break;
        }
    } while (!done);

    ShoppesFarewell(outcome);
}

// The stables: sells a horse for D_3c30 (adjusted by intelligence), placed on a free tile next to the party. Closed when there is none.
// CHECKED (loop)
void Stables(int member)
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
        if (actors[i].tile == 0)
            break;
    }
    freeActor = i;

    for (i = 0; i < 4; i++)
    {
        x = stableDx[i] + D_5896_mapX;
        y = stableDy[i] + D_5897_mapY;
        tile = *GetTileAddr(x, y);

        if (FindActorTileAtPos(x, y, D_5895_mapLevel) == 0)
        {
            if (tile == TILE_MAP_44 || tile == TILE_MAP_45 || tile == TILE_MAP_GRASS)
                break;
        }
    }

    // aabf
    if (i == 4 || freeActor == 0x20)
    {
        PrintString(/*0x7a48*/ "The stables are closed.\n");
    }
    else
    {
        outcome = done = 0;

        shopPrice = horsePrices[shopIndex];
        shopPrice += (shopPrice * -((s32)(party[member].intel * 3) - 100) / (u32)100);

        Greet();
        do
        {
            key_g = GetChar();
            switch (key_g)
            {
            case 0x59:
                PrintString(/*0x7a62*/ "Yes\n\n\"");
                PrintShopFileText(5699); // 0x1643
                PrintString(/*0x7a6a*/ "\n\nDeal?\" ");

                do
                {
                    key_g = GetChar();
                    if (key_g == 'Y')
                        break;
                } while (key_g != 'N');

                if (key_g == 'N')
                {
                    PrintString(/*0x7a74*/ "No");
                    done = 1;
                }
                else
                {
                    PrintString(/*0x7a78*/ "Yes!");
                    if (gold < shopPrice)
                    {
                        PrintString(/*0x7a7e*/ "\n\n\"Thou couldst not afford to ");
                        PrintShopText(/*0x7a9e*/ "feed it!\"\nyells $.\n");
                        outcome = -1;
                    }
                    else
                    {
                        // abda
                        gold -= shopPrice;
                        FalsehoodSurcharge();

                        horse = &actors[freeActor];
                        horse->animState = horse->extra = horse->value = 0;
                        horse->tile = horse->animTile = TILE_ACTOR_HORSE;
                        horse->x = x;
                        horse->y = y;
                        horse->z = D_5895_mapLevel;

                        UpdateFrame();
                        UpdateVitalsDisplay();
                        outcome = 1;
                    }

                    done = 1;
                }
                break;

            case U5_KEY_SPACE:
            case 0x4e:
                PrintString(/*0x7ab2*/ "No");
                done = 1;
                break;
            }
        } while (!done);

        ShoppesFarewell(outcome);
    }
}
u16 horsePrices[4] = {0x0064, 0x0082, 0x00a0, 0x00be};

s16 stableDx[4] = {0x0000, 0x0000, 0x0001, 0xffff};

s16 stableDy[4] = {0x0001, 0xffff, 0x0000, 0x0000};

s16 armsTexts[0x2f] =
{
    0x0095, 0x00d0, 0x011f, 0x0152, 0x0192, 0x01cb, 0x0209, 0x024d,
    0x0000, 0x02b2, 0x030a, 0x0342, 0x0376, 0x03c6, 0x0410, 0x0000,
    0x0465, 0x04a1, 0x04de, 0x0521, 0x0560, 0x05ab, 0x05f6, 0x0638,
    0x066e, 0x06b0, 0x06ee, 0x0721, 0x0742, 0x077c, 0x07a4, 0x07e7,
    0x082f, 0x0872, 0x08bb, 0x0000, 0x090e, 0x0959, 0x09a0, 0x0000,
    0x0000, 0x0000, 0x09f7, 0x0a60, 0x0ac2, 0x0b19, 0x0b78
};

char* buyOffers[4] =
{
    /*0x7ab6*/ "Wouldst thou buy one?",
    /*0x7acc*/ "Wilt thou take it?",
    /*0x7ae0*/ "Wish ye it?",
    /*0x7aec*/ "May I get one for thee?"
};

char* cantPayInsults[4] =
{
    /*0x7b04*/ "Can't pay?! Out with ye, orc-face!",
    /*0x7b28*/ "What be ye trying to pull? OUT!",
    /*0x7b48*/ "OUT, SLIME!",
    /*0x7b54*/ "BEAT IT!",
};

// Arms shop: price of an item (D_3a82), its sales talk, and the sale (at most 99 of an item, ammunition is set to 99). Returns 1 sold, -1 when the party cannot pay.
static int OfferArm(int member, int item, int outcome)
{
    byte key;

    shopPrice = itemBasePrices[item];
    shopPrice += (shopPrice * -((s32)(party[member].intel * 3) - 100) / (u32)100);

    PrintString(/*0x7b5e*/ "\n\n\"");
    PrintShopFileText(armsTexts[item]);
    PrintString(/*0x7b62*/ "\n\n");
    PrintString(buyOffers[RandomRange(0, 3)]);
    PrintString(/*0x7b66*/ "\" ");

    do
    {
        key = GetChar();
        if (key == 'N')
        {
            PrintString(/*0x7b6a*/ "No\n\n");
        }
        else if (key == 'Y')
        {
            PrintString(/*0x7b70*/ "Yes\n");
            if (equipment[item] == 99)
            {
                PrintString(/*0x7b76*/ "\n\"Thou canst not carry any more!\"\n");
                PrintShopText(/*0x7b9a*/ "says $.\n\n");
                GetChar();
            }
            else if (gold < shopPrice)
            {
                PrintString(/*0x7ba4*/ "\n\"");
                PrintString(cantPayInsults[RandomRange(0, 3)]);
                PrintShopText(/*0x7ba8*/ "\"\nyells $.\n");
                outcome = -1;
            }
            else
            {
                gold -= shopPrice;
                FalsehoodSurcharge();
                UpdateVitalsDisplay();
                if (item == 0x1b || item == 0x1d)
                {
                    equipment[item] = 99;
                }
                else
                {
                    IncreaseByte(item + equipment, 1, 99);
                }

                PrintShopText(/*0x7bb4*/ "\nSold!\n");
                outcome = 1;
            }
        }
    } while (key != 'N' && key != 'Y');

    if (outcome != -1)
    {
        PrintString(/*0x7bbc*/ "\"Anything else,\n");
        if (outcome != 0)
        {
            if (party[member].gender == 12)
            {
                PrintString(/*0x7bce*/ "milady?");
            }
            else
            {
                PrintString(/*0x7bd6*/ "sir?");
            }
        }
        else
        {
            PrintString(/*0x7bdc*/ "then?");
        }
    }

    return outcome;
}

char* showPrompts[4] =
{
    /*0x7be2*/ "What may I show thee?",
    /*0x7bf8*/ "Which wouldst thou like to see?",
    /*0x7c18*/ "What is thine interest?",
    /*0x7c30*/ "Which would ye see?",
};

// Arms shop: the lettered list of what this shop stocks (D_3ae2); the purchases. Returns the outcome for the farewell.
// CHECKED
static int BuyMenu(int member)
{
    bool done;
    int item;
    int rows;
    int outcome;
    byte key_g;

    outcome = done = 0;

    do
    {
        PrintString(/*0x7c44*/ "\n\n");

        for (rows = 0; rows < 8; rows++)
        {
            item = armsStock[shopIndex][rows];
            if (item == 0xff)
                break;

            // ae5c
            PrintChar((uint)rows + 0x61);
            PrintString(/*0x7c48*/ "...");

            if (StringLength(equipNames[item]) < 0xd)
            {
                PrintString(equipNames[item]);
            }
            else
            {
                PrintString(equipShortNames[item]);
            }

            if (GetCurrentTextX() != 0)
            {
                PrintChar('\n');
            }
        }

        PrintChar('\n');
        PrintString(showPrompts[RandomRange(0, 3)]);
        PrintString(/*0x7c4c*/ "\" ");

        do
        {
            // ae1b
            key_g = GetChar();
            item = key_g - 0x41; // - 'A'
            if (item < rows && item >= 0)
            {
                PrintChar((uint)item + 0x61);
                outcome = OfferArm(member, armsStock[shopIndex][item], outcome);
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
static int HasEquipment(void)
{
    int item;

    for (item = 0; item < 0x30; item++)
    {
        if (equipment[item] != 0)
        {
            return 1;
        }
    }

    return 0;
}

// Arms shop, selling: scroll the party's equipment list so the cursor item is visible and draw it (cursor inverted, arrows at the bottom). Returns the cursor's row.
static int DrawSellList(int* first, int* cursor)
{
    int row;
    int arrows;
    int item;
    int cursorRow;

    if (equipment[*cursor] == 0)
    {
        if (*first == *cursor)
        {
            *cursor = PrevItem(*cursor, 0x30, equipment, 0xff);
            if (*cursor == -1)
            {
                *cursor = NextItem(-1, 0x30, equipment, 0xff);
            }

            *first = *cursor;
        }
        else
        {
            *cursor = NextItem(*cursor, 0x30, equipment, 0xff);
            if (*cursor == -1)
            {
                *cursor = PrevItem(0x30, 0x30, equipment, 0xff);
                if (PrevItem(*first, 0x30, equipment, 0xff) > -1)
                {
                    *first = PrevItem(*first, 0x30, equipment, 0xff);
                }
            }
        }
    }

    SelectTextWindow(1);
    SetTextPosition(1, 1);

    item = *first;
    while (item != -1)
    {
        if (*cursor == item)
        {
            PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            cursorRow = GetCurrentTextY();
        }

        PrintItemRow(item, equipment, equipShortNames, 0x2d);

        if (*cursor == item)
        {
            PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
        }

        SetTextPosition(1, GetCurrentTextY());
        if (GetCurrentTextY() == 5)
            break;
        
        item = NextItem(item, 0x30, equipment, 0xff);
    }

    for (row = GetCurrentTextY(); row != 5; row = row + 1)
    {
        SetTextPosition(1, row);
        PrintString(/*0x7c50*/ "             ");
    }

    arrows = 0;
    if (PrevItem(*first, 0x30, equipment, 0xff) != -1)
    {
        arrows = 2;
    }

    if (item != -1 && NextItem(item, 0x30, equipment, 0xff) != -1)
    {
        arrows++;
    }

    if (arrows != 0)
    {
        SetTextPosition(6, 6);
        PrintLeftCap();
    }

    switch (arrows)
    {
    case 0:
        DrawStatsDivider();
        break;
    case 1:
        PrintChar(0x19);
        PrintRightCap();
        break;
    case 2:
        PrintChar(0x18);
        PrintRightCap();
        break;
    case 3:
        PrintChar(0x12);
        PrintRightCap();
        break;
    }

    SelectTextWindow(2);

    return cursorRow;
}
u16 buybackTexts[8] = {0x0bc0, 0x0bff, 0x0c2e, 0x0c55, 0x0ca1, 0x0ce8, 0x0d12, 0x0d3d};

char* shopItemNames[0x30] =
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
static int SellItem(int member, int item)
{
    byte key;

    if (item == 0x1b || item == 0x1d)
    {
        PrintShopText(/*0x7d32*/ "\n\n\"We don't deal in used ammunition!\"\ngrowls $.\n");
        return 1;
    }
    else
    {
        PrintString(/*0x7d64*/ "\n\n\"");
        if (itemBasePrices[item] != 0)
        {
            shopPrice = (party[member].intel * itemBasePrices[item] * (u32)3) / (u32)100 + 1;

            itemName = shopItemNames[item];
            if (shopItemNames[item] == 0)
            {
                itemName = equipNames[item];
            }

            //PrintShopText(buybackTexts[iVar5] + 0xa65e);
            PrintShopText((char*)&scratch[buybackTexts[RandomRange(0, 7)] + 0xa65e - 0xb21e]);
            PrintString(/*0x7d68*/ "\n\nDeal?\" ");
            do
            {
                key = GetChar();
            } while (key != 'N' && key != 'Y');

            if (key == 'N')
            {
                PrintString(/*0x7d72*/ "No");
            }
            else
            {
                PrintShopText(/*0x7d76*/ "Yes\n\n\"Done!\"\nsays $.");
                IncreaseInt(&gold, shopPrice, 9999);
                DecreaseByte(&equipment[item], 1);
            }
            UpdateGoldDisplay();
        }
        else
        {
            PrintShopText(/*0x7d8c*/ "That, I cannot buy from thee.\"\nsays $.");
        }

        return 0;
    }
}

char* sellPrompts[4] =
{
    /*0x7db4*/ "Which item wouldst thou like to sell?",
    /*0x7dda*/ "What dost thou wish to sell?",
    /*0x7df8*/ "Show me what ye got...",
    /*0x7e10*/ "What dost thou have for me to buy?"
};

char* farewells[4] =
{
    /*0x7e34*/ "Good-bye...",
    /*0x7e40*/ "Mayhap another time...",
    /*0x7e58*/ "Godspeed...",
    /*0x7e64*/ "Fare thee well..."
};

char* sellMorePrompts[4] =
{
    /*0x7e76*/ "What else can ye offer me?",
    /*0x7e92*/ "What else hath ye to sell?",
    /*0x7eae*/ "What else doth thou wish to sell?",
    /*0x7ed0*/ "What other arms wilt thou sell?"
};

// Arms shop, selling: the cursor list of the party's equipment (arrows, Enter, Esc) and the sales.
// CHECKED (di)
static void SellMenu(int member)
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

    if (HasEquipment() != 0)
    {
        PrintString(sellPrompts[RandomRange(0, 3)]);
        PrintString(/*0x7ef0*/ "\" ");
        ShowStatsTitle(/*0x7ef4*/ "Arms");
        SelectTextWindow(1);
        SetTextWindowSize(1, 0x18, 1, 0x26, 6);
        PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        SetTextWindowSize(1, 0x18, 1, 0x27, 9);
        PrintChar(0x10);

        for (i = 0; i < 0xd; i++)
        {
            PrintChar(0x11);
        }

        PrintChar(0x13);

        for (i = 1; i < 5; i++)
        {
            SetTextPosition(0, i);
            PrintChar(0x17);
            SetTextPosition(0xe, i);
            PrintChar(0x17);
        }

        PrintChar('\n');
        PrintChar(0x14);

        for (i = 0; i < 0xd; i++)
        {
            PrintChar(0x15);
        }

        PrintChar(0x16);

        cursor = first_h = NextItem(-1, 0x30, equipment, 0xff);
        cursorRow = DrawSellList(&first_h, &cursor);
        done = sold_m = 0;

        ReadFileFromDisk(/*0x7efa*/ "SHOPPE.DAT", scratch, 1500, 3008); // offset: 0xbc0

        // b317
        while (done == 0)
        {
            // b320
            key_g = GetChar();

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
                    neighbor = PrevItem(cursor, 0x30, equipment, 0xff);

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
                cursorRow = DrawSellList(&first_h, &cursor);
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
                    neighbor = NextItem(cursor, 0x30, equipment, 0xff);
                    if (neighbor > -1)
                    {
                        cursor = neighbor;
                        if (++cursorRow > 4)
                        {
                            first_h = NextItem(first_h, 0x30, equipment, 0xff);
                        }
                    }
                }

                cursorRow = DrawSellList(&first_h, &cursor);
                break;

            case U5_KEY_HOME:
                // b41a
                cursor = first_h = NextItem(-1, 0x30, equipment, 0xff);
                cursorRow = DrawSellList(&first_h, &cursor);
                break;

            case U5_KEY_END:
                // b436
                cursor = first_h = PrevItem(0x30, 0x30, equipment, 0xff);
                for (i = 1; i < 4; i++)
                {
                    neighbor = PrevItem(first_h, 0x30, equipment, 0xff);
                    if (neighbor == -1)
                        break;

                    first_h = neighbor;
                }

                cursorRow = DrawSellList(&first_h, &cursor);
                break;

            case U5_KEY_ENTER:
            case U5_KEY_SPACE:
                // b47a
                sold_m = SellItem(member, cursor);
                if (sold_m == 0 && HasEquipment() != 0)
                {
                    cursorRow = DrawSellList(&first_h, &cursor);
                    PrintString(/*0x7f06*/ "\n\n\"");
                    PrintString(sellMorePrompts[RandomRange(0, 3)]);
                    PrintString(/*0x7f0a*/ "\" ");
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
        UpdateVitalsDisplay();
        ClearStatsTitle();

        if (sold_m != 0)
        {
            return;
        }

        PrintString(/*0x7f0e*/ "\n\n\"");
        PrintString(farewells[RandomRange(0, 3)]);
        PrintString(/*0x7f12*/ "\"\n");

        if (HasEquipment() == 0)
        {
            return;
        }

        PrintShopText(/*0x7f16*/ "says $.\n");
    }
    else
    {
        PrintShopText(/*0x7f20*/ "Thou hast nothing to sell!\"\ngrowls $.\n");
    }
}
char* buyOrSellGreetings[2] =
{
    /*0x7f48*/ "Hail, friend! Wouldst thou Buy or Sell?",
    /*0x7f70*/ "Greetings, traveller! Wish ye to Buy, or hast thou wares to Sell?",
};

char* goodReplies[4] =
{
    /*0x7fb2*/ "Very good!\n",
    /*0x7fbe*/ "Excellent!\n",
    /*0x7fca*/ "Fine, fine!\n",
    /*0x7fd8*/ "But of course!\n"
};

char* stockIntros[4] =
{
    /*0x7fe8*/ "We have:",
    /*0x7ff2*/ "We stock:",
    /*0x7ffc*/ "Thou canst buy:",
    /*0x800c*/ "We've got:"
};

// the reply to buying a spell already known: an array, so in _DATA (DS:3d5a)
char noNeedOfArt[] = "\n\n\"Thou hast no need of this art!\"\nsays $.";

u8 healPrices[8] = {0x23, 0x28, 0x2d, 0x32, 0x37, 0x3c, 0x41, 0x46};

u8 curePrices[8] = {0x14, 0x19, 0x1e, 0x23, 0x28, 0x0f, 0x0a, 0x01};

u16 raisePrices[8] = {0x00c8, 0x00d7, 0x00e1, 0x00ed, 0x00f7, 0x00f9, 0x0106, 0x010e};

// The shop of arms and armour: greeting, then Buy or Sell, farewell.
void Armourer(int member)
{
    int outcome;
    byte key;

    outcome = 0;

    PrintShopText(/*0x8018*/ "\"Good @, and welcome to #!\"\n");
    GetChar();
    PrintShopText(/*0x8036*/ "\n$ says,\n\"");

    PrintString(buyOrSellGreetings[RandomRange(0, 1)]);
    PrintString(/*0x8042*/ "\" ");

    do
    {
        key = GetChar();
        switch (key)
        {
        case 0x42:
            PrintString(/*0x8046*/ "Buy\n\n\"");
            PrintString(goodReplies[RandomRange(0, 3)]);
            PrintString(stockIntros[RandomRange(0, 3)]);
            outcome = BuyMenu(member);
            break;
        case 0x53:
            PrintString(/*0x804e*/ "Sell\n\n\"");
            SellMenu(member);
            outcome = -1;
            break;
        case U5_KEY_SPACE:
            PrintString(/*0x8056*/ "No");
            break;
        }
    } while (key != 'B' && key != 'S' && key != U5_KEY_SPACE);

    ShoppesFarewell(outcome);
}

// Healer: "Who needs my aid?"; the one member of a party of one, -1 for no one.
static int ChoosePatient(void)
{
    int patient;

    if (partyCount == 1)
    {
        patient = 0;
    }
    else
    {
        PrintString(/*0x805a*/ "\n\n\"Who needs my aid?\" ");
        patient = SelectPlayer();
        if (patient == -1)
        {
            PrintString(/*0x8072*/ "No one");
        }
    }

    return patient;
}

// Healer: the screen flashes (red, white) with sounds as the spell is cast.
static void HealingFlash(void)
{
    GRAP_2d_SetPenColor(redColor);
    GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    AudioPulse(0x100e, 1, 0x57e4, 5000, 1);
    AudioPulse(0x100e, 1, 0x57e4, 0x6b6c, -1);
    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    AudioPulse(0x11b2, 1, 40000, 1, 1);
    AudioPulse(0x11b2, 1, 40000, 40000, -1);
    GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    AudioPulse(0x8fc, 1, 18000, 1, 2);
    AudioPulse(0x8fc, 1, 18000, 36000, -2);
}

// Healer: "for N gold. Wilt thou pay?"; takes the gold. Returns 1 when refused or when the party is too poor.
// CHECKED (si)
static int AskPayment(void)
{
    bool refused;
    int key;

    PrintShopText(/*0x807a*/ "for % gold.\n\nWilt thou\npay?\" ");
    do
    {
        // b708
        key = GetChar();
        if ((uint)key == 'Y')
        {
            PrintString(/*0x8098*/ "Yes");
        }
        else if ((uint)key == 'N')
        {
            PrintString(/*0x809c*/ "No");
        }

        // b726
    } while ((uint)key != 'Y' && (uint)key != 'N');

    // b732
    refused = key == 'N';

    // b742
    if (refused == 0 && gold < shopPrice && (shopPrice > 100 || mapId != 7))
    {
        // b75e
        PrintShopFileText(9131); // 0x23ab
        refused = 1;
    }

    // b76a
    if (refused == 0 && gold >= shopPrice)
    {
        gold -= shopPrice;
        FalsehoodSurcharge();
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
void Healer(int unused)
{
    int done_j;
    byte key_g;
    int patient;

    Greet();

    done_j = 0;
    while (!done_j)
    {
        do
        {
            key_g = GetChar();
            if (key_g == 'N')
            {
                PrintString(/*0x80a0*/ "No");
            }
            else if (key_g == 'Y')
            {
                PrintString(/*0x80a4*/ "Yes\n\n");
            }
        } while (key_g != 'N' && key_g != 'Y');

        if (key_g == 'N')
            break;

        PrintString(/*0x80aa*/ "\"We have powers to Cure, Heal, or Resurrect.\"\n");
        PrintShopText(/*0x80da*/ "says $.\n\n\"What is the nature of thy need?\" ");

        do
        {
            key_g = GetChar();
        } while (key_g != 'C' && key_g != 'H' && key_g != 'R' &&
                 key_g != U5_KEY_SPACE && key_g != U5_KEY_ENTER);

        switch (key_g)
        {
        case 0x43:
            // b826
            PrintString(/*0x8106*/ "Curing");
            patient = ChoosePatient();
            if (patient == -1)
                goto L_b838;
            {
                // b856
                if (party[patient].status != STATUS_POISONED)
                {
                    // b864
                    PrintShopText(noNeedOfArt);
                }
                else
                {
                    // b86e
                    PrintString(/*0x810e*/ "\n\n\"");
                    if (mapId == 5)
                    {
                        PrintString(/*0x8112*/ "Receive now the Light!\"");
                        // -> b8a1
                        goto L_b8a1;
                    }
                    else
                    {
                        // b886
                        PrintString(/*0x812a*/ "I can cure thy poisoned body ");
                        shopPrice = curePrices[shopIndex];
                        if (AskPayment() == 0)
                        {
                            // b8a1
L_b8a1:
                            HealingFlash();
                            party[patient].status = STATUS_GOOD;
                        }
                    }
                }
            }
            break;
        case 0x48:
            // b8b2
            PrintString(/*0x8148*/ "Healing");
            patient = ChoosePatient();
            if (patient != -1)
            {
                // b8c7
                if (party[patient].status != STATUS_DEAD && party[patient].hp != party[patient].maxHp)
                {
                    PrintString(/*0x8150*/ "\n\n\"");
                    if (mapId == 5)
                    {
                        PrintString(/*0x8154*/ "Receive now the Light!\"");
                        // -> b914
                        goto L_b914;
                    }
                    else
                    {
                        // b8f6
                        PrintString(/*0x816c*/ "I can heal thee ");
                        shopPrice = healPrices[shopIndex];
                        if (AskPayment() == 0)
                        {
                            // b914
L_b914:
                            HealingFlash();
                            party[patient].hp = party[patient].maxHp;
                        }
                    }
                }
                else
                {
                    // -> b864
                    PrintShopText(noNeedOfArt);
                }
            }
            break;
        case 0x52:
            // b92a
            PrintString(/*0x817e*/ "Resurrect");
            patient = ChoosePatient();
            if (patient != -1)
            {
                if (party[patient].status == STATUS_DEAD)
                {
                    PrintString(/*0x8188*/ "\n\n\"");
                    PrintString(/*0x818c*/ "I can raise this unfortunate person from ");
                    PrintString(/*0x81b6*/ "the dead ");

                    shopPrice = raisePrices[shopIndex];

                    if (AskPayment() == 0)
                    {
L_b9c1:
                        HealingFlash();
                        Resurrect(patient, 0xff);
                        party[patient].hp = party[patient].maxHp;
                        UpdateVitalsDisplay();
                    }
                }
                else
                {
                    PrintShopText(noNeedOfArt);
                }
            }
            break;
        case U5_KEY_ENTER:
        case U5_KEY_SPACE:
            // b99e
            PrintString(/*0x81c0*/ "Nothing");
            done_j = 1;
            break;
        default:
            goto L_b838;
        }
        goto L_b838;

L_b838:
        if (!done_j)
        {
            UpdateVitalsDisplay();
            PrintString(/*0x81c8*/ "\n\n\"Is there any other way in which I may\n");
            PrintString(/*0x81f2*/ "aid thee?\" ");
        }
    }

    ShoppesFarewell(1);
}
