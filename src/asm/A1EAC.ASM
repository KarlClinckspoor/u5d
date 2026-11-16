; A1EAC.ASM: ULTIMA.EXE CS:1eac-1fa0, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A1EAC
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

	EXTRN	D_5350:WORD
	EXTRN	D_535C:WORD
	EXTRN	D_539A_CURRENTTEXTWINDOW:WORD
	EXTRN	D_53AB_TEXT_BG_COLOR:BYTE
	EXTRN	D_541E:BYTE

_TEXT	SEGMENT

	PUBLIC	ULTIMA_1EAC_SETDEFAULTDRIVE
ULTIMA_1EAC_SETDEFAULTDRIVE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov WORD PTR D_535C,0
	mov ax,[bp+4]
	call	ULTIMA_1EF7_CONVERTDRIVELETTERT
	jb	L_1ee4
	mov BYTE PTR D_541E,dl
	mov ah,0eh
	int 21h
	cmp WORD PTR D_535C,0
	jne	L_1ee4
	mov ah,19h
	int 21h
	cmp BYTE PTR D_541E,al
	jne	L_1ee4
	mov WORD PTR D_535C,0
	mov ax,1
	jmp	SHORT L_1ef0
L_1ee4:
	mov WORD PTR D_535C,1
	call dword ptr ds:[5394h]
	xor ax,ax
L_1ef0:
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	ULTIMA_1EF7_CONVERTDRIVELETTERT
ULTIMA_1EF7_CONVERTDRIVELETTERT:
	cmp al,61h
	jl	L_1f01
	cmp al,7ah
	jg	L_1f01
	sub al,20h
L_1f01:
	mov dl,al
	sub dl,41h
	cmp al,41h
	jl	L_1f10
	cmp al,5ah
	jg	L_1f10
	clc
	ret
L_1f10:
	stc
	ret
L_1f12:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,WORD PTR D_539A_CURRENTTEXTWINDOW
	mov al,[si+4]
	xor ah,ah
	pop ds
	pop di
	pop si
	pop bp
	ret

	PUBLIC	ULTIMA_1F26_SETTEXTBACKGROUNDCO
ULTIMA_1F26_SETTEXTBACKGROUNDCO:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,WORD PTR D_539A_CURRENTTEXTWINDOW
	mov ax,[bp+4]
	and al,0fh
	mov BYTE PTR D_53AB_TEXT_BG_COLOR,al
	mov cl,4
	shl al,cl
	mov ah,[si+6]
	and ah,0fh
	or ah,al
	mov [si+6],ah
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	ULTIMA_1F4E
ULTIMA_1F4E:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,WORD PTR D_539A_CURRENTTEXTWINDOW
	call	ULTIMA_1F77_CONVERTCHARCOORDTOP
	mov si,[bp+4]
	shl si,1
	shl si,1
	shl si,1
	neg si
	mov WORD PTR D_5350,27h
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	ULTIMA_1F77_CONVERTCHARCOORDTOP
ULTIMA_1F77_CONVERTCHARCOORDTOP:
	xor ah,ah
	mov bh,ah
	mov ch,ah
	mov dh,ah
	mov al,[si]
	mov bl,[si+1]
	mov cl,[si+2]
	mov dl,[si+3]
	mov di,cx
	mov cl,3
	shl ax,cl
	shl bx,cl
	shl di,cl
	shl dx,cl
	add di,7
	add dx,7
	mov cx,di
	ret
	db	000h

_TEXT	ENDS
	END
