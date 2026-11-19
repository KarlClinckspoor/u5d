#include "common/common.h"

// this module's communal variables (vars.h, DS:a9bc)
u8 stripLeft;
u8 stripRight;
u8 stripWiden;
u8 sceneNo;
u8 virtueOffered[8];
u8 virtueRejected[8];
u8 D_bd3a_unused; // no code refers to it
u8 D_bd3b_unused; // no code refers to it
u8 newInt;
u8 newDex;
u8 newStr;

#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include <string.h>

#include "font.h"
#include "intro.h"
#include "tiles.h"

// glyph widths from ' ' (never read: spaces are spaceWidth wide) to 'z'
u8 glyphWidths[0x5b] =
{
    0x00,
    0x02, 0x06, 0x07, 0x07, 0x07, 0x07, 0x03, 0x04,
    0x04, 0x05, 0x06, 0x03, 0x03, 0x02, 0x07, 0x06,
    0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x02, 0x03, 0x05, 0x05, 0x05, 0x06, 0x07,
    0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x04, 0x07, 0x07, 0x06, 0x07, 0x06, 0x06, 0x07,
    0x06, 0x07, 0x06, 0x06, 0x06, 0x06, 0x07, 0x06,
    0x06, 0x06, 0x03, 0x07, 0x03, 0x07, 0x08, 0x03,
    0x05, 0x05, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x02, 0x03, 0x05, 0x02, 0x07, 0x06, 0x05, 0x05,
    0x05, 0x04, 0x04, 0x04, 0x05, 0x05, 0x07, 0x05,
    0x05, 0x04
};

u16 textLeftX[2] = {0, 0};
u16 D_514a_unused = 0; // no code refers to it

u16 textRightX[2] = {0x0140, 0x0140};

u16 D_5150_regionTop = 0x00c8;

u16 D_5152_regionBottom = 0x00c8;

u16 spaceWidth = 5;

u16 D_5156_penX = 0;

u16 D_5158_penY = 0x003c;
u16 toneStep = 0;

char* sceneCaptions[4] =
{
    /*0xa010*/ "The Summoning",
    /*0xa01e*/ "The Journey",
    /*0xa02a*/ "The Arrival",
    /*0xa036*/ "The Welcoming"
};

u8 intBonus[8] = {0x02, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00};

u8 dexBonus[8] = {0x00, 0x02, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00};

u8 strBonus[8] = {0x00, 0x00, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00};

u16 questionOffset[8][8] =
{
    0x0000, 0x06c2, 0x07a7, 0x088c, 0x0960, 0x0a47, 0x0b2d, 0x0c11,
    0x06c2, 0x0000, 0x0cda, 0x0d8f, 0x0e77, 0x0f56, 0x1039, 0x111b,
    0x07a7, 0x0cda, 0x0000, 0x11ff, 0x12db, 0x1374, 0x1456, 0x14ed,
    0x088c, 0x0d8f, 0x11ff, 0x0000, 0x15d1, 0x16af, 0x1794, 0x185b,
    0x0960, 0x0e77, 0x12db, 0x15d1, 0x0000, 0x192f, 0x1a18, 0x1b00,
    0x0a47, 0x0f56, 0x1374, 0x16af, 0x192f, 0x0000, 0x1be3, 0x1cbb,
    0x0b2d, 0x1039, 0x1456, 0x1794, 0x1a18, 0x1be3, 0x0000, 0x1d87,
    0x0c11, 0x111b, 0x14ed, 0x185b, 0x1b00, 0x1cbb, 0x1d87, 0x0000
};

u8 cardX[8] = {0x28, 0x30, 0x30, 0x28, 0x28, 0x30, 0x28, 0x30};

u8 cardY[8] = {0x05, 0x07, 0x04, 0x0a, 0x08, 0x00, 0x05, 0x06};

// The DOS build takes DrawJustifiedText from src/asm/AFONT0.ASM (assembly written from the
// original's bytes, linked before this object): its C reconstruction does not match yet
// (670 of 674 bytes) and is kept in research/font0000.c.

// Draws one cell of the scene strip: the map tile, or else the actor's tile that AnimateScene left
// under it.
static void DrawStripCell(int x, int y)
{
    byte tile;

    tile = GetMap(x, y);
    if (tile != 0)
    {
        GRAP_51_PutTile(tileFrames[tile], x, y + 7);
        return;
    }
    else
    {
        tile = D_6608[x + y * 0x20 + 0x80];
        if (tile == 0x16)
        {
            return;
        }

        GRAP_51_PutTile(0x100 + tile, x, y + 7);
    }
}

// Runs the scene's animation for that many frames: actors animate, the strip widens, white noise or tones
// sound; returns 1 when a key was pressed.
// CHECKED
// The decompiler's lineWidth (x + y * 0x20, stored at bp-0Ah and never read) is MSC's common
// subexpression temporary: the source indexes D_6608 with the whole expression twice. As a
// local it took bp-8, a slot of its own before the loops' reserved temporaries.
int AnimateScene(int frames)
{
    int row;
    int index_h;
    ActorFmt* actor_g;

    do
    {
        AnimateActors();
        AnimateWD();

        for (index_h = 0; index_h < 32; index_h++)
        {
            actor_g = &D_5c5a_actors[index_h];

            if (actor_g->tile != 0)
            {
                D_6608[actor_g->x + actor_g->y * 0x20] = 0;
                D_6608[actor_g->x + actor_g->y * 0x20 + 0x80] = actor_g->animTile;
            }
        }

        for (index_h = stripLeft; index_h <= stripRight; index_h++)
        {
            for (row = 0; row < 4; row++)
            {
                if (GetMap(index_h, row) != 0xfe)
                {
                    DrawStripCell(index_h, row);
                }
            }
        }

        if (stripLeft != 0 && stripWiden != 0)
        {
            stripLeft -= 1;
            stripRight += 1;
        }

        stripWiden = !stripWiden;

        if (PollKey() != 0)
        {
            return 1;
        }

        if (frames != 0)
        {
            WaitTicks(1);
        }

        if (sceneNo == 2)
        {
            ULTIMA_223c_AudioWhiteNoise(0x14, 0x3c, 10000);
        }
        else if (sceneNo == 3)
        {
            toneStep++;
            if ((int)toneStep > 7)
            {
                toneStep = 0;
            }

            if (toneStep == 0)
            {
                ULTIMA_22c0_AudioTone(3000, 3);
            }
            else if (toneStep == 4)
            {
                ULTIMA_22c0_AudioTone(2000, 3);
            }
        }
    } while (--frames != 0);

    return 0;
}

// Starts a scene: prints its title and copies its four rows of map from the scratch buffer.
// CHECKED
static void LoadSceneStrip(int scene)
{
    int row;
    int col_b;

    PrintCaptionBar(sceneCaptions[scene]);

    for (col_b = 0; col_b < 0x13; col_b++)
    {
        for (row = 0; row < 4; row++)
        {
            GetCopiedMap(col_b, row) = GetMap(col_b, row) = scratch[scene * 0x80 + col_b + row * 0x20];
        }
    }

    stripLeft = stripRight = 9;
    stripWiden = 0;
    sceneNo = scene;
}

// assembly in the original (src/asm/AFONT.ASM): save and restore the video page buffer
void SaveVideoPage(void);
void RestoreVideoPage(void);

// The introduction's story ("view"): draws the strip and plays the script at 0x200 of MISCMAPS.DAT (place,
// remove and move actors, moongates opening and closing, lightning, repeats); a key ends it.
// CHECKED
// Case 0xb's loop is a while: as a for (same code), MSC frees its temporaries (bp-20h,
// bp-22h) and case 0xc's pointer walk reuses them, giving a frame of 22h instead of the
// original's 28h (0xc at bp-24h..28h, the middle one in memory). The source steps
// region/local_14 with += in case 0xb (the decompiler's scan * 9 / * 3 gave MSC's
// closed form instead); += in a for header moves the counter's increment first.
void PlayIntroStory(void)
{
    int dir;
    int repeat_k;
    int argY_i;
    int argX_j;
    int step;
    int pos_h;
    ActorFmt* actor;
    int tile_l;
    int gateY_p;
    int gateX;
    int loopStart;

    vdp.tileYOffset = 0x10;
    stripLeft = stripRight = 9;
    stripWiden = 0;

    ULTIMA_0a70_GRAP_2d_SetPenColor(blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x78, 0x78, 199, 0x7e);
    ULTIMA_0a70_GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(0x78, 0x7f, 199, 0x7f);

    if (D_5893_mapId != 0x42)
    {
        ULTIMA_256e_ReadFileFromDisk(/*0xa044*/ "MISCMAPS.DAT", scratch, 2000, 0x2c0);
        if (D_5893_mapId == 0x40)
        {
            SaveVideoPage();
            while (LoadTileset(*tilesFileName) == 0) {}
            RestoreVideoPage();
        }
    }

    D_5893_mapId = 0x42;

    for (pos_h = 0x200; pos_h < 2000; pos_h++)
    {
        switch (scratch[pos_h])
        {
        case 0:
            actor = &D_5c5a_actors[scratch[++pos_h]];
            actor->tile = actor->animTile = scratch[++pos_h];
            actor->x = scratch[++pos_h];
            actor->y = scratch[++pos_h];
            actor->animState = 0;
            break;

        case 1:
            actor = &D_5c5a_actors[scratch[++pos_h]];
            actor->tile = actor->animTile = actor->animState = 0;

            GetMap(actor->x, actor->y) = GetCopiedMap(actor->x, actor->y);
            break;

        case 2:
            actor = &D_5c5a_actors[scratch[++pos_h]];
            dir = scratch[++pos_h];
            GetMap(actor->x, actor->y) = GetCopiedMap(actor->x, actor->y);
            actor->x += facingDx[dir];
            actor->y += facingDy[dir];
            break;

        case 3:
            if (AnimateScene(scratch[++pos_h]) != 0)
            {
                return;
            }
            break;

        case 4:
            gateX = scratch[++pos_h];
            gateY_p = scratch[++pos_h];
            GetMap(gateX, gateY_p) = GetCopiedMap(gateX, gateY_p) = TILE_MAP_FE;

            for (step = 1; step < 0x10; step++)
            {
                GRAP_60_Moongate(step, gateX, gateY_p + 7);
                if (AnimateScene(1) != 0)
                {
                    return;
                }
            }

            GetMap(gateX, gateY_p) = GetCopiedMap(gateX, gateY_p) = TILE_MAP_MOONGATE;
            if (AnimateScene(2) != 0)
            {
                return;
            }
            break;

        case 5:
            GetMap(gateX, gateY_p) = GetCopiedMap(gateX, gateY_p) = TILE_MAP_FE;

            step = 0xf;
            do
            {
                GRAP_60_Moongate(step, gateX, gateY_p + 7);
                if (AnimateScene(1) != 0)
                {
                    return;
                }
                step--;
            } while (step > 0);

            GetMap(gateX, gateY_p) = GetCopiedMap(gateX, gateY_p) = TILE_MAP_GRASS;
            if (AnimateScene(2) != 0)
            {
                return;
            }
            break;

        case 6:
            LoadSceneStrip(scratch[++pos_h]);
            break;

        case 7:
            actor = &D_5c5a_actors[scratch[++pos_h]];
            tile_l = actor->tile;

            actor->tile = actor->animTile = TILE_ACTOR_CIRCLE;

            if (ULTIMA_1068_RevealTile(0x100 + tile_l, actor->x, actor->y + 7) != 0)
            {
                return;
            }
            actor->tile = actor->animTile = tile_l;
            break;

        case 8:
            actor = &D_5c5a_actors[scratch[++pos_h]];
            argX_j = actor->x;
            argY_i = actor->y;
            tile_l = actor->tile;

            actor->tile = actor->animTile = TILE_ACTOR_CIRCLE;

            if (ULTIMA_1068_RevealTile(GetCopiedMap(argX_j, argY_i), argX_j, argY_i + 7) != 0)
            {
                return;
            }

            actor->tile = actor->animTile = tile_l;
            break;

        case 9:
            pos_h = 0x1ff;
            break;

        case 10:
            tile_l = scratch[++pos_h];
            argX_j = scratch[++pos_h];
            argY_i = scratch[++pos_h];
            GetCopiedMap(argX_j, argY_i) = tile_l;
            GetMap(argX_j, argY_i) = tile_l;
            break;

        case 0xb:
            // ...
            argX_j = argY_i = 0;
            step = 0;
            while (step < 5)
            {
                if (AnimateScene(1) != 0)
                {
                    return;
                }
                ULTIMA_0a70_GRAP_2d_SetPenColor(brightWhiteColor);
                GRAP_Line(argX_j + 0x80, argY_i + 0x98, argX_j + 0x89, argY_i + 0x9b);
                GRAP_Line(argX_j + 0x80, argY_i + 0x99, argX_j + 0x89, argY_i + 0x9c);
                argX_j += 9;
                argY_i += 3;
                step++;
            }

            pos_h++;
            pos_h++;
            actor = &D_5c5a_actors[scratch[pos_h]];
            GRAP_51_PutTile(0, actor->x, actor->y + 7);
            ULTIMA_223c_AudioWhiteNoise(1, 0x4b0, 4000);
            if (AnimateScene(3) != 0)
            {
                return;
            }
            break;

        case 0xc:
            for (step = 0; step < 0x20; step++)
            {
                D_5c5a_actors[step].animTile = D_5c5a_actors[step].tile = D_5c5a_actors[step].animState = 0;
            }
            break;

        case 0xd:
            actor = &D_5c5a_actors[scratch[++pos_h]];
            dir = scratch[++pos_h];
            GetMap(actor->x, actor->y) = GetCopiedMap(actor->x, actor->y);
            actor->x += facingDx[dir];
            actor->y += facingDy[dir];
            if (AnimateScene(7) != 0)
            {
                return;
            }
            break;

        case 0xe:
            repeat_k = scratch[++pos_h];
            loopStart = pos_h;
            break;

        case 0xf:
            repeat_k--;
            if (repeat_k != 0)
            {
                pos_h = loopStart;
            }
            break;
        }

        if (PollKey() != 0)
        {
            return;
        }
    }
}

// 0998 ~ 0b0a: character creation

// A random virtue (0 to 7) that was not offered yet and has not lost a question; marks it offered.
static int PickVirtue(void)
{
    int virtue;

    do
    {
        virtue = ULTIMA_2092_RandomRange(0, 7);
    } while (virtueOffered[virtue] != 0 || virtueRejected[virtue] != 0);

    virtueOffered[virtue] = 1;

    return virtue;
}

// One of the gypsy's questions: two virtues are shown, A or B adds the chosen one's bonuses to the new
// intelligence, dexterity and strength, and the other virtue is out.
static void AskVirtueQuestion(byte* fontImage, byte* cardImage)
{
    int rightCard;
    int leftCard;
    int swap_f;
    byte firstKey;
    int virtueB_o;
    int virtueA;
    byte key_g;

    GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    GRAP_4b_PutImage(cardImage, 1, 0x10, 0, 0);
    GRAP_4b_PutImage(cardImage, 1, 200, 0, 0);

    D_5156_penX = 0;
    D_5158_penY = 0x98;

    virtueA = PickVirtue();
    virtueB_o = PickVirtue();
    if (virtueA > virtueB_o)
    {
        leftCard = virtueB_o;
        rightCard = virtueA;
        firstKey = 'B';
    }
    else
    {
        leftCard = virtueA;
        rightCard = virtueB_o;
        firstKey = 'A';
    }

    GRAP_4b_PutImage(cardImage, leftCard + 2, cardX[leftCard], cardY[leftCard], 0);
    GRAP_4b_PutImage(cardImage, rightCard + 2, cardX[rightCard] + 0xb8, cardY[rightCard], 0);
    ULTIMA_256e_ReadFileFromDisk(/*0xa052*/ "QUESTION.DAT", scratch, 2000, questionOffset[virtueA][virtueB_o]);
    DrawJustifiedText(fontImage, (char*)scratch);
    GRAP_1b_TransferFullscreen(1, 0);

    do
    {
        key_g = ToUpperChar(PollKey());
        if (key_g == 'A')
            break;
    } while (key_g != 'B');

    if (key_g != firstKey)
    {
        swap_f = virtueA;
        virtueA = virtueB_o;
        virtueB_o = swap_f;
    }

    newInt += intBonus[virtueA];
    newDex += dexBonus[virtueA];
    newStr += strBonus[virtueA];
    virtueRejected[virtueB_o] = 1;
}

// Character creation: asks the name and sex, the gypsy's seven questions (4, 2 and 1), then sets the stats
// and writes the new saved game (SAVED.GAM).
// CHECKED
void CreateCharacter(void)
{
    byte* fontImage;
    byte round;
    byte* gypsyImage;
    byte key_g;

    if (D_5893_mapId != 64)
    {
        UnloadTileset();
    }

    D_a9cb = 0xff;

    do
    {
        fontImage = LoadResourceFile(fileNames[0]);
    } while (fontImage == 0);

    do
    {
        gypsyImage = LoadImageFile(fileNames[0x10]);
    } while (gypsyImage == 0);

    ULTIMA_256e_ReadFileFromDisk(/*0xa060*/ "INIT.GAM", &saveStart, (int)&saveEnd - (int)&saveStart, 0);
    ULTIMA_0a70_GRAP_2d_SetPenColor(blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x78, 0x78, 200, 0x7e);
    ULTIMA_0a70_GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(0x78, 0x7f, 200, 0x7f);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 0x80, 0x137, 0xbf);
    SetTextPosition(3, 0x11);

    ULTIMA_1850_PrintString(/*0xa06a*/ "By what name shalt thou be known?");
    SetTextPosition(0xe, 0x13);
    ULTIMA_16ba_PrintChar(':');
    IntroGetString(D_55a8_party[0].name, 8);

    if (D_55a8_party[0].name[0] != 0)
    {
        SetTextPosition(8, 0x15);
        ULTIMA_1850_PrintString(/*0xa08c*/ "Art thou Male or Female? ");

        do
        {
            key_g = ToUpperChar(WaitForKeystroke(0));
            if (key_g == 'M')
                break;
        } while (key_g != 'F');

        ULTIMA_16ba_PrintChar(key_g);
        if (key_g == 'M')
        {
            D_55a8_party[0].gender = 0xb;
        }
        else
        {
            D_55a8_party[0].gender = 0xc;
        }

        D_5156_penX = 0;
        D_5158_penY = 9;
        textLeftX[0] = 0;
        textRightX[0] = 0x140;
        textLeftX[1] = 0xaf;
        textRightX[1] = 0x140;
        D_5150_regionTop = 0x59;
        D_5152_regionBottom = 200;

        ULTIMA_256e_ReadFileFromDisk(/*0xa0a6*/ "QUESTION.DAT", scratch, 2000, 0);

        GRAP_0f_SelectPage(1);
        promptMode = 2;
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        GRAP_4b_PutImage(gypsyImage, 0, 0, 0x60, 0);
        DrawJustifiedText(fontImage, scratch);
        GRAP_1b_TransferFullscreen(1, 0);

        D_5150_regionTop = 200;

        while (PollKey() == 0) {}

        newInt = D_55a8_party[0].intel;
        newDex = D_55a8_party[0].dex;
        newStr = D_55a8_party[0].str;

        for (round = 0; round < 8; round++)
        {
            virtueOffered[round] = virtueRejected[round] = 0;
        }

        for (round = 0; round < 4; round++)
        {
            AskVirtueQuestion(fontImage, gypsyImage);
        }

        for (round = 0; round < 8; round++)
        {
            virtueOffered[round] = 0;
        }

        for (round = 0; round < 2; round++)
        {
            AskVirtueQuestion(fontImage, gypsyImage);
        }

        for (round = 0; round < 8; round++)
        {
            virtueOffered[round] = 0;
        }

        AskVirtueQuestion(fontImage, gypsyImage);

        D_5156_penX = D_5158_penY = 0;
        D_5150_regionTop = 0x5a;
        textLeftX[1] = 0;
        textRightX[1] = 0xa6;
        spaceWidth = 4;

        ULTIMA_256e_ReadFileFromDisk(/*0xa0b4*/ "QUESTION.DAT", scratch, 2000, 0x322);
        GRAP_0f_SelectPage(1);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        GRAP_4b_PutImage(gypsyImage, 10, 0xa8, 100, 0);

        DrawJustifiedText(fontImage, scratch);
        spaceWidth = 5;
        GRAP_1b_TransferFullscreen(1, 0);

        while (PollKey() == 0)
        {}

        FreeImage(gypsyImage);
        FreeBitImage(fontImage);

        GRAP_0f_SelectPage(1);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        GRAP_0f_SelectPage(0);

        promptMode = 0;
        
        D_55a8_party[0].mag = D_55a8_party[0].intel = newInt;
        D_55a8_party[0].dex = newDex;
        //D_55a8_party[0].actorIdx = (newStr - 0x14 & ~-(newStr < 0x14)) + 0x14;
        D_55a8_party[0].str = newStr <= 0x14 ? 0x14 : newStr;
        ULTIMA_256e_ReadFileFromDisk(/*0xa0c2*/ "INIT.OOL", D_b31e, 0x100, 0);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        SetTextPosition(0, 10);
        SwitchDisks(3);

        // NOT MATCHING: sub al, al vs sub ax, ax
        memset(scratch, 0, 0x100);

        WriteFileToDisk(/*0xa0cc*/ "SAVED.OOL", scratch, 0x200);
        WriteFileToDisk(/*0xa0d6*/ "SAVED.GAM", &saveStart, (int)&saveEnd - (int)&saveStart);
    }

    SwitchDisks(0);
    D_5893_mapId = 0x40;
}

