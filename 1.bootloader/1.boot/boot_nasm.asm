; from https://github.com/yifengyou/The-design-and-implementation-of-a-64-bit-os/blob/master/code/c03/1/boot.asm
; 引导程序
; [org 0x7c00解疑](https://blog.csdn.net/judyge/article/details/52333656)
; [Interrupt Jump Table](http://www.ctyme.com/intr/int.htm)
org 0x7c00 ; org即origin, 表示起始地址, org指令本身并不能决定程序将要加载到的内存位置，它只是告诉编译器，程序在编译好后需要加载到该地址，由编译器调整好数据访问时的地址(把所有程序用到的段内偏移地址自动加上org后跟的数值); 没有org时, 程序默认使用0x0000作为起始地址, 这里没有org会出问题, 因为bios结束后是会跳到0x7c00开始执行的, 因此0x7c00为引导程序执行的起点
BaseOfStack equ 0x7c00 ; equ作用: 让其左边的标识符可代表右边的表达式, 并不会给标识符分配内存. 标识符不能重名, 也不能重新定义.
Lable_Start:
    mov ax, cs ; 用cs寄存器的段基地址设置到ax, ds, es, ss
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, BaseOfStack ; 等价于`mov sp, 0x7c00`, 初始化栈指针寄存器, 让栈从0x7c00开始增长

; [汇编中的10H中断int 10h详细说明](https://blog.csdn.net/hua19880705/article/details/8125706)
; [INT_10H](https://zh.wikipedia.org/wiki/INT_10H)
; 在没有操作系统的系统调用带来的好处时，各种功能的实现都需要用到中断，之后也会大量使用中断. 各中断的用法可以查阅处理器对应的文档.
;=======	clear screen : 因为之前bios输出过东西

	mov	ax,	0600h ; h表示16进制, **推荐使用`0x`开头的写法**. eax是32位, 其低8位是8位寄存器al, 紧靠的8位是ah; 低16位是ax. 因此ah=0x06, al=0
	mov	bx,	0700h ; `struct carattr attr = {' ', regs->bh, 1};`即attr属性的一部分
	mov	cx,	0 ; (CH、CL)＝ 窗口的左上角位置(Y坐标，X坐标)
	mov	dx,	184fh ; (DH、DL)＝窗口的右下角位置(Y坐标，X坐标)
	int	10h ; 触发中断`int	10h`. qemu默认使用seabios, 在seabios-1.13.0代码中查找`INT 10h`, 在`vgasrc/vgabios.c`中找到`handle_10`, 因ah=0x06->`handle_1006`, 为向上滚屏, AL＝滚动行数(0即清窗口)

;=======	set focus

	mov	ax,	0200h ; ah=0x02; 设置光标位置
	mov	bx,	0000h ; BH=页码
	mov	dx,	0000h ; DH＝行(Y坐标), DL＝ 列(X坐标)
	int	10h

;=======	display on screen : Start Booting......

	mov	ax,	1301h ; ah=0x13, 在Teletype模式下显示字符串; al(写入模式)=0x01, 字符串显示属性在BL中, 长度在%cx, 单位B. 显示后，光标位置改变移动至字符串尾端.
	mov	bx,	000fh ; BH=页码
	mov	dx,	0000h ; 光标开始时的(DH=行，DL=列)
	mov	cx,	10 ; 字符串长度为10
	push	ax ; sub sp, 2 + mov WORD PTR [sp], ax
	mov	ax,	ds
	mov	es,	ax ; es = ds = 0x7c00, 不直接使用`mov es, ds`的原因: mov不允许在两个段寄存器间直接传送数据
	pop	ax ; mv ax, WORD PTR [sp] + add sp, 2
	mov	bp,	StartBootMessage ; es:bp => 要显示字符串的内存地址
	int	10h

;=======	reset floppy

	xor	ah,	ah ; ah置为0x00, 重置磁盘驱动器, 为下一次读取软盘做准备
	xor	dl,	dl ; dl置为0x00, 驱动器号0x00~0x7F是软盘,每多一个软盘, 序号加一; 0x80~0xFF是硬盘, 每多一个硬盘, 序号加一
	int	13h ; 通过"INT 113"找到`src/disk.c#handle_13()`=> 找到某个可引导设备的引导扇区（MBR 扇区，Master Boot Record）的 512 个字节(前446B是boot loader)的数据加载到物理内存地址为 0x7C00 ~ 0x7E00 的区域，然后程序就跳转到 0x7C00 处开始执行

	jmp	$

StartBootMessage:	db	"Start Boot" ; 定义字符串常量值. StartBootMessage相当于c语言变量名. `DB\DW\DD\DF\DQ\DT 初值序列`分别表示告诉汇编器需要在当前内存位置写入1B,2B,4B,6B,8B,10B的数据.

;=======	fill zero until whole sector

	times	510 - ($ - $$)	db	0 ; `$ - $$`=`$-0x7c00`:本行程序距离节（section）开始处的相对距离, 因此$~510间用0填充. times不是汇编指令, 是NASM的伪指令, 用来重复定义数据或指令.
	dw	0xaa55 ; MBR(512字节) = 引导程序（446字节）+DPT分区表（64字节）+ 55AA结束标志（2字节, 因为cpu是小端, 因此内存中的顺序是0xaa55）