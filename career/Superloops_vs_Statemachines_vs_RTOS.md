# Superloops vs. State Machines vs. RTOS

## Three Main Firmware Architectures — When to Use Each

### 1. Super Loop
**Best for:** simple, sequential systems with no concurrency needs

- A `while(1)` loop that executes tasks in order, repeatedly
- Example: read temperature sensor → update display → repeat every 100ms
- Timer ISR sets a flag; loop checks the flag and acts
- Sleep between cycles to save CPU power

**Limitations:** adding complex features (e.g., authentication with locked/unlocked states) becomes messy — lots of if-statements, hard to separate logic

### 2. State Machine (FSM)
**Best for:** systems with distinct operational modes and event-driven transitions

- Use when you need to add **states** (locked, idle, sensing, error) that change system behavior
- Clean `switch-case` on an enum of states
- Each state handles only its own events and transitions
- Easy to add new states without touching existing logic

**Example upgrade:** add user authentication (PIN entry) to the temperature display
- `LOCKED` → waiting for PIN → `IDLE` → timer triggers → `SENSING` → read data → display
- Each state is isolated and testable

**Limitations:** can't handle concurrent/background tasks — a slow operation (e.g., SD card write) blocks everything

### 3. RTOS (Real-Time Operating System)
**Best for:** systems needing **concurrent tasks** at different priorities/rates

- Each task gets its own `while(1)` loop and priority level
- Scheduler switches between tasks — preemptive multitasking
- Tasks communicate via **queues**, **semaphores**, **event groups**

**Example upgrade:** add SD card logging in the background
- **Auth task** (highest priority): wakes on button press, verifies PIN
- **Sensor task** (medium): reads sensor, updates display, pushes data to queue
- **Logging task** (lowest): writes to SD card when queue has data
- SD card writes are slow (~1s) but don't block the display or authentication
- CPU switches between tasks seamlessly

### Decision Guide
| Criteria | Super Loop | State Machine | RTOS |
|---|---|---|---|
| Task complexity | Simple, sequential | Multiple modes/states | Concurrent tasks |
| Blocking tolerance | Acceptable | Acceptable per-state | Not acceptable |
| Adding features | Gets messy fast | Clean, linear growth | Clean, isolated tasks |
| Resource overhead | Minimal | Minimal | Scheduler + stack per task |
| Common interview Q | "When would you use each?" | | |
