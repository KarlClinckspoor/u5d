#include "common/common.h"

// this module's communal variables (vars.h, DS:a9bc)
u16 talkNpcIndex; // the slot of the NPC talked to
u8* talkPtr; // the place in the NPC's script
u8 talkCodeArgs[4]; // the bytes after a script code (ProcessCodeArgument)
char talkWord[0x10]; // the word being collected, printed at its end (word wrap)
u8 talkLabel; // the label being asked, 0x91-0x9f
u16 talkKeywordIndex; // the NPC's keyword that matched
char inputText[16]; // what the player typed

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

static int ProcessChar(byte code);
static void StartTalk(int a);

// CHECKED
// 1 when the two strings are the same, ignoring case and the high bit.
static int CompareStringNoCase(char* first, char* second)
{
    while (1)
    {
        if (ToUpperChar((*(byte*)first++) & 0x7f) != ToUpperChar((*(byte*)second++) & 0x7f))
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
// reaches one tile further (TalkCmd).
static int CanTalkAcrossTile(int x, int y)
{
    switch (*GetTileAddr(x, y))
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
static int AskPay(void)
{
    char ch;

    PrintString(/*0x9052*/ "\n\nDost thou pay?\n\n:");

    do
    {
        ch = ToUpperChar(GetChar());
        if (ch == 'Y')
        {
            PrintString(/*0x9066*/ "Yes\n");
            return 0;
        }
    } while (ch != 'N');

    PrintString(/*0x906c*/ "No!\n");
    return 1;
}

// CHECKED
// talk to merchant (shoppe)
// merchantCode is the NPC's dialog number, 0x81-0x88: the shop kind + 0x81. Finds this
// town in the kind's row of shopMapIds (the shop index) and runs the shop.
// Which kind is which: 2 the stables, 6 the healer, 7 the inn (their texts); the others
// are not named yet.
static void TalkToMerchant(int merchantCode)
{
    int playerIndex;

    if ((partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE && merchantCode != 0x83)
    {
        PrintString(/*0x9072*/ "A merchant says:\n\"GET THAT HORSE OUT OF HERE!\"\n");
    }
    else
    {
        if (activePlayer != 0xff)
        {
            playerIndex = activePlayer;
        }
        else
        {
            GetFirstActivePartyMember();
            playerIndex = tempX;
        }

        shopKind = merchantCode - 0x81;

        for (shopIndex = 0; (&shopMapIds[0][0])[shopIndex + shopKind * 16] != mapId && shopIndex < 0x10; shopIndex++)
        {
        }

        shopName = shopNames[shopKind][shopIndex];
        shopOwner = shopOwners[shopKind][shopIndex];

        switch (shopKind)
        {
        case 0:
            Armourer(playerIndex);
            break;
        case 1:
            Pub(playerIndex);
            break;
        case 2:
            Stables(playerIndex);
            break;
        case 3:
            Shipwright(playerIndex);
            break;
        case 4:
            Reagents(playerIndex);
            break;
        case 5:
            Provisioner(playerIndex);
            break;
        case 6:
            Healer(playerIndex);
            break;
        case 7:
            Inn(playerIndex);
        }
    }
}

// the password CompareStringNoCase checks: an array, so in _DATA (DS:4a9a)
char password[] = "IMPE";

// CHECKED
// Dialog number 0xff: someone who lets the party pass for a price. On map 5 half the
// gold "to charity"; on map 0x12 the password (only when statusIcon is 0x1d; the text says
// "bearer of the Badge", so probably the Black Badge is worn); elsewhere a guard's
// tribute of 10 gp for each living member. Returns 0 when paid or passed, 1 when not.
static int GuardDemandsTribute(void)
{
    int tribute_i;
    uint member;
    char answer[0xf];

    if (mapId != 0x12)
    {
        // 01f3
        if (mapId == 5)
        {
            PrintChar('\"');
            PrintString(/*0x90a2*/ "Thou wilt give\nhalf thy gold to\ncharity!");
            PrintChar('\"');

            if (AskPay() != 0)
            {
                // 0216
                return 1;
            }

            // 021c
            gold /= 2;

            // 0228
            UpdateVitalsDisplay();
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
            for (member = 0; member != (int)partyCount; member++)
            {
                // 025a
                if (party[member].status != STATUS_DEAD)
                {
                    tribute_i += 10;
                }
            }

            // 026c
            PrintString(/*0x90cc*/ "A guard demands\na ");
            PrintNumber(tribute_i, 2, 0x20);
            PrintString(/*0x90e0*/ " gp tribute\nto Blackthorn!");

            if (AskPay() != 0)
            {
                // 028d -> 0216
                return 1;
            }

            // 028f
            if (gold >= (uint)tribute_i) // jbe: an unsigned compare
            {
                // 029a
                gold -= tribute_i;

                UpdateVitalsDisplay();
                return 0;
            }

            // 0297 -> 0216
            return 1;
        }
    }
    else
    {
        // 02a4
        if (statusIcon == 0x1d)
        {
            // 02ae
            PrintChar('\"');
            PrintString(/*0x90fc*/ "Give now the\npassword, bearer\nof the Badge!");
            PrintChar('\"');
            PrintString(/*0x9128*/ "\n\nYour response?\n");
            GetString(answer, 0xe);
            PrintChar('\n');
            answer[4] = 0;

            if (CompareStringNoCase(password, answer) != 0)
            {
                // 02f2
                PrintChar('\n');
                PrintChar('\"');
                PrintString(/*0x913a*/ "Pass, friend!");
                PrintChar('\"');
                PrintChar('\n');
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
int TalkToNpc(int npcIndex)
{
    int dialogNumber_f;
    NpcFmt* npc;
    NpcScheduleFmt* schedule;

    PrintChar('\n');

    talkNpcIndex = npcIndex;
    npc = &npcs[npcIndex];
    schedule = &npcSchedules[npcIndex];

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
        if (actors[npc->actorIdx].tile != TILE_ACTOR_GUARD || ((npc->scheduleIdx & 1) != 0 && (npc->dialogNumber) != 0))
            goto L_join;
        PrintString(/*0x9148*/ "The guard offers\nno response!\n");
        return 0;
    }
L_join:
    dialogNumber_f = npc->dialogNumber;
    if (dialogNumber_f == 0)
    {
        PrintString(/*0x9168*/ "No response!\n");
        return 0;
    }
    else
    {
        // c316
        if (dialogNumber_f < 0x80)
        {
            StartTalk(dialogNumber_f);
            return 0;
        }

        if (dialogNumber_f == 0xfd)
        {
            PrintChar('\"');
            PrintString(/*0x9176*/ "Don't hurt me!\nPlease go away!");
            PrintChar('\"');
            PrintChar('\n');
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
            return GuardDemandsTribute();
        }

        if ((npc->scheduleIdx & 1) != 0 && (ScheduleSlot(talkNpcIndex, D_587f_hour) & 1) != 0)
        {
            TalkToMerchant(dialogNumber_f);
            return 0;
        }
        else
        {
            PrintString(/*0x9196*/ "A merchant says:\n\"Come see me at\nmy shoppe, ");
            PrintString(/*0x91c4*/ "when\nit's open!\"\n");
            return 0;
        }
    }
}

int TalkCmd(void)
{
    int dirY;
    int y_o;
    int dirX;
    int x_i;
    int npcIndex_f;

    if (SelectDirection() == 0)
    {
        return 0;
    }

    dirX = tempX;
    dirY = tempY;
    x_i = dirX + D_5896_mapX;
    y_o = dirY + D_5897_mapY;

    if (FindActorTileAtPos(x_i, y_o, D_5895_mapLevel) == 0 && CanTalkAcrossTile(x_i, y_o) != 0)
    {
        x_i += dirX;
        y_o += dirY;
    }

    if (FindActorTileAtPos(x_i, y_o, D_5895_mapLevel) == 0)
    {
        PrintString(/*0x91d6*/ "\nNobody's here!\n");
    }
    else
    {
        npcIndex_f = FindNpcByActor(tempX);
        switch (*GetTileAddr(x_i, y_o))
        {
        case TILE_MAP_BED:
            PrintString(/*0x91e8*/ "\n\"Zzzzzz...\"\n");
            break;
        case TILE_MAP_MIRROR:
            PrintString(/*0x91f6*/ "\nNo response!\n");
            break;
        default:
            return TalkToNpc(npcIndex_f);
        }
    }

    return 0;
}

static void PrintCr(void) { ProcessChar(0x80 | 0xd); }

static void PrintQuoteMark(void) { ProcessChar(0x80 | 0x22); }

// CHECKED
// Print the word in talkWord. A character with the high bit set is printed in
// charset 0, one without it in charset 1 (the runes: script code 0x8e flips
// talkCharMask). A line is not started with a space.
static void PrintWord(void)
{
    byte ch;
    int pos;

    pos = 0;

    // c469
    if (talkWordLength != 0)
    {
        while (pos < talkWordLength) // c4c9
        {
            // c4d5 -> c47c
            if (talkWord[pos] & 0x80)
                SelectCharset(0);
            else
                SelectCharset(1);
            ch = talkWord[pos] & 0x7f;
            pos++;

            if (GetCurrentTextX() != 0 || ch != 0x20)
            {
                // c49a
                if (talkTextX < 0xf || ch != 10)
                {
                    // c4a7
                    if (ch != 10)
                    {
                        talkTextX = GetCurrentTextX();
                    }

                    // c4b3
                    PrintChar(ch);
                }
                else if (ch == 10) // c4be
                {
                    talkTextX = 0;
                }
            }
        }

        // c4e0
        talkWordLength = 0;
        SelectCharset(0);
    }
}

// Add ch to the word; at a space or a newline (or when the buffer is full) print it,
// on a new line when it would pass column 18.
static void BuildOrPrintWord(byte ch)
{
    if (talkWordLength != 0x10)
    {
        talkWord[talkWordLength++] = ch;

        if (ch != 0x8a && ch != 0xa0) // 0xa / 0x20
        {
            return;
        }

        if (GetCurrentTextX() + (uint)talkWordLength >= 0x12)
        {
            PrintChar('\n');
        }
    }

    PrintWord();
}

static int AskInterest(void);
char* talkFileNames[4] =
{
    /*0x9206*/ "TOWNE.TLK",
    /*0x9210*/ "DWELLING.TLK",
    /*0x921e*/ "CASTLE.TLK",
    /*0x922a*/ "KEEP.TLK"
};

char* standardWords[0x22 + 1] =
{
    /*0x9234*/ "NAME",
    /*0x923a*/ "JOB",
    /*0x923e*/ "WORK",
    /*0x9244*/ "BYE",
    /*0x9248*/ "THANK",
    /*0x924e*/ "FUCK",
    /*0x9254*/ "SHIT",
    /*0x925a*/ "DAMN",
    /*0x9260*/ "DICK",
    /*0x9266*/ "PRICK",
    /*0x926c*/ "PUSSY",
    /*0x9272*/ "CUNT",
    /*0x9278*/ "ASS",
    /*0x927c*/ "BUTT",
    /*0x9282*/ "BOOGER",
    /*0x928a*/ "PISS",
    /*0x9290*/ "JACK OFF",
    /*0x929a*/ "MASTURBATE",
    /*0x92a6*/ "SUCK",
    /*0x92ac*/ "FART",
    /*0x92b2*/ "TITS",
    /*0x92b8*/ "BOOB",
    /*0x92be*/ "MELONS",
    /*0x92c6*/ "BLOW",
    /*0x92cc*/ "PENIS",
    /*0x92d2*/ "BREAST",
    /*0x92da*/ "CLIT",
    /*0x92e0*/ "BALLS",
    /*0x92e6*/ "SCROTUM",
    /*0x92ee*/ "NUTS",
    /*0x92f4*/ "BULLSHIT",
    /*0x92fe*/ "CUM",
    /*0x9302*/ "CROTCH",
    /*0x930a*/ "MOTHERFUCKER",
    /*0x9318*/ "ELECTRONIC ARTS" // hidden string
};

u8 talkPendingCode = 0;

u8 talkCodeArgCount = 0;

u8 talkCharMask = 0xff;

u8 talkWordLength = 0;

u8 talkInLabel = 0;

u8 talkTextX = 0;

u8 talkLastChar = 0;

u8 talkAfterWord = 0;

// Script code 0x85 (gold): the three digits after it are the price. Pays it (to a beggar:
// karma, once in 100 turns), or says the party cannot and goes back to "Your interest?".
static int PayGold(void)
{
    int amount;

    amount = (talkCodeArgs[0] & 0x7f) * 100 + (talkCodeArgs[1] & 0x7f) * 10 + (talkCodeArgs[2] & 0x7f) - 0x14d0; // 0x14d0 == 5328
    if (gold >= amount)
    {
        gold -= amount;
        UpdateVitalsDisplay();
        if ((actors[npcs[talkNpcIndex].actorIdx].tile & 0xfc) == TILE_ACTOR_BEGGAR && turnCount >= 100)
        {
            turnCount = 0;
            IncreaseByte(&karma, 1, 99);
            if (gold == 0)
            {
                IncreaseByte(&karma, 2, 99);
            }
        }

        return 0;
    }
    else
    {
        talkWordLength = 0;
        PrintChar('\"');
        PrintString(/*0x9328*/ "Thou hast not enough gold!");
        PrintChar('\"');
        PrintString(/*0x9344*/ "\n\n");

        talkCodeArgCount = talkPendingCode = 0;

        return AskInterest();
    }
}

// Script code 0x86 (the NPC gives something): below 0x40 an equipment number, 0x41 food,
// 0x42 gold, 0x43 keys, 0x44 gems, 0x45 torches, 0x46 grapple, 0x47 magic carpets,
// 0x48 sextant, 0x49 spyglass, 0x4a black badge, 0x4b skull keys (vars.h's savegame names).
static void ReceiveItem(byte itemCode)
{
    if (itemCode < 0x40)
    {
        IncreaseByte(&equipment[itemCode], 1, 99);
        return;
    }

    switch (itemCode)
    {
    case 0x41:
        IncreaseInt(&food, 1, 9999);
        UpdateVitalsDisplay();
        break;
    case 0x42:
        IncreaseInt(&gold, 1, 9999);
        UpdateVitalsDisplay();
        break;
    case 0x43:
        IncreaseByte(&keys, 1, 99);
        break;
    case 0x44:
        IncreaseByte(&gems, 1, 99);
        break;
    case 0x45:
        IncreaseByte(&torches, 1, 99);
        break;
    case 0x46:
        IncreaseByte(&grapple, 1, 99);
        break;
    case 0x47:
        IncreaseByte(&carpets, 1, 99);
        break;
    case 0x48:
        sextants = 0xff;
        break;
    case 0x49:
        spyglasses = 0xff;
        break;
    case 0x4a:
        blackBadge = 0xff;
        break;
    case 0x4b:
        IncreaseByte(&skullKeys, 1, 99);
        break;
    }
}

// skip one talk string
// Move talkPtr past the next terminator byte (returns 1) or endMarker byte (0).
static int SkipOneTalkString(byte terminator, byte endMarker)
{
    byte value;

    do
    {
        value = *talkPtr++;
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
// Point talkPtr at string n of the NPC's script in scratch. The strings end in 0:
// 0 name, 1 description, 2 greeting, 3 job, 4 bye, then keyword and answer pairs (Ultima Codex, "Ultima V internal formats").
static void SelectNthTalkString(int n)
{
    talkPtr = scratch;

    if (n != 0)
    {
        for (; n != 0; n--)
        {
            SkipOneTalkString(0, 0x90);
        }
    }
}

static int ProcessChar(byte code);

// process talk string
// Process the string at talkPtr. 1 when the conversation is over.
static int ProcessTalkString(void)
{
    while (*talkPtr != 0)
    {
        if (ProcessChar(*talkPtr++) != 0)
        {
            return 1;
        }
    }

    return 0;
}

static int ProcessNthTalkString(byte n)
{
    SelectNthTalkString(n);
    return ProcessTalkString();
}

// Script code 0x87 (or): the keyword before it shares the answer of the keyword after it.
// Skips that keyword and processes what follows (probably: the layout is from the Codex page).
static int ProcessOrAnswer(void)
{
    talkPtr++;

    while (*talkPtr == 0)
    {
        talkPtr++;
    }

    while (*talkPtr++ != 0)
    {
        // empty
    }

    return ProcessTalkString();
}

static void PrintAvatarName(void)
{
    byte* name;
    name = party[0].name;

    while (*name != 0)
    {
        ProcessChar(*name++ | 0x80);
    }
}

// CHECKED
// Script code 0x84 (join). The NPC's roster entry is found by the first three letters of
// its name, from the end of the roster down, and swapped into the first free party place.
static int Join(void)
{
    int i; // 30..2f
    byte* savedPtr_o;   // 2e..2d
    char rosterName_h[4]; // 2c..29
    S_55a8 member;  // 28..09 (size: 0x20)
    char npcName[4];  // 08..05
    int rosterIndex_h;      // 04..03

    savedPtr_o = talkPtr;
    rosterIndex_h = 0xf;

    if (partyCount == 6)
    {
        GetChar();
        PrintString(/*0x9348*/ "\"Thou hast no room for me in thy party! ");
        PrintString(/*0x9372*/ "Seek me again if one of thy members doth leave\nthee.");

        return 0;
    }

    SelectNthTalkString(0);

    // byte-copy loops, which MSC 5.0 turns into mov cx,1; repne movsw; movsb (memcpy gives
    // shr cx,1 / adc cx,cx); the (char*) cast is needed for this one to become a block move
    for (i = 0; i < 3; i++)
        npcName[i] = ((char*)talkPtr)[i];
    talkPtr += 3;

    npcName[3] = 0;
    rosterName_h[3] = 0;

    // c7e6
    while (1)
    {
        for (i = 0; i < 3; i++)
            rosterName_h[i] = party[rosterIndex_h].name[i];

        // c805
        if (CompareStringNoCase(npcName, rosterName_h) != 0)
        {
            // i = DI;
            break;
        }

        rosterIndex_h--;
        if (rosterIndex_h == 0)
        {
            // c824
            BuildOrPrintWord(0x22);
            BuildOrPrintWord(10);
            PrintString(/*0x93a8*/ "\nSystem Error -\nNo Match!");
            talkPtr = savedPtr_o;
            // i = DI;

            // -> c8b3
            return 1;
        }
    }

    // c848
    party[rosterIndex_h].innMapId = 0;

    member = party[rosterIndex_h];
    party[rosterIndex_h] = party[partyCount];
    party[partyCount] = member;

    partyCount++;

    SetNpcKilled(talkNpcIndex);
    DespawnNpc(talkNpcIndex);

    UpdateVitalsDisplay();
    PrintWord();

    talkPtr = savedPtr_o;

    return 1;
}

// Find label talkLabel (a 0x90 byte, then the label's code) and process its text,
// the question.
static int ProcessLabelQuestion(void)
{
    talkPtr = scratch;
    while (*talkPtr != talkLabel)
    {
        SkipOneTalkString(0x90, 0x9f);
    }
    talkPtr++;
    return ProcessTalkString();
}

// Process the string after the one the talk pointer is in.
static int ProcessNextString(void)
{
    SkipOneTalkString(0, 0x90);
    return ProcessTalkString();
}

// The label's default answer: the string after its question.
static int ProcessLabelDefault(void)
{
    talkPtr = scratch;
    while (*talkPtr != talkLabel)
    {
        SkipOneTalkString(0x90, 0x9f);
    }
    SkipOneTalkString(0, 0x9f);
    return ProcessTalkString();
}

// Point the talk pointer at keyword number `keyword`: string 2 * keyword + 5 (its answer
// is the next). 0 when the keywords end first (a 0x90 byte).
static int SelectKeyword(int keyword)
{
    talkPtr = scratch;
    keyword = keyword * 2 + 5;

    while (keyword != 0)
    {
        if (SkipOneTalkString(0, 0x90) == 0)
            return 0;

        keyword--;
    }

    return 1;
}

// Look for one of the NPC's keywords in the typed text, at its start or after a space.
// talkKeywordIndex is the keyword found. CompareWord gives the place, or -1.
static int FindKeyword(void)
{
    int matchPos;

    talkKeywordIndex = 0;
    while (1)
    {
        if (SelectKeyword(talkKeywordIndex) == 0)
        {
            return 0;
        }

        matchPos = CompareWord(talkPtr, inputText);
        if (matchPos != -1 && (matchPos == 0 || inputText[matchPos - 1] == ' '))
        {
            return 1;
        }

        talkPtr++;
        talkKeywordIndex++;
    }

    return 1;
}

static void TalkGetString(void)
{
    PrintWord();
    GetString(inputText, 0xf);
}

// Print the NPC's farewell (string 4). Returns 1: the conversation is over.
static int SayBye(void)
{
    PrintQuoteMark();
    if (ProcessNthTalkString(TALK_KEY_4) == 0)
    {
        PrintQuoteMark();
        PrintCr();
    }
    return 1;
}

// Answer a standard word (an index into standardWords): NAME, JOB or WORK, BYE or
// THANK; the others get a rebuke. 0 answered, 1 the conversation is over, 2 not
// answered here (while a label's question is open, talkInLabel).
static int AnswerStandardWord(byte wordIndex)
{
    int tick;

    switch (wordIndex)
    {
    case 0:
        // ca1c
        if (talkInLabel != 0)
        {
            return 2;
        }

        PrintString(/*0x93c2*/ "\"My name is ");
        if (ProcessNthTalkString(TALK_KEY_NAME) != 0)
        {
            return 1;
        }

        PrintQuoteMark();
        PrintCr();
        PrintCr();
        return 0;

    case 1:
    case 2:
        // ca3e
        if (talkInLabel != 0)
        {
            return 2;
        }

        PrintQuoteMark();
        if (ProcessNthTalkString(TALK_KEY_3) != 0)
        {
            return 1;
        }

        PrintQuoteMark();
        PrintCr();
        PrintCr();
        return 0;

    case 3:
    case 4:
        // ca60
        if (talkInLabel != 0)
        {
            return 2;
        }

        return SayBye();

    default:
        // c9f8
        PrintString(/*0x93d0*/ "\"With language like that, how did you become an Avatar?");
        PrintQuoteMark();
        PrintCr();
        PrintCr();

        for (tick = 0; tick < 0x1c; tick++)
        {
            UpdateFrame();
            if (PollKey() != 0)
                break;

            WaitTicks(1);
        }

        ClearKbdBuffer();
        return 0;
    }
}

// "Your interest?", until the conversation is over. An empty line is BYE.
static int AskInterest(void)
{
    int matchPos;
    int result;
    byte wordIndex_k;

    while (1)
    {
        talkInLabel = 0;
        PrintString(/*0x9408*/ "Your interest?\n:");
        TalkGetString();
        if (inputText[0] == 0)
        {
            PrintString(/*0x941a*/ "BYE\n\n");
            return SayBye();
        }
        PrintCr();
        PrintCr();
        for (wordIndex_k = 0, result = -1; wordIndex_k < 0x22; wordIndex_k++)
        {
            matchPos = CompareWord((byte*)standardWords[wordIndex_k], inputText);
            if (matchPos != -1 && (matchPos == 0 || inputText[matchPos - 1] == ' '))
            {
                result = AnswerStandardWord(wordIndex_k);
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
            if (FindKeyword() == 0)
            {
                PrintString(/*0x9420*/ "\"I cannot help thee with that.");
                PrintQuoteMark();
                PrintCr();
                PrintCr();
            }
            else
            {
                PrintQuoteMark();
                if (ProcessNthTalkString(talkKeywordIndex * 2 + 6) != 0)
                {
                    return 1;
                }
                PrintQuoteMark();
                PrintCr();
                PrintCr();
            }
        }
    }
}

// Look for the typed text among the answers of label talkLabel (keyword and
// reply pairs after its question and its default answer). 1 when found: the reply follows.
static int FindLabelAnswer(void)
{
    int matchPos;

    talkPtr = scratch;
    while (*talkPtr != talkLabel)
    {
        SkipOneTalkString(0x90, 0x9f);
    }

    SkipOneTalkString(0, 0x9f);
    SkipOneTalkString(0, 0x9f);

    while (1)
    {
        matchPos = CompareWord(talkPtr, inputText);
        if (matchPos != -1 && (matchPos == 0 || inputText[matchPos - 1] == ' '))
        {
            return 1;
        }

        if (SkipOneTalkString(0, 0x90) == 0)
        {
            return 0;
        }

        if (SkipOneTalkString(0, 0x90) == 0)
        {
            return 0;
        }
    }
}

// CHECKED
// process label?
// A label: print its question, read the reply, print the matching answer or the default,
// then back to "Your interest?".
static int AskLabelQuestion(void)
{
    int result;
    int matchPos_a;
    byte wordIndex_k;

    while (1)
    {
        PrintQuoteMark();
        if (ProcessLabelQuestion() != 0)
        {
            return 1;
        }

        PrintQuoteMark();

        do
        {
            PrintCr();
            PrintCr();
            talkInLabel = 0xff;
            PrintString(/*0x9440*/ "You respond-\n:");
            TalkGetString();
            if (inputText[0] == 0)
            {
                PrintString(/*0x9450*/ "\n\n\"What didst thou say?");
            }
        } while (inputText[0] == 0);

        PrintCr();
        PrintCr();
        for (wordIndex_k = 0, result = -1; wordIndex_k < 0x22; wordIndex_k++)
        {
            matchPos_a = CompareWord(standardWords[wordIndex_k], inputText);
            if (matchPos_a != -1 && (matchPos_a == 0 || inputText[matchPos_a - 1] == ' '))
            {
                result = AnswerStandardWord(wordIndex_k);
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
        if (FindLabelAnswer() == 0)
        {
            PrintQuoteMark();
            if (ProcessLabelDefault() != 0)
            {
                return 1;
            }
            PrintQuoteMark();
            PrintCr();
            PrintCr();
            return AskInterest();
        }
        else
        {
            PrintQuoteMark();
            if (ProcessNextString() != 0)
            {
                return 1;
            }
            PrintQuoteMark();
            PrintCr();
            PrintCr();
            return AskInterest();
        }
    }
}

// NOT MATCHING (u32 operation)
// set npc met flag
static int SetNpcMet(int npcIndex)
{
    u32 mask;

    mask = 1;
    mask <<= npcIndex; // aNNalshl
    (*(u32*)&npcMet[(mapId - 1) * 4]) |= mask;
}

// NOT MATCHING (u32 operation)
// check npc met flag
static int IsNpcMet(int npcIndex)
{
    u32 mask;

    mask = 1;
    mask <<= npcIndex; // aNNalshl
    return (*(u32*)&npcMet[(mapId - 1) * 4] & mask) != 0;
}

// The bytes after a script code that takes arguments (talkPendingCode): 0x85 gold,
// three digits; 0x86 an item; 0x8c a label to go to when the NPC knows the Avatar's name
// (0xff: just go on); 0xfe a karma value and a label to go to when the karma is at least that.
static int ProcessCodeArgument(byte value)
{
    switch (talkPendingCode)
    {
    default:
        // 0dda
        talkCodeArgCount = talkPendingCode = 0;
        // 0dfc ->
        // 0e74
        break;

    case 0x85:
        // 0de4
        talkCodeArgs[talkCodeArgCount++] = value;
        if (talkCodeArgCount != 3)
        {
            // 0dfc
            return 0;
        }

        // 0e02
        if (PayGold() != 0)
        {
            // -> 0e74
            return 1;
        }

        // 0dda
        talkCodeArgCount = talkPendingCode = 0;
        break;

    case 0x86:
        // 0e0e
        ReceiveItem(value & 0x7f);

        // 0dda
        talkCodeArgCount = talkPendingCode = 0;
        break;

    case 0x8c:
        // 0e1c
        talkCodeArgCount = talkPendingCode = 0;

        if (IsNpcMet(talkNpcIndex) == 0)
        {
            // 0dfc
            return 0;
        }

        if (value == 0xff)
        {
            return AskInterest();
        }

        // 0e3a
        talkLabel = value;

        // 0e3d
        return AskLabelQuestion();

    case 0xfe:
        // 0e46
        talkCodeArgs[talkCodeArgCount++] = value;
        if (talkCodeArgCount != 2)
        {
            // 0dfc
            return 0;
        }

        talkCodeArgCount = talkPendingCode = 0;

        if (karma < talkCodeArgs[0])
        {
            // 0dfc
            return 0;
        }

        talkLabel = talkCodeArgs[1];

        // 0e3d
        return AskLabelQuestion();
    }

    return 0;
}

// CHECKED
static void AskName(void)
{
    int matchPos_i;
    int i;
    int member;
    char name[5];

    member = 0;
    PrintQuoteMark();
    PrintWord();
    PrintString(/*0x9468*/ "What is thy name?\"\n");
    PrintString(/*0x947c*/ "\nYou respond-\n:");
    TalkGetString();

    talkPendingCode = talkCodeArgCount = 0;

    if (inputText[0] == 0)
    {
        PrintString(/*0x948c*/ "\n\n\"If you say so...");
    }
    else
    {
        for (; member < partyCount; member++)
        {
            for (i = 0; i < 4; i++)
            {
                name[i] = party[member].name[i];
            }

            name[4] = 0;

            matchPos_i = CompareWord(name, inputText);
            if (matchPos_i != -1 && (matchPos_i == 0 || (inputText[matchPos_i - 1] == ' ')))
            {
                SetNpcMet(talkNpcIndex);
                PrintString(/*0x94a0*/ "\n\n\"A pleasure!");
                return;
            }
        }

        PrintString(/*0x94b0*/ "\n\n\"If you say so...");
    }
}

// Process one byte of an NPC's script: a script code (0x81-0x9f, 0xfe, 0xff), a word of
// the word table (below 0x81, wordTable) or a character. Returns 1 when the
// conversation is over. The .TLK format and the codes 0x81, 0x83, 0x87, 0x88, 0x8c, 0x8d,
// 0x8f and 0x91-0x9b are on the Ultima Codex, "Ultima V internal formats"; the other
// codes' meanings are read from this code.
static int ProcessChar(byte code)
{
    char* word;
    int tick;
    byte* savedPtr;

    if (talkPendingCode != 0)
    {
        return ProcessCodeArgument(code);
    }

    // 0f4c
    talkCodeArgCount = 0;
    if ((code == 0xa2) && (talkLastChar == 0xa2))
    {
        // 0f5e
        return 0;
    }

    // 0f64
    switch (code)
    {
    case 0x81: // avatar
        // 0f86
        PrintAvatarName();
        break;
    case 0x82: // end conversation
        // 0f8c
        return 1;
    case 0x83: // pause
        // 0f92
        for (tick = 0; tick < 0x1c; tick++)
        {
            UpdateFrame();
            if (PollKey() != 0)
                break;
            WaitTicks(1);
        }
        ClearKbdBuffer();
        return 0; // -> 0f5e
    case 0x84:    // join
        // 0fb6
        return Join(); // -> 1114
    case 0x87:                // key_or
        // 0fbc
        savedPtr = talkPtr;
        if (ProcessOrAnswer() != 0)
        {
            return 1;
        }
        talkPtr = savedPtr;
        return 0; // -> 0f5e
    case 0x88:    // ask name
        // 0fd2
        AskName();
        return 0; // -> 0f5e
    case 0x89:    // karma +1
        // 0fd8
        IncreaseByte(&karma, 1, 99);
        return 0; // -> 0f5e
    case 0x8a:    // karma -1
        // 0fea
        DecreaseByte(&karma, 1);
        return 0; // -> 0f5e
    case 0x8b:    // call guards
        // 0ff8
        RaiseAlarm();
        return 0; // -> 0f5e
    case 0x8e:    // rune
        // 0ffe
        talkCharMask ^= 0x80;
        return 0; // -> 0f5e
    case 0xff:    // no selection
        // 1006
        PrintWord();
        return AskInterest(); // -> 1114
    case 0x8f:                // wait
        // 1010
        GetChar();
        return 0; // -> 0f5e
    case 0xfe:
    case 0x85: // gold-
    case 0x86: // change item 65
    case 0x8c: // if else name
        // 1016
        talkPendingCode = code;
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
        talkLabel = code;
        return AskLabelQuestion(); // -> 1114
        // ----------
    default:
        // 10ce
        if (code < 0x81)
        {
            BuildOrPrintWord(0xa0); // 0x20

            word = wordTable[code - 1];
            while (*word != 0)
            {
                BuildOrPrintWord((byte)*word++ | 0x80);
            }

            if (*wordTable[code - 1] == 0)
            {
                BuildOrPrintWord(code);
                return 0; // -> 0f5e
            }

            talkAfterWord = 1;
        }
        else
        {
            // 1062
            code |= 0x80;
            if (code == 0x8d) // 0xd newline
            {
                code = 0x8a;
            }

            talkLastChar = code &= talkCharMask;

            if (talkAfterWord != 0)
            {
                BuildOrPrintWord(0xa0); // 0x20
            }

            BuildOrPrintWord(code);
            talkAfterWord = 0;
        }
        break;
    }

    return 0;
}

// The start of a conversation: "You see" and the description, then the greeting when the
// NPC knows the Avatar's name, else (one time in two) "I am called" and the name.
static int Introduce(void)
{
    PrintString(/*0x94c4*/ "You see ");

    if (ProcessNthTalkString(TALK_KEY_DESC) != 0)
    {
        return 1;
    }
    else
    {
        PrintCr();
        PrintCr();
        if (IsNpcMet(talkNpcIndex) == 0)
        {
            SeedRandom(GetTime());
            if (RandomRange(0, 1) != 0)
            {
                // "I am called\nNAME"\n\n
                PrintString(/*0x94ce*/ "\"I am called ");
                if (ProcessNthTalkString(TALK_KEY_NAME) == 0)
                {
                    PrintQuoteMark();
                    PrintCr();
                    PrintCr();
                }
                else
                {
                    return 1;
                }
            }
        }
        else
        {
            PrintQuoteMark();
            if (ProcessNthTalkString(TALK_KEY_2) == 0)
            {
                PrintQuoteMark();
                PrintCr();
                PrintCr();
            }
            else
            {
                return 1;
            }
        }

        return 0;
    }
}

// After a conversation in a town where shadowlord 0 is (townShadowlord; Falsehood,
// if the Codex's order of the three is right): something is stolen. A key, a gem or a
// torch; when there are none a piece of equipment, a potion, a scroll, else gold.
static void FalsehoodSteals(void)
{
    int item;

    if (townShadowlord == 0)
    {
        PrintString(/*0x94dc*/ "\nSomething was stolen!\n");
        AudioSweepTone(800, 2000, 1, 0x32);
        SeedRandom(GetTime());

        if ((keys | gems | torches) != 0)
        {
            do
            {
                while (1)
                {
                    switch (RandomRange(0, 2))
                    {
                    case 0:
                        if (keys != 0)
                        {
                            DecreaseByte(&keys, 1);
                            return;
                        }
                        break;

                    case 1:
                        if (gems != 0)
                        {
                            DecreaseByte(&gems, 1);
                            return;
                        }
                        break;

                    case 2:
                        if (torches != 0)
                        {
                            DecreaseByte(&torches, 1);
                            return;
                        }
                        break;
                    default:
                        return;
                    }
                }
            } while (gems == 0);

            DecreaseByte(&gems, 1); // BUG?
        }
        else
        {
            for (item = 0x2f; item >= 0; item--)
            {
                if (equipment[item] != 0)
                {
                    DecreaseByte(&equipment[item], 1);
                    return;
                }
            }

            for (item = 7; item >= 0; item--)
            {
                if (potions[item] != 0)
                {
                    DecreaseByte(&potions[item], 1);
                    return;
                }
            }

            for (item = 7; item >= 0; item--)
            {
                if (scrolls[item] != 0)
                {
                    DecreaseByte(&scrolls[item], 1);
                    return;
                }
            }

            DecreaseInt(&gold, RandomRange(1, 0xf));
            UpdateVitalsDisplay();
        }
    }
}

// Read the NPC's script from the map's .TLK file into scratch and hold the conversation.
// The file starts with a count and that many (dialog number, offset) pairs (Ultima Codex, "Ultima V internal formats").
static void StartTalk(int dialogNumber)
{
    s16 count;
    s16 entry_g;
    s16 offset_b;
    s16* index;
    uint fileIndex;

    fileIndex = (mapId - 1) >> 3;
    ReadFileFromDisk(talkFileNames[fileIndex], scratch, 0x200, 0);

    // 12a9
    index = (s16*)scratch;
    count = *index++;

    // 12ad
    for (entry_g = 1; *index++ != dialogNumber && entry_g <= count; index++, entry_g++)
    {
    }

    // 12d6
    offset_b = *index;
    ReadFileFromDisk(talkFileNames[fileIndex], scratch, 0x400, offset_b);

    if (Introduce() == 0)
    {
        if (AskInterest() == 0)
        {
            SayBye();
        }
    }

    FalsehoodSteals();
}
