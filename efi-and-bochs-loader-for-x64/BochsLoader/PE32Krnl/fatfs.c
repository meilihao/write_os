// Copyright by Xiyue87 2022

// only for FAT32
// only support to read disk from ATA0 first active partition

#include "ctype.h"
#include "string.h"
#include "stdio.h"
#include "ata_direct.h"
#include "fatfs.h"
#include "siodebug.h"

struct FAT32_DEV BootFatDev;

int Fat32ReadFat(struct FAT32_DEV* FatDev, uint32_t SecOffsetInFat)
{
    read_sector(FatDev->FatSecBuffer, FatDev->FAT1SecLba + SecOffsetInFat);
    FatDev->FatSecBufBlkNum = SecOffsetInFat;
    return 0;
}

int Fat32ReadData(struct FAT32_DEV* FatDev, uint32_t SecOffsetInData)
{
    read_sector(FatDev->DataSecBuffer, FatDev->DataStartLba + SecOffsetInData);
    FatDev->DataSecBufBlkNum = SecOffsetInData;
    return 0;
}

int InitFileSystem()
{
    int i;
    uint8_t* MbrEntry = NULL;
    uint32_t ReservedSecs;
    char IntNumBuf[40] = { 0 };
    struct FAT32_DEV* FatDev = &BootFatDev;

    read_sector(FatDev->DataSecBuffer, 0); // use DataSecBuffer for swap data

    for (i = 0; i < 4; i++)
    {
        MbrEntry = FatDev->DataSecBuffer + MBR_TABLE_OFFSET + i * 16;
        if (MbrEntry[MBR_ENTRY_ACTIVE_OFFSET] != 0x80
            || *(uint32_t*)(MbrEntry + MBR_ENTRY_NUM_SEC_OFFSET) == 0)
        {
            continue;
        }

        FatDev->PBRSecLba = *(uint32_t*)(MbrEntry + MBR_ENTRY_LBA_START_OFFSET);
        break;
    }

    if (FatDev->PBRSecLba == 0)
    {
        goto ERROR_OUT;
    }

    read_sector(FatDev->DataSecBuffer, FatDev->PBRSecLba); // use DataSecBuffer for swap data

    if (memcmp(FatDev->DataSecBuffer + FAT_PBR_FS_TYPE, FAT_PBR_SYSID_FAT32, FAT_PBR_SYSID_LEN) != 0)
    {
        goto ERROR_OUT;
    }

    FatDev->TotalSecs = *(uint32_t*)(FatDev->DataSecBuffer + FAT_PBR_SECTORS32);
    FatDev->SecPerCluster = *(uint8_t*)(FatDev->DataSecBuffer + FAT_PBR_SEC_PER_CLUST);
    FatDev->SectorSize = *(uint16_t*)(FatDev->DataSecBuffer + FAT_PBR_BYTES_PER_SEC);
    FatDev->ClusterSize = FatDev->SectorSize * FatDev->SecPerCluster;
    FatDev->RootCluster = *(uint32_t*)(FatDev->DataSecBuffer + FAT_PBR_ROOT_CLUSTER);
    FatDev->NumOfFat = *(uint8_t*)(FatDev->DataSecBuffer + FAT_PBR_FAT_NUMS);
    FatDev->NumSecsPerFat = *(uint32_t*)(FatDev->DataSecBuffer + FAT_PBR_SEC_PER_FAT32);
    ReservedSecs = *(uint16_t*)(FatDev->DataSecBuffer + FAT_PBR_NRESRVD_SECS);

    FatDev->FAT1SecLba = FatDev->PBRSecLba + ReservedSecs;

    FatDev->DataStartLba = FatDev->FAT1SecLba + FatDev->NumOfFat * FatDev->NumSecsPerFat;

    Fat32ReadFat(FatDev, 0);
    Fat32ReadData(FatDev, 0);

    SioPuts("Init FAT32 device at 0x");
    SioPuts(itoh((int)(long)FatDev, IntNumBuf));
    SioPuts("\n");

    return 0;

ERROR_OUT:
    SioPuts("ERROR\n");
    return -1;
}

int IsLastFatCluster(uint32_t ClusterNumber)
{
    ClusterNumber = ClusterNumber & FAT32_CLUSTER_MASK;
    if (ClusterNumber > 0xFFFFFF8)
        return 1;
    else
        return 0;
}

uint32_t Fat32CacheReadFatEntry(struct FAT32_DEV* FatDev, uint32_t MaskedCluster)
{
    uint32_t Offset; // XXXX - TXY, may be overflow
    uint32_t Ret = 0xFFFFFFFF;
    Offset = MaskedCluster * sizeof(uint32_t);

    if (Offset / SECTOR_SIZE != FatDev->FatSecBufBlkNum)
    {
        Fat32ReadFat(FatDev, Offset / SECTOR_SIZE);
    }

    Ret = *(uint32_t*)(FatDev->FatSecBuffer + (Offset % SECTOR_SIZE));

    return Ret;

}
uint32_t Fat32GetNextChain(struct FAT32_DEV* FatDev, uint32_t ClusterNumber)
{

    uint32_t MaskedCluster;
    if (IsLastFatCluster(ClusterNumber) == 1)
    {
        return -1;
    }
    MaskedCluster = ClusterNumber & FAT32_CLUSTER_MASK;

    return Fat32CacheReadFatEntry(FatDev, MaskedCluster);
}

int Fat32CacheReadData(struct FAT32_DEV* FatDev, uint32_t SecOffsetInData, void* SecBuffer)
{
    if (SecOffsetInData != FatDev->DataSecBufBlkNum)
        Fat32ReadData(FatDev, SecOffsetInData);

    if (SecBuffer != NULL)
        memcpy(SecBuffer, FatDev->DataSecBuffer, SECTOR_SIZE);

    return 0;
}

uint32_t Fat32ClusterToDataBlockOffset(struct FAT32_DEV* FatDev, uint32_t ClusterNumber)
{
    uint32_t MaskedCluster;
    uint32_t DataBlockOffset;
    if (IsLastFatCluster(ClusterNumber) == 1)
    {
        return -1;
    }
    MaskedCluster = ClusterNumber & FAT32_CLUSTER_MASK;
    MaskedCluster = MaskedCluster - 2;

    DataBlockOffset = MaskedCluster * FatDev->SecPerCluster;

    return DataBlockOffset;
}

int ListRoot()
{
    struct FAT32_DEV* FatDev = &BootFatDev;
    uint32_t CurrentCluster = FatDev->RootCluster;
    uint32_t DataBlockNumber;
    uint32_t i, j;
    struct FAT_DIR_ENTRY* DirEntry;
    char IntNumBuf[64] = { 0 };
    char Name8Dot3[12] = { 0 };

    SioPuts("List Root Directory:\n");

    while (IsLastFatCluster(CurrentCluster) == 0)
    {
        DataBlockNumber = Fat32ClusterToDataBlockOffset(FatDev, CurrentCluster);


        for (i = 0; i < FatDev->SecPerCluster; i++)
        {
            Fat32CacheReadData(FatDev, DataBlockNumber + i, NULL);

            for (j = 0; j < SECTOR_SIZE / sizeof(struct FAT_DIR_ENTRY); j++)
            {
                DirEntry = (struct FAT_DIR_ENTRY*)FatDev->DataSecBuffer;
                DirEntry = DirEntry + j;

                if ((uint8_t)DirEntry->name[0] == (uint8_t)0xE5 // deleted name
                    || DirEntry->name[0] == 0)  // free slot
                    continue;
                if (DirEntry->attribute == 0x0F  // long name
                    || DirEntry->attribute == 0x08) // volume label
                    continue;

                memset(Name8Dot3, 0, sizeof(Name8Dot3));
                memcpy(Name8Dot3, DirEntry->name, sizeof(DirEntry->name));

                SioPuts(Name8Dot3);

                if ((DirEntry->attribute & 0x10) != 0) // directory
                {
                    SioPuts("    <DIR> ");
                }
                else
                {
                    SioPuts("          ");
                }

                memset(IntNumBuf, 0, sizeof(IntNumBuf));
                SioPuts(itoa(DirEntry->size, IntNumBuf));
                SioPuts("\n");
            }
        }
        CurrentCluster = Fat32GetNextChain(FatDev, CurrentCluster);
    }
    SioPuts("------------------------\n");

    return 0;
}

int LoadFileFromCluster(struct FAT32_DEV* FatDev, uint32_t StartCluster, uint32_t Size, char* Buffer)
{
    uint32_t CurrentCluster = StartCluster;
    uint32_t DataBlockNumber;
    uint32_t i;
    uint32_t SavedSize = Size;

    char IntNumBuf[64] = { 0 };

    while (IsLastFatCluster(CurrentCluster) == 0)
    {
        DataBlockNumber = Fat32ClusterToDataBlockOffset(FatDev, CurrentCluster);

        for (i = 0; i < FatDev->SecPerCluster; i++)
        {
            Fat32CacheReadData(FatDev, DataBlockNumber + i, NULL);
            if (Size > FatDev->SectorSize)
            {
                memcpy(Buffer, FatDev->DataSecBuffer, FatDev->SectorSize);
                Size = Size - FatDev->SectorSize;
                Buffer = Buffer + FatDev->SectorSize;
                SioPuts("+");
            }
            else
            {
                memcpy(Buffer, FatDev->DataSecBuffer, Size);
                Size = 0;
                SioPuts(".\n");
                return SavedSize - Size;
            }
        }
        CurrentCluster = Fat32GetNextChain(FatDev, CurrentCluster);
    }
    SioPuts("\n");
    return SavedSize - Size;
}

char* LoadKernelFile(char* FileName)
{
    struct FAT32_DEV* FatDev = &BootFatDev;
    uint32_t CurrentCluster = FatDev->RootCluster;
    uint32_t DataBlockNumber;
    uint32_t i, j;
    uint32_t FileCluster;
    struct FAT_DIR_ENTRY* DirEntry;
    char IntNumBuf[64] = { 0 };
    char* Buf;
    SioPuts("Reading ");
    SioPuts(FileName);
    SioPuts(" ...\n");

    while (IsLastFatCluster(CurrentCluster) == 0)
    {
        DataBlockNumber = Fat32ClusterToDataBlockOffset(FatDev, CurrentCluster);


        for (i = 0; i < FatDev->SecPerCluster; i++)
        {
            Fat32CacheReadData(FatDev, DataBlockNumber + i, NULL);

            for (j = 0; j < SECTOR_SIZE / sizeof(struct FAT_DIR_ENTRY); j++)
            {
                DirEntry = (struct FAT_DIR_ENTRY*)FatDev->DataSecBuffer;
                DirEntry = DirEntry + j;

                if ((uint8_t)DirEntry->name[0] == (uint8_t)0xE5 // deleted name
                    || DirEntry->name[0] == 0)  // free slot
                    continue;
                if (DirEntry->attribute == 0x0F  // long name
                    || DirEntry->attribute == 0x08) // volume label
                    continue;

                if (memcmp(DirEntry->name, FileName, sizeof(DirEntry->name)) == 0)
                {
                    memset(IntNumBuf, 0, sizeof(IntNumBuf));
                    SioPuts("Got file with size ");
                    SioPuts(itoa(DirEntry->size, IntNumBuf));
                    SioPuts(" at cluster ");
                    FileCluster = (((uint32_t)DirEntry->cluster_high) << 16) | DirEntry->cluster_low;
                    memset(IntNumBuf, 0, sizeof(IntNumBuf));
                    SioPuts(itoa(FileCluster, IntNumBuf));
                    SioPuts("\nLoading...\n");

                    Buf = (char *)0x00200000;

                    LoadFileFromCluster(FatDev, FileCluster, DirEntry->size, Buf);

                    SioPuts("Loaded to 0x");
                    memset(IntNumBuf, 0, sizeof(IntNumBuf));
                    SioPuts(itoh((int)(long)Buf, IntNumBuf));
                    SioPuts("\n");

                    return Buf;
                }
            }
        }
        CurrentCluster = Fat32GetNextChain(FatDev, CurrentCluster);
    }
    SioPuts("------------------------\n");

    return NULL;
}