// SPDX-License-Identifier: BSD-2-Clause-Patent
// Early progress marks only; scanout interpretation remains a hardware probe.
#pragma once
#include <Base.h>
#include <Library/CacheMaintenanceLib.h>
STATIC inline VOID ThorSmokeStageMark (UINTN FirstRow, UINT32 Color)
{
  volatile UINT32 *Frame = (volatile UINT32 *)(UINTN)0xB8000000;
  UINTN Index;
  if (FirstRow > 16) { return; }
  for (Index = 0; Index < 1440U * 16U; ++Index) {
    Frame[FirstRow * 1440U + Index] = Color;
  }
  WriteBackDataCacheRange ((VOID *)(UINTN)(0xB8000000ULL + FirstRow * 1440U * 4U),
                          1440U * 16U * 4U);
}
