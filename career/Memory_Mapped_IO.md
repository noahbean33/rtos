# Memory-Mapped I/O — How Code Controls Hardware

## Core Concept
In ARM Cortex-M (and most MCUs), you control hardware by **writing values to specific memory addresses**. The processor's bus routes these writes to the appropriate peripheral hardware.

## Signal Path: Code → LED
1. **C code:** write `1` to a volatile pointer at the GPIO output data register address
2. **Assembly:** `STR R1, [R0]` — store value in R1 to memory address in R0
3. **AHB (Advanced High-Performance Bus):** data travels from CPU onto the system bus
4. **APB Bridge (Advanced Peripheral Bus):** routes to the peripheral bus (addresses starting with `0x4...`)
5. **GPIO peripheral block:** address decoder + AND gate checks if the address matches this peripheral
6. **D flip-flop:** latches the data bit, sends enable signal
7. **CMOS transistor:** outputs high voltage to the physical output pin
8. **LED lights up** (connected to that pin on the board)

## Professional GPIO Code Pattern

### Bad Way — Magic Numbers
```c
*(volatile uint32_t *)0x40020014 = 1;  // unreadable, unmaintainable
```

### Good Way — Struct-Based Register Mapping
```c
typedef struct {
    volatile uint32_t MODER;   // offset +0x00
    volatile uint32_t ODR;     // offset +0x14
    // ... other registers
} GPIO_TypeDef;

GPIO_TypeDef *GPIOA = (GPIO_TypeDef *)0x40020000;
GPIOA->MODER |= (1 << 10);   // configure as output
GPIOA->ODR   |= (1 << 5);    // set pin 5 high
```
- **Readable:** named fields instead of raw addresses
- **Flexible:** works for any GPIO port by changing the base address
- **Standard pattern** used in all professional embedded codebases and vendor HALs

## Key Details
- Use **`volatile`** on all hardware register pointers — prevents compiler from caching/optimizing away reads/writes
- The peripheral address space (e.g., `0x4000xxxx`) is routed by hardware to the peripheral bus
- XOR with a pin mask to **toggle** a pin: `GPIOA->ODR ^= (1 << 5);`
