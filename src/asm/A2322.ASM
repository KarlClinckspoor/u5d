; A2322.ASM: ULTIMA.EXE CS:2322-251e, DISKSWAPMESSAGE as the GOG release has it.
; In 1.16 this was MSC 5.1 output, the second function of diskswap.c's object; GOG patched it in
; place (2405-240c: the default drive is taken instead of asking for a disk; 24aa-24ac: the
; drive letter is no longer echoed), so its bytes are no compiler's output. The DOS build
; of the GOG target takes it from here. The 1988 target leaves this module out
; (tools/u5.py ASM_EXCLUDED; the link script's lines for it say target=gog) and compiles
; 1.16's C in src/diskswap.c (RELEASE_1988) instead, with LASTDISK and the strings.
; Its _DATA is the object's: LASTDISK (the disk last selected) and the strings, DS:545e-54b6.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	A2322
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

	EXTRN	ULTIMA_0A70_GRAP_2D_SETPENCOLOR:NEAR
	EXTRN	ULTIMA_0AA6_GRAP_3F_FILLRECT:NEAR
	EXTRN	GRAP_0F_SELECTPAGE:NEAR
	EXTRN	GRAP_1B_TRANSFERFULLSCREEN:NEAR
	EXTRN	GETDEFAULTDRIVE:NEAR
	EXTRN	ULTIMA_16BA_PRINTCHAR:NEAR
	EXTRN	ULTIMA_1850_PRINTSTRING:NEAR
	EXTRN	SETTEXTWINDOWSIZE:NEAR
	EXTRN	WAITFORKEYSTROKE:NEAR
	EXTRN	SETDEFAULTDRIVE:NEAR
	EXTRN	TOUPPERCHAR:NEAR
	EXTRN	DISKSWAPFN:WORD
	EXTRN	NODISKSWAPMESSAGE:FAR
	EXTRN	CURDISK:BYTE
	EXTRN	FLOPPYCOUNT:BYTE
	EXTRN	DISKDRIVES:BYTE

; this module's own data in DGROUP (DS:545e-54b6)
_DATA	SEGMENT
	PUBLIC	LASTDISK
LASTDISK	db	0ffh		; the disk (CURDISK[0]) last selected, 0ffh: none
D_545F	db	0ah, 'Please insert the Ultima ', 00h	; printed by 1.16 at 2405
	PUBLIC	D_547A
D_547A	db	'V ', 00h
	PUBLIC	D_547D
D_547D	db	'Program', 00h
	PUBLIC	D_5485
D_5485	db	'Britannia', 00h
	PUBLIC	D_548F
D_548F	db	'Player', 00h
	PUBLIC	D_5496
D_5496	db	' Disk', 00h
	PUBLIC	D_549C
D_549C	db	' and press drive letter: ', 00h
_DATA	ENDS

_TEXT	SEGMENT

	PUBLIC	DISKSWAPMESSAGE
DISKSWAPMESSAGE:
	push bp
	mov bp,sp
	sub sp,4
	push si
	mov WORD PTR DISKSWAPFN,OFFSET NODISKSWAPMESSAGE
	mov	WORD PTR DISKSWAPFN+2,_TEXT
	mov bl,BYTE PTR CURDISK
	sub bh,bh
	mov al,[bx-5638h]
	mov [bp-4],al
	cmp al,0ffh
	je	L_2374
	mov al,bl
	cmp BYTE PTR LASTDISK,al
	je	L_2374
	call	GETDEFAULTDRIVE
	mov cl,[bp-4]
	sub ch,ch
	cmp ax,cx
	je	L_2374
	mov bl,BYTE PTR CURDISK
	sub bh,bh
	mov al,[bx-5638h]
	sub ah,ah
	push ax
	call	SETDEFAULTDRIVE
	mov al,BYTE PTR CURDISK
L_236d:
	mov BYTE PTR LASTDISK,al
	jmp	NEAR PTR L_250c
	db	090h
L_2374:
	mov al,BYTE PTR CURDISK
	sub ah,ah
	mov si,ax
	add si,0a9c8h
	cmp byte ptr [si],43h
	jae	L_239d
	cmp BYTE PTR FLOPPYCOUNT,1
	jbe	L_239d
	xor byte ptr [si],3
	mov al,[si]
	push ax
	call	SETDEFAULTDRIVE
	mov al,BYTE PTR CURDISK
	cmp BYTE PTR LASTDISK,al
	jne	L_236d
L_239d:
	mov al,BYTE PTR CURDISK
	sub ah,ah
	mov si,ax
	add si,0a9c8h
	cmp byte ptr [si],43h
	jb	L_23c4
	cmp byte ptr [si],0ffh
	je	L_23c4
	mov byte ptr [si],41h
	mov ax,41h
	push ax
	call	SETDEFAULTDRIVE
	mov BYTE PTR LASTDISK,0ffh
	jmp	NEAR PTR L_250c
L_23c4:
	cmp BYTE PTR CURDISK+1,2
	jne	L_2400
	sub ax,ax
	push ax
	call	GRAP_0F_SELECTPAGE
	sub ax,ax
	push ax
	mov ax,6
	push ax
	mov ax,0ch
	push ax
	mov ax,21h
	push ax
	mov ax,0dh
	push ax
	call	SETTEXTWINDOWSIZE
	sub ax,ax
	push ax
	call	ULTIMA_0A70_GRAP_2D_SETPENCOLOR
	mov ax,28h
	push ax
	mov ax,58h
	push ax
	mov ax,117h
	push ax
	mov ax,77h
	push ax
	call	ULTIMA_0AA6_GRAP_3F_FILLRECT
L_2400:
	mov BYTE PTR LASTDISK,0ffh
; GOG: the drive is the default one, and no key is read (in 1.16: mov ax,OFFSET D_545F;
; push ax; call ULTIMA_1850_PRINTSTRING)
	call	GETDEFAULTDRIVE
	jmp	SHORT L_247d
	db	044h, 0f4h	; the rest of 1.16's call ULTIMA_1850_PRINTSTRING
	cmp BYTE PTR CURDISK,4
	jne	L_241a
	mov ax,49h
	push ax
	call	ULTIMA_16BA_PRINTCHAR
L_241a:
	mov ax,OFFSET DGROUP:D_547A
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov al,BYTE PTR CURDISK
	sub ah,ah
	or ax,ax
	je	L_243c
	cmp ax,1
	je	L_2460
	cmp ax,3
	je	L_2460
	cmp ax,4
	je	L_2466
	jmp	SHORT L_2443
	db	090h
L_243c:
	mov ax,OFFSET DGROUP:D_547D
L_243f:
	push ax
	call	ULTIMA_1850_PRINTSTRING
L_2443:
	mov ax,OFFSET DGROUP:D_5496
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov bl,BYTE PTR CURDISK
	sub bh,bh
	cmp byte ptr [bx-5638h],0ffh
	jne	L_246c
	mov ax,OFFSET DGROUP:D_549C
	push ax
	call	ULTIMA_1850_PRINTSTRING
	jmp	SHORT L_2473
L_2460:
	mov ax,OFFSET DGROUP:D_5485
	jmp	SHORT L_243f
	db	090h
L_2466:
	mov ax,OFFSET DGROUP:D_548F
	jmp	SHORT L_243f
	db	090h
L_246c:
	mov ax,2eh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
L_2473:
	sub ax,ax
	push ax
	call	WAITFORKEYSTROKE
	push ax
	call	TOUPPERCHAR
L_247d:
	mov [bp-2],al
	mov bl,BYTE PTR CURDISK
	sub bh,bh
	cmp byte ptr [bx-5638h],0ffh
	jne	L_2497
	sub ah,ah
	push ax
	call	SETDEFAULTDRIVE
	or ax,ax
	je	L_2473
L_2497:
	mov bl,BYTE PTR CURDISK
	sub bh,bh
	cmp byte ptr [bx-5638h],0ffh
	jne	L_24d4
	mov al,[bp-2]
	sub ah,ah
	push ax
; GOG: 1.16's call ULTIMA_16BA_PRINTCHAR (echo the drive letter) made three nops
	nop
	nop
	nop
	cmp byte ptr [bp-2],42h
	jne	L_24b8
	mov BYTE PTR FLOPPYCOUNT,2
L_24b8:
	mov bl,BYTE PTR CURDISK
	sub bh,bh
	mov al,[bp-2]
	mov [bx-5638h],al
	cmp bl,3
	jne	L_24d4
	cmp BYTE PTR DISKDRIVES+1,0ffh
	jne	L_24d4
	mov BYTE PTR DISKDRIVES+1,al
L_24d4:
	cmp BYTE PTR CURDISK+1,0
	jne	L_24e4
	mov ax,0ah
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	jmp	SHORT L_250c
L_24e4:
	cmp BYTE PTR CURDISK+1,2
	jne	L_250c
	mov ax,1
	push ax
	call	GRAP_0F_SELECTPAGE
	sub ax,ax
	push ax
	push ax
	push ax
	mov ax,27h
	push ax
	mov ax,18h
	push ax
	call	SETTEXTWINDOWSIZE
	mov ax,1
	push ax
	sub ax,ax
	push ax
	call	GRAP_1B_TRANSFERFULLSCREEN
L_250c:
	mov WORD PTR DISKSWAPFN,OFFSET DISKSWAPMESSAGE
	mov	WORD PTR DISKSWAPFN+2,_TEXT
	pop si
	mov sp,bp
	pop bp
	db	0cbh	; retf
	db	090h

_TEXT	ENDS
	END
