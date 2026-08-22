#include <stdbool.h>
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

static const chord_t macro_movement[] = {{0, HID_KEY_ESCAPE}};
static const chord_t macro_type[] = {{0, HID_KEY_I}};
static const chord_t macro_prev_line[] = {{0, HID_KEY_K}};
static const chord_t macro_next_line[] = {{0, HID_KEY_J}};
static const chord_t macro_start_line[] = {{0, HID_KEY_0}};
static const chord_t macro_end_line[] = {{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_4}};
static const chord_t macro_next_word[] = {{0, HID_KEY_W}};
static const chord_t macro_next_space_word[] = {{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_W}};
static const chord_t macro_7_times[] = {{0, HID_KEY_7}};
static const chord_t macro_13_times[] = {{0, HID_KEY_1}, {0, HID_KEY_3}};
static const chord_t macro_delete_char[] = {{0, HID_KEY_X}};
static const chord_t macro_delete_word[] = {{0, HID_KEY_D}, {0, HID_KEY_W}};

static const macro_t keymap[KEY_COUNT] = {
    {macro_movement, 1},
    {macro_type, 1},
    {macro_prev_line, 1},
    {macro_next_line, 1},
    {macro_start_line, 1},
    {macro_end_line, 1},
    {macro_next_word, 1},
    {macro_next_space_word, 1},
    {macro_7_times, 1},
    {macro_13_times, 2},
    {macro_delete_char, 1},
    {macro_delete_word, 2},
};

static uint8_t event_queue[EVENT_QUEUE_SIZE];
static uint8_t event_head;
static uint8_t event_tail;

static bool raw_previous[KEY_COUNT];
static bool debounced[KEY_COUNT];
static uint8_t debounce_count[KEY_COUNT];

static int8_t active_macro = -1;
static uint8_t active_chord;
static bool release_pending;

static bool queue_empty(void) {
    return event_head == event_tail;
}

static bool queue_full(void) {
    return (uint8_t)((event_head + 1u) % EVENT_QUEUE_SIZE) == event_tail;
}

static void queue_keypress(uint8_t key_index) {
    if (queue_full()) {
        return;
    }

    event_queue[event_head] = key_index;
    event_head = (uint8_t)((event_head + 1u) % EVENT_QUEUE_SIZE);
}

static uint8_t queue_pop(void) {
    uint8_t key_index = event_queue[event_tail];
    event_tail = (uint8_t)((event_tail + 1u) % EVENT_QUEUE_SIZE);
    return key_index;
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
        queue_keypress(key_index);
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

    if (active_macro < 0) {
        if (queue_empty()) {
            return;
        }

        active_macro = (int8_t)queue_pop();
        active_chord = 0;
        release_pending = false;
    }

    const macro_t *macro = &keymap[(uint8_t)active_macro];

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
        active_macro = -1;
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
