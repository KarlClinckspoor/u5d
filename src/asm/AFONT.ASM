; AFONT.ASM: FONT.OVL 0e52-0e97, hand-written assembly in the original.
; The last two routines of FONT.OVL, linked after font.c. PLAYINTROSTORY ("view") loads a
; different tile set; around that, these save the first 0x3000 bytes of the video page
; buffer (segment PAGESEGMENT) into the code segment, and copy them back. The place is overlay
; area 2: SAVEDVIDEOPAGE is FLAMES_0000_RETURNADDRESS's return address there (see AFLAMES.ASM), taken on first use.
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

	EXTRN	PAGESEGMENT:WORD
	EXTRN	FLAMES_0000_RETURNADDRESS:NEAR

; this module's own data, after font.c's in DGROUP (DS:520c)
_DATA	SEGMENT
	PUBLIC	SAVEDVIDEOPAGE
SAVEDVIDEOPAGE	dw	0
_DATA	ENDS

_TEXT	SEGMENT

; save: PAGESEGMENT:0000 (0x1800 words) to CS:[SAVEDVIDEOPAGE]
	PUBLIC	SAVEVIDEOPAGE
SAVEVIDEOPAGE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	cmp SAVEDVIDEOPAGE,0
	jne L_0e65
	call FLAMES_0000_RETURNADDRESS
	mov SAVEDVIDEOPAGE,ax
L_0e65:
	mov di,SAVEDVIDEOPAGE
	xor si,si
	mov ds,PAGESEGMENT
	push cs
	pop es
	mov cx,1800h
	rep movsw
	pop ds
	pop di
	pop si
	pop bp
	ret

; restore: CS:[SAVEDVIDEOPAGE] to PAGESEGMENT:0000
	PUBLIC	RESTOREVIDEOPAGE
RESTOREVIDEOPAGE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,SAVEDVIDEOPAGE
	xor di,di
	mov es,PAGESEGMENT
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
