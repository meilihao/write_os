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

   mov byte [gs:320], 'P' ; 错开其他打印信息. 320/(80*2, 每个字符占2B) = 2, 即第三行

   ; -------------------------   加载kernel img  ----------------------
   mov esi, KERNEL_START_SECTOR        ; kernel.bin所在的扇区号
   mov edi, KERNEL_BIN_BASE_ADDR       ; 从磁盘读出后，写入到ebx指定的地址
   mov cl, 200                ; 读入的扇区数

   call rd_disk_m_32

   ; 创建页目录及页表并初始化页内存位图
   call setup_page

;要将描述符表地址及偏移量写入内存gdt_ptr,一会用新地址重新加载
   sgdt [gdt_ptr]	      ; 存储到原来gdt所有的位置

   ; 打印功能将来也是在内核中实现, 肯定不能让用户进程直接能控制显存. 故显存段的段基址也要改为 3GB 以上才行
   ;将gdt描述符中视频段描述符中的段基址+0xc0000000
   mov ebx, [gdt_ptr + 2]  ; 得到gdt的首地址
   or dword [ebx + 0x18 + 4], 0xc0000000      ;视频段是第3个段描述符,每个描述符是8字节,故0x18。
					      ;段描述符的高4字节的最高位是段基址的31~24位

   ; 修改 GDT 基址
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
   ; 无需更新SELECTOR_VIDEO, 因为它保持的是相对gdt的索引

   mov byte [gs:480], 'V'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:482], 'i'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:484], 'r'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:486], 't'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:488], 'u'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:490], 'a'     ;视频段段基址已经被更新,用字符v表示virtual addr
   mov byte [gs:492], 'l'     ;视频段段基址已经被更新,用字符v表示virtual addr

   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;  此时不刷新流水线也没问题  ;;;;;;;;;;;;;;;;;;;;;;;;
;由于一直处在32位下,原则上不需要强制刷新,经过实际测试没有以下这两句也没问题.
;但以防万一，还是加上啦，免得将来出来莫句奇妙的问题.
   jmp SELECTOR_CODE:enter_kernel     ;强制刷新流水线,更新gdt
enter_kernel:    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   mov byte [gs:640], 'k'     ;视频段段基址已经被更新
   mov byte [gs:642], 'e'     ;视频段段基址已经被更新
   mov byte [gs:644], 'r'     ;视频段段基址已经被更新
   mov byte [gs:646], 'n'     ;视频段段基址已经被更新
   mov byte [gs:648], 'e'     ;视频段段基址已经被更新
   mov byte [gs:650], 'l'     ;视频段段基址已经被更新

   call kernel_init
   mov esp, 0xc009f000 ; 涉及以后pcb(程序控制块), 需要4k对齐, 9f000是最接近最大可用地址 Ox9fbff的
   jmp KERNEL_ENTRY_POINT                 ; 用地址0x1500访问测试，结果ok

;-----------------   将kernel.bin中的segment拷贝到编译的地址   -----------
; [手拆ELF32（一，文件头）](https://blog.csdn.net/qq_36963214/article/details/109047922)
; [计算机那些事(4)——ELF文件结构](http://chuquan.me/2018/05/21/elf-introduce/)
; [ELF格式解读-程序头与内存布局](https://blog.csdn.net/qfanmingyiq/article/details/124544305)
kernel_init:
   xor eax, eax
   xor ebx, ebx      ;ebx记录程序头表地址
   xor ecx, ecx      ;cx记录程序头表中的program header数量
   xor edx, edx      ;dx 记录program header尺寸,即e_phentsize

   mov dx, [KERNEL_BIN_BASE_ADDR + 42]   ; 偏移文件42字节处的属性是e_phentsize,表示program header大小
   mov cx, [KERNEL_BIN_BASE_ADDR + 44]    ; 偏移文件开始部分44字节的地方是e_phnum,表示有几个program header
   mov ebx, [KERNEL_BIN_BASE_ADDR + 28]   ; 偏移文件开始部分28字节的地方是e_phoff,表示第1 个program header在文件中的偏移量
                 ; 程序头表是程序头 program header 的数组
   add ebx, KERNEL_BIN_BASE_ADDR
.each_segment: ; 遍历程序头表
   cmp byte [ebx + 0], PT_NULL        ; 若p_type等于 PT_NULL,说明此program header未使用。
   je .PTNULL

   ;为函数memcpy压入参数,参数是从右往左依然压入.函数原型类似于 memcpy(dst,src,size)
   push dword [ebx + 16]        ; program header中偏移16字节的地方是p_filesz,压入函数memcpy的第三个参数:size
   mov eax, [ebx + 4]           ; 距程序头偏移量为4字节的位置是p_offset
   add eax, KERNEL_BIN_BASE_ADDR   ; 加上kernel.bin被加载到的物理地址,eax为该段的物理地址
   push eax            ; 压入函数memcpy的第二个参数:源地址
   push dword [ebx + 8]         ; 压入函数memcpy的第一个参数:目的地址,偏移程序头8字节的位置是p_vaddr，这就是目的地址
   call mem_cpy              ; 调用mem_cpy完成段复制
   add esp,12             ; 清理栈中压入的三个参数
.PTNULL:
   add ebx, edx              ; edx为program header大小,即e_phentsize,在此ebx指向下一个program header 
   loop .each_segment
   ret

;----------  逐字节拷贝 mem_cpy(dst,src,size) ------------
;输入:栈中三个参数(dst,src,size)
;输出:无
;---------------------------------------------------------
mem_cpy:          
   cld ; cld 是指 clean direction, 该指令是将 etlags 寄存器中的方向标志位 DF 置为 0, 这样 rep 在循环执行后面的字符串指令时,［e]si 和［e]di 根据使用的字符串搬运指令, 自动加上所搬运数据的字节大小. std 是 set direction, 该指令是将方向标志位DF 置为 1, 每次 rep 循环执行后面字符串指令时,［e]si 和［e]di 自动减去所搬运数据的字节大小
   push ebp
   mov ebp, esp
   push ecx       ; rep指令用到了ecx，但ecx对于外层段的循环还有用，故先入栈备份
   mov edi, [ebp + 8]      ; dst
   mov esi, [ebp + 12]     ; src
   mov ecx, [ebp + 16]     ; size
   rep movsb         ; 逐字节拷贝. 执行一次 movsb, [e]si和［e]di 就自动加 1 

   ;恢复环境
   pop ecx     
   pop ebp
   ret

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
   add eax, 0x1000 			     ; 此时eax为第一个页表的位置及属性(后3B), 0x1000=4096, eax=0x101000
   mov ebx, eax				     ; 此处为ebx赋值，是为.create_pte做准备，ebx为基址。

;   下面将页目录项0和0xc00都存为第一个页表的地址，
;   一个页表可表示4MB内存,这样0xc0000000~0xc03fffff和0~0x003fffff的虚拟地址都指向相同的页表
;   这是为将地址映射为内核地址做准备
;   kernel(计划70k以内)准备放在低端1MB内
   or eax, PG_US_U | PG_RW_W | PG_P	     ; 页目录项的属性RW和P位为1,US为1,表示用户属性,所有特权级别都可以访问.
                                         ; 使用PG_US_U原因: 将来的init需要访问内核
   mov [PAGE_DIR_TABLE_POS + 0x0], eax       ; 第1个目录项,在页目录表中的第1个目录项写入第一个页表的位置(0x101000)及属性(7)
   mov [PAGE_DIR_TABLE_POS + 0xc00], eax     ; 一个页表项占用4字节,0xc00表示第768个页表占用的目录项,0xc00以上的目录项用于内核空间,
					     ; 也就是页表的0xc0000000~0xffffffff共计1G属于内核,0x0~0xbfffffff共计3G属于用户进程. 用户占3/4页表项, 因此内核目录项开始位置=(1024*3/4)*4=3072=0xc00
                    ; (0,0xc00)是用户空间页表项的偏移范围, 
                    ; 0和0xc00都指向第一个页表(0~0x3fffff, 包含1MB)原因: 当前loader在1MB内运行, 必须保证之前段机制下的线性地址和分页后的虚拟地址对应的物理地址一致. kernel将来指向的物理地址范围是低端4M, 只是只用了 1MB 的空间, 其余 3MB 并未使用
   sub eax, 0x1000
   mov [PAGE_DIR_TABLE_POS + 4092], eax	     ; 使最后一个目录项指向页目录表自己的首地址. eax=Ox100007. 用虚拟地址访问页表的关键步骤, 此时是将页目录项当做一个页表来使用, 叫页目录自映射

;下面创建页表项(PTE)
   mov ecx, 256				     ; 1M低端内存 / 每页大小4k = 256
   mov esi, 0
   mov edx, PG_US_U | PG_RW_W | PG_P	     ; 属性为7,US=1,RW=1,P=1
.create_pte:				     ; 创建Page Table Entry
   mov [ebx+esi*4],edx			     ; 此时的ebx已经在上面通过eax赋值为0x101000,也就是第一个页表的地址 
   add edx,4096      ; 每页4k
   inc esi
   loop .create_pte

; 创建内核其它页表的PDE
; 为了真正实现内核被所有进程共享, 还是在页目录表中为内核额外安装了 254个页表的 PDE （第 255 PDE 已经指向了页目录表本身）, 也就是内核空间的实际大小是 4GB 减去 4MB
; 将来要完成的任务是让每个用户进程都有独立的页表，也就是独立的虚拟 4GB 空间, 其中低 3GB 属于用户进程自己的空间, 高 1GB 是内核空间，内核将被所有用户进程共享. 为了实现所有用户进程共享内核, 各用户进程的高 1GB 必须“都”指向内核所在的物理内存空间
; 进程陷入内核时，假设内核为了某些需求为内核空间新增页表（通常是申请大量内存），因此还需要把新内核页表同步到其他进程的页表中，否则内核无法被“完全”共享，只能是“部分”共享。所以，实现内核完全共享最简单的办法是提前把内核的所有页目录项定下来，也就是提前把内核的页表固定下来，这是实现内核共享的关键
   mov eax, PAGE_DIR_TABLE_POS
   add eax, 0x2000 		     ; 此时eax为第二个页表的位置
   or eax, PG_US_U | PG_RW_W | PG_P  ; 页目录项的属性US,RW和P位都为1
   mov ebx, PAGE_DIR_TABLE_POS
   mov ecx, 254			     ; 范围为第769~1022的所有目录项数量, 1023已指向页目录表自己的首地址
   mov esi, 769
.create_kernel_pde:
   mov [ebx+esi*4], eax
   inc esi
   add eax, 0x1000
   loop .create_kernel_pde
   ret

rd_disk_m_32:
    ; 1: 检查disk status
    mov dx,0x1f7     ; 0x1f7=primary channel's status
.not_ready1:
    nop              ; 只是为了增加延迟
    in al,dx
    and al,0xc0      ; 0xc0=1100_0000b取bit 6~7
    cmp al,0x40      ; 检查bit 6, 设备是否就绪
    jnz .not_ready1  ;若未准备好，继续等
    ; 2: 设置要读取的扇区数
    mov dx,0x1f2         ; 0x1f2=primary channel's sector count, 8 位寄存器，最大值为 255 ，若指定为 0，则表示要操作 256 个扇区, 见`硬盘控制器主要端口寄存器`
    mov al,cl
    out dx,al            ;读取的扇区数
    ; 3: 将LBA地址存入0x1f3 ~ 0x1f6
    mov eax,esi
    ;LBA地址7~0位写入端口0x1f3
    mov dx,0x1f3      ;   0x1f3=primary channel's lba low
    out dx,al

    ;LBA地址15~8位写入端口0x1f4
    shr eax,8         ;   eax值右移8位
    mov dx,0x1f4      ;   0x1f4=primary channel's lba mid
    out dx,al

    ;LBA地址23~16位写入端口0x1f5
    shr eax,8
    mov dx,0x1f5      ;   0x1f5=primary channel's lba high
    out dx,al

    ; 4: 设置device端口
    shr eax,8
    and al,0x0f      ; lba第24~27位, 其他bit置为0
    or al,0xe0       ; 设置7～4位为1110,表示lba模式, 并使用主盘
    mov dx,0x1f6     ; 0x1f6=primary channel's device
    out dx,al

    ; 5：向0x1f7端口写入读命令，0x20
    mov dx,0x1f7     ; 0x1f7=primary channel's status
    mov al,0x20      ; 0x20, 读取扇区        
    out dx,al

    mov bl,cl

.next_sector:
    ; 6: 检查disk status
.not_ready2:
    mov dx,0x1f7
    in al,dx         ; 因为status 寄存器依然是 0x1f7 端口, 所以不需要再为dx 重新赋值
    and al,0x88      ;第4位为1表示硬盘控制器已准备好数据传输，第7位为1表示硬盘忙
    cmp al,0x08
    jnz .not_ready2       ;若未准备好，继续等

    ; 7：从0x1f0端口读数据. data 寄存器是 16 位，即每次 in 操作只读入 2 字节
    mov cx, 256       ; cx是操作次数. 一个扇区有512字节，每次读入2个字，共需512/2次=256
    mov dx, 0x1f0    ; 0x1f0=primary channel's data
.go_on_read:
    in ax,dx
    mov [edi],ax      ; 使用edi, 因为KERNEL_BIN_BASE_ADDR超过了di
    add edi,2
    loop .go_on_read ; loop会cx-=1, 并判断cx是否为0进而继续循环还是往下走
    dec bl
    cmp bl,0
    jnz .next_sector
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