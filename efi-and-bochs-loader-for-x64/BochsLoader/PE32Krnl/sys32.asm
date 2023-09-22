; Copyright by Xiyue87 2022
.MODEL flat
.CODE

_io_in_byte PROC
    mov edx, [esp + 4]
    in al, dx
    ret
_io_in_byte ENDP

_io_out_byte PROC
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, al
    ret
_io_out_byte ENDP

_asm_delay PROC
    nop
    nop
    nop
    nop
    ret
_asm_delay ENDP

_io_in_word_string PROC
    push edi
    mov edx, [esp + 4 + 4]
    mov edi, [esp + 8 + 4]
    mov ecx, [esp + 12 + 4]
    rep insw 
    mov eax, edi
    pop edi
    ret
_io_in_word_string ENDP

END