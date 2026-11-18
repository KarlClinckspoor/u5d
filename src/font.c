#include "common/common.h"
#include "savegame.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include <string.h>

#include "audio/audio.h"

#include "font.h"
#include "intro.h"
#include "tiles.h"

// glyph widths from ' ' (never read: spaces are D_5154_spaceWidth wide) to 'z'
u8 D_50ea_glyphWidths[0x5b] =
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

u16 D_5146_textLeftX[2] = {0, 0};
u16 D_514a_unused = 0; // no code refers to it

u16 D_514c_textRightX[2] = {0x0140, 0x0140};

u16 D_5150_regionTop = 0x00c8;

u16 D_5152_regionBottom = 0x00c8;

u16 D_5154_spaceWidth = 5;

u16 D_5156_penX = 0;

u16 D_5158_penY = 0x003c;
u16 D_515a_toneStep = 0;

char* D_515c_sceneCaptions[4] =
{
    _STATIC_TEXT(0xa010, "The Summoning"),
    _STATIC_TEXT(0xa01e, "The Journey"),
    _STATIC_TEXT(0xa02a, "The Arrival"),
    _STATIC_TEXT(0xa036, "The Welcoming")
};

u8 D_5164_intBonus[8] = {0x02, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00};

u8 D_516c_dexBonus[8] = {0x00, 0x02, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00};

u8 D_5174_strBonus[8] = {0x00, 0x00, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00};

u16 D_517c_questionOffset[8][8] =
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

u8 D_51fc_cardX[8] = {0x28, 0x30, 0x30, 0x28, 0x28, 0x30, 0x28, 0x30};

u8 D_5204_cardY[8] = {0x05, 0x07, 0x04, 0x0a, 0x08, 0x00, 0x05, 0x06};

// The DOS build takes FONT_0000_DrawJustifiedText from src/asm/AFONT0.ASM (assembly written from the
// original's bytes, linked before this object): the C below does not match yet, so it is
// compiled for the native build only.
#if !defined(TARGET_DOS16)
// Prints text justified in the proportional font: the glyph image and the text; the margins and the cursor
// are the globals D_5146_textLeftX..D_5158_penY (two regions, so that text can run beside a picture). Assembly in the DOS
// build.
// CLEAN ROOM (written from the disassembly). Prints justified text: glyphs the glyph
// image, text the text. Per line: loop A measures up to the line width, loop B backs up
// to the last space (or a "_" hyphenation point), loop C draws, spreading the slack over
// the spaces; then the next line's margins.
// NOT MATCHING: 56 bytes differ, length 670 against 674 (before: 513 bytes, 676/674).
// What gives the original's code so far:
// - the outer loop is a for with an increment expression. With "while" or "for (; c; )"
//   the three inner loops get no registers; "for (;;) { if (!c) break; ..." gives them but
//   leaves loop A's register loads at the top of the body. With an increment the test sits
//   at the bottom with loop A's loads right after it, as in the original. The increment
//   here is "0" (no code); the original's is not known ("lineStart++" would be the natural
//   one, but the original's test at the end of a line jumps past the increment to the
//   loop test, which no continue or break does: hence the goto to L_next);
// - loop B's "--i" is in its condition (the first break is then the space case, and the
//   code after the loop follows it);
// - loop C is "if (i < n) do ... while (n != ++i)";
// - "if (text[start] == 0) goto L_next;" and not an if around the rest (that one keeps
//   text[start] in AL for the "_" test).
// Still different:
// - the frame is 1Ah, the original's 1Ch: at 025f the original keeps region * 2 in BX
//   with a dead store at bp-1Ch (shl ax,1; mov [bp-1Ch],ax; mov bx,ax), where we get
//   mov si,ax; shl si,1 as at the top of the function. Tried: the condition written out at
//   each use, assigned inside the index, D_5146_textLeftX[region] read twice;
// - 0173: the original adds D_50f7's temporary first (mov ax,[bp-14h]; add ax,si), we
//   mov ax,si; add ax,[bp-14h]. Tried: the operands in every order, the cast on either
//   side of the comparison.
void FONT_0000_DrawJustifiedText(byte* glyphs, byte* text)
{
    int region;
    int scan;
    int lineStart;
    int lineWidth;
    int spaces;
    int slack;
    int lineEnd;
    int used;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    region = (int)D_5158_penY > (int)D_5150_regionTop && (int)D_5158_penY < (int)D_5152_regionBottom;
    lineWidth = D_514c_textRightX[region] - D_5146_textLeftX[region];
    lineStart = lineEnd = 0;
    used = D_5156_penX - D_5146_textLeftX[region];

    for (; text[lineStart] != 0; 0)
    {
        spaces = 0;
        for (scan = lineStart; text[scan] != 0 && lineWidth > used && text[scan] != '\n'; scan++)
        {
            if (text[scan] <= ' ')
            {
                used += D_5154_spaceWidth;
                spaces++;
            }
            else if (text[scan] == '{')
            {
                used += 0xf;
            }
            else if (text[scan] != '_')
            {
                used += D_50ea_glyphWidths[text[scan] - 0x20] + 1;
            }
        }

        while ((text[scan] != 0 || used >= lineWidth) && text[scan] != '\n' && lineStart < --scan)
        {
            if (text[scan] == ' ')
            {
                used -= D_5154_spaceWidth;
                spaces--;
                break;
            }
            if (text[scan] == '_' && (uint)(used + D_50f7 + 1) < lineWidth)
            {
                used += D_50f7 + 1;
                break;
            }
            if (text[scan] != '_' && text[scan] != '{')
            {
                used -= D_50ea_glyphWidths[text[scan] - 0x20] + 1;
            }
        }

        slack = lineWidth - used;
        lineEnd = scan;

        scan = lineStart;
        if (scan < lineEnd)
        do
        {
            if (text[scan] <= ' ')
            {
                D_5156_penX += D_5154_spaceWidth;
                if (spaces != 0 && text[lineEnd] != 0 && text[lineEnd] != '\n')
                {
                    D_5156_penX += slack / spaces;
                    slack -= slack / spaces;
                    spaces--;
                }
            }
            else if (text[scan] == '{')
            {
                D_5156_penX += 0xf;
            }
            else if (text[scan] != '_')
            {
                if ((int)D_5158_penY < 0xc0)
                {
                    ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(glyphs, text[scan] - 0x20, D_5156_penX, D_5158_penY);
                }
                D_5156_penX += D_50ea_glyphWidths[text[scan] - 0x20] + 1;
            }
        } while (lineEnd != ++scan);

        lineStart = lineEnd;
        if (text[lineStart] == 0)
        {
            goto L_next;
        }
        {
            if (text[lineStart] == '_' && (int)D_5158_penY < 0xc0)
            {
                ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(glyphs, 0xd, D_5156_penX, D_5158_penY);
            }
            used = 0;
            D_5158_penY += 9;
            region = (int)D_5158_penY > (int)D_5150_regionTop && (int)D_5158_penY < (int)D_5152_regionBottom;
            D_5156_penX = D_5146_textLeftX[region];
            lineWidth = D_514c_textRightX[region] - D_5156_penX;
            lineStart++;
        }
L_next:;
    }

    ULTIMA_0c22_GRAP_0f_SelectPage(0);
}
#endif

// Draws one cell of the scene strip: the map tile, or else the actor's tile that FONT_02fc_AnimateScene left
// under it.
static void FONT_02a2_DrawStripCell(int x, int y)
{
    byte tile;

    tile = GetMap(x, y);
    if (tile != 0)
    {
        ULTIMA_10e0_GRAP_51_PutTile(D_b11e_tileFrames[tile], x, y + 7);
        return;
    }
    else
    {
        tile = D_6608[x + y * 0x20 + 0x80];
        if (tile == 0x16)
        {
            return;
        }

        ULTIMA_10e0_GRAP_51_PutTile(0x100 + tile, x, y + 7);
    }
}

// Runs the scene's animation for that many frames: actors animate, the strip widens, white noise or tones
// sound; returns 1 when a key was pressed.
// CHECKED
// The decompiler's lineWidth (x + y * 0x20, stored at bp-0Ah and never read) is MSC's common
// subexpression temporary: the source indexes D_6608 with the whole expression twice. As a
// local it took bp-8, a slot of its own before the loops' reserved temporaries.
int FONT_02fc_AnimateScene(int frames)
{
    int row;
    int index_h;
    ActorFmt* actor_g;

    do
    {
        ULTIMA_4552_AnimateActors();
        INTRO_2090_AnimateWD();

        for (index_h = 0; index_h < 32; index_h++)
        {
            actor_g = &D_5c5a_actors[index_h];

            if (actor_g->tile != 0)
            {
                D_6608[actor_g->x + actor_g->y * 0x20] = 0;
                D_6608[actor_g->x + actor_g->y * 0x20 + 0x80] = actor_g->animTile;
            }
        }

        for (index_h = D_bd26_stripLeft; index_h <= D_bd27_stripRight; index_h++)
        {
            for (row = 0; row < 4; row++)
            {
                if (GetMap(index_h, row) != 0xfe)
                {
                    FONT_02a2_DrawStripCell(index_h, row);
                }
            }
        }

        if (D_bd26_stripLeft != 0 && D_bd28_stripWiden != 0)
        {
            D_bd26_stripLeft -= 1;
            D_bd27_stripRight += 1;
        }

        D_bd28_stripWiden = !D_bd28_stripWiden;

        if (ULTIMA_1d5e_PollKey() != 0)
        {
            return 1;
        }

        if (frames != 0)
        {
            ULTIMA_20fa_WaitTicks(1);
        }

        if (D_bd29_sceneNo == 2)
        {
            ULTIMA_223c_AudioWhiteNoise(0x14, 0x3c, 10000);
        }
        else if (D_bd29_sceneNo == 3)
        {
            D_515a_toneStep++;
            if ((int)D_515a_toneStep > 7)
            {
                D_515a_toneStep = 0;
            }

            if (D_515a_toneStep == 0)
            {
                ULTIMA_22c0_AudioTone(3000, 3);
            }
            else if (D_515a_toneStep == 4)
            {
                ULTIMA_22c0_AudioTone(2000, 3);
            }
        }
    } while (--frames != 0);

    return 0;
}

// Starts a scene: prints its title and copies its four rows of map from the scratch buffer.
// CHECKED
static void FONT_0418_LoadSceneStrip(int scene)
{
    int row;
    int col_b;

    ASSERT(scene < 4);

    INTRO_043e_PrintCaptionBar(D_515c_sceneCaptions[scene]);

    for (col_b = 0; col_b < 0x13; col_b++)
    {
        for (row = 0; row < 4; row++)
        {
            GetCopiedMap(col_b, row) = GetMap(col_b, row) = D_b21e_scratch[scene * 0x80 + col_b + row * 0x20];
        }
    }

    D_bd26_stripLeft = D_bd27_stripRight = 9;
    D_bd28_stripWiden = 0;
    D_bd29_sceneNo = scene;
}

// assembly in the original (src/asm/AFONT.ASM): save and restore the video page buffer
void FONT_0e52_SaveVideoPage(void);
void FONT_0e7b_RestoreVideoPage(void);

// The introduction's story ("view"): draws the strip and plays the script at 0x200 of MISCMAPS.DAT (place,
// remove and move actors, moongates opening and closing, lightning, repeats); a key ends it.
// CHECKED
// Case 0xb's loop is a while: as a for (same code), MSC frees its temporaries (bp-20h,
// bp-22h) and case 0xc's pointer walk reuses them, giving a frame of 22h instead of the
// original's 28h (0xc at bp-24h..28h, the middle one in memory). The source steps
// region/local_14 with += in case 0xb (the decompiler's scan * 9 / * 3 gave MSC's
// closed form instead); += in a for header moves the counter's increment first.
void FONT_04a4_PlayIntroStory(void)
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

    D_52ba_vdp.tileYOffset = 0x10;
    D_bd26_stripLeft = D_bd27_stripRight = 9;
    D_bd28_stripWiden = 0;

    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x78, 0x78, 199, 0x7e);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(0x78, 0x7f, 199, 0x7f);

    if (D_5893_mapId != 0x42)
    {
        ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa044, "MISCMAPS.DAT"), D_b21e_scratch, 2000, 0x2c0);
        if (D_5893_mapId == 0x40)
        {
            FONT_0e52_SaveVideoPage();
            while (ULTIMA_0ff4_LoadTileset(*D_25f0) == 0) {}
            FONT_0e7b_RestoreVideoPage();
        }
    }

    D_5893_mapId = 0x42;

    for (pos_h = 0x200; pos_h < 2000; pos_h++)
    {
        switch (D_b21e_scratch[pos_h])
        {
        case 0:
            actor = &D_5c5a_actors[D_b21e_scratch[++pos_h]];
            actor->tile = actor->animTile = D_b21e_scratch[++pos_h];
            actor->x = D_b21e_scratch[++pos_h];
            actor->y = D_b21e_scratch[++pos_h];
            actor->animState = 0;
            break;

        case 1:
            actor = &D_5c5a_actors[D_b21e_scratch[++pos_h]];
            actor->tile = actor->animTile = actor->animState = 0;

            GetMap(actor->x, actor->y) = GetCopiedMap(actor->x, actor->y);
            break;

        case 2:
            actor = &D_5c5a_actors[D_b21e_scratch[++pos_h]];
            dir = D_b21e_scratch[++pos_h];
            GetMap(actor->x, actor->y) = GetCopiedMap(actor->x, actor->y);
            actor->x += D_24d6_facingDx[dir];
            actor->y += D_24de_facingDy[dir];
            break;

        case 3:
            if (FONT_02fc_AnimateScene(D_b21e_scratch[++pos_h]) != 0)
            {
                return;
            }
            break;

        case 4:
            gateX = D_b21e_scratch[++pos_h];
            gateY_p = D_b21e_scratch[++pos_h];
            GetMap(gateX, gateY_p) = GetCopiedMap(gateX, gateY_p) = TILE_MAP_FE;

            for (step = 1; step < 0x10; step++)
            {
                ULTIMA_1112_GRAP_60_Moongate(step, gateX, gateY_p + 7);
                if (FONT_02fc_AnimateScene(1) != 0)
                {
                    return;
                }
            }

            GetMap(gateX, gateY_p) = GetCopiedMap(gateX, gateY_p) = TILE_MAP_MOONGATE;
            if (FONT_02fc_AnimateScene(2) != 0)
            {
                return;
            }
            break;

        case 5:
            GetMap(gateX, gateY_p) = GetCopiedMap(gateX, gateY_p) = TILE_MAP_FE;

            step = 0xf;
            do
            {
                ULTIMA_1112_GRAP_60_Moongate(step, gateX, gateY_p + 7);
                if (FONT_02fc_AnimateScene(1) != 0)
                {
                    return;
                }
                step--;
            } while (step > 0);

            GetMap(gateX, gateY_p) = GetCopiedMap(gateX, gateY_p) = TILE_MAP_GRASS;
            if (FONT_02fc_AnimateScene(2) != 0)
            {
                return;
            }
            break;

        case 6:
            FONT_0418_LoadSceneStrip(D_b21e_scratch[++pos_h]);
            break;

        case 7:
            actor = &D_5c5a_actors[D_b21e_scratch[++pos_h]];
            tile_l = actor->tile;

            actor->tile = actor->animTile = TILE_ACTOR_CIRCLE;

            if (ULTIMA_1068_RevealTile(0x100 + tile_l, actor->x, actor->y + 7) != 0)
            {
                return;
            }
            actor->tile = actor->animTile = tile_l;
            break;

        case 8:
            actor = &D_5c5a_actors[D_b21e_scratch[++pos_h]];
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
            tile_l = D_b21e_scratch[++pos_h];
            argX_j = D_b21e_scratch[++pos_h];
            argY_i = D_b21e_scratch[++pos_h];
            GetCopiedMap(argX_j, argY_i) = tile_l;
            GetMap(argX_j, argY_i) = tile_l;
            break;

        case 0xb:
            // ...
            argX_j = argY_i = 0;
            step = 0;
            while (step < 5)
            {
                if (FONT_02fc_AnimateScene(1) != 0)
                {
                    return;
                }
                ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
                ULTIMA_0b10_GRAP_Line(argX_j + 0x80, argY_i + 0x98, argX_j + 0x89, argY_i + 0x9b);
                ULTIMA_0b10_GRAP_Line(argX_j + 0x80, argY_i + 0x99, argX_j + 0x89, argY_i + 0x9c);
                argX_j += 9;
                argY_i += 3;
                step++;
            }

            pos_h++;
            pos_h++;
            actor = &D_5c5a_actors[D_b21e_scratch[pos_h]];
            ULTIMA_10e0_GRAP_51_PutTile(0, actor->x, actor->y + 7);
            ULTIMA_223c_AudioWhiteNoise(1, 0x4b0, 4000);
            if (FONT_02fc_AnimateScene(3) != 0)
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
            actor = &D_5c5a_actors[D_b21e_scratch[++pos_h]];
            dir = D_b21e_scratch[++pos_h];
            GetMap(actor->x, actor->y) = GetCopiedMap(actor->x, actor->y);
            actor->x += D_24d6_facingDx[dir];
            actor->y += D_24de_facingDy[dir];
            if (FONT_02fc_AnimateScene(7) != 0)
            {
                return;
            }
            break;

        case 0xe:
            repeat_k = D_b21e_scratch[++pos_h];
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

        if (ULTIMA_1d5e_PollKey() != 0)
        {
            return;
        }
    }
}

// 0998 ~ 0b0a: character creation

// A random virtue (0 to 7) that was not offered yet and has not lost a question; marks it offered.
static int FONT_0998_PickVirtue(void)
{
    int virtue;

    do
    {
        virtue = ULTIMA_2092_RandomRange(0, 7);
    } while (D_bd2a_virtueOffered[virtue] != 0 || D_bd32_virtueRejected[virtue] != 0);

    D_bd2a_virtueOffered[virtue] = 1;

    return virtue;
}

// One of the gypsy's questions: two virtues are shown, A or B adds the chosen one's bonuses to the new
// intelligence, dexterity and strength, and the other virtue is out.
static void FONT_09c8_AskVirtueQuestion(byte* fontImage, byte* cardImage)
{
    int rightCard;
    int leftCard;
    int swap_f;
    byte firstKey;
    int virtueB_o;
    int virtueA;
    byte key_g;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_0d4c_GRAP_4b_PutImage(cardImage, 1, 0x10, 0, 0);
    ULTIMA_0d4c_GRAP_4b_PutImage(cardImage, 1, 200, 0, 0);

    D_5156_penX = 0;
    D_5158_penY = 0x98;

    virtueA = FONT_0998_PickVirtue();
    virtueB_o = FONT_0998_PickVirtue();
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

    ULTIMA_0d4c_GRAP_4b_PutImage(cardImage, leftCard + 2, D_51fc_cardX[leftCard], D_5204_cardY[leftCard], 0);
    ULTIMA_0d4c_GRAP_4b_PutImage(cardImage, rightCard + 2, D_51fc_cardX[rightCard] + 0xb8, D_5204_cardY[rightCard], 0);
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa052, "QUESTION.DAT"), D_b21e_scratch, 2000, D_517c_questionOffset[virtueA][virtueB_o]);
    FONT_0000_DrawJustifiedText(fontImage, (char*)D_b21e_scratch);
    ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);

    do
    {
        key_g = ULTIMA_2032_ToUpper(ULTIMA_1d5e_PollKey());
        if (key_g == 'A')
            break;
    } while (key_g != 'B');

    if (key_g != firstKey)
    {
        swap_f = virtueA;
        virtueA = virtueB_o;
        virtueB_o = swap_f;
    }

    D_bd3c_newInt += D_5164_intBonus[virtueA];
    D_bd3d_newDex += D_516c_dexBonus[virtueA];
    D_bd3e_newStr += D_5174_strBonus[virtueA];
    D_bd32_virtueRejected[virtueB_o] = 1;
}

// Character creation: asks the name and sex, the gypsy's seven questions (4, 2 and 1), then sets the stats
// and writes the new saved game (SAVED.GAM).
// CHECKED
void FONT_0b0a_CreateCharacter(void)
{
    byte* fontImage;
    byte round;
    byte* gypsyImage;
    byte key_g;

    if (D_5893_mapId != 64)
    {
        ULTIMA_102e_UnloadTileset();
    }

    D_a9cb = 0xff;

    do
    {
        fontImage = ULTIMA_0fae_LoadResourceFile(D_25ea_fileNames[0]);
    } while (fontImage == 0);

    do
    {
        gypsyImage = ULTIMA_0bae_LoadImageFile(D_25ea_fileNames[0x10]);
    } while (gypsyImage == 0);

#if !defined(TARGET_DOS16)
    FILE_ReadSavegameFile(_TEXT(0xa060, "INIT.GAM"));
#else
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa060, "INIT.GAM"), &D_55a6_saveStart, (int)&D_6606_saveEnd - (int)&D_55a6_saveStart, 0);
#endif
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(0x78, 0x78, 200, 0x7e);
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(0x78, 0x7f, 200, 0x7f);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 0x80, 0x137, 0xbf);
    ULTIMA_1bf2_SetTextPosition(3, 0x11);

    ULTIMA_1850_PrintString(_TEXT(0xa06a, "By what name shalt thou be known?"));
    ULTIMA_1bf2_SetTextPosition(0xe, 0x13);
    ULTIMA_16ba_PrintChar(':');
    ULTIMA_1e38_IntroGetString(D_55a8_party[0].name, 8);

    if (D_55a8_party[0].name[0] != 0)
    {
        ULTIMA_1bf2_SetTextPosition(8, 0x15);
        ULTIMA_1850_PrintString(_TEXT(0xa08c, "Art thou Male or Female? "));

        do
        {
            key_g = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0));
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
        D_5146_textLeftX[0] = 0;
        D_514c_textRightX[0] = 0x140;
        D_5146_textLeftX[1] = 0xaf;
        D_514c_textRightX[1] = 0x140;
        D_5150_regionTop = 0x59;
        D_5152_regionBottom = 200;

#if defined(ENABLE_BGM)
        AUDIO_StopBgm();
#endif

        ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa0a6, "QUESTION.DAT"), D_b21e_scratch, 2000, 0);

        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        D_a9be_promptMode = 2;
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_0d4c_GRAP_4b_PutImage(gypsyImage, 0, 0, 0x60, 0);
        FONT_0000_DrawJustifiedText(fontImage, D_b21e_scratch);
        ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);

        D_5150_regionTop = 200;

        while (ULTIMA_1d5e_PollKey() == 0) {}

        D_bd3c_newInt = D_55a8_party[0].intel;
        D_bd3d_newDex = D_55a8_party[0].dex;
        D_bd3e_newStr = D_55a8_party[0].str;

        for (round = 0; round < 8; round++)
        {
            D_bd2a_virtueOffered[round] = D_bd32_virtueRejected[round] = 0;
        }

        for (round = 0; round < 4; round++)
        {
            FONT_09c8_AskVirtueQuestion(fontImage, gypsyImage);
        }

        for (round = 0; round < 8; round++)
        {
            D_bd2a_virtueOffered[round] = 0;
        }

        for (round = 0; round < 2; round++)
        {
            FONT_09c8_AskVirtueQuestion(fontImage, gypsyImage);
        }

        for (round = 0; round < 8; round++)
        {
            D_bd2a_virtueOffered[round] = 0;
        }

        FONT_09c8_AskVirtueQuestion(fontImage, gypsyImage);

        D_5156_penX = D_5158_penY = 0;
        D_5150_regionTop = 0x5a;
        D_5146_textLeftX[1] = 0;
        D_514c_textRightX[1] = 0xa6;
        D_5154_spaceWidth = 4;

        ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa0b4, "QUESTION.DAT"), D_b21e_scratch, 2000, 0x322);
        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_0d4c_GRAP_4b_PutImage(gypsyImage, 10, 0xa8, 100, 0);

        FONT_0000_DrawJustifiedText(fontImage, D_b21e_scratch);
        D_5154_spaceWidth = 5;
        ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);

        while (ULTIMA_1d5e_PollKey() == 0)
        {}

        ULTIMA_0be4_FreeImage(gypsyImage);
        ULTIMA_0fdc_FreeBitImage(fontImage);

        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_0c22_GRAP_0f_SelectPage(0);

        D_a9be_promptMode = 0;
        
        D_55a8_party[0].mag = D_55a8_party[0].intel = D_bd3c_newInt;
        D_55a8_party[0].dex = D_bd3d_newDex;
        //D_55a8_party[0].actorIdx = (D_bd3e_newStr - 0x14 & ~-(D_bd3e_newStr < 0x14)) + 0x14;
        D_55a8_party[0].str = D_bd3e_newStr <= 0x14 ? 0x14 : D_bd3e_newStr;
        ULTIMA_256e_ReadFileFromDisk(_TEXT(0xa0c2, "INIT.OOL"), D_b31e, 0x100, 0);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_1bf2_SetTextPosition(0, 10);
        ULTIMA_251e_SwitchDisks(3);

        // NOT MATCHING: sub al, al vs sub ax, ax
        memset(D_b21e_scratch, 0, 0x100);

        ULTIMA_25d8_WriteFileToDisk(_TEXT(0xa0cc, "SAVED.OOL"), D_b21e_scratch, 0x200);
#if !defined(TARGET_DOS16)
        FILE_WriteSavegameFile(_TEXT(0xa0d6, "SAVED.GAM"));
#else
        ULTIMA_25d8_WriteFileToDisk(_TEXT(0xa0d6, "SAVED.GAM"), &D_55a6_saveStart, (int)&D_6606_saveEnd - (int)&D_55a6_saveStart);
#endif
    }

    ULTIMA_251e_SwitchDisks(0);
    D_5893_mapId = 0x40;
}

#if !defined(TARGET_DOS16) // assembly in the original: src/asm/AFONT.ASM
// Assembly (AFONT.ASM): saves the video page buffer before the tile set is changed.
// The original saves the first 0x3000 bytes of the video page buffer (segment D_5354_pageSegment) into
// overlay area 2 before the tile set is changed, and restores them after; nothing to do here.
void FONT_0e52_SaveVideoPage(void) {}

// Assembly (AFONT.ASM): puts it back.
void FONT_0e7b_RestoreVideoPage(void) {}
#endif
