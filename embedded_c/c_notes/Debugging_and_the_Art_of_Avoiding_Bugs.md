# Debugging and the Art of Avoiding Bugs

> Based on a talk by **Eskil Steenberg** — C developer, member of the ISO C (WG14) board

---

## Table of Contents

1. [Debugging Is the Most Important Skill](#debugging-is-the-most-important-skill)
2. [Prioritize Your Bugs](#prioritize-your-bugs)
3. [Warnings and Errors: Configure Your Compiler](#warnings-and-errors-configure-your-compiler)
4. [Code Readability as Bug Prevention](#code-readability-as-bug-prevention)
5. [Features That Cause Bugs](#features-that-cause-bugs)
6. [Bug Severity Hierarchy](#bug-severity-hierarchy)
7. [Zero Initialization Is Harmful](#zero-initialization-is-harmful)
8. [Debug Mode vs. Release Mode](#debug-mode-vs-release-mode)
9. [Building Debug-Friendly APIs](#building-debug-friendly-apis)
10. [Writing a Memory Debugger](#writing-a-memory-debugger)
11. [Visual Debugging and Inspection Tools](#visual-debugging-and-inspection-tools)
12. [Deterministic Reproduction](#deterministic-reproduction)
13. [Debugging Techniques](#debugging-techniques)
14. [Indices vs. Pointers](#indices-vs-pointers)
15. [Testing Philosophy](#testing-philosophy)
16. [Key Takeaways](#key-takeaways)

---

## Debugging Is the Most Important Skill

> "Good code is not written, it's debugged."
> — Like the saying: good books are not written, they're rewritten.

Debugging is rarely taught in schools, has very few books about it, and yet it is the single most critical skill a programmer can develop.

---

## Prioritize Your Bugs

Not all bugs are equal. Focus on:

- **Bugs that are hard to find** — not the trivial ones that are obvious
- **Bugs that will cause serious damage** — data corruption, security, crashes in production
- **You can't check for everything** — if you try, you're not prioritizing, and you'll waste time looking for things that aren't there

---

## Warnings and Errors: Configure Your Compiler

### Don't Blindly Set All Warnings to Errors

```
-Werror  ← Many people think this is best practice. It's actually counterproductive.
```

**Why it's harmful:**
- Compiler writers become afraid to add new useful warnings (because they'll break builds)
- The best warnings are often NOT on by default for this reason
- You miss out on valuable diagnostic information

### Better Approach: Curate Your Warning List

```c
// Selectively promote warnings you care about to errors:
#pragma GCC diagnostic error "-Wimplicit-function-declaration"
#pragma GCC diagnostic error "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wunused-variable"  // Not a concern for you

// Build this list over time based on YOUR mistake patterns
```

Everyone writes different kinds of bugs. Configure your compiler to catch **your** specific weaknesses.

---

## Code Readability as Bug Prevention

### Debugging = Reconciling Your Mental Model with Reality

You think the program does X. It actually does Y. Your mental model is wrong. Debugging is the process of finding where your model diverges from reality.

> "The compiler almost always understands your code. You almost always don't."

### Code Communicates to Humans, Not Just Computers

```c
// Your code has TWO audiences:
// 1. The compiler (which almost always gets it right)
// 2. YOU (who almost always gets it wrong)
// → Optimize for #2
```

### Consistent Formatting Matters

```c
// Eskil's switch formatting — breaks are visually prominent:
switch (type) {
    case TYPE_A:
        handle_a();
    break;
    case TYPE_B:
        handle_b();
    break;
}
// "I very rarely forget a break because they are visually prominent."
```

Pick a style, keep it consistent. You'll read it faster and think less about syntax.

---

## Features That Cause Bugs

Things that look "elegant" but hide bugs:

| Feature | Problem |
|---|---|
| **Macros** | Hard to debug, confusing error messages |
| **Templates** | Obscure type errors, hard to trace |
| **Meta-programming** | Invisible code generation |
| **Name overloading** | Ambiguous which function is called |
| **Operator overloading** | `a * b` — dot product or pairwise multiply? |
| **Invisible jumps** | Destructors, discards, exceptions — code jumps you can't see |

### The JavaScript Trap

```javascript
// JavaScript: no declaration required
obj.hello = 42;    // writes to 'hello'
x = obj.Hello;     // reads from 'Hello' (capital H) — gets undefined/0
// No error. Hours of debugging for a typo.
```

```c
// C: must declare everything
typedef struct { int hello; } MyObj;
MyObj obj;
obj.hello = 42;
x = obj.Hello;    // COMPILE ERROR — 'Hello' doesn't exist
// Bug found in seconds, not hours.
```

> "Saving a few seconds of typing is a terrible trade for getting a more complicated bug. Typing is easy; debugging is hard."

---

## Bug Severity Hierarchy

From easiest to hardest to fix:

| Level | Type | Difficulty |
|---|---|---|
| 1 | **Compile error** | Instant — compiler tells you exactly what's wrong |
| 2 | **Tool error** (linter, static analysis) | Almost instant — tool points to the issue |
| 3 | **Runtime crash** (null deref, segfault) | Moderate — at least it stops where the problem is |
| 4 | **Wrong behavior** (works but does the wrong thing) | Hard — you may not even notice |
| 5 | **Silent corruption** (rarely triggers, non-deterministic) | Hardest — may never be found |

### Move Bugs Up the Hierarchy

```
Goal: Move bugs from Level 5 → Level 1

Runtime error → Compile error?  Great.
Silent corruption → Loud crash?  Good.
Non-deterministic → Deterministic?  Helpful.
```

Additional factors that make bugs harder:
- **Rare occurrence** — hard to reproduce
- **Non-deterministic** — different results each time
- **Stack corruption** — overwrites unrelated variables, causing chaos far from the source
- **Heap corruption** — generally easier to isolate than stack corruption

---

## Zero Initialization Is Harmful

### The Common Argument

> "If we zero-initialize everything, missed fields will have safe defaults."

### Why It's Wrong

```c
typedef struct {
    int   length;
    char *array;
} Data;

// If you forget to set 'array' but set 'length = 5':
//   length=5, array=NULL → dereference crash (if you're lucky)
//
// If you forget to set 'length' but set 'array':
//   length=0, array=valid → silently processes nothing — bug HIDDEN
```

Zero is a **valid value** for most fields. If a bug produces zero, it looks normal and hides.

### Better: Initialize to a Poison Value

```c
// In debug mode, initialize all memory to 0xCD
memset(&data, 0xCD, sizeof(data));

// Why 0xCD?
// - As a byte: 205 — unusual, recognizable value
// - As a pointer: 0xCDCDCDCD — unmapped address, instant crash on dereference
// - As an int: 3,452,816,845 — absurdly large, stands out in logs
// - All bytes are the same, so even partial reads show the pattern
```

### Real-World Example: The Hidden Reuse Bug

```c
void process(const char *a, const char *b) {
    Data d;
    memset(&d, 0, sizeof(d));  // zero-init (BAD)
    init_data(&d);              // BUG: forgets to set d.text = NULL

    if (a != NULL) d.text = a;
    print_data(&d);             // Works: prints a, or prints nothing if a==NULL

    memset(&d, 0, sizeof(d));  // "clear" for reuse
    init_data(&d);              // Still forgets d.text

    if (b != NULL) d.text = b;
    print_data(&d);             // BUG: if a!=NULL and b==NULL, prints a instead of nothing!
}
// The bug only manifests in 1 of 4 input combinations.
// With 0xCD init, it would crash immediately on the first call.
```

---

## Debug Mode vs. Release Mode

### They Are Two Completely Different Programs

| | Debug Mode | Release Mode |
|---|---|---|
| **Goal** | Find bugs as fast as possible | Run correctly for the user |
| **On error** | Crash hard, stop immediately | Gracefully continue if possible |
| **Checks** | Maximum validation, assertions | Minimal overhead |
| **Divide by zero** | Break at the point the divisor was created | Doesn't matter — shouldn't happen |

### Separate Your Debug Mode from the Compiler's

```c
// DON'T tie your debugging to the compiler's debug flag:
#ifdef DEBUG        // ← Compiler's debug mode
    validate(data);
#endif

// DO use your own independent flag:
#ifdef MY_MODULE_DEBUG    // ← Your module's debug mode
    validate(data);
#endif

// WHY: If a bug only happens at -O2, you need your debug tools
// available even in release/optimized builds.
```

### Example: Debug-Guarded Function

```c
void my_func(int a, int b) {
    #ifdef MY_DEBUG
    if (a > b) {
        fprintf(stderr, "ERROR: my_func called with a(%d) > b(%d) "
                "at %s:%d\n", a, b, __FILE__, __LINE__);
        MY_DEBUG_BREAK();  // Macro: can be exit(), breakpoint, or no-op
    }
    #endif

    // ... actual implementation
}
```

---

## Writing a Memory Debugger

A custom memory debugger is one of the most valuable tools you can build. It takes only a couple of days.

### How It Works

```c
// 1. Replace malloc/realloc/free with macros that inject __FILE__ and __LINE__
#define malloc(size)    debug_malloc(size, __FILE__, __LINE__)
#define realloc(p, s)   debug_realloc(p, s, __FILE__, __LINE__)
#define free(p)         debug_free(p, __FILE__, __LINE__)

// 2. For every allocation, record:
//    - Source file and line number
//    - Size requested
//    - Whether it's been freed
//    - A sentinel (magic number) before and after the allocation
```

### Capabilities

```c
// Print all allocations with their source locations:
debug_memory_print();
// Output:
//   main.c:42    — 256 bytes (allocated, not freed)
//   parser.c:88  — 1024 bytes (allocated, freed)
//   main.c:55    — 64 bytes (allocated, not freed)  ← LEAK

// Check for buffer overruns/underruns:
debug_memory_bounds_check();
// Output:
//   ERROR: Buffer overrun detected!
//   Allocated at parser.c:88 (1024 bytes)
//   Sentinel bytes corrupted at offset 1024

// Report total memory consumption:
size_t used = debug_memory_consumption();
```

### Sentinel Byte Layout

```
┌──────────┬────────────────────────┬──────────┐
│ SENTINEL │     User's Memory      │ SENTINEL │
│ (magic)  │   (requested size)     │ (magic)  │
└──────────┴────────────────────────┴──────────┘
             ↑ pointer returned to user

If user writes before or after their region → sentinel is corrupted
→ debug_memory_bounds_check() catches it
```

### Detected Errors

- Buffer overruns and underruns
- Double frees
- Freeing stack pointers
- Freeing offset pointers (not the start of an allocation)
- Memory leaks (allocated but never freed)
- Use after free
- Total memory consumption tracking

---

## Visual Debugging and Inspection Tools

### Build Your Own Investigation Kit

| Tool | Purpose |
|---|---|
| **Print functions** | Dump data structures in human-readable form |
| **Draw functions** | Visualize geometry, UI state, spatial data on screen |
| **Validation functions** | Assert that data structures are internally consistent |
| **Logging** | Record everything that happens for post-mortem analysis |
| **Save/Load** | Serialize problematic data structures for reproducible testing |

### Validation Functions: Pepper Them Everywhere

```c
// Write a function that checks ALL invariants of your data structure:
void mesh_validate(Mesh *mesh) {
    assert(mesh->polygon_count <= mesh->polygon_allocated);
    for (int i = 0; i < mesh->polygon_count; i++) {
        assert(mesh->polygons[i].vertex_a < mesh->vertex_count);
        assert(mesh->polygons[i].vertex_b < mesh->vertex_count);
        assert(mesh->polygons[i].vertex_c < mesh->vertex_count);
        // Check for inverted normals, degenerate triangles, etc.
    }
}

// Then sandwich every operation with validation:
mesh_validate(mesh);
mesh_subdivide(mesh);
mesh_validate(mesh);     // If this fails → bug is in mesh_subdivide
mesh_smooth(mesh);
mesh_validate(mesh);     // If this fails → bug is in mesh_smooth
```

### Use a Visual Debugger

> "Visual Studio is awesome for debugging. It's terrible for a bunch of things, but for debugging, it's the best."

Also consider: RAD Debugger, RemedyBG, White Box.

> "When choosing a language, start by asking: what kind of debugger exists for it?"

---

## Deterministic Reproduction

> "If you could tell the murderer 'come back Thursday and do it again,' murders would be easy to solve."

### Make Your Code Repeatable

- **Record all inputs** — if you can replay inputs, you can reproduce any bug
- **Make multithreading optional** — single-threaded mode eliminates an entire class of bugs
- **Use deterministic random seeds** — same seed → same behavior every run

```c
// Record inputs for replay:
typedef struct {
    double timestamp;
    int    event_type;
    int    key_or_button;
    float  x, y;
} InputEvent;

// Save to file during normal run, load during debug replay
```

### Disabling Multithreading for Debugging

Multithreading adds bugs but doesn't remove old ones. If you can toggle it off:

- Debug single-threaded bugs in isolation
- Then enable threading and debug only the threading-specific issues

---

## Debugging Techniques

### Dynamic Breakpoints (No-Op Trick)

```c
for (int i = 0; i < x; i++) {
    if (i == 999) {
        i += 0;   // ← Place breakpoint here. No-op, compiler removes in release.
    }
    do_something(i);
}
// Runs 999 times uninterrupted, then breaks on the problematic iteration.
```

**Why `i += 0` specifically:**
- Compiler optimizes it away in release builds (no runtime cost if forgotten)
- You always use the same pattern → searchable across your entire codebase
- Breakpoints travel with the source code, not the project file

### Static Debug Counter

```c
void problematic_function(Data *data) {
    static int debug_id = 0;
    debug_id++;

    if (debug_id == 42) {
        debug_id += 0;  // ← Breakpoint here
    }

    // ... function body that crashes on the 42nd call
}
// Step 1: Run until crash, note debug_id value (e.g., 42)
// Step 2: Change the condition to 42, re-run, break just before the crash
```

### Infinite Loop for Serialization Debugging

```c
#ifdef MY_DEBUG
size_t expected = compute_serialized_size(data);
size_t actual   = serialize_to_buffer(data, buffer);

if (expected != actual) {
    // Infinite loop: step through in debugger repeatedly
    while (1) {
        expected = compute_serialized_size(data);
        actual   = serialize_to_buffer(data, buffer);
    }
}
#endif
```

You can step through both functions side by side, as many times as needed, without restarting.

### Magic Numbers in Serialization

```c
// When packing binary data, insert markers between fields in debug mode:
void pack_debug(Stream *s, int value, const char *name) {
    #ifdef MY_DEBUG
    pack_magic(s, 0xDEADBEEF);
    pack_string(s, name);
    #endif
    pack_int(s, value);
}

void unpack_debug(Stream *s, int *value, const char *expected_name) {
    #ifdef MY_DEBUG
    uint32_t magic = unpack_magic(s);
    assert(magic == 0xDEADBEEF);  // Sync error if this fails
    char *name = unpack_string(s);
    assert(strcmp(name, expected_name) == 0);  // Wrong field if this fails
    #endif
    *value = unpack_int(s);
}
```

If you read/write one field too many or too few, the magic number check catches it immediately instead of silently corrupting all subsequent data.

---

## Indices vs. Pointers

### Prefer Indices for Debuggability

| Property | Index | Pointer |
|---|---|---|
| **Human-readable** | `47` — "the 48th element" | `0x7FFA3C20` — meaningless |
| **Deterministic** | Same index every run | Different address every run |
| **Off-by-one with unsigned 32-bit** | Wraps to 4GB offset → **instant crash** | Off by one byte → silent corruption |

### The Off-by-One Crash Trick

```c
// Using unsigned 32-bit indices on a 64-bit machine:
uint32_t index = 0;
index--;  // Wraps to 0xFFFFFFFF = 4,294,967,295

// Accessing array[index] jumps ~4GB away from the array
// → Almost certainly unmapped memory → INSTANT CRASH
// → Easy to find in debugger

// Compare with signed 64-bit:
int64_t index = 0;
index--;  // Becomes -1 → one byte before the array
// → Likely still in mapped memory → SILENT CORRUPTION
// → Nightmare to debug
```

> Use **`uint32_t`** for array indices on 64-bit systems — off-by-one errors become immediate crashes instead of silent corruption.

---

## Testing Philosophy

### Don't Worship Test Coverage

> "Test coverage is one of the stupidest metrics ever."

- If you're already using a function, you're already testing it
- Writing a test just to "cover" a function you're actively using adds no value
- **Write tests when you need them** — for combinatorial logic, parsers, edge cases

### When Tests Make Sense

```c
// A parser that handles thousands of input combinations:
// → Generate 1000+ test cases automatically
for (int i = 0; i < 1000; i++) {
    char *input = generate_random_valid_input(seed + i);
    ParseResult result = parse(input);
    assert(validate_result(result));
}
```

### Test Things You Don't Control

```c
// If you depend on an external library, test the boundary:
void test_external_lib_behavior(void) {
    // Verify assumptions about the library's behavior
    assert(external_lib_version() >= MINIMUM_VERSION);
    assert(external_parse("edge case") == EXPECTED_RESULT);
}
// If the library changes, this test catches it before your code breaks.
```

### Don't Run Tests That Can't Fail

If you haven't changed the code, it worked last time, it'll work this time. Don't slow your build with tests for unchanged code.

### When in Doubt: Write More Debug Code

> "A lot of people just stare at the code. Once you find you're not making progress, start writing debug code. Print values, detect states, validate structures. Even if it doesn't help now, it might help later."

---

## Key Takeaways

1. **Debugging starts when you write code**, not when something breaks
2. **Prioritize** — focus on bugs that are hard to find and cause serious damage
3. **Make bugs loud** — crashes are good; silent corruption is the enemy
4. **Initialize to poison values (0xCD)**, not zero — make uninitialized data obvious
5. **Separate debug/release modes** — and keep your debug tools independent of the compiler's debug flag
6. **Build your own tools** — memory debugger, validation functions, visualization
7. **Use a visual debugger** — it's the most important tool in your kit
8. **Make code deterministic and reproducible** — record inputs, disable threading for isolation
9. **Use `uint32_t` indices** on 64-bit systems — off-by-one errors crash instead of corrupting
10. **Write tests selectively** — for complex combinatorics and external dependencies, not for coverage metrics
11. **When stuck, write more code** — debug code, print statements, validators — anything beats staring
