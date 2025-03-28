#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/GraphicsOutput.h>
#include <Guid/FileInfo.h>
#include <IndustryStandard/Bmp.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BmpSupportLib.h>

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  EFI_STATUS Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Sfsp;
  EFI_FILE_PROTOCOL *RootDir;
  EFI_FILE_PROTOCOL *BmpFile;
  UINTN FileSize;
  VOID *BmpData;

  // 初始化文件协议
  Status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (VOID**)&Sfsp);
  if (EFI_ERROR(Status)) {
    Print(L"无法获取文件系统协议: %r\n", Status);
    return Status;
  }

  Status = Sfsp->OpenVolume(Sfsp, &RootDir);
  if (EFI_ERROR(Status)) {
    Print(L"无法打开根目录: %r\n", Status);
    return Status;
  }

  // 打开BMP文件
  Status = RootDir->Open(RootDir, &BmpFile, L"Apple.bmp", EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    Print(L"无法打开BMP文件: %r\n", Status);
    RootDir->Close(RootDir);
    return Status;
  }

  // 获取文件大小
  UINTN          InfoSize;
  EFI_FILE_INFO *FileInfo;
  InfoSize = sizeof(EFI_FILE_INFO) + 128;
  FileInfo = AllocatePool(InfoSize);
  Status = BmpFile->GetInfo(BmpFile, &gEfiFileInfoGuid, &InfoSize, FileInfo);
  if (EFI_ERROR(Status)) {
    Print(L"获取文件信息失败: %r\n", Status);
    BmpFile->Close(BmpFile);
    RootDir->Close(RootDir);
    return Status;
  }
  FileSize = FileInfo->FileSize;
  FreePool(FileInfo);

  // 分配内存并读取文件
  BmpData = AllocatePool(FileSize);
  Status = BmpFile->Read(BmpFile, &FileSize, BmpData);
  if (EFI_ERROR(Status)) {
    Print(L"文件读取失败: %r\n", Status);
    FreePool(BmpData);
    BmpFile->Close(BmpFile);
    RootDir->Close(RootDir);
    return Status;
  }

  // 解析BMP头结构
  BMP_IMAGE_HEADER *BmpHeader = (BMP_IMAGE_HEADER *)BmpData;
  if (BmpHeader->CharB != 'B' || BmpHeader->CharM != 'M') {
    Print(L"无效的BMP文件头\n");
    FreePool(BmpData);
    BmpFile->Close(BmpFile);
    RootDir->Close(RootDir);
    return EFI_INVALID_PARAMETER;
  }

  Print(L"成功读取32位BMP文件\n");
  Print(L"图像尺寸: %dx%d\n", BmpHeader->PixelWidth, BmpHeader->PixelHeight);
  Print(L"位深度: %d\n", BmpHeader->BitPerPixel);

  // 获取图形输出协议
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
  if (EFI_ERROR(Status)) {
    Print(L"获取图形输出协议失败: %r\n", Status);
    FreePool(BmpData);
    BmpFile->Close(BmpFile);
    RootDir->Close(RootDir);
    return Status;
  }

  // 转换32位BMP像素数据
  UINT8 *PixelData = (UINT8 *)BmpData + BmpHeader->ImageOffset;
  UINT32 PixelCount = BmpHeader->PixelWidth * BmpHeader->PixelHeight;

  // 分配Blt缓冲区
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer;
  BltBuffer = AllocatePool(PixelCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

  // 手动转换RGBA到BGRA格式
  for (UINTN y = 0; y < BmpHeader->PixelHeight; y++) {
    UINTN srcY = BmpHeader->PixelHeight - 1 - y;
    for (UINTN x = 0; x < BmpHeader->PixelWidth; x++) {
      UINTN srcIndex = (srcY * BmpHeader->PixelWidth + x);
      UINTN dstIndex = (y * BmpHeader->PixelWidth + x);
      
      BltBuffer[dstIndex].Blue     = PixelData[srcIndex*4 + 0];
      BltBuffer[dstIndex].Green    = PixelData[srcIndex*4 + 1];
      BltBuffer[dstIndex].Red      = PixelData[srcIndex*4 + 2];
      BltBuffer[dstIndex].Reserved = PixelData[srcIndex*4 + 3];
    }
  }

  // 输出转换结果
  Print(L"成功转换 %d 个像素\n", PixelCount);
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *背景;
  背景 = AllocatePool(PixelCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *混合结果;
  混合结果 = AllocatePool(PixelCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  Gop->Blt(
    Gop,
    背景,
    EfiBltVideoToBltBuffer,
    BmpHeader->PixelWidth, 0,
    0, 0,
    BmpHeader->PixelWidth,
    BmpHeader->PixelHeight,
    0
  );
  for(UINTN i = 0; i < PixelCount; i++){
    混合结果[i].Red = (背景[i].Red * (255-BltBuffer[i].Reserved) + BltBuffer[i].Red * BltBuffer[i].Reserved)/255;
    混合结果[i].Green = (背景[i].Green * (255-BltBuffer[i].Reserved) + BltBuffer[i].Green * BltBuffer[i].Reserved)/255;
    混合结果[i].Blue = (背景[i].Blue * (255-BltBuffer[i].Reserved) + BltBuffer[i].Blue * BltBuffer[i].Reserved)/255;
    混合结果[i].Reserved = 0;
  }
  // 将Blt缓冲区输出到屏幕
  Gop->Blt(
    Gop,
    混合结果,
    EfiBltBufferToVideo,
    0, 0,
    BmpHeader->PixelWidth, 0,
    BmpHeader->PixelWidth,
    BmpHeader->PixelHeight,
    0
  );


  if (EFI_ERROR(Status)) {
    Print(L"图像输出失败: %r\n", Status);
  } else {
    Print(L"图像显示成功\n");
  }

  FreePool(BltBuffer);
  FreePool(背景);
  FreePool(混合结果);
  FreePool(BmpData);
  BmpFile->Close(BmpFile);
  RootDir->Close(RootDir);
  return EFI_SUCCESS;
}