// Copyright by Xiyue87 2022

#include "ctype.h"
#include "string.h"
struct LongModeSegment
{
    uint32_t attribute;
    uint32_t reserved;
};

struct LongModeSegment LongModeSegmentTable[] =
{
    {0, 0}, /* skip first 4 segment*/
    {0, 0},
    {0, 0},
    {0, 0},

    /*
     *                     | --------------- TYPE ----------------|
     *  | P | DPL(2)   | 1 | Code | Confirm | Readable | Accessed | NotUsed(8) |
     *   15   14 - 13   12    11       10        9          8        7 - 0
     *
     *  | NotUsed(8) | Granularity | DefaultPrefix | Long Mode | AVL | NotUsed(4) |
     *     31 - 24         23            22              21      20     19 - 16
     */

     // Long Mode CS  0x20
     // 9A = (1001 1010) : P + DPL(0) + 1 + Code + Readable
     // A0 = (1010 0000) : Granularity + Long Mode
     {0xFFFF, (1 << 23) | (1 << 21) | (1 << 15) | (1 << 12) | (1 << 11) | (1 << 9) | (0xF0000)},

     // Compatibility Mode CS 0x28
     // 9A = (1001 1010) : P + DPL(0) + 1 + Code + Readable
     // C0 = (1100 0000) : Granularity + DefaultPrefix
     {0xFFFF, (1 << 23) | (1 << 22) | (1 << 15) | (1 << 12) | (1 << 11) | (1 << 9) | (0xF0000)},

     // 64 Mode data segement 0x30
     // 92 = (1001 0010) : P + 1 + Readable
     // C0 = (1100 0000) : Granularity + DefaultPrefix     
     {0xFFFF, (1 << 23) | (1 << 22) | (1 << 15) | (1 << 15) | (1 << 12) | (1 << 9) | (0xF0000)},
};

#define PLM4T_BASE              0x500000
#define PAGE_SIZE_2M            0x200000
#define PAGE_SIZE               0x1000

#define PAGE_PRESENT            (1 << 0)
#define PAGE_WRITABLE           (1 << 1)
#define PAGE_USER               (1 << 2)
#define PAGE_SIZE_FLAG          (1 << 7)

#define PAGE_ADDRESS_MASK       (~(PAGE_SIZE - 1ULL))

void goto_to_64(void* Gdt, void* PageTable, void* EntryOf64);

int Goto64(char* ImageBase)
{
    int i;
    int j;
    uint64_t GdtrBase;

    uint64_t* PLM4T = (uint64_t*)PLM4T_BASE;
    uint64_t* PDPT = (uint64_t*)(PLM4T_BASE + PAGE_SIZE);
    uint64_t* PDTBase = (uint64_t*)(PLM4T_BASE + PAGE_SIZE * 2);
    uint64_t* PDT;

    memset(PLM4T, 0, PAGE_SIZE);
    PLM4T[0] = PAGE_PRESENT | PAGE_WRITABLE | ((uint64_t)PDPT & PAGE_ADDRESS_MASK);

    memset(PDPT, 0, PAGE_SIZE);

    for (i = 0; i < 4; i++)
    {
        // every PDPT entry (PDPE) map 1GB memory
        // here map total 4G memory
        PDPT[i] = PAGE_PRESENT | PAGE_WRITABLE | ((((uint64_t)PDTBase) + i * PAGE_SIZE) & PAGE_ADDRESS_MASK);
        memset((void*)(long)(((uint64_t)PDTBase) + i * PAGE_SIZE), 0, PAGE_SIZE);
        PDT = (uint64_t*)(((uint64_t)PDTBase) + i * PAGE_SIZE);

        for (j = 0; j < PAGE_SIZE / sizeof(uint64_t); j++)
        {
            PDT[j] = PAGE_PRESENT | PAGE_WRITABLE | PAGE_SIZE_FLAG | ((PAGE_SIZE_2M * (j + i * (PAGE_SIZE) / sizeof(uint64_t))) & ~(PAGE_SIZE_2M - 1ULL));
        }
    }
    GdtrBase = (sizeof(LongModeSegmentTable) - 1);
    *(uint32_t*)((char*)&GdtrBase + 2) = (uint32_t)LongModeSegmentTable;
    goto_to_64(&GdtrBase, PLM4T, ImageBase);

    while (1)
    {

    }
    return 0;
}