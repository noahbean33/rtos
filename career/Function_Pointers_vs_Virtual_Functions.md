# C Function Pointers vs. C++ Virtual Functions

## Use Case
A sensor dashboard needs a **common interface** to read from multiple sensors (temperature, accelerometer, gyro) regardless of their underlying protocol (I2C, SPI, UART).

## Key Ideas

### C — Function Pointers
- A struct holds the sensor name + **function pointers** (`read`, `init`, `reset`)
- Each sensor type assigns its own implementation to these pointers
- To call: dereference the pointer and jump to the implementation in Flash

#### Memory Layout
- **RAM:** stores all function pointers (one per function, per sensor instance)
- **Flash:** stores the actual implementation code
- Scaling: adding N more functions = N more pointers **in RAM** per sensor instance

### C++ — Virtual Functions (vtable)
- A base `Sensor` class declares virtual `read()`, `init()`, `reset()`
- Each sensor subclass overrides with its own implementation
- Compiler creates a **vtable** (in Flash) containing pointers to all virtual function implementations
- Each object stores a single **vpointer** (in RAM) pointing to its class's vtable

#### Memory Layout
- **RAM:** only 1 vpointer per object (regardless of number of virtual functions)
- **Flash:** vtable + implementation code
- Scaling: adding N more functions = **zero additional RAM** — only the vtable in Flash grows

### Assembly Comparison (ARM GCC, -O2)
| Aspect | C (struct + fn pointers) | C++ (virtual functions) |
|---|---|---|
| RAM per object | 1 pointer per function | 1 vpointer (constant) |
| Flash usage | Implementation code only | vtable + implementation |
| Indirection | 1 level (pointer → code) | 2 levels (vpointer → vtable → code) |
| Stores in main | 4 per class (2 classes = 8) | 2 per class (2 classes = 4) |

### When to Use Which
- **RAM-constrained, many functions per interface:** prefer C++ vtables (saves RAM)
- **Flash-constrained, few functions:** prefer C function pointers (less Flash overhead)
- **Safety-critical, no C++ allowed:** C function pointers are the only option
- C function pointers have simpler, more direct dispatch (one jump vs. two)
