; OVUSER.ASM: Origin's replacement for OVERLAY.LIB's OVUSERM (the overlay loader's user
; module: $OVLYMU when an overlay file is not found, $OVLYIU at start, $OVLEND, $OVEXIT,
; the file name buffer and the DOS 2 flag), as ULTIMA.EXE has it at CS:7690-76c0 (code) and
; OVDATA 0092-00b1 (CS:7752). Written from the original's bytes for the PLINK experiment.

	TITLE	OVUSER
OVCODE	SEGMENT  PARA PUBLIC 'OVLOADER'
OVCODE	ENDS
OVDATA	SEGMENT  WORD PUBLIC 'OVLOADER'
OVDATA	ENDS
	EXTRN	D_5394_DISKSWAPFN:DWORD	; in DGROUP (A0DE0): no segment of DGROUP here
OVCODE	SEGMENT
	EXTRN	$OVCLOSE:FAR
OVCODE	ENDS
OVDATA	SEGMENT
	EXTRN	$OVDOS2:BYTE, $OVOM$:BYTE
OVDATA	ENDS

	PUBLIC	$OVLYMU, $OVLYIU, $OVLEND, $OVEXIT, $OVLYFN, $NEEDDOS2

OVCODE	SEGMENT
	ASSUME	CS:OVCODE, DS:OVDATA
$OVLYMU	PROC	NEAR
	cld
	or	al,al
	je	L1
	push	ds
	push	ss
	pop	ds
	call	ss:D_5394_DISKSWAPFN
	pop	ds
	xor	al,al
L1:	ret
$OVLYMU	ENDP
$OVLYIU	PROC	NEAR
	mov	$OVDOS2,0ffh
	mov	$OVOM$,0
	ret
$OVLYIU	ENDP
$OVLEND	PROC	NEAR
	pushf
	call	$OVCLOSE
	popf
	ret
$OVLEND	ENDP
$OVEXIT	PROC	NEAR
	call	$OVCLOSE
	mov	al,0ffh
	mov	ah,4ch
	int	21h
	ret
$OVEXIT	ENDP
OVCODE	ENDS

OVDATA	SEGMENT
$OVLYFN		db	13 dup (0)
$NEEDDOS2	db	0
		db	'Error...Call OSI$'
OVDATA	ENDS
	END
