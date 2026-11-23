#include "common/common.h"

// this module's communal variables (vars.h, DS:a9bc)
u8 pubKind;
s16 boughtSomething; // int?
int dinersFed; // int
int dinerCount; // int
int tableTile; // int
s16 drinksBought; // int?

#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "shoppes.h"
#include "shoppes2.h"

extern char inputText[16];

// Pub: count the living party members (the diners and the food they get); the price is pricePerHead times that. Prints two newlines.
static void CountDiners(short pricePerHead)
{
    int member;

    dinersFed = dinerCount = shopPrice = 0;

    for (member = 0; member != partyCount; member++)
    {
        if (party[member].status != STATUS_DEAD)
        {
            dinerCount++;
            dinersFed++;
            shopPrice += pricePerHead;
        }
    }

    PrintChar('\n');
    PrintChar('\n');
}

// Pub: print the number of diners in words ("two" to "six").
static void PrintDinerCount(void)
{
    switch (dinerCount)
    {
    case 2:
        PrintString(/*0x9aa8*/ "two");
        break;

    case 3:
        PrintString(/*0x9aac*/ "three");
        break;

    case 4:
        PrintString(/*0x9ab2*/ "four");
        break;

    case 5:
        PrintString(/*0x9ab8*/ "five");
        break;

    case 6:
        PrintString(/*0x9abe*/ "six");
        break;
    }
}

// Pub: print "sir" or "milady" after the gender of party member 0 (and clear the active player).
// CHECKED
// The original stores 0xff through AL (mov al,0FFh; mov [587B],al), MSC's form for a
// chained assignment (DUNGEON and ULTIMA.EXE have curCombatant = activePlayer = 0xff). Here the chain
// names activePlayer twice, probably two names of the same byte in the original.
// The label keeps MSC from folding local_4 = 0 into the index, as the original doesn't.
static void PrintHonorific(void)
{
    int member;

    activePlayer = activePlayer = 0xff; // two stores to one byte between sequence points: DOS build only
    member = 0;
L_e297:
    if (party[member].gender == 0xb)
    {
        PrintString(/*0x9ac2*/ "sir");
    }
    else
    {
        PrintString(/*0x9ac6*/ "milady");
    }
}

// Pub: "That will be N gold for the .. of ye": take the gold and give each living member food; the table gets its dish. Returns 1 when the party cannot pay and is thrown out.
static int ChargeForMeal(void)
{
    PrintChar('\"');
    PrintString(/*0x9ace*/ "That will be ");
    PrintNumber(shopPrice, 1, 0x20);
    PrintString(/*0x9adc*/ " gold for the ");
    PrintDinerCount();
    PrintString(/*0x9aec*/ " of ye,\n");
    PrintHonorific();
    PrintChar('.');

    if (shopPrice > gold)
    {
        PrintString(/*0x9af6*/ "\"\n\n\"CAN'T PAY?\nBeat it!\"\nyells ");
        PrintString(shopOwner);
        PrintChar('.');
        PrintChar('\n');
        return 1;
    }

    gold -= shopPrice;
    FalsehoodSurcharge();

    if (dinersFed != 0)
    {
        IncreaseInt(&food, dinersFed, 9999);

        if (*GetTileAddr(D_5896_mapX, D_5897_mapY - 1) == TILE_MAP_TABLE_95)
        {
            *GetTileAddr(D_5896_mapX, D_5897_mapY - 1) = TILE_MAP_TABLE_9B;
            tableTile = TILE_MAP_TABLE_9B;

            UpdateFrame();
        }
        else if (*GetTileAddr(D_5896_mapX, D_5897_mapY + 1) == TILE_MAP_TABLE_95)
        {
            *GetTileAddr(D_5896_mapX, D_5897_mapY + 1) = TILE_MAP_TABLE_9A;
            tableTile = TILE_MAP_TABLE_9A;

            UpdateFrame();
        }
    }
    else
    {
        drinksBought++;
    }

    PrintString(/*0x9b16*/ "\nEnjoy!\"\n\n");
    return 0;
}

// Pub: the party orders a meal (the pub kind's food key), price per head from mealPrices.
static int OrderFood(void)
{
    PrintChar((byte)foodKeys[pubKind]);
    CountDiners(mealPrices[shopIndex]);
    return ChargeForMeal();
}

// Pub: order a drink (ale; the wine list in a wine house). After three the keeper refuses; returns 1 when thrown out, 2 when the keeper's story follows.
static int OrderDrink(void)
{
    byte key;

    PrintChar((byte)drinkKeys[pubKind]);
    if (drinksBought == 3)
    {
        PrintString(/*0x9b22*/ "\n\n\"I beg thy\npardon, ");
        PrintHonorific();
        PrintString(/*0x9b38*/ ",\"\nsays ");
        PrintString(shopOwner);
        PrintString(/*0x9b42*/ ".\n\"But haven't\nye had enough\nto drink?\" ");
        do
        {
            key = GetChar();
            if (key == 'Y')
            {
                PrintString(/*0x9b6c*/ "Yes\n\n");
                return 0;
            }
            if (key == 'N')
            {
                PrintString(/*0x9b72*/ "No!");
                drunkTurns = 0x19;
                DecreaseByte(&karma, 1);
            }
        } while (key != 'Y' && key != 'N');
    }

    CountDiners(1);
    if (drinkKeys[pubKind] == 'W')
    {
        PrintString(/*0x9b76*/ "\"Our wine list,\n");
        PrintHonorific();
        PrintString(/*0x9b88*/ ".\n\n");
        PrintString(/*0x9b8c*/ "a) Rose.......18\n");
        PrintString(/*0x9b9e*/ "b) Claret....192\n");
        PrintString(/*0x9bb0*/ "c) Sauterne...79\n");
        PrintString(/*0x9bc2*/ "d) Muscatel...30\n");
        PrintString(/*0x9bd4*/ "e) Moselle...275\n");
        PrintString(/*0x9be6*/ "f) Chablis....98\n\n");
        PrintString(/*0x9bfa*/ "Thy choice?\" ");

        do
        {
            key = GetChar();
            if (key == U5_KEY_SPACE)
            {
                PrintChar('\n');
                PrintChar('\n');
                if (key == U5_KEY_SPACE)
                {
                    PrintShopFileText(0x1413);
                    return 2;
                }
                else
                {
                    return 0;
                }
            }
        } while (key < 0x41 || 0x46 < key);

        PrintChar((byte)(char)key); // TODO: ???
        PrintString(/*0x9c08*/ "\n\n\"Ah, a fine\nchoice, ");
        PrintHonorific();
        PrintChar('.');

        key -= 0x41;

        if (gold < (s16)winePrices[key])
        {
            PrintString(/*0x9c20*/ "\"\n\n\"CAN'T PAY?\nBeat it!\"\nyells ");
            PrintString(shopOwner);
            PrintChar('.');
            PrintChar('\n');
            return 1;
        }
        else
        {
            gold -= winePrices[key];
            FalsehoodSurcharge();
            drinksBought++;
            //key = 0x40;
            PrintString(/*0x9c40*/ "\nEnjoy!\"");
            PrintChar('\n');
            PrintChar('\n');
            if (key == 0x20)
            {
                PrintShopFileText(0x1413);
                return 2;
            }
            else
            {
                return 0;
            }
        }
    }
    else
    {
        dinersFed = 0;
        return ChargeForMeal();
    }
}

// Pub: buy rations (25 food each); the talker's intelligence lowers the price. Returns 1 when thrown out.
static int BuyRations(int member)
{
    int remaining;
    int wanted;

    PrintChar((byte)rationKeys[pubKind]);
    PrintChar('\n');
    PrintChar('\n');

    shopPrice = rationPrices[shopIndex];
    shopPrice += shopPrice * -((s32)(party[member].intel * 3) - 100) / (u32)100;

    PrintChar('\"');
    PrintShopFileText(rationTexts[RandomRange(0, 6)]);
    PrintString(/*0x9c4a*/ "\n\nHow many wouldst\nthou like?\" ");
    wanted = remaining = GetNumber(2);
    if (remaining == 0)
    {
        PrintString(/*0x9c6a*/ "\n\n\"Hrumph.\"");
        return 2;
    }

    for (; remaining != 0; remaining--)
    {
        // e61f
        if (gold >= shopPrice)
        {
            DecreaseInt(&gold, shopPrice);
            IncreaseInt(&food, 0x19, 9999);

            if (food == 9999)
                break;
        }
        else
        {
            wanted -= remaining;
            // e654
            PrintString(/*0x9c76*/ "\n\n");
            if (wanted == 0)
            {
                if (food < 3)
                {
                    IncreaseInt(&food, RandomRange(0, 1) + 1, 9999);
                    PrintShopFileText(0x143f);
                    return 1;
                }
                else
                {
                    PrintString(/*0x9c7a*/ "\"Thou hast\nneither gold nor\nneed! Out!\"\n");
                    PrintString(/*0x9ca4*/ "yells ");
                    PrintString(shopOwner);
                    PrintString(/*0x9cac*/ ".\n");
                    return 1;
                }
            }
            else
            {
                // e6b6
                PrintString(/*0x9cb0*/ "\"Thou canst\nafford only ");
                PrintNumber(wanted, 1, 0x20);
                PrintString(/*0x9cca*/ "!\"\n\n");
                return 0; // -> e689
            }
        }
    }

    // e6d4
    PrintString(/*0x9cd0*/ "\n\n");
    FalsehoodSurcharge();
    return 0;
}

char foodKeys[5] = /*0x4c1e*/ "MCBF";

char drinkKeys[5] = /*0x4c24*/ "AWRS";

char rationKeys[5] = /*0x4c2a*/ "R  P";

char gossipKeys[5] = /*0x4c30*/ "CTHA";

u16 mealPrices[9] = {0x0003, 0x0004, 0x0005, 0x0003, 0x0002, 0x0005, 0x0003, 0x0004, 0x0005};

u16 winePrices[6] = {0x0012, 0x00c0, 0x004f, 0x001e, 0x0113, 0x0062};

u16 rationPrices[9] = {0x000a, 0x000f, 0x0014, 0x0019, 0x001e, 0x0019, 0x0014, 0x0019, 0x001e};

u16 rationTexts[7] = {0x1198, 0x11d7, 0x121f, 0x1271, 0x1299, 0x12e3, 0x130c};

char* pubTopicWords[26] =
{
    /*0x9cd4*/ "hone",
    /*0x9cda*/ "comp",
    /*0x9ce0*/ "valo",
    /*0x9ce6*/ "just",
    /*0x9cec*/ "sacr",
    /*0x9cf2*/ "hono",
    /*0x9cf8*/ "spir",
    /*0x9cfe*/ "humi",
    /*0x9d04*/ "dece",
    /*0x9d0a*/ "desp",
    /*0x9d10*/ "dest",
    /*0x9d16*/ "wron",
    /*0x9d1c*/ "cove",
    /*0x9d22*/ "sham",
    /*0x9d28*/ "hyth",
    /*0x9d2e*/ "crow",
    /*0x9d34*/ "scep",
    /*0x9d3a*/ "amul",
    /*0x9d40*/ "fals",
    /*0x9d46*/ "hatr",
    /*0x9d4c*/ "cowa",
    /*0x9d52*/ "astr",
    /*0x9d58*/ "oppr",
    /*0x9d5e*/ "brit",
    /*0x9d64*/ "resi",
    /*0x9d6a*/ "unde"
};

char* pubPersons[26] =
{
    /*0x9d70*/ "Malik",
    /*0x9d76*/ "Greyson",
    /*0x9d7e*/ "Trian",
    /*0x9d84*/ "Jeremy",
    /*0x9d8c*/ "Rew",
    /*0x9d90*/ "Gruman",
    /*0x9d98*/ "Saul",
    /*0x9d9e*/ "Shirita",
    /*0x9da6*/ "Malifora",
    /*0x9db0*/ "Annon",
    /*0x9db6*/ "Trian",
    /*0x9dbc*/ "Felespar",
    /*0x9dc6*/ "the mother of Rew",
    /*0x9dd8*/ "Sindar",
    /*0x9de0*/ "Kaiko",
    /*0x9de6*/ "Terrance",
    /*0x9df0*/ "Greymarch",
    /*0x9dfa*/ "Simon and Tessa",
    /*0x9e0a*/ "Shalineth",
    /*0x9e14*/ "a daemon",
    /*0x9e1e*/ "Lord Malone",
    /*0x9e2a*/ "Zachariah",
    /*0x9e34*/ "Tactus",
    /*0x9e3c*/ "a daemon",
    /*0x9e46*/ "Terrance",
    /*0x9e50*/ "Jotham"
};

u8 gossipPlaceIdx[26] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x01,
    0x03, 0x09, 0x0b, 0x0a, 0x0c, 0x00, 0x04, 0x0a,
    0x01, 0x08
};

char* pubPlaces[13] =
{
    /*0x9e58*/ "Moonglow",
    /*0x9e62*/ "Britain",
    /*0x9e6a*/ "Jhelom",
    /*0x9e72*/ "Yew",
    /*0x9e76*/ "Minoc",
    /*0x9e7c*/ "Trinsic",
    /*0x9e84*/ "Skara Brae",
    /*0x9e90*/ "New Magincia",
    /*0x9e9e*/ "a lighthouse south of Britain",
    /*0x9ebc*/ "a hidden mountain keep",
    /*0x9ed4*/ "the desert",
    /*0x9ee0*/ "the Lycaeum",
    /*0x9eec*/ "Serpent's Hold"
};

u16 gossipPrices[0x1a] =
{
    0x0032, 0x004b, 0x0032, 0x0032, 0x004b, 0x004b, 0x0019, 0x0032,
    0x0064, 0x0096, 0x004b, 0x0096, 0x004b, 0x0064, 0x0064, 0x00c8,
    0x00c8, 0x00c8, 0x00fa, 0x00fa, 0x00fa, 0x0064, 0x0032, 0x0032,
    0x00c8, 0x0064
};

u16 gossipTexts[4] = {0x13a2, 0x13ae, 0x13d9, 0x13f3};

u8 pubKindByShop[0xa] = {0x00, 0x00, 0x00, 0x02, 0x03, 0x01, 0x00, 0x02, 0x00, 0x00};

u16 pubOrderTexts[4] = {0x0f09, 0x0f5a, 0x0f9e, 0x0fdf};

u16 pubMoreTexts[4] = {0x1042, 0x109a, 0x10ea, 0x1146};

// Pub: the keeper sells gossip: asks for a topic word (pubTopicWords), names a person and a place (pubPersons, pubPlaces) for a price. Returns 1 when the party cannot pay.
// CHECKED
static int BuyGossip(void)
{
    int matchPos;
    int index_h;
    int topic_b;

    PrintChar((byte)gossipKeys[pubKind]);
    PrintString(/*0x9efc*/ "\n\n\"");

    while (1)
    {
        // e70a
        PrintString(/*0x9f00*/ "Of what wouldst\nthou hear my\nlore, ");
        PrintHonorific();
        PrintString(/*0x9f24*/ "?\"\n\nYou respond:\n");
        GetString(inputText, 0xf);
        PrintString(/*0x9f36*/ "\n\n");
        if (inputText[0] == '\0')
        {
            // e734 -> e736 -> e83e
            return 0;
        }

        index_h = 0;
        topic_b = -1;
        for (; index_h < 0x1a; index_h++)
        {
            matchPos = CompareWord((byte*)pubTopicWords[index_h], inputText);
            if (matchPos == 0 || (0 < matchPos && inputText[matchPos - 1] == ' '))
            {
                // e765
                topic_b = index_h;
                break;
            }
        }

        // e76b
        if (topic_b == -1)
        {
            // e771
            PrintString(/*0x9f3a*/ "\"That, I cannot help thee with.\n\n");
        }
        else
        {
            // e786
            shopPrice = gossipPrices[topic_b];
            PrintShopFileText(0x134e);
            PrintString(/*0x9f5c*/ "\n\nFair 'nuff?\" ");
            index_h = 0;
            do
            {
                if ((uint)index_h == 'N')
                    break;
                index_h = GetChar();
            } while ((uint)index_h != 'Y');
            if ((uint)index_h == 'N')
            {
                PrintString(/*0x9f6c*/ "No\n\n");
                return 0;
            }
            else
            {
                PrintString(/*0x9f72*/ "Yes\n\n");

                if (shopPrice > gold)
                {
                    PrintString(/*0x9f78*/ "\"Sorry, ");
                    PrintHonorific();
                    PrintShopFileText(0x146a);
                    return 1;
                }

                gold -= shopPrice;
                UpdateVitalsDisplay();

                itemName = pubPersons[topic_b];
                placeName = pubPlaces[gossipPlaceIdx[topic_b]];

                PrintShopFileText(gossipTexts[RandomRange(0, 3)]);

                PrintString(/*0x9f82*/ "\nsays ");
                PrintString(shopOwner);
                PrintString(/*0x9f8a*/ ".\n\n");
                return 0;
            }
        }
    }
}

// Pub: the keeper's farewell.
static void Shoppes2Farewell(void) { ShoppesFarewell(boughtSomething); }

// The pub (shop kind of the taverns): food, drink, rations, gossip until the party has had enough. member is the talking party member.
void Pub(int member)
{
    int ordered;
    byte key_g;

    pubKind = pubKindByShop[shopIndex];
    tableTile = boughtSomething = 0;
    Greet();

    do
    {
        key_g = GetChar();
        if (key_g == 'N' || key_g == U5_KEY_SPACE)
        {
            PrintString(/*0x9f8e*/ "No");
            Shoppes2Farewell();
            return;
        }
    } while (key_g != 'Y');

    PrintString(/*0x9f92*/ "Yes\n\n\"");
    PrintShopFileText(pubOrderTexts[pubKind]);
    PrintChar('\"');
    PrintChar(' ');

    if (GetCurrentTextX() > 0xe)
    {
        PrintChar('\n');
    }

    // e8bd
    while (1)
    {
        while (1)
        {
            key_g = GetChar();
            ordered = 0;

            if (key_g == U5_KEY_SPACE || key_g == U5_KEY_ESC || key_g == U5_KEY_ENTER)
            {
                Shoppes2Farewell();
                return;
            }

            if (foodKeys[pubKind] == key_g)
            {
                if (OrderFood() != 0)
                {
                    return;
                }
            }

            // e8ea
            if (drinkKeys[pubKind] == key_g)
            {
                ordered = OrderDrink();
                if (ordered == 1)
                {
                    return;
                }
            }

            // e907
            if (rationKeys[pubKind] == key_g)
            {
                ordered = BuyRations(member);
                if (ordered == 1)
                {
                    return;
                }
            }

            // e927
            if (gossipKeys[pubKind] == key_g)
            {
                if (boughtSomething == 0)
                    continue;

                if (BuyGossip() != 0)
                {
                    return;
                }
            }

            // e947
            if (foodKeys[pubKind] == key_g || drinkKeys[pubKind] == key_g || rationKeys[pubKind] == key_g ||
                gossipKeys[pubKind] == key_g)
                break;
        }

        if (ordered != 2)
        {
            boughtSomething = 1;
        }

        // e977
        UpdateVitalsDisplay();
        PrintString(/*0x9f9a*/ "\"Anything else\nfor thee?\" ");

        do
        {
            key_g = GetChar();
            if (key_g == 'N')
            {
                PrintString(/*0x9fb6*/ "No");
                Shoppes2Farewell();
                return;
            }
        } while (key_g != 'Y');

        PrintString(/*0x9fba*/ "Yes\n\n\"");
        PrintShopFileText(pubMoreTexts[pubKind]);
        PrintChar('\"');
        PrintChar(' ');
    }
}
