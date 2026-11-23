#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "zstats.h"




// Choose the party member a command acts on: in combat the acting player, else ask "Player:". Returns the index, -1 for none, -2 for the whole party (key 0, when allowAll).
static int ChoosePlayer(int allowAll)
{
    int player;

    player = -1;

    if (mapId > 0x80 && (combatants[curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        player = combatants[curCombatant].entityIdx;
    }
    else
    {
        // 202e
        PrintString(/*0x96b4*/ "Player: ");
        player = SelectPlayerOrNone(allowAll); // Select player
        if (player >= 0)
        {
            PrintString(party[player].name);
            if (GetCurrentTextX() != 0)
            {
                PrintChar('\n');
            }
        }
    }

    // 205b
    if (player == -1)
    {
        PrintString(/*0x96be*/ "None!\n");
    }
    // 206a
    else if (player == -2)
    {
        PrintChar('\n');
    }

    // 2077
    return player;
}

// Z command, first page of a member: name, class, level, status, Str/Int/Dex, HP, HM, Ex, Magic.
static void ShowPlayerStats(int member)
{
    int classIdx_e;
    int pad;
    int statusIdx;
    S_55a8* player_c;

    HideScrollArrow();

    player_c = &party[member];

    ShowStatsTitle(player_c->name);
    SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    SetTextWindowSize(1, 0x18, 1, 0x27, 9);

    classIdx_e = FindIndex(/*0x96c6*/ CLASS_LIST, player_c->cls);
    statusIdx = FindIndex(/*0x96d0*/ STATUS_LIST, player_c->status);

    for (pad = 0; pad < classNamePad[classIdx_e]; pad++)
    {
        PrintChar(' ');
    }

    PrintChar(player_c->gender);
    PrintString(/*0x96d6*/ " Lv-");
    PrintNumber(player_c->level, 1, 0x20);
    PrintChar(' ');
    PrintString(classNames[classIdx_e]);
    SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    SetTextPosition(0, 1);
    PrintChar(CTRL_CHAR_CENTER_TEXT);
    PrintString(statusNames[statusIdx]);
    SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    SetTextPosition(0, 3);
    PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    PrintString(/*0x96dc*/ "Str=");
    PrintNumber(player_c->str, 2, 0x30);
    PrintString(/*0x96e2*/ "  HP:");
    PrintNumber(player_c->hp, 4, 0x20);
    PrintString(/*0x96e8*/ "\nInt=");
    PrintNumber(player_c->intel, 2, 0x30);
    PrintString(/*0x96ee*/ "  HM:");
    PrintNumber(player_c->maxHp, 4, 0x20);
    PrintString(/*0x96f4*/ "\nDex=");
    PrintNumber(player_c->dex, 2, 0x30);
    PrintString(/*0x96fa*/ "  Ex:");
    PrintNumber(player_c->exp, 4, 0x20);
    PrintString(/*0x9700*/ "\n\n    Magic:");
    PrintNumber(player_c->mag, 2, 0x20);
}

// Print one equipped item as a line of the Arms page; returns 0 for an empty slot (0xff).
static bool PrintArm(int item)
{
    if (item == 0xff)
    {
        return 0;
    }
    else
    {
        PrintChar(' ');
        PrintString(equipShortNames[item]);
        PrintChar('\n');
        return 1;
    }
}

// Z command, second page of a member: the "Arms" he or she has readied ("(None ready)" if empty).
static void ShowPlayerArms(int member)
{
    S_55a8* player;
    int shown;

    HideScrollArrow();

    player = &party[member];
    ShowStatsTitle(player->name);
    SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    PrintChar(CTRL_CHAR_CENTER_TEXT);
    PrintChar(CTRL_CHAR_TOGGLE_UNDERLINE);
    PrintString(/*0x970e*/ "Arms\n\n");
    PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    PrintChar(CTRL_CHAR_TOGGLE_UNDERLINE);

    shown = PrintArm(player->equips[0]);
    shown += PrintArm(player->equips[1]);
    shown += PrintArm(player->equips[2]);
    shown += PrintArm(player->equips[3]);
    shown += PrintArm(player->equips[4]);
    shown += PrintArm(player->equips[5]);

    if (shown == 0)
    {
        SetTextPosition(0, 4);
        PrintChar(CTRL_CHAR_CENTER_TEXT);
        PrintString(/*0x9716*/ "(None ready)");
        PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    }

    SetTextWindowSize(1, 0x18, 1, 0x27, 9);
}

// Z command, the party page: food, gold, keys, gems, torches and the grappling hook.
static void ShowSupplies(void)
{
    HideScrollArrow();
    ShowStatsTitle(/*0x9724*/ "Equipment");
    SetTextWindowSize(1, 0x18, 1, 0x26, 9);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    PrintString(/*0x972e*/ "\n Food: ");
    PrintNumber(food, 4, 0x20); // Food
    PrintString(/*0x9738*/ "\n Gold: ");
    PrintNumber(gold, 4, 0x20); // Gold
    PrintString(/*0x9742*/ "\n\n Keys.......");
    PrintNumber(keys, 2, 0x20); // Keys
    PrintString(/*0x9752*/ "\n Gems.......");
    PrintNumber(gems, 2, 0x20); // Gems
    PrintString(/*0x9760*/ "\n Torches....");
    PrintNumber(torches, 2, 0x20); // Torches
    if (grapple != 0)
    {
        // Grapple
        PrintString(/*0x976e*/ "\n Grapple");
    }
}

// Draw the bordered list box (graphic characters 0x10-0x17) used by the item lists, lastRow rows high.
void DrawListFrame(int lastRow)
{
    int i;

    SetTextWindowSize(1, 0x18, 1, 0x26, lastRow + 1);
    PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    SetTextWindowSize(1, 0x18, 1, 0x27, 9);
    PrintChar(0x10);

    for (i = 0; i < 0xd; i++)
    {
        PrintChar(0x11);
    }

    PrintChar(0x13);

    for (i = 1; i != lastRow; i++)
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
}

// 1 when the member has the item among the six equipped slots.
static int IsEquipped(int member, uint item)
{
    S_55a8* player = &party[member];

    return player->equips[0] == item || player->equips[1] == item || player->equips[2] == item ||
           player->equips[3] == item || player->equips[4] == item || player->equips[5] == item;
}

// The item before index with a count (or, when member is not 0xff, worn by the member); -1 if none.
int PrevItem(int index, int notused, byte* counts, int member)
{
    while (1)
    {
        if (--index >= 0)
        {
            if (counts[index] == 0)
            {
                if (member == 0xff)
                    continue;

                if (IsEquipped(member, index) == 0)
                    continue;
            }

            return index;
        }

        break;
    }

    return -1;
}

// The item after index with a count (or worn by the member), below count; -1 if none.
int NextItem(int index, int count, byte* counts, int member)
{
    while (1)
    {
        if (++index < count)
        {
            if (counts[index] == 0)
            {
                if (member == 0xff)
                    continue;
                if (IsEquipped(member, index) == 0)
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
void PrintItemRow(int index, byte* counts, char** names, uint marker)
{
    char owned;
    int column_n;

    owned = (char)counts[index];
    if (owned != -1)
    {
        if (owned != 0)
        {
            PrintNumber((byte)owned, 2, 0x20);
        }
        else
        {
            PrintString(/*0x9778*/ "--");
        }

        if (marker < 0x20)
        {
            SelectCharset(1);
        }

        PrintChar(marker);
        SelectCharset(0);
    }

    if (names[index][0] == '*')
    {
        SelectCharset(1);
        PrintString(/*0x977c*/ "\x1c + ");
        PrintString(&names[index][1]);
        SelectCharset(0);
    }
    else if (names[index][0] == '!')
    {
        SelectCharset(1);
        PrintString(/*0x9782*/ "\x1d + ");
        SelectCharset(0);
        PrintString(equipShortNames[40 + index]);
    }
    else if (names[index][0] == '(')
    {
        PrintString(/*0x9788*/ "Moonstone ");
        SelectCharset(1);
        PrintChar((byte)names[index][1]);
        SelectCharset(0);
    }
    else
    {
        PrintString(names[index]);
    }

    column_n = GetCurrentTextX();
    if (column_n < 0xe)
    {
        register int si = 0xe - column_n;
        column_n += si;

        do
        {
            PrintChar(' ');
            si--;
        } while (si != 0);
    }
    PrintChar('\n');
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
static int BrowseList(char* title, int count, byte* counts, char** names)
{
    int arrows;
    int key;
    int last;
    int step_d;
    bool leave_c;
    int first_h;
    int steps;

    ShowStatsTitle(title);
    DrawListFrame(8);
    leave_c = false;
    first_h = NextItem(-1, count, counts, 0xff);
    if (first_h == -1)
    {
        DrawListFrame(8);
        SetTextPosition(1, 4);
        PrintString(/*0x9794*/ "(None owned!)");
        HideScrollArrow();
        SelectTextWindow(2);
        return GetChar();
    }

    // 092d: enter the loop at its test, which sits after the END case
    goto L_test;
    {
    L_list:
        // -> 0756
        for (last = first_h; last != -1;
             // 0746
             last = NextItem(last, count, counts, 0xff))
        {
            PrintItemRow(last, counts, names, 0x2d);

            SetTextPosition(1, GetCurrentTextY());
            if (GetCurrentTextY() == 8)
                break;
        }

        // 077f
        arrows = 0;
        if (PrevItem(first_h, count, counts, 0xff) != -1)
        {
            arrows = 2;
        }

        // 079c
        if (last != -1 && NextItem(last, count, counts, 0xff) != -1)
        {
            arrows++;
        }

        // 07b8
        switch (arrows)
        {
        case 0:
            // 07d0
            HideScrollArrow();
            break;
        case 1:
            // 0806
            ShowScrollArrow(0x19);
            break;
        case 2:
            // 0810
            ShowScrollArrow(0x18);
            break;
        case 3:
            // 0816
            ShowScrollArrow(0x12);
            break;
        }

        // ^ ok

        // 07d3
        SelectTextWindow(2);
        key = GetChar();
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
                if (PrevItem(first_h, count, counts, 0xff) != -1)
                {
                    first_h = PrevItem(first_h, count, counts, 0xff);
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
                if (last != -1 && NextItem(last, count, counts, 0xff) != -1)
                {
                    first_h = NextItem(first_h, count, counts, 0xff);
                    last = NextItem(last, count, counts, 0xff);
                }
            }
            goto L_test;

        case U5_KEY_HOME:
            // 08d6
            first_h = NextItem(-1, count, counts, 0xff);
            goto L_test;

        case U5_KEY_END:
            // 08ec
            first_h = PrevItem(count, count, counts, 0xff);
            // 090f
            step_d = 0;
            goto L_end_test;
            do
            {
                first_h = last;
                step_d++;
            L_end_test:;
            } while (step_d < 6 && (last = PrevItem(first_h, count, counts, 0xff)) != 0xffff);
        L_test:
            if (leave_c)
                goto L_exit;
            SelectTextWindow(1);
            SetTextPosition(1, 1);
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
// Copy the saved game's scrolls, potions and quest items into the itemCounts table of owned item counts (the Use list).
// 099A
void FillItemCounts(void)
{
    int i;

    i = 0;
    do
    {
        itemCounts[i] = scrolls[i];
        potionCounts[i] = potions[i];
        moonstoneFlags[i] = moonstoneMap[i] == 0xff ? 0xff : 0;
    } while (++i < 8);

    carpetCount = carpets;
    skullKeyCount = skullKeys;
    amuletOwned = amulet;
    crownOwned = crown;
    sceptreOwned = sceptre;

    for (i = 0; i < 3; i++)
    {
        shardFlags[i] = shards[i];
    }

    spyglassCount = spyglasses;
    capePlansOwned = hmsCapePlans != 0 ? 0xff : 0;
    sextantCount = sextants;
    pocketWatchOwned = pocketWatch;
    blackBadgeOwned = blackBadge;
    sandalwoodBoxOwned = sandalwoodBox;
}

// Z command: flip through the party's stats pages (member stats and arms, supplies, reagents, spells, items, armaments).
// CHECKED
void ZstatsCmd(void)
{
    int member;
    byte key;
    int page_c;

    member = ChoosePlayer(1);
    if (member == -2)
    {
        member = 6;
    }
    else if (member < 0)
    {
        return;
    }

    // ec3f
    PrintString(/*0x97a2*/ "\nStatus: ");
    FillItemCounts();
    OpenStatsDivider();
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
                page_c = partyCount * 2 - 1;
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
            if (partyCount * 2 - 1 == page_c)
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
            if (key - 0x31 < partyCount)
            {
                page_c = key * 2 - 0x62;
            }
            break;
        }

        // ecab
        SelectTextWindow(1);
        if (page_c < 0xc)
        {
            if ((page_c & 1) != 0)
            {
                ShowPlayerArms(page_c >> 1);
            }
            else
            {
                ShowPlayerStats(page_c >> 1);
            }

            // ed3f
            SelectTextWindow(2);
            key = GetChar();
        }
        else if (page_c == 0xc)
        {
            ShowSupplies();

            // ed3f
            SelectTextWindow(2);
            key = GetChar();
        }
        else
        {
            // 0b70
            switch (page_c)
            {
            case 0xd:
                // ed6a
                key = BrowseList(/*0x97ac*/ "Reagents", 8, D_5850_reagents, D_19d2_reagentNames);
                break;
            case 0xe:
                key = BrowseList(/*0x97b6*/ "Spells", 0x30, mixedSpells, spellNames);
                break;
            case 0xf:
                key = BrowseList(/*0x97be*/ "Items", 0x26, itemCounts, itemNames);
                break;
            case 0x10:
                key = BrowseList(/*0x97c4*/ "Armaments", 0x30, equipment, equipShortNames);
                break;
            }
        }
    }

    // 0bd6
    HideScrollArrow();
    ClearStatsTitle();
    DrawStatsDivider();
    UpdateVitalsDisplay();
    PrintString(/*0x97ce*/ "Done\n");
}

// Print a refusal message and the "Item:" prompt again.
static void PrintEquipRefusal(char* message)
{
    PrintString(/*0x97d4*/ "\n\n");
    PrintString(message);
    PrintString(/*0x97d8*/ "\n\nItem: ");
}

// Which hand is free: 2 both, 0 the first (equips[2]), 1 the second (equips[3], when the first holds a one-handed item), 0xff none.
static int FreeHandSlot(int member)
{
    S_55a8* player = &party[member];

    if ((player->equips[2] == 0xff) && (player->equips[3] == 0xff))
    {
        return 2;
    }
    else if (player->equips[2] == 0xff)
    {
        return 0;
    }
    else if ((player->equips[3] == 0xff) && (equipSlotKind[player->equips[2]] != 0x30))
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
static int EquipOrRemove(int member, int item)
{
    byte* slot_i;
    int strongEnough_g;
    int hand;
    int i;
    int classIdx_e;
    ActorFmt* actor_g;
    uint load;

    if (mapId > 0x7f)
    {
        actor_g = &actors[combatants[curCombatant].actorIdx];
    }

    if (item == 0x1b || item == 0x1d)
    {
        return 0;
    }
    if (item >= 9 && item <= 0xf && mapId > 0x7f && combatOver == 0)
    {
        PrintEquipRefusal(/*0x97e2*/ "Thou canst not change armour in heated battle!");
    }
    else if (IsEquipped(member, item) != 0)
    {
        Unequip(member, item);
        if (equipment[item] < 99)
        {
            equipment[item]++;
        }

        if (mapId > 0x7f && item == 0x2a)
        {
            classIdx_e = FindIndex(/*0x9812*/ "AMBFDTPRS", party[member].cls);
            actor_g->tile = actor_g->animTile = classTile[classIdx_e];
        }
    }
    else if (((item == 0x1a || item == 0x24) && D_57db_arrows == 0) || (item == 0x1c && quarrels == 0))
    {
        PrintEquipRefusal(/*0x981c*/ "Thou hast no ammunition for that weapon!");
    }
    else
    {
        load = 0;
        slot_i = party[member].equips;
        for (i = 0; i < 6; i++)
        {
            if (*slot_i != 0xff)
            {
                load += equipWeight[*slot_i];
            }
            slot_i++;
        }

        strongEnough_g = equipWeight[item] + load <= party[member].str;

        switch (equipSlotKind[item])
        {
        case 0x80:
            if (party[member].equips[0] != 0xff)
            {
                PrintEquipRefusal(/*0x9846*/ "Remove first thy present helm!");
                return 0;
            }
            slot_i = &party[member].equips[0];
            break;
        case 0x40:
            if (party[member].equips[1] != 0xff)
            {
                PrintEquipRefusal(/*0x9866*/ "Thou must first remove thine other armour!");
                return 0;
            }
            slot_i = &party[member].equips[1];
            break;
        case 0x20:
            hand = FreeHandSlot(member);
            if (hand == 0xff)
            {
                PrintEquipRefusal(/*0x9892*/ "Thou must free one of thy hands first!");
                return 0;
            }
            if (hand == 2)
            {
                hand = 0;
            }
            slot_i = &party[member].equips[hand + 2];
            break;
        case 0x30:
            hand = FreeHandSlot(member);
            if (hand != 2)
            {
                PrintEquipRefusal(/*0x98ba*/ "Both hands must be free before thou canst wield that!");
                return 0;
            }
            slot_i = &party[member].equips[2];
            break;
        case 4:
            if (party[member].equips[5] != 0xff)
            {
                PrintEquipRefusal(/*0x98f0*/ "Thou must remove thine other amulet!");
                return 0;
            }
            slot_i = &party[member].equips[5];
            break;
        case 2:
            if (party[member].equips[4] != 0xff)
            {
                PrintEquipRefusal(/*0x9916*/ "Only one magic ring may be worn at a time!");
                return 0;
            }
            slot_i = &party[member].equips[4];
            break;
        }

        if (strongEnough_g != 0)
        {
            goto L_wear;
        }
        PrintEquipRefusal(/*0x9942*/ "Thou art not strong enough!");
        return 0;

L_wear:
        *slot_i = item;
        equipment[item]--;

        if ((item == 0x2a || item == 0x2c) && RandomRange(0, 0xf) == 0)
        {
            PrintString(/*0x995e*/ "\n\nRing vanishes!\n");
            party[member].equips[4] = 0xff;
            AudioSweepTone(0x4b0, 0x7d0, 1, 0x28);
            return 1;
        }

        if (item == 0x2a && mapId > 0x7f)
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
int SelectItem(int first, int member, int mode)
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
        counts_d = equipment;
        count_g = 0x30;
    }
    else
    {
        counts_d = itemCounts;
        count_g = 0x26;
    }

    // f142
    while (leave_c == 0)
    {
        SelectTextWindow(1);
        SetTextPosition(1, 1);

        rows_e = 0;
        // f159 -> f20b
        for (item_a = first; item_a != -1; item_a = NextItem(item_a, count_g, counts_d, member))
        {
            // f213
            rows_e++;

            if (GetCurrentTextY() == cursorRow_i)
            {
                PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
                selected_f = item_a;
            }

            // f22b
            if (mode == 0x52)
            {
                // f234
                if (IsEquipped(member, item_a) != 0)
                {
                    // f244
                    marker_n = equipMarker[item_a];
                }
                else
                {
                    // f15c
                    marker_n = 0x20;
                }

                // f161..f17e
                // Equipments
                PrintItemRow(item_a, counts_d, equipShortNames, marker_n);
            }
            else
            {
                // f170..f17e
                // Scrolls
                PrintItemRow(item_a, counts_d, itemNames, 0x20);
            }

            // f181
            SetTextPosition(1, GetCurrentTextY());

            if (GetCurrentTextY() - 1 == cursorRow_i)
            {
                PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
            }

            // f19c
            if (GetCurrentTextY() == 0x8)
            {
                break;
            }
        }

        // f1a4
        arrows_b = 0;
        if (PrevItem(first, count_g, counts_d, member) != -1)
        {
            arrows_b = 2;
        }

        // f1c0
        if (item_a != -1)
        {
            if (NextItem(item_a, count_g, counts_d, member) != -1)
            {
                arrows_b++;
            }
        }

        // f1db
        switch (arrows_b)
        {
        case 0:
            // f254
            HideScrollArrow();
            break;
        case 1:
            // f28a..f28d
            ShowScrollArrow(0x19);
            break;
        case 2:
            // f294..f28d
            ShowScrollArrow(0x18);
            break;
        case 3:
            // f29a..f28d
            ShowScrollArrow(0x12);
            break;
        }

        // L_1077
        // f257
        SelectTextWindow(2);
        key_g = GetChar();

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
                    if (PrevItem(first, count_g, counts_d, member) != -1)
                    {
                        first = PrevItem(first, count_g, counts_d, member);
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
                        if (item_a != -1 && NextItem(item_a, count_g, counts_d, member) != -1)
                        {
                            first = NextItem(first, count_g, counts_d, member);
                            item_a = NextItem(item_a, count_g, counts_d, member);
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
            first = NextItem(-1, count_g, counts_d, member);
            cursorRow_i = 1;
            // -> f462
            break;
        case U5_KEY_END:
            // f3bc
            // L_11dc
            first = PrevItem(count_g, count_g, counts_d, member);

            for (cursorRow_i = 1; cursorRow_i < 7; cursorRow_i++)
            {
                // f3d1
                // L_11f1
                selected_f = PrevItem(first, count_g, counts_d, member);
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
                leave_c = EquipOrRemove(member, selected_f);
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
            PrintString(mode == 0x52 ? /*0x9970*/ "Done\n" : /*0x9976*/ "None!\n");
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
void ReadyCmd(void)
{
    int member;
    int first;

    member = ChoosePlayer(0);

    if (member < 0)
        return;

    first = NextItem(-1, 0x30, equipment, member);
    if (first == -1)
    {
        PrintString(/*0x997e*/ "Thou art empty-\nhanded!\n");
        return;
    }

    PrintString(/*0x9998*/ "Item: ");
    OpenStatsDivider();
    SelectTextWindow(1);

    ShowStatsTitle(party[member].name);

    DrawListFrame(0x08);

    SelectItem(first, member, 0x52);

    HideScrollArrow();
    ClearStatsTitle();
    DrawStatsDivider();
    UpdateVitalsDisplay();
}
