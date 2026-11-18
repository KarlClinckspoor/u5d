// Resident code, CS:251e-256e: ULTIMA_251e, the last function of the object that starts at
// CS:2320 (r2320.c, MSC 5.1). The relocation table of ULTIMA.EXE has the words of 2320-256e
// in one falling run (2567, 252b, 2516, 2333): MSC writes the fixups of one code record from
// the last to the first, so 2320, 2322 and 251e were one object, and 256e starts the next
// (r256e.c). It is a file of its own here because the DOS build takes ULTIMA_2322, which
// lies between, from src/asm/A2322.ASM (tools/u5.py, OBJECT_PARTS).

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
    D_5394_diskSwapFn = ULTIMA_2320_NoDiskSwapMessage;

    if (param_1 == 2 || param_1 == 5)
    {
        param_1 = 1;
    }

    D_a9bd_curDisk[0] = param_1;

    if (D_a9c8_diskDrives[param_1] != 0xff)
    {
        ULTIMA_1eac_SetDefaultDrive(D_a9c8_diskDrives[param_1]);
        D_545e = 0xff;
    }

    D_5394_diskSwapFn = ULTIMA_2322_DiskSwapMessage;
}
