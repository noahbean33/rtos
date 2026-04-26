# Why We Never Use malloc() in Embedded Systems

## The Problem with Dynamic Memory Allocation

### Heap Fragmentation ("Swiss Cheese Effect")
1. Allocate blocks A(256B), B(128B), C(256B), D(128B) from a 1024B heap
2. Free B and D → 512 bytes free, but in two **non-contiguous 128B holes**
3. Attempt to allocate 300 bytes → **fails** despite 512 bytes "free"
4. Results in hard fault or system reboot

### Two Core Issues
- **Space:** fragmentation makes available memory unpredictable at compile time
- **Time:** searching for a free block grows **O(n)** — non-deterministic execution time
- Both are unacceptable for safety-critical systems (medical devices, etc.) that must behave identically for decades

## Solutions

### 1. Static Memory Allocation
- Declare a fixed buffer at compile time: `static uint8_t sensor_buffer[512];`
- Goes in the static/BSS memory section — reserved for the program's entire lifetime
- Trades memory efficiency for **determinism and safety**
- You know at compile time exactly how much memory is used

### 2. Memory Pooling (Custom Allocator)
- Pre-allocate a fixed number of **equal-sized blocks** (e.g., 10 × 64-byte blocks)
- Each block has an `is_used` flag
- To allocate: scan pool for unused block, mark as used, return pointer
- **No fragmentation** — all blocks are the same size; freed blocks are immediately reusable
- Runs in **O(1)** time (fixed pool size)
- Provides `malloc`-like flexibility with static memory safety
