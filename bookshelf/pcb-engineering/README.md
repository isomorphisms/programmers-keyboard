# Keyboard engineering bookshelf

Original STAR PKB-R1 synthesis, 2026-10-08. Repository: isomorphisms/programmers-keyboard. Audited main: [e37ceffaf2e4e23ff60eda25e5eecbd211942e5e](https://github.com/isomorphisms/programmers-keyboard/tree/e37ceffaf2e4e23ff60eda25e5eecbd211942e5e). This is the first evidence packet for [assignment #19](https://github.com/isomorphisms/programmers-keyboard/issues/19), reconciled with [overlapping tracker #20](https://github.com/isomorphisms/programmers-keyboard/issues/20).

The existing Movement board is a routing prototype. The audit reproduced three validator false passes, confirmed duplicate assembly reference D1, and found contradictory MCU/matrix net identities needing independent continuity analysis. No manufacturing, physical USB or EMI acceptance follows from its generated files.

- [Current-state audit and executed probes](baseline-2026-10-08.md)
- [24 SUN jobs, dependencies and execution waves](queue.md)
- [Dated primary sources and reading depth](sources.md)
- [First-order fabrication and assembly failure guide](first-order.md)
- [Decoupling, current return and the placement debate](pdn-return-paths.md)
- [Model feasibility and compute limits](model-feasibility.md)
- [Machine-readable probe receipts](audit-receipts.tsv)
- [Exact archive-member identities](archive-members.tsv)

Read source facts, observed test results, engineering inference and future test requirements as different kinds of evidence. PASS applies only to the named check and immutable input. NOT_RUN, UNSUPPORTED, INCONCLUSIVE and missing inputs must never become PASS.

The linked SUN issues are complete execution prompts. They own future maintained implementations and reproducible experiments; this STAR packet does not claim those jobs are done. Original notes summarize limited sections of linked materials, with no bulk copies of papers or transcripts.

Owner decisions are preserved in [factory/stackup #45](https://github.com/isomorphisms/programmers-keyboard/issues/45), [part/mating identities #46](https://github.com/isomorphisms/programmers-keyboard/issues/46), and [USB/protection/physical acceptance #47](https://github.com/isomorphisms/programmers-keyboard/issues/47). Evidence work can proceed before the owner answers. Visual design remains owner-held.
