#ifndef _DUNGEON_H
#define _DUNGEON_H

void SetUpMonster(int respawn);
void AfterStep(byte cell, int monsterMoves);
void DungeonMainLoop(int fresh);
void DrawView(int fullView);
void Redraw(void);
int Klimb(int dir, int mustBeEmpty);
void DUNGEON_1d08_Exit(void);
int DungeonAttackCmd(void);
int DungeonKlimbCmd(void);

#endif
