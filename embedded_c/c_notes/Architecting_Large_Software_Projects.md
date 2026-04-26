# Architecting Large Software Projects

> Based on a talk by **Eskil Steenberg** — C programmer, architect of long-lived software systems

---

## Table of Contents

1. [Design Goals](#design-goals)
2. [Managing Risk](#managing-risk)
3. [Language Choice and Dependability](#language-choice-and-dependability)
4. [Modularization: Black Boxes and APIs](#modularization-black-boxes-and-apis)
5. [Case Study 1: Video Editor](#case-study-1-video-editor)
6. [Primitives and Structure](#primitives-and-structure)
7. [Plugin Architectures](#plugin-architectures)
8. [Case Study 2: Healthcare System](#case-study-2-healthcare-system)
9. [Case Study 3: Jet Fighter](#case-study-3-jet-fighter)
10. [Format Design Principles](#format-design-principles)
11. [Key Takeaways](#key-takeaways)

---

## Design Goals

When planning a large project, optimize for:

| Goal | Why It Matters |
|---|---|
| **Dependability** | The project should last forever and never break |
| **Extendability** | You must be able to add new features and capabilities over time |
| **Team Scalability** | Each person works on an isolated section — no massive team meetings |
| **Development Velocity** | Avoid the typical slowdown as projects age |
| **Risk Reduction** | Anticipate what will fail and insulate against it |

> **Core rule:** It's faster to write five lines of code today than to write one line today and edit it in the future. Write code so it never breaks.

---

## Managing Risk

Things that can change and break your project:

- **Platforms** — APIs change, terms of service change, vendors disappear
- **Language implementations** — breaking changes across versions (e.g., Python 2 → 3)
- **Hardware** — new architectures, deprecated instruction sets
- **Priorities** — requirements shift over decades
- **People** — key developers leave

> A lot of small chances of failure equals a big chance of failure. Even "trivial" fixes force context switches that grind development to a halt.

---

## Language Choice and Dependability

```
"I use C89. It is the most dependable language you can have.
 It will compile everywhere. It will last forever."
```

The key insight isn't about C specifically — it's about **reducing your technology footprint**:

- Choose a language with long-term compiler support
- A jet fighter will be around for **50 years**; a healthcare system for decades
- Languages that break backwards compatibility (Python 2→3) are a major risk
- The rules in this talk apply regardless of language choice

---

## Modularization: Black Boxes and APIs

### Principles

- Modules should be **black boxes** — expose functionality through APIs/protocols only
- Users of a module should **never need to look at its internals**
- Each module should be implementable by **one person**

```
┌──────────────────────────────────────┐
│           Application Layer          │
│  (combines modules into a product)   │
├──────────┬──────────┬────────────────┤
│ Module A │ Module B │   Module C     │
│ (1 dev)  │ (1 dev)  │   (1 dev)     │
└──────────┴──────────┴────────────────┘
   Each module has a clear API boundary
```

### Benefits

- **Replaceable** — if a module is bad, re-implement behind the same API
- **Resilient to turnover** — if the author leaves, rewrite from the API spec
- **Parallelizable** — developers work in isolation
- **Senior/junior split** — hard modules go to senior devs, easy ones to junior devs

### Header Files as APIs (C Example)

```c
// betray.h — Platform abstraction layer
// This single header IS the entire public interface.
// All implementation details are hidden in .c files.

void betray_window_open(int width, int height);
void betray_input_poll(BetrayEvent *events, int *count);
void betray_window_close(void);
```

---

## Case Study 1: Video Editor

### Building the Stack Bottom-Up

```
┌─────────────────────────────┐
│       Video Editor UI       │  ← Application (small)
├─────────────────────────────┤
│        UI Toolkit           │  ← Buttons, sliders, text input
├─────────────────────────────┤
│      Text Rendering         │  ← Fonts, UTF-8, kerning
├─────────────────────────────┤
│      Drawing Library        │  ← Lines, surfaces, images, shaders
├─────────────────────────────┤
│      Platform Layer         │  ← Window, input, file I/O
└─────────────────────────────┘
```

### Rule: Wrap Everything You Don't Own

Even if you use a well-known library like SDL:

```c
// DON'T scatter platform calls everywhere:
SDL_CreateWindow(...);  // Directly in application code — bad

// DO wrap it:
betray_window_open(...);  // Your wrapper — if SDL changes, fix one place
```

This gives you:
- Freedom to swap backends (SDL → Win32 → custom)
- Insulation from upstream breaking changes
- A single place to fix when porting

### Write a Test Application First

Create a **minimal demo** that exercises your platform API:

- Opens a window
- Reads mouse/keyboard input
- Draws basic shapes and text
- Tests all API functions

This becomes your **porting tool** — when moving to a new platform, get the demo running first.

### Design APIs for the Future

```c
// BAD: "good enough for now" API
void draw_text(float x, float y, char *text);

// GOOD: future-proof API (even if some params are ignored today)
float draw_text(Font *font, float size, float spacing,
                const char *text, float x, float y,
                Color color, int max_length);
// - Returns width of drawn text
// - Supports fonts (even if only one exists today)
// - Supports UTF-8, color, length limiting
// - When Korean/Arabic support ships, NO callers need to change
```

> **Key insight:** None of the layers below the UI have anything to do with video editing. They're reusable for *any* application. This is your company's superpower.

### Helper Libraries Are Gold

Invest heavily in reusable libraries:

- File parsing, physics, scripting, data storage, networking
- Each one eliminates an entire category of problems forever

```c
// Example: Testify (networking library)
// After writing this once, you never touch the socket API again.
// You can improve internals without changing any application code.
```

### The Core: Generalize What Your Application Does

> "A video editor doesn't edit video — it edits a **timeline**."

```
Timeline = Clips + Animated Parameters
```

Everything is a clip. Every tool operates on clips. Every UI element modifies clip parameters. This single generalization simplifies the entire architecture.

**Examples of good generalizations:**
- **Unix:** Everything is a file
- **Houdini:** Everything is a node graph with parameters
- **Notch:** Same — node graph for visual effects

**Comparison — less generalized:**
- **Unreal Engine:** C++ + Blueprints + Scene Graph + Assets = hard to keep consistent

### Core Guarantees

The core data structure can enforce invariants:

```c
// The core guarantees:
// - Clips never have negative time
// - Clips are never longer than their source asset
// - Time is always consistent
// - Undo/redo is built into the core automatically

void core_set_clip_start(Core *core, ClipID clip, double time);
// Internally validates constraints, records undo state
```

---

## Primitives and Structure

### Choosing Your Primitive

The **primitive** is the fundamental data unit that flows through your system.

| Software | Primitive | Trade-offs |
|---|---|---|
| Unix | Text files | Great for grep/pipes — useless for video/images |
| Photoshop | Bitmap layers | Simple model — limited compositing |
| Nuke | Bitmap in a node graph | Same primitive, different structure |
| Teardown | Voxels | Easy collision/destruction — no smooth surfaces |
| CAD software | NURBS | Mathematical curves — complex implementation |

### Primitive vs. Structure

These are **two separate choices**:

```
STRUCTURE: How data flows (pipes? node graph? layers? timeline?)
PRIMITIVE: What the data IS (text? bitmaps? triangles? events?)
```

You can mix and match:
- Command-line pipes (structure) with video frames (primitive)
- Node graph (structure) with text data (primitive)

> **Key:** If you can find ONE primitive that works for everything, your software stays simple.

---

## Plugin Architectures

### Why Plugins?

If every format, effect, and codec is built into the core, the core becomes enormous and unmanageable. Instead:

```c
// Plugin descriptor
typedef struct {
    const char *name;          // "MP4 Decoder"
    const char *category;      // "video"
    int         num_inputs;
    ParamDesc  *input_params;  // What the plugin needs
    int         num_outputs;
    ParamDesc  *output_params; // What the plugin provides
    ProcessFunc process;       // The actual computation function
} PluginDescriptor;
```

### Architecture

```
┌──────────────────────────────────────────┐
│              UI / Launcher               │
├──────────────────────────────────────────┤
│                 Core                     │
│  (timeline, undo, validation, plugin mgr)│
├──────┬──────┬──────┬──────┬──────────────┤
│ DLL  │ DLL  │ DLL  │ DLL  │  ...plugins  │
│ MP4  │ Color│Sphere│ Ramp │              │
└──────┴──────┴──────┴──────┴──────────────┘
```

### Benefits

- Plugin authors work **completely isolated** from core developers
- Plugins live in separate DLLs — the core stays small
- The core can be reused without UI (e.g., command-line batch processor)
- Platform-specific code lives in optional plugins (e.g., Xbox controller support)

### Present Capabilities, Don't Hardcode Them

```c
// A washing machine (or any device) presents:
typedef struct {
    bool  running;
    Mode  mode;         // What mode am I in?
    float temperature;  // What's my current temperature?
    float load;         // What's my current load?
    int   seconds_left; // How long until done?
} WashingMachineStatus;

typedef struct {
    bool  run;          // Should I run?
    Mode  mode;         // Which mode?
    float temperature;  // What temperature?
} WashingMachineCommand;

// You don't need to know it's a washing machine to build a UI for it.
// Any device that presents status + accepts commands works the same way.
```

---

## Case Study 2: Healthcare System

### Choosing the Primitive

Not "medical journals" but **healthcare events**:

```
Event = { timestamp, patient, provider, clinic, type, data... }
```

Events cover:
- Historical records (past visits)
- Future appointments (scheduled events)
- Multiple access patterns (all events at a clinic today, full patient history)

### Architecture

```
┌─────────────────────────────────────────────────┐
│   Mobile App │ Website │ Machine Interface │ ... │
├──────────────┴─────────┴────────────────────┴────┤
│          Python API │ C++ API │ C API            │
├──────────────────────────────────────────────────┤
│              Glue Code (old system)              │
├──────────────────────────────────────────────────┤
│           ████ BLACK BOX (Core) ████             │
│     (storage, validation, access control)        │
│  implementation: SQL? NoSQL? Custom? Doesn't     │
│  matter — users only see the API above.          │
└──────────────────────────────────────────────────┘
```

### Key Design Decisions

1. **Never expose storage implementation** — no SQL queries in the API
2. **Wrap the old system** — glue code syncs old ↔ new bidirectionally (no big-bang migration)
3. **Multiple language bindings** — C core with Python/C++/etc. wrappers
4. **Black box is replaceable** — can swap storage backend without touching applications

---

## Case Study 3: Jet Fighter

### The Primitive: State of the World

Unlike healthcare (history-focused), a jet fighter cares about **right now**:

```c
typedef struct {
    SensorType source;      // Where does this data come from?
    float      confidence;  // How much do I trust this?
    float      accuracy;    // How precise is this measurement?
    DataFormat format;      // Relative to aircraft? Global coordinates?
    // ... actual sensor data
} WorldStateEntry;
```

### Architecture: Authoritative Core + Subscribers

```
┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│   Radar     │  │   Weapons   │  │   Engine    │
│ (subscriber)│  │ (subscriber)│  │ (subscriber)│
└──────┬──────┘  └──────┬──────┘  └──────┬──────┘
       │                │                │
       ▼                ▼                ▼
┌──────────────────────────────────────────────┐
│          AUTHORITATIVE CORE                  │
│  (canonical world state, sensor fusion)      │
│  Subscribers request specific data streams   │
│  at specific rates based on their needs.     │
└──────────────────────────────────────────────┘
       │                │                │
       ▼                ▼                ▼
┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│   Display   │  │   HUD       │  │   Recorder  │
│ (subscriber)│  │ (subscriber)│  │ (subscriber)│
└─────────────┘  └─────────────┘  └─────────────┘
```

### Subscriber Model

```c
// The engine subscribes to what it needs:
subscribe(core, ALTITUDE | HUMIDITY | FUEL_LEVEL, rate_hz=10);

// The radar subscribes differently:
subscribe(core, CONTACTS | WEATHER, rate_hz=50);

// Each subscriber gets a local partial copy of world state
```

### Tooling Is Critical

Build tools that **never ship on the aircraft** but are essential for development:

| Tool | Purpose |
|---|---|
| **Recorder** | Subscribe to all data → black box flight recorder |
| **Playback** | Feed recorded data to any subsystem for testing |
| **Python API** | Simulate scenarios (sensor failure, enemy behavior) |
| **Logger** | Print live data for debugging individual subsystems |
| **Visualizer** | Graphical display of world state |
| **Full simulator** | Test without hardware |

### Redundancy

```
Single core (prototype) → Multiple redundant cores (production)
Same API for subscribers either way.

Prototype core ships in 6 months → everyone starts development.
Redundant core ships in 3 years → drop-in replacement, same API.
```

### Generalization

The core knows nothing about aircraft. It passes data. This means:

- Put it in a **tank**, a **transport plane**, a **ship** — same system
- Connect **multiple aircraft** — they share world state through the same protocol
- Missile makers build to one API that works on any platform
- The core can be **open source** — it contains no secrets, only protocols

---

## Format Design Principles

> "Core to all software design is **format design**."

Formats include: APIs, file formats, network protocols, programming languages.

### Semantics vs. Structure

| | Example | What It Defines |
|---|---|---|
| **Semantics only** | Metric system (3 meters) | Meaning, not encoding |
| **Structure only** | JSON | Encoding, not meaning |
| **Both** | A well-designed API | How to send data AND what it means |

> Use a simple structure with rich semantics — this lets you reuse structural tools (JSON parsers) across domains.

### Implementability Is Everything

```
Complex format → fewer implementations → more bugs → incompatibilities
Simple format  → many implementations → higher quality → interoperability
```

- The **larger** a language, the **fewer** correct implementations it gets
- If your API is too complex, consumers will implement only a subset
- **Pick one primitive** even if some people are unhappy — implementing one thing well is better than implementing two things poorly

### Key Questions for Format Design

1. **Interactions** — How many levels of nesting? Can you have multiple timelines? Multiple radars?
2. **Implementation freedom** — Does your API lock you into a specific backend?
3. **Constraints** — What can you guarantee? (units, ranges, formats)
4. **Choices** — Make hard decisions. A format that "supports everything" will be implemented by no one.

### Plugin Philosophy

```
PREFER: Standalone modules that CAN be plugged in
AVOID:  Plugins that MUST live inside a host

"You don't want to live in somebody else's world.
 You want to live in your world and accept others into it."
```

---

## Key Takeaways

1. **Every piece of software should be writable by one person** — split into modules until this is true
2. **Wrap everything you don't own** — platforms, libraries, APIs
3. **Find your primitive** — one generalized data type that everything operates on
4. **Build the mountain, not the house** — reusable libraries are the real value; applications are small things on top
5. **Design APIs before implementations** — even incomplete implementations behind a good API let everyone keep working
6. **Never expose your storage/backend** — keep implementation freedom
7. **Build tools** — recorders, simulators, loggers, visualizers pay for themselves many times over
8. **Simple formats beat complex ones** — implementability determines real-world success
9. **Avoid big-bang migrations** — run old and new systems in parallel with glue code
10. **Write code that lasts decades** — dependability beats cleverness every time
