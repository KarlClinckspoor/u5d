#ifndef _TOWN_H
#define _TOWN_H

int CheckNpcKilled(int npc);
void SetNpcKilled(int npc);
void DespawnNpc(int npc);
int FindNpcByActor(int actorIdx);
void SwapDayNightTiles(void);
void RaiseAlarm(void);
int AttackCmd(void);
int KlimbCmd(void);
// called from town2.c
void SetupShadowlord(void);
void LoadTownMap(int placeNpcs);
bool MoveParty(int direction);
void SetPursue(int npc);
void SetFlee(int npc);
void FightNpc(int npc);
void WanderHorses(void);
uint GetTownKey(int prompt);
int NumberKey(int key);
void TileEffects(void);
void TOWN_10da_ShoutBegone(int npc);
void TownEntry(int fresh);
void TownMainLoop(void);
void PlaceActors(void);
void PlaceNpcActor(int npc, int x, int y, int z);

#endif
