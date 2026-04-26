# Basic Bit Manipulation for Firmware Engineers

## Why It Matters
- Required for writing drivers, bare-metal code, register/peripheral interaction
- Individual bits control different hardware functions — you must manipulate them safely
- Foundational for **every** firmware interview

## Core Operations

### Left Shift (`<<`)
- Shifts all bits left by N positions; discards overflow bits
- `x << 1` is equivalent to `x * 2`
- Used to position a bit at a specific location

### Set a Bit (OR)
```c
x |= (1 << position);
```
- Shifts a `1` to the target position, then ORs — sets that bit to 1 without touching others

### Clear a Bit (AND + NOT)
```c
x &= ~(1 << position);
```
- Creates a mask with all 1s except a 0 at the target position
- ANDing forces the target bit to 0, preserves everything else

### Toggle a Bit (XOR)
```c
x ^= (1 << position);
```
- XOR with 1 flips the bit: 0→1 or 1→0
- XOR truth: result is 1 if bits differ, 0 if same

### Check if Bit Is Set
```c
bool is_set = (x >> position) & 1;
```
- Shift the target bit down to position 0, AND with 1 to isolate it

### Count Set Bits (Population Count)
```c
int count = __builtin_popcount(x);
```
- GCC compiler intrinsic — maps to a single hardware instruction on most architectures

## Embedded Mindset
- Don't think in base-10 values — think in **individual bits in memory**
- Different bit ranges in a register may control different hardware functions
- e.g., bits 0–2 control one peripheral, bits 3–5 control another
- Always operate on specific bits without corrupting the rest of the register
