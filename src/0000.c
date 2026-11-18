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

static void ULTIMA_0991_ClipLineBisect(int* ax, int* bx, int* cx, int* dx);
static void ULTIMA_0a22_ClipOutcode(int ax, int bx, int cx, int dx, int* si, int* di);

static void ULTIMA_0b2d_GRAP_Line(int ax, int bx, int cx, int dx, byte carry);

void* ULTIMA_125d_LoadResourceFileImpl(char* file_name);

static bool ULTIMA_0ccd_HLineOutsideClip(int* pAX, int* pCX);

static int ULTIMA_0d2b_VLineOutsideClip(int bx, int dx);

void* ULTIMA_125d_LoadResourceFileImpl(char* file_name);

