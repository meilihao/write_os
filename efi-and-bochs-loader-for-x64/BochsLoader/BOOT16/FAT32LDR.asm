; Copyright by Xiyue87 2022

org 0x7c00
start:
    jmp PrePareSegment
    nop
    times 0x5A-($-$$) db 0      ; pad data for BPB

%define _StartAddress           0x7c00

%define _BS_BytesPerSec         0x0b + _StartAddress        ; 2 B
%define _BS_SecPerCls           0x0d + _StartAddress        ; 1 B
%define _BS_ReserveSec          0x0e + _StartAddress        ; 2 B
%define _BS_FATCnt              0x10 + _StartAddress        ; 1 B
%define _SecPerTrack            0x18 + _StartAddress        ; 2 B
%define _HeadCnt                0x1a + _StartAddress        ; 1 B
%define _BS_HiddenSec           0x1c + _StartAddress        ; 4 B
%define _BS_FATSize32           0x24 + _StartAddress        ; 4 B
%define _BS_RootDir32           0x2c + _StartAddress        ; 4 B
%define _DrvNum                 0x40 + _StartAddress        ; 1 B
%define data_startl             0x47 + _StartAddress        ; 4 B, here is volume label, I use it to store value
                                                            ; this value is data space start sector

%define ROOT_DIR                0x7e00

PrePareSegment:
    cli
    ; Init segment register to as same as CS
    ; Set stack pointer to 0x7bfe
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 7bfeh
    cld

GetDiskParam:
    push es
    mov [_DrvNum], dl                   ; Save Current driver number
    mov ah, 8h                          ; INT 13H, CMD 8H into al.
    int 13h                             ; BIOS drive get parameters
    and cx, 003Fh                       ; sectors per track in 6 LSB(its)'s

    mov [_SecPerTrack], cx

    ; max heads we can use w/BIOS in dx, add one for math
    inc dh                              ; For math, heads start at zero...
    xor dl, dl
    xchg dh, dl
    mov [_HeadCnt], dx
    pop es                              ; int 13 may change es, so reset es
    sti

PrintHello:                             ; print char 'F'
    mov si, 'F'
    call PrintChar

ProcessBootSec:
    ; Calc _BS_FATCnt * _BS_FATSize32
    mov cl, [_BS_FATCnt]
    mov bx, _BS_FATSize32
    call MUL_ADD
    call AddReservedSec
    ; Put value to data_startl
    mov [data_startl], ax
    mov [data_startl + 2], dx           ; data_startl is reserve sector count (reserved sector + FAT table size)

ReadFileNextCluster:
    mov cl, [_BS_SecPerCls]
    push cx
    mov bx, _BS_RootDir32
    mov ax, 2
    xor dx, dx
    ; Now DX:AX = 0000:0002
    ; Calc DS:[BX] (_BS_RootDir32) - DX:AX
    ; Then DS:[BX] = Root directory start cluster (0 - based)
    sub [bx], ax
    sbb [bx + 2], dx
    ; Calc RootDirCluster * SecPerCluster => Root Dir Sector
    call MUL_ADD
    ; Add Reserved sector
    add ax, [data_startl]
    adc dx, [data_startl + 2]       ; dx:ax => Root directory data sector number

RepProccessSectors:
    mov bx, ROOT_DIR
    call ReadCHSDisk                ; Read Sector Data to 0x7E00

    call [check_step_ptr]
    inc cx                          ; If found boot loader, cx will be 0xFFFF
    jz ReadNextFile                 ; Then prepare to read file

    add ax, 1                       ; Increase sector number
    adc dx, 0

    pop cx                          ; CX in stack is _BS_SecPerCls
    dec cx                          ; Reduce CX and store it to stack
    push cx
    jnz RepProccessSectors          ; If CX is not 0, read next sector

    pop cx                          ; Clear stack

    mov bx, _BS_RootDir32           ; Get next cluster

    mov ax, 2                       ; restore real cluster
    xor dx, dx
    add [bx], ax
    adc [bx + 2], dx

    mov cl, 4
    call MUL_ADD                    ; Get offset of cluster 4 * cluster number
    div word [_BS_BytesPerSec]      ; Get sector number and offset in sector
    push dx                         ; DX => offset in sector, AX => sector number
    xor dx, dx
    call AddReservedSec             ; Add reserved sector

    mov bx, ROOT_DIR                ; store FAT to 0x7e00
    call ReadCHSDisk                ; read disk
    pop si

    mov ax, [si + ROOT_DIR]         ; get next cluster value
    mov dx, [si + 2 + ROOT_DIR]

    mov [_BS_RootDir32], ax
    mov [_BS_RootDir32 + 2], dx     ; save next cluster number

    inc ax                          ; Is last cluster? (ax = 0xFFFF, ++ => 0, then ZF is set)
    jnz ReadFileNextCluster
    cmp dx, 0x0FFF
    jnz ReadFileNextCluster

    mov cx, [check_step_ptr]
    cmp cx, LoadFile
    jz GotoLoaderFromFAT

    mov si, 'E'
    call PrintChar
    call DeadLoop

ReadNextFile:
    pop cx
    jmp ReadFileNextCluster

GotoLoaderFromFAT:
    mov si, '!'
    call PrintChar
    mov dl, [_DrvNum]
    jmp far [BootStart]

DeadLoop:
    jmp DeadLoop

;-----------------------------------------------------------------------
; Check name
; Entry: bx - address of data
;        Find BootFileName in directory
;        If found it, cx will be 0xFFFF, and check_step_ptr will be LoadFile
;-----------------------------------------------------------------------
CheckName:
RepNextName:
    mov cx, 11
    mov si, BootFileName
    mov di, bx
    repe cmpsb
    jz FindName
    add bx, 32
    cmp bx, ROOT_DIR + 0x200
    jnz RepNextName

CheckNameReturn:
    ret

FindName:

    ; If found it, store file's first cluster to _BS_RootDir32
    mov dx, [bx + 0x14]
    mov ax, [bx + 0x1a]
    mov [_BS_RootDir32], ax
    mov [_BS_RootDir32 + 2], dx
    mov cx, LoadFile
    mov [check_step_ptr], cx
    mov cx, 0xFFFF
    jmp CheckNameReturn
    
;-----------------------------------------------------------------------
; LoadFile
; Entry: bx - address of data
;-----------------------------------------------------------------------
LoadFile:
    push es

    mov cx, [NextSegment]                          
    mov es, cx
    add cx, 0x20
    mov [NextSegment], cx           ; increase segment value 0x20 everytime, because (0x20 << 8) = 0x200 is a sector size
    mov cx, 0x200
    mov si, ROOT_DIR
    xor di, di
    repz movsb
    mov si, '+'
    call PrintChar
    pop es
    ret
;-----------------------------------------------------------------------
; calc (data8) * (data32) => (data32)
; Entry: data8 - cl
; Entry: data32 - [DS:BX]
; Exit: low16 => AX
;       high16 => DX
;       CL = 0
;-----------------------------------------------------------------------
MUL_ADD:
    xor dx, dx
    xor ax, ax
Rep_MulAdd:
    add ax, [bx]
    adc dx, [bx + 2]
    dec cl
    jnz Rep_MulAdd
    mov bx, dx
    ret

;-----------------------------------------------------------------------
; Add hidden sector and reserve sector to DX(High16):AX(Low16)
;-----------------------------------------------------------------------
AddReservedSec:
    add ax, [_BS_ReserveSec]
    adc dx, 0
    add ax, [_BS_HiddenSec]
    adc dx, [_BS_HiddenSec + 2]

    ret
;-----------------------------------------------------------------------
; PrintChar: si (low 8 bit) is the char to be printed
;-----------------------------------------------------------------------

PrintChar:
    push bx
    push ax

    mov ax, si
    mov ah, 0eh
    mov bx, 7
    int 10h

    pop ax
    pop bx
    ret

;-----------------------------------------------------------------------
; ReadCHSDisk
; Entry: DX(High16),AX(Low16) - sector to be read in LBA mode
;        ES:BX - address of buffer.
;-----------------------------------------------------------------------

; Cyliner: C
; Head : H
; Sector: S
; LBA = Head * SecPerTrack * C + Cyliner * H + S - 1
;
; C = (LBA Div SecPerTrack) Div Head
; H = (LBA Div SecPerTrack) Mod Head
; S = (LBA Mod SecPerTrack) + 1


; Int 13 CHS Mode
;
; AH = 02 (Read Disk)
; AL = How many sectors to be read
;
; (CL)6,7 (CH)0~7 = Cyliner
; (CL)0~5 = Sector
; DH = Head
;
; DL = Driver Number
;
; ES:BX = Buffer

ReadCHSDisk:
    push ax
    push cx
    push dx

    div word [_SecPerTrack]

    push dx                     ; DX = (LBA Mod SecPerTrack)

    div byte [_HeadCnt]

    xor cl, cl                  ; Clear CL
    mov ch, al                  ; AL = (LBA Div SecPerTrack) Div Head  ==> Cyliner, Save to CH
    mov dh, ah                  ; AH = (LBA Div SecPerTrack) Mod Head  ==> Header, save to DH
    pop ax                      ; push dx, pop ax. Now, AX = (LBA Mod SecPerTrack)
    inc ax
    mov cl, al                  ; Save sector number to CL

    mov ax, 0x0201              ; Read one sector
    mov dl, [_DrvNum]

    int 13h
    jnc ReadCHSDiskExit
    stc                         ; set CF when error
ReadCHSDiskExit:
    pop dx
    pop cx
    pop ax
    ret
;-----------------------------------------------------------------------
; Data for first sector
;-----------------------------------------------------------------------

DataPart:
    BootStart       dd  0x08000000
    BootFileName    db  'SYSLDR  BIN'
    check_step_ptr  dw  CheckName
    NextSegment     dw  0x800
times 510-($-$$) db 0xFF

    BootSignature   db 0x55, 0xAA



