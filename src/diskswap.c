// Resident code, CS:2320-251e: a module of its own in the original (MSC 5.1: its strings
// are in _DATA, DS:545f). ULTIMA_256e and ULTIMA_25d8 call ULTIMA_2322 with a far call,
// which MSC makes only for a far function of another object.
// The GOG (1992) DOS build takes ULTIMA_2322 (CS:2322-251e) from src/asm/A2322.ASM: the 1992
// patch changed the function in place, so its bytes are no compiler's output. A2322.ASM also
// holds this object's data (D_545e_lastDisk and the strings, DS:545e-54b6): with 2322 gone, no code
// here refers to D_545e_lastDisk, and the link places an object's data by its references.
// The 1988 build (RELEASE_1988) compiles ULTIMA_2322 and its data from the C below: it is
// byte-identical to the October 1988 ULTIMA.EXE (match.py with U5_GAMEDATA=../gamedata-1988).

#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <stdlib.h>
#include <time.h>
#if defined(RELEASE_1988) // GOG DOS build: src/asm/A2322.ASM
char D_545e_lastDisk = 0xff; // char, not u8: see vars.h
#endif

void far ULTIMA_2320_NoDiskSwapMessage(void)
{
    // nop
}

#if defined(RELEASE_1988)
// The 1988 release's function, EXACT against the 1988 ULTIMA.EXE. The GOG DOS build does not
// compile it: it has ULTIMA_2322 from src/asm/A2322.ASM (above), as the 1992 patch left it
// (it skips the drive selection; the patched bytes are not compiler output).
// What matched it: D_545e_lastDisk as a plain char (vars.h). With u8 it was 506 bytes against 508.
void far ULTIMA_2322_DiskSwapMessage(void)
{
    byte drive;
    byte currentDrive_d;

    D_5394_diskSwapFn = ULTIMA_2320_NoDiskSwapMessage;

    drive = D_a9c8_diskDrives[D_a9bd_curDisk[0]];
    if (drive != 0xff && D_545e_lastDisk != D_a9bd_curDisk[0] && ULTIMA_16a6_GetDefaultDrive() != drive)
    {
        // 235a
        ULTIMA_1eac_SetDefaultDrive(D_a9c8_diskDrives[D_a9bd_curDisk[0]]);
        // 236d
        D_545e_lastDisk = D_a9bd_curDisk[0];
    }
    else
    {
        // 2374
        if (D_a9c8_diskDrives[D_a9bd_curDisk[0]] < 0x43 && D_a9c2_floppyCount > 1)
        {
            D_a9c8_diskDrives[D_a9bd_curDisk[0]] ^= 3;
            ULTIMA_1eac_SetDefaultDrive(D_a9c8_diskDrives[D_a9bd_curDisk[0]]);
            // 2394
            if (D_545e_lastDisk != D_a9bd_curDisk[0])
            {
                // -> 236d
                D_545e_lastDisk = D_a9bd_curDisk[0];
                goto L_250c; // TODO: remove goto
            }
        }

        // 239d
        if (D_a9c8_diskDrives[D_a9bd_curDisk[0]] >= 'C' && D_a9c8_diskDrives[D_a9bd_curDisk[0]] != 0xff)
        {
            D_a9c8_diskDrives[D_a9bd_curDisk[0]] = 'A';
            ULTIMA_1eac_SetDefaultDrive('A');
            D_545e_lastDisk = 0xff;
        }
        else
        {
            if (D_a9be_promptMode == 2)
            {
                ULTIMA_0c22_GRAP_0f_SelectPage(0);
                ULTIMA_1c22_SetTextWindowSize(0, 6, 0xc, 0x21, 0xd);
                ULTIMA_0a70_GRAP_2d_SetPenColor(0);
                ULTIMA_0aa6_GRAP_3f_FillRect(0x28, 0x58, 0x117, 0x77);
            }

            // 2400
            D_545e_lastDisk = 0xff;

            // 2405
            ULTIMA_1850_PrintString(/*0x545f*/ "\nPlease insert the Ultima ");
            if (D_a9bd_curDisk[0] == 4)
            {
                ULTIMA_16ba_PrintChar('I');
            }

            ULTIMA_1850_PrintString(/*0x547a*/ "V ");

            // 360k: 0=Program 1,3=Britannia 4=Player 2=Dungeon 5=Underworld
            // 720k: 0=Program 1,3=Britannia 4=Player (2, 5 => 1)
            switch (D_a9bd_curDisk[0])
            {
            case 0:
                ULTIMA_1850_PrintString(/*0x547d*/ "Program");
                break;
            case 1:
            case 3:
                ULTIMA_1850_PrintString(/*0x5485*/ "Britannia");
                break;

            case 4:
                ULTIMA_1850_PrintString(/*0x548f*/ "Player");
                break;
            }

            ULTIMA_1850_PrintString(/*0x5496*/ " Disk");

            if (D_a9c8_diskDrives[D_a9bd_curDisk[0]] == 0xff)
            {
                ULTIMA_1850_PrintString(/*0x549c*/ " and press drive letter: ");
            }
            else
            {
                ULTIMA_16ba_PrintChar('.');
            }

            do
            {
                // 2473
                currentDrive_d = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0));

                // 247d
                if (D_a9c8_diskDrives[D_a9bd_curDisk[0]] != 0xff)
                    break;

            } while (ULTIMA_1eac_SetDefaultDrive(currentDrive_d) == 0);

            // 2497
            if (D_a9c8_diskDrives[D_a9bd_curDisk[0]] == 0xff)
            {
                // 24aa
                // GOG: PATCHED (3 nops)
                ULTIMA_16ba_PrintChar(currentDrive_d);

                if (currentDrive_d == 'B')
                {
                    D_a9c2_floppyCount = 2;
                }

                D_a9c8_diskDrives[D_a9bd_curDisk[0]] = currentDrive_d;
                if (D_a9bd_curDisk[0] == 3 && D_a9c9 == 0xff)
                {
                    D_a9c9 = currentDrive_d;
                }
            }

            if (D_a9be_promptMode == 0)
            {
                ULTIMA_16ba_PrintChar('\n');
            }
            else if (D_a9be_promptMode == 2)
            {
                ULTIMA_0c22_GRAP_0f_SelectPage(1);
                ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);
                ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
            }
        }
    }

L_250c:
    D_5394_diskSwapFn = ULTIMA_2322_DiskSwapMessage;
}
#endif
