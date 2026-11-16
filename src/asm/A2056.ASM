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
	db	000h, 055h, 08bh, 0ech, 056h, 057h, 01eh, 0a1h
	db	020h, 054h, 005h, 048h, 092h, 0d1h, 0c8h, 0d1h
	db	0c8h, 0d1h, 0c8h, 035h, 048h, 092h, 005h, 011h
	db	000h, 0a3h, 020h, 054h, 025h, 0ffh, 07fh, 08bh
	db	05eh, 006h, 08bh, 04eh, 004h, 02bh, 0cbh, 041h
	db	033h, 0d2h, 0f7h, 0f1h, 003h, 0d3h, 08bh, 0c2h
	db	01fh, 05fh, 05eh, 05dh, 0c2h, 004h, 000h

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
	db	0fbh, 01eh, 050h, 056h, 0b8h, 064h, 00fh, 08eh
	db	0d8h, 08dh, 036h, 048h, 054h, 0ffh, 004h, 05eh
	db	058h, 01fh, 0cfh

_TEXT	ENDS
	END
