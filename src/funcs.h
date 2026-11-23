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

extern void RestoreVideoMode(void);
extern void InitializeVideoDriver(int a);
extern int ClipRectCoord(int* x1, int* y1, int* x2, int* y2);
extern void GRAP_2d_SetPenColor(int a);
extern void GRAP_3f_FillRect(int x1, int y1, int x2, int y2);
extern void GRAP_18_TransferArea(int a, int b, int c, int d, int e, int f);
extern void GRAP_Line(int x1, int y1, int x2, int y2);
extern void GRAP_XorFillRect(int x1, int y1, int x2, int y2);
extern void* LoadImageFile(char* file_name);
extern void FreeImage(void* ptr);
extern void GRAP_63_PutMirrored(byte* param_1, int param_2, int param_3, int param_4, int param_5);
extern void GRAP_0f_SelectPage(int a);
extern void SetClipWindow(int a, int b, int c, int d);
extern void GRAP_30_Pset(int param_1, int param_2);
extern void GRAP_39_HorizLine(int x1, int y, int x2);
extern void GRAP_3c_VertLine(int a, int b, int c);
extern void GRAP_4b_PutImage(void* a, int b, int c, int d, int e);
extern int AnimateOriginLogo(byte* a);
extern void DetectVideo(void);
extern int LoadVideoDriver(void);
extern int GRAP_06_AllocPageBuffer(void);
extern void GRAP_66_Reveal(int x1, int y1, int x2, int y2);
extern void GRAP_1b_TransferFullscreen(int from, int to);
extern void GRAP_LineTo(int a, int b);

extern byte* LoadResourceFile(char* file_name);

extern void FreeBitImage(void* ptr);
extern int LoadTileset(char* a);

// CS:1000-1fff

// Grap cont.

extern void UnloadTileset(void);
extern void GRAP_4e_CopyBitImageIntoPage(byte* a, int b, int c, int d);
extern int ULTIMA_1068_RevealTile(int a, int b, int c);
extern void GRAP_51_PutTile(uint a, int b, int c);
extern void GRAP_60_Moongate(int a, int b, int c);
extern int GRAP_6f_AnimateOriginLogo(void);

// Timer

extern void InitTimer(void);

// File

extern int TestOpenFile(char* file_name);
extern int GetDefaultDrive(void);

// Text

extern void PrintChar(uint a);
extern void PrintString(char* a);
extern void PrintNumber(int a, int b, char c);

// Keyboard

extern void ClearKbdBuffer(void);
extern u16 PollKeyWithCursor(void);

// Text cont.

extern void SelectTextWindow(int a);
extern void SetTextPosition(int a, int b);
extern void SetTextWindowSize(int a, int b, int c, int d, int e);
extern void SelectCharset(int a);
extern void SetTextForegroundColor(int a);
extern int GetCurrentTextY(void);
extern int LoadCharset(char* a, int b);

// Keyboard cont.

extern int PollKey(void);
extern int WaitForKeystroke(int a);

// Input

extern void IntroGetString(char* buffer, int maxLength);

// File cont.

extern int SetDefaultDrive(int param_1);

// Text

extern int GetCurrentTextX(void);
extern void SetTextBackgroundColor(int a);

// Input cont.

extern void Backspace(int x);

// CS:2000-2fff

// Utils

extern u16 ToUpperChar(u8 x);
extern uint GetTime(void);
extern void SeedRandom(uint a);
extern int RandomRange(int a, int b);
extern void ULTIMA_20c8_Delay(int shiftIndex, int loopCount);
extern void WaitTicks(int a);
extern int StringLength(char* str);

// Audio

extern void AudioPulse(int param_1, int param_2, int param_3, int param_4, int param_5);
extern void AudioWhiteNoise(uint rate, uint dur, uint limit);
extern void AudioTone(uint freq, uint dur);
extern void PcspkOn(uint freq);
extern void PcspkOff(void);

// File cont.

extern void far NoDiskSwapMessage(void);
extern void far DiskSwapMessage(void);

extern void SwitchDisks(int a);
extern void ReadFileFromDisk(char* fileName, void* addr, u16 size, u16 offset);
extern void WriteFileToDisk(char* fileName, void* addr, u16 size);

// Input cont.

extern int GetChar(void);

// Game display

extern void UpdateGoldDisplay(void);
extern void UpdateVitalsDisplay(void);
extern void InvertPlayerRow(int playerIndex);
extern void DamagePlayer(int playerIndex, uint damage);
extern void DamageParty(void);
extern void EndTurn(void);
extern int IsWalkableTile(int actorTile, int targetTile);
extern int SelectPlayerOrNone(int a);
extern int SelectPlayer(void);
extern void SetWindDirection(int a);
extern void UpdateWindDirectionRandomly(void);
extern void SpringTrap(int playerIndex);

// CS:3000-3fff

extern void ShakeScreen(void);

// Game

extern int ProcessCommand(int key);
extern void HitEffectAt(int x, int y);
extern void HitEffectOnActor(int a);
extern int SelectDirection(void);
extern int FindActorTileAtPos(int x, int y, int z);
extern int FindLastActorTileAt(int mapX, int mapY, int mapLevel);
extern int FindActorSlot(void);
extern void SetTile(byte new_tile_id, byte x, byte y);
extern int GetFirstActivePartyMember(void);
extern void SetActor(byte tile, byte animTile, byte mapX, byte mapY, byte mapLevel, byte value, int slot);
extern int Random(int a);
extern int Random1To30(void);
extern void WaitFrames(int a);
extern void GetString(char* buffer, int maxLength);
extern int GetNumber(int maxLen);

extern void IncreaseByte(byte* value, int amount, int limit);
extern void IncreaseInt(s16* a, int b, int c);
extern void DecreaseByte(byte* value, int amount);
extern void DecreaseInt(s16* a, int b);
extern bool TestViewTileFlag(int viewX, int viewY);
extern void PixelToViewCell(int pixelX, int pixelY);

// CS:4000-4fff

void RingRegeneration();
int SetActivePlayer(int a);

// Audio cont.

void AudioPlayAmbientSfx(void);
void AudioFootstep(void);
void AudioVictoryTune(void);
void AudioSweepTone(int a, int b, int c, int d);

// Game cont.

byte* GetTileAddr(int x, int y);
void AnimateActors(void);
void UpdateMoongates(void);
int GoToMoonstone(int stoneIndex);
int EnterMoongate(void);
int GetActingPlayer(void);
void DrawSunAndMoons(void);
void ClearSky(void);

void PrintLeftCap(void);
void PrintRightCap(void);
int FindIndex(char* a, int b);
void HideScrollArrow(void);
void ShowScrollArrow(int a);
void ClearStatsTitle(void);
void ShowStatsTitle(char* a);
void OpenStatsDivider(void);

void DrawStatsDivider(void);
void AdvanceTime(int a);

// CS:5000-5fff

// Game cont.

void PutActorsInView(void);
int ULTIMA_5646_IsEnemy(int a);
void DrawMap(void);
void UpdateFrame(void);

void BuildView(int a, int x, int y, int d);
void BuildLightMap(void);
void SpecialMapHandler(int mapFlags, int actorOrGuard, int hours);

// CS:6000-6fff

// Game cont.

void Combat(int a);
void Camping(int guard, int hours);
void DrawFrame(void);

int PlaceCombatant(int a, int b, int c, int d, int e);
void WakeCombatant(int a);
void SleepIfNotPoisoned(int combatantIndex);
void PutToSleep(int a);
void InitCombatants(void);
int IsInView(int a, int b);
int PlayerDefense(int a);
int Unequip(int playerIndex, int item);

// (Grap)

int CompareWord(byte* word, char* text);
void SetUpCombat(int flags, int enemy);
void TilesetModePlus3(int mode);
void TilesetMode(int mode);
void GRAP_AnimateTiles(void);
int DistanceFromCenter(register int viewX, register int viewY);

// CS:7000-7fff

// Game cont.

void AnimateLighthouse(void);

// Grap cont.

void UpdateClockTiles(void);
void DRV_27_ScrollTextWindow1(int left, int top, int right, int bottom, int rows);
void DRV_27_ScrollTextWindow2(int left, int top, int right, int bottom, int rows);

// File

int ReadFile(char* fileName, void* addr, u16 size, u16 offset);
int WriteFile(char* fileName, void* addr, u16 size);

#endif
