// Copyright by Xiyue87 2022

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
    char DeviceName[20] = { "\\\\.\\A:" };
    char* DriveStr;
    char DiskBuf1[512];
    char DiskBuf2[512];
    if (argc != 3)
    {
        printf("Usage %s <source_bin> <target drive>\n", argv[0]);
        return -1;
    }
    DriveStr = argv[2];
    if (strlen(DriveStr) != 2 || DriveStr[0] < 'A' || DriveStr[0] > 'Z' || DriveStr[1] != ':')
    {
        printf("Wrong parameter %s\n", argv[2]);
        return -1;
    }

    DeviceName[4] = DriveStr[0];

    FILE* fin;

    fin = fopen(argv[1], "rb");

    if (fin == NULL)
    {
        printf("Can not open %s\n", argv[1]);
        return -1;
    }

    HANDLE hDisk;
    DWORD Bytes;
    hDisk = CreateFileA(DeviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hDisk == INVALID_HANDLE_VALUE)
    {
        fclose(fin);
        printf("Can not open %s\n", DeviceName);
        return -1;
    }

    if (ReadFile(hDisk, DiskBuf1, sizeof(DiskBuf1), &Bytes,NULL) == FALSE)
    {
        fclose(fin);
        CloseHandle(hDisk);

        printf("Can not read old PBR\n");
        return -1;
    }
    if (fread(DiskBuf2, sizeof(DiskBuf2), 1, fin) != 1)
    {
        fclose(fin);
        CloseHandle(hDisk);
        printf("Can not read new PBR\n");
        return -1;
    }
    memcpy(DiskBuf1, DiskBuf2, 3);
    memcpy(DiskBuf1 + 0x5A, DiskBuf2 + 0x5A, sizeof(DiskBuf1) - 0x5A);
    SetFilePointer(hDisk, 0, NULL, FILE_BEGIN);

    if (WriteFile(hDisk, DiskBuf1, sizeof(DiskBuf1), &Bytes, NULL) == FALSE)
    {
        fclose(fin);
        CloseHandle(hDisk);
        printf("Can not write new PBR\n");
        return -1;
    }
    fclose(fin);
    CloseHandle(hDisk);
    return 0;
}