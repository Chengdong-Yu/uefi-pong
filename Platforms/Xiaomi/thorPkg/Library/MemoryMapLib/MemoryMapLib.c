// SPDX-License-Identifier: BSD-2-Clause-Patent
// Source: original thor stock-uefiplat.cfg and runtime GIC reg properties.
// This smoke map allocates only the 124 MiB stock DXE heap.
// FD occupies the last 3 MiB of the old UEFI FD, after ABL ExitBootServices.
// Reserved mappings do not authorize arbitrary reads of secure/unallocated RAM.
#include <Library/MemoryMapLib.h>

STATIC EFI_MEMORY_REGION_DESCRIPTOR gRegions[] = {
  {"GICD", 0x17100000ULL, 0x10000ULL, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"GICR", 0x17180000ULL, 0x200000ULL, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"Low RAM reserved", 0x80000000ULL, 0x27000000ULL, NoHob, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK},
  {"FD prefix reserved", 0xA7000000ULL, 0x100000ULL, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK},
  {"UEFI FD", 0xA7100000ULL, 0x300000ULL, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
  {"Old firmware reserved", 0xA7400000ULL, 0x20D000ULL, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK},
  {"UEFI Stack", 0xA760D000ULL, 0x40000ULL, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
  {"Old heaps reserved", 0xA764D000ULL, 0x9B3000ULL, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK},
  {"Kernel and input reserved", 0xA8000000ULL, 0x10000000ULL, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK},
  {"Display Reserved", 0xB8000000ULL, 0x2B00000ULL, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_THROUGH},
  {"Other low RAM reserved", 0xBAB00000ULL, 0x3D500000ULL, NoHob, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK},
  /* THOR: high DRAM bank 0x800000000, per the vendor DT (1029 field evidence).
   * v79 Phase B fix: the original 921 MiB (0x39900000) ends at 0x839900000,
   * which crosses into the stock HYP reservation [0x830000000,0x840000000).
   * The EFI map reports the full 256 MiB of ConventionalMemory there, but
   * [0x830000000,0x839900000) = 153 MiB is HYP-reserved on thor and must
   * never be allocated. Clip Bank A to end at the HYP boundary: 0x30000000
   * = 768 MiB exactly. DarwinBoot's ReclaimPongRamPool already clips at
   * THOR_HIGH_HYP_START, but this prevents the DXE pool from ever seeing
   * those pages in the first place. B [0x840000000,0x900000000) and
   * C [0x900000000,0xa80000000) remain unverified until SMEM/FDT
   * ownership evidence is collected. */
  {"High RAM bank A", 0x800000000ULL, 0x30000000ULL, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK},
  {"DXE Heap", 0xF8000000ULL, 0x7C00000ULL, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK},
  {"Top low RAM reserved", 0xFFC00000ULL, 0x400000ULL, NoHob, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK},
};

VOID GetMemoryMap (
  OUT EFI_MEMORY_REGION_DESCRIPTOR **MemoryDescriptor,
  OUT UINT8 *MemoryDescriptorCount
  )
{
  *MemoryDescriptor = gRegions;
  *MemoryDescriptorCount = (UINT8)ARRAY_SIZE (gRegions);
}
