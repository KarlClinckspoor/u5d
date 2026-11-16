#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "shoppes.h"
#include "shoppes2.h"

static void SHOPPES2_0000(short param_1)
{
    int local_4;

    D_bd1a = D_bd1c = D_b118 = 0;

    for (local_4 = 0; local_4 != D_585b; local_4++)
    {
        if (D_55a8_party[local_4].status != STATUS_DEAD)
        {
            D_bd1c++;
            D_bd1a++;
            D_b118 += param_1;
        }
    }

    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_16ba_PrintChar('\n');
}

static void SHOPPES2_006a(void)
{
    switch (D_bd1c)
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

// CHECKED
// NOT MATCHING by 3 bytes: the original stores 0xff through AL (mov al,0FFh; mov [587B],al),
// MSC's form for a chained assignment (DUNGEON and ULTIMA.EXE have D_589e = D_587b = 0xff).
// The label keeps MSC from folding local_4 = 0 into the index, as the original doesn't.
// Tried for the store: -1, (u8)-1, (char)0xff, ~0, (u8)~0, (int)0xff, 0xffL, -(1): all
// mov byte ptr imm; local_4 = 0xff then D_587b = local_4, or D_587b = local_4 = 0xff:
// AL, but local_4 is stored too (4 bytes longer); the label before either statement, or
// local_4 = 0 first: worse; a second label: no change.
static void SHOPPES2_00ac(void)
{
    int local_4;

    D_587b = 0xff;
    local_4 = 0;
L_e297:
    if (D_55a8_party[local_4].gender == 0xb)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9ac2, "sir"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x9ac6, "milady"));
    }
}

static int SHOPPES2_00dc(void)
{
    ULTIMA_16ba_PrintChar('\"');
    ULTIMA_1850_PrintString(_TEXT(0x9ace, "That will be "));
    ULTIMA_1a3e_PrintNumber(D_b118, 1, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x9adc, " gold for the "));
    SHOPPES2_006a();
    ULTIMA_1850_PrintString(_TEXT(0x9aec, " of ye,\n"));
    SHOPPES2_00ac();
    ULTIMA_16ba_PrintChar('.');

    if (D_b118 > D_57aa)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9af6, "\"\n\n\"CAN'T PAY?\nBeat it!\"\nyells "));
        ULTIMA_1850_PrintString(D_aafe);
        ULTIMA_16ba_PrintChar('.');
        ULTIMA_16ba_PrintChar('\n');
        return 1;
    }

    D_57aa -= D_b118;
    SHOPPES_019a();

    if (D_bd1a != 0)
    {
        ULTIMA_3f14_IncreaseInt(&D_57a8, D_bd1a, 9999);

        if (*ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y - 1) == TILE_MAP_TABLE_95)
        {
            *ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y - 1) = TILE_MAP_TABLE_9B;
            D_bd1e = TILE_MAP_TABLE_9B;

            ULTIMA_5910_UpdateFrame();
        }
        else if (*ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y + 1) == TILE_MAP_TABLE_95)
        {
            *ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y + 1) = TILE_MAP_TABLE_9A;
            D_bd1e = TILE_MAP_TABLE_9A;

            ULTIMA_5910_UpdateFrame();
        }
    }
    else
    {
        D_bd20++;
    }

    ULTIMA_1850_PrintString(_TEXT(0x9b16, "\nEnjoy!\"\n\n"));
    return 0;
}

static int SHOPPES2_01d2(void)
{
    ULTIMA_16ba_PrintChar((byte)D_4c1e[D_bd16]);
    SHOPPES2_0000(D_4c36[D_b114]);
    return SHOPPES2_00dc();
}

static int SHOPPES2_01f4(void)
{
    byte local_4;

    ULTIMA_16ba_PrintChar((byte)D_4c24[D_bd16]);
    if (D_bd20 == 3)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9b22, "\n\n\"I beg thy\npardon, "));
        SHOPPES2_00ac();
        ULTIMA_1850_PrintString(_TEXT(0x9b38, ",\"\nsays "));
        ULTIMA_1850_PrintString(D_aafe);
        ULTIMA_1850_PrintString(_TEXT(0x9b42, ".\n\"But haven't\nye had enough\nto drink?\" "));
        do
        {
            local_4 = ULTIMA_266c_GetChar();
            if (local_4 == 'Y')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9b6c, "Yes\n\n"));
                return 0;
            }
            if (local_4 == 'N')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9b72, "No!"));
                D_5957 = 0x19;
                ULTIMA_3f36_DecreaseByte(&D_5888, 1);
            }
        } while (local_4 != 'Y' && local_4 != 'N');
    }

    SHOPPES2_0000(1);
    if (D_4c24[D_bd16] == 'W')
    {
        ULTIMA_1850_PrintString(_TEXT(0x9b76, "\"Our wine list,\n"));
        SHOPPES2_00ac();
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
            local_4 = ULTIMA_266c_GetChar();
            if (local_4 == U5_KEY_SPACE)
            {
                ULTIMA_16ba_PrintChar('\n');
                ULTIMA_16ba_PrintChar('\n');
                if (local_4 == U5_KEY_SPACE)
                {
                    SHOPPES_017a(0x1413);
                    return 2;
                }
                else
                {
                    return 0;
                }
            }
        } while (local_4 < 0x41 || 0x46 < local_4);

        ULTIMA_16ba_PrintChar((byte)(char)local_4); // TODO: ???
        ULTIMA_1850_PrintString(_TEXT(0x9c08, "\n\n\"Ah, a fine\nchoice, "));
        SHOPPES2_00ac();
        ULTIMA_16ba_PrintChar('.');

        local_4 -= 0x41;

        if (D_57aa < (s16)D_4c48[local_4])
        {
            ULTIMA_1850_PrintString(_TEXT(0x9c20, "\"\n\n\"CAN'T PAY?\nBeat it!\"\nyells "));
            ULTIMA_1850_PrintString(D_aafe);
            ULTIMA_16ba_PrintChar('.');
            ULTIMA_16ba_PrintChar('\n');
            return 1;
        }
        else
        {
            D_57aa -= D_4c48[local_4];
            SHOPPES_019a();
            D_bd20++;
            //local_4 = 0x40;
            ULTIMA_1850_PrintString(_TEXT(0x9c40, "\nEnjoy!\""));
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_16ba_PrintChar('\n');
            if (local_4 == 0x20)
            {
                SHOPPES_017a(0x1413);
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
        D_bd1a = 0;
        return SHOPPES2_00dc();
    }
}

static int SHOPPES2_0380(int param_1)
{
    int local_6_an;
    int local_4;

    ULTIMA_16ba_PrintChar((byte)D_4c2a[D_bd16]);
    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_16ba_PrintChar('\n');

    D_b118 = D_4c54[D_b114];
    D_b118 += D_b118 * -((s32)(D_55a8_party[param_1].intel * 3) - 100) / (u32)100;

    ULTIMA_16ba_PrintChar('\"');
    SHOPPES_017a(D_4c66[ULTIMA_2092_RandomRange(0, 6)]);
    ULTIMA_1850_PrintString(_TEXT(0x9c4a, "\n\nHow many wouldst\nthou like?\" "));
    local_4 = local_6_an = ULTIMA_3b9e(2);
    if (local_6_an == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9c6a, "\n\n\"Hrumph.\""));
        return 2;
    }

    for (; local_6_an != 0; local_6_an--)
    {
        // e61f
        if (D_57aa >= D_b118)
        {
            ULTIMA_3f54_DecreaseInt(&D_57aa, D_b118);
            ULTIMA_3f14_IncreaseInt(&D_57a8, 0x19, 9999);

            if (D_57a8 == 9999)
                break;
        }
        else
        {
            local_4 -= local_6_an;
            // e654
            ULTIMA_1850_PrintString(_TEXT(0x9c76, "\n\n"));
            if (local_4 == 0)
            {
                if (D_57a8 < 3)
                {
                    ULTIMA_3f14_IncreaseInt(&D_57a8, ULTIMA_2092_RandomRange(0, 1) + 1, 9999);
                    SHOPPES_017a(0x143f);
                    return 1;
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9c7a, "\"Thou hast\nneither gold nor\nneed! Out!\"\n"));
                    ULTIMA_1850_PrintString(_TEXT(0x9ca4, "yells "));
                    ULTIMA_1850_PrintString(D_aafe);
                    ULTIMA_1850_PrintString(_TEXT(0x9cac, ".\n"));
                    return 1;
                }
            }
            else
            {
                // e6b6
                ULTIMA_1850_PrintString(_TEXT(0x9cb0, "\"Thou canst\nafford only "));
                ULTIMA_1a3e_PrintNumber(local_4, 1, 0x20);
                ULTIMA_1850_PrintString(_TEXT(0x9cca, "!\"\n\n"));
                return 0; // -> e689
            }
        }
    }

    // e6d4
    ULTIMA_1850_PrintString(_TEXT(0x9cd0, "\n\n"));
    SHOPPES_019a();
    return 0;
}

char D_4c1e[5] = /*0x4c1e*/ "MCBF";

char D_4c24[5] = /*0x4c24*/ "AWRS";

char D_4c2a[5] = /*0x4c2a*/ "R  P";

char D_4c30[5] = /*0x4c30*/ "CTHA";

u16 D_4c36[9] = {0x0003, 0x0004, 0x0005, 0x0003, 0x0002, 0x0005, 0x0003, 0x0004, 0x0005};

u16 D_4c48[6] = {0x0012, 0x00c0, 0x004f, 0x001e, 0x0113, 0x0062};

u16 D_4c54[9] = {0x000a, 0x000f, 0x0014, 0x0019, 0x001e, 0x0019, 0x0014, 0x0019, 0x001e};

u16 D_4c66[7] = {0x1198, 0x11d7, 0x121f, 0x1271, 0x1299, 0x12e3, 0x130c};

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

u8 D_4cdc[26] =
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

u16 D_4d10[0x1a] =
{
    0x0032, 0x004b, 0x0032, 0x0032, 0x004b, 0x004b, 0x0019, 0x0032,
    0x0064, 0x0096, 0x004b, 0x0096, 0x004b, 0x0064, 0x0064, 0x00c8,
    0x00c8, 0x00c8, 0x00fa, 0x00fa, 0x00fa, 0x0064, 0x0032, 0x0032,
    0x00c8, 0x0064
};

u16 D_4d44[4] = {0x13a2, 0x13ae, 0x13d9, 0x13f3};

u8 D_4d4c[0xa] = {0x00, 0x00, 0x00, 0x02, 0x03, 0x01, 0x00, 0x02, 0x00, 0x00};

u16 D_4d56[4] = {0x0f09, 0x0f5a, 0x0f9e, 0x0fdf};

u16 D_4d5e[4] = {0x1042, 0x109a, 0x10ea, 0x1146};

u16 D_4d66[4] = {0x0258, 0x02f1, 0x028a, 0x02bc};

u16 D_4d6e[4] = {0x00c8, 0x00af, 0x007d, 0x0064};

u8 D_4d76[4] = {0x27, 0x97, 0x4f, 0x8a};

u8 D_4d7a[4] = {0xdd, 0x15, 0x6d, 0x9f};

// CHECKED
static int SHOPPES2_0508(void)
{
    int local_6;
    int local_4_ab;
    int local_8;

    ULTIMA_16ba_PrintChar((byte)D_4c30[D_bd16]);
    ULTIMA_1850_PrintString(_TEXT(0x9efc, "\n\n\""));

    while (1)
    {
        // e70a
        ULTIMA_1850_PrintString(_TEXT(0x9f00, "Of what wouldst\nthou hear my\nlore, "));
        SHOPPES2_00ac();
        ULTIMA_1850_PrintString(_TEXT(0x9f24, "?\"\n\nYou respond:\n"));
        ULTIMA_3b1c_GetString(D_bcf8, 0xf);
        ULTIMA_1850_PrintString(_TEXT(0x9f36, "\n\n"));
        if (D_bcf8[0] == '\0')
        {
            // e734 -> e736 -> e83e
            return 0;
        }

        local_4_ab = 0;
        local_8 = -1;
        for (; local_4_ab < 0x1a; local_4_ab++)
        {
            local_6 = ULTIMA_6f1e((byte*)D_4c74[local_4_ab], D_bcf8);
            if (local_6 == 0 || (0 < local_6 && D_bcf8[local_6 - 1] == ' '))
            {
                // e765
                local_8 = local_4_ab;
                break;
            }
        }

        // e76b
        if (local_8 == -1)
        {
            // e771
            ULTIMA_1850_PrintString(_TEXT(0x9f3a, "\"That, I cannot help thee with.\n\n"));
        }
        else
        {
            // e786
            D_b118 = D_4d10[local_8];
            SHOPPES_017a(0x134e);
            ULTIMA_1850_PrintString(_TEXT(0x9f5c, "\n\nFair 'nuff?\" "));
            local_6 = 0;

            // NOT MATCHING: the original keeps local_6 in SI without storing it at the
            // loop's end (e7b3); MSC stores it here.
            do
            {
                if ((uint)local_6 == 'N')
                    break;
                local_6 = ULTIMA_266c_GetChar();
            } while ((uint)local_6 != 'Y');

            // e7b3
            local_4_ab = local_6;
            if ((uint)local_4_ab == 'N')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9f6c, "No\n\n"));
                return 0;
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x9f72, "Yes\n\n"));

                if (D_b118 > D_57aa)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9f78, "\"Sorry, "));
                    SHOPPES2_00ac();
                    SHOPPES_017a(0x146a);
                    return 1;
                }

                D_57aa -= D_b118;
                ULTIMA_2900_UpdateVitalsDisplay();

                D_ab00 = D_4ca8[local_8];
                D_ac62 = D_4cf6[D_4cdc[local_8]];

                SHOPPES_017a(D_4d44[ULTIMA_2092_RandomRange(0, 3)]);

                ULTIMA_1850_PrintString(_TEXT(0x9f82, "\nsays "));
                ULTIMA_1850_PrintString(D_aafe);
                ULTIMA_1850_PrintString(_TEXT(0x9f8a, ".\n\n"));
                return 0;
            }
        }
    }
}

static void SHOPPES2_0664(void) { SHOPPES_0202(D_bd18); }

void SHOPPES2_066c(int param_1)
{
    int local_6_an;
    byte local_4;

    D_bd16 = D_4d4c[D_b114];
    D_bd1e = D_bd18 = 0;
    SHOPPES_01b6();

    do
    {
        local_4 = ULTIMA_266c_GetChar();
        if (local_4 == 'N' || local_4 == U5_KEY_SPACE)
        {
            ULTIMA_1850_PrintString(_TEXT(0x9f8e, "No"));
            SHOPPES2_0664();
            return;
        }
    } while (local_4 != 'Y');

    ULTIMA_1850_PrintString(_TEXT(0x9f92, "Yes\n\n\""));
    SHOPPES_017a(D_4d56[D_bd16]);
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
            local_4 = ULTIMA_266c_GetChar();
            local_6_an = 0;

            if (local_4 == U5_KEY_SPACE || local_4 == U5_KEY_ESC || local_4 == U5_KEY_ENTER)
            {
                SHOPPES2_0664();
                return;
            }

            if (D_4c1e[D_bd16] == local_4)
            {
                if (SHOPPES2_01d2() != 0)
                {
                    return;
                }
            }

            // e8ea
            if (D_4c24[D_bd16] == local_4)
            {
                local_6_an = SHOPPES2_01f4();
                if (local_6_an == 1)
                {
                    return;
                }
            }

            // e907
            if (D_4c2a[D_bd16] == local_4)
            {
                local_6_an = SHOPPES2_0380(param_1);
                if (local_6_an == 1)
                {
                    return;
                }
            }

            // e927
            if (D_4c30[D_bd16] == local_4)
            {
                if (D_bd18 == 0)
                    continue;

                if (SHOPPES2_0508() != 0)
                {
                    return;
                }
            }

            // e947
            if (D_4c1e[D_bd16] == local_4 || D_4c24[D_bd16] == local_4 || D_4c2a[D_bd16] == local_4 ||
                D_4c30[D_bd16] == local_4)
                break;
        }

        if (local_6_an != 2)
        {
            D_bd18 = 1;
        }

        // e977
        ULTIMA_2900_UpdateVitalsDisplay();
        ULTIMA_1850_PrintString(_TEXT(0x9f9a, "\"Anything else\nfor thee?\" "));

        do
        {
            local_4 = ULTIMA_266c_GetChar();
            if (local_4 == 'N')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9fb6, "No"));
                SHOPPES2_0664();
                return;
            }
        } while (local_4 != 'Y');

        ULTIMA_1850_PrintString(_TEXT(0x9fba, "Yes\n\n\""));
        SHOPPES_017a(D_4d5e[D_bd16]);
        ULTIMA_16ba_PrintChar('\"');
        ULTIMA_16ba_PrintChar(' ');
    }
}

static bool SHOPPES2_07e2(int param_1)
{
    if (param_1 > D_57aa)
    {
        SHOPPES_017a(0x198e);
        SHOPPES_0026(_TEXT(0x9fc2, "yells $.\n"));
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
        SHOPPES_017a(0x19ab);
        D_5953 = D_4d76[D_b114];
        D_5954 = D_4d7a[D_b114];
    }

    if (param_2 != 0)
    {
        D_57aa -= D_b118;
        D_bd24 = 1;
        SHOPPES_019a();
    }

    ULTIMA_2900_UpdateVitalsDisplay();
    SHOPPES_017a(0x19da);

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
static void SHOPPES2_08a8(int param_1)
{
    byte local_6;
    int local_4_ab;
    byte* local_8;
    byte* local_a;
    
    local_a = local_8 = &D_55a8_party[param_1].intel;

    local_4_ab = 0;
    while (1)
    {
        SHOPPES_017a(0x18eb);
        switch (ULTIMA_266c_GetChar())
        {
        case 0x46:
            // ead0
            ULTIMA_1850_PrintString(_TEXT(0x9fe4, "F\n\n"));
            local_4_ab = 1;
            if ((D_6605 & 0xc0) != 0)
            {
                SHOPPES_017a(0x19f2);

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
                D_b118 = D_4d66[D_b114];
                D_b118 += D_b118 * -((s32)(*local_a * 3) - 100) / (u32)100;

                // eb62
                SHOPPES_017a(0x183e);
                SHOPPES_017a(0x1a50);

                while ((local_6 = ULTIMA_266c_GetChar()) != 'Y' && local_6 != 'N')
                {
                }

                // eb82
                if (local_6 == 'Y')
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9ff0, "Yes\n\n"));

                    if (SHOPPES2_07e2(D_b118) == 0)
                    {
                        return;
                    }

                    // eb9d
                    D_6605 = 0x82;

                    // eba2 -> eba5 -> ec62 -> ec89
                    local_4_ab = SHOPPES2_080e(1, 1, param_1);
                    if (local_4_ab != 0)
                    {
                        return;
                    }
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9ff6, "No"));
                    return;
                }

                continue;
            }
            break;

        case 0x53:
            // ebb4
            ULTIMA_1850_PrintString(_TEXT(0x9ffa, "S\n\n"));

            D_b118 = D_4d6e[D_b114];
            D_b118 += D_b118 * -((s32)(*local_8 * 3) - 100) / (u32)100;

            SHOPPES_017a(0x188c);
            SHOPPES_017a(0x1a50);

            while ((local_6 = ULTIMA_266c_GetChar()) != 'Y' && local_6 != 'N')
            {
            }

            // ec1e
            local_4_ab = 1;

            if (local_6 == 'Y')
            {
                ULTIMA_1850_PrintString(_TEXT(0x9ffe, "Yes\n\n"));
                if (SHOPPES2_07e2(D_b118) == 0)
                {
                    return;
                }

                if ((D_6605 & 0xc0) == 0) // ec39
                {
                    D_6605 = 0x40; // ec40
                    local_4_ab = SHOPPES2_080e(1, 1, param_1);
                }
                else if ((D_6605 & 0x80) != 0) // ec4a
                {
                    // ec4a
                    SHOPPES_017a(0x193b);
                    D_6605++;
                    local_4_ab = SHOPPES2_080e(0, 1, param_1);
                }
                else
                {
                    // ec6c
                    if ((D_6605 & 0x40) == 0)
                    {
                        if (local_4_ab != 0)
                        {
                            return;
                        }

                        continue;
                    }

                    SHOPPES_017a(0x195f);
                    local_4_ab = SHOPPES2_080e(0, 0, param_1);
                }
            }
            else
            {
                // ec80
                ULTIMA_1850_PrintString(_TEXT(0xa004, "No"));
                return;
            }
            break;

        case U5_KEY_ESC:
        case U5_KEY_SPACE:
            // ec86
            local_4_ab = 1;
            break;
        }

        // ec89
        if (local_4_ab != 0)
        {
            return;
        }
    }
}

void SHOPPES2_0abc(int param_1)
{
    bool local_4;

    local_4 = 0;
    D_bd22 = 0;
    D_bd24 = 0;

    SHOPPES_01b6();

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
        SHOPPES_0202(D_bd24);
    }
}
