# Mastering Embedded Linux, Part 5: Platform Daemons

> Source: George Hilliard's blog — &>/dev/null

---

## 1. Overview

How to write a **platform daemon** — custom software that talks directly to hardware peripherals on an embedded Linux system. This article implements a proof-of-concept HTTP-accessible GPIO controller in **Rust**.

Part 6 (next) covers packaging this daemon into Buildroot.

---

## 2. What Is a Platform Daemon?

A platform daemon is **bespoke software** written for a specific target or family of targets. It picks up where the kernel leaves off:

- The kernel provides **drivers** to talk to peripherals
- The platform daemon **routes data between peripherals** and implements application logic
- Example: ADC readings → processing → USB output

### Characteristics
- Contains **intimate knowledge** of the embedded system
- Manages **state of all peripherals** as a unified entity
- Often includes device driver code for custom peripherals (though this ideally belongs in the kernel)
- Analogous to "firmware" on microcontrollers (an RTOS task waiting for driver events)

### Why Userland (Not Kernel)?
- Can **develop and debug on your workstation** before porting to hardware
- **Bugs don't crash the entire system** — userland is isolated from kernel
- Same APIs as desktop Linux

---

## 3. GPIO Access in Linux

### Modern Character Device API
- Linux provides GPIO access via **character devices** in `/dev/`
- One device per GPIO controller (e.g., `/dev/gpiochip0`, `/dev/gpiochip1`)
- A physical pin is identified by a **[peripheral, pin number]** pair
- Operations via `ioctl` commands on the device file:
  - Set input/output mode
  - Read high/low state
  - Drive high/low (if output)

### Useful Tools
- **libgpiod** — kernel's own GPIO command-line tools (packaged in Buildroot)
- **gpio-mockup** kernel module — simulates GPIO hardware for testing on workstations

> **Deprecated:** The older `/sys/class/gpio` sysfs ABI is deprecated (removal after 2020). Use the character device API.

### Pin Controller Note
A separate **pin controller** peripheral steers physical pins toward GPIO, SPI, MMC, etc. This is handled transparently by the userland GPIO API and configured via the device tree.

---

## 4. Implementation in Rust

### Why Rust?
- **Systems language** that eliminates entire classes of bugs at compile time
- Mature enough for production; still innovating rapidly
- Learning Rust makes you a better C/C++ programmer

### GPIO Data Model
```rust
#[derive(Serialize, Deserialize, Debug)]
enum GpioCmd {
    In,
    Out { value: bool },
}
```

### Key Design: File Descriptor Caching
The chardev GPIO API requires the **file descriptor to remain open** — closing it releases all GPIO lines. Solution: maintain a map of active pins:

```rust
type GpioPath = (String, u32);   // (chip name, pin number)

// Cache of active GPIO chips, shared across requests
let active_pins = BTreeMap::<GpioPath, Chip>::new();
let shared_pins_state = Arc::new(RwLock::new(active_pins));
```

### GPIO Modify Function
```rust
fn gpio_modify(chip: String, pin: u32,
               pins: Arc<RwLock<BTreeMap<GpioPath, Chip>>>,
               body: GpioCmd)
    -> GpioModifyResult
{
    let mut shared_pins = pins.write().unwrap();
    let mut our_pin_entry = shared_pins.entry((chip.clone(), pin));

    // Open chip if not already cached
    let chipdev = match our_pin_entry {
        Occupied(ref mut entry) => entry.get_mut(),
        Vacant(entry) => entry.insert(Chip::new(format!("/dev/{}", chip))?)
    };

    let line = chipdev.get_line(pin)?;

    match body {
        GpioCmd::Out { value } => {
            line.request(LineRequestFlags::OUTPUT, 0, "http-gpio")?
                .set_value(value as u8)
        }
        GpioCmd::In => {
            line.request(LineRequestFlags::INPUT, 0, "http-gpio")?;
            Ok(())
        }
    }
}
```

### Web Server (Warp Framework)

**Why Warp:**
- Runs on **stable Rust** (unlike Rocket at the time)
- **Lightweight** dependency list — important for embedded CPU/memory constraints
- Extremely straightforward setup

```rust
let with_pins_state = warp::any().map(move || shared_pins_state.clone());

// POST /gpio/chipname/pinnum -> String
let gpio_modify = warp::post()
    .and(warp::path!("gpio" / String / u32))
    .and(with_pins_state)
    .and(warp::body::json())
    .map(gpio_modify)
    .map(as_reply);
```

Error handling returns the GPIO error as an HTTP 500:
```rust
fn as_reply(value: GpioModifyResult) -> Box<dyn warp::Reply> {
    match value {
        Ok(_) => Box::new("Success"),
        Err(err) => Box::new(
            warp::reply::with_status(err.to_string(),
                                     StatusCode::INTERNAL_SERVER_ERROR))
    }
}
```

**Total: ~70 lines of Rust** for a functional platform daemon.

---

## 5. Testing on Workstation

### Simulate GPIO with gpio-mockup
```bash
modprobe gpio-mockup gpio_mockup_ranges=-1,32,-1,32
ls -lh /dev/gpio*
# /dev/gpiochip0
# /dev/gpiochip1
```
Creates two dummy GPIO peripherals with 32 pins each.

### Exercise the API
Using **httpie** (`http` command — "a better curl"):
```bash
http POST localhost:3030/gpio/gpiochip0/2 Out:='{"value": true}'
```

Sends JSON:
```json
{ "Out": { "value": true } }
```

Response:
```
HTTP/1.1 200 OK
Success
```

Malformed requests are rejected automatically by the framework.

---

## 6. Ideas for Improvement

- **Configuration file** (YAML or INI) in `/etc/` to specify:
  - Which GPIO character device to use
  - String aliases for pins (e.g., `POST /gpio/yellow-led`)
- **Device tree GPIO names** — provide abstraction at the kernel level for all of userland
- Sample config in the project README; actual config in the target's overlay directory

---

## 7. Key Takeaways

- If you're doing anything non-trivial with your embedded system, you'll end up with a **platform daemon**
- The platform daemon contains **target-specific knowledge** — routes data between peripherals, manages system state
- Use the **modern character device API** (`/dev/gpiochipX`) for GPIO, not the deprecated sysfs interface
- **GPIO file descriptors must stay open** — cache them for the daemon's lifetime
- **Develop and test on your workstation** using mock drivers (e.g., `gpio-mockup`) before porting to hardware
- **Rust** is excellent for platform daemons: memory safety, native performance, lightweight binaries
- **Avoid heavy frameworks** (e.g., Python + Flask) for platform daemons — startup time and runtime performance matter on embedded
- **Wear multiple hats:** think about maintainability while writing; you're likely also the maintainer

---

## 8. Recommended Reading

- **LWN.net** — in-depth Linux kernel articles (free after delay; subscription for immediate access)
  - *GPIO in the Kernel: an introduction*
  - *GPIO in the Kernel: future directions*
- **Character device GPIO tutorial** — command-line GPIO control
- **Hackaday's Linux-Fu series** — various technical userland topics useful for daemon development
