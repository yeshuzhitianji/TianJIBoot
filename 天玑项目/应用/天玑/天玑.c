#include "天玑.h"

EFI_STATUS
获取根目录(
  IN  EFI_HANDLE        ImageHandle,
  OUT EFI_FILE_PROTOCOL **RootDir
  )
{
  EFI_STATUS                      Status;
  EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FsProtocol;

  Status = gBS->HandleProtocol(
             ImageHandle,
             &gEfiLoadedImageProtocolGuid,
             (VOID**)&LoadedImage);
  if (EFI_ERROR(Status)) {
    Print(L"获取镜像协议失败: %r\n", Status);
    return Status;
  }

  Status = gBS->HandleProtocol(
             LoadedImage->DeviceHandle,
             &gEfiSimpleFileSystemProtocolGuid,
             (VOID**)&FsProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"获取文件系统协议失败: %r\n", Status);
    return Status;
  }

  Status = FsProtocol->OpenVolume(FsProtocol, RootDir);
  if (EFI_ERROR(Status)) {
    Print(L"打开根目录失败: %r\n", Status);
  }

  return Status;
}

EFI_STATUS
读取并转换Bmp(
  IN  EFI_FILE_PROTOCOL             *RootDir,
  IN  CHAR16                        *文件名,
  OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL **GopBlt,
  OUT UINTN                         *GopBltSize,
  OUT UINTN                         *PixelHeight,
  OUT UINTN                         *PixelWidth
  )
{
  EFI_STATUS          Status;
  EFI_FILE_PROTOCOL   *FileHandle = NULL;
  UINT8               *BmpData = NULL;
  BMP_IMAGE_HEADER    *BmpHeader;
  UINTN               FileSize;
  EFI_FILE_INFO       *FileInfo = NULL;
  UINTN               InfoSize;

  // 打开BMP文件
  Status = RootDir->Open(
             RootDir,
             &FileHandle,
             文件名,
             EFI_FILE_MODE_READ,
             0
             );
  if (EFI_ERROR(Status)) {
    Print(L"Error opening file %s: %r\n", 文件名, Status);
    return Status;
  }

  // 获取文件信息
  InfoSize = sizeof(EFI_FILE_INFO) + 128;
  FileInfo = AllocatePool(InfoSize);
  Status = FileHandle->GetInfo(
             FileHandle,
             &gEfiFileInfoGuid,
             &InfoSize,
             FileInfo
             );
  if (EFI_ERROR(Status)) {
    Print(L"Error getting file info: %r\n", Status);
    goto CLOSE_FILE;
  }
  FileSize = FileInfo->FileSize;

  // 读取文件数据
  BmpData = AllocatePool(FileSize);
  Status = FileHandle->Read(FileHandle, &FileSize, BmpData);
  if (EFI_ERROR(Status)) {
    Print(L"Error reading file: %r\n", Status);
    goto FREE_BUFFER;
  }

  // 验证BMP头
  BmpHeader = (BMP_IMAGE_HEADER *)BmpData;
  if (BmpHeader->CharB != 'B' || BmpHeader->CharM != 'M') {
    Print(L"Invalid BMP file\n");
    Status = EFI_INVALID_PARAMETER;
    goto FREE_BUFFER;
  }

  // 手动转换RGBA到BGRA格式
  UINT8 *PixelData = (UINT8 *)BmpData + BmpHeader->ImageOffset;
  UINT32 PixelCount = BmpHeader->PixelWidth * BmpHeader->PixelHeight;
  
  // 分配Blt缓冲区
  *GopBlt = AllocatePool(PixelCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  if (*GopBlt == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto FREE_BUFFER;
  }

  for (UINTN y = 0; y < BmpHeader->PixelHeight; y++) {
    UINTN srcY = BmpHeader->PixelHeight - 1 - y; // BMP存储是倒序的
    for (UINTN x = 0; x < BmpHeader->PixelWidth; x++) {
      UINTN srcIndex = (srcY * BmpHeader->PixelWidth + x);
      UINTN dstIndex = (y * BmpHeader->PixelWidth + x);
      
      (*GopBlt)[dstIndex].Blue     = PixelData[srcIndex*4 + 0];
      (*GopBlt)[dstIndex].Green    = PixelData[srcIndex*4 + 1];
      (*GopBlt)[dstIndex].Red      = PixelData[srcIndex*4 + 2];
      (*GopBlt)[dstIndex].Reserved = PixelData[srcIndex*4 + 3];
    }
  }

  // 设置输出参数
  *GopBltSize = PixelCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  *PixelHeight = BmpHeader->PixelHeight;
  *PixelWidth = BmpHeader->PixelWidth;

FREE_BUFFER:
  if (BmpData != NULL) {
    FreePool(BmpData);
  }
  if (FileInfo != NULL) {
    FreePool(FileInfo);
  }

CLOSE_FILE:
  if (FileHandle != NULL) {
    FileHandle->Close(FileHandle);
  }
  return Status;
}

/**
  Alpha混合两个BLT缓冲区
  @param[in]  前景图    带透明通道的前景图像数据
  @param[in]  背景图    背景图像数据
  @param[out] 混合结果  输出混合后的图像
  @param[in]  像素总数  需要处理的像素数量
  @retval EFI_SUCCESS     混合成功
  @retval EFI_INVALID_PARAMETER 无效参数
**/
EFI_STATUS
混合Blt(
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *前景图,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *背景图,
  OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *混合结果,
  IN  UINTN                          像素总数
  )
{
  // 参数有效性检查
  if (前景图 == NULL || 背景图 == NULL || 混合结果 == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // 逐像素进行Alpha混合
  for (UINTN 索引 = 0; 索引 < 像素总数; 索引++) {
    UINT8 透明度 = 前景图[索引].Reserved;  // 使用Reserved字段作为Alpha通道
    
    // 混合公式：结果 = (前景 * 透明度 + 背景 * (255 - 透明度)) / 255
    混合结果[索引].Red   = (前景图[索引].Red   * 透明度 + 背景图[索引].Red   * (255 - 透明度)) / 255;
    混合结果[索引].Green = (前景图[索引].Green * 透明度 + 背景图[索引].Green * (255 - 透明度)) / 255;
    混合结果[索引].Blue  = (前景图[索引].Blue  * 透明度 + 背景图[索引].Blue  * (255 - 透明度)) / 255;
    混合结果[索引].Reserved = 0;  // 清空透明通道
  }
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
启动OpenCore (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *FileSystem,
  IN  EFI_HANDLE                       DeviceHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL         *LoaderDevicePath
  )
{
  EFI_STATUS                Status;
  VOID                      *Buffer;
  UINTN                     LoaderPathSize;
  UINTN                     PrefixPathSize;
  UINTN                     RootPathLength;
  CHAR16                    *LoaderPath;
  UINT32                    BufferSize;
  EFI_DEVICE_PATH_PROTOCOL  *ImagePath;

  ASSERT (FileSystem != NULL);

  Buffer     = NULL;
  BufferSize = 0;

  LoaderPath = OcCopyDevicePathFullName (LoaderDevicePath, NULL);

  ImagePath = NULL;

  //
  // Try relative path: EFI\\XXX\\Subdir\\Launcher.efi -> EFI\\XXX\\OpenCore.efi
  //
  if (LoaderPath != NULL) {
    LoaderPathSize = StrSize (LoaderPath);
    if (  UnicodeGetParentDirectory (LoaderPath)
       && UnicodeGetParentDirectory (LoaderPath))
    {
      DEBUG ((DEBUG_INFO, "BS: Relative path - %s\n", LoaderPath));
      PrefixPathSize = StrSize (LoaderPath);
      if (LoaderPathSize - PrefixPathSize >= L_STR_SIZE (OPEN_CORE_APP_PATH)) {
        RootPathLength             = PrefixPathSize / sizeof (CHAR16) - 1;
        LoaderPath[RootPathLength] = '\\';
        CopyMem (&LoaderPath[RootPathLength + 1], OPEN_CORE_APP_PATH, L_STR_SIZE (OPEN_CORE_APP_PATH));
        Buffer = OcReadFile (FileSystem, LoaderPath, &BufferSize, BASE_16MB);
        DEBUG ((DEBUG_INFO, "BS: Startup path - %s (%p)\n", LoaderPath, Buffer));

        if (Buffer != NULL) {
          ImagePath = FileDevicePath (DeviceHandle, LoaderPath);
          if (ImagePath == NULL) {
            DEBUG ((DEBUG_INFO, "BS: File DP allocation failure, aborting\n"));
            FreePool (Buffer);
            Buffer = NULL;
          }
        }
      }
    }

    FreePool (LoaderPath);
  }

  //
  // Try absolute path: EFI\\BOOT\\BOOTx64.efi -> EFI\\OC\\OpenCore.efi
  //
  if (Buffer == NULL) {
    DEBUG ((
      DEBUG_INFO,
      "BS: Fallback to absolute path - %s\n",
      OPEN_CORE_ROOT_PATH L"\\" OPEN_CORE_APP_PATH
      ));

    Buffer = OcReadFile (
               FileSystem,
               OPEN_CORE_ROOT_PATH L"\\" OPEN_CORE_APP_PATH,
               &BufferSize,
               BASE_16MB
               );
    if (Buffer != NULL) {
      //
      // Failure to allocate this one is not too critical, as we will still be able
      // to choose it as a default path.
      //
      ImagePath = FileDevicePath (DeviceHandle, OPEN_CORE_ROOT_PATH L"\\" OPEN_CORE_APP_PATH);
    }
  }

  if (Buffer == NULL) {
    ASSERT (ImagePath == NULL);
    DEBUG ((DEBUG_ERROR, "BS: Failed to locate valid OpenCore image - %p!\n", Buffer));
    return EFI_NOT_FOUND;
  }

  DEBUG ((DEBUG_INFO, "BS: Read OpenCore image of %Lu bytes\n", (UINT64)BufferSize));

  //
  // Run OpenCore image
  //
  Status = OcLoadAndRunImage (
             ImagePath,
             Buffer,
             BufferSize,
             NULL,
             NULL
             );

  DEBUG ((DEBUG_ERROR, "BS: Failed to start OpenCore image - %r\n", Status));
  FreePool (Buffer);
  FreePool (ImagePath);

  return Status;
}

EFI_STATUS
运行EFI程序(
  IN EFI_FILE_PROTOCOL    *RootDir,
  IN EFI_HANDLE           ImageHandle,
  IN CHAR16               *文件名
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
  Status = RootDir->Open(RootDir, &FileHandle, 文件名, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    Print(L"打开文件 %s 失败: %r\n", 文件名, Status);
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
/**
 安全转换UINTN到CHAR16字符串
 **/
CHAR16* UintnToChar16(UINTN Number)
{
    CHAR16* Buffer = AllocatePool(21 * sizeof(CHAR16)); // 64位最大值长度+终止符
    if (!Buffer) return NULL;
    
    if (UnicodeValueToStringS(Buffer,  sizeof(CHAR16), 10, Number, 0) <= 0) {
        FreePool(Buffer);
        return NULL;
    }
    return Buffer;
}

/**
 生成随机背景文件名
 格式：\\EFI\\TianJI\\Image\\Background\\背景[1-44].图片
**/
CHAR16* 获取随机背景文件名(VOID)
{
    EFI_TIME 系统时间;
    UINTN 随机种子 = 0;
    
    // 获取系统时间作为种子
    if (!EFI_ERROR(gRT->GetTime(&系统时间, NULL))) {
        随机种子 += 系统时间.Year + 系统时间.Month + 系统时间.Day 
                + 系统时间.Hour + 系统时间.Minute + 系统时间.Second;
    }
    
    // 生成1-44随机数并计算数字位数
    UINTN 随机数 = (随机种子 % 44) + 1;
    UINTN 数字位数 = (随机数 < 10) ? 1 : 2;  // 动态计算数字位数
    
    // 动态计算各部分长度
    STATIC CHAR16 *前缀 = L"\\EFI\\TianJI\\Image\\Background\\背景";
    STATIC CHAR16 *后缀 = L".图片";
    UINTN 前缀长度 = StrLen(前缀);      // 33字符
    UINTN 后缀长度 = StrLen(后缀);      // 4字符
    
    // 总字符数 = 前缀33 + 数字1~2 + 后缀4 + 终止符1
    UINTN 总字符数 = 前缀长度 + 数字位数 + 后缀长度 + 1;
    
    CHAR16* 文件名 = AllocatePool(总字符数 * sizeof(CHAR16));
    if (!文件名) return NULL;
    
    // 安全构造路径
    UnicodeSPrint(文件名, 
                总字符数 * sizeof(CHAR16), 
                L"%s%u%s",  // 自动处理数字位数
                前缀, 随机数, 后缀);
    
    return 文件名;
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                  Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  UINTN                   启动项数 = 0;
  UINTN                   选择启动项 = 0;
  启动项信息                *启动项 = NULL;
  EFI_FILE_PROTOCOL           *根目录;
  图片坐标 背景坐标={0,0};
  图片坐标 指针坐标={0,0};
  图片坐标 图标坐标={0,0};
  图像信息 背景 = {0};
  图像信息 指针 = {L"\\EFI\\TianJI\\Image\\指针.图片",NULL,0,0,0};
  图像信息 图标尺寸 = {L"\\EFI\\TianJI\\Image\\Windows.图片",NULL,0,0,0};
  // 关闭看门狗定时器
  Status = gBS->SetWatchdogTimer(0, 0, 0, NULL);
  if (EFI_ERROR(Status)) {
    Print(L"关闭看门狗失败: %r\n", Status);
    return Status;
  }
  // 获取根目录
  Status = 获取根目录(ImageHandle, &根目录);
  if (EFI_ERROR(Status)) {
    goto CLOSE_FILE;
  }
  // 获取图形输出协议
  Status = gBS->LocateProtocol(
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  (VOID**)&Gop
                  );
  if (EFI_ERROR(Status)) {
    Print(L"Error getting GOP: %r\n", Status);
    return Status;
  }
  CHAR16* 随机背景文件名 = 获取随机背景文件名();
  if (!随机背景文件名) {
      Print(L"生成随机文件名失败\n");
      goto FREE_BUFFER;
      return EFI_OUT_OF_RESOURCES;
  }
  // 读取并转换背景
  Status = 读取并转换Bmp(
           根目录,
           随机背景文件名,
           &背景.图像数据,
           &背景.图像大小,
           &背景.高度,
           &背景.宽度
           );
  if (EFI_ERROR(Status)) {
    goto FREE_BUFFER;
    return Status;
  }
  
  背景坐标.X = (Gop->Mode->Info->HorizontalResolution-背景.宽度)/ 2;
  //Print(L"背景坐标X: %d\n", 背景坐标.X);
  背景坐标.Y = (Gop->Mode->Info->VerticalResolution-背景.高度)/ 2;
  //Print(L"背景坐标Y: %d\n", 背景坐标.Y);
  //绘制背景图片
  Status = Gop->Blt(
    Gop,
    背景.图像数据,
    EfiBltBufferToVideo,
    0,
    0,
    背景坐标.X,
    背景坐标.Y,
    背景.宽度,
    背景.高度,
    背景.宽度 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
    );
  
  //获取启动项
  EFI_BOOT_MANAGER_LOAD_OPTION *BootOptions;
  UINTN BootCount;
  BootOptions = EfiBootManagerGetLoadOptions(&BootCount, LoadOptionTypeBoot);
  UINTN Index;
  UINTN i = 0;
  if (BootCount>0) {
    启动项 = AllocatePool(BootCount * sizeof(启动项信息));
  }
  if (启动项 == NULL) {
    goto FREE_BUFFER;
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < BootCount; Index++) {
    CHAR16 *DevicePathText = ConvertDevicePathToText(BootOptions[Index].FilePath, TRUE, TRUE);
    
    if (DevicePathText != NULL) {
      if (StrStr(DevicePathText, L"Microsoft") != NULL) {
        启动项[i].启动选项 = BootOptions[Index];
        启动项[i].启动项名称 = L"Windows";
        //Print(L"Windows: %s\n", DevicePathText);
        i++;
      } 
      if (StrStr(DevicePathText, L"ubuntu") != NULL) {
        启动项[i].启动选项 = BootOptions[Index];
        启动项[i].启动项名称 = L"Ubuntu";
        //Print(L"Ubuntu: %s\n", DevicePathText);
        i++;
      } 
      if (StrStr(DevicePathText, L"OC") != NULL) {
        启动项[i].启动选项 = BootOptions[Index];
        启动项[i].启动项名称 = L"Apple";
        //Print(L"Apple: %s\n", DevicePathText);
        i++;
      } 
      if (StrStr(DevicePathText, L"fydeos") != NULL) {
        启动项[i].启动选项 = BootOptions[Index];
        启动项[i].启动项名称 = L"FydeOS";
        //Print(L"FydeOS: %s\n", DevicePathText);
        i++;
      }
      FreePool(DevicePathText);
    }
  }
  启动项数 = i;
  //Print(L"找到启动项数量: %u\n", 启动项数);
  Status = 读取并转换Bmp(
    根目录,
    图标尺寸.文件名,
    &图标尺寸.图像数据,
    &图标尺寸.图像大小,
    &图标尺寸.高度,
    &图标尺寸.宽度
  );
  if (EFI_ERROR(Status)) {
    goto FREE_BUFFER;
    return Status;
  }
  // 计算图片排列位置
  UINTN 图片总宽度 = (图标尺寸.宽度 * 启动项数) + (50 * (启动项数 - 1));
  UINTN 起始X = (Gop->Mode->Info->HorizontalResolution - 图片总宽度) / 2;
  图标坐标.X = 起始X;
  图标坐标.Y = Gop->Mode->Info->VerticalResolution-图标尺寸.高度-90;

  for (UINTN i = 0; i < 启动项数; i++) {
    图像信息 图标 = {0};
    // 根据启动项名称构造图片路径
    if (StrStr(启动项[i].启动项名称, L"Windows")!= NULL) {
      图标.文件名 = L"\\EFI\\TianJI\\Image\\Windows.图片";
    }
    else if (StrStr(启动项[i].启动项名称, L"Ubuntu")!= NULL) {
      图标.文件名 = L"\\EFI\\TianJI\\Image\\Ubuntu.图片";
    }
    else if (StrStr(启动项[i].启动项名称, L"Apple")!= NULL) {
      图标.文件名 = L"\\EFI\\TianJI\\Image\\Apple.图片";
    }
    if (StrStr(启动项[i].启动项名称, L"FydeOS")!= NULL) {
      图标.文件名 = L"\\EFI\\TianJI\\Image\\FydeOS.图片";
    }

    // 加载图标
    Status = 读取并转换Bmp(
      根目录,
      图标.文件名,
      &图标.图像数据,
      &图标.图像大小,
      &图标.高度,
      &图标.宽度
    );
    if (EFI_ERROR(Status)) {
      Print(L"加载[%s]图标失败: %r\n", 启动项[i].启动项名称, Status);
      continue;
    }
    UINTN 图标像素总数 = 图标.宽度 * 图标.高度;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *图标背景图 = AllocatePool(图标像素总数 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *图标混合结果 = AllocatePool(图标像素总数 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    if (!EFI_ERROR(Status)) {
      Gop->Blt(
        Gop,
        图标背景图,
        EfiBltVideoToBltBuffer,
        图标坐标.X,
        图标坐标.Y,
        0, 
        0,
        图标.宽度,
        图标.高度,
        0
      );
      Status = 混合Blt(图标.图像数据, 图标背景图, 图标混合结果, 图标像素总数);
      if (EFI_ERROR(Status)) {
          Print(L"图像混合失败: %r\n", Status);
      }
      // 绘制图标
      Gop->Blt(
        Gop,
        图标混合结果,
        EfiBltBufferToVideo,
        0,
        0,
        图标坐标.X,
        图标坐标.Y,
        图标.宽度,
        图标.高度,
        图标.宽度 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
      );
      
      // 绘制完成后释放内存
      FreePool(图标.图像数据);
      FreePool(图标背景图);
      FreePool(图标混合结果);
    }

    图标坐标.X += 图标尺寸.宽度 + 50;  // 每个图标间隔50像素
  }
  
  Status = 读取并转换Bmp(
           根目录,
           指针.文件名,
           &指针.图像数据,
           &指针.图像大小,
           &指针.高度,
           &指针.宽度
           );
  if (EFI_ERROR(Status)) {
    goto FREE_BUFFER;
    return Status;
  }
  UINTN 指针像素总数 = 指针.宽度 * 指针.高度;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *指针背景图 = AllocatePool(指针像素总数 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *指针混合结果 = AllocatePool(指针像素总数 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    
  UINTN 最左边X = (Gop->Mode->Info->HorizontalResolution - 图片总宽度 + 图标尺寸.宽度 - 指针.宽度)/ 2;
  UINTN 最右边X = 最左边X + (图标尺寸.宽度 + 50) * (启动项数 - 1);
  指针坐标.X = 最左边X;
  //Print(L"指针坐标X: %d\n", 指针坐标.X);
  指针坐标.Y = Gop->Mode->Info->VerticalResolution-70;
  //Print(L"指针坐标Y: %d\n", 指针坐标.Y);
  Gop->Blt(
    Gop,
    指针背景图,
    EfiBltVideoToBltBuffer,
    指针坐标.X,
    指针坐标.Y,
    0, 
    0,
    指针.宽度,
    指针.高度,
    0
  );
  Status = 混合Blt(指针.图像数据, 指针背景图, 指针混合结果, 指针像素总数);
  if (EFI_ERROR(Status)) {
      Print(L"图像混合失败: %r\n", Status);
  }
  // 绘制图标
  Gop->Blt(
    Gop,
    指针混合结果,
    EfiBltBufferToVideo,
    0,
    0,
    指针坐标.X,
    指针坐标.Y,
    指针.宽度,
    指针.高度,
    指针.宽度 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
  );
  //运行EFI程序(根目录, ImageHandle, L"\\EFI\\TianJI\\截屏.efi");
  // 初始化等待列表
  BOOLEAN 绘制;
  EFI_EVENT WaitList[] = { gST->ConIn->WaitForKey };
  while (1) {
        UINTN Index;
        // 等待键盘输入
        Status = gBS->WaitForEvent(1, WaitList, &Index);
        if (EFI_ERROR(Status)) break;

        // 读取按键
        EFI_INPUT_KEY Key;
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
        //Print(L"按键扫描码:0x%04X  Unicode字符:0x%04X\n", Key.ScanCode, Key.UnicodeChar);
        if (EFI_ERROR(Status)) break;
        // 处理控制指令
        switch (Key.ScanCode) {
            case SCAN_LEFT:  
            绘制 = TRUE;
            //清屏
            Gop->Blt(
              Gop,
              指针背景图,
              EfiBltBufferToVideo,
              0,
              0,
              指针坐标.X,
              指针坐标.Y,
              指针.宽度,
              指针.高度,
              指针.宽度 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
              );
            指针坐标.X -= (图标尺寸.宽度 + 50); 
            if(选择启动项 == 0){选择启动项 = 启动项数-1;}
            else{选择启动项--;}
            break;
            case SCAN_RIGHT: 
            绘制 = TRUE;
            //清屏
            Gop->Blt(
              Gop,
              指针背景图,
              EfiBltBufferToVideo,
              0,
              0,
              指针坐标.X,
              指针坐标.Y,
              指针.宽度,
              指针.高度,
              指针.宽度 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
              );
            指针坐标.X += (图标尺寸.宽度 + 50);
            if (选择启动项 == 启动项数-1) {选择启动项 = 0;}
            else {选择启动项++;}
            break;
            default:  
            绘制 = FALSE;
            break;
        }
        if(指针坐标.X < 最左边X) {
          指针坐标.X = 最右边X;
        }
        if(指针坐标.X > 最右边X) {
          指针坐标.X = 最左边X;
        }
        if(绘制){
            Gop->Blt(
              Gop,
              指针背景图,
              EfiBltVideoToBltBuffer,
              指针坐标.X,
              指针坐标.Y,
              0, 
              0,
              指针.宽度,
              指针.高度,
              0
            );
            Status = 混合Blt(指针.图像数据, 指针背景图, 指针混合结果, 指针像素总数);
            if (EFI_ERROR(Status)) {
                Print(L"图像混合失败: %r\n", Status);
            }
            // 绘制图标
            Gop->Blt(
              Gop,
              指针混合结果,
              EfiBltBufferToVideo,
              0,
              0,
              指针坐标.X,
              指针坐标.Y,
              指针.宽度,
              指针.高度,
              指针.宽度 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
            );
        }
        switch (Key.UnicodeChar)
        {
            //回车字符是UnicodeChar=0x000D
            case CHAR_CARRIAGE_RETURN: 
            // 执行选中的启动项
            if(启动项[选择启动项].启动项名称 != L"Apple"){
              EfiBootManagerBoot(&启动项[选择启动项].启动选项);
              }
            else{
              EFI_LOADED_IMAGE_PROTOCOL        *LoadedImage;
              EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *FileSystem;
              LoadedImage = NULL;
              Status = gBS->HandleProtocol (
                                  ImageHandle,
                                  &gEfiLoadedImageProtocolGuid,
                                  (VOID **)&LoadedImage
                                  );

              if (EFI_ERROR (Status)) {
                return EFI_NOT_FOUND;
              }
              FileSystem = OcLocateFileSystem (
                            LoadedImage->DeviceHandle,
                            LoadedImage->FilePath
                            );

              if (FileSystem == NULL) {
                return EFI_NOT_FOUND;
              }
              启动OpenCore (FileSystem, LoadedImage->DeviceHandle, LoadedImage->FilePath);
            }
            break;
            default:break;
        }
  }
  
FREE_BUFFER:
  if (随机背景文件名!= NULL) {
    FreePool(随机背景文件名);
  }
  if (BootOptions!= NULL) {
    EfiBootManagerFreeLoadOptions(BootOptions, BootCount);
  }
  if (启动项!= NULL) {
    FreePool(启动项);
  }
  if (背景.图像数据 != NULL) {
    FreePool(背景.图像数据);
  }
  if (图标尺寸.图像数据!= NULL) {
    FreePool(图标尺寸.图像数据);
  }
  if (指针.图像数据 != NULL) {
    FreePool(指针.图像数据);
  }
  if (指针背景图!= NULL) {
    FreePool(指针背景图);
  }
  if (指针混合结果!= NULL) {
    FreePool(指针混合结果);
  }
CLOSE_FILE:
  根目录->Close(根目录);
  return Status;
}
