#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "lookobj.h"

void GRAP_30_Pset(int param_1, int param_2);
int SelectPlayer();
void DrawMap(void);

// the description of a tile, from LOOK2.DAT (offset table at tile * 2)
static void PrintTileName(int tile)
{
    char text[0x80];
    u16 textOffset_c;

    textOffset_c = 0;
    ReadFileFromDisk(/*0x71f8*/ "LOOK2.DAT", &textOffset_c, 2, tile << 1);
    ReadFileFromDisk(/*0x7202*/ "LOOK2.DAT", text, 0x80, textOffset_c);
    PrintString(text);
}

// "a well": drop a coin and wish; a vehicle's name (Corvette, Horse...) in towns 0x16 or 0x1f makes a horse appear
static void Well(byte wellX, byte wellY, byte wellLevel)
{
    char answer;
    char wish_e[14];
    int slot_o;

    PrintString(/*0x720c*/ "a well.\n\nDrop a coin?");

    while ((answer = GetChar()) != 'Y' && answer != 'N')
    {
    }

    if (answer == 'N')
    {
        PrintString(/*0x7222*/ "No\n");
    }
    else
    {
        PrintString(/*0x7226*/ "Yes\n");
        if (gold == 0)
        {
            return;
        }

        PrintString(/*0x722c*/ "\nThy wish?\n");
        gold--;
        GetString(wish_e, 0xc);
        if (wish_e[0] == 0)
        {
            PrintString(/*0x7238*/ "Nothing\n");
            return;
        }

        if (CompareWord(/*0x7242*/ "Corvette", wish_e) > -1 || CompareWord(/*0x724c*/ "Ferrari", wish_e) > -1 ||
            CompareWord(/*0x7254*/ "Lamborghini", wish_e) > -1 || CompareWord(/*0x7260*/ "Lotus", wish_e) > -1 ||
            CompareWord(/*0x7266*/ "Porsche", wish_e) > -1 || CompareWord(/*0x726e*/ "Horse", wish_e) > -1)
        {
            if (mapId != 0x16 && mapId != 0x1f)
            {
                PrintString(/*0x7274*/ "\nNo effect...\n");
            }
            else
            {
                PrintString(/*0x7284*/ "\nPoof!\n");
                AudioWhiteNoise(10, 3000, 2000);
                slot_o = FindActorSlot();
                SetActor(TILE_ACTOR_HORSE, TILE_ACTOR_HORSE, wellX + 1, wellY, wellLevel, 0, slot_o);
            }
        }
        else
        {
            PrintString(/*0x728c*/ "\nNo effect...\n");
        }
    }
}

// "a gurgling fountain": who will drink? the dead and the asleep are incapacitated, nobody is healed
static void Fountain(void)
{
    byte status;
    int member_h;

    PrintString(/*0x729c*/ "a gurgling fountain!\n\n");
    PrintString(/*0x72b4*/ "Who will drink?\n");

    if ((member_h = SelectPlayer()) == -1)
    {
        PrintString(/*0x72c6*/ "None!\n");
    }
    else
    {
        status = party[member_h].status;
        if (status == STATUS_DEAD || status == STATUS_SLEEP)
        {
            PrintString(/*0x72ce*/ "Incapacitated!\n\n");
        }
        else
        {
            PrintString(/*0x72e0*/ "Refreshing...\n");
        }
    }
}

// a white star at the sky's cell (x, y): 3x3 pixels, with dots to each side, clipped at the right edge
static void DrawStar(int starX, int starY)
{
    int dx;
    int dy_d;

    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_30_Pset(starX * 8 + 6, starY * 8 + 8);

    for (dy_d = 0; dy_d < 3; dy_d++)
    {
        for (dx = 0; dx < 3; dx++)
        {
            if (starX * 8 + dx <= 0xb0)
            {
                GRAP_30_Pset(starX * 8 + dx + 7, starY * 8 + dy_d + 7);
            }
        }
    }

    if (starX * 8 <= 0xad)
    {
        GRAP_30_Pset(starX * 8 + 10, starY * 8 + 8);
    }
}

// a red mark (a flame shape) at the star's cell, for a planet in front of which a Shadowlord stands
static void DrawRedMark(int starX, int starY)
{
    int pixelY;
    int pixelX;

    pixelX = starX * 8;
    pixelY = starY * 8;

    GRAP_2d_SetPenColor(redColor);

    if (pixelX > 2)
    {
        GRAP_3c_VertLine(pixelX + 5, pixelY + 10, pixelY + 0xc);
    }

    if (pixelX > 2)
    {
        GRAP_3c_VertLine(pixelX + 6, pixelY + 10, pixelY + 0xc);
    }

    if (pixelX > 2)
    {
        GRAP_3c_VertLine(pixelX + 7, pixelY + 8, pixelY + 0xc);
    }

    if (pixelX <= 0xaf)
    {
        GRAP_3c_VertLine(pixelX + 8, pixelY + 8, pixelY + 0xc);
    }

    if (pixelX <= 0xae)
    {
        GRAP_3c_VertLine(pixelX + 9, pixelY + 6, pixelY + 10);
    }

    if (pixelX <= 0xad)
    {
        GRAP_3c_VertLine(pixelX + 10, pixelY + 6, pixelY + 10);
    }

    if (pixelX <= 0xac)
    {
        GRAP_3c_VertLine(pixelX + 0xb, pixelY + 5, pixelY + 8);
    }

    if (pixelX <= 0xab)
    {
        GRAP_3c_VertLine(pixelX + 0xc, pixelY + 5, pixelY + 7);
    }
}

// CHECKED
// by day "the sun!" (it burns the active player); at night draws the sky and the eight planets for the date
void LookSky(void)
{
    int day;
    int month;
    int planet;
    int yearOfCentury_b;
    int starY;
    byte key_j;
    int col_e;
    int starX;
    int row_k;
    int orbitPos;

    if (D_587f_hour >= 6 && D_587f_hour < 0x12)
    {
        PrintString(/*0x72f0*/ "the sun!\n");
        if (activePlayer == 0xff && GetFirstActivePartyMember() == 0)
        {
            activePlayer = tempX;
        }

        DamagePlayer(activePlayer, 1);
        UpdateVitalsDisplay();
    }
    else
    {
        viewDirty = 1;
        for (row_k = 0; row_k < 0xb; row_k++)
        {
            // memset?
            for (col_e = 0; col_e < 0xb; col_e++)
            {
                GetMapViewport(col_e, row_k) = 0xff;
            }
        }

        GetMapViewport(5, 10) = 0x59;
        DrawMap();
        GRAP_2d_SetPenColor(blueColor + 8);

        for (row_k = 0; row_k < 0x50; row_k++)
        {
            GRAP_30_Pset(RandomRange(9, 0xb6), RandomRange(9, 0xac));
        }

        for (planet = 0; planet < 8; planet++)
        {
            yearOfCentury_b = (int)year % 100;
            month = D_587d_month;
            day = D_587e_day;
            orbitPos = planetStartPos[planet];
            while (yearOfCentury_b > 0x27 || (yearOfCentury_b == 0x27 && month > 4) || (yearOfCentury_b == 0x27 && month == 4 && day > 5))
            {
                day--;
                if (day == 0)
                {
                    day = 0x1c;
                    month--;
                }

                if (month == 0)
                {
                    month = 0xd;
                    yearOfCentury_b--;
                }

                do
                {
                    orbitPos--;
                    if (orbitPos < 0)
                    {
                        orbitPos = 0x15;
                    }
                } while (orbitSteps[planet * 0x16 + orbitPos] == 0);
            }

            starX = orbitPos + 1;
            starY = planetRow[planet];
            DrawStar(starX, starY);
            starX--;
            for (col_e = 0; col_e < 3; col_e++)
            {
                if (shadowlordTowns[col_e] == planet + 1)
                {
                    DrawRedMark(starX, starY);
                }
            }
        }

        PrintString(/*0x72fa*/ "the night sky! ");

        do
        {
        } while ((key_j = PollKeyWithCursor()) == 0);
    }
}

// what a tile is: the sky, a well, a fountain, a clock (the time), the altars' words, a dungeon's name, else its LOOK2.DAT text
static void DescribeTile(int tile, int tileX, int tileY)
{
    int hour12;

    while (tile == TILE_MAP_E0 || tile == TILE_MAP_E1 || tile == TILE_MAP_E2)
    {
        switch (tile)
        {
        case TILE_MAP_E0:
            tileY--;
            break;

        case TILE_MAP_E1:
            tileX++;
            break;

        case TILE_MAP_E2:
            tileX--;
            break;
        }

        tile = *GetTileAddr(tileX, tileY);
    }

    if (tile == TILE_MAP_59)
    {
        LookSky();
    }
    else if (tile == TILE_MAP_WELL)
    {
        Well(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    }
    else if ((tile & 0xfc) == TILE_MAP_FOUNTAIN)
    {
        Fountain();
    }
    else
    {
        PrintTileName(tile);
        if ((tile & 0xfe) == TILE_MAP_CLOCK)
        {
            hour12 = D_587f_hour % 0xc;
            if (hour12 == 0)
            {
                hour12 = 0xc;
            }

            PrintNumber(hour12, 1, 0x20);
            PrintChar(':');
            PrintNumber(minute, 2, 0x30);

            if (D_587f_hour > 0xb)
            {
                PrintString(/*0x730a*/ " PM.\n");
            }
            else
            {
                PrintString(/*0x7310*/ " AM.\n");
            }
        }
        else if (tile == TILE_MAP_DE)
        {
            switch (mapId)
            {
            case 0x1e:
                PrintString(/*0x7316*/ "Truth\n");
                break;

            case 0x1f:
                PrintString(/*0x731e*/ "Love\n");
                break;

            case 0x20:
                PrintString(/*0x7324*/ "Courage\n");
                break;
            }
        }
        else if (tile == TILE_MAP_DF)
        {
            switch (tileX)
            {
            case 0xf0:
                PrintString(/*0x732e*/ "Deceit\n");
                break;

            case 0x5b:
                PrintString(/*0x7336*/ "Despise\n");
                break;

            case 0x48:
                PrintString(/*0x7340*/ "Destard\n");
                break;

            case 0x7e:
                PrintString(/*0x734a*/ "Wrong\n");
                break;

            case 0x9c:
                PrintString(/*0x7352*/ "Covetous\n");
                break;

            case 0x3a:
                PrintString(/*0x735c*/ "Shame\n");
                break;

            case 0xef:
                PrintString(/*0x7364*/ "Hythloth\n");
                break;

            case 0x80:
                PrintString(/*0x736e*/ "Doom\n");
                break;
            }
        }
        else
        {
            if (GetCurrentTextX() != 0)
            {
                PrintChar('\n');
            }
        }
    }
}

// the description of the creature or person looked at (LOOK2.DAT, offset table at 0x200 + tile * 2)
static void DescribeActor(int actorTile)
{
    char text[128];
    u16 textOffset_c;

    textOffset_c = 0;

    // NOTE: FMT does some other things here

    ReadFileFromDisk(/*0x7374*/ "LOOK2.DAT", &textOffset_c, 2, actorTile * 2 + 0x200);
    ReadFileFromDisk(/*0x737e*/ "LOOK2.DAT", text, 0x80, textOffset_c);

    PrintString(text);

    if (GetCurrentTextX() != 0)
    {
        PrintChar('\n');
    }
}
u8 planetStartPos[8] = { 0x12, 0x02, 0x08, 0x0f, 0x0b, 0x06, 0x04, 0x02 };

u8 planetRow[8] = { 0x12, 0x11, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x01 };

u8 orbitSteps[0xb0] =
{
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01,
    0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};

char* lookPictures[9] =
{
    /*0x7388*/ "g              g",
    /*0x739a*/ "jlllllllnllllllk",
    /*0x73ac*/ "8lllllllmllllll9",
    /*0x73be*/ "jllllllllllllllk",
    /*0x73d0*/ "8llllllllllllll9",
    /*0x73e2*/ "hllllk    jlllli",
    /*0x73f4*/ "jlllli    hllllk",
    /*0x7406*/ "     g    g     ",
    /*0x7418*/ "     hlllli     "
};

u8 shoreLandMask[16] =
{
    0x05, 0x06, 0x04, 0x01, 0x03, 0x06, 0x05, 0x07,
    0x07, 0x0e, 0x00, 0x00, 0x01, 0x02, 0x08, 0x04
};

u8 edgeBits[8] = { 0x0a, 0x05, 0x0c, 0x06, 0x03, 0x09, 0x0f, 0x00 };

// a sign's text from the scratch buffer (-1: the default "Live by the Eight Laws" sign): frame characters, a key press at 0xd
static void PrintSignText(int signPos)
{
    byte code;
    char* picture_c;

    if (signPos == -1)
    {
        SelectCharset(1);
        PrintString(/*0x742a*/ "abbbbbbbbbbbbbbc\ng              g\ng  LIVE@BY@[E  g\n");
        PrintString(/*0x745e*/ "g  EIGHT@LAWS  g\ng              g\ndeeeeeeeeeeeeeef\n");
        SelectCharset(0);
    }
    else
    {
        for (signPos = signPos + 4; scratch[signPos] == '\n'; signPos += 6)
        {
        }

        do
        {
            code = scratch[signPos];
            if (code & 0x80)
            {
                SelectCharset(0);
            }
            else
            {
                SelectCharset(1);
            }

            if (code == 0x26 || code == 0x27)
            {
                PrintChar(0x6c);
            }
            else if (code >= 0x29 && code <= 0x31)
            {
                SelectCharset(1);
                for (picture_c = lookPictures[code - 0x29]; *picture_c != '\0';)
                {
                    PrintChar((byte)*picture_c++);
                }
            }
            else if (code == 0xd)
            {
                SelectCharset(0);
                code = GetChar();
            }
            else
            {
                PrintChar(code & 0x7f);
            }
        } while (scratch[++signPos] != 0);

        SelectCharset(0);

        if (GetCurrentTextX() != 0)
        {
            PrintChar('\n');
        }
    }
}

// the sign at (level, x, y): the WANTED poster of the party in town 4, else SIGNS.DAT's text for the place
static void ReadSign(byte signLevel, byte signX, byte signY)
{
    s16 signOffsets[33];
    int signPos;
    int done_j;

    done_j = 0;
    if (mapId == 0x4 && signLevel == 0 && signX == 0x11 && signY == 0x15)
    {
        SelectCharset(1);
        PrintString(/*0x7492*/ "abbbbbbbbbbbbbc\ng   ");
        SelectCharset(0);
        PrintString(/*0x74a8*/ "Wanted:   ");
        SelectCharset(1);
        PrintString(/*0x74b4*/ "g\ng");
        SetTextPosition(0xe, GetCurrentTextY());
        PrintString(/*0x74b8*/ "g\ng");
        for (signPos = 0; signPos < 3; signPos++)
        {
            if (signPos < partyCount)
            {
                SelectCharset(0);
                SetTextPosition(7 - (StringLength(party[signPos].name) / 2),
                    GetCurrentTextY());
                PrintString(party[signPos].name);
                SelectCharset(1);
            }

            SetTextPosition(0xe, GetCurrentTextY());
            PrintString(/*0x74bc*/ "g\ng");
        }

        SetTextPosition(0xe, GetCurrentTextY());
        PrintString(/*0x74c0*/ "g\ng");
        SelectCharset(0);
        PrintString(/*0x74c4*/ "Dead or Alive");
        SelectCharset(1);
        PrintString(/*0x74d2*/ "g\ndeeeeeeeeeeeeef\n");
        SelectCharset(0);
    }
    else
    {
        ReadFileFromDisk(/*0x74e6*/ "signs.dat", signOffsets, 0x42, 0);
        signPos = signOffsets[mapId];
        memset(scratch, 0xff, SCRATCH_SIZE);

        if (signPos == 0)
        {
            PrintSignText(-1);
        }
        else
        {
            ReadFileFromDisk(/*0x74f0*/ "signs.dat", scratch, SCRATCH_SIZE, signPos);
            signPos = 0;
            // di = done_j
            // si = signPos
            do
            {
                if (scratch[signPos + 1] == signLevel && scratch[signPos + 2] == signX && scratch[signPos + 3] == signY)
                {
                    PrintSignText(signPos);
                    done_j++;
                }
                else
                {
                    signPos += 4;
                    while (scratch[signPos++] != 0)
                    {
                    }

                    done_j = scratch[signPos] == 0xff ? 1 : 0;

                    if (done_j)
                    {
                        PrintSignText(-1);
                    }
                }
            } while (done_j == 0);
        }
    }
}

// the Look command: a direction, then what is there: the creature, a sign, a tile; a tile 0x29 (a crystal ball?) gives a death vision or the view of the whole map
void LookCmd(void)
{
    int member_g;
    int lookY;
    int lookX;
    int actorTile;
    int tile_b;

    if (SelectDirection() == 0)
    {
        return;
    }

    lookX = D_5896_mapX + tempX;
    lookY = D_5897_mapY + tempY;
    tile_b = *GetTileAddr(lookX, lookY);
    actorTile = FindActorTileAtPos(lookX, lookY, D_5895_mapLevel);
    if (tile_b == TILE_MAP_29)
    {
        if ((member_g = GetActingPlayer()) == -1)
        {
            return;
        }

        if (party[member_g].intel <= RandomRange(1, 0x1e))
        {
            PrintString(/*0x74fa*/ "Death vision!\n");
            DamagePlayer(member_g, 1);
            UpdateVitalsDisplay();
            return;
        }
        PrintString(/*0x750a*/ "Strange vision!\n");
        LookobjViewCmd(D_5896_mapX, D_5897_mapY);
        return;
    }

    PrintString(/*0x751c*/ "\nThou dost see\n");
    if (actorTile != 0)
    {
        DescribeActor(actorTile);
        return;
    }

    switch (tile_b)
    {
    case TILE_MAP_89:
    case TILE_MAP_8A:
    case TILE_MAP_A0:
    case TILE_MAP_A4:
    case TILE_MAP_SIGN_F8:
        PrintString(/*0x752c*/ "\n");
        ReadSign(D_5895_mapLevel, lookX, lookY);
        break;

    default:
        DescribeTile(tile_b, lookX, lookY);
        break;
    }
}

// the pixel position (D_5876/D_5878) of the vision's cell (x, y): 4 pixels a cell, at 0x20
static void SetCellOrigin(int cellX, int cellY)
{
    tempX = cellX * 4 + 0x20;
    tempY = cellY * 4 + 0x20;
}

// 4 green dots in a checker
static void DrawGreenDots(void)
{
    GRAP_2d_SetPenColor(greenColor + 8);
    GRAP_30_Pset(tempX + 1, tempY);
    GRAP_30_Pset(tempX + 1, tempY + 2);
    GRAP_30_Pset(tempX + 3, tempY + 1);
    GRAP_30_Pset(tempX + 3, tempY + 3);
}

// a solid light green 4x4 cell
static void DrawGreenBlock(void)
{
    GRAP_2d_SetPenColor(greenColor + 8);
    GRAP_3f_FillRect(tempX, tempY, tempX + 3, tempY + 3);
}

// two white vertical bars
static void DrawVertBars(void)
{
    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(tempX, tempY, tempX, tempY + 3);
    GRAP_Line(tempX + 2, tempY, tempX + 2, tempY + 3);
}

// white lines along the top and bottom
static void DrawHorizBars(void)
{
    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(tempX, tempY, tempX + 3, tempY);
    GRAP_Line(tempX, tempY + 3, tempX + 3, tempY + 3);
}

// two short white lines in the middle
static void DrawInnerBars(void)
{
    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(tempX + 1, tempY + 1, tempX + 2, tempY + 1);
    GRAP_Line(tempX + 1, tempY + 2, tempX + 2, tempY + 2);
}

// a white frame around the cell
static void LookobjDrawFrame(void)
{
    GRAP_2d_SetPenColor(brightWhiteColor);
    GRAP_Line(tempX, tempY, tempX + 3, tempY);
    GRAP_Line(tempX, tempY + 3, tempX + 3, tempY + 3);
    GRAP_Line(tempX, tempY + 1, tempX, tempY + 2);
    GRAP_Line(tempX + 3, tempY + 1, tempX + 3, tempY + 2);
}

// yellow blocks in two corners
static void DrawYellowSteps(void)
{
    GRAP_2d_SetPenColor(brightYellowColor);
    GRAP_Line(tempX, tempY, tempX + 1, tempY);
    GRAP_Line(tempX, tempY + 1, tempX + 1, tempY + 1);
    GRAP_Line(tempX + 2, tempY + 2, tempX + 3, tempY + 2);
    GRAP_Line(tempX + 2, tempY + 3, tempX + 3, tempY + 3);
}

// green lines on rows 0 and 2 and two dots
static void DrawGreenStripes(void)
{
    GRAP_2d_SetPenColor(greenColor + 8);
    GRAP_Line(tempX, tempY, tempX + 3, tempY);
    GRAP_Line(tempX, tempY + 2, tempX + 3, tempY + 2);
    GRAP_30_Pset(tempX + 2, tempY + 1);
    GRAP_30_Pset(tempX, tempY + 3);
}

// CHECKED
// four dots: green on the land corners of a shore tile (0x60-0x6f, D_3822 masks), water-coloured on the others
static void DrawShore(int tile)
{
    int cornerBit;

    int corner_j;
    int colors[4];

    cornerBit = 8;

    for (corner_j = 0; corner_j < 4; corner_j++)
    {
        if ((tile & 0xf0) == 0x60 && (shoreLandMask[tile & 0xf] & cornerBit) == 0)
        {
            colors[corner_j] = greenColor + 8;
        }
        else if (D_52c8_videoDriver == 0 || D_52c8_videoDriver == 3)
        {
            colors[corner_j] = magentaColor;
        }
        else
        {
            colors[corner_j] = blueColor + 8;
        }

        cornerBit >>= 1;
    }

    GRAP_2d_SetPenColor(colors[0]);
    GRAP_30_Pset(tempX + 1, tempY);
    GRAP_2d_SetPenColor(colors[1]);
    GRAP_30_Pset(tempX + 3, tempY + 1);
    GRAP_2d_SetPenColor(colors[2]);
    GRAP_30_Pset(tempX + 1, tempY + 2);
    GRAP_2d_SetPenColor(colors[3]);
    GRAP_30_Pset(tempX + 3, tempY + 3);
}

// two water-coloured dots (magenta on drivers 0 and 3, else light blue)
static void DrawWaterDots(void)
{
    if (D_52c8_videoDriver == 0 || D_52c8_videoDriver == 3)
    {
        GRAP_2d_SetPenColor(magentaColor);
    }
    else
    {
        GRAP_2d_SetPenColor(blueColor + 8);
    }

    GRAP_30_Pset(tempX, tempY);
    GRAP_30_Pset(tempX + 2, tempY + 2);
}

// two green dots and two water-coloured dots
static void DrawMarsh(void)
{
    GRAP_2d_SetPenColor(greenColor + 8);
    GRAP_30_Pset(tempX + 1, tempY);
    GRAP_30_Pset(tempX + 3, tempY + 1);

    if ((D_52c8_videoDriver == 0) || (D_52c8_videoDriver == 3))
    {
        GRAP_2d_SetPenColor(magentaColor);
    }
    else
    {
        GRAP_2d_SetPenColor(blueColor + 8);
    }

    GRAP_30_Pset(tempX, tempY + 2);
    GRAP_30_Pset(tempX + 2, tempY + 3);
}

// tiles 0x20-0x26: green dots, a red centre, white edges as D_3832 says and a black dot in the corner of some
static void DrawEdgedBlock(int tile)
{
    int edges;

    DrawGreenDots();
    GRAP_2d_SetPenColor(redColor);
    GRAP_3f_FillRect(tempX + 1, tempY + 1, tempX + 2, tempY + 2);

    // tile: 0x20 ~ 0x26 (DrawMapCell -> cellPattern -> val == 0x10)
    edges = edgeBits[tile - 0x20];
    if ((edges & 8) != 0)
    {
        GRAP_Line(tempX + 1, tempY, tempX + 2, tempY);
    }
    if ((edges & 4) != 0)
    {
        GRAP_Line(tempX + 3, tempY + 1, tempX + 3, tempY + 2);
    }
    if ((edges & 2) != 0)
    {
        GRAP_Line(tempX + 1, tempY + 3, tempX + 2, tempY + 3);
    }
    if ((edges & 1) != 0)
    {
        GRAP_Line(tempX, tempY + 1, tempX, tempY + 2);
    }

    GRAP_2d_SetPenColor(0);

    switch (tile)
    {
    case 0x22:
        GRAP_30_Pset(tempX + 1, tempY + 2);
        break;

    case 0x23:
        GRAP_30_Pset(tempX + 1, tempY + 1);
        break;

    case 0x24:
        GRAP_30_Pset(tempX + 2, tempY + 1);
        break;

    case 0x25:
        GRAP_30_Pset(tempX + 2, tempY + 2);
        break;
    }
}

// draws one cell of the vision by the pattern D_1d1a gives its tile (0 nothing, 1-16 the pictures above, 90 white)
static void DrawMapCell(int tile, int cellX, int cellY)
{
    int pattern;

    pattern = cellPattern[tile];
    SetCellOrigin(cellX, cellY);

    switch (pattern)
    {
    case 0:
        break;
    case 1: // 0fba
        DrawGreenDots();
        break;
    case 2: // 0fc0
        DrawGreenBlock();
        break;
    case 3: // 0fc6
        GRAP_2d_SetPenColor(redColor);
        GRAP_3f_FillRect(tempX, tempY, tempX + 3, tempY + 3);
        break;
    case 4: // 0fea
        DrawHorizBars();
        break;
    case 5: // 0ff0
        DrawInnerBars();
        break;
    case 6: // 0ff6
        LookobjDrawFrame();
        break;
    case 7: // 0ffc -> 0fca
        GRAP_2d_SetPenColor(brightWhiteColor);
        GRAP_3f_FillRect(tempX, tempY, tempX + 3, tempY + 3);
        break;
    case 8: // 1002
        DrawYellowSteps();
        break;
    case 9: // 1008
        DrawGreenStripes();
        break;
    case 10: // 1011
        DrawShore(tile);
        break;
    case 11: // 1018
        DrawWaterDots();
        break;
    case 12: // 101e
        if ((D_52c8_videoDriver == 0) || (D_52c8_videoDriver == 3))
        {
            GRAP_2d_SetPenColor(magentaColor);
        }
        else
        {
            GRAP_2d_SetPenColor(blueColor + 8);
        }
        GRAP_30_Pset(tempX + 2, tempY + 2);
        break;
    case 13: // 1050
        DrawMarsh();
        break;
    case 14: // 1056
        GRAP_2d_SetPenColor(brightWhiteColor);
        GRAP_Line(tempX + 1, tempY, tempX + 1, tempY + 3);
        GRAP_Line(tempX + 2, tempY, tempX + 2, tempY + 3);
        break;
    case 15: // 108c
        GRAP_2d_SetPenColor(blueColor + 8);
        GRAP_3f_FillRect(tempX, tempY, tempX + 3, tempY + 3);
        break;
    case 16: // 1096
        DrawEdgedBlock(tile);
        break;
    case 90: // 0ffc
        GRAP_2d_SetPenColor(brightWhiteColor);
        GRAP_3f_FillRect(tempX, tempY, tempX + 3, tempY + 3);
        break;
    }
}

// the vision: draws the 32x32 map around the origin four pixels a cell and blinks the party's cell until a key
void LookobjViewCmd(int viewX, int viewY)
{
    int col;
    int row;

    viewX -= mapOriginX;
    viewY -= mapOriginY;
    viewX &= 0xff;
    viewY &= 0xff;

    GRAP_0f_SelectPage(1);
    GRAP_2d_SetPenColor(0);
    GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);

    for (row = 0; row < 0x20; row++)
    {
        for (col = 0; col < 0x20; col++)
        {
            DrawMapCell(*GetTileAddr((uint)mapOriginX + col, (uint)mapOriginY + row),
                           col, row);
        }
    }

    GRAP_18_TransferArea(1, 0, 8, 8, 0xb7, 0xb7);
    GRAP_0f_SelectPage(0);
    SetCellOrigin(viewX, viewY);

    row = 0;
    while (PollKeyWithCursor() == 0)
    {
        row++;
        if (row == 4)
        {
            row = 0;
            GRAP_2d_SetPenColor(brightWhiteColor);
            GRAP_XorFillRect(tempX, tempY, tempX + 3, tempY + 3);
        }
    }

    UpdateFrame();
}
