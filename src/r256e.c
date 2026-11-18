// Resident code, CS:256e-266c: a module of its own in the original, compiled with MSC 5.0:
// its one string is in CONST (DS:a0e0, after FONT.OVL's strings and before 3000.c's), and
// it calls ULTIMA_2322 with a far call, which MSC makes only for a far function of another
// object. Its relocated words follow those of 2320-256e in ULTIMA.EXE's table (r251e.c).

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

// CHECKED
void ULTIMA_256e_ReadFileFromDisk(char* fileName, void* addr, u16 size, u16 offset)
{
    int done = 0;

    if (D_a9bd_curDisk[0] == 3 && D_a9c8_diskDrives[D_a9bd_curDisk[0]] == 0xff)
    {
        ULTIMA_2322_DiskSwapMessage(); // in another object: a far call
    }

    ULTIMA_1eac_SetDefaultDrive(D_a9c8_diskDrives[D_a9bd_curDisk[0]]);

    while (done == 0)
    {
        done = ULTIMA_7234_ReadFile(fileName, addr, size, offset);
    }
}

static void far ULTIMA_25ca_WriteError(void)
{
    ULTIMA_1850_PrintString(/*0xa0e0*/ "\nYour disk may be write-protected. Try again.\n");
    ULTIMA_1dda_WaitForKeystroke(0);
}

// D_a9c8_diskDrives[D_a9bd_curDisk[0]] rather than D_a9c8_diskDrives[savedDisk]: the repeated D_a9bd_curDisk[0] is what MSC keeps
// in SI (a common subexpression), besides storing it in savedDisk.
void ULTIMA_25d8_WriteFileToDisk(char* fileName, void* addr, u16 size)
{
    int savedDisk;
    int done_j;

    done_j = 0;

    savedDisk = D_a9bd_curDisk[0];
    if (D_a9bd_curDisk[0] == 3 && D_a9c8_diskDrives[D_a9bd_curDisk[0]] == 0xff)
    {
        ULTIMA_2322_DiskSwapMessage(); // in another object: a far call
    }

    ULTIMA_1eac_SetDefaultDrive(D_a9c8_diskDrives[D_a9bd_curDisk[0]]);

    while (done_j == 0)
    {
        while (ULTIMA_1674_TestOpenFile(fileName) == 0) {}

        D_5394_diskSwapFn = ULTIMA_25ca_WriteError;

        done_j = ULTIMA_7296_WriteFile(fileName, addr, size);

        D_5394_diskSwapFn = ULTIMA_2322_DiskSwapMessage;
    }

    D_a9bd_curDisk[0] = savedDisk;
    ULTIMA_1eac_SetDefaultDrive(D_a9c8_diskDrives[D_a9bd_curDisk[0]]);
}
