# First PCB order: an evidence-based failure guide

Original STAR synthesis, 2026-10-08. Starting authorities and reading limits are in [sources](sources.md). This guide identifies failure mechanisms and review questions; it is not an order form, a hardware change or a manufacturing approval.

## Submission sequence

First fix identity and continuity contradictions. Then parse the actual exported layers independently, apply an explicitly selected manufacturing profile, reconcile components and placements, and assemble a candidate manifest. A factory upload preview comes after those steps. The preview is useful additional evidence, not an oracle for intended electrical connectivity.

| Failure mechanism | Present evidence | Required discriminating test / owner |
|---|---|---|
| Missing copper, outline or drills | Validator actually accepts missing F_Cu and NPTH drill | A1 enforces exact required identities; A2 parses every member and a broken counterpart |
| Malformed drill or wrong units | Invalid drill text currently passes size check | A2 rejects invalid syntax and compares metric/inch-equivalent fixtures, tools, hits, slots and bounds |
| Mask polarity misread | Export says negative file polarity | A2 independently interprets mask openings and checks layer registration; negative metadata is not itself a defect |
| Plated/NPTH interchange | Two separate files exist | A2/A6 compare each hole and slot to intended mechanical/electrical role, with a swapped-file negative |
| Tiny copper or annular ring | Nominal 0.100 mm trace / 0.200 mm drill / 0.050 mm ring occurs | A5 checks service-specific limits, plus tool/plating/registration tolerances; O1 retains factory choice |
| Disconnected row/column | MCU-local and board nets contradict readable output | A3 identifies scope/port mapping; A4 checks geometric copper and deliberately open/short fixtures |
| Wrong package or pin numbering | Generic socket/diode identities; 22 supplier-footprint warnings | A6/A7 overlay actual drawings and resolve pins, sidedness and exact ordering code |
| Assembly reference collision | LED and matrix diode are both D1 | A7 rejects ambiguous BOM/CPL joins and requires source-to-placement one-to-one identity |
| Bottom-side rotation/mirroring | Bottom sockets and diodes exist | A6/A7 test an asymmetric polarized fixture in the declared top-view coordinate convention; don't “correct” signs by eye |
| Copper-to-edge, mask dams, silk on pads | Not independently measured | A5 measures from composited geometry and flags unknowns; manual rendering supplements tests |
| USB protection or rail faults | Support package instantiated; no bench evidence | A8 checks exact nets and datasheets; O3 defines intended handling/power conditions |
| Connector, fastener or switch interference | Source dimensions only | A6 tolerances and full mating envelopes; O2 chooses actual parts after evidence |

The current PCB size and key pitch are audit facts, not newly selected design choices. A drawing that looks plausible does not establish contact alignment, insertion clearance, retention or key travel. Keep manufacturer nominal dimensions, tolerances and measured samples distinct.

## Bare board versus assembly

Bare-board fabrication supplies the patterned/drilled board. Assembly adds a parts identity and placement contract: exact MPN, value, footprint, side, rotation, pin 1/polarity, quantity, population status and sourcing. For this project, a duplicated D1 makes a flat reference join unsafe before availability or price becomes relevant. A blank matrix-diode MPN must remain unresolved.

A9's manifest should list required fabrication layers separately from optional fabrication drawings. Include outline/cutouts and plated/NPTH drills explicitly. Archive files in a deterministic manifest and compare both directions: every expected member exists and every submitted member is intentional. Hashes establish identity, not correctness.

Order settings such as board thickness, copper weight, finish, via treatment and assembly scope change the interpretation of rule limits and models. Record the exact service/revision with the candidate. Shipping, lead time, quantity, tooling/assembly fees and tax require a fresh quote if an order is later authorized; no price or deadline is inferred in STAR.

## Review and later physical work

Manual review should include independent layer overlays, registration/origin, edge and cutout continuity, drill purpose, mask/paste openings, polarized parts and connector mechanical envelopes. A good screenshot is a navigation aid; tests still need coordinates and tolerances.

Later, separately authorized bring-up should begin with unpowered short/continuity checks, polarity inspection and a current-limited power plan derived from A8, then rail/reset/clock checks before USB and key testing. B5 must identify the exact host, cable, firmware image and stimulus for enumeration, press/release, reconnect and Unicode observations. Avoid applying voltage or flashing hardware as an incidental “verification” step.

Preserve the first failure, original input, runtime/version and diagnostic. A partial candidate should say which checks passed, failed or remain unrun. [O1](https://github.com/isomorphisms/programmers-keyboard/issues/45), [O2](https://github.com/isomorphisms/programmers-keyboard/issues/46) and [O3](https://github.com/isomorphisms/programmers-keyboard/issues/47) gather decisions after the evidence exists.
