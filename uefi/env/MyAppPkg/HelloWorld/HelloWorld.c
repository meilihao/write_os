/** @file HelloWorld.c
 * EFI Hello World application
 **/
#include <Uefi.h>

/**
 * Entry point for the application
 *
 * @param[in] ImageHandle  The firmware allocated handle for the EFI image
 * @param[in] SystemTable  A pointer to the EFI System Table
 *
 * @retval EFI_SUCCESS  The entry point is executed successufully
 * @retval other        Some error occurs while executing the application
 **/

#include <Uefi.h>

EFI_STATUS EFIAPI UefiMain(
	IN EFI_HANDLE ImageHandle, // image句柄
	IN EFI_SYSTEM_TABLE *SystemTable) // 系统表指针
{
	SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Hello World\n");
	return EFI_SUCCESS;
}

