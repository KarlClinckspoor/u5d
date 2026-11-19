; A1B94.ASM: ULTIMA.EXE CS:1b94-1dda, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A1B94
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

	EXTRN	L_15c6:NEAR
	EXTRN	L_160e:NEAR
	EXTRN	L_1654:NEAR
	EXTRN	ULTIMA_1B24_CLEARKBDBUFFERIMPL:NEAR
	EXTRN	DOSERROR:WORD
	EXTRN	TEXTWINDOWS:BYTE
	EXTRN	TEXTWINDOWINDEX:WORD
	EXTRN	CHARSETINDEX:WORD
	EXTRN	NUMPADKEYS:WORD
	EXTRN	KEYBUFFEROFF:WORD
	EXTRN	CURRENTCHARSET:WORD
	EXTRN	CURRENTTEXTWINDOW:WORD
	EXTRN	UNDERLINE:WORD
	EXTRN	CENTERTEXT:WORD
	EXTRN	INVERSE:WORD
	EXTRN	TEXTFGCOLOR:BYTE
	EXTRN	TEXTBGCOLOR:BYTE

; this module's own data in DGROUP (DS:540e-541e)
_DATA	SEGMENT
	PUBLIC	ARROWKEYCODES
ARROWKEYCODES	LABEL	BYTE
	db	04Bh, 04Dh, 048h, 050h, 047h, 04Fh, 049h, 051h	; KMHPGOIQ
	PUBLIC	SPECIALKEYTABLE
SPECIALKEYTABLE	LABEL	BYTE
	db	001h, 002h, 003h, 004h, 0D3h, 0D4h, 0D5h, 0D6h
_DATA	ENDS

_TEXT	SEGMENT

	PUBLIC	SELECTTEXTWINDOW
SELECTTEXTWINDOW:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov bx,[bp+4]
	cmp bx,3
	ja	L_1beb
	mov WORD PTR TEXTWINDOWINDEX,bx
	mov cl,3
	shl bx,cl
	lea si,TEXTWINDOWS
	add si,bx
	mov WORD PTR CURRENTTEXTWINDOW,si
	add si,6
	lodsb
	mov ah,al
	and al,0fh
	mov BYTE PTR TEXTFGCOLOR,al
	and ah,0f0h
	inc cl
	shr ah,cl
	mov BYTE PTR TEXTBGCOLOR,ah
	lodsb
	xor ah,ah
	mov bx,ax
	mov cx,ax
	and ax,1
	mov WORD PTR UNDERLINE,ax
	db	083h, 0e3h, 002h	; and bx,2
	shr bx,1
	mov WORD PTR CENTERTEXT,bx
	db	083h, 0e1h, 004h	; and cx,4
	shr cx,1
	shr cx,1
	mov WORD PTR INVERSE,cx
L_1beb:
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	SETTEXTPOSITION
SETTEXTPOSITION:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+6]
	mov bx,[bp+4]
	mov si,WORD PTR CURRENTTEXTWINDOW
	mov cl,al
	mov dl,bl
	add cl,[si]
	add dl,[si+1]
	cmp cl,27h
	ja	L_1c1b
	cmp dl,18h
	ja	L_1c1b
	mov [si+4],al
	mov [si+5],bl
L_1c1b:
	pop ds
	pop di
	pop si
	pop bp
	ret 4

	PUBLIC	SETTEXTWINDOWSIZE
SETTEXTWINDOWSIZE:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov bx,[bp+0ch]
	cmp bx,4
	jae	L_1c54
	mov cl,3
	shl bx,cl
	lea si,TEXTWINDOWS
	add si,bx
	mov ax,[bp+0ah]
	mov bx,[bp+8]
	mov cx,[bp+6]
	mov dx,[bp+4]
	call	ULTIMA_1C5B_CLIPTEXTWINDOW
	mov [si],al
	mov [si+1],bl
	mov [si+2],cl
	mov [si+3],dl
L_1c54:
	pop ds
	pop di
	pop si
	pop bp
	ret 0ah

	PUBLIC	ULTIMA_1C5B_CLIPTEXTWINDOW
ULTIMA_1C5B_CLIPTEXTWINDOW:
	cmp al,0
	jge	L_1c61
	mov al,0
L_1c61:
	cmp al,27h
	jle	L_1c67
	mov al,27h
L_1c67:
	cmp bl,0
	jge	L_1c6e
	mov bl,0
L_1c6e:
	cmp bl,18h
	jle	L_1c75
	mov bl,18h
L_1c75:
	cmp cl,0
	jge	L_1c7c
	mov cl,0
L_1c7c:
	cmp cl,27h
	jle	L_1c83
	mov cl,27h
L_1c83:
	cmp dl,0
	jge	L_1c8a
	mov dl,0
L_1c8a:
	cmp dl,18h
	jle	L_1c91
	mov dl,18h
L_1c91:
	cmp al,cl
	jle	L_1c9d
	xchg al,cl
	cmp bl,dl
	jle	L_1c9d
	xchg bl,dl
L_1c9d:
	ret

	PUBLIC	SELECTCHARSET
SELECTCHARSET:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov bx,[bp+4]
	cmp bx,4
	jae	L_1cc2
	shl bx,1
	cmp word ptr [bx+539ch],0
	je	L_1cc2
	mov ax,[bx+539ch]
	mov WORD PTR CURRENTCHARSET,ax
	shr bx,1
	mov WORD PTR CHARSETINDEX,bx
L_1cc2:
	pop ds
	pop di
	pop si
	pop bp
	ret 2
	db	000h

	PUBLIC	SETTEXTFOREGROUNDCOLOR
SETTEXTFOREGROUNDCOLOR:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,WORD PTR CURRENTTEXTWINDOW
	mov ax,[bp+4]
	and al,0fh
	mov BYTE PTR TEXTFGCOLOR,al
	mov ah,[si+6]
	and ah,0f0h
	or ah,al
	mov [si+6],ah
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	GETCURRENTTEXTY
GETCURRENTTEXTY:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov si,WORD PTR CURRENTTEXTWINDOW
	mov al,[si+5]
	xor ah,ah
	pop ds
	pop di
	pop si
	pop bp
	ret

	PUBLIC	LOADCHARSET
LOADCHARSET:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov ax,[bp+4]
	cmp ax,4
	jl	L_1d15
	mov ax,0fffeh
	jmp	SHORT L_1d57
L_1d15:
	mov dx,[bp+6]
	call	L_1654
	jb	L_1d4e
	push ax
	mov bx,ax
	or ax,ax
	je	L_1d2a
	dec bx
	mov cl,4
	shr bx,cl
	inc bx
L_1d2a:
	call	L_15c6
	pop bx
	jb	L_1d49
	mov dx,[bp+6]
	push bx
	push ax
	call	L_160e
	pop bx
	pop cx
	jb	L_1d4e
	mov si,[bp+4]
	shl si,1
	mov [si+539ch],bx
	mov ax,cx
	jmp	SHORT L_1d57
L_1d49:
	mov ax,0ffffh
	jmp	SHORT L_1d57
L_1d4e:
	mov ax,WORD PTR DOSERROR
	call dword ptr ds:[5394h]
	xor ax,ax
L_1d57:
	pop ds
	pop di
	pop si
	pop bp
	ret 2

	PUBLIC	POLLKEY
POLLKEY:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	mov WORD PTR NUMPADKEYS,0
	mov ah,1
	int 16h
	jne	L_1d86
L_1d70:
	xor ax,ax
	pop ds
	pop di
	pop si
	pop bp
	ret
L_1d77:
	cmp WORD PTR KEYBUFFEROFF,0
	je	L_1d81
	call	ULTIMA_1B24_CLEARKBDBUFFERIMPL
L_1d81:
	pop ds
	pop di
	pop si
	pop bp
	ret
L_1d86:
	mov dl,0ffh
	mov ah,6
	int 21h
	xor ah,ah
	or al,al
	jne	L_1dc2
	mov dl,0ffh
	mov ah,6
	int 21h
	je	L_1d70
	xor ah,ah
	cmp al,3bh
	jb	L_1d70
	cmp al,44h
	ja	L_1da8
	add al,8eh
	jmp	SHORT L_1d77
L_1da8:
	xor bx,bx
L_1daa:
	cmp al,ARROWKEYCODES[bx]
	je	L_1db8
	inc bx
	cmp bx,8
	jne	L_1daa
	je	L_1d70
L_1db8:
	mov al,SPECIALKEYTABLE[bx]
	inc WORD PTR NUMPADKEYS
	jmp	SHORT L_1d77
L_1dc2:
	cmp al,31h
	jb	L_1d77
	cmp al,39h
	ja	L_1d77
	push ax
	mov ah,2
	int 16h
	and al,23h
	je	L_1dd7
	inc WORD PTR NUMPADKEYS
L_1dd7:
	pop ax
	jmp	SHORT L_1d77

_TEXT	ENDS
	END
