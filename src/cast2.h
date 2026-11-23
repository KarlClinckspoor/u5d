#ifndef _CAST2_H
#define _CAST2_H

void CastEffect(int level);
int OnWho(void);
int SelectSpell(void);
int AskDirection(void);
int HealMember(int member);
void ChangeWindDirection(int direction, int fromScroll);
void WisAnYlem(void);
int KalXenCorp(int fromScroll);
int Resurrect(int memberIdx, int mode);
void PrintPosition(void);
int InExPor(void);
int AnGrav(int withEffect);
void SetLightTurns(int param_1);
void SetSpellStatus(int icon, int turns, int level);
void Shrine(void);
void SaveGame(void);

#endif
