// Resident code, CS:1850: a module of its own in the original, between assembly modules,
// compiled with MSC 5.0 (its word-wrap loop is 5.0's code; it has no strings to show it).

#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

s16 D_5402_powersOfTen[5] = { 1, 10, 100, 1000, 10000 };

// CHECKED
// The block of comment lines after the declarations is needed for the match. Why:
// MSC 5.0's pass 2 (C2.EXE) does not optimize a function as a whole. It reads the statements
// into a tree pool and, when the pool is full, optimizes and emits what it holds and goes on
// with an empty pool (its output file grows while the function is still being read). A loop
// gets registers only if the pool has room left when it comes. Compiled in one piece this
// function fills about 505 of the pool's 612 units (a line record is one unit), and its
// last two loops (1a03: skip in CX; 1a14: printIndex in SI, the bound in DI) are not
// optimized: 64 bytes differ. The original has them optimized, so there the pool was emptied
// inside the function, before 19ca.
// Pass 1 writes a line record for every source line inside a function that is not blank
// (a comment-only line too; a comment over several lines is one; blank lines, #if'd-out
// lines and lines outside functions give none), and each record takes one unit. So extra
// lines before 19ca move a cut into the function: 0-107 lines change nothing, 108-122
// optimize 1a03 only (46 bytes), 123-563 match, 564-605 give other code, 606-713 are as
// none again, and so on with period 612 (measured by inserting records in pass 1's output
// and running C2 on it). Only the number of records counts: not the line numbers, the text,
// /Zd or /Fc; lines after 19ca, in another function or before this one do nothing.
// Less memory gives the same cut with no extra line: with 236.75 to 248.5 KB of DOS memory
// free for C2.EXE the pool is about 280 units, and this file compiles EXACT without the
// padding (research/c2mem.py print.c ULTIMA_1850_PrintString 15152 15920; above that the
// pool has its full size, below it the code is different again). So the original function
// either had 123 or more further lines (comments, lines compiled out), or was compiled with
// about 240 KB left for pass 2. Which, cannot be told from the code; the build cannot limit
// pass 2's memory, so the comment lines stand in for either.
// 0000:1850
void ULTIMA_1850_PrintString(char* text)
{
    int windowWidth_d;
    char line_e[40];
    uint centered_c;
    int savedLength_g;
    int savedPos_g;
    int skip_f;
    TextWindow* window_e;
    int room;
    int printIndex_f;
    int lineLength;
    int finished_a;
    int pos_j;
    int wrapped_h;
    int breakPending;

    // Padding: each of these lines is one more unit in pass 2's pool (see above).
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //
    //

    // debug("GRAP_PrintString(%s)", text);

    pos_j = 0;
    finished_a = 0;
    breakPending = 0;
    if (*text != '\0')
    {
        // 1872
        window_e = &D_535e_textWindows[D_5386_textWindowIndex];
        centered_c = window_e->flags & 2;
        windowWidth_d = (uint)window_e->right - (uint)window_e->left; // text_window_width = r - l;

        // 189a
        while (1)
        {
            if (breakPending != 0)
            {
                ULTIMA_16ba_PrintChar('\n');
            }

            // 18a7
            lineLength = breakPending = wrapped_h = skip_f = 0;
            room =
                windowWidth_d - ULTIMA_1f12_GetCurrentTextX(); // remaining_length = text_window_length - column

            for (; text[pos_j] != '\n' && text[pos_j] != '\r' && text[pos_j] != '\0' &&
                              lineLength <= room;
                 pos_j++, lineLength++)
            {
                line_e[lineLength] = text[pos_j];
            }

            // 18f4 (after loop)
            if (lineLength == 0)
            {
                if (text[pos_j] != '\0')
                {
                    ULTIMA_16ba_PrintChar((byte)text[pos_j++]);
                }
                else
                {
                    finished_a++;
                }
            }
            else
            {
                // 191c
                if (lineLength > room)
                {
                    // 1927
                    savedLength_g = lineLength;
                    savedPos_g = pos_j;

                    // 1939 (MSC 5.0 recomputes text + pos_j for each test; 5.1 would not)
                    for (; text[pos_j] != ' ' && lineLength != 0 && text[pos_j] != '\n' &&
                           text[pos_j] != '\0' && text[pos_j] != '\r';
                         lineLength--, pos_j--)
                    {
                    }

                    // 1968
                    if (lineLength == 0)
                    {
                        lineLength = savedLength_g;
                        pos_j = savedPos_g;

                        if (window_e->currentX != 0)
                        {
                            ULTIMA_16ba_PrintChar('\n');
                            wrapped_h = 1;
                        }
                    }
                    else
                    {
                        // 1996: lineLength in CX, stored after the loop
                        do
                        {
                            lineLength--;
                        } while (line_e[lineLength] == ' ' || line_e[lineLength] == '\r' || line_e[lineLength] == '\n');

                        if (text[pos_j] != '\0')
                            pos_j++;

                        // 19bf
                        if (lineLength < room)
                            breakPending++;
                    }

                    // 19ca
                    if (lineLength > room)
                        lineLength--;
                }
                else
                {
                    lineLength--;
                }

                // 19d5
                if (centered_c != 0)
                {
                    if (lineLength > windowWidth_d)
                    {
                        ULTIMA_16ba_PrintChar('\n');
                    }

                    // 19ea
                    ULTIMA_1bf2_SetTextPosition((room - lineLength) / 2, ULTIMA_1cee_GetCurrentTextY());
                }

                // 19fd; 1a03: skip_f in CX
                if (wrapped_h != 0)
                {
                    // 1a03
                    for (; line_e[skip_f] == ' '; skip_f++)
                    {
                    }
                }

                // 1a14: printIndex_f in SI, the bound lineLength in DI; the counter is stored in
                // printIndex_f ([bp-0Ch]) after this loop, so the loop is over printIndex_f.
                for (printIndex_f = skip_f; printIndex_f <= lineLength; printIndex_f++)
                {
                    // 1a1c
                    ULTIMA_16ba_PrintChar((byte)line_e[printIndex_f]);
                }
            }
            if (finished_a != 0)
                break;
        }
    }
    return;
}

// CHECKED
// print_integer(val, min_len, filler)
// fillChar is a char: as an int, MSC 5.0 turns the fill loop into "rep stosb" and needs a
// temporary (frame 0x34); the original's loop stores AL each time.
void ULTIMA_1a3e_PrintNumber(int value, int minWidth, char fillChar)
{
    int index;
    int negative; // is_negative
    int outPos_f; // space
    int digits_l; // required_length
    char buffer_f[40];

    digits_l = 1;
    negative = 0;

    if (minWidth > 39)
    {
        minWidth = 39;
    }

    // 1a5b
    if (value < 0) // is negative?
    {
        negative++; // is_negative = true
        value = -value;
    }

    // 1a6c: MSC walks D_5402_powersOfTen with a pointer (the extra stack slot) and sets index to 5
    for (index = 1; index < 5; index++)
    {
        if (D_5402_powersOfTen[index] <= value)
        {
            digits_l++;
        }
    }

    if (negative)
    {
        digits_l++;
    }

    // 1a94
    outPos_f = minWidth - digits_l;
    if (outPos_f <= 0)
    {
        outPos_f = 0;
    }
    else
    {
        // 1aa8
        for (index = 0; index != outPos_f; index++)
        {
            buffer_f[index] = fillChar;
        }
    }

    // 1ac0
    if (negative)
    {
        buffer_f[outPos_f++] = '-';
        digits_l--;
    }

    // 1ad3: the quotient assigned inside the digit expression (negative read once less)
    // lets MSC keep index in SI and outPos_f in DI, with negative in memory.
    do
    {
        index = D_5402_powersOfTen[digits_l - 1];
        buffer_f[outPos_f++] = (char)(negative = value / index) + '0';
        value -= index * negative;
    } while (--digits_l != 0);

    // 1afb
    buffer_f[outPos_f] = '\0';

    ULTIMA_1850_PrintString(buffer_f);
}
