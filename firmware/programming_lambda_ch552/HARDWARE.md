# CH552 controller contract for the Lambda programming keypad

This is the CH552 hardware contract used by `programming_lambda_ch552.ino`. It is intentionally smaller than a claim that the final Programming-keypad PCB is finished.

## Controller

Use a CH552G in SOP-16 for the first controller prototype.

The choice follows the CH55xduino CH552 three-key keyboard reference design, which uses CH552G/SOP-16 and direct native USB. The Lambda firmware uses the CH55xduino convention `port * 10 + bit` for GPIO numbers.

## Native USB

| CH552 pin | Function |
| --- | --- |
| P3.6 | USB D+ |
| P3.7 | USB D- |
| VCC | USB 5 V supply |
| GND | ground |
| V33 | internal 3.3 V regulator node / decoupling |

Keep P3.6 and P3.7 out of the key matrix.

Use local 0.1 uF decoupling at VCC and V33, following the CH55xduino CH552 keyboard reference. The reference feeds VCC from +5 V.

## 3x4 switch matrix

The matrix keeps the same electrical direction as `pcb/movement-prototype.tsx`: column -> switch -> diode anode -> diode cathode -> row.

| Matrix line | CH552 pin | CH55xduino number |
| --- | --- | ---: |
| ROW0 | P1.4 | 14 |
| ROW1 | P1.6 | 16 |
| ROW2 | P1.7 | 17 |
| COL0 | P3.0 | 30 |
| COL1 | P3.1 | 31 |
| COL2 | P3.2 | 32 |
| COL3 | P3.3 | 33 |

The firmware leaves rows high-impedance except for the row currently being scanned. Columns use internal pull-ups. The selected row is driven low, so a closed switch reads low at its column.

P1.5 is deliberately not assigned to the matrix. CH55xduino can use it as a bootloader-select input, and keeping it free costs nothing here.

## Current Lambda proof binding

Until the final Programming-keypad placement is routed:

- ROW0/COL0 -> `ACTION_LAMBDA`
- the other eleven matrix positions -> deliberately unbound

That is the same proof boundary as the RP2040 implementation. Moving Lambda to its eventual physical switch changes the action table, not the USB encoder.

## What is not being claimed yet

This controller contract settles the MCU, USB pins, matrix pins, matrix electrical direction, and firmware interface. It does **not** claim that the final Lambda/Programming keypad key placement, connector footprint, board outline, legends, or routing are finished.
