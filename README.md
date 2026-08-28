# Linux Device Drivers

A progressive collection of Linux kernel modules built and tested on a **BeagleBone Black**, moving from a minimal "hello world" module up through pseudo character drivers, platform drivers, Device Tree integration, sysfs-based control, and a real GPIO driver with a userspace LCD application.

Each folder is a self-contained, cross-compilable kernel module.

## Modules

| # | Folder | What it demonstrates |
|---|--------|----------------------|
| 001 | [`001_hello_world`](./001_hello_world) | Minimal loadable kernel module — `module_init`/`module_exit` |
| 002 | [`002_pseudo_char_driver`](./002_pseudo_char_driver) | Single pseudo character device, `file_operations`, manual major/minor allocation |
| 003 | [`003_pseudo_char_driver_multiple`](./003_pseudo_char_driver_multiple) | Multiple char devices from one driver, plus a userspace test app (`app/`) |
| 004 | [`004_pseudo_platform_drivers`](./004_pseudo_platform_drivers) | Linux platform driver model — separate platform device registration (`pcd_device_setup.c`) and platform driver (`pcd_platform_driver.c`) |
| 005 | [`005_pseudo_platform_drivers_dt`](./005_pseudo_platform_drivers_dt) | Same platform driver, now matched and probed via **Device Tree** (`of_device_id`) instead of static platform device registration |
| 006 | [`006_pcd_sysfs`](./006_pcd_sysfs) | Adds **sysfs attributes** (`max_size`, `serial_number`) to the platform device for runtime configuration/inspection from userspace |
| 007 | [`007_gpio_sysfs`](./007_gpio_sysfs) | Real hardware GPIO driver using the kernel `gpiod` consumer API, exposing `label`/`value` sysfs attributes per GPIO; includes a standalone LCD driver + userspace app (`lcd.c`, `lcd_app.c`) driven through `/sys/class/bone_gpios` |

`overlays/` contains a Device Tree overlay (`PCDEV0.dts`, compiled `.dtbo`) used to enable/configure the pseudo platform devices for modules 004–006.

## Hardware / target

- **Board:** BeagleBone Black
- **Kernel:** built against a matching kernel source tree (tested on 5.10.x TI kernel)
- **Toolchain:** `arm-linux-gnueabihf-gcc` (cross compiler)

## Building

Each module folder has its own `Makefile`. From inside a module directory:

```bash
make
```

By default this cross-compiles for ARM using the `KERN_DIR` kernel source path set at the top of the `Makefile` — **update `KERN_DIR` and `CROSS_COMPILE` to match your own kernel source tree and toolchain** before building.

To build against your host's running kernel instead (for local testing on x86, where applicable):

```bash
make host
```

To clean build artifacts:

```bash
make clean
```

Module 007 additionally builds the userspace GPIO/LCD application:

```bash
make app
```

This produces `lcd_app.elf`, cross-compiled from `lcd_app.c`, `lcd.c`, and `gpio.c`.

## Loading a module on the target

Copy the built `.ko` to the BeagleBone Black and load it:

```bash
scp <module>.ko debian@<board-ip>:~/
ssh debian@<board-ip>
sudo insmod <module>.ko
dmesg | tail
```

For the Device Tree–based modules (005 onward), load the corresponding overlay first (or apply it via `/sys/kernel/config/device-tree/overlays/` or your `uEnv.txt`, depending on kernel version) so the platform device matches and probes correctly.

Unload with:

```bash
sudo rmmod <module>
```

## Interacting with the drivers

- **Char drivers (002, 003):** interact via `/dev/<node>` using standard `read`/`write`/`open` (see `app/` in module 003 for an example).
- **Sysfs drivers (006, 007):** attributes appear under the device's entry in `/sys`. For example, module 007 exposes per-GPIO `label` and `value` files — `value` is both readable and writable to drive the pin, `label` is read-only.
- **LCD app (007):** `lcd_app.elf` talks to the LCD over GPIO pins via `/sys/class/bone_gpios`, using the helper functions in `gpio.c`/`gpio.h` (configure direction, read/write pin value).

## Repo layout

```
.
├── 001_hello_world/
├── 002_pseudo_char_driver/
├── 003_pseudo_char_driver_multiple/
│   └── app/                 # userspace test app
├── 004_pseudo_platform_drivers/
├── 005_pseudo_platform_drivers_dt/
├── 006_pcd_sysfs/
├── 007_gpio_sysfs/
│   ├── gpio_sysfs.c          # kernel driver
│   └── lcd.c / lcd_app.c     # userspace LCD app over sysfs GPIO
└── overlays/                 # Device Tree overlay source + compiled .dtbo
```

## Notes

- All modules are licensed `GPL` (see `MODULE_LICENSE` in each source file) and target learning/experimentation with Linux Device Driver (LDD) concepts rather than production use.
- Progression roughly follows: char drivers → platform driver model → Device Tree matching → sysfs interfaces → real hardware (GPIO/LCD).
