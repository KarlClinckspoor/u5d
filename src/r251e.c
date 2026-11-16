// Resident code, CS:251e-266c: a module of its own in the original, compiled with MSC 5.0:
// its one string is in CONST (DS:a0e0, after FONT.OVL's strings and before 3000.c's).

#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include "audio/aud_sfx.h"
#include "event/event.h"
#include "time/time.h"

#include <stdlib.h>
#include <time.h>

void ULTIMA_251e_SwitchDisks(int param_1)
{
    D_5394_fn = ULTIMA_2320_NoDiskSwapMessage;

    if (param_1 == 2 || param_1 == 5)
    {
        param_1 = 1;
    }

    D_a9bd[0] = param_1;

    if (D_a9c8[param_1] != 0xff)
    {
        ULTIMA_1eac_SetDefaultDrive(D_a9c8[param_1]);
        D_545e = 0xff;
    }

    D_5394_fn = ULTIMA_2322_DiskSwapMessage;
}

// CHECKED
void ULTIMA_256e_ReadFileFromDisk(char* fileName, void* addr, u16 size, u16 offset)
{
    int local_4 = 0;

#if !defined(TARGET_DOS16)
    while (local_4 == 0)
    {
        local_4 = ULTIMA_7234_ReadFile(fileName, addr, size, offset);
        if (local_4 == 0)
        {
            EVT_Yield();
        }
    }
    return;
#endif

    if (D_a9bd[0] == 3 && D_a9c8[D_a9bd[0]] == 0xff)
    {
        ULTIMA_2322_DiskSwapMessage(); // in another object: a far call
    }

    ULTIMA_1eac_SetDefaultDrive(D_a9c8[D_a9bd[0]]);

    while (local_4 == 0)
    {
        local_4 = ULTIMA_7234_ReadFile(fileName, addr, size, offset);
    }
}

static void FAR ULTIMA_25ca_WriteError(void)
{
    ULTIMA_1850_PrintString(_TEXT(0xa0e0, "\nYour disk may be write-protected. Try again.\n"));
    ULTIMA_1dda_WaitForKeystroke(0);
}

// D_a9c8[D_a9bd[0]] rather than D_a9c8[local_6]: the repeated D_a9bd[0] is what MSC keeps
// in SI (a common subexpression), besides storing it in local_6.
void ULTIMA_25d8_WriteFileToDisk(char* fileName, void* addr, u16 size)
{
#if !defined(TARGET_DOS16)
    ULTIMA_7296_WriteFile(fileName, addr, size);
    return;
#endif

    int local_6;
    int local_4_b;

    local_4_b = 0;

    local_6 = D_a9bd[0];
    if (D_a9bd[0] == 3 && D_a9c8[D_a9bd[0]] == 0xff)
    {
        ULTIMA_2322_DiskSwapMessage(); // in another object: a far call
    }

    ULTIMA_1eac_SetDefaultDrive(D_a9c8[D_a9bd[0]]);

    while (local_4_b == 0)
    {
        while (ULTIMA_1674_TestOpenFile(fileName) == 0) {}

        D_5394_fn = ULTIMA_25ca_WriteError;

        local_4_b = ULTIMA_7296_WriteFile(fileName, addr, size);

        D_5394_fn = ULTIMA_2322_DiskSwapMessage;
    }

    D_a9bd[0] = local_6;
    ULTIMA_1eac_SetDefaultDrive(D_a9c8[D_a9bd[0]]);
}
