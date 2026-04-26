# Advanced C Programming — Sensor Data Reading

## Problem Context
Reading data from an external sensor on a microcontroller — comparing a naive busy-poll approach vs. a production-grade DMA-based architecture.

## Key Ideas

### The Bad Way (Busy Polling)
- Global data array — anything can modify it (unsafe)
- Magic numbers for hardware addresses (unreadable)
- CPU stuck in a blocking `while` loop waiting for a SPI register flag
- CPU wastes cycles polling instead of doing useful work

### The Good Way (DMA + Professional Architecture)
- **DMA (Direct Memory Access):** offloads data transfer from CPU
  - DMA controller reads the SPI data register independently
  - Pushes data into an RX buffer in RAM
  - CPU just grabs data when ready — **zero wait, deterministic read**
- **RX buffer** placed in a DMA-accessible RAM section via `__attribute__((section(".dma_ram")))`

### Production-Grade Patterns Used
- **X Macros** for clean enum/error-string generation from a single source of truth
- **Opaque pointers** to hide internal data — no exposed globals
- **Atomic try-lock** to prevent simultaneous DMA write and CPU read (mutual exclusion)
- **Graceful error handling** with error enums and string lookups
- Separate **sensor driver header** (public API) and **private source** (implementation)

### Architecture Comparison
| Aspect | Before | After |
|---|---|---|
| Data access | Open global array | Opaque pointer, locked access |
| CPU usage | Busy-poll (blocking) | DMA (non-blocking) |
| Readability | Magic numbers | Named registers, enums |
| Scalability | Poor | Clean abstraction, portable |
