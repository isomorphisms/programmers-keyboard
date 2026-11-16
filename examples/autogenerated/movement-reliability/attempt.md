# Movement reliability

## Purpose and type sketch

Execute the maintained scanner, debouncer, queue and USB macro state machine.
Physical switch samples and USB submission results are explicit input traces.
An admitted press maps to one ordered macro; every accepted press report must
have an accepted release before advancing. Five consecutive equal samples
change a debounced state. Holding does not enqueue repeats. A sixteen-element
ring has fifteen usable entries. Drop-new is the existing policy, now observable
through saturating drop count and high-water diagnostics; no replay policy is
invented. Disconnect/reconnect policy remains outside this change.

`can_admit : Number → Bool` describes queue admission below capacity.
`usable_capacity : Number` is fifteen. Host C tests include the maintained C
translation unit rather than implementing its state machine again. Hardware
and USB APIs are deterministic test boundaries, not physical behavior evidence.
The Idriç example checks the pure capacity boundary; C execution uses the
exact recovered ICK frontend. No generated-C, RefC or generic C compiler route.

## Result

Idriç `2bce7677f513d711875e978bd6f5ad28b9e54d58` compiled QueueCapacity.idric
through Chez; the executable printed `True` and `False` for occupancy fourteen
and fifteen. No generated-C or RefC path was used.

The actual host C harness compiled and executed through recovered ICK
`515c0f29fe6e2e96e10495fbaf25da93532e7722`, x86_64-linux-gnu. Its driver,
frontend and link-driver SHA-256 values match the published closure from
artifact 11468256648, run 37589849631. Explicit GCC 13 headers/startup objects
are platform support only; they do not compile consumer C. Host compilation
uses `-std=c11 -O2 -Wall -Wextra -Werror -DMOVEMENT_HOST_TEST` and the real
maintained source, including the new literal assignment arrows.

Exact restored closure digests:

- driver: `86cdbaf8d405d7d05883f20ddcc0ac21a55a48bae20cc9f11aca78f1710867d3`
- frontend: `5aff3188c58cae7cfffb20bdade56ec3e9ab89667ff6feafcfeed066350f9400`
- link driver: `62e09cd8f93fba0224f0be1d2cd55da01c99d5d83029263d4714151cbe2ea4f9`

The maintained CI separately selects ICK
`fbe86e23d55cfec2000c08e61deea2a407fd7175` using the pinned ai-ci host action.
The local result does not inherit that compiler's qualification or establish
an RP2040 UF2, USB-host, reconnect or physical-device result.

Local execution passed 47 maintained-code cases and 30,000 seeded queue
operations. All seven actual-source mutants were rejected at their declared
semantic diagnostics. The Grease runner was executed through the verified
portable entrypoint; its inherited host engine requires leak detection disabled
under this container's process tracing. This does not disable checks in the
firmware binary. Initial runner attempts exposed missing explicit environment
lookup and a mutant include-path error; both setup faults were repaired before
counting any mutant result.

## Language work

The pure capacity contract needs no new feature. Calling file-static firmware
functions and controlling C platform APIs is a C integration boundary; the
host harness must execute the maintained implementation, not a translated
Idriç substitute. Compile/link/runtime, target UF2 and physical results remain
separate evidence categories.
