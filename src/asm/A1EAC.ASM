; A1EAC.ASM: ULTIMA.EXE CS:1eac-1fa0, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.
; The two releases differ in one byte (1f0b, IFDEF RELEASE_1988 below): the highest drive
; letter ULTIMA_1EF7 accepts is E in 1988 and Z in the January 1992 patch (the GOG release).

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

	EXTRN	D_5350_DRIVERENTRY:WORD
	EXTRN	D_535C_DOSERROR:WORD
	EXTRN	D_539A_CURRENTTEXTWINDOW:WORD
	EXTRN	D_53AB_TEXTBGCOLOR:BYTE

; this module's own data in DGROUP (DS:541e-5420)
_DATA	SEGMENT
	PUBLIC	D_541E_DRIVENUMBER
D_541E_DRIVENUMBER	LABEL	BYTE
	db	000h, 000h
_DATA	ENDS

_TEXT	SEGMENT

	PUBLIC	ULTIMA_1EAC_SETDEFAULTDRIVE
ULTIMA_1EAC_SETDEFAULTDRIVE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov WORD PTR D_535C_DOSERROR,0
	mov ax,[bp+4]
	call	ULTIMA_1EF7_CONVERTDRIVELETTERT
	jb	L_1ee4
	mov BYTE PTR D_541E_DRIVENUMBER,dl
	mov ah,0eh
	int 21h
	cmp WORD PTR D_535C_DOSERROR,0
	jne	L_1ee4
	mov ah,19h
	int 21h
	cmp BYTE PTR D_541E_DRIVENUMBER,al
	jne	L_1ee4
	mov WORD PTR D_535C_DOSERROR,0
	mov ax,1
	jmp	SHORT L_1ef0
L_1ee4:
	mov WORD PTR D_535C_DOSERROR,1
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
IFDEF RELEASE_1988
	cmp al,45h	; 'E'
ELSE
	cmp al,5ah	; 'Z' (1992 patch)
ENDIF
	jg	L_1f10
	clc
	ret
L_1f10:
	stc
	ret

	PUBLIC	ULTIMA_1F12_GETCURRENTTEXTX
ULTIMA_1F12_GETCURRENTTEXTX:
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
	mov BYTE PTR D_53AB_TEXTBGCOLOR,al
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

	PUBLIC	ULTIMA_1F4E_DRV27OFFSET
ULTIMA_1F4E_DRV27OFFSET:
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
	mov WORD PTR D_5350_DRIVERENTRY,27h
	call DWORD PTR D_5350_DRIVERENTRY
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
