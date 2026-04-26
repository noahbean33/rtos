# Introduction to Embedded Linux Part 2 - Yocto Project

> Source: Digi-Key Electronics - Introduction to Embedded Linux Series

---

## 1. Terminology & Concepts

### OpenEmbedded Project
- Community-driven project founded in **2003**
- Creates a suite of tools for building custom Linux distributions for embedded devices
- Many files in Yocto have the `oe-` or `OE` prefix because of this heritage

### Yocto Project
- Official **Linux Foundation** project started in **2011**
- Relies on OpenEmbedded tools and reference files
- Maintains support for popular architectures (x86, ARM, etc.)

### Key Definitions

| Term | Definition |
|------|-----------|
| **Metadata** | Any files used to configure the build process (settings, source code locations, etc.) |
| **Recipe** | A specific metadata file with instructions to download, configure, and compile source code |
| **Layer** | A directory grouping related metadata; usually prefixed with `meta-` |
| **Board Support Package (BSP)** | A special layer defining how to build for a particular board; includes board-specific drivers; maintained by vendors |
| **Distribution** | A specific implementation of Linux (like Ubuntu, Fedora, etc.) — you are creating your own |
| **Machine Configuration** | Specifies the physical architecture/board to build for; points to a specific BSP |
| **Image** | The final build output — contains kernel, bootloaders, drivers, packages, root filesystem |

### Poky Reference Distribution
- **Poky is NOT Yocto, and Yocto is NOT Poky**
- Poky is a reference distribution — a known-good collection of metadata, layers, BSPs, BitBake, and tools
- Serves as the starting point; you add your own layers on top of it to create your custom distribution

### Build Process Overview (from Yocto docs)
1. Configure metadata
2. Build system reads metadata to determine source code locations
3. Downloads source code
4. Applies patches
5. Tests generated packages
6. Creates output image(s)

The build can also generate a **Software Development Kit (SDK)** for writing code targeting your board.

---

## 2. Host Machine Requirements

- **Supported distros:** Ubuntu (LTS recommended), Fedora, others listed in Yocto reference manual
- **Minimum disk space:** ~50 GB free
- **Minimum RAM:** 4 GB (2 GB possible with workarounds — see XZ compression fix below)
- Run updates first:
  ```bash
  sudo apt update
  sudo apt upgrade
  ```

### Required Packages
```bash
sudo apt install gawk wget git diffstat unzip texinfo gcc build-essential \
  chrpath socat cpio python3 python3-pip python3-pexpect xz-utils \
  debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa \
  libsdl1.2-dev pylint3 xterm libssl-dev
```

### Python Alias (if `python` command doesn't work)
```bash
# Add to ~/.bashrc:
alias python=python3
source ~/.bashrc
python --version  # verify
```

---

## 3. Setting Up Yocto with Poky

### Clone Poky
```bash
cd ~/projects
git clone https://git.yoctoproject.org/poky
cd poky
```

### Key Directories in Poky
- `bitbake/` — the main build tool
- `meta/`, `meta-poky/`, `meta-*` — layers containing metadata
- `scripts/`, `documentation/`

### Branch Management (CRITICAL)
- All layers, BSPs, and Poky itself **must be on the same release branch**
- Check Yocto Project releases for Long-Term Support (LTS) versions (supported 2+ years)
- **Code name** (e.g., `dunfell`) must match across all repositories

```bash
# Check available branches
git branch -r

# Checkout the correct branch
git checkout dunfell
git status  # verify: "On branch dunfell"
```

### Verify Distribution Config
```bash
cat meta-poky/conf/distro/poky.conf
```
- Shows: reference distribution name, branch, sanity-tested host distros

---

## 4. Downloading the Board Support Package

### STM32MP1 BSP (ST Microelectronics)
```bash
cd ~/projects
git clone https://github.com/STMicroelectronics/meta-st-stm32mp.git
cd meta-st-stm32mp
git checkout dunfell   # MUST match Poky branch
git status             # verify
```

### Check BSP Dependencies (read the README!)
The `meta-st-stm32mp` BSP requires:
1. **OpenEmbedded Core** — included in Poky (no extra download)
2. **BitBake** — included in Poky
3. **meta-openembedded** — must be downloaded separately

```bash
cd ~/projects
git clone https://github.com/openembedded/meta-openembedded.git
cd meta-openembedded
git checkout dunfell
```
- This layer has no additional dependencies of its own

### Layer Index
The OpenEmbedded project maintains a [layer index](https://layers.openembedded.org/) with known-good layers (BSPs, security, networking, etc.).

---

## 5. Initializing the Build Environment

```bash
cd ~/projects/poky
source oe-init-build-env build-mp1
```

This command:
- Creates a build directory (`build-mp1/`)
- Adds `bitbake` to your PATH
- Places you inside the build directory

### Verify Available Layers
```bash
bitbake-layers show-layers
```
Default layers: `meta`, `meta-poky`, `meta-yocto-bsp` (builds for QEMU emulator by default).

---

## 6. Configuring Layers

### Edit `conf/bblayers.conf`
Add the BSP and its dependency layers using **absolute paths**:

```bash
vi conf/bblayers.conf
```

Add to the `BBLAYERS` variable:
```
/home/<user>/projects/meta-openembedded/meta-oe \
/home/<user>/projects/meta-openembedded/meta-python \
/home/<user>/projects/meta-st-stm32mp \
```

> **Important:** Point to specific sub-layers (e.g., `meta-oe`, `meta-python`) within `meta-openembedded`, not the top-level directory. The BSP machine config files will tell you which sub-layers are required.

### Verify
```bash
bitbake-layers show-layers
```

---

## 7. Configuring the Machine

### Find Available Machines
Browse the BSP's machine configuration files:
```bash
ls meta-st-stm32mp/conf/machine/
```

### Edit `conf/local.conf`
```bash
vi conf/local.conf
```

- Comment out the default machine (QEMU):
  ```
  #MACHINE ??= "qemux86-64"
  ```
- Set your target machine:
  ```
  MACHINE = "stm32mp1"
  ```

### Low-RAM Workaround (< 4 GB RAM)
Add to `local.conf` to prevent XZ compression from failing:
```
XZ_DEFAULTS = "--memlimit=256MiB"
```
Without this, XZ tries to grab ~2 GB RAM and fails on low-memory machines instead of using swap.

---

## 8. Building the Image

### Available Image Types
From the Yocto reference manual:

| Image | Description |
|-------|-------------|
| `core-image-minimal` | Bare-bones: command line, mostly empty root filesystem |
| `core-image-full-cmdline` | More command-line tools |
| `core-image-sato` | Graphical interface (Sato) |
| `core-image-weston` | Weston/Wayland compositor |
| `core-image-rt` | Real-time kernel |

### Optional: Kernel Configuration
```bash
bitbake -c menuconfig virtual/kernel
```
- First run takes ~10 minutes to parse
- Subsequent runs are much faster (Yocto caches previous work)

### Build
```bash
bitbake core-image-minimal
```

- **First build: 5-6+ hours** (be patient; let it run overnight)
- Subsequent builds are **much faster** (Yocto rebuilds only changed layers — a major advantage over Buildroot)
- If it fails on a missing `.h` file, install the required package via `apt`
- If interrupted, it **resumes where it left off**

### Clean Options
```bash
# Remove output images only (keeps intermediate steps):
bitbake -c cleanall core-image-minimal

# Full clean (remove everything, start from scratch):
rm -rf tmp/
```

---

## 9. Output Image Files

Located in:
```
tmp/deploy/images/stm32mp1/
```

You may find:
- `.ext4` filesystem images
- Boot filesystems, user filesystems, root filesystems
- Flash layout scripts (for SD card partitioning)
- FIP (Firmware Image Package) — part of the trusted boot chain
- ARM Trusted Firmware files

> **Note:** Unlike Buildroot, ST's BSP may not provide a single `sdcard.img` file. You may need to manually set up partitions and flash individual images (covered in Part 3).

---

## 10. Key Takeaways

- **Branch alignment is critical** — Poky, BSPs, and all layers must be on the same release branch (e.g., `dunfell`)
- **Yocto's incremental builds** save hours compared to Buildroot's full rebuilds
- **BitBake** is the core build engine that reads metadata, recipes, and layers to produce images
- **Poky** is just a reference starting point; you customize by adding your own layers
- The build process is resource-intensive (~50 GB disk, 4+ GB RAM, hours of compile time)
- Always read the **vendor documentation** and BSP READMEs for dependency and configuration details
