[Defines]
  PLATFORM_NAME                  = 天玑项目
  PLATFORM_GUID                  = 0458dade-8b6e-4e45-b773-1b27cbda3e06
  PLATFORM_VERSION               = 0.01
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/天玑项目
  SUPPORTED_ARCHITECTURES        = IA32|X64|ARM|AARCH64|LOONGARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

  DEFINE DEBUG_ENABLE_OUTPUT      = FALSE       # Set to TRUE to enable debug output
  DEFINE DEBUG_PRINT_ERROR_LEVEL  = 0x80000040  # Flags to control amount of debug output
  DEFINE DEBUG_PROPERTY_MASK      = 0
!ifndef CUSTOM_STACK_CHECK_LIB
  DEFINE CUSTOM_STACK_CHECK_LIB = NONE
!endif


[PcdsFeatureFlag]

[PcdsFixedAtBuild]
  gEfi天玑项目TokenSpaceGuid.PcdDebugPropertyMask|$(DEBUG_PROPERTY_MASK)
  gEfi天玑项目TokenSpaceGuid.PcdDebugPrintErrorLevel|$(DEBUG_PRINT_ERROR_LEVEL)

[LibraryClasses]
  OrderedCollectionLib|天玑项目/库/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  ArmTrngLib|天玑项目/库/BaseArmTrngLibNull/BaseArmTrngLibNull.inf
  RegisterFilterLib|天玑项目/库/RegisterFilterLibNull/RegisterFilterLibNull.inf
  CpuLib|天玑项目/库/BaseCpuLib/BaseCpuLib.inf
  SmmCpuRendezvousLib|天玑项目/库/SmmCpuRendezvousLibNull/SmmCpuRendezvousLibNull.inf
  SafeIntLib|天玑项目/库/BaseSafeIntLib/BaseSafeIntLib.inf
  SynchronizationLib|天玑项目/库/BaseSynchronizationLib/BaseSynchronizationLib.inf
  MmUnblockMemoryLib|天玑项目/库/MmUnblockMemoryLib/MmUnblockMemoryLibNull.inf
  StackCheckFailureHookLib|天玑项目/库/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf

!if $(CUSTOM_STACK_CHECK_LIB) == STATIC
  StackCheckLib|天玑项目/库/StackCheckLib/StackCheckLib.inf
!elseif $(CUSTOM_STACK_CHECK_LIB) == DYNAMIC
  StackCheckLib|天玑项目/库/StackCheckLib/StackCheckLib.inf
  DxeCoreEntryPoint|天玑项目/库/DynamicStackCookieEntryPointLib/DxeCoreEntryPoint.inf
  StandaloneMmDriverEntryPoint|天玑项目/库/DynamicStackCookieEntryPointLib/StandaloneMmDriverEntryPoint.inf
  UefiApplicationEntryPoint|天玑项目/库/DynamicStackCookieEntryPointLib/UefiApplicationEntryPoint.inf
  UefiDriverEntryPoint|天玑项目/库/DynamicStackCookieEntryPointLib/UefiDriverEntryPoint.inf

!else
  StackCheckLib|天玑项目/库/StackCheckLibNull/StackCheckLibNull.inf
!endif

  UefiApplicationEntryPoint|天玑项目/库/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  ShellCEntryLib|天玑项目/库/UefiShellCEntryLib/UefiShellCEntryLib.inf
  UefiDriverEntryPoint|天玑项目/库/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  
  OcDevicePathLib|天玑项目/库/OcDevicePathLib/OcDevicePathLib.inf
  OcFileLib|天玑项目/库/OcFileLib/OcFileLib.inf
  OcMiscLib|天玑项目/库/OcMiscLib/OcMiscLib.inf
  OcStringLib|天玑项目/库/OcStringLib/OcStringLib.inf
  BaseOverflowLib|天玑项目/库/BaseOverflowLib/BaseOverflowLib.inf

  BmpSupportLib|天玑项目/库/BaseBmpSupportLib/BaseBmpSupportLib.inf
  UefiBootManagerLib|天玑项目/库/UefiBootManagerLib/UefiBootManagerLib.inf
  BaseLib|天玑项目/库/BaseLib/BaseLib.inf
  BaseMemoryLib|天玑项目/库/BaseMemoryLib/BaseMemoryLib.inf
  UefiLib|天玑项目/库/UefiLib/UefiLib.inf
  PrintLib|天玑项目/库/BasePrintLib/BasePrintLib.inf
  PcdLib|天玑项目/库/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|天玑项目/库/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|天玑项目/库/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DxeServicesTableLib|天玑项目/库/DxeServicesTableLib/DxeServicesTableLib.inf
  DxeServicesLib|天玑项目/库/DxeServicesLib/DxeServicesLib.inf
  ReportStatusCodeLib|天玑项目/库/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  VariablePolicyHelperLib|天玑项目/库/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  UefiRuntimeServicesTableLib|天玑项目/库/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  !if $(DEBUG_ENABLE_OUTPUT)
    DebugLib|天玑项目/库/UefiDebugLibConOut/UefiDebugLibConOut.inf
    DebugPrintErrorLevelLib|天玑项目/库/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  !else   ## DEBUG_ENABLE_OUTPUT
    DebugLib|天玑项目/库/BaseDebugLibNull/BaseDebugLibNull.inf
  !endif  ## DEBUG_ENABLE_OUTPUT

  DevicePathLib|天玑项目/库/UefiDevicePathLib/UefiDevicePathLib.inf
  PeCoffGetEntryPointLib|天玑项目/库/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  IoLib|天玑项目/库/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  PciLib|天玑项目/库/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|天玑项目/库/BasePciCf8Lib/BasePciCf8Lib.inf
  SynchronizationLib|天玑项目/库/BaseSynchronizationLib/BaseSynchronizationLib.inf
  UefiRuntimeLib|天玑项目/库/UefiRuntimeLib/UefiRuntimeLib.inf
  HiiLib|天玑项目/库/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|天玑项目/库/UefiHiiServicesLib/UefiHiiServicesLib.inf
  PerformanceLib|天玑项目/库/DxePerformanceLib/DxePerformanceLib.inf
  HobLib|天玑项目/库/DxeHobLib/DxeHobLib.inf
  FileHandleLib|天玑项目/库/UefiFileHandleLib/UefiFileHandleLib.inf
  SortLib|天玑项目/库/UefiSortLib/UefiSortLib.inf

  ShellLib|天玑项目/库/UefiShellLib/UefiShellLib.inf

  CacheMaintenanceLib|天玑项目/库/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf

[LibraryClasses.common.SEC, LibraryClasses.common.PEI_CORE]
  StackCheckLib|天玑项目/库/StackCheckLibNull/StackCheckLibNull.inf

[LibraryClasses.ARM, LibraryClasses.AARCH64]
  NULL|天玑项目/库/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf

[Components]
  天玑项目/应用/天玑/天玑.inf
  天玑项目/应用/截屏/截屏.inf
  天玑项目/应用/获取启动菜单/获取启动菜单.inf
  天玑项目/应用/启动EFI程序/启动EFI程序.inf
  天玑项目/应用/设置分辨率/设置分辨率.inf
  天玑项目/应用/显示32位Bmp图片/显示32位Bmp图片.inf