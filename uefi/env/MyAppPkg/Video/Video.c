#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>


EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS Status = EFI_SUCCESS;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gGraphicsOutput = NULL;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info = NULL;
	UINTN InfoSize = 0;
	int i = 0;

	// ErrorCode in MdePkg/Include/Uefi/UefiBaseType.h
	Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,NULL,(VOID **)&gGraphicsOutput);
	if(EFI_ERROR(Status))
    {
        Print(L"Failed to LocateProtocol.\n");
        return Status;
    }

	Print(L"Current Mode:%02d,Version:%x,Format:%d,Horizontal:%d,Vertical:%d,ScanLine:%d,FrameBufferBase:%010lx,FrameBufferSize:%010lx\n",
		gGraphicsOutput->Mode->Mode,
		gGraphicsOutput->Mode->Info->Version,
		gGraphicsOutput->Mode->Info->PixelFormat,
		gGraphicsOutput->Mode->Info->HorizontalResolution,
		gGraphicsOutput->Mode->Info->VerticalResolution,
		gGraphicsOutput->Mode->Info->PixelsPerScanLine,
		gGraphicsOutput->Mode->FrameBufferBase,
		gGraphicsOutput->Mode->FrameBufferSize);

	for(i = 0;i < gGraphicsOutput->Mode->MaxMode;i++)
	{
		Status = gGraphicsOutput->QueryMode(gGraphicsOutput,i,&InfoSize,&Info);
		if(EFI_ERROR(Status))
		{
			Print(L"Failed to QueryMode.\n");
			return Status;
		}

		Print(L"Mode:%02d,Version:%x,Format:%d,Horizontal:%d,Vertical:%d,ScanLine:%d\n",
			i,Info->Version,Info->PixelFormat,Info->HorizontalResolution,Info->VerticalResolution,Info->PixelsPerScanLine);
		gBS->FreePool(Info);
	}

	// [通过HandleProtocol和LocateProtocol打开的Protocol因为没有指定AgentHandle，所以无法关闭。如果一定要关闭它，则要调用OpenProtocolInformation（）获得AgentHandle和ControllerHandle，然后再关闭它](https://blog.csdn.net/sevensevensevenday/article/details/70917178)
	// 建议使用 LocateHandleBuffer() 获取句柄 + OpenProtocol() 打开 + CloseProtocol() 关闭
	// **LocateProtocol 拿到的指针直接用即可，不需要 CloseProtocol; UEFI 规范对 CloseProtocol 的描述是关闭一个“之前用 OpenProtocol 打开”的句柄, 没有OpenProtocol也就无需CloseProtocol**
	Status = gBS->CloseProtocol(gGraphicsOutput,&gEfiGraphicsOutputProtocolGuid,ImageHandle,NULL);
	if(EFI_ERROR(Status))
	{
		Print(L"Failed to CloseProtocol:%d.\n", Status);
		return Status;
	}

	return EFI_SUCCESS;
}

