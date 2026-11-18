#ifndef _NPC_H
#define _NPC_H

void NPC_0000_LoadNpcFile(void);
void NPC_00d6_PlaceNpcs(int hour);
void NPC_0db4_MoveNpcs(int hour);
int NPC_12e0_ScheduleSlot(int npc, char hour);

#endif
