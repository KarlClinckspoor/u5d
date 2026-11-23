#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include "blckthrn.h"
#include "cast2.h"
#include "town.h"
u8 scriptAfterTest[0x3c] =
{
    0x13, 0x06, 0x44, 0x00, 0x04, 0x07, 0x11, 0x11,
    0x02, 0x02, 0x01, 0x13, 0x1f, 0x12, 0x11, 0x11,
    0x10, 0x01, 0x13, 0x1f, 0x01, 0x13, 0x1c, 0x01,
    0x10, 0x1c, 0x1c, 0x12, 0x06, 0xbb, 0x00, 0x04,
    0x08, 0x19, 0x02, 0x04, 0x01, 0x1a, 0x12, 0x1a,
    0x11, 0x09, 0x08, 0x02, 0x03, 0x11, 0x12, 0x09,
    0x06, 0x02, 0x03, 0x17, 0x16, 0x09, 0x07, 0x02,
    0x06, 0x08, 0x00, 0x00
};

u8 scriptThreat[0x27] =
{
    0x05, 0x16, 0x12, 0x02, 0x07, 0x11, 0x22, 0x02,
    0x02, 0x01, 0x13, 0x23, 0x22, 0x05, 0x03, 0x06,
    0x82, 0x05, 0x07, 0x09, 0x01, 0x07, 0x02, 0x05,
    0x13, 0x10, 0x05, 0x0c, 0x02, 0x03, 0x17, 0x06,
    0xe9, 0x05, 0x09, 0x02, 0x03, 0x15, 0x00//, 0x00 (gap 1?)
};

u8 scriptGuardsEnter[12] = {0x08, 0x01, 0x10, 0x14, 0x02, 0x03, 0x01, 0x13, 0x15, 0x05, 0x08, 0x00};

u8 scriptRelease[8] = {0x05, 0x0b, 0x13, 0x02, 0x04, 0x10, 0x11, 0x00};

u8 scriptBlackthornLeaves[10] = {0x05, 0x04, 0x19, 0x02, 0x05, 0x1a, 0x09, 0x08, 0x07, 0x00};

u16 fanfareFreq[6] = { 0x1130, 0x101d, 0x0e53, 0x0b75, 0x0ce4, 0x0ce4 };

u16 fanfareParam3[6] = { 0xc350, 0xc350, 0xc350, 0x7530, 0x9c40, 0x9c40 };

u16 fanfareParam4[6] = { 0x0bb8, 0x0bb8, 0x0bb8, 0x03e8, 0x0064, 0x9ca4 };

s16 fanfareParam5[6] = { 1, 1, 1, 1, 1, -1 };

// so many footsteps, two frames each
static void Footsteps(int count)
{
    int step;

    for (step = 0; step < count; step++)
    {
        AudioFootstep();
        WaitFrames(2);
    }
}

// a script move code: the actor it moves (0x10 -> 6, 0x14 -> 7, 0x18 -> 8, 0x1c -> 0, 0x20 -> 1); the step is left in D_5876/D_5878
static int MoveActorOf(int moveCode)
{
    int actorIndex;

    switch (moveCode & 0xfc)
    {
    case 0x10:
        actorIndex = 6;
        break;
    case 0x14:
        actorIndex = 7;
        break;
    case 0x18:
        actorIndex = 8;
        break;
    case 0x1c:
        actorIndex = 0;
        break;
    case 0x20:
        actorIndex = 1;
        break;
    }

    tempX = tempY = 0;

    switch (moveCode & 3)
    {
    case 0:
        tempY--;
        break;
    case 2:
        tempY++;
        break;
    case 1:
        tempX++;
        break;
    case 3:
        tempX--;
        break;
    }

    return actorIndex;
}

// CHECKED
// runs a cutscene script (codes above)
static void RunScript(byte* script)
{
    int stepBY;
    int stepAY;
    int stepBX;
    int stepAX;
    int actorB_i;
    int actorA_j;
    bool walkSound_k; // di
    bool pairMove;
    int stepNo_g;
    int repeatCount_e;
    int opcode_f;

    walkSound_k = 1;
    repeatCount_e = 1; // a35c
    pairMove = 0; // -> a4ce

    while ((opcode_f = *script++) != 0)
    {
        if (opcode_f < 0x10)
        {
            // a4ec
            switch (opcode_f)
            {
            case 1:
                // a360
                pairMove = 1;
                break;

            case 2:
                // a368
                repeatCount_e = *script;
                script++;
                break;

            case 3:
                // a378
                walkSound_k = 1;
                break;

            case 4:
                // a37e
                walkSound_k = 0;
                break;

            case 5:
                // a384
                WaitFrames(*script++);
                break;

            case 6:
                // a396
                actorA_j = *script++;
                stepAX = *script++;
                stepAY = *script++;
                GetCombatMap(stepAX, stepAY) = actorA_j;
                //script += 4;
                break;

            case 7:
                // a3cc
                UpdateFrame();
                break;

            case 8:
                // a3d2
                Footsteps(repeatCount_e);
                repeatCount_e = 1;
                break;

            case 9:
                // a3e0
                actorA_j = *script++;
                actors[actorA_j].tile = actors[actorA_j].animTile = 0;
                break;
            }

            continue;
        }

        // a41c
        actorA_j = MoveActorOf(opcode_f);
        stepAX = tempX;
        stepAY = tempY;
        if (pairMove)
        {
            actorB_i = MoveActorOf(*script++);
            stepBX = tempX;
            stepBY = tempY;
        }

        // a454 ->
        for (stepNo_g = 0; stepNo_g < repeatCount_e; stepNo_g++)
        {
            // a495
            actors[actorA_j].x += stepAX;
            actors[actorA_j].y += stepAY;
            if (pairMove)
            {
                actors[actorB_i].x += stepBX;
                actors[actorB_i].y += stepBY;
            }

            if (walkSound_k)
            {
                Footsteps(1);
            }
        }

        repeatCount_e = 1; // a4c9
        pairMove = 0; // a4ce
    }
}

// prints question 0-3 of the scene's text (D_b21e) and the virtue's name; question 3 has no virtue
static void AskVirtue(int question, int virtue)
{
    switch (question)
    {
    case 0:
        PrintString((char*)&scratch[0x0]);
        PrintString(virtueNames[virtue]);
        PrintString(/*0x6f6e*/ "?\"");
        break;
    case 1:
        PrintString((char*)&scratch[0x2d]);
        PrintString(virtueNames[virtue]);
        PrintString(/*0x6f72*/ "?\"");
        break;
    case 2:
        PrintString((char*)&scratch[0x52]);
        PrintString(virtueNames[virtue]);
        PrintString(/*0x6f76*/ "?\"");
        break;
    case 3:
        PrintString((char*)&scratch[0xac]);
        break;
    }
}

// reads "Your response?" and tells whether it contains the virtue's mantra (any case)
static int ReadMantraAnswer(int virtue)
{
    int mantraLen;
    int charNo_f;
    int startPos;
    int found_e;
    char answer[18];

    PrintString(/*0x6f7a*/ "\n\nYour response?\n:");
    GetString(answer, 0xe);
    PrintString(/*0x6f8e*/ "\n\n");
    mantraLen = StringLength(mantras[virtue]);
    found_e = 0;

    for (startPos = 0; startPos <= StringLength(answer) - mantraLen; startPos++)
    {
        found_e = 1;

        for (charNo_f = 0; charNo_f < mantraLen; charNo_f++)
        {
            if (ToUpperChar((byte)answer[startPos + charNo_f]) != ToUpperChar((byte)mantras[virtue][charNo_f]))
            {
                found_e = 0;
                break;
            }
        }

        if (found_e != 0)
        {
            break;
        }
    }

    return found_e;
}

// CHECKED
// a thunderbolt and a sound sweep, the actor at slot 1 vanishes; the second living member leaves the party (to slot 15)
static void SlicePartyMember(int withSpeech)
{
    int member;
    int aliveSeen_d;
    int counter;
    S_55a8 sliced_l;

    if (withSpeech != 0)
    {
        PrintString((char*)&scratch[0xf1]);
    }
    else
    {
        PrintString((char*)&scratch[0x12d]);
    }

    WaitFrames(10);

    for (counter = 2000; counter < 25000; counter += 0x32)
    {
        AudioPulse(0xa50, 1, 200, counter, 0);
    }

    for (counter = 25000; 2000 < counter; counter -= 0x32)
    {
        AudioPulse(0xa50, 1, 200, counter, 0);
    }

    HitEffectAt(actors[1].x, actors[1].y);

    actors[1].tile = actors[1].animTile = 0;
    GetCombatMap(5, 7) = 0x80;

    aliveSeen_d = 0;
    for (member = 0; member < partyCount; member++)
    {
        if (party[member].status != STATUS_DEAD)
        {
            aliveSeen_d++;
        }
        if (aliveSeen_d == 2)
            break;
    }

    // a6fc
    sliced_l = party[member];

    // NOT MATCHING
    for (counter = member; counter < 0xf; counter++)
    {
        // a73b
        party[counter] = party[counter + 1];
    }

    // a752
    party[0xf] = sliced_l;
    party[0xf].innMapId = 0x7f;

    partyCount--;
    UpdateVitalsDisplay();
    if (withSpeech != 0)
    {
        PrintString(/*0x6f92*/ "\n\n");
        PrintString(party[0xf].name);
        PrintString(/*0x6f96*/ " is sliced in half! ");
        GetChar();
        PrintString((char*)&scratch[0x1a0]);
    }

    PrintChar('\n');
}

// waits for a key, then runs the script D_369e
static void KeyThenScript(void)
{
    GetChar();
    RunScript(scriptAfterTest);
}

// the first threat: scene text, the script D_36da, then the second member is to die
static void ThreatenCompanion(void)
{
    PrintString((char*)&scratch[0x229]);
    RunScript(scriptThreat);
    PrintString((char*)&scratch[0x25f]);
    PrintString(party[1].name);
    PrintString(/*0x6fac*/ " die!\" ");
    GetChar();
    PrintString(/*0x6fb4*/ "\n\n");
}

// up to four rounds about the virtue: an answer with its mantra marks the shrine 0xff, costs 5 karma and ends it; a round may slice a member
static void Interrogation(int virtue, int aliveCount)
{
    bool threatened;
    int round_h;

    threatened = 0;
    PrintString(/*0x6fb8*/ "\n\n");

    for (round_h = 0; round_h < 4; round_h++)
    {
        AskVirtue(round_h, virtue);
        if (ReadMantraAnswer(virtue) != 0)
        {
            *(shrineDestroyed + virtue) = 0xff;
            DecreaseByte(&karma, 5);
            if (aliveCount > 1)
            {
                SlicePartyMember(0);
                KeyThenScript();
            }
            else
            {
                PrintString((char*)&scratch[0x29c]);
                KeyThenScript();
            }
            break;
        }

        if (aliveCount < 2)
        {
            PrintString((char*)&scratch[0x2dc]);
            KeyThenScript();
            break;
        }

        if (threatened)
        {
            AdvanceTime(2);
            UpdateVitalsDisplay();

            switch (round_h)
            {
            case 0:
                GetCombatMap(5, 9) = 0xea;
                break;
            case 1:
                GetCombatMap(5, 9) = 0xeb;
                break;
            case 2:
                GetCombatMap(5, 9) = 0xe8;
                break;
            case 3:
                SlicePartyMember(1);
                break;
            }
        }
        else
        {
            threatened = 1;
            ThreatenCompanion();
        }
    }
}

// the party is captured: the first intact shrine's virtue is the topic; guards, chains, Blackthorn's speech, the interrogation; then the party is set at the castle (10, 7)
void Capture(void)
{
    int unused_j;
    int col;
    int virtue_m;
    int aliveCount;
    int index;

    int classIndex;
    ActorFmt* actor_g;

    aliveCount = 0;
    for (index = 0; index != partyCount; index++)
    {
        if (party[index].status != STATUS_DEAD)
        {
            aliveCount = aliveCount + 1;
        }
    }

    partyTile = TILE_ACTOR_AVATAR;
    UpdateFrame();
    PrintString(/*0x6fbc*/ "\nThou art subdued and blindfolded!");

    for (virtue_m = 0; virtue_m < 8; virtue_m++)
    {
        if (shrineDestroyed[virtue_m] == 0)
            break;
    }

    if (virtue_m < 8)
    {
        unused_j = 0;

        WaitFrames(2);
        GRAP_2d_SetPenColor(0);
        GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
        D_5895_mapLevel = 0xff;
        DrawSunAndMoons();
        SetWindDirection(-1);

        for (index = 0; index < 5; index++)
        {
            WaitTicks(5);
            AudioFootstep();
        }

        PrintString(/*0x6fe0*/ "\n\nStrong guards drag thee away!");

        for (index = 0; index < (soundOn != 0 ? 0x12 : 3); index++)
        {
            WaitTicks(5);
            AudioFootstep();
        }

        for (index = 0; index < 0x20; index++)
        {
            SetActor(0, 0, 0, 0, 0, 0, index);
        }

        mapId = 0xff;
        curCombatant = 0xff;

        ReadFileFromDisk(/*0x7000*/ "MISCMAPS.DAT", viewActors, 0xb0, 0);
        ReadFileFromDisk(/*0x700e*/ "MISCMSG.DAT", scratch, 1000, 0);

        for (index = 0; index < 0xb; index++)
        {
            for (col = 0; col < 0xb; col++)
            {
                GetCombatMap(col, index) = viewActors[index * 0x10 + col];
            }
        }

        // a9ef
        for (index = 0; index < aliveCount; index++)
        {
            actor_g = &actors[index];
            actor_g->x = prisonCellX[prisonCellOf[(aliveCount - 1) * 8 + index]];
            actor_g->y = prisonCellY[prisonCellOf[(aliveCount - 1) * 8 + index]];

            classIndex = FindIndex(/*0x701a*/ CLASS_LIST, party[index].cls);
            actor_g->tile = actor_g->animTile = classTile[classIndex];
        }

        // aa61
        WaitFrames(0x10);
        PrintString(/*0x7024*/ "\n\nThou hast been chained and manacled!");
        WaitFrames(0x32);
        PrintString(/*0x704c*/ "\n\nFootsteps!");
        Footsteps(8);
        SetActor(TILE_ACTOR_GUARD, TILE_ACTOR_GUARD, 4, 10, 0, 0, 6);
        SetActor(TILE_ACTOR_GUARD, TILE_ACTOR_GUARD, 6, 10, 0, 0, 7);
        RunScript(scriptGuardsEnter);
        AudioPulse(0xaf0, 1, 13000, 100, 5);
        SetActor(TILE_ACTOR_CIRCLE, TILE_ACTOR_CIRCLE, 5, 5, 0, 0, 8);
        ULTIMA_1068_RevealTile(0x178, 5, 5);
        SetActor(TILE_ACTOR_BLACKTHORN, TILE_ACTOR_BLACKTHORN, 5, 5, 0, 0, 8);
        WaitFrames(8);
        PrintString(/*0x705a*/ "\n\nBlackthorn says:\n\n\"Ah, ");
        PrintString(party[0].name);
        PrintString(/*0x7074*/ "!\n'Tis indeed an honour to meet thee at last! ");
        GetChar();
        PrintString(/*0x70a4*/ "\n\nGUARD! Release this good");
        if (party[0].gender == 12)
        {
            PrintString(/*0x70c0*/ " lady ");
        }
        else if (party[0].gender == 11)
        {
            PrintString(/*0x70c8*/ "man ");
        }
        PrintString(/*0x70ce*/ "at once!\"");
        RunScript(scriptRelease);
        PrintString((char*)&scratch[0x32c]);
        GetChar();
        Interrogation(virtue_m, aliveCount);
        if (actors[8].tile != 0)
        {
            RunScript(scriptBlackthornLeaves);
        }
    }

    D_5895_mapLevel = 0xff;
    DrawSunAndMoons();
    SetWindDirection(-1);
    D_5896_mapX = 10;
    D_5897_mapY = 7;
    keys = 0;
    mapId = 0x12;
}

// CHECKED
// the death scene: darkness, "Thou hast found refuge", the old man speaks by karma, the party is resurrected and put at (10, 10) of map 0x11
void Death(void)
{
    int savedMapId;
    int pitch_n;
    uint index;
    int col;
    int karmaIndex;

    SwitchDisks(1);

    while (TestOpenFile(/*0x70d8*/ "BRIT.DAT") == 0) {}

    SetLightTurns(lightSpellTurns);
    partyTile = TILE_ACTOR_SLEEP;

    if (mapId < 0x21)
    {
        UpdateFrame();
    }

    WaitTicks(10);
    savedMapId = mapId;
    mapId = 0xff;
    curCombatant = 0xff;
    PrintString(/*0x70e2*/ "\nAn unending darkness engulfs thee...");
    GRAP_0f_SelectPage(1);
    GRAP_2d_SetPenColor(0);
    GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    GRAP_0f_SelectPage(0);
    GRAP_66_Reveal(8, 8, 0xb7, 0xb7);

    for (index = 0; index < 0xb; index++)
    {
        for (col = 0; col < 0xb; col++)
        {
            GetCombatMap(col, index) = 0xff;
        }
    }

    for (index = 0; index < 0x20; index++)
    {
        SetActor(0, 0, 0, 0, 0, 0, index);
    }

    UpdateFrame();

    PrintString(/*0x7108*/ "\n\nThou hast found refuge.");
    WaitTicks(0xe);
    PrintString(/*0x7122*/ "\n\nNo evil lives here, only peace and darkness.");
    WaitTicks(0x1c);
    PrintString(/*0x7152*/ "\n\nBut thy slumber is disturbed!");

    GetCombatMap(5, 5) = 0; // 0xadb9

    actors[0].tile = actors[0].animTile = TILE_ACTOR_AVATAR;
    actors[0].x = actors[0].y = 5;

    UpdateFrame();

    // ac9d: NOT MATCHING
    for (index = 0; index < 6; index++)
    {
        AudioPulse(fanfareFreq[index], 1, fanfareParam3[index], fanfareParam4[index], fanfareParam5[index]);
    }

    PrintString(/*0x7172*/ "\n\nSomeone shouts\n\n\"FORTIS FORTUNA\nAVENTARI\"");
    WaitTicks(6);

    actors[1].tile = actors[1].animTile = TILE_ACTOR_CIRCLE;
    actors[1].x = 2;
    actors[1].y = 7;
    GetCombatMap(2, 7) = 0;
    ULTIMA_1068_RevealTile(0x5e, 2, 7);
    GetCombatMap(2, 7) = TILE_MAP_5E;
    actors[1].tile = 0;
    UpdateFrame();

    WaitTicks(4);
    actors[1].tile = TILE_ACTOR_CIRCLE;
    actors[1].x = 8;
    GetCombatMap(8, 7) = 0;
    ULTIMA_1068_RevealTile(0x5f, 8, 7);
    GetCombatMap(8, 7) = TILE_MAP_5F;
    actors[1].tile = 0;
    UpdateFrame();

    WaitTicks(4);
    PrintString(/*0x719e*/ "\n\nThere is a peal of thunder!\n");
    ShakeScreen();
    ShakeScreen();
    GetCombatMap(5, 2) = 0;
    actors[1].tile = actors[1].animTile = TILE_ACTOR_CIRCLE;
    actors[1].x = 5;
    actors[1].y = 2;
    ULTIMA_1068_RevealTile(0x174, 5, 2);
    actors[1].tile = actors[1].animTile = TILE_ACTOR_74;
    UpdateFrame();

    // ad93
    karmaIndex = karma / 0x14;
    PrintString(/*0x71be*/ "\n\"");
    ReadFileFromDisk(/*0x71c2*/ "KARMA.DAT", scratch, 2000, karmaTextOffsets[karmaIndex]);
    PrintString((char*)scratch);
    PrintChar('\"');
    GetChar();
    PrintString(/*0x71cc*/ "\n\nStrange words are intoned.");
    WaitTicks(4);

    for (index = 0; index < partyCount; index++)
    {
        pitch_n = (u32)36400 / (index + 7);
        AudioPulse(pitch_n, 1, 30000, 2000, 2);
        Resurrect(index, 0xff);
        party[index].hp = party[index].maxHp;
        UpdateVitalsDisplay();
    }

    PrintString(/*0x71ea*/ "\n\nVertigo...\n");
    WaitTicks(4);
    GRAP_0f_SelectPage(1);
    GRAP_2d_SetPenColor(0);
    GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    GRAP_51_PutTile(0x11c, 5, 5);
    GRAP_0f_SelectPage(0);
    GRAP_66_Reveal(8, 8, 0xb7, 0xb7);

    if (karma < 0x4b)
    {
        karma = 0x4b;
    }

    mapId = 0x11;
    D_5895_mapLevel = 1;
    partyTile = TILE_ACTOR_AVATAR;

    D_5896_mapX = D_5897_mapY = 10;
    statusIcon = protectTurns = 0;

    while (D_587f_hour != 0x6)
    {
        AdvanceTime(9);
    }

    torchTurns = lightSpellTurns = 0;

    if (food == 0)
    {
        food = 0x3f;
    }

    if (savedMapId != 0 && savedMapId < 0x21)
    {
        TownEntry(1);
    }
}
