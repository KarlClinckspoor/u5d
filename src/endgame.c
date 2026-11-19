#include "common/common.h"

// this module's communal variables (vars.h, DS:a9bc)
char lineBuf[0x27 + 1];
u8 lineLen;

#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include <stdlib.h>

#include "endgame.h"
#include "font.h"
#include "tiles.h"

// The six illustrated text screens of the ending (pictures END1.16 / END2.16, the text read from END.DAT and
// set in the proportional font), then the closing ENDSC.16 picture.
// CHECKED (nop)
static void ShowEndScreens(void)
{
    byte* fontImage;
    uint loadedFile;
    int screen;
    byte* endImage_f;
    byte* bannerImage_f;

    SwitchDisks(2);

    do
    {
        fontImage = LoadResourceFile(fileNames[0]); // PROPORT.PCS
    } while (fontImage == 0);

    do
    {
        bannerImage_f = LoadImageFile(fileNames[0x11]); // TEXT.16
    } while (bannerImage_f == 0);

    do
    {
        endImage_f = LoadImageFile(fileNames[0x18]); // END1.16
    } while (endImage_f == 0);

    GRAP_66_Reveal(0, 0, 0x13f, 199);
    SelectTextWindow(0);

    loadedFile = 0x31;
    for (screen = 0; screen < 6; screen++)
    {
        if (D_3df4_imageFile[screen] != loadedFile)
        {
            FreeImage(endImage_f);
            loadedFile = (uint)D_3df4_imageFile[screen];
            do
            {
                endImage_f = LoadImageFile(fileNames[0x18 + loadedFile]);
            } while (endImage_f == 0);
        }

        // a32d
        GRAP_0f_SelectPage(1);

        promptMode = 2;
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

        if (hasBanner[screen] == 1)
        {
            switch (screen)
            {
            case 0:
                // a366
                GRAP_4b_PutImage(bannerImage_f, 0, 0xd8, 0, 0);
                GRAP_4b_PutImage(bannerImage_f, 4, 0x98, 0x1c, 0); // a385
                break;

            case 3:
                // a454
                GRAP_4b_PutImage(bannerImage_f, 5, 0xe0, 0, 0);
                GRAP_4b_PutImage(bannerImage_f, 0, 0xb0, 0, 0);
                // -> a385
                break;

            default:
                // a363: NOT MATCHING nop
                ;
            }
        }

        // a38c
        GRAP_4b_PutImage(endImage_f, D_3dee_imageIndex[screen], imageX[screen], imageY[screen], 0);

        textLeftX[0] = D_3da6_textLeft[screen].x;
        textLeftX[1] = D_3da6_textLeft[screen].y;
        textRightX[0] = D_3db2_textRight[screen].x;
        textRightX[1] = D_3db2_textRight[screen].y;
        D_5150_regionTop = D_3dd6_regionTop[screen];
        D_5152_regionBottom = D_3ddc_regionBottom[screen];
        D_5156_penX = D_3de2_startX[screen];
        D_5158_penY = D_3de8_startY[screen];

        ULTIMA_256e_ReadFileFromDisk(/*0x81fe*/ "END.DAT", scratch, 2000, textOffset[screen]);
        DrawJustifiedText(fontImage, scratch); // 7ce6 -> 1778:0566

        if (screen != 0)
        {
            // a40e
            while (PollKey() == 0)
                ;
        }

        // a415
        GRAP_1b_TransferFullscreen(1, 0);
    }

    // a43f
    FreeImage(endImage_f);
    FreeImage(bannerImage_f);
    FreeBitImage(fontImage);

    // a476
    while (PollKey() == 0)
        ;

    GRAP_0f_SelectPage(1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

    // a48b
    do
    {
        endImage_f = LoadImageFile(fileNames[0x1c]); // ENDSC.16
    } while (endImage_f == 0);

    GRAP_4b_PutImage(endImage_f, 0, 0x28, 0, 0);
    FreeImage(endImage_f);
    GRAP_1b_TransferFullscreen(1, 0);
    GRAP_0f_SelectPage(0);
    promptMode = 0;
}

// Adds text to the 39-character line buffer; a line that ends in a newline is printed and the buffer starts
// over.
static void AppendToLine(char* text)
{
    int notused;

    for (; *text != '\0' && lineLen < 39; lineLen++)
    {
        lineBuf[lineLen] = *text++;
    }

    lineBuf[lineLen] = 0;

    if (lineBuf[lineLen - 1] == '\n')
    {
        ULTIMA_1850_PrintString(lineBuf);
        lineLen = 0;
    }
}
Pair D_3da6_textLeft[6] = {0xac, 0x00, 0x00, 0x00, 0x00, 0xc4, 0xb3, 0x00, 0x00, 0xa1, 0x00, 0x00};

Pair2 D_3db2_textRight[6] = {0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x009a, 0x0140};

u16 textOffset[6] = {0x0000, 0x01a8, 0x03bc, 0x05fa, 0x08e8, 0x0b74};

u8 D_3dd6_regionTop[6] = {0x7e, 0x7e, 0x2a, 0x64, 0x52, 0x70};

u8 D_3ddc_regionBottom[6] = {0xc8, 0xc8, 0x94, 0xc8, 0xc8, 0xc8};

u8 D_3de2_startX[6] = {0xac, 0x00, 0x00, 0xb3, 0x00, 0x00};

u8 D_3de8_startY[6] = {0x42, 0x5c, 0x09, 0x26, 0x09, 0x00};

u8 D_3dee_imageIndex[6] = {0x00, 0x01, 0x02, 0x00, 0x01, 0x02};

u8 D_3df4_imageFile[6] = {0x00, 0x00, 0x00, 0x01, 0x01, 0x01};

u8 imageX[6] = {0x00, 0x40, 0x00, 0x00, 0x00, 0xa0};

u8 imageY[6] = {0x00, 0x00, 0x34, 0x00, 0x5c, 0x00};

u8 hasBanner[6] = {0x01, 0x00, 0x00, 0x01, 0x00, 0x00};

char* numberNames[19] =
{
    /*0x8206*/ "One",
    /*0x820a*/ "Two",
    /*0x820e*/ "Three",
    /*0x8214*/ "Four",
    /*0x821a*/ "Five",
    /*0x8220*/ "Six",
    /*0x8224*/ "Seven",
    /*0x822a*/ "Eight",
    /*0x8230*/ "Nine",
    /*0x8236*/ "Ten",
    /*0x823a*/ "Eleven",
    /*0x8242*/ "Twelve",
    /*0x824a*/ "Thirteen",
    /*0x8254*/ "Fourteen",
    /*0x825e*/ "Fifteen",
    /*0x8266*/ "Sixteen",
    /*0x826e*/ "Seventeen",
    /*0x8278*/ "Eighteen",
    /*0x8282*/ "Nineteen"
};

char* tensNames[8] =
{
    /*0x828c*/ "Twenty",
    /*0x8294*/ "Thirty",
    /*0x829c*/ "Forty",
    /*0x82a2*/ "Fifty",
    /*0x82a8*/ "Sixty",
    /*0x82ae*/ "Seventy",
    /*0x82b6*/ "Eighty",
    /*0x82be*/ "Ninety"
};

// Puts a number from 1 to 99 into the line in words ("Twenty-One").
static void SpellNumber(int number)
{
    if (number < 0x15)
    {
        AppendToLine(numberNames[number - 1]);
    }
    else
    {
        AppendToLine(tensNames[number / 10 - 2]);

        if (number % 10)
        {
            AppendToLine(/*0x82c6*/ "-");
            AppendToLine(numberNames[(number % 10) - 1]);
        }
    }
}
char* ordinalNames[12] =
{
    /*0x82c8*/ "First",
    /*0x82ce*/ "Second",
    /*0x82d6*/ "Third",
    /*0x82dc*/ "Fourth",
    /*0x82e4*/ "Fifth",
    /*0x82ea*/ "Sixth",
    /*0x82f0*/ "Seventh",
    /*0x82f8*/ "Eighth",
    /*0x8300*/ "Ninth",
    /*0x8306*/ "Tenth",
    /*0x830c*/ "Eleventh",
    /*0x8316*/ "Twelfth"
};

u8 partyDestX[6] = {0x05, 0x04, 0x06, 0x03, 0x05, 0x07};

u8 partyDestY[6] = {0x05, 0x06, 0x06, 0x07, 0x07, 0x07};

// Puts a number from 1 to 29 into the line as an ordinal ("First", "Twenty-Third").
static void SpellOrdinal(int number)
{
    if (number < 13)
    {
        AppendToLine(ordinalNames[number - 1]);
    }
    else if (number < 20)
    {
        SpellNumber(number);
        AppendToLine(/*0x831e*/ "th");
    }
    else
    {
        AppendToLine(/*0x8322*/ "Twent");
        if (number == 20)
        {
            AppendToLine(/*0x8328*/ "ieth");
        }
        else
        {
            AppendToLine(/*0x832e*/ "y-");
            AppendToLine(ordinalNames[number - 0x15]);
        }
    }
}

// The certificate: today's date in words, the Avatar's name, and how long the quest took (counted from day 5
// of month 4 of year 139). Never returns.
static void PrintCertificate(void)
{
    char numberText_k[8];
    int months;
    int years;
    int days;

    SetTextPosition(0, 1);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(/*0x8332*/ "Be it known that on\n");
    lineLen = 0;
    AppendToLine(/*0x8348*/ "the ");
    SpellOrdinal(D_587e_day);
    AppendToLine(/*0x834e*/ " Day of\n");
    AppendToLine(/*0x8358*/ "the ");
    SpellOrdinal(D_587d_month);
    AppendToLine(/*0x835e*/ " Month\n");
    ULTIMA_1850_PrintString(/*0x8366*/ "of the Year\n");
    SpellNumber((s16)year / 100);
    AppendToLine(/*0x8374*/ " Hundred\n");
    SpellNumber((s16)year % 100);
    AppendToLine(/*0x837e*/ "\n\n");
    AppendToLine(D_55a8_party[0].name);
    AppendToLine(/*0x8382*/ " the Avatar\n\n");
    ULTIMA_1850_PrintString(/*0x8390*/ "saved the life\n");
    ULTIMA_1850_PrintString(/*0x83a0*/ "of our sovereign\n");
    ULTIMA_1850_PrintString(/*0x83b2*/ "Lord British, thereby\n");
    ULTIMA_1850_PrintString(/*0x83ca*/ "saving our people\n");
    ULTIMA_1850_PrintString(/*0x83de*/ "and our land.\n\n");
    SelectCharset(1);
    ULTIMA_1850_PrintString(/*0x83ee*/ "[E@QUE_@OF@[E@AVATAR\n");
    ULTIMA_1850_PrintString(/*0x8404*/ "IS@FOREVER\n\n\n\n");
    SelectCharset(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x8414*/ "Report now, thy Quest compleat in\n");

    // a697
    years = year - 0x8b;
    months = D_587d_month - 4;
    days = D_587e_day - 5;
    if (days < 0)
    {
        // a6ba
        days += 0x1c;
        months--;
    }

    // a6c1
    if (months < 0)
    {
        months += 0xd;
        years--;
    }

    // a6ce
    if (years != 0)
    {
        AppendToLine(itoa(years, numberText_k, 10));
        AppendToLine(/*0x8438*/ " year");
        if (years > 1)
        {
            AppendToLine(/*0x843e*/ "s");
        }

        if (months != 0 || days != 0)
        {
            AppendToLine(/*0x8440*/ ", ");
        }
    }

    if (months != 0)
    {
        AppendToLine(itoa(months, numberText_k, 10));
        AppendToLine(/*0x8444*/ " month");
        if (months > 1)
        {
            AppendToLine(/*0x844c*/ "s");
        }

        if (days != 0)
        {
            AppendToLine(/*0x844e*/ ", ");
        }
    }

    if (days != 0)
    {
        AppendToLine(itoa(days, numberText_k, 10));
        AppendToLine(/*0x8452*/ " day");
        if (days > 1)
        {
            AppendToLine(/*0x8458*/ "s");
        }
    }

    AppendToLine(/*0x845a*/ "\n");
    ULTIMA_1850_PrintString(/*0x845c*/ "to Lord British at Origin Systems!");

    do
    {
        // infinite loop
        days++;
    } while (1);
}

// The pause and the footstep sound of one step.
static void StepPause(void)
{
    WaitFrames(2);
    ULTIMA_433e_AudioFootstep();
    WaitFrames(3);
}

// Moves an actor one cell toward (destX, destY); returns 1 while it moved, 0 when it is there (or the actor
// is unused).
// CHECKED (optimization)
static int StepActorToward(int actorIndex, int destX, int destY)
{
    int distX_d;
    ActorFmt* actor_g;
    int moving;
    int distY;

    moving = 1;
    actor_g = &D_5c5a_actors[actorIndex];

    if (actor_g->tile == 0)
    {
        // a7c0
        moving = 0;
    }
    else if (actor_g->x == destX && actor_g->y == destY) // a7c8
    {
        moving = 0;
    }
    else
    {
        // a7da
        distX_d = abs(actor_g->x - destX);
        distY = abs(actor_g->y - destY);
        if (distX_d < distY)
        {
            if (actor_g->y > destY)
            {
                actor_g->y--;
            }
            else
            {
                // a80c
                actor_g->y++;
            }
        }
        else if (actor_g->x > destX) // a812
        {
            actor_g->x--;
        }
        else
        {
            // a822
            actor_g->x++;
        }

        StepPause();

    }

    return moving;
}

// Half the time moves an actor to a random free neighbouring cell (tile 0x44), then waits a frame.
static void WanderActor(int actorIndex)
{
    int newY;
    int newX;
    ActorFmt* actor_g;
    int tries;
    actor_g = &D_5c5a_actors[actorIndex];

    if (actor_g->tile != 0 && ULTIMA_2092_RandomRange(0, 1) != 0)
    {
        for (tries = 0; tries < 8; tries++)
        {
            newX = actor_g->x;
            newY = actor_g->y;

            switch (ULTIMA_2092_RandomRange(0, 3))
            {
            case 0:
                newX++;
                break;
            case 1:
                newX--;
                break;
            case 2:
                newY++;
                break;
            case 3:
                newY--;
                break;
            }

            if (GetMapViewport(newX, newY) == TILE_MAP_44)
            {
                actor_g->x = newX;
                actor_g->y = newY;
                break;
            }
        }
    }

    WaitFrames(1);
}

// The ending in the throne room: the party walks in and the dead are raised, Lord British thanks the Avatar
// and asks about the sandalwood box and the gem (the good ending, then ENDGAME_0000 and the certificate) or
// says "I see..." (the bad ending, an endless loop of wandering).
// CHECKED
void EndgameMain(void)
{
    int col;
    int index;
    int pending;
    int classIndex;
    ActorFmt* actor_g;
    int key_g;

    curCombatant = 0xff;
    ULTIMA_2900_UpdateVitalsDisplay();
    TilesetModePlus3(1);
    ULTIMA_256e_ReadFileFromDisk(/*0x8480*/ "MISCMAPS.DAT", viewActors, 0xb0, 0x210);
    ULTIMA_256e_ReadFileFromDisk(/*0x848e*/ "ENDMSG.DAT", scratch, 1000, 0);

    for (index = 0; index < 0xb; index++)
    {
        for (col = 0; col < 0xb; col++)
        {
            GetCombatMap(col, index) = viewActors[index * 0x10 + col];
        }
    }

    // a949
    for (index = 0; index < 0x20; index++)
    {
        D_5c5a_actors[index].tile = D_5c5a_actors[index].animTile = 0;
    }

    // a966
    actor_g = &D_5c5a_actors[31];
    actor_g->tile = actor_g->animTile = TILE_ACTOR_7C;
    actor_g->x = 5;
    actor_g->y = 8;
    actor_g->animState = 0;
    WaitFrames(0x28);

    while (StepActorToward(0x1f, 5, 3) != 0)
        ;

    // a9ad
    // the party loop
    for (index = 0; index < D_585b_partyCount; index++)
    {
        if (D_55a8_party[index].status == STATUS_DEAD)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_1850_PrintString(D_55a8_party[index].name);
            ULTIMA_1850_PrintString(/*0x849a*/ " lives!\n");
            D_55a8_party[index].status = STATUS_GOOD;
            D_55a8_party[index].hp = D_55a8_party[index].maxHp;
            ULTIMA_0a70_GRAP_2d_SetPenColor(brightWhiteColor);
            GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
            ULTIMA_2192_AudioPulse(0x2260, 1, 40000, 5000, 1);
            ULTIMA_2900_UpdateVitalsDisplay();
        }

        // aa25
        classIndex = FindIndex(/*0x84a4*/ CLASS_LIST, D_55a8_party[index].cls);

        // aa37
        actor_g = &D_5c5a_actors[index];
        
        actor_g->tile = actor_g->animTile = classTile[classIndex];
        actor_g->x = 5;
        actor_g->y = 9;
        actor_g->animState = 0;

        // aa57
        WaitFrames(1);

        // aa5e: *(partyDestX + index), not partyDestX[index]: only then does MSC 5.0 rotate
        // this while and hoist both bytes into SI/DI, as the original does
        while (StepActorToward(index, *(partyDestX + index), *(partyDestY + index)) != 0)
            ;
    }

    // aabc
    WaitFrames(0x28);
    ULTIMA_1850_PrintString((char*)scratch);
    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_1850_PrintString(/*0x84ae*/ "!\"\n\n");
    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString((char*)&scratch[0x21]);

    do
    {
        key_g = ULTIMA_266c_GetChar();
        switch (key_g)
        {
        case 0x59:
            ULTIMA_1850_PrintString(/*0x84b4*/ "Yes\n\n");
            break;
        case 0x4e:
            ULTIMA_1850_PrintString(/*0x84ba*/ "No\n\n");
            break;
        }
    } while ((uint)key_g != 0x59 && (uint)key_g != 0x4e); // aaf8

    // ab0c
    if (key_g == 0x4e)
    {
        ULTIMA_1850_PrintString((char*)&scratch[0x49]);

        do
        {
            key_g = ULTIMA_266c_GetChar();
            switch (key_g)
            {
            case 0x59:
                ULTIMA_1850_PrintString(/*0x84c0*/ "Yes\n\n");
                break;
            case 0x4e:
                ULTIMA_1850_PrintString(/*0x84c6*/ "No\n\n");
                break;
            }
        } while ((uint)key_g != 0x59 && (uint)key_g != 0x4e);
    }

    // ab49
    if (key_g == 0x59 && sandalwoodBox != 0)
    {
        WaitFrames(8);

        while (StepActorToward(0, 5, 4) != 0)
            ;

        while (StepActorToward(0, 5, 5) != 0)
            ;

        WaitFrames(4);

        actor_g = &D_5c5a_actors[6];
        
        actor_g->tile = actor_g->animTile = TILE_ACTOR_SANDALWOOD_BOX;
        actor_g->x = 5;
        actor_g->y = 4;
        actor_g->animState = 0;

        ULTIMA_1850_PrintString((char*)&scratch[0xab]);
        WaitFrames(0x28);
        ULTIMA_1850_PrintString(/*0x84cc*/ "\n\nHe says:\n\n");
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&scratch[0xd3]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&scratch[0x128]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&scratch[0x167]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&scratch[0x1c9]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&scratch[0x211]);
        ULTIMA_266c_GetChar();
        ULTIMA_1850_PrintString((char*)&scratch[0x24b]);

        D_5c5a_actors[6].animTile = D_5c5a_actors[6].tile = TILE_ACTOR_GEM;

        ULTIMA_266c_GetChar();
        ULTIMA_2192_AudioPulse(0x1450, 1, 50000, 10000, 1);

        D_5c5a_actors[6].animTile = D_5c5a_actors[6].tile = 0;
        GetCombatMap(5, 4) = 0xdc;

        for (moongateHeight = 1; moongateHeight < 0x10; moongateHeight++)
        {
            WaitFrames(1);
        }

        WaitFrames(4);

        while (StepActorToward(0x1f, 5, 4) != 0)
            ;

        D_5c5a_actors[31].tile = D_5c5a_actors[31].animTile = 0;

        // ac64
        WaitFrames(1);

        for (index = 0; index < D_585b_partyCount; index++)
        {
            while (StepActorToward(index, 5, 4) != 0)
                ;

            D_5c5a_actors[index].tile = D_5c5a_actors[index].animTile = 0;
            WaitFrames(1);
        }

        for (moongateHeight = 0xf; moongateHeight > 0; moongateHeight--)
        {
            WaitFrames(1);
        }

        GRAP_51_PutTile(0x44, 5, 4);
        GRAP_0f_SelectPage(1);
        ULTIMA_0a70_GRAP_2d_SetPenColor(0);
        UnloadTileset();
        ULTIMA_0aa6_GRAP_3f_FillRect(0, 0, 0x13f, 199);
        GRAP_0f_SelectPage(0);
        ShowEndScreens();
        PrintCertificate();
        return;
    }

    // bad endgame
    ULTIMA_1850_PrintString(/*0x84da*/ "\"I see...\n");
    WaitFrames(0x28);
    ULTIMA_1850_PrintString((char*)&scratch[0x2d5]);
    D_5c5a_actors[0].y--;
    StepPause();

    do
    {
        pending = StepActorToward(2, 8, 6);
        pending += StepActorToward(0x1f, 4, 1);
        pending += StepActorToward(0, 8, 4);
    } while (pending != 0);

    do
    {
        WanderActor(1);
        WanderActor(3);
        WanderActor(4);
        WanderActor(5);
    } while (1);
}
