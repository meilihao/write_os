// Copyright by Xiyue87 2022

#define  _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PE_SIZE         480*1024

typedef void (*EntryFunc)(const char*);

char* LoadFile(const char* Filename, int* FileSize)
{
    FILE* fp;
    int Offset;
    char* Buf;

    /* 读取文件内容，申请buffer，保存全部内容并返回 */

    fp = fopen(Filename, "rb");

    if (fp == NULL)
    {
        printf("Can not open %s\n", Filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    Offset = ftell(fp);

    if (FileSize != NULL)
        *FileSize = Offset;

    Buf = (char*)malloc(Offset);

    if (Buf == NULL)
    {
        printf("Can not alloc buffer\n");
        fclose(fp);
        return NULL;
    }

    fseek(fp, 0, SEEK_SET);

    fread(Buf, 1, Offset, fp);

    fclose(fp);

    return Buf;
}

int LoadSections(char* PEImageBase, char* RawData, int OffsetSection, int NumSection, char** LastValidData)
{
    IMAGE_SECTION_HEADER* SectionHeader;
    int i;

    for (i = 0; i < NumSection; i++)
    {
        SectionHeader = (IMAGE_SECTION_HEADER*)(RawData + OffsetSection + i * sizeof(IMAGE_SECTION_HEADER));

        printf("Section Name            : [%s]\n", SectionHeader->Name);
        printf("Virtual Address         : [0x%08X]\n", SectionHeader->VirtualAddress);
        printf("Virtual Size            : [0x%08X]\n", SectionHeader->Misc.VirtualSize);
        printf("Size of Raw Data        : [0x%08X]\n", SectionHeader->SizeOfRawData);
        printf("Pointer to Raw Data     : [0x%08X]\n", SectionHeader->PointerToRawData);
        printf("Pointer to Rel Table    : [0x%08X]\n", SectionHeader->PointerToRelocations);
        printf("Pointer to Line Number  : [0x%08X]\n", SectionHeader->PointerToLinenumbers);
        printf("Number of Rel Entry     : [0x%08X]\n", SectionHeader->NumberOfRelocations);
        printf("Number of Line Number   : [0x%08X]\n", SectionHeader->NumberOfLinenumbers);
        /*
         * 从原始数据流中找到所有的section，并复制内容到ImageBase的地址上
         */

        if (SectionHeader->VirtualAddress + SectionHeader->Misc.VirtualSize >= PE_SIZE)
        {
            printf("Exceed max size\n");
            return -1;
        }
        memcpy(PEImageBase + SectionHeader->VirtualAddress, RawData + SectionHeader->PointerToRawData, SectionHeader->SizeOfRawData);
        *LastValidData = PEImageBase + SectionHeader->VirtualAddress + SectionHeader->Misc.VirtualSize;
    }

    return 0;
}

char* LoadPE(char* PEImageBase, char* RawData, char** LastValidOffset, char** ImageBase)
{
    IMAGE_NT_HEADERS* NtHeader;
    IMAGE_DOS_HEADER* DosHeader = (IMAGE_DOS_HEADER*)RawData;
    WORD NumSection;
    int OffsetSection;
    int ret;

    NtHeader = (IMAGE_NT_HEADERS*)(RawData + DosHeader->e_lfanew);

    NumSection = NtHeader->FileHeader.NumberOfSections;
    OffsetSection = DosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS);
    ret = LoadSections(PEImageBase, RawData, OffsetSection, NumSection, LastValidOffset);
    if (ret != 0)
    {
        return NULL;
    }

    // 忽略重定位表和导入表，对于内核来说不需要

    *ImageBase = (char*)NtHeader->OptionalHeader.ImageBase;
    return (char*)NtHeader->OptionalHeader.AddressOfEntryPoint + NtHeader->OptionalHeader.ImageBase;
}

char* PrepareBit16Header(char* Filename, int* EntryDataOffset, int* FileSize)
{
    char* Header16;
    int i;

    Header16 = LoadFile(Filename, FileSize);
    if (Header16 == NULL)
    {
        return NULL;
    }

    for (i = 0; i < *FileSize - sizeof(int); i++)
    {
        if (*(int*)(Header16 + i) == 0xCCDDEEFF)
        {
            *EntryDataOffset = i + sizeof(int);
            return Header16;
        }
    }

    free(Header16);

    return NULL;
}

int main(int argc, char* argv[])
{
    char* RawData;
    char* PEImageBase;
    char* RealBase;
    char* RealEntry;
    char* LastValidOffset;
    char* Header16;
    int EntryDataOffset;
    int Header16Size;
    int ProtectDataStart;
    int ProtectDataSize;
    int i;
    FILE* OutFp;

    if (argc != 4)
    {
        printf("Usage %s <input_pe_filename> <16bit_header> <binary_filename>\n", argv[0]);
        return -1;
    }

    RawData = LoadFile(argv[1], NULL);

    if (RawData == NULL)
    {

        return -1;
    }
    // Why use VitualAlloc here??
    PEImageBase = (char*)VirtualAlloc(0, PE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (PEImageBase == NULL)
    {
        printf("Can not allocate\n");
        return -1;
    }

    memset(PEImageBase, 0, PE_SIZE);

    RealEntry = LoadPE(PEImageBase, RawData, &LastValidOffset, &RealBase);

    if (RealEntry == NULL)
    {
        printf("Load PE fail\n");
        free(RawData);
        VirtualFree(PEImageBase, 0, MEM_RELEASE);
        return -1;
    }

    printf("RealEntry: 0x%08X RealBase: 0x%08X\n", (long)RealEntry, (long)RealBase);

    OutFp = fopen(argv[3], "wb");

    if (OutFp == NULL)
    {
        printf("Can not open %s for output\n", argv[3]);
        free(RawData);
        VirtualFree(PEImageBase, 0, MEM_RELEASE);
        return -1;
    }
    Header16 = PrepareBit16Header(argv[2], &EntryDataOffset, &Header16Size);
    fwrite(Header16, Header16Size, 1, OutFp);
    for (i = 0; i < 16; i++)
    {
        unsigned char nop = 0x90;
        fwrite(&nop, 1, 1, OutFp);
    }

    ProtectDataStart = ftell(OutFp);
    ProtectDataSize = LastValidOffset - PEImageBase;
    fseek(OutFp, EntryDataOffset, SEEK_SET);
    fwrite(&RealBase, sizeof(RealBase), 1, OutFp);
    fwrite(&ProtectDataStart, sizeof(ProtectDataStart), 1, OutFp);
    fwrite(&ProtectDataSize, sizeof(ProtectDataSize), 1, OutFp);
    fwrite(&RealEntry, sizeof(RealEntry), 1, OutFp);
    fseek(OutFp, ProtectDataStart, SEEK_SET);

    fwrite(PEImageBase, ProtectDataSize, 1, OutFp);

    fclose(OutFp);
    free(RawData);
    VirtualFree(PEImageBase, 0, MEM_RELEASE);

    return 0;
}
