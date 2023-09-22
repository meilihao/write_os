// Copyright by Xiyue87 2022

#include "fatfs.h"
#include "siodebug.h"
#include "string.h"
#include "stdio.h"

#define LONG_MODE_BASE 0x00800000

/*
 * Memory layout
 * PBR boot code: Load SYSLDR.BIN to memory and run
 *   start addr: 0000:0x7C00
 *
 * SYSLDR.BIN: 16/32 bit loader, to enable protect mode and load HELLO64.EXE to memory and run
 *   Real Mode:
 *     start addr: 0800:0x0000
 *
 *   Protect Mode: 4G flat
 *     Image Base: 0x00400000
 *     Image Entry: Depend on compiler
 *
 *     CS: 0008
 *     DS/ES/FS/GS: 0018
 *     SS: 0010
 *
 *     PE32+ RawData Base:  0x00200000, Size 0x00100000
 *     Stack:
 *       SS:ESP = 0018:00180000
 *       SS:EBP = 0018:00180000
 *
 *   64 Long Mode:
 *     Image Base: 0x00800000
 */

unsigned long long LoadPE32Plus(char* PEImageBase, char* RawData);
void SioPutHexValue(char* Prefix, uint32_t value, char* EndStr);
int Goto64(char* ImageBase);
int main()
{
    char p[] = { "Hello world" };
    char* vgabase = (char*)0xb8000;

    int i;
    char* PERawData;
    long long Entry;

    for (i = 0; p[i] != 0; i++)
    {
        vgabase[i << 1] = p[i];
    }
    SioPuts("Now we are in 32-bit protect mode (page not enabled)\n");
    
    InitFileSystem();
    ListRoot();
    PERawData = LoadKernelFile("HELLO64 EXE");

    memset((void *)LONG_MODE_BASE, 0, 0x100000);

    Entry = LoadPE32Plus((char*)LONG_MODE_BASE, PERawData);
    SioPutHexValue("Long Mode PE32+ Entry [", (long)Entry, "]\n");

    Goto64((char*)Entry);
    while (1)
    {

    }
    return 0;
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

void SioPutHexValue(char* Prefix, uint32_t value, char* EndStr)
{
    char HexBuf[30] = { 0 };
    SioPuts(Prefix);
    SioPuts(itoh(value, HexBuf));
    SioPuts(EndStr);
}

int LoadSections(char* PEImageBase, char* RawData, int OffsetSection, int NumSection)
{
    IMAGE_SECTION_HEADER* SectionHeader;
    int i;
    char SectionName[9];
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
        memcpy(PEImageBase + SectionHeader->VirtualAddress, RawData + SectionHeader->PointerToRawData, SectionHeader->SizeOfRawData);
    }

    return 0;
}

unsigned long long LoadPE32Plus(char* PEImageBase, char* RawData)
{       
    uint16_t NumSection;
    uint32_t OffsetSection;
    uint32_t NtHeaderOffset;
    uint64_t ImageBase;
    uint32_t ImageEntry;

    int ret;

    // IMAGE_DOS_HEADER->e_lfanew offset 0x3C ==> IMAGE_NT_HEADERS64
    NtHeaderOffset = *((uint32_t*)(RawData + 0x3C));

    // IMAGE_NT_HEADERS64->IMAGE_FILE_HEADER->NumberOfSections offst 0x06
    NumSection = *(uint16_t *)(RawData + NtHeaderOffset + 0x06);
    
    // sizeof(IMAGE_OPTIONAL_HEADER64) == 0x108
    OffsetSection = NtHeaderOffset + 0x108;

    // IMAGE_NT_HEADERS64->IMAGE_OPTIONAL_HEADER64->ImageBase offset 0x30
    ImageBase = *(uint64_t*)(RawData + NtHeaderOffset + 0x30);
    ImageEntry = *(uint32_t*)(RawData + NtHeaderOffset + 0x28);

    ret = LoadSections(PEImageBase, RawData, OffsetSection, NumSection);
    if (ret != 0)
    {
        return 0;
    }

    // �����ض�λ���͵�����������ں���˵����Ҫ

    return (uint64_t)ImageBase + ImageEntry;
}
