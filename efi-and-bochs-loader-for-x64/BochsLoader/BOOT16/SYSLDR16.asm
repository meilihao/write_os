; Copyright by Xiyue87 2022

[BITS 16]
;/*
;  Flat memory module

;  kernel run in a flat memory space, base address is 0x0, segment size is 4G

;  0x00000000 - 0x00007FFF boot sector space
;  0x00008000 - 0x00097FFF boot loader and kernel space
;  16-bit mode
;  ss:sp -> 0000:7FFF
;  32-bit mode
;  global segment:
;  index     type    base        limit       used for
;  0x0008    code    0x00000000  0xFFFFFFFF  cs
;  0x0010    stack   0x00000000  0xFFFFFFFF  ss
;  0x0018    data    0x00000000  0xFFFFFFFF  ds(es, fs, gs)
;  ss:esp -> 0018:00180000
;  ss:ebp -> 0018:00180000
; */


start:
    ; before we go here, CS:IP should be 0800:0000    
    jmp start16
    db 0xFF, 0xEE, 0xDD, 0xCC ; magic number for linker to update code32_* values
code32_base:
    db 00, 00, 00, 00
code32_in_file:    
    db 00, 00, 00, 00
code32_size:
    db 00, 00, 00, 00
code32_entry:
    db 00, 00, 00, 00


start16:    
    ; init segment regs
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; init stack
    mov sp, 0x7FFF
    mov bp, sp
    
    ; get memory information first
    call get_mem_info

    ; get 32-bit offset of GDT
    mov ax, cs
    and ax, 0xF000
    mov cl, 12
    shr ax, cl

    mov bx, cs
    mov cl, 4
    shl bx, cl

    mov cx, SegmentTable - start
    add bx, cx
    adc ax, 0

    push ax
    push bx

    mov dx, EndOfSegTbl - start
    sub dx, cx
    dec dx

    push dx

    mov si, sp

    ; load gdt
    lgdt [ds:si]

    ;enable A20
    cli
    in al, 92h
    or al, 00000010b
    out 92h, al

    ;to protect mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ;init regs
    mov ax, 0018h
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ax, 0010h
    mov ss, ax

    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    xor edi, edi
    xor esi, esi
    mov esp, 0x180000
    mov ebp, 0x180000

    mov eax, kernel_param
    push eax
    xor eax, eax
    push eax
    
    mov esi, code32_entry
    jmp real_to_protect_jmp

real_to_protect_jmp:
    jmp dword 0x0008:(protect_start + 0x8000)

SegmentTable:
;    length             baseL        baseHL    Att1    Att2    baseHH
db    0,       0,       0,      0,      0,      0,       0,       0      ;NULL
db    0xFF,    0xFF,    0x0,    0x0,    0x0,    0x98,    0xCF,    0x0    ;CS 0x0008
db    0x0,     0x0,     0x0,    0x0,    0x0,    0x96,    0xC0,    0x0    ;SS 0x0010
db    0xFF,    0xFF,    0x0,    0x0,    0x0,    0x92,    0xCF,    0x0    ;DS 0x0018
EndOfSegTbl:

kernel_param:
times 256 db 0
kernel_param_end:
; this function is used to get memory information.
; currently, it is not very useful because we can get it by ACPI interface in protect mode.
;
get_mem_info: 
    push es
    mov ax, cs
    mov es, ax
    mov di, kernel_param - start ; es:di point to parameter buffer
    xor si, si  ; si is used for counter
    add di, 0x20 ; first 32 bytes is left for store how many records
    
    xor ebx, ebx ; ebx should be 0 before call int 15h
    
loop:
    mov eax, 0xE820 ; eax = 0xE820
    mov ecx, kernel_param_end - start ; ecx point to the end of buffer
    sub ecx, edi
    jna memcheckerror ; check if there is enough buffer
    mov edx, 0534D4150h ; edx = 'SMAP'

    int 15h ; call int 15
    jc memcheckerror ; CF = 1, error occured

    add di, cx ; di point to next buffer
    cmp ebx, 0 ; ebx = 0, operation finished, else continue
    je memcheckok

    inc si
    jmp loop

memcheckerror:
    xor si, si ; set si to 0, means no record
memcheckok:
    mov di, kernel_param - start
    mov [es:di], si ; store record numbers
    mov di, 2
    mov [es:di], cx ; store how many bytes per record

    pop es
    
    ret

[BITS 32]
protect_start:
    mov eax, 0x8000
    
    mov esi, code32_in_file
    add esi, eax
    mov esi, [esi]
    add esi, eax

    mov edi, code32_base
    add edi, eax
    mov edi, [edi]

    mov ecx, code32_size
    add ecx, eax
    mov ecx, [ecx]

    cld
    rep movsb
    mov ebx, code32_entry
    add ebx, eax
    mov ebx, [ebx]

times 0x400-($-$$) db 0x90 ; fill some NOP, useless
    jmp ebx

