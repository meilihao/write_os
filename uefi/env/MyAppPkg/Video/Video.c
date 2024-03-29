#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>


EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS Status = EFI_SUCCESS;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gGraphicsOutput = 0;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info = 0;
	UINTN InfoSize = 0;
	int i = 0;

	// ErrorCode in MdePkg/Include/Uefi/UefiBaseType.h
	Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,NULL,(VOID **)&gGraphicsOutput);
	if(EFI_ERROR(Status))
    {
        Print(L"Failed to LocateProtocol.\n");
        return Status;
    }

	Print(L"Current Mode:%02d,Version:%x,Format:%d,Horizontal:%d,Vertical:%d,ScanLine:%d,FrameBufferBase:%010lx,FrameBufferSize:%010lx\n",gGraphicsOutput->Mode->Mode,gGraphicsOutput->Mode->Info->Version,gGraphicsOutput->Mode->Info->PixelFormat,gGraphicsOutput->Mode->Info->HorizontalResolution,gGraphicsOutput->Mode->Info->VerticalResolution,gGraphicsOutput->Mode->Info->PixelsPerScanLine,gGraphicsOutput->Mode->FrameBufferBase,gGraphicsOutput->Mode->FrameBufferSize);

	for(i = 0;i < gGraphicsOutput->Mode->MaxMode;i++)
	{
		Status = gGraphicsOutput->QueryMode(gGraphicsOutput,i,&InfoSize,&Info);
		if(EFI_ERROR(Status))
		{
			Print(L"Failed to QueryMode.\n");
			return Status;
		}

		Print(L"Mode:%02d,Version:%x,Format:%d,Horizontal:%d,Vertical:%d,ScanLine:%d\n",i,Info->Version,Info->PixelFormat,Info->HorizontalResolution,Info->VerticalResolution,Info->PixelsPerScanLine);
		gBS->FreePool(Info);
	}

	// [通过HandleProtocol和LocateProtocol打开的Protocol因为没有指定AgentHandle，所以无法关闭。如果一定要关闭它，则要调用OpenProtocolInformation（）获得AgentHandle和ControllerHandle，然后再关闭它](https://blog.csdn.net/sevensevensevenday/article/details/70917178)
	Status = gBS->CloseProtocol(gGraphicsOutput,&gEfiGraphicsOutputProtocolGuid,ImageHandle,NULL);
	if(EFI_ERROR(Status))
	{
		Print(L"Failed to CloseProtocol:%d.\n", Status);
		return Status;
	}

	return EFI_SUCCESS;
}

