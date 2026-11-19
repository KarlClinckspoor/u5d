#ifndef _MAINOUT_H
#define _MAINOUT_H

void SetUpOverworld(void);
void WaitIfActorNear(void);
void MainoutMoveParty(int stepX, int stepY);
int MainoutAttackCmd(void);
int EnterCmd(void);
void MainoutEntry(void);
int IsMovingActor(int tile);
int MonstersTurn(void);

#endif
