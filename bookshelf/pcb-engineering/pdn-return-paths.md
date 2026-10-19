# Decoupling and current return: what the placement debate can test

Original mathematical/engineering synthesis, 2026-10-08. Primary reading trail and limits are in [sources](sources.md). No circuit or field simulation was executed in STAR. The numerical example below is elementary model arithmetic, not a measurement of this keyboard.

## Start with a concrete current change

Suppose a device increases its supply current faster than a remote source can respond. A local capacitor can provide some of the changing current, but the complete outgoing-and-return path has resistance and inductance. Short Euclidean distance from capacitor body to chip is therefore an incomplete predictor. Pad escape, vias, return continuity, power/ground connection geometry and the load's rise time matter.

Peterson's article distinguishes close plane pairs from trace-connected construction. Hubing and colleagues' paper concerns multilayer boards; its verified publication date is May 1995, despite the article's 2002 reference. Those sources motivate alternative hypotheses. They do not establish that a capacitor may be placed arbitrarily on this two-layer prototype, whose stackup and high-frequency current paths remain unresolved.

A counterexample to “closest is always best” is a physically close part connected through a long narrow detour and poor return, compared with a farther part having a lower-inductance connection. A counterexample to “placement never matters” is a fast current change through a long trace loop. D1 must specify and vary these connections explicitly rather than merely relabel one model “near” and another “far.”

## The smallest falsifiable model

Use capacitance C, equivalent series resistance R, total series inductance L and angular frequency ω. For one idealized series RLC branch:

```math
Z(ω) = R + j[ωL − 1/(ωC)]
```

Here j identifies the quadrature part: inductive and capacitive voltage responses oppose there. L includes the connection contribution being modeled, not only the component's quoted package ESL. This branch is not yet the entire regulator/board network.

The reactances cancel when ωL = 1/(ωC), giving:

```math
f₀ = 1 / [2π√(LC)]
```

For a declared example C = 100 nF, changing L from 1 nH to 10 nH moves this ideal cancellation frequency from approximately 15.915 MHz to 5.033 MHz. These are illustrative inputs; neither inductance has been extracted from the Movement PCB. At cancellation, nonzero R prevents the ideal branch impedance from vanishing. A more complete parallel network can have additional peaks and requires its own analysis.

D1 should compare a pinned ngspice deck to these low/high-frequency limits and resonance, then add a declared source/regulator impedance and finite-rise load step. Record the port at which impedance and droop are observed. Change ESR, ESL, connection inductance, capacitance and rise time independently, preserve physical parameter ranges, and show timestep/frequency-grid sensitivity. A zeroed current stimulus or empty curve must fail the harness.

Inductive voltage scales with current-change rate through that inductance. This does not mean a passive capacitor independently emits arbitrary radio signals: radiation depends on driven currents, loop geometry, common-mode conversion and coupling to structures/cables. A lumped impedance plot alone is not an emissions prediction.

## Return-path and USB experiment boundary

A signal's return follows the full electromagnetic connection under the applicable frequency/geometry conditions; the label GND does not specify that path. First compare intact and interrupted reference geometries on a small coupon. Separate differential excitation from common-mode excitation and measure model sensitivity to reference spacing and edge rate.

The RP2040 guide's USB example assumes specific geometry and stackup. Its dimensions are not transferable manufacturing rules for an unspecified board. D2 must label dielectric, thickness, copper and reference assumptions and return parameter bounds when actual data is absent. D6 may later use a converged field model on that coupon, not declare whole-board USB or EMI compliance.

## Planned discrimination, not consensus

| Hypothesis | What could falsify it | Missing keyboard evidence |
|---|---|---|
| Lower connection inductance improves the declared high-frequency supply response | No improvement or worse peaks in the full stated network under a controlled change | Actual loop/plane geometry and load spectrum |
| Close plane coupling reduces sensitivity to lateral placement within a specified range | A sweep showing substantial impedance/transient change under the same conditions | Real stackup and applicable paper/model regime |
| An interrupted reference changes the coupon's return/common-mode behavior | No change above numerical error for the stated excitation and geometry | Calibrated reference and mesh/boundary convergence |
| One capacitor value is universally optimal | A different declared load/connection producing a worse result | Actual target impedance, load and regulator behavior |

C2's later video dossier should record observed timestamps, rise time, probe configuration and the author's stated conditions. The FEDEVEL chapter markers are publisher navigation leads; footage was not watched in STAR. Preserve disagreement until the compared experiments use comparable models and measurements.
