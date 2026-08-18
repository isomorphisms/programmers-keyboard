#!/usr/bin/env sh
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/../.." && pwd)
BUILD_DIR="$ROOT/build/programming-lambda-ch552"

: "${CH55XDUINO_SOURCE:?Set CH55XDUINO_SOURCE to a DeqingSun/ch55xduino checkout}"
ARDUINO_CLI=${ARDUINO_CLI:-arduino-cli}

HID_EXAMPLE="$CH55XDUINO_SOURCE/ch55xduino/ch55x/libraries/Generic_Examples/examples/05.USB/HidKeyboard/src"
FQBN='CH55xDuino:mcs51:ch552:clock=16internal,usb_settings=user148,upload_method=usb,bootloader_pin=p36'

if [ ! -d "$HID_EXAMPLE/userUsbHidKeyboard" ]; then
    echo "CH55xduino HID keyboard support not found at: $HID_EXAMPLE" >&2
    exit 1
fi

rm -rf "$HERE/src"
cp -R "$HID_EXAMPLE" "$HERE/src"
trap 'rm -rf "$HERE/src"' EXIT HUP INT TERM

mkdir -p "$BUILD_DIR"
"$ARDUINO_CLI" compile \
    --fqbn "$FQBN" \
    --output-dir "$BUILD_DIR" \
    "$HERE"

printf '%s\n' "CH552 Lambda firmware built in $BUILD_DIR"
