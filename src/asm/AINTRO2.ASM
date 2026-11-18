; AINTRO2.ASM: INTRO.OVL 132a-1e22, INTRO_132a_TransferFromU4 as the GOG release has it.
; In 1.16 this was MSC 5.1 output, the last function of intro2.c's object; GOG patched it in
; place (1332-134d: the map id and the drives; 1392-1399: the disk prompt is skipped), so its
; bytes are no compiler's output. The DOS build takes it from here, linked after intro2.c and
; before introb.c; the C in src/intro2.c is kept for the native build.
; Its strings stay where the object had them, after the rest of intro2.c's (DS:3345-3655):
; this module's _DATA is BYTE-aligned so that it follows intro2.c's packed strings directly.
; The two words that hold the code segment (13e3, 1419) are INTRO.OVL's relocations: the
; overlay's header lists them (tools/u5.py OVERLAY_HEADER).
; Drafted from the original bytes by tools/asmgen.py --ovl and checked by tools/asmbuild.py.

	TITLE	AINTRO2
_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT  BYTE PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST, _BSS, _DATA
	ASSUME	CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP

	EXTRN	INTRO_1016_CONVERTU4SAVEGAME:NEAR
	EXTRN	INTRO_1278_PRINTU4CLASS:NEAR
	EXTRN	INTRO_12EA_SCALEU4STAT:NEAR
	EXTRN	INTRO_1E22_PRINTFIELDLABEL:NEAR
	EXTRN	INTRO_2024_SETUPTRANSFERSCREEN:NEAR
	EXTRN	ULTIMA_16BA_PRINTCHAR:NEAR
	EXTRN	ULTIMA_1850_PRINTSTRING:NEAR
	EXTRN	ULTIMA_1A3E_PRINTNUMBER:NEAR
	EXTRN	ULTIMA_1B94_SELECTTEXTWINDOW:NEAR
	EXTRN	ULTIMA_1BF2_SETTEXTPOSITION:NEAR
	EXTRN	ULTIMA_1C22_SETTEXTWINDOWSIZE:NEAR
	EXTRN	ULTIMA_1DDA_WAITFORKEYSTROKE:NEAR
	EXTRN	ULTIMA_1E38_INTROGETSTRING:NEAR
	EXTRN	ULTIMA_1EAC_SETDEFAULTDRIVE:NEAR
	EXTRN	ULTIMA_2032_TOUPPER:NEAR
	EXTRN	ULTIMA_251E_SWITCHDISKS:NEAR
	EXTRN	ULTIMA_256E_READFILEFROMDISK:NEAR
	EXTRN	ULTIMA_25D8_WRITEFILETODISK:NEAR
	EXTRN	D_3304:WORD
	EXTRN	D_5394_DISKSWAPFN:WORD
	EXTRN	ULTIMA_2320_NODISKSWAPMESSAGE:FAR
	EXTRN	ULTIMA_2322_DISKSWAPMESSAGE:FAR
	EXTRN	D_55A8_PARTY:BYTE
	EXTRN	D_5893_MAPID:BYTE
	EXTRN	D_A9BD_CURDISK:BYTE
	EXTRN	D_A9C8_DISKDRIVES:BYTE

; this module's own data in DGROUP (DS:3345-3655)
_DATA	SEGMENT
	PUBLIC	D_3345
D_3345	db	'INIT.GAM', 00h
	PUBLIC	D_334E
D_334E	db	'INIT.OOL', 00h
	PUBLIC	D_3357
D_3357	db	'Transfer Character from Ultima IV', 00h
	PUBLIC	D_3379
D_3379	db	'Please insert the Ultima IV Player Disk', 00h
	PUBLIC	D_33A1
D_33A1	db	'and press drive letter', 00h
	PUBLIC	D_33B8
D_33B8	db	'or press <Esc> to abort transfer', 00h
	PUBLIC	D_33D9
D_33D9	db	'Found:', 0ah, 00h
	PUBLIC	D_33E1
D_33E1	db	'a level ', 00h
	PUBLIC	D_33EA
D_33EA	db	' Male ', 00h
	PUBLIC	D_33F1
D_33F1	db	' Female ', 00h
	PUBLIC	D_33FA
D_33FA	db	'STR:  ', 00h
	PUBLIC	D_3401
D_3401	db	'DEX:  ', 00h
	PUBLIC	D_3408
D_3408	db	'INT:  ', 00h
	PUBLIC	D_340F
D_340F	db	' is ', 00h
	PUBLIC	D_3414
D_3414	db	'an Avatar.', 00h
	PUBLIC	D_341F
D_341F	db	'not an Avatar', 00h
	PUBLIC	D_342D
D_342D	db	'Male', 00h
	PUBLIC	D_3432
D_3432	db	'Female', 00h
	PUBLIC	D_3439
D_3439	db	'Avatar', 00h
	PUBLIC	D_3440
D_3440	db	'Avatar', 00h
	PUBLIC	D_3447
D_3447	db	'Non-Avatar', 00h
	PUBLIC	D_3452
D_3452	db	'Name:', 00h
	PUBLIC	D_3458
D_3458	db	'Name:', 00h
	PUBLIC	D_345E
D_345E	db	'Keep this name?', 00h
	PUBLIC	D_346E
D_346E	db	'Enter new name: ', 00h
	PUBLIC	D_347F
D_347F	db	'Name:', 00h
	PUBLIC	D_3485
D_3485	db	'Sex:', 00h
	PUBLIC	D_348A
D_348A	db	'Name:', 00h
	PUBLIC	D_3490
D_3490	db	'Sex:', 00h
	PUBLIC	D_3495
D_3495	db	'Keep same sex?', 00h
	PUBLIC	D_34A4
D_34A4	db	'Male', 00h
	PUBLIC	D_34A9
D_34A9	db	'Female', 00h
	PUBLIC	D_34B0
D_34B0	db	'Sex:', 00h
	PUBLIC	D_34B5
D_34B5	db	'Class:', 00h
	PUBLIC	D_34BC
D_34BC	db	'Avatar', 00h
	PUBLIC	D_34C3
D_34C3	db	'Sex:', 00h
	PUBLIC	D_34C8
D_34C8	db	'Class:', 00h
	PUBLIC	D_34CF
D_34CF	db	'Thou art now an Avatar:', 00h
	PUBLIC	D_34E7
D_34E7	db	'Class remains intact', 00h
	PUBLIC	D_34FC
D_34FC	db	'Class:', 00h
	PUBLIC	D_3503
D_3503	db	'Exp:', 00h
	PUBLIC	D_3508
D_3508	db	'Class:', 00h
	PUBLIC	D_350F
D_350F	db	'Exp:', 00h
	PUBLIC	D_3514
D_3514	db	'Experience has been converted', 00h
	PUBLIC	D_3532
D_3532	db	'Exp:', 00h
	PUBLIC	D_3537
D_3537	db	'Level:', 00h
	PUBLIC	D_353E
D_353E	db	'Exp:', 00h
	PUBLIC	D_3543
D_3543	db	'Level:', 00h
	PUBLIC	D_354A
D_354A	db	'Level has been converted', 00h
	PUBLIC	D_3563
D_3563	db	'Level:', 00h
	PUBLIC	D_356A
D_356A	db	'STR:', 00h
	PUBLIC	D_356F
D_356F	db	'Level:', 00h
	PUBLIC	D_3576
D_3576	db	'STR:', 00h
	PUBLIC	D_357B
D_357B	db	'Strength: was ', 00h
	PUBLIC	D_358A
D_358A	db	'(50), now ', 00h
	PUBLIC	D_3595
D_3595	db	'(30)', 00h
	PUBLIC	D_359A
D_359A	db	'STR:', 00h
	PUBLIC	D_359F
D_359F	db	'DEX:', 00h
	PUBLIC	D_35A4
D_35A4	db	'STR:', 00h
	PUBLIC	D_35A9
D_35A9	db	'DEX:', 00h
	PUBLIC	D_35AE
D_35AE	db	'Dexterity: was ', 00h
	PUBLIC	D_35BE
D_35BE	db	'(50), now ', 00h
	PUBLIC	D_35C9
D_35C9	db	'(30)', 00h
	PUBLIC	D_35CE
D_35CE	db	'DEX:', 00h
	PUBLIC	D_35D3
D_35D3	db	'INT:', 00h
	PUBLIC	D_35D8
D_35D8	db	'DEX:', 00h
	PUBLIC	D_35DD
D_35DD	db	'INT:', 00h
	PUBLIC	D_35E2
D_35E2	db	'Intellect: was ', 00h
	PUBLIC	D_35F2
D_35F2	db	'(50), now ', 00h
	PUBLIC	D_35FD
D_35FD	db	'(30)', 00h
	PUBLIC	D_3602
D_3602	db	'INT:', 00h
	PUBLIC	D_3607
D_3607	db	'INT:', 00h
	PUBLIC	D_360C
D_360C	db	'Avatar', 00h
	PUBLIC	D_3613
D_3613	db	'Non-Avatar', 00h
	PUBLIC	D_361E
D_361E	db	0ah, 0ah, ' Conversion complete, saving...', 0ah, 00h
	PUBLIC	D_3641
D_3641	db	'SAVED.OOL', 00h
	PUBLIC	D_364B
D_364B	db	'SAVED.GAM', 00h
_DATA	ENDS

_TEXT	SEGMENT

	PUBLIC	INTRO_132A_TRANSFERFROMU4
INTRO_132A_TRANSFERFROMU4:
	push bp
	mov bp,sp
	sub sp,12h
	push di
	push si
; GOG (to 134d): a map id of 42h becomes 41h, as in 1.16, and both U4 drives (D_a9cb,
; D_a9cc) are the program's drive D_A9C8_DISKDRIVES (1.16: 0ffh, to be asked for)
	mov al,BYTE PTR D_5893_MAPID
L_1335:
	mov BYTE PTR D_5893_MAPID,al
	cmp al,42h
	jne	L_1340
	mov al,41h
	jmp	SHORT L_1335
L_1340:
	db	030h, 0e4h	; xor ah,ah
	mov [bp-0ch],ax
	mov al,BYTE PTR D_A9C8_DISKDRIVES
	mov BYTE PTR D_A9C8_DISKDRIVES+3,al
	mov BYTE PTR D_A9C8_DISKDRIVES+4,al
	mov ax,OFFSET DGROUP:D_3345
	push ax
	mov ax,55a6h
	push ax
	mov ax,6606h
	sub ax,55a6h
	push ax
	sub ax,ax
	push ax
	call	ULTIMA_256E_READFILEFROMDISK
	mov ax,OFFSET DGROUP:D_334E
	push ax
	mov ax,0b31eh
	push ax
	mov ax,100h
	push ax
	sub ax,ax
	push ax
	call	ULTIMA_256E_READFILEFROMDISK
	sub ax,ax
	push ax
	push ax
	push ax
	mov ax,27h
	push ax
	mov ax,18h
	push ax
	call	ULTIMA_1C22_SETTEXTWINDOWSIZE
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
; GOG (to 1399): the drive is the program's, and the prompt below is skipped (in 1.16:
; mov ax,4; push ax; mov ax,0ch; push ax, the arguments of the call at 139a)
	mov al,BYTE PTR D_A9C8_DISKDRIVES
	mov [bp-10h],al
	jmp	SHORT L_1402
; from here to 1401 only the GOG release's jump at 1398 skips over
L_139a:
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_3357
	push ax
	call	ULTIMA_1850_PRINTSTRING
	sub ax,ax
	push ax
	mov ax,0fh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_3379
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,8
	push ax
	mov ax,10h
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_33A1
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,3
	push ax
	mov ax,12h
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_33B8
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov WORD PTR D_5394_DISKSWAPFN,OFFSET ULTIMA_2320_NODISKSWAPMESSAGE
	mov	WORD PTR D_5394_DISKSWAPFN+2,_TEXT
	mov si,[bp-0ch]
L_13e8:
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	push ax
	call	ULTIMA_2032_TOUPPER
	mov [bp-10h],al
	cmp al,1bh
	jne	L_1402
	mov ax,si
L_13fb:
	mov BYTE PTR D_5893_MAPID,al
	jmp	NEAR PTR L_1e1c
	db	090h
L_1402:
	mov al,[bp-10h]
	sub ah,ah
	push ax
	call	ULTIMA_1EAC_SETDEFAULTDRIVE
	or ax,ax
	je	L_13e8
	mov WORD PTR D_5394_DISKSWAPFN,OFFSET ULTIMA_2322_DISKSWAPMESSAGE
	mov	WORD PTR D_5394_DISKSWAPFN+2,_TEXT
	call	INTRO_1016_CONVERTU4SAVEGAME
	or ax,ax
	je	L_1428
	mov al,[bp-0ch]
	jmp	SHORT L_13fb
	db	090h
L_1428:
	cmp WORD PTR D_3304,0
	je	L_1434
	mov BYTE PTR D_55A8_PARTY+10,41h
L_1434:
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	sub ax,ax
	push ax
	mov ax,0bh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fch
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_33D9
	push ax
	call	ULTIMA_1850_PRINTSTRING
	sub ax,ax
	push ax
	mov ax,0ch
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,55a8h
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fbh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0ch
	push ax
	mov ax,0dh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_33E1
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov al,BYTE PTR D_55A8_PARTY+22
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	cmp BYTE PTR D_55A8_PARTY+9,0bh
	jne	L_149a
	mov ax,OFFSET DGROUP:D_33EA
	jmp	SHORT L_149d
L_149a:
	mov ax,OFFSET DGROUP:D_33F1
L_149d:
	push ax
	call	ULTIMA_1850_PRINTSTRING
	call	INTRO_1278_PRINTU4CLASS
	mov ax,11h
	push ax
	mov ax,0fh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_33FA
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov al,BYTE PTR D_55A8_PARTY+12
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,11h
	push ax
	mov ax,10h
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_3401
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov al,BYTE PTR D_55A8_PARTY+13
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,11h
	push ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_3408
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov al,BYTE PTR D_55A8_PARTY+14
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,0ah
	push ax
	mov ax,14h
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,55a8h
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,OFFSET DGROUP:D_340F
	push ax
	call	ULTIMA_1850_PRINTSTRING
	cmp WORD PTR D_3304,0
	je	L_1530
	mov ax,OFFSET DGROUP:D_3414
	jmp	SHORT L_1533
	db	090h
L_1530:
	mov ax,OFFSET DGROUP:D_341F
L_1533:
	push ax
	call	ULTIMA_1850_PRINTSTRING
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	call	INTRO_2024_SETUPTRANSFERSCREEN
	sub si,si
L_1549:
	push si
	call	INTRO_1E22_PRINTFIELDLABEL
	inc si
	cmp si,8
	jl	L_1549
	mov [bp-0eh],si
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	sub ax,ax
	push ax
	mov ax,3
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fch
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,55a8h
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fbh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0ah
	push ax
	mov ax,5
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	cmp BYTE PTR D_55A8_PARTY+9,0bh
	jne	L_1592
	mov ax,OFFSET DGROUP:D_342D
	jmp	SHORT L_1595
L_1592:
	mov ax,OFFSET DGROUP:D_3432
L_1595:
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0ah
	push ax
	mov ax,6
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	cmp WORD PTR D_3304,0
	je	L_15b4
	mov ax,OFFSET DGROUP:D_3439
	push ax
	call	ULTIMA_1850_PRINTSTRING
	jmp	SHORT L_15b7
L_15b4:
	call	INTRO_1278_PRINTU4CLASS
L_15b7:
	mov ax,0ah
	push ax
	mov ax,8
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	push WORD PTR D_55A8_PARTY+20
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,0ah
	push ax
	mov ax,9
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov al,BYTE PTR D_55A8_PARTY+22
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,0ah
	push ax
	mov ax,0bh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov al,BYTE PTR D_55A8_PARTY+12
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,0ah
	push ax
	mov ax,0ch
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov al,BYTE PTR D_55A8_PARTY+13
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,0ah
	push ax
	mov ax,0dh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov al,BYTE PTR D_55A8_PARTY+14
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	sub ax,ax
	push ax
	mov ax,0fh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fch
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	cmp WORD PTR D_3304,0
	je	L_165e
	mov ax,OFFSET DGROUP:D_3440
	jmp	SHORT L_1661
L_165e:
	mov ax,OFFSET DGROUP:D_3447
L_1661:
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fbh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,7
	push ax
	mov ax,2
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3452
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,1
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,7
	push ax
	mov ax,2
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3458
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,2
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0ah
	push ax
	sub ax,ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_345E
	push ax
	call	ULTIMA_1850_PRINTSTRING
	jmp	SHORT L_16d4
	db	090h
L_16ce:
	cmp byte ptr [bp-10h],4eh
	je	L_16e5
L_16d4:
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	push ax
	call	ULTIMA_2032_TOUPPER
	mov [bp-10h],al
	cmp al,59h
	jne	L_16ce
L_16e5:
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	cmp byte ptr [bp-10h],4eh
	jne	L_1715
L_16f2:
	mov ax,1
	push ax
	sub ax,ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_346E
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,55a8h
	push ax
	mov ax,8
	push ax
	call	ULTIMA_1E38_INTROGETSTRING
	cmp BYTE PTR D_55A8_PARTY,0
	je	L_16f2
L_1715:
	mov ax,1
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	sub ax,ax
	push ax
	mov ax,3
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fch
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,55a8h
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fbh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,7
	push ax
	mov ax,2
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_347F
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3485
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,7
	push ax
	mov ax,2
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_348A
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3490
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,2
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0fch
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3495
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fbh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	jmp	SHORT L_17d2
	db	090h
L_17cc:
	cmp byte ptr [bp-10h],4eh
	je	L_17e3
L_17d2:
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	push ax
	call	ULTIMA_2032_TOUPPER
	mov [bp-10h],al
	cmp al,59h
	jne	L_17cc
L_17e3:
	mov ax,1
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ah
	push ax
	mov ax,5
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	cmp byte ptr [bp-10h],59h
	jne	L_1802
	cmp BYTE PTR D_55A8_PARTY+9,0bh
	je	L_180f
L_1802:
	cmp byte ptr [bp-10h],4eh
	jne	L_181e
	cmp BYTE PTR D_55A8_PARTY+9,0ch
	jne	L_181e
L_180f:
	mov ax,OFFSET DGROUP:D_34A4
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov BYTE PTR D_55A8_PARTY+9,0bh
	jmp	SHORT L_182a
	db	090h
L_181e:
	mov ax,OFFSET DGROUP:D_34A9
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov BYTE PTR D_55A8_PARTY+9,0ch
L_182a:
	mov ax,5
	push ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_34B0
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,3
	push ax
	mov ax,6
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_34B5
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0ah
	push ax
	mov ax,6
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	cmp WORD PTR D_3304,0
	je	L_1874
	mov ax,OFFSET DGROUP:D_34BC
	push ax
	call	ULTIMA_1850_PRINTSTRING
	jmp	SHORT L_1877
L_1874:
	call	INTRO_1278_PRINTU4CLASS
L_1877:
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,5
	push ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_34C3
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,3
	push ax
	mov ax,6
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_34C8
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,2
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,2
	push ax
	sub ax,ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	cmp WORD PTR D_3304,0
	je	L_18d0
	mov ax,OFFSET DGROUP:D_34CF
	jmp	SHORT L_18d3
L_18d0:
	mov ax,OFFSET DGROUP:D_34E7
L_18d3:
	push ax
	call	ULTIMA_1850_PRINTSTRING
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,3
	push ax
	mov ax,6
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_34FC
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	mov ax,8
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3503
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,1
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,3
	push ax
	mov ax,6
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_3508
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	mov ax,8
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_350F
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0ah
	push ax
	mov ax,8
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov cx,0ah
	mov ax,WORD PTR D_55A8_PARTY+20
	cwd
	idiv cx
	mov WORD PTR D_55A8_PARTY+20,ax
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,2
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0fch
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3514
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fbh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,5
	push ax
	mov ax,8
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_3532
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,3
	push ax
	mov ax,9
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3537
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,1
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,5
	push ax
	mov ax,8
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_353E
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,3
	push ax
	mov ax,9
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3543
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0ah
	push ax
	mov ax,9
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov BYTE PTR D_55A8_PARTY+22,1
	mov ax,WORD PTR D_55A8_PARTY+20
	cwd
	mov cx,64h
	idiv cx
	mov [bp-0eh],ax
	mov cx,ax
	jmp	SHORT L_1a32
	db	090h
L_1a2c:
	inc BYTE PTR D_55A8_PARTY+22
	sar cx,1
L_1a32:
	or cx,cx
	jg	L_1a2c
	mov [bp-0eh],cx
	mov al,1eh
	mul BYTE PTR D_55A8_PARTY+22
	mov WORD PTR D_55A8_PARTY+18,ax
	mov WORD PTR D_55A8_PARTY+16,ax
	mov al,BYTE PTR D_55A8_PARTY+22
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,2
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0fch
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_354A
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fbh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,3
	push ax
	mov ax,9
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_3563
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	mov ax,0bh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_356A
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,1
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,3
	push ax
	mov ax,9
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_356F
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	mov ax,0bh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_3576
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0ah
	push ax
	mov ax,0bh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov al,BYTE PTR D_55A8_PARTY+12
	sub ah,ah
	mov [bp-12h],ax
	push ax
	call	INTRO_12EA_SCALEU4STAT
	mov BYTE PTR D_55A8_PARTY+12,al
	cmp al,14h
	jae	L_1b13
	mov BYTE PTR D_55A8_PARTY+12,14h
L_1b13:
	mov al,BYTE PTR D_55A8_PARTY+12
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,2
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,1
	push ax
	sub ax,ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_357B
	push ax
	call	ULTIMA_1850_PRINTSTRING
	push word ptr [bp-12h]
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,OFFSET DGROUP:D_358A
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov al,BYTE PTR D_55A8_PARTY+12
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,OFFSET DGROUP:D_3595
	push ax
	call	ULTIMA_1850_PRINTSTRING
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,5
	push ax
	mov ax,0bh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_359A
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	mov ax,0ch
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_359F
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,1
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,5
	push ax
	mov ax,0bh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_35A4
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	mov ax,0ch
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_35A9
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0ah
	push ax
	mov ax,0ch
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov al,BYTE PTR D_55A8_PARTY+13
	sub ah,ah
	mov [bp-12h],ax
	push ax
	call	INTRO_12EA_SCALEU4STAT
	mov BYTE PTR D_55A8_PARTY+13,al
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,2
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,1
	push ax
	sub ax,ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_35AE
	push ax
	call	ULTIMA_1850_PRINTSTRING
	push word ptr [bp-12h]
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,OFFSET DGROUP:D_35BE
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov al,BYTE PTR D_55A8_PARTY+13
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,OFFSET DGROUP:D_35C9
	push ax
	call	ULTIMA_1850_PRINTSTRING
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,5
	push ax
	mov ax,0ch
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_35CE
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	mov ax,0dh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_35D3
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,1
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,5
	push ax
	mov ax,0ch
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_35D8
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,5
	push ax
	mov ax,0dh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,OFFSET DGROUP:D_35DD
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fdh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,0ah
	push ax
	mov ax,0dh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov al,BYTE PTR D_55A8_PARTY+14
	sub ah,ah
	mov [bp-12h],ax
	push ax
	call	INTRO_12EA_SCALEU4STAT
	mov BYTE PTR D_55A8_PARTY+14,al
	mov BYTE PTR D_55A8_PARTY+15,al
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,2
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,1
	push ax
	sub ax,ax
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_35E2
	push ax
	call	ULTIMA_1850_PRINTSTRING
	push word ptr [bp-12h]
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,OFFSET DGROUP:D_35F2
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov al,BYTE PTR D_55A8_PARTY+14
	sub ah,ah
	push ax
	mov ax,1
	push ax
	mov ax,20h
	push ax
	call	ULTIMA_1A3E_PRINTNUMBER
	mov ax,OFFSET DGROUP:D_35FD
	push ax
	call	ULTIMA_1850_PRINTSTRING
	sub ax,ax
	push ax
	call	ULTIMA_1DDA_WAITFORKEYSTROKE
	sub ax,ax
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,5
	push ax
	mov ax,0dh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_3602
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,1
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,5
	push ax
	mov ax,0dh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,OFFSET DGROUP:D_3607
	push ax
	call	ULTIMA_1850_PRINTSTRING
	sub ax,ax
	push ax
	mov ax,0fh
	push ax
	call	ULTIMA_1BF2_SETTEXTPOSITION
	mov ax,0fch
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	cmp WORD PTR D_3304,0
	je	L_1d9e
	mov ax,OFFSET DGROUP:D_360C
	jmp	SHORT L_1da1
	db	090h
L_1d9e:
	mov ax,OFFSET DGROUP:D_3613
L_1da1:
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov ax,0fbh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,2
	push ax
	call	ULTIMA_1B94_SELECTTEXTWINDOW
	mov ax,2
	push ax
	push ax
	mov ax,15h
	push ax
	mov ax,25h
	push ax
	mov ax,16h
	push ax
	call	ULTIMA_1C22_SETTEXTWINDOWSIZE
	mov ax,0ffh
	push ax
	call	ULTIMA_16BA_PRINTCHAR
	mov ax,3
	push ax
	call	ULTIMA_251E_SWITCHDISKS
	mov ax,OFFSET DGROUP:D_361E
	push ax
	call	ULTIMA_1850_PRINTSTRING
	mov cx,100h
	mov di,0b21eh
	mov ax,ds
	mov es,ax
	sub ax,ax
	repne stosb
	mov ax,OFFSET DGROUP:D_3641
	push ax
	mov ax,0b21eh
	push ax
	mov ax,200h
	push ax
	call	ULTIMA_25D8_WRITEFILETODISK
	mov ax,OFFSET DGROUP:D_364B
	push ax
	mov ax,55a6h
	push ax
	mov ax,6606h
	sub ax,55a6h
	push ax
	call	ULTIMA_25D8_WRITEFILETODISK
	sub ax,ax
	push ax
	call	ULTIMA_251E_SWITCHDISKS
	mov al,[bp-0ch]
	mov BYTE PTR D_5893_MAPID,al
	mov BYTE PTR D_A9BD_CURDISK+1,0
L_1e1c:
	pop si
	pop di
	mov sp,bp
	pop bp
	ret

_TEXT	ENDS
	END
