// SPDX-License-Identifier: BSD-2-Clause-Patent
// THOR: framebuffer console + direct UniXNU menu launcher.
// BDS/variable chain doesn't work (Boot#### never stored), so this driver
// launches the UniXNU application directly after installing the console.
#include <PiDxe.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/Bds.h>

#define FB_BASE   0xB8000000ULL
#define FB_WIDTH  1440
#define FONT_SCALE 3
#define CH_W      (8 * FONT_SCALE)
#define CH_H      (16 * FONT_SCALE)
#define CHARS_PER_LINE  (FB_WIDTH / CH_W)
#define MAX_LINES       50

static UINTN CursorX = 0;
static UINTN CursorY = 0;
static UINTN FgColor = 0xFFFFFF;
static UINTN BgColor = 0x000000;

/* Working font (verified on device). */
static const UINT8 Font[128][16] = {
  [32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  [33] = {0,0x18,0x18,0x18,0x18,0x18,0x18,0,0,0x18,0x18,0,0,0,0,0},
  [43] = {0,0,0,0x18,0x18,0x18,0xFF,0x18,0x18,0x18,0,0,0,0,0,0},
  [45] = {0,0,0,0,0,0,0,0x3C,0,0,0,0,0,0,0,0},
  [46] = {0,0,0,0,0,0,0,0,0,0,0x18,0x18,0,0,0,0},
  [47] = {0,0x02,0x02,0x04,0x04,0x08,0x08,0x10,0x10,0x20,0x20,0x40,0x40,0,0,0},
  [48] = {0,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0,0,0},
  [49] = {0,0x18,0x28,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x3C,0,0,0},
  [50] = {0,0x3C,0x42,0x42,0x02,0x02,0x04,0x08,0x10,0x20,0x40,0x40,0x7E,0,0,0},
  [51] = {0,0x3C,0x42,0x42,0x02,0x02,0x1C,0x02,0x02,0x02,0x42,0x42,0x3C,0,0,0},
  [52] = {0,0x04,0x0C,0x14,0x24,0x44,0x44,0x84,0xFE,0x04,0x04,0x04,0x04,0,0,0},
  [53] = {0,0x7E,0x40,0x40,0x7C,0x02,0x02,0x02,0x02,0x02,0x42,0x42,0x3C,0,0,0},
  [54] = {0,0x3C,0x42,0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0,0,0},
  [55] = {0,0xFE,0x02,0x04,0x08,0x08,0x10,0x10,0x20,0x20,0x20,0x20,0x20,0,0,0},
  [56] = {0,0x3C,0x42,0x42,0x42,0x42,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0,0,0},
  [57] = {0,0x3C,0x42,0x42,0x42,0x42,0x3E,0x02,0x02,0x02,0x02,0x42,0x3C,0,0,0},
  [58] = {0,0,0,0x18,0x18,0,0,0,0x18,0x18,0,0,0,0,0,0},
  [64] = {0,0x3C,0x42,0x9A,0xAA,0xAA,0xAA,0xB2,0xAA,0xAA,0x9A,0x5A,0x42,0x3C,0,0},
  [65] = {0,0x18,0x18,0x18,0x18,0x24,0x24,0x24,0x3C,0x42,0x42,0x42,0x7E,0x42,0x42,0},
  [66] = {0,0x3C,0x42,0x42,0x42,0x3C,0x42,0x42,0x42,0x42,0x3C,0x42,0x42,0x42,0x3C,0},
  [67] = {0,0x3C,0x42,0x42,0x42,0x40,0x40,0x40,0x40,0x42,0x42,0x42,0x3C,0,0,0},
  [68] = {0,0x38,0x44,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x44,0x38,0,0,0},
  [69] = {0,0x7E,0x40,0x40,0x40,0x7C,0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0,0,0},
  [70] = {0,0x7E,0x40,0x40,0x40,0x7C,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0,0,0},
  [71] = {0,0x3C,0x42,0x42,0x42,0x40,0x40,0x4E,0x42,0x42,0x42,0x44,0x3A,0,0,0},
  [72] = {0,0x42,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0x42,0x42,0,0,0},
  [73] = {0,0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0,0,0},
  [74] = {0,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x42,0x42,0x42,0x3C,0,0,0},
  [75] = {0,0x42,0x44,0x48,0x50,0x60,0x60,0x50,0x48,0x44,0x42,0x42,0x42,0,0,0},
  [76] = {0,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0,0,0},
  [77] = {0,0x42,0x66,0x66,0x66,0x5A,0x5A,0x42,0x42,0x42,0x42,0x42,0x42,0,0,0},
  [78] = {0,0x42,0x62,0x62,0x52,0x52,0x4A,0x4A,0x46,0x46,0x42,0x42,0x42,0,0,0},
  [79] = {0,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0,0,0},
  [80] = {0,0x3C,0x42,0x42,0x42,0x42,0x3C,0x40,0x40,0x40,0x40,0x40,0x40,0,0,0},
  [81] = {0,0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x4A,0x4A,0x46,0x3E,0,0,0},
  [82] = {0,0x3C,0x42,0x42,0x42,0x42,0x3C,0x48,0x44,0x42,0x42,0x42,0x42,0,0,0},
  [83] = {0,0x3C,0x42,0x42,0x42,0x40,0x3C,0x02,0x02,0x02,0x42,0x42,0x3C,0,0,0},
  [84] = {0,0xFF,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0,0,0},
  [85] = {0,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0,0,0},
  [86] = {0,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x24,0x24,0x18,0x18,0,0,0},
  [87] = {0,0x42,0x42,0x42,0x42,0x42,0x42,0x5A,0x5A,0x66,0x66,0x66,0x42,0,0,0},
  [88] = {0,0x42,0x42,0x24,0x24,0x18,0x18,0x18,0x24,0x24,0x42,0x42,0x42,0,0,0},
  [89] = {0,0x42,0x42,0x42,0x42,0x24,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0,0,0},
  [90] = {0,0x7E,0x02,0x04,0x04,0x08,0x08,0x10,0x10,0x20,0x20,0x40,0x7E,0,0,0},
  [95] = {0,0,0,0,0,0,0,0,0,0,0,0,0xFF,0,0,0},
  [63] = {0,0x3C,0x42,0x42,0x02,0x04,0x08,0x08,0x08,0,0x08,0x08,0,0,0,0},
};

static BOOLEAN GlyphDefined (UINT16 c)
{
  /* A glyph is present only if some row has pixels. Checking a few fixed
   * rows misclassifies punctuation (., : _ / live in the lower rows) and
   * made "..." render as "???" on screen. */
  for (UINTN row = 0; row < 16; ++row) {
    if (Font[c][row] != 0) {
      return TRUE;
    }
  }

  return FALSE;
}

static void PutChar (CHAR16 Ch, UINTN X, UINTN Y)
{
  volatile UINT32 *Fb = (volatile UINT32 *)(UINTN)FB_BASE;
  const UINT8 *Glyph;
  UINT16 c = (UINT16)Ch;
  if (c >= 'a' && c <= 'z') c -= 32;
  if (c != ' ' && (c >= 128 || !GlyphDefined (c))) c = '?';
  Glyph = Font[c];
  for (UINTN row = 0; row < 16; ++row) {
    for (UINTN sy = 0; sy < FONT_SCALE; ++sy) {
      for (UINTN col = 0; col < 8; ++col) {
        UINT32 color = (Glyph[row] & (0x80 >> col)) ? (UINT32)FgColor : (UINT32)BgColor;
        for (UINTN sx = 0; sx < FONT_SCALE; ++sx) {
          Fb[(Y + row * FONT_SCALE + sy) * FB_WIDTH + X + col * FONT_SCALE + sx] = color;
        }
      }
    }
  }
}

static void ScrollIfNeeded (void)
{
  if (CursorY >= MAX_LINES) {
    volatile UINT32 *Fb = (volatile UINT32 *)(UINTN)FB_BASE;
    for (UINTN y = CH_H; y < MAX_LINES * CH_H; ++y)
      for (UINTN x = 0; x < FB_WIDTH; ++x)
        Fb[(y - CH_H) * FB_WIDTH + x] = Fb[y * FB_WIDTH + x];
    for (UINTN y = (MAX_LINES - 1) * CH_H; y < MAX_LINES * CH_H; ++y)
      for (UINTN x = 0; x < FB_WIDTH; ++x)
        Fb[y * FB_WIDTH + x] = (UINT32)BgColor;
    CursorY = MAX_LINES - 1;
  }
}

/* EGA 16-colour palette for EFI_TEXT_ATTR(fg, bg) — RGB888. */
static CONST UINT32 mEgaColors[16] = {
  0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
  0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
};

static EFI_STATUS EFIAPI FbClearScreen (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *T);

static EFI_STATUS EFIAPI FbReset (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN BOOLEAN E)
{ (void)E; return FbClearScreen (This); }

static EFI_STATUS EFIAPI FbOutputString (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN CHAR16 *S)
{
  while (*S) {
    CHAR16 c = *S++;
    if (c == L'\r') { CursorX = 0; continue; }
    if (c == L'\n') { CursorX = 0; CursorY++; ScrollIfNeeded(); continue; }
    PutChar (c, CursorX * CH_W, CursorY * CH_H);
    CursorX++;
    if (CursorX >= CHARS_PER_LINE) { CursorX = 0; CursorY++; ScrollIfNeeded(); }
  }
  WriteBackDataCacheRange ((VOID *)(UINTN)FB_BASE, FB_WIDTH * 3200 * 4);
  return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI FbTestString (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *T, IN CHAR16 *S) { return EFI_SUCCESS; }
static EFI_STATUS EFIAPI FbQueryMode (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *T, IN UINTN M, OUT UINTN *C, OUT UINTN *R) { *C = CHARS_PER_LINE; *R = MAX_LINES; return EFI_SUCCESS; }
static EFI_STATUS EFIAPI FbSetMode (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *T, IN UINTN M) { return EFI_SUCCESS; }

static EFI_STATUS EFIAPI FbSetAttribute (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *T, IN UINTN A)
{
  /* EFI_TEXT_ATTR: low nibble foreground, bits 4-6 background. The menu and
   * BDS use attributes for highlights; honour them so text is readable. */
  FgColor = mEgaColors[A & 0x0F];
  BgColor = mEgaColors[(A >> 4) & 0x07];
  return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI FbClearScreen (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *T)
{
  volatile UINT32 *Fb = (volatile UINT32 *)(UINTN)FB_BASE;
  for (UINTN i = 0; i < FB_WIDTH * 3200; ++i) {
    Fb[i] = (UINT32)BgColor;
  }

  WriteBackDataCacheRange ((VOID *)(UINTN)FB_BASE, FB_WIDTH * 3200 * 4);
  CursorX = 0;
  CursorY = 0;
  return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI FbSetCursor (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *T, IN UINTN C, IN UINTN R) { CursorX = C; CursorY = R; return EFI_SUCCESS; }
static EFI_STATUS EFIAPI FbEnableCursor (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *T, IN BOOLEAN V) { return EFI_SUCCESS; }

static EFI_SIMPLE_TEXT_OUTPUT_MODE FbMode = {1, 0, 0, 0, 0, TRUE};
static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL FbTextOut = {
  FbReset, FbOutputString, FbTestString, FbQueryMode, FbSetMode,
  FbSetAttribute, FbClearScreen, FbSetCursor, FbEnableCursor, &FbMode
};

/* SimpleTextIn stub: BDS and the UniXNU menu need gST->ConIn to not be NULL.
 * This board has no keyboard; every read returns "no key". WaitForKey is a
 * plain periodic EVT_TIMER with no notify function: such events are legal
 * WaitForEvent targets (CoreCheckEvent rejects EVT_NOTIFY_SIGNAL), and the
 * 500 ms expiry keeps any key-wait loop responsive without a keyboard. */
static EFI_STATUS EFIAPI FbInReset (
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
  IN BOOLEAN ExtendedVerification)
{ (void)This; (void)ExtendedVerification; return EFI_SUCCESS; }

static EFI_STATUS EFIAPI FbInReadKeyStroke (
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
  OUT EFI_INPUT_KEY *Key)
{ (void)This; (void)Key; return EFI_NOT_READY; }

static EFI_SIMPLE_TEXT_INPUT_PROTOCOL FbTextIn = {
  FbInReset, FbInReadKeyStroke, NULL /* WaitForKey set at runtime */
};

/* Image-load tracer: every driver the DXE dispatcher loads after us is
 * announced on screen ("NAME @base +size"). When the CPU takes an exception,
 * the on-screen ELR can be matched against these ranges to name the module.
 * (RELEASE builds carry no PDB debug dir, so names come from this GUID table,
 * matched against the FvFile node of the image's device path.) */
typedef struct {
  EFI_GUID    Guid;
  CONST CHAR8 *Name;
} THOR_IMG_NAME;

static CONST THOR_IMG_NAME mImgNames[] = {
  { { 0xD6A2CB7F, 0x6A18, 0x4E2F, { 0xB4, 0x3B, 0x99, 0x20, 0xA7, 0x33, 0x70, 0x0A } }, "DxeCore" },
  { { 0x80CF7257, 0x87AB, 0x47F9, { 0xA3, 0xFE, 0xD5, 0x0B, 0x76, 0xD8, 0x95, 0x41 } }, "PcdDxe" },
  { { 0xB601F8C4, 0x43B7, 0x4784, { 0x95, 0xB1, 0xF4, 0x22, 0x6C, 0xB4, 0x0C, 0xEE } }, "RuntimeDxe" },
  { { 0xB8D9777E, 0xD72A, 0x451F, { 0x9B, 0xDB, 0xBA, 0xFB, 0x52, 0xA6, 0x84, 0x15 } }, "ArmCpuDxe" },
  { { 0xDE371F7C, 0xDEC4, 0x4D21, { 0xAD, 0xF1, 0x59, 0x3A, 0xBC, 0xC1, 0x58, 0x82 } }, "ArmGicDxe" },
  { { 0x4C6E0267, 0xC77D, 0x410D, { 0x81, 0x00, 0x14, 0x95, 0x91, 0x1A, 0x98, 0x9D } }, "MetronomeDxe" },
  { { 0x49EA041E, 0x6752, 0x42CA, { 0xB0, 0xB1, 0x73, 0x44, 0xFE, 0x25, 0x46, 0xB7 } }, "ArmTimerDxe" },
  { { 0xF80697E9, 0x7FD6, 0x4665, { 0x86, 0x46, 0x88, 0xE3, 0x3E, 0xF7, 0x1D, 0xFC } }, "SecurityStubDxe" },
  { { 0xF099D67F, 0x71AE, 0x4C36, { 0xB2, 0xA3, 0xDC, 0xEB, 0x0E, 0xB2, 0xB7, 0xD8 } }, "WatchdogTimer" },
  { { 0xCBD2E4D5, 0x7068, 0x4FF5, { 0xB4, 0x62, 0x98, 0x22, 0xB4, 0xAD, 0x8D, 0x60 } }, "VariableRuntimeDxe" },
  { { 0x9B680FCE, 0xAD6B, 0x4F3A, { 0xB6, 0x0B, 0xF5, 0x98, 0x99, 0x00, 0x34, 0x43 } }, "DevicePathDxe" },
  { { 0xDCFD1E6D, 0x788D, 0x4FFC, { 0x8E, 0x1B, 0xCA, 0x2F, 0x75, 0x65, 0x1A, 0x92 } }, "SimpleFbDxe" },
  { { 0x42857F0A, 0x13F2, 0x4B21, { 0x8A, 0x23, 0x53, 0xD3, 0xF7, 0x14, 0xB8, 0x40 } }, "CapsuleRuntimeDxe" },
  { { 0xFCABE6A7, 0x7953, 0x4A84, { 0xB7, 0xEC, 0xD2, 0x9E, 0x89, 0xB6, 0x2E, 0x87 } }, "EmbeddedMonotonicCounter" },
  { { 0x4B28E4C7, 0xFF36, 0x4E10, { 0x93, 0xCF, 0xA8, 0x21, 0x59, 0xE7, 0x77, 0xC5 } }, "ResetSystemRuntimeDxe" },
  { { 0xB336F62D, 0x4135, 0x4A55, { 0xAE, 0x4E, 0x49, 0x71, 0xBB, 0xF0, 0x88, 0x5D } }, "RealTimeClock" },
  { { 0x51CCF399, 0x4FDF, 0x4E55, { 0xA4, 0x5B, 0xE1, 0x23, 0xF8, 0x4D, 0x45, 0x6A } }, "ConPlatformDxe" },
  { { 0x9E863906, 0xA40F, 0x4875, { 0x97, 0x7F, 0x5B, 0x93, 0xFF, 0x23, 0x7F, 0xC6 } }, "TerminalDxe" },
  { { 0x6B38F7B4, 0xAD98, 0x40E9, { 0x90, 0x93, 0xAC, 0xA2, 0xB5, 0xA2, 0x53, 0xC4 } }, "DiskIoDxe" },
  { { 0x961578FE, 0xB6B7, 0x44C3, { 0xAF, 0x35, 0x6B, 0xC7, 0x05, 0xCD, 0x2B, 0x1F } }, "Fat" },
  { { 0x75CA63AD, 0xFBAE, 0x4A06, { 0x92, 0xA0, 0xF9, 0x37, 0xC6, 0x71, 0x25, 0x1D } }, "UniXNURamDiskDxe" },
  { { 0xC8D5E6F7, 0x2222, 0x4000, { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B } }, "ThorFbConsole" },
  { { 0x6D33944A, 0xEC75, 0x4855, { 0xA5, 0x4D, 0x80, 0x9C, 0x75, 0x24, 0x1F, 0x6C } }, "BdsDxe" },
  { { 0x29B950CF, 0x586D, 0x49EA, { 0x9D, 0x71, 0xF8, 0x86, 0x45, 0xCC, 0xDA, 0xF6 } }, "UniXNUDxe" },
  { { 0x9B665204, 0xA133, 0x47A2, { 0x93, 0x96, 0x9E, 0x2B, 0x37, 0x0A, 0x4E, 0x25 } }, "UniXNU" },
  { { 0xD6112048, 0x454F, 0x4629, { 0x87, 0x0F, 0x7F, 0xCA, 0x04, 0x5E, 0x44, 0x22 } }, "DarwinBoot24" },
};

static VOID *mImgReg;
static VOID *mBdsReg;

static CONST CHAR8 *
ImageNameForHandle (
  IN EFI_HANDLE  Handle
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *Dp;
  CONST EFI_GUID            *G;
  UINTN                     i;

  if (EFI_ERROR (gBS->HandleProtocol (Handle, &gEfiLoadedImageDevicePathProtocolGuid, (VOID **)&Dp))) {
    return NULL;
  }

  while (!IsDevicePathEnd (Dp)) {
    if ((Dp->Type == MEDIA_DEVICE_PATH) && (Dp->SubType == MEDIA_PIWG_FW_FILE_DP)) {
      G = (CONST EFI_GUID *)((UINT8 *)Dp + 4);
      for (i = 0; i < (sizeof (mImgNames) / sizeof (mImgNames[0])); i++) {
        if (CompareGuid (G, &mImgNames[i].Guid)) {
          return mImgNames[i].Name;
        }
      }

      return "?";
    }

    Dp = NextDevicePathNode (Dp);
  }

  return NULL;
}

static VOID EFIAPI ImageLoadNotify (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_HANDLE                 *Handles = NULL;
  UINTN                      Count = 0;
  UINTN                      i;
  EFI_LOADED_IMAGE_PROTOCOL  *Li;
  CONST CHAR8                *Name;
  CHAR16                     Line[80];

  if (EFI_ERROR (gBS->LocateHandleBuffer (ByRegisterNotify, NULL, mImgReg, &Count, &Handles)) || (Handles == NULL)) {
    return;
  }

  for (i = 0; i < Count; i++) {
    if (!EFI_ERROR (gBS->HandleProtocol (Handles[i], &gEfiLoadedImageProtocolGuid, (VOID **)&Li)) &&
        (Li != NULL) && (Li->ImageBase != NULL))
    {
      Name = ImageNameForHandle (Handles[i]);
      UnicodeSPrintAsciiFormat (Line, sizeof (Line), "IMG %a @%lx +%lx\n\r", Name ? Name : "?", (UINT64)(UINTN)Li->ImageBase, (UINT64)Li->ImageSize);
      FbOutputString (&FbTextOut, Line);
    }
  }

  gBS->FreePool (Handles);
}

static VOID EFIAPI BdsReadyNotify (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  FbOutputString (&FbTextOut, L"BDS ARCH READY\n\r");
}

EFI_STATUS EFIAPI ThorFbConsoleEntry (
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_HANDLE ConsoleHandle = NULL;

  EFI_HANDLE InputHandle = NULL;
  EFI_EVENT Event;

  /* Install SimpleTextOut */
  gBS->InstallProtocolInterface (&ConsoleHandle, &gEfiSimpleTextOutProtocolGuid,
    EFI_NATIVE_INTERFACE, &FbTextOut);

  /* Install SimpleTextIn stub + WaitForKey waitable timer (no notify function:
   * EVT_NOTIFY_SIGNAL events are rejected by WaitForEvent and would make the
   * UniXNU menu abandon its countdown loop immediately). */
  gBS->CreateEvent (EVT_TIMER, TPL_CALLBACK, NULL, NULL, &FbTextIn.WaitForKey);
  gBS->SetTimer (FbTextIn.WaitForKey, TimerPeriodic, EFI_TIMER_PERIOD_MILLISECONDS(500));
  gBS->InstallProtocolInterface (&InputHandle, &gEfiSimpleTextInProtocolGuid,
    EFI_NATIVE_INTERFACE, &FbTextIn);

  /* Take over the system console: BDS and the UniXNU menu read gST->ConOut/ConIn.
   * Without ConIn, BDS crashes (null pointer) and the menu can't wait for keys. */
  SystemTable->ConOut = &FbTextOut;
  SystemTable->ConsoleOutHandle = ConsoleHandle;
  SystemTable->ConIn = &FbTextIn;
  SystemTable->ConsoleInHandle = InputHandle;

  FbOutputString (&FbTextOut, L"\n\r  THOR console ready\n\r");

  /* Trace every image the dispatcher loads after us, and the moment the BDS
   * arch protocol appears. The UniXNU application itself is launched by the
   * standard BDS flow from the Boot#### option UniXNUDxe registers. */
  gBS->CreateEvent (EVT_NOTIFY_SIGNAL, TPL_CALLBACK, ImageLoadNotify, NULL, &Event);
  gBS->RegisterProtocolNotify (&gEfiLoadedImageProtocolGuid, Event, &mImgReg);
  gBS->CreateEvent (EVT_NOTIFY_SIGNAL, TPL_CALLBACK, BdsReadyNotify, NULL, &Event);
  gBS->RegisterProtocolNotify (&gEfiBdsArchProtocolGuid, Event, &mBdsReg);

  FbOutputString (&FbTextOut, L"  tracer on; BDS chain next\n\r");

  return EFI_SUCCESS;
}
