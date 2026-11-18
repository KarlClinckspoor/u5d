#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "shoppes.h"
#include "shoppes2.h"

// Pub: count the living party members (the diners and the food they get); the price is pricePerHead times that. Prints two newlines.
static void SHOPPES2_0000_CountDiners(short pricePerHead)
{
    int member;

    D_bd1a_dinersFed = D_bd1c_dinerCount = D_b118_shopPrice = 0;

    for (member = 0; member != D_585b_partyCount; member++)
    {
        if (D_55a8_party[member].status != STATUS_DEAD)
        {
            D_bd1c_dinerCount++;
            D_bd1a_dinersFed++;
            D_b118_shopPrice += pricePerHead;
        }
    }

    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_16ba_PrintChar('\n');
}

// Pub: print the number of diners in words ("two" to "six").
static void SHOPPES2_006a_PrintDinerCount(void)
{
    switch (D_bd1c_dinerCount)
    {
    case 2:
        ULTIMA_1850_PrintString(_TEXT(0x9aa8, "two"));
        break;

    case 3:
        ULTIMA_1850_PrintString(_TEXT(0x9aac, "three"));
        break;

    case 4:
        ULTIMA_1850_PrintString(_TEXT(0x9ab2, "four"));
        break;

    case 5:
        ULTIMA_1850_PrintString(_TEXT(0x9ab8, "five"));
        break;

    case 6:
        ULTIMA_1850_PrintString(_TEXT(0x9abe, "six"));
        break;
    }
}

// Pub: print "sir" or "milady" after the gender of party member 0 (and clear the active player).
// CHECKED
// The original stores 0xff through AL (mov al,0FFh; mov [587B],al), MSC's form for a
// chained assignment (DUNGEON and ULTIMA.EXE have D_589e_curCombatant = D_587b_activePlayer = 0xff). Here the chain
// names D_587b_activePlayer twice, probably two names of the same byte in the original.
// The label keeps MSC from folding local_4 = 0 into the index, as the original doesn't.
static void SHOPPES2_00ac_PrintHonorific(void)
{
    int member;

#if defined(TARGET_DOS16)
    D_587b_activePlayer = D_587b_activePlayer = 0xff; // two stores to one byte between sequence points: DOS build only
#else
    D_587b_activePlayer = 0xff;
#endif
    member = 0;
L_e297:
    if (D_55a8_party[member].gender == 0xb)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9ac2, "sir"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x9ac6, "milady"));
    }
}

// Pub: "That will be N gold for the .. of ye": take the gold and give each living member food; the table gets its dish. Returns 1 when the party cannot pay and is thrown out.
static int SHOPPES2_00dc_ChargeForMeal(void)
{
    ULTIMA_16ba_PrintChar('\"');
    ULTIMA_1850_PrintString(_TEXT(0x9ace, "That will be "));
    ULTIMA_1a3e_PrintNumber(D_b118_shopPrice, 1, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x9adc, " gold for the "));
    SHOPPES2_006a_PrintDinerCount();
    ULTIMA_1850_PrintString(_TEXT(0x9aec, " of ye,\n"));
    SHOPPES2_00ac_PrintHonorific();
    ULTIMA_16ba_PrintChar('.');

    if (D_b118_shopPrice > D_57aa_gold)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9af6, "\"\n\n\"CAN'T PAY?\nBeat it!\"\nyells "));
        ULTIMA_1850_PrintString(D_aafe_shopOwner);
        ULTIMA_16ba_PrintChar('.');
        ULTIMA_16ba_PrintChar('\n');
        return 1;
    }

    D_57aa_gold -= D_b118_shopPrice;
    SHOPPES_019a_FalsehoodSurcharge();

    if (D_bd1a_dinersFed != 0)
    {
        ULTIMA_3f14_IncreaseInt(&D_57a8_food, D_bd1a_dinersFed, 9999);

        if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY - 1) == TILE_MAP_TABLE_95)
        {
            *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY - 1) = TILE_MAP_TABLE_9B;
            D_bd1e_tableTile = TILE_MAP_TABLE_9B;

            ULTIMA_5910_UpdateFrame();
        }
        else if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY + 1) == TILE_MAP_TABLE_95)
        {
            *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY + 1) = TILE_MAP_TABLE_9A;
            D_bd1e_tableTile = TILE_MAP_TABLE_9A;

            ULTIMA_5910_UpdateFrame();
        }
    }
    else
    {
        D_bd20_drinksBought++;
    }

    ULTIMA_1850_PrintString(_TEXT(0x9b16, "\nEnjoy!\"\n\n"));
    return 0;
}

// Pub: the party orders a meal (the pub kind's food key), price per head from D_4c36_mealPrices.
static int SHOPPES2_01d2_OrderFood(void)
{
    ULTIMA_16ba_PrintChar((byte)D_4c1e_foodKeys[D_bd16_pubKind]);
    SHOPPES2_0000_CountDiners(D_4c36_mealPrices[D_b114_shopIndex]);
    return SHOPPES2_00dc_ChargeForMeal();
}

// Pub: order a drink (ale; the wine list in a wine house). After three the keeper refuses; returns 1 when thrown out, 2 when the keeper's story follows.
static int SHOPPES2_01f4_OrderDrink(void)
{
    byte key;

    ULTIMA_16ba_PrintChar((byte)D_4c24_drinkKeys[D_bd16_pubKind]);
    if (D_bd20_drinksBought == 3)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9b22, "\n\n\"I beg thy\npardon, "));
        SHOPPES2_00ac_PrintHonorific();
        ULTIMA_1850_PrintString(_TEXT(0x9b38, ",\"\nsays "));
        ULTIMA_1850_PrintString(D_aafe_shopOwner);
        ULTIMA_1850_PrintString(_TEXT(0x9b42, ".\n\"But haven't\nye had enough\nto drink?\" "));
        do
        {
            key = ULTIMA_266c_GetChar();
            if (key == 'Y')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9b6c, "Yes\n\n"));
                return 0;
            }
            if (key == 'N')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9b72, "No!"));
                D_5957_drunkTurns = 0x19;
                ULTIMA_3f36_DecreaseByte(&D_5888_karma, 1);
            }
        } while (key != 'Y' && key != 'N');
    }

    SHOPPES2_0000_CountDiners(1);
    if (D_4c24_drinkKeys[D_bd16_pubKind] == 'W')
    {
        ULTIMA_1850_PrintString(_TEXT(0x9b76, "\"Our wine list,\n"));
        SHOPPES2_00ac_PrintHonorific();
        ULTIMA_1850_PrintString(_TEXT(0x9b88, ".\n\n"));
        ULTIMA_1850_PrintString(_TEXT(0x9b8c, "a) Rose.......18\n"));
        ULTIMA_1850_PrintString(_TEXT(0x9b9e, "b) Claret....192\n"));
        ULTIMA_1850_PrintString(_TEXT(0x9bb0, "c) Sauterne...79\n"));
        ULTIMA_1850_PrintString(_TEXT(0x9bc2, "d) Muscatel...30\n"));
        ULTIMA_1850_PrintString(_TEXT(0x9bd4, "e) Moselle...275\n"));
        ULTIMA_1850_PrintString(_TEXT(0x9be6, "f) Chablis....98\n\n"));
        ULTIMA_1850_PrintString(_TEXT(0x9bfa, "Thy choice?\" "));

        do
        {
            key = ULTIMA_266c_GetChar();
            if (key == U5_KEY_SPACE)
            {
                ULTIMA_16ba_PrintChar('\n');
                ULTIMA_16ba_PrintChar('\n');
                if (key == U5_KEY_SPACE)
                {
                    SHOPPES_017a_PrintShopFileText(0x1413);
                    return 2;
                }
                else
                {
                    return 0;
                }
            }
        } while (key < 0x41 || 0x46 < key);

        ULTIMA_16ba_PrintChar((byte)(char)key); // TODO: ???
        ULTIMA_1850_PrintString(_TEXT(0x9c08, "\n\n\"Ah, a fine\nchoice, "));
        SHOPPES2_00ac_PrintHonorific();
        ULTIMA_16ba_PrintChar('.');

        key -= 0x41;

        if (D_57aa_gold < (s16)D_4c48_winePrices[key])
        {
            ULTIMA_1850_PrintString(_TEXT(0x9c20, "\"\n\n\"CAN'T PAY?\nBeat it!\"\nyells "));
            ULTIMA_1850_PrintString(D_aafe_shopOwner);
            ULTIMA_16ba_PrintChar('.');
            ULTIMA_16ba_PrintChar('\n');
            return 1;
        }
        else
        {
            D_57aa_gold -= D_4c48_winePrices[key];
            SHOPPES_019a_FalsehoodSurcharge();
            D_bd20_drinksBought++;
            //key = 0x40;
            ULTIMA_1850_PrintString(_TEXT(0x9c40, "\nEnjoy!\""));
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_16ba_PrintChar('\n');
            if (key == 0x20)
            {
                SHOPPES_017a_PrintShopFileText(0x1413);
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
        D_bd1a_dinersFed = 0;
        return SHOPPES2_00dc_ChargeForMeal();
    }
}

// Pub: buy rations (25 food each); the talker's intelligence lowers the price. Returns 1 when thrown out.
static int SHOPPES2_0380_BuyRations(int member)
{
    int remaining;
    int wanted;

    ULTIMA_16ba_PrintChar((byte)D_4c2a_rationKeys[D_bd16_pubKind]);
    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_16ba_PrintChar('\n');

    D_b118_shopPrice = D_4c54_rationPrices[D_b114_shopIndex];
    D_b118_shopPrice += D_b118_shopPrice * -((s32)(D_55a8_party[member].intel * 3) - 100) / (u32)100;

    ULTIMA_16ba_PrintChar('\"');
    SHOPPES_017a_PrintShopFileText(D_4c66_rationTexts[ULTIMA_2092_RandomRange(0, 6)]);
    ULTIMA_1850_PrintString(_TEXT(0x9c4a, "\n\nHow many wouldst\nthou like?\" "));
    wanted = remaining = ULTIMA_3b9e_GetNumber(2);
    if (remaining == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9c6a, "\n\n\"Hrumph.\""));
        return 2;
    }

    for (; remaining != 0; remaining--)
    {
        // e61f
        if (D_57aa_gold >= D_b118_shopPrice)
        {
            ULTIMA_3f54_DecreaseInt(&D_57aa_gold, D_b118_shopPrice);
            ULTIMA_3f14_IncreaseInt(&D_57a8_food, 0x19, 9999);

            if (D_57a8_food == 9999)
                break;
        }
        else
        {
            wanted -= remaining;
            // e654
            ULTIMA_1850_PrintString(_TEXT(0x9c76, "\n\n"));
            if (wanted == 0)
            {
                if (D_57a8_food < 3)
                {
                    ULTIMA_3f14_IncreaseInt(&D_57a8_food, ULTIMA_2092_RandomRange(0, 1) + 1, 9999);
                    SHOPPES_017a_PrintShopFileText(0x143f);
                    return 1;
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9c7a, "\"Thou hast\nneither gold nor\nneed! Out!\"\n"));
                    ULTIMA_1850_PrintString(_TEXT(0x9ca4, "yells "));
                    ULTIMA_1850_PrintString(D_aafe_shopOwner);
                    ULTIMA_1850_PrintString(_TEXT(0x9cac, ".\n"));
                    return 1;
                }
            }
            else
            {
                // e6b6
                ULTIMA_1850_PrintString(_TEXT(0x9cb0, "\"Thou canst\nafford only "));
                ULTIMA_1a3e_PrintNumber(wanted, 1, 0x20);
                ULTIMA_1850_PrintString(_TEXT(0x9cca, "!\"\n\n"));
                return 0; // -> e689
            }
        }
    }

    // e6d4
    ULTIMA_1850_PrintString(_TEXT(0x9cd0, "\n\n"));
    SHOPPES_019a_FalsehoodSurcharge();
    return 0;
}

char D_4c1e_foodKeys[5] = /*0x4c1e*/ "MCBF";

char D_4c24_drinkKeys[5] = /*0x4c24*/ "AWRS";

char D_4c2a_rationKeys[5] = /*0x4c2a*/ "R  P";

char D_4c30_gossipKeys[5] = /*0x4c30*/ "CTHA";

u16 D_4c36_mealPrices[9] = {0x0003, 0x0004, 0x0005, 0x0003, 0x0002, 0x0005, 0x0003, 0x0004, 0x0005};

u16 D_4c48_winePrices[6] = {0x0012, 0x00c0, 0x004f, 0x001e, 0x0113, 0x0062};

u16 D_4c54_rationPrices[9] = {0x000a, 0x000f, 0x0014, 0x0019, 0x001e, 0x0019, 0x0014, 0x0019, 0x001e};

u16 D_4c66_rationTexts[7] = {0x1198, 0x11d7, 0x121f, 0x1271, 0x1299, 0x12e3, 0x130c};

char* D_4c74[26] =
{
    _STATIC_TEXT(0x9cd4, "hone"),
    _STATIC_TEXT(0x9cda, "comp"),
    _STATIC_TEXT(0x9ce0, "valo"),
    _STATIC_TEXT(0x9ce6, "just"),
    _STATIC_TEXT(0x9cec, "sacr"),
    _STATIC_TEXT(0x9cf2, "hono"),
    _STATIC_TEXT(0x9cf8, "spir"),
    _STATIC_TEXT(0x9cfe, "humi"),
    _STATIC_TEXT(0x9d04, "dece"),
    _STATIC_TEXT(0x9d0a, "desp"),
    _STATIC_TEXT(0x9d10, "dest"),
    _STATIC_TEXT(0x9d16, "wron"),
    _STATIC_TEXT(0x9d1c, "cove"),
    _STATIC_TEXT(0x9d22, "sham"),
    _STATIC_TEXT(0x9d28, "hyth"),
    _STATIC_TEXT(0x9d2e, "crow"),
    _STATIC_TEXT(0x9d34, "scep"),
    _STATIC_TEXT(0x9d3a, "amul"),
    _STATIC_TEXT(0x9d40, "fals"),
    _STATIC_TEXT(0x9d46, "hatr"),
    _STATIC_TEXT(0x9d4c, "cowa"),
    _STATIC_TEXT(0x9d52, "astr"),
    _STATIC_TEXT(0x9d58, "oppr"),
    _STATIC_TEXT(0x9d5e, "brit"),
    _STATIC_TEXT(0x9d64, "resi"),
    _STATIC_TEXT(0x9d6a, "unde")
};

char* D_4ca8[26] =
{
    _STATIC_TEXT(0x9d70, "Malik"),
    _STATIC_TEXT(0x9d76, "Greyson"),
    _STATIC_TEXT(0x9d7e, "Trian"),
    _STATIC_TEXT(0x9d84, "Jeremy"),
    _STATIC_TEXT(0x9d8c, "Rew"),
    _STATIC_TEXT(0x9d90, "Gruman"),
    _STATIC_TEXT(0x9d98, "Saul"),
    _STATIC_TEXT(0x9d9e, "Shirita"),
    _STATIC_TEXT(0x9da6, "Malifora"),
    _STATIC_TEXT(0x9db0, "Annon"),
    _STATIC_TEXT(0x9db6, "Trian"),
    _STATIC_TEXT(0x9dbc, "Felespar"),
    _STATIC_TEXT(0x9dc6, "the mother of Rew"),
    _STATIC_TEXT(0x9dd8, "Sindar"),
    _STATIC_TEXT(0x9de0, "Kaiko"),
    _STATIC_TEXT(0x9de6, "Terrance"),
    _STATIC_TEXT(0x9df0, "Greymarch"),
    _STATIC_TEXT(0x9dfa, "Simon and Tessa"),
    _STATIC_TEXT(0x9e0a, "Shalineth"),
    _STATIC_TEXT(0x9e14, "a daemon"),
    _STATIC_TEXT(0x9e1e, "Lord Malone"),
    _STATIC_TEXT(0x9e2a, "Zachariah"),
    _STATIC_TEXT(0x9e34, "Tactus"),
    _STATIC_TEXT(0x9e3c, "a daemon"),
    _STATIC_TEXT(0x9e46, "Terrance"),
    _STATIC_TEXT(0x9e50, "Jotham")
};

u8 D_4cdc_gossipPlaceIdx[26] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x01,
    0x03, 0x09, 0x0b, 0x0a, 0x0c, 0x00, 0x04, 0x0a,
    0x01, 0x08
};

char* D_4cf6[13] =
{
    _STATIC_TEXT(0x9e58, "Moonglow"),
    _STATIC_TEXT(0x9e62, "Britain"),
    _STATIC_TEXT(0x9e6a, "Jhelom"),
    _STATIC_TEXT(0x9e72, "Yew"),
    _STATIC_TEXT(0x9e76, "Minoc"),
    _STATIC_TEXT(0x9e7c, "Trinsic"),
    _STATIC_TEXT(0x9e84, "Skara Brae"),
    _STATIC_TEXT(0x9e90, "New Magincia"),
    _STATIC_TEXT(0x9e9e, "a lighthouse south of Britain"),
    _STATIC_TEXT(0x9ebc, "a hidden mountain keep"),
    _STATIC_TEXT(0x9ed4, "the desert"),
    _STATIC_TEXT(0x9ee0, "the Lycaeum"),
    _STATIC_TEXT(0x9eec, "Serpent's Hold")
};

u16 D_4d10_gossipPrices[0x1a] =
{
    0x0032, 0x004b, 0x0032, 0x0032, 0x004b, 0x004b, 0x0019, 0x0032,
    0x0064, 0x0096, 0x004b, 0x0096, 0x004b, 0x0064, 0x0064, 0x00c8,
    0x00c8, 0x00c8, 0x00fa, 0x00fa, 0x00fa, 0x0064, 0x0032, 0x0032,
    0x00c8, 0x0064
};

u16 D_4d44_gossipTexts[4] = {0x13a2, 0x13ae, 0x13d9, 0x13f3};

u8 D_4d4c_pubKindByShop[0xa] = {0x00, 0x00, 0x00, 0x02, 0x03, 0x01, 0x00, 0x02, 0x00, 0x00};

u16 D_4d56_pubOrderTexts[4] = {0x0f09, 0x0f5a, 0x0f9e, 0x0fdf};

u16 D_4d5e_pubMoreTexts[4] = {0x1042, 0x109a, 0x10ea, 0x1146};

// Pub: the keeper sells gossip: asks for a topic word (D_4c74), names a person and a place (D_4ca8, D_4cf6) for a price. Returns 1 when the party cannot pay.
// CHECKED
static int SHOPPES2_0508_BuyGossip(void)
{
    int matchPos;
    int index_h;
    int topic_b;

    ULTIMA_16ba_PrintChar((byte)D_4c30_gossipKeys[D_bd16_pubKind]);
    ULTIMA_1850_PrintString(_TEXT(0x9efc, "\n\n\""));

    while (1)
    {
        // e70a
        ULTIMA_1850_PrintString(_TEXT(0x9f00, "Of what wouldst\nthou hear my\nlore, "));
        SHOPPES2_00ac_PrintHonorific();
        ULTIMA_1850_PrintString(_TEXT(0x9f24, "?\"\n\nYou respond:\n"));
        ULTIMA_3b1c_GetString(D_bcf8_inputText, 0xf);
        ULTIMA_1850_PrintString(_TEXT(0x9f36, "\n\n"));
        if (D_bcf8_inputText[0] == '\0')
        {
            // e734 -> e736 -> e83e
            return 0;
        }

        index_h = 0;
        topic_b = -1;
        for (; index_h < 0x1a; index_h++)
        {
            matchPos = ULTIMA_6f1e_CompareWord((byte*)D_4c74[index_h], D_bcf8_inputText);
            if (matchPos == 0 || (0 < matchPos && D_bcf8_inputText[matchPos - 1] == ' '))
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
            ULTIMA_1850_PrintString(_TEXT(0x9f3a, "\"That, I cannot help thee with.\n\n"));
        }
        else
        {
            // e786
            D_b118_shopPrice = D_4d10_gossipPrices[topic_b];
            SHOPPES_017a_PrintShopFileText(0x134e);
            ULTIMA_1850_PrintString(_TEXT(0x9f5c, "\n\nFair 'nuff?\" "));
            index_h = 0;
            do
            {
                if ((uint)index_h == 'N')
                    break;
                index_h = ULTIMA_266c_GetChar();
            } while ((uint)index_h != 'Y');
            if ((uint)index_h == 'N')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9f6c, "No\n\n"));
                return 0;
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x9f72, "Yes\n\n"));

                if (D_b118_shopPrice > D_57aa_gold)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9f78, "\"Sorry, "));
                    SHOPPES2_00ac_PrintHonorific();
                    SHOPPES_017a_PrintShopFileText(0x146a);
                    return 1;
                }

                D_57aa_gold -= D_b118_shopPrice;
                ULTIMA_2900_UpdateVitalsDisplay();

                D_ab00_itemName = D_4ca8[topic_b];
                D_ac62_placeName = D_4cf6[D_4cdc_gossipPlaceIdx[topic_b]];

                SHOPPES_017a_PrintShopFileText(D_4d44_gossipTexts[ULTIMA_2092_RandomRange(0, 3)]);

                ULTIMA_1850_PrintString(_TEXT(0x9f82, "\nsays "));
                ULTIMA_1850_PrintString(D_aafe_shopOwner);
                ULTIMA_1850_PrintString(_TEXT(0x9f8a, ".\n\n"));
                return 0;
            }
        }
    }
}

// Pub: the keeper's farewell.
static void SHOPPES2_0664_Farewell(void) { SHOPPES_0202_Farewell(D_bd18_boughtSomething); }

// The pub (shop kind of the taverns): food, drink, rations, gossip until the party has had enough. member is the talking party member.
void SHOPPES2_066c_Pub(int member)
{
    int ordered;
    byte key_g;

    D_bd16_pubKind = D_4d4c_pubKindByShop[D_b114_shopIndex];
    D_bd1e_tableTile = D_bd18_boughtSomething = 0;
    SHOPPES_01b6_Greet();

    do
    {
        key_g = ULTIMA_266c_GetChar();
        if (key_g == 'N' || key_g == U5_KEY_SPACE)
        {
            ULTIMA_1850_PrintString(_TEXT(0x9f8e, "No"));
            SHOPPES2_0664_Farewell();
            return;
        }
    } while (key_g != 'Y');

    ULTIMA_1850_PrintString(_TEXT(0x9f92, "Yes\n\n\""));
    SHOPPES_017a_PrintShopFileText(D_4d56_pubOrderTexts[D_bd16_pubKind]);
    ULTIMA_16ba_PrintChar('\"');
    ULTIMA_16ba_PrintChar(' ');

    if (ULTIMA_1f12_GetCurrentTextX() > 0xe)
    {
        ULTIMA_16ba_PrintChar('\n');
    }

    // e8bd
    while (1)
    {
        while (1)
        {
            key_g = ULTIMA_266c_GetChar();
            ordered = 0;

            if (key_g == U5_KEY_SPACE || key_g == U5_KEY_ESC || key_g == U5_KEY_ENTER)
            {
                SHOPPES2_0664_Farewell();
                return;
            }

            if (D_4c1e_foodKeys[D_bd16_pubKind] == key_g)
            {
                if (SHOPPES2_01d2_OrderFood() != 0)
                {
                    return;
                }
            }

            // e8ea
            if (D_4c24_drinkKeys[D_bd16_pubKind] == key_g)
            {
                ordered = SHOPPES2_01f4_OrderDrink();
                if (ordered == 1)
                {
                    return;
                }
            }

            // e907
            if (D_4c2a_rationKeys[D_bd16_pubKind] == key_g)
            {
                ordered = SHOPPES2_0380_BuyRations(member);
                if (ordered == 1)
                {
                    return;
                }
            }

            // e927
            if (D_4c30_gossipKeys[D_bd16_pubKind] == key_g)
            {
                if (D_bd18_boughtSomething == 0)
                    continue;

                if (SHOPPES2_0508_BuyGossip() != 0)
                {
                    return;
                }
            }

            // e947
            if (D_4c1e_foodKeys[D_bd16_pubKind] == key_g || D_4c24_drinkKeys[D_bd16_pubKind] == key_g || D_4c2a_rationKeys[D_bd16_pubKind] == key_g ||
                D_4c30_gossipKeys[D_bd16_pubKind] == key_g)
                break;
        }

        if (ordered != 2)
        {
            D_bd18_boughtSomething = 1;
        }

        // e977
        ULTIMA_2900_UpdateVitalsDisplay();
        ULTIMA_1850_PrintString(_TEXT(0x9f9a, "\"Anything else\nfor thee?\" "));

        do
        {
            key_g = ULTIMA_266c_GetChar();
            if (key_g == 'N')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9fb6, "No"));
                SHOPPES2_0664_Farewell();
                return;
            }
        } while (key_g != 'Y');

        ULTIMA_1850_PrintString(_TEXT(0x9fba, "Yes\n\n\""));
        SHOPPES_017a_PrintShopFileText(D_4d5e_pubMoreTexts[D_bd16_pubKind]);
        ULTIMA_16ba_PrintChar('\"');
        ULTIMA_16ba_PrintChar(' ');
    }
}
