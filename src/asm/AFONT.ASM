; AFONT.ASM: FONT.OVL 0e52-0e97, hand-written assembly in the original.
; The last two routines of FONT.OVL, linked after font.c. FONT_04a4 ("view") loads a
; different tile set; around that, these save the first 0x3000 bytes of the video page
; buffer (segment D_5354_PAGESEGMENT) into the code segment, and copy them back. The place is overlay
; area 2: D_520C is FLAMES_0000's return address there (see AFLAMES.ASM), taken on first use.
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
	EXTRN	FLAMES_0000:NEAR

; this module's own data, after font.c's in DGROUP (DS:520c)
_DATA	SEGMENT
	PUBLIC	D_520C
D_520C	dw	0
_DATA	ENDS

_TEXT	SEGMENT

; save: D_5354_PAGESEGMENT:0000 (0x1800 words) to CS:[D_520C]
	PUBLIC	FONT_0E52
FONT_0E52:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	cmp D_520C,0
	jne L_0e65
	call FLAMES_0000
	mov D_520C,ax
L_0e65:
	mov di,D_520C
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

; restore: CS:[D_520C] to D_5354_PAGESEGMENT:0000
	PUBLIC	FONT_0E7B
FONT_0E7B:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,D_520C
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
