; AFONT0.ASM: FONT.OVL 0000-02a2, FONT_0000_DRAWJUSTIFIEDTEXT: MSC 5.0 output in the original (font.c's first
; function), not yet matched in C. It prints justified text: [bp+4] the glyph image, [bp+6]
; the text (see the C version in src/font.c, kept for the native build). The C comes within
; 56 bytes (670 against 674), so until it matches, the DOS build takes the function from
; here: linked before font.c, at the start of FONT.OVL. It has no data of its own; the tables
; it reads are font.c's (DS:50ea-515a).
; Drafted from the original bytes by tools/asmgen.py --ovl and checked by tools/asmbuild.py.

	TITLE	AFONT0
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

	EXTRN	ULTIMA_0C22_GRAP_0F_SELECTPAGE:NEAR
	EXTRN	ULTIMA_1044_GRAP_4E_COPYBITIMAG:NEAR
	EXTRN	D_50EA_GLYPHWIDTHS:BYTE
	EXTRN	D_5146_TEXTLEFTX:WORD
	EXTRN	D_514C_TEXTRIGHTX:WORD
	EXTRN	D_5150_REGIONTOP:WORD
	EXTRN	D_5152_REGIONBOTTOM:WORD
	EXTRN	D_5154_SPACEWIDTH:WORD
	EXTRN	D_5156_PENX:WORD
	EXTRN	D_5158_PENY:WORD

_TEXT	SEGMENT

	PUBLIC	FONT_0000_DRAWJUSTIFIEDTEXT
FONT_0000_DRAWJUSTIFIEDTEXT:
	push bp
	mov bp,sp
	sub sp,1ch
	push di
	push si
	mov ax,1
	push ax
	call	ULTIMA_0C22_GRAP_0F_SELECTPAGE
	mov ax,WORD PTR D_5150_REGIONTOP
	cmp WORD PTR D_5158_PENY,ax
	jle	L_0026
	mov ax,WORD PTR D_5152_REGIONBOTTOM
	cmp WORD PTR D_5158_PENY,ax
	jge	L_0026
	mov ax,1
	jmp	SHORT L_0028
L_0026:
	sub ax,ax
L_0028:
	mov [bp-10h],ax
	mov si,ax
	shl si,1
	mov ax,D_514C_TEXTRIGHTX[si]
	sub ax,D_5146_TEXTLEFTX[si]
	mov [bp-0ah],ax
	sub ax,ax
	mov [bp-4],ax
	mov [bp-0ch],ax
	mov ax,WORD PTR D_5156_PENX
	sub ax,D_5146_TEXTLEFTX[si]
	mov [bp-2],ax
	jmp	NEAR PTR L_027b
	db	090h		; nop, as the compiler left it after this jmp
L_0050:
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],7bh
	jne	L_005e
	add si,0fh
	jmp	SHORT L_0078
L_005e:
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],5fh
	je	L_0078
	mov bx,cx
	add bx,dx
	mov bl,[bx]
	sub bh,bh
	mov al,D_50EA_GLYPHWIDTHS-20h[bx]
	sub ah,ah
	inc ax
	add si,ax
L_0078:
	inc cx
L_0079:
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],0
	je	L_00a0
	cmp [bp-0ah],si
	jle	L_00a0
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],0ah
	je	L_00a0
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],20h
	ja	L_0050
	add si,WORD PTR D_5154_SPACEWIDTH
	inc di
	jmp	SHORT L_0078
L_00a0:
	mov [bp-8],di
	mov [bp-2],si
	mov [bp-0eh],cx
	mov al,BYTE PTR D_50EA_GLYPHWIDTHS+0dh	; D_50F7_HYPHENWIDTH, the width of '-'
	sub ah,ah
	mov [bp-16h],ax
	inc ax
	mov [bp-12h],ax
	mov ax,[bp-16h]
	mov [bp-14h],ax
	mov di,[bp-0ah]
	mov dx,[bp+4]
L_00c1:
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],0
	jne	L_00ce
	cmp si,di
	jl	L_00f0
L_00ce:
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],0ah
	je	L_00f0
	dec cx
	cmp [bp-0ch],cx
	jge	L_00f0
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],20h
	je	L_00e9
	jmp	NEAR PTR L_016a
L_00e9:
	sub si,WORD PTR D_5154_SPACEWIDTH
	dec word ptr [bp-8]
L_00f0:
	mov [bp-2],si
	mov [bp-0eh],cx
	mov ax,[bp-0ah]
	sub ax,si
	mov [bp-6],ax
	mov ax,cx
	mov [bp-4],ax
	mov ax,[bp-0ch]
	mov [bp-0eh],ax
	mov ax,[bp-4]
	cmp [bp-0eh],ax
	jl	L_0114
	jmp	NEAR PTR L_020a
L_0114:
	mov bx,ax
	mov si,[bp+4]
	mov al,[bx+si]
	mov [bp-18h],al
	mov di,si
	mov si,[bp-0eh]
L_0123:
	mov bx,si
	add bx,di
	cmp byte ptr [bx],20h
	jbe	L_012f
	jmp	NEAR PTR L_01b0
L_012f:
	mov ax,WORD PTR D_5154_SPACEWIDTH
	add WORD PTR D_5156_PENX,ax
	cmp word ptr [bp-8],0
	jne	L_013f
	jmp	NEAR PTR L_01fe
L_013f:
	cmp byte ptr [bp-18h],0
	jne	L_0148
	jmp	NEAR PTR L_01fe
L_0148:
	cmp byte ptr [bp-18h],0ah
	jne	L_0151
	jmp	NEAR PTR L_01fe
L_0151:
	mov ax,[bp-6]
	cwd
	mov cx,[bp-8]
	idiv cx
	mov [bp-1ah],ax
	add WORD PTR D_5156_PENX,ax
	sub [bp-6],ax
	dec word ptr [bp-8]
	jmp	NEAR PTR L_01fe
L_016a:
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],5fh
	jne	L_0184
	mov ax,[bp-14h]
	add ax,si
	inc ax
	cmp ax,di
	jae	L_0184
	add si,[bp-12h]
	jmp	NEAR PTR L_00f0
	db	090h		; nop, as the compiler left it after this jmp
L_0184:
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],5fh
	jne	L_0190
	jmp	NEAR PTR L_00c1
L_0190:
	mov bx,cx
	add bx,dx
	cmp byte ptr [bx],7bh
	jne	L_019c
	jmp	NEAR PTR L_00c1
L_019c:
	mov bx,cx
	add bx,dx
	mov bl,[bx]
	sub bh,bh
	mov al,D_50EA_GLYPHWIDTHS-20h[bx]
	sub ah,ah
	inc ax
	sub si,ax
	jmp	NEAR PTR L_00c1
L_01b0:
	mov bx,si
	add bx,di
	cmp byte ptr [bx],7bh
	jne	L_01c0
	add WORD PTR D_5156_PENX,0fh
	jmp	SHORT L_01fe
L_01c0:
	mov bx,si
	add bx,di
	cmp byte ptr [bx],5fh
	je	L_01fe
	cmp WORD PTR D_5158_PENY,0c0h
	jge	L_01eb
	push word ptr [bp+6]
	mov bx,si
	add bx,di
	mov al,[bx]
	sub ah,ah
	sub ax,20h
	push ax
	push WORD PTR D_5156_PENX
	push WORD PTR D_5158_PENY
	call	ULTIMA_1044_GRAP_4E_COPYBITIMAG
L_01eb:
	mov bx,si
	add bx,di
	mov bl,[bx]
	sub bh,bh
	mov al,D_50EA_GLYPHWIDTHS-20h[bx]
	sub ah,ah
	inc ax
	add WORD PTR D_5156_PENX,ax
L_01fe:
	inc si
	cmp [bp-4],si
	je	L_0207
	jmp	NEAR PTR L_0123
L_0207:
	mov [bp-0eh],si
L_020a:
	mov ax,[bp-4]
	mov [bp-0ch],ax
	mov bx,ax
	mov si,[bp+4]
	cmp byte ptr [bx+si],0
	je	L_027b
	cmp byte ptr [bx+si],5fh
	jne	L_0239
	cmp WORD PTR D_5158_PENY,0c0h
	jge	L_0239
	push word ptr [bp+6]
	mov ax,0dh
	push ax
	push WORD PTR D_5156_PENX
	push WORD PTR D_5158_PENY
	call	ULTIMA_1044_GRAP_4E_COPYBITIMAG
L_0239:
	mov word ptr [bp-2],0
	add WORD PTR D_5158_PENY,9
	mov ax,WORD PTR D_5150_REGIONTOP
	cmp WORD PTR D_5158_PENY,ax
	jle	L_025a
	mov ax,WORD PTR D_5152_REGIONBOTTOM
	cmp WORD PTR D_5158_PENY,ax
	jge	L_025a
	mov ax,1
	jmp	SHORT L_025c
L_025a:
	sub ax,ax
L_025c:
	mov [bp-10h],ax
	shl ax,1
	mov [bp-1ch],ax
	mov bx,ax
	mov ax,D_5146_TEXTLEFTX[bx]
	mov WORD PTR D_5156_PENX,ax
	mov ax,D_514C_TEXTRIGHTX[bx]
	sub ax,WORD PTR D_5156_PENX
	mov [bp-0ah],ax
	inc word ptr [bp-0ch]
L_027b:
	mov bx,[bp-0ch]
	mov si,[bp+4]
	cmp byte ptr [bx+si],0
	je	L_0294
	sub di,di
	mov cx,bx
	mov si,[bp-2]
	mov dx,[bp+4]
	jmp	NEAR PTR L_0079
	db	090h		; nop, as the compiler left it after this jmp
L_0294:
	sub ax,ax
	push ax
	call	ULTIMA_0C22_GRAP_0F_SELECTPAGE
	pop si
	pop di
	mov sp,bp
	pop bp
	ret 4

_TEXT	ENDS
	END
