# Mastering Embedded Linux, Part 2: Hardware

> Source: George Hilliard's blog — &>/dev/null

---

## 1. Overview

A deep dive into the hardware options for embedded Linux development. Three main approaches:
1. **Buy it** — single-board computers (SBCs)
2. **Hack it** — repurpose existing consumer devices
3. **Build it** — design custom boards with bare processors or SoMs

---

## 2. Buy It — Single-Board Computers

### Raspberry Pi
- Great for getting started; abundant storage, memory, connectivity
- **Pi Zero** ($5): 512MB RAM, entry-level
- **Pi Zero W** ($10): adds Wi-Fi and Bluetooth
- Pi Zero / Zero W have **USB OTG** — can emulate Ethernet adapter, flash drive, etc.
- Downside: somewhat bulky; SD card is a reliability concern

> The series uses a **Raspberry Pi Zero W** as the reference hardware.

### Other SBCs
- **OrangePi** — wide range; superior interconnection (4G LTE, PCIe, mSATA, eMMC); some as low as $10
- **Hackerboards** database — comprehensive SBC directory
- **Linux Gizmos / CNXSoftware** — news sites that announce new Linux SBCs

---

## 3. Hack It — Repurposed Hardware

Many consumer electronics run Linux: routers, IP cameras, etc. The **OpenWRT** project targets many of these.

### Typical Device Formula
1. **Obscure processor** (typically MIPS or ARM)
2. **Small NAND or NOR flash** containing bootloader, kernel, rootfs
3. **One or two "cool" peripherals** (Wi-Fi, sensor, camera)
4. **Custom software** running within Linux

### Reverse Engineering Example: Reolink IP Camera
- **SoC:** Novatek NT9851x (Chinese MIPS, not hobbyist-purchasable)
- **Storage:** Small NOR flash on back of PCB
- **First step to hacking:** desolder flash → clone it (prevents bricking)
- **Debug headers:** Non-populated; probe with oscilloscope to find UART console
- **microSD footprint:** unpopulated but potentially solderable for loading tools

### Approach
1. Look for UART debug headers (serial console)
2. Clone the flash chip as a backup
3. Identify the SoC and look for Linux kernel support
4. Load tools via SD card or network if possible

---

## 4. Build It — Custom Boards

### Systems on a Module (SoM)
A SoM packages all complex parts (processor, RAM, storage) onto a single module. You build a simpler **carrier PCB** that the SoM mounts to.

| SoM | Architecture | RAM | Storage | Interface | Notes |
|-----|-------------|-----|---------|-----------|-------|
| **Onion Omega2S** | MIPS (MT7688) | 64MB | 16MB NOR | Castellated edges | Available on Mouser; built-in Wi-Fi |
| Onion Omega2 | Same | Same | Same | SBC form factor | Single-board cousin |

SoMs are a sweet spot: avoid ultra-fine-pitch soldering while still building custom hardware. Budget: **under $20**.

### Bare Processors (Systems in Package)
For space-constrained designs or those who enjoy a challenge. These include **built-in RAM** and come in solderable packages (QFP/QFN):

| Processor | Core | RAM | Package | Price | Notes |
|-----------|------|-----|---------|-------|-------|
| **Nuvoton NUC980** | ARM9 | Varies by pin count | QFP64–QFP216 | — | Great docs; many peripherals; "NuMaker Tomato" dev board |
| **Allwinner V3s** | ARM | 64MB | QFP | — | Powerful; **discontinued** but available on eBay/Taobao/AliExpress; some peripherals reverse-engineered |
| **Allwinner F1C100s** | ARM9 | 32MB | QFN | $1–1.40 | Very cheap; display driver included; middling Linux support; used in author's Linux business card |
| **Sunplus SP7021** | 4× ARM | 128MB or 512MB | QFP | ~$20 | Powers Banana Pi BPI-F2S; many peripherals |
| **Ingenic X1830** | MIPS | 128MB | BGA (0.65mm pitch) | — | Capable but BGA is harder to hand-solder |

---

## 5. Recommended Tools

### FTDI FT2232H Breakout Board
A "Swiss Army knife" for embedded hacking. Two independent channels, each can do:

| Tool | Function |
|------|----------|
| **Default** | UART serial console (channel A: ADBUS0=TXD, ADBUS1=RXD) |
| **flashrom** | SPI flash programming (unbrick routers, clone firmware) |
| **OpenOCD** | JTAG processor control and debugging |
| **Sigrok** | Low-speed logic analyzer |
| **ftdi-bitbang** | Manual pin control from command line |

Available cheaply on eBay/Amazon.

### SOP8 Test Clip
- For programming **SPI flash in-circuit** without desoldering
- Not needed for Raspberry Pi (boots from SD card only)
- Essential when working with SPI NOR/NAND flash devices

### Miscellaneous Supplies
- 0.1" header pins and breadboard wires
- Soldering iron
- USB cables
- Small SD cards (1GB or 2GB)

---

## 6. Shopping List (to Follow Along)

| Item | Notes |
|------|-------|
| **Raspberry Pi Zero W** | Get one **with header pins** for UART access |
| **microSD card** | Plus a way to plug it into your computer |
| **FT2232H breakout** | Any generic one works; author likes the purple "CJMCU" ones |
| **Breadboard wires** | For connecting FT2232H to Pi's serial console |

---

## 7. Key Takeaways

- **SBCs** (Raspberry Pi, OrangePi) are the easiest entry point
- **Repurposed devices** (routers, cameras) follow a predictable formula: obscure SoC + small flash + custom Linux software
- **SoMs** are ideal for custom hardware without fine-pitch soldering
- **Bare processors with built-in RAM** (V3s, F1C100s, NUC980) enable truly custom boards in hobbyist-friendly packages
- The **FT2232H breakout** is the single most versatile tool for embedded Linux hacking
- Always **clone flash before modifying** repurposed devices to prevent bricking
- The deeper you go, the more you'll encounter poorly-documented parts and overseas suppliers — patience and tinkering are essential
