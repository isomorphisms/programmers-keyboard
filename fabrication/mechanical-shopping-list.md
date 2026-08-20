# Mechanical parts shopping list

This is a scalable shopping list for the small dedicated programmer keypads in this repository. The exact number of keys on each keypad is still allowed to move; the useful unit is therefore **one key position**.

The current Movement prototype uses a 19.05 mm key pitch, a hot-swap-style switch footprint, and one matrix diode per key. The existing fabrication package is still a prototype and should receive a human footprint/mechanical review before a large order of sockets, plates, or PCBs.

## One ordinary key position

For each normal MX-style key position, allow:

| Part | Required per key | Notes |
| --- | ---: | --- |
| MX-compatible mechanical switch | 1 | 3-pin or 5-pin is fine mechanically if the final PCB/plate supports it. |
| MX-compatible keycap | 1 | Usually 1u. Legends can be blank, labeled, printed, or custom. |
| MX hot-swap socket | 1 | Omit only if the final board intentionally solders switches directly. Match the socket to the final PCB footprint before ordering in bulk. |
| Matrix diode | 1 | Electrically rather than mechanically a key part, but it scales exactly with key count. Current prototype uses a SOD-123 footprint. |
| Plate opening | 1 | Part of the switch plate rather than a separately purchased item. |

So, ignoring the PCB and case, the basic rule is:

**N keys = N switches + N keycaps + N hot-swap sockets + N diodes.**

A small spare margin is cheap and useful. Buying about 10–20% extra switches, sockets, and diodes avoids turning one damaged part into another order.

## Example keypad sizes

These are planning numbers, not claims that the present layouts have exactly these counts.

| Approximate keypad size | Needed switches | Needed keycaps | Needed sockets | Needed diodes | Sensible order quantity for switches/sockets/diodes |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 16 keys | 16 | 16 | 16 | 16 | 20 each |
| 17 keys | 17 | 17 | 17 | 17 | 20 each |
| 32 keys | 32 | 32 | 32 | 32 | 36–40 each |

If building one keypad of each example size, that is **65 key positions total**. A practical combined order would therefore be roughly:

- 75 MX switches;
- 75 matching hot-swap sockets;
- 75 SOD-123 matrix diodes; and
- at least 65 usable keycaps, plus spare blanks or replacements.

Keycaps are often cheaper to buy as sets rather than as exactly 65 individual caps, especially while legends are still changing.

## Per-key exceptions already relevant to these layouts

Not every control we have discussed is a normal 1u key.

- A 2u or wider key still normally uses one electrical switch, one socket, and one diode, but needs the correct wider keycap and usually a stabilizer.
- The proposed double-space and quad-space controls therefore should not be counted as multiple electrical switches merely because their keycaps are wide.
- A 3-way bracket rocker is a different switch mechanism and needs its own mechanical/electrical treatment rather than being substituted blindly for three MX keys.
- Any deliberately large Execute, Cancel, Escape, or similar key may need a non-1u keycap and stabilizer even though it is still one matrix position.

## Per keypad, not per key

Each physical keypad also needs a small set of shared parts:

| Part | Typical quantity | Notes |
| --- | ---: | --- |
| PCB | 1 | Carries the switch/socket matrix and controller electronics. |
| Switch plate | 1 | Strongly recommended for a clean mechanical assembly. |
| Bottom plate or case | 1 | Can begin as a simple flat backplate before a molded/printed enclosure exists. |
| Standoffs/spacers | 4–6 | Quantity depends on board size and flex. |
| Matching screws | 8–12 | Typically two per standoff when plate/PCB/backplate are stacked. |
| Rubber feet | 4 | Optional but useful on a desktop keypad. |
| USB cable | 1 | Match the controller/connector; the present RP2040 prototype is USB-C. |
| RP2040 controller electronics | 1 set | Either an RP2040 development board in a hand-built prototype or the integrated RP2040/USB circuitry on a fabricated PCB. |

## What to buy first

For experimentation, the low-risk bulk purchases are the generic parts that can be reused across every keypad:

1. MX-compatible switches;
2. generic 1u keycaps/blanks;
3. matrix diodes; and
4. only after the footprint is checked, the matching hot-swap sockets.

Plates, custom legends, stabilizers, cases, and PCB quantities should follow the individual keypad layouts because those depend on key widths and final geometry.
