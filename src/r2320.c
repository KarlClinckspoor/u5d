// Resident code, CS:2320-251e: a module of its own in the original (MSC 5.1: its strings
// are in _DATA, DS:545f). ULTIMA_256e and ULTIMA_25d8 call ULTIMA_2322 with a far call,
// which MSC makes only for a far function of another object.
// The DOS build takes ULTIMA_2322 (CS:2322-251e) from src/asm/A2322.ASM: GOG patched the
// function in place, so its bytes are no compiler's output. A2322.ASM also holds this
// object's data (D_545e and the strings, DS:545e-54b6): with 2322 gone, no code here refers
// to D_545e, and the link places an object's data by its references.

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
#if !defined(TARGET_DOS16) // DOS build: src/asm/A2322.ASM
u8 D_545e = 0xff;
#endif


void FAR ULTIMA_2320_NoDiskSwapMessage(void)
{
    // nop
}

#if !defined(TARGET_DOS16)
void FAR ULTIMA_2322_DiskSwapMessage(void)
{
    D_5394_fn = ULTIMA_2320_NoDiskSwapMessage;

    if (D_a9bd[0] == 4)
    {
        ULTIMA_16ba_PrintChar(CTRL_CHAR_CLEAR_WINDOW);
        ULTIMA_1850_PrintString(_TEXT(0x11400, "Please place the Ultima IV 'party.sav' file into the 'U4SAVE' directory."));
        ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0));
        ULTIMA_16ba_PrintChar('\n');
    }

    D_5394_fn = ULTIMA_2322_DiskSwapMessage;
}
#elif 0
// Not compiled: the DOS build has ULTIMA_2322 from src/asm/A2322.ASM (above). This is 1.16's
// function with GOG's patch written as gotos, kept for reference; the patched bytes are not
// compiler output, and this came within 2 bytes (506 against 508).
// GOG patched: skip drive selection
void FAR ULTIMA_2322_DiskSwapMessage(void)
{
    byte local_6_an;
    byte local_4;

    D_5394_fn = ULTIMA_2320_NoDiskSwapMessage;

    local_6_an = D_a9c8[D_a9bd[0]];
    if (local_6_an != 0xff && D_545e != D_a9bd[0] && ULTIMA_16a6_GetDefaultDrive() != local_6_an)
    {
        // 235a
        ULTIMA_1eac_SetDefaultDrive(D_a9c8[D_a9bd[0]]);
        // 236d
        D_545e = D_a9bd[0];
    }
    else
    {
        // 2374
        if (D_a9c8[D_a9bd[0]] < 0x43 && D_a9c2 > 1)
        {
            D_a9c8[D_a9bd[0]] ^= 3;
            ULTIMA_1eac_SetDefaultDrive(D_a9c8[D_a9bd[0]]);
            // 2394
            if (D_545e != D_a9bd[0])
            {
                // -> 236d
                D_545e = D_a9bd[0];
                goto L_250c; // TODO: remove goto
            }
        }

        // 239d
        if (D_a9c8[D_a9bd[0]] >= 'C' && D_a9c8[D_a9bd[0]] != 0xff)
        {
            D_a9c8[D_a9bd[0]] = 'A';
            ULTIMA_1eac_SetDefaultDrive('A');
            D_545e = 0xff;
        }
        else
        {
            if (D_a9bd[1] == 2)
            {
                ULTIMA_0c22_GRAP_0f_SelectPage(0);
                ULTIMA_1c22_SetTextWindowSize(0, 6, 0xc, 0x21, 0xd);
                ULTIMA_0a70_GRAP_2d_SetPenColor(0);
                ULTIMA_0aa6_GRAP_3f_FillRect(0x28, 0x58, 0x117, 0x77);
            }

            // 2400
            D_545e = 0xff;

#if defined(GOG_BUILD)
            // 2405 GOG: PATCHED (different instructions)
            local_4 = ULTIMA_16a6_GetDefaultDrive();
            goto L_GOG_247d;
#endif

            // 2405
            ULTIMA_1850_PrintString(_TEXT(0x545f, "\nPlease insert the Ultima "));
            if (D_a9bd[0] == 4)
            {
                ULTIMA_16ba_PrintChar('I');
            }

            ULTIMA_1850_PrintString(_TEXT(0x547a, "V "));

            // 360k: 0=Program 1,3=Britannia 4=Player 2=Dungeon 5=Underworld
            // 720k: 0=Program 1,3=Britannia 4=Player (2, 5 => 1)
            switch (D_a9bd[0])
            {
            case 0:
                ULTIMA_1850_PrintString(_TEXT(0x547d, "Program"));
                break;
            case 1:
            case 3:
                ULTIMA_1850_PrintString(_TEXT(0x5485, "Britannia"));
                break;

            case 4:
                ULTIMA_1850_PrintString(_TEXT(0x548f, "Player"));
                break;
            }

            ULTIMA_1850_PrintString(_TEXT(0x5496, " Disk"));

            if (D_a9c8[D_a9bd[0]] == 0xff)
            {
                ULTIMA_1850_PrintString(_TEXT(0x549c, " and press drive letter: "));
            }
            else
            {
                ULTIMA_16ba_PrintChar('.');
            }

            do
            {
                // 2473
                local_4 = ULTIMA_2032_ToUpper(ULTIMA_1dda_WaitForKeystroke(0));

#if defined(GOG_BUILD)
L_GOG_247d:
#endif
                // 247d
                if (D_a9c8[D_a9bd[0]] != 0xff)
                    break;

            } while (ULTIMA_1eac_SetDefaultDrive(local_4) == 0);

            // 2497
            if (D_a9c8[D_a9bd[0]] == 0xff)
            {
                // 24aa
#if !defined(GOG_BUILD)
                // GOG: PATCHED (3 nops)
                ULTIMA_16ba_PrintChar(local_4);
#endif

                if (local_4 == 'B')
                {
                    D_a9c2 = 2;
                }

                D_a9c8[D_a9bd[0]] = local_4;
                if (D_a9bd[0] == 3 && D_a9c9 == 0xff)
                {
                    D_a9c9 = local_4;
                }
            }

            if (D_a9bd[1] == 0)
            {
                ULTIMA_16ba_PrintChar('\n');
            }
            else if (D_a9bd[1] == 2)
            {
                ULTIMA_0c22_GRAP_0f_SelectPage(1);
                ULTIMA_1c22_SetTextWindowSize(0, 0, 0, 0x27, 0x18);
                ULTIMA_0f6e_GRAP_1b_TransferFullscreen(1, 0);
            }
        }
    }

L_250c:
    D_5394_fn = ULTIMA_2322_DiskSwapMessage;
}
#endif
