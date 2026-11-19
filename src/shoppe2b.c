#include "common/common.h"

// this module's communal variables (vars.h, DS:a9bc)
s16 D_bd22_thrownOut;
s16 D_bd24_shipSold;

#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "shoppes.h"
#include "shoppes2.h"

// SHOPPES2.OVL is two MSC 5.0 objects: shoppes2.c (to SHOPPES2_066c_Pub) and this one, the
// shipwright (from SHOPPES2_07e2_CanAfford). SHOPPES2_08a8_ShipMenu gets its loop registers only without
// SHOPPES2_0000_CountDiners-0508 before it (docs/MATCHING.md, "A module's earlier functions use up the
// optimizer's capacity"), so the boundary is somewhere from 0664 to 08a8. 07e2 is where the
// tables split by user: D_4d4c_pubKindByShop-D_4d5e_pubMoreTexts serve 066c, D_4d66_frigatePrices-D_4d7a_shipY only 080e and 08a8.

u16 D_4d66_frigatePrices[4] = {0x0258, 0x02f1, 0x028a, 0x02bc};

u16 D_4d6e_skiffPrices[4] = {0x00c8, 0x00af, 0x007d, 0x0064};

u8 D_4d76_shipX[4] = {0x27, 0x97, 0x4f, 0x8a};

u8 D_4d7a_shipY[4] = {0xdd, 0x15, 0x6d, 0x9f};

// Shipwright: 1 when the party has the gold; else the shipwright yells and D_bd22_thrownOut is set.
static bool SHOPPES2_07e2_CanAfford(int price)
{
    if (price > D_57aa_gold)
    {
        SHOPPES_017a_PrintShopFileText(0x198e);
        SHOPPES_0026_PrintShopText(/*0x9fc2*/ "yells $.\n");
        D_bd22_thrownOut = 1;
        return 0;
    }

    return 1;
}

// Shipwright: complete a sale (a new ship is put at D_4d76/D_4d7a, the gold taken), then ask "anything else". Returns 1 when the party is done.
static bool SHOPPES2_080e_FinishSale(int newShip, int charge, int member)
{
    byte key;

    if (newShip != 0)
    {
        SHOPPES_017a_PrintShopFileText(0x19ab);
        D_5953_boughtShipX = D_4d76_shipX[D_b114_shopIndex];
        D_5954_boughtShipY = D_4d7a_shipY[D_b114_shopIndex];
    }

    if (charge != 0)
    {
        D_57aa_gold -= D_b118_shopPrice;
        D_bd24_shipSold = 1;
        SHOPPES_019a_FalsehoodSurcharge();
    }

    ULTIMA_2900_UpdateVitalsDisplay();
    SHOPPES_017a_PrintShopFileText(0x19da);

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

// Shipwright: the menu of frigate (F) and skiff (S): buy, or add a skiff to a frigate. D_6605_boughtShip: bit 0x80 a frigate (low bits its skiffs aboard), 0x40 a skiff.
// CHECKED
// frigates / skiffs
// The party member's intelligence is read through the array: MSC hoists its address out of
// the loop into two temporaries ([bp-6], [bp-8]) but not the byte itself, and the loop
// keeps local_4_ab in SI and param_1 in DI. With pointer locals it hoisted the whole price
// factor. The returns are gotos past the do-while, where SI is stored.
static void SHOPPES2_08a8_ShipMenu(int member)
{
    byte key;
    int done;

    done = 0;

    do
    {
        SHOPPES_017a_PrintShopFileText(0x18eb);
        switch (ULTIMA_266c_GetChar())
        {
        case 0x46:
            ULTIMA_1850_PrintString(/*0x9fe4*/ "F\n\n");
            done = 1;
            if ((D_6605_boughtShip & 0xc0) != 0)
            {
                SHOPPES_017a_PrintShopFileText(0x19f2);

                while ((key = ULTIMA_266c_GetChar()) != 'Y' && key != 'N')
                {
                }

                if (key == 'Y')
                {
                    ULTIMA_1850_PrintString(/*0x9fe8*/ "Yes");
                    SHOPPES2_07e2_CanAfford(10000);
                }
                else
                {
                    ULTIMA_1850_PrintString(/*0x9fec*/ "No");
                    SHOPPES2_08a8_ShipMenu(member);
                }
            }
            else
            {
                D_b118_shopPrice = D_4d66_frigatePrices[D_b114_shopIndex];
                D_b118_shopPrice += D_b118_shopPrice * -((s32)(D_55a8_party[member].intel * 3) - 100) / (u32)100;

                SHOPPES_017a_PrintShopFileText(0x183e);
                SHOPPES_017a_PrintShopFileText(0x1a50);

                while ((key = ULTIMA_266c_GetChar()) != 'Y' && key != 'N')
                {
                }

                if (key == 'Y')
                {
                    ULTIMA_1850_PrintString(/*0x9ff0*/ "Yes\n\n");

                    if (SHOPPES2_07e2_CanAfford(D_b118_shopPrice) == 0)
                    {
                        goto L_end;
                    }

                    D_6605_boughtShip = 0x82;
                    done = SHOPPES2_080e_FinishSale(1, 1, member);
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

            D_b118_shopPrice = D_4d6e_skiffPrices[D_b114_shopIndex];
            D_b118_shopPrice += D_b118_shopPrice * -((s32)(D_55a8_party[member].intel * 3) - 100) / (u32)100;

            SHOPPES_017a_PrintShopFileText(0x188c);
            SHOPPES_017a_PrintShopFileText(0x1a50);

            while ((key = ULTIMA_266c_GetChar()) != 'Y' && key != 'N')
            {
            }

            done = 1;

            if (key == 'Y')
            {
                ULTIMA_1850_PrintString(/*0x9ffe*/ "Yes\n\n");
                if (SHOPPES2_07e2_CanAfford(D_b118_shopPrice) == 0)
                {
                    goto L_end;
                }

                if ((D_6605_boughtShip & 0xc0) == 0)
                {
                    D_6605_boughtShip = 0x40;
                    done = SHOPPES2_080e_FinishSale(1, 1, member);
                }
                else if ((D_6605_boughtShip & 0x80) != 0)
                {
                    SHOPPES_017a_PrintShopFileText(0x193b);
                    D_6605_boughtShip++;
                    done = SHOPPES2_080e_FinishSale(0, 1, member);
                }
                else if ((D_6605_boughtShip & 0x40) != 0)
                {
                    SHOPPES_017a_PrintShopFileText(0x195f);
                    done = SHOPPES2_080e_FinishSale(0, 0, member);
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
void SHOPPES2_0abc_Shipwright(int member)
{
    bool done;

    done = 0;
    D_bd22_thrownOut = 0;
    D_bd24_shipSold = 0;

    SHOPPES_01b6_Greet();

    do
    {
        switch (ULTIMA_266c_GetChar())
        {
        case 'Y':
            ULTIMA_1850_PrintString(/*0xa008*/ "Yes");
            SHOPPES2_08a8_ShipMenu(member);
            done = 1;
            break;

        case U5_KEY_SPACE:
        case 'N':
            ULTIMA_1850_PrintString(/*0xa00c*/ "No");
            done = 1;
            break;
        }

    } while (!done);

    if (D_bd22_thrownOut == 0)
    {
        SHOPPES_0202_Farewell(D_bd24_shipSold);
    }
}
