#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bsp/board_api.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "tusb.h"

#define ROW_COUNT 3u
#define COL_COUNT 4u
#define KEY_COUNT (ROW_COUNT * COL_COUNT)
#define DEBOUNCE_SCANS 5u
#define EVENT_QUEUE_SIZE 16u

/*
 * The final Programming keypad PCB is not routed yet.  For an end-to-end
 * firmware proof, row 0 / column 0 of the existing 3x4 RP2040 matrix is the
 * temporary physical trigger for the semantic Lambda action.
 */
#define LAMBDA_PROOF_KEY_INDEX 0u

static const uint row_pins[ROW_COUNT] = {0u, 1u, 2u};
static const uint col_pins[COL_COUNT] = {3u, 4u, 5u, 6u};

typedef struct {
    uint8_t modifier;
    uint8_t keycode;
} chord_t;

typedef struct {
    const chord_t *chords;
    uint8_t count;
} macro_t;

typedef enum {
    ACTION_NONE = 0,
    ACTION_LAMBDA,
} action_t;

/*
 * USB HID keyboard reports describe physical keyboard usages, not Unicode
 * code points.  Keep the semantic action separate from this host encoding.
 * On Linux text input, Ctrl+Shift+U 03bb Enter produces U+03BB: lambda (λ).
 */
static const chord_t lambda_linux_chords[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_U},
    {0, HID_KEY_0},
    {0, HID_KEY_3},
    {0, HID_KEY_B},
    {0, HID_KEY_B},
    {0, HID_KEY_ENTER},
};

static const macro_t lambda_linux_macro = {
    lambda_linux_chords,
    (uint8_t)(sizeof(lambda_linux_chords) / sizeof(lambda_linux_chords[0])),
};

static const action_t key_actions[KEY_COUNT] = {
    [LAMBDA_PROOF_KEY_INDEX] = ACTION_LAMBDA,
};

static action_t event_queue[EVENT_QUEUE_SIZE];
static uint8_t event_head;
static uint8_t event_tail;

static bool raw_previous[KEY_COUNT];
static bool debounced[KEY_COUNT];
static uint8_t debounce_count[KEY_COUNT];

static action_t active_action = ACTION_NONE;
static uint8_t active_chord;
static bool release_pending;

static const macro_t *macro_for_action(action_t action) {
    switch (action) {
        case ACTION_LAMBDA:
            return &lambda_linux_macro;
        case ACTION_NONE:
        default:
            return NULL;
    }
}

static bool queue_empty(void) {
    return event_head == event_tail;
}

static bool queue_full(void) {
    return (uint8_t)((event_head + 1u) % EVENT_QUEUE_SIZE) == event_tail;
}

static void queue_action(action_t action) {
    if (action == ACTION_NONE || queue_full()) {
        return;
    }

    event_queue[event_head] = action;
    event_head = (uint8_t)((event_head + 1u) % EVENT_QUEUE_SIZE);
}

static action_t queue_pop(void) {
    action_t action = event_queue[event_tail];
    event_tail = (uint8_t)((event_tail + 1u) % EVENT_QUEUE_SIZE);
    return action;
}

static void matrix_init(void) {
    for (uint row = 0; row < ROW_COUNT; ++row) {
        gpio_init(row_pins[row]);
        gpio_set_dir(row_pins[row], GPIO_IN);
        gpio_disable_pulls(row_pins[row]);
    }

    for (uint col = 0; col < COL_COUNT; ++col) {
        gpio_init(col_pins[col]);
        gpio_set_dir(col_pins[col], GPIO_IN);
        gpio_pull_up(col_pins[col]);
    }
}

static void debounce_sample(uint8_t key_index, bool pressed) {
    if (pressed != raw_previous[key_index]) {
        raw_previous[key_index] = pressed;
        debounce_count[key_index] = 1;
    } else if (debounce_count[key_index] < DEBOUNCE_SCANS) {
        ++debounce_count[key_index];
    }

    if (debounce_count[key_index] < DEBOUNCE_SCANS ||
        debounced[key_index] == pressed) {
        return;
    }

    debounced[key_index] = pressed;
    if (pressed) {
        queue_action(key_actions[key_index]);
    }
}

static void matrix_scan(void) {
    for (uint row = 0; row < ROW_COUNT; ++row) {
        gpio_put(row_pins[row], 0);
        gpio_set_dir(row_pins[row], GPIO_OUT);
        busy_wait_us_32(2);

        for (uint col = 0; col < COL_COUNT; ++col) {
            uint8_t key_index = (uint8_t)(row * COL_COUNT + col);
            debounce_sample(key_index, !gpio_get(col_pins[col]));
        }

        gpio_set_dir(row_pins[row], GPIO_IN);
    }
}

static void matrix_task(void) {
    static uint64_t next_scan_us;
    uint64_t now = time_us_64();

    if (now < next_scan_us) {
        return;
    }

    next_scan_us = now + 1000u;
    matrix_scan();
}

static void hid_task(void) {
    if (!tud_hid_ready()) {
        return;
    }

    if (active_action == ACTION_NONE) {
        if (queue_empty()) {
            return;
        }

        active_action = queue_pop();
        active_chord = 0;
        release_pending = false;
    }

    const macro_t *macro = macro_for_action(active_action);
    if (macro == NULL) {
        active_action = ACTION_NONE;
        return;
    }

    if (!release_pending) {
        uint8_t keycodes[6] = {0};
        const chord_t *chord = &macro->chords[active_chord];
        keycodes[0] = chord->keycode;

        if (tud_hid_keyboard_report(0, chord->modifier, keycodes)) {
            release_pending = true;
        }
        return;
    }

    if (!tud_hid_keyboard_report(0, 0, NULL)) {
        return;
    }

    release_pending = false;
    ++active_chord;
    if (active_chord >= macro->count) {
        active_action = ACTION_NONE;
    }
}

int main(void) {
    board_init();
    matrix_init();

    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO,
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    board_init_after_tusb();

    while (true) {
        tud_task();
        matrix_task();
        hid_task();
    }
}

uint16_t tud_hid_get_report_cb(uint8_t instance,
                               uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer,
                               uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance,
                           uint8_t report_id,
                           hid_report_type_t report_type,
                           const uint8_t *buffer,
                           uint16_t bufsize) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}
