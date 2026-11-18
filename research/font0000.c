// research/font0000.c: FONT_0000_DrawJustifiedText, the unmatched C reconstruction (670 of 674
// bytes against the original, MSC 5.0). The build takes the function from src/asm/AFONT0.ASM in
// both targets; this C is kept for whoever works on it next. To try it: paste it into
// src/font.c in place of the note that names AFONT0.ASM and use tools/variants.py (font.c is
// FONT.OVL's 5.0 object; MATCHING.md, "What pass 2 counts, measured", has the latest leads).
// Moved here from src/font.c when the native port was removed (it was compiled only there).
// Names as of the cleanup; the globals it uses are declared in src/vars.h.

// Prints text justified in the proportional font: the glyph image and the text; the margins and the cursor
// are the globals D_5146_textLeftX..D_5158_penY (two regions, so that text can run beside a picture). Assembly in the DOS
// build.
// CLEAN ROOM (written from the disassembly). Prints justified text: glyphs the glyph
// image, text the text. Per line: loop A measures up to the line width, loop B backs up
// to the last space (or a "_" hyphenation point), loop C draws, spreading the slack over
// the spaces; then the next line's margins.
// NOT MATCHING: 56 bytes differ, length 670 against 674 (before: 513 bytes, 676/674).
// What gives the original's code so far:
// - the outer loop is a for with an increment expression. With "while" or "for (; c; )"
//   the three inner loops get no registers; "for (;;) { if (!c) break; ..." gives them but
//   leaves loop A's register loads at the top of the body. With an increment the test sits
//   at the bottom with loop A's loads right after it, as in the original. The increment
//   here is "0" (no code); the original's is not known ("lineStart++" would be the natural
//   one, but the original's test at the end of a line jumps past the increment to the
//   loop test, which no continue or break does: hence the goto to L_next);
// - loop B's "--i" is in its condition (the first break is then the space case, and the
//   code after the loop follows it);
// - loop C is "if (i < n) do ... while (n != ++i)";
// - "if (text[start] == 0) goto L_next;" and not an if around the rest (that one keeps
//   text[start] in AL for the "_" test).
// Still different:
// - the frame is 1Ah, the original's 1Ch: at 025f the original keeps region * 2 in BX
//   with a dead store at bp-1Ch (shl ax,1; mov [bp-1Ch],ax; mov bx,ax), where we get
//   mov si,ax; shl si,1 as at the top of the function. Tried: the condition written out at
//   each use, assigned inside the index, D_5146_textLeftX[region] read twice;
// - 0173: the original adds D_50f7_hyphenWidth's temporary first (mov ax,[bp-14h]; add ax,si), we
//   mov ax,si; add ax,[bp-14h]. Tried: the operands in every order, the cast on either
//   side of the comparison.
void FONT_0000_DrawJustifiedText(byte* glyphs, byte* text)
{
    int region;
    int scan;
    int lineStart;
    int lineWidth;
    int spaces;
    int slack;
    int lineEnd;
    int used;

    ULTIMA_0c22_GRAP_0f_SelectPage(1);
    region = (int)D_5158_penY > (int)D_5150_regionTop && (int)D_5158_penY < (int)D_5152_regionBottom;
    lineWidth = D_514c_textRightX[region] - D_5146_textLeftX[region];
    lineStart = lineEnd = 0;
    used = D_5156_penX - D_5146_textLeftX[region];

    for (; text[lineStart] != 0; 0)
    {
        spaces = 0;
        for (scan = lineStart; text[scan] != 0 && lineWidth > used && text[scan] != '\n'; scan++)
        {
            if (text[scan] <= ' ')
            {
                used += D_5154_spaceWidth;
                spaces++;
            }
            else if (text[scan] == '{')
            {
                used += 0xf;
            }
            else if (text[scan] != '_')
            {
                used += D_50ea_glyphWidths[text[scan] - 0x20] + 1;
            }
        }

        while ((text[scan] != 0 || used >= lineWidth) && text[scan] != '\n' && lineStart < --scan)
        {
            if (text[scan] == ' ')
            {
                used -= D_5154_spaceWidth;
                spaces--;
                break;
            }
            if (text[scan] == '_' && (uint)(used + D_50f7_hyphenWidth + 1) < lineWidth)
            {
                used += D_50f7_hyphenWidth + 1;
                break;
            }
            if (text[scan] != '_' && text[scan] != '{')
            {
                used -= D_50ea_glyphWidths[text[scan] - 0x20] + 1;
            }
        }

        slack = lineWidth - used;
        lineEnd = scan;

        scan = lineStart;
        if (scan < lineEnd)
        do
        {
            if (text[scan] <= ' ')
            {
                D_5156_penX += D_5154_spaceWidth;
                if (spaces != 0 && text[lineEnd] != 0 && text[lineEnd] != '\n')
                {
                    D_5156_penX += slack / spaces;
                    slack -= slack / spaces;
                    spaces--;
                }
            }
            else if (text[scan] == '{')
            {
                D_5156_penX += 0xf;
            }
            else if (text[scan] != '_')
            {
                if ((int)D_5158_penY < 0xc0)
                {
                    ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(glyphs, text[scan] - 0x20, D_5156_penX, D_5158_penY);
                }
                D_5156_penX += D_50ea_glyphWidths[text[scan] - 0x20] + 1;
            }
        } while (lineEnd != ++scan);

        lineStart = lineEnd;
        if (text[lineStart] == 0)
        {
            goto L_next;
        }
        {
            if (text[lineStart] == '_' && (int)D_5158_penY < 0xc0)
            {
                ULTIMA_1044_GRAP_4e_CopyBitImageIntoPage(glyphs, 0xd, D_5156_penX, D_5158_penY);
            }
            used = 0;
            D_5158_penY += 9;
            region = (int)D_5158_penY > (int)D_5150_regionTop && (int)D_5158_penY < (int)D_5152_regionBottom;
            D_5156_penX = D_5146_textLeftX[region];
            lineWidth = D_514c_textRightX[region] - D_5156_penX;
            lineStart++;
        }
L_next:;
    }

    ULTIMA_0c22_GRAP_0f_SelectPage(0);
}
