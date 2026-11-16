; A2192.ASM: ULTIMA.EXE CS:2192-2320, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A2192
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

	EXTRN	ULTIMA_20C8_SOMEDELAY:NEAR
	EXTRN	D_5356:WORD
	EXTRN	D_A9CE:BYTE

_TEXT	SEGMENT

	PUBLIC	ULTIMA_2192_AUDIOPULSE
ULTIMA_2192_AUDIOPULSE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,WORD PTR D_5356
	cmp ax,64h
	jge	L_21a4
	xor ax,ax
	je	L_21a9
L_21a4:
	mov cl,18h
	div cl
	cbw
L_21a9:
	mov ds:[5454h],ax
	mov ax,3ch
	out 42h,al
	mov al,ah
	out 42h,al
	mov bx,[bp+6]
	mov cx,[bp+8]
	xor dx,dx
	cmp BYTE PTR D_A9CE,0
	jne	L_21f1
L_21c4:
	mov ax,[bp+0ch]
	add dx,ax
	push ax
	push cx
	add bx,[bp+4]
	mov ax,ds:[5454h]
	or ax,ax
	je	L_21eb
	mov cx,[bp+0ah]
	mov ds:[5450h],cx
L_21dc:
	mov ds:[5452h],ax
L_21df:
	dec word ptr ds:[5452h]
	jne	L_21df
	dec word ptr ds:[5450h]
	jne	L_21dc
L_21eb:
	pop cx
	pop ax
	loop	L_21c4
	jcxz	L_2234
L_21f1:
	mov ax,[bp+0ch]
	add dx,ax
	push ax
	push cx
	cmp dx,bx
	ja	L_2204
	in al,61h
	and al,0fch
	out 61h,al
	jmp	SHORT L_220a
L_2204:
	in al,61h
	or al,3
	out 61h,al
L_220a:
	add bx,[bp+4]
	mov ax,ds:[5454h]
	or ax,ax
	je	L_222a
	mov cx,[bp+0ah]
	mov ds:[5450h],cx
L_221b:
	mov ds:[5452h],ax
L_221e:
	dec word ptr ds:[5452h]
	jne	L_221e
	dec word ptr ds:[5450h]
	jne	L_221b
L_222a:
	pop cx
	pop ax
	loop	L_21f1
	in al,61h
	and al,0fch
	out 61h,al
L_2234:
	pop ds
	pop di
	pop si
	pop bp
	ret 0ah
	db	000h

	PUBLIC	ULTIMA_223C_AUDIOWHITENOISE
ULTIMA_223C_AUDIOWHITENOISE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov word ptr ds:[5456h],0
	cmp BYTE PTR D_A9CE,0
	je	L_2255
	in al,61h
	or al,3
	out 61h,al
L_2255:
	mov ax,ds:[545ch]
	add ax,9248h
	mov cl,3
	ror ax,cl
	xor ax,9248h
	add ax,11h
	mov ds:[545ch],ax
	mov bx,64h
	mov cx,[bp+4]
	sub cx,bx
	inc cx
	xor dx,dx
	div cx
	add dx,bx
	mov ax,dx
	mov cx,ax
	mov dx,12h
	mov ax,34deh
	div cx
	out 42h,al
	mov al,ah
	out 42h,al
	mov ax,WORD PTR D_5356
	mov cl,4
	shr ax,cl
	mov cx,[bp+8]
	add ds:[5456h],cx
	mov ds:[5458h],cx
L_229b:
	mov ds:[545ah],ax
L_229e:
	dec word ptr ds:[545ah]
	jne	L_229e
	dec word ptr ds:[5458h]
	jne	L_229b
	mov ax,ds:[5456h]
	cmp ax,[bp+6]
	jl	L_2255
	in al,61h
	and al,0fch
	out 61h,al
	pop ds
	pop di
	pop si
	pop bp
	ret 6
	db	000h

	PUBLIC	ULTIMA_22C0_AUDIOTONE
ULTIMA_22C0_AUDIOTONE:
	push bp
	mov bp,sp
	cmp BYTE PTR D_A9CE,0
	je	L_22d0
	push word ptr [bp+6]
	call	ULTIMA_22E2_PCSPKON
L_22d0:
	mov ax,1
	push ax
	push word ptr [bp+4]
	call	ULTIMA_20C8_SOMEDELAY
	call	ULTIMA_230E_PCSPKOFF
	pop bp
	ret 4
	db	090h

	PUBLIC	ULTIMA_22E2_PCSPKON
ULTIMA_22E2_PCSPKON:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	cmp WORD PTR D_A9CE,0
	je	L_2306
	mov cx,[bp+4]
	mov dx,12h
	mov ax,34deh
	div cx
	out 42h,al
	mov al,ah
	out 42h,al
	in al,61h
	or al,3
	out 61h,al
L_2306:
	pop ds
	pop di
	pop si
	pop bp
	ret 2
	db	000h

	PUBLIC	ULTIMA_230E_PCSPKOFF
ULTIMA_230E_PCSPKOFF:
	in al,61h
	and al,0fch
	out 61h,al
	ret
	db	000h, 055h, 08bh, 0ech, 083h, 0ech, 006h, 08bh
	db	0e5h, 05dh, 0c3h

_TEXT	ENDS
	END
