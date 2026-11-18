#include "common/common.h"

#include "tiles.h"

#if defined(TARGET_DOS16)
// DOS build: this object holds no data, only the communal (uninitialised) variables
// (vars.h, BSS); every initialised variable is defined by the module that owns it.
#define BSS
#else
#define STUB
#endif
#include "vars.h"

#include "macros.h"

// 00 00

// 00 00

#if !defined(TARGET_DOS16) // DS:52ba-545f: the assembly modules' own data in the original
VideoDriverParams D_52ba_vdp =
{
    0,      // 52ba
    8,      // 52bc
    8,      // 52be
    0x10,   // 52c0
    0x10,   // 52c2
    0,      // 52c4
    0,      // 52c6
    0,      // 52c8
    0,      // 52ca
    0,      // 52cc
    0,      // 52ce
    0,      // 52d0
    0x013f, // 52d2
    0,      // 52d4
    0xc7,   // 52d6
    0       // 52d8
};

int D_5304 = -1;

u16 D_5358_memorySize = 0xffff;

u16 D_538e = 1;

u16 D_5390 = 5;

u16 D_5392 = 4; // TODO: int

TextWindow* D_539a_currentTextWindow = 0;

// arrow key: L, R, D, U, Home, End, PgUp, PgDn
u8 D_540e[8] = { 0x4b, 0x4d, 0x48, 0x50, 0x47, 0x4f, 0x49, 0x51 };

u8 D_5416[8] = { 0x01, 0x02, 0x03, 0x04, 0xd3, 0xd4, 0xd5, 0xd6 };
#endif

// D_535e_textWindows: filled with 0

//u16 D_5394 = 0x1232;

// NOTE: [55a6..6606): filled with 0
// NOTE: [6608..6a08): filled with 0

