#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop = NULL;
  
  // 1. 定位Graphics Output Protocol
  Status = gBS->LocateProtocol(
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  (VOID**)&Gop
                );
  
 
  // 修正第23行：
if (EFI_ERROR(Status)) { // 添加缺失的右括号
    Print(L"Error locating GOP: %r\n", Status);
    return Status;
  }
  
/*
  // 2. 获取当前模式信息
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINTN SizeOfInfo;
  Status = Gop->QueryMode(Gop, Gop->Mode == NULL ? 0 : Gop->Mode->Mode, &SizeOfInfo, &Info);
  
  if (EFI_ERROR(Status)) {
    Print(L"Error querying current mode: %r\n", Status);
    return Status;
  }

  Print(L"Current mode: %dx%d\n", Info->HorizontalResolution, Info->VerticalResolution);

  // 3. 列出所有可用模式
  Print(L"Available modes:\n");
  for (UINT32 i = 0; i < Gop->Mode->MaxMode; i++) {
    Status = Gop->QueryMode(Gop, i, &SizeOfInfo, &Info);
    if (EFI_ERROR(Status)) continue;
    
    Print(L"Mode %d: %dx%d (PixelFormat: %d)\n", 
          i, 
          Info->HorizontalResolution, 
          Info->VerticalResolution,
          Info->PixelFormat);
  }*/

  // 4. 设置新分辨率（示例设置为模式0）
  UINT32 NewMode = 15; // 修改此值为目标模式号
  //Print(L"Attempting to set mode %d...\n", NewMode);
  
  Status = Gop->SetMode(Gop, NewMode);
  if (EFI_ERROR(Status)) {
    Print(L"Error setting mode: %r\n", Status);
    return Status;
  }

  Print(L"Resolution changed successfully!\n");
  
  // 等待用户输入
  return EFI_SUCCESS;
}