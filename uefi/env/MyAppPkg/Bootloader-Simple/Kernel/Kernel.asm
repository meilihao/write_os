BITS 64 
mov ecx, 0x0013C680 ; 分辨率: 1440*900
mov eax, 0xFF18679A ; FrameBuffer像素格式BGRR
xor rdi, rdi
mov rdi, 0x80000000 ; Gop->Mode->FrameBufferBase

Write:
    mov [rdi], eax
    add rdi, 4
    loop Write

jmp $