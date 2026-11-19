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

	EXTRN	ULTIMA_20C8_DELAY:NEAR
	EXTRN	TIMERCALIBRATION:WORD
	EXTRN	D_A9CE_SOUNDON:BYTE

; this module's own data in DGROUP (DS:5450-545e)
_DATA	SEGMENT
	PUBLIC	D_5450
D_5450	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_5452
D_5452	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_5454
D_5454	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_5456
D_5456	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_5458
D_5458	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_545A
D_545A	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_545C
D_545C	LABEL	WORD
	db	064h, 076h
_DATA	ENDS

_TEXT	SEGMENT

	PUBLIC	ULTIMA_2192_AUDIOPULSE
ULTIMA_2192_AUDIOPULSE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,WORD PTR TIMERCALIBRATION
	cmp ax,64h
	jge	L_21a4
	xor ax,ax
	je	L_21a9
L_21a4:
	mov cl,18h
	div cl
	cbw
L_21a9:
	mov D_5454,ax
	mov ax,3ch
	out 42h,al
	mov al,ah
	out 42h,al
	mov bx,[bp+6]
	mov cx,[bp+8]
	xor dx,dx
	cmp BYTE PTR D_A9CE_SOUNDON,0
	jne	L_21f1
L_21c4:
	mov ax,[bp+0ch]
	add dx,ax
	push ax
	push cx
	add bx,[bp+4]
	mov ax,D_5454
	or ax,ax
	je	L_21eb
	mov cx,[bp+0ah]
	mov D_5450,cx
L_21dc:
	mov D_5452,ax
L_21df:
	dec word ptr D_5452
	jne	L_21df
	dec word ptr D_5450
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
	mov ax,D_5454
	or ax,ax
	je	L_222a
	mov cx,[bp+0ah]
	mov D_5450,cx
L_221b:
	mov D_5452,ax
L_221e:
	dec word ptr D_5452
	jne	L_221e
	dec word ptr D_5450
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
	mov word ptr D_5456,0
	cmp BYTE PTR D_A9CE_SOUNDON,0
	je	L_2255
	in al,61h
	or al,3
	out 61h,al
L_2255:
	mov ax,D_545C
	add ax,9248h
	mov cl,3
	ror ax,cl
	xor ax,9248h
	add ax,11h
	mov D_545C,ax
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
	mov ax,WORD PTR TIMERCALIBRATION
	mov cl,4
	shr ax,cl
	mov cx,[bp+8]
	add D_5456,cx
	mov D_5458,cx
L_229b:
	mov D_545A,ax
L_229e:
	dec word ptr D_545A
	jne	L_229e
	dec word ptr D_5458
	jne	L_229b
	mov ax,D_5456
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
	cmp BYTE PTR D_A9CE_SOUNDON,0
	je	L_22d0
	push word ptr [bp+6]
	call	ULTIMA_22E2_PCSPKON
L_22d0:
	mov ax,1
	push ax
	push word ptr [bp+4]
	call	ULTIMA_20C8_DELAY
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
	cmp WORD PTR D_A9CE_SOUNDON,0
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
L_2315:
	add [di-75h],dl
	in al,dx
	sub sp,6
	mov sp,bp
	pop bp
	ret

_TEXT	ENDS
	END
