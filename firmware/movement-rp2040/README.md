# RP2040 Movement keypad firmware

This is the first firmware for the checked-in Movement keypad Gerbers. It targets the RP2040 already on that board and scans the existing 3×4 switch matrix:

- rows: GPIO0, GPIO1, GPIO2
- columns: GPIO3, GPIO4, GPIO5, GPIO6

The columns use the RP2040's internal pull-ups. One row at a time is driven low; the other rows float. The switch diodes on the PCB therefore make a pressed key read low on its column. Five consecutive 1 ms samples are required before a state change is accepted.

## What the twelve keys send

The first board is the Vim-oriented movement segment, so the semantic keys become ordinary USB HID keyboard reports:

| Key face | HID output |
| --- | --- |
| Movement | `Esc` |
| Type | `i` |
| Previous Line | `k` |
| Next Line | `j` |
| Start Line | `0` |
| End Line | `$` (`Shift+4`) |
| Next Word | `w` |
| Next Space-Separated Word | `W` (`Shift+w`) |
| 7 Times | `7` |
| 13 Times | `13` |
| Delete Character | `x` |
| Delete Word | `dw` |

Multi-character operations are emitted as press/release sequences, so `13` and `dw` are real key sequences rather than invented HID codes.

## Build a UF2

Install a recent Raspberry Pi Pico SDK and the Arm embedded GCC toolchain, then point `PICO_SDK_PATH` at the SDK checkout.

```sh
cd firmware/movement-rp2040
mkdir -p build
cd build
cmake ..
cmake --build . -j
```

The flashable file is:

```text
build/movement_keypad.uf2
```

The repository workflow also builds this UF2 and uploads it as the `movement-keypad-uf2` Actions artifact, so a local cross-compiler is not required just to obtain the firmware image.

## Flash it over the board's USB-C connector

A separate PIC programmer is not needed for this board. The checked-in RP2040 support circuit already has `SW_BOOT` and `SW_RUN` plus SWD test points.

1. Connect the board to a computer with a USB-C data cable.
2. Hold `SW_BOOT`.
3. Tap `SW_RUN`, then release `SW_BOOT`.
4. The RP2040 ROM should enumerate as the `RPI-RP2` USB mass-storage device.
5. Copy `movement_keypad.uf2` onto that drive. The board reboots into the firmware automatically.

If USB boot is ever unavailable, the `SWDIO`, `SWCLK`, `3V3`, and `GND` test points provide the lower-level recovery/programming path.

## Scope

This firmware deliberately matches the existing Movement prototype rather than pretending the rest of the keyboard PCB already exists. The USB VID/PID are TinyUSB-style prototype values (`CAFE:4004`); allocate a proper VID/PID before distributing a product.
