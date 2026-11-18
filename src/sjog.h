#ifndef _SJOG_H
#define _SJOG_H

int SJOG_006c_AskDungeonCell(int facing);
void SJOG_095c_SearchCmd(void);
void SJOG_0d4a_JimmyCmd(void);
void SJOG_1374_OpenCmd(void);
void SJOG_18ce_GetCmd(void);
int SJOG_1b34_UnequipAll(int item);
void SJOG_1b6c_CountCombatants(void);
int SJOG_1c56_CombatMovePlayer(int combatant, int dir);
int SJOG_1d6a_CombatKlimb(void);
void SJOG_1ea4_CheckMirror(void);
int SJOG_1f26_CombatMiscCmd(char* word, int kind);
int SJOG_1f7a_CombatSetActivePlayer(int member);
void SJOG_2012_TurnUpkeep(void);
void SJOG_203e_EndCombat(int actor);
int SJOG_20d8_IsCellBlocked(int x, int y, int combatant);
int SJOG_2148_IsSurrounded(int a);
int SJOG_21ce_BreakCharm(void);

#endif
