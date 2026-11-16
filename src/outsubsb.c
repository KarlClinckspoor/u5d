#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "audio/aud_mus.h"

#include "outsubs.h"

// The part of OUTSUBS.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").

u16 D_3a26[6] = { 0x0a3c, 0x0a3c, 0x0a3c, 0x0e74, 0x0f3c, 0x1040 };

// CHECKED
void OUTSUBS_0658(void)
{
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6_aa;
    ActorFmt* local_ptr;

    ULTIMA_1850_PrintString(_TEXT(0x7750, "An apparition!\n"));
    ULTIMA_2192_AudioPulse(0x0a3c, 1, 10000, 0x9c4, 6);

    for (local_8 = 0; local_8 < 6; local_8++)
    {
        ULTIMA_2192_AudioPulse(D_3a26[local_8], 1, 5000, 200, 0xd);
    }

    local_ptr = &D_5c5a[10];
    
    D_5c5a[10]._2_x = D_5c5a[10]._3_y = 5;
    D_5c5a[10]._0_tile = D_5c5a[10]._1_animTile = TILE_ACTOR_CIRCLE;

    ULTIMA_1068(0x174, 5, 5);

    D_5c5a[10]._0_tile = D_5c5a[10]._1_animTile = TILE_ACTOR_74;

    for (local_8 = 0; local_8 < D_585b; local_8++)
    {
        if (D_55a8_party[local_8].status != STATUS_DEAD)
        {
            D_55a8_party[local_8].hp = D_55a8_party[local_8].maxHp;
            D_55a8_party[local_8].status = STATUS_GOOD;

            for (local_a = 0; local_a < 32; local_a++)
            {
                if (D_ba14[local_a].entityIdx == local_8)
                {
                    local_ptr = &D_5c5a[D_ba14[local_a].actorIdx];
                    break;
                }
            }

            local_6_aa = ULTIMA_4d76_FindIndex(_TEXT(0x7760, CLASS_LIST), D_55a8_party[local_8].cls);
            local_ptr->_0_tile = local_ptr->_1_animTile = D_1ade[local_6_aa];

            D_5c5a[10]._6 = 0;

            ULTIMA_3ae6(1);
            ULTIMA_2192_AudioPulse(5500, 1, 5000, 200, 13);
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
            ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
            ULTIMA_2192_AudioPulse(5500, 1, 60000, 2500, 1);

            for (local_a = 0; local_a < 3; local_a++)
            {
                D_5c5a[10]._6 = 1;
                ULTIMA_3ae6(1);
            }

            local_e_am = 1;
            for (local_a = D_55a8_party[local_8].exp / 100; local_a > 0; local_a >>= 1)
            {
                local_e_am++;
            }

            // 070b
            if (D_55a8_party[local_8].level != local_e_am)
            {
                D_55a8_party[local_8].level = local_e_am;
                D_55a8_party[local_8].hp = D_55a8_party[local_8].maxHp = local_e_am * 0x1e;

                ULTIMA_1850_PrintString(_TEXT(0x776a, "\n\"Hail, "));
                ULTIMA_1850_PrintString(D_55a8_party[local_8].name);
                ULTIMA_1850_PrintString(_TEXT(0x7774, "!\nFor thy valiant deeds, I shall reward thee!\n"));
                ULTIMA_1850_PrintString(_TEXT(0x77a4, "Thou art now level "));
                ULTIMA_1a3e_PrintNumber(local_e_am, 1, 0x20);
                ULTIMA_1850_PrintString(_TEXT(0x77b8, ", and\n"));

                switch (ULTIMA_2092_RandomRange(1, 3))
                {
                case 1:
                    ULTIMA_1850_PrintString(_TEXT(0x77c0, "stronger!"));
                    ULTIMA_3ef0_IncreaseByte(&D_55a8_party[local_8].str, 1, 0x1e);
                    break;
                case 2:
                    ULTIMA_1850_PrintString(_TEXT(0x77ca, "quicker!"));
                    ULTIMA_3ef0_IncreaseByte(&D_55a8_party[local_8].dex, 1, 0x1e);
                    break;
                case 3:
                    ULTIMA_1850_PrintString(_TEXT(0x77d4, "wiser!"));
                    ULTIMA_3ef0_IncreaseByte(&D_55a8_party[local_8].intel, 1, 0x1e);
                    break;
                }

                ULTIMA_1850_PrintString(_TEXT(0x77dc, "\" "));
                ULTIMA_266c_GetChar();
                ULTIMA_16ba_PrintChar('\n');
            }
        }
        
        switch (D_55a8_party[local_8].cls)
        {
        case CLASS_AVATAR:
        case CLASS_MAGE:
            D_55a8_party[local_8].mag = D_55a8_party[local_8].intel;
            break;
        case CLASS_BARD:
            D_55a8_party[local_8].mag = D_55a8_party[local_8].intel >> 1;
            break;
        }

        ULTIMA_2900_UpdateVitalsDisplay();
    }

    ULTIMA_1850_PrintString(_TEXT(0x77e0, "\n\""));

    local_c = D_5888 / 20;
    if (local_c < 4)
    {
        ULTIMA_256e_ReadFileFromDisk(_TEXT(0x77e4, "KARMA.DAT"), D_b21e, 2000, D_1a74[local_c]);
    }
    else
    {
        ULTIMA_256e_ReadFileFromDisk(_TEXT(0x77ee, "KARMA.DAT"), D_b21e, 2000, 0x29f);
    }

    ULTIMA_1850_PrintString(D_b21e);
    ULTIMA_16ba_PrintChar('\"');
    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString(_TEXT(0x77f8, "\n\nThe strangely familiar old man vanishes...\n"));

    D_5c5a[10]._0_tile = D_5c5a[10]._1_animTile = TILE_ACTOR_CIRCLE;

    ULTIMA_1068(GetCombatMap(5, 5), 5, 5);

    D_5c5a[10]._0_tile = D_5c5a[10]._1_animTile = 0;

    ULTIMA_3ae6(1);
    ULTIMA_4f7c(0);
}
