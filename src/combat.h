#ifndef _COMBAT_H
#define _COMBAT_H

int COMBAT_0000_IsCellFree(int actorTile, int cellX, int cellY);
int COMBAT_0b94_MainLoop(void);
int COMBAT_111a_FireMapTrigger(int cellX, int cellY);
int COMBAT_120e_RandomCell(void);
void COMBAT_1236_RemoveEntity(int which);
int COMBAT_13e2_QueryStat(int combatant, int stat);
int COMBAT_14d6_RollToHit(int target, int attacker, int unused, int weapon);
int COMBAT_1574_ApplyDamage(int target, int damage);
void COMBAT_18ba_Poison(int target, int attacker);
void COMBAT_194a_ResolveHit(int target, int attacker);
int COMBAT_1a5c_WoundLevel(int combatant);

#endif
