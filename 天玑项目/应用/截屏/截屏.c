#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/BmpSupportLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Protocol/LoadedImage.h>
#include <Library/DevicePathLib.h>

EFI_STATUS EFIAPI UefiMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable)
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *Gop;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltImage;
  UINTN                       BltSize;
  VOID                         *Bmp;
  UINT32                       BmpSize;
  CHAR16                        *FileName = L"截屏.图片";
  EFI_FILE_PROTOCOL             *File;

  // 获取图形输出协议
  Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
  if (EFI_ERROR(Status)) {
    //Print(L"无法获取图形输出协议: %r\n", Status);
    return Status;
  }

  // 分配缓冲区存储屏幕数据
  BltSize = Gop->Mode->Info->HorizontalResolution * Gop->Mode->Info->VerticalResolution * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  BltImage = AllocatePool(BltSize);
  if (BltImage == NULL) {
    Print(L"内存分配失败\n");
    return EFI_OUT_OF_RESOURCES;
  }

  // 获取屏幕数据
  Status = Gop->Blt(
    Gop,
    BltImage,
    EfiBltVideoToBltBuffer,
    0,
    0,
    0,
    0,
    Gop->Mode->Info->HorizontalResolution,
    Gop->Mode->Info->VerticalResolution,
    0
  );
  if (EFI_ERROR(Status)) {
    Print(L"屏幕数据获取失败: %r\n", Status);
    FreePool(BltImage);
    return Status;
  }
  // 转换为BMP格式
  Status = TranslateGopBltToBmp(
    BltImage,
    Gop->Mode->Info->VerticalResolution,
    Gop->Mode->Info->HorizontalResolution,
    &Bmp,
    &BmpSize
  );
  if (EFI_ERROR(Status)) {
    Print(L"BMP转换失败: %r\n", Status);
    FreePool(BltImage);
    FreePool(Bmp);
    return Status;
  }


  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Sfs;
  Status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (VOID**)&Sfs);
  if (EFI_ERROR(Status)) {
    Print(L"找不到文件系统协议: %r\n", Status);
    FreePool(BltImage);
    FreePool(Bmp);
    return Status;
  }
  // 打开根目录
  EFI_FILE_PROTOCOL *Root;
  Status = Sfs->OpenVolume(Sfs, &Root);
  if (EFI_ERROR(Status)) {
    Print(L"卷打开失败: %r\n", Status);
    Root->Close(Root);
    FreePool(BltImage);
    FreePool(Bmp);
    return Status;
  }
  UINTN FileSize = BmpSize;
  // 创建并写入文件
  Status = Root->Open(
    Root,
    &File,
    FileName,
    EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_ARCHIVE
  );
  if (EFI_ERROR(Status)) {
    Print(L"文件创建失败: %r\n", Status);
    File->Close(File);
    Root->Close(Root);
    FreePool(BltImage);
    FreePool(Bmp);
    return Status;
  }

  Status = File->Write(File, &FileSize, Bmp);
  FreePool(BltImage);
  FreePool(Bmp);
  File->Close(File);
  Root->Close(Root);
  if (EFI_ERROR(Status)) {
    Print(L"文件写入失败: %r\n", Status);
    return Status;
  }

  Print(L"截屏已保存为%s\n", FileName);
  return EFI_SUCCESS;
}