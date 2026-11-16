; A0878.ASM: ULTIMA.EXE CS:0878-0d72, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A0878
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

	EXTRN	ULTIMA_125D_LOADRESOURCEFILEIMP:NEAR
	EXTRN	D_5350:WORD
	EXTRN	D_535C:WORD

; this module's own data in DGROUP (DS:52ba-5306)
_DATA	SEGMENT
	PUBLIC	D_52BA_VDP
D_52BA_VDP	LABEL	WORD
	db	000h, 000h, 008h, 000h, 008h, 000h, 010h, 000h, 010h, 000h
	PUBLIC	D_52C4
D_52C4	LABEL	WORD
	db	000h, 000h, 000h, 000h
	PUBLIC	D_52C8
D_52C8	LABEL	WORD
	db	000h, 000h, 000h, 000h
	PUBLIC	D_52CC
D_52CC	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52CE
D_52CE	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52D0
D_52D0	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52D2
D_52D2	LABEL	WORD
	db	03Fh, 001h
	PUBLIC	D_52D4
D_52D4	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52D6
D_52D6	LABEL	WORD
	db	0C7h, 000h, 000h, 000h
	PUBLIC	D_52DA_PEN_COLOR
D_52DA_PEN_COLOR	LABEL	WORD
	db	000h, 000h, 000h, 000h, 000h, 001h, 002h, 003h, 004h, 005h, 006h, 007h
	db	038h, 039h, 03Ah, 03Bh, 03Ch, 03Dh, 03Eh, 03Fh, 000h	; 89:;<=>?.
	PUBLIC	D_52EF_FORCEEGA
D_52EF_FORCEEGA	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52F1_FORCETANDY
D_52F1_FORCETANDY	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52F3_FORCEHERC
D_52F3_FORCEHERC	LABEL	WORD
	db	000h, 000h, 090h
	PUBLIC	D_52F6
D_52F6	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52F8
D_52F8	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52FA
D_52FA	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52FC
D_52FC	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_52FE
D_52FE	LABEL	WORD
	db	000h, 000h
	PUBLIC	D_5300
D_5300	LABEL	WORD
	db	000h, 000h, 000h, 000h
	PUBLIC	D_5304
D_5304	LABEL	WORD
	db	0FFh, 0FFh
_DATA	ENDS

_TEXT	SEGMENT

	PUBLIC	ULTIMA_0878_RESTOREVIDEOMODE
ULTIMA_0878_RESTOREVIDEOMODE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov bx,WORD PTR D_5304
	cmp bx,0ffffh
	je	L_088d
	mov ah,0
	mov al,bl
	int 10h
L_088d:
	pop ds
	pop di
	pop si
	pop bp
	ret

	PUBLIC	ULTIMA_0892_INITIALIZEVIDEODRIV
ULTIMA_0892_INITIALIZEVIDEODRIV:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	cmp word ptr ds:[5352h],0
	je	L_08c5
	mov ax,[bp+4]
	or ax,ax
	je	L_08b8
	cmp WORD PTR D_5304,0ffffh
	jne	L_08b8
	push ax
	mov ah,0fh
	int 10h
	xor ah,ah
	mov WORD PTR D_5304,ax
	pop ax
L_08b8:
	mov bx,OFFSET DGROUP:D_52BA_VDP
	mov WORD PTR D_5350,3
	call DWORD PTR D_5350
L_08c5:
	pop ds
	pop di
	pop si
	pop bp
	ret

	PUBLIC	ULTIMA_08CA_INSIDECLIPWINDOW
ULTIMA_08CA_INSIDECLIPWINDOW:
	cmp ax,D_52D0
	jl	L_08e4
	cmp ax,D_52D2
	jg	L_08e4
	cmp bx,D_52D4
	jl	L_08e4
	cmp bx,D_52D6
	jg	L_08e4
	clc
	ret
L_08e4:
	stc
	ret

	PUBLIC	ULTIMA_08E6_CLIPRECTCOORD
ULTIMA_08E6_CLIPRECTCOORD:
	cmp ax,cx
	jle	L_08eb
	xchg cx,ax
L_08eb:
	cmp bx,dx
	jle	L_08f1
	xchg bx,dx
L_08f1:
	cmp ax,0
	jge	L_08f9
	mov ax,0
L_08f9:
	cmp ax,13fh
	jle	L_0901
	mov ax,13fh
L_0901:
	cmp bx,0
	jge	L_0909
	mov bx,0
L_0909:
	cmp bx,0c7h
	jle	L_0912
	mov bx,0c7h
L_0912:
	cmp cx,0
	jge	L_091a
	mov cx,0
L_091a:
	cmp cx,13fh
	jle	L_0923
	mov cx,13fh
L_0923:
	cmp dx,0
	jge	L_092b
	mov dx,0
L_092b:
	cmp dx,0c7h
	jle	L_0934
	mov dx,0c7h
L_0934:
	ret

	PUBLIC	ULTIMA_0935_CLIPLINECOORD
ULTIMA_0935_CLIPLINECOORD:
	call	ULTIMA_0A22
	or si,si
	jne	L_0942
	or di,di
	jne	L_0942
	clc
	ret
L_0942:
	db	085h, 0f7h	; test di,si
	je	L_0948
	stc
	ret
L_0948:
	mov WORD PTR D_52FA,ax
	mov WORD PTR D_52FC,bx
	mov WORD PTR D_52FE,cx
	mov WORD PTR D_5300,dx
	push di
	or si,si
	je	L_0971
	xchg cx,ax
	xchg bx,dx
	call	ULTIMA_0991
	mov WORD PTR D_52FA,ax
	mov WORD PTR D_52FC,bx
	mov cx,WORD PTR D_52FE
	mov dx,WORD PTR D_5300
L_0971:
	pop di
	or di,di
	je	L_0980
	call	ULTIMA_0991
	mov WORD PTR D_52FE,ax
	mov WORD PTR D_5300,bx
L_0980:
	mov ax,WORD PTR D_52FA
	mov bx,WORD PTR D_52FC
	mov cx,WORD PTR D_52FE
	mov dx,WORD PTR D_5300
	clc
	ret

	PUBLIC	ULTIMA_0991
ULTIMA_0991:
	mov si,ax
	mov di,bx
	add ax,cx
	add bx,dx
	test ax,8000h
	js	L_09a2
	shr ax,1
	jmp	SHORT L_09ac
L_09a2:
	push cx
	push dx
	mov cx,2
	cwd
	idiv cx
	pop dx
	pop cx
L_09ac:
	test bx,8000h
	js	L_09b6
	shr bx,1
	jmp	SHORT L_09c2
L_09b6:
	push cx
	push dx
	mov cx,2
	xchg bx,ax
	cwd
	idiv cx
	pop dx
	pop cx
	xchg bx,ax
L_09c2:
	sub si,ax
	sub di,bx
	mov WORD PTR D_52F6,si
	mov WORD PTR D_52F8,di
	call	ULTIMA_0A22
	or si,si
	je	L_09e3
	mov cx,ax
	mov dx,bx
	add ax,WORD PTR D_52F6
	add bx,WORD PTR D_52F8
	jmp	SHORT ULTIMA_0991
L_09e3:
	cmp ax,D_52D0
	je	L_09ef
	cmp ax,D_52D2
	jne	L_09fb
L_09ef:
	cmp bx,D_52D4
	je	L_0a21
	cmp bx,D_52D6
	je	L_0a21
L_09fb:
	mov si,WORD PTR D_52F6
	mov di,WORD PTR D_52F8
	test si,8000h
	jns	L_0a0b
	neg si
L_0a0b:
	test di,8000h
	jns	L_0a13
	neg di
L_0a13:
	cmp si,di
	jge	L_0a19
	mov si,di
L_0a19:
	cmp si,1
	jl	L_0a21
	jmp	NEAR PTR ULTIMA_0991
L_0a21:
	ret

	PUBLIC	ULTIMA_0A22
ULTIMA_0A22:
	xor si,si
	mov di,si
	cmp ax,D_52D0
	jge	L_0a2f
	db	083h, 0ceh, 001h	; or si,1
L_0a2f:
	cmp cx,D_52D0
	jge	L_0a38
	db	083h, 0cfh, 001h	; or di,1
L_0a38:
	cmp ax,D_52D2
	jle	L_0a41
	db	083h, 0ceh, 002h	; or si,2
L_0a41:
	cmp cx,D_52D2
	jle	L_0a4a
	db	083h, 0cfh, 002h	; or di,2
L_0a4a:
	cmp bx,D_52D6
	jle	L_0a53
	db	083h, 0ceh, 004h	; or si,4
L_0a53:
	cmp dx,D_52D6
	jle	L_0a5c
	db	083h, 0cfh, 004h	; or di,4
L_0a5c:
	cmp bx,D_52D4
	jge	L_0a65
	db	083h, 0ceh, 008h	; or si,8
L_0a65:
	cmp dx,D_52D4
	jge	L_0a6e
	db	083h, 0cfh, 008h	; or di,8
L_0a6e:
	ret
	db	000h

	PUBLIC	ULTIMA_0A70_GRAP_2D_SETPENCOLOR
ULTIMA_0A70_GRAP_2D_SETPENCOLOR:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+4]
	cmp ax,0ffffh
	je	L_0a8d
	mov bx,0fh
	cmp word ptr D_52C8,0
	jne	L_0a8b
	mov bx,3
L_0a8b:
	and ax,bx
L_0a8d:
	mov WORD PTR D_52DA_PEN_COLOR,ax
	cmp ax,0ffffh
	je	L_0a9f
	mov WORD PTR D_5350,2dh
	call DWORD PTR D_5350
L_0a9f:
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	ULTIMA_0AA6_GRAP_3F_FILLRECT
ULTIMA_0AA6_GRAP_3F_FILLRECT:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+0ah]
	mov bx,[bp+8]
	mov cx,[bp+6]
	mov dx,[bp+4]
	call	ULTIMA_08E6_CLIPRECTCOORD
	clc
	mov WORD PTR D_5350,3fh
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 8
	db	000h

	PUBLIC	ULTIMA_0ACE_GRAP_18_TRANSFERARE
ULTIMA_0ACE_GRAP_18_TRANSFERARE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+0ah]
	mov bx,[bp+8]
	mov cx,[bp+6]
	mov dx,[bp+4]
	mov si,[bp+0eh]
	mov di,[bp+0ch]
	cmp si,di
	je	L_0b09
	cmp si,1
	ja	L_0b09
	cmp di,1
	ja	L_0b09
	call	ULTIMA_08E6_CLIPRECTCOORD
	or si,si
	je	L_0afe
	stc
	jb	L_0aff
L_0afe:
	clc
L_0aff:
	mov WORD PTR D_5350,18h
	call DWORD PTR D_5350
L_0b09:
	pop ds
	pop di
	pop si
	pop bp
	ret 0ch

	PUBLIC	ULTIMA_0B10_GRAP_LINE
ULTIMA_0B10_GRAP_LINE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+0ah]
	mov bx,[bp+8]
	mov cx,[bp+6]
	mov dx,[bp+4]
	clc
	call	ULTIMA_0B2D_GRAP_LINE
	pop ds
	pop di
	pop si
	pop bp
	ret 8

	PUBLIC	ULTIMA_0B2D_GRAP_LINE
ULTIMA_0B2D_GRAP_LINE:
	mov D_52CC,cx
	mov D_52CE,dx
	pushf
	cmp word ptr D_52C4,0
	je	L_0b42
	call	ULTIMA_0935_CLIPLINECOORD
	jb	L_0b84
L_0b42:
	cmp bx,dx
	jne	L_0b59
	cmp ax,cx
	je	L_0b78
	call	ULTIMA_08E6_CLIPRECTCOORD
	popf
	mov WORD PTR D_5350,39h
	call DWORD PTR D_5350
	ret
L_0b59:
	cmp ax,cx
	jne	L_0b6c
	call	ULTIMA_08E6_CLIPRECTCOORD
	popf
	mov WORD PTR D_5350,3ch
	call DWORD PTR D_5350
	ret
L_0b6c:
	popf
	mov WORD PTR D_5350,33h
	call DWORD PTR D_5350
	ret
L_0b78:
	popf
	mov WORD PTR D_5350,30h
	call DWORD PTR D_5350
	ret
L_0b84:
	popf
	ret

	PUBLIC	ULTIMA_0B86_GRAP_XORFILLRECT
ULTIMA_0B86_GRAP_XORFILLRECT:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+0ah]
	mov bx,[bp+8]
	mov cx,[bp+6]
	mov dx,[bp+4]
	call	ULTIMA_08E6_CLIPRECTCOORD
	stc
	mov WORD PTR D_5350,3fh
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 8
	db	000h

	PUBLIC	ULTIMA_0BAE_LOADIMAGEFILE
ULTIMA_0BAE_LOADIMAGEFILE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov dx,[bp+4]
	call	ULTIMA_125D_LOADRESOURCEFILEIMP
	jae	L_0bc5
	or bx,bx
	jne	L_0bd3
	mov ax,0ffffh
	jmp	SHORT L_0bdc
L_0bc5:
	push bx
	mov WORD PTR D_5350,42h
	call DWORD PTR D_5350
	pop ax
	jmp	SHORT L_0bdc
L_0bd3:
	mov WORD PTR D_535C,ax
	call dword ptr ds:[5394h]
	xor ax,ax
L_0bdc:
	pop ds
	pop di
	pop si
	pop bp
	ret 2
	db	000h

	PUBLIC	ULTIMA_0BE4_FREEIMAGE
ULTIMA_0BE4_FREEIMAGE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov bx,[bp+4]
	push es
	mov es,bx
	mov ah,49h
	int 21h
	pop es
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	ULTIMA_0BFC_GRAP_63
ULTIMA_0BFC_GRAP_63:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,[bp+8]
	mov di,[bp+6]
	mov ax,[bp+0ch]
	mov bx,[bp+0ah]
	mov cx,[bp+4]
	mov WORD PTR D_5350,63h
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 0ah

	PUBLIC	ULTIMA_0C22_GRAP_0F_SELECTPAGE
ULTIMA_0C22_GRAP_0F_SELECTPAGE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+4]
	mov WORD PTR D_5350,0fh
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	ULTIMA_0C3C
ULTIMA_0C3C:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+0ah]
	mov bx,[bp+8]
	mov cx,[bp+6]
	mov dx,[bp+4]
	mov D_52D0,ax
	mov D_52D4,bx
	mov D_52D2,cx
	mov D_52D6,dx
	pop ds
	pop di
	pop si
	pop bp
	ret 8

	PUBLIC	ULTIMA_0C64_GRAP_30_PSET
ULTIMA_0C64_GRAP_30_PSET:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+6]
	mov bx,[bp+4]
	mov D_52CC,ax
	mov D_52CE,bx
	cmp WORD PTR D_52DA_PEN_COLOR,0ffffh
	je	L_0c95
	cmp word ptr D_52C4,0
	je	L_0c8a
	call	ULTIMA_08CA_INSIDECLIPWINDOW
	jb	L_0c95
L_0c8a:
	clc
	mov WORD PTR D_5350,30h
	call DWORD PTR D_5350
L_0c95:
	pop ds
	pop di
	pop si
	pop bp
	ret 4

	PUBLIC	ULTIMA_0C9C_GRAP_39_HORIZLINE
ULTIMA_0C9C_GRAP_39_HORIZLINE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+8]
	mov bx,[bp+6]
	mov cx,[bp+4]
	mov D_52CC,cx
	cmp word ptr D_52C4,0
	je	L_0cbb
	call	ULTIMA_0CCD
	jb	L_0cc6
L_0cbb:
	clc
	mov WORD PTR D_5350,39h
	call DWORD PTR D_5350
L_0cc6:
	pop ds
	pop di
	pop si
	pop bp
	ret 6

	PUBLIC	ULTIMA_0CCD
ULTIMA_0CCD:
	cmp ax,cx
	jge	L_0cd2
	xchg cx,ax
L_0cd2:
	cmp ax,D_52D0
	jl	L_0cf0
	cmp cx,D_52D2
	jg	L_0cf0
	cmp ax,0
	jl	L_0cf0
	cmp ax,13fh
	jg	L_0cf0
	cmp cx,13fh
	jg	L_0cf0
	clc
	ret
L_0cf0:
	stc
	ret

	PUBLIC	ULTIMA_0CF2_GRAP_3C_VERTLINE
ULTIMA_0CF2_GRAP_3C_VERTLINE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+8]
	mov bx,[bp+6]
	mov dx,[bp+4]
	mov cx,ax
	mov D_52CE,dx
	cmp bx,dx
	jle	L_0d0d
	xchg bx,dx
L_0d0d:
	cmp word ptr D_52C4,0
	je	L_0d19
	call	ULTIMA_0D2B
	jb	L_0d24
L_0d19:
	clc
	mov WORD PTR D_5350,3ch
	call DWORD PTR D_5350
L_0d24:
	pop ds
	pop di
	pop si
	pop bp
	ret 6

	PUBLIC	ULTIMA_0D2B
ULTIMA_0D2B:
	cmp bx,D_52D4
	jl	L_0d4a
	cmp dx,D_52D6
	jg	L_0d4a
	cmp bx,0
	jl	L_0d4a
	cmp bx,0c7h
	jg	L_0d4a
	cmp dx,0c7h
	jg	L_0d4a
	clc
	ret
L_0d4a:
	stc
	ret

	PUBLIC	ULTIMA_0D4C_GRAP_4B_PUTIMAGE
ULTIMA_0D4C_GRAP_4B_PUTIMAGE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,[bp+8]
	mov di,[bp+6]
	mov ax,[bp+0ch]
	mov bx,[bp+0ah]
	mov cx,[bp+4]
	mov WORD PTR D_5350,4bh
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 0ah

_TEXT	ENDS
	END
