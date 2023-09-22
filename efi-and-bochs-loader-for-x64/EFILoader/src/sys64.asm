; Copyright by Xiyue 2022

.CODE

io_in_byte PROC
    PUSH RDX

    MOV EDX, ECX
    XOR EAX, EAX
    IN AL, DX

    POP RDX
    RET
io_in_byte ENDP

io_out_byte PROC
    PUSH RDX

    MOV EAX, EDX
    MOV EDX, ECX
    OUT DX, AL

    POP RDX
    RET
io_out_byte ENDP

END