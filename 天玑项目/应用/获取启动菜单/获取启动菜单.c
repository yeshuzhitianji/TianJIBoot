#include <Uefi.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>

EFI_STATUS
EFIAPI
UefiMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable)
{
  EFI_BOOT_MANAGER_LOAD_OPTION *BootOptions;
  UINTN                        BootCount;
  UINTN                        Index;

  Print(L"\n=== UEFI Boot Manager Menu ===\n");

  // 获取启动选项列表
  BootOptions = EfiBootManagerGetLoadOptions(&BootCount, LoadOptionTypeBoot);

  // 打印启动选项
  for (Index = 0; Index < BootCount; Index++) {
    CHAR16 *Description = BootOptions[Index].Description;
    Print(L"名称：%d. %s\n", Index + 1, Description ? Description : L"Unnamed Option");
    CHAR16 *DevicePathText = ConvertDevicePathToText(BootOptions[Index].FilePath, TRUE, TRUE);
    Print(L"文件路径: %s\n", DevicePathText ? DevicePathText : L"Unknown Path");
    if (Description!=NULL) {
        FreePool(Description);
      }
    if (DevicePathText!=NULL) {
      FreePool(DevicePathText);
    }
  }

  // 执行启动
  EfiBootManagerBoot(&BootOptions[3]);

  EfiBootManagerFreeLoadOptions(BootOptions, BootCount);

  return EFI_SUCCESS;
}
