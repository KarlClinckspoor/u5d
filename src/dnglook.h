#ifndef _DNGLOOK_H
#define _DNGLOOK_H

void DNGLOOK_0000_LookCmdInDungeon(void);
void DNGLOOK_06a8_ViewCmd(void);
void DNGLOOK_0844_MarkRoomCleared(int room);
void DNGLOOK_093a_ApplyClearedRooms(void);
void DNGLOOK_0c6c_BuildWallRing(void);
void DNGLOOK_0d3e_SetUpDungeonFight(void);
void DNGLOOK_0fda_StepAfterCombat(void);
void DNGLOOK_1130_FreeDungeonImages(void);
void DNGLOOK_117e_PlaceCombatants(int cellKind, int mode);

#endif
