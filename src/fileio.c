// Resident code, CS:256e-266c: a module of its own in the original, compiled with MSC 5.0:
// its one string is in CONST (DS:a0e0, after FONT.OVL's strings and before commands.c's), and
// it calls ULTIMA_2322 with a far call, which MSC makes only for a far function of another
// object. Its relocated words follow those of 2320-256e in ULTIMA.EXE's table (swdisks.c).

#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <stdlib.h>
#include <time.h>

// CHECKED
void ReadFileFromDisk(char* fileName, void* addr, u16 size, u16 offset)
{
    int done = 0;

    if (curDisk[0] == 3 && diskDrives[curDisk[0]] == 0xff)
    {
        DiskSwapMessage(); // in another object: a far call
    }

    SetDefaultDrive(diskDrives[curDisk[0]]);

    while (done == 0)
    {
        done = ReadFile(fileName, addr, size, offset);
    }
}

static void far WriteError(void)
{
    PrintString(/*0xa0e0*/ "\nYour disk may be write-protected. Try again.\n");
    WaitForKeystroke(0);
}

// diskDrives[curDisk[0]] rather than diskDrives[savedDisk]: the repeated curDisk[0] is what MSC keeps
// in SI (a common subexpression), besides storing it in savedDisk.
void WriteFileToDisk(char* fileName, void* addr, u16 size)
{
    int savedDisk;
    int done_j;

    done_j = 0;

    savedDisk = curDisk[0];
    if (curDisk[0] == 3 && diskDrives[curDisk[0]] == 0xff)
    {
        DiskSwapMessage(); // in another object: a far call
    }

    SetDefaultDrive(diskDrives[curDisk[0]]);

    while (done_j == 0)
    {
        while (TestOpenFile(fileName) == 0) {}

        diskSwapFn = WriteError;

        done_j = WriteFile(fileName, addr, size);

        diskSwapFn = DiskSwapMessage;
    }

    curDisk[0] = savedDisk;
    SetDefaultDrive(diskDrives[curDisk[0]]);
}
