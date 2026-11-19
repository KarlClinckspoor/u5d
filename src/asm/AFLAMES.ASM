; AFLAMES.ASM: FLAMES.OVL 0000-0011, hand-written assembly in the original.
; All of FLAMES.OVL. FLAMES_0000_RETURNADDRESS returns, in AX, its own return address from FLAMES_000E_POPCALLER:
; an address in overlay area 2, which SAVEVIDEOPAGE (AFONT.ASM) uses as a scratch buffer.
; Written by hand from the original bytes; tools/asmbuild.py checks it.

	TITLE	AFLAMES
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

	PUBLIC	FLAMES_0000_RETURNADDRESS
FLAMES_0000_RETURNADDRESS:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	call FLAMES_000E_POPCALLER
	pop ds
	pop di
	pop si
	pop bp
	ret

	PUBLIC	FLAMES_000E_POPCALLER
FLAMES_000E_POPCALLER:
	pop ax			; AX = the return address
	push ax
	ret

_TEXT	ENDS
	END
