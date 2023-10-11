# Context switch
#
#   void swtch(struct context **old, struct context *new);
# 
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.
# swtch的作用是保存相关context寄存器到cpu栈，将当前cpu的scheduler指向保存位置，然后将栈顶指针指向进程的context（进程的内核栈），将相关寄存器弹出之后执行ret指令，由于此时esp指向进程context的eip，因此将从栈中弹出该数据作为eip。该eip实际指向forkret函数入口地址，不同于call指令，调用该函数不会将下一条指令的地址进栈。因此后面forkret启动了一个log之后返回时将继续弹栈作为新的eip，即trapret地址，见tramasm.S
.globl swtch
swtch: # 调用传参压栈是从左往右
  movl 4(%esp), %eax # load *(esp + 4) into eax, that is &cpu->scheduler, pointer's addr
  movl 8(%esp), %edx # load *(esp + 8) into edx, that is p->context, pointer's value

  # Save old callee-saved registers # 进入swtch函数之前，eip已经进栈
  # 旧的context进栈，进的是CPU的context栈，不是进程内核栈
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi

  # Switch stacks
  movl %esp, (%eax) ;&cpu->scheduler=esp 即现在指向了当前cpu栈顶，保存了相关context寄存器
  movl %edx, %esp  ;esp=p->context

  # Load new callee-saved registers
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp
  ret # ret会返回"ret addr"处继续执行，也就是返回到forkret()继续执行. forkret return时会将栈中的trapret弹出给eip
