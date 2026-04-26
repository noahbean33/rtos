# Introduction to Embedded Linux Part 6 - Add Custom Application in Yocto

> Source: Digi-Key Electronics - Introduction to Embedded Linux Series

---

## 1. Overview

This episode covers:
- Writing a C application that reads temperature data from a TMP102 I2C sensor
- Creating a Yocto recipe to cross-compile and install the application
- Including the application in the custom image so it's available on boot

**Hardware:** TMP102 temperature sensor on I2C5 (`/dev/i2c-1`) of the STM32MP157D-DK1.

---

## 2. Development Options

| Method | Description |
|--------|-------------|
| **Yocto SDK** | Official approach; integrates with Eclipse for debugging; recommended for serious development |
| **Native development on similar board** | Use a Raspberry Pi or similar with GCC installed to develop/test before cross-compiling |
| **Vendor SDK** | ST provides an SDK specifically for the STM32MP1 series with peripheral access libraries |
| **Manual (shown here)** | Write source in the layer, cross-compile via BitBake — educational but painful for debugging |

> For production, use the Yocto SDK or vendor SDK. The manual approach shown here teaches the fundamentals.

---

## 3. Layer Directory Structure

Create the following structure inside `meta-custom`:

```
meta-custom/
├── recipes-apps/
│   └── mytemp/
│       ├── files/
│       │   └── src/
│       │       └── gettemp.c
│       └── gettemp_0.1.bb
├── recipes-core/
│   └── images/
│       └── custom-image.bb
├── recipes-kernel/
│   └── linux/
│       └── ...
└── ...
```

```bash
mkdir -p meta-custom/recipes-apps/mytemp/files/src
```

---

## 4. The C Application (`gettemp.c`)

### Required Headers
```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
```
Most of these libraries are baked into the kernel, available even in `core-image-minimal`.

### Constants
```c
#define TMP102_ADDR     0x48    // I2C address (from i2cdetect)
#define TEMP_REG        0x00    // Temperature register (read-only, 2 bytes)
#define I2C_DEVICE      "/dev/i2c-1"  // I2C5 maps to i2c-1 in Linux
```

### Program Flow

1. **Open the I2C device file** for read/write:
   ```c
   int file = open(I2C_DEVICE, O_RDWR);
   ```

2. **Set the I2C slave address** using `ioctl`:
   ```c
   ioctl(file, I2C_SLAVE, TMP102_ADDR);
   ```

3. **Write the register address** to select the temperature register:
   ```c
   unsigned char buf[2] = {TEMP_REG};
   write(file, buf, 1);  // Should return 1
   ```

4. **Read 2 bytes** of temperature data:
   ```c
   read(file, buf, 2);   // Should return 2
   ```

5. **Convert raw data to Celsius:**
   ```c
   int16_t temp_raw = (buf[0] << 4) | (buf[1] >> 4);
   if (temp_raw > 0x7FF) {
       temp_raw |= 0xF000;  // Sign-extend for negative values
   }
   float temp_c = temp_raw * 0.0625;
   ```

6. **Print and exit:**
   ```c
   printf("Temperature: %.2f C\n", temp_c);
   return 0;
   ```

### TMP102 Data Sheet Notes
- Temperature register is 12-bit, 2's complement
- Bit 0 of the first byte is the MSB of the temperature
- If the MSB (sign bit) is 1, the temperature is negative
- Multiply raw value by 0.0625 to get degrees Celsius
- Refer to the I2C timing diagrams in the datasheet for read/write protocol

### Error Handling
- Check return values of `open()`, `write()`, `read()`
- Use `exit(1)` (not `return`) to signal errors to the calling process/OS

> **Tip:** For a simpler first test, replace the I2C code with `printf("Hello World\n"); return 0;`

---

## 5. Creating the BitBake Recipe (`gettemp_0.1.bb`)

### Licensing

Yocto requires every recipe to specify a license. To use MIT:

1. Find it in Poky:
   ```bash
   ls poky/meta/files/common-licenses/MIT
   ```

2. Get its MD5 hash:
   ```bash
   md5sum poky/meta/files/common-licenses/MIT
   ```

### Recipe Contents

```bitbake
SUMMARY = "Get temperature from TMP102 I2C sensor"
DESCRIPTION = "A simple C application that reads temperature data from \
a TMP102 sensor connected via I2C and prints it to the console."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=<hash>"

SRC_URI = "file://src/gettemp.c"

S = "${WORKDIR}"

TARGET_CC_ARCH += "${LDFLAGS}"

do_compile() {
    ${CC} src/gettemp.c ${CFLAGS} ${LDFLAGS} -o gettemp
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 gettemp ${D}${bindir}
}
```

### Recipe Explanation

| Variable/Function | Purpose |
|-------------------|---------|
| `SUMMARY` | Short description of the recipe |
| `DESCRIPTION` | Longer description |
| `LICENSE` | License type (must be specified) |
| `LIC_FILES_CHKSUM` | MD5 hash of the license file — ensures integrity |
| `SRC_URI` | Where to find source files; `file://` = local; can also be `https://`, `git://`, `ftp://` |
| `S` | Source directory (working directory for the build) |
| `TARGET_CC_ARCH += "${LDFLAGS}"` | **Required** — passes linker flags to the cross-compiler; BitBake fails without this |
| `do_compile()` | Compilation step — calls GCC (cross-compiler for ARM); can use Makefiles or CMake here |
| `do_install()` | Installation step — copies binary to `${D}${bindir}` (`/usr/bin` on target); sets permissions to 755 |

### Source File Search Path
- `SRC_URI = "file://src/gettemp.c"` searches in `<recipe-dir>/files/src/`
- BitBake automatically looks in a `files/` subdirectory relative to the recipe
- For remote sources, use `https://`, `git://`, etc.

### Variable Quoting
All variable assignments in Yocto **must be quoted**:
```bitbake
S = "${WORKDIR}"     # Correct
S = ${WORKDIR}       # ERROR — will fail parsing
```

---

## 6. Including the Application in the Image

### Edit `custom-image.bb`

Append the recipe name (without version number) to `IMAGE_INSTALL`:

```bitbake
IMAGE_INSTALL += "gettemp"
```

This tells BitBake to run the `gettemp` recipe and include its output in the root filesystem during image creation.

---

## 7. Building

```bash
source poky/oe-init-build-env build-mp1
bitbake custom-image
```

- Should be relatively fast since only the root filesystem changes (no kernel rebuild)
- Fix any errors (common: missing quotes, typos in variable names)

### Verify the Binary Was Installed
```bash
ls tmp/work/stm32mp1-poky-linux-gnueabi/custom-image/*/rootfs/usr/bin/
```
Look for `gettemp` in the list.

---

## 8. Flashing and Testing

### Flash Only the Root Filesystem (bootfs unchanged)
```bash
sudo dd if=custom-image-stm32mp1.ext4 of=/dev/mmcblk2p5 bs=1M
```

> No need to reflash bootfs or update `extlinux.conf` if the kernel wasn't changed.

### Boot and Test
```bash
picocom -b 115200 /dev/ttyACM0
# Login: root / toor

# Verify the application is installed
ls /usr/bin/gettemp

# Run it
gettemp
# Output: Temperature: 24.50 C  (or similar)
```

- Run multiple times to watch temperature change
- Breathe on the sensor or expose to heat/cold to see values shift

### Shutdown Properly
```bash
shutdown now
```

---

## 9. Key Takeaways

- **Custom applications live in the custom layer** under `recipes-apps/<name>/files/src/`
- **Every recipe needs a license** — use `LIC_FILES_CHKSUM` with an MD5 hash
- **`SRC_URI`** can point to local files or remote repositories (GitHub, FTP, etc.)
- **`do_compile()`** handles cross-compilation — can invoke GCC directly or use Makefiles/CMake
- **`do_install()`** places the binary in the target filesystem (typically `/usr/bin`)
- **`IMAGE_INSTALL +=`** in the image recipe includes the application in the build
- **`TARGET_CC_ARCH += "${LDFLAGS}"`** is required in modern Yocto or BitBake will fail
- For production development, use the **Yocto SDK** or **vendor SDK** instead of manual vi + cross-compile
- **I2C user-space communication** works via `open()`, `ioctl()`, `write()`, `read()` on `/dev/i2c-*` device files
- Always quote variable assignments in BitBake recipes
