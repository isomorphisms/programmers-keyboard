#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bsp/board_api.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "tusb.h"

#define BUTTON_COUNT 2u
#define DEBOUNCE_SCANS 5u

/*
 * USB HID Consumer usages, HID Usage Tables 1.5:
 *   0x00cd  Play/Pause
 *   0x008c  Media Select Telephone
 *
 * On the stock Linux -> Android input path:
 *   0x00cd -> KEY_PLAYPAUSE -> Android MEDIA_PLAY_PAUSE
 *   0x008c -> KEY_PHONE     -> Android CALL
 *
 * Android telephony compatibility rules give MEDIA_PLAY_PAUSE headset-hook
 * behavior.  Short press answers an incoming call and disconnects an ongoing
 * call.  Holding the physical button keeps the HID usage asserted so Android
 * can also distinguish its documented long-press actions.
 *
 * The CALL usage is kept as a separate probe because it is mapped by AOSP, but
 * its exact action is phone/dialer state dependent.
 */
#define CONSUMER_PLAY_PAUSE 0x00cdu
#define CONSUMER_MEDIA_SELECT_TELEPHONE 0x008cu

/* Active-low buttons using the RP2040's internal pull-ups. */
static const uint button_pins[BUTTON_COUNT] = {2u, 3u};
static const uint16_t button_usages[BUTTON_COUNT] = {
    CONSUMER_PLAY_PAUSE,
    CONSUMER_MEDIA_SELECT_TELEPHONE,
};

static bool raw_previous[BUTTON_COUNT];
static bool debounced[BUTTON_COUNT];
static uint8_t debounce_count[BUTTON_COUNT];

static uint16_t desired_usage;
static uint16_t reported_usage;

static void buttons_init(void) {
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);

        bool pressed = !gpio_get(button_pins[i]);
        raw_previous[i] = pressed;
        debounced[i] = pressed;
        debounce_count[i] = DEBOUNCE_SCANS;
    }
}

static void debounce_sample(uint8_t button_index, bool pressed) {
    if (pressed != raw_previous[button_index]) {
        raw_previous[button_index] = pressed;
        debounce_count[button_index] = 1u;
        return;
    }

    if (debounce_count[button_index] < DEBOUNCE_SCANS) {
        ++debounce_count[button_index];
    }

    if (debounce_count[button_index] >= DEBOUNCE_SCANS) {
        debounced[button_index] = pressed;
    }
}

static uint16_t current_usage(void) {
    /* HOOK has priority if both physical buttons are held simultaneously. */
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
        if (debounced[i]) {
            return button_usages[i];
        }
    }
    return 0u;
}

static void button_task(void) {
    static uint64_t next_scan_us;
    uint64_t now = time_us_64();

    if (now < next_scan_us) {
        return;
    }

    next_scan_us = now + 1000u;
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
        debounce_sample(i, !gpio_get(button_pins[i]));
    }

    desired_usage = current_usage();
}

static bool send_consumer_usage(uint16_t usage) {
    /* HID report descriptor is a single little-endian 16-bit Consumer usage. */
    uint8_t report[2] = {
        (uint8_t)(usage & 0x00ffu),
        (uint8_t)((usage >> 8) & 0x00ffu),
    };
    return tud_hid_report(0, report, sizeof(report));
}

static void hid_task(void) {
    if (desired_usage == reported_usage || !tud_hid_ready()) {
        return;
    }

    if (send_consumer_usage(desired_usage)) {
        reported_usage = desired_usage;
    }
}

int main(void) {
    board_init();
    buttons_init();

    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO,
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    board_init_after_tusb();

    while (true) {
        tud_task();
        button_task();
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
