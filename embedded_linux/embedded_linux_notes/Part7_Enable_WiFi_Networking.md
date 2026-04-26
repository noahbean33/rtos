# Introduction to Embedded Linux Part 7 - Enable WiFi Networking with Yocto

> Source: Digi-Key Electronics - Introduction to Embedded Linux Series

---

## 1. Overview

This episode covers:
- Finding the correct Wi-Fi driver/chipset for a USB dongle
- Enabling kernel modules and drivers via `menuconfig`
- Saving kernel configuration as a `defconfig` in the custom layer
- Installing Wi-Fi tools (iw, wpa_supplicant, dhclient) in the image
- Connecting to a WPA/WPA2 network from the command line

**Hardware:** TP-Link TL-WN725N USB Wi-Fi dongle (Realtek RTL8188EU chipset) on STM32MP157D-DK1.

> The DK1 board does **not** have onboard Wi-Fi/Bluetooth (the DK2 does). A USB dongle is required.

---

## 2. Identifying the Wi-Fi Chipset

### Linux Wireless LAN Support Page
1. Visit the Linux wireless compatibility page
2. Filter by **USB** interface
3. Search for your dongle model (e.g., `TL-WN725N`)
4. Note the **chipset**: Realtek RTL8188EU
5. Check the compatibility status:
   - **Green** = well-supported
   - **Yellow** = may work (expect troubleshooting)
   - **Red** = unlikely to work

> **Expect trial and error.** Different dongles, firmware versions, and kernel modules may or may not work together. This process took significant effort to figure out.

---

## 3. Kernel Configuration for Wi-Fi

### Enter Kernel menuconfig
```bash
source poky/oe-init-build-env build-mp1
bitbake -c menuconfig virtual/kernel
```

### Required Settings

#### Networking Support → Networking Options
- [x] **TCP/IP networking** — required for internet communication
- [x] **IP: kernel level autoconfiguration** (DHCP, BOOTP, RARP)
- [x] **The IPv6 protocol** — recommended depending on network

#### Networking Support → Wireless
- [x] **cfg80211 - wireless configuration API**
- [x] **cfg80211 wireless extensions compatibility** — needed for older chipsets and USB dongles
- [x] **Generic IEEE 802.11 Networking Stack (mac80211)**
- [x] **Minstrel** — rate control algorithm for 802.11

#### Device Drivers → Network Device Support → Wireless LAN
- [ ] **Realtek rtlwifi family** — **DISABLE this** to prevent driver conflicts with the staging driver
- Ensure no other conflicting Realtek drivers are enabled

#### Device Drivers → USB Support
- [x] **Support for Host-side USB** — allows communication with USB devices
- [x] **Enable USB persist by default** — prevents connection drops during idle periods

#### Device Drivers → Staging Drivers
- [x] **Enable Staging drivers** (enter submenu)
- [x] **Realtek RTL8188EU Wireless LAN NIC driver** — set as **module (M)**
- [ ] Access Point mode — optional (useful for device provisioning via captive portal)

> **Note on staging drivers:** These are experimental drivers not yet fully integrated into the kernel. In future kernel versions, the RTL8188EU driver may move to the main Realtek section.

#### Enable Loadable Module Support
- [x] **Module signature verification**
- [x] **Require modules to be validly signed**
- [x] **Automatically sign all modules**
- **Signing algorithm:** SHA-256 (SHA-1 is insecure)

> Module signing is required because the **wireless regulatory database** (used for country-specific Wi-Fi regulations) only comes in a signed format. Without signature verification, the driver cannot load the regulatory database.

### Save and Exit
Select **Yes** when prompted to save.

---

## 4. Understanding the Kernel Config Files

| File | Description |
|------|-------------|
| `.config` | Current active configuration |
| `.config.old` | Previous configuration |
| `.config.orig` | Original configuration before any modifications |

Location: `tmp/work/<machine>/<kernel>/<version>/build/`

### Verify Settings
```bash
# Check if the RTL8188EU driver is enabled (note: uses r8188eu, not rtl8188eu)
grep "R8188" tmp/work/.../build/.config
# Should show: CONFIG_R8188EU=m

# Compare with original
grep "R8188" tmp/work/.../build/.config.orig
# Should show: # CONFIG_R8188EU is not set
```

---

## 5. Saving Configuration as a Defconfig

### Why?
Changes made via `menuconfig` are **local to the build directory**. To make them permanent and shareable via the custom layer, save as a `defconfig`.

### Generate Defconfig
```bash
bitbake -c savedefconfig virtual/kernel
```

This creates a `defconfig` file at:
```
tmp/work/<machine>/<kernel>/<version>/build/defconfig
```

### Add to Custom Layer

```bash
mkdir -p meta-custom/recipes-kernel/linux/files/
cp tmp/work/.../build/defconfig meta-custom/recipes-kernel/linux/files/defconfig
```

> **The filename `defconfig` is critical** — do not rename it. BitBake expects this exact name.

### Update the `.bbappend` Recipe

Edit `meta-custom/recipes-kernel/linux/linux-stm32mp_%.bbappend`:

```bitbake
# File search paths
FILESEXTRAPATHS_prepend := "${THISDIR}/stm32mp1:${THISDIR}/files:"

# I2C5 device tree patch (from Part 5)
SRC_URI += "file://0001-add-i2c5-userspace-dts.patch"

# Apply the default kernel configuration
KERNEL_DEFCONFIG_stm32mp1 = "defconfig"
```

**Key points:**
- **`FILESEXTRAPATHS_prepend`**: Now includes both `stm32mp1/` (for patches) and `files/` (for defconfig); colon-delimited
- **`KERNEL_DEFCONFIG_<machine>`**: Must match the machine name from `local.conf` (e.g., `stm32mp1`)
- The defconfig filename must be exactly `defconfig`

---

## 6. Adding Wi-Fi Packages to the Image

### Edit `custom-image.bb`

Add the required packages to `IMAGE_INSTALL`:

```bitbake
# Wi-Fi driver and firmware
IMAGE_INSTALL += "kernel-module-r8188eu \
    linux-firmware-rtl8188 \
    dhcp-client \
    iw \
    wpa-supplicant \
    wireless-regdb-static \
"
```

| Package | Purpose |
|---------|---------|
| `kernel-module-r8188eu` | Kernel module (driver) for the RTL8188EU chipset |
| `linux-firmware-rtl8188` | Firmware binary loaded onto the USB dongle |
| `dhcp-client` | Obtains IP address automatically from the router |
| `iw` | Modern wireless configuration tool (replaces `wireless-tools`) |
| `wpa-supplicant` | Handles WPA/WPA2 key negotiation and secure connection |
| `wireless-regdb-static` | Regulatory database for country-specific Wi-Fi regulations |

### Auto-Load the Kernel Module

Without explicit configuration, you'd have to manually run `modprobe r8188eu` after every boot. Adding the module to `IMAGE_INSTALL` with the `kernel-module-` prefix ensures it is loaded automatically at boot.

---

## 7. Building

```bash
bitbake custom-image
```

- Takes **1-1.5 hours** because the kernel must be recompiled with new driver modules
- After completion, verify the manifest:
  ```bash
  grep rtl tmp/deploy/images/stm32mp1/custom-image-*.manifest
  # Should show: linux-firmware-rtl8188
  ```

---

## 8. Flashing

Both bootfs and rootfs must be reflashed (kernel changes affect both):

```bash
# Boot filesystem
sudo dd if=st-image-bootfs-poky-stm32mp1.ext4 of=/dev/mmcblk2p4 bs=1M

# Root filesystem
sudo dd if=custom-image-stm32mp1.ext4 of=/dev/mmcblk2p5 bs=1M
```

### Fix `extlinux.conf` (as always after bootfs flash)
```bash
sudo vi /media/<username>/bootfs/mmc0_extlinux/extlinux.conf
# Change root= to: /dev/mmcblk0p5
```

Unmount and sync before removing the SD card.

---

## 9. Connecting to Wi-Fi

### Boot and Login
```bash
picocom -b 115200 /dev/ttyACM0
# Login: root / toor
```

> Note: Boot time increases with new drivers/modules loaded. This is normal and can be optimized later by removing unnecessary modules.

### Verify the Driver Is Loaded
```bash
ls /lib/modules/$(uname -r)/kernel/drivers/net/
# Should show the r8188eu driver

ifconfig -a
# Should show wlan0 (or similar) interface
```

### Bring Up the Interface
```bash
ifconfig wlan0 up
ifconfig
# wlan0 should now appear
```

### Configure WPA Supplicant

1. **Create the base config file:**
   ```bash
   vi /etc/wpa_supplicant.conf
   ```
   Add:
   ```
   ctrl_interface=/var/run/wpa_supplicant
   update_config=1
   ```

2. **Add your network credentials (hashed password):**
   ```bash
   wpa_passphrase "MySSID" "MyPassword" >> /etc/wpa_supplicant.conf
   ```
   This generates a PSK hash so the password isn't stored in plain text.

3. **Remove the plain-text password comment** from the file:
   ```bash
   vi /etc/wpa_supplicant.conf
   # Delete the line: #psk="MyPassword"
   ```

### Connect
```bash
wpa_supplicant -B -i wlan0 -D wext -c /etc/wpa_supplicant.conf
```

| Flag | Meaning |
|------|---------|
| `-B` | Run in background (daemon mode) |
| `-i wlan0` | Use the wlan0 interface |
| `-D wext` | Use the wireless extensions driver (enabled in kernel config) |
| `-c /etc/wpa_supplicant.conf` | Configuration file location |

You should see:
- `wpa_supplicant` initialized
- Association success
- Link becoming ready

### Get an IP Address
```bash
dhclient wlan0
```

### Verify Connection
```bash
ifconfig wlan0
# Should show an assigned IP address

ping 8.8.8.8
# Should get responses

wget -O - http://example.com
# Should display HTML content
```

---

## 10. Known Issues and Troubleshooting

| Issue | Notes |
|-------|-------|
| `iw dev wlan0 scan` not working | May be specific to staging drivers; the dongle still works for connections |
| `rfkill` can't detect the dongle | Common with staging USB Wi-Fi drivers; doesn't prevent connectivity |
| `iw dev wlan0 link` not reporting | Same staging driver limitation |
| Longer boot time after enabling Wi-Fi | New modules/drivers take time to initialize; optimize by removing unused modules |
| Different dongle chipsets may not work | Each chipset requires specific kernel modules and firmware; trial and error expected |

---

## 11. User Experience Considerations for Production

For consumer devices, having users type Wi-Fi credentials into a console is not acceptable. Common approaches:

- **Captive portal / Access Point mode:** Device broadcasts its own AP → user connects with phone → enters Wi-Fi credentials via a web page → device switches to client mode
- **Bluetooth provisioning:** Companion phone app sends credentials via Bluetooth
- **WPS:** Push-button or PIN-based Wi-Fi Protected Setup
- **Pre-configured:** Credentials baked into the image for known deployment environments

---

## 12. Key Takeaways

- **Wi-Fi on embedded Linux requires:** correct kernel modules, firmware, and user-space tools (wpa_supplicant, dhclient, iw)
- **Chipset identification is the first step** — use the Linux wireless compatibility page
- **Staging drivers** are experimental but may be the only option for newer or less common chipsets
- **Module signing must be enabled** for the wireless regulatory database to load
- **`defconfig`** preserves kernel settings in the custom layer — critical for reproducible builds
- **`bitbake -c savedefconfig virtual/kernel`** generates the defconfig from current settings
- Disable conflicting drivers (e.g., `rtlwifi`) to avoid driver conflicts and blacklist issues
- **WPA Supplicant** handles secure Wi-Fi connections; always hash passwords with `wpa_passphrase`
- Boot time increases with more modules — optimize by removing unnecessary drivers for production
- This series covered: Buildroot, Yocto setup, boot chain, custom images/layers, device tree patching, custom applications, and Wi-Fi networking
