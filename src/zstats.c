#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "zstats.h"




// Choose the party member a command acts on: in combat the acting player, else ask "Player:". Returns the index, -1 for none, -2 for the whole party (key 0, when allowAll).
static int ZSTATS_0000_ChoosePlayer(int allowAll)
{
    int player;

    player = -1;

    if (D_5893_mapId > 0x80 && (D_ba14_combatants[D_589e_curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        player = D_ba14_combatants[D_589e_curCombatant].entityIdx;
    }
    else
    {
        // 202e
        ULTIMA_1850_PrintString(/*0x96b4*/ "Player: ");
        player = ULTIMA_2d7a_SelectPlayerOrNone(allowAll); // Select player
        if (player >= 0)
        {
            ULTIMA_1850_PrintString(D_55a8_party[player].name);
            if (ULTIMA_1f12_GetCurrentTextX() != 0)
            {
                ULTIMA_16ba_PrintChar('\n');
            }
        }
    }

    // 205b
    if (player == -1)
    {
        ULTIMA_1850_PrintString(/*0x96be*/ "None!\n");
    }
    // 206a
    else if (player == -2)
    {
        ULTIMA_16ba_PrintChar('\n');
    }

    // 2077
    return player;
}

// Z command, first page of a member: name, class, level, status, Str/Int/Dex, HP, HM, Ex, Magic.
static void ZSTATS_0082_ShowPlayerStats(int member)
{
    int classIdx_e;
    int pad;
    int statusIdx;
    S_55a8* player_c;

    ULTIMA_4daa_HideScrollArrow();

    player_c = &D_55a8_party[member];

    ULTIMA_4e50_ShowStatsTitle(player_c->name);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);

    classIdx_e = ULTIMA_4d76_FindIndex(/*0x96c6*/ CLASS_LIST, player_c->cls);
    statusIdx = ULTIMA_4d76_FindIndex(/*0x96d0*/ STATUS_LIST, player_c->status);

    for (pad = 0; pad < D_1a58_classNamePad[classIdx_e]; pad++)
    {
        ULTIMA_16ba_PrintChar(' ');
    }

    ULTIMA_16ba_PrintChar(player_c->gender);
    ULTIMA_1850_PrintString(/*0x96d6*/ " Lv-");
    ULTIMA_1a3e_PrintNumber(player_c->level, 1, 0x20);
    ULTIMA_16ba_PrintChar(' ');
    ULTIMA_1850_PrintString(D_1a44_classNames[classIdx_e]);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_1bf2_SetTextPosition(0, 1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(D_1a6a_statusNames[statusIdx]);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    ULTIMA_1bf2_SetTextPosition(0, 3);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1850_PrintString(/*0x96dc*/ "Str=");
    ULTIMA_1a3e_PrintNumber(player_c->str, 2, 0x30);
    ULTIMA_1850_PrintString(/*0x96e2*/ "  HP:");
    ULTIMA_1a3e_PrintNumber(player_c->hp, 4, 0x20);
    ULTIMA_1850_PrintString(/*0x96e8*/ "\nInt=");
    ULTIMA_1a3e_PrintNumber(player_c->intel, 2, 0x30);
    ULTIMA_1850_PrintString(/*0x96ee*/ "  HM:");
    ULTIMA_1a3e_PrintNumber(player_c->maxHp, 4, 0x20);
    ULTIMA_1850_PrintString(/*0x96f4*/ "\nDex=");
    ULTIMA_1a3e_PrintNumber(player_c->dex, 2, 0x30);
    ULTIMA_1850_PrintString(/*0x96fa*/ "  Ex:");
    ULTIMA_1a3e_PrintNumber(player_c->exp, 4, 0x20);
    ULTIMA_1850_PrintString(/*0x9700*/ "\n\n    Magic:");
    ULTIMA_1a3e_PrintNumber(player_c->mag, 2, 0x20);
}

// Print one equipped item as a line of the Arms page; returns 0 for an empty slot (0xff).
static bool ZSTATS_0278_PrintArm(int item)
{
    if (item == 0xff)
    {
        return 0;
    }
    else
    {
        ULTIMA_16ba_PrintChar(' ');
        ULTIMA_1850_PrintString(D_1962_equipShortNames[item]);
        ULTIMA_16ba_PrintChar('\n');
        return 1;
    }
}

// Z command, second page of a member: the "Arms" he or she has readied ("(None ready)" if empty).
static void ZSTATS_02a8_ShowPlayerArms(int member)
{
    S_55a8* player;
    int shown;

    ULTIMA_4daa_HideScrollArrow();

    player = &D_55a8_party[member];
    ULTIMA_4e50_ShowStatsTitle(player->name);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_UNDERLINE);
    ULTIMA_1850_PrintString(/*0x970e*/ "Arms\n\n");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_UNDERLINE);

    shown = ZSTATS_0278_PrintArm(player->equips[0]);
    shown += ZSTATS_0278_PrintArm(player->equips[1]);
    shown += ZSTATS_0278_PrintArm(player->equips[2]);
    shown += ZSTATS_0278_PrintArm(player->equips[3]);
    shown += ZSTATS_0278_PrintArm(player->equips[4]);
    shown += ZSTATS_0278_PrintArm(player->equips[5]);

    if (shown == 0)
    {
        ULTIMA_1bf2_SetTextPosition(0, 4);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
        ULTIMA_1850_PrintString(/*0x9716*/ "(None ready)");
        ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    }

    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
}

// Z command, the party page: food, gold, keys, gems, torches and the grappling hook.
static void ZSTATS_039c_ShowSupplies(void)
{
    ULTIMA_4daa_HideScrollArrow();
    ULTIMA_4e50_ShowStatsTitle(/*0x9724*/ "Equipment");
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    ULTIMA_1850_PrintString(/*0x972e*/ "\n Food: ");
    ULTIMA_1a3e_PrintNumber(D_57a8_food, 4, 0x20); // Food
    ULTIMA_1850_PrintString(/*0x9738*/ "\n Gold: ");
    ULTIMA_1a3e_PrintNumber(D_57aa_gold, 4, 0x20); // Gold
    ULTIMA_1850_PrintString(/*0x9742*/ "\n\n Keys.......");
    ULTIMA_1a3e_PrintNumber(D_57ac_keys, 2, 0x20); // Keys
    ULTIMA_1850_PrintString(/*0x9752*/ "\n Gems.......");
    ULTIMA_1a3e_PrintNumber(D_57ad_gems, 2, 0x20); // Gems
    ULTIMA_1850_PrintString(/*0x9760*/ "\n Torches....");
    ULTIMA_1a3e_PrintNumber(D_57ae_torches, 2, 0x20); // Torches
    if (D_57af_grapple != 0)
    {
        // Grapple
        ULTIMA_1850_PrintString(/*0x976e*/ "\n Grapple");
    }
}

// Draw the bordered list box (graphic characters 0x10-0x17) used by the item lists, lastRow rows high.
void ZSTATS_045e_DrawListFrame(int lastRow)
{
    int i;

    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x26, lastRow + 1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    ULTIMA_16ba_PrintChar(0x10);

    for (i = 0; i < 0xd; i++)
    {
        ULTIMA_16ba_PrintChar(0x11);
    }

    ULTIMA_16ba_PrintChar(0x13);

    for (i = 1; i != lastRow; i++)
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
}

// 1 when the member has the item among the six equipped slots.
static int ZSTATS_0518_IsEquipped(int member, uint item)
{
    S_55a8* player = &D_55a8_party[member];

    return player->equips[0] == item || player->equips[1] == item || player->equips[2] == item ||
           player->equips[3] == item || player->equips[4] == item || player->equips[5] == item;
}

// The item before index with a count (or, when member is not 0xff, worn by the member); -1 if none.
int ZSTATS_056c_PrevItem(int index, int notused, byte* counts, int member)
{
    while (1)
    {
        if (--index >= 0)
        {
            if (counts[index] == 0)
            {
                if (member == 0xff)
                    continue;

                if (ZSTATS_0518_IsEquipped(member, index) == 0)
                    continue;
            }

            return index;
        }

        break;
    }

    return -1;
}

// The item after index with a count (or worn by the member), below count; -1 if none.
int ZSTATS_05a4_NextItem(int index, int count, byte* counts, int member)
{
    while (1)
    {
        if (++index < count)
        {
            if (counts[index] == 0)
            {
                if (member == 0xff)
                    continue;
                if (ZSTATS_0518_IsEquipped(member, index) == 0)
                    continue;
            }

            return index;
        }

        break;
    }

    return -1;
}

// Print one row of an item list: count ("--" when worn), marker, name (a leading '*', '!' or '(' in the name selects a special form), padded to 14 columns.
// show inven count
void ZSTATS_05e2_PrintItemRow(int index, byte* counts, char** names, uint marker)
{
    char owned;
    int column_n;

    owned = (char)counts[index];
    if (owned != -1)
    {
        if (owned != 0)
        {
            ULTIMA_1a3e_PrintNumber((byte)owned, 2, 0x20);
        }
        else
        {
            ULTIMA_1850_PrintString(/*0x9778*/ "--");
        }

        if (marker < 0x20)
        {
            ULTIMA_1c9e_SelectCharset(1);
        }

        ULTIMA_16ba_PrintChar(marker);
        ULTIMA_1c9e_SelectCharset(0);
    }

    if (names[index][0] == '*')
    {
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_1850_PrintString(/*0x977c*/ "\x1c + ");
        ULTIMA_1850_PrintString(&names[index][1]);
        ULTIMA_1c9e_SelectCharset(0);
    }
    else if (names[index][0] == '!')
    {
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_1850_PrintString(/*0x9782*/ "\x1d + ");
        ULTIMA_1c9e_SelectCharset(0);
        ULTIMA_1850_PrintString(D_1962_equipShortNames[40 + index]);
    }
    else if (names[index][0] == '(')
    {
        ULTIMA_1850_PrintString(/*0x9788*/ "Moonstone ");
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_16ba_PrintChar((byte)names[index][1]);
        ULTIMA_1c9e_SelectCharset(0);
    }
    else
    {
        ULTIMA_1850_PrintString(names[index]);
    }

    column_n = ULTIMA_1f12_GetCurrentTextX();
    if (column_n < 0xe)
    {
        register int si = 0xe - column_n;
        column_n += si;

        do
        {
            ULTIMA_16ba_PrintChar(' ');
            si--;
        } while (si != 0);
    }
    ULTIMA_16ba_PrintChar('\n');
}

#pragma loop_opt(off)
// Scrolling list of the Z command (reagents, spells, items, armaments); arrows scroll, Esc/space/1-6/0 leave. Returns the key that ended it.
// CHECKED: the selection loop is written with gotos. The original places the loop test
// (with the window setup that starts the next pass) right after the END case, which falls
// into it, and the return after the switch's dispatch. A while loop's test block floats
// behind the first case break that jumps to it (HOME's); with the test written inside the
// switch after END, followed by the window setup and "goto L_list" (the for's init block then
// floats in behind that jump), every case ends in "goto L_test" and the exit is a separate
// label after the switch, the layout is the original's. "break" to a return after the switch
// instead of "goto L_exit" lets the return block float up behind the test (107 bytes). The END
// loop is a do-while entered at its test, with != 0xffff for the original's "cmp ax,0FFFFh".
static int ZSTATS_06e8_BrowseList(char* title, int count, byte* counts, char** names)
{
    int arrows;
    int key;
    int last;
    int step_d;
    bool leave_c;
    int first_h;
    int steps;

    ULTIMA_4e50_ShowStatsTitle(title);
    ZSTATS_045e_DrawListFrame(8);
    leave_c = false;
    first_h = ZSTATS_05a4_NextItem(-1, count, counts, 0xff);
    if (first_h == -1)
    {
        ZSTATS_045e_DrawListFrame(8);
        ULTIMA_1bf2_SetTextPosition(1, 4);
        ULTIMA_1850_PrintString(/*0x9794*/ "(None owned!)");
        ULTIMA_4daa_HideScrollArrow();
        ULTIMA_1b94_SelectTextWindow(2);
        return ULTIMA_266c_GetChar();
    }

    // 092d: enter the loop at its test, which sits after the END case
    goto L_test;
    {
    L_list:
        // -> 0756
        for (last = first_h; last != -1;
             // 0746
             last = ZSTATS_05a4_NextItem(last, count, counts, 0xff))
        {
            ZSTATS_05e2_PrintItemRow(last, counts, names, 0x2d);

            ULTIMA_1bf2_SetTextPosition(1, ULTIMA_1cee_GetCurrentTextY());
            if (ULTIMA_1cee_GetCurrentTextY() == 8)
                break;
        }

        // 077f
        arrows = 0;
        if (ZSTATS_056c_PrevItem(first_h, count, counts, 0xff) != -1)
        {
            arrows = 2;
        }

        // 079c
        if (last != -1 && ZSTATS_05a4_NextItem(last, count, counts, 0xff) != -1)
        {
            arrows++;
        }

        // 07b8
        switch (arrows)
        {
        case 0:
            // 07d0
            ULTIMA_4daa_HideScrollArrow();
            break;
        case 1:
            // 0806
            ULTIMA_4dea_ShowScrollArrow(0x19);
            break;
        case 2:
            // 0810
            ULTIMA_4dea_ShowScrollArrow(0x18);
            break;
        case 3:
            // 0816
            ULTIMA_4dea_ShowScrollArrow(0x12);
            break;
        }

        // ^ ok

        // 07d3
        ULTIMA_1b94_SelectTextWindow(2);
        key = ULTIMA_266c_GetChar();
        switch (key)
        {
        case U5_KEY_UP:
        case U5_KEY_PGUP:
            // 081c
            if (key == U5_KEY_PGUP)
            {
                steps = 7;
            }
            else
            {
                steps = 1;
            }

            for (step_d = 0; step_d < steps; step_d++)
            {
                if (ZSTATS_056c_PrevItem(first_h, count, counts, 0xff) != -1)
                {
                    first_h = ZSTATS_056c_PrevItem(first_h, count, counts, 0xff);
                }
            }
            goto L_test;

        case U5_KEY_DOWN:
        case U5_KEY_PGDN:
            // 086c
            if (key == U5_KEY_PGDN)
            {
                steps = 7;
            }
            else
            {
                steps = 1;
            }

            for (step_d = 0; step_d < steps; step_d++)
            {
                if (last != -1 && ZSTATS_05a4_NextItem(last, count, counts, 0xff) != -1)
                {
                    first_h = ZSTATS_05a4_NextItem(first_h, count, counts, 0xff);
                    last = ZSTATS_05a4_NextItem(last, count, counts, 0xff);
                }
            }
            goto L_test;

        case U5_KEY_HOME:
            // 08d6
            first_h = ZSTATS_05a4_NextItem(-1, count, counts, 0xff);
            goto L_test;

        case U5_KEY_END:
            // 08ec
            first_h = ZSTATS_056c_PrevItem(count, count, counts, 0xff);
            // 090f
            step_d = 0;
            goto L_end_test;
            do
            {
                first_h = last;
                step_d++;
            L_end_test:;
            } while (step_d < 6 && (last = ZSTATS_056c_PrevItem(first_h, count, counts, 0xff)) != 0xffff);
        L_test:
            if (leave_c)
                goto L_exit;
            ULTIMA_1b94_SelectTextWindow(1);
            ULTIMA_1bf2_SetTextPosition(1, 1);
            goto L_list;

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
            leave_c = 1;
            goto L_test;
        default:
            goto L_test;
        }
    }

L_exit:
    return key;
}

#pragma loop_opt(on)
// Copy the saved game's scrolls, potions and quest items into the D_b9ee_itemCounts table of owned item counts (the Use list).
// 099A
void ZSTATS_099a_FillItemCounts(void)
{
    int i;

    i = 0;
    do
    {
        D_b9ee_itemCounts[i] = D_5820_scrolls[i];
        D_b9f6_potionCounts[i] = D_5828_potions[i];
        D_ba03_moonstoneFlags[i] = D_5840_moonstoneMap[i] == 0xff ? 0xff : 0;
    } while (++i < 8);

    D_b9fe_carpetCount = D_57b0_carpets;
    D_b9ff_skullKeyCount = D_57b1_skullKeys;
    D_ba00_amuletOwned = D_57b3_amulet;
    D_ba01_crownOwned = D_57b4_crown;
    D_ba02_sceptreOwned = D_57b5_sceptre;

    for (i = 0; i < 3; i++)
    {
        D_ba0b_shardFlags[i] = D_57b6_shards[i];
    }

    D_ba0e_spyglassCount = D_57ba_spyglasses;
    D_ba0f_capePlansOwned = D_57bb_hmsCapePlans != 0 ? 0xff : 0;
    D_ba10_sextantCount = D_57bc_sextants;
    D_ba11_pocketWatchOwned = D_57bd_pocketWatch;
    D_ba12_blackBadgeOwned = D_57be_blackBadge;
    D_ba13_sandalwoodBoxOwned = D_57bf_sandalwoodBox;
}

// Z command: flip through the party's stats pages (member stats and arms, supplies, reagents, spells, items, armaments).
// CHECKED
void ZSTATS_0a3a_ZstatsCmd(void)
{
    int member;
    byte key;
    int page_c;

    member = ZSTATS_0000_ChoosePlayer(1);
    if (member == -2)
    {
        member = 6;
    }
    else if (member < 0)
    {
        return;
    }

    // ec3f
    ULTIMA_1850_PrintString(/*0x97a2*/ "\nStatus: ");
    ZSTATS_099a_FillItemCounts();
    ULTIMA_4efc_OpenStatsDivider();
    page_c = member << 1;
    key = 0;

    // ec58
    while (true)
    {
        // 0a78
        if (key == U5_KEY_SPACE || key == U5_KEY_ESC)
        {
            break;
        }

        switch (key)
        {
        case U5_KEY_UP:
        case U5_KEY_LEFT:
            // 0aa6
            if (page_c == 0xc)
            {
                page_c = D_585b_partyCount * 2 - 1;
            }
            else if (page_c > 0)
            {
                page_c--;
            }
            else
            {
                page_c = 0x10;
            }
            break;

        case U5_KEY_RIGHT:
        case U5_KEY_DOWN:
            // 0aea
            if (D_585b_partyCount * 2 - 1 == page_c)
            {
                page_c = 0xc;
            }
            else
            {
                if (page_c < 0x10)
                {
                    page_c++;
                }
                else
                {
                    page_c = 0;
                }
            }
            break;

        case 0x30:
            // 0af7
            page_c = 0xc;
            break;

        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
            // 0b12 (NOT MATCHING)
            if (key - 0x31 < D_585b_partyCount)
            {
                page_c = key * 2 - 0x62;
            }
            break;
        }

        // ecab
        ULTIMA_1b94_SelectTextWindow(1);
        if (page_c < 0xc)
        {
            if ((page_c & 1) != 0)
            {
                ZSTATS_02a8_ShowPlayerArms(page_c >> 1);
            }
            else
            {
                ZSTATS_0082_ShowPlayerStats(page_c >> 1);
            }

            // ed3f
            ULTIMA_1b94_SelectTextWindow(2);
            key = ULTIMA_266c_GetChar();
        }
        else if (page_c == 0xc)
        {
            ZSTATS_039c_ShowSupplies();

            // ed3f
            ULTIMA_1b94_SelectTextWindow(2);
            key = ULTIMA_266c_GetChar();
        }
        else
        {
            // 0b70
            switch (page_c)
            {
            case 0xd:
                // ed6a
                key = ZSTATS_06e8_BrowseList(/*0x97ac*/ "Reagents", 8, D_5850_reagents, D_19d2_reagentNames);
                break;
            case 0xe:
                key = ZSTATS_06e8_BrowseList(/*0x97b6*/ "Spells", 0x30, D_57f0_mixedSpells, D_19e2_spellNames);
                break;
            case 0xf:
                key = ZSTATS_06e8_BrowseList(/*0x97be*/ "Items", 0x26, D_b9ee_itemCounts, D_1916_itemNames);
                break;
            case 0x10:
                key = ZSTATS_06e8_BrowseList(/*0x97c4*/ "Armaments", 0x30, D_57c0_equipment, D_1962_equipShortNames);
                break;
            }
        }
    }

    // 0bd6
    ULTIMA_4daa_HideScrollArrow();
    ULTIMA_4e20_ClearStatsTitle();
    ULTIMA_4f3c_DrawStatsDivider();
    ULTIMA_2900_UpdateVitalsDisplay();
    ULTIMA_1850_PrintString(/*0x97ce*/ "Done\n");
}

// Print a refusal message and the "Item:" prompt again.
static void ZSTATS_0bee_PrintEquipRefusal(char* message)
{
    ULTIMA_1850_PrintString(/*0x97d4*/ "\n\n");
    ULTIMA_1850_PrintString(message);
    ULTIMA_1850_PrintString(/*0x97d8*/ "\n\nItem: ");
}

// Which hand is free: 2 both, 0 the first (equips[2]), 1 the second (equips[3], when the first holds a one-handed item), 0xff none.
static int ZSTATS_0c0a_FreeHandSlot(int member)
{
    S_55a8* player = &D_55a8_party[member];

    if ((player->equips[2] == 0xff) && (player->equips[3] == 0xff))
    {
        return 2;
    }
    else if (player->equips[2] == 0xff)
    {
        return 0;
    }
    else if ((player->equips[3] == 0xff) && (D_1a7e_equipSlotKind[player->equips[2]] != 0x30))
    {
        return 1;
    }
    else
    {
        return 0xff;
    }
}

// Ready or unready an item for a member, with the rules of the original (no armour change in battle, ammunition, strength, one helm, one ring, one amulet, hands). Returns 1 when a magic ring vanished.
// CLEAN ROOM (written from the disassembly). param_1: party member, param_2: item.
// Returns 1 when the ring vanished. An else-if chain with one final return 0 (so the
// armour message jumps forward into the ammunition one), early returns inside the
// switch, and the strength test as "if (strong) goto L_wear; message; return 0;": the
// goto puts the message block after the last case, ahead of the ring's tile change.
static int ZSTATS_0c5c_EquipOrRemove(int member, int item)
{
    byte* slot_i;
    int strongEnough_g;
    int hand;
    int i;
    int classIdx_e;
    ActorFmt* actor_g;
    uint load;

    if (D_5893_mapId > 0x7f)
    {
        actor_g = &D_5c5a_actors[D_ba14_combatants[D_589e_curCombatant].actorIdx];
    }

    if (item == 0x1b || item == 0x1d)
    {
        return 0;
    }
    if (item >= 9 && item <= 0xf && D_5893_mapId > 0x7f && D_58a3_combatOver == 0)
    {
        ZSTATS_0bee_PrintEquipRefusal(/*0x97e2*/ "Thou canst not change armour in heated battle!");
    }
    else if (ZSTATS_0518_IsEquipped(member, item) != 0)
    {
        ULTIMA_6e60_Unequip(member, item);
        if (D_57c0_equipment[item] < 99)
        {
            D_57c0_equipment[item]++;
        }

        if (D_5893_mapId > 0x7f && item == 0x2a)
        {
            classIdx_e = ULTIMA_4d76_FindIndex(/*0x9812*/ "AMBFDTPRS", D_55a8_party[member].cls);
            actor_g->tile = actor_g->animTile = D_1ade_classTile[classIdx_e];
        }
    }
    else if (((item == 0x1a || item == 0x24) && D_57db_arrows == 0) || (item == 0x1c && D_57dd_quarrels == 0))
    {
        ZSTATS_0bee_PrintEquipRefusal(/*0x981c*/ "Thou hast no ammunition for that weapon!");
    }
    else
    {
        load = 0;
        slot_i = D_55a8_party[member].equips;
        for (i = 0; i < 6; i++)
        {
            if (*slot_i != 0xff)
            {
                load += D_1aae_equipWeight[*slot_i];
            }
            slot_i++;
        }

        strongEnough_g = D_1aae_equipWeight[item] + load <= D_55a8_party[member].str;

        switch (D_1a7e_equipSlotKind[item])
        {
        case 0x80:
            if (D_55a8_party[member].equips[0] != 0xff)
            {
                ZSTATS_0bee_PrintEquipRefusal(/*0x9846*/ "Remove first thy present helm!");
                return 0;
            }
            slot_i = &D_55a8_party[member].equips[0];
            break;
        case 0x40:
            if (D_55a8_party[member].equips[1] != 0xff)
            {
                ZSTATS_0bee_PrintEquipRefusal(/*0x9866*/ "Thou must first remove thine other armour!");
                return 0;
            }
            slot_i = &D_55a8_party[member].equips[1];
            break;
        case 0x20:
            hand = ZSTATS_0c0a_FreeHandSlot(member);
            if (hand == 0xff)
            {
                ZSTATS_0bee_PrintEquipRefusal(/*0x9892*/ "Thou must free one of thy hands first!");
                return 0;
            }
            if (hand == 2)
            {
                hand = 0;
            }
            slot_i = &D_55a8_party[member].equips[hand + 2];
            break;
        case 0x30:
            hand = ZSTATS_0c0a_FreeHandSlot(member);
            if (hand != 2)
            {
                ZSTATS_0bee_PrintEquipRefusal(/*0x98ba*/ "Both hands must be free before thou canst wield that!");
                return 0;
            }
            slot_i = &D_55a8_party[member].equips[2];
            break;
        case 4:
            if (D_55a8_party[member].equips[5] != 0xff)
            {
                ZSTATS_0bee_PrintEquipRefusal(/*0x98f0*/ "Thou must remove thine other amulet!");
                return 0;
            }
            slot_i = &D_55a8_party[member].equips[5];
            break;
        case 2:
            if (D_55a8_party[member].equips[4] != 0xff)
            {
                ZSTATS_0bee_PrintEquipRefusal(/*0x9916*/ "Only one magic ring may be worn at a time!");
                return 0;
            }
            slot_i = &D_55a8_party[member].equips[4];
            break;
        }

        if (strongEnough_g != 0)
        {
            goto L_wear;
        }
        ZSTATS_0bee_PrintEquipRefusal(/*0x9942*/ "Thou art not strong enough!");
        return 0;

L_wear:
        *slot_i = item;
        D_57c0_equipment[item]--;

        if ((item == 0x2a || item == 0x2c) && ULTIMA_2092_RandomRange(0, 0xf) == 0)
        {
            ULTIMA_1850_PrintString(/*0x995e*/ "\n\nRing vanishes!\n");
            D_55a8_party[member].equips[4] = 0xff;
            ULTIMA_43ae_AudioSweepTone(0x4b0, 0x7d0, 1, 0x28);
            return 1;
        }

        if (item == 0x2a && D_5893_mapId > 0x7f)
        {
            actor_g->tile = actor_g->animTile = 0x1d;
        }
    }
    return 0;
}

// The cursor list of Ready (mode 0x52) and Use: arrows move, Enter/space select. Returns the item chosen, -1 for Esc.
// CHECKED: the two page loops (UP, DOWN) keep their trip counts in two temporaries
// (bp-1Ah, bp-1Ch) only when the DOWN loop sits in a block that declares a local.
// The listing loop is a for whose increment shares its store with the init, the 0518 test
// reads "!= 0" first, and HOME/END come before ENTER/ESC.
// select item: used in use, ready
int ZSTATS_0f2e_SelectItem(int first, int member, int mode)
{
    int selected_f;
    int arrows_b;
    int key_g;
    byte* counts_d; // equipments
    int item_a;
    int cursorRow_i;
    int step_d;
    int marker_n;
    int count_g;
    int leave_c;
    int steps;
    int rows_e;

    // f10e
    leave_c = 0;
    cursorRow_i = 1;

    if (mode == 0x52)
    {
        counts_d = D_57c0_equipment;
        count_g = 0x30;
    }
    else
    {
        counts_d = D_b9ee_itemCounts;
        count_g = 0x26;
    }

    // f142
    while (leave_c == 0)
    {
        ULTIMA_1b94_SelectTextWindow(1);
        ULTIMA_1bf2_SetTextPosition(1, 1);

        rows_e = 0;
        // f159 -> f20b
        for (item_a = first; item_a != -1; item_a = ZSTATS_05a4_NextItem(item_a, count_g, counts_d, member))
        {
            // f213
            rows_e++;

            if (ULTIMA_1cee_GetCurrentTextY() == cursorRow_i)
            {
                ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
                selected_f = item_a;
            }

            // f22b
            if (mode == 0x52)
            {
                // f234
                if (ZSTATS_0518_IsEquipped(member, item_a) != 0)
                {
                    // f244
                    marker_n = D_1ae8_equipMarker[item_a];
                }
                else
                {
                    // f15c
                    marker_n = 0x20;
                }

                // f161..f17e
                // Equipments
                ZSTATS_05e2_PrintItemRow(item_a, counts_d, D_1962_equipShortNames, marker_n);
            }
            else
            {
                // f170..f17e
                // Scrolls
                ZSTATS_05e2_PrintItemRow(item_a, counts_d, D_1916_itemNames, 0x20);
            }

            // f181
            ULTIMA_1bf2_SetTextPosition(1, ULTIMA_1cee_GetCurrentTextY());

            if (ULTIMA_1cee_GetCurrentTextY() - 1 == cursorRow_i)
            {
                ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            }

            // f19c
            if (ULTIMA_1cee_GetCurrentTextY() == 0x8)
            {
                break;
            }
        }

        // f1a4
        arrows_b = 0;
        if (ZSTATS_056c_PrevItem(first, count_g, counts_d, member) != -1)
        {
            arrows_b = 2;
        }

        // f1c0
        if (item_a != -1)
        {
            if (ZSTATS_05a4_NextItem(item_a, count_g, counts_d, member) != -1)
            {
                arrows_b++;
            }
        }

        // f1db
        switch (arrows_b)
        {
        case 0:
            // f254
            ULTIMA_4daa_HideScrollArrow();
            break;
        case 1:
            // f28a..f28d
            ULTIMA_4dea_ShowScrollArrow(0x19);
            break;
        case 2:
            // f294..f28d
            ULTIMA_4dea_ShowScrollArrow(0x18);
            break;
        case 3:
            // f29a..f28d
            ULTIMA_4dea_ShowScrollArrow(0x12);
            break;
        }

        // L_1077
        // f257
        ULTIMA_1b94_SelectTextWindow(2);
        key_g = ULTIMA_266c_GetChar();

        switch (key_g)
        {
        case U5_KEY_LEFT:
        case U5_KEY_UP:
        case U5_KEY_PGUP:
            // 10c0
            if (key_g == U5_KEY_PGUP)
                steps = 7;
            else
                steps = 1;

            for (step_d = 0; step_d < steps; step_d++)
            {
                if (cursorRow_i == 4 || cursorRow_i == 1)
                {
                    // 10fa
                    if (ZSTATS_056c_PrevItem(first, count_g, counts_d, member) != -1)
                    {
                        first = ZSTATS_056c_PrevItem(first, count_g, counts_d, member);
                    }
                    else
                    {
                        // 111c
                        if (cursorRow_i == 4)
                        {
                            // 1121
                            cursorRow_i--;
                        }
                    }
                }
                else
                {
                    // 1121
                    cursorRow_i--;
                }
                // 1122
            }
            break;

        case U5_KEY_RIGHT:
        case U5_KEY_DOWN:
        case U5_KEY_PGDN:
            // 1130
            if (key_g == U5_KEY_PGDN)
                steps = 7;
            else
                steps = 1;

            // A block with a declaration gives this loop its own trip-count temporary
            // (bp-1Ch); without it both page loops share bp-1Ah. The local is unused.
            {
                int local_x;

                for (step_d = 0; step_d < steps; step_d++)
                {
                    if (cursorRow_i == 4 || cursorRow_i == 7 || cursorRow_i >= rows_e)
                    {
                        // 116f
                        if (item_a != -1 && ZSTATS_05a4_NextItem(item_a, count_g, counts_d, member) != -1)
                        {
                            first = ZSTATS_05a4_NextItem(first, count_g, counts_d, member);
                            item_a = ZSTATS_05a4_NextItem(item_a, count_g, counts_d, member);
                        }
                        else
                        {
                            // 11a8
                            if (cursorRow_i == 4 && rows_e > 4)
                            {
                                cursorRow_i++;
                            }
                        }
                    }
                    else
                    {
                        // 11b3
                        cursorRow_i++;
                    }
                    // 11b4
                }
            }
            break;

        case U5_KEY_HOME:
            // 11C0
            // f3a0
            first = ZSTATS_05a4_NextItem(-1, count_g, counts_d, member);
            cursorRow_i = 1;
            // -> f462
            break;
        case U5_KEY_END:
            // f3bc
            // L_11dc
            first = ZSTATS_056c_PrevItem(count_g, count_g, counts_d, member);

            for (cursorRow_i = 1; cursorRow_i < 7; cursorRow_i++)
            {
                // f3d1
                // L_11f1
                selected_f = ZSTATS_056c_PrevItem(first, count_g, counts_d, member);
                if (selected_f == -1)
                {
                    // L_1207
                    break;
                }
                // f3e7
                // L_1210
                first = selected_f;
            }
            // -> f462
            break;
        case U5_KEY_ENTER:
        case U5_KEY_SPACE:
            // -> 121c;
            // f3fc
            if (mode == 0x52)
            {
                leave_c = ZSTATS_0c5c_EquipOrRemove(member, selected_f);
            }
            else
            {
                // f410
                leave_c = 1;
            }
            // -> f462
            break;
        case U5_KEY_ESC:
            // 1238
            ULTIMA_1850_PrintString(mode == 0x52 ? /*0x9970*/ "Done\n" : /*0x9976*/ "None!\n");
            leave_c = 1;
            selected_f = -1;

            // -> f462
            break;
        }
        // f462
    }

    return selected_f;
}

// R command: ready weapons and armour for a party member.
void ZSTATS_1296_ReadyCmd(void)
{
    int member;
    int first;

    member = ZSTATS_0000_ChoosePlayer(0);

    if (member < 0)
        return;

    first = ZSTATS_05a4_NextItem(-1, 0x30, D_57c0_equipment, member);
    if (first == -1)
    {
        ULTIMA_1850_PrintString(/*0x997e*/ "Thou art empty-\nhanded!\n");
        return;
    }

    ULTIMA_1850_PrintString(/*0x9998*/ "Item: ");
    ULTIMA_4efc_OpenStatsDivider();
    ULTIMA_1b94_SelectTextWindow(1);

    ULTIMA_4e50_ShowStatsTitle(D_55a8_party[member].name);

    ZSTATS_045e_DrawListFrame(0x08);

    ZSTATS_0f2e_SelectItem(first, member, 0x52);

    ULTIMA_4daa_HideScrollArrow();
    ULTIMA_4e20_ClearStatsTitle();
    ULTIMA_4f3c_DrawStatsDivider();
    ULTIMA_2900_UpdateVitalsDisplay();
}
