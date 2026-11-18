#ifndef _OUTSUBS_H
#define _OUTSUBS_H

void OUTSUBS_01b4_LoadViewChunks(int stepX, int stepY);
void OUTSUBS_02c8_ShiftChunks(int stepX, int stepY);
char* OUTSUBS_0368_GetWorldSavefile(void);
int OUTSUBS_0388_EnterTown(char* verb);
void OUTSUBS_0458_Falls(void);
void OUTSUBS_0566_PlaceAmuletShards(void);
void OUTSUBS_05ee_StepOnBurningTile(void);
void OUTSUBS_05fc_StepOnPoisonTile(void);
void OUTSUBS_0658_Apparition(void);

#endif
