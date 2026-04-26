# Opaque Pointers in C

## What Are Opaque Pointers?
An opaque pointer hides the internal implementation of a data type, exposing only a handle and a set of API functions. The user never sees the struct definition — only the header with function prototypes.

## Key Ideas

### Concept (like `FILE*` in C)
- `fopen()` returns a `FILE*` — you never know what's inside the struct
- You interact only through `fopen`, `fread`, `fclose`, etc.
- Different OS implementations (Windows, macOS, RTOS) can have completely different internals
- This is **C's version of private class members** from C++

### How to Implement
1. **Header file (public):** declare a forward `typedef struct motor* motor_t;` and API functions (`motor_create`, `motor_set_speed`, `motor_stop`, `motor_destroy`)
2. **Source file (private):** define the actual struct with internal fields (e.g., `safety_threshold`, `current_speed`, `pin`)
3. Users only get the header — they **cannot access internal struct members**; the compiler will error if they try

### Motor Example
- `motor_create(pin)` → returns opaque `motor_t`
- `motor_set_speed(motor, 50)` → sets speed via API
- Attempting `motor->safety_threshold` → **compile error** (struct definition hidden)

### PID Controller Example
- Internal state: `integral`, `prev_error`, `output` (private)
- User-settable: `kp`, `ki`, `kd` (set through `pid_set_gains()`)
- User can `pid_create`, `pid_compute`, `pid_reset`, `pid_destroy` — never touches internals

### Why Use Them
- **Encapsulation:** prevent unauthorized modification of safety-critical values
- **Portability:** swap implementations without changing the public API
- **Professional library design:** clean interfaces for drivers, controllers, etc.
