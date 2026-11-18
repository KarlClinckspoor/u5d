; AFONT.ASM: FONT.OVL 0e52-0e97, hand-written assembly in the original.
; The last two routines of FONT.OVL, linked after font.c. FONT_04A4_PLAYINTROSTORY ("view") loads a
; different tile set; around that, these save the first 0x3000 bytes of the video page
; buffer (segment D_5354_PAGESEGMENT) into the code segment, and copy them back. The place is overlay
; area 2: D_520C_SAVEDVIDEOPAGE is FLAMES_0000_RETURNADDRESS's return address there (see AFLAMES.ASM), taken on first use.
; Written by hand from the original bytes; tools/asmbuild.py checks it.

	TITLE	AFONT
_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST, _BSS, _DATA
	ASSUME	CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP

	EXTRN	D_5354_PAGESEGMENT:WORD
	EXTRN	FLAMES_0000_RETURNADDRESS:NEAR

; this module's own data, after font.c's in DGROUP (DS:520c)
_DATA	SEGMENT
	PUBLIC	D_520C_SAVEDVIDEOPAGE
D_520C_SAVEDVIDEOPAGE	dw	0
_DATA	ENDS

_TEXT	SEGMENT

; save: D_5354_PAGESEGMENT:0000 (0x1800 words) to CS:[D_520C_SAVEDVIDEOPAGE]
	PUBLIC	FONT_0E52_SAVEVIDEOPAGE
FONT_0E52_SAVEVIDEOPAGE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	cmp D_520C_SAVEDVIDEOPAGE,0
	jne L_0e65
	call FLAMES_0000_RETURNADDRESS
	mov D_520C_SAVEDVIDEOPAGE,ax
L_0e65:
	mov di,D_520C_SAVEDVIDEOPAGE
	xor si,si
	mov ds,D_5354_PAGESEGMENT
	push cs
	pop es
	mov cx,1800h
	rep movsw
	pop ds
	pop di
	pop si
	pop bp
	ret

; restore: CS:[D_520C_SAVEDVIDEOPAGE] to D_5354_PAGESEGMENT:0000
	PUBLIC	FONT_0E7B_RESTOREVIDEOPAGE
FONT_0E7B_RESTOREVIDEOPAGE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,D_520C_SAVEDVIDEOPAGE
	xor di,di
	mov es,D_5354_PAGESEGMENT
	push cs
	pop ds
	mov cx,1800h
	rep movsw
	pop ds
	pop di
	pop si
	pop bp
	ret

_TEXT	ENDS
	END
