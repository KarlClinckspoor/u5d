; ANPC.ASM: NPC.OVL 12e0-132a, hand-written assembly in the original.
; The last routine of NPC.OVL, linked after npc.c. NPC_12E0(npc, hour): of the four times
; in the NPC's schedule (D_5D5E[npc].time, 16-byte entries, field at +0Ch), which one the
; hour is nearest after, counting back in byte arithmetic. The answer for the fourth time is
; 1, not 3, in the original.
; Written by hand from the original bytes; tools/asmbuild.py checks it.

	TITLE	ANPC
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

	EXTRN	D_5D5E:BYTE

_TEXT	SEGMENT

; int pascal NPC_12E0(int npc [bp+6], char hour [bp+4])
	PUBLIC	NPC_12E0
NPC_12E0:
	push bp
	mov bp,sp
	push si
	push di
	push ds
	lea si,D_5D5E
	mov ax,[bp+6]
	mov cl,4
	shl ax,cl
	add si,ax
	add si,0Ch		; .time[0..3]
	mov ax,[bp+4]
	mov ah,al
	mov bx,ax
	sub al,[si]		; hour - time[i], for the four
	sub ah,[si+1]
	sub bl,[si+2]
	sub bh,[si+3]
	xor dx,dx
	cmp al,ah
	jbe L_1311
	mov al,ah
	inc dx
L_1311:
	cmp al,bl
	jbe L_131a
	mov al,bl
	mov dx,2
L_131a:
	cmp al,bh
	jbe L_1321
	mov dx,1		; (sic)
L_1321:
	mov ax,dx
	pop ds
	pop di
	pop si
	pop bp
	ret 4

_TEXT	ENDS
	END
