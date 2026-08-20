# RP2040 samples for the remaining keypad layouts

This directory extends the existing RP2040/TinyUSB pattern without claiming that the remaining keypad PCBs have been routed.  It builds one UF2 per currently unimplemented layout:

- Concept Separation
- Incantation Assistance
- Math
- Regular Expressions
- Several Pastebins
- Signals

The routed Movement board remains in `firmware/movement-rp2040`.  The Programming `λ` proof remains on `firmware/programming-lambda`; this directory does not replace either of them.

## Hardware assumption for these samples

The remaining layouts do not yet have checked-in PCB wiring, and Regex currently needs thirteen distinct actions.  The sample therefore assumes one common diode-isolated 4×4 switch matrix:

- rows: GPIO0, GPIO1, GPIO2, GPIO3
- columns: GPIO4, GPIO5, GPIO6, GPIO7
- columns use RP2040 internal pull-ups
- one row at a time is driven low; inactive rows float
- a closed switch reads low on its column
- five consecutive 1 ms samples are required before a state change is accepted

This is **not** the routed Movement prototype pinout.  Movement is 3×4 with rows GPIO0–GPIO2 and columns GPIO3–GPIO6.  These UF2s are samples for breadboarding or later PCB work; do not flash one onto the Movement board and assume its physical key positions match.

All tables below use row-major matrix positions: position 0 is row 0/column 0, position 1 is row 0/column 1, and so on.  Unlisted positions do nothing.

## Concept Separation

| Position | Current meaning | Sample host encoding |
| ---: | --- | --- |
| 0 | QUAD SPACE | U+2001 EM QUAD via Linux `Ctrl+Shift+U 2001 Enter` |
| 1 | FOUR SPACES | four ASCII spaces |
| 2 | DOUBLE SPACE | two ASCII spaces |
| 3 | SINGLE SPACE | one ASCII space |
| 4 | INDENT | `Tab` |
| 5 | UNDERSCORE | `_` |

`QUAD SPACE` is kept as a distinct semantic action from `FOUR SPACES`; this sample interprets the former as the Unicode EM QUAD rather than silently making the two keys identical.

## Incantation Assistance

| Position | Current meaning | HID output |
| ---: | --- | --- |
| 0 | TAB / COMPLETE | `Tab` |
| 1 | FINISH INCANTATION | `Enter` |
| 2 | CHANT HISTORY | `Ctrl-R` |

## Math

Unicode operators use the same Linux Unicode-input approach as the existing Programming `λ` proof.

| Position | Current meaning | Sample host encoding |
| ---: | --- | --- |
| 0 | ADD | `+` |
| 1 | SUBTRACT | U+2212 `−` |
| 2 | MULTIPLY | U+00D7 `×` |
| 3 | DIVIDE | U+00F7 `÷` |
| 4 | EQUAL | `=` |
| 5 | NOT EQUAL | U+2260 `≠` |
| 6 | `<` side of `< / ≤` | `<` |
| 7 | `≤` side of `< / ≤` | U+2264 `≤` |
| 8 | `>` side of `> / ≥` | `>` |
| 9 | `≥` side of `> / ≥` | U+2265 `≥` |

The source layout groups `< / ≤` and `> / ≥` as paired meanings.  Because no final rocker/switch hardware is defined, the sample gives the two sides separate matrix positions.  A future two-way rocker can bind its two directions to the same two semantic actions.

## Regular Expressions

This encoder targets PCRE2/Perl-like syntax on a US keyboard layout.  The semantic names stay separate from that dialect choice.

| Position | Current meaning | Sample text |
| ---: | --- | --- |
| 0 | START OF LINE | `^` |
| 1 | END OF LINE | `$` |
| 2 | END OF SLURP | `\z` |
| 3 | maybe (ok if not?) | `?` |
| 4 | big capture (greedy) | `(.*)` |
| 5 | small capture (non greedy) | `(.*?)` |
| 6 | group of characters in exactly this order | `()` |
| 7 | any character from this list | `[]` |
| 8 | letter | `\p{L}` |
| 9 | number | `\d` |
| 10 | non weird character | `\w` |
| 11 | weird character | `\W` |
| 12 | incantation runes / control characters | `\p{Cc}` |

If another regex dialect is desired, change only these text encodings; the matrix positions and semantic names do not need to change.

## Several Pastebins

The repository defines pastebin operations but not a universal host shortcut or daemon.  Guessing clipboard shortcuts would change the meaning.  The sample therefore emits otherwise-unused extended function-key usages for a host binding layer:

| Position | Current meaning | HID transport token |
| ---: | --- | --- |
| 0 | VIEW₁ | `F13` |
| 1 | VIEW₂ | `F14` |
| 2 | VIEW₃ | `F15` |
| 3 | REMEMBER₁ | `F16` |
| 4 | REMEMBER₂ | `F17` |
| 5 | REMEMBER₃ | `F18` |
| 6 | WRITE₁ | `F19` |
| 7 | WRITE₂ | `F20` |
| 8 | WRITE₃ | `F21` |

A host binding can then implement the actual three pastebins without changing firmware key identity.

## Signals

A USB keyboard cannot itself select an arbitrary process and call `kill(2)`.  These samples therefore preserve each signal as a distinct HID transport token rather than pretending an ordinary keystroke universally means SIGTERM or SIGKILL.

| Position | Current meaning | Firmware behavior |
| ---: | --- | --- |
| 0 | QUIT PROGRAM — SIGINT | `F13` token |
| 1 | SLEEP PROGRAM — SIGTSTP | `F14` token |
| 2 | WAKE PROGRAM — SIGCONT | `F15` token |
| 3 | SHUTDOWN — SIGTERM `[hold]` | after 1 s hold, `F16` token |
| 4 | QUIT AND DUMP — SIGQUIT | `F17` token |
| 5 | KILL PROGRAM — SIGKILL `[guarded]` | after 2 s hold, `F18` token |

The final SIGKILL key is still expected to have a physical guard/cover.  The 2 s software hold is additional sample protection, not a replacement for that hardware assumption.  Host software must bind the tokens to the intended process and exact signals.

## Build all six UF2 files

Use Raspberry Pi Pico SDK 2.2.0 and an Arm embedded GCC toolchain:

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S firmware/keypad-samples-rp2040 -B build/keypad-samples-rp2040
cmake --build build/keypad-samples-rp2040 --parallel
```

Outputs:

```text
build/keypad-samples-rp2040/concept_separation_keypad.uf2
build/keypad-samples-rp2040/incantation_keypad.uf2
build/keypad-samples-rp2040/math_keypad.uf2
build/keypad-samples-rp2040/regex_keypad.uf2
build/keypad-samples-rp2040/pastebins_keypad.uf2
build/keypad-samples-rp2040/signals_keypad.uf2
```

The GitHub Actions workflow uploads those six files together as `keypad-samples-rp2040-uf2`.

## Prototype USB identity

These samples use prototype VID/PID `CAFE:4005`.  As with the existing Movement firmware, allocate a real VID/PID before distributing a product.
