#include "common/common.h"

#include "tiles.h"

#define STUB
#include "vars.h"

#include "macros.h"

// 00 00

// 00 00

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

int D_5306[7] = {0x0018, 0x0028, 0x0048, 0x0070, 0x0098, 0x00d8, 0x0118};
int D_5314[7] = {0x0003, 0x0007, 0x000b, 0x0014, 0x0020, 0x002d, 0x003d};

u16 D_5358 = 0xffff;

// D_535e_textWindows: filled with 0

u16 D_538e = 1;
u16 D_5390 = 5;
u16 D_5392 = 4; // TODO: int
//u16 D_5394 = 0x1232;

TextWindow* D_539a_currentTextWindow = 0;

s16 D_5402[5] = { 1, 10, 100, 1000, 10000 };

// arrow key: L, R, D, U, Home, End, PgUp, PgDn
u8 D_540e[8] = { 0x4b, 0x4d, 0x48, 0x50, 0x47, 0x4f, 0x49, 0x51 };
u8 D_5416[8] = { 0x01, 0x02, 0x03, 0x04, 0xd3, 0xd4, 0xd5, 0xd6 };

u8 D_545e = 0xff;

// NOTE: [55a6..6606): filled with 0
// NOTE: [6608..6a08): filled with 0












