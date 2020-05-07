.intel_syntax noprefix
.code16

.equ BaseTmpOfKernelAddr, 0x00
.equ OffsetTmpOfKernelFile, 0x7E00 # kernel.bin的临时转存空间

.equ BaseOfKernelFile,	0x00
.equ OffsetOfKernelFile, 0x100000 # 1M处

.section .text
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

.equ RootDirSectors, 14 # 根据FAT12文件系统提供的信息经过计算而得,即(BPB_RootEntCnt * 32 + BPB_BytesPerSec-1)/BPB_BytesPerSec = (224 × 32 + 512 -1)/512 =14
.equ SectorNumOfRootDirStart, 19 # boot扇区BPB_RsvdSecCnt+FAT table的扇区数BPB_FATSz16 * BPB_NumFATs = 1 + 9*2
.equ SectorNumOfFAT1Start,	1 # FAT table开始的扇区
.equ SectorDataStart,	33	 # SectorNumOfRootDirStart + RootDirSectors #因为FAT表的第0, 1两项是保留项, 因此有效簇号从2开始. 

.include "fat12.s"

# 放到结尾不知为什么会导致symbol的地址与实际内存地址对不上, 而导致输出的字符错乱
# =======	tmp variable
RootDirSizeForLoop:	.int	RootDirSectors # 剩余待查找的扇区数
SectorNo:	.int	0
PrintLine:	.word	0x0200

# =======	display messages
DiskError:      .ascii "ERROR: Disk error!"
NoKernelMessage: .ascii	"ERROR:No KERNEL Found"
KernelFileName:	 .ascii	"KERNEL  BIN" # fat12会处理扩展名中的`.`
StartLoaderMessage:	.ascii	"Start Loader"
OpenA20Message:	.ascii	"Open A20 Done"
FindKernel: .ascii "FindKernel" # len=10

Label_Start:
	mov	ax,	cs
	mov	ds,	ax
	mov	es,	ax
	mov	ax,	0x00
	mov	ss,	ax
	mov	sp,	0x7c00


# =======	display on screen : Start Loader......

	mov cx,12
	lea bp, StartLoaderMessage
	call PrintInfo

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

	mov cx,13
	lea bp, OpenA20Message
	call PrintInfo

# =======	reset floppy

	xor	ah,	ah
	xor	dl,	dl
	int	0x13

# =======	search kernel.bin
    mov	word ptr	[SectorNo],	SectorNumOfRootDirStart # 从RootDir开始查找loader


	mov	ax,	0x00
	mov	es,	ax # 	mov	es,	0x00, es不允许直接改
	mov	bx,	0x8000 # es:bx=>数据缓冲区 # bx最大偏移64k

Lable_Search_In_Root_Dir_Begin: # 开始查找

	cmp	word ptr	[RootDirSizeForLoop],	0
	je	Label_No_KernelBin
	dec	word ptr	[RootDirSizeForLoop]	

	mov	ax,	[SectorNo] # 插入待读取的LBA扇区号
	mov	cl,	1 # 要读入的扇区个数
	call	Func_ReadSectors

	mov	di,	0x8000
	mov	dx,	0x10 # 标记已读到一个扇区, DX寄存器记录着每个扇区可容纳的目录项个数=512/32=16

Label_Search_For_KernelBin:

	cmp	dx,	0
	je	Label_Goto_Next_Sector_In_Root_Dir # 没读到
	dec	dx

	cld # cld相对应的指令是std，二者均是用来操作方向标志位DF（Direction Flag）. cld使DF 复位，即是让DF=0，std使DF置位，即DF=1
	mov	cx,	11 # cx寄存器记录着目录项的文件名长度11B, 即文件名需比较11个字符
	lea	si,	KernelFileName # 读到数据继续执行 by ds:si, es:di with cx times
	rep cmpsb # 字符串比较指令, cmpsb会递增si, di
	je	Load_FAT # 找到了文件, 开始load fat
	
	# di =32n+x#  0<=x<=11
	# 11=0b0000 1011# di在一个目录项中，最多是di+11,而11根本不会影响倒数第5位
	# 因此and di,0fff0h#和and di ,0ffe0h没区别, 都是把之前比对的11字节对di的影响而消除了即舍弃
	and	di,	0x0ffe0

	add	di,	0x20 # 比较下一个文件项

	jmp Label_Search_For_KernelBin

Label_Goto_Next_Sector_In_Root_Dir:
	add	word ptr	[SectorNo],	1
	jmp	Lable_Search_In_Root_Dir_Begin

# =======	read one sector from floppy
# 负责实现软盘读取功能
#       absolut sector = (logical sector / sector per track) + 1
#       absolut head   = (logical sector / sector per track) % heads
#       absolut track  = logical sector / (sector per track / heads)
# 代入3.5英寸软盘(1.44M)的PS=18, PH=2:
# - C = LBA/36
# - H = (LBA/18)%2
# - S = LBA%18 + 1
# .type LBA2CHS @function
LBA2CHS:
	xor dx, dx
	div	word ptr [BPB_SecPerTrk] # 每个磁道的扇区数
	# 因为被除数16bit, 则 ax 存储除法操作的商, dx 存储除法操作的余数
	inc dl
	mov cl, dl # cl是扇区数

	xor dx, dx
	div word ptr [BPB_NumHeads]
	mov dh, dl # 磁头数

	mov ch, al # 柱面数
	ret

Func_ReadSectors:
	push	bp
	mov	bp,	sp

	push cx # 要读入的扇区个数保存
	
	call LBA2CHS

	mov	dl,	byte ptr [BS_DrvNum] # BS_DrvNum是当前中断0x13使用到的驱动器号
	mov di, 0x5 # try times

Label_Go_On_Reading:
	mov	ah,	0x2 # INT 13h , AH=02h 功能:读取磁盘扇区
	mov	al,	byte ptr [bp-2] # load读入的扇区个数
	int	0x13
	jnc	Label_Go_On_ReadingDone # 当运算产生进位标志时，即CF=1时，是失败需重试, 成功是`CF clear`

	xor ah, ah                                  # Reset Drive func of int 13h
    int 0x13                                    # Call int 13h (BIOS disk I/O)

	dec di
	jnz Label_Go_On_Reading # 不为0则跳转

	jmp Desk_Err
Label_Go_On_ReadingDone:
	mov sp, bp
	pop	bp
	ret

# todo 合并打印by call
# 没找到loader.bin
Label_No_KernelBin:
	mov cx,21
	lea bp, NoKernelMessage # NoKernelMessage在0x10000之上, bp溢出, 导致打印乱码
	call PrintError
	jmp $

Desk_Err:
	mov cx,18
	lea bp, DiskError
	call PrintError
	jmp $

# .type Print @function
PrintInfo:
	push dx

	mov	ax,	0x1301
	mov	bx,	0x000f
	mov dx, [PrintLine]
	push ax
	mov	ax,	ds
	mov	es,	ax
	pop ax

	int	0x10

	add dx, 0x0100
	mov [PrintLine], dx
	pop dx

	ret

PrintError:
	mov	ax,	0x1301
	mov	bx,	0x008c
	push ax
	mov	ax,	ds
	mov	es,	ax
	pop ax
	int	0x10
	ret

# =======	found loader.bin name in root director struct

# 载入整个fat tab
Load_FAT:
	and	di,	0x0ffe0
	add di, 0x1a # 起始簇号的偏移量26
	mov	cx,	word ptr es:[di] # cx是起始簇号
	push	cx # 保存簇号, 因为Load_File用到. 空文件的簇号是0, 要处理 # TODO

	mov	ax,	0x00
	mov	es,	ax
	mov	bx,	0x8000 # es:bx=>数据缓冲区
	mov	ax,	word ptr [BPB_RsvdSecCnt] # 待读取的起始LBA扇区号
	mov	cx,	word ptr [BPB_FATSz16] # 要读入的扇区个数
	call	Func_ReadSectors

Load_File:
	# 设置[es:bx]指定loader.bin在内存中的起始位置
	mov	ax,	BaseTmpOfKernelAddr
	mov	es,	ax
	mov	di,	OffsetTmpOfKernelFile	

	mov cx,10
	lea bp, FindKernel # NoKernelMessage在0x10000之上, bp溢出, 导致打印乱码
	call PrintInfo
	jmp $
	
	pop ax # 取Load_FAT保存的有效簇号
	call Read_Clusters

	push	fs
	push	ds
	push eax
	push ecx
	push	edi
	push	esi

	# 计算读到的字节数
	mov cx, es
	shl ecx, 4
	and edi, 0x0000FFFF
	add ecx, edi
	sub ecx, OffsetTmpOfKernelFile

	mov	ax,	BaseOfKernelFile
	mov	fs,	ax
	mov	edi, OffsetOfKernelFile
	
	mov	ax,	BaseTmpOfKernelAddr
	mov	ds,	ax
	mov	esi,	OffsetTmpOfKernelFile

Label_Mov_Kernel:
	
	mov	al,	byte ptr ds:[esi]
	mov	byte ptr fs:[edi],	al

	inc	esi
	inc	edi

	loop	Label_Mov_Kernel # 循环次数由计数寄存器CX指定

	pop	esi
	pop	edi
	pop	ecx
	pop	eax
	pop	ds
	pop	fs

Label_File_Loaded:
	mov	ax, 0x0B800
	mov	gs, ax # 设定gs 在0xB800, 因为从0xB800开始是一段专门用于显示字符的内存空间, 每个字符占2B, 低字节保存字符, 高字节保存字符的颜色属性.
	mov	ah, 0x0F				# 0000: 黑底    1111: 白字
	mov	al, 'G'
	mov	[gs:((80 * 0 + 39) * 2)], ax	# 屏幕第 0 行, 第 39 列

KillMotor: # 关闭软盘
	
	push	dx
	mov	dx,	0x03F2
	mov	al,	0	
	out	dx,	al # 通过io端口实现
	pop	dx

	jmp $

Read_Clusters:
#
# Read file clusters, starting at the given cluster,
# expects FAT to be loaded into the disk buffer.
#
# Expects: AX    = Starting cluster
#          ES:DX = Location to load clusters
#
# Returns: None
#
# --------------------------------------------------
  .clusterLoop:
    push ax
    sub ax, 2 # 有效簇是从2开始的

	xor bh, bh # 避免下面的乘法受错误影响
	mov bl, byte ptr [BPB_SecPerClus]
	xor dx, dx
	mul bx # Multiply (cluster - 2) * sectorsPerCluster. 16*16时ax存低位  dx存高位 # 1.44M=1440k, 最多=1440*1024/512=2880个扇区, 一定在ax范围内. max(ax)=63336, 一次这里不用管dx
	add ax, SectorDataStart # 数据的开始扇区号
	
	xor ch, ch # 避免下面的Func_ReadSectors受错误影响
	mov cl,      byte ptr [BPB_SecPerClus]      # N Sectors to read
	mov bx, di # int 13始终使用es:bx作为缓冲区地址
	call Func_ReadSectors                            # Read the sectors

    pop ax                                      # Current cluster number

  	# Get the next cluster for FAT12 (cluster + (cluster * 1.5))
	xor bh, bh
    mov bl, 3                                   # We want to multiply by 1.5 so divide by 3/2 
    mul bx                                      # Multiply the cluster by the numerator
		# 16*16时ax存低位  dx存高位
    mov bl, 2                                   # Return value in ax and remainder in dx
    div bx                                      # Divide the cluster by the denominator. 
		# 如果除数为16位，则AX存储除法操作的商，DX存储除法操作的余数. 余数dx与ax的奇偶性相同
   
    push ds
    push si
               
    mov si, 0x0   # Tempararly use fat tab
    mov ds, si
    mov si, 0x8000

    add si, ax                                  # Point to the next cluster in the FAT entry
    mov ax, word ptr ds:[si]                        # Load ax to the next cluster in FAT
    
    pop si
    pop ds
    
    and dx, 0x1                                   # Is the cluster caluclated even?
    jz .evenCluster

  .oddCluster: # 奇数
    shr ax, 4
    jmp .nextClusterCalculated

  .evenCluster: # 偶数
    and ax, 0x0fff                              # Drop the last 4 bits of next cluster
        
  .nextClusterCalculated:
    cmp ax, 0x0ff8                              # Are we at the end of the file?
    jge .done

	xchg cx,ax # cx 保存next cluster
   	xor dx, dx
    xor bh, bh
	mov bl, byte ptr [BPB_SecPerClus]            # and mul that by the sectors per cluster
	mov ax, word ptr [BPB_BytesPerSec]
	mul bx # 考虑实际=>bx为0
	xchg ax,cx # ax 保存next cluster, cx 保存偏移变化

    clc # 清除CF位(进位标记)
    add di, cx                                 # Add to the pointer offset
    jnc .clusterLoop # 没有进位, 进位则表示buffer要溢出了 

  .fixBuffer:                                   # An error will occur if the buffer in memory???
    mov dx, es                                  # overlaps a 64k page boundry, when di overflows
    add dh, 0x10                                # it will trigger the carry flag, so correct
    mov es, dx                                  # extra segment by 0x1000
	# es+=(0x1000 << 4) = 64k, di溢出部分会被丢弃, 加上进位后的es,刚好相等 

    jmp .clusterLoop                            # Load the next file cluster

  .done:
    ret
