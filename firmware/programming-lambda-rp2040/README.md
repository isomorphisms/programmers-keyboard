# RP2040 programming-key lambda proof

This branch proves the missing firmware path for the Programming keypad's semantic `λ` key without pretending that the final Programming PCB has already been routed.

The executable reuses the known RP2040 3×4 matrix wiring from the Movement prototype:

- rows: GPIO0, GPIO1, GPIO2
- columns: GPIO3, GPIO4, GPIO5, GPIO6

For this proof only, matrix position row 0 / column 0 is bound to the semantic `ACTION_LAMBDA`. The other eleven positions do nothing. When the final Programming PCB exists, only the physical key-to-action table should need to change.

## What pressing the proof key does

USB HID keyboard reports do not contain arbitrary Unicode code points. The firmware therefore separates the semantic Lambda action from its current host encoding.

The first encoding target is Linux text input. `ACTION_LAMBDA` emits:

```text
Ctrl+Shift+U
0
3
b
b
Enter
```

On a Linux text field that supports the standard Unicode input sequence, that produces U+03BB, `λ`.

This is intentionally not described as universal. Windows, macOS, Android, and custom keyboard layouts may require a different encoding. Those should be alternate encoders for `ACTION_LAMBDA`, not different matrix definitions.

## Build

Use Raspberry Pi Pico SDK 2.2.0 and an Arm embedded GCC toolchain:

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S firmware/programming-lambda-rp2040 -B build/programming-lambda-rp2040
cmake --build build/programming-lambda-rp2040 --parallel
```

The flashable image is:

```text
build/programming-lambda-rp2040/programming_lambda_keypad.uf2
```

The branch workflow builds the same image and uploads it as `programming-lambda-uf2`.

## Flash/test on the current RP2040 prototype

1. Put the RP2040 board into BOOTSEL mode with `SW_BOOT` / `SW_RUN`.
2. Copy `programming_lambda_keypad.uf2` to the `RPI-RP2` drive.
3. Focus a Linux text field.
4. Press the switch at matrix row 0 / column 0.
5. The field should receive `λ`.

This is a firmware proof. It does not establish that the final Programming keypad PCB, key placement, or legends are finished.
