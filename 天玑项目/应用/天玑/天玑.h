#ifndef __天玑_H__
#define __天玑_H__
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BmpSupportLib.h>
#include <IndustryStandard/Bmp.h>
#include <Guid/FileInfo.h>
#include <Library/PrintLib.h>  
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadedImage.h>
#include <Library/OcDevicePathLib.h>
#include <Library/OcDebugLogLib.h>
#include <Library/OcFileLib.h>
#include <Library/OcStringLib.h>
#include <Library/OcMainLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimpleTextIn.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
typedef struct {
  CHAR16                        *文件名;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *图像数据;
  UINTN                         图像大小;
  UINTN                         高度;
  UINTN                         宽度;
} 图像信息;
typedef struct {
  UINTN                         X;
  UINTN                         Y;
} 图片坐标;
typedef struct 
{
  EFI_BOOT_MANAGER_LOAD_OPTION  启动选项;
  CHAR16                        *启动项名称;
}启动项信息;

#endif