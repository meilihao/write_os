#include "mmu.h"

  # vectors.S sends all traps here.
# 首先压入其他的寄存器形成一个完整的中断帧, 然后设置内核的数据段寄存器，接着调用trap函数，trap函数的参数的中断帧地址，所以把esp压栈。当trap函数结束后，恢复栈指针
; alltraps将进程相关的寄存器压入到ssX:espX指定的栈中，这里是p->kstack指向的进程内核栈
; pushal后, 整个栈此时就是一个trapframe结构:
;                  /   +---------------+ <-- stack base(= p->kstack + KSTACKSIZE)
;                  |   | ss            |                           
;                  |   +---------------+                           
;                  |   | esp           |                           
;                  |   +---------------+                           
;                  |   | eflags        |                           
;                  |   +---------------+                           
;                  |   | cs            |                           
;                  |   +---------------+                           
;                  |   | eip           | <-- 这里往上在iret时自动弹出到相关寄存器中
;                  |   +---------------+    
;                  |   | err           |  
;                  |   +---------------+  
;                  |   | trapno        |  
;                  |   +---------------+                       
;                  |   | ds            |                           
;                  |   +---------------+                           
;                  |   | es            |                           
;                  |   +---------------+                           
;                  |   | fs            |                           
; struct trapframe |   +---------------+                           
;                  |   | gs            |                           
;                  |   +---------------+   
;                  |   | eax           |   
;                  |   +---------------+   
;                  |   | ecx           |   
;                  |   +---------------+   
;                  |   | edx           |   
;                  |   +---------------+   
;                  |   | ebx           |   
;                  |   +---------------+                        
;                  |   | oesp          |   
;                  |   +---------------+   
;                  |   | ebp           |   
;                  |   +---------------+   
;                  |   | esi           |   
;                  |   +---------------+   
;                  |   | edi           |   
;                  \   +---------------+ <-- %esp
.globl alltraps
alltraps:
  # Build trap frame.
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs
  pushal
  
  # Set up data segments.
  #此时已经由p->tf->esp指向的进程用户栈切换到ssX:espX指定的栈中，
  #这里是p->kstack指向的进程内核栈，当前进程相关的寄存器保存到了进程内核栈中
  movw $(SEG_KDATA<<3), %ax
  movw %ax, %ds
  movw %ax, %es

  # Call trap(tf), where tf=%esp
  #push %esp的原因是将当前esp指向的tf结构基地址作为传递的参数
  pushl %esp
  call trap # trap.c call指令将IP或者CS和IP压入栈中，实现转移
  addl $4, %esp # 干掉之前压栈的esp, 完成之后%esp又指向trapframe结构

# 当中断函数处理完后，调用trapret
# trapret首先弹出一些寄存器，然后调整栈指针到保存eip值的地方，这样执行iret指令后，处理器就回返回到原来被打断的代码处继续执行
#主要是弹出保存在ssX:espX指定的栈中的进程相关的寄存器，返回执行进程代码。
  #第一次切换到用户态执行代码时，ssX:espX指定的栈中的进程相关的寄存器是在userinit函数中指定的
  # Return falls through to trapret...
/* `addl $0x8, %esp`前=中断调用alltraps前:
 +---------------+ <-- stack base(= p->kstack + KSTACKSIZE)
 | ss            |                           
 +---------------+                           
 | esp           |                           
 +---------------+                           
 | eflags        |                           
 +---------------+                           
 | cs            |                           
 +---------------+                           
 | eip           |
 +---------------+
 | err           |
 +---------------+
 | trapno        |
 +---------------+ <-- %esp
  */
.globl trapret
trapret:
  popal #弹出通用寄存器中的上下文环境
  popl %gs
  popl %fs
  popl %es
  popl %ds
  addl $0x8, %esp  # skip trapno and errcode
  iret # initcode进程执行时: esp指向进程tf的eip, 这个栈分布就和从用户空间产生中断陷入内核空间后，cpu硬件在内核栈中保存的数据结构一致了，当执行iret指令后，cpu就从当前的内核级别恢复到用户级别执行eip指示的用户空间代码了，同时栈空间也被恢复为用户空间栈. 因此return后eip=0(在userinit函数中被设置为0), 开始执行initcode.S内容
# iret：用于从中断返回，会弹出IP/EIP，然后CS，以及一些标志。然后从CS：IP执行
# ret：也可以叫做近返回，即段内返回。处理器从堆栈中弹出IP或者EIP，然后根据当前的CS：IP跳转到新的执行地址。如果之前压栈的还有其余的参数，则这些参数也会被弹出