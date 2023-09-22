// Copyright by Xiyue87 2022

#include "ctype.h"
#include "ata_direct.h"
#ifndef _INCLUDE_FATFS_H_
#define _INCLUDE_FATFS_H_

#define MBR_ENTRY_ACTIVE_OFFSET         0x00
#define MBR_ENTRY_CHS_START_OFFSET      0x01
#define MBR_ENTRY_TYPE_OFFSET           0x04
#define MBR_ENTRY_CHS_END_OFFSET        0x05
#define MBR_ENTRY_LBA_START_OFFSET      0x08
#define MBR_ENTRY_NUM_SEC_OFFSET        0x0C

#define MBR_TABLE_OFFSET        0x1BE

#define FAT_PBR_JMP             0x00
#define FAT_PBR_BYTES_PER_SEC   0x0B
#define FAT_PBR_SEC_PER_CLUST   0x0D
#define FAT_PBR_NRESRVD_SECS    0x0E
#define FAT_PBR_FAT_NUMS        0x10
#define FAT_PBR_SECTORS16       0x13

#define FAT_PBR_NHIDDEN_SECS    0x1C
#define FAT_PBR_SECTORS32       0x20
#define FAT_PBR_SEC_PER_FAT32   0x24
#define FAT_PBR_ROOT_CLUSTER    0x2C
#define FAT_PBR_FS_TYPE         0x52    /* string FAT32 */
#define FAT_PBR_SYSID_FAT32     "FAT32   "
#define FAT_PBR_SYSID_LEN       8
#define FAT_PBR_SIG_OFFSET      0x1BE

#define FAT32_CLUSTER_MASK      0xFFFFFFF

struct FAT32_DEV
{
    uint32_t PBRSecLba;
    uint32_t FAT1SecLba;
    uint32_t DataStartLba;

    uint32_t TotalSecs;
    uint32_t SecPerCluster;
    uint32_t SectorSize;
    uint32_t ClusterSize;
    uint32_t RootCluster;
    uint32_t NumOfFat;
    uint32_t NumSecsPerFat;

    uint32_t FatSecBufBlkNum;
    char FatSecBuffer[SECTOR_SIZE];
    uint32_t DataSecBufBlkNum;
    char DataSecBuffer[SECTOR_SIZE];
};


#pragma pack(push, 1)
struct FAT_DIR_ENTRY
{
    char name[11];          // offset 00
    uint8_t attribute;      // offset 0b
    uint8_t reserved;       // offset 0c
    uint8_t ctime0;         // offset 0d
    uint16_t ctime1;        // offset 0e
    uint16_t ctime2;        // offset 10
    uint16_t atime;         // offset 12
    uint16_t cluster_high;  // offset 14
    uint16_t mtime0;        // offset 16
    uint16_t mtime1;        // offset 18
    uint16_t cluster_low;   // offset 1a
    uint32_t size;          // offset 1c

};
#pragma pack(pop)

int Fat32ReadFat(struct FAT32_DEV* FatDev, uint32_t SecOffsetInFat);
int Fat32ReadData(struct FAT32_DEV* FatDev, uint32_t SecOffsetInData);
int InitFileSystem();
int ListRoot();
char* LoadKernelFile(char* FileName);

#endif