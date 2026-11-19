; A71AA.ASM: ULTIMA.EXE CS:71aa-72e0, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A71AA
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

	EXTRN	CLIPRECTCOORD:NEAR
	EXTRN	DRIVERENTRY:WORD
	EXTRN	DOSERROR:WORD
	EXTRN	D_587F_HOUR:BYTE
	EXTRN	MINUTE:BYTE

_TEXT	SEGMENT

	PUBLIC	UPDATECLOCKTILES
UPDATECLOCKTILES:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,2
	mov bl,BYTE PTR D_587F_HOUR
	mov bh,BYTE PTR MINUTE
	mov WORD PTR DRIVERENTRY,6ch
	call DWORD PTR DRIVERENTRY
	pop ds
	pop di
	pop si
	pop bp
	ret

	PUBLIC	DRV_27_SCROLLTEXTWINDOW1
DRV_27_SCROLLTEXTWINDOW1:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+0ch]
	mov bx,[bp+0ah]
	mov cx,[bp+8]
	mov dx,[bp+6]
	mov si,[bp+4]
	call	CLIPRECTCOORD
	mov di,dx
	sub di,bx
	cmp si,di
	jle	L_71ec
	mov si,di
L_71ec:
	neg si
	mov WORD PTR DRIVERENTRY,27h
	call DWORD PTR DRIVERENTRY
	pop ds
	pop di
	pop si
	pop bp
	ret 0ah
	db	000h

	PUBLIC	DRV_27_SCROLLTEXTWINDOW2
DRV_27_SCROLLTEXTWINDOW2:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+0ch]
	mov bx,[bp+0ah]
	mov cx,[bp+8]
	mov dx,[bp+6]
	mov si,[bp+4]
	call	CLIPRECTCOORD
	mov di,dx
	sub di,bx
	cmp si,di
	jle	L_7222
	mov si,di
L_7222:
	mov WORD PTR DRIVERENTRY,27h
	call DWORD PTR DRIVERENTRY
	pop ds
	pop di
	pop si
	pop bp
	ret 0ah
	db	000h

	PUBLIC	READFILE
READFILE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov WORD PTR DOSERROR,0
	mov dx,[bp+0ah]
	mov al,2
	mov ah,3dh
	int 21h
	mov bx,ax
	jb	L_7286
	cmp WORD PTR DOSERROR,0
	jne	L_7286
	mov dx,[bp+4]
	xor cx,cx
	or dx,dx
	je	L_7265
	xor al,al
	mov ah,42h
	int 21h
	jb	L_7280
L_7265:
	mov cx,[bp+6]
	or cx,cx
	jne	L_726f
	mov cx,0ffffh
L_726f:
	mov dx,[bp+8]
	mov ah,3fh
	int 21h
	jb	L_7280
	push ax
	mov ah,3eh
	int 21h
	pop ax
	jmp	SHORT L_728f
L_7280:
	push ax
	mov ah,3eh
	int 21h
	pop ax
L_7286:
	mov WORD PTR DOSERROR,ax
	call dword ptr ds:[5394h]
	xor ax,ax
L_728f:
	pop ds
	pop di
	pop si
	pop bp
	ret 8

	PUBLIC	WRITEFILE
WRITEFILE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov dx,[bp+8]
	xor cx,cx
	mov ah,3ch
	int 21h
	mov bx,ax
	jb	L_72c5
	mov cx,[bp+4]
	mov dx,[bp+6]
	mov ah,40h
	int 21h
	push ax
	pushf
	mov ah,3eh
	int 21h
	popf
	pop ax
	jb	L_72c5
	mov WORD PTR DOSERROR,0
	jae	L_72ce
L_72c5:
	mov WORD PTR DOSERROR,ax
	call dword ptr ds:[5394h]
	xor ax,ax
L_72ce:
	pop ds
	pop di
	pop si
	pop bp
	ret 6
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h

_TEXT	ENDS
	END
