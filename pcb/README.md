# Movement keypad PCB prototype

This is the first deliberately small electrical segment of the programmers' keyboard. It is not the complete keyboard.

The board contains:

- one complete RP2040 support circuit;
- one USB-C connector for USB data and power;
- twelve Kailh-style hot-swap key positions;
- twelve matrix diodes;
- a 3-row by 4-column key matrix; and
- four non-plated mounting holes.

The key faces use semantic operations rather than Vim letters:

| Row | Keys |
| --- | --- |
| 1 | Movement, Type, Previous Line, Next Line |
| 2 | Start Line, End Line, Next Word, Next Space-Separated Word |
| 3 | 7 Times, 13 Times, Delete Character, Delete Word |

The controller maps GPIO0–GPIO2 to the rows and GPIO3–GPIO6 to the columns. The rectangular board is 132 mm by 70 mm. Placement is explicit; tscircuit's local autorouter handles the copper.

## Generate the fabrication package

From this directory:

```sh
npm ci
npm run typecheck
npx tsci export movement-prototype.tsx --format circuit-json \
  --output ../fabrication/movement-prototype.circuit.json
npx tsci export movement-prototype.tsx --format pcb-svg \
  --output ../fabrication/movement-prototype-pcb.svg
npx tsci export movement-prototype.tsx --format schematic-svg \
  --output ../fabrication/movement-prototype-schematic.svg
npx tsci export movement-prototype.tsx --format readable-netlist \
  --output ../fabrication/movement-prototype-readable.netlist
npx tsci export movement-prototype.tsx --format gerbers \
  --output ../fabrication/movement-prototype-gerbers.zip
```

Extract the ZIP before validating:

```sh
rm -rf ../fabrication/movement-prototype-gerbers
mkdir -p ../fabrication/movement-prototype-gerbers
unzip -q ../fabrication/movement-prototype-gerbers.zip \
  -d ../fabrication/movement-prototype-gerbers
node scripts/validate-output.mjs
```

The GitHub workflow repeats this process and checks that the committed Gerber layers, drills, previews, netlist, and Circuit JSON reproduce from the source. The validator rejects circuit error elements, missing keys or diodes, missing RP2040/USB-C parts, too few routed traces, missing Gerber layers, and missing or implausibly small drill files.

The currently checked-in run reports 63 components, 146 source traces, 125 routed PCB traces, 10 Gerber layers, and 2 drill files.

This is a routing prototype, not yet a production release. Switch choice, key spacing, board outline, ESD protection, firmware, component availability, and mechanical enclosure still need human review before ordering boards.
