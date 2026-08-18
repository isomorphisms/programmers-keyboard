# CH552 Lambda-key firmware proof

This is the CH552 version of the Programming keypad's semantic `λ` firmware proof.

The goal is not to make the CH552 pretend that USB HID has a lambda key. USB HID keyboard reports carry keyboard usages, so the firmware is split into three layers:

1. scan and debounce the physical 3x4 switch matrix;
2. translate a physical position into the semantic action `ACTION_LAMBDA`;
3. encode that action for the current host.

For the first host encoder, Linux Unicode input is used:

```text
Ctrl+Shift+U
0
3
b
b
Enter
```

In Linux text fields that implement the standard Unicode-entry sequence, that enters U+03BB, `λ`.

Windows, macOS, Android, and custom input methods may need different encoders. They should be added as encoders for `ACTION_LAMBDA`; they should not change the matrix definition.

## Hardware contract

See `HARDWARE.md`.

Current matrix pins:

- rows: P1.4, P1.6, P1.7
- columns: P3.0, P3.1, P3.2, P3.3
- native USB: P3.6 D+, P3.7 D-

For the proof, row 0 / column 0 is `ACTION_LAMBDA`. The other eleven positions are intentionally unbound until the Programming keypad's final physical placement is settled.

## Toolchain

This firmware targets CH552 through CH55xduino and SDCC. The CI build pins CH55xduino 0.0.26 and uses the CH55xduino public-domain HID-keyboard example's `src` directory as the USB HID implementation.

The firmware itself is `programming_lambda_ch552.ino`; the USB stack remains an upstream dependency rather than being copied into this repository.

## Build

Install Arduino CLI and the CH55xduino core, then check out CH55xduino separately. With `CH55XDUINO_SOURCE` pointing at that checkout:

```sh
export CH55XDUINO_SOURCE=/path/to/ch55xduino
./firmware/programming_lambda_ch552/build.sh
```

The board configuration used by the build is:

```text
CH55xDuino:mcs51:ch552:clock=16internal,usb_settings=user148,upload_method=usb,bootloader_pin=p36
```

The resulting flash image is a `.hex` file under:

```text
build/programming-lambda-ch552/
```

## Flash/test

Flash the generated CH552 `.hex` with the CH55xduino USB uploader or WCH-compatible ISP tooling.

Then:

1. connect the CH552 keypad over USB;
2. focus a Linux text field that supports Unicode entry;
3. press matrix position ROW0/COL0;
4. verify that one `λ` appears;
5. hold the key and verify that the debounce/action layer does not repeatedly emit the macro without a new press transition.

This branch now has two independent controller proofs for the same semantic key: RP2040 and CH552. The CH552 version is the smaller controller path intended for the Lambda/Programming keypad.
