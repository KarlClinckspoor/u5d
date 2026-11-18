#ifndef _CAST2_H
#define _CAST2_H

void CAST2_0000_CastEffect(int level);
int CAST2_009e_OnWho(void);
int CAST2_00de_SelectSpell(void);
int CAST2_0306_AskDirection(void);
int CAST2_03c2_HealMember(int member);
void CAST2_040a_ChangeWindDirection(int direction, int fromScroll);
void CAST2_046c_WisAnYlem(void);
int CAST2_04c2_KalXenCorp(int fromScroll);
int CAST2_05e0_Resurrect(int memberIdx, int mode);
void CAST2_06ec_PrintPosition(void);
int CAST2_0768_InExPor(void);
int CAST2_07bc_AnGrav(int withEffect);
void CAST2_08ea_SetLightTurns(int param_1);
void CAST2_08f8_SetSpellStatus(int icon, int turns, int level);
void CAST2_0e76_Shrine(void);
void CAST2_10fe_SaveGame(void);

#endif
