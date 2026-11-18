#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include "audio/aud_mus.h"

#include "blckthrn.h"
#include "cast2.h"
#include "town.h"
u8 D_369e_scriptAfterTest[0x3c] =
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

u8 D_36da_scriptThreat[0x27] =
{
    0x05, 0x16, 0x12, 0x02, 0x07, 0x11, 0x22, 0x02,
    0x02, 0x01, 0x13, 0x23, 0x22, 0x05, 0x03, 0x06,
    0x82, 0x05, 0x07, 0x09, 0x01, 0x07, 0x02, 0x05,
    0x13, 0x10, 0x05, 0x0c, 0x02, 0x03, 0x17, 0x06,
    0xe9, 0x05, 0x09, 0x02, 0x03, 0x15, 0x00//, 0x00 (gap 1?)
};

u8 D_3702_scriptGuardsEnter[12] = {0x08, 0x01, 0x10, 0x14, 0x02, 0x03, 0x01, 0x13, 0x15, 0x05, 0x08, 0x00};

u8 D_370e_scriptRelease[8] = {0x05, 0x0b, 0x13, 0x02, 0x04, 0x10, 0x11, 0x00};

u8 D_3716_scriptBlackthornLeaves[10] = {0x05, 0x04, 0x19, 0x02, 0x05, 0x1a, 0x09, 0x08, 0x07, 0x00};

u16 D_3720_fanfareFreq[6] = { 0x1130, 0x101d, 0x0e53, 0x0b75, 0x0ce4, 0x0ce4 };

u16 D_372c_fanfareParam3[6] = { 0xc350, 0xc350, 0xc350, 0x7530, 0x9c40, 0x9c40 };

u16 D_3738_fanfareParam4[6] = { 0x0bb8, 0x0bb8, 0x0bb8, 0x03e8, 0x0064, 0x9ca4 };

s16 D_3744_fanfareParam5[6] = { 1, 1, 1, 1, 1, -1 };

// so many footsteps, two frames each
static void BLCKTHRN_0000_Footsteps(int count)
{
    int step;

    for (step = 0; step < count; step++)
    {
        ULTIMA_433e_AudioFootstep();
        ULTIMA_3ae6_WaitFrames(2);
    }
}

// a script move code: the actor it moves (0x10 -> 6, 0x14 -> 7, 0x18 -> 8, 0x1c -> 0, 0x20 -> 1); the step is left in D_5876/D_5878
static int BLCKTHRN_002e_MoveActorOf(int moveCode)
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

    D_5876_tempX = D_5878_tempY = 0;

    switch (moveCode & 3)
    {
    case 0:
        D_5878_tempY--;
        break;
    case 2:
        D_5878_tempY++;
        break;
    case 1:
        D_5876_tempX++;
        break;
    case 3:
        D_5876_tempX--;
        break;
    }


    return actorIndex;
}

// CHECKED
// runs a cutscene script (codes above)
static void BLCKTHRN_00be_RunScript(byte* script)
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
                ULTIMA_3ae6_WaitFrames(*script++);
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
                ULTIMA_5910_UpdateFrame();
                break;

            case 8:
                // a3d2
                BLCKTHRN_0000_Footsteps(repeatCount_e);
                repeatCount_e = 1;
                break;

            case 9:
                // a3e0
                actorA_j = *script++;
                D_5c5a_actors[actorA_j].tile = D_5c5a_actors[actorA_j].animTile = 0;
                break;
            }

            continue;
        }

        // a41c
        actorA_j = BLCKTHRN_002e_MoveActorOf(opcode_f);
        stepAX = D_5876_tempX;
        stepAY = D_5878_tempY;
        if (pairMove)
        {
            actorB_i = BLCKTHRN_002e_MoveActorOf(*script++);
            stepBX = D_5876_tempX;
            stepBY = D_5878_tempY;
        }

        // a454 ->
        for (stepNo_g = 0; stepNo_g < repeatCount_e; stepNo_g++)
        {
            // a495
            D_5c5a_actors[actorA_j].x += stepAX;
            D_5c5a_actors[actorA_j].y += stepAY;
            if (pairMove)
            {
                D_5c5a_actors[actorB_i].x += stepBX;
                D_5c5a_actors[actorB_i].y += stepBY;
            }

            if (walkSound_k)
            {
                BLCKTHRN_0000_Footsteps(1);
            }
        }

        repeatCount_e = 1; // a4c9
        pairMove = 0; // a4ce
    }
}

// prints question 0-3 of the scene's text (D_b21e) and the virtue's name; question 3 has no virtue
static void BLCKTHRN_0278_AskVirtue(int question, int virtue)
{
    switch (question)
    {
    case 0:
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x0]);
        ULTIMA_1850_PrintString(D_1f4e_virtueNames[virtue]);
        ULTIMA_1850_PrintString(/*0x6f6e*/ "?\"");
        break;
    case 1:
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x2d]);
        ULTIMA_1850_PrintString(D_1f4e_virtueNames[virtue]);
        ULTIMA_1850_PrintString(/*0x6f72*/ "?\"");
        break;
    case 2:
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x52]);
        ULTIMA_1850_PrintString(D_1f4e_virtueNames[virtue]);
        ULTIMA_1850_PrintString(/*0x6f76*/ "?\"");
        break;
    case 3:
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0xac]);
        break;
    }
}

// reads "Your response?" and tells whether it contains the virtue's mantra (any case)
static int BLCKTHRN_02ea_ReadMantraAnswer(int virtue)
{
    int mantraLen;
    int charNo_f;
    int startPos;
    int found_e;
    char answer[18];

    ULTIMA_1850_PrintString(/*0x6f7a*/ "\n\nYour response?\n:");
    ULTIMA_3b1c_GetString(answer, 0xe);
    ULTIMA_1850_PrintString(/*0x6f8e*/ "\n\n");
    mantraLen = ULTIMA_216c_strlen(D_1f5e_mantras[virtue]);
    found_e = 0;

    for (startPos = 0; startPos <= ULTIMA_216c_strlen(answer) - mantraLen; startPos++)
    {
        found_e = 1;

        for (charNo_f = 0; charNo_f < mantraLen; charNo_f++)
        {
            if (ULTIMA_2032_ToUpper((byte)answer[startPos + charNo_f]) != ULTIMA_2032_ToUpper((byte)D_1f5e_mantras[virtue][charNo_f]))
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
static void BLCKTHRN_03ae_SlicePartyMember(int withSpeech)
{
    int member;
    int aliveSeen_d;
    int counter;
    S_55a8 sliced_l;

    if (withSpeech != 0)
    {
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0xf1]);
    }
    else
    {
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x12d]);
    }

    ULTIMA_3ae6_WaitFrames(10);

    for (counter = 2000; counter < 25000; counter += 0x32)
    {
        ULTIMA_2192_AudioPulse(0xa50, 1, 200, counter, 0);
    }

    for (counter = 25000; 2000 < counter; counter -= 0x32)
    {
        ULTIMA_2192_AudioPulse(0xa50, 1, 200, counter, 0);
    }

    ULTIMA_3522_HitEffectAt(D_5c5a_actors[1].x, D_5c5a_actors[1].y);

    D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = 0;
    GetCombatMap(5, 7) = 0x80;

    aliveSeen_d = 0;
    for (member = 0; member < D_585b_partyCount; member++)
    {
        if (D_55a8_party[member].status != STATUS_DEAD)
        {
            aliveSeen_d++;
        }
        if (aliveSeen_d == 2)
            break;
    }

    // a6fc
    sliced_l = D_55a8_party[member];

    // NOT MATCHING
    for (counter = member; counter < 0xf; counter++)
    {
        // a73b
        D_55a8_party[counter] = D_55a8_party[counter + 1];
    }

    // a752
    D_55a8_party[0xf] = sliced_l;
    D_55a8_party[0xf].innMapId = 0x7f;

    D_585b_partyCount--;
    ULTIMA_2900_UpdateVitalsDisplay();
    if (withSpeech != 0)
    {
        ULTIMA_1850_PrintString(/*0x6f92*/ "\n\n");
        ULTIMA_1850_PrintString(D_55a8_party[0xf].name);
        ULTIMA_1850_PrintString(/*0x6f96*/ " is sliced in half! ");
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x1a0]);
    }

    ULTIMA_16ba_PrintChar('\n');
}

// waits for a key, then runs the script D_369e
static void BLCKTHRN_0510_KeyThenScript(void)
{
    ULTIMA_266c_GetChar();
    BLCKTHRN_00be_RunScript(D_369e_scriptAfterTest);
}

// the first threat: scene text, the script D_36da, then the second member is to die
static void BLCKTHRN_051c_ThreatenCompanion(void)
{
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x229]);
    BLCKTHRN_00be_RunScript(D_36da_scriptThreat);
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x25f]);
    ULTIMA_1850_PrintString(D_55a8_party[1].name);
    ULTIMA_1850_PrintString(/*0x6fac*/ " die!\" ");
    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString(/*0x6fb4*/ "\n\n");
}

// up to four rounds about the virtue: an answer with its mantra marks the shrine 0xff, costs 5 karma and ends it; a round may slice a member
static void BLCKTHRN_054a_Interrogation(int virtue, int aliveCount)
{
    bool threatened;
    int round_h;

    threatened = 0;
    ULTIMA_1850_PrintString(/*0x6fb8*/ "\n\n");

    for (round_h = 0; round_h < 4; round_h++)
    {
        BLCKTHRN_0278_AskVirtue(round_h, virtue);
        if (BLCKTHRN_02ea_ReadMantraAnswer(virtue) != 0)
        {
            *(D_58d8_shrineDestroyed + virtue) = 0xff;
            ULTIMA_3f36_DecreaseByte(&D_5888_karma, 5);
            if (aliveCount > 1)
            {
                BLCKTHRN_03ae_SlicePartyMember(0);
                BLCKTHRN_0510_KeyThenScript();
            }
            else
            {
                ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x29c]);
                BLCKTHRN_0510_KeyThenScript();
            }
            break;
        }

        if (aliveCount < 2)
        {
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x2dc]);
            BLCKTHRN_0510_KeyThenScript();
            break;
        }

        if (threatened)
        {
            ULTIMA_4f7c_AdvanceTime(2);
            ULTIMA_2900_UpdateVitalsDisplay();

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
                BLCKTHRN_03ae_SlicePartyMember(1);
                break;
            }
        }
        else
        {
            threatened = 1;
            BLCKTHRN_051c_ThreatenCompanion();
        }
    }
}

// the party is captured: the first intact shrine's virtue is the topic; guards, chains, Blackthorn's speech, the interrogation; then the party is set at the castle (10, 7)
void BLCKTHRN_060e_Capture(void)
{
    int unused_j;
    int col;
    int virtue_m;
    int aliveCount;
    int index;

    int classIndex;
    ActorFmt* actor_g;

    aliveCount = 0;
    for (index = 0; index != D_585b_partyCount; index++)
    {
        if (D_55a8_party[index].status != STATUS_DEAD)
        {
            aliveCount = aliveCount + 1;
        }
    }

    D_587c_partyTile = TILE_ACTOR_AVATAR;
    ULTIMA_5910_UpdateFrame();
    ULTIMA_1850_PrintString(/*0x6fbc*/ "\nThou art subdued and blindfolded!");

    for (virtue_m = 0; virtue_m < 8; virtue_m++)
    {
        if (D_58d8_shrineDestroyed[virtue_m] == 0)
            break;
    }

    if (virtue_m < 8)
    {
        unused_j = 0;

        ULTIMA_3ae6_WaitFrames(2);
        ULTIMA_0a70_GRAP_2d_SetPenColor(0);
        ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
        D_5895_mapLevel = 0xff;
        ULTIMA_4a84_DrawSunAndMoons();
        ULTIMA_2e96_SetWindDirection(-1);

        for (index = 0; index < 5; index++)
        {
            ULTIMA_20fa_WaitTicks(5);
            ULTIMA_433e_AudioFootstep();
        }

        ULTIMA_1850_PrintString(/*0x6fe0*/ "\n\nStrong guards drag thee away!");

        for (index = 0; index < (D_a9ce_soundOn != 0 ? 0x12 : 3); index++)
        {
            ULTIMA_20fa_WaitTicks(5);
            ULTIMA_433e_AudioFootstep();
        }

        for (index = 0; index < 0x20; index++)
        {
            ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, index);
        }

        D_5893_mapId = 0xff;
        D_589e_curCombatant = 0xff;

        ULTIMA_256e_ReadFileFromDisk(/*0x7000*/ "MISCMAPS.DAT", D_ac64_viewActors, 0xb0, 0);
        ULTIMA_256e_ReadFileFromDisk(/*0x700e*/ "MISCMSG.DAT", D_b21e_scratch, 1000, 0);

        for (index = 0; index < 0xb; index++)
        {
            for (col = 0; col < 0xb; col++)
            {
                GetCombatMap(col, index) = D_ac64_viewActors[index * 0x10 + col];
            }
        }

        // a9ef
        for (index = 0; index < aliveCount; index++)
        {
            actor_g = &D_5c5a_actors[index];
            actor_g->x = D_1f42_prisonCellX[D_1f12_prisonCellOf[(aliveCount - 1) * 8 + index]];
            actor_g->y = D_1f48_prisonCellY[D_1f12_prisonCellOf[(aliveCount - 1) * 8 + index]];

            classIndex = ULTIMA_4d76_FindIndex(/*0x701a*/ CLASS_LIST, D_55a8_party[index].cls);
            actor_g->tile = actor_g->animTile = D_1ade_classTile[classIndex];
        }

        // aa61
        ULTIMA_3ae6_WaitFrames(0x10);
        ULTIMA_1850_PrintString(/*0x7024*/ "\n\nThou hast been chained and manacled!");
        ULTIMA_3ae6_WaitFrames(0x32);
        ULTIMA_1850_PrintString(/*0x704c*/ "\n\nFootsteps!");
        BLCKTHRN_0000_Footsteps(8);
        ULTIMA_3a74_SetActor(TILE_ACTOR_GUARD, TILE_ACTOR_GUARD, 4, 10, 0, 0, 6);
        ULTIMA_3a74_SetActor(TILE_ACTOR_GUARD, TILE_ACTOR_GUARD, 6, 10, 0, 0, 7);
        BLCKTHRN_00be_RunScript(D_3702_scriptGuardsEnter);
        ULTIMA_2192_AudioPulse(0xaf0, 1, 13000, 100, 5);
        ULTIMA_3a74_SetActor(TILE_ACTOR_CIRCLE, TILE_ACTOR_CIRCLE, 5, 5, 0, 0, 8);
        ULTIMA_1068_RevealTile(0x178, 5, 5);
        ULTIMA_3a74_SetActor(TILE_ACTOR_BLACKTHORN, TILE_ACTOR_BLACKTHORN, 5, 5, 0, 0, 8);
        ULTIMA_3ae6_WaitFrames(8);
        ULTIMA_1850_PrintString(/*0x705a*/ "\n\nBlackthorn says:\n\n\"Ah, ");
        ULTIMA_1850_PrintString(D_55a8_party[0].name);
        ULTIMA_1850_PrintString(/*0x7074*/ "!\n'Tis indeed an honour to meet thee at last! ");
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString(/*0x70a4*/ "\n\nGUARD! Release this good");
        if (D_55a8_party[0].gender == 12)
        {
            ULTIMA_1850_PrintString(/*0x70c0*/ " lady ");
        }
        else if (D_55a8_party[0].gender == 11)
        {
            ULTIMA_1850_PrintString(/*0x70c8*/ "man ");
        }
        ULTIMA_1850_PrintString(/*0x70ce*/ "at once!\"");
        BLCKTHRN_00be_RunScript(D_370e_scriptRelease);
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x32c]);
        ULTIMA_266c_GetChar();
        BLCKTHRN_054a_Interrogation(virtue_m, aliveCount);
        if (D_5c5a_actors[8].tile != 0)
        {
            BLCKTHRN_00be_RunScript(D_3716_scriptBlackthornLeaves);
        }
    }

    D_5895_mapLevel = 0xff;
    ULTIMA_4a84_DrawSunAndMoons();
    ULTIMA_2e96_SetWindDirection(-1);
    D_5896_mapX = 10;
    D_5897_mapY = 7;
    D_57ac_keys = 0;
    D_5893_mapId = 0x12;
}

// CHECKED
// the death scene: darkness, "Thou hast found refuge", the old man speaks by karma, the party is resurrected and put at (10, 10) of map 0x11
void BLCKTHRN_0910_Death(void)
{
    int savedMapId;
    int pitch_n;
    uint index;
    int col;
    int karmaIndex;

    ULTIMA_251e_SwitchDisks(1);

    while (ULTIMA_1674_TestOpenFile(/*0x70d8*/ "BRIT.DAT") == 0) {}

    CAST2_08ea_SetLightTurns(D_58a6_lightSpellTurns);
    D_587c_partyTile = TILE_ACTOR_SLEEP;

    if (D_5893_mapId < 0x21)
    {
        ULTIMA_5910_UpdateFrame();
    }

    ULTIMA_20fa_WaitTicks(10);
    savedMapId = D_5893_mapId;
    D_5893_mapId = 0xff;
    D_589e_curCombatant = 0xff;
    ULTIMA_1850_PrintString(/*0x70e2*/ "\nAn unending darkness engulfs thee...");
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_0f46_GRAP_66_Reveal(8, 8, 0xb7, 0xb7);

    for (index = 0; index < 0xb; index++)
    {
        for (col = 0; col < 0xb; col++)
        {
            GetCombatMap(col, index) = 0xff;
        }
    }

    for (index = 0; index < 0x20; index++)
    {
        ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, index);
    }

    ULTIMA_5910_UpdateFrame();

    ULTIMA_1850_PrintString(/*0x7108*/ "\n\nThou hast found refuge.");
    ULTIMA_20fa_WaitTicks(0xe);
    ULTIMA_1850_PrintString(/*0x7122*/ "\n\nNo evil lives here, only peace and darkness.");
    ULTIMA_20fa_WaitTicks(0x1c);
    ULTIMA_1850_PrintString(/*0x7152*/ "\n\nBut thy slumber is disturbed!");

    GetCombatMap(5, 5) = 0; // 0xadb9

    D_5c5a_actors[0].tile = D_5c5a_actors[0].animTile = TILE_ACTOR_AVATAR;
    D_5c5a_actors[0].x = D_5c5a_actors[0].y = 5;

    ULTIMA_5910_UpdateFrame();

    // ac9d: NOT MATCHING
    for (index = 0; index < 6; index++)
    {
        ULTIMA_2192_AudioPulse(D_3720_fanfareFreq[index], 1, D_372c_fanfareParam3[index], D_3738_fanfareParam4[index], D_3744_fanfareParam5[index]);
    }

    ULTIMA_1850_PrintString(/*0x7172*/ "\n\nSomeone shouts\n\n\"FORTIS FORTUNA\nAVENTARI\"");
    ULTIMA_20fa_WaitTicks(6);

    D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = TILE_ACTOR_CIRCLE;
    D_5c5a_actors[1].x = 2;
    D_5c5a_actors[1].y = 7;
    GetCombatMap(2, 7) = 0;
    ULTIMA_1068_RevealTile(0x5e, 2, 7);
    GetCombatMap(2, 7) = TILE_MAP_5E;
    D_5c5a_actors[1].tile = 0;
    ULTIMA_5910_UpdateFrame();

    ULTIMA_20fa_WaitTicks(4);
    D_5c5a_actors[1].tile = TILE_ACTOR_CIRCLE;
    D_5c5a_actors[1].x = 8;
    GetCombatMap(8, 7) = 0;
    ULTIMA_1068_RevealTile(0x5f, 8, 7);
    GetCombatMap(8, 7) = TILE_MAP_5F;
    D_5c5a_actors[1].tile = 0;
    ULTIMA_5910_UpdateFrame();

    ULTIMA_20fa_WaitTicks(4);
    ULTIMA_1850_PrintString(/*0x719e*/ "\n\nThere is a peal of thunder!\n");
    ULTIMA_3072_ShakeScreen();
    ULTIMA_3072_ShakeScreen();
    GetCombatMap(5, 2) = 0;
    D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = TILE_ACTOR_CIRCLE;
    D_5c5a_actors[1].x = 5;
    D_5c5a_actors[1].y = 2;
    ULTIMA_1068_RevealTile(0x174, 5, 2);
    D_5c5a_actors[1].tile = D_5c5a_actors[1].animTile = TILE_ACTOR_74;
    ULTIMA_5910_UpdateFrame();

    // ad93
    karmaIndex = D_5888_karma / 0x14;
    ULTIMA_1850_PrintString(/*0x71be*/ "\n\"");
    ULTIMA_256e_ReadFileFromDisk(/*0x71c2*/ "KARMA.DAT", D_b21e_scratch, 2000, D_1a74_karmaTextOffsets[karmaIndex]);
    ULTIMA_1850_PrintString((char*)D_b21e_scratch);
    ULTIMA_16ba_PrintChar('\"');
    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString(/*0x71cc*/ "\n\nStrange words are intoned.");
    ULTIMA_20fa_WaitTicks(4);

    for (index = 0; index < D_585b_partyCount; index++)
    {
        pitch_n = (u32)36400 / (index + 7);
        ULTIMA_2192_AudioPulse(pitch_n, 1, 30000, 2000, 2);
        CAST2_05e0_Resurrect(index, 0xff);
        D_55a8_party[index].hp = D_55a8_party[index].maxHp;
        ULTIMA_2900_UpdateVitalsDisplay();
    }

    ULTIMA_1850_PrintString(/*0x71ea*/ "\n\nVertigo...\n");
    ULTIMA_20fa_WaitTicks(4);
    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);
    ULTIMA_10e0_GRAP_51_PutTile(0x11c, 5, 5);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    ULTIMA_0f46_GRAP_66_Reveal(8, 8, 0xb7, 0xb7);

    if (D_5888_karma < 0x4b)
    {
        D_5888_karma = 0x4b;
    }

    D_5893_mapId = 0x11;
    D_5895_mapLevel = 1;
    D_587c_partyTile = TILE_ACTOR_AVATAR;

    D_5896_mapX = D_5897_mapY = 10;
    D_587a_statusIcon = D_588e_protectTurns = 0;

    while (D_587f_hour != 0x6)
    {
        ULTIMA_4f7c_AdvanceTime(9);
    }

    D_58a7_torchTurns = D_58a6_lightSpellTurns = 0;

    if (D_57a8_food == 0)
    {
        D_57a8_food = 0x3f;
    }

    if (savedMapId != 0 && savedMapId < 0x21)
    {
        TOWN_11f0_Entry(1);
    }
}
