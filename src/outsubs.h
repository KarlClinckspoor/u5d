#ifndef _OUTSUBS_H
#define _OUTSUBS_H

void LoadViewChunks(int stepX, int stepY);
void ShiftChunks(int stepX, int stepY);
char* GetWorldSavefile(void);
int EnterTown(char* verb);
void Falls(void);
void PlaceAmuletShards(void);
void StepOnBurningTile(void);
void StepOnPoisonTile(void);
void Apparition(void);

#endif
