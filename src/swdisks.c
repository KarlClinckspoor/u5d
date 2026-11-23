// Resident code, CS:251e-256e: ULTIMA_251e, the last function of the object that starts at
// CS:2320 (diskswap.c, MSC 5.1). The relocation table of ULTIMA.EXE has the words of 2320-256e
// in one falling run (2567, 252b, 2516, 2333): MSC writes the fixups of one code record from
// the last to the first, so 2320, 2322 and 251e were one object, and 256e starts the next
// (fileio.c). It is a file of its own here because the DOS build takes ULTIMA_2322, which
// lies between, from src/asm/A2322.ASM (tools/u5.py, OBJECT_PARTS).

#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <stdlib.h>
#include <time.h>

void SwitchDisks(int diskNumber)
{
    diskSwapFn = NoDiskSwapMessage;

    if (diskNumber == 2 || diskNumber == 5)
    {
        diskNumber = 1;
    }

    curDisk[0] = diskNumber;

    if (diskDrives[diskNumber] != 0xff)
    {
        SetDefaultDrive(diskDrives[diskNumber]);
        lastDisk = 0xff;
    }

    diskSwapFn = DiskSwapMessage;
}
