; PLSTART.ASM: ULTIMA.EXE CS:81d0-86f0, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.
; This is Origin's copy-protection wrapper, not PLINK86's: it was appended to the linked
; program and took the entry point (081d:0000). The linked program starts at PLINK86's
; start-up, PLMGR's PLINK_START (072e:0322), with the stack after DGROUP (1b38:0800).
; The releases differ in the first 35 bytes (81d0-81f3, IFDEF RELEASE_1988 below): the
; 1988 file enters the wrapper (a jump over its variables, then a call into it); the January
; 1992 patch (the GOG release) put a stub there that sets that stack and returns to
; PLINK_START, so the rest of the wrapper is never run. The rest is the same in both.

	TITLE	PLSTART
PLSTART_TEXT	SEGMENT  PARA PUBLIC 'CODE'
PLSTART_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST, _BSS, _DATA
	ASSUME	CS: PLSTART_TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP


PLSTART_TEXT	SEGMENT
L_81d0:
IFDEF RELEASE_1988
; 1988: into the wrapper, past its variables
	jmp	SHORT L_81dc
	db	000h, 000h, 000h, 012h, 000h
L_81d7:
	db	0eah, 000h, 000h, 000h, 000h	; jmp 0:0 (the wrapper's way out)
L_81dc:
	pushf
	push ax
	push cx
	call	L_83e0
	db	0c2h, 0dah, 050h, 017h, 052h, 01fh, 052h, 007h
	db	068h, 0c5h, 011h, 0fdh, 0e0h, 0deh, 0a1h, 0cch
	db	056h
ELSE
; 1992 patch (GOG): SS:SP = (load + 1b38):0800, return to PLINK_START (load + 072e):0322
	mov ax,ds
	mov es,ax
	add ax,1b48h
L_81d7:
	mov ss,ax
	mov sp,800h
	sub ax,140ah
	push ax
	mov ax,322h
	push ax
	db	031h, 0c0h	; xor ax,ax
	db	089h, 0c3h	; mov bx,ax
	db	089h, 0c1h	; mov cx,ax
	db	089h, 0c2h	; mov dx,ax
	db	089h, 0c5h	; mov bp,ax
	db	089h, 0c6h	; mov si,ax
	db	089h, 0c7h	; mov di,ax
	db	0cbh	; retf
ENDIF
	db	00ch, 034h, 099h, 0dch, 05ch, 020h, 0dch, 0a8h
	db	0dfh, 035h, 01bh, 0dch, 035h, 0aeh, 0ddh, 06eh
	db	0dch, 034h
L_8205:
	db	0e9h, 0dch, 05ch	; jmp 5d14h
	db	020h, 0dch, 0a8h, 02fh, 06eh, 0ddh, 034h
L_820f:
	neg sp
	pop sp
	db	020h, 0dch	; and ah,bl
	test al,35h
	push cx
	db	0cah, 090h, 0ddh	; retf 0dd90h
	db	068h, 0d5h, 011h, 0fdh, 068h, 0dch, 011h
L_8221:
	db	0cah, 06eh, 0dch	; retf 0dc6eh
	db	034h, 0c9h, 0dch, 05ch, 020h, 0dch, 0a8h, 00fh
	db	06eh, 0ddh, 034h, 0d7h, 0dch, 05ch, 020h, 0dch
	db	0a8h, 015h, 035h, 051h, 0dch, 035h, 055h, 0dch
	db	064h, 0dch, 0dch, 052h, 01ch, 06ah, 0dch, 067h
	db	0a4h, 0dch, 0fah, 057h, 0dbh, 057h, 014h, 067h
	db	0a6h, 0dch, 0fah, 057h, 0dbh, 052h, 01ch, 057h
	db	005h, 06ch, 0dfh, 0fah, 054h, 09bh, 0dfh, 06ch
	db	05ch, 0fah, 054h, 09bh, 0d9h, 050h, 017h, 052h
	db	007h, 052h, 01fh, 067h, 0dch, 0dch, 065h, 0d9h
	db	0dch, 08dh, 06dh, 0deh, 069h, 0dah, 06ch, 0d8h
	db	068h, 0d8h, 011h, 0cfh, 085h, 05ch, 020h, 0dch
	db	0a8h, 0d9h, 03eh, 031h, 037h, 09fh, 04ch, 064h
	db	0dch, 0dch, 052h, 01ch, 067h, 0a4h, 0dch, 0fah
	db	057h, 0dbh, 057h, 014h, 067h, 0a6h, 0dch, 0fah
	db	057h, 0dbh, 052h, 01ch, 057h, 005h, 06ch, 0deh
	db	0fah, 054h, 09bh, 0dfh, 06ch, 0c6h, 0fah, 054h
	db	09bh, 0d9h, 050h, 017h, 052h, 01fh, 06ah, 0dch
	db	067h, 0dch, 0dch, 065h, 0d9h, 0dch, 08dh, 06dh
	db	067h, 069h, 0dah, 06ch, 0d8h, 068h, 0d8h, 011h
	db	0cfh, 085h
L_82be:
	pop sp
	db	020h, 0dch	; and ah,bl
	test al,0deh
	db	03eh, 031h, 01fh	; xor [bx],bx
	push ax
	pop ss
	push dx
	pop es
	push cx
	db	0cah, 0fbh, 0ddh	; retf 0ddfbh
	db	068h, 0d5h, 011h, 0fdh
L_82d2:
	db	067h, 0dch, 0dch, 052h, 01fh, 067h, 0a4h, 0dch
	db	0fah, 057h, 0dbh, 057h, 014h, 067h, 0a6h, 0dch
	db	0fah, 057h, 0dbh, 052h, 01ch, 06ch, 0deh, 0fah
	db	054h, 09bh, 0deh, 06ch, 0f6h, 0fah, 054h, 09bh
	db	0d9h, 068h, 090h, 011h, 0fdh, 0d1h, 0d6h, 0d1h
	db	0d6h, 08ch, 090h, 099h, 09dh, 08fh, 099h, 0fch
	db	089h, 08fh, 099h, 0fch, 093h, 08eh, 095h, 09bh
	db	095h, 092h, 09dh, 090h, 0fch, 098h, 095h, 08fh
	db	097h, 0fch, 093h, 092h, 090h, 085h, 0f2h, 0d1h
	db	0d6h, 0f8h, 0d1h, 0d6h, 0d1h, 0d6h, 08ch, 090h
	db	099h, 09dh, 08fh, 099h, 0fch, 095h, 092h, 08fh
	db	099h, 08eh, 088h, 0fch, 093h, 08eh, 095h, 09bh
	db	095h, 092h, 09dh, 090h, 0fch, 098h, 095h, 08fh
	db	097h, 0fch, 095h, 092h, 0fch, 098h, 08eh, 095h
	db	08ah, 099h, 0fch, 09dh, 0fch, 093h, 08eh, 0fch
	db	09eh, 0d1h, 0d6h, 088h, 094h, 099h, 092h, 0fch
	db	08ch
L_8353:
	mov ds,[bx+di-7071h]
	cld
	popf
	xchg dx,ax
	db	085h, 0fch	; test sp,di
	xchg di,ax
	cwd
	db	085h, 0fch	; test sp,di
	mov [bp+di-6004h],dl
	xchg bx,ax
	xchg dx,ax
	mov [di-766eh],dl
	cwd
	cld
	db	0f2h, 0fch	; cld
	db	0f2h, 0fch	; cld
	db	0f2h, 0d1h, 0d6h	; rcl si,1
	clc
	push ax
	pop ss
	push dx
	pop es
	push dx
	pop ds
	push cx
	ret 0dfc4h
	db	065h, 0d9h, 0dch, 08dh, 06dh, 0ddh, 069h, 0dah
	db	06ch, 0ddh, 068h, 0deh, 011h, 0cfh, 085h
L_838d:
	pop sp
	db	020h, 0dch	; and ah,bl
	test al,0d9h
	db	03eh, 031h, 035h	; xor [di],si
	db	0f3h, 023h, 0dbh	; and bx,bx
	ret
	db	057h, 02fh, 05dh, 01ah
L_839d:
	db	0dch, 0ddh, 0f2h, 0dfh
L_83a1:
	db	0eah, 0deh, 0dch, 0c2h, 050h	; jmp 50c2h:0dcdeh
	db	007h, 05fh, 01fh, 0cch, 050h, 014h, 052h, 01ch
	db	052h, 004h, 063h, 0d4h, 0dch, 071h, 077h, 071h
	db	0dfh, 01fh, 077h, 071h, 057h, 014h
L_83bc:
	jno	L_839d
	pop ds
	xchg ax,[bp+si-62a3h]
	call	L_83e0
	mov ds,bx
	mov es,bx
	mov ss,ax
	mov sp,cx
	mov ax,bp
	mov cx,si
	mov bx,0
	mov dx,bx
	mov bp,dx
	mov si,dx
	mov di,dx
	jmp	NEAR PTR L_81d7
L_83e0:
	pushf
	push ds
	push es
	push ax
	push bx
	push cx
	push dx
	mov bx,cs
	mov ds,bx
	mov es,bx
	lea bx,ds:[12h]
	lea cx,ds:[1f3h]
	sub cx,2
	sub cx,bx
L_83fa:
	mov al,[bx]
	xor al,0dch
	mov [bx],al
	inc bx
	loop	L_83fa
	pop dx
	pop cx
	pop bx
	pop ax
	pop es
	pop ds
	popf
	ret
L_840b:
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	pushf
	push cx
	push dx
	push ax
	call	L_84dd
	push dx
	push cx
	cmp al,0
	je	L_843e
	add dx,0b0h
	jae	L_8439
	inc cx
L_8439:
	add cx,18h
	jb	L_8468
L_843e:
	sub dx,cs:[316h]
	jae	L_8446
	dec cx
L_8446:
	sub cx,cs:[314h]
	jb	L_8468
	mov cs:[29bh],cx
	mov cs:[29dh],dx
	pop cx
	pop dx
	mov cs:[314h],cx
	mov cs:[316h],dx
	pop ax
	pop dx
	pop cx
	popf
	ret
L_8468:
	jmp	NEAR PTR L_82d2
L_846b:
	push es
	db	000h, 040h, 000h	; add [bx+si],al
	pushf
	push cx
	push dx
	mov cx,cs:[29bh]
	cmp cx,0
	jne	L_8468
	mov cx,cs:[29dh]
	cmp cx,cs:[2bch]
	jae	L_8468
	pop dx
	pop cx
	popf
	ret
L_848c:
	nop
	db	000h, 0e8h	; add al,ch
	dec sp
	add ds:[0e89h],ch
	adc al,3
	mov cs:[316h],dx
	mov ax,0
	mov es,ax
	mov ax,es:[4eh]
	cmp ax,0bfffh
	jle	L_84ad
	mov ax,es:[12h]
L_84ad:
	mov cx,ax
	mov ax,es:[6]
	cmp al,0f0h
	jge	L_84be
	cmp cx,ax
	je	L_84be
	jmp	NEAR PTR L_82d2
L_84be:
	mov ax,es:[0eh]
	cmp al,0f0h
	jge	L_84cd
	cmp ax,cx
	je	L_84cd
	jmp	NEAR PTR L_82d2
L_84cd:
	lea bx,ds:[23ah]
	mov ax,0c3h
	mov [bx],ax
	mov bx,cs
	mov ds,bx
	mov es,bx
	ret
L_84dd:
	mov ah,0
	pushf
	int 1ah
	popf
	ret
	db	020h, 000h, 000h, 002h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h

PLSTART_TEXT	ENDS
	END
