MBT_HDR2_MAGIC	EQU 0xe85250d6
global _start
extern inithead_entry
[section .text]
[bits 32]
_start:
	jmp _entry
ALIGN 8
mbhdr:
	DD	MBT_HDR2_MAGIC
	DD	0
	DD	mhdrend - mbhdr
	DD	-(MBT_HDR2_MAGIC + 0 + (mhdrend - mbhdr))
	DW	2, 0
	DD	24
	DD	mbhdr
	DD	_start
	DD	0
	DD	0
	DW	3, 0
	DD	12
	DD	_entry 
	DD      0  
	DW	0, 0
	DD	8
mhdrend:

_entry:
	cli

	in al, 0x70
	or al, 0x80	
	out 0x70,al ;关掉不可屏蔽中断

	lgdt [GDT_PTR]
	jmp dword 0x8 :_32bits_mode

_32bits_mode:
	mov ax, 0x10
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
	mov esp,0x7c00 ;设置栈顶为0x7c00
	call inithead_entry ; 调用inithead_entry函数在inithead.c中实现
	jmp 0x200000 ; 这个地址正是在 inithead.c 中由 write_ldrkrlfile() 函数放置的 initldrkrl.bin 文件，这一跳就进入了二级引导器的主模块入口ldrkrl32.asm



GDT_START:
knull_dsc: dq 0
kcode_dsc: dq 0x00cf9e000000ffff
kdata_dsc: dq 0x00cf92000000ffff
k16cd_dsc: dq 0x00009e000000ffff
k16da_dsc: dq 0x000092000000ffff
GDT_END:
GDT_PTR:
GDTLEN	dw GDT_END-GDT_START-1	;GDT界限
GDTBASE	dd GDT_START
