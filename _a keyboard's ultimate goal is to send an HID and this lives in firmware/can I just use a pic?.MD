# Can I just use a PIC?

Yes.

For a keyboard, a PIC can absolutely be the microcontroller that sits between the physical switches and the USB cable. The important qualification is that **PIC is a family, not one chip**, and the USB side of the job narrows the choice substantially.

If the goal is a wired USB keyboard with as little surrounding machinery as practical, the clean version of the idea is:

> **Use a PIC that already contains a USB device peripheral, give it enough GPIO for the keyboard matrix, and let its firmware turn matrix positions into USB HID reports.**

That is a perfectly ordinary architecture. Microchip has even published USB keyboard/keypad examples for small USB-capable PICs.

The current movement prototype in this repository uses an RP2040. Replacing it with a PIC would therefore be a real hardware choice, not merely compiling the existing firmware for a different target. The matrix concept can stay the same, the USB connector can stay, and the high-level firmware job stays the same, but the processor support circuit, programming interface, clocking details, USB peripheral setup, and firmware binary all become PIC-specific.

## What does the microcontroller actually have to accomplish?

It helps to strip the keyboard down to the actual chain of jobs.

```text
switch closes
    |
    v
row/column electrical state changes
    |
    v
PIC scans the matrix
    |
    v
PIC debounces the switch
    |
    v
firmware identifies the key
    |
    v
firmware updates a USB HID input report
    |
    v
USB host asks for input
    |
    v
PIC USB peripheral returns the report
```

None of the first four steps requires an RP2040. None requires an ARM processor. None requires 32 bits. A small 8-bit processor is entirely capable of driving rows, reading columns, counting enough time to debounce a switch, looking up a key identity, and maintaining a few bytes of report state.

The part that changes the hardware question is USB. Scanning a matrix is extremely easy for almost any microcontroller. Participating correctly as a USB device is much easier if the silicon already contains the USB transceiver and device peripheral.

## So can I use the simplest possible PIC?

Maybe, but “simplest PIC” and “simplest USB keyboard” are not exactly the same question.

A tiny PIC with no native USB hardware could easily scan the switches. Then, however, it still needs some way to communicate with the computer. The possibilities would be something like:

1. add a separate USB interface chip;
2. implement some USB signaling in software, if the exact processor, speed, electrical interface, and USB mode make that possible; or
3. abandon USB and use some other electrical protocol.

All three defeat part of the reason for choosing the tiny PIC in the first place.

For this project, I would draw the line this way:

> **The simplest relevant PIC is not the PIC with the fewest transistors. It is the simplest PIC that already has the hardware needed to be the USB keyboard.**

Two concrete examples are useful:

- **PIC16F1459** — a small 8-bit PIC with an integrated USB 2.0 full-speed device module. Microchip has a USB keypad application note built around this family.
- **PIC18F25K50** — a larger 8-bit PIC18 with native USB and substantially more GPIO, which may be more comfortable when the physical keyboard matrix gets larger.

Those are examples, not yet a final part selection. Before putting one on the PCB we should count the exact matrix rows and columns, LEDs or other outputs, programming pins, and any other peripherals we actually need.

## GPIO is probably the next constraint after USB

Suppose a keyboard matrix has eight rows and eight columns. That gives 64 possible switch positions while consuming 16 matrix signals:

```text
8 row outputs
8 column inputs
-------------
16 GPIO
```

The processor also needs its USB pins, power pins, reset/programming connections, and perhaps LEDs or other controls. A 20-pin device can become tight quickly even though its CPU is more than powerful enough.

This is why the number of keys does not translate directly into the number of processor pins. A matrix compresses the wiring very effectively, but the number of **rows + columns** still matters when choosing the package.

There are other ways to reduce pin count — shift registers, I/O expanders, multiplexers — but again they add parts and protocol. If our goal is to understand the keyboard directly, a PIC with enough native GPIO is probably clearer than choosing the smallest possible package and then surrounding it with helpers.

## What would the PIC firmware contain?

At a high level, very little magic is required.

The main loop could conceptually be as plain as:

```text
initialize clock
initialize GPIO
initialize USB device

forever:
    scan matrix
    debounce changes
    translate matrix positions to semantic keys
    construct current HID report
    service USB
```

The real code has more state because USB is a protocol, switches bounce, and presses and releases must both be represented correctly. But the architecture can remain small and legible.

For a matrix scan, one row is driven at a time. The column pins are read. If row 3 is active and column 7 reads as closed, the firmware knows only this at first:

```text
row 3, column 7 is closed
```

Then a table gives that physical position meaning:

```text
row 3, column 7 -> LAMBDA
```

or:

```text
row 3, column 7 -> EQUAL_QUESTION
```

That semantic identity can remain distinct inside our firmware even if the compatibility layer eventually has to emit ordinary keyboard usages or a sequence of characters.

This is an important separation in this project:

```text
physical matrix position
        !=
semantic key identity
        !=
USB representation
        !=
Unicode character on the screen
```

The PIC does not need to confuse those layers merely because conventional keyboards often make them look like one thing.

## What exactly gets sent over USB?

Saying “the keyboard sends an HID keycode” is a useful shorthand, but the more exact statement is:

> **The firmware presents a USB HID interface and supplies HID input reports whose fields contain keyboard usages or other values described by the HID report descriptor.**

For a conventional boot-compatible keyboard, a familiar report format contains modifier state plus a small set of simultaneously pressed keyboard usage IDs. Other HID report descriptors can represent more keys, bitmaps for larger rollover, media controls, or project-specific semantic information.

The important thing for the PIC question is that the processor does not need an operating system. It needs enough firmware to:

- enumerate as a USB device;
- describe its HID interface;
- respond to the standard control requests required during setup;
- maintain its input report state; and
- make that report available through the interrupt-IN endpoint when the host schedules the transfer.

The USB peripheral handles the low-level electrical signaling and much of the packet machinery. The firmware still has to implement the USB device behavior above that hardware.

## Does it have to be written in C?

No.

Ultimately the PIC executes machine instructions for that particular PIC architecture. C is only one way to produce those instructions.

You could write keyboard firmware in:

- PIC assembly;
- C using Microchip XC8 or another suitable compiler;
- another language that has a working backend/runtime for the selected PIC; or
- a mixture, with low-level routines in assembly and most protocol logic in C.

If the question is “what is the lowest practical source language?”, assembly is below C. A matrix scanner and debounce routine are small enough that writing them in assembly is quite reasonable if the purpose is to see exactly what the processor does.

USB is where the tradeoff changes. USB enumeration, descriptors, endpoint state, setup requests, and error cases create enough bookkeeping that a pure assembly implementation becomes a larger exercise. It is still possible; it just stops being the obviously simplest way to get a working keyboard.

A useful first implementation could therefore be very conservative:

```text
C:
    reset/setup
    USB device state machine
    HID descriptors and reports

very small, inspectable routines:
    scan rows
    read columns
    debounce
    map position -> semantic key
```

Nothing prevents replacing more of it with assembly later. The important design goal is that each layer remains understandable rather than hiding the entire keyboard behind a giant framework.

## How would I flash a PIC?

This is one of the concrete places where a PIC board differs from the present RP2040 prototype.

The RP2040 has a ROM USB BOOTSEL mechanism and normally runs firmware from external QSPI flash. Many small PICs instead contain their program flash **inside the microcontroller** and are commonly programmed through Microchip's in-circuit programming interface.

For a typical PIC board we would deliberately expose the programming signals used by the selected device. A common Microchip ICSP arrangement includes:

```text
programmer                 keyboard PCB

VPP / MCLR  -------------- MCLR / programming voltage
VDD         -------------- target supply/reference
VSS         -------------- ground
PGD         -------------- programming data
PGC         -------------- programming clock
```

A PICkit can then program the firmware without removing the chip from the board. The exact electrical requirements, low-voltage-programming options, and pin assignments must come from the datasheet for the exact PIC we choose.

That gives us a very understandable physical story:

```text
source code
   |
compiler / assembler
   |
PIC machine-code image
   |
PICkit + ICSP pins
   |
internal PIC program flash
   |
power/reset
   |
PIC executes keyboard firmware
```

We could later add a USB bootloader if we want ordinary users to reflash the keyboard without a PICkit. But that is a separate convenience layer. It is not necessary for the first proof that the keyboard works.

## What changes electrically if we switch the current board from RP2040 to PIC?

The keyboard matrix itself does not need to change merely because the processor changes. A switch still closes a row/column path. Diodes still do the same anti-ghosting job. Pull-ups or pull-downs still establish known input states. The USB cable is still USB.

The processor support circuit would change, though.

The RP2040-specific pieces include its external QSPI flash and its particular BOOTSEL/debug arrangement. A USB-capable PIC with internal flash can eliminate the separate firmware flash chip, but it introduces its own requirements:

- PIC power and decoupling arrangement;
- MCLR/reset circuit as required;
- ICSP programming header or pads;
- the exact clock arrangement required for reliable USB on that device;
- any USB regulator/capacitor pins required by that PIC family;
- the PIC's D+ and D- pins routed correctly to the USB connector; and
- enough GPIO pins routed to all matrix rows and columns.

So “use a PIC” can make the processor conceptually smaller while still requiring a fresh schematic review. We should never assume that an RP2040 support circuit can simply have its processor symbol replaced with a PIC symbol.

## Is the PIC actually simpler than the RP2040?

There are at least three different meanings of simple here.

### Simple processor

An 8-bit PIC is much smaller conceptually than the RP2040. There is less processor to understand, and keyboard firmware does not need two ARM Cortex-M0+ cores or the RP2040's more elaborate peripherals merely to scan switches and speak HID.

### Simple first-time flashing

The RP2040 is unusually friendly here because its BOOTSEL loader is in ROM and accepts UF2 over USB. A bare PIC board commonly expects an ICSP programmer unless we provide a bootloader.

### Simple PCB

A USB-capable PIC with internal program flash can remove the RP2040's separate QSPI flash. On the other hand, the exact PIC may have its own clock, voltage, USB, and programming requirements.

So there is no universal winner called “simplest.” The choice depends on what we are trying to make simple.

For this project, a PIC is especially attractive if the goal is:

> **I want to be able to account for almost every meaningful operation between a switch closing and a HID report existing.**

That is a good reason to prefer a deliberately small processor even when a larger processor would also work.

## A good first PIC experiment

Do not begin by porting the entire keyboard.

The smallest proof should be something like:

```text
one USB-capable PIC
one pushbutton
one pull-up/pull-down arrangement
one USB connector
one conventional HID keyboard report
```

Press the button. The PIC detects one input transition. Firmware changes one report field. The host receives one ordinary keyboard usage. Release the button. The report returns to released state.

Once that works, expand in visible steps:

1. one switch;
2. a 2 x 2 matrix;
3. matrix scanning;
4. debounce;
5. several simultaneous keys;
6. the real semantic key table;
7. the full matrix;
8. conventional HID plus whatever semantic HID representation we decide to expose.

That path isolates the questions. If USB does not enumerate when there is only one button, the matrix is not to blame. If a 2 x 2 matrix scans correctly before USB is added, a HID descriptor bug is not an electrical matrix bug.

## Provisional answer for this keyboard

Yes: **we can just use a PIC**, provided we mean a USB-capable PIC with enough pins for the chosen matrix.

I would not choose the exact part solely because “PICs are simple.” I would choose it by checking four things in this order:

```text
native USB device peripheral?
        |
enough GPIO for rows + columns + extras?
        |
enough program flash/RAM for our small HID firmware?
        |
programming/debug path we are happy to put on the PCB?
```

If those answers are good, an 8-bit PIC is not a toy or a workaround here. It is enough computer for the actual job.

And that may be the useful philosophical boundary for the keyboard: the processor should be only as complicated as necessary to turn **electrical switch state** into **meaningful HID report state**.

## Primary references

- [Microchip PIC16F1459 product page](https://www.microchip.com/en-us/product/pic16f1459) — small 8-bit PIC with integrated USB.
- [Microchip AN1546: USB Keypad](https://www.microchip.com/content/dam/mchp/documents/OTH/ApplicationNotes/ApplicationNotes/00001546A.pdf) — USB keypad example using the PIC16F1459.
- [Microchip PIC18F25K50 product page](https://www.microchip.com/en-us/product/PIC18F25K50) — 8-bit PIC18 family example with USB and more I/O.
- [Microchip USB HID keyboard documentation](https://onlinedocs.microchip.com/oxy/GUID-49CD424A-D8EB-4F60-95E5-12E07036AA34-en-US-4/GUID-29B87F62-8501-4FC8-A12C-7CEC705C9FAB.html) — HID keyboard support and report structure.
