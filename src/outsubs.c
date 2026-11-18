#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "audio/aud_mus.h"

#include "outsubs.h"

u8 D_3866_dungeonChunks[8] = {0x4F, 0x45, 0xA4, 0x17, 0x19, 0x63, 0xFE, 0x88};

u8 D_386e_shrineChunks[8] = {0x4E, 0x58, 0xE2, 0x04, 0x2C, 0xC5, 0x00, 0xDE};

u8 D_3876_chunkFileIndex[0x100] = {
    0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0xFF, 0x0C,
    0xFF, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0xFF,
    0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22,
    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0xFF,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41,
    0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51,
    0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61,
    0x62, 0x63, 0x64, 0xFF, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0xFF,
    0xFF, 0xFF, 0x70, 0x71, 0xFF, 0x72, 0x73, 0x74,
    0xFF, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xFF,
    0xFF, 0xFF, 0x7B, 0x7C, 0xFF, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0xFF,
    0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x88, 0x89, 0xFF,
    0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0xFF,
    0x91, 0xFF, 0xFF, 0x92, 0x93, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x9A, 0x9B,
    0xFF, 0x9C, 0x9D, 0xFF, 0x9E, 0x9F, 0xA0, 0xA1,
    0xFF, 0xFF, 0xA2, 0xA3, 0xFF, 0xA4, 0xA5, 0xA6,
    0xFF, 0xA7, 0xA8, 0xFF, 0xA9, 0xAA, 0xAB, 0xAC,
    0xFF, 0xFF, 0xAD, 0xAE, 0xFF, 0xAF, 0xB0, 0xB1,
    0xFF, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8,
    0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0,
    0xFF, 0xFF, 0xFF, 0xC1, 0xC2, 0xFF, 0xC3, 0xC4,
    0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC
};

// the chunk of a dungeon (table D_3866) whose dungeon is sealed (D_58d0 == 0)? 1 when not in the table too
static int OUTSUBS_0000_DungeonSealedAt(uint chunkWord)
{
    int index;
    int sealed = 1;

    chunkWord = (byte)(chunkWord >> 8);

    for (index = 0; index < 8; index++)
    {
        if (D_3866_dungeonChunks[index] == chunkWord)
        {
            sealed = D_58d0_dungeonOpen[index] == 0;
            break;
        }
    }

    return sealed;
}

// the chunk is a shrine's (D_386e) and the shrine is destroyed (bit 7 of D_58d8)
static int OUTSUBS_004a_ShrineDestroyedAt(int chunkWord)
{
    int index_i;
    int destroyed = 0;

    chunkWord >>= 8;

    for (index_i = 0; index_i < 8; index_i++)
    {
        // 0063
        if (D_386e_shrineChunks[index_i] == chunkWord)
        {
            destroyed = 0x7f < D_58d8_shrineDestroyed[index_i];
            break;
        }
    }

    return destroyed;
}

// reads one 16x16 chunk into slot 0-3 of D_6608; sealed dungeons and destroyed shrines get their changed tile
static void OUTSUBS_0098_LoadChunk(char* fileName, int slot, int chunkWord)
{
    int row;
    int col_g;
    int baseY;
    int baseX;
    int fileOffset_j;

    if (*fileName == 'B') // "BRIT.DAT"
    {
        fileOffset_j = D_3876_chunkFileIndex[(byte)(chunkWord >> 8)];
        if (fileOffset_j == 0xff)
        {
            memset(&D_6608[slot * 0x100], 1, 0x100);
            return; // -> 01ac
        }

        // 00d4
        fileOffset_j <<= 8;
    }
    else
    {
        // 00dc
        fileOffset_j = chunkWord;
    }

    // 00e2
    if (D_5895_mapLevel != 0)
    {
        ULTIMA_251e_SwitchDisks(5);
    }

    // 00f0
    ULTIMA_256e_ReadFileFromDisk(fileName, D_6608 + slot * 0x100, 0x100, fileOffset_j);
    baseX = chunkWord >> 4 & 0xf0;
    baseY = chunkWord >> 8 & 0xf0;

    for (row = 0; row < 0x10; row++)
    {
        // 0125
        for (col_g = 0; col_g < 0x10; col_g++)
        {
            switch (GetMapOverworld(col_g, row, slot))
            {
            case 0x16:
            case 0x17:
            case 0x18:
                // 012c
                if (OUTSUBS_0000_DungeonSealedAt(chunkWord) != 0)
                {
                    *ULTIMA_4402_GetTileAddr(col_g + baseX, row + baseY) = TILE_MAP_DF;
                }
                break;
            case 0x19:
                // 017d
                if (OUTSUBS_004a_ShrineDestroyedAt(chunkWord) != 0)
                {
                    *ULTIMA_4402_GetTileAddr(col_g + baseX, row + baseY) = TILE_MAP_RUINS;
                }
                break;
            }
        }
    }
}

// loads the chunks that come into view (-1/1 steps in x and y); finds the lighthouse beacon (tile 0x1b) in the four
void OUTSUBS_01b4_LoadViewChunks(int stepX, int stepY)
{
    int beaconOffset;
    int originWord_k;
    byte* beaconPtr;
    int rightStep;
    char* fileName;

    originWord_k = D_589b_mapOriginX * 0x10 + D_589c_mapOriginY * 0x100;

    if (D_589b_mapOriginX == 0xf0)
    {
        rightStep = -0xf00; // 0xf100?
    }
    else
    {
        // 01dc
        rightStep = 0x100;
    }

    // 01e1
    if (D_5895_mapLevel > 0x7f)
    {
        fileName = /*0x3976*/ "UNDER.DAT";
    }
    else
    {
        fileName = /*0x3980*/ "BRIT.DAT";
    }

    // 01f5
    if (stepX == -1 || stepY == -1)
    {
        OUTSUBS_0098_LoadChunk(fileName, 0, originWord_k);
    }
    // 020d
    if (stepX == 1 || stepY == -1)
    {
        OUTSUBS_0098_LoadChunk(fileName, 1, originWord_k + rightStep);
    }
    // 022a
    if (stepX == -1 || stepY == 1)
    {
        OUTSUBS_0098_LoadChunk(fileName, 2, originWord_k + 0x1000);
    }
    // 0247
    if (stepX == 1 || stepY == 1)
    {
        OUTSUBS_0098_LoadChunk(fileName, 3, (originWord_k + rightStep) + 0x1000);
    }

    // 0267
    beaconPtr = memchr(D_6608, 0x1b, 0x400);
    if (beaconPtr == 0)
    {
        D_217e_beaconX = D_2180_beaconY = -1;
    }
    else
    {
        // 028c
        beaconOffset = (int)(beaconPtr - D_6608);
        D_217e_beaconX = beaconOffset & 0xf;
        D_2180_beaconY = ((byte)beaconOffset & 0xf0) >> 4;
        if ((beaconOffset & 0x100) != 0)
        {
            D_217e_beaconX += 0x10;
        }
        if ((beaconOffset & 0x200) != 0)
        {
            D_2180_beaconY += 0x10;
        }
    }
}

// after a scroll, copies the two chunks that stay in view to their new slots
void OUTSUBS_02c8_ShiftChunks(int stepX, int stepY)
{
    int srcSlotB;
    int srcSlotA;

    int dstSlotB_b;
    int dstSlotA_c;

    dstSlotA_c = stepX < 0 || stepY < 0 ? 3 : 0;
    srcSlotA = stepX < 0 || stepY > 0 ? 2 : 1;

    dstSlotB_b = dstSlotA_c + stepY + stepX * 2;
    srcSlotB = srcSlotA + stepY + stepX * 2;

    memcpy(dstSlotA_c * 0x100 + D_6608, srcSlotA * 0x100 + D_6608, 0x100);
    memcpy(dstSlotB_b * 0x100 + D_6608, srcSlotB * 0x100 + D_6608, 0x100);
}

// the saved-map file of the level: BRIT.OOL on the surface, UNDER.OOL below
char* OUTSUBS_0368_GetWorldSavefile(void)
{
    char* fileName;
    if (D_5895_mapLevel == 0)
    {
        fileName = /*0x3989*/ "BRIT.OOL";
    }
    else
    {
        fileName = /*0x3992*/ "UNDER.OOL";
    }

    return fileName;
}

// prints the verb, finds the town at the party's position (D_1e8a/D_1eb2) and enters it: 0, or 1 with "What town?"
int OUTSUBS_0388_EnterTown(char* verb)
{
    int placeIndex;
    int failed;

    failed = 1;
    ULTIMA_1850_PrintString(verb);

    for (placeIndex = 0; placeIndex < 0x20; placeIndex++)
    {
        if (D_1e8a_placeX[placeIndex] == D_5896_mapX && (D_1eb2_placeY[placeIndex] == D_5897_mapY))
            break;
    }

    if (placeIndex < 0x20)
    {
        if (placeIndex < 0xd || placeIndex > 0x11)
        {
            ULTIMA_1850_PrintString(_TEXT(0x399c, "\n\n"));
            ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
            ULTIMA_1850_PrintString(D_1e3a_mapNames[placeIndex]);
            ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
        }

        ULTIMA_16ba_PrintChar('\n');
        if (D_a9bd_curDisk[0] != 1)
        {
            ULTIMA_251e_SwitchDisks(1);

            while (ULTIMA_1674_TestOpenFile(/*0x399f*/ "BRIT.DAT") == 0)
                ;
        }

        ULTIMA_25d8_WriteFileToDisk(OUTSUBS_0368_GetWorldSavefile(), D_5c5a_actors, 0x100);
        D_5893_mapId = placeIndex + 1;
        D_5895_mapLevel = 0;
        D_5896_mapX = 0xf;
        D_5897_mapY = 0x1e;
        failed = 0;
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x39a8, "\nWhat town?\n"));
    }

    return failed;
}

void MAINOUT_0000_SetUpOverworld(void);
void MAINOUT_0354_MoveParty(int stepX, int stepY);

u8 D_3a06_shardX[4] = { 0xC0, 0x82, 0xB0 };

u8 D_3a0a_shardY[4] = { 0x50, 0x41, 0xB8 };

u8 D_3a0e_shardValue[3] = { 0xF0, 0xF1, 0xF2 };

// "F-A-L-L-S!!!": hurts the party, and at the one hole (0x36, 0x8a) falls into the underworld
void OUTSUBS_0458_Falls(void)
{
    int savedTile;
    int member_i;

    ULTIMA_1850_PrintString(_TEXT(0x39b5, "F-A-L-L-S!!!\n"));
    MAINOUT_0354_MoveParty(0, 1);
    ULTIMA_3ae6_WaitFrames(1);
    MAINOUT_0354_MoveParty(0, 1);
    ULTIMA_43ae_AudioSweepTone(0x9c4, 800, 1, 300);

    savedTile = D_587c_partyTile;
    D_587c_partyTile = 0;

    ULTIMA_3ae6_WaitFrames(1);

    for (member_i = 0; member_i < D_585b_partyCount; member_i++)
    {
        if (D_55a8_party[member_i].status != STATUS_DEAD && D_55a8_party[member_i].dex <= ULTIMA_3abe_Random1To30())
        {
            ULTIMA_2a52_DamagePlayer(member_i, 1);
        }
    }

    ULTIMA_3ae6_WaitFrames(2);
    D_587c_partyTile = savedTile;

    if (D_5896_mapX == 0x36 && D_5897_mapY == 0x8a)
    {
        ULTIMA_1850_PrintString(_TEXT(0x39c3, "Falling into underworld!!\n"));

        D_5895_mapLevel = 0xff;

        ULTIMA_25d8_WriteFileToDisk(/*0x39de*/ "BRIT.OOL", D_5c5a_actors, 0x100);
        ULTIMA_256e_ReadFileFromDisk(/*0x39e7*/ "UNDER.OOL", D_5c5a_actors, 0x100, 0);
        ULTIMA_251e_SwitchDisks(5);

        while (ULTIMA_1674_TestOpenFile(/*0x39f1*/ "UNDER.DAT") == 0)
            ;

        ULTIMA_25d8_WriteFileToDisk(/*0x39fb*/ "UNDER.OOL", D_5c5a_actors, 0x100);
        MAINOUT_0000_SetUpOverworld(); // THUNK 7b7e
    }
}

// in the underworld: the Amulet's piece and the shards not yet taken (Shadowlord alive) are actors on the map
void OUTSUBS_0566_PlaceAmuletShards(void)
{
    int index;
    ActorFmt* actor_h;

    if (D_5895_mapLevel != 0)
    {
        if (D_57b3_amulet == 0)
        {
            // D_5c5a_actors[28] == 5d3a
            actor_h = &D_5c5a_actors[28];
            actor_h->tile = actor_h->animTile = TILE_ACTOR_AMULET_LB;
            actor_h->x = 0x69;
            actor_h->y = 0xe1;
            actor_h->z = 0xff;
            actor_h->value = 0xf3;
        }

        // 059d
        for (index = 0; index < 3; index++)
        {
            if (D_57b6_shards[index] == 0 && D_58c8_shadowlordTowns[index] < 0x80)
            {
                // D_5c5a_actors[29] == 5d42
                actor_h = &D_5c5a_actors[index + 29];
                actor_h->tile = actor_h->animTile = TILE_ACTOR_SHARD;
                actor_h->x = D_3a06_shardX[index];
                actor_h->y = D_3a0a_shardY[index];
                actor_h->z = 0xff;
                actor_h->value = D_3a0e_shardValue[index];
            }
        }
    }
}

// "Burning!": the party is damaged
void OUTSUBS_05ee_StepOnBurningTile(void)
{
    ULTIMA_5910_UpdateFrame();
    ULTIMA_1850_PrintString(_TEXT(0x3a11, "Burning!\n"));
    ULTIMA_2aa8_DamageParty();
}

// each living member may be poisoned (dexterity against a roll of 1 to 30)
void OUTSUBS_05fc_StepOnPoisonTile(void)
{
    int member;

    for (member = 0; member < D_585b_partyCount; member++)
    {
        if (D_55a8_party[member].status != STATUS_DEAD && D_55a8_party[member].status != STATUS_POISONED)
        {
            if (D_55a8_party[member].dex < ULTIMA_2092_RandomRange(1, 0x1e))
            {
                D_55a8_party[member].status = STATUS_POISONED;
                ULTIMA_1850_PrintString(_TEXT(0x3a1b, "Poisoned!\n"));
            }
        }
    }
}
