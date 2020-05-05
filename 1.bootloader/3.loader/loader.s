.intel_syntax noprefix
.code16

.text
.globl _start
_start:
    jmp Label_Start

# gdt # GDT的每个表项，抽象地可以看成包含四个字段的数据结构：基地址（Base），大小（Limit），标志（Flag），访问信息（Access Byte), 共8字节
gdt:
LABEL_GDT:		.int	0,0
LABEL_DESC_CODE32:	.int	0x0000FFFF,0x00CF9A00
LABEL_DESC_DATA32:	.int	0x0000FFFF,0x00CF9200

.equ GdtLen, . - LABEL_GDT # 共24B
GdtPtr: .word GdtLen -1 # 因为size为两字节，所以GDT最大的大小为65535字节（供8192表项）
        .int gdt + 0x10000 # address gdt, 因为使用了`ld -Ttext 0x0`需手动加Offset

.equ SelectorData32, LABEL_DESC_DATA32 - LABEL_GDT

# gdt64

LABEL_GDT64:		.quad	0x0000000000000000
LABEL_DESC_CODE64:	.quad	0x0020980000000000
LABEL_DESC_DATA64:	.quad	0x0000920000000000

Label_Start:
	mov	ax,	cs
	mov	ds,	ax
	mov	es,	ax
	mov	ax,	0x00
	mov	ss,	ax
	mov	sp,	0x7c00

# =======	display on screen : Start Loader......

	mov	ax,	0x1301
	mov	bx,	0x000f
	mov	dx,	0x0200		# row 2
	mov	cx,	12
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	lea	bp,	StartLoaderMessage
	int	0x10

# =======	open address A20  [A20总线](https://en.wikipedia.org/wiki/A20_line) 
    # 访问 A20快速门来开启 A20功能,
    push	ax
	in	al,	0x92
	or	al,	0b00000010
	out	0x92,	al
	pop	ax

	cli # 关闭外部中断

    # lgdt 加载保护模式数据结构信息, 再置位CR0寄存器的第0位来开启保护模式 
	.byte	0x66 # 当编译器处于 16位宽 (.code16)状态下,使用 32位宽数据指令时需要在指令前加入前缀Ox66 ,使用 32位宽地址指令时需要在指令前加入前缀Ox67. 而在32位宽(.code32) 状态下,使用 16位宽指令也需要加入指令前缀.
	lgdt	GdtPtr # 将源操作数中的值加载到全局描述符表格寄存器 (GDTR register, Global Descriptor Table，全局描述符表) 

	mov	eax,	cr0
	or	eax,	1
	mov	cr0,	eax

    # 为fs段寄存器加载新的数据段值, 加载完成后就从保护模式退出, 再开启外部中断
    # 通过设置fs开启了big real mode, 以实现real mode下的4g寻址空间
	mov	ax,	SelectorData32
	mov	fs,	ax
	mov	eax,	cr0
	and	al,	0b11111110
	mov	cr0,	eax

	sti

    mov	ax,	0x1301
	mov	bx,	0x000f
	mov	dx,	0x0200		# row 2
	mov	cx,	13
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	lea	bp,	OpenA20Message
	int	0x10

    jmp $


# =======	reset floppy

	xor	ah,	ah
	xor	dl,	dl
	int	0x13


# =======	display messages

StartLoaderMessage:	.ascii	"Start Loader"
OpenA20Message:	.ascii	"Open A20 Done"
