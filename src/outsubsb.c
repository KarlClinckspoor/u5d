#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "outsubs.h"

// The part of OUTSUBS.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").

u16 D_3a26_apparitionFreq[6] = { 0x0a3c, 0x0a3c, 0x0a3c, 0x0e74, 0x0f3c, 0x1040 };

// CHECKED
// "An apparition!": heals the party, raises levels by experience, then speaks a speech chosen by karma
void OUTSUBS_0658_Apparition(void)
{
    int newLevel_i;
    int karmaIndex_g;
    int step;
    int member;
    int classIndex;
    ActorFmt* memberActor_n;

    ULTIMA_1850_PrintString(/*0x7750*/ "An apparition!\n");
    ULTIMA_2192_AudioPulse(0x0a3c, 1, 10000, 0x9c4, 6);

    for (member = 0; member < 6; member++)
    {
        ULTIMA_2192_AudioPulse(D_3a26_apparitionFreq[member], 1, 5000, 200, 0xd);
    }

    memberActor_n = &D_5c5a_actors[10];
    
    D_5c5a_actors[10].x = D_5c5a_actors[10].y = 5;
    D_5c5a_actors[10].tile = D_5c5a_actors[10].animTile = TILE_ACTOR_CIRCLE;

    ULTIMA_1068_RevealTile(0x174, 5, 5);

    D_5c5a_actors[10].tile = D_5c5a_actors[10].animTile = TILE_ACTOR_74;

    for (member = 0; member < D_585b_partyCount; member++)
    {
        if (D_55a8_party[member].status != STATUS_DEAD)
        {
            D_55a8_party[member].hp = D_55a8_party[member].maxHp;
            D_55a8_party[member].status = STATUS_GOOD;

            for (step = 0; step < 32; step++)
            {
                if (D_ba14_combatants[step].entityIdx == member)
                {
                    memberActor_n = &D_5c5a_actors[D_ba14_combatants[step].actorIdx];
                    break;
                }
            }

            classIndex = ULTIMA_4d76_FindIndex(/*0x7760*/ CLASS_LIST, D_55a8_party[member].cls);
            memberActor_n->tile = memberActor_n->animTile = D_1ade_classTile[classIndex];

            D_5c5a_actors[10].animState = 0;

            ULTIMA_3ae6_WaitFrames(1);
            ULTIMA_2192_AudioPulse(5500, 1, 5000, 200, 13);
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
            ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
            ULTIMA_2192_AudioPulse(5500, 1, 60000, 2500, 1);

            for (step = 0; step < 3; step++)
            {
                D_5c5a_actors[10].animState = 1;
                ULTIMA_3ae6_WaitFrames(1);
            }

            newLevel_i = 1;
            for (step = D_55a8_party[member].exp / 100; step > 0; step >>= 1)
            {
                newLevel_i++;
            }

            // 070b
            if (D_55a8_party[member].level != newLevel_i)
            {
                D_55a8_party[member].level = newLevel_i;
                D_55a8_party[member].hp = D_55a8_party[member].maxHp = newLevel_i * 0x1e;

                ULTIMA_1850_PrintString(/*0x776a*/ "\n\"Hail, ");
                ULTIMA_1850_PrintString(D_55a8_party[member].name);
                ULTIMA_1850_PrintString(/*0x7774*/ "!\nFor thy valiant deeds, I shall reward thee!\n");
                ULTIMA_1850_PrintString(/*0x77a4*/ "Thou art now level ");
                ULTIMA_1a3e_PrintNumber(newLevel_i, 1, 0x20);
                ULTIMA_1850_PrintString(/*0x77b8*/ ", and\n");

                switch (ULTIMA_2092_RandomRange(1, 3))
                {
                case 1:
                    ULTIMA_1850_PrintString(/*0x77c0*/ "stronger!");
                    ULTIMA_3ef0_IncreaseByte(&D_55a8_party[member].str, 1, 0x1e);
                    break;
                case 2:
                    ULTIMA_1850_PrintString(/*0x77ca*/ "quicker!");
                    ULTIMA_3ef0_IncreaseByte(&D_55a8_party[member].dex, 1, 0x1e);
                    break;
                case 3:
                    ULTIMA_1850_PrintString(/*0x77d4*/ "wiser!");
                    ULTIMA_3ef0_IncreaseByte(&D_55a8_party[member].intel, 1, 0x1e);
                    break;
                }

                ULTIMA_1850_PrintString(/*0x77dc*/ "\" ");
                ULTIMA_266c_GetChar();
                ULTIMA_16ba_PrintChar('\n');
            }
        }
        
        switch (D_55a8_party[member].cls)
        {
        case CLASS_AVATAR:
        case CLASS_MAGE:
            D_55a8_party[member].mag = D_55a8_party[member].intel;
            break;
        case CLASS_BARD:
            D_55a8_party[member].mag = D_55a8_party[member].intel >> 1;
            break;
        }

        ULTIMA_2900_UpdateVitalsDisplay();
    }

    ULTIMA_1850_PrintString(/*0x77e0*/ "\n\"");

    karmaIndex_g = D_5888_karma / 20;
    if (karmaIndex_g < 4)
    {
        ULTIMA_256e_ReadFileFromDisk(/*0x77e4*/ "KARMA.DAT", D_b21e_scratch, 2000, D_1a74_karmaTextOffsets[karmaIndex_g]);
    }
    else
    {
        ULTIMA_256e_ReadFileFromDisk(/*0x77ee*/ "KARMA.DAT", D_b21e_scratch, 2000, 0x29f);
    }

    ULTIMA_1850_PrintString(D_b21e_scratch);
    ULTIMA_16ba_PrintChar('\"');
    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString(/*0x77f8*/ "\n\nThe strangely familiar old man vanishes...\n");

    D_5c5a_actors[10].tile = D_5c5a_actors[10].animTile = TILE_ACTOR_CIRCLE;

    ULTIMA_1068_RevealTile(GetCombatMap(5, 5), 5, 5);

    D_5c5a_actors[10].tile = D_5c5a_actors[10].animTile = 0;

    ULTIMA_3ae6_WaitFrames(1);
    ULTIMA_4f7c_AdvanceTime(0);
}
