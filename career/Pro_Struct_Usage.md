# Pro Struct Usage in Embedded C/C++

## Five Key Use Cases for Structs

### 1. Data Grouping
- Instead of passing 4+ separate arguments to a function, group them in a struct
- Pass a **pointer** to the struct — cleaner API, fewer parameters, more readable
- Logically related data should live together

### 2. Hardware Abstraction (Register Mapping)
- Hardware registers are at **contiguous memory addresses** separated by fixed offsets
- Define a struct with fields matching each register offset
- Assign the struct pointer to the **base address** → all fields auto-map to correct hardware addresses
```c
volatile TimerRegs *timer = (volatile TimerRegs *)0x40000000;
timer->control = 0x01;  // writes to base + 0
timer->reload  = 1000;  // writes to base + 4
```
- One assignment maps **multiple registers** — no need for separate address defines

### 3. Struct Padding & Memory Layout
- Compiler adds **padding bytes** for alignment (typically 4-byte boundaries)
- A struct with `uint32_t + uint16_t + uint8_t` = 7 bytes of data but **8 bytes** total (1 byte padding)
- **Why it matters:**
  - Network packets: both sides must have identical padding or data is corrupted
  - Memory-constrained devices: poor ordering can waste ~25% of RAM
- **`__attribute__((packed))`**: removes padding but slows memory access (multiple instructions needed)
- **Best practice:** order fields **largest to smallest** to minimize padding naturally

### 4. Linked Lists
- Struct containing data + a **pointer to the next node** of the same struct type
- Standard pattern for dynamic data structures in embedded C (task lists, message queues)

### 5. Type Punning with Unions
- **Union** overlays multiple struct/types on the **same memory**
- Example: a `pixel` union with `raw` (uint32_t) and `channels` (r, g, b, a as uint8_t)
- Write all 32 bits at once via `raw`, then read individual color channels
- Common in networking (packet headers) and graphics
- Total size = size of the largest member (they share the same memory)
