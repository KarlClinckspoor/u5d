#ifndef _SJOG_H
#define _SJOG_H

int AskDungeonCell(int facing);
void SearchCmd(void);
void JimmyCmd(void);
void OpenCmd(void);
void GetCmd(void);
int UnequipAll(int item);
void CountCombatants(void);
int CombatMovePlayer(int combatant, int dir);
int CombatKlimb(void);
void CheckMirror(void);
int CombatMiscCmd(char* word, int kind);
int CombatSetActivePlayer(int member);
void TurnUpkeep(void);
void EndCombat(int actor);
int IsCellBlocked(int x, int y, int combatant);
int IsSurrounded(int a);
int BreakCharm(void);

#endif
