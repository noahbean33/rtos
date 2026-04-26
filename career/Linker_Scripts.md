# Linker Scripts for Embedded Systems

## What Is a Linker Script?
The gateway between your compiled code (object files) and the actual memory layout on a microcontroller. It defines **where** code and data are placed in Flash and RAM.

## Compilation Flow
1. **C/C++ source** → preprocessor → compiler → assembler → **object files** (`.o`)
2. Object files contain: header, text segment, data segment, symbol table, relocation info, debug info
3. **Linker** (`ld`) + **linker script** (`.ld`) → combines object files into a single executable
4. Outputs: `.map` file (human-readable layout) + `.elf` (executable linkable format)
5. **`objcopy`** → converts ELF to raw **binary** that gets flashed onto the microcontroller

## Linker Script Structure

### `MEMORY` Section — Defines Physical Memory
```ld
MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}
```
- Specifies start address and size of each memory region
- Modify these if you upgrade hardware (e.g., larger Flash chip)

### `SECTIONS` Section — Maps Program Elements to Memory
| Section | Goes To | Contents |
|---|---|---|
| `.text` | Flash | Program code (machine instructions) |
| `.rodata` | Flash | Read-only data, constants |
| `.vectors` | Flash (first!) | Interrupt vector table — must be at start |
| `.data` | RAM (copied from Flash on boot) | Initialized global variables |
| `.bss` | RAM | Uninitialized/zero-initialized globals |
| `.stack` | RAM | Stack space for local variables |

## Key Points
- **Flash** is persistent through power cycles; **RAM** is cleared on power-off
- On boot, `.data` values are copied from Flash into RAM; `.bss` is zeroed
- The **interrupt vector table** must be the first thing in Flash — it initializes the system
- Stack size is configurable in the linker script
- You can create **custom sections** (e.g., `.dma_ram`) for special hardware needs
- To put code in RAM (faster execution): define a custom RAM text section in the linker script

## Practical Uses
- **Debugging crashes:** look up a faulting program counter address in the `.map` file to find the function
- **Memory budgeting:** check section sizes to see how much Flash/RAM is used
- **Custom placement:** force a variable to a specific address for hardware-mapped I/O
