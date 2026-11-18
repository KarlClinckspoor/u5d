#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "shoppes.h"
#include "shoppes2.h"

// SHOPPES2.OVL is two MSC 5.0 objects: shoppes2.c (to SHOPPES2_066c) and this one, the
// shipwright (from SHOPPES2_07e2). SHOPPES2_08a8 gets its loop registers only without
// SHOPPES2_0000-0508 before it (docs/MATCHING.md, "A module's earlier functions use up the
// optimizer's capacity"), so the boundary is somewhere from 0664 to 08a8. 07e2 is where the
// tables split by user: D_4d4c-D_4d5e serve 066c, D_4d66-D_4d7a only 080e and 08a8.

u16 D_4d66[4] = {0x0258, 0x02f1, 0x028a, 0x02bc};

u16 D_4d6e[4] = {0x00c8, 0x00af, 0x007d, 0x0064};

u8 D_4d76[4] = {0x27, 0x97, 0x4f, 0x8a};

u8 D_4d7a[4] = {0xdd, 0x15, 0x6d, 0x9f};

static bool SHOPPES2_07e2(int param_1)
{
    if (param_1 > D_57aa_gold)
    {
        SHOPPES_017a_PrintShopFileText(0x198e);
        SHOPPES_0026_PrintShopText(_TEXT(0x9fc2, "yells $.\n"));
        D_bd22 = 1;
        return 0;
    }

    return 1;
}

static bool SHOPPES2_080e(int param_1, int param_2, int param_3)
{
    byte local_4;

#if !defined(MATCHING_BUILD)
    local_4 = 0;
#endif

    if (param_1 != 0)
    {
        SHOPPES_017a_PrintShopFileText(0x19ab);
        D_5953_boughtShipX = D_4d76[D_b114_shopIndex];
        D_5954_boughtShipY = D_4d7a[D_b114_shopIndex];
    }

    if (param_2 != 0)
    {
        D_57aa_gold -= D_b118_shopPrice;
        D_bd24 = 1;
        SHOPPES_019a_FalsehoodSurcharge();
    }

    ULTIMA_2900_UpdateVitalsDisplay();
    SHOPPES_017a_PrintShopFileText(0x19da);

    if (D_55a8_party[param_3].gender == 'F') // BUG
    {
        ULTIMA_1850_PrintString(_TEXT(0x9fcc, "milady"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x9fd4, "sir"));
    }

    ULTIMA_1850_PrintString(_TEXT(0x9fd8, "?\" "));

    while ((local_4 = ULTIMA_266c_GetChar()) != 'Y' && local_4 != 'N')
    {
    }

    if (local_4 == 'Y')
    {
        ULTIMA_1850_PrintString(_TEXT(0x9fdc, "Yes"));
        return 0;
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x9fe0, "No"));
        return 1;
    }
}

// CHECKED
// frigates / skiffs
// The party member's intelligence is read through the array: MSC hoists its address out of
// the loop into two temporaries ([bp-6], [bp-8]) but not the byte itself, and the loop
// keeps local_4_ab in SI and param_1 in DI. With pointer locals it hoisted the whole price
// factor. The returns are gotos past the do-while, where SI is stored.
static void SHOPPES2_08a8(int param_1)
{
    byte local_6;
    int local_4_ab;

    local_4_ab = 0;

    do
    {
        SHOPPES_017a_PrintShopFileText(0x18eb);
        switch (ULTIMA_266c_GetChar())
        {
        case 0x46:
            ULTIMA_1850_PrintString(_TEXT(0x9fe4, "F\n\n"));
            local_4_ab = 1;
            if ((D_6605_boughtShip & 0xc0) != 0)
            {
                SHOPPES_017a_PrintShopFileText(0x19f2);

                while ((local_6 = ULTIMA_266c_GetChar()) != 'Y' && local_6 != 'N')
                {
                }

                if (local_6 == 'Y')
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9fe8, "Yes"));
                    SHOPPES2_07e2(10000);
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9fec, "No"));
                    SHOPPES2_08a8(param_1);
                }
            }
            else
            {
                D_b118_shopPrice = D_4d66[D_b114_shopIndex];
                D_b118_shopPrice += D_b118_shopPrice * -((s32)(D_55a8_party[param_1].intel * 3) - 100) / (u32)100;

                SHOPPES_017a_PrintShopFileText(0x183e);
                SHOPPES_017a_PrintShopFileText(0x1a50);

                while ((local_6 = ULTIMA_266c_GetChar()) != 'Y' && local_6 != 'N')
                {
                }

                if (local_6 == 'Y')
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9ff0, "Yes\n\n"));

                    if (SHOPPES2_07e2(D_b118_shopPrice) == 0)
                    {
                        goto L_end;
                    }

                    D_6605_boughtShip = 0x82;
                    local_4_ab = SHOPPES2_080e(1, 1, param_1);
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9ff6, "No"));
                    goto L_end;
                }
            }
            break;

        case 0x53:
            ULTIMA_1850_PrintString(_TEXT(0x9ffa, "S\n\n"));

            D_b118_shopPrice = D_4d6e[D_b114_shopIndex];
            D_b118_shopPrice += D_b118_shopPrice * -((s32)(D_55a8_party[param_1].intel * 3) - 100) / (u32)100;

            SHOPPES_017a_PrintShopFileText(0x188c);
            SHOPPES_017a_PrintShopFileText(0x1a50);

            while ((local_6 = ULTIMA_266c_GetChar()) != 'Y' && local_6 != 'N')
            {
            }

            local_4_ab = 1;

            if (local_6 == 'Y')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9ffe, "Yes\n\n"));
                if (SHOPPES2_07e2(D_b118_shopPrice) == 0)
                {
                    goto L_end;
                }

                if ((D_6605_boughtShip & 0xc0) == 0)
                {
                    D_6605_boughtShip = 0x40;
                    local_4_ab = SHOPPES2_080e(1, 1, param_1);
                }
                else if ((D_6605_boughtShip & 0x80) != 0)
                {
                    SHOPPES_017a_PrintShopFileText(0x193b);
                    D_6605_boughtShip++;
                    local_4_ab = SHOPPES2_080e(0, 1, param_1);
                }
                else if ((D_6605_boughtShip & 0x40) != 0)
                {
                    SHOPPES_017a_PrintShopFileText(0x195f);
                    local_4_ab = SHOPPES2_080e(0, 0, param_1);
                }
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0xa004, "No"));
                goto L_end;
            }
            break;

        case U5_KEY_ESC:
        case U5_KEY_SPACE:
            local_4_ab = 1;
            break;
        }
    } while (local_4_ab == 0);
L_end:;
}

void SHOPPES2_0abc(int param_1)
{
    bool local_4;

    local_4 = 0;
    D_bd22 = 0;
    D_bd24 = 0;

    SHOPPES_01b6_Greet();

    do
    {
        switch (ULTIMA_266c_GetChar())
        {
        case 'Y':
            ULTIMA_1850_PrintString(_TEXT(0xa008, "Yes"));
            SHOPPES2_08a8(param_1);
            local_4 = 1;
            break;

        case U5_KEY_SPACE:
        case 'N':
            ULTIMA_1850_PrintString(_TEXT(0xa00c, "No"));
            local_4 = 1;
            break;
        }

    } while (!local_4);

    if (D_bd22 == 0)
    {
        SHOPPES_0202_Farewell(D_bd24);
    }
}
