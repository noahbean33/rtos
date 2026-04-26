# C/C++ Memory Allocation Model

## Four Main Memory Sections

### 1. Stack
- **Local variables**, function parameters, return addresses, saved registers
- Automatically managed — cleaned up when function goes out of scope
- Cannot reference a local variable from outside its function
- Used during context switches to save CPU state

### 2. Heap
- **Global** — accessible from any function
- Allocated **dynamically at runtime** with `malloc()` (uninitialized) or `calloc()` (zero-initialized)
- Must be manually freed to avoid memory leaks
- Not recommended for safety-critical embedded (see malloc lecture)

### 3. Static Memory
- **Global** — accessible from any function, but allocated **at compile time**
- Three sub-sections:
  - **`.bss`** — uninitialized or zero-initialized globals (compact storage, just a size)
  - **`.data`** — initialized globals (stores actual values)
  - **`.rodata`** — read-only constants and string literals

### 4. Text (`.text`)
- The actual **program code** (machine instructions)
- Stored in Flash on embedded systems

## Practical Examples

| Declaration | Location | Why |
|---|---|---|
| `int buffer[100];` (global, uninitialized) | Static → `.bss` | Global + uninitialized |
| `int value = 42;` (global) | Static → `.data` | Global + initialized |
| `static int x = 10;` (global) | Static → `.data` | Initialized, file-scoped |
| `const char *msg = "hello";` | Static → `.rodata` | String literal, read-only |
| `int *p = malloc(sizeof(int));` | Heap | Dynamic allocation |
| `static int counter = 0;` (inside function) | Static → `.bss` | Persists across calls; initialized once at compile time |

## Key Patterns
- **Return a pointer from a function:** must allocate on heap (not stack — stack gets destroyed)
- **Persist state across function calls:** use `static` local variable
- **Zero-initialized array:** use `calloc(n, sizeof(int))` on heap
- `static` local variables are initialized **once at compile time**, not on each function call
