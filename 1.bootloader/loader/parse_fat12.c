// [FAT32文件系统介绍中文版](https://wenku.baidu.com/view/8b483d6baf1ffc4ffe47ac75.html)
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BOOT_START_ADDR 0x7c00

//  dereferencing pointer to incomplete type ‘struct _FAT12_HEADER’: `struct _FAT12_HEADER未找到`,  起先名称放在了__attribute__属性后面, 影响了struct的定义.
struct _FAT12_HEADER {
    uint8_t     BS_jmpBoot[3];          /* 0  Jump to boot code, 因为BS_jmpBoot后的数据是不可执行的 */ //BS(boot sector)
    uint8_t     BS_OEMName[8];          /* 3  OEM name & version */
    uint16_t    BPB_BytesPerSec;         /* 11 Bytes per sector hopefully 512, 每个扇区的大小, 单位字节 */ // BPB(BIOS parameter block)
    uint8_t     BPB_SecPerClus;         /* 13 Cluster size in sectors, 每个cluster的扇区数, BPB_SecPerClus总是2的整数次方（1，2，4，8，……）. 引入簇是为了io性能优化, **簇是FAT类文件系统的最小数据存储单位** */
    uint16_t    BPB_RsvdSecCnt;         /* 14 Number of reserved (boot) sectors,  为boot保留的扇区数量, 必须是1, 因为mbr就是一个扇区的大小 */
    uint8_t     BPB_NumFATs;            /* 16 Number of FAT tables hopefully 2, fat12的FAT表的数量 */
    uint16_t    BPB_RootEntCnt;         /* 17 Number of directory slots, 根目录可容纳的目录项, BPB_RootEntCnt*32必定是BPB_BytsPerSec的偶数倍. 对于FAT32，此项必须为0. 对于FAT12和FAT16，此数乘以32必为BPB_BytesPerSec的偶数倍，为了达到更好的兼容性，FAT12和FAT16都应该取值为512 */

    /*
     * If 0, look in sector_count_large
     */
    uint16_t    BPB_TotSec16;           /* 19 Total sectors on disk, 扇区的总数(包括保留扇区即引导扇区, FAT表, 根目录区和数据区), 如果该值为0, 那BPB_TotSec32必定非0 */
    uint8_t     BPB_Media;              /* 21 Media descriptor=first byte of FAT, 描述存储介质: 不可移动介质标准值是0xF8, 可移动介质是0xF0, [还有其他值](https://zh.wikipedia.org/wiki/FAT). 同时也必须向FAT表项[0](即fat表项的第一项)的低字节写入相同的值 */

    uint16_t    BPB_FATSz16;             /* 每个FAT表的扇区数, FAT表1和FAT表2的容量相同 */
    uint16_t    BPB_SecPerTrk;           /* 每个磁道的扇区数*/
    uint16_t    BPB_NumHeads;            /* 磁头数*/
    uint32_t    BPB_HiddSec;             /* 隐藏扇区数 */
    uint32_t    BPB_TotSec32;            /* 如果BPB_TotSec16为0,该值就是扇区的总数  */
    uint8_t     BS_DrvNum;              /* 中断0x13的驱动器号 */
    uint8_t     BS_Reserved1;           /* 未使用,保留 */
    uint8_t     BS_BootSig;             /* 扩展引导标记, 0x29, 用于指明此后的3个属性可用 */
    uint32_t    BS_VolID;               /* 卷序列号 */
    uint8_t     BS_VolLab[11];          /* 卷标, linux/windows系统中显示的磁盘名 */ 
    uint8_t     BS_FileSysType[8];      /* 文件系统类型, 固定为"FAT12  ",必须是8个字符，不足填充空格 */  

} __attribute__ ((packed)); // 告诉编译器，这个结构体是不需要对齐的(GNU GCC有效)，如果不指定这个关键字，编译器在编译这个结构体时，会将其对齐，这样解析起Boot扇区就不正确了

typedef struct _FAT12_HEADER FAT12_HEADER;
typedef struct _FAT12_HEADER *PFAT12_HEADER;

// FAT1和FAT2是两个完全相同的FAT表，每个FAT占用9个扇区. 其中FAT1占用1—9扇区，FAT2占用10—18扇区 
// fat12以簇来管理数据区的空间. 每个簇的长度是`BPB_BytesPerSec * BPB_SecPerClus`, 数据区的簇号与fat表的表项是一一对应的.
// fat表中的表项位宽与FAT类型有关, fat12是12bit, fat16是16bit, fat32是32bit(FAT32只使用32位中的28位, 高4位通常是0但它们是保留位，不要更改它们). 借助fat表项, 可将不连续的文件片段按簇号链接起来.
// [fat12表项(通常直接跳过头两个表项)](https://zh.wikipedia.org/wiki/FAT#FAT12):
// 表项编号　　值(12位)　　备注　
// 000　　　|　FF0　　|　磁盘标示字, 低8bit与BPB_Media相同, 剩余位全部置1
// 001　　　|　FFF　　|　0xFFFF ，固定值，FAT标志, 防止文件系统错误分配该表项
// ...      |  ...    |  0x002 - 0xFEF, 可用簇, 指向下一个簇; 0xFF0 - 0xFF6, 保留值;0xFF7, 坏簇; 0xFF8 - 0xFFF, 文件的最后一个簇, 标识文件的结束

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
}__attribute__((packed));

typedef struct _FILE_HEADER FILE_HEADER;
typedef struct _FILE_HEADER *PFILE_HEADER;

// 根目录区仅能保存目录项, 但数据区可保存目录项和文件内容的数据.

// 每个目录下会有两个特殊目录项`.`和`..`. `.`是当前目录的别名，首簇就应该指向自己，而`..`首簇就改指向上级目录文件的首簇. 如果上级目录是根目录，根目录区并没有在用户数据区分配内容. 因为用户簇从2开始（其实这正是从2开始的原因，将特殊簇号留给特殊条目），因此用0或1作为根目录代表都可以，从实际看，用0代表根目录.
// 文件操作:
//     1. 文件查找算法
//         1. 先查看根目录区，是否有匹配的目录，如果有，通过对应目录项的首簇段获取其目录文件的首簇号
//         1. 通过fat表获得该目录文件的全部内容，遍历该文件，一次偏移32字节继续查找目录项，匹配查询路径中对应的项. 如果查到则类似1，2查找对应的目录文件及目录项，否则说明找不到，结束.

// 3、如果在倒数第一层目录文件中找到了被查文件的目录项，从中获取首簇号，即可通过fat表访问该文件整个相关簇。
// 删除一个文件

//  目录项第一个字节修改为E5代表删除。
// 回收已用的簇

//  在fat表中，从被删除文件的首簇开始，将每个fat项置为00，实际文件扇区内容不用修改，这应该就是所谓的标记删除了吧，所以我们可以找到e5打头的目录项尝试恢复文件嘿嘿~
// 如何创建一个文件

// 1、创建文件时，需要首先定位到文件所在的目录文件，然后查找目录项（以32字节为偏移递增），如果第一个字节为0或e5表示可用。

// 2、根据文件大小计算出需要的簇数目，然后从fat首部开始（第4个字节），查看值为0的项，如果是则可用。找到第一个为0的项，将其索引值（以0开始）写入步骤1找到的目录项的首簇段。接着搜寻为0的项，将其索引值写入前一步找到的项。这样形成一个链，如果是最后一簇，其对应的簇项的值为fff。

// 3、填写文件目录项的创建时间，属性，大小等字段。

void printImage(unsigned char *pImageBuffer)
{
    puts("\nStart to print image:\n");

    PFAT12_HEADER pFAT12Header = (PFAT12_HEADER)pImageBuffer;

    // calculate start address of boot program
    // BS_jmpBoot的内容是`eb 3c 90`, 其中，JMP Offset的操作码为0xEB，操作数(Offset)占一个Byte，NOP为0x90，占一个Byte, 所以，这整个就是 0xEB Offset 0x90.
    uint16_t wBootStart = BOOT_START_ADDR + pFAT12Header->BS_jmpBoot[1] +2; // 2: JMP Offset的指令长度，因为Offset是针对当前指令的下一条指令地址来的
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

uint32_t GetLSB(uint32_t ClusOfTable, PFAT12_HEADER pFAT12Header)
{
    // 将数据区前面所有的扇区号都加起来，得到数据区的起始扇区，然后将给出的FAT项减2，再乘上每簇的扇区数，加上数据区的起始扇区号，最后就得到了当前FAT项的LSB
    uint32_t dwDataStartClusSec =  pFAT12Header->BPB_HiddSec + pFAT12Header->BPB_RsvdSecCnt + pFAT12Header->BPB_NumFATs * pFAT12Header->BPB_FATSz16 + \
                            pFAT12Header->BPB_RootEntCnt * 32 / pFAT12Header->BPB_BytesPerSec; // pFAT12Header->BPB_RootEntCnt * 32 / pFAT12Header->BPB_BytesPerSec 是根目录区占用的扇区数

    // 数据开始的扇区index
    return dwDataStartClusSec + (ClusOfTable - 2) * pFAT12Header->BPB_SecPerClus; // ClusOfTable - 2 : 因为FAT表的第0, 1两项是保留项, 因此有效簇号从2开始.
}

uint32_t ReadData(unsigned char *pImageBuffer, uint32_t LSB, unsigned char *outBuffer)
{
    PFAT12_HEADER pFAT12Header = (PFAT12_HEADER)pImageBuffer;

    uint32_t dwReadPosBytes = LSB * pFAT12Header->BPB_BytesPerSec; // 得到要读的扇区的字节起始值

    memcpy(outBuffer, pImageBuffer + dwReadPosBytes, pFAT12Header->BPB_SecPerClus * pFAT12Header->BPB_BytesPerSec);

    return pFAT12Header->BPB_SecPerClus * pFAT12Header->BPB_BytesPerSec; // 读取一个cluster的字节数
}

// GetFATNext根据当前给出的FAT表项，得到它在FAT表里的下一项
uint16_t GetFATNext(uint8_t *FATTable, uint16_t CurOffset)
{
    uint16_t tabOff = CurOffset * 1.5; //当前FAT项在FAT tab中的偏移量. FAT表中的每一项是1.5 Bytes(6 Bits). 这样不要理解的话, 可分CurOffset是奇偶来判断. CurOffset为奇数时, 它是它前面一项(是偶数)按word读取的后半部分.

    uint16_t nextOff = *(uint16_t *)(FATTable + tabOff);

    // 而又没有任何一种数据类型能够表示1.5 Bytes，所以需要用一个Word，也就是2 Bytes来存储它
    // 判断这个偏移是奇数还是偶数. 如果是奇数，则将前4位清0(与上0x0fff)，如果是偶数，则将其右移4位，最终得到下一项的FAT表偏移
    nextOff = CurOffset % 2 == 0 ?  nextOff & 0x0fff : nextOff >> 4;

    return nextOff;
}

//  ReadFile : 读取文件
// 未判读是否已删除
uint32_t ReadFile(unsigned char *pImageBuffer, PFILE_HEADER pFileHeader)
{
    // file read buffer
    unsigned char outBuffer[2048];

    PFAT12_HEADER pFAT12Header = (PFAT12_HEADER)pImageBuffer;

    char nameBuffer[20];
    memcpy(nameBuffer, pFileHeader->DIR_Name, 11);
    nameBuffer[11] = 0;

    printf("The FAT chain of file %s:\n", nameBuffer);

    // calculate the pointer of FAT Table
    // 用指向ImageBuffer的指针加上FAT表的偏移, 得到FAT表的指针
    uint8_t *pbStartOfFATTab = pImageBuffer + (pFAT12Header->BPB_HiddSec + pFAT12Header->BPB_RsvdSecCnt) * pFAT12Header->BPB_BytesPerSec;

    uint16_t next = pFileHeader->DIR_FstClus;

    uint32_t readBytes = 0;
    do
    {
        printf(", 0x%03x", next);

        // get the LSB of clus num
        uint32_t dwCurLSB = GetLSB(next, pFAT12Header);

        // read data
        readBytes += ReadData(pImageBuffer, dwCurLSB, outBuffer + readBytes);

        // get next clus num according to current clus num
        next = GetFATNext(pbStartOfFATTab, next);
    }while(next <= 0xfef);

    printf("\nfile size: %d, file content:\n^%s$\n\n", readBytes,outBuffer);// file size指占用的cluster, 而不是实际大小

    return readBytes;
}

// 用来遍历Root Directory，将文件名、文件属性和文件首簇号打印出来，并将其目录结构作为_FILE_HEADER结构体存储在一个数组中
void SeekRootDir(unsigned char *pImageBuffer)
{
    PFAT12_HEADER pFAT12Header = (PFAT12_HEADER)pImageBuffer;
    uint16_t rootEntCnt = pFAT12Header->BPB_RootEntCnt;

    puts("\nStart seek files of root dir:");

    // sectors number of start of root directory
    // 计算出了根目录的起始扇区。计算方法为：隐藏扇区数 + 保留扇区数(Boot Sector) + FAT表数量 × FAT表大小(Sectors)。也就是将根目录前面所有的扇区数加起来。 得到起始扇区数后，将其乘上每扇区的字节数就能得到根目录的起始字节偏移了
    uint16_t wRootDirStartSec = pFAT12Header->BPB_HiddSec + pFAT12Header->BPB_RsvdSecCnt + pFAT12Header->BPB_NumFATs * pFAT12Header->BPB_FATSz16;

    printf("Start sector of root directory:    %u\n", wRootDirStartSec);

    // bytes num of start of root directory
    uint16_t dwRootDirStartBytes = wRootDirStartSec * pFAT12Header->BPB_BytesPerSec;
    printf("Start bytes of root directory:      %u\n",dwRootDirStartBytes);

    // pImageBuffer地址加上计算出的根目录字节偏移就能得到根目录第一个文件的_FILE_HEADER结构体
    PFILE_HEADER pFileHeader = (PFILE_HEADER)(pImageBuffer + dwRootDirStartBytes);

    int fileNum = 1;
    while(rootEntCnt > 0)
    {
        char buffer[20];
        memcpy(buffer,pFileHeader->DIR_Name,11); // 文件名
        buffer[11] = 0;

        printf("File no.            %d\n", fileNum);
        printf("File name:          %s\n", buffer);
        printf("File attributes:    0x%02x\n", pFileHeader->DIR_Attr);
        printf("First clus num:     %u\n\n", pFileHeader->DIR_FstClus);

        if ((pFileHeader->DIR_Attr & 0x10) == 0) { // 0x10是目录
            printf("is file\n");
            ReadFile(pImageBuffer, pFileHeader);
        }else{
            printf("is dir\n");
        }

        ++pFileHeader;
        ++fileNum;
        rootEntCnt--;
    }
}

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
    unsigned char *pImageBuffer = (unsigned char *)malloc(lFileSize);

    if(pImageBuffer == NULL)
    {
        puts("buffer alloc failed!");
        return 1;
    }

    // set file pointer to the beginning
    fseek(pImgFile,0,SEEK_SET);

    // read the whole image file into memmory
    long lReadResult = fread(pImageBuffer,1,lFileSize,pImgFile); // 从给定流 stream 读取数据到 ptr 所指向的数组中, 1表示要读取的每个元素的大小，以字节为单位

    printf("read size: %ld\n",lReadResult);

    if(lReadResult != lFileSize)
    {
        puts("read file error!");
        free(pImageBuffer);
        fclose(pImgFile);
        return 1;
    }

    // finish reading, close file
    fclose(pImgFile);

    // print FAT12 structure
    printImage(pImageBuffer);

     // seek files of root directory
    SeekRootDir(pImageBuffer);

    return 0;
}