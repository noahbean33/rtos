# Mastering Embedded Linux, Part 3: Buildroot

> Source: George Hilliard's blog — &>/dev/null

---

## 1. Overview

First hands-on tutorial: building a complete Linux OS from source code using **Buildroot** and booting it on a Raspberry Pi (or virtual machine). Buildroot is the right combination of "powerful" and "easy to use."

### Prerequisites
- Host workstation running Linux (Ubuntu recommended; VM okay with caveats)
- **15 GiB** free disk space
- **2–4 hours** free time
- Working knowledge of Linux shell

---

## 2. Getting Started — Six Commands

### Step 1: Install Host Tools
```bash
sudo apt install -y git build-essential wget cpio unzip rsync bc libncurses5-dev screen
```
> Ubuntu-specific; only host-dependent command in the guide.

### Step 2: Download Buildroot
```bash
git clone git://git.buildroot.net/buildroot
cd buildroot/
```

### Step 3: Check Out a Release
```bash
git checkout 2019.11.1
```
Detached HEAD is expected and fine.

### Step 4: Configure for Target Hardware
| Target | Command |
|--------|---------|
| Raspberry Pi Zero W | `make raspberrypi0w_defconfig` |
| Raspberry Pi Zero | `make raspberrypi0_defconfig` |
| Virtual machine | `make qemu_x86_64_defconfig` |

Configurations live in the `configs/` directory. Output confirms:
```
# configuration written to /home/user/buildroot/.config
```

### Step 5: Build
```bash
make
```
- Takes **2–3 hours** depending on workstation speed
- Requires active internet connection to download source code
- Optional: `nice make` to lower build priority and keep workstation responsive

---

## 3. How Buildroot Works

### Compilation Flow
```
1. Build the toolchain (cross-compilers for target architecture)
2. Download source code for each software component
3. For each package: unpack → patch → configure → compile → install into rootfs
4. Copy extra files (board-specific configs) into rootfs
5. Assemble final firmware image from rootfs
```

### Key Directories
| Directory | Purpose |
|-----------|---------|
| `board/` | Files and scripts for each target board |
| `configs/` | Build configurations (defconfigs) |
| `package/` | Package definitions (2,289 in Buildroot 2019.11) |
| `output/host/` | Build tools that run on the workstation |
| `output/target/` | Target root filesystem (staged binaries) |
| `output/images/` | Final firmware image and filesystem images |

### Packages
- Grouped compilation scripts defining **config options**, **build steps**, and **dependencies**
- Config options use **Kconfig** language (originally from Linux kernel)
- Each package has at minimum a toggle to enable/disable it
- Packages can "select" their smaller dependencies automatically
- 2,289 packages available: from Nginx to Chocolate Doom

### Build Configurations (defconfigs)
- A **defconfig** stores all non-default configuration options for all packages
- `make raspberrypi0w_defconfig` → copies options into working `.config`
- `.config` is the **working configuration** — not version controlled
- **`make savedefconfig`** saves working config back to the defconfig (version controlled)
- **`make menuconfig`** provides a TUI to browse and change options

---

## 4. Output Files

After a successful build:
```bash
ls -lh output/images/
```

| File | Description |
|------|-------------|
| `sdcard.img` | Complete SD card image ready to burn |
| `boot.vfat` | Boot partition (kernel, device tree, firmware) |
| `rootfs.ext2` / `rootfs.ext4` | Root filesystem |
| `zImage` | Compressed Linux kernel |
| `bcm2708-rpi-zero.dtb` | Device tree blob for Pi Zero |

The `output/target/` directory contains the staged root filesystem. Note the `THIS_IS_NOT_YOUR_ROOT_FILESYSTEM` file — some special files are created during final image assembly.

---

## 5. Booting the Image

### Using `dmesg` to Find Device Names
```bash
dmesg -w
```
Plug in SD card → observe assigned device name (e.g., `mmcblk0`, `sdb`). Press `Ctrl+C` to stop.

### Burn SD Card
```bash
sudo dd if=output/images/sdcard.img of=/dev/mmcblkX bs=1M status=progress
sync
```

> **WARNING:** Double-check `of=` — `dd` will happily overwrite your hard drive.

| dd Option | Explanation |
|-----------|-------------|
| `if=` | Input file — read from here |
| `of=` | Output file — write to here |
| `bs=` | Block size — write this many bytes at once |
| `status=` | Show progress |

### Virtual Machine Alternative
```bash
output/host/bin/qemu-system-x86_64 -M pc \
    -kernel output/images/bzImage \
    -drive file=output/images/rootfs.ext2,if=virtio,format=raw \
    -append "rootwait root=/dev/vda" \
    -net nic,model=virtio -net user
```
> VM boots in seconds. Command found in `boards/qemu/x86_64/readme.txt`.

### Connect Serial Console (Raspberry Pi)

#### FT2232H Wiring
| Pi Header Pin | FT2232H Pin | Signal |
|--------------|-------------|--------|
| GPIO 14 (TXD) | ADBUS1 (RXD) | Pi TX → FT2232 RX |
| GPIO 15 (RXD) | ADBUS0 (TXD) | FT2232 TX → Pi RX |
| GND | GND | Common ground |

> ADBUS0 may be labeled D0 or AD0 depending on adapter.

#### Open Serial Console
```bash
sudo screen -fn /dev/ttyUSBX 115200
```
- **115200 baud**, no hardware flow control (universal convention)
- Quit: `Ctrl+a` then `\`

### Boot
Plug in Pi power → serial console shows boot messages:
```
Booting Linux on physical CPU 0x0
Linux version 4.19.66 ...
...
Welcome to Buildroot!
buildroot login:
```
Login: **root** (no password).

---

## 6. Key Takeaways

- **You own all the code** — everything is open source, compiled on your machine. If something is broken, change it.
- **All the work is automated** — Buildroot lets you focus on one thing at a time, starting from a working Linux system.
- **The target rootfs is tiny** — default image is only **57MB**, mostly kernel modules that can be disabled. Buildroot can fit into as little as **4MB**.
- **defconfig** captures the entire build configuration reproducibly
- **`make menuconfig`** is the TUI for browsing/changing options; **`make savedefconfig`** persists changes
- **`.config`** is the working config (not version controlled); defconfigs are version controlled
- The compilation flow is: **toolchain → download → patch → configure → compile → install → assemble image**

---

## 7. Recommended Reading

- **How Linux Works, 2nd Edition** by Brian Ward — covers shell commands through X11 and DBus
- **Bootlin's Buildroot training** — thorough slide deck (free); paid instructor-led training available
- **Buildroot user manual** — comprehensive reference for writing packages and using the build system
