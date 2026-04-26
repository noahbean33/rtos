# Mastering Embedded Linux, Part 4: Adding Features

> Source: George Hilliard's blog — &>/dev/null

---

## 1. Overview

How to add high-level "features" to a Buildroot firmware image by taking off-the-shelf open-source software, configuring it, and integrating it. The example: turning a **Raspberry Pi Zero W** into a **wireless access point** with automatic DHCP.

### Feature Workflow (Generally Applicable)
1. **Plan** — define user-facing behavior and error handling
2. **Select** — choose software packages (kernel drivers, daemons, scripts)
3. **Configure** — tailor software to your specific use case
4. **Test** — boot with new code, inspect, debug, iterate
5. **Finalize** — check changes into source control

> The philosophy: **"no setup needed"** — entire functionality is reproducibly captured in source code and configuration, not on one random SD card.

---

## 2. Access Point Architecture

### Required Components
| Component | Purpose |
|-----------|---------|
| **hostapd** | Creates the Wi-Fi access point; manages connect/disconnect events; sets channel and security |
| **dnsmasq** | DHCP server; assigns IP addresses to connecting clients |
| **Wi-Fi firmware** | Binary blob for the Broadcom Wi-Fi chip (`/lib/firmware`) |
| **Kernel module** | `brcmfmac` driver for the Wi-Fi hardware |

### Network Configuration
- **AP SSID:** `MasteringEmbeddedLinux`
- **Wi-Fi mode:** 802.11g, channel 6, open (no encryption)
- **Target IP:** `10.33.40.1` (static, on `wlan0`)
- **DHCP range:** `10.33.40.10` – `10.33.40.200`, subnet `/24`, 24-hour lease

---

## 3. Configuration Files

### `dnsmasq.conf`
```ini
interface=wlan0
dhcp-range=10.33.40.10,10.33.40.200,255.255.255.0,24h
```

### `hostapd.conf`
```ini
interface=wlan0
driver=nl80211
ssid=MasteringEmbeddedLinux
hw_mode=g
channel=6
```

### `/etc/network/interfaces`
```
auto wlan0
iface wlan0 inet static
    address 10.33.40.1
    netmask 255.255.255.0
```
Overwrites the default Buildroot `interfaces` file (which configures DHCP on `eth0`).

> These config files are **not Buildroot-specific** — you could test them on any Linux workstation first for faster iteration.

---

## 4. Startup Scripts (Init System)

Buildroot uses **SysV init scripts**: executable files in `/etc/init.d/` run in **alphabetical order** at boot. Convention: `S` prefix + number for ordering.

### `S02modprobe` — Load Wi-Fi Kernel Module
```bash
#!/bin/sh
/sbin/modprobe brcmfmac
```
Must run early (before hostapd).

### `S90hostapd` — Start Access Point Daemon
```bash
#!/bin/sh

case "$1" in
    start)
        printf "Starting hostapd: "
        start-stop-daemon -S -x /usr/sbin/hostapd -- -B /etc/hostapd.conf
        [ $? = 0 ] && echo "OK" || echo "FAIL"
        ;;
    stop)
        printf "Stopping hostapd: "
        start-stop-daemon -K -q -x /usr/sbin/hostapd
        [ $? = 0 ] && echo "OK" || echo "FAIL"
        ;;
    restart|reload)
        $0 stop
        $0 start
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
esac
exit 0
```

> **Init scripts must be executable** (`chmod +x`), or init will emit "permission denied" errors.

---

## 5. Implementing in Buildroot

### Using `menuconfig`

```bash
make menuconfig
```

#### Navigation
| Key/Symbol | Meaning |
|-----------|---------|
| `--->` | Submenu; enter with `<Select>`, exit with `<Exit>` |
| `[ ]` | Checkbox; toggle with `Space` |
| `***` | Comment (informational) |
| `/` | Search (Vim-style) — search by package name or `BR2_` variable |

> If an option is invisible, its **dependencies** aren't enabled. Search shows dependency info.

#### Enable ccache (Recommended First Step)
- Path: **Build Options → Enable compiler cache**
- Caches compilation output; drastically lowers rebuild times
- With warm cache, full rebuild from scratch: **~20 minutes**
- Warm up: `make clean && make` overnight

#### Enable Required Packages
- **dnsmasq:** Target packages → Networking applications → dnsmasq
- **hostapd:** Target packages → Networking applications → hostapd
- **Wi-Fi firmware:** Target packages → Hardware handling → Firmware (`BR2_PACKAGE_RPI_WIFI_FIRMWARE`)

### Rootfs Overlay
Board-specific config files go in an **overlay directory** — files are copied onto the generated filesystem after all packages compile.

```bash
mkdir -p board/raspberrypi/rootfs_overlay/etc/init.d
mkdir -p board/raspberrypi/rootfs_overlay/etc/network
```

| Source File | Overlay Destination |
|------------|-------------------|
| dnsmasq config | `etc/dnsmasq.conf` |
| hostapd config | `etc/hostapd.conf` |
| ifupdown config | `etc/network/interfaces` |
| Module loader script | `etc/init.d/S02modprobe` |
| hostapd startup script | `etc/init.d/S90hostapd` |

```bash
chmod +x board/raspberrypi/rootfs_overlay/etc/init.d/*
```

Set the overlay path in menuconfig: `BR2_ROOTFS_OVERLAY = board/raspberrypi/rootfs_overlay/`

---

## 6. Rebuilding and Redoing

### Rebuild
```bash
make
```
Recompiles only changed/new packages; takes minutes with warm ccache.

### Redo Logic

| Action | Command | When to Use |
|--------|---------|-------------|
| **Full clean** | `make clean` | Sweeping changes (architecture, compiler, C library); pre-release builds |
| **Single package clean** | `make <package>-dirclean` | Changed build flags for one package |
| **Post-build scripts** | Automatic on `make` | Always re-run (e.g., image assembly); picks up manual `output/target` changes |

> `make clean` is the "nuclear option." `<package>-dirclean` is the "tactical nuke." Post-build scripts always re-run.

**Important:** `dirclean` only erases the build directory, not files already copied into `output/target`.

---

## 7. Testing

### Flash and Boot
```bash
sudo dd if=output/images/sdcard.img of=/dev/mmcblkX bs=1M status=progress
```

Serial console output should show:
```
Starting dnsmasq: OK
Starting hostapd: OK
```

The access point should appear on phones/workstations.

> **Android caveat:** May fail connectivity check and refuse to route traffic (AP has no internet). Workstations connect without issues.

---

## 8. Saving Changes to Git

```bash
make savedefconfig        # Save working config → defconfig
git add board/raspberrypi/rootfs_overlay/
git add configs/
git commit -m "rpi: implement simple wireless access point"
```

> **`.config`** is NOT version controlled. **Defconfigs** ARE version controlled.

---

## 9. Experimenting on the Fly

For rapid iteration:
1. Boot a half-finished image
2. Modify config files directly on the target via serial console
3. Test manually
4. Once working, **copy changes back to the Buildroot overlay**

> Buildroot's power is in **reproducibility** — manual changes to a running system aren't captured unless migrated back.

---

## 10. Key Takeaways

- **Plan → Select → Configure → Test → Finalize** is the backbone of embedded system customization
- Much of embedded Linux engineering is **packaging existing software**, not writing new code
- **ccache** dramatically lowers rebuild times — enable it first
- **Rootfs overlay** is for board-specific customization (config files, scripts)
- **All changes are automated and captured in Git** — `make` reproduces the firmware
- **SysV init scripts** in `/etc/init.d/` run alphabetically; use `S##` prefix for ordering
- Test on your workstation first when possible — faster iteration than on-target debugging
- `make savedefconfig` persists menuconfig changes; `make <pkg>-dirclean` rebuilds individual packages
