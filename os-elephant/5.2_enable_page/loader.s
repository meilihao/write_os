  ; from https://github.com/yifengyou/os-elephant/blob/master/code/c04/a/boot/loader.S
   section loader vstart=LOADER_BASE_ADDR

;打印字符，"2 LOADER"说明loader已经成功加载
; 输出背景色绿色，前景色红色，并且跳动的字符串"1 MBR"
mov byte [gs:160],'2'
mov byte [gs:161],0xA4     ; A表示绿色背景闪烁，4表示前景色为红色

mov byte [gs:162],' '
mov byte [gs:163],0xA4

mov byte [gs:164],'L'
mov byte [gs:165],0xA4   

mov byte [gs:166],'O'
mov byte [gs:167],0xA4

mov byte [gs:168],'A'
mov byte [gs:169],0xA4

mov byte [gs:170],'D'
mov byte [gs:171],0xA4

mov byte [gs:172],'E'
mov byte [gs:173],0xA4

mov byte [gs:174],'R'
mov byte [gs:175],0xA4




;------------------------------------------------------------
;INT 0x10    功能号:0x13    功能描述:打印字符串
;------------------------------------------------------------
;输入:
;AH 子功能号=13H
;BH = 页码
;BL = 属性(若AL=00H或01H)
;CX＝字符串长度
;(DH、DL)＝坐标(行、列)
;ES:BP＝字符串地址 
;AL＝显示输出方式
;   0——字符串中只含显示字符，其显示属性在BL中。显示后，光标位置不变
;   1——字符串中只含显示字符，其显示属性在BL中。显示后，光标位置改变
;   2——字符串中含显示字符和显示属性。显示后，光标位置不变
;   3——字符串中含显示字符和显示属性。显示后，光标位置改变
;无返回值
   mov	 sp, LOADER_BASE_ADDR
   mov	 bp, loadermsg           ; ES:BP = 字符串地址
   mov	 cx, 17			 ; CX = 字符串长度
   mov	 ax, 0x1301		 ; AH = 13,  AL = 01h
   mov	 bx, 0x00A4		 ; 页号为0(BH = 0) 蓝底粉红字(BL = 1fh). 这里换成0xA4, 因为1fh不明显
   mov	 dx, 0x1800		 ; 0x18=24, 即屏幕最后一行
   int	 0x10                    ; 10h 号中断

;----------------------------------------   准备进入保护模式   ------------------------------------------
									;1 打开A20
									;2 加载gdt
									;3 将cr0的pe位置1


   ;-----------------  打开A20  ----------------
   in al,0x92
   or al,0000_0010B    ; 打开 A20Gate 的方式: 将端口 Ox92 的第 1 位置 1 即可
   out 0x92,al

   ;-----------------  加载GDT  ----------------
   cli
   lgdt [gdt_ptr]


   ;-----------------  cr0第0位置1  ----------------
   mov eax, cr0
   or eax, 0x00000001
   mov cr0, eax
   ; 下面开始进入16位保护模式
   jmp  SELECTOR_CODE:p_mode_start	     ; 刷新流水线，避免分支预测的影响,这种cpu优化策略，最怕jmp跳转，
					     ; 这将导致之前做的预测失效，从而起到了刷新的作用。
                    ; jmp后没有指定dword原因, 虽然已进入保护模式但是处于16位保护模式, 因为SELECTOR_CODE还未载入cpu, 当前段描述符缓冲寄存器中的 D 位是0, 因此操作数是 16 位, 故可省略dword

[bits 32]
p_mode_start:
   mov ax, SELECTOR_DATA
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov esp,LOADER_STACK_TOP
   mov ax, SELECTOR_VIDEO
   mov gs, ax

   mov byte [gs:320], 'P' ; 320, 错开其他打印信息

   ; 创建页目录及页表并初始化页内存位图
   call setup_page

;要将描述符表地址及偏移量写入内存gdt_ptr,一会用新地址重新加载
   sgdt [gdt_ptr]	      ; 存储到原来gdt所有的位置

   ;将gdt描述符中视频段描述符中的段基址+0xc0000000
   mov ebx, [gdt_ptr + 2]  
   or dword [ebx + 0x18 + 4], 0xc0000000      ;视频段是第3个段描述符,每个描述符是8字节,故0x18。
					      ;段描述符的高4字节的最高位是段基址的31~24位

   ;将gdt的基址加上0xc0000000使其成为内核所在的高地址
   add dword [gdt_ptr + 2], 0xc0000000

   add esp, 0xc0000000        ; 将栈指针同样映射到内核地址

   ; 把页目录地址赋给cr3
   mov eax, PAGE_DIR_TABLE_POS
   mov cr3, eax

   ; 打开cr0的pg位(第31位)
   mov eax, cr0
   or eax, 0x80000000
   mov cr0, eax

   ;在开启分页后,用gdt新的地址重新加载
   lgdt [gdt_ptr]             ; 重新加载

   mov byte [gs:160], 'V'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:162], 'i'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:164], 'r'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:166], 't'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:168], 'u'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:170], 'a'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:172], 'l'     ;视频段段基址已经被更新,用字符v表示virtual addr

   jmp $

;-------------   创建页目录及页表   ---------------
setup_page:
;先把页目录占用的空间逐字节清0
   mov ecx, 4096 ; 1k*4
   mov esi, 0
.clear_page_dir:
   mov byte [PAGE_DIR_TABLE_POS + esi], 0
   inc esi
   loop .clear_page_dir

;开始创建页目录项(PDE)
.create_pde:				     ; 创建Page Directory Entry
   mov eax, PAGE_DIR_TABLE_POS
   add eax, 0x1000 			     ; 此时eax为第一个页表的位置及属性
   mov ebx, eax				     ; 此处为ebx赋值，是为.create_pte做准备，ebx为基址。

;   下面将页目录项0和0xc00都存为第一个页表的地址，
;   一个页表可表示4MB内存,这样0xc03fffff以下的地址和0x003fffff以下的地址都指向相同的页表，
;   这是为将地址映射为内核地址做准备
   or eax, PG_US_U | PG_RW_W | PG_P	     ; 页目录项的属性RW和P位为1,US为1,表示用户属性,所有特权级别都可以访问.
   mov [PAGE_DIR_TABLE_POS + 0x0], eax       ; 第1个目录项,在页目录表中的第1个目录项写入第一个页表的位置(0x101000)及属性(7)
   mov [PAGE_DIR_TABLE_POS + 0xc00], eax     ; 一个页表项占用4字节,0xc00表示第768个页表占用的目录项,0xc00以上的目录项用于内核空间,
					     ; 也就是页表的0xc0000000~0xffffffff共计1G属于内核,0x0~0xbfffffff共计3G属于用户进程.
   sub eax, 0x1000
   mov [PAGE_DIR_TABLE_POS + 4092], eax	     ; 使最后一个目录项指向页目录表自己的地址

;下面创建页表项(PTE)
   mov ecx, 256				     ; 1M低端内存 / 每页大小4k = 256
   mov esi, 0
   mov edx, PG_US_U | PG_RW_W | PG_P	     ; 属性为7,US=1,RW=1,P=1
.create_pte:				     ; 创建Page Table Entry
   mov [ebx+esi*4],edx			     ; 此时的ebx已经在上面通过eax赋值为0x101000,也就是第一个页表的地址 
   add edx,4096      ; edx
   inc esi
   loop .create_pte

;创建内核其它页表的PDE
   mov eax, PAGE_DIR_TABLE_POS
   add eax, 0x2000 		     ; 此时eax为第二个页表的位置
   or eax, PG_US_U | PG_RW_W | PG_P  ; 页目录项的属性US,RW和P位都为1
   mov ebx, PAGE_DIR_TABLE_POS
   mov ecx, 254			     ; 范围为第769~1022的所有目录项数量
   mov esi, 769
.create_kernel_pde:
   mov [ebx+esi*4], eax
   inc esi
   add eax, 0x1000
   loop .create_kernel_pde
   ret

%include "boot.inc"
LOADER_STACK_TOP equ LOADER_BASE_ADDR; LOADER_STACK_TOP用于loader 在保护模式下的栈
;构建gdt及其内部的描述符
   GDT_BASE:   dd    0x00000000 ; GDT_BASEgdt的起始地址, 第 0 个段描述符没用.
          dd    0x00000000

   CODE_DESC:  dd    0x0000FFFF ; 代码段描述符. seg_limit=0xFFFFF*4k=4G
          dd    DESC_CODE_HIGH4

   DATA_STACK_DESC:  dd    0x0000FFFF ; 数据和栈段描述符
           dd    DESC_DATA_HIGH4

   VIDEO_DESC: dd    0x80000007         ; 显存段描述符 limit=(0xbffff-0xb8000)/4k=0x7
          dd    DESC_VIDEO_HIGH4  ; 此时dpl已改为0 
   GDT_SIZE   equ   $ - GDT_BASE ; 应该在times后, 原始loader.S应该是错误的, 因为gdt不连续了
   GDT_LIMIT   equ   GDT_SIZE -  1
   SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0         ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0. `<<3`是因为`TI_GDT + RPL0`
   SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0    ; 同上
   SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0   ; 同上 

   ;以下是定义gdt的指针，前2字节是gdt界限，后4字节是gdt起始地址

   gdt_ptr  dw  GDT_LIMIT 
       dd  GDT_BASE
   loadermsg db '2 loader in real.'