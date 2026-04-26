# Advanced C: Undefined Behavior and Optimizations That Trick Good Programmers

> Based on a talk by **Eskil Steenberg** (Swedish representative in ISO WG14 — the C standard working group)

---

## Table of Contents

1. [C Is Not High-Level Assembly](#c-is-not-high-level-assembly)
2. [The As-If Rule](#the-as-if-rule)
3. [Observable Behavior and `volatile`](#observable-behavior-and-volatile)
4. [Multithreading: Volatile, Atomicity, and Acquire/Release](#multithreading-volatile-atomicity-and-acquirerelease)
5. [Undefined Behavior (UB)](#undefined-behavior-ub)
6. [UB-Driven Optimizations](#ub-driven-optimizations)
7. [Integer Promotion Traps](#integer-promotion-traps)
8. [Null Pointer Time Travel](#null-pointer-time-travel)
9. [Uninitialized Values and Indeterminate State](#uninitialized-values-and-indeterminate-state)
10. [Struct Padding and Optimization](#struct-padding-and-optimization)
11. [The Memory Model and Type Aliasing](#the-memory-model-and-type-aliasing)
12. [The `restrict` Keyword](#the-restrict-keyword)
13. [Pointer Provenance](#pointer-provenance)
14. [Survival Tips](#survival-tips)

---

## C Is Not High-Level Assembly

A common misconception is that C is just a thin wrapper over assembly. In reality:

- **Compilers perform extensive transformations** — what you write is not what gets executed.
- **CPUs also reorder and transform** instructions at runtime (out-of-order execution, branch prediction, etc.).
- Even assembly language doesn't tell the full story of what your code does on modern hardware.

---

## The As-If Rule

The compiler may transform your code in **any way it wants**, as long as the **observable end result** is the same. This is called the **as-if rule**.

### Dead Store Elimination

```c
x = 0;
x = 1;
// Compiler can remove "x = 0;" — x is eventually 1 either way.
```

### Reordering Independent Statements

```c
a = 0;
b = 1;
// Compiler may reorder these — the end result is identical.
```

### Non-Reorderable: Observable Side Effects

```c
printf("hello");
printf("goodbye");
// Cannot be reordered — the output is observable.
```

> **Benchmarking Warning:** A loop that overwrites the same value repeatedly may be collapsed into a single write by the compiler.

---

## Observable Behavior and `volatile`

**Observable** means the result can be noticed outside the program (I/O, hardware interaction, etc.).

The `volatile` keyword tells the compiler: *"This variable may change or be read by something external — do not optimize accesses to it."*

### Preventing Dead Store Elimination

```c
volatile int a;
a = 0;  // Must execute — observable event
a = 1;  // Must execute — observable event
```

### Preventing Read Coalescing

```c
volatile int x;
int r1 = x;  // Must read from x
int r2 = x;  // Must read again — x may have changed between reads
```

### Preventing Loop Hoisting

```c
// Without volatile:
while (a) { printf("looping"); }
// Compiler can transform to: if (a) { while(1) printf("looping"); }

// With volatile:
volatile int a;
while (a) { printf("looping"); }
// Compiler must re-check 'a' every iteration.
```

### Preventing Reordering

```c
volatile int a, b;
a = 0;
b = 1;
// Order is preserved — both writes are observable.
```

### Common Uses of `volatile`

- **Hardware registers** — memory-mapped I/O, microcontroller pins
- **Memory-mapped files** — another process can modify the content
- **"Touch" reads** — pre-faulting memory into RAM before acquiring a lock
- **Multithreading** — (with caveats; see next section)

---

## Multithreading: Volatile, Atomicity, and Acquire/Release

When sharing data between threads, three properties are needed:

| Property | Meaning |
|---|---|
| **Volatile** | The value may change externally — don't cache reads/writes |
| **Atomic** | The read/write happens as one indivisible operation (no half-writes) |
| **Acquire/Release** | Establishes ordering guarantees across threads |

### Why `volatile` Alone Is Not Enough

```c
// Thread 1:
char *data = malloc(100);
memset(data, value, 100);
volatile shared_ptr = data;   // volatile ensures the write happens
// BUT: the compiler can still reorder memset AFTER the pointer write
// if memset's target isn't volatile.
```

### Atomic Operations

```c
// A simple pointer write may be atomic on x86, but NOT on all architectures.
// Always use atomic functions for portability and clarity:
atomic_exchange(&shared_ptr, data, memory_order_release);
```

### Acquire and Release Semantics

```
RELEASE: Everything I did BEFORE this point must complete before the release.
         (Things after can be moved up, but nothing moves down past a release.)

ACQUIRE: Nothing AFTER this point can execute until the acquire completes.
         (Things before can be moved down, but nothing moves up past an acquire.)

FULL FENCE: Both acquire + release. Nothing moves in either direction.
```

> **Critical:** Acquire/release are not just compiler hints — they generate **different CPU instructions** that flush caches and ensure memory visibility across cores.

---

## Undefined Behavior (UB)

The C standard has **six pages** of listed undefined behavior (growing with each revision). UB includes:

- Explicitly listed cases (e.g., calling `exit()` more than once)
- Anything not defined by the standard at all

### Why Is UB Left Undefined?

1. **Enables powerful compiler optimizations**
2. **Reduces implementation burden** — e.g., `exit()` doesn't have to be thread-safe
3. **Portability** — allows C to run on diverse hardware without imposing specific behavior

---

## UB-Driven Optimizations

### Signed Overflow Elimination

```c
int x;
x = x * 2 / 2;
// Signed overflow is UB → compiler assumes it won't happen
// → "x * 2 / 2" is optimized to just "x"

// For unsigned int, wrapping IS defined → compiler cannot optimize this away
```

### Assumed Absence of UB (Out-of-Bounds)

```c
int a[5];
a[x] = 0;
// Compiler can assume x is in [0, 4] — otherwise UB
// Any later test like "if (x >= 5)" can be removed as unreachable
```

### Divide-by-Zero Inference

```c
int result = a / x;
if (x == 0) { /* handle error */ }
// Compiler: "x can't be 0 (divide-by-zero is UB), so remove the check"
```

### `__builtin_unreachable()` / `__assume()`

```c
switch (a) {
    case 0: func0(); break;
    case 1: func1(); break;
    case 2: func2(); break;
    case 3: func3(); break;
    default: __builtin_unreachable();
}
// Compiler can use a direct jump table without bounds checking
```

### Wrap-Around Check Elimination

```c
int a;
if (a + 1 > a) { a = a + 1; }
// Compiler: signed overflow is UB → a+1 is ALWAYS > a → removes the if
```

---

## Integer Promotion Traps

One of the most surprising real-world bugs:

```c
void func(unsigned short a, unsigned short b) {
    unsigned int x = a * b;
    if (x >= 0x80000000u) {
        printf("%u is large\n", x);
    } else {
        printf("%u is small\n", x);
    }
}

func(65535, 65535);
// Expected: prints "large" (65535 * 65535 = 4,294,836,225)
// Actual:   prints "small"!
```

**Why?** The multiplication promotes `a` and `b` to **signed `int`** (because all `unsigned short` values fit in a signed int). The signed result is then converted to `unsigned int`, but the compiler already inferred the value can't exceed `INT_MAX` — so it eliminates the `>= 0x80000000` branch.

> **Rule:** Unsigned types can be promoted to signed types if all values fit. Be extremely careful with arithmetic on types smaller than `int`.

---

## Null Pointer Time Travel

```c
*x = 42;               // dereference x
if (x == NULL) {        // check if x is null
    printf("error");
}
// Compiler: "x was dereferenced above, so x can't be NULL (that would be UB)"
// → removes the null check entirely
```

Even scarier — **time travel**:

```c
if (x == NULL) { printf("error"); }
*x = 42;    // dereference happens AFTER the check
// Compiler: these two statements don't interact with each other
// → reorders: *x = 42 first → x can't be NULL → removes the if
// The bug becomes visible BEFORE the UB-causing line
```

---

## Uninitialized Values and Indeterminate State

An uninitialized variable is in an **indeterminate state** — it is NOT simply "unknown bits."

```c
int foo(void) {
    int a;           // uninitialized
    if (a == 0)
        return 0;
    return 1;
}
// LLVM compiles this to just "return" — no check, no value.
// An indeterminate value can behave as BOTH zero and non-zero.
```

### Memory Can Change Between Reads

```c
char *buffer = malloc(1);
if (!buffer) return;
volatile char c1 = buffer[0];   // read 1
volatile char c2 = buffer[0];   // read 2
// c1 and c2 may differ! The OS may reclaim unwritten pages between reads.
```

> **Rule:** Never assume uninitialized memory is stable. Always initialize before reading.

---

## Struct Padding and Optimization

```c
typedef struct {
    int   a;   // 4 bytes
    char  b;   // 1 byte + 1 byte padding
    short c;   // 2 bytes
} MyStruct;    // Total: 8 bytes (with padding)
```

### `memset` Enables Better Optimization

```c
// Without memset — compiler writes a, b, c separately (3 instructions):
MyStruct s;
s.a = 1; s.b = 2; s.c = 3;

// With memset — compiler can write the entire struct in one 64-bit store:
MyStruct s;
memset(&s, 0, sizeof(s));
s.a = 1; s.b = 2; s.c = 3;
```

The `memset` tells the compiler you don't care about padding values, enabling a single wide write.

> **Warning:** Padding bytes are undefined. The compiler may overwrite them as a side effect of optimizing adjacent writes.

---

## The Memory Model and Type Aliasing

### The Aliasing Problem

```c
void clear(int *array, int *count) {
    for (int i = 0; i < *count; i++)
        array[i] = 0;
}
// Compiler CANNOT replace this with memset — what if array and count overlap?
// Writing to array[i] might modify *count!
```

### Type-Based Alias Analysis (Strict Aliasing)

```c
void clear(float *array, int *count) {
    for (int i = 0; i < *count; i++)
        array[i] = 0.0f;
}
// float* and int* point to different types → compiler assumes no aliasing
// → CAN read *count once and use memset
```

### Strict Aliasing Violation via Unions

```c
union { unsigned int a; float b; } convert;
convert.b = 3.14f;
printf("%u", convert.a);   // LEGAL: reading through union member

unsigned int *p = &convert.a;
convert.b = 3.14f;
printf("%u", *p);           // DANGEROUS: compiler may assume p and convert.b don't alias
```

### Exception: `unsigned char` Aliases Everything

```c
// unsigned char is treated as raw memory — aliases all types.
// This is why memcpy (which takes unsigned char*) works on any data.
```

---

## The `restrict` Keyword

An explicit promise that two pointers do not alias:

```c
void clear(int *restrict array, int *restrict count) {
    for (int i = 0; i < *count; i++)
        array[i] = 0;
}
// With restrict: compiler can safely read *count once → optimize to memset
// Breaking this promise is UB.
```

---

## Pointer Provenance

Memory objects do **not** have a defined spatial relationship to each other in C.

### Cannot Compute Relationships Between Objects

```c
int a, b;
int *p = &a + 1;       // one past 'a'
if (p == &b) {          // are they adjacent?
    printf("same\n");
}
// Compiler: p comes from 'a', it can never point to 'b'
// → may remove the if entirely, even if addresses match at runtime
```

### Dead Pointers Lose Their Identity

```c
int *a = malloc(4);
free(a);
int *b = malloc(4);
if (a == b) {           // same address reused?
    b = malloc(4);      // get a different one
    free(b);
}
// Compiler: 'a' is freed → its value is meaningless → may remove the if
```

> **Workaround:** Cast pointers to `uintptr_t` for comparison after freeing. But **do not** cast integers back to pointers.

### The ABA Problem

A classic lock-free multithreading bug:

1. Thread 2 reads pointer `a`
2. Thread 1 frees `a`, allocates `b`, then allocates `c` (which reuses `a`'s address)
3. Thread 2 compares and sees the pointer hasn't changed — but the data has

---

## Survival Tips

| Tip | Details |
|---|---|
| **Avoid bit fields** | Promotion rules are obscure and error-prone |
| **Avoid VLAs** | Variable-length arrays are poorly supported and no longer required by the standard |
| **Don't be clever with the preprocessor** | Use `#include` and `#define` — stop there |
| **Use a visual debugger** | Visual Studio, RAD Debugger, etc. |
| **Use Quick Watch** | Explore memory interactively in Visual Studio |
| **Use sanitizers and Valgrind** | Catch UB, memory errors, and leaks |
| **Use GFlags (Windows)** | Places allocations at page boundaries to catch overflows instantly |
| **Write your own memory debugger** | Wrap `malloc`/`free` with file + line tracking and sentinel bytes |
| **Avoid dynamic arrays on the stack** | Stack overwrites are extremely hard to debug |
| **Read MISRA C** | Industry standard for safe, secure C |
| **Use Compiler Explorer (godbolt.org)** | See what the compiler actually generates |
| **Learn basic assembly** | Understand what optimizations produce |
| **Read compiler flags** | Know what `-O2` actually enables (e.g., GCC defines writes to NULL at `-O2`+) |
| **Read *What Every Programmer Should Know About Memory*** | Ulrich Drepper's essential deep-dive |
