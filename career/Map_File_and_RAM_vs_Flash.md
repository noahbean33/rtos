# Map File & RAM vs. Flash

## Core Concept
Understanding where variables physically reside — **RAM (SRAM)** vs. **Flash (ROM)** — is critical for memory-constrained embedded systems and common in interviews.

## Key Ideas

### Memory Sections
- **Flash (ROM):** `.text` (code/assembly), `.rodata` (read-only data, constants)
- **RAM:** `.data` (initialized globals), `.bss` (zero-initialized globals), stack
- `.bss` is stored more compactly — no need to store explicit zeros; just a size declaration
- `.data` must store all actual initial values

### Important Keywords & Their Effect
| Keyword | Effect on Placement |
|---|---|
| `const` | Moves global variable from RAM → Flash (read-only) |
| `volatile` | Keeps value in RAM; prevents compiler caching in registers |
| `static` | Stays in RAM but **hides symbol** from the global symbol table (file-scoped) |

### Examples
- `const uint32_t firmware_version` → Flash (.rodata)
- `volatile uint32_t status_register` → RAM (.bss or .data)
- `static uint32_t local_state_machine` → RAM (.bss), but not in global symbol table
- Pointer to const (`const uint32_t *p`) → pointer in RAM, points to Flash
- Const pointer (`uint32_t * const p`) → pointer in Flash, points to RAM
- `static` local inside `main()` → persistent RAM (.bss)

### Compiler Tools
- **Map file** (`-Map` flag): shows where globals are placed, section sizes, function addresses
  - Useful for debugging crashes — look up program counter address to find the faulting function
  - Static/local variables won't appear in the map's global listing but still affect section sizes
- **`nm` utility**: quick snapshot of symbols from the ELF file
  - `R` = read-only, `B` = BSS, `T` = text; uppercase = global, lowercase = local
