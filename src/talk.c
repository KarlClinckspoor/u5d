#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "npc.h"
#include "shoppes.h"
#include "shoppes2.h"
#include "shoppes3.h"
#include "talk.h"
#include "town.h"

#define TALK_KEY_NAME 0
#define TALK_KEY_DESC 1
#define TALK_KEY_2 2
#define TALK_KEY_3 3
#define TALK_KEY_4 4

static int TALK_0f32_ProcessChar(byte code);
static void TALK_127e_StartTalk(int a);

// CHECKED
// 1 when the two strings are the same, ignoring case and the high bit.
static int TALK_0000_CompareStringNoCase(char* first, char* second)
{
    while (1)
    {
        if (ULTIMA_2032_ToUpper((*(byte*)first++) & 0x7f) != ULTIMA_2032_ToUpper((*(byte*)second++) & 0x7f))
        {
            return 0;
        }

        if ((*first == 0) && (*second == 0))
        {
            return 1;
        }
    }
}

// 1 when the tile at (x, y) is a table, a desk, a counter or a door: the Talk command then
// reaches one tile further (TALK_041c_TalkCmd).
static int TALK_0054_CanTalkAcrossTile(int x, int y)
{
    switch (*ULTIMA_4402_GetTileAddr(x, y))
    {
    case TILE_MAP_29:
    case TILE_MAP_TABLE_94:
    case TILE_MAP_TABLE_95:
    case TILE_MAP_TABLE_96:
    case TILE_MAP_97:
    case TILE_MAP_98:
    case TILE_MAP_99:
    case TILE_MAP_TABLE_9A:
    case TILE_MAP_TABLE_9B:
    case TILE_MAP_TABLE_9C:
    case TILE_MAP_DESK:
    case TILE_MAP_AE:
    case TILE_MAP_DOOR_BA:
    case TILE_MAP_DOOR_BB:
    case TILE_MAP_TABLE_BE:
    case TILE_MAP_CA:
    case TILE_MAP_CB:
        return 1;
    }

    return 0;
}

// ask pay
// Returns 0 for Yes, 1 for No.
static int TALK_00ac_AskPay(void)
{
    char ch;

    ULTIMA_1850_PrintString(_TEXT(0x9052, "\n\nDost thou pay?\n\n:"));

    do
    {
        ch = ULTIMA_2032_ToUpper(ULTIMA_266c_GetChar());
        if (ch == 'Y')
        {
            ULTIMA_1850_PrintString(_TEXT(0x9066, "Yes\n"));
            return 0;
        }
    } while (ch != 'N');

    ULTIMA_1850_PrintString(_TEXT(0x906c, "No!\n"));
    return 1;
}

// CHECKED
// talk to merchant (shoppe)
// merchantCode is the NPC's dialog number, 0x81-0x88: the shop kind + 0x81. Finds this
// town in the kind's row of D_23ca_shopMapIds (the shop index) and runs the shop.
// Which kind is which: 2 the stables, 6 the healer, 7 the inn (their texts); the others
// are not named yet.
static void TALK_00e6_TalkToMerchant(int merchantCode)
{
    int playerIndex;

    if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE && merchantCode != 0x83)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9072, "A merchant says:\n\"GET THAT HORSE OUT OF HERE!\"\n"));
    }
    else
    {
        if (D_587b_activePlayer != 0xff)
        {
            playerIndex = D_587b_activePlayer;
        }
        else
        {
            ULTIMA_39fc_GetFirstActivePartyMember();
            playerIndex = D_5876_tempX;
        }

        D_b116_shopKind = merchantCode - 0x81;

        for (D_b114_shopIndex = 0; (&D_23ca_shopMapIds[0][0])[D_b114_shopIndex + D_b116_shopKind * 16] != D_5893_mapId && D_b114_shopIndex < 0x10; D_b114_shopIndex++)
        {
        }

        D_aafc_shopName = D_21ca_shopNames[D_b116_shopKind][D_b114_shopIndex];
        D_aafe_shopOwner = D_22ca_shopOwners[D_b116_shopKind][D_b114_shopIndex];

        switch (D_b116_shopKind)
        {
        case 0:
            SHOPPES_12b2_Armourer(playerIndex);
            break;
        case 1:
            SHOPPES2_066c_Pub(playerIndex);
            break;
        case 2:
            SHOPPES_07be_Stables(playerIndex);
            break;
        case 3:
            SHOPPES2_0abc_Shipwright(playerIndex);
            break;
        case 4:
            SHOPPES_075e_Reagents(playerIndex);
            break;
        case 5:
            SHOPPES_04a2_Provisioner(playerIndex);
            break;
        case 6:
            SHOPPES_14f8_Healer(playerIndex);
            break;
        case 7:
            SHOPPES3_08b4_Inn(playerIndex);
        }
    }
}

// the password TALK_0000_CompareStringNoCase checks: an array, so in _DATA (DS:4a9a)
char D_4a9a_password[] = "IMPE";

// CHECKED
// Dialog number 0xff: someone who lets the party pass for a price. On map 5 half the
// gold "to charity"; on map 0x12 the password (only when D_587a_statusIcon is 0x1d; the text says
// "bearer of the Badge", so probably the Black Badge is worn); elsewhere a guard's
// tribute of 10 gp for each living member. Returns 0 when paid or passed, 1 when not.
static int TALK_01e2_GuardDemandsTribute(void)
{
    int tribute_i;
    uint member;
    char answer[0xf];

    if (D_5893_mapId != 0x12)
    {
        // 01f3
        if (D_5893_mapId == 5)
        {
            ULTIMA_16ba_PrintChar('\"');
            ULTIMA_1850_PrintString(_TEXT(0x90a2, "Thou wilt give\nhalf thy gold to\ncharity!"));
            ULTIMA_16ba_PrintChar('\"');

            if (TALK_00ac_AskPay() != 0)
            {
                // 0216
                return 1;
            }

            // 021c
            D_57aa_gold /= 2;

            // 0228
            ULTIMA_2900_UpdateVitalsDisplay();
            // 022b
            return 0;
        }
        else
        {
            // 0230
            tribute_i = 0;

            // 0243 With an unsigned index MSC keeps the count in one more temporary (bp-1Ch),
            // as the original; the (int) cast makes it an end-pointer loop (cmp si,dx) and !=
            // gives the je for an empty party.
            for (member = 0; member != (int)D_585b_partyCount; member++)
            {
                // 025a
                if (D_55a8_party[member].status != STATUS_DEAD)
                {
                    tribute_i += 10;
                }
            }

            // 026c
            ULTIMA_1850_PrintString(_TEXT(0x90cc, "A guard demands\na "));
            ULTIMA_1a3e_PrintNumber(tribute_i, 2, 0x20);
            ULTIMA_1850_PrintString(_TEXT(0x90e0, " gp tribute\nto Blackthorn!"));

            if (TALK_00ac_AskPay() != 0)
            {
                // 028d -> 0216
                return 1;
            }

            // 028f
            if (D_57aa_gold >= (uint)tribute_i) // jbe: an unsigned compare
            {
                // 029a
                D_57aa_gold -= tribute_i;

                ULTIMA_2900_UpdateVitalsDisplay();
                return 0;
            }

            // 0297 -> 0216
            return 1;
        }
    }
    else
    {
        // 02a4
        if (D_587a_statusIcon == 0x1d)
        {
            // 02ae
            ULTIMA_16ba_PrintChar('\"');
            ULTIMA_1850_PrintString(_TEXT(0x90fc, "Give now the\npassword, bearer\nof the Badge!"));
            ULTIMA_16ba_PrintChar('\"');
            ULTIMA_1850_PrintString(_TEXT(0x9128, "\n\nYour response?\n"));
            ULTIMA_3b1c_GetString(answer, 0xe);
            ULTIMA_16ba_PrintChar('\n');
            answer[4] = 0;

            if (TALK_0000_CompareStringNoCase(D_4a9a_password, answer) != 0)
            {
                // 02f2
                ULTIMA_16ba_PrintChar('\n');
                ULTIMA_16ba_PrintChar('\"');
                ULTIMA_1850_PrintString(_TEXT(0x913a, "Pass, friend!"));
                ULTIMA_16ba_PrintChar('\"');
                ULTIMA_16ba_PrintChar('\n');
                // 0315 -> 022b
                return 0;
            }
        }

        // 02ef -> 0216
        return 1;
    }
}

// CHECKED
// talk to npc
// Talk to the NPC in slot npcIndex. Its dialog number (NpcFmt.dialogNumber): 0 no response,
// 1-0x7f an entry of the map's .TLK file, 0x81-0x88 a merchant, 0xfd, 0xfe and 0xff
// special cases (below).
int TALK_031e_TalkToNpc(int npcIndex)
{
    int dialogNumber_f;
    NpcFmt* npc;
    NpcScheduleFmt* schedule;

    ULTIMA_16ba_PrintChar('\n');

    D_bcdc_talkNpcIndex = npcIndex;
    npc = &D_5f5e_npcs[npcIndex];
    schedule = &D_5d5e_npcSchedules[npcIndex];

    if (schedule->type[npc->scheduleIdx] == 4)
    {
        schedule->type[npc->scheduleIdx] = 1;
    }
    // c2ea: the guard test jumps past its message with a goto. As an else-if, MSC keeps the
    // guard's copy of the shared print tail and reloads npc for the guard test. With the
    // goto, the guard falls into the "No response!" copy (the original's three nops after
    // jmp near are that then-branch's dead jump over its else) and BX still holds npc.
    else
    {
        if (D_5c5a_actors[npc->actorIdx].tile != TILE_ACTOR_GUARD || ((npc->scheduleIdx & 1) != 0 && (npc->dialogNumber) != 0))
            goto L_join;
        ULTIMA_1850_PrintString(_TEXT(0x9148, "The guard offers\nno response!\n"));
        return 0;
    }
L_join:
    dialogNumber_f = npc->dialogNumber;
    if (dialogNumber_f == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9168, "No response!\n"));
        return 0;
    }
    else
    {
        // c316
        if (dialogNumber_f < 0x80)
        {
            TALK_127e_StartTalk(dialogNumber_f);
            return 0;
        }

        if (dialogNumber_f == 0xfd)
        {
            ULTIMA_16ba_PrintChar('\"');
            ULTIMA_1850_PrintString(_TEXT(0x9176, "Don't hurt me!\nPlease go away!"));
            ULTIMA_16ba_PrintChar('\"');
            ULTIMA_16ba_PrintChar('\n');
            return 0;
        }

        // c34c
        if (dialogNumber_f == 0xfe)
        {
            TOWN_10da_ShoutBegone(); // BUG: TOWN_10da_ShoutBegone(unaff_SI)
            return 0;
        }

        if (dialogNumber_f == 0xff)
        {
            return TALK_01e2_GuardDemandsTribute();
        }

        if ((npc->scheduleIdx & 1) != 0 && (NPC_12e0_ScheduleSlot(D_bcdc_talkNpcIndex, D_587f_hour) & 1) != 0)
        {
            TALK_00e6_TalkToMerchant(dialogNumber_f);
            return 0;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x9196, "A merchant says:\n\"Come see me at\nmy shoppe, "));
            ULTIMA_1850_PrintString(_TEXT(0x91c4, "when\nit's open!\"\n"));
            return 0;
        }
    }
}

int TALK_041c_TalkCmd(void)
{
    int dirY;
    int y_o;
    int dirX;
    int x_i;
    int npcIndex_f;

    if (ULTIMA_35ec_SelectDirection() == 0)
    {
        return 0;
    }

    dirX = D_5876_tempX;
    dirY = D_5878_tempY;
    x_i = dirX + D_5896_mapX;
    y_o = dirY + D_5897_mapY;

    if (ULTIMA_368e_FindActorTileAtPos(x_i, y_o, D_5895_mapLevel) == 0 && TALK_0054_CanTalkAcrossTile(x_i, y_o) != 0)
    {
        x_i += dirX;
        y_o += dirY;
    }

    if (ULTIMA_368e_FindActorTileAtPos(x_i, y_o, D_5895_mapLevel) == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x91d6, "\nNobody's here!\n"));
    }
    else
    {
        npcIndex_f = TOWN_011e_FindNpcByActor(D_5876_tempX);
        switch (*ULTIMA_4402_GetTileAddr(x_i, y_o))
        {
        case TILE_MAP_BED:
            ULTIMA_1850_PrintString(_TEXT(0x91e8, "\n\"Zzzzzz...\"\n"));
            break;
        case TILE_MAP_MIRROR:
            ULTIMA_1850_PrintString(_TEXT(0x91f6, "\nNo response!\n"));
            break;
        default:
            return TALK_031e_TalkToNpc(npcIndex_f);
        }
    }

    return 0;
}

static void TALK_04d2_PrintCr(void) { TALK_0f32_ProcessChar(0x80 | 0xd); }

static void TALK_04da_PrintQuoteMark(void) { TALK_0f32_ProcessChar(0x80 | 0x22); }

// CHECKED
// Print the word in D_bce4_talkWord. A character with the high bit set is printed in
// charset 0, one without it in charset 1 (the runes: script code 0x8e flips
// D_4af0_talkCharMask). A line is not started with a space.
static void TALK_04e2_PrintWord(void)
{
    byte ch;
    int pos;

    pos = 0;

    // c469
    if (D_4af1_talkWordLength != 0)
    {
        while (pos < D_4af1_talkWordLength) // c4c9
        {
            // c4d5 -> c47c
            if (D_bce4_talkWord[pos] & 0x80)
                ULTIMA_1c9e_SelectCharset(0);
            else
                ULTIMA_1c9e_SelectCharset(1);
            ch = D_bce4_talkWord[pos] & 0x7f;
            pos++;

            if (ULTIMA_1f12_GetCurrentTextX() != 0 || ch != 0x20)
            {
                // c49a
                if (D_4af3_talkTextX < 0xf || ch != 10)
                {
                    // c4a7
                    if (ch != 10)
                    {
                        D_4af3_talkTextX = ULTIMA_1f12_GetCurrentTextX();
                    }

                    // c4b3
                    ULTIMA_16ba_PrintChar(ch);
                }
                else if (ch == 10) // c4be
                {
                    D_4af3_talkTextX = 0;
                }
            }
        }

        // c4e0
        D_4af1_talkWordLength = 0;
        ULTIMA_1c9e_SelectCharset(0);
    }
}

// Add ch to the word; at a space or a newline (or when the buffer is full) print it,
// on a new line when it would pass column 18.
static void TALK_0574_BuildOrPrintWord(byte ch)
{
    if (D_4af1_talkWordLength != 0x10)
    {
        D_bce4_talkWord[D_4af1_talkWordLength++] = ch;

        if (ch != 0x8a && ch != 0xa0) // 0xa / 0x20
        {
            return;
        }

        if (ULTIMA_1f12_GetCurrentTextX() + (uint)D_4af1_talkWordLength >= 0x12)
        {
            ULTIMA_16ba_PrintChar('\n');
        }
    }

    TALK_04e2_PrintWord();
}

static int TALK_0b04_AskInterest(void);
char* D_4aa0_talkFileNames[4] =
{
    /*0x9206*/ "TOWNE.TLK",
    /*0x9210*/ "DWELLING.TLK",
    /*0x921e*/ "CASTLE.TLK",
    /*0x922a*/ "KEEP.TLK"
};

char* D_4aa8_standardWords[0x22 + 1] =
{
    _STATIC_TEXT(0x9234, "NAME"),
    _STATIC_TEXT(0x923a, "JOB"),
    _STATIC_TEXT(0x923e, "WORK"),
    _STATIC_TEXT(0x9244, "BYE"),
    _STATIC_TEXT(0x9248, "THANK"),
    _STATIC_TEXT(0x924e, "FUCK"),
    _STATIC_TEXT(0x9254, "SHIT"),
    _STATIC_TEXT(0x925a, "DAMN"),
    _STATIC_TEXT(0x9260, "DICK"),
    _STATIC_TEXT(0x9266, "PRICK"),
    _STATIC_TEXT(0x926c, "PUSSY"),
    _STATIC_TEXT(0x9272, "CUNT"),
    _STATIC_TEXT(0x9278, "ASS"),
    _STATIC_TEXT(0x927c, "BUTT"),
    _STATIC_TEXT(0x9282, "BOOGER"),
    _STATIC_TEXT(0x928a, "PISS"),
    _STATIC_TEXT(0x9290, "JACK OFF"),
    _STATIC_TEXT(0x929a, "MASTURBATE"),
    _STATIC_TEXT(0x92a6, "SUCK"),
    _STATIC_TEXT(0x92ac, "FART"),
    _STATIC_TEXT(0x92b2, "TITS"),
    _STATIC_TEXT(0x92b8, "BOOB"),
    _STATIC_TEXT(0x92be, "MELONS"),
    _STATIC_TEXT(0x92c6, "BLOW"),
    _STATIC_TEXT(0x92cc, "PENIS"),
    _STATIC_TEXT(0x92d2, "BREAST"),
    _STATIC_TEXT(0x92da, "CLIT"),
    _STATIC_TEXT(0x92e0, "BALLS"),
    _STATIC_TEXT(0x92e6, "SCROTUM"),
    _STATIC_TEXT(0x92ee, "NUTS"),
    _STATIC_TEXT(0x92f4, "BULLSHIT"),
    _STATIC_TEXT(0x92fe, "CUM"),
    _STATIC_TEXT(0x9302, "CROTCH"),
    _STATIC_TEXT(0x930a, "MOTHERFUCKER"),
    _STATIC_TEXT(0x9318, "ELECTRONIC ARTS") // hidden string
};

u8 D_4aee_talkPendingCode = 0;

u8 D_4aef_talkCodeArgCount = 0;

u8 D_4af0_talkCharMask = 0xff;

u8 D_4af1_talkWordLength = 0;

u8 D_4af2_talkInLabel = 0;

u8 D_4af3_talkTextX = 0;

u8 D_4af4_talkLastChar = 0;

u8 D_4af5_talkAfterWord = 0;

// Script code 0x85 (gold): the three digits after it are the price. Pays it (to a beggar:
// karma, once in 100 turns), or says the party cannot and goes back to "Your interest?".
static int TALK_05b6_PayGold(void)
{
    int amount;

    amount = (D_bce0_talkCodeArgs[0] & 0x7f) * 100 + (D_bce0_talkCodeArgs[1] & 0x7f) * 10 + (D_bce0_talkCodeArgs[2] & 0x7f) - 0x14d0; // 0x14d0 == 5328
    if (D_57aa_gold >= amount)
    {
        D_57aa_gold -= amount;
        ULTIMA_2900_UpdateVitalsDisplay();
        if ((D_5c5a_actors[D_5f5e_npcs[D_bcdc_talkNpcIndex].actorIdx].tile & 0xfc) == TILE_ACTOR_BEGGAR && D_588b_turnCount >= 100)
        {
            D_588b_turnCount = 0;
            ULTIMA_3ef0_IncreaseByte(&D_5888_karma, 1, 99);
            if (D_57aa_gold == 0)
            {
                ULTIMA_3ef0_IncreaseByte(&D_5888_karma, 2, 99);
            }
        }

        return 0;
    }
    else
    {
        D_4af1_talkWordLength = 0;
        ULTIMA_16ba_PrintChar('\"');
        ULTIMA_1850_PrintString(_TEXT(0x9328, "Thou hast not enough gold!"));
        ULTIMA_16ba_PrintChar('\"');
        ULTIMA_1850_PrintString(_TEXT(0x9344, "\n\n"));

        D_4aef_talkCodeArgCount = D_4aee_talkPendingCode = 0;

        return TALK_0b04_AskInterest();
    }
}

// Script code 0x86 (the NPC gives something): below 0x40 an equipment number, 0x41 food,
// 0x42 gold, 0x43 keys, 0x44 gems, 0x45 torches, 0x46 grapple, 0x47 magic carpets,
// 0x48 sextant, 0x49 spyglass, 0x4a black badge, 0x4b skull keys (vars.h's savegame names).
static void TALK_0682_ReceiveItem(byte itemCode)
{
    if (itemCode < 0x40)
    {
        ULTIMA_3ef0_IncreaseByte(&D_57c0_equipment[itemCode], 1, 99);
        return;
    }

    switch (itemCode)
    {
    case 0x41:
        ULTIMA_3f14_IncreaseInt(&D_57a8_food, 1, 9999);
        ULTIMA_2900_UpdateVitalsDisplay();
        break;
    case 0x42:
        ULTIMA_3f14_IncreaseInt(&D_57aa_gold, 1, 9999);
        ULTIMA_2900_UpdateVitalsDisplay();
        break;
    case 0x43:
        ULTIMA_3ef0_IncreaseByte(&D_57ac_keys, 1, 99);
        break;
    case 0x44:
        ULTIMA_3ef0_IncreaseByte(&D_57ad_gems, 1, 99);
        break;
    case 0x45:
        ULTIMA_3ef0_IncreaseByte(&D_57ae_torches, 1, 99);
        break;
    case 0x46:
        ULTIMA_3ef0_IncreaseByte(&D_57af_grapple, 1, 99);
        break;
    case 0x47:
        ULTIMA_3ef0_IncreaseByte(&D_57b0_carpets, 1, 99);
        break;
    case 0x48:
        D_57bc_sextants = 0xff;
        break;
    case 0x49:
        D_57ba_spyglasses = 0xff;
        break;
    case 0x4a:
        D_57be_blackBadge = 0xff;
        break;
    case 0x4b:
        ULTIMA_3ef0_IncreaseByte(&D_57b1_skullKeys, 1, 99);
        break;
    }
}

// skip one talk string
// Move D_bcde_talkPtr past the next terminator byte (returns 1) or endMarker byte (0).
static int TALK_0728_SkipOneTalkString(byte terminator, byte endMarker)
{
    byte value;

    do
    {
        value = *D_bcde_talkPtr++;
        if (value == terminator)
        {
            return 1;
        }

        if (value == endMarker)
        {
            return 0;
        }
    } while (1);
}

// CHECKED
// select nth talk string
// Point D_bcde_talkPtr at string n of the NPC's script in D_b21e_scratch. The strings end in 0:
// 0 name, 1 description, 2 greeting, 3 job, 4 bye, then keyword and answer pairs (Ultima Codex, "Ultima V internal formats").
static void TALK_075a_SelectNthTalkString(int n)
{
    D_bcde_talkPtr = D_b21e_scratch;

    if (n != 0)
    {
        for (; n != 0; n--)
        {
            TALK_0728_SkipOneTalkString(0, 0x90);
        }
    }
}

static int TALK_0f32_ProcessChar(byte code);

// process talk string
// Process the string at D_bcde_talkPtr. 1 when the conversation is over.
static int TALK_0788_ProcessTalkString(void)
{
    while (*D_bcde_talkPtr != 0)
    {
        if (TALK_0f32_ProcessChar(*D_bcde_talkPtr++) != 0)
        {
            return 1;
        }
    }

    return 0;
}

static int TALK_07aa_ProcessNthTalkString(byte n)
{
    TALK_075a_SelectNthTalkString(n);
    return TALK_0788_ProcessTalkString();
}

// Script code 0x87 (or): the keyword before it shares the answer of the keyword after it.
// Skips that keyword and processes what follows (probably: the layout is from the Codex page).
static int TALK_07be_ProcessOrAnswer(void)
{
    D_bcde_talkPtr++;

    while (*D_bcde_talkPtr == 0)
    {
        D_bcde_talkPtr++;
    }

    while (*D_bcde_talkPtr++ != 0)
    {
        // empty
    }

    return TALK_0788_ProcessTalkString();
}

static void TALK_07e4_PrintAvatarName(void)
{
    byte* name;
    name = D_55a8_party[0].name;

    while (*name != 0)
    {
        TALK_0f32_ProcessChar(*name++ | 0x80);
    }
}

// CHECKED
// Script code 0x84 (join). The NPC's roster entry is found by the first three letters of
// its name, from the end of the roster down, and swapped into the first free party place.
static int TALK_080a_Join(void)
{
    int i; // 30..2f
    byte* savedPtr_o;   // 2e..2d
    char rosterName_h[4]; // 2c..29
    S_55a8 member;  // 28..09 (size: 0x20)
    char npcName[4];  // 08..05
    int rosterIndex_h;      // 04..03

    savedPtr_o = D_bcde_talkPtr;
    rosterIndex_h = 0xf;

    if (D_585b_partyCount == 6)
    {
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString(_TEXT(0x9348, "\"Thou hast no room for me in thy party! "));
        ULTIMA_1850_PrintString(_TEXT(0x9372, "Seek me again if one of thy members doth leave\nthee."));

        return 0;
    }

    TALK_075a_SelectNthTalkString(0);

    // byte-copy loops, which MSC 5.0 turns into mov cx,1; repne movsw; movsb (memcpy gives
    // shr cx,1 / adc cx,cx); the (char*) cast is needed for this one to become a block move
    for (i = 0; i < 3; i++)
        npcName[i] = ((char*)D_bcde_talkPtr)[i];
    D_bcde_talkPtr += 3;

    npcName[3] = 0;
    rosterName_h[3] = 0;

    // c7e6
    while (1)
    {
        for (i = 0; i < 3; i++)
            rosterName_h[i] = D_55a8_party[rosterIndex_h].name[i];

        // c805
        if (TALK_0000_CompareStringNoCase(npcName, rosterName_h) != 0)
        {
            // i = DI;
            break;
        }

        rosterIndex_h--;
        if (rosterIndex_h == 0)
        {
            // c824
            TALK_0574_BuildOrPrintWord(0x22);
            TALK_0574_BuildOrPrintWord(10);
            ULTIMA_1850_PrintString(_TEXT(0x93a8, "\nSystem Error -\nNo Match!"));
            D_bcde_talkPtr = savedPtr_o;
            // i = DI;

            // -> c8b3
            return 1;
        }
    }

    // c848
    D_55a8_party[rosterIndex_h].innMapId = 0;

    member = D_55a8_party[rosterIndex_h];
    D_55a8_party[rosterIndex_h] = D_55a8_party[D_585b_partyCount];
    D_55a8_party[D_585b_partyCount] = member;

    D_585b_partyCount++;

    TOWN_0052_SetNpcKilled(D_bcdc_talkNpcIndex);
    TOWN_00b0_DespawnNpc(D_bcdc_talkNpcIndex);

    ULTIMA_2900_UpdateVitalsDisplay();
    TALK_04e2_PrintWord();

    D_bcde_talkPtr = savedPtr_o;

    return 1;
}

// Find label D_bcf4_talkLabel (a 0x90 byte, then the label's code) and process its text,
// the question.
static int TALK_093a_ProcessLabelQuestion(void)
{
    D_bcde_talkPtr = D_b21e_scratch;
    while (*D_bcde_talkPtr != D_bcf4_talkLabel)
    {
        TALK_0728_SkipOneTalkString(0x90, 0x9f);
    }
    D_bcde_talkPtr++;
    return TALK_0788_ProcessTalkString();
}

// Process the string after the one the talk pointer is in.
static int TALK_0960_ProcessNextString(void)
{
    TALK_0728_SkipOneTalkString(0, 0x90);
    return TALK_0788_ProcessTalkString();
}

// The label's default answer: the string after its question.
static int TALK_096e_ProcessLabelDefault(void)
{
    D_bcde_talkPtr = D_b21e_scratch;
    while (*D_bcde_talkPtr != D_bcf4_talkLabel)
    {
        TALK_0728_SkipOneTalkString(0x90, 0x9f);
    }
    TALK_0728_SkipOneTalkString(0, 0x9f);
    return TALK_0788_ProcessTalkString();
}

// Point the talk pointer at keyword number `keyword`: string 2 * keyword + 5 (its answer
// is the next). 0 when the keywords end first (a 0x90 byte).
static int TALK_099a_SelectKeyword(int keyword)
{
    D_bcde_talkPtr = D_b21e_scratch;
    keyword = keyword * 2 + 5;

    while (keyword != 0)
    {
        if (TALK_0728_SkipOneTalkString(0, 0x90) == 0)
            return 0;

        keyword--;
    }

    return 1;
}

// Look for one of the NPC's keywords in the typed text, at its start or after a space.
// D_bcf6_talkKeywordIndex is the keyword found. ULTIMA_6f1e_CompareWord gives the place, or -1.
static int TALK_09d8_FindKeyword(void)
{
    int matchPos;

    D_bcf6_talkKeywordIndex = 0;
    while (1)
    {
        if (TALK_099a_SelectKeyword(D_bcf6_talkKeywordIndex) == 0)
        {
            return 0;
        }

        matchPos = ULTIMA_6f1e_CompareWord(D_bcde_talkPtr, D_bcf8_inputText);
        if (matchPos != -1 && (matchPos == 0 || D_bcf8_inputText[matchPos - 1] == ' '))
        {
            return 1;
        }

        D_bcde_talkPtr++;
        D_bcf6_talkKeywordIndex++;
    }

    return 1;
}

static void TALK_0a2c_GetString(void)
{
    TALK_04e2_PrintWord();
    ULTIMA_3b1c_GetString(D_bcf8_inputText, 0xf);
}

// Print the NPC's farewell (string 4). Returns 1: the conversation is over.
static int TALK_0a3c_SayBye(void)
{
    TALK_04da_PrintQuoteMark();
    if (TALK_07aa_ProcessNthTalkString(TALK_KEY_4) == 0)
    {
        TALK_04da_PrintQuoteMark();
        TALK_04d2_PrintCr();
    }
    return 1;
}

// Answer a standard word (an index into D_4aa8_standardWords): NAME, JOB or WORK, BYE or
// THANK; the others get a rebuke. 0 answered, 1 the conversation is over, 2 not
// answered here (while a label's question is open, D_4af2_talkInLabel).
static int TALK_0a54_AnswerStandardWord(byte wordIndex)
{
    int tick;

    switch (wordIndex)
    {
    case 0:
        // ca1c
        if (D_4af2_talkInLabel != 0)
        {
            return 2;
        }

        ULTIMA_1850_PrintString(_TEXT(0x93c2, "\"My name is "));
        if (TALK_07aa_ProcessNthTalkString(TALK_KEY_NAME) != 0)
        {
            return 1;
        }

        TALK_04da_PrintQuoteMark();
        TALK_04d2_PrintCr();
        TALK_04d2_PrintCr();
        return 0;

    case 1:
    case 2:
        // ca3e
        if (D_4af2_talkInLabel != 0)
        {
            return 2;
        }

        TALK_04da_PrintQuoteMark();
        if (TALK_07aa_ProcessNthTalkString(TALK_KEY_3) != 0)
        {
            return 1;
        }

        TALK_04da_PrintQuoteMark();
        TALK_04d2_PrintCr();
        TALK_04d2_PrintCr();
        return 0;

    case 3:
    case 4:
        // ca60
        if (D_4af2_talkInLabel != 0)
        {
            return 2;
        }

        return TALK_0a3c_SayBye();

    default:
        // c9f8
        ULTIMA_1850_PrintString(_TEXT(0x93d0, "\"With language like that, how did you become an Avatar?"));
        TALK_04da_PrintQuoteMark();
        TALK_04d2_PrintCr();
        TALK_04d2_PrintCr();

        for (tick = 0; tick < 0x1c; tick++)
        {
            ULTIMA_5910_UpdateFrame();
            if (ULTIMA_1d5e_PollKey() != 0)
                break;

            ULTIMA_20fa_WaitTicks(1);
        }

        ULTIMA_1b16_ClearKbdBuffer();
        return 0;
    }
}

// "Your interest?", until the conversation is over. An empty line is BYE.
static int TALK_0b04_AskInterest(void)
{
    int matchPos;
    int result;
    byte wordIndex_k;

    while (1)
    {
        D_4af2_talkInLabel = 0;
        ULTIMA_1850_PrintString(_TEXT(0x9408, "Your interest?\n:"));
        TALK_0a2c_GetString();
        if (D_bcf8_inputText[0] == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x941a, "BYE\n\n"));
            return TALK_0a3c_SayBye();
        }
        TALK_04d2_PrintCr();
        TALK_04d2_PrintCr();
        for (wordIndex_k = 0, result = -1; wordIndex_k < 0x22; wordIndex_k++)
        {
            matchPos = ULTIMA_6f1e_CompareWord((byte*)D_4aa8_standardWords[wordIndex_k], D_bcf8_inputText);
            if (matchPos != -1 && (matchPos == 0 || D_bcf8_inputText[matchPos - 1] == ' '))
            {
                result = TALK_0a54_AnswerStandardWord(wordIndex_k);
                if (result == 0)
                {
                    break;
                }

                if (result == 1)
                {
                    return 1;
                }
            }
        }

        if (result != 0)
        {
            if (TALK_09d8_FindKeyword() == 0)
            {
                ULTIMA_1850_PrintString(_TEXT(0x9420, "\"I cannot help thee with that."));
                TALK_04da_PrintQuoteMark();
                TALK_04d2_PrintCr();
                TALK_04d2_PrintCr();
            }
            else
            {
                TALK_04da_PrintQuoteMark();
                if (TALK_07aa_ProcessNthTalkString(D_bcf6_talkKeywordIndex * 2 + 6) != 0)
                {
                    return 1;
                }
                TALK_04da_PrintQuoteMark();
                TALK_04d2_PrintCr();
                TALK_04d2_PrintCr();
            }
        }
    }
}

// Look for the typed text among the answers of label D_bcf4_talkLabel (keyword and
// reply pairs after its question and its default answer). 1 when found: the reply follows.
static int TALK_0bd4_FindLabelAnswer(void)
{
    int matchPos;

    D_bcde_talkPtr = D_b21e_scratch;
    while (*D_bcde_talkPtr != D_bcf4_talkLabel)
    {
        TALK_0728_SkipOneTalkString(0x90, 0x9f);
    }

    TALK_0728_SkipOneTalkString(0, 0x9f);
    TALK_0728_SkipOneTalkString(0, 0x9f);

    while (1)
    {
        matchPos = ULTIMA_6f1e_CompareWord(D_bcde_talkPtr, D_bcf8_inputText);
        if (matchPos != -1 && (matchPos == 0 || D_bcf8_inputText[matchPos - 1] == ' '))
        {
            return 1;
        }

        if (TALK_0728_SkipOneTalkString(0, 0x90) == 0)
        {
            return 0;
        }

        if (TALK_0728_SkipOneTalkString(0, 0x90) == 0)
        {
            return 0;
        }
    }
}

// CHECKED
// process label?
// A label: print its question, read the reply, print the matching answer or the default,
// then back to "Your interest?".
static int TALK_0c5c_AskLabelQuestion(void)
{
    int result;
    int matchPos_a;
    byte wordIndex_k;

    while (1)
    {
        TALK_04da_PrintQuoteMark();
        if (TALK_093a_ProcessLabelQuestion() != 0)
        {
            return 1;
        }

        TALK_04da_PrintQuoteMark();

        do
        {
            TALK_04d2_PrintCr();
            TALK_04d2_PrintCr();
            D_4af2_talkInLabel = 0xff;
            ULTIMA_1850_PrintString(_TEXT(0x9440, "You respond-\n:"));
            TALK_0a2c_GetString();
            if (D_bcf8_inputText[0] == 0)
            {
                ULTIMA_1850_PrintString(_TEXT(0x9450, "\n\n\"What didst thou say?"));
            }
        } while (D_bcf8_inputText[0] == 0);

        TALK_04d2_PrintCr();
        TALK_04d2_PrintCr();
        for (wordIndex_k = 0, result = -1; wordIndex_k < 0x22; wordIndex_k++)
        {
            matchPos_a = ULTIMA_6f1e_CompareWord(D_4aa8_standardWords[wordIndex_k], D_bcf8_inputText);
            if (matchPos_a != -1 && (matchPos_a == 0 || D_bcf8_inputText[matchPos_a - 1] == ' '))
            {
                result = TALK_0a54_AnswerStandardWord(wordIndex_k);
                if (result == 0)
                {
                    break;
                }

                if (result == 1)
                {
                    return 1;
                }
            }
        }

        if (result == 0)
            continue;

        // The PrintCr tail is written in both branches: MSC merges the two copies (the
        // original's shared "or ax,ax; je" and PrintCr block placed before the else). One
        // copy after the if/else gives a test per branch.
        if (TALK_0bd4_FindLabelAnswer() == 0)
        {
            TALK_04da_PrintQuoteMark();
            if (TALK_096e_ProcessLabelDefault() != 0)
            {
                return 1;
            }
            TALK_04da_PrintQuoteMark();
            TALK_04d2_PrintCr();
            TALK_04d2_PrintCr();
            return TALK_0b04_AskInterest();
        }
        else
        {
            TALK_04da_PrintQuoteMark();
            if (TALK_0960_ProcessNextString() != 0)
            {
                return 1;
            }
            TALK_04da_PrintQuoteMark();
            TALK_04d2_PrintCr();
            TALK_04d2_PrintCr();
            return TALK_0b04_AskInterest();
        }
    }
}

// NOT MATCHING (u32 operation)
// set npc met flag
static int TALK_0d42_SetNpcMet(int npcIndex)
{
    u32 mask;

    mask = 1;
    mask <<= npcIndex; // aNNalshl
    (*(u32*)&D_5bda_npcMet[(D_5893_mapId - 1) * 4]) |= mask;
}

// NOT MATCHING (u32 operation)
// check npc met flag
static int TALK_0d7a_IsNpcMet(int npcIndex)
{
    u32 mask;

    mask = 1;
    mask <<= npcIndex; // aNNalshl
    return (*(u32*)&D_5bda_npcMet[(D_5893_mapId - 1) * 4] & mask) != 0;
}

// The bytes after a script code that takes arguments (D_4aee_talkPendingCode): 0x85 gold,
// three digits; 0x86 an item; 0x8c a label to go to when the NPC knows the Avatar's name
// (0xff: just go on); 0xfe a karma value and a label to go to when the karma is at least that.
static int TALK_0dbe_ProcessCodeArgument(byte value)
{
    switch (D_4aee_talkPendingCode)
    {
    default:
        // 0dda
        D_4aef_talkCodeArgCount = D_4aee_talkPendingCode = 0;
        // 0dfc ->
        // 0e74
        break;

    case 0x85:
        // 0de4
        D_bce0_talkCodeArgs[D_4aef_talkCodeArgCount++] = value;
        if (D_4aef_talkCodeArgCount != 3)
        {
            // 0dfc
            return 0;
        }

        // 0e02
        if (TALK_05b6_PayGold() != 0)
        {
            // -> 0e74
            return 1;
        }

        // 0dda
        D_4aef_talkCodeArgCount = D_4aee_talkPendingCode = 0;
        break;

    case 0x86:
        // 0e0e
        TALK_0682_ReceiveItem(value & 0x7f);

        // 0dda
        D_4aef_talkCodeArgCount = D_4aee_talkPendingCode = 0;
        break;

    case 0x8c:
        // 0e1c
        D_4aef_talkCodeArgCount = D_4aee_talkPendingCode = 0;

        if (TALK_0d7a_IsNpcMet(D_bcdc_talkNpcIndex) == 0)
        {
            // 0dfc
            return 0;
        }

        if (value == 0xff)
        {
            return TALK_0b04_AskInterest();
        }

        // 0e3a
        D_bcf4_talkLabel = value;

        // 0e3d
        return TALK_0c5c_AskLabelQuestion();

    case 0xfe:
        // 0e46
        D_bce0_talkCodeArgs[D_4aef_talkCodeArgCount++] = value;
        if (D_4aef_talkCodeArgCount != 2)
        {
            // 0dfc
            return 0;
        }

        D_4aef_talkCodeArgCount = D_4aee_talkPendingCode = 0;

        if (D_5888_karma < D_bce0_talkCodeArgs[0])
        {
            // 0dfc
            return 0;
        }

        D_bcf4_talkLabel = D_bce0_talkCodeArgs[1];

        // 0e3d
        return TALK_0c5c_AskLabelQuestion();
    }

    return 0;
}

// CHECKED
static void TALK_0e78_AskName(void)
{
    int matchPos_i;
    int i;
    int member;
    char name[5];

    member = 0;
    TALK_04da_PrintQuoteMark();
    TALK_04e2_PrintWord();
    ULTIMA_1850_PrintString(_TEXT(0x9468, "What is thy name?\"\n"));
    ULTIMA_1850_PrintString(_TEXT(0x947c, "\nYou respond-\n:"));
    TALK_0a2c_GetString();

    D_4aee_talkPendingCode = D_4aef_talkCodeArgCount = 0;

    if (D_bcf8_inputText[0] == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x948c, "\n\n\"If you say so..."));
    }
    else
    {
        for (; member < D_585b_partyCount; member++)
        {
            for (i = 0; i < 4; i++)
            {
                name[i] = D_55a8_party[member].name[i];
            }

            name[4] = 0;

            matchPos_i = ULTIMA_6f1e_CompareWord(name, D_bcf8_inputText);
            if (matchPos_i != -1 && (matchPos_i == 0 || (D_bcf8_inputText[matchPos_i - 1] == ' ')))
            {
                TALK_0d42_SetNpcMet(D_bcdc_talkNpcIndex);
                ULTIMA_1850_PrintString(_TEXT(0x94a0, "\n\n\"A pleasure!"));
                return;
            }
        }

        ULTIMA_1850_PrintString(_TEXT(0x94b0, "\n\n\"If you say so..."));
    }
}

// Process one byte of an NPC's script: a script code (0x81-0x9f, 0xfe, 0xff), a word of
// the word table (below 0x81, D_24ea_wordTable) or a character. Returns 1 when the
// conversation is over. The .TLK format and the codes 0x81, 0x83, 0x87, 0x88, 0x8c, 0x8d,
// 0x8f and 0x91-0x9b are on the Ultima Codex, "Ultima V internal formats"; the other
// codes' meanings are read from this code.
static int TALK_0f32_ProcessChar(byte code)
{
    char* word;
    int tick;
    byte* savedPtr;

    if (D_4aee_talkPendingCode != 0)
    {
        return TALK_0dbe_ProcessCodeArgument(code);
    }

    // 0f4c
    D_4aef_talkCodeArgCount = 0;
    if ((code == 0xa2) && (D_4af4_talkLastChar == 0xa2))
    {
        // 0f5e
        return 0;
    }

    // 0f64
    switch (code)
    {
    case 0x81: // avatar
        // 0f86
        TALK_07e4_PrintAvatarName();
        break;
    case 0x82: // end conversation
        // 0f8c
        return 1;
    case 0x83: // pause
        // 0f92
        for (tick = 0; tick < 0x1c; tick++)
        {
            ULTIMA_5910_UpdateFrame();
            if (ULTIMA_1d5e_PollKey() != 0)
                break;
            ULTIMA_20fa_WaitTicks(1);
        }
        ULTIMA_1b16_ClearKbdBuffer();
        return 0; // -> 0f5e
    case 0x84:    // join
        // 0fb6
        return TALK_080a_Join(); // -> 1114
    case 0x87:                // key_or
        // 0fbc
        savedPtr = D_bcde_talkPtr;
        if (TALK_07be_ProcessOrAnswer() != 0)
        {
            return 1;
        }
        D_bcde_talkPtr = savedPtr;
        return 0; // -> 0f5e
    case 0x88:    // ask name
        // 0fd2
        TALK_0e78_AskName();
        return 0; // -> 0f5e
    case 0x89:    // karma +1
        // 0fd8
        ULTIMA_3ef0_IncreaseByte(&D_5888_karma, 1, 99);
        return 0; // -> 0f5e
    case 0x8a:    // karma -1
        // 0fea
        ULTIMA_3f36_DecreaseByte(&D_5888_karma, 1);
        return 0; // -> 0f5e
    case 0x8b:    // call guards
        // 0ff8
        TOWN_0958_RaiseAlarm();
        return 0; // -> 0f5e
    case 0x8e:    // rune
        // 0ffe
        D_4af0_talkCharMask ^= 0x80;
        return 0; // -> 0f5e
    case 0xff:    // no selection
        // 1006
        TALK_04e2_PrintWord();
        return TALK_0b04_AskInterest(); // -> 1114
    case 0x8f:                // wait
        // 1010
        ULTIMA_266c_GetChar();
        return 0; // -> 0f5e
    case 0xfe:
    case 0x85: // gold-
    case 0x86: // change item 65
    case 0x8c: // if else name
        // 1016
        D_4aee_talkPendingCode = code;
        return 0;
    case 0x91: // label_0
    case 0x92: // label_1
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
    case 0x98:
    case 0x99:
    case 0x9a:
    case 0x9b: // label_10
    case 0x9c: // label?
    case 0x9d: // label?
    case 0x9e: // label?
    case 0x9f: // label?
        // 1020
        D_bcf4_talkLabel = code;
        return TALK_0c5c_AskLabelQuestion(); // -> 1114
        // ----------
    default:
        // 10ce
        if (code < 0x81)
        {
            TALK_0574_BuildOrPrintWord(0xa0); // 0x20

            word = D_24ea_wordTable[code - 1];
            while (*word != 0)
            {
                TALK_0574_BuildOrPrintWord((byte)*word++ | 0x80);
            }

            if (*D_24ea_wordTable[code - 1] == 0)
            {
                TALK_0574_BuildOrPrintWord(code);
                return 0; // -> 0f5e
            }

            D_4af5_talkAfterWord = 1;
        }
        else
        {
            // 1062
            code |= 0x80;
            if (code == 0x8d) // 0xd newline
            {
                code = 0x8a;
            }

            D_4af4_talkLastChar = code &= D_4af0_talkCharMask;

            if (D_4af5_talkAfterWord != 0)
            {
                TALK_0574_BuildOrPrintWord(0xa0); // 0x20
            }

            TALK_0574_BuildOrPrintWord(code);
            D_4af5_talkAfterWord = 0;
        }
        break;
    }

    return 0;
}

// The start of a conversation: "You see" and the description, then the greeting when the
// NPC knows the Avatar's name, else (one time in two) "I am called" and the name.
static int TALK_111c_Introduce(void)
{
    ULTIMA_1850_PrintString(_TEXT(0x94c4, "You see "));

    if (TALK_07aa_ProcessNthTalkString(TALK_KEY_DESC) != 0)
    {
        return 1;
    }
    else
    {
        TALK_04d2_PrintCr();
        TALK_04d2_PrintCr();
        if (TALK_0d7a_IsNpcMet(D_bcdc_talkNpcIndex) == 0)
        {
            ULTIMA_207e_srand(ULTIMA_2056_GetTime());
            if (ULTIMA_2092_RandomRange(0, 1) != 0)
            {
                // "I am called\nNAME"\n\n
                ULTIMA_1850_PrintString(_TEXT(0x94ce, "\"I am called "));
                if (TALK_07aa_ProcessNthTalkString(TALK_KEY_NAME) == 0)
                {
                    TALK_04da_PrintQuoteMark();
                    TALK_04d2_PrintCr();
                    TALK_04d2_PrintCr();
                }
                else
                {
                    return 1;
                }
            }
        }
        else
        {
            TALK_04da_PrintQuoteMark();
            if (TALK_07aa_ProcessNthTalkString(TALK_KEY_2) == 0)
            {
                TALK_04da_PrintQuoteMark();
                TALK_04d2_PrintCr();
                TALK_04d2_PrintCr();
            }
            else
            {
                return 1;
            }
        }

        return 0;
    }
}

// After a conversation in a town where shadowlord 0 is (D_5958_townShadowlord; Falsehood,
// if the Codex's order of the three is right): something is stolen. A key, a gem or a
// torch; when there are none a piece of equipment, a potion, a scroll, else gold.
static void TALK_1180_FalsehoodSteals(void)
{
    int item;

    if (D_5958_townShadowlord == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x94dc, "\nSomething was stolen!\n"));
        ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
        ULTIMA_207e_srand(ULTIMA_2056_GetTime());

        if ((D_57ac_keys | D_57ad_gems | D_57ae_torches) != 0)
        {
            do
            {
                while (1)
                {
                    switch (ULTIMA_2092_RandomRange(0, 2))
                    {
                    case 0:
                        if (D_57ac_keys != 0)
                        {
                            ULTIMA_3f36_DecreaseByte(&D_57ac_keys, 1);
                            return;
                        }
                        break;

                    case 1:
                        if (D_57ad_gems != 0)
                        {
                            ULTIMA_3f36_DecreaseByte(&D_57ad_gems, 1);
                            return;
                        }
                        break;

                    case 2:
                        if (D_57ae_torches != 0)
                        {
                            ULTIMA_3f36_DecreaseByte(&D_57ae_torches, 1);
                            return;
                        }
                        break;
                    default:
                        return;
                    }
                }
            } while (D_57ad_gems == 0);

            ULTIMA_3f36_DecreaseByte(&D_57ad_gems, 1); // BUG?
        }
        else
        {
            for (item = 0x2f; item >= 0; item--)
            {
                if (D_57c0_equipment[item] != 0)
                {
                    ULTIMA_3f36_DecreaseByte(&D_57c0_equipment[item], 1);
                    return;
                }
            }

            for (item = 7; item >= 0; item--)
            {
                if (D_5828_potions[item] != 0)
                {
                    ULTIMA_3f36_DecreaseByte(&D_5828_potions[item], 1);
                    return;
                }
            }

            for (item = 7; item >= 0; item--)
            {
                if (D_5820_scrolls[item] != 0)
                {
                    ULTIMA_3f36_DecreaseByte(&D_5820_scrolls[item], 1);
                    return;
                }
            }

            ULTIMA_3f54_DecreaseInt(&D_57aa_gold, ULTIMA_2092_RandomRange(1, 0xf));
            ULTIMA_2900_UpdateVitalsDisplay();
        }
    }
}

// Read the NPC's script from the map's .TLK file into D_b21e_scratch and hold the conversation.
// The file starts with a count and that many (dialog number, offset) pairs (Ultima Codex, "Ultima V internal formats").
static void TALK_127e_StartTalk(int dialogNumber)
{
    s16 count;
    s16 entry_g;
    s16 offset_b;
    s16* index;
    uint fileIndex;

    fileIndex = (D_5893_mapId - 1) >> 3;
    ULTIMA_256e_ReadFileFromDisk(D_4aa0_talkFileNames[fileIndex], D_b21e_scratch, 0x200, 0);

    // 12a9
    index = (s16*)D_b21e_scratch;
    count = *index++;

    // 12ad
    for (entry_g = 1; *index++ != dialogNumber && entry_g <= count; index++, entry_g++)
    {
    }

    // 12d6
    offset_b = *index;
    ULTIMA_256e_ReadFileFromDisk(D_4aa0_talkFileNames[fileIndex], D_b21e_scratch, 0x400, offset_b);

    if (TALK_111c_Introduce() == 0)
    {
        if (TALK_0b04_AskInterest() == 0)
        {
            TALK_0a3c_SayBye();
        }
    }

    TALK_1180_FalsehoodSteals();
}
