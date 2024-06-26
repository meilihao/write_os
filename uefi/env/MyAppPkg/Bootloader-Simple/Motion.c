#include "Motion.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL Grey = {166, 166, 166, 0};

EFI_STATUS VideoInit(
    IN EFI_HANDLE ImageHandle,
    OUT VIDEO_CONFIG *VideoConfig
)
{
    EFI_STATUS Status = EFI_SUCCESS;

    Status = GetGopHandle(ImageHandle, &Gop);

    Status = SetVideoMode(Gop);

    VideoConfig->FrameBufferBase = Gop->Mode->FrameBufferBase;
    VideoConfig->FrameBufferSize = Gop->Mode->FrameBufferSize;
    VideoConfig->HorizontalResolution = Gop->Mode->Info->HorizontalResolution;
    VideoConfig->VerticalResolution = Gop->Mode->Info->VerticalResolution;
    VideoConfig->PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;
    
    return Status;
}

// 其他显示图片的项目:
// http://yiiyee.cn/blog/2021/08/20/uefi%E5%BC%80%E5%8F%91%E6%8E%A2%E7%B4%A296-%E6%B8%A9%E5%BA%A6%E8%AE%A1%E5%B0%8F%E6%B8%B8%E6%88%8F/
EFI_STATUS DrawLogo(
    IN EFI_HANDLE ImageHandle
)
{
    EFI_STATUS Status = EFI_SUCCESS;

    CHAR16 *FileName = L"\\Narrow.BMP";
    UINTN Hor = Gop->Mode->Info->HorizontalResolution;
    UINTN Ver = Gop->Mode->Info->VerticalResolution;
    if(Hor * 3 == Ver * 4)
    {
        FileName = L"\\Logo.BMP"; 
    }
    EFI_FILE_PROTOCOL *Logo;
    Status = GetFileHandle(ImageHandle, FileName, &Logo);
    
    EFI_PHYSICAL_ADDRESS LogoAddress;
    Status = ReadFile(Logo, &LogoAddress);

    BMP_CONFIG BmpConfig;
    Status = BmpTransform(LogoAddress, &BmpConfig);

    UINTN X = (Hor - BmpConfig.Width) / 2;
    UINTN Y = (Ver - BmpConfig.Height) / 2;

    Status = DrawBmp(Gop, BmpConfig, X, Y);
    
    return Status;
}

EFI_STATUS DrawStep(
    IN UINTN Step
)
{
    //Print(L"start draw step.\n");

    EFI_STATUS Status = EFI_SUCCESS;

    UINTN BlockWidth = Gop->Mode->Info->HorizontalResolution >> 6;
    UINTN BlockHeight = Gop->Mode->Info->VerticalResolution >> 6;
    // 进度条: GAP是间隙, 总宽度=(BlockWidth + GAP) * 10 - GAP
    UINTN StartX = (Gop->Mode->Info->HorizontalResolution - (BlockWidth + GAP) * 10 - GAP) / 2;
    UINTN StartY = (Gop->Mode->Info->VerticalResolution * 3) >> 2; // VerticalResolution*3/4

    UINTN X = StartX + (BlockWidth + GAP) * Step;

    Status = Gop->Blt(Gop, &Grey, EfiBltVideoFill, 0, 0, X, StartY, BlockWidth, BlockHeight, 0);
    
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR:Failed to Blt Block.\n");
        return Status;
    }
    //Print(L"SUCCESS:DrawStep:%d.\n", Step);
    #endif

    return Status;
}
