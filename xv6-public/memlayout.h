// Memory layout

#define EXTMEM  0x100000            // Start of extended memory
#define PHYSTOP 0xE000000           // Top physical memory. 物理内存的边界224M
#define DEVSPACE 0xFE000000         // Other devices are at high addresses // 一些设备的地址，比如apic的一些寄存器

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000         // First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked

#define V2P(a) (((uint) (a)) - KERNBASE)
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts
/*
                   +-------------------+  4GB                 
                   |                   |                      
                   |                   |                      
                   |                   |                      
                   |                   |                      
                   |                   |                      
                   |                   |                      
                   |                   |                      
                   |                   |                      
                   +-------------------+                      
                   |                   |                      
 (main.c)main() -> |      kernel       |                      
                   |                   |                      
  0x100000(1MB) -> +-------------------+   (0x80100000)                 
                   |                   |                      
  0x10000(64KB) -> +elf hdr of kern img+    (tmp use. elf header content)                    
                   |                   |                      
   0x7c00 + 512 -> |      \x55\xAA     |                      
                   |                   |                      
       .gdtdesc -> +-------------------+                      
                   |                   |                      
           .gdt -> +-----+-------------+ <- gdtr(GDT Register)
                   |     |  seg null   |                      
                   | GDT |  seg code   |                      
                   |     |  seg data   |                      
                   +-----+-------------+                      
                   |                   |                      
                   |                   |                      
    bootmain()  -> |                   |                      
                   |        code       |                      
                   |                   |                      
      .start32  -> |                   |                      
                   |                   |                      
(0x7c00).start  -> +---------+---------+ <- esp               
                   |         |         |                      
                   |         v         |                      
                   |       stack       |                      
                   |                   |                      
                   |                   |                      
                   +-------------------+  0GB (0x80000000)
*/ 