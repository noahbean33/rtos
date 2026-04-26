# Mastering Embedded Linux, Part 1: Concepts

> Source: George Hilliard's blog — &>/dev/null

---

## 1. Overview

A high-level introduction to the concepts behind hacking cheap embedded Linux systems. The key insight: embedded Linux is within reach for makers and hobbyists, often for **less than $10**.

### Series Roadmap
1. **Concepts** (this article)
2. **Hardware** — choosing a development board or building your own
3. **Buildroot** — compiling a complete OS from source
4. **Customizing firmware** — adding features and configurations
5. **Going smaller** — fitting Linux into 4MB of storage
6. **Hacking U-Boot and Linux** — updating out-of-date code
7. **Building a board from scratch**

### Prerequisites
- Linux command line knowledge
- Understanding of embedded system components (flash, processor, peripherals)
- Time for learning and experimenting

---

## 2. Microcontrollers vs. Microprocessors

### Microcontroller (e.g., STM32F1)
- **Self-contained:** Flash, RAM, processor core, and peripherals in a single package
- May include specialized peripherals (Bluetooth, USB 3.0, MIPI camera)
- Typically programmed "bare metal" — no real OS
- Hobbyist-friendly packages (SOIC, QFP)

### Microprocessor (Linux-capable)
- **Packed with peripherals** but typically **lacks built-in RAM and storage**
- More complex boot process: Boot ROM → Bootloader → Linux
- Once Linux boots, the environment looks **identical to desktop Linux** — same APIs, filesystem, networking protocols
- System images can be as small as **4MB**

### Architecture Comparison

| Feature | Microcontroller | Microprocessor |
|---------|----------------|----------------|
| RAM | Built-in | External (SDRAM, DDR, DDR2/3) |
| Storage | Built-in flash | External (SD, eMMC, NOR/NAND flash) |
| Boot process | Direct execution | Multi-stage (ROM → Bootloader → Kernel) |
| OS | Bare metal / RTOS | Full Linux |
| Package | SOIC / QFP | Often BGA (harder for hobbyists) |

---

## 3. Hardware Components

### Microprocessor
- Most cheap embedded Linux systems use **ARM** or **MIPS** cores
- **RISC-V** is emerging as an open-source ISA with growing industry interest
- Silicon vendors add peripherals (USB, SPI, etc.) configured via memory-mapped registers
- Manufacturers reuse peripheral IP across product lines

### Memory (RAM)
- Provided separately from the processor
- Types: SDRAM (low end), DDR, DDR2, DDR3
- Memory controller initialized by boot code or first-stage bootloader
- **Some processors include RAM in-package** — critical for hobbyists because PCB layout for external RAM is difficult (impedance matching, trace length matching)

### Storage (Non-volatile)
| Type | Controller | Notes |
|------|-----------|-------|
| **SD / microSD** | Built-in | Easy to use; **poor reliability** (single point of failure) |
| **eMMC** | Built-in | Embedded SD card; more reliable; BGA packages (hard to solder) |
| **NOR flash** | None (raw) | Slow writes, cheap, low density; Boot ROMs usually support SPI NOR |
| **NAND flash** | None (raw) | Faster, denser, slightly more expensive; SPI or dedicated bus |

### Raw Flash Challenges
- Can only write once before erasing in **large blocks**
- Limited write cycles: **1,000–100,000** depending on technology
- Linux **UBI subsystem** helps manage raw flash (wear leveling, bad block management)

---

## 4. Software Stack

The software follows a predictable layered pattern:

```
┌─────────────────────────────────┐
│  Custom Programs / Web Server   │
│  SSH Server / User Interface    │
├─────────────────────────────────┤
│         Linux Kernel            │
├─────────────────────────────────┤
│         Bootloader              │
├─────────────────────────────────┤
│         Hardware                │
└─────────────────────────────────┘
```

Embedded Linux distributions (like Buildroot) provide toolkits to build firmware images containing all layers.

### Bootloader
- First program the engineer controls
- Almost certainly **Das U-Boot** ("the universal bootloader")
- Contains stripped-down drivers for storage and minimal peripherals
- Reads kernel into memory and starts execution
- Modifiable — it's just a program like any other

### Boot ROM
- Small code chunk embedded in the processor by the manufacturer
- Runs immediately on power-up; loads the real bootloader
- Boot order specified in the processor's datasheet
- Often speaks **USB** — allows flashing an unprogrammed board from a computer
  - NXP/Freescale: **Download Mode**
  - Allwinner: **FEL Mode**
- Makes boards nearly **impossible to brick** if present

### Linux Kernel
- Must be **ported** to each architecture, part, and board
- All drivers ship in a single source tree
- Key components of a port:
  - **Architecture code** — low-level routines (register manipulation, synchronization)
  - **Drivers** — bulk of kernel source; most are irrelevant to a given embedded system
  - **Device tree** — "config file" for drivers; describes how hardware is connected; makes drivers reusable across boards

### Userland
Everything running above the kernel — identical to desktop Linux counterparts.

#### Filesystem
| Storage Type | Recommended Filesystems |
|-------------|------------------------|
| Block device (eMMC, SD) | ext2/3/4, **f2fs** (flash-friendly) |
| Raw flash | JFFS2, UBIFS, **squashfs on UBI** (author's favorite) |

#### Init System
| System | Use Case |
|--------|----------|
| **systemd** | Larger embedded systems |
| **SysV init** (shell scripts) | Small systems; simpler and lighter |

#### Shell
- Typically accessed over **UART serial connection**
- Reaching a shell prompt = system is up and running

---

## 5. Key Takeaways

- Embedded Linux processors are **cheap and powerful** but require external RAM and storage
- The boot chain is: **Boot ROM → Bootloader (U-Boot) → Linux Kernel → Userland**
- **Device trees** are the configuration files that tell Linux how hardware is connected
- The **entire software stack is open source** — if something is broken, you can change it
- System images can be as small as **4MB** while providing full Linux functionality
- Once Linux boots, the application environment is **identical to desktop Linux**
- Boot ROM USB modes make boards nearly impossible to brick
- Raw flash requires special filesystems and management (UBI/UBIFS)
- The filesystem, init system, and shell are the three major userland components
