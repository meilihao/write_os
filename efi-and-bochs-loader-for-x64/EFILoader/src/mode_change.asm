; Copyright by Xiyue 2022

.CODE

long_jmp_rip dd 0, 0, 0, 0, 0, 0, 0, 0

goto_to_64 PROC
    ; disable interrupt
    cli
    
    ; save jump address to memory
    mov qword ptr [long_jmp_rip], r8
    mov rax, 20h 
    mov word ptr [long_jmp_rip + 8], ax

    ; load GDT
    lgdt fword ptr [rcx]

    ; set page table base
    mov rax, rdx
    mov cr3, rax

    mov rax, 30h
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov rcx, offset long_jmp_rip
    ; use REX.W prefix to force far jump with m16:m64 address
    db 48h
    jmp far ptr [rcx]

goto_to_64 ENDP

END