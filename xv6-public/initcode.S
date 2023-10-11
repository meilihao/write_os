# Initial process execs /init.
# This code runs in user space.

#include "syscall.h"
#include "traps.h"


# exec(init, argv)
; CPU在执行int指令时会执行以下动作：
; 从 IDT 中获得第 n 个描述符，n 就是 int 的参数。
; 检查 %cs 的域 CPL <= DPL，DPL 是描述符中记录的特权级。
; 如果目标段选择符的 PL < CPL，就在 CPU 内部的寄存器中保存user %esp 和 %ss 的值
; 从一个任务段描述符中加载 %ss0 和 %esp0
; 将 user %ss, %esp 压栈
; 将 %eflags 压栈
; 将 %cs 压栈
; 将 %eip 压栈
; 清除 %eflags 的一些位
; 设置 %cs 和 %eip 为描述符中的值
; 上面总结起来就是:
; 栈转换，stack转换到tr寄存器指定的tss里面的ss0:esp0，这是该进程对应的内核栈
; 自动将相关寄存器的内容进栈：ss, esp(如果涉及到ring变化的话), eflag, cs, eip。这样返回用户态之后才能恢复执行
; 最后, 跳转到中断处理程序处（trap.c），也就是vectorXXX处，这里为每个中断设置了一个中断处理程序，xv6的中断处理表是用vector.pl生成的(make之后见vectors.S)，基本每个表项都是一样的（上述动作还应包含触发中断时的权限检查。）
# 如果这条指令没有导致特权级转换，x86 就不会保存 %ss 和 %esp。在任何一种情况下，%eip 都指向中断描述符表中指定的地址，这个地址的第一条指令就是将要执行的下一条指令，也是 int n 的中断处理程序的第一条指令
; CPU执行int指令时自动压栈的，加上中断处理程序压栈
;                       +---------------+ <-- stack base(= p->kstack + KSTACKSIZE)
;                       | user ss       |                           
;                       +---------------+                           
;                       | user esp      | <-- 0x00000ff4(压进三个参数)                          
;                       +---------------+                           
;                       | eflags        |                           
;                       +---------------+                           
;                       | user cs       |                           
;                       +---------------+                           
;                       | eip           | <-- 0x00000013(int指令的下一条指令地址exit)
;                       +---------------+ 
;                       | err           | <-- 中断处理程序压入
;                       +---------------+
;                       | trapno        |
;                       +---------------+ <-- %esp
.globl start
start: # from user stack->kernel status
  pushl $argv
  pushl $init
  pushl $0  // where caller pc would be. 模拟函数调用的eip压栈
  movl $SYS_exec, %eax # 通过系统调用exec执行/init
  int $T_SYSCALL # traps.h # 当int指令被调用前，硬件必须保存当前的PC(eip)、eflags、esp等寄存器的值保存到栈上

# for(;;) exit();
exit:
  movl $SYS_exit, %eax
  int $T_SYSCALL
  jmp exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 }; // 数组中 end = 0 表示终点
.p2align 2
argv:
  .long init
  .long 0

