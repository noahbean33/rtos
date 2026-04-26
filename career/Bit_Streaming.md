# Bit Streaming — Firmware C Interview Question

## Problem Statement
Write a utility function that processes a stream of 8-bit sensor readings:
- Extract the **5 least significant bits** of each input byte
- Pack them **contiguously** (no gaps) into an output buffer, starting from the MSB of each output byte

## Key Ideas

### Memory Allocation Strategy
- **Never use `malloc`** in embedded/firmware — it's unpredictable on memory-constrained devices
- The **caller** should be responsible for allocating the output buffer (typically statically allocated)
- Memory goes in the **BSS section** (static, uninitialized) — not heap, not stack
- Modified the function signature to accept an `output` pointer and `output_length` from the caller

### Bit Manipulation Approach
1. **Mask** each input byte with `0x1F` to extract the 5 LSBs
2. **Iterate** bit-by-bit through the masked data
3. **Select** individual bits: `(post_masked_data >> j) & 1`
4. **Pack** into the output buffer using OR and shifting: `output[current_byte] |= (bit_out << current_bit)`
5. Track `current_byte` and `current_bit` (starting at 7, decrementing); when `current_bit < 0`, advance to next byte and reset to 7

### Interview Tips
- Get a **simple, correct solution first** — optimize later
- Talk through memory allocation decisions with the interviewer
- This problem tests **bit manipulation** and **memory management** — two firmware fundamentals
