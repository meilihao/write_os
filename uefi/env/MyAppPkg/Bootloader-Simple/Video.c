#include "Video.h"
#include <Protocol/DevicePathToText.h>

// https://en.wikipedia.org/wiki/BMP_file_format
#pragma pack(1) // 让 bmp_header 结构体以 1 字节对齐，使得 bmp_header 的数据成员在内存中紧密排布
struct bmp_header {
  UINT8 sig[2];
  UINT32 file_size;
  UINT16 reserved1;
  UINT16 reserved2;
  UINT32 header_size;
  UINT32 info_header_size;
  UINT32 width;
  UINT32 height;
  UINT16 plane_num;
  UINT16 color_bit;
  UINT32 compression_type;
  UINT32 compression_size;
  UINT32 horizontal_pixel;
  UINT32 vertical_pixel;
  UINT32 color_num;
  UINT32 essentail_num;
};

EFI_STATUS GetGopHandle(
    IN EFI_HANDLE ImageHandle,
    OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Gop
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN HandleCount = 0;
    EFI_HANDLE *HandleBuffer;

    //LocateHandleBuffer:从句柄数据库中检索满足搜索条件的句柄列表, 返回缓冲区是自动分配的
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiGraphicsOutputProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
    );
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR:Failed to LocateHanleBuffer of GraphicsOutputProtocol.\n");
        return Status;
    }
    Print(L"SUCCESS:Get %d handles that supported GraphicsOutputProtocol.\n", HandleCount);

    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL* Device2TextProtocol = 0;
    gBS->LocateProtocol(&gEfiDevicePathToTextProtocolGuid,NULL,(VOID**)&Device2TextProtocol);

    UINTN HandleIndex = 0;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    CHAR16* TextDevicePath;

	for(HandleIndex=0;HandleIndex<HandleCount;HandleIndex++){
        //???: 发现2个, 但测试环境打开第二个返回3, 表示不支持获取设备路径
        Status = gBS->OpenProtocol(HandleBuffer[HandleIndex],
                                    &gEfiDevicePathProtocolGuid,
                                    (VOID**)&DevicePath,
                                    gImageHandle,
                                    NULL,
                                    EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	
        if (EFI_ERROR(Status)){
            Print(L"OpenProtocol for gEfiGraphicsOutputProtocolGuid Error:%d\n",Status);
            continue;
        }  

        TextDevicePath = Device2TextProtocol->ConvertDevicePathToText(DevicePath,TRUE,TRUE);
        Print(L"TextDevicePath:%s\n",TextDevicePath);

        if(TextDevicePath)
        {
            gBS->FreePool(TextDevicePath);
        }

		Print(L"\n\n");
	}

    
    #endif
    Status = gBS->OpenProtocol(
        HandleBuffer[0],
        &gEfiGraphicsOutputProtocolGuid,
        (VOID **)Gop,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR:Failed to open first handle that supported GraphicsOutputProtocol.\n");
        return Status;
    }

    Print(L"SUCCESS:GraphicsOutputProtocol is opened with first handle.\n");
    
    #endif

    return Status;
}

EFI_STATUS SetVideoMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
)
{
    EFI_STATUS Status = EFI_SUCCESS;

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo;
    UINTN ModeInfoSize = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    UINTN H = 0;
    UINTN V = 0;
    UINTN ModeIndex = 0;

    for(UINTN i = 0; i < Gop->Mode->MaxMode; i++)
    {
        Status = Gop->QueryMode(Gop, i, &ModeInfoSize, &ModeInfo);
        H = ModeInfo->HorizontalResolution;
        V = ModeInfo->VerticalResolution;

        Print(L"Mode:%02d,Version:%x,Format:%d,Horizontal:%d,Vertical:%d,ScanLine:%d\n",i,ModeInfo->Version,ModeInfo->PixelFormat,ModeInfo->HorizontalResolution,ModeInfo->VerticalResolution,ModeInfo->PixelsPerScanLine);

        if(((H == 1024) && (V == 768)) || ((H == 1440) && (V == 900)))
        {    
            ModeIndex = i;            
        }
    }

    Status = Gop->SetMode(Gop, ModeIndex);

    Print(L"FrameBufferBase: %llx.\n", Gop->Mode->FrameBufferBase);

    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR:Failed to SetMode.\n");
        return Status;
    }
    Print(L"SUCCESS:SetMode to Index:%d.\n", ModeIndex);
    
    #endif
    
    return Status;
}

// bmp数据上下颠倒
void memswap(void *buf1, void *buf2, UINTN size) {
    const UINTN loopCnt = size / sizeof(UINTN);
    const UINTN remainder = size % sizeof(UINTN);

    UINTN swapBuf1 = 0;
    UINTN* buf1_UINTN_ptr = (UINTN *)buf1;
    UINTN* buf2_UINTN_ptr = (UINTN *)buf2;
    for (UINTN i = 0; i < loopCnt; i++) {
        swapBuf1 = buf1_UINTN_ptr[i];
        buf1_UINTN_ptr[i] = buf2_UINTN_ptr[i];
        buf2_UINTN_ptr[i] = swapBuf1;
    }

    UINT8 swapBuf2 = 0;
    UINT8* buf1_UINT8_ptr = (UINT8 *)buf1 + (size - remainder);
    UINT8* buf2_UINT8_ptr = (UINT8 *)buf2 + (size - remainder);
    for (UINTN i = 0; i < remainder; i++) {
        swapBuf2 = buf1_UINT8_ptr[i];
        buf1_UINT8_ptr[i] = buf2_UINT8_ptr[i];
        buf2_UINT8_ptr[i] = swapBuf2;
    }
}

void memswap2(void *buf1, void *buf2, UINTN size) {
    UINT8 swapBuf1 = 0;
    UINT8* buf1_UINTN_ptr = (UINT8 *)buf1;
    UINT8* buf2_UINTN_ptr = (UINT8 *)buf2;
    for (UINTN i = 0; i < size; i++) {
        swapBuf1 = *buf1_UINTN_ptr;
        *buf1_UINTN_ptr = *buf2_UINTN_ptr;
        *buf2_UINTN_ptr = swapBuf1;

        buf1_UINTN_ptr++;
        buf2_UINTN_ptr++;
    }
}

EFI_STATUS BmpTransform(
    IN EFI_PHYSICAL_ADDRESS BmpBase,
    OUT BMP_CONFIG *BmpConfig
)    
{  
    EFI_STATUS Status = EFI_SUCCESS;
    // Not used, just for example
    struct bmp_header *bheader = (struct bmp_header *)BmpBase;

    BmpConfig->Size = bheader->file_size;
    BmpConfig->PageSize = (BmpConfig->Size >> 12) + 1;
    BmpConfig->Offset = bheader->header_size;
    BmpConfig->Width = bheader->width;
    BmpConfig->Height = bheader->height;
    
    // // bmp_header未1对齐时的取法
    // BmpConfig->Size = GetValue(BmpBase, 0x02, 4);
    // BmpConfig->PageSize = (BmpConfig->Size >> 12) + 1;
    // BmpConfig->Offset = GetValue(BmpBase, 0x0A, 4);
    
    // BmpConfig->Width = GetValue(BmpBase, 0x12, 4);
    // BmpConfig->Height = GetValue(BmpBase, 0x16, 4);

    // data size = width * height * color_bit/8
    Print(L"Bmp size = %d\n", BmpConfig->Size);
    Print(L"Bmp header size = %d\n", BmpConfig->Offset);
    Print(L"Bmp width = %d\n", BmpConfig->Width);
    Print(L"Bmp height = %d\n", BmpConfig->Height);
    Print(L"Bmp color_bit = %d\n", bheader->color_bit);
    Print(L"Bmp compression_type = %d\n", bheader->compression_type);
    Print(L"Bmp compression_size = %d\n", bheader->compression_size);
    
    // ///???原有代码没画出logo
    // EFI_PHYSICAL_ADDRESS PixelStart;
    // Status = gBS->AllocatePages(
    //     AllocateAnyPages,
    //     EfiLoaderData,
    //     BmpConfig->PageSize,
    //     &PixelStart
    // );

    // #ifdef DEBUG
    // if(EFI_ERROR(Status))
    // {
    //     Print(L"ERROR:Failed to AllocatePages for PixelArea.\n");
    //     return Status;
    // }
    // Print(L"SUCCESS:Memory for PixelArea is ready.\n");
    // #endif

    // EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelFromFile = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)(BmpBase 
    //                                                 + BmpConfig->Offset 
    //                                                 + BmpConfig->Width * BmpConfig->Height * 4);
    // EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelToBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)PixelStart;
    // for(UINTN i = 0; i < BmpConfig->Height; i++)
    // {
    //     PixelFromFile -= BmpConfig->Width;
    //     for(UINTN j = 0; j < BmpConfig->Width; j++)
    //     {
    //         *PixelToBuffer = *PixelFromFile;
    //         PixelToBuffer++;
    //         PixelFromFile++;
    //     }
    //     PixelFromFile -= BmpConfig->Width;
    // }

    // BmpConfig->PixelStart = PixelStart;

    BmpConfig->PixelStart = BmpBase + BmpConfig->Offset;
    const EFI_PHYSICAL_ADDRESS PixelStart = BmpConfig->PixelStart;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)PixelStart;
    // inplace swap
    const UINTN Height = BmpConfig->Height;
    const UINTN Width  = BmpConfig->Width;
    for (UINTN i = 0; i < Height / 2; i++) {
        if (i%50==0){
            Print(L"Bmp  memswap2= %d,%d\n", i, Height / 2);
        }
        // memswap也是同样效果
        memswap2(
            PixelBuffer + i * Width,
            PixelBuffer + (Height - i - 1) * Width,
            Width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
        );
    }

    Print(L"Bmp PixelStart = %08lx\n", BmpConfig->PixelStart);

    return Status;
}

EFI_STATUS DrawBmp(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
    IN BMP_CONFIG BmpConfig,
    IN UINTN X,
    IN UINTN Y
)
{
    Print(L"start draw Logo.\n");

    EFI_STATUS Status = EFI_SUCCESS;

    Status = Gop->Blt(
        Gop,
        (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)BmpConfig.PixelStart,
        EfiBltBufferToVideo,
        0,0,
        X,Y,
        BmpConfig.Width,BmpConfig.Height,0
    );

    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR:Failed to Blt Logo.BMP, code=%d.\n", Status);
        return Status;
    }
    Print(L"SUCCESS:You should see the Logo.\n");
    #endif
    return Status;
}
