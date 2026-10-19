# SUN queue and execution order

STAR PKB-R1, 2026-10-08. Repository: isomorphisms/programmers-keyboard. Baseline e37ceffaf2e4e23ff60eda25e5eecbd211942e5e. Each linked issue is a full execution prompt with paths, prerequisites, build lane, positive/negative tests, outputs, failures and exclusions. All 24 bodies were read back after creation; the dependency graph is acyclic.

The candidate labels in parent #19 were refined to separate source connectivity from geometric copper, and build qualification from CI integration. The labels below and in the created issues are authoritative for this queue; do not use parent candidate numbering to infer dependencies.

## Work lanes

| Job | Executable issue | Hard prerequisites | Work type |
|---|---|---|---|
| A1 | [#21 — Reject incomplete fabrication packages and prove validator failures](https://github.com/isomorphisms/programmers-keyboard/issues/21) | None | implementation |
| A2 | [#22 — Qualify independent Gerber and Excellon readers on real exports](https://github.com/isomorphisms/programmers-keyboard/issues/22) | None | implementation and library qualification |
| A3 | [#23 — Resolve MCU–matrix net scope and enforce the wiring contract](https://github.com/isomorphisms/programmers-keyboard/issues/23) | None | implementation and electrical diagnosis |
| A4 | [#34 — Compare geometric copper connectivity with intended nets](https://github.com/isomorphisms/programmers-keyboard/issues/34) | A2, A3 | implementation |
| A5 | [#35 — Implement service-specific fabrication rule profiles](https://github.com/isomorphisms/programmers-keyboard/issues/35) | A2 | implementation and manufacturer research |
| A6 | [#36 — Audit footprints, registration, mounting and mating geometry](https://github.com/isomorphisms/programmers-keyboard/issues/36) | A2 | implementation and mechanical review |
| A7 | [#24 — Repair BOM and placement identity acceptance, including duplicate D1](https://github.com/isomorphisms/programmers-keyboard/issues/24) | None | implementation |
| A8 | [#25 — Audit RP2040 support circuitry against primary electrical sources](https://github.com/isomorphisms/programmers-keyboard/issues/25) | None | engineering decision study |
| A9 | [#44 — Build a fabrication-candidate manifest with honest acceptance states](https://github.com/isomorphisms/programmers-keyboard/issues/44) | A1, A4, A5, A6, A7, A8, B4 | implementation |
| B3 | [#26 — Qualify actual firmware and host-test build targets under ICK/NDK policy](https://github.com/isomorphisms/programmers-keyboard/issues/26) | None | toolchain qualification and deferred-decision study |
| B1 | [#37 — Execute maintained matrix/debounce logic in host tests](https://github.com/isomorphisms/programmers-keyboard/issues/37) | B3 | implementation |
| B2 | [#40 — Verify HID macro lifecycle and finite-queue overload recovery](https://github.com/isomorphisms/programmers-keyboard/issues/40) | B1 | implementation |
| B4 | [#42 — Harden existing CI, artifact identity and bounded compute campaigns](https://github.com/isomorphisms/programmers-keyboard/issues/42) | A1, B3, B1, B2, B5 | implementation |
| B5 | [#41 — Check HID descriptors, semantic actions and host-specific Unicode contracts](https://github.com/isomorphisms/programmers-keyboard/issues/41) | B1, B2 | implementation and protocol acceptance |
| B6 | [#43 — Establish a portable semantic-event seam without changing controllers](https://github.com/isomorphisms/programmers-keyboard/issues/43) | B1, B2, B5 | bounded decision study and executable comparison |
| C1 | [#27 — Write a first fabrication, assembly and bring-up failure handbook](https://github.com/isomorphisms/programmers-keyboard/issues/27) | None | research with concrete checklists |
| C2 | [#28 — Build a critical Altium Academy and practitioner source dossier](https://github.com/isomorphisms/programmers-keyboard/issues/28) | None | research with reproduced examples |
| C3 | [#29 — Curate primary specifications, component datasheets and engineering glossary](https://github.com/isomorphisms/programmers-keyboard/issues/29) | None | research and source qualification |
| D1 | [#30 — Test capacitor connection and PDN resonance hypotheses](https://github.com/isomorphisms/programmers-keyboard/issues/30) | None | bounded numerical research experiment |
| D2 | [#31 — Bound USB trace and return-path models with unknown stackup](https://github.com/isomorphisms/programmers-keyboard/issues/31) | None | bounded numerical research experiment |
| D3 | [#32 — Check typed electrical identities, units and geometric constraints in Idriç](https://github.com/isomorphisms/programmers-keyboard/issues/32) | None | bounded language experiment |
| D4 | [#38 — Compare copper graph topology with ordinary connectivity checks](https://github.com/isomorphisms/programmers-keyboard/issues/38) | A3 | bounded mathematical research experiment |
| D5 | [#33 — Evaluate certified tolerance bounds and algebraic circuit constraints](https://github.com/isomorphisms/programmers-keyboard/issues/33) | None | bounded mathematical research experiment |
| D6 | [#39 — Qualify openEMS on a reference coupon before any board field claim](https://github.com/isomorphisms/programmers-keyboard/issues/39) | D2 | optional bounded field-solver research |

A1 strengthens the existing validator with falsifiable package invariants; it does not duplicate the generator. A3 owns the firmware/PCB wiring contract; a separate redundant wiring issue was rejected. A4 consumes A2 parsing and A3 intended nets. A7 fixes identity acceptance before anyone trusts a BOM join. B3 establishes lawful actual build targets before host tests claim execution. C jobs produce substantive source/measurement dossiers; D jobs have executable experiments and may conclude a method is unsuitable.

## Parallel execution waves

A wave lists jobs whose hard prerequisites have completed with usable receipts. It is not an instruction to dispatch all jobs at once or allocate maximum compute.

| Wave | Newly eligible jobs |
|---|---|
| 1 | [A1 #21](https://github.com/isomorphisms/programmers-keyboard/issues/21), [A2 #22](https://github.com/isomorphisms/programmers-keyboard/issues/22), [A3 #23](https://github.com/isomorphisms/programmers-keyboard/issues/23), [A7 #24](https://github.com/isomorphisms/programmers-keyboard/issues/24), [A8 #25](https://github.com/isomorphisms/programmers-keyboard/issues/25), [B3 #26](https://github.com/isomorphisms/programmers-keyboard/issues/26), [C1 #27](https://github.com/isomorphisms/programmers-keyboard/issues/27), [C2 #28](https://github.com/isomorphisms/programmers-keyboard/issues/28), [C3 #29](https://github.com/isomorphisms/programmers-keyboard/issues/29), [D1 #30](https://github.com/isomorphisms/programmers-keyboard/issues/30), [D2 #31](https://github.com/isomorphisms/programmers-keyboard/issues/31), [D3 #32](https://github.com/isomorphisms/programmers-keyboard/issues/32), [D5 #33](https://github.com/isomorphisms/programmers-keyboard/issues/33) |
| 2 | [A4 #34](https://github.com/isomorphisms/programmers-keyboard/issues/34), [A5 #35](https://github.com/isomorphisms/programmers-keyboard/issues/35), [A6 #36](https://github.com/isomorphisms/programmers-keyboard/issues/36), [B1 #37](https://github.com/isomorphisms/programmers-keyboard/issues/37), [D4 #38](https://github.com/isomorphisms/programmers-keyboard/issues/38), [D6 #39](https://github.com/isomorphisms/programmers-keyboard/issues/39) |
| 3 | [B2 #40](https://github.com/isomorphisms/programmers-keyboard/issues/40) |
| 4 | [B5 #41](https://github.com/isomorphisms/programmers-keyboard/issues/41) |
| 5 | [B4 #42](https://github.com/isomorphisms/programmers-keyboard/issues/42), [B6 #43](https://github.com/isomorphisms/programmers-keyboard/issues/43) |
| 6 | [A9 #44](https://github.com/isomorphisms/programmers-keyboard/issues/44) |

**Recommended first allocation:** A1, A2, A3, A7 and B3 address current acceptance defects and unblock the greatest amount of engineering work. Run C1/C2 and bounded D1 alongside as resources allow; A8 and C3 can independently research electrical identities. A5/A6 need A2's geometry interface, not a mere closed issue. C work does not delay A/B.

Dependency core: A2 + A3 → A4; A2 → A5/A6; B3 → B1 → B2 → B5; A1 + B3 + B1 + B2 + B5 → B4; A1 + A4 + A5 + A6 + A7 + A8 + B4 → A9. D4 follows A3; D6 follows D2; B6 follows B1/B2/B5.

Workers must consume exact reviewed prerequisite SHAs and receipts. They must refresh main/open work and avoid concurrent edits to another job's implementation. Separate checker directories permit parallel development. B4 integrates shared workflows after entrypoints stabilize; C2/D1 serialize overlapping pdn-return-paths.md edits, and all jobs serialize shared index changes. A3/A8 may propose hardware corrections separately but must not merge them.

Sun owns judgment and each bounded output. Return complete Earth/Moon handoff prompts only when a real next task is justified; do not automatically spawn workers or pad the queue. Issue closure without the required executable evidence does not satisfy a dependency.

## Deferred decisions

| Decision | Evidence first | Owner choice later |
|---|---|---|
| [O1 #45](https://github.com/isomorphisms/programmers-keyboard/issues/45) | A5/A6/A7/C1 | Factory/service, stackup, thickness/copper, finish and bare-board/assembly scope |
| [O2 #46](https://github.com/isomorphisms/programmers-keyboard/issues/46) | A6/A7 | Actual switch/socket/diode identities and mating constraints |
| [O3 #47](https://github.com/isomorphisms/programmers-keyboard/issues/47) | A8/B5/D2 | Power/host/handling scenarios, protection and physical acceptance requirements |

No immediate owner answer is needed to begin the evidence work. Key aesthetics, layout, typography, enclosure and feel remain owner-held. RP2040 Movement, hypothetical 4×4 samples, CH552 λ, possible PIC, Android IME and a Unicode picker remain distinct.

## Completion evidence

STAR completed source/history research, disposable validator probes, archive byte-identity checks, this bookshelf and the executable queue. SUN implementations, solver results, physical tests and manufacturing acceptance remain outstanding. A9 emits a candidate manifest with separate statuses, never an automatic “ready to order.” See [baseline](baseline-2026-10-08.md) and [model stopping rules](model-feasibility.md).
