#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "outsubs.h"

// The part of OUTSUBS.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").

u16 apparitionFreq[6] = { 0x0a3c, 0x0a3c, 0x0a3c, 0x0e74, 0x0f3c, 0x1040 };

// CHECKED
// "An apparition!": heals the party, raises levels by experience, then speaks a speech chosen by karma
void Apparition(void)
{
    int newLevel_i;
    int karmaIndex_g;
    int step;
    int member;
    int classIndex;
    ActorFmt* memberActor_n;

    PrintString(/*0x7750*/ "An apparition!\n");
    AudioPulse(0x0a3c, 1, 10000, 0x9c4, 6);

    for (member = 0; member < 6; member++)
    {
        AudioPulse(apparitionFreq[member], 1, 5000, 200, 0xd);
    }

    memberActor_n = &actors[10];
    
    actors[10].x = actors[10].y = 5;
    actors[10].tile = actors[10].animTile = TILE_ACTOR_CIRCLE;

    ULTIMA_1068_RevealTile(0x174, 5, 5);

    actors[10].tile = actors[10].animTile = TILE_ACTOR_74;

    for (member = 0; member < partyCount; member++)
    {
        if (party[member].status != STATUS_DEAD)
        {
            party[member].hp = party[member].maxHp;
            party[member].status = STATUS_GOOD;

            for (step = 0; step < 32; step++)
            {
                if (combatants[step].entityIdx == member)
                {
                    memberActor_n = &actors[combatants[step].actorIdx];
                    break;
                }
            }

            classIndex = FindIndex(/*0x7760*/ CLASS_LIST, party[member].cls);
            memberActor_n->tile = memberActor_n->animTile = classTile[classIndex];

            actors[10].animState = 0;

            WaitFrames(1);
            AudioPulse(5500, 1, 5000, 200, 13);
            GRAP_2d_SetPenColor(brightWhiteColor);
            GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
            AudioPulse(5500, 1, 60000, 2500, 1);

            for (step = 0; step < 3; step++)
            {
                actors[10].animState = 1;
                WaitFrames(1);
            }

            newLevel_i = 1;
            for (step = party[member].exp / 100; step > 0; step >>= 1)
            {
                newLevel_i++;
            }

            // 070b
            if (party[member].level != newLevel_i)
            {
                party[member].level = newLevel_i;
                party[member].hp = party[member].maxHp = newLevel_i * 0x1e;

                PrintString(/*0x776a*/ "\n\"Hail, ");
                PrintString(party[member].name);
                PrintString(/*0x7774*/ "!\nFor thy valiant deeds, I shall reward thee!\n");
                PrintString(/*0x77a4*/ "Thou art now level ");
                PrintNumber(newLevel_i, 1, 0x20);
                PrintString(/*0x77b8*/ ", and\n");

                switch (RandomRange(1, 3))
                {
                case 1:
                    PrintString(/*0x77c0*/ "stronger!");
                    IncreaseByte(&party[member].str, 1, 0x1e);
                    break;
                case 2:
                    PrintString(/*0x77ca*/ "quicker!");
                    IncreaseByte(&party[member].dex, 1, 0x1e);
                    break;
                case 3:
                    PrintString(/*0x77d4*/ "wiser!");
                    IncreaseByte(&party[member].intel, 1, 0x1e);
                    break;
                }

                PrintString(/*0x77dc*/ "\" ");
                GetChar();
                PrintChar('\n');
            }
        }
        
        switch (party[member].cls)
        {
        case CLASS_AVATAR:
        case CLASS_MAGE:
            party[member].mag = party[member].intel;
            break;
        case CLASS_BARD:
            party[member].mag = party[member].intel >> 1;
            break;
        }

        UpdateVitalsDisplay();
    }

    PrintString(/*0x77e0*/ "\n\"");

    karmaIndex_g = karma / 20;
    if (karmaIndex_g < 4)
    {
        ReadFileFromDisk(/*0x77e4*/ "KARMA.DAT", scratch, 2000, karmaTextOffsets[karmaIndex_g]);
    }
    else
    {
        ReadFileFromDisk(/*0x77ee*/ "KARMA.DAT", scratch, 2000, 0x29f);
    }

    PrintString(scratch);
    PrintChar('\"');
    GetChar();
    PrintString(/*0x77f8*/ "\n\nThe strangely familiar old man vanishes...\n");

    actors[10].tile = actors[10].animTile = TILE_ACTOR_CIRCLE;

    ULTIMA_1068_RevealTile(GetCombatMap(5, 5), 5, 5);

    actors[10].tile = actors[10].animTile = 0;

    WaitFrames(1);
    AdvanceTime(0);
}
