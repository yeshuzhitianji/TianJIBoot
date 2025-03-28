#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>

EFI_STATUS
RunEfiApplication(
  IN EFI_FILE_PROTOCOL    *RootDir,
  IN EFI_HANDLE           ImageHandle,
  IN CHAR16               *FileName
  )
{
  EFI_STATUS      Status;
  EFI_FILE_PROTOCOL *FileHandle;
  UINTN           FileSize;
  VOID            *SourceBuffer;
  EFI_HANDLE      NewImageHandle;
  EFI_FILE_INFO   *FileInfo;
  UINTN           InfoSize;

  // 打开目标EFI文件
  Status = RootDir->Open(RootDir, &FileHandle, FileName, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    Print(L"打开文件 %s 失败: %r\n", FileName, Status);
    RootDir->Close(RootDir);
    return Status;
  }

  // 获取文件信息
  InfoSize = sizeof(EFI_FILE_INFO) + 128;
  FileInfo = AllocatePool(InfoSize);
  Status = FileHandle->GetInfo(FileHandle, &gEfiFileInfoGuid, &InfoSize, FileInfo);
  if (EFI_ERROR(Status)) {
    Print(L"获取文件信息失败: %r\n", Status);
    goto CLOSE_FILE;
  }
  FileSize = FileInfo->FileSize;
  FreePool(FileInfo);

  // 读取文件内容
  SourceBuffer = AllocatePool(FileSize);
  Status = FileHandle->Read(FileHandle, &FileSize, SourceBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"读取文件失败: %r\n", Status);
    goto FREE_BUFFER;
  }

  // 加载并执行EFI镜像
  Status = gBS->LoadImage(
           FALSE,
           ImageHandle,
           NULL,
           SourceBuffer,
           FileSize,
           &NewImageHandle);
  if (EFI_ERROR(Status)) {
    Print(L"加载镜像失败: %r\n", Status);
    goto FREE_BUFFER;
  }

  Status = gBS->StartImage(NewImageHandle, NULL, NULL);
  if (EFI_ERROR(Status)) {
    Print(L"执行失败: %r\n", Status);
    gBS->UnloadImage(NewImageHandle);
  }

FREE_BUFFER:
  FreePool(SourceBuffer);
CLOSE_FILE:
  FileHandle->Close(FileHandle);
  return Status;
}

EFI_STATUS EFIAPI UefiMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS                  Status;
  EFI_LOADED_IMAGE_PROTOCOL   *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FsProtocol;
  EFI_FILE_PROTOCOL           *RootDir;
  CHAR16                      *FileName = L"\\EFI\\TianJI\\天玑终端.efi";

  // 获取当前镜像信息
  Status = gBS->HandleProtocol(
             ImageHandle,
             &gEfiLoadedImageProtocolGuid,
             (VOID**)&LoadedImage);
  if (EFI_ERROR(Status)) {
    Print(L"获取镜像协议失败: %r\n", Status);
    return Status;
  }

  // 获取文件系统协议
  Status = gBS->HandleProtocol(
             LoadedImage->DeviceHandle,
             &gEfiSimpleFileSystemProtocolGuid,
             (VOID**)&FsProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"获取文件系统协议失败: %r\n", Status);
    return Status;
  }

  // 打开根目录
  Status = FsProtocol->OpenVolume(FsProtocol, &RootDir);
  if (EFI_ERROR(Status)) {
    Print(L"打开根目录失败: %r\n", Status);
    return Status;
  }

  // 执行目标应用程序
  Status = RunEfiApplication(RootDir, ImageHandle, FileName);
  
  RootDir->Close(RootDir);
  return Status;
}