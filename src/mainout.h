#ifndef _MAINOUT_H
#define _MAINOUT_H

void MAINOUT_0000_SetUpOverworld(void);
void MAINOUT_007a_WaitIfActorNear(void);
void MAINOUT_0354_MoveParty(int stepX, int stepY);
int MAINOUT_06ec_AttackCmd(void);
int MAINOUT_08de_EnterCmd(void);
void MAINOUT_0d22_Entry(void);
int MAINOUT_105c_IsMovingActor(int tile);
int MAINOUT_1a60_MonstersTurn(void);

#endif
