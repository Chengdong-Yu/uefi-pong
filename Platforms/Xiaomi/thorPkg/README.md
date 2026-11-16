# Xiaomi 12S Ultra UEFI

This is the temporary-boot firmware platform for the Xiaomi 12S Ultra (Thor,
SM8475). It configures the Thor memory map, framebuffer console and FAT payload
path. It selects the Thor DarwinBoot24 driver to hand the device tree and
owned RAM to the Darwin 24 kernel.

## Build inputs

Use the Thor variant from the `pong-darwin-boot24` component revision
`831afc41df166491813aac26251ecd2ab21fa182`. Supply that component checkout
in `PACKAGES_PATH`, alongside the firmware workspace, `Platforms/Xiaomi`,
Mu Basecore, Mu Plus, Mu OEM Sample, `Silicon/Silicium` and
`Silicon/Qualcomm`.

The Thor FAT discovery and BDS code modifies six files in the UniXNU firmware
package. Stage them over the existing vendored package and put the staging
directory in `PACKAGES_PATH` before any other `UniXNUPkg` copy.

```sh
mkdir -p Build/thor-packages
cp -R Platforms/Nothing/UniXNUPkg Build/thor-packages/
git apply --directory=Build/thor-packages Platforms/Xiaomi/thorPkg/UniXNUPkg.patch
```

For the host build of this source, use the v137 Mu revisions below.

| Package | Revision |
| --- | --- |
| Mu Basecore | `b44fcc6d615b9de806d8a4938b9806ddbd47aa54` |
| Mu Plus | `566cf5c8c2c8adf8e1a337ced1d147bc6246856f` |
| Mu OEM Sample | `9292f9254adb94c5e5bb09a370b9662fcd66a953` |

With the EDK II BaseTools installed and `PACKAGES_PATH` set, run from the workspace.

```sh
build -a AARCH64 -b RELEASE -t CLANGPDB -p Platforms/Xiaomi/thorPkg/thor.dsc \
  -D ENABLE_SECUREBOOT=0 -D FD_BASE=0xA7100000 -D FD_SIZE=0x300000 \
  -D FD_BLOCKS=0x300 -D MEMORY_PROTECTION=TRUE -D SHIP_MODE=FALSE \
  -D BUILDID_STRING=UniXNU-Thor
```

The 3 MiB firmware volume is written to
`Build/thorPkg/RELEASE_CLANGPDB/FV/SILICIUM_UEFI.fd`. The UniXNU Thor device
repository supplies the boot image layout, device tree and OS profile.
