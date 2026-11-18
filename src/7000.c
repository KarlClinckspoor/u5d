#include "common/common.h"
#include "funcs.h"
#include "vars.h"

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

// ULTIMA:1000 MAX: < ULTIMA_7296
