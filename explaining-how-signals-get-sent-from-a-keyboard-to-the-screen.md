# Explaining how signals get sent from a keyboard to the screen

This note follows one concrete example: pressing the physical **λ** key on the programmers keyboard and eventually seeing `λ` in an editor or terminal.

The central distinction is:

> **The physical key, the USB event, and the Unicode character do not have to be the same thing.**

That distinction is important for this keyboard. `EQUAL`, `EQUAL_QUESTION`, `DEFINED_AS`, `ASSIGN_LEFT`, and `ASSIGN_RIGHT` are different meanings even if compatibility software sometimes has to spell them with conventional ASCII sequences. Likewise `DEREFERENCE`, `POINTER_TYPE`, `GLOB`, and `SPLAT` should remain different semantic events even though an ordinary language may spell all four with `*`.

## 1. The switch changes an electrical state

A mechanical key switch is initially just an electrical component.

In a keyboard matrix, each switch sits at a row/column intersection. The microcontroller repeatedly drives one side of the matrix and senses the other. Pressing λ closes its switch and changes the electrical state at one row/column intersection. Diodes can prevent unwanted current paths when several switches are pressed simultaneously.

The controller also debounces the switch because mechanical contacts can make and break several times during one physical press.

At this point there is no Unicode character and no Linux, macOS, or Windows key meaning. The controller initially knows something equivalent to:

```text
row 2, column 5: released -> pressed
```

Our firmware can then map that matrix position to our own semantic identity:

```text
LAMBDA: pressed
```

## 2. Firmware makes a USB HID report

A USB keyboard normally presents itself as a **Human Interface Device (HID)**. Its HID report descriptor tells the host what fields and usages can occur in its reports.

The ordinary HID Keyboard/Keypad usage page mostly identifies keyboard controls, not arbitrary Unicode characters. A conventional keyboard therefore reports things corresponding to keys such as A, Enter, Left Shift, and so forth; the operating system later interprets those according to a layout.

USB HID usages describe the purpose of fields in reports. USB-IF explicitly warns that defining a HID usage does not imply that every host operating system supports it.

For this project there are two useful levels:

1. **Conventional keyboard collection.** Ordinary keys can use ordinary HID keyboard usages and work without special software.
2. **Semantic collection.** Our unusual controls can carry stable identities such as `LAMBDA`, `DEFINED_AS`, `EQUAL_QUESTION`, `DEREFERENCE`, or `END_INSTRUCTION`. A small user-space translator can decide how those meanings become text or editor actions.

A composite HID device can expose both at once. That is attractive because the keyboard remains usable as a normal keyboard while preserving the distinctions that motivated this project.

On an ordinary USB 2 connection the packets ultimately travel electrically over the differential D+ and D- pair. USB is host-scheduled: the host controller services the device's endpoint and receives its current input report.

The path so far is therefore approximately:

```text
finger
  ↓
mechanical switch closes
  ↓
row/column electrical state changes
  ↓
microcontroller scans matrix
  ↓
debounce
  ↓
firmware identifies LAMBDA
  ↓
HID input report
  ↓
USB packets
  ↓
host computer
```

## 3. Linux

For a conventional USB HID keyboard, the Linux path is roughly:

```text
USB host-controller driver
        ↓
usbhid / Linux HID core
        ↓
hid-input
        ↓
Linux input subsystem
        ↓
EV_KEY events through evdev (/dev/input/eventN)
```

The Linux input API represents an event with a **type, code, and value**. Keyboard-like state changes use `EV_KEY`; a press normally has value 1 and a release value 0. A `KEY_A` event means the A key in the Linux input vocabulary. It is not inherently the text character `a` or `A`.

On a typical graphical desktop, software above the kernel then handles layout and text interpretation. A Wayland compositor commonly consumes input through libinput, and XKB/libxkbcommon maps keycodes plus modifiers/layout state to keysyms and Unicode text when appropriate.

That is the layer where a conventional physical key can become different characters under different layouts.

### Linux and our semantic keys

Linux also provides **hidraw**, which exposes HID reports to user space without first forcing every field through the normal keyboard interpretation. The Linux kernel documentation specifically describes hidraw as useful for user-space drivers and custom HID devices.

So the programmers keyboard does **not** inherently need a new kernel driver merely because it has a λ key.

A clean project-specific path can be:

```text
physical λ key
→ firmware event LAMBDA
→ custom HID report
→ /dev/hidrawN
→ programmers-keyboard user-space translator
→ Unicode U+03BB (`λ`) or a semantic editor event
```

The translator could use a virtual input device when ordinary applications need to see conventional keyboard events, or communicate directly with an editor/terminal that understands our semantic protocol.

This is preferable to pretending that every unusual key is some existing QWERTY key, because we retain its meaning all the way to user space.

## 4. How `λ` becomes text

Once the text-input layer decides the desired character is Unicode **U+03BB GREEK SMALL LETTER LAMDA**, the character is represented as Unicode text.

In UTF-8:

```text
λ        U+03BB
UTF-8    CE BB
```

If the destination is a terminal emulator, the terminal's input handling writes the UTF-8 bytes to the pseudoterminal attached to the shell, REPL, Vim, or other program.

If the destination is a graphical editor, its GUI toolkit/text-input API can deliver Unicode text directly rather than going through a pseudoterminal.

## 5. macOS

The bottom of the stack is conceptually similar on macOS:

```text
USB HID report
→ Apple's HID stack
→ keyboard/HID events
→ keyboard layout or text-input system
→ application
→ Unicode text
```

Apple exposes HID facilities through IOKit/DriverKit/CoreHID. Standard keyboard HID input is handled by the operating system; applications may also communicate with HID devices and custom reports.

That gives this project essentially the same architectural choice as Linux: ordinary controls can use the standard keyboard path, while semantic controls can be carried in a custom HID collection and translated in user space.

A special λ key therefore does not imply that we must write a macOS kernel extension.

## 6. Windows

Windows likewise separates raw keyboard/HID input from character input.

For ordinary keyboards, Microsoft's documentation describes the keyboard driver converting reported HID usages into keyboard scan-code/key events. Windows then applies keyboard-layout and input processing and can deliver character messages such as Unicode text to an application.

Windows also exposes **Raw Input**, including raw HID data. A project-specific user-space program can therefore receive custom HID reports without treating every semantic key as an ordinary PC keyboard key.

Conceptually:

```text
USB HID report
→ Windows HID / keyboard stack
→ key event or Raw Input
→ keyboard layout / text services when appropriate
→ application
→ Unicode text
```

Again, a special λ key does not by itself require a custom kernel-mode keyboard driver.

## 7. Finally: text becomes pixels

The character is still not a picture when the application receives it. It is a Unicode value in a string.

For a graphical program, the remainder is approximately:

```text
Unicode `λ`
  ↓
application's text buffer
  ↓
font selection
  ↓
glyph lookup and text shaping
  ↓
glyph rasterization
  ↓
window surface / graphics commands
  ↓
compositor
  ↓
GPU / display engine
  ↓
framebuffer or scan-out image
  ↓
display pixels
```

The font maps U+03BB to a glyph design. A rasterizer converts that outline/bitmap into pixel coverage. The window system/compositor combines the application's surface with the rest of the desktop, and the display hardware scans the resulting image to the screen.

Only at the far end does the abstract character become illuminated pixels that look like **λ**.

## 8. The useful architecture for this keyboard

The smallest architecture that preserves this project's semantics is probably:

```text
                            ┌─ ordinary HID keyboard usages ─→ normal OS keyboard path
physical keys → firmware ──┤
                            └─ semantic HID reports ─────────→ small user-space translator
                                                                  ↓
                                      ┌───────────────────────────┼────────────────────────┐
                                      ↓                           ↓                        ↓
                                  Unicode text              editor action           terminal action
```

Examples of semantic events worth keeping distinct include:

```text
LAMBDA
EQUAL
EQUAL_QUESTION
DEFINED_AS
ASSIGN_LEFT
ASSIGN_RIGHT
COMPOSE
ADDRESS
DEREFERENCE
POINTER_TYPE
GLOB
SPLAT
END_INSTRUCTION
START_TEXT
PAIRED_GUILLEMETS
PAIRED_CURLY_QUOTES
OPEN_CORNER
CLOSE_CORNER
```

The translator is where a meaning gets a compatibility spelling. For example:

```text
LAMBDA             → λ
EQUAL              → =
EQUAL_QUESTION     → ≟
DEFINED_AS         → ≝
ASSIGN_LEFT        → ←
ASSIGN_RIGHT       → →
END_INSTRUCTION    → ;
```

The keycap does not have to display the literal compatibility character. A semicolon-producing key may therefore be labeled **END INSTRUCTION** while the semantic event remains `END_INSTRUCTION` and its default text mapping is `;`.

Likewise the corner characters `┌` and `┘` are not floor and ceiling. In this keyboard they belong to the delimiter/text-boundary vocabulary alongside paired guillemets, paired curly quotes, angle delimiters, and block/heredoc text controls.

## 9. What actually needs a driver?

For the first prototype, probably **no new kernel driver**.

Use the operating systems' existing USB HID support. Put normal controls in a normal keyboard HID collection. Put project-specific semantic events in a custom/vendor-defined HID collection and translate those reports in a small user-space program.

A kernel driver becomes interesting only if the operating system itself should understand those semantic events as a new system-wide input vocabulary rather than having a user-space translator do it.

That lets the hardware and firmware establish a clean semantic protocol now without making Linux, macOS, and Windows kernel work a prerequisite for the first keypad.

## Primary references

- USB-IF, *Human Interface Devices (HID) Specifications and Tools* and *HID Usage Tables*.
- Linux kernel documentation, *Introduction to HID report descriptors*, *HIDRAW — Raw Access to USB and Bluetooth Human Interface Devices*, and *Input event codes*.
- libinput documentation, *Keyboard events*.
- libxkbcommon documentation, *Keysyms* and the XKB keymap format.
- Apple Developer Documentation, *Core HID*, *HIDDriverKit*, and *Handling Keyboard Events from a Human Interface Device*.
- Microsoft Learn, *Keyboard Input Overview* and *Raw Input Overview*.
