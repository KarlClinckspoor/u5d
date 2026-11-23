#ifndef _DNGLOOK_H
#define _DNGLOOK_H

void LookCmdInDungeon(void);
void DnglookViewCmd(void);
void MarkRoomCleared(int room);
void ApplyClearedRooms(void);
void BuildWallRing(void);
void SetUpDungeonFight(void);
void StepAfterCombat(void);
void FreeDungeonImages(void);
void PlaceCombatants(int cellKind, int mode);

#endif
