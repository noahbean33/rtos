# C++ Template Programming for Embedded Systems

## Problem
Writing to a GPIO register to turn on a peripheral (e.g., an LED). Comparing a standard C approach vs. a C++ template approach in terms of **code size** and **performance**.

## Key Ideas

### C Approach — The Problem
- Define a GPIO struct and macros for port address and pin
- Use an `inline` function that takes `port` and `pin` as runtime arguments
- **Issue:** `inline` in C is only a *suggestion* — the compiler may not inline it
- Result: compiler generates a **function call** with branching overhead
- Assembly output: **~31 lines**, includes `call` instructions (expensive context switch, stack save)

### C++ Template Approach — The Solution
- Use a **template class** parameterized on port address and pin mask
- Values are compile-time constants → compiler can fully inline and verify
- The `on()` method takes **no runtime arguments** — everything is baked into the template
```cpp
template<uint32_t PORT_ADDR, uint32_t PIN_MASK>
class GPIO {
    void on() { /* write PIN_MASK to PORT_ADDR offset */ }
};
using LED = GPIO<GPIOA_BASE, PIN_5>;
LED::on();
```

### Assembly Comparison (RISC-V 32-bit, -O2)
| Metric | C | C++ Template |
|---|---|---|
| Assembly lines | ~31 | ~14 |
| Function calls | Yes (`call`) | None |
| Instructions | Loads, stores, branches | Only adds, stores, loads |

### Why Templates Win
- **No function call overhead** — no branching, no stack frame creation
- All values known at compile time → compiler generates minimal, direct register writes
- Can add `static_assert` for compile-time validation of port/pin values
- **2x reduction in code size** for this use case

### Takeaway
- C++ templates give you **zero-cost abstractions** — the generics are resolved at compile time
- Especially useful in embedded where code size (flash) and speed matter
- Consider C++ templates when you have compile-time-known hardware constants
