#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "lookobj.h"

void ULTIMA_0c64_GRAP_30_Pset(int param_1, int param_2);
int ULTIMA_2e8e_SelectPlayer();
void ULTIMA_56ac_DrawMap(void);

// the description of a tile, from LOOK2.DAT (offset table at tile * 2)
static void LOOKOBJ_0000_PrintTileName(int tile)
{
    char text[0x80];
    u16 textOffset_c;

    textOffset_c = 0;
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0x71f8, "LOOK2.DAT"), &textOffset_c, 2, tile << 1);
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0x7202, "LOOK2.DAT"), text, 0x80, textOffset_c);
    ULTIMA_1850_PrintString(text);
}

// "a well": drop a coin and wish; a vehicle's name (Corvette, Horse...) in towns 0x16 or 0x1f makes a horse appear
static void LOOKOBJ_0042_Well(byte wellX, byte wellY, byte wellLevel)
{
    char answer;
    char wish_e[14];
    int slot_o;

    ULTIMA_1850_PrintString(_TEXT(0x720c, "a well.\n\nDrop a coin?"));

    while ((answer = ULTIMA_266c_GetChar()) != 'Y' && answer != 'N')
    {
    }

    if (answer == 'N')
    {
        ULTIMA_1850_PrintString(_TEXT(0x7222, "No\n"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x7226, "Yes\n"));
        if (D_57aa_gold == 0)
        {
            return;
        }

        ULTIMA_1850_PrintString(_TEXT(0x722c, "\nThy wish?\n"));
        D_57aa_gold--;
        ULTIMA_3b1c_GetString(wish_e, 0xc);
        if (wish_e[0] == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x7238, "Nothing\n"));
            return;
        }

        if (ULTIMA_6f1e_CompareWord(_TEXT(0x7242, "Corvette"), wish_e) > -1 || ULTIMA_6f1e_CompareWord(_TEXT(0x724c, "Ferrari"), wish_e) > -1 ||
            ULTIMA_6f1e_CompareWord(_TEXT(0x7254, "Lamborghini"), wish_e) > -1 || ULTIMA_6f1e_CompareWord(_TEXT(0x7260, "Lotus"), wish_e) > -1 ||
            ULTIMA_6f1e_CompareWord(_TEXT(0x7266, "Porsche"), wish_e) > -1 || ULTIMA_6f1e_CompareWord(_TEXT(0x726e, "Horse"), wish_e) > -1)
        {
            if (D_5893_mapId != 0x16 && D_5893_mapId != 0x1f)
            {
                ULTIMA_1850_PrintString(_TEXT(0x7274, "\nNo effect...\n"));
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x7284, "\nPoof!\n"));
                ULTIMA_223c_AudioWhiteNoise(10, 3000, 2000);
                slot_o = ULTIMA_38e4_FindActorSlot();
                ULTIMA_3a74_SetActor(TILE_ACTOR_HORSE, TILE_ACTOR_HORSE, wellX + 1, wellY, wellLevel, 0, slot_o);
            }
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x728c, "\nNo effect...\n"));
        }
    }
}

// "a gurgling fountain": who will drink? the dead and the asleep are incapacitated, nobody is healed
static void LOOKOBJ_0162_Fountain(void)
{
    byte status;
    int member_h;

    ULTIMA_1850_PrintString(_TEXT(0x729c, "a gurgling fountain!\n\n"));
    ULTIMA_1850_PrintString(_TEXT(0x72b4, "Who will drink?\n"));

    if ((member_h = ULTIMA_2e8e_SelectPlayer()) == -1)
    {
        ULTIMA_1850_PrintString(_TEXT(0x72c6, "None!\n"));
    }
    else
    {
        status = D_55a8_party[member_h].status;
        if (status == STATUS_DEAD || status == STATUS_SLEEP)
        {
            ULTIMA_1850_PrintString(_TEXT(0x72ce, "Incapacitated!\n\n"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x72e0, "Refreshing...\n"));
        }
    }
}

// a white star at the sky's cell (x, y): 3x3 pixels, with dots to each side, clipped at the right edge
static void LOOKOBJ_01ac_DrawStar(int starX, int starY)
{
    int dx;
    int dy_d;

    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0c64_GRAP_30_Pset(starX * 8 + 6, starY * 8 + 8);

    for (dy_d = 0; dy_d < 3; dy_d++)
    {
        for (dx = 0; dx < 3; dx++)
        {
            if (starX * 8 + dx <= 0xb0)
            {
                ULTIMA_0c64_GRAP_30_Pset(starX * 8 + dx + 7, starY * 8 + dy_d + 7);
            }
        }
    }

    if (starX * 8 <= 0xad)
    {
        ULTIMA_0c64_GRAP_30_Pset(starX * 8 + 10, starY * 8 + 8);
    }
}

// a red mark (a flame shape) at the star's cell, for a planet in front of which a Shadowlord stands
static void LOOKOBJ_024c_DrawRedMark(int starX, int starY)
{
    int pixelY;
    int pixelX;

    pixelX = starX * 8;
    pixelY = starY * 8;

    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor);

    if (pixelX > 2)
    {
        ULTIMA_0cf2_GRAP_3c_VertLine(pixelX + 5, pixelY + 10, pixelY + 0xc);
    }

    if (pixelX > 2)
    {
        ULTIMA_0cf2_GRAP_3c_VertLine(pixelX + 6, pixelY + 10, pixelY + 0xc);
    }

    if (pixelX > 2)
    {
        ULTIMA_0cf2_GRAP_3c_VertLine(pixelX + 7, pixelY + 8, pixelY + 0xc);
    }

    if (pixelX <= 0xaf)
    {
        ULTIMA_0cf2_GRAP_3c_VertLine(pixelX + 8, pixelY + 8, pixelY + 0xc);
    }

    if (pixelX <= 0xae)
    {
        ULTIMA_0cf2_GRAP_3c_VertLine(pixelX + 9, pixelY + 6, pixelY + 10);
    }

    if (pixelX <= 0xad)
    {
        ULTIMA_0cf2_GRAP_3c_VertLine(pixelX + 10, pixelY + 6, pixelY + 10);
    }

    if (pixelX <= 0xac)
    {
        ULTIMA_0cf2_GRAP_3c_VertLine(pixelX + 0xb, pixelY + 5, pixelY + 8);
    }

    if (pixelX <= 0xab)
    {
        ULTIMA_0cf2_GRAP_3c_VertLine(pixelX + 0xc, pixelY + 5, pixelY + 7);
    }
}

// CHECKED
// by day "the sun!" (it burns the active player); at night draws the sky and the eight planets for the date
void LOOKOBJ_0366_LookSky(void)
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
        ULTIMA_1850_PrintString(_TEXT(0x72f0, "the sun!\n"));
        if (D_587b_activePlayer == 0xff && ULTIMA_39fc_GetFirstActivePartyMember() == 0)
        {
            D_587b_activePlayer = D_5876_tempX;
        }

        ULTIMA_2a52_DamagePlayer(D_587b_activePlayer, 1);
        ULTIMA_2900_UpdateVitalsDisplay();
    }
    else
    {
        D_24e6_viewDirty = 1;
        for (row_k = 0; row_k < 0xb; row_k++)
        {
            // memset?
            for (col_e = 0; col_e < 0xb; col_e++)
            {
                GetMapViewport(col_e, row_k) = 0xff;
            }
        }

        GetMapViewport(5, 10) = 0x59;
        ULTIMA_56ac_DrawMap();
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);

        for (row_k = 0; row_k < 0x50; row_k++)
        {
            ULTIMA_0c64_GRAP_30_Pset(ULTIMA_2092_RandomRange(9, 0xb6), ULTIMA_2092_RandomRange(9, 0xac));
        }

        for (planet = 0; planet < 8; planet++)
        {
            yearOfCentury_b = (int)D_5874_year % 100;
            month = D_587d_month;
            day = D_587e_day;
            orbitPos = D_3750_planetStartPos[planet];
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
                } while (D_3760_orbitSteps[planet * 0x16 + orbitPos] == 0);
            }

            starX = orbitPos + 1;
            starY = D_3758_planetRow[planet];
            LOOKOBJ_01ac_DrawStar(starX, starY);
            starX--;
            for (col_e = 0; col_e < 3; col_e++)
            {
                if (D_58c8_shadowlordTowns[col_e] == planet + 1)
                {
                    LOOKOBJ_024c_DrawRedMark(starX, starY);
                }
            }
        }

        ULTIMA_1850_PrintString(_TEXT(0x72fa, "the night sky! "));

        do
        {
        } while ((key_j = ULTIMA_1b38_PollKeyWithCursor()) == 0);
    }
}

// what a tile is: the sky, a well, a fountain, a clock (the time), the altars' words, a dungeon's name, else its LOOK2.DAT text
static void LOOKOBJ_0502_DescribeTile(int tile, int tileX, int tileY)
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

        tile = *ULTIMA_4402_GetTileAddr(tileX, tileY);
    }

    if (tile == TILE_MAP_59)
    {
        LOOKOBJ_0366_LookSky();
    }
    else if (tile == TILE_MAP_WELL)
    {
        LOOKOBJ_0042_Well(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
    }
    else if ((tile & 0xfc) == TILE_MAP_FOUNTAIN)
    {
        LOOKOBJ_0162_Fountain();
    }
    else
    {
        LOOKOBJ_0000_PrintTileName(tile);
        if ((tile & 0xfe) == TILE_MAP_CLOCK)
        {
            hour12 = D_587f_hour % 0xc;
            if (hour12 == 0)
            {
                hour12 = 0xc;
            }

            ULTIMA_1a3e_PrintNumber(hour12, 1, 0x20);
            ULTIMA_16ba_PrintChar(':');
            ULTIMA_1a3e_PrintNumber(D_5881_minute, 2, 0x30);

            if (D_587f_hour > 0xb)
            {
                ULTIMA_1850_PrintString(_TEXT(0x730a, " PM.\n"));
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x7310, " AM.\n"));
            }
        }
        else if (tile == TILE_MAP_DE)
        {
            switch (D_5893_mapId)
            {
            case 0x1e:
                ULTIMA_1850_PrintString(_TEXT(0x7316, "Truth\n"));
                break;

            case 0x1f:
                ULTIMA_1850_PrintString(_TEXT(0x731e, "Love\n"));
                break;

            case 0x20:
                ULTIMA_1850_PrintString(_TEXT(0x7324, "Courage\n"));
                break;
            }
        }
        else if (tile == TILE_MAP_DF)
        {
            switch (tileX)
            {
            case 0xf0:
                ULTIMA_1850_PrintString(_TEXT(0x732e, "Deceit\n"));
                break;

            case 0x5b:
                ULTIMA_1850_PrintString(_TEXT(0x7336, "Despise\n"));
                break;

            case 0x48:
                ULTIMA_1850_PrintString(_TEXT(0x7340, "Destard\n"));
                break;

            case 0x7e:
                ULTIMA_1850_PrintString(_TEXT(0x734a, "Wrong\n"));
                break;

            case 0x9c:
                ULTIMA_1850_PrintString(_TEXT(0x7352, "Covetous\n"));
                break;

            case 0x3a:
                ULTIMA_1850_PrintString(_TEXT(0x735c, "Shame\n"));
                break;

            case 0xef:
                ULTIMA_1850_PrintString(_TEXT(0x7364, "Hythloth\n"));
                break;

            case 0x80:
                ULTIMA_1850_PrintString(_TEXT(0x736e, "Doom\n"));
                break;
            }
        }
        else
        {
            if (ULTIMA_1f12_GetCurrentTextX() != 0)
            {
                ULTIMA_16ba_PrintChar('\n');
            }
        }
    }
}

// the description of the creature or person looked at (LOOK2.DAT, offset table at 0x200 + tile * 2)
static void LOOKOBJ_06a4_DescribeActor(int actorTile)
{
    char text[128];
    u16 textOffset_c;

    textOffset_c = 0;

    // NOTE: FMT does some other things here

    ULTIMA_256e_ReadFileFromDisk(_TEXT(0x7374, "LOOK2.DAT"), &textOffset_c, 2, actorTile * 2 + 0x200);
    ULTIMA_256e_ReadFileFromDisk(_TEXT(0x737e, "LOOK2.DAT"), text, 0x80, textOffset_c);

    ULTIMA_1850_PrintString(text);

    if (ULTIMA_1f12_GetCurrentTextX() != 0)
    {
        ULTIMA_16ba_PrintChar('\n');
    }
}
u8 D_3750_planetStartPos[8] = { 0x12, 0x02, 0x08, 0x0f, 0x0b, 0x06, 0x04, 0x02 };

u8 D_3758_planetRow[8] = { 0x12, 0x11, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x01 };

u8 D_3760_orbitSteps[0xb0] =
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

char* D_3810[9] =
{
    _STATIC_TEXT(0x7388, "g              g"),
    _STATIC_TEXT(0x739a, "jlllllllnllllllk"),
    _STATIC_TEXT(0x73ac, "8lllllllmllllll9"),
    _STATIC_TEXT(0x73be, "jllllllllllllllk"),
    _STATIC_TEXT(0x73d0, "8llllllllllllll9"),
    _STATIC_TEXT(0x73e2, "hllllk    jlllli"),
    _STATIC_TEXT(0x73f4, "jlllli    hllllk"),
    _STATIC_TEXT(0x7406, "     g    g     "),
    _STATIC_TEXT(0x7418, "     hlllli     ")
};

u8 D_3822_shoreLandMask[16] =
{
    0x05, 0x06, 0x04, 0x01, 0x03, 0x06, 0x05, 0x07,
    0x07, 0x0e, 0x00, 0x00, 0x01, 0x02, 0x08, 0x04
};

u8 D_3832_edgeBits[8] = { 0x0a, 0x05, 0x0c, 0x06, 0x03, 0x09, 0x0f, 0x00 };

// a sign's text from the scratch buffer (-1: the default "Live by the Eight Laws" sign): frame characters, a key press at 0xd
static void LOOKOBJ_06f8_PrintSignText(int signPos)
{
    byte code;
    char* picture_c;

    if (signPos == -1)
    {
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_1850_PrintString(_TEXT(0x742a, "abbbbbbbbbbbbbbc\ng              g\ng  LIVE@BY@[E  g\n"));
        ULTIMA_1850_PrintString(_TEXT(0x745e, "g  EIGHT@LAWS  g\ng              g\ndeeeeeeeeeeeeeef\n"));
        ULTIMA_1c9e_SelectCharset(0);
    }
    else
    {
        for (signPos = signPos + 4; D_b21e_scratch[signPos] == '\n'; signPos += 6)
        {
        }

        do
        {
            code = D_b21e_scratch[signPos];
            if (code & 0x80)
            {
                ULTIMA_1c9e_SelectCharset(0);
            }
            else
            {
                ULTIMA_1c9e_SelectCharset(1);
            }

            if (code == 0x26 || code == 0x27)
            {
                ULTIMA_16ba_PrintChar(0x6c);
            }
            else if (code >= 0x29 && code <= 0x31)
            {
                ULTIMA_1c9e_SelectCharset(1);
                for (picture_c = D_3810[code - 0x29]; *picture_c != '\0';)
                {
                    ULTIMA_16ba_PrintChar((byte)*picture_c++);
                }
            }
            else if (code == 0xd)
            {
                ULTIMA_1c9e_SelectCharset(0);
                code = ULTIMA_266c_GetChar();
            }
            else
            {
                ULTIMA_16ba_PrintChar(code & 0x7f);
            }
        } while (D_b21e_scratch[++signPos] != 0);

        ULTIMA_1c9e_SelectCharset(0);

        if (ULTIMA_1f12_GetCurrentTextX() != 0)
        {
            ULTIMA_16ba_PrintChar('\n');
        }
    }
}

// the sign at (level, x, y): the WANTED poster of the party in town 4, else SIGNS.DAT's text for the place
static void LOOKOBJ_07e4_ReadSign(byte signLevel, byte signX, byte signY)
{
    s16 signOffsets[33];
    int signPos;
    int done_j;

    done_j = 0;
    if (D_5893_mapId == 0x4 && signLevel == 0 && signX == 0x11 && signY == 0x15)
    {
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_1850_PrintString(_TEXT(0x7492, "abbbbbbbbbbbbbc\ng   "));
        ULTIMA_1c9e_SelectCharset(0);
        ULTIMA_1850_PrintString(_TEXT(0x74a8, "Wanted:   "));
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_1850_PrintString(_TEXT(0x74b4, "g\ng"));
        ULTIMA_1bf2_SetTextPosition(0xe, ULTIMA_1cee_GetCurrentTextY());
        ULTIMA_1850_PrintString(_TEXT(0x74b8, "g\ng"));
        for (signPos = 0; signPos < 3; signPos++)
        {
            if (signPos < D_585b_partyCount)
            {
                ULTIMA_1c9e_SelectCharset(0);
                ULTIMA_1bf2_SetTextPosition(7 - (ULTIMA_216c_strlen(D_55a8_party[signPos].name) / 2),
                    ULTIMA_1cee_GetCurrentTextY());
                ULTIMA_1850_PrintString(D_55a8_party[signPos].name);
                ULTIMA_1c9e_SelectCharset(1);
            }

            ULTIMA_1bf2_SetTextPosition(0xe, ULTIMA_1cee_GetCurrentTextY());
            ULTIMA_1850_PrintString(_TEXT(0x74bc, "g\ng"));
        }

        ULTIMA_1bf2_SetTextPosition(0xe, ULTIMA_1cee_GetCurrentTextY());
        ULTIMA_1850_PrintString(_TEXT(0x74c0, "g\ng"));
        ULTIMA_1c9e_SelectCharset(0);
        ULTIMA_1850_PrintString(_TEXT(0x74c4, "Dead or Alive"));
        ULTIMA_1c9e_SelectCharset(1);
        ULTIMA_1850_PrintString(_TEXT(0x74d2, "g\ndeeeeeeeeeeeeef\n"));
        ULTIMA_1c9e_SelectCharset(0);
    }
    else
    {
        ULTIMA_256e_ReadFileFromDisk(_TEXT(0x74e6, "signs.dat"), signOffsets, 0x42, 0);
        signPos = signOffsets[D_5893_mapId];
        memset(D_b21e_scratch, 0xff, SCRATCH_SIZE);

        if (signPos == 0)
        {
            LOOKOBJ_06f8_PrintSignText(-1);
        }
        else
        {
            ULTIMA_256e_ReadFileFromDisk(_TEXT(0x74f0, "signs.dat"), D_b21e_scratch, SCRATCH_SIZE, signPos);
            signPos = 0;
            // di = done_j
            // si = signPos
            do
            {
                if (D_b21e_scratch[signPos + 1] == signLevel && D_b21e_scratch[signPos + 2] == signX && D_b21e_scratch[signPos + 3] == signY)
                {
                    LOOKOBJ_06f8_PrintSignText(signPos);
                    done_j++;
                }
                else
                {
                    signPos += 4;
#if !defined(MATCHING_BUILD)
                    for (; signPos < SCRATCH_SIZE; )
                    {
                        if (D_b21e_scratch[signPos++] == 0)
                            break;
                    }

                    done_j = signPos == SCRATCH_SIZE;
#else
                    while (D_b21e_scratch[signPos++] != 0)
                    {
                    }

                    done_j = D_b21e_scratch[signPos] == 0xff ? 1 : 0;
#endif

                    if (done_j)
                    {
                        LOOKOBJ_06f8_PrintSignText(-1);
                    }
                }
            } while (done_j == 0);
        }
    }
}

// the Look command: a direction, then what is there: the creature, a sign, a tile; a tile 0x29 (a crystal ball?) gives a death vision or the view of the whole map
void LOOKOBJ_099c_LookCmd(void)
{
    int member_g;
    int lookY;
    int lookX;
    int actorTile;
    int tile_b;

    if (ULTIMA_35ec_SelectDirection() == 0)
    {
        return;
    }

    lookX = D_5896_mapX + D_5876_tempX;
    lookY = D_5897_mapY + D_5878_tempY;
    tile_b = *ULTIMA_4402_GetTileAddr(lookX, lookY);
    actorTile = ULTIMA_368e_FindActorTileAtPos(lookX, lookY, D_5895_mapLevel);
    if (tile_b == TILE_MAP_29)
    {
        if ((member_g = ULTIMA_4988_GetActingPlayer()) == -1)
        {
            return;
        }

        if (D_55a8_party[member_g].intel <= ULTIMA_2092_RandomRange(1, 0x1e))
        {
            ULTIMA_1850_PrintString(_TEXT(0x74fa, "Death vision!\n"));
            ULTIMA_2a52_DamagePlayer(member_g, 1);
            ULTIMA_2900_UpdateVitalsDisplay();
            return;
        }
        ULTIMA_1850_PrintString(_TEXT(0x750a, "Strange vision!\n"));
        LOOKOBJ_10fc_ViewCmd(D_5896_mapX, D_5897_mapY);
        return;
    }

    ULTIMA_1850_PrintString(_TEXT(0x751c, "\nThou dost see\n"));
    if (actorTile != 0)
    {
        LOOKOBJ_06a4_DescribeActor(actorTile);
        return;
    }

    switch (tile_b)
    {
    case TILE_MAP_89:
    case TILE_MAP_8A:
    case TILE_MAP_A0:
    case TILE_MAP_A4:
    case TILE_MAP_SIGN_F8:
        ULTIMA_1850_PrintString(_TEXT(0x752c, "\n"));
        LOOKOBJ_07e4_ReadSign(D_5895_mapLevel, lookX, lookY);
        break;

    default:
        LOOKOBJ_0502_DescribeTile(tile_b, lookX, lookY);
        break;
    }
}

// the pixel position (D_5876/D_5878) of the vision's cell (x, y): 4 pixels a cell, at 0x20
static void LOOKOBJ_0a9c_SetCellOrigin(int cellX, int cellY)
{
    D_5876_tempX = cellX * 4 + 0x20;
    D_5878_tempY = cellY * 4 + 0x20;
}

// 4 green dots in a checker
static void LOOKOBJ_0abe_DrawGreenDots(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b4_greenColor + 8);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 1, D_5878_tempY);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 1, D_5878_tempY + 2);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 3, D_5878_tempY + 1);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 3, D_5878_tempY + 3);
}

// a solid light green 4x4 cell
static void LOOKOBJ_0b04_DrawGreenBlock(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b4_greenColor + 8);
    ULTIMA_0aa6_GRAP_3f_FillRect(D_5876_tempX, D_5878_tempY, D_5876_tempX + 3, D_5878_tempY + 3);
}

// two white vertical bars
static void LOOKOBJ_0b28_DrawVertBars(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY, D_5876_tempX, D_5878_tempY + 3);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX + 2, D_5878_tempY, D_5876_tempX + 2, D_5878_tempY + 3);
}

// white lines along the top and bottom
static void LOOKOBJ_0b60_DrawHorizBars(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY, D_5876_tempX + 3, D_5878_tempY);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY + 3, D_5876_tempX + 3, D_5878_tempY + 3);
}

// two short white lines in the middle
static void LOOKOBJ_0b98_DrawInnerBars(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX + 1, D_5878_tempY + 1, D_5876_tempX + 2, D_5878_tempY + 1);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX + 1, D_5878_tempY + 2, D_5876_tempX + 2, D_5878_tempY + 2);
}

// a white frame around the cell
static void LOOKOBJ_0bd0_DrawFrame(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY, D_5876_tempX + 3, D_5878_tempY);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY + 3, D_5876_tempX + 3, D_5878_tempY + 3);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY + 1, D_5876_tempX, D_5878_tempY + 2);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX + 3, D_5878_tempY + 1, D_5876_tempX + 3, D_5878_tempY + 2);
}

// yellow blocks in two corners
static void LOOKOBJ_0c36_DrawYellowSteps(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b8_brightYellowColor);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY, D_5876_tempX + 1, D_5878_tempY);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY + 1, D_5876_tempX + 1, D_5878_tempY + 1);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX + 2, D_5878_tempY + 2, D_5876_tempX + 3, D_5878_tempY + 2);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX + 2, D_5878_tempY + 3, D_5876_tempX + 3, D_5878_tempY + 3);
}

// green lines on rows 0 and 2 and two dots
static void LOOKOBJ_0c9c_DrawGreenStripes(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b4_greenColor + 8);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY, D_5876_tempX + 3, D_5878_tempY);
    ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY + 2, D_5876_tempX + 3, D_5878_tempY + 2);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 2, D_5878_tempY + 1);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX, D_5878_tempY + 3);
}

// CHECKED
// four dots: green on the land corners of a shore tile (0x60-0x6f, D_3822 masks), water-coloured on the others
static void LOOKOBJ_0cf4_DrawShore(int tile)
{
    int cornerBit;

    int corner_j;
    int colors[4];

    cornerBit = 8;

    for (corner_j = 0; corner_j < 4; corner_j++)
    {
        if ((tile & 0xf0) == 0x60 && (D_3822_shoreLandMask[tile & 0xf] & cornerBit) == 0)
        {
            colors[corner_j] = D_13b4_greenColor + 8;
        }
        else if (D_52c8_videoDriver == 0 || D_52c8_videoDriver == 3)
        {
            colors[corner_j] = D_13b6_magentaColor;
        }
        else
        {
            colors[corner_j] = D_13b2_blueColor + 8;
        }

        cornerBit >>= 1;
    }

    ULTIMA_0a70_GRAP_2d_SetPenColor(colors[0]);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 1, D_5878_tempY);
    ULTIMA_0a70_GRAP_2d_SetPenColor(colors[1]);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 3, D_5878_tempY + 1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(colors[2]);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 1, D_5878_tempY + 2);
    ULTIMA_0a70_GRAP_2d_SetPenColor(colors[3]);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 3, D_5878_tempY + 3);
}

// two water-coloured dots (magenta on drivers 0 and 3, else light blue)
static void LOOKOBJ_0dda_DrawWaterDots(void)
{
    if (D_52c8_videoDriver == 0 || D_52c8_videoDriver == 3)
    {
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b6_magentaColor);
    }
    else
    {
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
    }

    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX, D_5878_tempY);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 2, D_5878_tempY + 2);
}

// two green dots and two water-coloured dots
static void LOOKOBJ_0e16_DrawMarsh(void)
{
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b4_greenColor + 8);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 1, D_5878_tempY);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 3, D_5878_tempY + 1);

    if ((D_52c8_videoDriver == 0) || (D_52c8_videoDriver == 3))
    {
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b6_magentaColor);
    }
    else
    {
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
    }

    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX, D_5878_tempY + 2);
    ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 2, D_5878_tempY + 3);
}

// tiles 0x20-0x26: green dots, a red centre, white edges as D_3832 says and a black dot in the corner of some
static void LOOKOBJ_0e7a_DrawEdgedBlock(int tile)
{
    int edges;

    LOOKOBJ_0abe_DrawGreenDots();
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor);
    ULTIMA_0aa6_GRAP_3f_FillRect(D_5876_tempX + 1, D_5878_tempY + 1, D_5876_tempX + 2, D_5878_tempY + 2);

    // tile: 0x20 ~ 0x26 (LOOKOBJ_0f7e_DrawMapCell -> D_1d1a_cellPattern -> val == 0x10)
    edges = D_3832_edgeBits[tile - 0x20];
    if ((edges & 8) != 0)
    {
        ULTIMA_0b10_GRAP_Line(D_5876_tempX + 1, D_5878_tempY, D_5876_tempX + 2, D_5878_tempY);
    }
    if ((edges & 4) != 0)
    {
        ULTIMA_0b10_GRAP_Line(D_5876_tempX + 3, D_5878_tempY + 1, D_5876_tempX + 3, D_5878_tempY + 2);
    }
    if ((edges & 2) != 0)
    {
        ULTIMA_0b10_GRAP_Line(D_5876_tempX + 1, D_5878_tempY + 3, D_5876_tempX + 2, D_5878_tempY + 3);
    }
    if ((edges & 1) != 0)
    {
        ULTIMA_0b10_GRAP_Line(D_5876_tempX, D_5878_tempY + 1, D_5876_tempX, D_5878_tempY + 2);
    }

    ULTIMA_0a70_GRAP_2d_SetPenColor(0);

    switch (tile)
    {
    case 0x22:
        ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 1, D_5878_tempY + 2);
        break;

    case 0x23:
        ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 1, D_5878_tempY + 1);
        break;

    case 0x24:
        ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 2, D_5878_tempY + 1);
        break;

    case 0x25:
        ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 2, D_5878_tempY + 2);
        break;
    }
}

// draws one cell of the vision by the pattern D_1d1a gives its tile (0 nothing, 1-16 the pictures above, 90 white)
static void LOOKOBJ_0f7e_DrawMapCell(int tile, int cellX, int cellY)
{
    int pattern;

    pattern = D_1d1a_cellPattern[tile];
    LOOKOBJ_0a9c_SetCellOrigin(cellX, cellY);

    switch (pattern)
    {
    case 0:
        break;
    case 1: // 0fba
        LOOKOBJ_0abe_DrawGreenDots();
        break;
    case 2: // 0fc0
        LOOKOBJ_0b04_DrawGreenBlock();
        break;
    case 3: // 0fc6
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor);
        ULTIMA_0aa6_GRAP_3f_FillRect(D_5876_tempX, D_5878_tempY, D_5876_tempX + 3, D_5878_tempY + 3);
        break;
    case 4: // 0fea
        LOOKOBJ_0b60_DrawHorizBars();
        break;
    case 5: // 0ff0
        LOOKOBJ_0b98_DrawInnerBars();
        break;
    case 6: // 0ff6
        LOOKOBJ_0bd0_DrawFrame();
        break;
    case 7: // 0ffc -> 0fca
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
        ULTIMA_0aa6_GRAP_3f_FillRect(D_5876_tempX, D_5878_tempY, D_5876_tempX + 3, D_5878_tempY + 3);
        break;
    case 8: // 1002
        LOOKOBJ_0c36_DrawYellowSteps();
        break;
    case 9: // 1008
        LOOKOBJ_0c9c_DrawGreenStripes();
        break;
    case 10: // 1011
        LOOKOBJ_0cf4_DrawShore(tile);
        break;
    case 11: // 1018
        LOOKOBJ_0dda_DrawWaterDots();
        break;
    case 12: // 101e
        if ((D_52c8_videoDriver == 0) || (D_52c8_videoDriver == 3))
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b6_magentaColor);
        }
        else
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
        }
        ULTIMA_0c64_GRAP_30_Pset(D_5876_tempX + 2, D_5878_tempY + 2);
        break;
    case 13: // 1050
        LOOKOBJ_0e16_DrawMarsh();
        break;
    case 14: // 1056
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
        ULTIMA_0b10_GRAP_Line(D_5876_tempX + 1, D_5878_tempY, D_5876_tempX + 1, D_5878_tempY + 3);
        ULTIMA_0b10_GRAP_Line(D_5876_tempX + 2, D_5878_tempY, D_5876_tempX + 2, D_5878_tempY + 3);
        break;
    case 15: // 108c
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor + 8);
        ULTIMA_0aa6_GRAP_3f_FillRect(D_5876_tempX, D_5878_tempY, D_5876_tempX + 3, D_5878_tempY + 3);
        break;
    case 16: // 1096
        LOOKOBJ_0e7a_DrawEdgedBlock(tile);
        break;
    case 90: // 0ffc
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
        ULTIMA_0aa6_GRAP_3f_FillRect(D_5876_tempX, D_5878_tempY, D_5876_tempX + 3, D_5878_tempY + 3);
        break;
    }
}

// the vision: draws the 32x32 map around the origin four pixels a cell and blinks the party's cell until a key
void LOOKOBJ_10fc_ViewCmd(int viewX, int viewY)
{
    int col;
    int row;

    viewX -= D_589b_mapOriginX;
    viewY -= D_589c_mapOriginY;
    viewX &= 0xff;
    viewY &= 0xff;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    ULTIMA_0a70_GRAP_2d_SetPenColor(0);
    ULTIMA_0aa6_GRAP_3f_FillRect(8, 8, 0xb7, 0xb7);

    for (row = 0; row < 0x20; row++)
    {
        for (col = 0; col < 0x20; col++)
        {
            LOOKOBJ_0f7e_DrawMapCell(*ULTIMA_4402_GetTileAddr((uint)D_589b_mapOriginX + col, (uint)D_589c_mapOriginY + row),
                           col, row);
        }
    }

    ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 8, 8, 0xb7, 0xb7);
    ULTIMA_0c22_GRAP_0f_SelectPage(0);
    LOOKOBJ_0a9c_SetCellOrigin(viewX, viewY);

    row = 0;
    while (ULTIMA_1b38_PollKeyWithCursor() == 0)
    {
        row++;
        if (row == 4)
        {
            row = 0;
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
            ULTIMA_0b86_GRAP_XorFillRect(D_5876_tempX, D_5878_tempY, D_5876_tempX + 3, D_5878_tempY + 3);
        }
    }

    ULTIMA_5910_UpdateFrame();
}
