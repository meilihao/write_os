;主引导程序 from https://github.com/yifengyou/os-elephant/tree/master/code/c03/b/boot. 这里的mbr来自../loader/mbr.s, 只是修改了`读入扇区数`
;------------------------------------------------------------
SECTION MBR vstart=0x7c00         
   mov ax,cs      
   mov ds,ax
   mov es,ax
   mov ss,ax
   mov fs,ax
   mov sp,0x7c00
   mov ax,0xb800
   mov gs,ax

; 清屏
;利用0x06号功能，上卷全部行，则可清屏。
; -----------------------------------------------------------
;INT 0x10   功能号:0x06	   功能描述:上卷窗口
;------------------------------------------------------
;输入：
;AH 功能号= 0x06
;AL = 上卷的行数(如果为0,表示全部)
;BH = 上卷行属性
;(CL,CH) = 窗口左上角的(X,Y)位置
;(DL,DH) = 窗口右下角的(X,Y)位置
;无返回值：
   mov     ax, 0600h
   mov     bx, 0700h
   mov     cx, 0                   ; 左上角: (0, 0)
   mov     dx, 184fh		   ; 右下角: (80,25),
				   ; 因为VGA文本模式中，一行只能容纳80个字符,共25行。
				   ; 下标从0开始，所以0x18=24,0x4f=79
   int     10h                     ; int 10h

   ; 输出字符串:MBR
   mov byte [gs:0x00],'1'
   mov byte [gs:0x01],0xA4

   mov byte [gs:0x02],' '
   mov byte [gs:0x03],0xA4

   mov byte [gs:0x04],'M'
   mov byte [gs:0x05],0xA4	   ;A表示绿色背景闪烁，4表示前景色为红色

   mov byte [gs:0x06],'B'
   mov byte [gs:0x07],0xA4

   mov byte [gs:0x08],'R'
   mov byte [gs:0x09],0xA4
	 
   mov eax,LOADER_START_SECTOR	 ; 起始扇区lba地址
   mov bx,LOADER_BASE_ADDR       ; 写入的地址
   mov cx,4			 ; 待读入的扇区数, 4: 因为loader.bin超过512
   call rd_disk_m_16		 ; 以下读取程序的起始部分（一个扇区）
  
   jmp LOADER_BASE_ADDR
       
;-------------------------------------------------------------------------------
;功能:读取硬盘n个扇区
rd_disk_m_16:	   
;-------------------------------------------------------------------------------
				       ; eax=LBA扇区号
				       ; ebx=将数据写入的内存地址
				       ; ecx=读入的扇区数
      mov esi,eax	  ;备份eax, in/out 需要ax
      mov di,cx		  ;备份cx, 下面读取数据时会用到cx
;读写硬盘:
;第1步：设置要读取的扇区数
      mov dx,0x1f2         ; 0x1f2=primary channel's sector count, 见`硬盘控制器主要端口寄存器`
      mov al,cl
      out dx,al            ;读取的扇区数

      mov eax,esi	   ;恢复ax

;第2步：将LBA地址存入0x1f3 ~ 0x1f6

      ;LBA地址7~0位写入端口0x1f3
      mov dx,0x1f3      ;   0x1f3=primary channel's lba low        
      out dx,al                          

      ;LBA地址15~8位写入端口0x1f4
      mov cl,8
      shr eax,cl        ;   eax值右移8位
      mov dx,0x1f4      ;   0x1f4=primary channel's lba mid
      out dx,al

      ;LBA地址23~16位写入端口0x1f5
      shr eax,cl
      mov dx,0x1f5      ;   0x1f5=primary channel's lba high
      out dx,al

      shr eax,cl
      and al,0x0f	   ; lba第24~27位, 其他bit置为0
      or al,0xe0	   ; 设置7～4位为1110,表示lba模式, 并使用主盘
      mov dx,0x1f6     ; 0x1f6=primary channel's device
      out dx,al

;第3步：向0x1f7端口写入读命令，0x20 
      mov dx,0x1f7     ; 0x1f7=primary channel's status
      mov al,0x20      ; 0x20, 读取扇区              
      out dx,al

;第4步：检测硬盘状态即检测 status 寄存器的 BSY 位
  .not_ready:
      ;同一端口，写时表示写入命令字，读时表示读入硬盘状态
      nop              ; 只是为了增加延迟
      in al,dx         ; 因为status 寄存器依然是 0x1f7 端口, 所以不需要再为dx 重新赋值
      and al,0x88	   ;第4位为1表示硬盘控制器已准备好数据传输，第7位为1表示硬盘忙
      cmp al,0x08
      jnz .not_ready	   ;若未准备好，继续等

;第5步：从0x1f0端口读数据. data 寄存器是 16 位，即每次 in 操作只读入 2 字节
      mov ax, di       ; ax=1(1个扇区)
      mov dx, 256
      mul dx           ; dx=dx*ax即需要操作的次数. 如果操作数是 8 位，被乘数就是 al 寄存器的值，乘积就是 16 位，位于缸寄存器。如果操作数是 16 位，被乘数就是 ax 寄存器的值，乘积就是 32位，积的高 16 位在 dx 寄存器，积的低 16 位在 ax 寄存器.
      mov cx, ax	   ; ax是操作次数. di为要读取的扇区数，一个扇区有512字节，每次读入2个字，共需di*512/2次，所以di*256
      mov dx, 0x1f0    ; 0x1f0=primary channel's data
  .go_on_read:
      in ax,dx
      mov [bx],ax      ; bx初始值是LOADER_BASE_ADDR
      add bx,2		  
      loop .go_on_read ; loop会cx-=1, 并判断cx是否为0进而继续循环还是往下走
      ret

   times 510-($-$$) db 0
   db 0x55,0xaa

%include "boot.inc"