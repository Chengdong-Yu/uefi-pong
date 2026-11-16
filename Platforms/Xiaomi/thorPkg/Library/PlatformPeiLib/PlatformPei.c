// SPDX-License-Identifier: BSD-2-Clause-Patent
#include <Library/PcdLib.h>
#include <Library/HobLib.h>
EFI_STATUS EFIAPI PlatformPeim (VOID)
{
  BuildFvHob (PcdGet64 (PcdFvBaseAddress), PcdGet32 (PcdFvSize));
  return EFI_SUCCESS;
}
