; copyright by Xiyue87 2022
.CODE

; uint8_t io_in_byte(uint16_t port);

io_in_byte PROC
    push rdx
    mov edx, ecx
    xor eax, eax
    in al, dx
    pop rdx
    ret
io_in_byte ENDP

; void io_out_byte(uint16_t port, uint8_t data);

io_out_byte PROC
    push rdx
    mov eax, edx
    mov edx, ecx
    out dx, al
    pop rdx
    ret
io_out_byte ENDP

; io_out_word(uint16_t port, uint16_t data);

io_out_word PROC
    push rdx
    mov eax, edx
    mov edx, ecx
    out dx, ax
    pop rdx
    ret
io_out_word ENDP

; unsigned long get_rip();

get_rip PROC
    pop rax
    push rax
    ret
get_rip ENDP

; unsigned long get_rsp();

get_rsp PROC
    mov rax, rsp
    ret
get_rsp ENDP

; unsigned short get_cs();

get_cs PROC
    mov ax, cs
    ret
get_cs ENDP

; unsigned short get_ss();

get_ss PROC
    mov ax, ss
    ret
get_ss ENDP

; unsigned long get_cr0();

get_cr0 PROC
    mov rax, cr0
    ret
get_cr0 ENDP

; unsigned long get_cr2();

get_cr2 PROC
    mov rax, cr2
    ret
get_cr2 ENDP

; unsigned long get_cr3();

get_cr3 PROC
    mov rax, cr3
    ret
get_cr3 ENDP

; unsigned long get_cr4();

get_cr4 PROC
    mov rax, cr4
    ret
get_cr4 ENDP
  
; void lock_int();

lock_int PROC
    cli
    ret
lock_int ENDP

; void unlock_int();

unlock_int PROC
    sti
    ret
unlock_int ENDP

; unsigned long read_msr(unsigned int msr_addr);

read_msr PROC
    push rdx
    push rcx
    
    xor rax, rax

    rdmsr
    mov cl, 32
    shl rdx, cl
    add rax, rdx

    pop rcx
    pop rdx
    ret
read_msr ENDP

END