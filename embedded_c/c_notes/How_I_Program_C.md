# How I Program C

> Based on a talk by **Eskil Steenberg** — prolific C developer, author of numerous open-source libraries and games

---

## Table of Contents

1. [Philosophy: Control Over Convenience](#philosophy-control-over-convenience)
2. [Small Technology Footprint](#small-technology-footprint)
3. [Code Is for Humans to Read](#code-is-for-humans-to-read)
4. [Explicitness Over Cleverness](#explicitness-over-cleverness)
5. [Naming Conventions](#naming-conventions)
6. [Function Length and Sequential Code](#function-length-and-sequential-code)
7. [API Design](#api-design)
8. [File and Module Organization](#file-and-module-organization)
9. [Object Orientation in C](#object-orientation-in-c)
10. [Opaque Handles with Void Pointers](#opaque-handles-with-void-pointers)
11. [Macros: When They're Acceptable](#macros-when-theyre-acceptable)
12. [The `__FILE__` and `__LINE__` Macros](#the-__file__-and-__line__-macros)
13. [Memory: How Pointers Really Work](#memory-how-pointers-really-work)
14. [Smart `sizeof` Usage](#smart-sizeof-usage)
15. [Struct Alignment and Padding](#struct-alignment-and-padding)
16. [Struct-Based Inheritance in C](#struct-based-inheritance-in-c)
17. [Memory Blocks and Virtual Memory](#memory-blocks-and-virtual-memory)
18. [`realloc` Is Awesome](#realloc-is-awesome)
19. [GFlags: Finding Memory Corruption](#gflags-finding-memory-corruption)
20. [Memory Is Slow, Math Is Fast](#memory-is-slow-math-is-fast)
21. [Linked Lists Are Terrible](#linked-lists-are-terrible)
22. [Dynamic Arrays with `realloc`](#dynamic-arrays-with-realloc)
23. [Don't Store Data Twice](#dont-store-data-twice)
24. [Advanced Allocation Tricks](#advanced-allocation-tricks)
25. [Stride: Flexible Data Access](#stride-flexible-data-access)
26. [Build the Mountain](#build-the-mountain)
27. [Fix Your Code Now](#fix-your-code-now)
28. [Immediate-Mode UI](#immediate-mode-ui)
29. [Useful Bit Tricks](#useful-bit-tricks)

---

## Philosophy: Control Over Convenience

> "In the beginning you always want results. In the end, all you want is control."

Like building a web page — at first you just want text on screen. Eventually you want to control every pixel. The complexity you resisted at first becomes exactly what you need.

### The Kitchen Analogy

- **Approach A:** "Just make me an omelet" — you get one thing, fast
- **Approach B:** "What does this kitchen have?" — you discover ingredients and tools, and eventually create something extraordinary

### Garbage Collection Example

```c
// With garbage collection:
// - Easy at first: just allocate, don't worry about freeing
// - Eventually: GC pauses at the wrong time, no control over when memory is freed
// - You wish you had free()

// With manual memory management:
// - More work upfront
// - Total control over when memory is allocated and freed
// - ~20 minutes per year finding memory leaks (with good tools)
```

> If you have the power to control memory, memory leaks are a solvable problem. Without that control, you may hit a wall you can't fix.

---

## Small Technology Footprint

```
"I use C89. C99 is still too new. C11 is super new and isn't implemented everywhere."
```

### Principles

- **Zero unwrapped dependencies** — every external API is wrapped so you control 100% of your code
- **Code should run on any compiler** — the simpler your requirements, the fewer things can break
- **Code should last decades** — some of Eskil's code from 15+ years ago still works perfectly

> Every dependency is a risk. Companies go bankrupt, APIs change, platforms disappear. If you wrap everything, you control your fate.

---

## Code Is for Humans to Read

> "You spend more time reading your own code than the compiler does."

Code has **two audiences**:
1. The compiler — almost always understands what you wrote
2. You — almost always wrong about what you wrote

Therefore:
- **Ambiguity is the enemy** — if the compiler has to guess, you'll misread it too
- **Compiler errors are your friend** — they catch your mistakes before they become bugs
- **More errors = better communication** from the compiler

---

## Explicitness Over Cleverness

> "Cleverness is the root of all evil — not premature optimization."

### Operator Overloading Is Harmful

```c
// C++ with operator overloading:
Vector3 result = a * b;
// Is this a dot product or pairwise multiplication?
// Half the room says one thing, half says the other.

// Explicit C:
float   dot_result  = vec3_dot(a, b);
Vector3 pair_result = vec3_mul_pairwise(a, b);
// Impossible to confuse.
```

### Function Overloading Hides Bugs

```cpp
// C++ overloaded functions:
int   add(int a, int b);
float add(float a, float b);
double add(double a, double b);

add(4, 0.3);  // Which overload? int! 0.3 becomes 0. Adds 4 + 0 = 4.
               // You expected float addition. Silent bug.
```

```c
// Explicit C:
int    add_i(int a, int b);
float  add_f(float a, float b);
double add_d(double a, double b);

add_f(4, 0.3f);  // Crystal clear. Warning if types mismatch.
```

> One extra character (`_f`) saves hours of debugging.

### Crashes Are Good

```c
// If software has a bug:
// BAD:  Silently logs to a file nobody reads, data slowly corrupts
// GOOD: Crashes hard, debugger stops at the exact line, fix is obvious

// Crashes ruin your day → you fix them immediately
// Silent bugs ruin your users' data → nobody ever fixes them
```

---

## Naming Conventions

### Type Naming Rules

```c
#define MY_CONSTANT 42              // ALL_CAPS with underscores

typedef struct MyTypeName MyTypeName; // CapitalCamelCase (no underscores)

void my_module_function(void);       // lowercase_with_underscores

int my_variable;                     // lowercase_with_underscores
```

### Be Descriptive (Wide Code Is Good Code)

> "Really wide code is good code. Wide means long names. Long names means descriptive."

### Consistent Variable Names

```c
// ALWAYS use the same names for the same purposes:
int i, j, k;          // Loop counters — always integers
float f, f2, f3;      // Temporary floats — never anything else
unsigned int count;    // Number of elements
unsigned int length;   // Size of something
void *found;           // Result of a search
Node *next, *previous; // Linked structure navigation
```

This creates familiarity — you can read old code instantly because the patterns are always the same.

### Special Naming Conventions

| Suffix/Name | Meaning |
|---|---|
| `_type` | Part of an enum |
| `_array` | Array of elements |
| `_node` | Element that links to others |
| `_list` | Linked list |
| `_entity` | Generic networked object |
| `_handle` | Opaque pointer to a structure |
| `_func` | Function pointer (not called directly) |
| `_internal` | Should not be called outside its module |

### Searchable Spacing

```c
// Use consistent spacing around operators:
a = 10;     // With spaces: searchable as "a = " (won't match "a ==")
a=10;       // Without spaces: search for "a=" also matches "a==", "a==" etc.

// RULE: Pick one style. NEVER mix them.
// Same for function calls:
func(x);    // No space before paren — search "func(" finds only calls
func (x);   // Space before paren — search "func(" misses this
```

---

## Function Length and Sequential Code

> "Long functions are usually really good."

### Why Sequential Code Is Better

```c
// Sequential code (long function):
void render_frame(void) {
    glEnable(GL_BLEND);
    draw_background();
    draw_terrain();
    glDisable(GL_BLEND);
    draw_characters();
    glEnable(GL_BLEND);
    draw_particles();
    draw_ui();
    // ... 1000+ lines
}
// You can scroll up/down and ALWAYS know the exact state.
// "glEnable was called on line 450, glDisable on line 520 — I know the state."
```

```c
// Fragmented code (many tiny functions):
void render_frame(void) {
    render_background();    // Does this enable blend? Disable it?
    render_characters();    // What GL state does this assume?
    render_ui();            // Does this restore state? Who knows?
}
// You have to jump into each function to understand state.
// Like a Choose-Your-Own-Adventure book — page-flipping constantly.
```

### The Gripen Fighter Jet Example

> "The Gripen control code has one main loop that calls about 10 functions. Those functions never call other functions. One level of indirection. 20 years later, they still haven't found a single bug."

### Warning Sign: Manager/Handler/Controller Functions

If your codebase is full of `SomethingManager`, `SomethingHandler`, `SomethingController` — you're writing code that manages code, not code that does something. Prefer functions with clear, direct purposes.

---

## API Design

### Design from the Outside In

```c
// Step 1: Write the API you WANT to use
void imagine_library_create(const char *name);
void imagine_library_destroy(ImagineLib *lib);
int  imagine_library_count(void);
ImagineLib *imagine_library_get_by_name(const char *name);

// Step 2: Implement the internals behind it
// The implementation can change without touching any caller.
```

### Hierarchical Naming = Directory Structure

```c
// Module: imagine
// Sub-area: library
// Action: create/destroy/count/get

imagine_library_create(...)
imagine_library_destroy(...)
imagine_library_count(...)
imagine_library_get_by_name(...)
imagine_library_get_by_number(...)

imagine_interface_create(...)
imagine_interface_destroy(...)
imagine_interface_register(...)

imagine_mutex_lock(...)
imagine_mutex_unlock(...)

imagine_settings_get(...)
imagine_settings_set(...)
```

Benefits:
- Seeing `imagine_library_create` tells you: module = `imagine`, file = `imagine_library.c`
- Autocomplete groups related functions together
- Easy to find all functions for a sub-area

### Paired Function Names

```c
// GOOD — opposites match:
object_create()  / object_destroy()
lock_acquire()   / lock_release()

// BAD — inconsistent opposites:
object_create()  / object_remove()    // opposite of "create" is "destroy", not "remove"
object_add()     / object_remove()    // opposite of "add" is "subtract"?
```

---

## File and Module Organization

### One Header File Per Module

```
seduce/                    ← Module folder
  seduce.h                 ← THE public API (lives outside the folder for easy access)
  seduce/
    seduce_button.c        ← Implementation files
    seduce_slider.c
    seduce_text_input.c
    seduce_internal.h      ← Shared internals (NOT for external use)
    ...30+ files
```

```c
// Any code that uses the UI library includes ONE file:
#include "seduce.h"

// NOT dozens of individual headers:
// #include "seduce_button.h"    ← BAD
// #include "seduce_slider.h"    ← BAD
```

### Internal Headers

```c
// seduce_internal.h — shared between .c files within the module
// Contains structs and functions that external code should never see
// Named "internal" as a clear signal: "don't look in here"
```

---

## Object Orientation in C

### Objects Don't Exist in Hardware

```
Code and data are SEPARATE on modern systems:
- Data: read-write memory (no execute permission)
- Code: execute-only memory (no write permission)

Object-oriented languages pretend code and data live together.
They don't. C doesn't pretend.
```

### OOP in C: Just Use Functions with Handles

```cpp
// C++ style:
Object *obj = new Object();
obj->doSomething(42);
```

```c
// C style — equally simple:
ObjectHandle obj = object_create();
object_do_something(obj, 42);
```

```c
// C advantage: functions that take MULTIPLE objects
// No implicit "this" — both objects are equal participants
void transfer_data(ObjectHandle src, ObjectHandle dst);
```

---

## Opaque Handles with Void Pointers

### External API: User Gets a Void Pointer

```c
// In the public header (testify.h):
typedef void *THandle;  // Opaque — user can't read or write internals

THandle testify_stream_create(const char *address, int port);
void    testify_stream_send(THandle handle);
void    testify_stream_destroy(THandle handle);
```

### Internal Implementation: Cast to Real Struct

```c
// In the private implementation (testify_internal.h):
typedef struct {
    char   *buffer;
    size_t  buffer_size;
    int     socket_fd;
    // ... lots of internal state
} TStreamInternal;

void testify_stream_send(THandle handle) {
    TStreamInternal *stream = (TStreamInternal *)handle;
    send(stream->socket_fd, stream->buffer, stream->buffer_size, 0);
}
```

**Benefits:**
- Users cannot accidentally modify internal state
- Internal struct can change without breaking any external code
- Clean separation of interface and implementation

---

## Macros: When They're Acceptable

### Generally Avoid Macros

Macros break debuggers, produce unreadable error messages, and hide logic.

### Acceptable Use: Code Generation for Type Variants

```c
// When you need the SAME algorithm for many types:
#define DEFINE_MULTIPLY(TYPE_A, TYPE_B)          \
void multiply_##TYPE_A##_##TYPE_B(               \
    TYPE_A *a, TYPE_B *b, unsigned int count) {  \
    for (unsigned int i = 0; i < count; i++)     \
        a[i] *= (TYPE_A)b[i];                    \
}

DEFINE_MULTIPLY(float, float)
DEFINE_MULTIPLY(float, double)
DEFINE_MULTIPLY(double, float)
DEFINE_MULTIPLY(int, float)
// Generates correct, tested code for every combination
```

> This is **extremely rare**. If you can type it out instead, do that.

### Never Do This

```c
// HORRIBLE — hides basic syntax behind macros:
#define LOOP(var, max) for(int var = 0; var < max; var++)
LOOP(i, 10) { ... }
// "Nobody has any idea what the hell it does"
// A kindergartener can read a for loop. Nobody can read this.
```

---

## The `__FILE__` and `__LINE__` Macros

These are the most valuable macros in C — they enable powerful debugging tools.

### Memory Debugger

```c
// Wrap malloc to record where every allocation comes from:
#define malloc(size) debug_malloc(size, __FILE__, __LINE__)

void *debug_malloc(size_t size, const char *file, int line) {
    void *ptr = real_malloc(size + SENTINEL_SIZE * 2);
    record_allocation(ptr, size, file, line);
    write_sentinels(ptr, size);
    return ptr + SENTINEL_SIZE;
}

// Now you can print: "Leaked 256 bytes allocated at parser.c:88"
// And detect: "Buffer overrun on allocation from main.c:42"
```

### Binary Protocol Debugger (Testify)

```c
// Wrap pack/unpack to include source location:
#define testify_pack_int(stream, value, name) \
    testify_pack_int_debug(stream, value, name, __FILE__, __LINE__)

// In debug mode, every packed field includes type + name metadata
// On unpack, mismatches produce:
//   "ERROR at client.c:55: Expected float 'velocity' but found int 'health'"
```

---

## Memory: How Pointers Really Work

### Everything Has an Address

```
Memory = one giant array of bytes, indexed by address (a number).

Think of it as street addresses:
- Every house has a number
- If you know the number, you can find the house
- If you're at house 9, house 11 is 2 houses down
- That's ALL a pointer is — a house number.
```

### Pointer Types Determine Step Size

```c
void *p = some_address;
short *s = (short *)p;  // 2 bytes per element
int   *i = (int *)p;    // 4 bytes per element

// Both point to the SAME address, but:
*s;        // reads 2 bytes
*i;        // reads 4 bytes

s + 1;     // moves forward 2 bytes
i + 1;     // moves forward 4 bytes
// After incrementing, s and i point to DIFFERENT addresses!
```

### Pointers ARE Arrays

```c
// A pointer to one thing is also a pointer to its neighbors:
int *arr = malloc(10 * sizeof(int));
arr[0];  // = *(arr + 0) — don't move, read here
arr[3];  // = *(arr + 3) — move 3 steps (12 bytes), read there

// This is why arrays are 0-indexed:
// arr[0] means "move 0 steps from start" = stay at start
```

### Pointer as Counter (Faster Loops)

```c
// Traditional loop (multiplication every iteration):
for (int i = 0; i < 10; i++)
    array[i] = 0;   // internally: *(array + sizeof(int) * i) = 0

// Pointer loop (no multiplication):
int *end = array + 10;
for (int *p = array; p != end; p++)
    *p = 0;          // just increment pointer by sizeof(int) each time
// Potentially faster — one add vs. multiply+add
```

---

## Smart `sizeof` Usage

### `sizeof` Is a Cast, Not a Function

```c
// Most people write (looks like a function call):
float *a = malloc(sizeof(float) * 10);

// Better — use the variable itself:
float *a = malloc(sizeof *a * 10);

// WHY? If you change the type of 'a':
double *a = malloc(sizeof(float) * 10);   // BUG! Allocates too little
double *a = malloc(sizeof *a * 10);        // Correct automatically
```

> `sizeof *a` always matches the type of `a`, no matter what you change it to.

---

## Struct Alignment and Padding

### How Alignment Works

```c
typedef struct {
    uint8_t  a;   // 1 byte
    uint32_t b;   // 4 bytes (must be aligned to 4-byte boundary)
} MyStruct;

// Memory layout:
// [a][pad][pad][pad][b][b][b][b]
// sizeof(MyStruct) = 8, not 5!
// 3 bytes are wasted as padding.
```

### Use Padding Wisely

```c
// BAD — wasted space:
typedef struct {
    uint8_t  a;   // 1 byte + 3 padding
    uint32_t b;   // 4 bytes
    uint8_t  c;   // 1 byte + 3 padding
} Bad;            // sizeof = 12 (only 6 bytes useful!)

// GOOD — reorder to minimize padding:
typedef struct {
    uint32_t b;   // 4 bytes
    uint8_t  a;   // 1 byte
    uint8_t  c;   // 1 byte + 2 padding
} Good;           // sizeof = 8 (saved 4 bytes per instance)
```

### Fill the Padding with Useful Data

```c
typedef struct {
    uint8_t  a;
    uint8_t  x;   // Free! Uses padding that was wasted
    uint8_t  y;   // Free!
    uint8_t  z;   // Free!
    uint32_t b;
} Packed;         // sizeof = 8 — same size, but 3 more useful fields
```

### Why `sizeof` Includes Trailing Padding

```c
typedef struct {
    uint32_t b;   // 4 bytes
    uint8_t  a;   // 1 byte
} S;
// sizeof(S) = 8, NOT 5

// Because if you make an array:
S arr[2];
// arr[1].b must be aligned to 4 bytes
// If sizeof were 5: arr[1] starts at byte 5 — NOT 4-byte aligned → broken
// Trailing padding ensures arrays work correctly
```

---

## Struct-Based Inheritance in C

### The Trick: Common Header as First Member

```c
typedef enum { MST_INT, MST_FLOAT, MST_COUNT } MyStructType;

typedef struct {
    char         *name;
    MyStructType  type;
} Header;

typedef struct {
    Header header;    // MUST be first member
    int    value;
} MyStructInt;

typedef struct {
    Header header;    // MUST be first member
    float  value;
} MyStructFloat;
```

### Why It Works

```c
// A pointer to MyStructInt is ALSO a valid pointer to Header
// because Header is at offset 0 in both structs.

void set_value(Header *h) {
    switch (h->type) {
        case MST_INT: {
            MyStructInt *si = (MyStructInt *)h;
            si->value = 42;
            break;
        }
        case MST_FLOAT: {
            MyStructFloat *sf = (MyStructFloat *)h;
            sf->value = 3.14f;
            break;
        }
    }
}

// Usage:
MyStructInt si = { .header = { "health", MST_INT }, .value = 100 };
set_value((Header *)&si);  // Works! Cast is safe because header is first.
```

### Real-World Example: Game Entities

```c
typedef struct {
    EntityType type;
    Vec3       position;
    BlockID    block_id;
    EntityID   id;
    Entity    *next;
    Entity    *previous;
} EntityHeader;

typedef struct {
    EntityHeader head;   // Common to all entities
    float        health;
    Inventory    inventory;
    Animation    anim;
} Character;

typedef struct {
    EntityHeader head;   // Common to all entities
    float        width;
    float        height;
    Material     material;
} Block;

// Generic function — works on ANY entity:
Vec3 entity_get_position(EntityHeader *e) { return e->position; }

// Specific function — only for characters:
void character_take_damage(Character *c, float dmg) { c->health -= dmg; }

// Dispatch based on type:
void entity_update(EntityHeader *e) {
    switch (e->type) {
        case ENTITY_CHARACTER: character_update((Character *)e); break;
        case ENTITY_BLOCK:     block_update((Block *)e); break;
    }
}
```

---

## Memory Blocks and Virtual Memory

### Memory Is Virtualized

```
Physical RAM is divided into ~4KB blocks (pages).
Each page has permissions: Read / Write / Execute.
Virtual addresses don't correspond directly to physical locations.

Process A's address 0x1000 → physical page 57
Process B's address 0x1000 → physical page 203
They're isolated — one process can't corrupt another.
```

### Key Implications

- **`realloc` doesn't always copy** — the OS can remap virtual pages
- **`malloc(1)` allocates an entire page** — you get ~4KB even for 1 byte
- **Adjacent allocations may share a page** — writing past one allocation corrupts the next
- **Pages can be read/write/execute protected** — enables crash-on-overflow tools

---

## `realloc` Is Awesome

### The "realloc Is Slow" Myth

```
Myth: realloc copies the entire buffer every time.
Truth: The OS can remap virtual pages without copying.
       Only the last partial page may need copying.
       A 1GB realloc may only copy ~4KB.
```

### Growing Arrays with `realloc`

```c
typedef struct {
    int   *data;
    size_t length;
    size_t allocated;
} Array;

void array_add(Array *arr, int value) {
    if (arr->length == arr->allocated) {
        arr->allocated += 16;  // Grow by 16 elements
        arr->data = realloc(arr->data, arr->allocated * sizeof(int));
    }
    arr->data[arr->length++] = value;
}
```

### Exponential Growth Strategy

```c
void array_add(Array *arr, int value) {
    if (arr->length == arr->allocated) {
        arr->allocated = arr->allocated ? arr->allocated * 2 : 16;
        arr->data = realloc(arr->data, arr->allocated * sizeof(int));
    }
    arr->data[arr->length++] = value;
}
// realloc is called O(log n) times — very rare for large arrays
```

---

## GFlags: Finding Memory Corruption

### The Problem: Adjacent Allocations

```c
char *a = malloc(1);   // Gets 1 byte inside a 4KB page
char *b = malloc(1);   // Gets the next byte in the SAME page

a[1] = 0;  // Off-by-one: writes into b's memory
            // No crash — it's the same page, all legal
            // b is now corrupted. Good luck finding this.
```

### GFlags Solution (Windows)

```
With GFlags enabled:
- Every allocation gets its OWN page
- Allocation is placed at the END of the page
- The NEXT page is marked NO ACCESS (read/write = crash)

char *a = malloc(1);   // Gets last byte of page N
                        // Page N+1 is a "kill page"
a[1] = 0;              // Writes into page N+1 → INSTANT CRASH
                        // Debugger stops right here. Bug found.
```

**Warning:** Turning on GFlags makes your entire OS unstable — every program with buffer overflows will crash. Use a dedicated development machine.

### Stack Equivalent: The Infinite Loop Bug

```c
uint32_t a[10];
int i;

for (i = 0; i < 11; i++)  // BUG: should be < 10
    a[i] = 0;

// Likely result: a[10] overwrites 'i' (adjacent on stack)
// i gets set to 0 → loop restarts → INFINITE LOOP
// Looking at the code: "i only goes up, how can it restart?!"
// This is why stack arrays are terrifying.
```

---

## Memory Is Slow, Math Is Fast

### Cache Hierarchy

| Level | Latency | Size |
|---|---|---|
| Register | 0 cycles | Bytes |
| L1 Cache | 2–3 cycles | ~32–64 KB |
| L2 Cache | 10–15 cycles | ~256 KB – 1 MB |
| Main RAM | ~50 cycles | Gigabytes |

### The Math

```
One memory read from RAM: ~50 cycles
Modern SIMD: 4 multiplications per cycle
50 cycles × 4 muls = 200 multiplications

→ 200 multiplications are faster than reading ONE byte from RAM.
```

> When optimizing: **reduce memory access first**. The math is essentially free.

### Cache Friendliness

When you read one byte, the CPU prefetches an entire **cache line** (~64 bytes) of surrounding data. If your next access is nearby, it's already in cache (cache hit). If it's far away, you wait ~50 cycles (cache miss).

---

## Linked Lists Are Terrible

```c
// Linked list traversal — every node is a random memory location:
for (Node *n = head; n != NULL; n = n->next)
    process(n->data);
// Every n->next is a potential cache miss (50 cycles).
// Each node also stores a 'next' pointer — 2x memory overhead.

// Array traversal — all elements are adjacent:
for (int i = 0; i < count; i++)
    process(array[i]);
// Sequential access → CPU prefetches ahead → almost all cache hits.
// No overhead from 'next' pointers.
```

### Fast Removal from Arrays

```c
// Unordered removal (O(1)):
void array_remove_unordered(Array *arr, int index) {
    arr->data[index] = arr->data[--arr->length];
    // Move last element into the gap. Done.
}

// Ordered removal by searching backwards:
void array_remove_ordered(Array *arr, int value) {
    for (int i = arr->length - 1; i >= 0; i--) {
        if (arr->data[i] == value) {
            // Found it — shift everything already touched in cache
            memmove(&arr->data[i], &arr->data[i+1],
                    (arr->length - i - 1) * sizeof(int));
            arr->length--;
            return;
        }
        // Already touching this memory for the search,
        // so the memmove is nearly free (data is in cache).
    }
}
```

> A linked list's O(1) removal advantage disappears because you still need O(n) to **find** the element — and that search is slower due to cache misses.

---

## Don't Store Data Twice

### The Problem

```c
typedef struct {
    float width;
    float length;
    float area;    // Redundant! area = width * length
} Plane;

// If someone changes width without updating area → inconsistent data
// Which is "correct" — width*length or area? You don't know.
```

### The Solution

```c
// Option 1: Compute on the fly (preferred — math is cheap)
float plane_area(Plane *p) { return p->width * p->length; }

// Option 2: If computation is expensive, use getters/setters
typedef struct PlaneImpl PlaneImpl;  // Opaque

void  plane_set_width(PlaneImpl *p, float width);
void  plane_set_length(PlaneImpl *p, float length);
float plane_get_area(PlaneImpl *p);  // Returns cached value

// Internally, set_width and set_length ALSO update the cached area.
// External code cannot desync the values.
```

---

## Advanced Allocation Tricks

### Flexible Array Members (Single Allocation for Struct + Data)

```c
// TWO allocations (slow — data may be far from header):
typedef struct {
    size_t length;
    int   *data;     // Separate allocation
} ArrayA;
ArrayA *a = malloc(sizeof(ArrayA));
a->data = malloc(length * sizeof(int));

// ONE allocation (fast — data is adjacent to header):
typedef struct {
    size_t length;
    int    data[];   // C99 flexible array member
} ArrayB;
ArrayB *b = malloc(sizeof(ArrayB) + length * sizeof(int));
b->length = length;
// b->data is right after b->length in memory — cache friendly
```

### Co-Allocating Multiple Structs

```c
typedef struct { uint32_t x; double y; } StructA;  // 16 bytes (with padding)
typedef struct { float a; float b; }     StructB;   // 8 bytes

// Single allocation for both:
StructA *a = malloc(sizeof(StructA) + sizeof(StructB));
StructB *b = (StructB *)(a + 1);  // b starts right after a

// Both structs are adjacent in memory — one allocation, one free.
// WARNING: Be careful about alignment requirements!
```

---

## Stride: Flexible Data Access

### The Problem

```c
// This function REQUIRES tightly packed RGB data:
void color_correct(uint8_t *rgb, int pixel_count);

// But your data is RGBA:
uint8_t *rgba_buffer = ...;
// Can't use color_correct without copying all RGB values out. Slow!
```

### The Solution: Add a Stride Parameter

```c
void color_correct(uint8_t *rgb, int pixel_count, int stride) {
    for (int i = 0; i < pixel_count; i++) {
        uint8_t *pixel = rgb + i * stride;
        pixel[0] = adjust_r(pixel[0]);
        pixel[1] = adjust_g(pixel[1]);
        pixel[2] = adjust_b(pixel[2]);
    }
}

// Now works with ANY memory layout:
color_correct(rgb_buffer,  count, 3);              // Packed RGB
color_correct(rgba_buffer, count, 4);              // RGBA (skips alpha)
color_correct(&my_struct->color, count, sizeof(MyStruct)); // Inside structs!
```

> Stride makes functions vastly more versatile with minimal complexity.

---

## Build the Mountain

> "Build a mountain, not a house. The application is a small house on a big mountain."

### The Apple Example

```
QuickTime (boring tech)
  → iTunes (people cared)
    → iPod (hardware product)

WebObjects (boring tech)
  → iTunes Store (commerce platform)

OS X + OpenGL (boring tech)
  → iOS
    → App Store (ecosystem)

Safari (why another browser?)
  → iPhone (needs a browser → Safari was ready)
```

Each "boring" technology became a building block for the next breakthrough.

### Apply This to Your Code

```
DON'T: Build a music player (one product, one use)
DO:    Build a UI library + a sound library + a platform layer
       → Music player is trivial on top
       → Video player is trivial on top
       → Any future app is trivial on top
```

Every library you write is a permanent asset. Applications are small things you assemble from your libraries.

---

## Fix Your Code Now

> "It's never going to be easier to rewrite your code than it is today."

- **Don't accumulate technical debt** — fix code you don't like immediately
- **The more you use code, the harder it is to change** — change it while it's fresh
- **Don't show hacks to your manager** — only show finished, debugged work
  - Showing a quick prototype sets unrealistic expectations
  - "Why did the last 1% take 6 months?" → because you faked the first 99%

---

## Immediate-Mode UI

### Traditional (Retained-Mode) UI

```c
// Create elements once, manage IDs and callbacks:
ElementID panel = ui_create_element();
ButtonID  btn   = ui_add_button(panel, "Hello", x, y, on_click, user_data);

// To move the button: need the ID
ui_set_position(panel, btn, new_x, new_y);

// To animate: need ID, timer, update loop... messy
```

### Immediate-Mode UI (Eskil's Approach)

```c
// Just call draw functions every frame:
if (ui_button(x + sin(time) * 50, y, "Hello")) {
    // Button was clicked!
}
ui_surface(x, y, width, height);
```

### How Overlapping/Click Testing Works

The UI library internally:
1. Records the position and ID of every widget drawn this frame
2. Resolves occlusion (surfaces covering buttons)
3. On the next frame, uses stored positions to answer click queries

### IDs Without Explicit Allocation

```c
// Most widgets already have a unique pointer (e.g., the value they modify):
float slider_value = 0.5f;
ui_slider(&slider_value, x, y, width);  // &slider_value IS the unique ID

// If you need an arbitrary ID:
ui_button("my_unique_label", x, y);     // String literal address is unique
```

---

## Useful Bit Tricks

### Carmack's Inverse Square Root

```c
float fast_inverse_sqrt(float number) {
    long i;
    float x2, y;
    x2 = number * 0.5f;
    y  = number;
    i  = *(long *)&y;
    i  = 0x5f3759df - (i >> 1);   // What the...?
    y  = *(float *)&i;
    y  = y * (1.5f - (x2 * y * y));  // One Newton-Raphson iteration
    return y;
}
// ~4x faster than 1.0/sqrt(x) on old hardware
// Still useful for graphics, physics, procedural generation
```

### Fast Integer Hash (Random Number Generator)

```c
unsigned int hash(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
// Few instructions, excellent distribution
// Perfect for procedural generation, stochastic ray tracing, etc.
```
