// Copyright by Xiyue 2022

#include "EFILoader.h"

char* PERawBuf = (char*)0x00200000;
char* PEExpandBuf = (char*)0x00500000;

int LoadPE64(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, const char* Filename, UINTN MapKey)
{

    EFI_BOOT_SERVICES* BootService;
    EFI_LOADED_IMAGE* LoadedImage;
    EFI_FILE_IO_INTERFACE* Volume;
    EFI_FILE* RootFile, * EfiFile;
    EFI_FILE_INFO* FileInfo;
    UINT8 FileInfoBuffer[1024];
    CHAR16 WidthFilename[256];
    char ShortString[256];
    UINTN InfoSize = sizeof(FileInfoBuffer);
    CHAR16 IntStr[64]; // �ļ���С����ֵת�ַ���
    UINTN PERawDataSize = 0x100000;

    SioPuts("\nLoading ");
    SioPuts(Filename);
    SioPuts(" ...\n");

    ShortStringToWidthString(Filename, WidthFilename, sizeof(WidthFilename) / sizeof(CHAR16));
    BootService = SystemTable->BootServices;

    // ͨ��ImageHandle�����EFI_LOADED_IMAGE
    if (BootService->HandleProtocol(ImageHandle, &LoadedImageProtocol, (void**)&LoadedImage) != EFI_SUCCESS)
    {
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }

    // ͨ��EFI_LOADED_IMAGE�����EFI_FILE_IO_INTERFACE��Ҳ�������loader���ڵ�����
    if (BootService->HandleProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&Volume) != EFI_SUCCESS)
    {
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }

    // ��volume����ø�Ŀ¼handle
    if (Volume->OpenVolume(Volume, &RootFile) != EFI_SUCCESS)
    {
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }

    // ���ļ�

    if (RootFile->Open(RootFile, &EfiFile, WidthFilename, EFI_FILE_MODE_READ, 0) != EFI_SUCCESS)
    {
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }

    FileInfo = (EFI_FILE_INFO*)FileInfoBuffer;
    // �������
    if (EfiFile->GetInfo(EfiFile, &GenericFileInfo, &InfoSize, FileInfo) != EFI_SUCCESS)
    {
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }

    // ��ʾ
    SioPuts("File Size: ");

    if (FileInfo->Attribute & EFI_FILE_DIRECTORY)
    {
        SioPuts("ERROR: is a directory\n");
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }
    else
    {
        // �����ļ�����ʾ�ļ���С
        memset(IntStr, 0, sizeof(IntStr));
        itoa((int)FileInfo->FileSize, ShortString);
        SioPuts(ShortString);
        SioPuts("\n");
    }

    if (EfiFile->Read(EfiFile, &PERawDataSize, PERawBuf) != EFI_SUCCESS)
    {
        SioPuts("ERROR: Read fail\n");
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }    

    uint64_t RealEntry;
    uint64_t RealBase;
    uint32_t MaxVirtualAddress;
    // clean up all memory (init for BSS segment)
    memset(PEExpandBuf, 0, 0x100000);
    RealEntry = LoadPEFileData(PEExpandBuf, PERawBuf, &RealBase, &MaxVirtualAddress);
    SioPuts("\nPE32+ File Information:\n");
    SioPutHexValue("RealBase               : [", (uint32_t)RealBase, "]\n");
    SioPutHexValue("PEExpandDataBuffer     : [", (uint32_t)(uint64_t)PEExpandBuf, "]\n");
    SioPutHexValue("MaxVirtualAddress      : [", (uint32_t)MaxVirtualAddress, "]\n");
    SioPutHexValue("RealEntry              : [", (uint32_t)RealEntry, "]\n");

    memcpy((void *)RealBase, PEExpandBuf, MaxVirtualAddress);

    BootService->ExitBootServices(ImageHandle, MapKey);

    Goto64((char*)RealEntry);

    return 0;
ERROR_DEAD:
    return -1;
}

typedef struct _IMAGE_SECTION_HEADER {
    uint8_t     Name[8];
    uint32_t    VirtualSize;
    uint32_t    VirtualAddress;
    uint32_t    SizeOfRawData;
    uint32_t    PointerToRawData;
    uint32_t    PointerToRelocations;
    uint32_t    PointerToLinenumbers;
    uint16_t    NumberOfRelocations;
    uint16_t    NumberOfLinenumbers;
    uint32_t    Characteristics;
} IMAGE_SECTION_HEADER, * PIMAGE_SECTION_HEADER;

uint32_t LoadSections(char* PEImageBase, char* RawData, int OffsetSection, int NumSection)
{
    IMAGE_SECTION_HEADER* SectionHeader;
    int i;
    char SectionName[9];
    uint32_t MaxVirtualAddress = 0;

    for (i = 0; i < NumSection; i++)
    {
        SectionHeader = (IMAGE_SECTION_HEADER*)(RawData + OffsetSection + i * sizeof(IMAGE_SECTION_HEADER));
        memset(SectionName, 0, sizeof(SectionName));
        memcpy(SectionName, SectionHeader->Name, sizeof(SectionHeader->Name));

        SioPuts("Section Name            : [");
        SioPuts(SectionName);
        SioPuts("]\n");

        SioPutHexValue("Virtual Address         : [", SectionHeader->VirtualAddress, "]\n");
        SioPutHexValue("Virtual Size            : [", SectionHeader->VirtualSize, "]\n");
        SioPutHexValue("Size of Raw Data        : [", SectionHeader->SizeOfRawData, "]\n");
        SioPutHexValue("Pointer to Raw Data     : [", SectionHeader->PointerToRawData, "]\n");

        /*
         * ��ԭʼ���������ҵ����е�section�����������ݵ�ImageBase�ĵ�ַ��
         */

        if (SectionHeader->VirtualAddress + SectionHeader->VirtualSize >= 0x100000)
        {
            SioPuts("Exceed max size\n");
            return -1;
        }

        if (MaxVirtualAddress < SectionHeader->VirtualAddress + SectionHeader->VirtualSize)
            MaxVirtualAddress = SectionHeader->VirtualAddress + SectionHeader->VirtualSize;

        memcpy(PEImageBase + SectionHeader->VirtualAddress, RawData + SectionHeader->PointerToRawData, SectionHeader->SizeOfRawData);
    }

    return MaxVirtualAddress;
}

uint64_t LoadPEFileData(char* PEImageBase, char* RawData, uint64_t * ImageBaseRet, uint32_t * MaxVirtual)
{
    uint16_t NumSection;
    uint32_t OffsetSection;
    uint32_t NtHeaderOffset;
    uint64_t ImageBase;
    uint32_t ImageEntry;
    uint32_t MaxVirtualAddress;

    // IMAGE_DOS_HEADER->e_lfanew offset 0x3C ==> IMAGE_NT_HEADERS64
    NtHeaderOffset = *((uint32_t*)(RawData + 0x3C));

    // IMAGE_NT_HEADERS64->IMAGE_FILE_HEADER->NumberOfSections offst 0x06
    NumSection = *(uint16_t*)(RawData + NtHeaderOffset + 0x06);

    // sizeof(IMAGE_OPTIONAL_HEADER64) == 0x108
    OffsetSection = NtHeaderOffset + 0x108;

    // IMAGE_NT_HEADERS64->IMAGE_OPTIONAL_HEADER64->ImageBase offset 0x30
    ImageBase = *(uint64_t*)(RawData + NtHeaderOffset + 0x30);
    ImageEntry = *(uint32_t*)(RawData + NtHeaderOffset + 0x28);

    MaxVirtualAddress = LoadSections(PEImageBase, RawData, OffsetSection, NumSection);
    if (MaxVirtualAddress == -1)
    {
        return 0;
    }

    // �����ض�λ���͵�����������ں���˵����Ҫ
    if (ImageBaseRet != NULL)
        *ImageBaseRet = ImageBase;

    if (MaxVirtual != NULL)
        *MaxVirtual = MaxVirtualAddress;

    return (uint64_t)ImageBase + ImageEntry;
}

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

int Goto64(char* ImageEntry)
{
    int i;
    int j;
    uint64_t GdtrBase[2] = { 0 };

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
        memset((void*)(((uint64_t)PDTBase) + i * PAGE_SIZE), 0, PAGE_SIZE);
        PDT = (uint64_t*)(((uint64_t)PDTBase) + i * PAGE_SIZE);

        for (j = 0; j < PAGE_SIZE / sizeof(uint64_t); j++)
        {
            PDT[j] = PAGE_PRESENT | PAGE_WRITABLE | PAGE_SIZE_FLAG | ((PAGE_SIZE_2M * (j + i * (PAGE_SIZE) / sizeof(uint64_t))) & ~(PAGE_SIZE_2M - 1ULL));
        }
    }

    // 0-15 bits : GDT Limitation
    // 16-79 bits : GDT Base
    GdtrBase[0] = (sizeof(LongModeSegmentTable) - 1);
    *(uint64_t*)((char*)GdtrBase + 2) = (uint64_t)LongModeSegmentTable;

    goto_to_64(GdtrBase, PLM4T, ImageEntry);

    return 0;
}