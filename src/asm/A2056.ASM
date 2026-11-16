; A2056.ASM: ULTIMA.EXE CS:2056-216c, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A2056
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

	EXTRN	D_1F7E:BYTE
	EXTRN	D_5356:WORD
	EXTRN	D_5420:WORD
	EXTRN	D_5422:WORD
	EXTRN	D_5424:WORD
	EXTRN	D_5448:WORD
	EXTRN	D_544A:WORD

_TEXT	SEGMENT

	PUBLIC	ULTIMA_2056_GETTIME
ULTIMA_2056_GETTIME:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ah,2ch
	int 21h
	xor ax,ax
	shl ch,1
	shl cl,1
	shl cl,1
	shl dh,1
	shl dh,1
	shl dh,1
	add ax,dx
	add ax,cx
	xor ax,91ebh
	and ax,0fffh
	pop ds
	pop di
	pop si
	pop bp
	ret
	db	000h

	PUBLIC	ULTIMA_207E_SRAND
ULTIMA_207E_SRAND:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+4]
	mov WORD PTR D_5420,ax
	pop ds
	pop di
	pop si
	pop bp
	ret 2
	db	000h

	PUBLIC	ULTIMA_2092_RANDOMRANGE
ULTIMA_2092_RANDOMRANGE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,WORD PTR D_5420
	add ax,9248h
	ror ax,1
	ror ax,1
	ror ax,1
	xor ax,9248h
	add ax,11h
	mov WORD PTR D_5420,ax
	and ax,7fffh
	mov bx,[bp+6]
	mov cx,[bp+4]
	sub cx,bx
	inc cx
	xor dx,dx
	div cx
	add dx,bx
	mov ax,dx
	pop ds
	pop di
	pop si
	pop bp
	ret 4

	PUBLIC	ULTIMA_20C8_SOMEDELAY
ULTIMA_20C8_SOMEDELAY:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,WORD PTR D_5356
	mov bx,[bp+6]
	mov cx,[bx+5426h]
	jcxz	L_20dc
	shr ax,cl
L_20dc:
	mov cx,[bp+4]
	mov WORD PTR D_5424,cx
L_20e3:
	mov WORD PTR D_5422,ax
L_20e6:
	dec WORD PTR D_5422
	jne	L_20e6
	dec WORD PTR D_5424
	jne	L_20e3
	pop ds
	pop di
	pop si
	pop bp
	ret 4
	db	000h

	PUBLIC	ULTIMA_20FA_WAITTICKS
ULTIMA_20FA_WAITTICKS:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+4]
	cmp ax,1
	jne	L_2110
	cmp WORD PTR D_5356,0f0h
	jle	L_2152
L_2110:
	mov WORD PTR D_544A,ax
	mov WORD PTR D_5448,0
	push es
	mov al,1ch
	mov ah,35h
	int 21h
	mov ax,es
	pop es
	mov ds:[544eh],bx
	mov ds:[544ch],ax
	push ds
	push cs
	pop ds
	lea dx,D_1F7E+475
	mov al,1ch
	mov ah,25h
	int 21h
	pop ds
L_2138:
	mov ax,WORD PTR D_5448
	cmp ax,WORD PTR D_544A
	jb	L_2138
	mov ax,ds:[544ch]
	mov dx,ds:[544eh]
	push ds
	mov ds,ax
	mov al,1ch
	mov ah,25h
	int 21h
	pop ds
L_2152:
	pop ds
	pop di
	pop si
	pop bp
	ret 2
L_2159:
	sti
	push ds
	push ax
	push si
	mov	ax,DGROUP
	mov ds,ax
	lea si,D_5448
	inc word ptr [si]
	pop si
	pop ax
	pop ds
	iret

_TEXT	ENDS
	END
