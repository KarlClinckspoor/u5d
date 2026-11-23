#ifndef _COMBAT_H
#define _COMBAT_H

int IsCellFree(int actorTile, int cellX, int cellY);
int CombatMainLoop(void);
int FireMapTrigger(int cellX, int cellY);
int RandomCell(void);
void RemoveEntity(int which);
int QueryStat(int combatant, int stat);
int RollToHit(int target, int attacker, int unused, int weapon);
int ApplyDamage(int target, int damage);
void CombatPoison(int target, int attacker);
void ResolveHit(int target, int attacker);
int WoundLevel(int combatant);

#endif
