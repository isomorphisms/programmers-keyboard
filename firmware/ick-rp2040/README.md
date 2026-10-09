# RP2040 division-source producer

All three firmware projects compile maintained C through ICK
`c61e448251744a2f40ad743ebef1a027bdcd2f9d`, materialized against GCC
`6294f1d9e7536e5ffcde09d1528c918d63abfef5`. This is an actual
`arm-none-eabi` compiler, configured for the bare-metal target. The Android
ARM compiler is not used for firmware.

`compiler.cmake` selects ICK for C and its resource headers before the
declared embedded newlib headers. The Pico SDK still selects its existing
GNU Arm C++ compiler, assembler and C++ link driver; those supply the
Cortex-M0+ runtime, startup, multilib, linker scripts and UF2 tools. SDK and
TinyUSB C compile through ICK as well. Existing optimization, debug flags,
USB descriptors, key maps and target definitions remain intact.

The four migrated ratios include an ordinary `sizeof` macro body, which
ICK handles after preprocessing. The compiler qualifier emits an actual
Thumb Cortex-M0+ object, checks its ELF architecture, and verifies that a
dynamic glyph division calls the declared `__aeabi_idiv` runtime boundary.
Each workflow builds and uploads its original UF2 targets. The SDK is
pinned to the existing sample project's 2.2.0 revision
`a1438dff1d38bd9c65dbd693f0e5db4b9ae91779` for all three projects.

For local builds, first build the compiler with this directory's Makefile,
passing absolute `ICK_SOURCE`, `ICK_BUILD`, and `ICK_STAGE` paths. The
source checkout must include the pinned GCC submodule. The `qualify` target
runs on every cache restore. Then set `ICK_RP2040` to the installed
`ICK_STAGE/bin/arm-none-eabi-gcc` and use each firmware directory's existing
CMake instructions. `PICO_SDK_PATH` still identifies the Pico SDK;
`ICK_NEWLIB_INCLUDE` defaults to Ubuntu's `/usr/include/newlib` and can be
set explicitly for a relocated embedded toolchain. Use clean build
directories when switching compilers.

Compiler bootstrap uses host GCC/G++; firmware C uses ICK. Physical keypad
flashing, key reports and USB operation require hardware acceptance and
are not inferred from successful UF2 production.
