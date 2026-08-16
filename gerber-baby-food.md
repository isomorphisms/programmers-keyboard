# Gerber baby food

A **Gerber file is a 2D manufacturing drawing for one PCB layer**. It is not the schematic, not the editable PCB project, and not usually the literal machine program that drives the factory equipment. Think of it as: **for this layer, put material or artwork here and not there**. One Gerber might describe top copper, another bottom copper, another solder mask, another silkscreen, and so on. A complete fabrication package also normally includes the board outline and separate drill data for holes.

For a simple two-layer keyboard PCB, the handoff commonly looks roughly like:

```text
PCB design
  |
  +-- top copper Gerber
  +-- bottom copper Gerber
  +-- top solder-mask Gerber
  +-- bottom solder-mask Gerber
  +-- top silkscreen Gerber
  +-- bottom silkscreen Gerber, if used
  +-- board-outline / profile data
  +-- drill file(s)
```

Modern Gerber is an image-description format: lines, flashes/pads, filled regions, coordinates, apertures, polarity, and layer metadata describe the intended 2D image. The layers are all emitted at the same scale and coordinate origin so that the factory can stack them correctly.

## Does the factory actually use the Gerbers?

**Yes, but not by feeding them untouched into an etching machine.** The board house imports the fabrication package into CAM software. Its automated system and/or CAM staff check that the layers make sense, identify their functions, run manufacturability checks, check drills and board outlines, and usually arrange one or more copies of the board into a manufacturing panel. From that prepared CAM job the factory generates the particular imaging, drilling, routing, inspection, test, and other machine programs needed by its equipment.

So the chain is approximately:

```text
our KiCad/EDA PCB
      |
      v
Gerbers + drill data
      |
      v
manufacturer CAM / DFM / panelization
      |
      v
factory-specific machine jobs
      |
      v
physical PCB
```

The important boundary is that **we specify what board we want; the fabricator decides how its factory will manufacture it**.

## What Gerber does not contain

A Gerber layer does not explain the electrical intention of the circuit. A copper trace in the file is simply copper geometry. The fabricator does not need to know that one trace means `LAMBDA`, that another scans a keyboard row, or that a capacitor is decoupling the microcontroller. Those meanings live in our schematic, PCB design, firmware, and documentation.

That is why it is useful to keep the editable design files as the source of truth and treat Gerber/drill files as **fabrication output**.

## Primary reference

Ucamco maintains the Gerber standard and describes one Gerber file as one 2D PCB layer image:

https://www.ucamco.com/en/gerber/downloads
