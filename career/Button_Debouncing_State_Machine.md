# Button Debouncing State Machine

## Why Debouncing Is Needed
- Mechanical switches **bounce** when pressed — the electrical signal oscillates between high and low for a few milliseconds before settling
- Without debouncing, firmware reads multiple false press/release events from a single physical press
- Standard debounce threshold: **~50 ms** of stable signal

## Key Ideas

### Constraints
- **Non-blocking:** no `sleep()` or `delay_ms()` — don't waste CPU cycles waiting
- Use a **finite state machine** (FSM) instead

### Four States
1. **RELEASED** — idle, no press detected
2. **MAYBE_PRESSED** — raw input went high; start timing
3. **PRESSED** — signal stable high for ≥50 ms; confirmed press
4. **MAYBE_RELEASED** — raw input went low while pressed; start timing

### State Transitions
```
RELEASED → MAYBE_PRESSED    (raw input goes high)
MAYBE_PRESSED → RELEASED    (raw input goes low before 50ms)
MAYBE_PRESSED → PRESSED     (stable high for ≥50ms) → return true
PRESSED → MAYBE_RELEASED    (raw input goes low)
MAYBE_RELEASED → PRESSED    (raw input goes high before 50ms)
MAYBE_RELEASED → RELEASED   (stable low for ≥50ms)
```

### Time Tracking
- Use a global `system_time_ms` incremented by a **SysTick ISR** (hardware timer interrupt)
- Log timestamp (`last_tick_ms`) on each state transition
- Compare `current_time - last_tick_ms >= 50` to confirm stable signal

### Implementation Details
- Debouncer struct holds: `state`, `last_tick_ms`
- `update_debounce(db, raw_level)` — called each cycle, returns `true` only when button is confirmed pressed
- Bounce filtering applies **both directions** (press and release)
- GPIO pin read provides the `raw_level` input
