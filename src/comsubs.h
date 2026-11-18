#ifndef _COMSUBS_H
#define _COMSUBS_H

int COMSUBS_0000_TargetResists(int attacker, int target, int weaponKind);
void COMSUBS_0056_FadeFields(void);
void COMSUBS_0094_PrintName(int combatant);
int COMSUBS_00f4_MonsterSpecial(int combatant);
void COMSUBS_0312_ReportHit(int target, int attacker);
int COMSUBS_048a_Distance(int fromX, int fromY, int toX, int toY);
int COMSUBS_04d4_CombatantDistance(int fromIdx, int toIdx);
int COMSUBS_0504_AimCrosshair(int shooter, int maxRange);
int COMSUBS_0748_CombatantAt(int cellX, int cellY);
void COMSUBS_07d4_PickNearbyCell(int centerX, int centerY);
int COMSUBS_0822_ResolveShot(int shooter, int aimX, int aimY, int onTarget, int missileKind);
int COMSUBS_09fc_EnemyInterferes(int combatant);
void COMSUBS_0bf8_MeleeBlow(int attacker, int victim, int weapon);
void COMSUBS_0c52_AttackWith(int attacker, int weapon);
void COMSUBS_0d96_AttackCmd(int attacker, int weaponCount);
int COMSUBS_12de_FlyMissile(int fromX, int fromY, int toX, int toY, int missileKind);

#endif
