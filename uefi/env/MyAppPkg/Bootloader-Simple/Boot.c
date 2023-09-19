#include "Boot.h"

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS Status = EFI_SUCCESS;   

    #ifdef LOG
    Status = LogInitial(ImageHandle);
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR:Failed to LogInitial.\n");
    }else
    {
        LogTip("Log is good now.\n");
    }
    
    #endif
    VIDEO_CONFIG VideoConfig;
    Status = VideoInit(ImageHandle, &VideoConfig);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status);
    }else
    {
        LogTip("Video is good now.\n");
    }
    #endif
    
    Status = DrawLogo(ImageHandle);

    for(UINTN i = 0; i < 10; i++)
    {
        Status = DrawStep(i);
        gBS->Stall(1000);
    }
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status);
    }else
    {
        LogTip("DrawLogo is good now.\n");
    }
    #endif

    //return Status; //获取FrameBufferBase

    EFI_FILE_PROTOCOL *Bin;
    Status = GetFileHandle(ImageHandle, L"\\Kernel.bin", &Bin);
    EFI_PHYSICAL_ADDRESS BinAddress;
    Status = ReadFile(Bin, &BinAddress);
    Status = ByeBootServices(ImageHandle);

    Print(L"Kernel.bin EnterPoint:%p", BinAddress);
    asm("jmp *%0": : "m"(BinAddress)); 

    return Status;
}

EFI_STATUS ByeBootServices(
    IN EFI_HANDLE ImageHandle
)
{
    // 获取GetMemoryMap
    Print(L"\nBye BS.\n");
    EFI_STATUS Status = EFI_SUCCESS;
    MEMORY_MAP MemMap = {4096 * 4, NULL,4096,0,0,0};

    Status = gBS->AllocatePool(
        EfiLoaderData,
        MemMap.BufferSize,
        &MemMap.Buffer
    );
    if(EFI_ERROR(Status))
    {
        Print(L"Allocate pool for memory map failed.\n");
        return Status;
    }

    Print(L"\nAllocate pool for memory map ok\n");

    Status = gBS->GetMemoryMap( // for ExitBootServices's MapKey
        &MemMap.BufferSize,
        (EFI_MEMORY_DESCRIPTOR *)MemMap.Buffer,
        &MemMap.MapKey,
        &MemMap.DescriptorSize,
        &MemMap.DescriptorVersion
    );

    Print(L"\ncall GetMemoryMap\n");

    if(EFI_ERROR(Status))
    {
        Print(L"Get memory map error.\n");
        return Status;
    }

    //Print(L"\nGet memory map ok\n"); ???GetMemoryMap后没有Print, ByeBootServices会卡住. 用自编译edk2-stable202308遇到同样问题
 
    Status = gBS->ExitBootServices(
        ImageHandle, MemMap.MapKey
    );

    if(EFI_ERROR(Status))
    {
        Print(L"ExitBootServices error.\n");
        return Status;
    }

    //Print(L"ExitBootServices ok.\n");

    return Status;
}

