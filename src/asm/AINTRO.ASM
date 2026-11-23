; AINTRO.ASM: INTRO.OVL 2090-20c7, hand-written assembly in the original.
; The last two routines of INTRO.OVL, linked after introb.c: calls of the video driver's
; function 69h ("Warriors of Destiny" animation) through the far pointer at DRIVERENTRY, the way
; A0878.ASM calls the driver. Carry clear: animate one step; carry set: AX is the image to
; show. INTRO_20AE returns with a plain RET, leaving its argument to the caller.
; Written by hand from the original bytes; tools/asmbuild.py checks it.

	TITLE	AINTRO
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

	EXTRN	DRIVERENTRY:WORD
	EXTRN	WAITTICKS:NEAR

_TEXT	SEGMENT

	PUBLIC	ANIMATEWD
ANIMATEWD:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,1
	push ax
	call WAITTICKS
	clc
	mov WORD PTR DRIVERENTRY,69h
	call DWORD PTR DRIVERENTRY
	pop ds
	pop di
	pop si
	pop bp
	ret

	db	0		; 20ad: a zero byte, as between two word-aligned objects

	PUBLIC	SHOWWD
SHOWWD:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+4]
	stc
	mov WORD PTR DRIVERENTRY,69h
	call DWORD PTR DRIVERENTRY
	pop ds
	pop di
	pop si
	pop bp
	ret

_TEXT	ENDS
	END
