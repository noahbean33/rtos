# Classic Bit Manipulation with Compiler Acceleration

## Two Problems Covered

### 1. Swap Endianness (Big ↔ Little Endian)
- Move byte 3 → byte 0, byte 2 → byte 1, byte 1 → byte 2, byte 0 → byte 3
- Implementation: shift each byte to its new position, mask with `0xFF` at the target location, OR all results together
```c
result = ((val >> 24) & 0x000000FF) |
         ((val >> 8)  & 0x0000FF00) |
         ((val << 8)  & 0x00FF0000) |
         ((val << 24) & 0xFF000000);
```

### 2. Count Set Bits (Population Count)

#### Explicit SIMD-Style Approach
Accumulate counts at progressively wider bit groups:
1. Count set bits in each **2-bit** pair → store in those 2 bits
2. Accumulate into each **4-bit** nibble
3. Accumulate into each **byte**
4. Sum all bytes into byte 3, then shift to get total

Uses clever masks: `0x55555555`, `0x33333333`, `0x0F0F0F0F`, and a multiply-and-shift trick.

- Safe from overflow: max value is 32, fits in a single byte (max 255)

#### One-Line Compiler Intrinsic
```c
__builtin_popcount(val)
```
- GCC/Clang maps this to a **single hardware instruction**:
  - ARM64 (Apple M-series): `CNT` vector instruction
  - x86: `POPCNT` instruction
- One clock cycle — no loops needed

## Key Takeaway
- The explicit approach is good to understand and discuss in interviews
- The compiler intrinsic is what you'd use in production — leverages hardware acceleration
- Know both: interviewers may ask you to implement it manually, then optimize
