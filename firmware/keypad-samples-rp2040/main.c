#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bsp/board_api.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "tusb.h"

#define ROW_COUNT 4u
#define COL_COUNT 4u
#define KEY_COUNT (ROW_COUNT * COL_COUNT)
#define DEBOUNCE_SCANS 5u
#define EVENT_QUEUE_SIZE 32u

#define LAYOUT_CONCEPT_SEPARATION 1
#define LAYOUT_INCANTATION 2
#define LAYOUT_MATH 3
#define LAYOUT_REGEX 4
#define LAYOUT_PASTEBINS 5
#define LAYOUT_SIGNALS 6

#ifndef KEYPAD_LAYOUT
#error KEYPAD_LAYOUT must select one sample layout
#endif

/*
 * These remaining layouts do not have routed PCBs yet.  The sample firmware
 * assumes a diode-isolated 4x4 matrix so the largest current layout (regex)
 * fits without inventing layers.  Rows are driven low one at a time; columns
 * use the RP2040 internal pull-ups.
 */
static const uint row_pins[ROW_COUNT] = {0u, 1u, 2u, 3u};
static const uint col_pins[COL_COUNT] = {4u, 5u, 6u, 7u};

typedef struct {
    uint8_t modifier;
    uint8_t keycode;
} chord_t;

typedef struct {
    const chord_t *chords;
    uint8_t count;
    uint16_t hold_ms;
} key_binding_t;

#define CHORD_COUNT(array) ((uint8_t)(sizeof(array) / sizeof((array)[0])))
#define BIND(array) {(array), CHORD_COUNT(array), 0u}
#define HOLD_BIND(array, milliseconds) {(array), CHORD_COUNT(array), (milliseconds)}
#define UNUSED_BINDING {NULL, 0u, 0u}

#if KEYPAD_LAYOUT == LAYOUT_CONCEPT_SEPARATION

/* Linux Unicode input: Ctrl+Shift+U, 2001, Enter -> U+2001 EM QUAD. */
static const chord_t macro_quad_space[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_U},
    {0, HID_KEY_2}, {0, HID_KEY_0}, {0, HID_KEY_0}, {0, HID_KEY_1},
    {0, HID_KEY_ENTER},
};
static const chord_t macro_four_spaces[] = {
    {0, HID_KEY_SPACE}, {0, HID_KEY_SPACE},
    {0, HID_KEY_SPACE}, {0, HID_KEY_SPACE},
};
static const chord_t macro_double_space[] = {
    {0, HID_KEY_SPACE}, {0, HID_KEY_SPACE},
};
static const chord_t macro_single_space[] = {{0, HID_KEY_SPACE}};
static const chord_t macro_indent[] = {{0, HID_KEY_TAB}};
static const chord_t macro_underscore[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_MINUS},
};

static const key_binding_t keymap[KEY_COUNT] = {
    BIND(macro_quad_space),
    BIND(macro_four_spaces),
    BIND(macro_double_space),
    BIND(macro_single_space),
    BIND(macro_indent),
    BIND(macro_underscore),
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
    UNUSED_BINDING, UNUSED_BINDING,
};

#elif KEYPAD_LAYOUT == LAYOUT_INCANTATION

static const chord_t macro_tab_complete[] = {{0, HID_KEY_TAB}};
static const chord_t macro_finish_incantation[] = {{0, HID_KEY_ENTER}};
static const chord_t macro_chant_history[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_R},
};

static const key_binding_t keymap[KEY_COUNT] = {
    BIND(macro_tab_complete),
    BIND(macro_finish_incantation),
    BIND(macro_chant_history),
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
    UNUSED_BINDING,
};

#elif KEYPAD_LAYOUT == LAYOUT_MATH

static const chord_t macro_add[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_EQUAL},
};
static const chord_t macro_subtract[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_U},
    {0, HID_KEY_2}, {0, HID_KEY_2}, {0, HID_KEY_1}, {0, HID_KEY_2},
    {0, HID_KEY_ENTER},
};
static const chord_t macro_multiply[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_U},
    {0, HID_KEY_D}, {0, HID_KEY_7}, {0, HID_KEY_ENTER},
};
static const chord_t macro_divide[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_U},
    {0, HID_KEY_F}, {0, HID_KEY_7}, {0, HID_KEY_ENTER},
};
static const chord_t macro_equal[] = {{0, HID_KEY_EQUAL}};
static const chord_t macro_not_equal[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_U},
    {0, HID_KEY_2}, {0, HID_KEY_2}, {0, HID_KEY_6}, {0, HID_KEY_0},
    {0, HID_KEY_ENTER},
};
static const chord_t macro_less[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_COMMA},
};
static const chord_t macro_less_equal[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_U},
    {0, HID_KEY_2}, {0, HID_KEY_2}, {0, HID_KEY_6}, {0, HID_KEY_4},
    {0, HID_KEY_ENTER},
};
static const chord_t macro_greater[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_PERIOD},
};
static const chord_t macro_greater_equal[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_U},
    {0, HID_KEY_2}, {0, HID_KEY_2}, {0, HID_KEY_6}, {0, HID_KEY_5},
    {0, HID_KEY_ENTER},
};

/*
 * The rendered Math layout has eight faces.  Its last two source entries are
 * paired meanings (< / <= and > / >=).  The sample treats those faces as
 * two-contact rockers: positions 6/10 are the two less-than contacts and
 * positions 7/11 are the two greater-than contacts.
 */
static const key_binding_t keymap[KEY_COUNT] = {
    BIND(macro_add),             /* 0: ADD */
    BIND(macro_subtract),        /* 1: SUBTRACT */
    BIND(macro_multiply),        /* 2: MULTIPLY */
    BIND(macro_divide),          /* 3: DIVIDE */
    BIND(macro_equal),           /* 4: EQUAL */
    BIND(macro_not_equal),       /* 5: NOT EQUAL */
    BIND(macro_less),            /* 6: < rocker contact */
    BIND(macro_greater),         /* 7: > rocker contact */
    UNUSED_BINDING,              /* 8 */
    UNUSED_BINDING,              /* 9 */
    BIND(macro_less_equal),      /* 10: <= rocker contact */
    BIND(macro_greater_equal),   /* 11: >= rocker contact */
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
};

#elif KEYPAD_LAYOUT == LAYOUT_REGEX

/* PCRE2-like text encoder on a US keyboard layout. */
static const chord_t macro_start_line[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_6},
};
static const chord_t macro_end_line[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_4},
};
static const chord_t macro_end_slurp[] = {
    {0, HID_KEY_BACKSLASH}, {0, HID_KEY_Z},
};
static const chord_t macro_maybe[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_SLASH},
};
static const chord_t macro_big_capture[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_9},
    {0, HID_KEY_PERIOD},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_8},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_0},
};
static const chord_t macro_small_capture[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_9},
    {0, HID_KEY_PERIOD},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_8},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_SLASH},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_0},
};
static const chord_t macro_exact_order_group[] = {
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_9},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_0},
};
static const chord_t macro_any_from_list[] = {
    {0, HID_KEY_BRACKET_LEFT}, {0, HID_KEY_BRACKET_RIGHT},
};
static const chord_t macro_letter[] = {
    {0, HID_KEY_BACKSLASH}, {0, HID_KEY_P},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_BRACKET_LEFT},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_L},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_BRACKET_RIGHT},
};
static const chord_t macro_number[] = {
    {0, HID_KEY_BACKSLASH}, {0, HID_KEY_D},
};
static const chord_t macro_non_weird[] = {
    {0, HID_KEY_BACKSLASH}, {0, HID_KEY_W},
};
static const chord_t macro_weird[] = {
    {0, HID_KEY_BACKSLASH},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_W},
};
static const chord_t macro_control_characters[] = {
    {0, HID_KEY_BACKSLASH}, {0, HID_KEY_P},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_BRACKET_LEFT},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_C}, {0, HID_KEY_C},
    {KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_BRACKET_RIGHT},
};

/* The 13-key regex text list has no updated rendered row layout yet. */
static const key_binding_t keymap[KEY_COUNT] = {
    BIND(macro_start_line),
    BIND(macro_end_line),
    BIND(macro_end_slurp),
    BIND(macro_maybe),
    BIND(macro_big_capture),
    BIND(macro_small_capture),
    BIND(macro_exact_order_group),
    BIND(macro_any_from_list),
    BIND(macro_letter),
    BIND(macro_number),
    BIND(macro_non_weird),
    BIND(macro_weird),
    BIND(macro_control_characters),
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
};

#elif KEYPAD_LAYOUT == LAYOUT_PASTEBINS

/* Distinct HID usages are transport tokens; host software owns pastebin state. */
static const chord_t macro_view_1[] = {{0, HID_KEY_F13}};
static const chord_t macro_view_2[] = {{0, HID_KEY_F14}};
static const chord_t macro_view_3[] = {{0, HID_KEY_F15}};
static const chord_t macro_remember_1[] = {{0, HID_KEY_F16}};
static const chord_t macro_remember_2[] = {{0, HID_KEY_F17}};
static const chord_t macro_remember_3[] = {{0, HID_KEY_F18}};
static const chord_t macro_write_1[] = {{0, HID_KEY_F19}};
static const chord_t macro_write_2[] = {{0, HID_KEY_F20}};
static const chord_t macro_write_3[] = {{0, HID_KEY_F21}};

/* Preserve the rendered 3 x 3 rows inside the common 4 x 4 matrix. */
static const key_binding_t keymap[KEY_COUNT] = {
    BIND(macro_view_1), BIND(macro_view_2), BIND(macro_view_3), UNUSED_BINDING,
    BIND(macro_remember_1), BIND(macro_remember_2), BIND(macro_remember_3), UNUSED_BINDING,
    BIND(macro_write_1), BIND(macro_write_2), BIND(macro_write_3), UNUSED_BINDING,
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
};

#elif KEYPAD_LAYOUT == LAYOUT_SIGNALS

/*
 * A USB keyboard cannot select an arbitrary process and send it a POSIX signal.
 * F13-F18 are therefore explicit host-binding tokens, one per semantic signal.
 */
static const chord_t macro_sigint[] = {{0, HID_KEY_F13}};
static const chord_t macro_sigtstp[] = {{0, HID_KEY_F14}};
static const chord_t macro_sigcont[] = {{0, HID_KEY_F15}};
static const chord_t macro_sigterm[] = {{0, HID_KEY_F16}};
static const chord_t macro_sigquit[] = {{0, HID_KEY_F17}};
static const chord_t macro_sigkill[] = {{0, HID_KEY_F18}};

/* Preserve the rendered two rows of three keys inside the 4 x 4 matrix. */
static const key_binding_t keymap[KEY_COUNT] = {
    BIND(macro_sigint), BIND(macro_sigtstp), BIND(macro_sigcont), UNUSED_BINDING,
    HOLD_BIND(macro_sigterm, 1000u), BIND(macro_sigquit), HOLD_BIND(macro_sigkill, 2000u), UNUSED_BINDING,
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
    UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING, UNUSED_BINDING,
};

#else
#error Unknown KEYPAD_LAYOUT
#endif

static uint8_t event_queue[EVENT_QUEUE_SIZE];
static uint8_t event_head;
static uint8_t event_tail;

static bool raw_previous[KEY_COUNT];
static bool debounced[KEY_COUNT];
static uint8_t debounce_count[KEY_COUNT];
static uint64_t pressed_since_us[KEY_COUNT];
static bool hold_fired[KEY_COUNT];

static int8_t active_key = -1;
static uint8_t active_chord;
static bool release_pending;

static bool queue_empty(void) {
    return event_head == event_tail;
}

static bool queue_full(void) {
    return (uint8_t)((event_head + 1u) % EVENT_QUEUE_SIZE) == event_tail;
}

static bool queue_keypress(uint8_t key_index) {
    if (queue_full()) {
        return false;
    }

    event_queue[event_head] = key_index;
    event_head = (uint8_t)((event_head + 1u) % EVENT_QUEUE_SIZE);
    return true;
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

static void accept_state_change(uint8_t key_index, bool pressed, uint64_t now) {
    const key_binding_t *binding = &keymap[key_index];

    debounced[key_index] = pressed;
    if (!pressed) {
        pressed_since_us[key_index] = 0;
        hold_fired[key_index] = false;
        return;
    }

    pressed_since_us[key_index] = now;
    hold_fired[key_index] = false;

    if (binding->count == 0u) {
        hold_fired[key_index] = true;
        return;
    }

    if (binding->hold_ms == 0u) {
        hold_fired[key_index] = queue_keypress(key_index);
    }
}

static void debounce_sample(uint8_t key_index, bool pressed, uint64_t now) {
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

    accept_state_change(key_index, pressed, now);
}

static void hold_task(uint64_t now) {
    for (uint8_t key_index = 0; key_index < KEY_COUNT; ++key_index) {
        const key_binding_t *binding = &keymap[key_index];
        if (!debounced[key_index] || hold_fired[key_index] ||
            binding->count == 0u || binding->hold_ms == 0u) {
            continue;
        }

        uint64_t required_us = (uint64_t)binding->hold_ms * 1000u;
        if (now - pressed_since_us[key_index] >= required_us &&
            queue_keypress(key_index)) {
            hold_fired[key_index] = true;
        }
    }
}

static void matrix_scan(uint64_t now) {
    for (uint row = 0; row < ROW_COUNT; ++row) {
        gpio_put(row_pins[row], 0);
        gpio_set_dir(row_pins[row], GPIO_OUT);
        busy_wait_us_32(2);

        for (uint col = 0; col < COL_COUNT; ++col) {
            uint8_t key_index = (uint8_t)(row * COL_COUNT + col);
            debounce_sample(key_index, !gpio_get(col_pins[col]), now);
        }

        gpio_set_dir(row_pins[row], GPIO_IN);
    }

    hold_task(now);
}

static void matrix_task(void) {
    static uint64_t next_scan_us;
    uint64_t now = time_us_64();

    if (now < next_scan_us) {
        return;
    }

    next_scan_us = now + 1000u;
    matrix_scan(now);
}

static void hid_task(void) {
    if (!tud_hid_ready()) {
        return;
    }

    if (active_key < 0) {
        if (queue_empty()) {
            return;
        }

        active_key = (int8_t)queue_pop();
        active_chord = 0u;
        release_pending = false;
    }

    const key_binding_t *binding = &keymap[(uint8_t)active_key];
    if (binding->count == 0u) {
        active_key = -1;
        return;
    }

    if (!release_pending) {
        uint8_t keycodes[6] = {0};
        const chord_t *chord = &binding->chords[active_chord];
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
    if (active_chord >= binding->count) {
        active_key = -1;
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
