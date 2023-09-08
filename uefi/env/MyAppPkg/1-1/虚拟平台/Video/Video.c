/***************************************************
*		版权声明
*
*	本操作系统名为：MINE
*	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
*	只允许个人学习以及公开交流使用
*
*	代码最终所有权及解释权归田宇所有；
*
*	本模块作者：	田宇
*	EMail:		345538255@qq.com
*
*
***************************************************/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>


EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gGraphicsOutput = 0;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info = 0;
	UINTN InfoSize = 0;
	int i = 0;

	gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,NULL,(VOID **)&gGraphicsOutput);
	Print(L"Current Mode:%02d,Version:%x,Format:%d,Horizontal:%d,Vertical:%d,ScanLine:%d,FrameBufferBase:%010lx,FrameBufferSize:%010lx\n",gGraphicsOutput->Mode->Mode,gGraphicsOutput->Mode->Info->Version,gGraphicsOutput->Mode->Info->PixelFormat,gGraphicsOutput->Mode->Info->HorizontalResolution,gGraphicsOutput->Mode->Info->VerticalResolution,gGraphicsOutput->Mode->Info->PixelsPerScanLine,gGraphicsOutput->Mode->FrameBufferBase,gGraphicsOutput->Mode->FrameBufferSize);

	for(i = 0;i < gGraphicsOutput->Mode->MaxMode;i++)
	{
		gGraphicsOutput->QueryMode(gGraphicsOutput,i,&InfoSize,&Info);
		Print(L"Mode:%02d,Version:%x,Format:%d,Horizontal:%d,Vertical:%d,ScanLine:%d\n",i,Info->Version,Info->PixelFormat,Info->HorizontalResolution,Info->VerticalResolution,Info->PixelsPerScanLine);
		gBS->FreePool(Info);
	}

	gBS->CloseProtocol(gGraphicsOutput,&gEfiGraphicsOutputProtocolGuid,ImageHandle,NULL);

	return EFI_SUCCESS;
}

