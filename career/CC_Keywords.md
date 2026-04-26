# Important C/C++ Keywords for Embedded Programming

## Keywords & Their Purpose

### `volatile`
- Tells the compiler: **always load from memory**, never cache in a register
- Use when a variable can be modified externally (ISR, another thread, another core, hardware register)
- Prevents stale data in multi-threaded or multi-core systems (e.g., shared flag on ESP32 dual-core)

### `const`
- Variable won't be modified → compiler can store in **read-only memory (Flash)**
- Enables caching in registers for faster access
- Saves stack/heap space by moving data to static read-only memory

### `restrict` (C only, not C++)
- Declares that a pointer is the **only reference** to its memory location
- Enables compiler optimizations by preventing pointer aliasing
- OS/compiler doesn't need to handle multiple pointers to the same address

### `_Atomic` / `atomic` (C11 / C++)
- Ensures proper **memory ordering** across threads
- Only one thread can modify the variable at a time (implements mutex-like behavior under the hood)
- Prevents race conditions — read operations complete before writes can begin
- Low overhead; commonly used in embedded multi-threaded code

### `inline`
- Suggests the compiler **expand the function body** at each call site (no branching/jumping)
- Critical for **ISRs** — you want non-branching, non-jumping code inside interrupt handlers
- Caveat: don't inline large functions — text section grows exponentially if called in many places
- Only beneficial for small, frequently-called functions

### `static`
- **On a function:** limits scope to the declaring file (not visible to other translation units)
- **On a local variable:** stored in static memory (BSS/data), persists across function calls
  - Not on the stack — survives after function scope ends
  - Initialized only once at compile time, not each function call

### `extern`
- Declares that a variable is **defined and allocated elsewhere** (another file or linker script)
- Tells the compiler to look for it during linking, not in the current translation unit
