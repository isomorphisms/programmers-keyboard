# How a key press travels electrically

Take one physical key, such as **λ**.

The first important fact is that there is **not** one copper trace that carries a special λ-shaped electrical signal all the way from the switch to the PC. The trace only lets the keyboard's microcontroller discover that a particular switch has changed state. After that, the microcontroller represents the press as digital data and participates in the USB protocol.

## 1. Power arrives from USB

For an ordinary wired keyboard, USB provides power as well as communication. The board receives USB VBUS and ground. Depending on the chosen microcontroller and circuit, the board may use that supply directly where appropriate or regulate it to the voltage the electronics require.

Small capacitors near the microcontroller and other ICs are normally used for decoupling: they help keep the local supply voltage steady when the digital electronics switch rapidly. A normal wired keyboard does **not** need a battery merely to smooth USB power.

Conceptually:

```text
USB VBUS ---- power/regulation ---- microcontroller + keyboard electronics
USB GND  ------------------------- common return
```

The USB data wires are separate from the power path.

## 2. The switch changes an electrical connection

Keyboard switches are commonly arranged as a **matrix** so that dozens of switches do not require dozens of independent microcontroller pins.

A simplified matrix looks like this:

```text
              columns
              C0   C1   C2
               |    |    |
row R0 --------o----o----o
               |    |    |
row R1 --------o----o----o
               |    |    |
row R2 --------o----o----o
```

Each `o` is a switch position. Pressing a key closes one row-to-column electrical path.

The microcontroller repeatedly scans the matrix. One common arrangement is:

1. drive one row to a known electrical level;
2. read the column inputs;
3. drive the next row;
4. read again;
5. repeat very quickly.

Suppose λ is physically at `R1,C2`. When that switch closes, the level observed on C2 changes while R1 is being scanned. The firmware can therefore infer:

```text
R1,C2 changed from open to closed
```

Per-switch diodes are commonly added so that several simultaneous key presses cannot create misleading current paths through other switches. This is one reason our two independent `TWO SPACES` switches can coexist cleanly: each is an independently detectable matrix position.

## 3. Debouncing

A mechanical switch does not usually go electrically from perfectly open to perfectly closed in one ideal instant. Its contacts can bounce for a short time.

Firmware therefore waits for a stable state or otherwise filters the transitions. After debouncing, it records one real press rather than several accidental presses.

```text
raw electrical transitions
        |
        v
     debounce
        |
        v
one logical key press
```

## 4. Firmware gives that matrix position meaning

The matrix itself does not know Greek. Firmware contains the mapping:

```text
R1,C2 -> LAMBDA
```

For this project, keeping the semantic identity is useful. The firmware can distinguish things such as:

```text
LAMBDA
EQUAL
EQUAL_QUESTION
DEFINED_AS
ASSIGN_LEFT
ASSIGN_RIGHT
END_INSTRUCTION
DOUBLE_SPACE_LEFT
DOUBLE_SPACE_RIGHT
```

rather than immediately pretending that every key is some old PC keyboard key.

## 5. USB is not a keypress pulse wire

USB uses separate differential data lines, D+ and D-. The microcontroller's USB peripheral and firmware encode USB packets onto those wires.

A USB device does not simply notice λ and launch an unsolicited electrical pulse down the cable saying `λ`. USB transfers are **host initiated**. The PC's USB host controller schedules/polls the keyboard's input endpoint. When asked for input, the keyboard responds with the current report or relevant data.

A simplified path is:

```text
λ switch closes
   |
   v
matrix voltage changes
   |
   v
microcontroller detects R1,C2
   |
   v
firmware records LAMBDA pressed
   |
   v
PC polls USB input endpoint
   |
   v
keyboard returns HID report
   |
   v
D+ / D- carry encoded USB packets
   |
   v
PC receives report
```

That distinction is central: **the copper matrix trace carries a switch state; USB carries a protocol.**

## 6. What happens after the PC gets the report

For a conventional HID keyboard report, the operating system's existing USB/HID/input stack turns the report into key events. Layout and text-input software above that may then turn a key event into Unicode text.

For our unusual semantic controls, we can also expose a project-specific HID report and let a small user-space translator turn `LAMBDA` into `λ`, `END_INSTRUCTION` into `;`, or a semantic command into an editor action.

The screen comes much later. Eventually the application receives Unicode text, a font maps characters to glyphs, the graphics stack rasterizes those glyphs, and pixels are displayed.

## The whole path in one picture

```text
USB power
   |
   v
keyboard electronics powered

finger
   |
   v
switch closes
   |
   v
row/column electrical path changes
   |
   v
microcontroller scans + debounces
   |
   v
semantic key identity
   |
   v
USB HID report
   |
   v
host-initiated USB transaction over D+ / D-
   |
   v
OS / translator
   |
   v
Unicode or editor action
   |
   v
font + graphics
   |
   v
pixels
```

The editable schematic tells us **why the electrical connections exist**. The PCB layout tells us **where the copper physically runs**. The Gerbers tell the board manufacturer **what geometry to fabricate**. Firmware tells the microcontroller **what the detected switch means**.
