#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct _FAT12_HEADER FAT12_HEADER;
typedef struct _FAT12_HEADER *PFAT12_HEADER;

typedef struct {
    uint8_t     BS_jmpBoot[3];          /* 0  Jump to boot code, 因为BS_jmpBoot后的数据是不可执行的 */
    uint8_t     BS_OEMName[8];          /* 3  OEM name & version */
    uint16_t    BPB_BytsPerSec;         /* 11 Bytes per sector hopefully 512, 每个扇区的大小, 单位字节 */
    uint8_t     BPB_SecPerClus;         /* 13 Cluster size in sectors, 每个cluster的扇区数, 引入簇是为了io性能优化, **簇是FAT类文件系统的最小数据存储单位** */
    uint16_t    BPB_RsvdSecCnt;         /* 14 Number of reserved (boot) sectors,  为boot保留的扇区数量, 必须是1, 因为mbr就是一个扇区的大小 */
    uint8_t     BPB_NumFATs;            /* 16 Number of FAT tables hopefully 2, fat12的FAT表的数量 */
    uint16_t    BPB_RootEntCnt;         /* 17 Number of directory slots, 根目录可容纳的目录项, BPB_RootEntCnt*32必定是BPB_BytsPerSec的偶数倍 */

    /*
     * If 0, look in sector_count_large
     */
    uint16_t    BPB_TotSec16;           /* 19 Total sectors on disk, 扇区的总数(包括保留扇区即引导扇区, FAT表, 根目录区和数据区), 如果该值为0, 那BPB_TotSec32必定非0 */
    uint8_t     BPB_Media;              /* 21 Media descriptor=first byte of FAT, 描述存储介质: 不可移动介质标准值是0xF8, 可移动介质是0xF0, [还有其他值](https://zh.wikipedia.org/wiki/FAT). 同时也必须向FAT表项[0](即fat表项的第一项)的低字节写入相同的值 */

    uint16_t    BS_FATSz16;             /* 每个FAT表的扇区数, FAT表1和FAT表2的容量相同 */
    uint16_t    BS_SecPerTrk;           /* 每个磁道的扇区数*/
    uint16_t    BS_NumHeads;            /* 磁头数*/
    uint32_t    BS_HiddSec;             /* 隐藏扇区数 */
    uint32_t    BS_TotSec32;            /* 如果BPB_TotSec16为0,该值就是扇区的总数  */
    uint8_t     BS_DrvNum;              /* 中断0x13的驱动器号 */
    uint8_t     BS_Reserved1;           /* 未使用,保留 */
    uint8_t     BS_BootSig;             /* 扩展引导标记, 0x29 */
    uint32_t    BS_VolID;               /* 卷序列号 */
    uint8_t     BS_VolLab[11];          /* 卷标, linux/windows系统中显示的磁盘名 */ 
    uint8_t     BS_FileSysType[8];      /* 文件系统类型, 固定为"FAT12  ",必须是8个字符，不足填充空格 */  

} __attribute__ ((packed)) _FAT12_HEADER; // 告诉编译器，这个结构体是不需要对齐的(GNU GCC有效)，如果不指定这个关键字，编译器在编译这个结构体时，会将其对齐，这样解析起Boot扇区就不正确了

// FAT1和FAT2是两个完全相同的FAT表，每个FAT占用9个扇区. 其中FAT1占用1—9扇区，FAT2占用10—18扇区 
// fat12以簇来管理数据区的空间. 每个簇的长度是`BPB_BytesPerSec * BPB_SecPerClus`, 数据区的簇号与fat表的表项是一一对应的.
// fat表中的表项位宽与FAT类型有关, fat12是12bit, fat16是16bit, fat32是32bit(FAT32只使用32位中的28位, 高4位通常是0但它们是保留位，不要更改它们). 借助fat表项, 可将不连续的文件片段按簇号链接起来.
// [fat12表项(通常直接跳过头两个表项)](https://zh.wikipedia.org/wiki/FAT#FAT12):
// 表项编号　　值(12位)　　备注　
// 000　　　|　FF0　　|　磁盘标示字, 低8bit与BPB_Media相同, 剩余位全部置1
// 001　　　|　FFF　　|　0xFFFF ，固定值，FAT标志, 防止文件系统错误分配该表项
// ...      |  ...    |  0x002 - 0xFEF, 可用簇, 指向下一个簇; 0xFF0 - 0xFF6, 保留值;0xFF7, 坏簇; 0xFF8 - 0xFFF, 文件的最后一个簇, 标识文件的结束

typedef struct _FILE_HEADER FILE_HEADER;
typedef struct _FILE_HEADER *PFILE_HEADER;

// 根目录起始地址：1(boot区大小)+2(fat数)*9(fat的扇区数)*512(扇区大小)=0x2600h
// 根目录区的开始扇区号是19，它是由若干个目录条目(Directory Entry)组成，条目最多有BPB_RootEntCnt个，由于根目录区的大小是依赖于BPB_RootEntCnt的，所以长度不固定
struct _FILE_HEADER {        // 共32B
    uint8_t    DIR_Name[11]; // 文件名11B, 扩展名3B
    uint8_t    DIR_Attr;     // 文件属性, 比如是文件还是目录. 文件的属性可以叠加使用，可以具有多重属性，即设置为只读的时候也可以同时隐藏:
                                // - 00000000：普通文件，可随意读写
                                // - 00000001：只读文件，不可改写
                                // - 00000010：隐藏文件，浏览文件时隐藏列表
                                // - 00000100：系统文件，删除的时候会有提示
                                // - 00001000：卷标，作为磁盘的卷标识符
                                // - 00010000：目录文件，此文件是一个子目录，它的内容就是此目录下的所有文件目录项
                                // - 00100000：归档文件
    uint8_t    Reserved[10]; // 保留位                             
    uint16_t   DIR_WrtTime;  // 最后一次写入时间
    uint16_t   DIR_WrtDate;  // 最后一次写入日期
    uint16_t   DIR_FstClus;  // 起始簇号, 因为fat表项的头2个是保留项, 因此有效簇号从2开始
    uint32_t   DIR_FileSize; // 文件大小
}__attribute__((packed)) _FILE_HEADER;


// 根目录区仅能保存目录项, 但数据区可保存目录项和文件内容的数据.

// 每个目录下会有两个特殊目录项`.`和`..`. `.`是当前目录的别名，首簇就应该指向自己，而`..`首簇就改指向上级目录文件的首簇. 如果上级目录是根目录，根目录区并没有在用户数据区分配内容. 因为用户簇从2开始（其实这正是从2开始的原因，将特殊簇号留给特殊条目），因此用0或1作为根目录代表都可以，从实际看，用0代表根目录.

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        puts("nvalid args count, must 2");

        return 1;
    }

    // open image
    FILE *pImgFile = fopen(argv[1], "rb");
    if(pImgFile == NULL)
    {
        puts("read fat12 image failed!");
        return 1;
    }

    // get file size
    fseek(pImgFile,0,SEEK_END); // 将文件位置指针移到文件结尾
    long lFileSize = ftell(pImgFile); // ftell:得到文件位置指针当前位置相对于文件首的偏移字节数

    printf("image size: %ld\n",lFileSize);

    // alloc buffer
    unsigned char *pImgBuf = (unsigned char *)malloc(lFileSize);

    if(pImgBuf == NULL)
    {
        puts("buffer alloc failed!");
        return 1;
    }

    // set file pointer to the beginning
    fseek(pImgFile,0,SEEK_SET);

    // read the whole image file into memmory
    long lReadResult = fread(pImgBuf,1,lFileSize,pImgFile); // 从给定流 stream 读取数据到 ptr 所指向的数组中, 1表示要读取的每个元素的大小，以字节为单位

    printf("read size: %ld\n",lReadResult);

    if(lReadResult != lFileSize)
    {
        puts("read file error!");
        free(pImgBuf);
        fclose(pImgFile);
        return 1;
    }

    // finish reading, close file
    fclose(pImgFile);

    // print FAT12 structure
    printImage(pImgBuf);

    return 0;
}

void printImage(unsigned char *pImgBuf)
{
    puts("\nStart to print image:\n");

    PFAT12_HEADER pFAT12Header = (PFAT12_HEADER)pImgBuf;

    // calculate start address of boot program
    WORD wBootStart = BOOT_START_ADDR + pFAT12Header->JmpCode[1] + 2;
    printf("Boot start address: 0x%04x\n",wBootStart);

    char buffer[20];

    memcpy(buffer,pFAT12Header->BS_OEMName,8);
    buffer[8] = 0;

    printf("BS_OEMName:         %s\n",buffer);
    printf("BPB_BytesPerSec:    %u\n",pFAT12Header->BPB_BytesPerSec);
    printf("BPB_SecPerClus:     %u\n",pFAT12Header->BPB_SecPerClus);
    printf("BPB_RsvdSecCnt:     %u\n",pFAT12Header->BPB_RsvdSecCnt);
    printf("BPB_NumFATs:        %u\n",pFAT12Header->BPB_NumFATs);
    printf("BPB_RootEntCnt:     %u\n",pFAT12Header->BPB_RootEntCnt);
    printf("BPB_TotSec16:       %u\n",pFAT12Header->BPB_TotSec16);
    printf("BPB_Media:          0x%02x\n",pFAT12Header->BPB_Media);
    printf("BPB_FATSz16:        %u\n",pFAT12Header->BPB_FATSz16);
    printf("BPB_SecPerTrk:      %u\n",pFAT12Header->BPB_SecPerTrk);
    printf("BPB_NumHeads:       %u\n",pFAT12Header->BPB_NumHeads);
    printf("BPB_HiddSec:        %u\n",pFAT12Header->BPB_HiddSec);
    printf("BPB_TotSec32:       %u\n",pFAT12Header->BPB_TotSec32);
    printf("BS_DrvNum:          %u\n",pFAT12Header->BS_DrvNum);
    printf("BS_Reserved1:       %u\n",pFAT12Header->BS_Reserved1);
    printf("BS_BootSig:         %u\n",pFAT12Header->BS_BootSig);
    printf("BS_VolID:           %u\n",pFAT12Header->BS_VolID);

    memcpy(buffer,pFAT12Header->BS_VolLab,11);
    buffer[11] = 0;
    printf("BS_VolLab:          %s\n",buffer);

    memcpy(buffer,pFAT12Header->BS_FileSysType,8);
    buffer[11] = 0;
    printf("BS_FileSysType:     %s\n",buffer);
}