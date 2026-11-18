; A6F1E.ASM: ULTIMA.EXE CS:6f1e-7040, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A6F1E
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

	EXTRN	D_5350_DRIVERENTRY:WORD
	EXTRN	D_539C:BYTE

; this module's own data, after 5000.c's in DGROUP (DS:6aa4)
_DATA	SEGMENT
	PUBLIC	D_6AA4
D_6AA4	dw	0
D_6AA6	dw	0
	PUBLIC	D_6AA8
D_6AA8	db	032h, 029h, 022h, 01Dh, 01Ah, 019h
	db	029h, 020h, 019h, 014h, 011h, 010h
	db	022h, 019h, 012h, 00Dh, 00Ah, 009h
	db	01Dh, 014h, 00Dh, 008h, 005h, 004h
	db	01Ah, 011h, 00Ah, 005h, 002h, 001h
	db	019h, 010h, 009h, 004h, 001h, 000h
_DATA	ENDS

_TEXT	SEGMENT

	PUBLIC	ULTIMA_6F1E_COMPAREWORD
ULTIMA_6F1E_COMPAREWORD:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	cld
	push ds
	pop es
	mov di,[bp+6]
	call	ULTIMA_6F90_STRLENDI
	mov WORD PTR D_6AA4,di
	mov di,[bp+4]
	call	ULTIMA_6F90_STRLENDI
	cmp WORD PTR D_6AA4,di
	jg	L_6f84
	sub di,WORD PTR D_6AA4
	mov D_6AA6,di
	xor bx,bx
L_6f47:
	mov cx,WORD PTR D_6AA4
	mov si,[bp+6]
	mov di,[bp+4]
	add di,bx
L_6f53:
	lodsb
	or si,si
	je	L_6f87
	mov ah,es:[di]
	inc di
	and ax,7f7fh
	cmp ah,60h
	jle	L_6f67
	and ah,5fh
L_6f67:
	cmp al,60h
	jle	L_6f6d
	and al,5fh
L_6f6d:
	cmp ah,al
	jne	L_6f75
	loop	L_6f53
	jmp	SHORT L_6f87
L_6f75:
	mov dx,WORD PTR D_6AA4
	sub dx,cx
	add bx,dx
	inc bx
	cmp bx,D_6AA6
	jle	L_6f47
L_6f84:
	mov bx,0ffffh
L_6f87:
	mov ax,bx
	pop ds
	pop di
	pop si
	pop bp
	ret 4

	PUBLIC	ULTIMA_6F90_STRLENDI
ULTIMA_6F90_STRLENDI:
	xor al,al
	mov cx,0ffffh
	mov bx,di
	repne scasb
	sub di,bx
	dec di
	ret
	db	000h

	PUBLIC	ULTIMA_6F9E_TILESETMODEPLUS3
ULTIMA_6F9E_TILESETMODEPLUS3:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+4]
	add ax,3
	mov WORD PTR D_5350_DRIVERENTRY,6ch
	call DWORD PTR D_5350_DRIVERENTRY
	pop ds
	pop di
	pop si
	pop bp
	ret 2
	db	000h

	PUBLIC	ULTIMA_6FBC_TILESETMODE
ULTIMA_6FBC_TILESETMODE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+4]
	mov WORD PTR D_5350_DRIVERENTRY,6ch
	call DWORD PTR D_5350_DRIVERENTRY
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	ULTIMA_6FD6_GRAP_ANIMATETILES
ULTIMA_6FD6_GRAP_ANIMATETILES:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	clc
	mov ax,WORD PTR D_539C
	mov WORD PTR D_5350_DRIVERENTRY,60h
	call DWORD PTR D_5350_DRIVERENTRY
	pop ds
	pop di
	pop si
	pop bp
	ret
	db	000h

	PUBLIC	ULTIMA_6FF0_DISTANCEFROMCENTER
ULTIMA_6FF0_DISTANCEFROMCENTER:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov bx,[bp+6]
	mov dx,[bp+4]
	cmp bx,0bh
	jge	L_703b
	cmp dx,0bh
	jge	L_703b
	cmp bx,5
	jg	L_7010
	cmp dx,5
	jle	L_7026
L_7010:
	cmp bx,5
	jle	L_701b
	mov ax,0ah
	xchg bx,ax
	sub bx,ax
L_701b:
	cmp dx,5
	jle	L_7026
	mov ax,0ah
	xchg dx,ax
	sub dx,ax
L_7026:
	shl dx,1
	add bx,dx
	shl dx,1
	add bx,dx
	mov al,D_6AA8[bx]
	xor ah,ah
L_7034:
	pop ds
	pop di
	pop si
	pop bp
	ret 4
L_703b:
	xor ax,ax
	jmp	SHORT L_7034
	db	000h

_TEXT	ENDS
	END
