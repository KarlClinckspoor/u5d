// TOWN.OVL is two MSC 5.1 objects: town.c (to TOWN_0f02) and this one (from TOWN_10da).
// TOWN_0f02 is the last function of the first: its last loop is compiled with pass 2's
// memory used up (see its note), and the functions from TOWN_10da compile as the original
// only in an object of their own.
#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <memory.h>

#include "audio/aud_mus.h"

#include "blckthrn.h"
#include "cast2.h"
#include "npc.h"
#include "talk.h"
#include "town.h"


void TOWN_10da(int param_1)
{
    ULTIMA_1850_PrintString(_TEXT(0x278a, "\"Begone,\nvermin!\"\n"));
    ULTIMA_2aa8();
    TOWN_08d4(param_1);
}

static int TOWN_10f2(int param_1)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_4_ab = 0;
    for (local_6 = 0; local_6 < 4; local_6++)
    {
        if (D_5d5e[param_1].time[local_6] != 0)
        {
            local_4_ab = 1;
        }
    }

    local_8 = D_659e[local_6];
    if (local_8 < TILE_ACTOR_WIZARD || local_8 >= TILE_ACTOR_74)
    {
        local_4_ab = 0;
    }

    if (ULTIMA_2092_RandomRange(0, 1) != 0)
    {
        local_4_ab = 0;
    }

    return local_4_ab;
}

static void TOWN_1156(void)
{
    int local_4;

    switch (D_5958)
    {
    case 1:
        for (local_4 = 0; local_4 < 0x20; local_4++)
        {
            if (TOWN_10f2(local_4) != 0)
            {
                TOWN_085e(local_4);
                D_5f5e[local_4]._a = 0xfe;
            }
        }
        break;

    case 2:
        for (local_4 = 0; local_4 < 0x20; local_4++)
        {
            if (TOWN_10f2(local_4) != 0)
            {
                TOWN_08d4(local_4);
                D_5f5e[local_4]._a = 0xfd;
            }
        }
        break;
    }
}

char* D_27dc[3] = {
    _STATIC_TEXT(0x279d, "falsehood"),
    _STATIC_TEXT(0x27a7, "hatred"),
    _STATIC_TEXT(0x27ae, "cowardice")
};


static void TOWN_11b8(int param_1)
{
    ULTIMA_1850_PrintString(_TEXT(0x27b8, "\nAn air of\n"));
    ULTIMA_1850_PrintString(D_27dc[param_1]);
    ULTIMA_1850_PrintString(_TEXT(0x27c4, " doth surround thee...\n"));
    ULTIMA_2192_AudioPulse(0x19c8, 1, 60000, 2000, 1);
}

void TOWN_11f0_Entry(int param_1)
{
    int local_4;

#if defined(ENABLE_BGM)
    AUDIO_StopBgm();
#endif

    D_a9bc = D_24e6 = 1;
    if (param_1 != 0)
    {
        for (local_4 = 1; local_4 < 0x20; local_4++)
        {
            D_5c5a[local_4]._0_tile = 0;
        }
        D_5957 = 0;
        D_58a4 = 1;
        NPC_0000_LoadNpcFile();
        NPC_00d6(D_587f);
    }
    D_5958 = 0xff;
    TOWN_0408(param_1);
    TOWN_02ae();

    for (local_4 = 0; local_4 < 0x20; local_4++)
    {
        if (TOWN_0000_CheckNpcKilled(local_4) != 0)
            TOWN_00b0_DespawnNpc(local_4);
    }

    if ((D_5893_map_id == 29) && (D_57b5 != 0))
        TOWN_00b0_DespawnNpc(9);

    ULTIMA_2900_UpdateVitalsDisplay();

    if (ULTIMA_39fc_GetFirstActivePartyMember() >= 0)
    {
        ULTIMA_5910_UpdateFrame();
        if (D_5893_map_id == 29)
        {
            for (local_4 = 2; local_4 >= 0; local_4--)
            {
                if (D_58c8[local_4] < 0x80)
                {
                    TOWN_11b8(local_4);
                }
            }
        }
        else if (D_5958 != (byte)-1)
        {
            TOWN_11b8(D_5958);
            TOWN_1156();
        }
    }

#if defined(ENABLE_BGM)
    AUDIO_PlayBgmPerMap();
#endif
}

static int TOWN_12ae(void)
{
    int local_6_an;
    char local_4;

    local_6_an = 0;
    if (D_5893_map_id == 0x12)
    {
        if (ULTIMA_39fc_GetFirstActivePartyMember() >= 0)
        {
            BLCKTHRN_060e_Capture();
            TOWN_11f0_Entry(1);
        }
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x27e2, "\n\"Thou art under arrest!\"\n\n"));
        ULTIMA_1850_PrintString(_TEXT(0x27fe, "\"Wilt thou come quietly?\"\n\n:"));

        do
        {
            local_4 = ULTIMA_266c_GetChar();
        } while (local_4 != 'N' && local_4 != 'Y');

        if (local_4 == 'Y')
        {
            ULTIMA_1850_PrintString(_TEXT(0x281b, "Yes\n\nThe guard strikes thee unconscious!\n"));
            ULTIMA_0a70_GRAP_2d_SetPenColor(0);
            ULTIMA_1850_PrintString(_TEXT(0x2845, "\nThou dost awaken to...\n"));
            D_5893_map_id = 4;
            D_5896_map_x = 0x19;
            D_5897_map_y = 4;
            D_24e6 = 1;

            while (D_587f != 8)
            {
                ULTIMA_4f7c(0x14);
            }

            D_5895_map_level = D_57ac = 0;

            TOWN_11f0_Entry(1);
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x285e, "No\n\n\"Then defend thyself, rogue!\"\n"));
            TOWN_0958();
            local_6_an = 1;
        }
    }

    return local_6_an;
}

static void TOWN_1352(int param_1)
{
    ActorFmt* local_6_p;
    int local_4;

    local_4 = 0;
    local_6_p = &D_5c5a[D_5f5e[D_65bf]._c];

    ULTIMA_5910_UpdateFrame();

    if (D_65be == 0x61)
    {
        if (D_5f5e[D_65bf]._a == 0xfe)
        {
            ULTIMA_16ba_PrintChar('\n');
            TOWN_10da(D_65bf);
        }
        else if (local_6_p->_0_tile != TILE_ACTOR_GUARD)
        {
            local_4 = 1;
        }
        else
        {
            local_4 = TOWN_12ae();
        }
    }
    else if (param_1 == 0)
    {
        if (D_5f5e[D_65bf]._a != 0 && TALK_031e_TalkToNpc(D_65bf) != 0)
        {
            local_4 = TOWN_12ae();
        }
    }
    else
    {
        local_4 = TOWN_12ae();
    }

    if (local_4 != 0)
    {
        if (D_5c5a[D_5f5e[D_65bf]._c]._0_tile >= TILE_ACTOR_WIZARD)
        {
            ULTIMA_1850_PrintString(_TEXT(0x2881, "\nAttacked!\n"));
            TOWN_09bc(D_65bf);
        }
        else
        {
            TOWN_00b0_DespawnNpc(D_65bf);
        }
    }
}

void TOWN_141e_MainLoop(void)
{
    int local_14; // unused
    int local_12_ab;
    int local_10_ad;
    int local_e;
    uint local_c;
    int local_a;
    uint local_8;
    int local_6;
    int local_4_ab;

    local_6 = local_10_ad = 0;

    do
    {
        local_e = 0;
        local_c = 1;
        local_a = ULTIMA_39fc_GetFirstActivePartyMember();
        // 1456
        if (local_a == 1)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a();
            ULTIMA_1850_PrintString(_TEXT(0x288d, "Zzzzzz...\n"));
        }
        else if (local_a == -1)
        {
            BLCKTHRN_0910_Death();
            local_c = 0;
        }
        else
        {
            if (D_a9bc != 0)
            {
                D_a9bc = 0;
            }
            else
            {
                if (ULTIMA_48a8() != 0) // FMT: LOOKOBJ_UNK();
                {
                    CAST2_0e76_Shrine(); // thunk f89a
                }
            }
            if (D_5893_map_id == 0)
            {
                return;
            }

            local_4_ab = 1;

            do {
                // 148f
                local_8 = TOWN_0dc4(local_4_ab);
                local_4_ab = 0;

                if (local_8 < 0x20)
                {
                    // 14a5
                    switch (local_8)
                    {
                    case U5_KEY_CTRL_K: // 14b8 (9688)
                        ULTIMA_1a3e_PrintNumber(D_5888, 1, 0x20);
                        // 14ff
                        ULTIMA_16ba_PrintChar('\n');   // 94ea
                        // 154b
                        local_c = 0;
                        break;

                    case U5_KEY_CTRL_E: // 14cc (969c)
                        ULTIMA_1850_PrintString(_TEXT(0x2898, "Exit to DOS? "));
                        local_8 = ULTIMA_266c_GetChar();
                        if (local_8 == 0x59)
                        {
                            ULTIMA_0878_RestoreVideoMode();
                            exit(0); // cdecl
                        }
                        else
                        {
                            ULTIMA_1850_PrintString(_TEXT(0x28a6, "N\n"));
                        }
                        break;

                    case U5_KEY_CTRL_V: // 14f8 (96c8)
                        ULTIMA_1850_PrintString(_TEXT(0x28a9, U5_VERSION));
                        ULTIMA_16ba_PrintChar('\n');
                        local_c = 0;
                        break;

                    case U5_KEY_CTRL_S: // 1508 (96d8)
                        ULTIMA_1850_PrintString(_TEXT(0x28ae, "Sound "));
                        if (D_a9ce != 0)
                        {
                            ULTIMA_1850_PrintString(_TEXT(0x28b5, "Off\n"));
                        }
                        else
                        {
                            ULTIMA_1850_PrintString(_TEXT(0x28ba, "On\n"));
                        }

                        D_a9ce = !D_a9ce;
                        // 154b
                        local_c = 0;
                        break;

                    case U5_KEY_LEFT:
                    case U5_KEY_RIGHT:
                    case U5_KEY_UP:
                    case U5_KEY_DOWN: // 1532 (9702)
                        local_e = TOWN_0600(local_8); // bp-06
                        local_c = (uint)(local_e == 0); // -> 159a
                        break;

                    default: // 1544 (9714)
                        ULTIMA_1850_PrintString(_TEXT(0x28be, "What?\n"));
                        local_c = 0;
                        break;
                    }
                }
                else if ((local_8 < 0x30) || (local_8 > 0x39))
                {
                    local_c = ULTIMA_3178_ProcessCommand(local_8);
                }
                else
                {
                    local_c = TOWN_0e34(local_8);
                }
                // 159d
            } while (local_c == 3);
        }
        // 15a6
        if (D_5893_map_id == 0)
        {
            local_e = 1;
        }
        else if (local_c != 0 && ULTIMA_39fc_GetFirstActivePartyMember() != -1)
        {
            local_12_ab = D_587f;
            ULTIMA_4f7c(1);
            if ((D_587f != local_12_ab) && (D_587f == 20 || D_587f == 5))
            {
                TOWN_0170();
            }
            // 15ec
            TOWN_0f02();
            if ((D_594f != 0) && (D_5952--, D_5952 == 0))
            {
                ULTIMA_39cc_SetTile(D_594f, D_5950, D_5951);
            }
            // 160d
            D_5c5a[0]._2_x = D_5896_map_x;
            D_5c5a[0]._3_y = D_5897_map_y;
            D_5c5a[0]._4_z = D_5895_map_level;
            if ((D_587c_partyTile < TILE_ACTOR_RIDING_HORSE || D_587c_partyTile >= TILE_ACTOR_CIRCLE || local_8 == 0x20 ||
                (local_6 = !local_6, !local_6)) &&
                D_587a != 'T' && (D_587a != 'Q' || (local_10_ad = !local_10_ad, !local_10_ad)))
            {
                // 165f
                TOWN_0c78();
                if ((int)local_c < 2)
                {
                    NPC_0db4(D_587f);
                }
                // 1671
                if (D_65bf != 0 || local_c == 2)
                {
                    // 167e
                    TOWN_1352(local_c - 1);
                }
            }
        }
        // 1686
    } while (local_e == 0);

    // 168f
}

void TOWN_1694(void)
{
    int local_6_an;
    int local_4;

    for (local_4 = 1; local_4 < 0x20; local_4++)
    {
        ULTIMA_3a74(0, 0, 0, 0, 0, 0, local_4);
        D_5f5e[local_4]._c = 0;
    }

    for (local_4 = 1; local_4 < 0x20; local_4++)
    {
        if (D_659e[local_4] != 0)
        {
            local_6_an = NPC_12e0(local_4, D_587f);
            TOWN_1726(local_4, D_5d5e[local_4].x[local_6_an], D_5d5e[local_4].y[local_6_an], D_5d5e[local_4].z[local_6_an]);
            D_5f5e[local_4]._0 = 1;
            D_5f5e[local_4]._e = local_6_an;
            D_655e[local_4] = -1;
        }
    }
}

void TOWN_1726(int param_1, int param_2, int param_3, int param_4)
{
    int local_6;
    int local_8;
    NpcFmt* local_4_c;

    if (D_659e[param_1] == 0)
        return;

    local_4_c = &D_5f5e[param_1];

    if (param_4 != D_5895_map_level && local_4_c->_c != 0)
    {
        D_5c5a[local_4_c->_c]._0_tile = 0;
        local_4_c->_c = 0;
    }

    if (param_4 == D_5895_map_level && local_4_c->_c == 0)
    {
        local_6 = ULTIMA_38e4();

        if (D_659e[param_1] == 1)
        {
            local_8 = 0x1e;
        }
        else if ((D_28c6[D_5893_map_id - 1] & ((u32)1 << param_1)) != 0)
        {
            local_8 = 0xff;
        }
        else
        {
            local_8 = 0;
        }

        ULTIMA_3a74(D_659e[param_1], D_659e[param_1], param_2, param_3, param_4, local_8, local_6);

        D_5c5a[local_6]._6 = 0;
        local_4_c->_c = local_6;
    }

    if (param_4 == D_5895_map_level && local_4_c->_c != 0)
    {
        local_6 = local_4_c->_c;
        D_5c5a[local_6]._2_x = param_2;
        D_5c5a[local_6]._3_y = param_3;
        D_5c5a[local_6]._4_z = param_4;
    }

    local_4_c->_2_x = param_2;
    local_4_c->_4_y = param_3;
    local_4_c->_6_z = param_4;
    local_4_c->_0 = 1;
}

// after the last of TOWN's strings in its _DATA (DS:28c6)
u32 D_28c6[0x20] =
{
    0, 0, 0, 0x28000, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0x3F8, 0x1E0, 0, 0, 0
};
