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
;INT 0x10   功能号:0x06       功能描述:上卷窗口
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
   mov     dx, 184fh           ; 右下角: (80,25),
                   ; 因为VGA文本模式中，一行只能容纳80个字符,共25行。
                   ; 下标从0开始，所以0x18=24,0x4f=79
   int     10h                     ; int 10h

   ; 输出字符串:MBR
   mov byte [gs:0x00],'1'
   mov byte [gs:0x01],0xA4

   mov byte [gs:0x02],' '
   mov byte [gs:0x03],0xA4

   mov byte [gs:0x04],'M'
   mov byte [gs:0x05],0xA4     ;A表示绿色背景闪烁，4表示前景色为红色

   mov byte [gs:0x06],'B'
   mov byte [gs:0x07],0xA4

   mov byte [gs:0x08],'R'
   mov byte [gs:0x09],0xA4
     
   mov esi,LOADER_START_SECTOR   ; 起始扇区lba地址
   mov di,LOADER_BASE_ADDR            ; 写入的地址
   mov cl,4                      ; 待读入的扇区数
   call rd_disk_m_16

   jmp LOADER_BASE_ADDR

rd_disk_m_16:
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
    mov [di],ax      ; di初始值是DISK_BUFFER
    add di,2
    loop .go_on_read ; loop会cx-=1, 并判断cx是否为0进而继续循环还是往下走
    dec bl
    cmp bl,0
    jnz .next_sector
    ret

   times 510-($-$$) db 0
   db 0x55,0xaa

%include "boot.inc"