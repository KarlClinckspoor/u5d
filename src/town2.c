// TOWN.OVL is two MSC 5.1 objects: town.c (to TOWN_0f02_TileEffects) and this one (from TOWN_10da_ShoutBegone).
// TOWN_0f02_TileEffects is the last function of the first: its last loop is compiled with pass 2's
// memory used up (see its note), and the functions from TOWN_10da_ShoutBegone compile as the original
// only in an object of their own.
#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <memory.h>

#include "blckthrn.h"
#include "cast2.h"
#include "npc.h"
#include "talk.h"
#include "town.h"

// "Begone, vermin!": the party is damaged and the NPC becomes a flee-er.
void TOWN_10da_ShoutBegone(int npc)
{
    ULTIMA_1850_PrintString(/*0x278a*/ "\"Begone,\nvermin!\"\n");
    ULTIMA_2aa8_DamageParty();
    TOWN_08d4_SetFlee(npc);
}

// 1 for a villager with a schedule time, half the time; picks the NPCs a shadowlord's air works on.
static int TOWN_10f2_PickNpcForAlarm(int npc)
{
    int type;
    int i_h;
    int picked;

    picked = 0;
    for (i_h = 0; i_h < 4; i_h++)
    {
        if (D_5d5e_npcSchedules[npc].time[i_h] != 0)
        {
            picked = 1;
        }
    }

    type = D_659e_npcTypes[i_h];
    if (type < TILE_ACTOR_WIZARD || type >= TILE_ACTOR_74)
    {
        picked = 0;
    }

    if (ULTIMA_2092_RandomRange(0, 1) != 0)
    {
        picked = 0;
    }

    return picked;
}

// The shadowlord's air at the entry: hatred (1) makes villagers pursue (dialog 0xfe), cowardice (2) makes them flee (0xfd).
static void TOWN_1156_ShadowlordEffect(void)
{
    int npc;

    switch (D_5958_townShadowlord)
    {
    case 1:
        for (npc = 0; npc < 0x20; npc++)
        {
            if (TOWN_10f2_PickNpcForAlarm(npc) != 0)
            {
                TOWN_085e_SetPursue(npc);
                D_5f5e_npcs[npc].dialogNumber = 0xfe;
            }
        }
        break;

    case 2:
        for (npc = 0; npc < 0x20; npc++)
        {
            if (TOWN_10f2_PickNpcForAlarm(npc) != 0)
            {
                TOWN_08d4_SetFlee(npc);
                D_5f5e_npcs[npc].dialogNumber = 0xfd;
            }
        }
        break;
    }
}

char* D_27dc_shadowlordTraits[3] = {
    /*0x279d*/ "falsehood",
    /*0x27a7*/ "hatred",
    /*0x27ae*/ "cowardice"
};

// "An air of falsehood/hatred/cowardice doth surround thee..." with a sound.
static void TOWN_11b8_PrintAirOf(int shadowlord)
{
    ULTIMA_1850_PrintString(/*0x27b8*/ "\nAn air of\n");
    ULTIMA_1850_PrintString(D_27dc_shadowlordTraits[shadowlord]);
    ULTIMA_1850_PrintString(/*0x27c4*/ " doth surround thee...\n");
    ULTIMA_2192_AudioPulse(0x19c8, 1, 60000, 2000, 1);
}

// Enter a town (fresh: clear actors, load the NPC file and place the NPCs): map, shadowlord, killed NPCs removed, the airs of the shadowlords.
void TOWN_11f0_Entry(int fresh)
{
    int i;

    D_a9bc_townJustEntered = D_24e6_viewDirty = 1;
    if (fresh != 0)
    {
        for (i = 1; i < 0x20; i++)
        {
            D_5c5a_actors[i].tile = 0;
        }
        D_5957_drunkTurns = 0;
        D_58a4_viewOn = 1;
        NPC_0000_LoadNpcFile();
        NPC_00d6_PlaceNpcs(D_587f_hour);
    }
    D_5958_townShadowlord = 0xff;
    TOWN_0408_LoadTownMap(fresh);
    TOWN_02ae_SetupShadowlord();

    for (i = 0; i < 0x20; i++)
    {
        if (TOWN_0000_CheckNpcKilled(i) != 0)
            TOWN_00b0_DespawnNpc(i);
    }

    if ((D_5893_mapId == 29) && (D_57b5_sceptre != 0))
        TOWN_00b0_DespawnNpc(9);

    ULTIMA_2900_UpdateVitalsDisplay();

    if (ULTIMA_39fc_GetFirstActivePartyMember() >= 0)
    {
        ULTIMA_5910_UpdateFrame();
        if (D_5893_mapId == 29)
        {
            for (i = 2; i >= 0; i--)
            {
                if (D_58c8_shadowlordTowns[i] < 0x80)
                {
                    TOWN_11b8_PrintAirOf(i);
                }
            }
        }
        else if (D_5958_townShadowlord != (byte)-1)
        {
            TOWN_11b8_PrintAirOf(D_5958_townShadowlord);
            TOWN_1156_ShadowlordEffect();
        }
    }
}

// "Thou art under arrest!": in the jail map (0x12) the party is captured; elsewhere answer to come quietly (wake in the jail map at 8 o'clock) or fight. Returns 1 when the party fights.
static int TOWN_12ae_Arrest(void)
{
    int fight;
    char key_h;

    fight = 0;
    if (D_5893_mapId == 0x12)
    {
        if (ULTIMA_39fc_GetFirstActivePartyMember() >= 0)
        {
            BLCKTHRN_060e_Capture();
            TOWN_11f0_Entry(1);
        }
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x27e2*/ "\n\"Thou art under arrest!\"\n\n");
        ULTIMA_1850_PrintString(/*0x27fe*/ "\"Wilt thou come quietly?\"\n\n:");

        do
        {
            key_h = ULTIMA_266c_GetChar();
        } while (key_h != 'N' && key_h != 'Y');

        if (key_h == 'Y')
        {
            ULTIMA_1850_PrintString(/*0x281b*/ "Yes\n\nThe guard strikes thee unconscious!\n");
            ULTIMA_0a70_GRAP_2d_SetPenColor(0);
            ULTIMA_1850_PrintString(/*0x2845*/ "\nThou dost awaken to...\n");
            D_5893_mapId = 4;
            D_5896_mapX = 0x19;
            D_5897_mapY = 4;
            D_24e6_viewDirty = 1;

            while (D_587f_hour != 8)
            {
                ULTIMA_4f7c_AdvanceTime(0x14);
            }

            D_5895_mapLevel = D_57ac_keys = 0;

            TOWN_11f0_Entry(1);
        }
        else
        {
            ULTIMA_1850_PrintString(/*0x285e*/ "No\n\n\"Then defend thyself, rogue!\"\n");
            TOWN_0958_RaiseAlarm();
            fight = 1;
        }
    }

    return fight;
}

// An NPC next to the Avatar acts: attack (D_65be_npcApproach == 'a'), talk (TALK_031e) or arrest by a guard; the NPC may be killed or removed.
static void TOWN_1352_NpcApproaches(int forced)
{
    ActorFmt* actor;
    int attacked;

    attacked = 0;
    actor = &D_5c5a_actors[D_5f5e_npcs[D_65bf_npcApproaching].actorIdx];

    ULTIMA_5910_UpdateFrame();

    if (D_65be_npcApproach == 0x61)
    {
        if (D_5f5e_npcs[D_65bf_npcApproaching].dialogNumber == 0xfe)
        {
            ULTIMA_16ba_PrintChar('\n');
            TOWN_10da_ShoutBegone(D_65bf_npcApproaching);
        }
        else if (actor->tile != TILE_ACTOR_GUARD)
        {
            attacked = 1;
        }
        else
        {
            attacked = TOWN_12ae_Arrest();
        }
    }
    else if (forced == 0)
    {
        if (D_5f5e_npcs[D_65bf_npcApproaching].dialogNumber != 0 && TALK_031e_TalkToNpc(D_65bf_npcApproaching) != 0)
        {
            attacked = TOWN_12ae_Arrest();
        }
    }
    else
    {
        attacked = TOWN_12ae_Arrest();
    }

    if (attacked != 0)
    {
        if (D_5c5a_actors[D_5f5e_npcs[D_65bf_npcApproaching].actorIdx].tile >= TILE_ACTOR_WIZARD)
        {
            ULTIMA_1850_PrintString(/*0x2881*/ "\nAttacked!\n");
            TOWN_09bc_FightNpc(D_65bf_npcApproaching);
        }
        else
        {
            TOWN_00b0_DespawnNpc(D_65bf_npcApproaching);
        }
    }
}

// The main loop of a town: a command each turn (Ctrl-K karma, Ctrl-E exit, Ctrl-V version, Ctrl-S sound, moves, digits, other commands), then time, tile effects, horses, NPCs.
void TOWN_141e_MainLoop(void)
{
    int local_14; // unused
    int hourBefore_o;
    int quickToggle;
    int leave;
    uint turn;
    int leader_m;
    uint key;
    int mountToggle;
    int prompt;

    mountToggle = quickToggle = 0;

    do
    {
        leave = 0;
        turn = 1;
        leader_m = ULTIMA_39fc_GetFirstActivePartyMember();
        // 1456
        if (leader_m == 1)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a_PrintLeftCap();
            ULTIMA_1850_PrintString(/*0x288d*/ "Zzzzzz...\n");
        }
        else if (leader_m == -1)
        {
            BLCKTHRN_0910_Death();
            turn = 0;
        }
        else
        {
            if (D_a9bc_townJustEntered != 0)
            {
                D_a9bc_townJustEntered = 0;
            }
            else
            {
                if (ULTIMA_48a8_EnterMoongate() != 0) // FMT: LOOKOBJ_UNK();
                {
                    CAST2_0e76_Shrine(); // thunk f89a
                }
            }
            if (D_5893_mapId == 0)
            {
                return;
            }

            prompt = 1;

            do {
                // 148f
                key = TOWN_0dc4_GetTownKey(prompt);
                prompt = 0;

                if (key < 0x20)
                {
                    // 14a5
                    switch (key)
                    {
                    case U5_KEY_CTRL_K: // 14b8 (9688)
                        ULTIMA_1a3e_PrintNumber(D_5888_karma, 1, 0x20);
                        // 14ff
                        ULTIMA_16ba_PrintChar('\n');   // 94ea
                        // 154b
                        turn = 0;
                        break;

                    case U5_KEY_CTRL_E: // 14cc (969c)
                        ULTIMA_1850_PrintString(/*0x2898*/ "Exit to DOS? ");
                        key = ULTIMA_266c_GetChar();
                        if (key == 0x59)
                        {
                            ULTIMA_0878_RestoreVideoMode();
                            exit(0); // cdecl
                        }
                        else
                        {
                            ULTIMA_1850_PrintString(/*0x28a6*/ "N\n");
                        }
                        break;

                    case U5_KEY_CTRL_V: // 14f8 (96c8)
                        ULTIMA_1850_PrintString(/*0x28a9*/ "1.16");
                        ULTIMA_16ba_PrintChar('\n');
                        turn = 0;
                        break;

                    case U5_KEY_CTRL_S: // 1508 (96d8)
                        ULTIMA_1850_PrintString(/*0x28ae*/ "Sound ");
                        if (D_a9ce_soundOn != 0)
                        {
                            ULTIMA_1850_PrintString(/*0x28b5*/ "Off\n");
                        }
                        else
                        {
                            ULTIMA_1850_PrintString(/*0x28ba*/ "On\n");
                        }

                        D_a9ce_soundOn = !D_a9ce_soundOn;
                        // 154b
                        turn = 0;
                        break;

                    case U5_KEY_LEFT:
                    case U5_KEY_RIGHT:
                    case U5_KEY_UP:
                    case U5_KEY_DOWN: // 1532 (9702)
                        leave = TOWN_0600_MoveParty(key); // bp-06
                        turn = (uint)(leave == 0); // -> 159a
                        break;

                    default: // 1544 (9714)
                        ULTIMA_1850_PrintString(/*0x28be*/ "What?\n");
                        turn = 0;
                        break;
                    }
                }
                else if ((key < 0x30) || (key > 0x39))
                {
                    turn = ULTIMA_3178_ProcessCommand(key);
                }
                else
                {
                    turn = TOWN_0e34_NumberKey(key);
                }
                // 159d
            } while (turn == 3);
        }
        // 15a6
        if (D_5893_mapId == 0)
        {
            leave = 1;
        }
        else if (turn != 0 && ULTIMA_39fc_GetFirstActivePartyMember() != -1)
        {
            hourBefore_o = D_587f_hour;
            ULTIMA_4f7c_AdvanceTime(1);
            if ((D_587f_hour != hourBefore_o) && (D_587f_hour == 20 || D_587f_hour == 5))
            {
                TOWN_0170_SwapDayNightTiles();
            }
            // 15ec
            TOWN_0f02_TileEffects();
            if ((D_594f_openDoorTile != 0) && (D_5952_openDoorTurns--, D_5952_openDoorTurns == 0))
            {
                ULTIMA_39cc_SetTile(D_594f_openDoorTile, D_5950_openDoorX, D_5951_openDoorY);
            }
            // 160d
            D_5c5a_actors[0].x = D_5896_mapX;
            D_5c5a_actors[0].y = D_5897_mapY;
            D_5c5a_actors[0].z = D_5895_mapLevel;
            if ((D_587c_partyTile < TILE_ACTOR_RIDING_HORSE || D_587c_partyTile >= TILE_ACTOR_CIRCLE || key == 0x20 ||
                (mountToggle = !mountToggle, !mountToggle)) &&
                D_587a_statusIcon != 'T' && (D_587a_statusIcon != 'Q' || (quickToggle = !quickToggle, !quickToggle)))
            {
                // 165f
                TOWN_0c78_WanderHorses();
                if ((int)turn < 2)
                {
                    NPC_0db4_MoveNpcs(D_587f_hour);
                }
                // 1671
                if (D_65bf_npcApproaching != 0 || turn == 2)
                {
                    // 167e
                    TOWN_1352_NpcApproaches(turn - 1);
                }
            }
        }
        // 1686
    } while (leave == 0);

    // 168f
}

// Clear every NPC actor and put each NPC with a type on its place of the schedule entry in force at the hour.
void TOWN_1694_PlaceActors(void)
{
    int slot;
    int npc;

    for (npc = 1; npc < 0x20; npc++)
    {
        ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, npc);
        D_5f5e_npcs[npc].actorIdx = 0;
    }

    for (npc = 1; npc < 0x20; npc++)
    {
        if (D_659e_npcTypes[npc] != 0)
        {
            slot = NPC_12e0_ScheduleSlot(npc, D_587f_hour);
            TOWN_1726_PlaceNpcActor(npc, D_5d5e_npcSchedules[npc].x[slot], D_5d5e_npcSchedules[npc].y[slot], D_5d5e_npcSchedules[npc].z[slot]);
            D_5f5e_npcs[npc].mode = 1;
            D_5f5e_npcs[npc].scheduleIdx = slot;
            D_655e_moveListPos[npc] = -1;
        }
    }
}

// Give the NPC an actor on this level (or none when it is on another) and set its position; mode 1.
void TOWN_1726_PlaceNpcActor(int npc, int x, int y, int z)
{
    int actorSlot;
    int value;
    NpcFmt* npcRec;

    if (D_659e_npcTypes[npc] == 0)
        return;

    npcRec = &D_5f5e_npcs[npc];

    if (z != D_5895_mapLevel && npcRec->actorIdx != 0)
    {
        D_5c5a_actors[npcRec->actorIdx].tile = 0;
        npcRec->actorIdx = 0;
    }

    if (z == D_5895_mapLevel && npcRec->actorIdx == 0)
    {
        actorSlot = ULTIMA_38e4_FindActorSlot();

        if (D_659e_npcTypes[npc] == 1)
        {
            value = 0x1e;
        }
        else if ((D_28c6_npcValueMask[D_5893_mapId - 1] & ((u32)1 << npc)) != 0)
        {
            value = 0xff;
        }
        else
        {
            value = 0;
        }

        ULTIMA_3a74_SetActor(D_659e_npcTypes[npc], D_659e_npcTypes[npc], x, y, z, value, actorSlot);

        D_5c5a_actors[actorSlot].animState = 0;
        npcRec->actorIdx = actorSlot;
    }

    if (z == D_5895_mapLevel && npcRec->actorIdx != 0)
    {
        actorSlot = npcRec->actorIdx;
        D_5c5a_actors[actorSlot].x = x;
        D_5c5a_actors[actorSlot].y = y;
        D_5c5a_actors[actorSlot].z = z;
    }

    npcRec->x = x;
    npcRec->y = y;
    npcRec->z = z;
    npcRec->mode = 1;
}

// after the last of TOWN's strings in its _DATA (DS:28c6)
u32 D_28c6_npcValueMask[0x20] =
{
    0, 0, 0, 0x28000, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0x3F8, 0x1E0, 0, 0, 0
};
