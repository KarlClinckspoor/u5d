#ifndef _ULTIMA_4000_H
#define _ULTIMA_4000_H

void ULTIMA_400c_RingRegeneration();
int ULTIMA_4080_SetActivePlayer(int a);

// Audio cont.

void ULTIMA_4102_AudioPlayAmbientSfx(void);
void ULTIMA_433e_AudioFootstep(void);
void ULTIMA_4368_AudioVictoryTune(void);
void ULTIMA_43ae_AudioSweepTone(int a, int b, int c, int d);

// Game cont.

byte* ULTIMA_4402_GetTileAddr(int x, int y);
void ULTIMA_4552_AnimateActors(void);
void ULTIMA_475a_UpdateMoongates(void);
int ULTIMA_47f4_GoToMoonstone(int param_1);
int ULTIMA_48a8_EnterMoongate(void);
int ULTIMA_4988_GetActingPlayer(void);
void ULTIMA_4a84_DrawSunAndMoons(void);
void ULTIMA_4be8_ClearSky(void);

void ULTIMA_4c2a_PrintLeftCap(void);
void ULTIMA_4cce_PrintRightCap(void);
int ULTIMA_4d76_FindIndex(char* a, int b);
void ULTIMA_4daa_HideScrollArrow(void);
void ULTIMA_4dea_ShowScrollArrow(int a);
void ULTIMA_4e20_ClearStatsTitle(void);
void ULTIMA_4e50_ShowStatsTitle(char* a);
void ULTIMA_4efc_OpenStatsDivider(void);

void ULTIMA_4f3c_DrawStatsDivider(void);
void ULTIMA_4f7c_AdvanceTime(int a);

#endif
