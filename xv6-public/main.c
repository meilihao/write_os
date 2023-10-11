#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t *kpgdir;
extern char end[]; // first address after kernel loaded from ELF file. end是变量地址0x801154d0, end在bss末尾(by kernel.ld), 见kernel.map

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  kinit1(end, P2V(4*1024*1024)); // phys page allocator [xv6 的内存管理](https://juejin.cn/post/6996936887562666015)
  kvmalloc();      // kernel page table
  mpinit();        // detect other processors
  lapicinit();     // interrupt controller
  seginit();       // segment descriptors
  picinit();       // disable pic
  ioapicinit();    // another interrupt controller
  consoleinit();   // console hardware
  uartinit();      // serial port
  pinit();         // process table
  tvinit();        // trap vectors
  binit();         // buffer cache
  fileinit();      // file table
  ideinit();       // disk 
  startothers();   // start other processors
  kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()
  userinit();      // first user process
  mpmain();        // finish this processor's setup
}

// Other CPUs jump here from entryother.S.
static void
mpenter(void)
{
  switchkvm();
  seginit();
  lapicinit();
  mpmain();
}

// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
  idtinit();       // load idt register
  xchg(&(mycpu()->started), 1); // tell startothers() we're up
  scheduler();     // start running processes
}

pde_t entrypgdir[];  // For entry.S

// Start the non-boot (AP) processors.
// 用来启动其他从核运行，大致的思想就是：首先设置好从核需要运行的代码的地址，然后使用核间中断IPI通知从核启动，从核启动后就从设置的代码地址处开始运行
// 从核的入口代码在entryother.S中，从核运行的代码地址必须在4KB对齐的地方
// 将entryothers移动到物理地址0x7000处使其能正常运行。因为这是其他CPU最初运行的内核代码，所以没有开启保护模式和分页机制，entryothers将页表设置为entrypgdir，在设置页表前，虚拟地址等于物理地址
// entryother.S是作为独立的二进制文件与内核二进制文件一起组成整体的ELF文件，通过在LD链接器中-b参数来整合一个独立的二进制文件，在内核中通过_binary_entryother_start和_binary_entryother_size来引用
static void
startothers(void)
{
  extern uchar _binary_entryother_start[], _binary_entryother_size[]; // entryothers文件在内存中的起始虚拟地址和大小, 由ld赋值
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = P2V(0x7000);
  memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == mycpu())  // We've started already.
      continue;

    // Tell entryother.S what stack to use, where to enter, and what
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    stack = kalloc(); // 为每个AP分配stack（每个CPU都一个自己的stack）
    *(void**)(code-4) = stack + KSTACKSIZE; // 设置esp, 结合entryother.S看, 比如这里对应其里面的`(start-4)`
    *(void(**)(void))(code-8) = mpenter;
    *(int**)(code-12) = (void *) V2P(entrypgdir);

    lapicstartap(c->apicid, V2P(code)); // 要启动的cpu, 要启动的cpu运行代码的地址

    // wait for cpu to finish mpmain() // mpenter's mpmain
    while(c->started == 0)
      ;
  }
}

// The boot page table used in entry.S and entryother.S.
// Page directories (and page tables) must start on page boundaries,
// hence the __aligned__ attribute.
// PTE_PS in a page directory entry enables 4Mbyte pages.
// 页目录表项的PS位置为1, 且cr4寄存器的PSE位置为1，那么CPU自动使用4M大小的内存页
// 临时页表: pde_t [1024]{[0] = (pde_t)131U, [512] = (pde_t)131U}, 当前内核最多只能使用 4MB 的内存
// |      VA      |     P   |
// |--------------|---------|
// |0 ~ 4MB       | 0 ~ 4MB |
// |2GB ~ 2GB+4MB | 0 ~ 4MB |
// entrypgdir vaddr=0x80109000(by ld map). 页表对齐: 页表条目的start vaddr需要page size对齐. entrypgdir仅需要4k(by pd size)对齐for ia32
// 分页硬件运行起来以后，处理器仍然在低地址空间执行指令因为entrypgdir被映射到低地址空间。如果xv6的entrypgdir没有第0项，电脑在运行开启分页硬件的后一条指令时将崩溃.
// 同时在启动多处理器的时候，还需要从低地址启动，因为这些CPU（non-boot CPU，也叫做AP）要需要从real mode启动，见entryOther.S
//
// 注意x86运行两种分页同时存在，比如cr4的PSE位设为1，而有些page dir表项设置PS位而有些
// 则不设置，这样就同时存在两种分页机制。为何要使用4MB页呢，考虑这种场景：kernel img
// 大小大约为1M，如果使用4M页映射kernel img，则TLB只需缓存一个页目录项即可，而如果是
// 4K页则需要256个页目录项，这么多的表项是无法都缓存到TLB中的，这会使得地址翻译变慢很多。
// 所以kernel img部分一般用一个4M页进行映射，而其他则使用4K页。
// 对于xv6来说，使用4M页只是临时，不用创建复杂的页表，内核启动之后很快就会重新创建4k页表
__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0] = (0) | PTE_P | PTE_W | PTE_PS, // 0x083
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

