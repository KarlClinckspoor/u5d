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

static void ULTIMA_1184_InitTextWindow(void);
static void ULTIMA_11b4_CalibrateTimer(void);
static void ULTIMA_1226_InitTextVars(void);

static int ULTIMA_1588_IsFileCompressed(char* fileName);

// ASM
// ULTIMA_135a_ReadLzwCompressedFile(...)

// ASM
// ULTIMA_15c6_AllocateMemory
// ULTIMA_160e_ReadFile
// ULTIMA_1649_DosReadFile
// ULTIMA_1654_DosGetFileSize

//
// REGION: TEXT
//

static void ULTIMA_17f4_ApplyCharEffects(byte* es, int di);
static void ULTIMA_1f77_ConvertCharCoordToPixel(TextWindow* window, int* pAX, int* pBX, int* pCX, int* pDX);

//
// REGION: KEYBOARD
//

static void ULTIMA_1b24_ClearKbdBufferImpl();

static byte ULTIMA_1c5b_ClipTextWindow(int* x1, int* y1, int* x2, int* y2);

// ASM

static int ULTIMA_1ef7_ConvertDriveLetterToNumber(char* al, char* dl);

// ASM

