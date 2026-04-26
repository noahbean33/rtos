# RTOS Task Scheduler — O(1) Priority Scheduling

## Problem Statement
Implement a priority-based task scheduler that can find the highest-priority ready task in **O(1) time** using a bitmap and a compiler intrinsic.

## Key Ideas

### Data Structure: Priority Bitmap
- A **32-bit integer** where each bit represents a priority level (0–31)
- MSB = highest priority, LSB = lowest priority
- Bit set to `1` = a task at that priority is ready to run
- Each priority level points to a **task control block (TCB)** (linked list of tasks at that level)

### Three Core Operations

#### 1. Set Task Ready
```c
ready_task_bitmap |= (1 << priority);
```
- Sets the bit corresponding to the given priority level

#### 2. Clear Task Ready
```c
ready_task_bitmap &= ~(1 << priority);
```
- Clears the bit using AND with inverted mask — preserves all other bits

#### 3. Get Highest Priority Task — O(1)
```c
int leading_zeros = __builtin_clz(ready_task_bitmap);
return 31 - leading_zeros;
```
- `__builtin_clz()` — GCC built-in that maps to a **single CPU instruction** (Count Leading Zeros)
- Counts zeros from the MSB → gives distance to the highest set bit
- Subtract from 31 to get the priority index
- **Single clock cycle** on most architectures — true O(1)

### Why Not O(n)?
- A naive linear scan checks each bit left-to-right → O(n)
- The CLZ hardware instruction does it in **one cycle** regardless of bitmap value

### Scaling Beyond 32 Priorities
- For 1024 priorities, you can't fit in a single 32-bit value
- Possible approach: hierarchical bitmap (group of bitmaps with a summary bitmap on top)
