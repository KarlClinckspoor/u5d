; PLMGR.ASM: ULTIMA.EXE CS:72e0-7780, hand-written assembly in the original.
; Drafted from the original bytes by tools/asmgen.py and checked by tools/asmbuild.py.

	TITLE	PLMGR
PLMGR_TEXT	SEGMENT  PARA PUBLIC 'CODE'
PLMGR_TEXT	ENDS
PLTAB	SEGMENT  PARA PUBLIC 'CODE'
PLTAB	ENDS
SEG_076C	SEGMENT  PARA PUBLIC 'MARK'
SEG_076C	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST, _BSS, _DATA
	ASSUME	CS: PLMGR_TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP

	EXTRN	L_007b:NEAR
	EXTRN	L_0292:NEAR
	EXTRN	L_0297:NEAR
	EXTRN	__astart:NEAR

PLMGR_TEXT	SEGMENT
	db	0f9h, 0c3h
L_72e2:
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
	add [bx+si],al
L_72f0:
	mov ah,25h
	int 21h
	ret
L_72f5:
	mov ah,35h
	int 21h
	ret
L_72fa:
	call	L_72f5
	call	L_72f0
	ret
L_7301:
	xor dx,dx
L_7303:
	shl ax,1
	rcl dx,1
	dec cl
	jne	L_7303
	ret
L_730c:
	shr dx,1
	rcr ax,1
	dec cl
	jne	L_730c
	ret
L_7315:
	xor si,si
L_7317:
	db	026h, 08bh, 08ch, 006h, 000h	; mov cx,es:[si+6]
	inc cx
	je	L_734d
	db	026h, 08bh, 094h, 00ah, 000h	; mov dx,es:[si+0ah]
	db	026h, 08bh, 085h, 00ah, 000h	; mov ax,es:[di+0ah]
	push ax
	sub ax,dx
	pop ax
	jae	L_7338
	mov ax,dx
	db	026h, 08bh, 095h, 00ch, 000h	; mov dx,es:[di+0ch]
	jmp	SHORT L_733d
L_7338:
	db	026h, 08bh, 094h, 00ch, 000h	; mov dx,es:[si+0ch]
L_733d:
	sub ax,dx
	jae	L_7348
	db	026h, 081h, 0a4h, 006h, 000h, 0ffh, 07fh	; and word ptr es:[si+6],7fffh
L_7348:
	add si,10h
	jmp	SHORT L_7317
L_734d:
	ret
L_734e:
	add [bx+si],al
	db	0cbh	; retf
L_7351:
	mov bp,sp
	mov word ptr [bp+0eh],0ffffh
	mov al,0
	iret
L_735b:
	push ds
	push bx
	mov	bx,SEG_076C
	mov ds,bx
	test word ptr ds:[8ch],0ffffh
	je	L_737c
	push ax
	mov bx,ds:[2]
	push dx
	mov ah,3eh
	int 21h
	pop dx
	pop ax
	mov word ptr ds:[8ch],0
L_737c:
	pop bx
	pop ds
	db	0cbh	; retf
L_737f:
	mov bx,ds:[8ch]
	or al,al
	jne	L_738f
	db	026h, 03bh, 09dh, 00eh, 000h	; cmp bx,es:[di+0eh]
	jne	L_738f
	ret
L_738f:
	db	09ah, 07bh, 000h
	dw	PLMGR_TEXT	; call 072eh:007bh
	db	026h, 08bh, 09dh, 00eh, 000h	; mov bx,es:[di+0eh]
	mov ds:[8eh],bx
L_739d:
	push ax
	mov si,ds:[8eh]
	mov di,92h
L_73a5:
	lods byte ptr es:[si]
	mov [di],al
	inc di
	or al,al
	jne	L_73a5
	pop ax
	push es
	push ds
	pop es
	call	L_7690
	pop es
	or al,al
	je	L_73bc
	stc
	ret
L_73bc:
	mov al,ds:[1]
	xor si,si
	push dx
	mov ah,3dh
	mov dx,92h
	int 21h
	pop dx
	jb	L_73d0
	mov ds:[2],ax
	ret
L_73d0:
	mov al,1
	or si,si
	je	L_73d8
	inc al
L_73d8:
	jmp	SHORT L_739d
L_73da:
	db	026h, 08bh, 09dh, 010h, 000h	; mov bx,es:[di+10h]
	add bx,dx
	db	026h, 08bh, 085h, 012h, 000h	; mov ax,es:[di+12h]
	adc ax,0
	mov cx,4
L_73ec:
	shl bx,1
	rcl ax,1
	loop	L_73ec
	mov cx,ax
	mov dx,bx
	mov bx,ds:[2]
	mov al,0
	push dx
	mov ah,42h
	int 21h
	pop dx
	ret
L_7403:
	push ax
	push dx
	mov ax,10h
	mul cx
	mov cx,0fff0h
	div cx
	mov cx,dx
	pop dx
	pop bx
L_7413:
	or ax,ax
	jne	L_7420
	or cx,cx
	jne	L_741c
	ret
L_741c:
	push ax
	push ax
	jmp	SHORT L_7426
L_7420:
	dec ax
	push ax
	push cx
	mov cx,0fff0h
L_7426:
	push dx
	push bx
	mov ax,ds:[2]
	push cx
	push ds
	mov ds,bx
	mov bx,ax
	xor si,si
	push dx
	mov ah,3fh
	int 21h
	pop dx
	pop ds
	pop cx
	jb	L_7445
	or si,si
	jne	L_7445
	cmp ax,cx
	je	L_7448
L_7445:
	stc
	jmp	SHORT L_7449
L_7448:
	clc
L_7449:
	pop bx
	pop dx
	pop cx
	pop ax
	jb	L_7455
	add bx,0fffh
	jmp	SHORT L_7413
L_7455:
	mov al,3
	or si,si
	je	L_745d
	inc al
L_745d:
	stc
	ret
	db	043h, 06fh, 070h, 079h, 072h, 069h, 067h, 068h
	db	074h, 020h, 028h, 043h, 029h, 020h, 031h, 039h
	db	038h, 034h, 02ch, 020h, 031h, 039h, 038h, 035h
	db	02ch, 020h, 031h, 039h, 038h, 036h, 020h, 062h
	db	079h, 020h, 050h, 068h, 06fh, 065h, 06eh, 069h
	db	078h, 020h, 053h, 06fh, 066h, 074h, 077h, 061h
	db	072h, 065h, 020h, 041h, 073h, 073h, 06fh, 063h
	db	069h, 061h
L_7499:
	je	L_7500
	jae	L_74bd
	dec sp
	je	L_7504
			; push ds (overlapping code)
	db	02eh
L_74a1:
	db	01eh
	mov al,24h
	push cs
	pop ds
	mov dx,71h
	call	L_72fa
	pop ds
	push bx
	push es
	mov	ax,PLTAB
	mov es,ax
	mov al,0
L_74b6:
	push di
	call	L_737f
	pop di
	jae	L_74c0
L_74bd:
	jmp	SHORT L_7515
	db	090h
L_74c0:
	push bx
	db	026h, 08bh, 09dh, 00eh, 000h	; mov bx,es:[di+0eh]
	mov ds:[8ch],bx
	pop bx
	db	026h, 08bh, 095h, 008h, 000h	; mov dx,es:[di+8]
	add dx,3
	shr dx,1
	shr dx,1
	call	L_73da
	db	026h, 08bh, 08dh, 014h, 000h	; mov cx,es:[di+14h]
	db	026h, 08bh, 085h, 00ch, 000h	; mov ax,es:[di+0ch]
	db	026h, 02bh, 085h, 00ah, 000h	; sub ax,es:[di+0ah]
	cmp ax,cx
	jae	L_74ef
	mov cx,ax
L_74ef:
	db	026h, 08bh, 085h, 00ah, 000h	; mov ax,es:[di+0ah]
	xor dx,dx
	call	L_7403
	jb	L_74b6
	xor dx,dx
	call	L_73da
L_7500:
			; mov cx,es:[di+8] (overlapping code)
	db	026h, 08bh, 08dh, 008h
L_7504:
	db	000h
	xor bx,bx
	jcxz	L_7515
	mov dx,es:[0]
	push es
	call	L_7526
	pop es
	jb	L_74b6
L_7515:
	pushf
	pop cx
	pop ds
	pop dx
	mov al,24h
	call	L_72f0
	push cx
	popf
	mov	ax,SEG_076C
	mov ds,ax
	ret
L_7526:
	or bx,bx
	jne	L_754c
	push cx
	push dx
	mov ax,ds
	mov dx,4
	cmp cx,20h
	jbe	L_7539
	mov cx,20h
L_7539:
	push cx
	add cx,3
	shr cx,1
	shr cx,1
	call	L_7403
	pop bx
	pop dx
	pop cx
	jb	L_7571
	mov si,4
L_754c:
	dec bx
	mov bp,[si]
	mov ax,[si+2]
	or bp,bp
	jne	L_755a
	or ax,ax
	je	L_756c
L_755a:
	push es
	push dx
	push word ptr es:[0]
	add ax,dx
	mov es,ax
	pop dx
	add es:[bp],dx
	pop dx
	pop es
L_756c:
	add si,4
	loop	L_7526
L_7571:
	ret
L_7572:
	db	0eah, 097h, 002h
	dw	PLMGR_TEXT	; jmp 072eh:0297h
L_7577:
	cld
	push bp
	push ds
	push es
	mov	bx,SEG_076C
	mov ds,bx
	mov	bx,PLTAB
	mov es,bx
	mov ds:[90h],ah
L_7589:
	mov ax,10h
	dec cx
	mul cx
	mov di,ax
	db	026h, 0f7h, 085h, 006h, 000h, 000h, 080h	; test word ptr es:[di+6],8000h
	jne	L_75a6
	push cx
	call	L_7315
	pop cx
	call	L_74a1
	jb	L_75ba
	jmp	SHORT L_75a6
L_75a6:
	db	026h, 081h, 08dh, 006h, 000h, 000h, 080h	; or word ptr es:[di+6],8000h
	db	026h, 08bh, 08dh, 006h, 000h	; mov cx,es:[di+6]
	and cx,1fffh
	jne	L_7589
	jmp	SHORT L_75c5
L_75ba:
	test byte ptr ds:[90h],0ffh
	jne	L_75c4
	call	L_76b4
L_75c4:
	stc
L_75c5:
	call	L_76ac
	pop es
	pop ds
	pop bp
	db	0cbh	; retf
; $OVLYV (OVLYV's public): the entry the overlay call stubs call (PLMGR_TEXT:02ec); tools/u5link.py
; writes the stubs with it.
	PUBLIC	$OVLYV
$OVLYV:
L_75cc:
	pushf
	push ax
	push bx
	push cx
	push dx
	push bp
	push si
	push di
	push ds
	push es
	mov bp,sp
	mov	ax,PLTAB
	mov es,ax
	mov ax,[bp+16h]
	mov ds,ax
	mov si,[bp+14h]
	add word ptr [bp+14h],2
	mov cx,[si]
	mov	ax,SEG_076C
	mov ds,ax
	mov ah,0
	db	09ah, 092h, 002h
	dw	PLMGR_TEXT	; call 072eh:0292h
	pop es
	pop ds
	pop di
	pop si
	pop bp
	pop dx
	pop cx
	pop bx
	pop ax
	popf
	db	0cbh	; retf
; PLINK86's start-up (072e:0322): the linked program's entry point, before Origin's wrapper
; (src/asm/PLSTART.ASM) took it. tools/u5link.py computes the EXE checksum with it.
	PUBLIC	PLINK_START
PLINK_START:
L_7602:
	cli
	mov di,es
	mov si,ss
	mov cx,sp
	mov	dx,PLTAB
	mov es,dx
	mov ss,es:[4]
	mov sp,100h
	sti
	mov es,di
	push si
	push cx
	push ax
	push ds
	push es
	mov cs:[6eh],es
	mov	ax,SEG_076C
	mov ds,ax
	call	L_76a1
	mov	ax,PLTAB
	mov es,ax
	mov ax,cs
	sub ax,es:[0]
	mov es:[0],ax
	cmp byte ptr ds:[0],0ffh
	je	L_764e
	push dx
	mov ah,9
	mov dx,9fh
	int 21h
	pop dx
	call	L_76b4
L_764e:
	mov	dx,PLTAB
	mov es,dx
	xor di,di
	mov cx,1
L_7658:
	db	026h, 08bh, 085h, 006h, 000h	; mov ax,es:[di+6]
	inc ax
	je	L_767f
	db	026h, 0f7h, 085h, 006h, 000h, 000h, 040h	; test word ptr es:[di+6],4000h
	je	L_7679
	push di
	push cx
	mov ah,0
	db	09ah, 092h, 002h
	dw	PLMGR_TEXT	; call 072eh:0292h
	pop cx
	pop di
	jae	L_7679
	call	L_76b4
L_7679:
	inc cx
	add di,10h
	jmp	SHORT L_7658
L_767f:
	pop es
	pop ds
	pop ax
	pop cx
	pop bx
	cli
	mov sp,cx
	mov ss,bx
	sti
	db	0eah
	dw	OFFSET __astart
	dw	SEG __astart
	db	000h
L_7690:
	cld
	or al,al
	je	L_76a0
	push ds
	push ss
	pop ds
	call dword ptr ss:[5394h]
	pop ds
	xor al,al
L_76a0:
	ret
L_76a1:
	mov byte ptr ds:[0],0ffh
	mov byte ptr ds:[1],0
	ret
L_76ac:
	pushf
	db	09ah, 07bh, 000h
	dw	PLMGR_TEXT	; call 072eh:007bh
	popf
	ret
L_76b4:
	db	09ah, 07bh, 000h
	dw	PLMGR_TEXT	; call 072eh:007bh
	mov al,0ffh
	mov ah,4ch
	int 21h
	ret
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
	db	000h, 000h, 000h, 000h, 090h, 000h, 000h, 000h
	db	002h, 000h, 0fch, 008h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	045h, 072h, 072h, 06fh, 072h, 02eh, 02eh, 02eh
	db	043h, 061h, 06ch, 06ch, 020h, 04fh, 053h, 049h
	db	024h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h

PLMGR_TEXT	ENDS
	END
