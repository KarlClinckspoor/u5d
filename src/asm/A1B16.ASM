; A1B16.ASM: ULTIMA.EXE CS:1b16-1b38, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A1B16
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


_TEXT	SEGMENT

	PUBLIC	ULTIMA_1B16_CLEARKBDBUFFER
ULTIMA_1B16_CLEARKBDBUFFER:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	call	ULTIMA_1B24_CLEARKBDBUFFERIMPL
	pop ds
	pop di
	pop si
	pop bp
	ret

	PUBLIC	ULTIMA_1B24_CLEARKBDBUFFERIMPL
ULTIMA_1B24_CLEARKBDBUFFERIMPL:
	push ds
	mov dx,40h
	mov ds,dx
	mov word ptr ds:[1ah],1eh
	mov word ptr ds:[1ch],1eh
	pop ds
	ret

_TEXT	ENDS
	END
