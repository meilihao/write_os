// Copyright by Xiyue 2022

#include "EFILoader.h"

EFI_GUID LoadedImageProtocol = LOADED_IMAGE_PROTOCOL;
EFI_GUID FileSystemProtocol = SIMPLE_FILE_SYSTEM_PROTOCOL;
EFI_GUID GenericFileInfo = EFI_FILE_INFO_ID;

EFI_STATUS __cdecl efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_BOOT_SERVICES* BootServices;    
    EFI_STATUS ret;
    EFI_MEMORY_DESCRIPTOR* EfiMemMap = NULL;
    void* Buffer;
    UINTN MapSize = 0;
    UINTN MapKey;
    UINTN MapDescSize;
    UINT32 MapVersion;
    UINTN TotalSize;

    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"HELLO EFI 64\r\n");

    BootServices = SystemTable->BootServices;

    // ��һ�ε��ã�MapSize��0�������ܻ��MemoryMap������

    ret = BootServices->GetMemoryMap(&MapSize, EfiMemMap, &MapKey, &MapDescSize, &MapVersion);

    // ��һ�ε��ã�����ֵ��ȻӦ����EFI_BUFFER_TOO_SMALL
    if (ret != EFI_BUFFER_TOO_SMALL)
    {
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }

    TotalSize = MapSize + sizeof(EFI_MEMORY_DESCRIPTOR) * 20; // �����ڴ�ᵼ��MemoryMap�б仯�����Զ�����һЩ
    // �����ڴ棬���ڴ��EFI_MEMORY_DESCRIPTOR
    if (BootServices->AllocatePool(EfiRuntimeServicesData, TotalSize, &Buffer) != EFI_SUCCESS)
    {
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }

    memset(Buffer, 0, TotalSize);
    EfiMemMap = (EFI_MEMORY_DESCRIPTOR*)Buffer;
    //�ٴλ�ȡ����һ��Ӧ���ܳɹ�
    ret = BootServices->GetMemoryMap(&TotalSize, EfiMemMap, &MapKey, &MapDescSize, &MapVersion);

    if (ret != EFI_SUCCESS || MapVersion != EFI_MEMORY_DESCRIPTOR_VERSION)
    {
        OutputErrorLine(SystemTable, __LINE__);
        goto ERROR_DEAD;
    }
    Wait5Seconds(ImageHandle, SystemTable);
    LoadPE64(ImageHandle, SystemTable, "HELLO64.EXE", MapKey);

ERROR_DEAD:

    while (1)
    {

    }
    return 0;
}

void SioPutHexValue(char* Prefix, uint32_t value, char* EndStr)
{
    char HexBuf[40] = { 0 };
    SioPuts(Prefix);
    SioPuts(itoh(value, HexBuf));
    SioPuts(EndStr);
}

CHAR16* ShortStringToWidthString(const char* ShortString, CHAR16* WidthString, INTN Size)
{
    INTN i;

    for (i = 0; i < Size - 1 && *ShortString != 0; i++)
    {
        WidthString[i] = ShortString[i];
    }
    WidthString[i] = 0;
    return WidthString;
}

char* WidthStringToShortString(const CHAR16* WidthString, char* ShortString, INTN Size)
{
    INTN i;

    for (i = 0; i < Size - 1 && *ShortString != 0; i++)
    {
        ShortString[i] = (char)WidthString[i];
    }
    ShortString[i] = 0;
    return ShortString;
}

void OutputDbgLine(EFI_SYSTEM_TABLE* SystemTable, CHAR16* TypeString, int LineNo)
{
    CHAR16 LnStr[40] = { 0 };
    char LnSStr[40] = { 0 };
    
    itoa(LineNo, LnSStr);
    ShortStringToWidthString(LnSStr, LnStr, sizeof(LnStr) / sizeof(CHAR16));

    SystemTable->ConOut->OutputString(SystemTable->ConOut, TypeString);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, LnStr);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, CRLF_STRING);
}

void OutputErrorLine(EFI_SYSTEM_TABLE* SystemTable, int LineNo)
{
    OutputDbgLine(SystemTable, ERR_STRING, LineNo);
}

void Wait5Seconds(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_BOOT_SERVICES* BootServices = SystemTable->BootServices;

    EFI_EVENT Timer;
    EFI_EVENT TimerWait;
    UINTN EventIndex;
    int i;

    BootServices->CreateEvent(EVT_TIMER, 0, NULL, NULL, &Timer);

    for (i = 5; i > 0; i--)
    {
        CHAR16 SecondStr[3] = { ' ', 0 };
        SecondStr[1] = '0' + i;
        SystemTable->ConOut->OutputString(SystemTable->ConOut, SecondStr);
        BootServices->SetTimer(Timer, TimerRelative, 10000000);
        TimerWait = Timer;
        BootServices->WaitForEvent(1, &TimerWait, &EventIndex);
    }

    BootServices->CloseEvent(Timer);

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");
    return;
}
