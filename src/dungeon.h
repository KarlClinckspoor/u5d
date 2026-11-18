#ifndef _DUNGEON_H
#define _DUNGEON_H

void DUNGEON_0134_SetUpMonster(int respawn);
void DUNGEON_0c76_AfterStep(byte cell, int monsterMoves);
void DUNGEON_0e2e_MainLoop(int fresh);
void DUNGEON_1a90_DrawView(int fullView);
void DUNGEON_1be0_Redraw(void);
int DUNGEON_1c6a_Klimb(int dir, int mustBeEmpty);
void DUNGEON_1d08_Exit(void);
int DUNGEON_1d4a_AttackCmd(void);
int DUNGEON_1e10_KlimbCmd(void);

#endif
