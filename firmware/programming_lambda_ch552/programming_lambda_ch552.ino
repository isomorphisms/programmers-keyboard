/*
 * Lambda programming keypad: CH552 firmware proof.
 *
 * The switch matrix produces semantic actions.  The USB encoder is a
 * separate layer because USB HID keyboard reports contain key usages, not
 * arbitrary Unicode code points.
 */

#ifndef USER_USB_RAM
#error "Select a CH55xduino USER USB RAM setting (user148 is enough)."
#endif

#include <stdbool.h>
#include <stdint.h>

#include "src/userUsbHidKeyboard/USBHIDKeyboard.h"

#define ROW_COUNT 3u
#define COL_COUNT 4u
#define KEY_COUNT (ROW_COUNT * COL_COUNT)
#define DEBOUNCE_SCANS 5u

/*
 * CH55xduino pin numbers are port * 10 + bit:
 * P1.4 -> 14, P1.6 -> 16, P1.7 -> 17,
 * P3.0 -> 30, ... P3.3 -> 33.
 *
 * P1.5 is intentionally left free as an alternate bootloader-select pin.
 * P3.6/P3.7 are intentionally left free for native USB D+/D-.
 */
static const uint8_t row_pins[ROW_COUNT] = {14u, 16u, 17u};
static const uint8_t col_pins[COL_COUNT] = {30u, 31u, 32u, 33u};

typedef enum {
    ACTION_NONE = 0,
    ACTION_LAMBDA,
} action_t;

/*
 * This keeps the same proof contract as the RP2040 branch: until the final
 * Programming-keypad placement is routed, row 0 / column 0 is the temporary
 * physical position of the semantic Lambda action.  The other eleven matrix
 * positions are deliberately unbound rather than guessed.
 */
static const action_t key_actions[KEY_COUNT] = {
    ACTION_LAMBDA,
    ACTION_NONE,
    ACTION_NONE,
    ACTION_NONE,
    ACTION_NONE,
    ACTION_NONE,
    ACTION_NONE,
    ACTION_NONE,
    ACTION_NONE,
    ACTION_NONE,
    ACTION_NONE,
    ACTION_NONE,
};

static bool raw_previous[KEY_COUNT];
static bool debounced[KEY_COUNT];
static uint8_t debounce_count[KEY_COUNT];

static void tap_key(uint8_t key) {
    Keyboard_press(key);
    Keyboard_release(key);
}

/*
 * Linux Unicode input encoder for U+03BB, lowercase lambda.
 *
 * This is intentionally an encoder for ACTION_LAMBDA, not the definition of
 * the key itself.  Other host encoders can replace this function later while
 * the matrix/action layer stays unchanged.
 */
static void emit_lambda_linux(void) {
    Keyboard_press(KEY_LEFT_CTRL);
    Keyboard_press(KEY_LEFT_SHIFT);
    tap_key('u');
    Keyboard_release(KEY_LEFT_SHIFT);
    Keyboard_release(KEY_LEFT_CTRL);

    delay(8);
    tap_key('0');
    tap_key('3');
    tap_key('b');
    tap_key('b');
    tap_key(KEY_RETURN);
}

static void emit_action(action_t action) {
    switch (action) {
        case ACTION_LAMBDA:
            emit_lambda_linux();
            break;
        case ACTION_NONE:
        default:
            break;
    }
}

static void matrix_init(void) {
    uint8_t row;
    uint8_t col;

    /* Rows idle high-impedance; one row at a time is driven low to scan. */
    for (row = 0; row < ROW_COUNT; ++row) {
        pinMode(row_pins[row], INPUT);
    }

    /* A closed switch on the selected row therefore reads LOW. */
    for (col = 0; col < COL_COUNT; ++col) {
        pinMode(col_pins[col], INPUT_PULLUP);
    }
}

static void debounce_sample(uint8_t key_index, bool pressed) {
    if (pressed != raw_previous[key_index]) {
        raw_previous[key_index] = pressed;
        debounce_count[key_index] = 1u;
    } else if (debounce_count[key_index] < DEBOUNCE_SCANS) {
        ++debounce_count[key_index];
    }

    if (debounce_count[key_index] < DEBOUNCE_SCANS ||
        debounced[key_index] == pressed) {
        return;
    }

    debounced[key_index] = pressed;
    if (pressed) {
        emit_action(key_actions[key_index]);
    }
}

static void matrix_scan(void) {
    uint8_t row;
    uint8_t col;

    for (row = 0; row < ROW_COUNT; ++row) {
        /* Select exactly one row. */
        pinMode(row_pins[row], OUTPUT);
        digitalWrite(row_pins[row], LOW);
        delayMicroseconds(3);

        for (col = 0; col < COL_COUNT; ++col) {
            uint8_t key_index = (uint8_t)(row * COL_COUNT + col);
            bool pressed = (digitalRead(col_pins[col]) == LOW);
            debounce_sample(key_index, pressed);
        }

        /* Do not drive inactive rows; this also avoids matrix contention. */
        pinMode(row_pins[row], INPUT);
    }
}

void setup(void) {
    matrix_init();
    USBInit();
}

void loop(void) {
    matrix_scan();
    delay(1);
}
