%include "ldrasm.inc"
global _start
global realadr_call_entry
global IDT_PTR
extern ldrkrl_entry
[section .text]
[bits 32]
_start:
_entry:
	cli
	lgdt [GDT_PTR]
	lidt [IDT_PTR]
	jmp dword 0x8 :_32bits_mode

_32bits_mode:
	mov ax, 0x10	; 数据段选择子(目的)
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	xor eax,eax
	xor ebx,ebx
	xor ecx,ecx
	xor edx,edx
	xor edi,edi
	xor esi,esi
	xor ebp,ebp
	xor esp,esp
	mov esp,0x90000 ; 使得栈底指向了0x90000
	call ldrkrl_entry ; 二级引导器的主函数 ldrkrl_entry
	xor ebx,ebx
	jmp 0x2000000 ; 跳转到0x2000000的内存地址
	jmp $

; 在当前 C 函数中调用 BIOS 中断是不可能的, 因为 当前 C 代码工作在 32 位保护模式下，BIOS 中断工作在 16 位的实模式
; C 语言环境下调用 BIOS 中断，需要的步骤:
; 1. 保存 C 语言环境下的 CPU 上下文 ，即保护模式下的所有通用寄存器、段寄存器、程序指针寄存器，栈寄存器，把它们都保存在内存中
; 2. 切换回实模式，调用 BIOS 中断，把 BIOS 中断返回的相关结果，保存在内存中
; 3. 切换回保护模式，重新加载第 1 步中保存的寄存器。这样 C 语言代码才能重新恢复执行
realadr_call_entry:
	pushad
	push    ds
	push    es
	push    fs
	push    gs
	call save_eip_jmp
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad
	ret
save_eip_jmp:
	pop esi
	mov [PM32_EIP_OFF],esi
	mov [PM32_ESP_OFF],esp
	jmp dword far [cpmty_mode] ; 长跳转，表示把[cpmty_mode]处的数据装入 CS：EIP，也就是把 0x18：0x1000 装入到 CS：EIP 中.  0x18 是段描述索引, 它指向 GDT 中的 16 位代码段描述符; 0x1000 代表段内的偏移地址即realintsve.asm
cpmty_mode:
	dd 0x1000
	dw 0x18
	jmp $

GDT_START:
knull_dsc: dq 0
kcode_dsc: dq 0x00cf9a000000ffff ;a-e
kdata_dsc: dq 0x00cf92000000ffff
k16cd_dsc: dq 0x00009a000000ffff ;a-e
k16da_dsc: dq 0x000092000000ffff
GDT_END:

GDT_PTR:
GDTLEN	dw GDT_END-GDT_START-1	;GDT界限
GDTBASE	dd GDT_START

IDT_PTR:
IDTLEN	dw 0x3ff
IDTBAS	dd 0 ; 这是BIOS中断表的地址和长度
