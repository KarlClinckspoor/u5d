; A0DE0.ASM: ULTIMA.EXE CS:0de0-1850, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A0DE0
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

	EXTRN	STUB_FONT_02FC:NEAR
	EXTRN	ULTIMA_08E6_CLIPRECTCOORD:NEAR
	EXTRN	ULTIMA_0B2D_GRAP_LINE:NEAR
	EXTRN	ULTIMA_1F77_CONVERTCHARCOORDTOP:NEAR
	EXTRN	ULTIMA_5910_UPDATEFRAME:NEAR
	EXTRN	D_52BA_VDP:WORD
	EXTRN	D_52DA_PEN_COLOR:WORD
	EXTRN	D_52EF_FORCEEGA:WORD
	EXTRN	D_52F1_FORCETANDY:WORD
	EXTRN	D_52F3_FORCEHERC:WORD
	EXTRN	D_5350:WORD
	EXTRN	D_5354:WORD
	EXTRN	D_5356:WORD
	EXTRN	D_5358:WORD
	EXTRN	D_535A:WORD
	EXTRN	D_535C:WORD
	EXTRN	D_5386_CURRENT_TEXT_WINDOW_IDX:WORD
	EXTRN	D_538E:WORD
	EXTRN	D_5398_CURRENTCHARSET:WORD
	EXTRN	D_539A_CURRENTTEXTWINDOW:WORD
	EXTRN	D_53A4_UNDERLINE:WORD
	EXTRN	D_53A6:WORD
	EXTRN	D_53A8_INVERSE:WORD
	EXTRN	D_53AA_TEXT_FG_COLOR:BYTE
	EXTRN	D_53AB_TEXT_BG_COLOR:BYTE
	EXTRN	D_53B4:WORD
	EXTRN	D_53B6:WORD
	EXTRN	D_53B8:WORD
	EXTRN	D_53BA:WORD
	EXTRN	D_53BC:WORD
	EXTRN	D_53BE:WORD
	EXTRN	D_5893_MAP_ID:BYTE

_TEXT	SEGMENT

	PUBLIC	ULTIMA_0DE0_DETECTVIDEO
ULTIMA_0DE0_DETECTVIDEO:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,0f000h
	mov es,ax
	mov di,0fffeh
	mov ax,0
	mov bl,es:[di]
	cmp bl,0fdh
	je	L_0e09
	cmp bl,0ffh
	je	L_0e09
	mov ax,2
	cmp bl,0fch
	je	L_0e09
	mov ax,6
L_0e09:
	mov ds:[52cah],ax
	cmp bl,0fdh
	je	L_0e83
	cmp bl,0ffh
	jne	L_0e3c
	mov di,0a000h
	mov cx,5fffh
L_0e1c:
	mov al,54h
	repne scasb
	jne	L_0e3c
	push di
	push cx
	mov si,5322h
	dec di
	mov cx,5
	repe cmpsb
	pop cx
	pop di
	jne	L_0e1c
	mov word ptr ds:[52cah],3
	mov ax,2
	jmp	SHORT L_0e8b
L_0e3c:
	mov ah,12h
	mov bl,10h
	mov cx,0ffffh
	int 10h
	cmp cx,0ffffh
	je	L_0e5e
	mov al,bh
	and ax,1
	xor al,1
	or al,al
	je	L_0e88
	cmp al,1
	jne	L_0e5e
	mov ax,1
	jmp	SHORT L_0e8b
L_0e5e:
	mov ah,0fh
	int 10h
	cmp al,7
	je	L_0e68
	jmp	SHORT L_0e83
L_0e68:
	mov dx,3bah
	in al,dx
	and al,80h
	mov ah,al
	mov cx,8000h
L_0e73:
	in al,dx
	and al,80h
	cmp al,ah
	jne	L_0e7e
	loop	L_0e73
	jmp	SHORT L_0e83
L_0e7e:
	mov ax,3
	jmp	SHORT L_0e8b
L_0e83:
	mov ax,0
	jmp	SHORT L_0e8b
L_0e88:
	mov ax,0ffffh
L_0e8b:
	mov ds:[52c8h],ax
	pop ds
	pop di
	pop si
	pop bp
	ret
	db	000h

	PUBLIC	ULTIMA_0E94_LOADVIDEODRIVER
ULTIMA_0E94_LOADVIDEODRIVER:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	cmp word ptr ds:[52c8h],0ffffh
	jne	L_0ea4
	jmp	NEAR PTR L_0f24
L_0ea4:
	cmp WORD PTR D_52BA_VDP,0
	je	L_0eb3
	mov word ptr ds:[52c8h],0
	jmp	SHORT L_0ede
L_0eb3:
	cmp WORD PTR D_52EF_FORCEEGA,0
	je	L_0ec2
	mov word ptr ds:[52c8h],1
	jmp	SHORT L_0ede
L_0ec2:
	cmp WORD PTR D_52F3_FORCEHERC,0
	je	L_0ed1
	mov word ptr ds:[52c8h],3
	jmp	SHORT L_0ede
L_0ed1:
	cmp WORD PTR D_52F1_FORCETANDY,0
	je	L_0ede
	mov word ptr ds:[52c8h],2
L_0ede:
	mov bx,ds:[52c8h]
	cmp bx,2
	jne	L_0f01
	mov bx,400h
	call	L_15c6
	jae	L_0ef3
	mov ah,4ch
	int 21h
L_0ef3:
	mov bx,50h
	mov es,bx
	mov bx,10h
	mov es:[bx],ax
	mov bx,2
L_0f01:
	shl bx,1
	mov dx,[bx+5328h]
	call	ULTIMA_125D_LOADRESOURCEFILEIMP
	jb	L_0f12
	mov ds:[5352h],bx
	jae	L_0f24
L_0f12:
	or bx,bx
	je	L_0f21
	mov WORD PTR D_535C,ax
	call dword ptr ds:[5394h]
	xor ax,ax
	jmp	SHORT L_0f24
L_0f21:
	mov ax,0ffffh
L_0f24:
	pop ds
	pop di
	pop si
	pop bp
	ret
	db	000h

	PUBLIC	ULTIMA_0F2A_GRAP_06_ALLOCPAGEBU
ULTIMA_0F2A_GRAP_06_ALLOCPAGEBU:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	stc
	mov WORD PTR D_5350,6
	call DWORD PTR D_5350
	mov WORD PTR D_5354,ax
	pop ds
	pop di
	pop si
	mov sp,bp
	pop bp
	ret
	db	000h

	PUBLIC	ULTIMA_0F46_GRAP_66_REVEAL
ULTIMA_0F46_GRAP_66_REVEAL:
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
	mov WORD PTR D_5350,66h
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 8
	db	000h

	PUBLIC	ULTIMA_0F6E_GRAP_1B_TRANSFERFUL
ULTIMA_0F6E_GRAP_1B_TRANSFERFUL:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+6]
	mov bx,[bp+4]
	cmp ax,bx
	je	L_0f88
	mov WORD PTR D_5350,1bh
	call DWORD PTR D_5350
L_0f88:
	pop ds
	pop di
	pop si
	pop bp
	ret 4
	db	000h

	PUBLIC	ULTIMA_0F90_GRAP_PEN
ULTIMA_0F90_GRAP_PEN:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,ds:[52cch]
	mov bx,ds:[52ceh]
	mov cx,[bp+6]
	mov dx,[bp+4]
	clc
	call	ULTIMA_0B2D_GRAP_LINE
	pop ds
	pop di
	pop si
	pop bp
	ret 4

	PUBLIC	ULTIMA_0FAE_LOADRESOURCEFILE
ULTIMA_0FAE_LOADRESOURCEFILE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov dx,[bp+4]
	call	ULTIMA_125D_LOADRESOURCEFILEIMP
	jae	L_0fc2
	or bx,bx
	je	L_0fc6
	jne	L_0fcb
L_0fc2:
	mov ax,bx
	jmp	SHORT L_0fd4
L_0fc6:
	mov ax,0ffffh
	jmp	SHORT L_0fd4
L_0fcb:
	mov WORD PTR D_535C,ax
	call dword ptr ds:[5394h]
	xor ax,ax
L_0fd4:
	pop ds
	pop di
	pop si
	pop bp
	ret 2
	db	000h

	PUBLIC	ULTIMA_0FDC_FREEBITIMAGE
ULTIMA_0FDC_FREEBITIMAGE:
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

	PUBLIC	ULTIMA_0FF4_LOADTILESET
ULTIMA_0FF4_LOADTILESET:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov dx,[bp+4]
	call	ULTIMA_125D_LOADRESOURCEFILEIMP
	jae	L_100b
	or bx,bx
	jne	L_101d
	mov ax,0ffffh
	jmp	SHORT L_1026
L_100b:
	mov cx,ax
	dec cx
	mov WORD PTR D_5350,48h
	call DWORD PTR D_5350
	mov ax,1
	jmp	SHORT L_1026
L_101d:
	mov WORD PTR D_535C,ax
	call dword ptr ds:[5394h]
	xor ax,ax
L_1026:
	pop ds
	pop di
	pop si
	pop bp
	ret 2
	db	000h

	PUBLIC	ULTIMA_102E_UNLOADTILESET
ULTIMA_102E_UNLOADTILESET:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov WORD PTR D_5350,5ah
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret
	db	000h

	PUBLIC	ULTIMA_1044_GRAP_4E_COPYBITIMAG
ULTIMA_1044_GRAP_4E_COPYBITIMAG:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+0ah]
	mov bx,[bp+8]
	mov si,[bp+6]
	mov di,[bp+4]
	mov WORD PTR D_5350,4eh
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 8
	db	000h

	PUBLIC	ULTIMA_1068
ULTIMA_1068:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	xor di,di
L_1070:
	push ds
	mov ax,[bp+6]
	mov bx,[bp+4]
	mov si,[bp+8]
	mov cx,ds:[52bch]
	mov dx,ds:[52beh]
	stc
	mov WORD PTR D_5350,66h
	call DWORD PTR D_5350
	pop ds
	push ds
	inc di
	mov ax,[bp+6]
	mov bx,[bp+4]
	mov si,[bp+8]
	mov cx,ds:[52bch]
	mov dx,ds:[52beh]
	stc
	mov WORD PTR D_5350,66h
	call DWORD PTR D_5350
	pop ds
	inc di
	cmp di,100h
	je	L_10d6
	test di,7
	jne	L_1070
	push di
	cmp BYTE PTR D_5893_MAP_ID,42h
	jne	L_10d0
	mov ax,1
	push ax
	call	STUB_FONT_02FC
	pop di
	or ax,ax
	je	L_1070
	jne	L_10d8
L_10d0:
	call	ULTIMA_5910_UPDATEFRAME
	pop di
	jmp	SHORT L_1070
L_10d6:
	xor ax,ax
L_10d8:
	pop ds
	pop di
	pop si
	pop bp
	ret 6
	db	000h

	PUBLIC	ULTIMA_10E0_GRAP_51_PUTTILE
ULTIMA_10E0_GRAP_51_PUTTILE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+6]
	mov bx,[bp+4]
	mov ah,bl
	mov bx,[bp+8]
	mov cx,ds:[52bch]
	mov dx,ds:[52beh]
	mov si,ds:[52c0h]
	mov di,ds:[52c2h]
	mov WORD PTR D_5350,51h
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 6

	PUBLIC	ULTIMA_1112_GRAP_60
ULTIMA_1112_GRAP_60:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+8]
	mov bl,BYTE PTR D_5893_MAP_ID
	mov cx,ds:[52bch]
	mov dx,ds:[52beh]
	mov si,[bp+6]
	mov di,[bp+4]
	stc
	mov WORD PTR D_5350,60h
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret 6
	db	000h

	PUBLIC	ULTIMA_1140_GRAP_6F_ANIMATEORIG
ULTIMA_1140_GRAP_6F_ANIMATEORIG:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,WORD PTR D_5356
	mov WORD PTR D_5350,6fh
	call DWORD PTR D_5350
	pop ds
	pop di
	pop si
	pop bp
	ret

	PUBLIC	ULTIMA_1158_INITTIMER
ULTIMA_1158_INITTIMER:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	push ds
	push cs
	pop ds
	mov dx,1233h
	mov al,24h
	mov ah,25h
	int 21h
	push cs
	pop ds
	mov dx,1247h
	mov al,23h
	mov ah,25h
	int 21h
	pop ds
	call	ULTIMA_1184_INITTEXTWINDOW
	call	ULTIMA_1226_INITTEXTVARS
	call	ULTIMA_11B4_CALIBRATETIMER
	pop ds
	pop di
	pop si
	pop bp
	ret

	PUBLIC	ULTIMA_1184_INITTEXTWINDOW
ULTIMA_1184_INITTEXTWINDOW:
	mov si,535eh
	mov WORD PTR D_539A_CURRENTTEXTWINDOW,si
	xor ax,ax
	mov cx,4
L_1190:
	mov [si],al
	mov [si+1],al
	mov byte ptr [si+2],27h
	mov byte ptr [si+3],18h
	mov [si+4],al
	mov [si+5],al
	mov byte ptr [si+6],0fh
	mov [si+7],al
	add si,8
	loop	L_1190
	cbw
	mov WORD PTR D_5386_CURRENT_TEXT_WINDOW_IDX,ax
	ret

	PUBLIC	ULTIMA_11B4_CALIBRATETIMER
ULTIMA_11B4_CALIBRATETIMER:
	mov WORD PTR D_5356,0
	mov WORD PTR D_535A,0
	mov al,1ch
	mov ah,35h
	int 21h
	push es
	push bx
	push ds
	mov ax,cs
	mov ds,ax
	lea dx,ds:[1214h]
	mov al,1ch
	mov ah,25h
	int 21h
	pop ds
L_11d8:
	cmp WORD PTR D_535A,0
	je	L_11d8
	mov WORD PTR D_535A,0
L_11e5:
	inc WORD PTR D_5356
	cmp WORD PTR D_535A,0
	je	L_11e5
	pop dx
	pop ax
	push ds
	mov ds,ax
	mov al,1ch
	mov ah,25h
	int 21h
	pop ds
	mov ax,WORD PTR D_5356
	xor dx,dx
	mov cx,12h
	mul cx
	mov cx,2eeh
	div cx
	mov WORD PTR D_5356,ax
	int 12h
	mov WORD PTR D_5358,ax
	ret
L_1214:
	sti
	push ds
	push si
	push ax
	mov	ax,DGROUP
	mov ds,ax
	mov si,535ah
	inc word ptr [si]
	pop ax
	pop si
	pop ds
	iret

	PUBLIC	ULTIMA_1226_INITTEXTVARS
ULTIMA_1226_INITTEXTVARS:
	xor ax,ax
	mov WORD PTR D_53A4_UNDERLINE,ax
	mov WORD PTR D_53A6,ax
	mov WORD PTR D_53A8_INVERSE,ax
	ret
	db	0cbh
L_1233:
	sti
	push ds
	push bx
	mov	bx,DGROUP
	mov ds,bx
	mov bx,535ch
	mov word ptr [bx],1
	xor al,al
	pop bx
	pop ds
	iret
L_1247:
	sti
	clc
	iret
L_124a:
	mov ax,3
	int 10h
	mov dx,53d6h
	mov ah,9
	int 21h
	mov ax,4c01h
	int 21h
L_125b:
	jmp	SHORT L_125b

	PUBLIC	ULTIMA_125D_LOADRESOURCEFILEIMP
ULTIMA_125D_LOADRESOURCEFILEIMP:
	call	ULTIMA_1588_ISFILECOMPRESSED
	jae	L_1264
	jmp	SHORT L_12ac
L_1264:
	push dx
	call	L_1654
	jb	L_12a6
	push ax
	or ax,ax
	jne	L_1274
	or dx,dx
	je	L_12a6
	dec ax
L_1274:
	mov bx,ax
	or ax,ax
	je	L_1280
	dec bx
	mov cl,4
	shr bx,cl
	inc bx
L_1280:
	call	L_15c6
	pop bx
	jb	L_12a0
	pop dx
	push bx
	push ax
	call	L_160e
	pop bx
	pop cx
	jae	L_129c
	push ax
	push es
	mov es,bx
	mov ah,49h
	int 21h
	pop es
	pop ax
	jmp	SHORT L_12a7
L_129c:
	mov ax,cx
	clc
	ret
L_12a0:
	pop bx
	xor bx,bx
	stc
	jmp	SHORT L_124a
L_12a6:
	pop bx
L_12a7:
	mov bx,1
	stc
	ret
L_12ac:
	mov WORD PTR D_53BA,0
	mov WORD PTR D_53BC,0
	mov WORD PTR D_53BE,0
	mov WORD PTR D_53B8,0
	mov al,2
	mov ah,3dh
	int 21h
	mov bx,ax
	jb	L_1321
	mov WORD PTR D_53B8,bx
	mov cx,4
	mov dx,53b4h
	mov ah,3fh
	int 21h
	jb	L_1321
	mov bx,WORD PTR D_53B4
	mov dx,WORD PTR D_53B6
	mov cl,4
L_12e8:
	shr dx,1
	rcr bx,1
	loop	L_12e8
	inc bx
	call	L_15c6
	jae	L_12fc
	mov WORD PTR D_53BA,0ffffh
	jmp	SHORT L_1321
L_12fc:
	mov WORD PTR D_53BA,ax
	mov WORD PTR D_53BC,ax
	mov bx,40h
	call	L_15c6
	jae	L_1312
	mov WORD PTR D_53BA,0ffffh
	jmp	SHORT L_1321
L_1312:
	mov WORD PTR D_53BE,ax
	call	L_135a
	clc
	mov ax,WORD PTR D_53B4
	or ax,ax
	jne	L_1321
	dec ax
L_1321:
	pushf
	push ax
	mov bx,WORD PTR D_53BE
	or bx,bx
	je	L_1333
	push es
	mov es,bx
	mov ah,49h
	int 21h
	pop es
L_1333:
	mov bx,WORD PTR D_53B8
	je	L_133d
	mov ah,3eh
	int 21h
L_133d:
	mov bx,WORD PTR D_53BA
	cmp bx,0ffffh
	jne	L_134c
	inc bx
	pop ax
	popf
	jmp	NEAR PTR L_124a
L_134c:
	pop ax
	popf
	jae	L_1355
	mov bx,1
	stc
	ret
L_1355:
	mov bx,WORD PTR D_53BC
	ret
L_135a:
	mov word ptr ds:[53d2h],9
	mov word ptr ds:[53c2h],0
	mov word ptr ds:[53c4h],0
	mov word ptr ds:[53cch],102h
	mov word ptr ds:[53d0h],200h
	mov ax,WORD PTR D_5354
	mov ds:[53c0h],ax
	mov bx,WORD PTR D_53B8
	push ds
	mov ds,WORD PTR D_53BE
	xor dx,dx
	mov cx,400h
	mov ah,3fh
	int 21h
	pop ds
L_1391:
	push bp
	mov ax,ds:[53c4h]
	add ax,ds:[53d2h]
	xchg ax,ds:[53c4h]
	mov cx,8
	xor dx,dx
	div cx
	cmp ax,3fdh
	jl	L_13d8
	push dx
	add dx,ds:[53d2h]
	mov ds:[53c4h],dx
	mov cx,400h
	mov bp,ax
	sub cx,ax
	push ds
	mov bx,WORD PTR D_53B8
	mov ds,WORD PTR D_53BE
	push ds
	pop es
	xor si,si
	mov di,si
	add si,ax
	rep movsb
	mov cx,bp
	mov dx,di
	mov ah,3fh
	int 21h
	pop ds
	xor ax,ax
	pop dx
L_13d8:
	push ds
	mov ds,WORD PTR D_53BE
	mov si,ax
	lodsw
	mov bx,ax
	lodsb
	mov cx,dx
	jcxz	L_13ed
L_13e7:
	shr al,1
	rcr bx,1
	loop	L_13e7
L_13ed:
	pop ds
	mov ax,bx
	mov bx,ds:[53d2h]
	sub bx,9
	shl bx,1
	and ax,[bx+53ach]
	pop bp
	cmp ax,101h
	jne	L_1411
	mov cx,1800h
	xor ax,ax
	mov es,ds:[53c0h]
	mov di,ax
	rep stosw
	ret
L_1411:
	cmp ax,100h
	je	L_1419
	jmp	NEAR PTR L_14c5
L_1419:
	mov word ptr ds:[53d2h],9
	mov word ptr ds:[53d0h],200h
	mov word ptr ds:[53cch],102h
	push bp
	mov ax,ds:[53c4h]
	add ax,ds:[53d2h]
	xchg ax,ds:[53c4h]
	mov cx,8
	xor dx,dx
	div cx
	cmp ax,3fdh
	jl	L_1472
	push dx
	add dx,ds:[53d2h]
	mov ds:[53c4h],dx
	mov cx,400h
	mov bp,ax
	sub cx,ax
	push ds
	mov bx,WORD PTR D_53B8
	mov ds,WORD PTR D_53BE
	push ds
	pop es
	xor si,si
	mov di,si
	add si,ax
	rep movsb
	mov cx,bp
	mov dx,di
	mov ah,3fh
	int 21h
	pop ds
	xor ax,ax
	pop dx
L_1472:
	push ds
	mov ds,WORD PTR D_53BE
	mov si,ax
	lodsw
	mov bx,ax
	lodsb
	mov cx,dx
	jcxz	L_1487
L_1481:
	shr al,1
	rcr bx,1
	loop	L_1481
L_1487:
	pop ds
	mov ax,bx
	mov bx,ds:[53d2h]
	sub bx,9
	shl bx,1
	and ax,[bx+53ach]
	pop bp
	mov ds:[53c6h],ax
	mov ds:[53c8h],ax
	mov ds:[53d5h],al
	mov ds:[53d4h],al
	push es
	mov es,WORD PTR D_53BA
	mov di,ds:[53c2h]
	stosb
	pop es
	inc word ptr ds:[53c2h]
	cmp di,10h
	jne	L_14c2
	inc WORD PTR D_53BA
	mov word ptr ds:[53c2h],0
L_14c2:
	jmp	NEAR PTR L_1391
L_14c5:
	mov ds:[53c6h],ax
	mov ds:[53cah],ax
	mov es,ds:[53c0h]
	cmp ax,ds:[53cch]
	jl	L_14e3
	mov ax,ds:[53c8h]
	mov ds:[53c6h],ax
	mov al,ds:[53d4h]
	push ax
	inc word ptr ds:[53ceh]
L_14e3:
	cmp word ptr ds:[53c6h],0ffh
	jle	L_1508
	mov bx,ds:[53c6h]
	push ax
	mov ax,bx
	shl bx,1
	add bx,ax
	pop ax
	mov al,es:[bx+2]
	push ax
	inc word ptr ds:[53ceh]
	mov ax,es:[bx]
	mov ds:[53c6h],ax
	jmp	SHORT L_14e3
L_1508:
	push ds
	pop es
	mov ax,ds:[53c6h]
	mov ds:[53d4h],al
	mov ds:[53d5h],al
	push ax
	inc word ptr ds:[53ceh]
	mov cx,ds:[53ceh]
	jcxz	L_153f
L_151e:
	pop ax
	push es
	mov es,WORD PTR D_53BA
	mov di,ds:[53c2h]
	stosb
	pop es
	inc word ptr ds:[53c2h]
	cmp di,10h
	jne	L_153d
	inc WORD PTR D_53BA
	mov word ptr ds:[53c2h],0
L_153d:
	loop	L_151e
L_153f:
	mov ds:[53ceh],cx
	mov bx,ds:[53cch]
	push ax
	mov ax,bx
	shl bx,1
	add bx,ax
	pop ax
	push es
	mov es,ds:[53c0h]
	mov al,ds:[53d5h]
	mov es:[bx+2],al
	mov ax,ds:[53c8h]
	mov es:[bx],ax
	pop es
	inc word ptr ds:[53cch]
	mov ax,ds:[53cah]
	mov ds:[53c8h],ax
	mov bx,ds:[53cch]
	cmp bx,ds:[53d0h]
	jl	L_1585
	cmp word ptr ds:[53d2h],0ch
	je	L_1585
	inc word ptr ds:[53d2h]
	shl word ptr ds:[53d0h],1
L_1585:
	jmp	NEAR PTR L_1391

	PUBLIC	ULTIMA_1588_ISFILECOMPRESSED
ULTIMA_1588_ISFILECOMPRESSED:
	push bx
	push si
	push di
	mov bx,25eah
L_158e:
	mov si,[bx]
	cmp word ptr [si],0
	je	L_15c1
	add bx,2
	mov di,dx
L_159a:
	lodsb
	mov ah,[di]
	cmp ah,61h
	jl	L_15aa
	cmp ah,7ah
	jg	L_15aa
	sub ah,20h
L_15aa:
	cmp al,61h
	jl	L_15b4
	cmp al,7ah
	jg	L_15b4
	sub al,20h
L_15b4:
	cmp ah,al
	jne	L_158e
	cmp byte ptr [si],0
	je	L_15c0
	inc di
	jmp	SHORT L_159a
L_15c0:
	stc
L_15c1:
	pop di
	pop si
	pop bx
	ret
	db	090h

	PUBLIC	L_15c6
L_15c6:
	push bx
	mov ah,30h
	int 21h
	cmp al,3
	je	L_15f2
	mov bx,0ffffh
	mov ah,48h
	int 21h
	pop ax
	sub bx,ax
	dec bx
	push ax
	mov ah,48h
	int 21h
	pop bx
	push ax
	mov ah,48h
	int 21h
	pop bx
	push ax
	pushf
	push es
	mov es,bx
	mov ah,49h
	int 21h
	pop es
	jmp	SHORT L_160b
L_15f2:
	mov al,1
	mov bx,2
	mov ah,58h
	int 21h
	pop bx
	mov ah,48h
	int 21h
	push ax
	pushf
	mov al,1
	mov bx,0
	mov ah,58h
	int 21h
L_160b:
	popf
	pop ax
	ret

	PUBLIC	L_160e
L_160e:
	push ds
	push ax
	push bx
	mov al,2
	mov ah,3dh
	int 21h
	mov bx,ax
	jb	L_1650
	pop cx
	pop ds
	xor dx,dx
	or cx,cx
	jne	L_1624
	dec cx
L_1624:
	push cx
	call	L_1649
	pop cx
	jb	L_163f
	cmp ax,cx
	jb	L_163e
	cmp cx,0ffffh
	jne	L_163e
	push cx
	mov dx,cx
	mov cx,1
	call	L_1649
	pop ax
L_163e:
	clc
L_163f:
	push ax
	pushf
	mov ah,3eh
	int 21h
	popf
	pop ax
	pop ds
	ret
L_1649:
	push bx
	mov ah,3fh
	int 21h
	pop bx
	ret
L_1650:
	pop bx
	pop bx
	pop ds
	ret

	PUBLIC	L_1654
L_1654:
	mov al,2
	mov ah,3dh
	int 21h
	mov bx,ax
	jb	L_1672
	xor cx,cx
	mov dx,cx
	push bx
	mov al,2
	mov ah,42h
	int 21h
	pop bx
	push ax
	push dx
	mov ah,3eh
	int 21h
	pop dx
	pop ax
L_1672:
	ret
	db	000h

	PUBLIC	ULTIMA_1674_TESTOPENFILE
ULTIMA_1674_TESTOPENFILE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov dx,[bp+4]
	mov al,2
	mov ah,3dh
	int 21h
	mov bx,ax
	jb	L_1696
	mov ah,3eh
	int 21h
	mov WORD PTR D_535C,0
	mov ax,1
	jmp	SHORT L_169f
L_1696:
	mov WORD PTR D_535C,ax
	call dword ptr ds:[5394h]
	xor ax,ax
L_169f:
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	ULTIMA_16A6_GETDEFAULTDRIVE
ULTIMA_16A6_GETDEFAULTDRIVE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ah,19h
	int 21h
	xor ah,ah
	add al,41h
	pop ds
	pop di
	pop si
	pop bp
	ret
	db	000h

	PUBLIC	ULTIMA_16BA_PRINTCHAR
ULTIMA_16BA_PRINTCHAR:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov di,[bp+4]
	and di,0ffh
	mov dx,di
	mov si,WORD PTR D_539A_CURRENTTEXTWINDOW
	cmp dl,7fh
	jbe	L_16d5
	jmp	NEAR PTR L_176e
L_16d5:
	cmp dl,0ah
	je	L_1742
	cmp dl,0dh
	je	L_1745
	mov cl,3
	shl di,cl
	cmp word ptr ds:[52c8h],3
	jne	L_16f0
	mov ax,di
	shl di,1
	add di,ax
L_16f0:
	mov es,WORD PTR D_5398_CURRENTCHARSET
	mov dl,BYTE PTR D_53AA_TEXT_FG_COLOR
	mov dh,BYTE PTR D_53AB_TEXT_BG_COLOR
	mov al,[si+4]
	add al,[si]
	xor ah,ah
	mov bl,[si+5]
	add bl,[si+1]
	mov bh,ah
	call	ULTIMA_17F4_APPLYCHAREFFECTS
	mov WORD PTR D_5350,5dh
	call DWORD PTR D_5350
	mov si,53eah
	mov cx,4
	cmp word ptr ds:[52c8h],3
	jne	L_1728
	mov cx,0ch
L_1728:
	rep movsw
	mov si,WORD PTR D_539A_CURRENTTEXTWINDOW
	cmp WORD PTR D_538E,0
	je	L_1767
	inc byte ptr [si+4]
	mov al,[si+4]
	add al,[si]
	cmp al,[si+2]
	jle	L_1767
L_1742:
	inc byte ptr [si+5]
L_1745:
	mov byte ptr [si+4],0
	mov al,[si+5]
	add al,[si+1]
	cmp al,[si+3]
	jle	L_1767
	call	ULTIMA_1F77_CONVERTCHARCOORDTOP
	dec byte ptr [si+5]
	mov si,0fff8h
	mov WORD PTR D_5350,27h
	call DWORD PTR D_5350
L_1767:
	pop ds
	pop di
	pop si
	pop bp
	ret 2
L_176e:
	cmp dl,0ffh
	je	L_17bb
	cmp dl,0feh
	je	L_17b0
	cmp dl,0fdh
	je	L_17a5
	cmp dl,0fch
	je	L_1799
	cmp dl,0fbh
	je	L_178d
	and dl,7fh
	jmp	NEAR PTR L_16d5
L_178d:
	mov WORD PTR D_53A6,0
	and byte ptr [si+7],0fdh
	jmp	SHORT L_1767
L_1799:
	mov WORD PTR D_53A6,1
	or byte ptr [si+7],2
	jmp	SHORT L_1767
L_17a5:
	db	083h, 036h, 0a8h, 053h, 001h	; xor word ptr ds:[53a8h],1
	xor byte ptr [si+7],4
	jmp	SHORT L_1767
L_17b0:
	db	083h, 036h, 0a4h, 053h, 001h	; xor word ptr ds:[53a4h],1
	xor byte ptr [si+7],1
	jmp	SHORT L_1767
L_17bb:
	mov byte ptr [si+4],0
	mov byte ptr [si+5],0
	mov al,[si+6]
	mov cl,4
	shr al,cl
	and al,0fh
	mov WORD PTR D_5350,2dh
	call DWORD PTR D_5350
	call	ULTIMA_1F77_CONVERTCHARCOORDTOP
	clc
	mov WORD PTR D_5350,3fh
	call DWORD PTR D_5350
	mov ax,WORD PTR D_52DA_PEN_COLOR
	mov WORD PTR D_5350,2dh
	call DWORD PTR D_5350
	jmp	NEAR PTR L_1767

	PUBLIC	ULTIMA_17F4_APPLYCHAREFFECTS
ULTIMA_17F4_APPLYCHAREFFECTS:
	mov si,di
	push di
	mov di,53eah
	push cx
	mov cx,4
	cmp word ptr ds:[52c8h],3
	jne	L_1808
	mov cx,0ch
L_1808:
	push es
	push ds
	pop es
	pop ds
	rep movsw
	pop cx
	pop di
	push ds
	push es
	pop ds
	pop es
	cmp WORD PTR D_53A4_UNDERLINE,0
	je	L_182f
	cmp word ptr ds:[52c8h],3
	jne	L_182a
	mov word ptr es:[di+16h],0ffffh
	jmp	SHORT L_182f
L_182a:
	mov byte ptr es:[di+7],0ffh
L_182f:
	cmp WORD PTR D_53A8_INVERSE,0
	je	L_184f
	push cx
	push di
	mov cx,4
	cmp word ptr ds:[52c8h],3
	jne	L_1845
	mov cx,0ch
L_1845:
	not word ptr es:[di]
	add di,2
	loop	L_1845
	pop di
	pop cx
L_184f:
	ret

_TEXT	ENDS
	END
