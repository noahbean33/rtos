# Introduction to Embedded Linux Part 5 - Patch Device Tree for I2C in Yocto

> Source: Digi-Key Electronics - Introduction to Embedded Linux Series

---

## 1. Overview

This episode demonstrates how to:
- Enable an I2C port on the STM32MP157D-DK1 by **patching the device tree**
- Create a patch file and include it in the custom Yocto layer
- Verify the I2C port is accessible from Linux user space
- Use `i2cdetect` to probe for connected devices

**Target hardware:** TMP102 temperature sensor connected via I2C5 on the Raspberry Pi-style header.

---

## 2. Understanding Device Trees

### What Is a Device Tree?
- A **configuration file** that tells the Linux kernel which hardware peripherals are present, what drivers to use, and how to communicate with them
- Written in a C-like syntax as **Device Tree Source (`.dts`)** files
- Compiled into **Device Tree Blobs (`.dtb`)** that are loaded at boot
- Uses `#include`-style directives to pull in **Device Tree Source Include (`.dtsi`)** files

### Key Files for STM32MP1
| File | Purpose |
|------|---------|
| `stm32mp157d-dk1.dts` | Main device tree source for the DK1 board |
| `stm32mp15xx-dkx.dtsi` | Shared include for DK-series boards; defines I2C nodes (often disabled by default) |
| `stm32mp15-pinctrl.dtsi` | Pin control definitions (~2000 lines); maps I2C buses to physical pins |

---

## 3. I2C Port Selection

### STM32MP1 I2C Bus Availability

The MP1 chip has **6 I2C buses**. On the DK1 board:

| I2C Port | Status | Usage |
|----------|--------|-------|
| I2C1 | In use | LCD touchscreen, power management IC |
| I2C4 | In use | USB Type-C connector |
| I2C5 | **Available** | Exposed on RPi header (pins 3 & 5) |
| Others | Varies | Check datasheet |

### I2C5 Pin Mapping (from `stm32mp15-pinctrl.dtsi`)
- **Pins A (selected):** PA11 (SCL), PA12 (SDA) — maps to RPi header pins 5 and 3
- **Pins B (alternate):** PD0, PD1

### Physical Wiring (TMP102 Sensor)
| TMP102 Pin | Board Connection |
|------------|-----------------|
| VIN | 3.3V |
| SDA | Pin 3 (PA12) |
| SCL | Pin 5 (PA11) |
| GND | GND |

---

## 4. Linux I2C Device Mapping

### User-Space Device Files
```bash
ls -l /dev/i2c-*
```
Lists available I2C device files (e.g., `/dev/i2c-0`, `/dev/i2c-1`, `/dev/i2c-2`).

> **Important:** Linux device numbers do NOT necessarily match hardware bus numbers!

### Identifying Physical Ports
```bash
ls /sys/bus/i2c/devices/
```
Each entry points to a memory address. Cross-reference with the **MP1 Reference Manual** (Section 2 — Memory Organization):

| Memory Address | Physical I2C Port | Linux Device |
|---------------|-------------------|-------------|
| `0x40012000` | I2C1 | `/dev/i2c-0` |
| `0x5C002000` | I2C4 | `/dev/i2c-2` |
| `0x40015000` | I2C5 (after enabling) | `/dev/i2c-1` |

---

## 5. Modifying the Device Tree

### Approach
- Do **NOT** modify the `.dts` file directly in the Yocto work directory (changes are temporary and Yocto will reject them)
- Instead, create a **patch file** in the custom layer that modifies the `.dts` before compilation

### Locate the Original DTS File
The working copy is at:
```
tmp/work-shared/stm32mp1/kernel-source/arch/arm/boot/dts/stm32mp157d-dk1.dts
```

### Create the Patch

1. **Copy the original file to a temporary location:**
   ```bash
   cp <path-to-dts>/stm32mp157d-dk1.dts ~/Documents/
   cp ~/Documents/stm32mp157d-dk1.dts ~/Documents/stm32mp157d-dk1.dts.orig
   ```

2. **Edit the copy** — add the I2C5 node at the end of the root node:
   ```dts
   &i2c5 {
       pinctrl-names = "default", "sleep";
       pinctrl-0 = <&i2c5_pins_a>;
       pinctrl-1 = <&i2c5_pins_sleep_a>;
       i2c-scl-rising-time-ns = <185>;
       i2c-scl-falling-time-ns = <20>;
       clock-frequency = <100000>;
       status = "okay";
   };
   ```

   Key changes from the disabled default:
   - **`clock-frequency`**: Set to 100 kHz (safe default; TMP102 supports 400 kHz)
   - **`status`**: Changed from `"disabled"` to `"okay"` — this enables the port
   - Uses **pins_a** and **sleep_pins_a** to map I2C5 to PA11/PA12

3. **Generate the patch with `git diff`:**
   ```bash
   git diff --no-index \
       ~/Documents/stm32mp157d-dk1.dts.orig \
       ~/Documents/stm32mp157d-dk1.dts \
       > ~/Documents/0001-add-i2c5-userspace-dts.patch
   ```
   The `--no-index` flag allows diffing files not tracked by Git.

4. **Fix the file paths in the patch:**
   The patch must reference the file's location relative to the `arch/` directory in the kernel source:
   ```
   --- a/arch/arm/boot/dts/stm32mp157d-dk1.dts
   +++ b/arch/arm/boot/dts/stm32mp157d-dk1.dts
   ```
   Edit the patch to replace the temporary file paths with these correct relative paths.

---

## 6. Adding the Patch to the Custom Layer

### Directory Structure
```bash
mkdir -p meta-custom/recipes-kernel/linux/stm32mp1/
cp ~/Documents/0001-add-i2c5-userspace-dts.patch \
   meta-custom/recipes-kernel/linux/stm32mp1/
```

### Find the Kernel Name
```bash
oe-pkgdata-util lookup-recipe virtual/kernel
# Output: linux-stm32mp
```

### Create the `.bbappend` Recipe
Create `meta-custom/recipes-kernel/linux/linux-stm32mp_%.bbappend`:

```bitbake
# Add this directory to the file search path
FILESEXTRAPATHS_prepend := "${THISDIR}/stm32mp1:"

# Include the patch file
SRC_URI += "file://0001-add-i2c5-userspace-dts.patch"
```

**Explanation:**
- **`FILESEXTRAPATHS_prepend`**: Adds the `stm32mp1/` subdirectory to the front of BitBake's file search path
- **`SRC_URI +=`**: Appends the patch file to the source URI list; `file://` means local file
- **`_%`**: Wildcard version — applies to any kernel version (use a specific version for tighter control)
- The `.bbappend` suffix tells BitBake to append this recipe to the matching `linux-stm32mp` kernel recipe

---

## 7. Enabling `i2cdetect` in BusyBox

```bash
bitbake -c menuconfig busybox
```

1. Navigate to **Miscellaneous Utilities**
2. Find and enable **`i2cdetect`** (space bar to toggle)
3. Exit and **save configuration**

> **Note:** This change is local to the build directory. To make it permanent, save it as a defconfig in your custom layer (covered in Part 7).

---

## 8. Building and Flashing

### Build
```bash
bitbake custom-image
```
This rebuild takes ~1 hour since the kernel must be recompiled with the new device tree.

### Flash (same process as previous episodes)
```bash
# Boot filesystem (partition 4)
sudo dd if=st-image-bootfs-poky-stm32mp1.ext4 of=/dev/mmcblk2p4 bs=1M

# Root filesystem (partition 5)
sudo dd if=custom-image-stm32mp1.ext4 of=/dev/mmcblk2p5 bs=1M
```

### Fix `extlinux.conf` (required every time bootfs is reflashed)
```bash
sudo vi /media/<username>/bootfs/mmc0_extlinux/extlinux.conf
# Change root= to: /dev/mmcblk0p5
```

---

## 9. Verification

### Check I2C Devices
```bash
ls /sys/bus/i2c/devices/
```
Should now show three I2C ports including the one at `0x40015000` (I2C5).

```bash
ls /dev/i2c-*
```
I2C5 maps to `/dev/i2c-1`.

### Probe for the TMP102 Sensor
```bash
i2cdetect -y 1
```
- Should show address **`0x48`** when the TMP102 is connected
- Removing power/connections makes the address disappear, confirming the detection works

---

## 10. Important Notes on I2C in Linux

- **I2C is not ideal for Linux user-space applications** — for production, write a proper kernel device driver
- User-space I2C uses file I/O control (`ioctl`), which has limitations:
  - Cannot hold lines high between write-then-read sequences as some devices require
  - Timing is not guaranteed
- **Simple devices like the TMP102 work fine** because their protocol is straightforward
- Complex I2C devices may require kernel-level drivers for reliable communication

---

## 11. Key Takeaways

- **Device trees** define hardware configuration for the Linux kernel — enable/disable peripherals by changing `status` between `"disabled"` and `"okay"`
- **Never modify files in the Yocto work directory directly** — use patches in your custom layer
- **Patch workflow:** Copy DTS → modify → `git diff --no-index` → fix paths → add to layer via `.bbappend`
- **`.bbappend` files** extend existing recipes without modifying them
- **Linux I2C device numbers don't match hardware bus numbers** — use `/sys/bus/i2c/devices/` and the reference manual to find the correct mapping
- `i2cdetect` is a useful debugging tool but must be explicitly enabled in BusyBox
