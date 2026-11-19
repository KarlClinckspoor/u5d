#ifndef _NPC_H
#define _NPC_H

void LoadNpcFile(void);
void NPC_00d6_PlaceNpcs(int hour);
void MoveNpcs(int hour);
int ScheduleSlot(int npc, char hour);

#endif
