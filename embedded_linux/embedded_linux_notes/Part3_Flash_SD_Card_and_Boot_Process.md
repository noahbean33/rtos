# Introduction to Embedded Linux Part 3 - Flash SD Card and Boot Process

> Source: Digi-Key Electronics - Introduction to Embedded Linux Series

---

## 1. Overview

After building a custom Linux distribution with the Yocto Project, the output image files must be flashed onto an SD card. Unlike Buildroot (which may produce a single `sdcard.img`), the STM32MP1 BSP produces multiple separate image files that must be placed on specific partitions.

---

## 2. The Linux Boot Chain

Linux on embedded devices does **not** boot from nothing — it goes through a multi-stage boot process, analogous to a multi-stage rocket launch.

### Boot Stages

| Stage | What It Does |
|-------|-------------|
| **ROM Code** | Hard-coded in the microprocessor; configures basic clocks; reads pin configuration (boot switches) to determine where to find the first-stage bootloader |
| **First-Stage Bootloader (FSBL)** | Finishes clock initialization; sets up external RAM; locates the second-stage bootloader |
| **Second-Stage Bootloader (SSBL)** | Typically U-Boot or GRUB; sets up Ethernet, reads device tree blobs (`.dtb`), locates root filesystem; launches the Linux kernel |
| **Linux Kernel** | Mounts the root filesystem; provides access to Linux user space (console) |

### Boot Switch Configuration (STM32MP157D-DK1)
- Set **BOOT0** and **BOOT2** to **ON** to boot from SD card
- These switches tell the ROM code where to find the FSBL

---

## 3. SD Card Partition Layout

The vendor documentation (ST) specifies the required partition scheme. For the STM32MP1:

| Partition | Name | Size | Type | Contents |
|-----------|------|------|------|----------|
| 1 | `fsbl1` | 256 KiB | Microsoft Basic Data (FAT) | First-Stage Bootloader (copy 1) |
| 2 | `fsbl2` | 256 KiB | Microsoft Basic Data (FAT) | First-Stage Bootloader (copy 2 — backup) |
| 3 | `fip` | 2 MiB | Microsoft Basic Data (FAT) | Second-Stage Bootloader (Firmware Image Package) |
| 4 | `bootfs` | 64 MiB | Linux filesystem (ext4) | Boot filesystem — kernel, device tree, U-Boot image, `extlinux.conf` |
| 5 | `rootfs` | Remainder | Linux filesystem (ext4) | Root filesystem |

**Key points:**
- Two copies of the FSBL are required (in case one is corrupted)
- **GPT partition table** is required (not MBR/DOS)
- `vendorfs` and `userfs` are optional and skipped for a minimal setup
- The `bootfs` partition must be marked as **legacy BIOS bootable**

---

## 4. Partitioning the SD Card with `fdisk`

### Step-by-Step

```bash
# Unmount any auto-mounted partitions first
# Identify the SD card device
lsblk

# Launch fdisk (e.g., /dev/mmcblk2)
sudo fdisk /dev/mmcblk2
```

### Inside fdisk:

1. **Delete all existing partitions:**
   ```
   d  (repeat until all partitions are deleted)
   p  (verify — should show no partitions)
   ```

2. **Set partition table to GPT:**
   ```
   g  (converts to GPT)
   p  (verify: "Disklabel type: gpt")
   ```

3. **Create Partition 1 — FSBL copy 1:**
   ```
   n → Partition 1 → default start → +256KiB
   t → type 11 (Microsoft Basic Data)
   ```

4. **Create Partition 2 — FSBL copy 2:**
   ```
   n → Partition 2 → default start → +256KiB
   t → partition 2 → type 11
   ```

5. **Create Partition 3 — FIP (SSBL):**
   ```
   n → Partition 3 → default start → +2MiB
   t → partition 3 → type 11
   ```

6. **Create Partition 4 — Boot filesystem:**
   ```
   n → Partition 4 → default start → +64MiB
   (leave as Linux filesystem type)
   ```

7. **Create Partition 5 — Root filesystem:**
   ```
   n → Partition 5 → default start → default end (use all remaining space)
   (leave as Linux filesystem type)
   ```

### Expert Mode — Labels and Boot Flag

```
x  (enter expert mode)
```

8. **Name each partition:**
   ```
   n → 1 → fsbl1
   n → 2 → fsbl2
   n → 3 → fip
   n → 4 → bootfs
   n → 5 → rootfs
   ```

9. **Mark bootfs as legacy BIOS bootable:**
   ```
   A → 4
   ```
   > Without this flag, the boot filesystem may not mount correctly.

10. **Return and write:**
    ```
    r  (return to normal mode)
    w  (write changes to disk)
    ```

---

## 5. Flashing Image Files to Partitions

### Locate the Image Files
```
tmp/deploy/images/stm32mp1/
```

### Image-to-Partition Mapping

| Partition | Image File Path |
|-----------|----------------|
| p1 (fsbl1) | `arm-trusted-firmware/tf-a-stm32mp157d-dk1-sdcard.stm32` |
| p2 (fsbl2) | Same as p1 (duplicate copy) |
| p3 (fip) | `fip/fip-stm32mp157d-dk1-trusted.bin` |
| p4 (bootfs) | `st-image-bootfs-poky-stm32mp1.ext4` |
| p5 (rootfs) | `core-image-minimal-stm32mp1.ext4` |

### Flash Commands
```bash
# FSBL copy 1
sudo dd if=arm-trusted-firmware/tf-a-stm32mp157d-dk1-sdcard.stm32 of=/dev/mmcblk2p1

# FSBL copy 2
sudo dd if=arm-trusted-firmware/tf-a-stm32mp157d-dk1-sdcard.stm32 of=/dev/mmcblk2p2

# Second-Stage Bootloader (FIP)
sudo dd if=fip/fip-stm32mp157d-dk1-trusted.bin of=/dev/mmcblk2p3

# Boot filesystem
sudo dd if=st-image-bootfs-poky-stm32mp1.ext4 of=/dev/mmcblk2p4 bs=1M

# Root filesystem
sudo dd if=core-image-minimal-stm32mp1.ext4 of=/dev/mmcblk2p5 bs=1M
```

---

## 6. Fixing the Root Filesystem Reference

### The Problem
The second-stage bootloader (U-Boot) reads `extlinux.conf` from the boot filesystem to determine where the root filesystem is located. By default, it references a **partition UUID** that was set by ST's proprietary flashing tool — which we didn't use.

### The Fix

1. **Mount the boot filesystem:**
   - Unplug and replug the SD card (or use `mount`)
   - Navigate to the mounted boot partition

2. **Edit `extlinux.conf`:**
   ```bash
   sudo vi /media/<username>/bootfs/mmc0_extlinux/extlinux.conf
   ```

3. **Find the `APPEND` line** and replace the `root=PARTUUID=...` section with:
   ```
   root=/dev/mmcblk0p5
   ```
   > Note: The SD card is seen as `mmcblk0` by the board (not `mmcblk2` like on the host).

4. **Save and unmount** before removing the SD card.

---

## 7. Booting the Board

### Serial Console Setup
```bash
picocom -b 115200 /dev/ttyACM0
```

> **Tip:** The ST-Link chip on the DK1 can power on from just the USB cable, allowing you to watch serial output as the board boots.

### Power On
1. Connect USB cable to ST-Link port first (for serial monitoring)
2. Apply power (USB-C, 5V/3A)
3. Watch the boot sequence:
   - First-stage bootloader messages
   - Second-stage bootloader messages
   - `Starting kernel...`
   - Kernel boot messages
   - **Login prompt**

### Login
- **User:** `root`
- **Password:** (none)

### Post-Boot Observations
- Basic Linux with minimal tools
- Root filesystem is very small (~1.8 MB available) — needs expansion
- `df` command works for checking disk usage
- No automatic resize script (unlike Raspberry Pi OS)

---

## 8. Key Takeaways

- **The boot chain is multi-staged:** ROM code → FSBL → SSBL (U-Boot) → Linux kernel
- **GPT partitioning** is required for STM32MP1; each partition has a specific purpose and size
- **Two copies of the FSBL** are needed for redundancy
- The `extlinux.conf` file must be updated to point to the correct root filesystem partition when flashing manually
- ST provides proprietary tools for automated flashing — recommended for production, but manual flashing teaches the underlying process
- The root filesystem is initially tiny and needs expansion for practical use (addressed in Part 4)
- Always **unmount/sync** before removing the SD card to prevent corruption
