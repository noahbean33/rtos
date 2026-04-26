# Why You Need State Machines

## The Problem: Spaghetti Code
A simple coffee pot controller with 4 states (idle, heating, brewing, error) implemented with **boolean flags and nested if-statements**:
- Multiple global booleans: `is_running`, `is_heating`, `is_brewing`, `has_error`
- Every event handler must check **all flags** with complex conditions
- Adding a new state (e.g., "cleaning") requires modifying logic in **every existing case**
- Complexity grows **exponentially** with each new state
- Impossible to tell at a glance what states are valid or what transitions are legal
- Globals modified inside functions — terrible practice
- Magic numbers instead of enums

## The Solution: Finite State Machine (FSM)

### Clean Architecture
- **One enum** for all possible states (replaces multiple booleans)
- **One struct** holds the machine's current state
- **Switch-case** on current state — each case handles only its own logic
- Adding a state = adding a `case` block — **linear complexity growth**

### Coffee Pot FSM
```
States: IDLE → HEATING → BREWING → IDLE
              ↘ ERROR ↙
```
- Each state has clearly defined transitions and events it responds to
- No global boolean flags — state is a single variable in a struct
- Much easier to test: pass a pointer to the state struct, check output state

### Benefits Over Spaghetti Code
| Aspect | Spaghetti | FSM |
|---|---|---|
| State representation | N boolean flags | 1 enum variable |
| Adding a state | Modify all existing logic | Add 1 case block |
| Readability | Complex nested conditions | Clear per-state logic |
| Testability | Check multiple globals | Check single struct |
| Complexity growth | Exponential | Linear |
| Global mutation | Yes (dangerous) | No (state in struct) |

### Key Takeaway
- Once you see the FSM pattern, you can't unsee it
- It's the **standard professional approach** for any system with distinct modes of operation
- Scales cleanly, reads clearly, tests easily
