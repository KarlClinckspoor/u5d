#include "common/common.h"
#include "vars.h"
#include "funcs.h"

#include "grap_drv.h"

#include <stdio.h>

#include "macros.h"

// 017e..0877: crt (slibce)

// 017e: _astart (crt0)
// 0230: _cinit
// 02f4: exit
// 037d: (dynamic near call)
// 038c: (dynamic far call)
// 03a0: aNuldiv (32bit division)
// 0402: memchr
// 0426: itoa
// 0442: aNlmul (32bit multiply)
// aNulmul == aNlmul?
// 0476: aNNalshl (32bit shl)
// 0496: aNldiv (32bit division)
// 0532: _NMSG_TEXT
// 055d: _NMSG_WRITE
// 0586: _FF_MSGBANNER
// 05a6: _setargv
// 0734: _setenvp
// 07a2: _nullcheck
// 07c4: _fptrap
// 07d6: _cxtoa
// 082a: aNlshl
// 0836: _myalloc

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB
void ULTIMA_0878_RestoreVideoMode(void)
{
    debug("ULTIMA_0878_RestoreVideoMode");
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB
void ULTIMA_0892_InitializeVideoDriver(int a)
{
    debug("ULTIMA_0892_InitializeVideoDriver(%d)", a);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// AX, BX => CARRY
static int ULTIMA_08ca_InsideClipWindow(int ax, int bx)
{
	int x = ax;
	int y = bx;

	if (D_52ba_vdp.clipX1 <= x && x <= D_52ba_vdp.clipX2 &&
		D_52ba_vdp.clipY1 <= y && y <= D_52ba_vdp.clipY2)
	{
		return 0;
	}

	return -1;
}
#endif

#if !defined(TARGET_DOS16) // used only by the C versions of assembly routines
// xchg
static void SWAP(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
int ULTIMA_08e6_ClipRectCoord(int *x1, int *y1, int *x2, int *y2)
{
	if (*x1 > *x2)
		SWAP(x1, x2);

	// 08eb
	if (*y1 > *y2)
		SWAP(y1, y2);

	// 08f1
	if (*x1 < 0)
		*x1 = 0;

	// 08f9
	if (*x1 > 319)
		*x1 = 319;

	// 0901
	if (*y1 < 0)
		*y1 = 0;

	// 0909
	if (*y1 > 199)
		*y1 = 199;

	// 0912
	if (*x2 < 0)
		*x2 = 0;

	// 091a
	if (*x2 > 319)
		*x2 = 319;

	// 0923
	if (*y2 < 0)
		*y2 = 0;

	// 092b
	if (*y2 > 199)
		*y2 = 199;

	// 0934
	return *x1; // void?
}
#endif

static void ULTIMA_0991_ClipLineBisect(int* ax, int* bx, int* cx, int* dx);
static void ULTIMA_0a22_ClipOutcode(int ax, int bx, int cx, int dx, int* si, int* di);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static bool ULTIMA_0935_ClipLineCoord(int* x1, int* y1, int* x2, int* y2)
{
	int si = 0;
	int di = 0;
	ULTIMA_0a22_ClipOutcode(*x1, *y1, *x2, *y2, &si, &di);

	if (si == 0 || di == 0)
		return 0; // CLC

	if (di != si)
		return -1; // STC

	// 0948
	D_52fa = *x1;
	D_52fc = *y1;
	D_52fe = *x2;
	D_5300 = *y2;

	if (si != 0)
	{
		SWAP(x1, x2);
		SWAP(y1, y2);
		ULTIMA_0991_ClipLineBisect(x1, y1, x2, y2);
		D_52fa = *x1;
		D_52fc = *y1;
		*x2 = D_52fe;
		*y2 = D_5300;
	}

	// 0971
	if (di != 0)
	{
		ULTIMA_0991_ClipLineBisect(x1, y1, x2, y2);
		D_52fe = *x1;
		D_5300 = *y1;
	}
	// 0980
	*x1 = D_52fa;
	*y1 = D_52fc;
	*x2 = D_52fe;
	*y2 = D_5300;
	return 0;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static void ULTIMA_0991_ClipLineBisect(int* ax, int* bx, int* cx, int* dx)
{
	int si;
	int di;
	while (1)
	{
		while (1)
		{
			si = *ax;
			di = *bx;

			*ax += *cx;
			*bx += *dx;
			if (*ax >= 0)
			{
				*ax >>= 1;
			}
			else
			{
				*ax /= 2;
			}

			if (*bx >= 0)
			{
				*bx >>= 1;
			}
			else
			{
				*bx /= 2;
			}

			D_52f6 = si - *ax;
			D_52f8 = di - *bx;

			ULTIMA_0a22_ClipOutcode(*ax, *bx, *cx, *dx, &si, &di);
			if (si == 0)
				break;
			*cx = *ax;
			*dx = *bx;
			*ax += D_52f6;
			*bx += D_52f8;
		}

		// 09e3
		if (*ax == D_52ba_vdp.clipX1 || *ax == D_52ba_vdp.clipX2)
		{
			// 09ef
			if (*bx == D_52ba_vdp.clipY1 || *bx == D_52ba_vdp.clipY2)
				return;
		}

		// 09fb
		si = D_52f6;
		di = D_52f8;
		if (si < 0)
			si = -si;

		// 0a0b
		if (di < 0)
			di = -di;

		// 0a13
		if (si < di)
			si = di;

		// 0a19
		if (si < 1)
			return;
	}
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static void ULTIMA_0a22_ClipOutcode(int ax, int bx, int cx, int dx, int *si, int *di)
{
	int x1 = ax;
	int y1 = bx;
	int x2 = cx;
	int y2 = dx;

	if (x1 < D_52ba_vdp.clipX1)
		*si |= 1;

	if (x2 < D_52ba_vdp.clipX1)
		*di |= 1;

	if (x1 > D_52ba_vdp.clipX2)
		*si |= 2;

	if (x2 > D_52ba_vdp.clipX2)
		*di |= 2;

	if (y1 > D_52ba_vdp.clipY2)
		*si |= 4;

	if (y2 > D_52ba_vdp.clipY2)
		*di |= 4;

	if (y1 < D_52ba_vdp.clipY1)
		*si |= 8;

	if (y2 < D_52ba_vdp.clipY1)
		*di |= 8;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0a70_GRAP_2d_SetPenColor(int param_1)
{
	if (param_1 != -1)
	{
		param_1 &= D_52ba_vdp.videoDriver == VDP_VIDEO_CGA ? 3 : 0xf;
	}

	D_52da_penColor = param_1;

	if (param_1 != -1)
	{
		DRV_2d(param_1);
	}
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0aa6_GRAP_3f_FillRect(int x1, int y1, int x2, int y2)
{
    ASSERT(x1 < x2);
    ASSERT(y1 < y2);
    ULTIMA_08e6_ClipRectCoord(&x1, &y1, &x2, &y2);
    ASSERT(x1 < x2);
    ASSERT(y1 < y2);
	DRV_3f(x1, y1, x2, y2, 0);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// src_page, dst_page, x1, y1, x2, y2
void ULTIMA_0ace_GRAP_18_TransferArea(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6)
{
    int ax = param_3; // x1
    int bx = param_4; // y1
    int cx = param_5; // x2
    int dx = param_6; // y2
    int si = param_1;
    int di = param_2;

	int carry;

    //debug("ULTIMA_0ace(%d,%d,%d,%d,%d,%d)", param_1, param_2, param_3, param_4, param_5, param_6);

	if (si != di && si <= 1 && di <= 1)
    {
        ASSERT(ax <= cx);
        ASSERT(bx <= dx);
        ULTIMA_08e6_ClipRectCoord(&ax, &bx, &cx, &dx);
        ASSERT(ax <= cx);
        ASSERT(bx <= dx);

		carry = si != 0;
		// DRV_18 (transfer_area)
        DRV_18(ax, bx, cx, dx, si, di, carry);
    }
}
#endif

static void ULTIMA_0b2d_GRAP_Line(int ax, int bx, int cx, int dx, byte carry);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0b10_GRAP_Line(int x1, int y1, int x2, int y2)
{
	ULTIMA_0b2d_GRAP_Line(x1, y1, x2, y2, 0);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static void ULTIMA_0b2d_GRAP_Line(int ax, int bx, int cx, int dx, byte carry)
{
	int x1 = ax;
	int y1 = bx;
	int x2 = cx;
	int y2 = dx;

	D_52ba_vdp.penX = x2;
	D_52ba_vdp.penY = y2;

	// pushf
	if (D_52ba_vdp.clipEnabled != 0)
	{
		if (ULTIMA_0935_ClipLineCoord(&ax, &bx, &cx, &dx))
			return; // JC return
	}

	if (y1 == y2)
	{
		if (x1 == x2)
		{
			// 0b78
			// 0x30: pset
			DRV_30(x1, y1);
			return;
		}
		else
		{
			ULTIMA_08e6_ClipRectCoord(&ax, &bx, &cx, &dx);
			DRV_39(ax, bx, cx);
		}
	}
	else
	{
		// 0b59
		if (x1 == x2)
		{
			ULTIMA_08e6_ClipRectCoord(&ax, &bx, &cx, &dx);
			// 0x3c: v_line
			DRV_3c(ax, bx, dx);
			return;
		}
		else
		{
			// 0b6c
			// 0x33: line
			DRV_33(ax, bx, cx, dx);
			return;
		}
	}

	// 0b84
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0b86_GRAP_XorFillRect(int x1, int y1, int x2, int y2)
{
    ULTIMA_08e6_ClipRectCoord(&x1, &y1, &x2, &y2);
    DRV_3f(x1, y1, x2, y2, 1);
}
#endif

void* ULTIMA_125d_LoadResourceFileImpl(char* file_name);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void* ULTIMA_0bae_LoadImageFile(char* file_name)
{
    debug("ULTIMA_0bae_LoadImageFile(%s)", file_name);

	return ULTIMA_125d_LoadResourceFileImpl(file_name);

	// if (error)
    //     if (ret == 0) ax = -1;
    //     D_535c_dosErrorCode = ax
	//     D_5394_diskSwapFn(); // disk swap message
	// else
	//     DRV_42(); // prepare image data for ega (swizzle)
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0be4_FreeImage(void* ptr)
{
    debug("ULTIMA_0be4_FreeImage");
	// int 21,49h
    //free(ptr);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// put image (forced hflip)
// Driver function 63: put an image mirrored (the dungeon view's walls).
void ULTIMA_0bfc_GRAP_63_PutMirrored(byte* param_1, int param_2, int param_3, int param_4, int param_5)
{
    //debug("ULTIMA_0bfc_GRAP_63_PutMirrored(ptr,%d,%d,%d,%d)", param_2, param_3, param_4, param_5);

    DRV_63(param_1, param_2, param_3, param_4, param_5);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0c22_GRAP_0f_SelectPage(int a)
{
	DRV_0f(a);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// set clip window coord
// Set the clip window (x1, y1, x2, y2); D_52c4_clipEnabled switches it on.
void ULTIMA_0c3c_SetClipWindow(int a, int b, int c, int d)
{
    D_52ba_vdp.clipX1 = a;
    D_52ba_vdp.clipY1 = b;
    D_52ba_vdp.clipX2 = c;
    D_52ba_vdp.clipY2 = d;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0c64_GRAP_30_Pset(int param_1, int param_2)
{
    D_52ba_vdp.penX = param_1;
    D_52ba_vdp.penY = param_2;

	if (D_52da_penColor != -1)
    {
        if (D_52ba_vdp.clipEnabled != 0 && ULTIMA_08ca_InsideClipWindow(param_1, param_2))
        {
            return;
        }

        DRV_30(param_1, param_2);
    }
}
#endif

static bool ULTIMA_0ccd_HLineOutsideClip(int* pAX, int* pCX);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0c9c_GRAP_39_HorizLine(int x1, int y, int x2)
{
	int ax = x1;
	int bx = y;
	int cx = x2;

	D_52ba_vdp.penX = x2;

	if (D_52ba_vdp.clipEnabled != 0)
	{
		if (ULTIMA_0ccd_HLineOutsideClip(&ax, &cx))	// JC end
			return;
	}

	DRV_39(ax, bx, cx);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static bool ULTIMA_0ccd_HLineOutsideClip(int *pAX, int *pCX)
{
	if (*pAX < *pCX)
	{
		SWAP(pAX, pCX);
	}

	if ((*pAX >= D_52ba_vdp.clipX1) && (*pCX <= D_52ba_vdp.clipX2) &&
		*pAX >= 0 && *pAX <= 319 && *pCX <= 319)
	{
		return false; // CLC
	}

	return true; // STC
}
#endif

static int ULTIMA_0d2b_VLineOutsideClip(int bx, int dx);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// vline
void ULTIMA_0cf2_GRAP_3c_VertLine(int param_1, int param_2, int param_3)
{
	int ax, bx, cx, dx;

    ax = param_1;
    bx = param_2;
    dx = param_3;
    cx = ax;
    D_52ba_vdp.penY = dx;
	if (bx > dx)
    {
		// xchg bx, dx
        bx ^= dx;
        dx ^= bx;
        bx ^= dx;
	}
    if (D_52ba_vdp.clipEnabled == 0 || !ULTIMA_0d2b_VLineOutsideClip(bx, dx))
    {
        // 0d19
        DRV_3c(ax, bx, dx);
	}
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
static int ULTIMA_0d2b_VLineOutsideClip(int bx, int dx)
{
    if (D_52ba_vdp.clipY1 <= bx && dx <= D_52ba_vdp.clipY2 && 0 <= bx && bx < 200 && dx < 200)
    {
        return 0; // clc
    }

    return -1; // stc
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// put_image(rsrc, imageIdx, x, y, vflip?)
void ULTIMA_0d4c_GRAP_4b_PutImage(void* rsrc, int idx, int x, int y, int flags)
{
    //debug("ULTIMA_0d4c_GRAP_4b_PutImage(rsrc:ptr,idx:%d,x:%d,y:%d,flags:%d)", idx, x, y, flags);
	
	DRV_4b(rsrc, idx, x, y, flags);
}
#endif


#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0de0_DetectVideo(void)
{
    debug("ULTIMA_0de0_DetectVideo");
    D_52ba_vdp.videoDriver = VDP_VIDEO_EGA;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB (asm)
int ULTIMA_0e94_LoadVideoDriver(void)
{
    debug("ULTIMA_0e94_LoadVideoDriver");
    // DUMMY
    return 1;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB (asm)
int ULTIMA_0f2a_GRAP_06_AllocPageBuffer(void)
{
    debug("ULTIMA_0f2a_GRAP_06_AllocPageBuffer");
    // DUMMY
    return 1;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0f46_GRAP_66_Reveal(int x1, int y1, int x2, int y2)
{
    debug("ULTIMA_0f46_GRAP_66_Reveal(%d,%d,%d,%d)", x1, y1, x2, y2);
    ULTIMA_08e6_ClipRectCoord(&x1, &y1, &x2, &y2);
	DRV_66(x1, y1, x2, y2, 0, 0, 0);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
void ULTIMA_0f6e_GRAP_1b_TransferFullscreen(int from, int to)
{
    debug("ULTIMA_0f6e_GRAP_1b_TransferFullscreen(%d,%d)", from, to);
    if (from != to)
    {
        DRV_1b(from, to);
    }
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// Draw a line from the pen position to (x, y).
void ULTIMA_0f90_GRAP_LineTo(int x, int y)
{
	ULTIMA_0b2d_GRAP_Line(D_52ba_vdp.penX, D_52ba_vdp.penY, x, y, 0);
}
#endif

void* ULTIMA_125d_LoadResourceFileImpl(char* file_name);

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
byte* ULTIMA_0fae_LoadResourceFile(char* file_name)
{
	byte* mem;
    debug("ULTIMA_0fae_LoadFile(%s)", file_name);
	mem = (byte*)ULTIMA_125d_LoadResourceFileImpl(file_name);
	if (!mem)
	{
		D_535c_dosError = 1;
	    D_5394_diskSwapFn();
	}

	// if error (cf == 1)
	//     if ret == 0
	//         ret = -1
	//     else
	//         535c_dosErrorCode = ret
	//         5394_diskSwapMessage();

	return mem;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
// NOTE: same as 0be4
void ULTIMA_0fdc_FreeBitImage(void* ptr)
{
    debug("ULTIMA_0fdc_FreeBitImage(ptr)");
	// int 21,49
    //free(ptr);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// ASM
int ULTIMA_0ff4_LoadTileset(char* file_name)
{
	byte* tileset;
    debug("ULTIMA_0ff4_LoadTileset(%s)", file_name);
	tileset = (byte*)ULTIMA_125d_LoadResourceFileImpl(file_name);

	DRV_48_LoadTileset(tileset);

	return 1;
}
#endif
