#ifndef _COMSUBS_H
#define _COMSUBS_H

int TargetResists(int attacker, int target, int weaponKind);
void FadeFields(void);
void PrintName(int combatant);
int MonsterSpecial(int combatant);
void ReportHit(int target, int attacker);
int ComsubsDistance(int fromX, int fromY, int toX, int toY);
int CombatantDistance(int fromIdx, int toIdx);
int AimCrosshair(int shooter, int maxRange);
int CombatantAt(int cellX, int cellY);
void PickNearbyCell(int centerX, int centerY);
int ResolveShot(int shooter, int aimX, int aimY, int onTarget, int missileKind);
int EnemyInterferes(int combatant);
void MeleeBlow(int attacker, int victim, int weapon);
void AttackWith(int attacker, int weapon);
void ComsubsAttackCmd(int attacker, int weaponCount);
int FlyMissile(int fromX, int fromY, int toX, int toY, int missileKind);

#endif
