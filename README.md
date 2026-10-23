# Project Silicium UEFI for the Nothing Phone (2)

UEFI firmware source for Nothing Phone (2), codename **Pong**, SoC **SM8475**. This repository keeps the Mu-Silicium UEFI tree and the Pong board package used by UniXNU.

## Description

The Pong platform is ARM64 and uses the Qualcomm SM8475 configuration, a 1080 × 2412 framebuffer, and the Pong device tree. The board files live under `Platforms/Nothing/pongPkg`. The UniXNU and DarwinBoot application/driver integration is included in the Pong DSC/FDF configuration.

This repository contains firmware source and build scripts. Apple restore assets and generated firmware/boot images are not checked in. The companion [UniXNU-pong device source](https://github.com/Chengdong-Yu/UniXNU-pong) fetches matching iOS 16.6.1 / 20G81 components when required.

## Resources

- [Building](Building.md)
- [Pong status](Status.md)
- [UniXNU-pong](https://github.com/Chengdong-Yu/UniXNU-pong)
- [UniXNU documentation](https://unixnu.com)

## Frequently Asked Questions

### Is Pong supported?

The Pong board configuration and UEFI image packaging are included. The device has reached the UniXNU menu and hands off to DarwinBoot in development runs. UEFI build/boot support does not by itself guarantee that every operating system or hardware feature works.

### How do I build the UEFI firmware?

Clone this repository and its submodules, then place it beside the UniXNU core checkout. For the UniXNU-integrated Pong firmware, run:

```sh
git clone --recurse-submodules https://github.com/Chengdong-Yu/uefi-pong
git clone --recurse-submodules https://github.com/UniXNU/UniXNU
cd uefi-pong
python3 -m pip install -r pip-requirements.txt
./setup_env.sh -p apt
bash scripts/build-firmware.sh
```

`build-firmware.sh` uses `UNIXNU_ROOT` if the core checkout is elsewhere. To build the base UEFI target without the UniXNU DarwinBoot integration, use the standard Mu-Silicium command:

```sh
python3 -m pip install -r pip-requirements.txt
python3 build_uefi.py -d pong
```

`scripts/build-bootimg.py` packages the firmware volume and Pong DTB in an Android boot image v4 container. It does not write to a device.

## Credits

This source tree is based on [Project Silicium / Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium), Project Mu, and the Qualcomm and ARM platform packages referenced by the submodules. Pong hardware configuration is based on the publicly available Mu-Silicium Pong package and Nothing SM8475 device-tree sources.

## License

Mu-Silicium is licensed under BSD-2-Clause-Patent, with the upstream GPL exceptions documented in the license. Individual source files retain their SPDX headers.
