#include "common/common.h"
#include "structs.h"

// this module's communal variables (vars.h, DS:a9bc)
S_bb1c D_bb1c_u4Virtues;
u8 D_bbd2_unused[0xb6]; // no code refers to it; as large as D_bb1c_u4Virtues
S_bc88_U4Party D_bc88_u4Member;
u16 D_bcb0_unused; // no code refers to it

#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include <stdlib.h>

#include "font.h"
#include "intro.h"

#include <string.h>

extern u16 D_bb18_pathPos;

// INTRO.OVL's MSC 5.1 code is three objects: intro0.c (INTRO_0010_PutBackdropStrips, INTRO_0050), intro.c
// (INTRO_014e-INTRO_094e) and this one, from INTRO_0986_Main. Compiled after the first part's functions, INTRO_0986 loses
// optimizations (local_c is not kept in DI); compiled as its own object it is exact.

// defined in intro0.c
void INTRO_0010_PutBackdropStrips(void);
int INTRO_0050_LordBritishAnim(int x, int y);

// defined in intro.c
void INTRO_014e_Introduction(void);
void INTRO_05b0_DisplayTitle(int withSound);
void INTRO_06bc_BuildMainMenu(int selected);
void INTRO_072e_Acknowledgements(void);
int INTRO_094e_Pause(int ticks);

// defined in introb.c, the part of INTRO.OVL built with MSC 5.0
void INTRO_2090_AnimateWD(void);

void INTRO_132a_TransferFromU4(void);     // GOG DOS build: src/asm/AINTRO2.ASM
void INTRO_1e22_PrintFieldLabel(int field);
void INTRO_2024_SetUpTransferScreen(void);
void INTRO_20ae_ShowWD(byte* ptr);

// The menu keys are a char array in the original: it follows the strings of the next
// function, on an even address.
static char S_0x3270[8] = "JCTUAR";

// The start of the program: detects and starts the video, sound and timer, loads the character sets, plays
// the title sequence (Origin logo, "presents", Lord British, production, title, story), and loops over the
// main menu: J Journey Onward, C Create, T Transfer, U Introduction, A Acknowledgements, R Return to the
// View; returns when a saved game was loaded.
// CHECKED
void INTRO_0986_Main(void)
{
    byte* britishImage;
    byte* titleImage;
    byte key_i;
    int index_j;
    int selected_i;
    int state;
    int savedCalibration_g;
    char* fileName;
    int tries_j;

    // 098e
    D_a9ce_soundOn = 1;
    index_j = 0;

    // 0995
    for (index_j = 0; index_j < 0x100; index_j++)
    {
        D_b11e_tileFrames[index_j] = (char)index_j;
    }

    ULTIMA_0de0_DetectVideo();
    ULTIMA_1158_InitTimer();
    savedCalibration_g = D_5356_timerCalibration;

    // force cga if low memory
    if (D_52c8_videoDriver == VDP_VIDEO_TANDY && D_5358_memorySize < 0x170)
    {
        D_52f1_forceTandy = 0;
        D_52ba_vdp.forceCga = 1;
        D_52c8_videoDriver = VDP_VIDEO_CGA;
    }

    // 09d2
    while (ULTIMA_0e94_LoadVideoDriver() == 0)
    {
    }

    ULTIMA_0892_InitializeVideoDriver(1);

    if (D_52c8_videoDriver != VDP_VIDEO_CGA && D_52c8_videoDriver != VDP_VIDEO_HERC)
    {
        D_13b2_blueColor = 1;
        D_13ae_redColor = 4;
        D_13b0_brightWhiteColor = 0xf;
        D_13b6_magentaColor = 5;
        D_13b4_greenColor = 2;
        D_13b8_brightYellowColor = 0xe;
        D_13ba_lightGrayColor = 7;
    }
    else
    {
        // 0a1a
        index_j = 0;

        // cga / herc: *.16 -> *.4
        while (*(fileName = D_25ea_fileNames[index_j++]) != 0) // 0a35
        {
            // 0a22
            while (*fileName++ != '.')
            {
            }

            if (*fileName == '1')
            {
                *fileName++ = '4';
                *fileName = '\0';
            }
        }
    }

    // 0a49
    if (ULTIMA_0f2a_GRAP_06_AllocPageBuffer() == 0)
    {
        ULTIMA_0878_RestoreVideoMode();
        exit(1);
    }
    if (D_52c8_videoDriver == VDP_VIDEO_HERC)
    {
        // 0a64
        while (ULTIMA_1d02_LoadCharset(/*0x3182*/ "ibm.hcs", 0) == 0)
        {
        }
        while (ULTIMA_1d02_LoadCharset(/*0x318a*/ "runes.hcs", 1) == 0)
        {
        }
    }
    else
    {
        // 0a84
        while (ULTIMA_1d02_LoadCharset(/*0x3194*/ "ibm.ch", 0) == 0)
        {
        }
        while (ULTIMA_1d02_LoadCharset(/*0x319b*/ "runes.ch", 1) == 0)
        {
        }
    }

    // 0aa1
    ULTIMA_1c9e_SelectCharset(0);
    ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);
    ULTIMA_1b94_SelectTextWindow(0);
    state = ULTIMA_2032_ToUpper(ULTIMA_1d5e_PollKey());
    if (state == 0x4a) // 'J'
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
        ULTIMA_1bf2_SetTextPosition(0, 0xb);
        ULTIMA_1850_PrintString(/*0x31a4*/ "Journey Onward\n\n");
        ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
        // 0af2
        while (ULTIMA_0ff4_LoadTileset(D_25f0_tilesFileName[0]) == 0)
        {
        }
        D_5893_mapId = 0x41;
        // -> 0cc9
    }
    else
    {
        // 0b06
        state = 1;

        do
        {
            titleImage = ULTIMA_0fae_LoadResourceFile(D_25ea_fileNames[2]); // "TITLE.BIT"
        } while (titleImage == 0);

        do
        {
            britishImage = ULTIMA_0fae_LoadResourceFile(D_25ea_fileNames[1]); // "BRITISH.BIT"
        } while (britishImage == 0);

        ULTIMA_256e_ReadFileFromDisk(/*0x31b5*/ "BRITISH.PTH", &D_55a6_saveStart, ((int)&D_6606_saveEnd - (int)&D_55a6_saveStart) /*0x1060*/, 0);
        // "origin systems inc."
        if (state != 0)
            state = ULTIMA_0d72_AnimateOriginLogo(titleImage) == 0;

        // 0b56
        // "presents"
        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_0aa6_GRAP_3f_FillRect(0, 0x8c, 0x13f, 199);
        ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(titleImage, 7, 0x6c, 0x8c);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0, 0x8c, 0x13f, 199);
        if (state != 0)
        {
            ULTIMA_20fa_WaitTicks(0x12);
            state = INTRO_094e_Pause(0x14) == 0;
        }
        // 0bb6
        if (state != 0)
        {
            // 0bbf
            ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
            // "a"
            ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(titleImage, 0x8, 0x98, 0);
            ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
            state = INTRO_094e_Pause(0x14) == 0;
            if (state != 0)
            {
                ULTIMA_0c22_GRAP_0f_SelectPage(0);
                D_bb18_pathPos = 0;
                state = 0;
                // "lord british" animation
                if ((INTRO_0050_LordBritishAnim(0x2c, 0x44) != 0) && (INTRO_0050_LordBritishAnim(0x40, 0x5e) != 0) && (INTRO_0050_LordBritishAnim(0x8f, 0x4e) != 0))
                {
                    state = INTRO_0050_LordBritishAnim(0xa7, 0x69);
                }
                // 0c42
                ULTIMA_0c22_GRAP_0f_SelectPage(1);
            }
            // 0c49
            // "lord british"
            ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(britishImage, 0, 0x18, 0x42);
            ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
            // "production"
            ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(titleImage, 9, 0x68, 0xa0);
            ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
            if (state != 0)
            {
                state = INTRO_094e_Pause(0x14) == 0;
            }
        }

        // 0c97
        ULTIMA_0fdc_FreeBitImage(britishImage);
        ULTIMA_0fdc_FreeBitImage(titleImage);

        D_5893_mapId = 0x40;
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_0c22_GRAP_0f_SelectPage(0);
        INTRO_05b0_DisplayTitle(state);
        if (state != 0)
        {
            FONT_04a4_PlayIntroStory();
        }
        // 0cc4
        state = 1;
    }

    ULTIMA_207e_srand(ULTIMA_2056_GetTime());

    // 0cd0
    do
    {
        ULTIMA_1c22_SetTextWindowSize(0, 1, 0x10, 0x26, 0x17);
        ULTIMA_1b94_SelectTextWindow(0);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);
        ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
        ULTIMA_1c22_SetTextWindowSize(2, 0x18, 0xb, 0x27, 0x17);
        if (state == 0x4a) // 'J'
        {
            key_i = 0x4a; // 'J'
        }
        else
        {
            // 0d3e: selected_i lives in DI from here on (sub di,di). MSC 5.1 does that only
            // with this function compiled in an object of its own (see the top of the file).
            selected_i = 0;
            INTRO_043e_PrintCaptionBar(/*0x31c1*/ "Copyright 1988 Lord British");
            INTRO_06bc_BuildMainMenu(0);
            do
            {
                // 0d4d
                ULTIMA_1bf2_SetTextPosition(0xf, 0xf);
                ULTIMA_4c2a_PrintLeftCap();
                ULTIMA_1850_PrintString(/*0x31dd*/ "Select: ");
                ULTIMA_4cce_PrintRightCap();
                ULTIMA_1bf2_SetTextPosition(0x17, 0xf);
                D_5356_timerCalibration = 0x113;
                tries_j = 0;
                do
                {
                    // 0d75
                    key_i = ULTIMA_2032_ToUpper(ULTIMA_1b38_PollKeyWithCursor());
                    if (key_i == 0)
                    {
                        INTRO_2090_AnimateWD();
                    }
                    // 0d86
                    tries_j++;
                } while (tries_j < 200 && key_i == 0);
                D_5356_timerCalibration = savedCalibration_g;
                switch (key_i)
                {
                case U5_KEY_LEFT:
                case U5_KEY_UP:
                    selected_i--;
                    if (selected_i < 0)
                    {
                        selected_i = 5;
                    }
                    INTRO_06bc_BuildMainMenu(selected_i);
                    break;
                case U5_KEY_RIGHT:
                case U5_KEY_DOWN:
                    selected_i++;
                    if (5 < selected_i)
                    {
                        selected_i = 0;
                    }
                    INTRO_06bc_BuildMainMenu(selected_i);
                    break;
                case U5_KEY_ENTER:  // CR
                case U5_KEY_SPACE: // ' '
                    // 0de2
                    key_i = S_0x3270[selected_i];
                    break;
                case 0:
                    key_i = 0x52;
                    break;
                case 0x4a: // 'J'
                    INTRO_06bc_BuildMainMenu(0);
                    break;
                case 0x43: // 'C'
                    INTRO_06bc_BuildMainMenu(1);
                    break;
                case 0x54: // 'T'
                    INTRO_06bc_BuildMainMenu(2);
                    break;
                case 0x55: // 'U'
                    INTRO_06bc_BuildMainMenu(3);
                    break;
                case 0x41: // 'A'
                    INTRO_06bc_BuildMainMenu(4);
                    break;
                case 0x52: // 'R'
                    INTRO_06bc_BuildMainMenu(5);
                    break;
                default:
                    key_i = 0;
                    break;
                }
            } while (key_i <= 0x20);
        }

        // 0e47
        switch ((int)key_i)
        {
        case 0x4a: // 'J'

            // 0e7c
            ULTIMA_637e_DrawFrame();
            ULTIMA_2e96_SetWindDirection(0);
            ULTIMA_1b94_SelectTextWindow(2);
            ULTIMA_1bf2_SetTextPosition(0, 0xc);
            if (D_5893_mapId == 0x40)
            {
                while (ULTIMA_0ff4_LoadTileset(D_25f0_tilesFileName[0]) == 0)
                {
                }
                D_5893_mapId = 0x41;
            }
            // 0ead
            ULTIMA_251e_SwitchDisks(3);
            ULTIMA_256e_ReadFileFromDisk(/*0x31e6*/ "SAVED.GAM", &D_55a6_saveStart, ((int)&D_6606_saveEnd - (int)&D_55a6_saveStart) /*0x1060*/, 0);
            if (D_55a8_party[0].name[0] == '\0')
            {
                ULTIMA_1850_PrintString(/*0x31f0*/ "\n\nNo active game. ");
                ULTIMA_1850_PrintString(/*0x3203*/ "Please create a character ");
                ULTIMA_1850_PrintString(/*0x321e*/ "or transfer one from Ultima IV. ");
                ULTIMA_1dda_WaitForKeystroke(0);
                D_a9cb = 0xff;
                ULTIMA_251e_SwitchDisks(0);
                ULTIMA_1b94_SelectTextWindow(0);
                ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
                ULTIMA_0c22_GRAP_0f_SelectPage(1);
                ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
                ULTIMA_0c22_GRAP_0f_SelectPage(0);
                INTRO_05b0_DisplayTitle(0);
                state = 0;
            }
            else
            {
                // 0f26
                ULTIMA_256e_ReadFileFromDisk(/*0x323f*/ "SAVED.OOL", D_b21e_scratch, 0x200, 0);
                ULTIMA_251e_SwitchDisks(1);
                ULTIMA_25d8_WriteFileToDisk(/*0x3249*/ "BRIT.OOL", D_b21e_scratch, 0x100);
                ULTIMA_25d8_WriteFileToDisk(/*0x3252*/ "UNDER.OOL", D_b31e, 0x100);
                if ((D_5893_mapId == 0) && (D_5895_mapLevel != 0))
                {
                    ULTIMA_251e_SwitchDisks(5);
                    while (ULTIMA_1674_TestOpenFile(/*0x325c*/ "UNDER.DAT") == 0)
                    {
                    }
                    ULTIMA_25d8_WriteFileToDisk(/*0x3266*/ "UNDER.OOL", D_b31e, 0x100);
                }
                ULTIMA_2e96_SetWindDirection(-1);
                D_52ba_vdp.tileYOffset = 8;
                return;
            }
            break;

        case 0x54: // 'T'

            ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
            INTRO_132a_TransferFromU4();
            goto L_0fab; // TODO: get rid of goto
            // break;

        case 0x43: // 'C'
            // 0fa8
            FONT_0b0a_CreateCharacter();
        L_0fab:
            ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);
            ULTIMA_1b94_SelectTextWindow(0);
            ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
            ULTIMA_0c22_GRAP_0f_SelectPage(1);
            ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
            INTRO_05b0_DisplayTitle(0);
            INTRO_0010_PutBackdropStrips();
            ULTIMA_0c22_GRAP_0f_SelectPage(0);
            break;

        case 0x55: // 'U'

            INTRO_014e_Introduction();
            INTRO_05b0_DisplayTitle(0);
            break;

        case 0x41: // 'A'

            // 0ff4
            if (D_5893_mapId != 0x40)
            {
                ULTIMA_102e_UnloadTileset();
                D_5893_mapId = 0x40;
            }
            INTRO_072e_Acknowledgements();
            break;

        case 0x52:       // 'R'
            FONT_04a4_PlayIntroStory(); // thunk
            break;
        }
    } while (1);
}

u16 D_3304_notAvatar = 0;

// Reads the Ultima IV party file (PARTY.SAV): refuses bad data, copies name, sex, class, stats and
// experience into the first party member, and sets D_3304_notAvatar when the virtues are all zero (not an Avatar); 1
// on bad data.
// CHECKED
// Not static: in the DOS build INTRO_132a (src/asm/AINTRO2.ASM) calls this one,
// INTRO_1278 and INTRO_12ea_ScaleU4Stat from another object.
int INTRO_1016_ConvertU4Savegame(void)
{
    int offset;
    int i_j;
    S_bb1c* virtues;
    int slot;
    S_bc88_U4Party* u4Member;
    S_55a8* member_i;
    bool bad_j;

    bad_j = 0;
    u4Member = &D_bc88_u4Member;
    slot = 0;
    offset = 8;

    D_a9be_promptMode = 1;
    D_a9bd_curDisk[0] = 4;

    D_a9c8_diskDrives[D_a9bd_curDisk[0]] = ULTIMA_16a6_GetDefaultDrive();
    // the label keeps MSC from folding offset = 8 into the call, as the original doesn't
L_1048:
    ULTIMA_256e_ReadFileFromDisk(/*0x3278*/ "party.sav", &D_bc88_u4Member, 0x28, offset);

    if (D_bc88_u4Member.str > 0x46 || D_bc88_u4Member.dex > 0x46 || D_bc88_u4Member.intel > 0x46 || D_bc88_u4Member.exp > 9999 || D_bc88_u4Member.hp > 9999 ||
        D_bc88_u4Member.maxHp > 9999 || D_bc88_u4Member.cls > 7)
    {
        bad_j = 1;
    }

    for (i_j = 0; i_j < 8; i_j++)
    {
        if (u4Member->name[i_j] != 0 && u4Member->name[i_j] < 0x20)
        {
            bad_j = 1;
            break;
        }

        // 92b6
        if (u4Member->name == 0) // bug?
        {
            break;
        }
    }

    if (bad_j)
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
        ULTIMA_1bf2_SetTextPosition(0, 5);
        ULTIMA_1850_PrintString(/*0x3282*/ "Error:  Your Ultima IV game\n\ncontains bad data.\n\n");
        ULTIMA_1850_PrintString(/*0x32b4*/ "Unable to continue transfer.\n\n\n");
        ULTIMA_1850_PrintString(/*0x32d4*/ "Press any key to return to the menu.");
        ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
        ULTIMA_1dda_WaitForKeystroke(0);
        return 1;
    }

    member_i = &D_55a8_party[slot];

    for (i_j = 0; i_j < 8; i_j++)
    {
        if (u4Member->name[i_j] != 0)
        {
            member_i->name[i_j] = u4Member->name[i_j];
        }
        else
        {
            break;
        }
    }

    member_i->name[i_j] = 0;

    if (u4Member->gender == 11)
    {
        member_i->gender = 0xb;
    }
    else
    {
        member_i->gender = 0xc;
    }

    switch (u4Member->cls)
    {
    case 0:
        member_i->cls = CLASS_MAGE;
        break;
    case 1:
        member_i->cls = CLASS_BARD;
        break;
    case 2:
        member_i->cls = CLASS_FIGHTER;
        break;
    case 3:
        member_i->cls = CLASS_DRUID;
        break;
    case 4:
        member_i->cls = CLASS_TINKER;
        break;
    case 5:
        member_i->cls = CLASS_PALADIN;
        break;
    case 6:
        member_i->cls = CLASS_RANGER;
        break;
    case 7:
        member_i->cls = CLASS_SHEPHERD;
    }

    member_i->status = STATUS_GOOD;
    member_i->str = u4Member->str;
    member_i->dex = u4Member->dex;
    member_i->intel = u4Member->intel;
    member_i->mag = u4Member->mag;
    member_i->hp = u4Member->hp;
    member_i->maxHp = u4Member->maxHp;
    member_i->exp = u4Member->exp;

    member_i->level = u4Member->maxHp / 100;

    offset = 0x140;
    virtues = &D_bb1c_u4Virtues;

    ULTIMA_256e_ReadFileFromDisk(/*0x32f9*/ "party.sav", &D_bb1c_u4Virtues, 0xb6, 0x140);

    if (D_bb1c_u4Virtues.honesty == 0 && D_bb1c_u4Virtues.compassion == 0 && D_bb1c_u4Virtues.valor == 0 && D_bb1c_u4Virtues.justice == 0 && D_bb1c_u4Virtues.sacrifice == 0 && D_bb1c_u4Virtues.honor == 0 &&
        D_bb1c_u4Virtues.spirituality == 0 && D_bb1c_u4Virtues.humility == 0)
    {
        D_3304_notAvatar = 1;
    }

    return 0;
}

// Prints the name of the first member's class and a newline.
void INTRO_1278_PrintU4Class(void)
{
    switch (D_55a8_party[0].cls)
    {
    case CLASS_MAGE:
        ULTIMA_1850_PrintString(/*0x3306*/ "Mage\n");
        break;
    case CLASS_BARD:
        ULTIMA_1850_PrintString(/*0x330c*/ "Bard\n");
        break;
    case CLASS_FIGHTER:
        ULTIMA_1850_PrintString(/*0x3312*/ "Fighter\n");
        break;
    case CLASS_DRUID:
        ULTIMA_1850_PrintString(/*0x331b*/ "Druid\n");
        break;
    case CLASS_TINKER:
        ULTIMA_1850_PrintString(/*0x3322*/ "Tinker\n");
        break;
    case CLASS_PALADIN:
        ULTIMA_1850_PrintString(/*0x332a*/ "Paladin\n");
        break;
    case CLASS_RANGER:
        ULTIMA_1850_PrintString(/*0x3333*/ "Ranger\n");
        break;
    case CLASS_SHEPHERD:
        ULTIMA_1850_PrintString(/*0x333b*/ "Shepherd\n");
        break;
    }
}

// Brings an Ultima IV stat (up to 50) to Ultima V's scale (up to 30): below 10 unchanged, below 30 by
// halves, above by quarters.
int INTRO_12ea_ScaleU4Stat(int stat)
{
    if (stat < 10)
    {
        return stat;
    }

    if (stat >= 30)
    {
        return (stat - 30) / 4 + 20;
    }
    else
    {
        return (stat - 9) / 2 + 10;
    }
}

// The GOG (1992) DOS build takes INTRO_132a from src/asm/AINTRO2.ASM (assembly written from
// the original's bytes, linked after this object), with its strings (DS:3345-3655): the 1992
// patch changed the function in place, so its bytes are no compiler's output. This C is
// compiled for the 1988 build (RELEASE_1988), where it is EXACT against the October 1988
// INTRO.OVL (match.py --target 1988).
// What matched it (it was 2796 bytes against 2808):
// - the frame is 12h: the drive letter and the Y/N answers are one variable (key, bp-10h),
//   and ten bytes nearest BP are never used (unusedBuf; one array or several locals, the
//   code cannot tell);
// - the two Y/N loops are `while ((key = ...) != 'Y' && key != 'N');` (jmp to the read, the
//   'N' test re-reading key from the stack), not a do-while with a break;
// - the level loop is `i = exp / 100; while (i > 0) { level++; i >>= 1; }`: as a `while` MSC
//   5.1 keeps i in CX (stored before and after the loop); as a `for` it stays in memory.
#if defined(RELEASE_1988)
// Transfer from Ultima IV: asks for the disk and drive, converts the character, shows the old and new values
// (name, sex, class, experience, level, stats) with questions about the name and sex, and saves SAVED.GAM.
void INTRO_132a_TransferFromU4(void)
{
    int oldStat;
    int savedMap;
    char unusedBuf[10];
    byte key;
    int i;

    // 1988 (1.16)
    if (D_5893_mapId == 66)
    {
        D_5893_mapId = 65;
    }

    savedMap = D_5893_mapId;

    D_a9cb = D_a9cc = 0xff;

    ULTIMA_256e_ReadFileFromDisk(/*0x3345*/ "INIT.GAM", &D_55a6_saveStart, ((byte*)&D_6606_saveEnd - (byte*)&D_55a6_saveStart), 0);
    ULTIMA_256e_ReadFileFromDisk(/*0x334e*/ "INIT.OOL", D_b31e, 0x100, 0);
    ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

    ULTIMA_1bf2_SetTextPosition(4, 0xc);
    ULTIMA_1850_PrintString(/*0x3357*/ "Transfer Character from Ultima IV");
    ULTIMA_1bf2_SetTextPosition(0, 0xf);
    ULTIMA_1850_PrintString(/*0x3379*/ "Please insert the Ultima IV Player Disk");
    ULTIMA_1bf2_SetTextPosition(8, 0x10);
    ULTIMA_1850_PrintString(/*0x33a1*/ "and press drive letter");
    ULTIMA_1bf2_SetTextPosition(3, 0x12);
    ULTIMA_1850_PrintString(/*0x33b8*/ "or press <Esc> to abort transfer");

    D_5394_diskSwapFn = ULTIMA_2320_NoDiskSwapMessage;

    do
    {
        key = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0));
        if (key == U5_KEY_ESC)
        {
            D_5893_mapId = savedMap;
            return;
        }
    } while (ULTIMA_1eac_SetDefaultDrive(key) == 0);

    D_5394_diskSwapFn = ULTIMA_2322_DiskSwapMessage;

    if (INTRO_1016_ConvertU4Savegame() != 0)
    {
        D_5893_mapId = savedMap;
        return;
    }

    if (D_3304_notAvatar != 0)
    {
        D_55a8_party[0].cls = CLASS_AVATAR;
    }

    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(0, 0xb);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(/*0x33d9*/ "Found:\n");
    ULTIMA_1bf2_SetTextPosition(0, 0xc);
    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1bf2_SetTextPosition(0xc, 0xd);
    ULTIMA_1850_PrintString(/*0x33e1*/ "a level ");
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].level, 1, 0x20);

    if (D_55a8_party[0].gender == 11)
    {
        ULTIMA_1850_PrintString(/*0x33ea*/ " Male ");
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x33f1*/ " Female ");
    }

    INTRO_1278_PrintU4Class();
    ULTIMA_1bf2_SetTextPosition(0x11, 0xf);
    ULTIMA_1850_PrintString(/*0x33fa*/ "STR:  ");
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].str, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(0x11, 0x10);
    ULTIMA_1850_PrintString(/*0x3401*/ "DEX:  ");
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].dex, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(0x11, 0x11);
    ULTIMA_1850_PrintString(/*0x3408*/ "INT:  ");
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].intel, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(10, 0x14);

    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_1850_PrintString(/*0x340f*/ " is ");
    if (D_3304_notAvatar != 0)
    {
        ULTIMA_1850_PrintString(/*0x3414*/ "an Avatar.");
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x341f*/ "not an Avatar");
    }

    ULTIMA_1dda_WaitForKeystroke(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

    INTRO_2024_SetUpTransferScreen();

    for (i = 0; i < 8; i++)
    {
        INTRO_1e22_PrintFieldLabel(i);
    }

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(0, 3);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1bf2_SetTextPosition(10, 5);

    if (D_55a8_party[0].gender == 11)
    {
        ULTIMA_1850_PrintString(/*0x342d*/ "Male");
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x3432*/ "Female");
    }

    ULTIMA_1bf2_SetTextPosition(10, 6);

    if (D_3304_notAvatar != 0)
    {
        ULTIMA_1850_PrintString(/*0x3439*/ "Avatar");
    }
    else
    {
        INTRO_1278_PrintU4Class();
    }

    ULTIMA_1bf2_SetTextPosition(10, 8);
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].exp, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(10, 9);
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].level, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(10, 0xb);
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].str, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(10, 0xc);
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].dex, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(10, 0xd);
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].intel, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(0, 0xf);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);

    if (D_3304_notAvatar != 0)
    {
        ULTIMA_1850_PrintString(/*0x3440*/ "Avatar");
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x3447*/ "Non-Avatar");
    }

    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1bf2_SetTextPosition(7, 2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x3452*/ "Name:");

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(7, 2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x3458*/ "Name:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(10, 0);
    ULTIMA_1850_PrintString(/*0x345e*/ "Keep this name?");

    while ((key = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0))) != 'Y' && key != 'N')
        ;

    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

    if (key == 'N')
    {
        do
        {
            ULTIMA_1bf2_SetTextPosition(1, 0);
            ULTIMA_1850_PrintString(/*0x346e*/ "Enter new name: ");
            ULTIMA_1e38_IntroGetString(D_55a8_party[0].name, 8);
        } while (D_55a8_party[0].name[0] == 0);
    }

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(0, 3);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1bf2_SetTextPosition(7, 2);
    ULTIMA_1850_PrintString(/*0x347f*/ "Name:");
    ULTIMA_1bf2_SetTextPosition(5, 5);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x3485*/ "Sex:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(7, 2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x348a*/ "Name:");
    ULTIMA_1bf2_SetTextPosition(5, 5);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x3490*/ "Sex:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(/*0x3495*/ "Keep same sex?");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);

    while ((key = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0))) != 'Y' && key != 'N')
        ;

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(10, 5);

    if ((key == 'Y' && D_55a8_party[0].gender == 11) || (key == 'N' && D_55a8_party[0].gender == 12))
    {
        ULTIMA_1850_PrintString(/*0x34a4*/ "Male");
        D_55a8_party[0].gender = 0xb;
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x34a9*/ "Female");
        D_55a8_party[0].gender = 0xc;
    }

    ULTIMA_1bf2_SetTextPosition(5, 5);
    ULTIMA_1850_PrintString(/*0x34b0*/ "Sex:");
    ULTIMA_1bf2_SetTextPosition(3, 6);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x34b5*/ "Class:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 6);
    if (D_3304_notAvatar != 0)
    {
        ULTIMA_1850_PrintString(/*0x34bc*/ "Avatar");
    }
    else
    {
        INTRO_1278_PrintU4Class();
    }

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 5);
    ULTIMA_1850_PrintString(/*0x34c3*/ "Sex:");
    ULTIMA_1bf2_SetTextPosition(3, 6);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x34c8*/ "Class:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(2, 0);
    if (D_3304_notAvatar != 0)
    {
        ULTIMA_1850_PrintString(/*0x34cf*/ "Thou art now an Avatar:");
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x34e7*/ "Class remains intact");
    }
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(3, 6);
    ULTIMA_1850_PrintString(/*0x34fc*/ "Class:");
    ULTIMA_1bf2_SetTextPosition(5, 8);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x3503*/ "Exp:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(3, 6);
    ULTIMA_1850_PrintString(/*0x3508*/ "Class:");
    ULTIMA_1bf2_SetTextPosition(5, 8);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x350f*/ "Exp:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 8);
    D_55a8_party[0].exp /= 10;
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].exp, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(/*0x3514*/ "Experience has been converted");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 8);
    ULTIMA_1850_PrintString(/*0x3532*/ "Exp:");
    ULTIMA_1bf2_SetTextPosition(3, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x3537*/ "Level:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(5, 8);
    ULTIMA_1850_PrintString(/*0x353e*/ "Exp:");
    ULTIMA_1bf2_SetTextPosition(3, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x3543*/ "Level:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 9);
    D_55a8_party[0].level = 1;
    i = D_55a8_party[0].exp / 100;
    while (i > 0)
    {
        D_55a8_party[0].level++;
        i >>= 1;
    }

    D_55a8_party[0].hp = D_55a8_party[0].maxHp = D_55a8_party[0].level * 0x1e;
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].level, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(/*0x354a*/ "Level has been converted");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(3, 9);
    ULTIMA_1850_PrintString(/*0x3563*/ "Level:");
    ULTIMA_1bf2_SetTextPosition(5, 0xb);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x356a*/ "STR:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(3, 9);
    ULTIMA_1850_PrintString(/*0x356f*/ "Level:");
    ULTIMA_1bf2_SetTextPosition(5, 0xb);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x3576*/ "STR:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 0xb);
    oldStat = D_55a8_party[0].str;
    D_55a8_party[0].str = INTRO_12ea_ScaleU4Stat(oldStat);
    if (D_55a8_party[0].str < 0x14)
    {
        D_55a8_party[0].str = 0x14;
    }
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].str, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(1, 0);
    ULTIMA_1850_PrintString(/*0x357b*/ "Strength: was ");
    ULTIMA_1a3e_PrintNumber(oldStat, 1, 0x20);
    ULTIMA_1850_PrintString(/*0x358a*/ "(50), now ");
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].str, 1, 0x20);
    ULTIMA_1850_PrintString(/*0x3595*/ "(30)");
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 0xb);
    ULTIMA_1850_PrintString(/*0x359a*/ "STR:");
    ULTIMA_1bf2_SetTextPosition(5, 0xc);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x359f*/ "DEX:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(5, 0xb);
    ULTIMA_1850_PrintString(/*0x35a4*/ "STR:");
    ULTIMA_1bf2_SetTextPosition(5, 0xc);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x35a9*/ "DEX:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 0xc);
    oldStat = D_55a8_party[0].dex;
    D_55a8_party[0].dex = INTRO_12ea_ScaleU4Stat(oldStat);
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].dex, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(1, 0);
    ULTIMA_1850_PrintString(/*0x35ae*/ "Dexterity: was ");
    ULTIMA_1a3e_PrintNumber(oldStat, 1, 0x20);
    ULTIMA_1850_PrintString(/*0x35be*/ "(50), now ");
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].dex, 1, 0x20);
    ULTIMA_1850_PrintString(/*0x35c9*/ "(30)");
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 0xc);
    ULTIMA_1850_PrintString(/*0x35ce*/ "DEX:");
    ULTIMA_1bf2_SetTextPosition(5, 0xd);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x35d3*/ "INT:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(5, 0xc);
    ULTIMA_1850_PrintString(/*0x35d8*/ "DEX:");
    ULTIMA_1bf2_SetTextPosition(5, 0xd);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(/*0x35dd*/ "INT:");
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 0xd);

    oldStat = D_55a8_party[0].intel;
    D_55a8_party[0].mag = D_55a8_party[0].intel = INTRO_12ea_ScaleU4Stat(oldStat);
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].mag, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(1, 0);
    ULTIMA_1850_PrintString(/*0x35e2*/ "Intellect: was ");
    ULTIMA_1a3e_PrintNumber(oldStat, 1, 0x20);
    ULTIMA_1850_PrintString(/*0x35f2*/ "(50), now ");
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].intel, 1, 0x20);
    ULTIMA_1850_PrintString(/*0x35fd*/ "(30)");
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 0xd);
    ULTIMA_1850_PrintString(/*0x3602*/ "INT:");

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(5, 0xd);
    ULTIMA_1850_PrintString(/*0x3607*/ "INT:");
    ULTIMA_1bf2_SetTextPosition(0, 0xf);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    if (D_3304_notAvatar != 0)
    {
        ULTIMA_1850_PrintString(/*0x360c*/ "Avatar");
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x3613*/ "Non-Avatar");
    }
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_1c22_SetTextWindowSize(2, 2, 0x15, 0x25, 0x16);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_251e_SwitchDisks(3);
    ULTIMA_1850_PrintString(/*0x361e*/ "\n\n Conversion complete, saving...\n");

    memset(D_b21e_scratch, 0, 0x100);

    ULTIMA_25d8_WriteFileToDisk(/*0x3641*/ "SAVED.OOL", D_b21e_scratch, 0x200);
    ULTIMA_25d8_WriteFileToDisk(/*0x364b*/ "SAVED.GAM", &D_55a6_saveStart, ((byte*)&D_6606_saveEnd - (byte*)&D_55a6_saveStart));
    ULTIMA_251e_SwitchDisks(0);

    D_5893_mapId = savedMap;
    D_a9be_promptMode = 0;
}
#endif
