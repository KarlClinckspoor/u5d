#ifndef _ULTIMA_3000_H
#define _ULTIMA_3000_H

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

#endif
