#include "common/common.h"

#include "funcs.h"
#include "vars.h"
#include "macros.h"

#include "grap_drv.h"

#include "common/lzw.h"
#include "key/key.h"
#include "time/time.h"

#include "font.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define VERBOSE_LOG

void DRV_66(int ax, int bx, int cx, int dx, int si, int di, int cf);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_102e_UnloadTileset(void)
{
    debug("ULTIMA_102e_UnloadTileset");
    DRV_5a_FreeTileset();
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(byte* img, int idx, int x, int y)
{
    //debug("ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(ptr,%d,%d,%d)", idx, x, y);
    // ax = a, bx = idx, si = c, di = d
    DRV_4e(img, idx, x, y);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// Let a tile appear in the view cell (x, y) in 256 steps (driver function 66), the map animated meanwhile; on map 0x42 a key stops it and is returned; else 0.
int ULTIMA_1068_RevealTile(int param_1, int param_2, int param_3)
{
    int iVar1;
    int di;

    di = 0;

    do
    {
        while (1)
        {
            do
            {
                // ax = param_2 <- x
                // bx = param_3 <- y
                // si = param_1 <- tile
                // di = progress
                // cx = 52bc <- offset x
                // dx = 52be <- offset y
                DRV_66(param_2, param_3, D_52ba_vdp.tileXOffset, D_52ba_vdp.tileYOffset, param_1, di, 1);

                di++;
                DRV_66(param_2, param_3, D_52ba_vdp.tileXOffset, D_52ba_vdp.tileYOffset, param_1, di, 1);

                di++;

                if (di == 0x100)
                {
                    return 0;
                }
            } while ((di & 7) != 0);

            if (D_5893_mapId == 0x42)
                break;

            ULTIMA_5910_UpdateFrame();
        }

        iVar1 = FONT_02fc_AnimateScene(1);
    } while (iVar1 == 0);

    return iVar1;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_10e0_GRAP_51_PutTile(uint tile, int x, int y)
{
    // al = x
    // ah = y
    // bx = tile
    // cx: x offset
    // dx: y offset
    // si = 52c0, di = 52c2
    DRV_51_PutTile(x, y, tile, D_52ba_vdp.tileXOffset, D_52ba_vdp.tileYOffset, D_52ba_vdp._52c0, D_52ba_vdp._52c2);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// Driver function 60: draw the moongate tile in view cell (x, y), its lowest `rows` rows only.
void ULTIMA_1112_GRAP_60_Moongate(int a, int b, int c)
{
    //debug("ULTIMA_1112(%d, %d, %d)", a, b, c);
    // (int ax, byte bl, int cx, int dx, int si, int di, int carry)
    // ax
    // bl
    // cx: x offset
    // dx: y offset
    // cf: code path
    DRV_60(a, D_5893_mapId, D_52ba_vdp.tileXOffset, D_52ba_vdp.tileYOffset, b, c, 1);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
int ULTIMA_1140_GRAP_6f_AnimateOriginLogo(void)
{
    return DRV_6f(D_5356_timerCalibration);
}
#endif

static void ULTIMA_1184_InitTextWindow(void);
static void ULTIMA_11b4_CalibrateTimer(void);
static void ULTIMA_1226_InitTextVars(void);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_1158_InitTimer(void)
{
    debug("ULTIMA_1158_InitTimer");

    ULTIMA_1184_InitTextWindow();
    ULTIMA_1226_InitTextVars();
    ULTIMA_11b4_CalibrateTimer();
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static void ULTIMA_1184_InitTextWindow(void)
{
    int i;
    TextWindow* win;

    D_539a_currentTextWindow = &D_535e_textWindows[0];

    for (i = 0; i < 4; i++)
    {
        win = &D_535e_textWindows[i];

        win->left = 0;
        win->top = 0;
        win->right = 39;
        win->bottom = 24;
        win->currentX = 0;
        win->currentY = 0;
        win->colors = 0xf;
        win->flags = 0;
    }

    D_5386_textWindowIndex = 0;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB (asm)
static void ULTIMA_11b4_CalibrateTimer(void)
{
    debug("ULTIMA_11b4_CalibrateTimer");

    // calibrates D_5356_timerCalibration, D_535a
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static void ULTIMA_1226_InitTextVars(void)
{
    D_53a8_inverse = D_53a6 = D_53a4_underline = 0;
}
#endif

static int ULTIMA_1588_IsFileCompressed(char* fileName);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// return: BX
// - CF==0: ok
// - CF==1: fail
void* ULTIMA_125d_LoadResourceFileImpl(char* file_name)
{
    FILE* fp;
    u32 size;
    byte* buf = NULL;

    debug("ULTIMA_125d_ReadFileImpl(%s)", file_name);

    fp = fopen(file_name, "rb");
    if (!fp)
        return NULL;

    if (!ULTIMA_1588_IsFileCompressed(file_name))
    {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buf = malloc(size);
        fread(buf, size, 1, fp);
    }
    else
    {
        LzwDecompressFile(fp, &buf, &size);
    }

    fclose(fp);

    return buf;
}
#endif

// ASM
// ULTIMA_135a_ReadLzwCompressedFile(...)

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// NOTE: ret: carry
static int ULTIMA_1588_IsFileCompressed(char* fileName)
{
    char* piVar1;
    char cVar2;
    char cVar3;
    char** puVar4;
    char* piVar5;
    char* pcVar6;

    puVar4 = D_25ea_fileNames;
    while (1)
    {
        piVar5 = *puVar4;
        if (*piVar5 == 0)
        {
            return 0;
        }

        puVar4++;
        pcVar6 = fileName;
        while (1)
        {
            piVar1 = piVar5++;

            cVar2 = *piVar1;
            cVar3 = *pcVar6;
            if (cVar3 > 96 && cVar3 < 123)
            {
                cVar3 -= 0x20;
            }

            if (cVar2 > 96 && cVar2 < 123)
            {
                cVar2 -= 0x20;
            }

            if (cVar3 != cVar2)
                break;

            if (*piVar5 == '\0')
            {
                return 1;
            }

            pcVar6++;
        }
    }
}
#endif

// ASM
// ULTIMA_15c6_AllocateMemory
// ULTIMA_160e_ReadFile
// ULTIMA_1649_DosReadFile
// ULTIMA_1654_DosGetFileSize

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
int ULTIMA_1674_TestOpenFile(char* file_name)
{
    debug("ULTIMA_1674_TestOpenFile(%s)", file_name);

#if !defined(TARGET_DOS16)
    FILE* fp = fopen(file_name, "r");
    if (fp)
    {
        fclose(fp);
        return 1;
    }
    return 0;
#else
    // FMT (dummy)
    return 1;
#endif
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB (asm)
int ULTIMA_16a6_GetDefaultDrive(void)
{
    debug("ULTIMA_16a6_GetDefaultDrive");
    // FMT (dummy)
    return 'D';
}
#endif

//
// REGION: TEXT
//

static void ULTIMA_17f4_ApplyCharEffects(byte* es, int di);
static void ULTIMA_1f77_ConvertCharCoordToPixel(TextWindow* window, int* pAX, int* pBX, int* pCX, int* pDX);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_16ba_PrintChar(uint ch)
{
    int iVar4;
    byte bVar6;
    int uVar7;
    TextWindow* text_window;

    bVar6 = ch;
    text_window = D_539a_currentTextWindow;
    if (ch > 0x7f)
    {
        if (ch == CTRL_CHAR_CLEAR_WINDOW) // clear text window
        {
            int ax, bx, cx, dx;

            text_window->currentX = 0;
            text_window->currentY = 0;
            DRV_2d((text_window->colors >> 4) & 0xf);
            ULTIMA_1f77_ConvertCharCoordToPixel(text_window, &ax, &bx, &cx, &dx);
            DRV_3f(ax, bx, cx, dx, 0);
            DRV_2d(D_52da_penColor);
            return;
        }
        if (ch == CTRL_CHAR_TOGGLE_UNDERLINE) // toggle underline
        {
            D_53a4_underline ^= 1;
            text_window->flags ^= 1; // underline
            return;
        }
        if (ch == CTRL_CHAR_TOGGLE_INVERSION) // toggle inversion
        {
            D_53a8_inverse ^= 1;
            text_window->flags ^= 4; // inverse
            return;
        }
        if (ch == CTRL_CHAR_CENTER_TEXT) // center text
        {
            D_53a6 = 1;
            text_window->flags |= 2;
            return;
        }
        if (ch == CTRL_CHAR_UNCENTER_TEXT) // uncenter text
        {
            D_53a6 = 0;
            text_window->flags &= 0xfd;
            return;
        }
        ch &= 0x7f;
    }

    if (ch != 0xa)
    {
        if (ch == 0xd)
        {
            goto LAB_1000_1745;
        }

        // Render Character
        uVar7 = bVar6 * 8;
        if (D_52ba_vdp.videoDriver == VDP_VIDEO_HERC)
        {
            uVar7 = bVar6 * 24;
        }
        ULTIMA_17f4_ApplyCharEffects(D_5398_currentCharset, uVar7);

        // es = 5398
        // di = uVar7
        // dl = [D_53aa_textFgColor]
        // dh = [D_53ab_textBgColor]
        // al = [text_window + current_x] + [text_window + left]
        // ah = 0
        // bl = [text_window + current_y] + [text_window + top]
        // bh = 0
        DRV_5d(D_5398_currentCharset, uVar7, D_53aa_textFgColor, D_53ab_textBgColor,
               text_window->currentX + text_window->left, text_window->currentY + text_window->top);

        iVar4 = 4;
        if (D_52ba_vdp.videoDriver == VDP_VIDEO_HERC)
        {
            iVar4 = 12;
        }

        memcpy(D_5398_currentCharset + uVar7, D_53ea, iVar4 * 2);

        text_window = D_539a_currentTextWindow;
        if (D_538e == 0)
        {
            return;
        }

        text_window->currentX++;
        if ((char)(text_window->currentX + text_window->left) <= text_window->right)
        {
            return;
        }
    }

    text_window->currentY++;
#if defined(ENABLE_SPEED_FIX)
    TIME_SleepMs(10);
#endif

LAB_1000_1745:
    text_window->currentX = 0;
    if (text_window->bottom < text_window->currentY + text_window->top)
    {
        int ax, bx, cx, dx;
        ULTIMA_1f77_ConvertCharCoordToPixel(text_window, &ax, &bx, &cx, &dx);
        text_window->currentY--;
#if defined(ENABLE_SPEED_FIX)
        TIME_SleepMs(10);
#endif
        DRV_27(ax, bx, cx, dx, -8);
    }
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// ptr = (es:)di
static void ULTIMA_17f4_ApplyCharEffects(byte* es, int di)
{
    int size;
    byte* ptr = es + di;

    size = 4;
    if (D_52ba_vdp.videoDriver == VDP_VIDEO_HERC)
    {
        size = 12;
    }

    memcpy(D_53ea, ptr, size * 2);

    if (D_53a4_underline != 0)
    {
        if (D_52ba_vdp.videoDriver == VDP_VIDEO_HERC)
        {
            // herc
            *(u16*)&ptr[0x16] = 0xffff;
        }
        else
        {
            // ega/cga/t1k
            ptr[7] = 0xff;
        }
    }

    if (D_53a8_inverse != 0)
    {
        size = 4;
        if (D_52ba_vdp.videoDriver == VDP_VIDEO_HERC)
        {
            // herc
            size = 12;
        }

        size *= 2;
        for (; size > 0; size--)
        {
            *ptr = ~*ptr;
            ptr++;
        }
    }
}
#endif



//
// REGION: KEYBOARD
//

static void ULTIMA_1b24_ClearKbdBufferImpl();

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB (asm)
void ULTIMA_1b16_ClearKbdBuffer(void)
{
    // push ds
    ULTIMA_1b24_ClearKbdBufferImpl();
    // pop ds
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB (asm)
static void ULTIMA_1b24_ClearKbdBufferImpl(void)
{
    // BIOS key buffer
    // DS = 0x40
    // (word)DS:[0x1a] = 0x1e;
    // (word)DS:[0x1c] = 0x1e;

    // *headPtr = 0x1e;
    // *tailPtr = 0x1e;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_1b94_SelectTextWindow(int id)
{
    register int b;

#ifdef VERBOSE_LOG
    debug("ULTIMA_1b94_SelectTextWindow(%d)", id);
#endif

    if (id <= 3)
    {
        D_5386_textWindowIndex = id;
        D_539a_currentTextWindow = &D_535e_textWindows[id];

        b = D_539a_currentTextWindow->colors;
        D_53aa_textFgColor = b & 0xf;
        D_53ab_textBgColor = (b & 0xf0) >> 4;

        b = D_539a_currentTextWindow->flags;
        D_53a4_underline = b & 1;
        D_53a6 = (b & 2) >> 1;
        D_53a8_inverse = (b & 4) >> 2;
    }
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_1bf2_SetTextPosition(int x, int y)
{
#ifdef VERBOSE_LOG
    debug("ULTIMA_1bf2_SetTextPosition(%d,%d)", x, y);
#endif

    if ((byte)x + D_539a_currentTextWindow->left < 40 && (byte)y + D_539a_currentTextWindow->top < 25)
    {
        D_539a_currentTextWindow->currentX = x;
        D_539a_currentTextWindow->currentY = y;
    }
}
#endif

static byte ULTIMA_1c5b_ClipTextWindow(int* x1, int* y1, int* x2, int* y2);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_1c22_SetTextWindowSize(int idx, int x1, int y1, int x2, int y2)
{
#ifdef VERBOSE_LOG
    debug("ULTIMA_1c22_SetTextWindowSize(%d,%d,%d,%d,%d)", idx, x1, y1, x2, y2);
#endif
    if (idx < 4)
    {
        TextWindow* win = &D_535e_textWindows[idx];
        ULTIMA_1c5b_ClipTextWindow(&x1, &y1, &x2, &y2);
        ASSERT(x1 <= x2);
        ASSERT(y1 <= y2);
        win->left = (u8)x1;
        win->top = (u8)y1;
        win->right = (u8)x2;
        win->bottom = (u8)y2;
    }
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static byte ULTIMA_1c5b_ClipTextWindow(int* x1, int* y1, int* x2, int* y2)
{
    if (*x1 < 0)
        *x1 = 0;

    if (*x1 > 39)
        *x1 = 39;

    if (*y1 < 0)
        *y1 = 0;

    if (*y1 > 24)
        *y1 = 24;

    if (*x2 < 0)
        *x2 = 0;

    if (*x2 > 39)
        *x2 = 39;

    if (*y2 < 0)
        *y2 = 0;

    if (*y2 > 24)
        *y2 = 24;

    if (*x1 > *x2)
    {
        // XCHG
        int temp = *x1;
        *x1 = *x2;
        *x2 = temp;
    }

    if (*y1 > *y2)
    {
        // XCHG
        int temp = *y1;
        *y1 = *y2;
        *y2 = temp;
    }

    return *x1;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_1c9e_SelectCharset(int param_1)
{
#ifdef VERBOSE_LOG
    debug("ULTIMA_1c9e_SelectCharset(%d)", a);
#endif

    if (param_1 < 4 && D_539c[param_1] != 0)
    {
        D_5398_currentCharset = D_539c[param_1];
        D_5388_charsetIndex = param_1;
    }
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_1cca_SetTextForegroundColor(int a)
{
#ifdef VERBOSE_LOG
    debug("ULTIMA_1cca_SetTextForegroundColor(%d) [%d]", a, D_5386_textWindowIndex);
#endif

    D_53aa_textFgColor = a & 0xf;
    D_539a_currentTextWindow->colors = (D_539a_currentTextWindow->colors & 0xf0) | (a & 0xf);
}
#endif

// ASM
#if !defined(TARGET_DOS16) // an entry point inside SetTextForegroundColor: src/asm
int ULTIMA_1cee_GetCurrentTextY(void) { return D_539a_currentTextWindow->currentY; }
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
int ULTIMA_1d02_LoadCharset(char* a, int b)
{
    debug("ULTIMA_1d02_LoadCharset(%s,%d)", a, b);

    D_539c[b] = ULTIMA_0fae_LoadResourceFile(a);

    // FMT
    return 1;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
int ULTIMA_1d5e_PollKey(void)
{
    return KEY_PollKey();
}
#endif

static int ULTIMA_1ef7_ConvertDriveLetterToNumber(char* al, char* dl);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
int ULTIMA_1eac_SetDefaultDrive(int param_1)
{
    char al = param_1;
    char dl;

    debug("ULTIMA_1eac_SetDefaultDrive(%d)", param_1);

    D_535c_dosError = 0;

    if (ULTIMA_1ef7_ConvertDriveLetterToNumber(&al, &dl) == 0)
    {
        D_541e = dl;

        // int 21,0e; // Select Disk
        //if (D_535c_dosError == 0) // success
        //{
        //    // int 21,19; // Get Current Default Drive
        //    if (D_541e == ret)
        //    {
        //        D_535c_dosError = 0;
        //        return 1;
        //    }
        //}
        //
        //return 0;

        // STUB
        return 1;
    }

    D_535c_dosError = 1;
    D_5394_diskSwapFn();

    return 1;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static int ULTIMA_1ef7_ConvertDriveLetterToNumber(char* al, char* dl)
{
    if ('a' <= *al && *al <= 'z')
    {
        *al -= 0x20; // to upper
    }

    *dl = *al - 'A';
    if ('A' <= *al && *al <= 'Z')
    {
        return 0;
    }

    return 1;
}
#endif

// ASM
#if !defined(TARGET_DOS16) // an entry point inside SetTextBackgroundColor: src/asm
int ULTIMA_1f12_GetCurrentTextX(void) { return D_539a_currentTextWindow->currentX; }
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_1f26_SetTextBackgroundColor(int a)
{
#ifdef VERBOSE_LOG
    debug("ULTIMA_1f26_SetTextBackgroundColor(%d) [%d]", a, D_5386_textWindowIndex);
#endif

    D_53ab_textBgColor = a & 0xf;
    D_539a_currentTextWindow->colors = (D_539a_currentTextWindow->colors & 0xf) | ((a & 0xf) << 4);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB (asm)
static void ULTIMA_1f4e_Drv27Offset(int param_1)
{
    // call ULTIMA_1f77_ConvertCharCoordToPixel
    // far call DRV_27(-(param_1 * 8)
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// param: SI = window
static void ULTIMA_1f77_ConvertCharCoordToPixel(TextWindow* window, int* pAX, int* pBX, int* pCX, int* pDX)
{
    int ax = (int)window->left << 3;
    int bx = (int)window->top << 3;
    int cx = ((int)window->right << 3) + 7;
    int dx = ((int)window->bottom << 3) + 7;

    *pAX = ax;
    *pBX = bx;
    *pCX = cx;
    *pDX = dx;
}
#endif
