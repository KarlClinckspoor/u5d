#ifndef _TOWN_H
#define _TOWN_H

int TOWN_0000_CheckNpcKilled(int npc);
void TOWN_0052_SetNpcKilled(int npc);
void TOWN_00b0_DespawnNpc(int npc);
int TOWN_011e_FindNpcByActor(int actorIdx);
void TOWN_0170_SwapDayNightTiles(void);
void TOWN_0958_RaiseAlarm(void);
int TOWN_09e6_AttackCmd(void);
int TOWN_0b82_KlimbCmd(void);
// called from town2.c
void TOWN_02ae_SetupShadowlord(void);
void TOWN_0408_LoadTownMap(int placeNpcs);
bool TOWN_0600_MoveParty(int direction);
void TOWN_085e_SetPursue(int npc);
void TOWN_08d4_SetFlee(int npc);
void TOWN_09bc_FightNpc(int npc);
void TOWN_0c78_WanderHorses(void);
uint TOWN_0dc4_GetTownKey(int prompt);
int TOWN_0e34_NumberKey(int key);
void TOWN_0f02_TileEffects(void);
void TOWN_10da_ShoutBegone(int npc);
void TOWN_11f0_Entry(int fresh);
void TOWN_141e_MainLoop(void);
void TOWN_1694_PlaceActors(void);
void TOWN_1726_PlaceNpcActor(int npc, int x, int y, int z);

#endif
