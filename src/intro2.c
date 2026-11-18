#include "audio/audio.h"
#include "common/common.h"
#include "funcs.h"
#include "grap_drv.h"
#include "savegame.h"
#include "vars.h"
#include "macros.h"

#include <stdlib.h>

#include "font.h"
#include "intro.h"

#include <string.h>

#include "time/time.h"

// INTRO.OVL's MSC 5.1 code is three objects: intro0.c (INTRO_0010, INTRO_0050), intro.c
// (INTRO_014e-INTRO_094e) and this one, from INTRO_0986_Main. Compiled after the first part's functions, INTRO_0986 loses
// optimizations (local_c is not kept in DI); compiled as its own object it is exact.

// defined in intro0.c
void INTRO_0010(void);
int INTRO_0050_LordBritishAnim(int param_1, int param_2);

// defined in intro.c
void INTRO_014e_Introduction(void);
void INTRO_05b0_DisplayTitle(int param_1);
void INTRO_06bc_BuildMainMenu(int param_1);
void INTRO_072e_Acknowledgements(void);
int INTRO_094e_Pause(int param_1);

// defined in introb.c, the part of INTRO.OVL built with MSC 5.0
void INTRO_2090_AnimateWD(void);

void INTRO_132a_TransferFromU4(void);     // DOS build: src/asm/AINTRO2.ASM
void INTRO_1e22(int param_1);
void INTRO_2024(void);
void INTRO_20ae_ShowWD(byte* ptr);

// The menu keys are a char array in the original: it follows the strings of the next
// function, on an even address.
#if defined(TARGET_DOS16)
static char S_0x3270[8] = "JCTUAR";
#endif

// CHECKED
// intro_main (initialize video) (8b46)
void INTRO_0986_Main(void)
{
    byte* local_14;
    byte* local_12_ab;
    byte local_10_ad;
    int local_e;
    int local_c;
    int local_a;
    int local_8;
    char* local_6;
    int local_4_ab;

    // 098e
    D_a9ce_soundOn = 1;
    local_e = 0;

    // 0995
    for (local_e = 0; local_e < 0x100; local_e++)
    {
        D_b11e_tileFrames[local_e] = (char)local_e;
    }

    ULTIMA_0de0_DetectVideo();
    ULTIMA_1158_InitTimer();
    local_8 = D_5356_timerCalibration;

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
        local_e = 0;

        // cga / herc: *.16 -> *.4
        while (*(local_6 = D_25ea_fileNames[local_e++]) != 0) // 0a35
        {
            // 0a22
            while (*local_6++ != '.')
            {
            }

            if (*local_6 == '1')
            {
                *local_6++ = '4';
                *local_6 = '\0';
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
    local_a = ULTIMA_2032_ToUpper(ULTIMA_1d5e_PollKey());
    if (local_a == 0x4a) // 'J'
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
        ULTIMA_1bf2_SetTextPosition(0, 0xb);
        ULTIMA_1850_PrintString(_TEXT(0x31a4, "Journey Onward\n\n"));
        ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
        // 0af2
        while (ULTIMA_0ff4_LoadTileset(D_25f0[0]) == 0)
        {
        }
        D_5893_mapId = 0x41;
        // -> 0cc9
    }
    else
    {
        // 0b06
        local_a = 1;

        do
        {
            local_12_ab = ULTIMA_0fae_LoadResourceFile(D_25ea_fileNames[2]); // "TITLE.BIT"
        } while (local_12_ab == 0);

        do
        {
            local_14 = ULTIMA_0fae_LoadResourceFile(D_25ea_fileNames[1]); // "BRITISH.BIT"
        } while (local_14 == 0);

#if !defined(MATCHING_BUILD)
        ULTIMA_256e_ReadFileFromDisk(/*0x31b5*/ "BRITISH.PTH", D_b21e_scratch, SCRATCH_SIZE, 0);
#else
        ULTIMA_256e_ReadFileFromDisk(/*0x31b5*/ "BRITISH.PTH", &D_55a6_saveStart, ((int)&D_6606_saveEnd - (int)&D_55a6_saveStart) /*0x1060*/, 0);
#endif
        // "origin systems inc."
        if (local_a != 0)
            local_a = ULTIMA_0d72_AnimateOriginLogo(local_12_ab) == 0;

        // 0b56
        // "presents"
        ULTIMA_0c22_GRAP_0f_SelectPage(1);
        ULTIMA_0aa6_GRAP_3f_FillRect(0, 0x8c, 0x13f, 199);
        ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(local_12_ab, 7, 0x6c, 0x8c);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 0, 0x8c, 0x13f, 199);
        if (local_a != 0)
        {
            ULTIMA_20fa_WaitTicks(0x12);
            local_a = INTRO_094e_Pause(0x14) == 0;
        }
        // 0bb6
        if (local_a != 0)
        {
            // 0bbf
            ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
            // "a"
            ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(local_12_ab, 0x8, 0x98, 0);
            ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
            local_a = INTRO_094e_Pause(0x14) == 0;
            if (local_a != 0)
            {
                ULTIMA_0c22_GRAP_0f_SelectPage(0);
                D_bb18 = 0;
                local_a = 0;
                // "lord british" animation
                if ((INTRO_0050_LordBritishAnim(0x2c, 0x44) != 0) && (INTRO_0050_LordBritishAnim(0x40, 0x5e) != 0) && (INTRO_0050_LordBritishAnim(0x8f, 0x4e) != 0))
                {
                    local_a = INTRO_0050_LordBritishAnim(0xa7, 0x69);
                }
                // 0c42
                ULTIMA_0c22_GRAP_0f_SelectPage(1);
            }
            // 0c49
            // "lord british"
            ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(local_14, 0, 0x18, 0x42);
            ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
            // "production"
            ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(local_12_ab, 9, 0x68, 0xa0);
            ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
            if (local_a != 0)
            {
                local_a = INTRO_094e_Pause(0x14) == 0;
            }
        }

        // 0c97
        ULTIMA_0fdc_FreeBitImage(local_14);
        ULTIMA_0fdc_FreeBitImage(local_12_ab);

        D_5893_mapId = 0x40;
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_0c22_GRAP_0f_SelectPage(0);
        INTRO_05b0_DisplayTitle(local_a);
        if (local_a != 0)
        {
#if defined(ENABLE_BGM)
            AUDIO_PlayBgm(BGM_ID_1);
#endif

            FONT_04a4_PlayIntroStory();
        }
        // 0cc4
        local_a = 1;
    }

    ULTIMA_207e_srand(ULTIMA_2056_GetTime());

    // 0cd0
    do
    {
#if defined(ENABLE_BGM)
        AUDIO_PlayBgm(BGM_ID_1);
#endif

        ULTIMA_1c22_SetTextWindowSize(0, 1, 0x10, 0x26, 0x17);
        ULTIMA_1b94_SelectTextWindow(0);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);
        ULTIMA_1c22_SetTextWindowSize(1, 0x18, 1, 0x27, 9);
        ULTIMA_1c22_SetTextWindowSize(2, 0x18, 0xb, 0x27, 0x17);
        if (local_a == 0x4a) // 'J'
        {
            local_10_ad = 0x4a; // 'J'
        }
        else
        {
            // 0d3e: local_c lives in DI from here on (sub di,di). MSC 5.1 does that only
            // with this function compiled in an object of its own (see the top of the file).
            local_c = 0;
            INTRO_043e(_TEXT(0x31c1, "Copyright 1988 Lord British"));
            INTRO_06bc_BuildMainMenu(0);
            do
            {
                // 0d4d
                ULTIMA_1bf2_SetTextPosition(0xf, 0xf);
                ULTIMA_4c2a_PrintLeftCap();
                ULTIMA_1850_PrintString(_TEXT(0x31dd, "Select: "));
                ULTIMA_4cce_PrintRightCap();
                ULTIMA_1bf2_SetTextPosition(0x17, 0xf);
                D_5356_timerCalibration = 0x113;
                local_4_ab = 0;
                do
                {
                    // 0d75
                    local_10_ad = ULTIMA_2032_ToUpper(ULTIMA_1b38_PollKeyWithCursor());
                    if (local_10_ad == 0)
                    {
                        INTRO_2090_AnimateWD();
                    }
                    // 0d86
                    local_4_ab++;
                } while (local_4_ab < 200 && local_10_ad == 0);
                D_5356_timerCalibration = local_8;
                switch (local_10_ad)
                {
                case U5_KEY_LEFT:
                case U5_KEY_UP:
                    local_c--;
                    if (local_c < 0)
                    {
                        local_c = 5;
                    }
                    INTRO_06bc_BuildMainMenu(local_c);
                    break;
                case U5_KEY_RIGHT:
                case U5_KEY_DOWN:
                    local_c++;
                    if (5 < local_c)
                    {
                        local_c = 0;
                    }
                    INTRO_06bc_BuildMainMenu(local_c);
                    break;
                case U5_KEY_ENTER:  // CR
                case U5_KEY_SPACE: // ' '
                    // 0de2
                    local_10_ad = _XTEXT(0x3270, "JCTUAR")[local_c];
                    break;
                case 0:
                    local_10_ad = 0x52;
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
                    local_10_ad = 0;
                    break;
                }
            } while (local_10_ad <= 0x20);
        }

        // 0e47
        switch ((int)local_10_ad)
        {
        case 0x4a: // 'J'
#if defined(ENABLE_BGM)
            AUDIO_StopBgm();
#endif

            // 0e7c
            ULTIMA_637e_DrawFrame();
            ULTIMA_2e96_SetWindDirection(0);
            ULTIMA_1b94_SelectTextWindow(2);
            ULTIMA_1bf2_SetTextPosition(0, 0xc);
            if (D_5893_mapId == 0x40)
            {
                while (ULTIMA_0ff4_LoadTileset(D_25f0[0]) == 0)
                {
                }
                D_5893_mapId = 0x41;
            }
            // 0ead
            ULTIMA_251e_SwitchDisks(3);
#if !defined(TARGET_DOS16)
            FILE_ReadSavegameFile(/*0x31e6*/ "SAVED.GAM");
#else
            ULTIMA_256e_ReadFileFromDisk(/*0x31e6*/ "SAVED.GAM", &D_55a6_saveStart, ((int)&D_6606_saveEnd - (int)&D_55a6_saveStart) /*0x1060*/, 0);
#endif
            if (D_55a8_party[0].name[0] == '\0')
            {
                ULTIMA_1850_PrintString(_TEXT(0x31f0, "\n\nNo active game. "));
                ULTIMA_1850_PrintString(_TEXT(0x3203, "Please create a character "));
                ULTIMA_1850_PrintString(_TEXT(0x321e, "or transfer one from Ultima IV. "));
                ULTIMA_1dda_WaitForKeystroke(0);
                D_a9cb = 0xff;
                ULTIMA_251e_SwitchDisks(0);
                ULTIMA_1b94_SelectTextWindow(0);
                ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
                ULTIMA_0c22_GRAP_0f_SelectPage(1);
                ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
                ULTIMA_0c22_GRAP_0f_SelectPage(0);
                INTRO_05b0_DisplayTitle(0);
                local_a = 0;
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
#if defined(ENABLE_BGM)
            AUDIO_StopBgm();
#endif

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
            INTRO_0010();
            ULTIMA_0c22_GRAP_0f_SelectPage(0);
            break;

        case 0x55: // 'U'
#if defined(ENABLE_BGM)
            AUDIO_StopBgm();
#endif

            INTRO_014e_Introduction();
            INTRO_05b0_DisplayTitle(0);
            break;

        case 0x41: // 'A'
#if defined(ENABLE_BGM)
            AUDIO_StopBgm();
#endif

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

u16 D_3304 = 0;

// CHECKED
// Not static: in the DOS build INTRO_132a (src/asm/AINTRO2.ASM) calls this one,
// INTRO_1278 and INTRO_12ea from another object.
int INTRO_1016_ConvertU4Savegame(void)
{
    int local_10_ad;
    int local_e;
    S_bb1c* local_c;
    int local_a;
    S_bc88_U4Party* local_8;
    S_55a8* local_6;
    bool local_4_ab;

    local_4_ab = 0;
    local_8 = &D_bc88;
    local_a = 0;
    local_10_ad = 8;

    D_a9be = 1;
    D_a9bd_curDisk[0] = 4;

    D_a9c8_diskDrives[D_a9bd_curDisk[0]] = ULTIMA_16a6_GetDefaultDrive();
    // the label keeps MSC from folding local_10_ad = 8 into the call, as the original doesn't
L_1048:
    ULTIMA_256e_ReadFileFromDisk(/*0x3278*/ "party.sav", &D_bc88, 0x28, local_10_ad);

    if (D_bc88.str > 0x46 || D_bc88.dex > 0x46 || D_bc88.intel > 0x46 || D_bc88.exp > 9999 || D_bc88.hp > 9999 ||
        D_bc88.maxHp > 9999 || D_bc88.cls > 7)
    {
        local_4_ab = 1;
    }

    for (local_e = 0; local_e < 8; local_e++)
    {
        if (local_8->name[local_e] != 0 && local_8->name[local_e] < 0x20)
        {
            local_4_ab = 1;
            break;
        }

        // 92b6
        if (local_8->name == 0) // bug?
        {
            break;
        }
    }

    if (local_4_ab)
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
        ULTIMA_1bf2_SetTextPosition(0, 5);
        ULTIMA_1850_PrintString(_TEXT(0x3282, "Error:  Your Ultima IV game\n\ncontains bad data.\n\n"));
        ULTIMA_1850_PrintString(_TEXT(0x32b4, "Unable to continue transfer.\n\n\n"));
        ULTIMA_1850_PrintString(_TEXT(0x32d4, "Press any key to return to the menu."));
        ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
        ULTIMA_1dda_WaitForKeystroke(0);
        return 1;
    }

    local_6 = &D_55a8_party[local_a];

    for (local_e = 0; local_e < 8; local_e++)
    {
        if (local_8->name[local_e] != 0)
        {
            local_6->name[local_e] = local_8->name[local_e];
        }
        else
        {
            break;
        }
    }

    local_6->name[local_e] = 0;

    if (local_8->gender == 11)
    {
        local_6->gender = 0xb;
    }
    else
    {
        local_6->gender = 0xc;
    }

    switch (local_8->cls)
    {
    case 0:
        local_6->cls = CLASS_MAGE;
        break;
    case 1:
        local_6->cls = CLASS_BARD;
        break;
    case 2:
        local_6->cls = CLASS_FIGHTER;
        break;
    case 3:
        local_6->cls = CLASS_DRUID;
        break;
    case 4:
        local_6->cls = CLASS_TINKER;
        break;
    case 5:
        local_6->cls = CLASS_PALADIN;
        break;
    case 6:
        local_6->cls = CLASS_RANGER;
        break;
    case 7:
        local_6->cls = CLASS_SHEPHERD;
    }

    local_6->status = STATUS_GOOD;
    local_6->str = local_8->str;
    local_6->dex = local_8->dex;
    local_6->intel = local_8->intel;
    local_6->mag = local_8->mag;
    local_6->hp = local_8->hp;
    local_6->maxHp = local_8->maxHp;
    local_6->exp = local_8->exp;

    local_6->level = local_8->maxHp / 100;

    local_10_ad = 0x140;
    local_c = &D_bb1c;

    ULTIMA_256e_ReadFileFromDisk(/*0x32f9*/ "party.sav", &D_bb1c, 0xb6, 0x140);

    if (D_bb1c.honesty == 0 && D_bb1c.compassion == 0 && D_bb1c.valor == 0 && D_bb1c.justice == 0 && D_bb1c.sacrifice == 0 && D_bb1c.honor == 0 &&
        D_bb1c.spirituality == 0 && D_bb1c.humility == 0)
    {
        D_3304 = 1;
    }

    return 0;
}

void INTRO_1278_PrintU4Class(void)
{
    switch (D_55a8_party[0].cls)
    {
    case CLASS_MAGE:
        ULTIMA_1850_PrintString(_TEXT(0x3306, "Mage\n"));
        break;
    case CLASS_BARD:
        ULTIMA_1850_PrintString(_TEXT(0x330c, "Bard\n"));
        break;
    case CLASS_FIGHTER:
        ULTIMA_1850_PrintString(_TEXT(0x3312, "Fighter\n"));
        break;
    case CLASS_DRUID:
        ULTIMA_1850_PrintString(_TEXT(0x331b, "Druid\n"));
        break;
    case CLASS_TINKER:
        ULTIMA_1850_PrintString(_TEXT(0x3322, "Tinker\n"));
        break;
    case CLASS_PALADIN:
        ULTIMA_1850_PrintString(_TEXT(0x332a, "Paladin\n"));
        break;
    case CLASS_RANGER:
        ULTIMA_1850_PrintString(_TEXT(0x3333, "Ranger\n"));
        break;
    case CLASS_SHEPHERD:
        ULTIMA_1850_PrintString(_TEXT(0x333b, "Shepherd\n"));
        break;
    }
}

int INTRO_12ea(int param_1)
{
    if (param_1 < 10)
    {
        return param_1;
    }

    if (param_1 >= 30)
    {
        return (param_1 - 30) / 4 + 20;
    }
    else
    {
        return (param_1 - 9) / 2 + 10;
    }
}

// The DOS build takes INTRO_132a from src/asm/AINTRO2.ASM (assembly written from the
// original's bytes, linked after this object), with its strings (DS:3345-3655): GOG patched
// the function in place, so its bytes are no compiler's output. This C is compiled for the
// native build only.
#if !defined(TARGET_DOS16)
void INTRO_132a_TransferFromU4(void)
{
    int local_14;
    int local_e;
    byte cVar4;
    byte bVar5;
    byte uVar6;
    int iVar7;
    int uVar8;
    byte local_12;

#if defined(GOG_BUILD)
    // GOG patched (1332..134c)
    local_e = D_5893_mapId == 66 ? 65 : D_5893_mapId;
    D_a9cc = D_a9cb = D_a9c8_diskDrives[0];
#else
    // MATCHING (1.16)
    if (D_5893_mapId == 66)
    {
        D_5893_mapId = 65;
    }

    local_e = D_5893_mapId;

    D_a9cb = D_a9cc = 0xff;
#endif

#if !defined(TARGET_DOS16)
    FILE_ReadSavegameFile(/*0x3345*/ "INIT.GAM");
#else
    ULTIMA_256e_ReadFileFromDisk(/*0x3345*/ "INIT.GAM", &D_55a6_saveStart, ((byte*)&D_6606_saveEnd - (byte*)&D_55a6_saveStart), 0);
#endif
    ULTIMA_256e_ReadFileFromDisk(/*0x334e*/ "INIT.OOL", D_b31e, 0x100, 0);
    ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

#if defined(GOG_BUILD)
    // GOG patched (1392..1399)
    local_12 = D_a9c8_diskDrives[0];
    goto L_GOG_1402;
#endif

    ULTIMA_1bf2_SetTextPosition(4, 0xc);
    ULTIMA_1850_PrintString(_TEXT(0x3357, "Transfer Character from Ultima IV"));
    ULTIMA_1bf2_SetTextPosition(0, 0xf);
    ULTIMA_1850_PrintString(_TEXT(0x3379, "Please insert the Ultima IV Player Disk"));
    ULTIMA_1bf2_SetTextPosition(8, 0x10);
    ULTIMA_1850_PrintString(_TEXT(0x33a1, "and press drive letter"));
    ULTIMA_1bf2_SetTextPosition(3, 0x12);
    ULTIMA_1850_PrintString(_TEXT(0x33b8, "or press <Esc> to abort transfer"));

    D_5394_diskSwapFn = ULTIMA_2320_NoDiskSwapMessage;

    do
    {
        local_12 = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0));
        if (local_12 == U5_KEY_ESC)
        {
            D_5893_mapId = local_e;
            return;
        }
#if defined(GOG_BUILD)
L_GOG_1402:
        ;
#endif
    } while (ULTIMA_1eac_SetDefaultDrive(local_12) == 0);

    D_5394_diskSwapFn = ULTIMA_2322_DiskSwapMessage;

    if (INTRO_1016_ConvertU4Savegame() != 0)
    {
        D_5893_mapId = local_e;
        return;
    }

    if (D_3304 != 0)
    {
        D_55a8_party[0].cls = CLASS_AVATAR;
    }

    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(0, 0xb);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(_TEXT(0x33d9, "Found:\n"));
    ULTIMA_1bf2_SetTextPosition(0, 0xc);
    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1bf2_SetTextPosition(0xc, 0xd);
    ULTIMA_1850_PrintString(_TEXT(0x33e1, "a level "));
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].level, 1, 0x20);

    if (D_55a8_party[0].gender == 11)
    {
        ULTIMA_1850_PrintString(_TEXT(0x33ea, " Male "));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x33f1, " Female "));
    }

    INTRO_1278_PrintU4Class();
    ULTIMA_1bf2_SetTextPosition(0x11, 0xf);
    ULTIMA_1850_PrintString(_TEXT(0x33fa, "STR:  "));
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].str, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(0x11, 0x10);
    ULTIMA_1850_PrintString(_TEXT(0x3401, "DEX:  "));
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].dex, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(0x11, 0x11);
    ULTIMA_1850_PrintString(_TEXT(0x3408, "INT:  "));
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].intel, 1, 0x20);
    ULTIMA_1bf2_SetTextPosition(10, 0x14);

    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_1850_PrintString(_TEXT(0x340f, " is "));
    if (D_3304 != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x3414, "an Avatar."));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x341f, "not an Avatar"));
    }

    ULTIMA_1dda_WaitForKeystroke(0);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

    INTRO_2024();

    for (iVar7 = 0; iVar7 < 8; iVar7++)
    {
        INTRO_1e22(iVar7);
    }

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(0, 3);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1bf2_SetTextPosition(10, 5);

    if (D_55a8_party[0].gender == 11)
    {
        ULTIMA_1850_PrintString(_TEXT(0x342d, "Male"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x3432, "Female"));
    }

    ULTIMA_1bf2_SetTextPosition(10, 6);

    if (D_3304 != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x3439, "Avatar"));
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

    if (D_3304 != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x3440, "Avatar"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x3447, "Non-Avatar"));
    }

    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1bf2_SetTextPosition(7, 2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x3452, "Name:"));

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(7, 2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x3458, "Name:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(10, 0);
    ULTIMA_1850_PrintString(_TEXT(0x345e, "Keep this name?"));

    do // NOT MATCHING
    {
        cVar4 = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0));
        if (cVar4 == 'Y')
            break;
    } while (cVar4 != 'N');

    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);

    if (cVar4 == 'N')
    {
        do
        {
            ULTIMA_1bf2_SetTextPosition(1, 0);
            ULTIMA_1850_PrintString(_TEXT(0x346e, "Enter new name: "));
            ULTIMA_1e38_IntroGetString(D_55a8_party[0].name, 8);
        } while (D_55a8_party[0].name[0] == 0);
    }

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(0, 3);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(D_55a8_party[0].name);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1bf2_SetTextPosition(7, 2);
    ULTIMA_1850_PrintString(_TEXT(0x347f, "Name:"));
    ULTIMA_1bf2_SetTextPosition(5, 5);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x3485, "Sex:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(7, 2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x348a, "Name:"));
    ULTIMA_1bf2_SetTextPosition(5, 5);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x3490, "Sex:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(_TEXT(0x3495, "Keep same sex?"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);

    do // NOT MATCHING
    {
        cVar4 = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0));
        if (cVar4 == 'Y')
            break;
    } while (cVar4 != 'N');

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(10, 5);

    if ((cVar4 == 'Y' && D_55a8_party[0].gender == 11) || (cVar4 == 'N' && D_55a8_party[0].gender == 12))
    {
        ULTIMA_1850_PrintString(_TEXT(0x34a4, "Male"));
        D_55a8_party[0].gender = 0xb;
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x34a9, "Female"));
        D_55a8_party[0].gender = 0xc;
    }

    ULTIMA_1bf2_SetTextPosition(5, 5);
    ULTIMA_1850_PrintString(_TEXT(0x34b0, "Sex:"));
    ULTIMA_1bf2_SetTextPosition(3, 6);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x34b5, "Class:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 6);
    if (D_3304 != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x34bc, "Avatar"));
    }
    else
    {
        INTRO_1278_PrintU4Class();
    }

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 5);
    ULTIMA_1850_PrintString(_TEXT(0x34c3, "Sex:"));
    ULTIMA_1bf2_SetTextPosition(3, 6);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x34c8, "Class:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(2, 0);
    if (D_3304 != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x34cf, "Thou art now an Avatar:"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x34e7, "Class remains intact"));
    }
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(3, 6);
    ULTIMA_1850_PrintString(_TEXT(0x34fc, "Class:"));
    ULTIMA_1bf2_SetTextPosition(5, 8);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x3503, "Exp:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(3, 6);
    ULTIMA_1850_PrintString(_TEXT(0x3508, "Class:"));
    ULTIMA_1bf2_SetTextPosition(5, 8);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x350f, "Exp:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 8);
    D_55a8_party[0].exp /= 10;
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].exp, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(_TEXT(0x3514, "Experience has been converted"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 8);
    ULTIMA_1850_PrintString(_TEXT(0x3532, "Exp:"));
    ULTIMA_1bf2_SetTextPosition(3, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x3537, "Level:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(5, 8);
    ULTIMA_1850_PrintString(_TEXT(0x353e, "Exp:"));
    ULTIMA_1bf2_SetTextPosition(3, 9);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x3543, "Level:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 9);
    D_55a8_party[0].level = 1;
    for (iVar7 = D_55a8_party[0].exp / 100; iVar7 > 0; iVar7 >>= 1)
    {
        D_55a8_party[0].level++;
    }

    D_55a8_party[0].hp = D_55a8_party[0].maxHp = D_55a8_party[0].level * 0x1e;
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].level, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    ULTIMA_1850_PrintString(_TEXT(0x354a, "Level has been converted"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(3, 9);
    ULTIMA_1850_PrintString(_TEXT(0x3563, "Level:"));
    ULTIMA_1bf2_SetTextPosition(5, 0xb);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x356a, "STR:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(3, 9);
    ULTIMA_1850_PrintString(_TEXT(0x356f, "Level:"));
    ULTIMA_1bf2_SetTextPosition(5, 0xb);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x3576, "STR:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 0xb);
    local_14 = D_55a8_party[0].str;
    D_55a8_party[0].str = INTRO_12ea(local_14);
    if (D_55a8_party[0].str < 0x14)
    {
        D_55a8_party[0].str = 0x14;
    }
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].str, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(1, 0);
    ULTIMA_1850_PrintString(_TEXT(0x357b, "Strength: was "));
    ULTIMA_1a3e_PrintNumber(local_14, 1, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x358a, "(50), now "));
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].str, 1, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x3595, "(30)"));
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 0xb);
    ULTIMA_1850_PrintString(_TEXT(0x359a, "STR:"));
    ULTIMA_1bf2_SetTextPosition(5, 0xc);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x359f, "DEX:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(5, 0xb);
    ULTIMA_1850_PrintString(_TEXT(0x35a4, "STR:"));
    ULTIMA_1bf2_SetTextPosition(5, 0xc);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x35a9, "DEX:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 0xc);
    local_14 = D_55a8_party[0].dex;
    D_55a8_party[0].dex = INTRO_12ea(local_14);
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].dex, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(1, 0);
    ULTIMA_1850_PrintString(_TEXT(0x35ae, "Dexterity: was "));
    ULTIMA_1a3e_PrintNumber(local_14, 1, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x35be, "(50), now "));
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].dex, 1, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x35c9, "(30)"));
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 0xc);
    ULTIMA_1850_PrintString(_TEXT(0x35ce, "DEX:"));
    ULTIMA_1bf2_SetTextPosition(5, 0xd);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x35d3, "INT:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(5, 0xc);
    ULTIMA_1850_PrintString(_TEXT(0x35d8, "DEX:"));
    ULTIMA_1bf2_SetTextPosition(5, 0xd);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1850_PrintString(_TEXT(0x35dd, "INT:"));
    ULTIMA_16ba_PrintChar(CTRL_CHAR_TOGGLE_INVERSION);
    ULTIMA_1bf2_SetTextPosition(10, 0xd);

    local_14 = D_55a8_party[0].intel;
    D_55a8_party[0].mag = D_55a8_party[0].intel = INTRO_12ea(local_14);
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].mag, 1, 0x20);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_1bf2_SetTextPosition(1, 0);
    ULTIMA_1850_PrintString(_TEXT(0x35e2, "Intellect: was "));
    ULTIMA_1a3e_PrintNumber(local_14, 1, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x35f2, "(50), now "));
    ULTIMA_1a3e_PrintNumber(D_55a8_party[0].intel, 1, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x35fd, "(30)"));
    ULTIMA_1dda_WaitForKeystroke(0);

    ULTIMA_1b94_SelectTextWindow(0);
    ULTIMA_1bf2_SetTextPosition(5, 0xd);
    ULTIMA_1850_PrintString(_TEXT(0x3602, "INT:"));

    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_1bf2_SetTextPosition(5, 0xd);
    ULTIMA_1850_PrintString(_TEXT(0x3607, "INT:"));
    ULTIMA_1bf2_SetTextPosition(0, 0xf);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
    if (D_3304 != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x360c, "Avatar"));
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x3613, "Non-Avatar"));
    }
    ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);

    ULTIMA_1b94_SelectTextWindow(2);
    ULTIMA_1c22_SetTextWindowSize(2, 2, 0x15, 0x25, 0x16);
    ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
    ULTIMA_251e_SwitchDisks(3);
    ULTIMA_1850_PrintString(_TEXT(0x361e, "\n\n Conversion complete, saving...\n"));

    memset(D_b21e_scratch, 0, 0x100);

    ULTIMA_25d8_WriteFileToDisk(/*0x3641*/ "SAVED.OOL", D_b21e_scratch, 0x200);
#if !defined(TARGET_DOS16)
    FILE_WriteSavegameFile(/*0x364b*/ "SAVED.GAM");
#else
    ULTIMA_25d8_WriteFileToDisk(/*0x364b*/ "SAVED.GAM", &D_55a6_saveStart, ((byte*)&D_6606_saveEnd - (byte*)&D_55a6_saveStart));
#endif
    ULTIMA_251e_SwitchDisks(0);

    D_5893_mapId = local_e;
    D_a9be = 0;
}
#endif
