// Copyright by Xiyue 2022

#pragma warning (disable : 4091)

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _EFI_LOADER_H_
#define _EFI_LOADER_H_

#include <efi.h>
#include <efilib.h>


    typedef unsigned char       uint8_t;
    typedef char                int8_t;
    typedef unsigned short      uint16_t;
    typedef short               int16_t;

    typedef unsigned long long  uint64_t;
    typedef long long           int64_t;

#define ERR_STRING              (CHAR16*) L"Error At "
#define CRLF_STRING             (CHAR16*) L"\r\n"

    void OutputDbgLine(EFI_SYSTEM_TABLE* SystemTable, CHAR16* TypeString, int LineNo);
    void OutputErrorLine(EFI_SYSTEM_TABLE* SystemTable, int LineNo);

    CHAR16* ShortStringToWidthString(const char* ShortString, CHAR16* WidthString, INTN Size);
    char* WidthStringToShortString(const CHAR16* WidthString, char* ShortString, INTN Size);
    void SioPutHexValue(char* Prefix, uint32_t value, char* EndStr);

    uint8_t io_in_byte(uint16_t port);
    void io_out_byte(uint16_t port, uint8_t data);

    void SioPuts(const char* String);
    int LoadPE64(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, const char* Filename, UINTN MapKey);
    uint64_t LoadPEFileData(char* PEImageBase, char* RawData, uint64_t* ImageBaseRet, uint32_t* MaxVirtual);
    void Wait5Seconds(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

    int Goto64(char* ImageBase);
    void goto_to_64(void* Gdt, void* PageTable, void* EntryOf64);

    char* strcpy(char* s1, const char* s2);
    char* strcat(char* s1, const char* s2);
    void* memmove(void* dst, const void* src, size_t n);
    void* memcpy(void* dst, const void* src, size_t n);
    void* memset(void* s, int c, size_t n);

    char* itoh(int v, char* b);
    char* itoa(int v, char* b);
#endif /* _EFI_LOADER_H_ */


#ifdef __cplusplus
}
#endif