# How to flash the keyboard firmware

The current movement prototype uses an **RP2040** microcontroller. For an RP2040 board, the easiest normal flashing method is through the same USB connection the keyboard already uses.

## The short version

1. Write the keyboard firmware.
2. Build it into an RP2040 `.uf2` firmware file.
3. Put the RP2040 into **BOOTSEL** mode.
4. Plug the keyboard into the computer by USB.
5. The computer sees a removable drive named `RPI-RP2`.
6. Copy the `.uf2` file onto that drive.
7. The RP2040 writes the program into its external flash memory and reboots.
8. The keyboard now runs that firmware whenever it receives power.

This is conceptually similar to flashing an ESP32, AVR, or other microcontroller: put the chip into a programming mode, send it a compiled program, store that program in nonvolatile memory, and reboot into it. The physical programming interface differs from chip family to chip family.

## What is physically happening?

The USB cable provides both power and data. When the RP2040 starts normally, it reads the program stored in its external QSPI flash chip and executes it.

The RP2040 also contains a small first-stage bootloader permanently manufactured into read-only memory inside the chip. Holding **BOOTSEL** while the chip starts tells that ROM code not to boot the ordinary keyboard program. Instead it starts a USB bootloader.

The computer then sees the board as a USB mass-storage device named `RPI-RP2`. Copying a `.uf2` file to that device is not ordinary long-term file storage. The bootloader interprets the UF2 blocks and programs the external flash chip. When copying finishes, the board reboots and starts executing the newly written firmware.

Because the BOOTSEL bootloader lives in ROM, accidentally flashing broken keyboard firmware does not erase the recovery mechanism. You can return to BOOTSEL and replace the program.

## What should physically be on our PCB?

For a board that a human is expected to modify and fork, flashing should not require soldering wires onto tiny IC pins.

The board should deliberately expose at least:

- the normal USB-C connector;
- a **BOOTSEL** button or easily accessible BOOTSEL pads;
- preferably a **RESET** button or reset pads; and
- **SWD** programming/debug pads: `SWDIO`, `SWCLK`, and `GND`.

With a BOOTSEL button and USB, ordinary reflashing is simple. A reset control is useful because it can let someone enter BOOTSEL without physically unplugging the USB cable: hold BOOTSEL, reset the RP2040, then release BOOTSEL.

The current `pcb/movement-prototype.tsx` instantiates an RP2040 support circuit and USB-C interface, but the board-level source does not yet explicitly place user-accessible BOOTSEL, RESET, or SWD controls. Those should be reviewed before a production PCB is ordered.

## Why keep SWD if USB flashing already works?

USB BOOTSEL is enough for ordinary firmware installation. **SWD — Serial Wire Debug — is the lower-level development and recovery interface for the ARM cores inside the RP2040.**

A small external debugger, such as a Raspberry Pi Debug Probe or another CMSIS-DAP-compatible probe, connects approximately as:

```text
programmer/debug probe       keyboard PCB

SWDIO ---------------------- SWDIO
SWCLK ---------------------- SWCLK
GND   ---------------------- GND
```

The debugger itself connects to the development computer over USB.

SWD lets the development computer do more than copy firmware. It can program an ELF image directly, stop the processor, single-step instructions, inspect registers and memory, and set breakpoints. Raspberry Pi documents OpenOCD as one standard tool for doing this.

So the practical hierarchy is:

```text
normal user / ordinary update
        ↓
USB + BOOTSEL + UF2

firmware developer / difficult bug / board bring-up
        ↓
SWD debug probe
```

## What is the "jumper cord" I might remember?

That depends on the microcontroller family.

An ESP32 board is commonly flashed through USB-to-UART or native USB while boot-strapping pins place the chip into its download mode. Older AVR boards commonly use an ISP programmer. ARM microcontrollers often expose SWD. The RP2040 gives us an especially friendly option because its ROM already knows how to accept UF2 firmware over USB.

For this keyboard, the ordinary owner should therefore need only a USB cable plus access to BOOTSEL. A firmware developer should additionally be able to attach a three-wire SWD debugger.

## Where does the firmware actually live?

On an RP2040 system, the main program normally lives in a separate **QSPI flash memory chip** connected to the RP2040. The RP2040's ROM boot code knows how to load and run the program from that flash.

So, simplified:

```text
computer
   │
   │ USB
   ▼
RP2040 ROM bootloader
   │
   │ writes firmware
   ▼
QSPI flash
   │
   │ normal boot
   ▼
RP2040 executes keyboard firmware
```

The firmware then performs the keyboard-specific work: scan rows and columns, debounce switches, identify semantic keys such as `LAMBDA` or `EQUAL_QUESTION`, build USB HID reports, and respond to the host computer.

## First-board recommendation

For the first programmer-keyboard PCB, do not make firmware installation depend on a special programming fixture.

Provide:

```text
USB-C
BOOTSEL
RESET
SWDIO test pad
SWCLK test pad
GND test pad
```

Use **USB BOOTSEL/UF2 as the ordinary flashing path** and **SWD as the development/debug path**. That makes the board easy for another person to build, modify, recover, and fork.

## Primary references

- Raspberry Pi documentation, *Pico-series Microcontrollers* — BOOTSEL and UF2 flashing.
- Raspberry Pi documentation, *Raspberry Pi Debug Probe* — SWD wiring, OpenOCD, uploading and debugging.
