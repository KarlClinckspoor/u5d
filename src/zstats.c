#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "zstats.h"




static int ZSTATS_0000(int param_1)
{
    int local_4;

    local_4 = -1;

    if (D_5893_map_id > 0x80 && (D_ba14[D_589e].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        local_4 = D_ba14[D_589e].entityIdx;
    }
    else
    {
        // 202e
        ULTIMA_1850_PrintString(_TEXT(0x96b4, "Player: "));
        local_4 = ULTIMA_2d7a(param_1); // Select player
        if (local_4 >= 0)
        {
            ULTIMA_1850_PrintString(D_55a8_party[local_4].name);
            if (ULTIMA_1f12_GetCurrentTextX() != 0)
            {
                ULTIMA_16ba_PrintChar('\n');
            }
        }
    }

    // 205b
    if (local_4 == -1)
    {
        ULTIMA_1850_PrintString(_TEXT(0x96be, "None!\n"));
    }
    // 206a
    else if (local_4 == -2)
    {
        ULTIMA_16ba_PrintChar('\n');
    }

    // 2077
    return local_4;
}

static void ZSTATS_0082(int param_1)
{
    int local_6;
    int local_8;
    int local_a;
    S_55a8* local_4_ab;

    ULTIMA_4daa();

    local_4_ab = &D_55a8_party[param_1];

    ULTIMA_4e50(local_4_ab->name);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);

    local_6 = ULTIMA_4d76_FindIndex(_TEXT(0x96c6, CLASS_LIST), local_4_ab->cls);
    local_a = ULTIMA_4d76_FindIndex(_TEXT(0x96d0, STATUS_LIST), local_4_ab->status);

    for (local_8 = 0; local_8 < D_1a58[local_6]; local_8++)
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    ULTIMA_16ba_PrintChar(local_4_ab->gender);
    ULTIMA_1850_PrintString(_TEXT(0x96d6, " Lv-"));
    ULTIMA_1a3e_PrintNumber(local_4_ab->level, 1, 0x20);
    ULTIMA_16ba_PrintChar(' ');
    ULTIMA_1850_PrintString(D_1a44[local_6]);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_1bf2_SetTextPosition(0, 1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(D_1a6a[local_a]);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    ULTIMA_1bf2_SetTextPosition(0, 3);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1850_PrintString(_TEXT(0x96dc, "Str="));
    ULTIMA_1a3e_PrintNumber(local_4_ab->str, 2, 0x30);
    ULTIMA_1850_PrintString(_TEXT(0x96e2, "  HP:"));
    ULTIMA_1a3e_PrintNumber(local_4_ab->hp, 4, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x96e8, "\nInt="));
    ULTIMA_1a3e_PrintNumber(local_4_ab->intel, 2, 0x30);
    ULTIMA_1850_PrintString(_TEXT(0x96ee, "  HM:"));
    ULTIMA_1a3e_PrintNumber(local_4_ab->maxHp, 4, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x96f4, "\nDex="));
    ULTIMA_1a3e_PrintNumber(local_4_ab->dex, 2, 0x30);
    ULTIMA_1850_PrintString(_TEXT(0x96fa, "  Ex:"));
    ULTIMA_1a3e_PrintNumber(local_4_ab->exp, 4, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x9700, "\n\n    Magic:"));
    ULTIMA_1a3e_PrintNumber(local_4_ab->mag, 2, 0x20);
}

static bool ZSTATS_0278(int param_1)
{
    if (param_1 == 0xff)
    {
        return 0;
    }
    else
    {
        ULTIMA_16ba_PrintChar(' ');
        ULTIMA_1850_PrintString(D_1962[param_1]);
        ULTIMA_16ba_PrintChar('\n');
        return 1;
    }
}

static void ZSTATS_02a8(int param_1)
{
    S_55a8* local_6_an;
    int local_4;

    ULTIMA_4daa();

    local_6_an = &D_55a8_party[param_1];
    ULTIMA_4e50(local_6_an->name);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_UNDERLINE);
    ULTIMA_1850_PrintString(_TEXT(0x970e, "Arms\n\n"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_UNDERLINE);

    local_4 = ZSTATS_0278(local_6_an->equips[0]);
    local_4 += ZSTATS_0278(local_6_an->equips[1]);
    local_4 += ZSTATS_0278(local_6_an->equips[2]);
    local_4 += ZSTATS_0278(local_6_an->equips[3]);
    local_4 += ZSTATS_0278(local_6_an->equips[4]);
    local_4 += ZSTATS_0278(local_6_an->equips[5]);

    if (local_4 == 0)
    {
        ULTIMA_1bf2_SetTextPosition(0, 4);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
        ULTIMA_1850_PrintString(_TEXT(0x9716, "(None ready)"));
        ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    }

    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
}

static void ZSTATS_039c(void)
{
    ULTIMA_4daa();
    ULTIMA_4e50(_TEXT(0x9724, "Equipment"));
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    ULTIMA_1850_PrintString(_TEXT(0x972e, "\n Food: "));
    ULTIMA_1a3e_PrintNumber(D_57a8, 4, 0x20); // Food
    ULTIMA_1850_PrintString(_TEXT(0x9738, "\n Gold: "));
    ULTIMA_1a3e_PrintNumber(D_57aa, 4, 0x20); // Gold
    ULTIMA_1850_PrintString(_TEXT(0x9742, "\n\n Keys......."));
    ULTIMA_1a3e_PrintNumber(D_57ac, 2, 0x20); // Keys
    ULTIMA_1850_PrintString(_TEXT(0x9752, "\n Gems......."));
    ULTIMA_1a3e_PrintNumber(D_57ad, 2, 0x20); // Gems
    ULTIMA_1850_PrintString(_TEXT(0x9760, "\n Torches...."));
    ULTIMA_1a3e_PrintNumber(D_57ae, 2, 0x20); // Torches
    if (D_57af != 0)
    {
        // Grapple
        ULTIMA_1850_PrintString(_TEXT(0x976e, "\n Grapple"));
    }
}

void ZSTATS_045e(int param_1)
{
    int local_4;

    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, param_1 + 1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    ULTIMA_16ba_PrintChar(0x10);

    for (local_4 = 0; local_4 < 0xd; local_4++)
    {
        ULTIMA_16ba_PrintChar(0x11);
    }

    ULTIMA_16ba_PrintChar(0x13);

    for (local_4 = 1; local_4 != param_1; local_4++)
    {
        ULTIMA_1bf2_SetTextPosition(0, local_4);
        ULTIMA_16ba_PrintChar(0x17);
        ULTIMA_1bf2_SetTextPosition(0xe, local_4);
        ULTIMA_16ba_PrintChar(0x17);
    }

    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_16ba_PrintChar(0x14);

    for (local_4 = 0; local_4 < 0xd; local_4++)
    {
        ULTIMA_16ba_PrintChar(0x15);
    }

    ULTIMA_16ba_PrintChar(0x16);
}

static int ZSTATS_0518(int param_1, uint param_2)
{
    S_55a8* local_4 = &D_55a8_party[param_1];

    return local_4->equips[0] == param_2 || local_4->equips[1] == param_2 || local_4->equips[2] == param_2 ||
           local_4->equips[3] == param_2 || local_4->equips[4] == param_2 || local_4->equips[5] == param_2;
}

int ZSTATS_056c(int param_1, int notused, byte* param_3, int param_4)
{
    while (1)
    {
        if (--param_1 >= 0)
        {
            if (param_3[param_1] == 0)
            {
                if (param_4 == 0xff)
                    continue;

                if (ZSTATS_0518(param_4, param_1) == 0)
                    continue;
            }

            return param_1;
        }

        break;
    }

    return -1;
}

int ZSTATS_05a4(int param_1, int param_2, byte* param_3, int param_4)
{
    while (1)
    {
        if (++param_1 < param_2)
        {
            if (param_3[param_1] == 0)
            {
                if (param_4 == 0xff)
                    continue;
                if (ZSTATS_0518(param_4, param_1) == 0)
                    continue;
            }

            return param_1;
        }

        break;
    }

    return -1;
}

// show inven count
void ZSTATS_05e2(int param_1, byte* param_2, char** param_3, uint param_4)
{
    char local_6_an;
    int local_4;

    local_6_an = (char)param_2[param_1];
    if (local_6_an != -1)
    {
        if (local_6_an != 0)
        {
            ULTIMA_1a3e_PrintNumber((byte)local_6_an, 2, 0x20);
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x9778, "--"));
        }

        if (param_4 < 0x20)
        {
            ULTIMA_1c9e_SelectCharset(1);
        }

        ULTIMA_16ba_PrintChar(param_4);
        ULTIMA_1c9e_SelectCharset(0);
    }

    if (param_3[param_1][0] == '*')
    {
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_1850_PrintString(_TEXT(0x977c, "\x1c + "));
        ULTIMA_1850_PrintString(&param_3[param_1][1]);
        ULTIMA_1c9e_SelectCharset(0);
    }
    else if (param_3[param_1][0] == '!')
    {
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_1850_PrintString(_TEXT(0x9782, "\x1d + "));
        ULTIMA_1c9e_SelectCharset(0);
        ULTIMA_1850_PrintString(D_1962[40 + param_1]);
    }
    else if (param_3[param_1][0] == '(')
    {
        ULTIMA_1850_PrintString(_TEXT(0x9788, "Moonstone "));
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_16ba_PrintChar((byte)param_3[param_1][1]);
        ULTIMA_1c9e_SelectCharset(0);
    }
    else
    {
        ULTIMA_1850_PrintString(param_3[param_1]);
    }

    local_4 = ULTIMA_1f12_GetCurrentTextX();
    if (local_4 < 0xe)
    {
        register int si = 0xe - local_4;
        local_4 += si;

        do
        {
            ULTIMA_16ba_PrintChar(' ');
            si--;
        } while (si != 0);
    }
    ULTIMA_16ba_PrintChar('\n');
}

#if defined(TARGET_DOS16)
#pragma loop_opt(off)
#endif
// NOT MATCHING: slots and code match up to the END case's loop. The original compiles it with
// the test at the bottom ("cmp ax,0FFFFh; jne body", then the break), so the while test block
// is placed after END; MSC here ends that loop with "inc ax; je exit; jmp body" and places the
// while test after HOME instead, which shifts every jump to it. Tried: the loop as for with
// break, while, an empty for with "local_6 = local_c, local_a++" as the increment (the body
// then matches but the test still jumps out), "(local_c = f(), local_c != -1)" and
// "!= 0xffff" (cmp instead of inc, same placement), goto loops, a do-while entered by goto,
// no pragma, and "#pragma loop_opt(on)" around the END case (SI/DI register variables, which
// the original has nowhere in the function).
// The END loop's own code comes out as the original's (CODE 692/690 bytes, 212 differing)
// written as a do-while entered at its test:
//     local_a = 0; goto L_end_test;
//     do { local_6 = local_c; local_a++; L_end_test:; }
//     while (local_a < 6 && (local_c = ZSTATS_056c(...)) != 0xffff);
// (with "!= -1" it is "inc ax; je"). Its exit then falls into the break's jmp as in the
// original, but MSC still moves the floating while-test block (test, window setup, jump into
// the listing loop) behind HOME's break, the first unconditional jump to it, where the
// original has it behind END's. Moving END before HOME in the source moves the block behind
// END, so the choice is "first jump in code order". Not changed by: HOME's or END's break as
// continue, HOME as "goto" to a label on END's break, the while as for(;!local_8;), as
// "while (local_8 == 0)" or as an if/goto loop. The file keeps the version that is 690 long.
static int ZSTATS_06e8(char* param_1, int param_2, byte* param_3, char** param_4)
{
    int local_10_aa;
    int local_e_am;
    int local_c;
    int local_a;
    bool local_8;
    int local_6;
    int local_4_ab;

    ULTIMA_4e50(param_1);
    ZSTATS_045e(8);
    local_8 = false;
    local_6 = ZSTATS_05a4(-1, param_2, param_3, 0xff);
    if (local_6 == -1)
    {
        ZSTATS_045e(8);
        ULTIMA_1bf2_SetTextPosition(1, 4);
        ULTIMA_1850_PrintString(_TEXT(0x9794, "(None owned!)"));
        ULTIMA_4daa();
        ULTIMA_1b94_SelectTextWindow(2);
        return ULTIMA_266c_GetChar();
    }

    // 092d
    while (!local_8)
    {
        ULTIMA_1b94_SelectTextWindow(1);
        ULTIMA_1bf2_SetTextPosition(1, 1);

        // -> 0756
        for (local_c = local_6; local_c != -1;
             // 0746
             local_c = ZSTATS_05a4(local_c, param_2, param_3, 0xff))
        {
            ZSTATS_05e2(local_c, param_3, param_4, 0x2d);

            ULTIMA_1bf2_SetTextPosition(1, ULTIMA_1cee_GetCurrentTextY());
            if (ULTIMA_1cee_GetCurrentTextY() == 8)
                break;
        }

        // 077f
        local_10_aa = 0;
        if (ZSTATS_056c(local_6, param_2, param_3, 0xff) != -1)
        {
            local_10_aa = 2;
        }

        // 079c
        if (local_c != -1 && ZSTATS_05a4(local_c, param_2, param_3, 0xff) != -1)
        {
            local_10_aa++;
        }

        // 07b8
        switch (local_10_aa)
        {
        case 0:
            // 07d0
            ULTIMA_4daa();
            break;
        case 1:
            // 0806
            ULTIMA_4dea(0x19);
            break;
        case 2:
            // 0810
            ULTIMA_4dea(0x18);
            break;
        case 3:
            // 0816
            ULTIMA_4dea(0x12);
            break;
        }

        // ^ ok

        // 07d3
        ULTIMA_1b94_SelectTextWindow(2);
        local_e_am = ULTIMA_266c_GetChar();
        switch (local_e_am)
        {
        case U5_KEY_UP:
        case U5_KEY_PGUP:
            // 081c
            if (local_e_am == U5_KEY_PGUP)
            {
                local_4_ab = 7;
            }
            else
            {
                local_4_ab = 1;
            }

            for (local_a = 0; local_a < local_4_ab; local_a++)
            {
                if (ZSTATS_056c(local_6, param_2, param_3, 0xff) != -1)
                {
                    local_6 = ZSTATS_056c(local_6, param_2, param_3, 0xff);
                }
            }
            break;

        case U5_KEY_DOWN:
        case U5_KEY_PGDN:
            // 086c
            if (local_e_am == U5_KEY_PGDN)
            {
                local_4_ab = 7;
            }
            else
            {
                local_4_ab = 1;
            }

            for (local_a = 0; local_a < local_4_ab; local_a++)
            {
                if (local_c != -1 && ZSTATS_05a4(local_c, param_2, param_3, 0xff) != -1)
                {
                    local_6 = ZSTATS_05a4(local_6, param_2, param_3, 0xff);
                    local_c = ZSTATS_05a4(local_c, param_2, param_3, 0xff);
                }
            }
            break; // -> 092d

        case U5_KEY_HOME:
            // 08d6
            local_6 = ZSTATS_05a4(-1, param_2, param_3, 0xff);
            break; // -> 092d

        case U5_KEY_END:
            // 08ec
            local_6 = ZSTATS_056c(param_2, param_2, param_3, 0xff);
            // 090f
            for (local_a = 0; local_a < 6 && (local_c = ZSTATS_056c(local_6, param_2, param_3, 0xff)) != -1; local_a++)
            {
                local_6 = local_c;
            }
            break; // -> 092d

        case U5_KEY_LEFT:
        case U5_KEY_RIGHT:
        case U5_KEY_ESC:
        case U5_KEY_SPACE:
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
            // 0948
            local_8 = 1;
            break;
        }
    }

    return local_e_am;
}

#if defined(TARGET_DOS16)
#pragma loop_opt(on)
#endif
// 099A
void ZSTATS_099a(void)
{
    int local_4;

    local_4 = 0;
    do
    {
        D_b9ee[local_4] = D_5820[local_4];
        D_b9f6[local_4] = D_5828[local_4];
        D_ba03[local_4] = D_5840[local_4] == 0xff ? 0xff : 0;
    } while (++local_4 < 8);

    D_b9fe = D_57b0;
    D_b9ff = D_57b1;
    D_ba00 = D_57b3;
    D_ba01 = D_57b4;
    D_ba02 = D_57b5;

    for (local_4 = 0; local_4 < 3; local_4++)
    {
        D_ba0b[local_4] = D_57b6[local_4];
    }

    D_ba0e = D_57ba;
    D_ba0f = D_57bb != 0 ? 0xff : 0;
    D_ba10 = D_57bc;
    D_ba11 = D_57bd;
    D_ba12 = D_57be;
    D_ba13 = D_57bf;
}

// CHECKED
void ZSTATS_0a3a_ZstatsCmd(void)
{
    int local_8;
    byte local_6;
    int local_4_ab;

    local_8 = ZSTATS_0000(1);
    if (local_8 == -2)
    {
        local_8 = 6;
    }
    else if (local_8 < 0)
    {
        return;
    }

    // ec3f
    ULTIMA_1850_PrintString(_TEXT(0x97a2, "\nStatus: "));
    ZSTATS_099a();
    ULTIMA_4efc();
    local_4_ab = local_8 << 1;
    local_6 = 0;

    // ec58
    while (true)
    {
        // 0a78
        if (local_6 == U5_KEY_SPACE || local_6 == U5_KEY_ESC)
        {
            break;
        }

        switch (local_6)
        {
        case U5_KEY_UP:
        case U5_KEY_LEFT:
            // 0aa6
            if (local_4_ab == 0xc)
            {
                local_4_ab = D_585b * 2 - 1;
            }
            else if (local_4_ab > 0)
            {
                local_4_ab--;
            }
            else
            {
                local_4_ab = 0x10;
            }
            break;

        case U5_KEY_RIGHT:
        case U5_KEY_DOWN:
            // 0aea
            if (D_585b * 2 - 1 == local_4_ab)
            {
                local_4_ab = 0xc;
            }
            else
            {
                if (local_4_ab < 0x10)
                {
                    local_4_ab++;
                }
                else
                {
                    local_4_ab = 0;
                }
            }
            break;

        case 0x30:
            // 0af7
            local_4_ab = 0xc;
            break;

        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
            // 0b12 (NOT MATCHING)
            if (local_6 - 0x31 < D_585b)
            {
                local_4_ab = local_6 * 2 - 0x62;
            }
            break;
        }

        // ecab
        ULTIMA_1b94_SelectTextWindow(1);
        if (local_4_ab < 0xc)
        {
            if ((local_4_ab & 1) != 0)
            {
                ZSTATS_02a8(local_4_ab >> 1);
            }
            else
            {
                ZSTATS_0082(local_4_ab >> 1);
            }

            // ed3f
            ULTIMA_1b94_SelectTextWindow(2);
            local_6 = ULTIMA_266c_GetChar();
        }
        else if (local_4_ab == 0xc)
        {
            ZSTATS_039c();

            // ed3f
            ULTIMA_1b94_SelectTextWindow(2);
            local_6 = ULTIMA_266c_GetChar();
        }
        else
        {
            // 0b70
            switch (local_4_ab)
            {
            case 0xd:
                // ed6a
                local_6 = ZSTATS_06e8(_TEXT(0x97ac, "Reagents"), 8, D_5850, D_19d2);
                break;
            case 0xe:
                local_6 = ZSTATS_06e8(_TEXT(0x97b6, "Spells"), 0x30, D_57f0, D_19e2);
                break;
            case 0xf:
                local_6 = ZSTATS_06e8(_TEXT(0x97be, "Items"), 0x26, D_b9ee, D_1916);
                break;
            case 0x10:
                local_6 = ZSTATS_06e8(_TEXT(0x97c4, "Armaments"), 0x30, D_57c0, D_1962);
                break;
            }
        }
    }

    // 0bd6
    ULTIMA_4daa();
    ULTIMA_4e20();
    ULTIMA_4f3c();
    ULTIMA_2900_UpdateVitalsDisplay();
    ULTIMA_1850_PrintString(_TEXT(0x97ce, "Done\n"));
}

static void ZSTATS_0bee(char* param_1)
{
    ULTIMA_1850_PrintString(_TEXT(0x97d4, "\n\n"));
    ULTIMA_1850_PrintString(param_1);
    ULTIMA_1850_PrintString(_TEXT(0x97d8, "\n\nItem: "));
}

static int ZSTATS_0c0a(int param_1)
{
    S_55a8* s = &D_55a8_party[param_1];

    if ((s->equips[2] == 0xff) && (s->equips[3] == 0xff))
    {
        return 2;
    }
    else if (s->equips[2] == 0xff)
    {
        return 0;
    }
    else if ((s->equips[3] == 0xff) && (D_1a7e[s->equips[2]] != 0x30))
    {
        return 1;
    }
    else
    {
        return 0xff;
    }
}

// CHECKED
// equip item
// param_1: whom
// param_2: item id
// NOT MATCHING: block order and tail merging. The original keeps the ammunition message's
// "ZSTATS_0bee(); return 0;" and every other message jumps into it. It places the
// "not strong enough" block last, after the switch cases, and the success code right
// after the strength test. The strength test comes first in the source: its string
// precedes "Ring vanishes!" in CONST, which is what makes ZSTATS' CONST (and DGROUP) match.
// The ring test read D_57c0[param_2] instead of param_2 ("cmp bx,2Ah" after the dec): fixed,
// 710 -> 706 bytes (original 722). Layout: the code after the switch goes to case 0x80's
// break (its first) up to the first jump. In the original that chain is test, store, ring
// (the test jumps far to the weak block, placed after case 2 just before the "!= 0x2a"
// tail); ours is test, weak. Tried: if/else around the weak block (no change), "if (e != 0)
// goto L_ok" (724 bytes), the weak block as a label inside the switch after case 2 with
// "if (local_e_g == 0) goto L_weak" (identical code: MSC still puts it after the test).
static int ZSTATS_0c5c_EquipItem(int param_1, int param_2)
{
    byte* local_10_l;
    int local_e_g;
    int local_c_h;
    int local_a_i;
    int local_8;
    ActorFmt* local_6_b;
    int local_4_c;

#if !defined(MATCHING_BUILD)
    local_6_b = 0;
#endif

    if (D_5893_map_id > 0x7f)
    {
        local_6_b = &D_5c5a[D_ba14[D_589e].actorIdx];
    }

    // ee62
    if (param_2 == 0x1b || param_2 == 0x1d)
    {
        return 0;
    }

    // ee74
    if (param_2 >= 9 && param_2 <= 0xf && D_5893_map_id > 0x7f && D_58a3 == 0)
    {
        // NOT MATCHING
        ZSTATS_0bee(_TEXT(0x97e2, "Thou canst not change armour in heated battle!"));
        return 0;
    }

    // ee94
    if (ZSTATS_0518(param_1, param_2) != 0)
    {
        ULTIMA_6e60(param_1, param_2);
        if (D_57c0[param_2] < 99)
        {
            D_57c0[param_2]++;
        }
        // eeb8
        if (D_5893_map_id <= 0x7f)
        {
            return 0;
        }
        if (param_2 != 0x2a)
        {
            return 0;
        }
        local_8 = ULTIMA_4d76_FindIndex(_TEXT(0x9812, CLASS_LIST), D_55a8_party[param_1].cls);
        // ..eee8
        local_6_b->_0_tile = local_6_b->_1_animTile = D_1ade[local_8];
        return 0;
    }

    if (((param_2 == 0x1a || param_2 == 0x24) && D_57db == 0) || (param_2 == 0x1c && D_57dd == 0))
    {
        // NOT MATCHING
        ZSTATS_0bee(_TEXT(0x981c, "Thou hast no ammunition for that weapon!"));
        return 0;
    }

    // ef16
    local_4_c = 0;
    local_10_l = &D_55a8_party[param_1].equips[0];
    local_a_i = 6;

    // ef30
    for (local_a_i = 0; local_a_i < 6; local_a_i++)
    {
        if (*local_10_l != 0xff)
        {
            local_4_c += D_1aae[*local_10_l];
        }
        // ef41
        local_10_l++;
    }

    local_e_g = D_1aae[param_2] + local_4_c <= D_55a8_party[param_1].str /*strength*/;

    // ef6e
    switch (D_1a7e[param_2])
    {
    case 0x80:
        // efa7
        if (D_55a8_party[param_1].equips[0] != 0xff)
        {
            ZSTATS_0bee(_TEXT(0x9846, "Remove first thy present helm!"));
            return 0;
        }
        // efbc
        local_10_l = &D_55a8_party[param_1].equips[0];
        break;

    case 0x40:
        // f02c
        if (D_55a8_party[param_1].equips[1] != 0xff)
        {
            ZSTATS_0bee(_TEXT(0x9866, "Thou must first remove thine other armour!"));
            return 0;
        }
        // f040..efc6
        local_10_l = &D_55a8_party[param_1].equips[1];
        break;

    case 0x20:
        // f04e
        local_c_h = ZSTATS_0c0a(param_1);
        if (local_c_h == 0xff)
        {
            ZSTATS_0bee(_TEXT(0x9892, "Thou must free one of thy hands first!"));
            return 0;
        }
        // f062
        if (local_c_h == 2)
        {
            local_c_h = 0;
        }
        // f06d..f077..efc6
        local_10_l = &D_55a8_party[param_1].equips[2] + local_c_h;
        break;

    case 0x30:
        // f07e
        local_c_h = ZSTATS_0c0a(param_1);
        if (local_c_h != 2)
        {
            ZSTATS_0bee(_TEXT(0x98ba, "Both hands must be free before thou canst wield that!"));
            return 0;
        }
        // f092..f077..efc6
        local_10_l = &D_55a8_party[param_1].equips[2];
        break;

    case 4:
        // f09c
        if (D_55a8_party[param_1].equips[5] != 0xff)
        {
            ZSTATS_0bee(_TEXT(0x98f0, "Thou must remove thine other amulet!"));
            return 0;
        }
        // f0b0..efc6
        local_10_l = &D_55a8_party[param_1].equips[5];
        break;

    case 2:
        // f0be
        if (D_55a8_party[param_1].equips[4] != 0xff)
        {
            ZSTATS_0bee(_TEXT(0x9916, "Only one magic ring may be worn at a time!"));
            return 0;
        }
        // f0d2..efc6
        local_10_l = &D_55a8_party[param_1].equips[4];
        break;
    }

    // efc9
    if (local_e_g == 0)
    {
        // f0e0
        ZSTATS_0bee(_TEXT(0x9942, "Thou art not strong enough!"));
        return 0;
    }

    // efd2
    *local_10_l = param_2;
    --D_57c0[param_2];
    if ((param_2 == 0x2a || param_2 == 0x2c) && (ULTIMA_2092_RandomRange(0, 0xf) == 0))
    {
        // efff
        ULTIMA_1850_PrintString(_TEXT(0x995e, "\n\nRing vanishes!\n"));
        D_55a8_party[param_1].equips[4] = 0xff;
        ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
        return 1;
    }

    // f0e6
    if (param_2 != 0x2a)
    {
        // ee6e
        return 0;
    }

    // f0ef
    if (D_5893_map_id <= 0x7f)
    {
        // ee6e
        return 0;
    }

    // f0f9
    // eee8
    local_6_b->_0_tile = local_6_b->_1_animTile = TILE_ACTOR_1D;

    // ee6e
    return 0;
}

// NOT MATCHING (checked)
// select item: used in use, ready
int ZSTATS_0f2e(int param_1, int param_2, int param_3)
{
    int local_1a_ac;
    int local_18_al;
    int local_16_an;
    byte* local_14_ap; // equipments
    int local_12_ab;
    int local_10_ad;
    int local_e_a;
    int local_c_ac;
    int local_a_ae;
    int local_8_an;
    int local_6;
    int local_4_ab;

#if !defined(MATCHING_BUILD)
    local_1a_ac = 0;
#endif

    // f10e
    local_8_an = 0;
    local_10_ad = 1;

    if (param_3 == 0x52)
    {
        local_14_ap = D_57c0;
        local_a_ae = 0x30;
    }
    else
    {
        local_14_ap = D_b9ee;
        local_a_ae = 0x26;
    }

    // f142
    while (local_8_an == 0)
    {
        ULTIMA_1b94_SelectTextWindow(1);
        ULTIMA_1bf2_SetTextPosition(1, 1);

        local_4_ab = 0;
        local_12_ab = param_1;

        // f159 -> f20b
        while (local_12_ab != -1)
        {
            // f213
            local_4_ab++;

            if (ULTIMA_1cee_GetCurrentTextY() == local_10_ad)
            {
                ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
                local_1a_ac = local_12_ab;
            }

            // f22b
            if (param_3 == 0x52)
            {
                // f234
                if (ZSTATS_0518(param_2, local_12_ab) == 0)
                {
                    // f15c
                    local_c_ac = 0x20;
                }
                else
                {
                    // f244
                    local_c_ac = D_1ae8[local_12_ab];
                }

                // f161..f17e
                // Equipments
                ZSTATS_05e2(local_12_ab, local_14_ap, D_1962, local_c_ac);
            }
            else
            {
                // f170..f17e
                // Scrolls
                ZSTATS_05e2(local_12_ab, local_14_ap, D_1916, 0x20);
            }

            // f181
            ULTIMA_1bf2_SetTextPosition(1, ULTIMA_1cee_GetCurrentTextY());

            if (ULTIMA_1cee_GetCurrentTextY() - 1 == local_10_ad)
            {
                ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            }

            // f19c
            if (ULTIMA_1cee_GetCurrentTextY() == 0x8)
            {
                break;
            }

            // f1fc
            local_12_ab = ZSTATS_05a4(local_12_ab, local_a_ae, local_14_ap, param_2);
            // -> f20b
        }

        // f1a4
        local_18_al = 0;
        if (ZSTATS_056c(param_1, local_a_ae, local_14_ap, param_2) != -1)
        {
            local_18_al = 2;
        }

        // f1c0
        if (local_12_ab != -1)
        {
            if (ZSTATS_05a4(local_12_ab, local_a_ae, local_14_ap, param_2) != -1)
            {
                local_18_al++;
            }
        }

        // f1db
        switch (local_18_al)
        {
        case 0:
            // f254
            ULTIMA_4daa();
            break;
        case 1:
            // f28a..f28d
            ULTIMA_4dea(0x19);
            break;
        case 2:
            // f294..f28d
            ULTIMA_4dea(0x18);
            break;
        case 3:
            // f29a..f28d
            ULTIMA_4dea(0x12);
            break;
        }

        // L_1077
        // f257
        ULTIMA_1b94_SelectTextWindow(2);
        local_16_an = ULTIMA_266c_GetChar();

        switch (local_16_an)
        {
        case U5_KEY_LEFT:
        case U5_KEY_UP:
        case U5_KEY_PGUP:
            // 10c0
            if (local_16_an == U5_KEY_PGUP)
                local_6 = 7;
            else
                local_6 = 1;

            for (local_e_a = 0; local_e_a < local_6; local_e_a++)
            {
                if (local_10_ad == 4 || local_10_ad == 1)
                {
                    // 10fa
                    if (ZSTATS_056c(param_1, local_a_ae, local_14_ap, param_2) != -1)
                    {
                        param_1 = ZSTATS_056c(param_1, local_a_ae, local_14_ap, param_2);
                    }
                    else
                    {
                        // 111c
                        if (local_10_ad == 4)
                        {
                            // 1121
                            local_10_ad--;
                        }
                    }
                }
                else
                {
                    // 1121
                    local_10_ad--;
                }
                // 1122
            }
            break;

        case U5_KEY_RIGHT:
        case U5_KEY_DOWN:
        case U5_KEY_PGDN:
            // 1130
            if (local_16_an == U5_KEY_PGDN)
                local_6 = 7;
            else
                local_6 = 1;

            for (local_e_a = 0; local_e_a < local_6; local_e_a++)
            {
                if (local_10_ad == 4 || local_10_ad == 7 || local_10_ad >= local_4_ab)
                {
                    // 116f
                    if (local_12_ab != -1 && ZSTATS_05a4(local_12_ab, local_a_ae, local_14_ap, param_2) != -1)
                    {
                        param_1 = ZSTATS_05a4(param_1, local_a_ae, local_14_ap, param_2);
                        local_12_ab = ZSTATS_05a4(local_12_ab, local_a_ae, local_14_ap, param_2);
                    }
                    else
                    {
                        // 11a8
                        if (local_10_ad == 4 && local_4_ab > 4)
                        {
                            local_10_ad++;
                        }
                    }
                }
                else
                {
                    // 11b3
                    local_10_ad++;
                }
                // 11b4
            }
            break;

        case U5_KEY_ENTER:
        case U5_KEY_SPACE:
            // -> 121c;
            // f3fc
            if (param_3 == 0x52)
            {
                local_8_an = ZSTATS_0c5c_EquipItem(param_2, local_1a_ac);
            }
            else
            {
                // f410
                local_8_an = 1;
            }
            // -> f462
            break;
        case U5_KEY_ESC:
            // 1238
            ULTIMA_1850_PrintString(param_3 == 0x52 ? _TEXT(0x9970, "Done\n") : _TEXT(0x9976, "None!\n"));
            local_8_an = 1;
            local_1a_ac = -1;

            // -> f462
            break;
        case U5_KEY_HOME:
            // 11C0
            // f3a0
            param_1 = ZSTATS_05a4(-1, local_a_ae, local_14_ap, param_2);
            local_10_ad = 1;
            // -> f462
            break;
        case U5_KEY_END:
            // f3bc
            // L_11dc
            param_1 = ZSTATS_056c(local_a_ae, local_a_ae, local_14_ap, param_2);

            for (local_10_ad = 1; local_10_ad < 7; local_10_ad++)
            {
                // f3d1
                // L_11f1
                local_1a_ac = ZSTATS_056c(param_1, local_a_ae, local_14_ap, param_2);
                if (local_1a_ac == -1)
                {
                    // L_1207
                    break;
                }
                // f3e7
                // L_1210
                param_1 = local_1a_ac;
            }
            // -> f462
            break;
        }
        // f462
    }

    return local_1a_ac;
}

void ZSTATS_1296_ReadyCmd(void)
{
    int local_6_an;
    int local_4;

    local_6_an = ZSTATS_0000(0);

    if (local_6_an < 0)
        return;

    local_4 = ZSTATS_05a4(-1, 0x30, D_57c0, local_6_an);
    if (local_4 == -1)
    {
        ULTIMA_1850_PrintString(_TEXT(0x997e, "Thou art empty-\nhanded!\n"));
        return;
    }

    ULTIMA_1850_PrintString(_TEXT(0x9998, "Item: "));
    ULTIMA_4efc();
    ULTIMA_1b94_SelectTextWindow(1);

    ULTIMA_4e50(D_55a8_party[local_6_an].name);

    ZSTATS_045e(0x08);

    ZSTATS_0f2e(local_4, local_6_an, 0x52);

    ULTIMA_4daa();
    ULTIMA_4e20();
    ULTIMA_4f3c();
    ULTIMA_2900_UpdateVitalsDisplay();
}
