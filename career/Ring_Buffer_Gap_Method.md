# Ring Buffer (Gap Method) — Classic Embedded Interview Question

## Problem Statement
Implement a circular buffer for passing data between an **ISR (producer)** and a **main loop (consumer)** on a resource-constrained embedded system.

## Key Ideas

### Ring Buffer Basics
- Fixed-size array with **head** (read/pop) and **tail** (write/push) indices
- Both indices wrap around using **modulo** (`% capacity`)
- ISR pushes sensor data at the tail; main loop pops from the head

### The Gap Method (No Count Variable)
- **Problem:** without a count, head == tail could mean either empty or full
- **Solution:** always leave **one slot empty** — sacrifice 1 element of storage
- **Empty:** `head == tail`
- **Full:** `(tail + 1) % capacity == head`
- Trade-off: lose 1 byte of data storage to save 8 bytes (`size_t count` is 64-bit)

### Implementation

#### Init
- Set `buffer` pointer to caller-provided memory, `capacity` to given size
- Initialize `head = 0`, `tail = 0`

#### Push
1. Calculate `next = (tail + 1) % capacity`
2. If `next == head` → buffer full, return error (don't overwrite in this implementation)
3. `buffer[tail] = data`
4. `tail = next`

#### Pop
1. If `head == tail` → buffer empty, return error
2. `*data = buffer[head]`
3. `head = (head + 1) % capacity`

### Volatile for Shared Access
- `head` and `tail` must be declared **`volatile`** because:
  - Push (ISR) modifies `tail` and reads `head`
  - Pop (main loop) modifies `head` and reads `tail`
  - Prevents the compiler from caching these in registers — forces reads from main memory

### Overflow Handling Options
- **Overwrite oldest data:** keep pushing, let tail overtake head (good for "freshest data" use cases)
- **Block until space available:** stop pushing until head advances (no data loss)
