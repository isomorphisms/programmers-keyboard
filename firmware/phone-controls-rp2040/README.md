# RP2040 phone call controls

This is an experiment for physical phone controls that travel through the normal
USB HID -> Linux input -> Android input path.  It deliberately does **not** use
F-keys as stand-ins for phone actions.

## The useful stock-Android path

Button 0 emits USB HID Consumer usage `0x00cd` (`Play/Pause`).  The generic
Linux HID input driver maps that usage to `KEY_PLAYPAUSE`; Android's generic key
layout maps that Linux key to `MEDIA_PLAY_PAUSE` / `KEYCODE_MEDIA_PLAY_PAUSE`.
Android documents the same usage for headset call control:

- incoming call + short press -> answer
- incoming call + long press -> reject
- ongoing call + short press -> disconnect
- ongoing call + long press -> mute or unmute microphone

The firmware keeps `0x00cd` asserted for exactly as long as the physical HOOK
button remains down, rather than synthesizing a fixed-duration tap.  That leaves
short-versus-long interpretation to Android.

This makes one physical **HOOK** button useful without an Android companion app
or an F-key remapping layer.  Device/dialer behavior still has to be tested on
the actual phone: Android's telephony compatibility document strongly
recommends the call behavior, while its USB-headset specification gives the
same `0x00cd` mapping explicitly.

## A separate CALL probe

Button 1 emits USB HID Consumer usage `0x008c` (`Media Select Telephone`).  The
generic Linux HID input driver maps it to `KEY_PHONE` and AOSP's `Generic.kl`
maps Linux key 169 to Android `CALL` / `KEYCODE_CALL`.

This is intentionally called a **probe**.  `CALL` is a real Android key, but the
action it produces depends on telephony and dialer state.  Testing it on the
actual phone tells us whether it is useful as a separate ANSWER/CALL control.

## Why there is no fake END CALL button

Android defines `KEYCODE_ENDCALL`, but current AOSP `Generic.kl` does not map a
generic external Linux input key to `ENDCALL`.  Newer Linux headers have
`KEY_HANGUP_PHONE`, but AOSP's generic external-keyboard map does not give that
key `ENDCALL` semantics.  The generic Linux HID telephony-page handling also
does not currently turn the HID telephony Hook Switch / Drop usages into a
portable answer-only and hang-up-only pair.

So two firmware buttons labelled ANSWER and HANG UP cannot honestly be made
independent merely by assigning two arbitrary HID/F-key codes.  A dedicated
hang-up-only button needs one of these additional boundaries:

- a phone-specific Android key-layout mapping,
- an Android-side service with permission to act on call state,
- a custom/system Android build,
- or a two-way accessory protocol in which the phone tells the keyboard the
  current call state.

Until one of those exists, the state-dependent HOOK button is the clean
firmware-only control.

## Wiring

Both buttons are active-low and use RP2040 internal pull-ups:

| GPIO | label | USB Consumer usage | intended experiment |
| ---: | --- | ---: | --- |
| 2 | HOOK | `0x00cd` | answer/reject ringing; hang up/mute active call |
| 3 | CALL probe | `0x008c` | observe Android `CALL` behavior |

Each switch simply connects its GPIO pin to GND when pressed.  If both buttons
are held at once, HOOK has priority.

## Build

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S firmware/phone-controls-rp2040 -B build/phone-controls-rp2040
cmake --build build/phone-controls-rp2040 --parallel
```

The flashable output is `build/phone-controls-rp2040/phone_controls.uf2`.

## What to verify on hardware

The useful acceptance cases are behavioral, not merely "Android saw a USB
keyboard":

1. idle phone + HOOK: must not unexpectedly place a call;
2. incoming call + HOOK short press: answers;
3. incoming call + HOOK long press: rejects;
4. active call + HOOK short press: disconnects;
5. active call + HOOK long press: mutes/unmutes;
6. incoming/active/idle states + CALL probe: record exactly what the phone does;
7. remove the accessory and confirm ordinary phone behavior is unchanged.

If Linux input tracing is available during bench testing, the expected events
are `KEY_PLAYPAUSE` for HOOK and `KEY_PHONE` for CALL probe.

## Sources that define the boundary

- AOSP generic external-keyboard map:
  https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/data/keyboards/Generic.kl
- Android `KeyEvent` key-code definitions:
  https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/core/java/android/view/KeyEvent.java
- Linux generic HID input mapping:
  https://github.com/torvalds/linux/blob/master/drivers/hid/hid-input.c
- Linux input key definitions:
  https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
- Android compatibility requirements:
  https://source.android.com/docs/compatibility/cdd
- Android USB-headset HID mapping:
  https://source.android.com/docs/core/interaction/accessories/headset/usb-device

## Separate hardware problem: keeping the USB-C port usable

This firmware only defines the accessory's input behavior.  Simultaneous USB
host operation and charging/pass-through is a USB-C hub / role / power-delivery
hardware problem.  It must not be represented as something the key firmware
solves.
