# Stop Using Floats! — Fixed-Point vs. Floating-Point in Embedded C

## The Difference

### Floating-Point
- Stored as: **sign bit + exponent + mantissa**
- Exponent determines decimal place location (floating)
- Multiplication requires: extract sign/exponent/mantissa → multiply mantissas → determine sign → normalize → shift → round → check overflow
- Can take **up to 150 cycles** on processors without hardware FPU (e.g., STM32 Cortex-M0)

### Fixed-Point
- Stored as a **plain integer** with an implied fixed decimal position
- Multiplication: integer multiply → right-shift to realign decimal → optional truncation/rounding
- As few as **~5 cycles** — just a multiply + shift (both single instructions)

## Benchmark Results (1000 iterations)

| Microprocessor | Fixed-Point vs. Float | Why |
|---|---|---|
| **ESP32** | Float is **7x faster** | Dedicated hardware FPU — float multiply is a single native instruction |
| **Raspberry Pi Pico** (Cortex-M0) | Fixed-point is **1.4x faster** | No hardware FPU, but RP2040 has optimized soft-float assembly in ROM |
| **STM32 L03** (Cortex-M0) | Fixed-point is **2.3x faster** | No hardware FPU, uses default GCC soft-float library (unoptimized) |

## Key Takeaways
- **Don't assume** fixed-point is always faster — it depends on the processor's hardware
- **ESP32:** has a hardware FPU → just use floats, simpler code
- **STM32 Cortex-M0:** no FPU → fixed-point gives the biggest speedup (2.3x)
- **RPi Pico:** same Cortex-M0 as STM32 but Raspberry Pi engineers added optimized soft-float routines in ROM → narrows the gap
- When choosing a microprocessor for performance-critical math:
  1. Check if it has a **hardware FPU**
  2. Check if the vendor provides **optimized soft-float libraries**
  3. If neither → use **fixed-point arithmetic** for significant speedups
