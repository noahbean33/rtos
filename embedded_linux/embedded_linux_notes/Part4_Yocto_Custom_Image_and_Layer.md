# Introduction to Embedded Linux Part 4 - Yocto Custom Image and Layer

> Source: Digi-Key Electronics - Introduction to Embedded Linux Series

---

## 1. Overview

Creating a **custom layer** in Yocto is the primary way to:
- Modify variables in other layers without editing them directly
- Install custom drivers and packages
- Change login/password settings
- Include custom applications (explored in Part 6)
- Control root filesystem size

The goal in this episode: create a custom layer, build a custom image with an expanded root filesystem, and set a default root password.

---

## 2. Understanding Poky's Image Build

### Image Recipe Hierarchy

1. **`image.bbclass`** (`meta/classes/image.bbclass`)
   - Master recipe/template for constructing images
   - Defines variables and functions (do_compile, do_build, etc.)
   - **Never modify files inside the Poky directory**

2. **`core-image.bbclass`**
   - Inherits `image.bbclass`
   - Defines the `IMAGE_INSTALL` variable (soft assignment with `?=`)
   - Includes `packagegroup-core-boot` (minimum packages to boot Linux + BusyBox) and `packagegroup-base-extended`

3. **`core-image-minimal.bb`** (`recipes-core/images/core-image-minimal.bb`)
   - Overwrites the soft assignment — uses only `packagegroup-core-boot` plus `${CORE_IMAGE_EXTRA_INSTALL}`
   - This is the base we copy to create our custom image

### Variable Assignment Types in Yocto

| Syntax | Meaning |
|--------|---------|
| `VAR ?= "value"` | **Soft assignment** — sets a default; can be overridden by another `?=` or `=` |
| `VAR = "value"` | **Hard assignment** — always overwrites; last `=` wins during parsing |
| `VAR += "value"` | **Append** — adds to the variable with a space delimiter |

---

## 3. Creating a Custom Layer

### Initialize the Build Environment
```bash
cd ~/projects/yocto
source poky/oe-init-build-env build-mp1
```

### Create the Layer with BitBake
```bash
bitbake-layers create-layer ../meta-custom
```

This generates a directory structure at `~/projects/yocto/meta-custom/`:
```
meta-custom/
├── conf/
│   └── layer.conf
├── recipes-example/
│   └── example/
│       └── example_0.1.bb
├── COPYING.MIT
└── README
```

### Key Points About Custom Layers
- **Keep layers separate** from the build directory — the build dir is local/temporary
- **Put layers under version control** (Git) — this is what you maintain and share
- **Branch naming** should match Yocto releases (e.g., `dunfell`)
- The example recipe is just a template; safe to remove

---

## 4. Creating a Custom Image Recipe

### Copy the Base Image
```bash
mkdir -p meta-custom/recipes-core/images/
cp poky/meta/recipes-core/images/core-image-minimal.bb \
   meta-custom/recipes-core/images/custom-image.bb
```

### Edit `custom-image.bb`

```bitbake
SUMMARY = "Custom Linux image for STM32MP1"

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"

inherit core-image
inherit extrausers

# Expand root filesystem to ~200 MB
IMAGE_ROOTFS_SIZE ?= "204800"
IMAGE_OVERHEAD_FACTOR = "1"

# Set default root password to "toor"
EXTRA_USERS_PARAMS = "\
    usermod -P toor root; \
"
```

### Explanation of Changes

- **`IMAGE_ROOTFS_SIZE`**: Sets the root filesystem size in KB. Default was ~8 MB (only ~1 MB usable). Set to `204800` (~200 MB).
  - Soft assigned (`?=`) so it can be overridden in `local.conf`
- **`IMAGE_OVERHEAD_FACTOR`**: Set to `1` to prevent Yocto from adding extra overhead beyond the specified size
- **`IMAGE_ROOTFS_EXTRA_SPACE`**: Removed (not needed with explicit size)
- **`inherit extrausers`**: Imports the `extrausers.bbclass` which provides `EXTRA_USERS_PARAMS`
- **`EXTRA_USERS_PARAMS`**: Uses `usermod -P toor root` to set root password
  - **Capital `-P`** = plain text password (Yocto-specific, not standard Linux `usermod`)
  - Backslashes allow multi-line variable assignments
  - **Security warning:** Plain text passwords in recipes are visible in source control — not suitable for production!

### Production Password Strategies
- Assign a unique password per device (e.g., based on serial number or device ID)
- Print credentials on a physical sticker
- Use a setup script that forces password change on first boot

---

## 5. Adding the Layer to the Build

### Edit `conf/bblayers.conf`
```bash
vi conf/bblayers.conf
```
Add your custom layer path to the `BBLAYERS` variable:
```
/home/<user>/projects/yocto/meta-custom \
```

---

## 6. Fixing the `debug-tweaks` Conflict

### The Problem
By default, `local.conf` contains:
```
EXTRA_IMAGE_FEATURES ?= "debug-tweaks"
```
`debug-tweaks` allows root login **without a password**. This **conflicts** with setting a default password — the result is you can't log in at all.

### Diagnose
```bash
bitbake -e | grep IMAGE_FEATURES
```

### Fix — Edit `conf/local.conf`
Comment out the debug-tweaks line:
```bash
#EXTRA_IMAGE_FEATURES ?= "debug-tweaks"
```

### Verify
```bash
bitbake -e | grep IMAGE_FEATURES
# Should now be empty
```

> **Note:** Remove `debug-tweaks` before production deployment anyway — it's only for development.

---

## 7. Building the Custom Image

```bash
bitbake custom-image
```

- Much faster than the initial build (~15 minutes) since only the changed parts are rebuilt
- Output is in `tmp/deploy/images/stm32mp1/`

### Verify Output
- New image files are named `custom-image-*` alongside the old `core-image-minimal-*` files
- Check the **manifest file** (`custom-image-*.manifest`) to see all installed packages
- BusyBox provides most user-space tools (vi, cat, ls, etc.)

---

## 8. Flashing the Custom Image

### Flash Root Filesystem
```bash
sudo dd if=custom-image-stm32mp1.ext4 of=/dev/mmcblk2p5 bs=1M
```

### Flash Boot Filesystem (good habit even if unchanged)
```bash
sudo dd if=st-image-bootfs-poky-stm32mp1.ext4 of=/dev/mmcblk2p4 bs=1M
```

### Fix `extlinux.conf` (required after every boot filesystem flash)
```bash
sudo vi /media/<username>/bootfs/mmc0_extlinux/extlinux.conf
# Change root= to: /dev/mmcblk0p5
```

### Unmount and Sync
```bash
sudo umount /media/<username>/bootfs
sudo umount /media/<username>/rootfs
```

---

## 9. Booting and Verifying

### Serial Console
```bash
picocom -b 115200 /dev/ttyACM0
```

### Login
- **User:** `root`
- **Password:** `toor`

### Verify Root Filesystem Size
```bash
df -h
```
Should show ~200 MB root filesystem (not exact due to build process calculations).

### Observations
- Ethernet module auto-loads from the BSP (useful, but adds boot time)
- To optimize boot time: remove unnecessary modules (Ethernet, USB, etc.)
- Fewer modules = faster boot + smaller attack surface

### Proper Shutdown
```bash
shutdown now
```
Always shut down properly to prevent filesystem corruption.

---

## 10. Key Takeaways

- **Never modify Poky or vendor layers directly** — create your own custom layer
- Custom layers should be **version-controlled** and maintained separately from build directories
- **Variable assignment order matters** — hard assignments (`=`) override soft assignments (`?=`); `+=` appends
- The `extrausers` class enables user/password management in recipes
- `debug-tweaks` must be removed when setting custom passwords (they conflict)
- **Yocto's incremental builds** make iteration fast — only changed layers are rebuilt
- Root filesystem size is controlled via `IMAGE_ROOTFS_SIZE` and `IMAGE_OVERHEAD_FACTOR`
- Always update `extlinux.conf` after flashing a new boot filesystem
- Use `shutdown now` to safely power off the board
