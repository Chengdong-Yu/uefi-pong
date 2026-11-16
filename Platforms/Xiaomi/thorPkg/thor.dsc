## @file
# Temporary boot target for Xiaomi 12S Ultra (thor / SM8475).
# SPDX-License-Identifier: BSD-2-Clause-Patent
# FAT payload and bounded firmware memory policy for the Thor boot chain.
[Defines]
  PLATFORM_NAME = thor
  PLATFORM_GUID = 60618FE3-8207-418B-85D7-AE991CE750B8
  PLATFORM_VERSION = 0.1
  DSC_SPECIFICATION = 0x00010005
  OUTPUT_DIRECTORY = Build/thorPkg
  SUPPORTED_ARCHITECTURES = AARCH64
  BUILD_TARGETS = RELEASE
  SKUID_IDENTIFIER = DEFAULT
  FLASH_DEFINITION = thorPkg/thor.fdf
  USE_CUSTOM_DISPLAY_DRIVER = 0
  USE_PHYSICAL_TIMER = 0

!include thorPkg/Include/MinimalCommon.dsc.inc

[BuildOptions]
  *_CLANGPDB_AARCH64_CC_FLAGS = -march=armv9-a+crypto+rcpc

[PcdsFixedAtBuild]
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0xA760D000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x40000
  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|3
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x17100000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x17180000
  gArmTokenSpaceGuid.PcdMonitorConduitHvc|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvStoreReserved|0
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemManufacturer|"Xiaomi"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemModel|"12S Ultra (thor)"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"thor"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"2203121C"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemBoardModel|"thor"
  # Panel timing is confirmed; actual bootloader scanout stride/format is a probe input.
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferWidth|1440
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferHeight|3200
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferColorDepth|32

[PcdsFeatureFlag]
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportProcessCapsuleAtRuntime|FALSE

[LibraryClasses]
  MemoryMapLib|thorPkg/Library/MemoryMapLib/MemoryMapLib.inf
  PlatformSecLib|thorPkg/Library/PlatformSecLib/PlatformSecLib.inf
  PlatformPeiLib|thorPkg/Library/PlatformPeiLib/PlatformPeiLib.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  UniXNULib|UniXNUPkg/Library/UniXNULib/UniXNULib.inf

[Components]
  SiliciumPkg/Sec/Sec.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }
!include thorPkg/Include/Components.inc
