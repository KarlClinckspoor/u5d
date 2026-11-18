#include "common/common.h"
#include "funcs.h"
#include "vars.h"

#include "grap_drv.h"
#include "common/file.h"

// Write one frame (param_1 mod 16) of a lighthouse beam, 16 cells, to the combat map around (param_3,param_4): param_2 = 1 sets 0xff, else 0.
static void ULTIMA_7040_SetBeamCells(int frame, int setCells, int beaconX, int beaconY)
{
    int cellIndex_i;
    PairS8* cell;
    u8 fill;

    fill = 0;
    if (setCells == 1)
    {
        fill = 0xff;
    }

    frame = (frame % 0x10);
    cell = &D_1f7e_beamCells[frame * 0x10];

    for (cellIndex_i = 0; cellIndex_i < 0x10; cellIndex_i++)
    {
        D_ad14_combatMap[(cell->y + beaconY) * 0x20 + cell->x + beaconX] = fill;
        cell++;
    }
}

// Turn the beam of the lighthouse lamps in the dark (D_217e_beaconX...).
void ULTIMA_70a6_AnimateLighthouse(void)
{
    if (D_58a5_lightLevel >= 0x32 || D_217e_beaconX == -1)
    {
        D_2186_beaconStep = 0xff;
        return;
    }

    D_24e6_viewDirty = 1;
    if (D_2186_beaconStep == 0xff)
    {
        D_2186_beaconStep++;
        ULTIMA_7040_SetBeamCells(0, 1, D_217e_beaconX, D_2180_beaconY);
        ULTIMA_7040_SetBeamCells(1, 1, D_217e_beaconX, D_2180_beaconY);
        ULTIMA_7040_SetBeamCells(2, 1, D_217e_beaconX, D_2180_beaconY);

        if (D_2182_beacon2X != -1)
        {
            ULTIMA_7040_SetBeamCells(0, 1, D_2182_beacon2X, D_2184_beacon2Y);
            ULTIMA_7040_SetBeamCells(1, 1, D_2182_beacon2X, D_2184_beacon2Y);
            ULTIMA_7040_SetBeamCells(2, 1, D_2182_beacon2X, D_2184_beacon2Y);
        }
    }
    else
    {
        ULTIMA_7040_SetBeamCells(D_2186_beaconStep, 0, D_217e_beaconX, D_2180_beaconY);

        if (D_2182_beacon2X != -1)
        {
            ULTIMA_7040_SetBeamCells(D_2186_beaconStep, 0, D_2182_beacon2X, D_2184_beacon2Y);
        }

        D_2186_beaconStep++;
        ULTIMA_7040_SetBeamCells(D_2186_beaconStep + 2, 1, D_217e_beaconX, D_2180_beaconY);

        if (D_2182_beacon2X != -1)
        {
            ULTIMA_7040_SetBeamCells(D_2186_beaconStep + 2, 1, D_2182_beacon2X, D_2184_beacon2Y);
        }
    }

    if (D_2186_beaconStep > 0xf)
    {
        D_2186_beaconStep = 0;
    }
}

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// NOT MATCHING: driver
// Driver function 6c, mode 2, with the hour and the minute: probably the hands of the town clock tile.
void ULTIMA_71aa_UpdateClockTiles(void)
{
    int ax = 2;
    byte bl = D_587f_hour;
    byte bh = D_5881_minute;
    DRV_6c(ax, bl, bh);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// NOT MATCHING: driver
void ULTIMA_71ca_DRV_27_ScrollTextWindow1(int left, int top, int right, int bottom, int rows)
{
    int di;

    ULTIMA_08e6_ClipRectCoord(&left, &top, &right, &bottom);

    di = bottom - top;
    if (rows > di)
    {
        rows = di;
    }

    // scroll text window
    DRV_27(left, top, right, bottom, -rows);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// NOT MATCHING: driver
void ULTIMA_7200_DRV_27_ScrollTextWindow2(int left, int top, int right, int bottom, int rows)
{
    int di;

    ULTIMA_08e6_ClipRectCoord(&left, &top, &right, &bottom);

    di = bottom - top;
    if (rows > di)
    {
        rows = di;
    }

    // scroll text window
    DRV_27(left, top, right, bottom, rows);
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB
int ULTIMA_7234_ReadFile(char* fileName, void* addr, u16 size, u16 offset)
{
    D_535c_dosError = 0;
    if (FILE_ReadFile(fileName, addr, size, offset) != 0)
    {
        D_535c_dosError = 1;
        D_5394_diskSwapFn();

        return 0;
    }

    return 1;
}
#endif

#if !defined(TARGET_DOS16) // assembly in the original: src/asm
// STUB
int ULTIMA_7296_WriteFile(char* fileName, void* addr, u16 size)
{
    D_535c_dosError = 0;
    if (FILE_WriteFile(fileName, addr, size, 0) != 0)
    {
        D_535c_dosError = 1;
        D_5394_diskSwapFn();

        return 0;
    }

    return 1;
}
#endif

// ULTIMA:1000 MAX: < ULTIMA_7296
