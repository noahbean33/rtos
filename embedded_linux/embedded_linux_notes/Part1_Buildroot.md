# Introduction to Embedded Linux Part 1 - Buildroot

> Source: Digi-Key Electronics - Introduction to Embedded Linux Series

---

## 1. Why Embedded Linux?

Linux is the dominant OS for embedded/IoT development. In a 2018 Eclipse Foundation survey, Linux outranked all other operating systems for IoT device development by a wide margin.

---

## 2. Embedded Development Approaches Compared

### Bare Metal Programming
- Used on 8-bit, 16-bit, and some 32-bit microcontrollers
- **Pros:** Very efficient code, minimal overhead, direct hardware control (register-level), low power consumption
- **Cons:** Not portable (code is tied to a specific MCU family), limited to single-purpose or simple applications
- **Best for:** Strict timing requirements (motor control, LEDs), simple or single-purpose applications

### Real-Time Operating Systems (RTOS)
- Includes a background scheduler, so requires more powerful MCUs
- **Pros:** Vendor support packages improve portability, multi-threading support, can still meet strict timing deadlines (hard RTOS or hardware timers)
- **Cons:** Some overhead from scheduler, still relatively low-level
- **Best for:** Networking, user interfaces, tasks requiring concurrency with timing guarantees

### Embedded General-Purpose OS (e.g., Embedded Linux)
- Requires a full "tiny computer" — a microprocessor with external RAM and non-volatile storage (e.g., SD card)
- **Pros:**
  - Access to fast CPUs (hundreds of MHz to GHz) and large memory (potentially GBs)
  - Highly portable code across different boards
  - Access to high-level languages (Python, Java)
  - Advanced features: networking, file systems, complex UIs, many concurrent processes
  - Huge ecosystem of vendor/community libraries
- **Cons:**
  - Significant overhead (scheduler, memory management, background tasks)
  - Loss of direct hardware control; difficult to meet strict timing deadlines
  - Requires more powerful and expensive hardware
- **Best for:** Video, audio, games, AI, computer vision, networking, databases, complex algorithms

### Hybrid Approach
Many chip manufacturers include a **microcontroller on the same die** as the microprocessor:
- Microprocessor handles networking, complex algorithms, OS tasks
- Microcontroller handles low-level hardware control and strict timing

---

## 3. Use Cases for Embedded Linux

- Networking gear (routers, lightweight servers)
- Robotics (complex task processing)
- Aerospace (e.g., SpaceX Falcon 9 runs Linux)
- Consumer electronics
- Kiosk interfaces
- IoT devices

In most of these, a **custom OS** is preferred to reduce power consumption and production costs — no unnecessary bloat.

---

## 4. Getting Linux onto a Board

### Option 1: Pre-made Image (Easiest)
- Download image from vendor, flash to SD card, boot
- **Pro:** Fast, likely works out of the box
- **Con:** Bloated — includes unneeded software, longer boot times, more flash/power usage; per-board customization is tedious at scale

### Option 2: Manual Driver Assembly (Hardest)
- Find or write every driver, fight with compilers
- Could take months; this is why designing custom SBCs is so difficult

### Option 3: Automated Build Tools (Recommended)
The three most popular tools:

| Tool | Strengths | Weaknesses |
|------|-----------|------------|
| **Buildroot** | Extremely easy to use | Limited config options; full rebuild needed for every change |
| **OpenWrt** | Great for networking gear (routers) | Focused on package-manager-based updates; less suited for fleet deployment |
| **Yocto Project** | Highly customizable; large active community; incremental builds | Steep learning curve; requires significant disk space and processing power |

> **Note:** Even with these tools, custom SBCs still require you to create or locate drivers.

---

## 5. Buildroot Walkthrough

### Prerequisites
- A Linux host machine (Ubuntu recommended, Linux Mint also works)
- Options for running Linux: dual boot, VM, WSL (untested), Docker (untested)
- Install mandatory packages:
  ```bash
  sudo apt update
  sudo apt upgrade
  sudo apt install build-essential libncurses-dev
  ```

### Steps to Build an Image with Buildroot

1. **Clone Buildroot:**
   ```bash
   mkdir ~/projects && cd ~/projects
   git clone <buildroot-repo-url>
   cd buildroot
   ```

2. **Browse available configs:**
   ```bash
   ls configs/
   ```
   Look for a config matching your board. For STM32MP157D-DK1, the `stm32mp157a` config was used as a starting point.

3. **Load a default configuration:**
   ```bash
   make stm32mp157a_dk1_defconfig
   ```
   This reads the defconfig and generates a `.config` file.

4. **Customize with menuconfig (optional):**
   ```bash
   make menuconfig
   ```
   Adjust kernel options, packages, etc. through a graphical TUI.

5. **Build the image:**
   ```bash
   make
   ```
   - **Takes 1+ hours.** Monitor for errors.
   - Output is in `output/images/` — look for `sdcard.img`.

### Flash to SD Card

1. **Unmount any auto-mounted partitions** on the SD card.
2. **Identify the device:**
   ```bash
   lsblk
   ```
   (e.g., `/dev/mmcblk2` for an 8 GB card)

3. **Write the image with `dd`:**
   ```bash
   sudo dd if=output/images/sdcard.img of=/dev/mmcblk2 bs=1M
   ```
   - `dd` shows no progress by default; wait for completion.

### Boot the Board

1. **Set boot switches** — for STM32MP157D-DK1, set BOOT0 and BOOT2 to ON (boot from SD card).
2. **Connect serial console:**
   - UART4 is tied to the onboard ST-Link (USB-to-serial). Connect via USB micro cable.
   - If no onboard translator, use an external FTDI board.
3. **Power the board** (USB-C, 5V/3A).
4. **Open serial terminal on host:**
   ```bash
   # Install picocom if needed
   sudo apt install picocom

   # Add user to dialout group (required for permission)
   sudo usermod -aG dialout $USER
   groups  # verify dialout appears

   # Connect
   picocom -b 115200 /dev/ttyACM0
   ```
5. **Press reset** on the board to watch boot messages.
6. **Login:** `root` (no password for basic Buildroot image).

### What You Get
- Minimal Linux — almost nothing installed
- No package manager
- Basic tools only (e.g., `vi`)
- You develop applications on the host, cross-compile, and deploy binaries to the image

### Quick Test
```bash
vi test.sh
# (press i, type: echo "Hello from Embedded Linux!", press Esc, type :wq)
sh test.sh
```

### Exit picocom
```
Ctrl+A then Ctrl+X
```

---

## 6. Key Takeaways

- **Buildroot** is the fastest way to get a custom Linux image, but every change requires a full rebuild (hours).
- Embedded Linux devices are typically **single-purpose** — you design an image for a specific application and deploy it to many devices.
- For more control and incremental builds, the **Yocto Project** is the better choice (covered in Part 2).
- The STM32MP157D-DK1 discovery kit is used throughout the series as the reference board.
- The 157A config works on the 157D board but may underclock the CPU (650 MHz vs. 800 MHz) — find the correct config for production use.
