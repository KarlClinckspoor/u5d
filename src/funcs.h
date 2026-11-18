#ifndef _FUNCS_H
#define _FUNCS_H

// The resident code (ULTIMA.EXE, CS:0000-72e0) by address: the runtime library, then the
// prototypes of the assembly modules' and the C files' functions.

// 017e..0877: the runtime library (SLIBCR.LIB, LIBH.LIB; src/u5d.lnk). tools/match.py and
// tools/rootmap.py read the names of its routines from these lines.

// 017e: _astart (crt0)
// 0230: _cinit
// 02f4: exit
// 037d: (dynamic near call)
// 038c: (dynamic far call)
// 03a0: aNuldiv (32bit division)
// 0402: memchr
// 0426: itoa
// 0442: aNlmul (32bit multiply)
// aNulmul == aNlmul?
// 0476: aNNalshl (32bit shl)
// 0496: aNldiv (32bit division)
// 0532: _NMSG_TEXT
// 055d: _NMSG_WRITE
// 0586: _FF_MSGBANNER
// 05a6: _setargv
// 0734: _setenvp
// 07a2: _nullcheck
// 07c4: _fptrap
// 07d6: _cxtoa
// 082a: aNlshl
// 0836: _myalloc

// CS:0000-0fff

// Grap

extern void ULTIMA_0878_RestoreVideoMode(void);
extern void ULTIMA_0892_InitializeVideoDriver(int a);
extern int ULTIMA_08e6_ClipRectCoord(int* x1, int* y1, int* x2, int* y2);
extern void ULTIMA_0a70_GRAP_2d_SetPenColor(int a);
extern void ULTIMA_0aa6_GRAP_3f_FillRect(int x1, int y1, int x2, int y2);
extern void ULTIMA_0ace_GRAP_18_TransferArea(int a, int b, int c, int d, int e, int f);
extern void ULTIMA_0b10_GRAP_Line(int x1, int y1, int x2, int y2);
extern void ULTIMA_0b86_GRAP_XorFillRect(int x1, int y1, int x2, int y2);
extern void* ULTIMA_0bae_LoadImageFile(char* file_name);
extern void ULTIMA_0be4_FreeImage(void* ptr);
extern void ULTIMA_0bfc_GRAP_63_PutMirrored(byte* param_1, int param_2, int param_3, int param_4, int param_5);
extern void ULTIMA_0c22_GRAP_0f_SelectPage(int a);
extern void ULTIMA_0c3c_SetClipWindow(int a, int b, int c, int d);
extern void ULTIMA_0c64_GRAP_30_Pset(int param_1, int param_2);
extern void ULTIMA_0c9c_GRAP_39_HorizLine(int x1, int y, int x2);
extern void ULTIMA_0cf2_GRAP_3c_VertLine(int a, int b, int c);
extern void ULTIMA_0d4c_GRAP_4b_PutImage(void* a, int b, int c, int d, int e);
extern int ULTIMA_0d72_AnimateOriginLogo(byte* a);
extern void ULTIMA_0de0_DetectVideo(void);
extern int ULTIMA_0e94_LoadVideoDriver(void);
extern int ULTIMA_0f2a_GRAP_06_AllocPageBuffer(void);
extern void ULTIMA_0f46_GRAP_66_Reveal(int x1, int y1, int x2, int y2);
extern void ULTIMA_0f6e_GRAP_1b_TransferFullscreen(int from, int to);
extern void ULTIMA_0f90_GRAP_LineTo(int a, int b);

extern byte* ULTIMA_0fae_LoadResourceFile(char* file_name);

extern void ULTIMA_0fdc_FreeBitImage(void* ptr);
extern int ULTIMA_0ff4_LoadTileset(char* a);

// CS:1000-1fff

// Grap cont.

extern void ULTIMA_102e_UnloadTileset(void);
extern void ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(byte* a, int b, int c, int d);
extern int ULTIMA_1068_RevealTile(int a, int b, int c);
extern void ULTIMA_10e0_GRAP_51_PutTile(uint a, int b, int c);
extern void ULTIMA_1112_GRAP_60_Moongate(int a, int b, int c);
extern int ULTIMA_1140_GRAP_6f_AnimateOriginLogo(void);

// Timer

extern void ULTIMA_1158_InitTimer(void);

// File

extern int ULTIMA_1674_TestOpenFile(char* file_name);
extern int ULTIMA_16a6_GetDefaultDrive(void);

// Text

extern void ULTIMA_16ba_PrintChar(uint a);
extern void ULTIMA_1850_PrintString(char* a);
extern void ULTIMA_1a3e_PrintNumber(int a, int b, char c);

// Keyboard

extern void ULTIMA_1b16_ClearKbdBuffer(void);
extern u16 ULTIMA_1b38_PollKeyWithCursor(void);

// Text cont.

extern void ULTIMA_1b94_SelectTextWindow(int a);
extern void ULTIMA_1bf2_SetTextPosition(int a, int b);
extern void ULTIMA_1c22_SetTextWindowSize(int a, int b, int c, int d, int e);
extern void ULTIMA_1c9e_SelectCharset(int a);
extern void ULTIMA_1cca_SetTextForegroundColor(int a);
extern int ULTIMA_1cee_GetCurrentTextY(void);
extern int ULTIMA_1d02_LoadCharset(char* a, int b);

// Keyboard cont.

extern int ULTIMA_1d5e_PollKey(void);
extern int ULTIMA_1dda_WaitForKeystroke(int a);

// Input

extern void ULTIMA_1e38_IntroGetString(char* buffer, int maxLength);

// File cont.

extern int ULTIMA_1eac_SetDefaultDrive(int param_1);

// Text

extern int ULTIMA_1f12_GetCurrentTextX(void);
extern void ULTIMA_1f26_SetTextBackgroundColor(int a);

// Input cont.

extern void ULTIMA_1fa0_Backspace(int x);

// CS:2000-2fff

// Utils

extern u16 ULTIMA_2032_ToUpper(u8 x);
extern uint ULTIMA_2056_GetTime(void);
extern void ULTIMA_207e_srand(uint a);
extern int ULTIMA_2092_RandomRange(int a, int b);
extern void ULTIMA_20c8_Delay(int shiftIndex, int loopCount);
extern void ULTIMA_20fa_WaitTicks(int a);
extern int ULTIMA_216c_strlen(char* str);

// Audio

extern void ULTIMA_2192_AudioPulse(int param_1, int param_2, int param_3, int param_4, int param_5);
extern void ULTIMA_223c_AudioWhiteNoise(uint rate, uint dur, uint limit);
extern void ULTIMA_22c0_AudioTone(uint freq, uint dur);
extern void ULTIMA_22e2_PcspkOn(uint freq);
extern void ULTIMA_230e_PcspkOff(void);

// File cont.

extern void far ULTIMA_2320_NoDiskSwapMessage(void);
extern void far ULTIMA_2322_DiskSwapMessage(void);

extern void ULTIMA_251e_SwitchDisks(int a);
extern void ULTIMA_256e_ReadFileFromDisk(char* fileName, void* addr, u16 size, u16 offset);
extern void ULTIMA_25d8_WriteFileToDisk(char* fileName, void* addr, u16 size);

// Input cont.

extern int ULTIMA_266c_GetChar(void);

// Game display

extern void ULTIMA_2884_UpdateGoldDisplay(void);
extern void ULTIMA_2900_UpdateVitalsDisplay(void);
extern void ULTIMA_2a28_InvertPlayerRow(int playerIndex);
extern void ULTIMA_2a52_DamagePlayer(int playerIndex, uint damage);
extern void ULTIMA_2aa8_DamageParty(void);
extern void ULTIMA_2ae8_EndTurn(void);
extern int ULTIMA_2c4c_IsWalkableTile(int actorTile, int targetTile);
extern int ULTIMA_2d7a_SelectPlayerOrNone(int a);
extern int ULTIMA_2e8e_SelectPlayer(void);
extern void ULTIMA_2e96_SetWindDirection(int a);
extern void ULTIMA_2f62_UpdateWindDirectionRandomly(void);
extern void ULTIMA_2fd0_SpringTrap(int playerIndex);

// CS:3000-3fff

extern void ULTIMA_3072_ShakeScreen(void);

// Game

extern int ULTIMA_3178_ProcessCommand(int key);
extern void ULTIMA_3522_HitEffectAt(int x, int y);
extern void ULTIMA_3564_HitEffectOnActor(int a);
extern int ULTIMA_35ec_SelectDirection(void);
extern int ULTIMA_368e_FindActorTileAtPos(int x, int y, int z);
extern int ULTIMA_3702_FindLastActorTileAt(int mapX, int mapY, int mapLevel);
extern int ULTIMA_38e4_FindActorSlot(void);
extern void ULTIMA_39cc_SetTile(byte new_tile_id, byte x, byte y);
extern int ULTIMA_39fc_GetFirstActivePartyMember(void);
extern void ULTIMA_3a74_SetActor(byte tile, byte animTile, byte mapX, byte mapY, byte mapLevel, byte value, int slot);
extern int ULTIMA_3aae_Random(int a);
extern int ULTIMA_3abe_Random1To30(void);
extern void ULTIMA_3ae6_WaitFrames(int a);
extern void ULTIMA_3b1c_GetString(char* buffer, int maxLength);
extern int ULTIMA_3b9e_GetNumber(int maxLen);

extern void ULTIMA_3ef0_IncreaseByte(byte* value, int amount, int limit);
extern void ULTIMA_3f14_IncreaseInt(s16* a, int b, int c);
extern void ULTIMA_3f36_DecreaseByte(byte* value, int amount);
extern void ULTIMA_3f54_DecreaseInt(s16* a, int b);
extern bool ULTIMA_3f6e_TestViewTileFlag(int viewX, int viewY);
extern void ULTIMA_3fb4_PixelToViewCell(int pixelX, int pixelY);

// CS:4000-4fff

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
int ULTIMA_47f4_GoToMoonstone(int stoneIndex);
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

// CS:5000-5fff

// Game cont.

void ULTIMA_5394_PutActorsInView(void);
int ULTIMA_5646_IsEnemy(int a);
void ULTIMA_56ac_DrawMap(void);
void ULTIMA_5910_UpdateFrame(void);

void ULTIMA_5d0a_BuildView(int a, int x, int y, int d);
void ULTIMA_5e4a_BuildLightMap(void);
void ULTIMA_5f86_SpecialMapHandler(int mapFlags, int actorOrGuard, int hours);

// CS:6000-6fff

// Game cont.

void ULTIMA_6150_Combat(int a);
void ULTIMA_6360_Camping(int guard, int hours);
void ULTIMA_637e_DrawFrame(void);

int ULTIMA_6506_PlaceCombatant(int a, int b, int c, int d, int e);
void ULTIMA_6800_WakeCombatant(int a);
void ULTIMA_6880_SleepIfNotPoisoned(int combatantIndex);
void ULTIMA_68ae_PutToSleep(int a);
void ULTIMA_6936_InitCombatants(void);
int ULTIMA_6d82_IsInView(int a, int b);
int ULTIMA_6da8_PlayerDefense(int a);
int ULTIMA_6e60_Unequip(int playerIndex, int item);

// (Grap)

int ULTIMA_6f1e_CompareWord(byte* word, char* text);
void ULTIMA_6bc2_SetUpCombat(int flags, int enemy);
void ULTIMA_6f9e_TilesetModePlus3(int mode);
void ULTIMA_6fbc_TilesetMode(int mode);
void ULTIMA_6fd6_GRAP_AnimateTiles(void);
int ULTIMA_6ff0_DistanceFromCenter(register int viewX, register int viewY);

// CS:7000-7fff

// Game cont.

void ULTIMA_70a6_AnimateLighthouse(void);

// Grap cont.

void ULTIMA_71aa_UpdateClockTiles(void);
void ULTIMA_71ca_DRV_27_ScrollTextWindow1(int left, int top, int right, int bottom, int rows);
void ULTIMA_7200_DRV_27_ScrollTextWindow2(int left, int top, int right, int bottom, int rows);

// File

int ULTIMA_7234_ReadFile(char* fileName, void* addr, u16 size, u16 offset);
int ULTIMA_7296_WriteFile(char* fileName, void* addr, u16 size);

#endif
