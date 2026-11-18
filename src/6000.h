#ifndef _ULTIMA_6000_H
#define _ULTIMA_6000_H

// Game cont.

void ULTIMA_6150_Combat(int a);
void ULTIMA_6360_Camping(int param_1, int param_2);
void ULTIMA_637e_DrawFrame(void);

int ULTIMA_6506_PlaceCombatant(int a, int b, int c, int d, int e);
void ULTIMA_6800_WakeCombatant(int a);
void ULTIMA_6880_SleepIfNotPoisoned(int param_1);
void ULTIMA_68ae_PutToSleep(int a);
void ULTIMA_6936_InitCombatants(void);
int ULTIMA_6d82_IsInView(int a, int b);
int ULTIMA_6da8_PlayerDefense(int a);
int ULTIMA_6e60_Unequip(int param_1, int param_2);

// (Grap)

int ULTIMA_6f1e_CompareWord(byte* param_1, char* param_2);
void ULTIMA_6bc2_SetUpCombat(int param_1, int param_2);
void ULTIMA_6f9e_TilesetModePlus3(int param_1);
void ULTIMA_6fbc_TilesetMode(int param_1);
void ULTIMA_6fd6_GRAP_AnimateTiles(void);
int ULTIMA_6ff0_DistanceFromCenter(register int param_1, register int param_2);

#endif
