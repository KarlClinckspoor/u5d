#include "common/common.h"

// this module's communal variables (vars.h, DS:a9bc)
s16 thrownOut;
s16 shipSold;

#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "shoppes.h"
#include "shoppes2.h"

// SHOPPES2.OVL is two MSC 5.0 objects: shoppes2.c (to Pub) and this one, the
// shipwright (from CanAfford). ShipMenu gets its loop registers only without
// CountDiners-0508 before it (docs/MATCHING.md, "A module's earlier functions use up the
// optimizer's capacity"), so the boundary is somewhere from 0664 to 08a8. 07e2 is where the
// tables split by user: pubKindByShop-pubMoreTexts serve 066c, frigatePrices-shipY only 080e and 08a8.

u16 frigatePrices[4] = {0x0258, 0x02f1, 0x028a, 0x02bc};

u16 skiffPrices[4] = {0x00c8, 0x00af, 0x007d, 0x0064};

u8 shipX[4] = {0x27, 0x97, 0x4f, 0x8a};

u8 shipY[4] = {0xdd, 0x15, 0x6d, 0x9f};

// Shipwright: 1 when the party has the gold; else the shipwright yells and thrownOut is set.
static bool CanAfford(int price)
{
    if (price > gold)
    {
        PrintShopFileText(0x198e);
        PrintShopText(/*0x9fc2*/ "yells $.\n");
        thrownOut = 1;
        return 0;
    }

    return 1;
}

// Shipwright: complete a sale (a new ship is put at D_4d76/D_4d7a, the gold taken), then ask "anything else". Returns 1 when the party is done.
static bool FinishSale(int newShip, int charge, int member)
{
    byte key;

    if (newShip != 0)
    {
        PrintShopFileText(0x19ab);
        boughtShipX = shipX[shopIndex];
        boughtShipY = shipY[shopIndex];
    }

    if (charge != 0)
    {
        gold -= shopPrice;
        shipSold = 1;
        FalsehoodSurcharge();
    }

    ULTIMA_2900_UpdateVitalsDisplay();
    PrintShopFileText(0x19da);

    if (D_55a8_party[member].gender == 'F') // BUG
    {
        ULTIMA_1850_PrintString(/*0x9fcc*/ "milady");
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x9fd4*/ "sir");
    }

    ULTIMA_1850_PrintString(/*0x9fd8*/ "?\" ");

    while ((key = ULTIMA_266c_GetChar()) != 'Y' && key != 'N')
    {
    }

    if (key == 'Y')
    {
        ULTIMA_1850_PrintString(/*0x9fdc*/ "Yes");
        return 0;
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x9fe0*/ "No");
        return 1;
    }
}

// Shipwright: the menu of frigate (F) and skiff (S): buy, or add a skiff to a frigate. boughtShip: bit 0x80 a frigate (low bits its skiffs aboard), 0x40 a skiff.
// CHECKED
// frigates / skiffs
// The party member's intelligence is read through the array: MSC hoists its address out of
// the loop into two temporaries ([bp-6], [bp-8]) but not the byte itself, and the loop
// keeps local_4_ab in SI and param_1 in DI. With pointer locals it hoisted the whole price
// factor. The returns are gotos past the do-while, where SI is stored.
static void ShipMenu(int member)
{
    byte key;
    int done;

    done = 0;

    do
    {
        PrintShopFileText(0x18eb);
        switch (ULTIMA_266c_GetChar())
        {
        case 0x46:
            ULTIMA_1850_PrintString(/*0x9fe4*/ "F\n\n");
            done = 1;
            if ((boughtShip & 0xc0) != 0)
            {
                PrintShopFileText(0x19f2);

                while ((key = ULTIMA_266c_GetChar()) != 'Y' && key != 'N')
                {
                }

                if (key == 'Y')
                {
                    ULTIMA_1850_PrintString(/*0x9fe8*/ "Yes");
                    CanAfford(10000);
                }
                else
                {
                    ULTIMA_1850_PrintString(/*0x9fec*/ "No");
                    ShipMenu(member);
                }
            }
            else
            {
                shopPrice = frigatePrices[shopIndex];
                shopPrice += shopPrice * -((s32)(D_55a8_party[member].intel * 3) - 100) / (u32)100;

                PrintShopFileText(0x183e);
                PrintShopFileText(0x1a50);

                while ((key = ULTIMA_266c_GetChar()) != 'Y' && key != 'N')
                {
                }

                if (key == 'Y')
                {
                    ULTIMA_1850_PrintString(/*0x9ff0*/ "Yes\n\n");

                    if (CanAfford(shopPrice) == 0)
                    {
                        goto L_end;
                    }

                    boughtShip = 0x82;
                    done = FinishSale(1, 1, member);
                }
                else
                {
                    ULTIMA_1850_PrintString(/*0x9ff6*/ "No");
                    goto L_end;
                }
            }
            break;

        case 0x53:
            ULTIMA_1850_PrintString(/*0x9ffa*/ "S\n\n");

            shopPrice = skiffPrices[shopIndex];
            shopPrice += shopPrice * -((s32)(D_55a8_party[member].intel * 3) - 100) / (u32)100;

            PrintShopFileText(0x188c);
            PrintShopFileText(0x1a50);

            while ((key = ULTIMA_266c_GetChar()) != 'Y' && key != 'N')
            {
            }

            done = 1;

            if (key == 'Y')
            {
                ULTIMA_1850_PrintString(/*0x9ffe*/ "Yes\n\n");
                if (CanAfford(shopPrice) == 0)
                {
                    goto L_end;
                }

                if ((boughtShip & 0xc0) == 0)
                {
                    boughtShip = 0x40;
                    done = FinishSale(1, 1, member);
                }
                else if ((boughtShip & 0x80) != 0)
                {
                    PrintShopFileText(0x193b);
                    boughtShip++;
                    done = FinishSale(0, 1, member);
                }
                else if ((boughtShip & 0x40) != 0)
                {
                    PrintShopFileText(0x195f);
                    done = FinishSale(0, 0, member);
                }
            }
            else
            {
                ULTIMA_1850_PrintString(/*0xa004*/ "No");
                goto L_end;
            }
            break;

        case U5_KEY_ESC:
        case U5_KEY_SPACE:
            done = 1;
            break;
        }
    } while (done == 0);
L_end:;
}

// The shipwright (frigates and skiffs): greeting, the ship menu, farewell.
void Shipwright(int member)
{
    bool done;

    done = 0;
    thrownOut = 0;
    shipSold = 0;

    Greet();

    do
    {
        switch (ULTIMA_266c_GetChar())
        {
        case 'Y':
            ULTIMA_1850_PrintString(/*0xa008*/ "Yes");
            ShipMenu(member);
            done = 1;
            break;

        case U5_KEY_SPACE:
        case 'N':
            ULTIMA_1850_PrintString(/*0xa00c*/ "No");
            done = 1;
            break;
        }

    } while (!done);

    if (thrownOut == 0)
    {
        ShoppesFarewell(shipSold);
    }
}
