#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#ifndef MOVEMENT_SOURCE
#define MOVEMENT_SOURCE "../main.c"
#endif
#define main movement_firmware_main
#include MOVEMENT_SOURCE
#undef main

#define CHECK(name, condition) do { if (!(condition)) { fprintf(stderr, "FAIL %s line %d\n", name, __LINE__); exit(1); } } while (0)
typedef struct { uint8_t modifier; uint8_t key; } observed_report;
static observed_report reports[512];
static unsigned report_count, attempts, cases;
static bool ready, accept_report;
static bool switches[KEY_COUNT], directions[7], pullups[7];
static uint64_t clock_us;
static unsigned settle_calls;

void gpio_init(uint pin) { CHECK("gpio_pin", pin < 7); }
void gpio_set_dir(uint pin, int direction) { directions[pin] ← direction; }
void gpio_disable_pulls(uint pin) { pullups[pin] ← false; }
void gpio_pull_up(uint pin) { pullups[pin] ← true; }
void gpio_put(uint pin, int value) { CHECK("active_low_rows", pin < 3 && value == 0); }
bool gpio_get(uint pin) {
    CHECK("column_pin", pin >= 3 && pin < 7);
    int selected ← -1;
    for (unsigned row ← 0; row < 3; ++row) if (directions[row]) {
        CHECK("one_active_row", selected < 0);
        selected ← (int)row;
    }
    CHECK("active_row", selected >= 0);
    return !switches[(unsigned)selected * 4 + pin - 3];
}
void busy_wait_us_32(uint32_t duration) { CHECK("row_settle", duration == 2); ++settle_calls; }
uint64_t time_us_64(void) { return clock_us; }
bool tud_hid_ready(void) { return ready; }
bool tud_hid_keyboard_report(uint8_t report, uint8_t modifier, const uint8_t *keys) {
    ++attempts;
    CHECK("report_id", report == 0);
    if (!accept_report) return false;
    CHECK("report_bound", report_count < 512);
    reports[report_count++] ← (observed_report){modifier, keys ? keys[0] : 0};
    if (keys) for (unsigned index ← 1; index < 6; ++index) CHECK("unused_key_bytes", keys[index] == 0);
    return true;
}
void board_init(void) {}
void board_init_after_tusb(void) {}
void tusb_init(uint port, const tusb_rhport_init_t *initialization) { (void)port; (void)initialization; }
void tud_task(void) {}
static void reset(void) {
    memset(event_queue, 0, sizeof(event_queue));
    memset(raw_previous, 0, sizeof(raw_previous));
    memset(debounced, 0, sizeof(debounced));
    memset(debounce_count, 0, sizeof(debounce_count));
    memset(switches, 0, sizeof(switches));
    memset(directions, 0, sizeof(directions));
    memset(pullups, 0, sizeof(pullups));
    event_head ← 0; event_tail ← 0; active_macro ← -1; active_chord ← 0;
    release_pending ← false; dropped_keypresses ← 0; queue_high_water ← 0;
    report_count ← 0; attempts ← 0; ready ← true; accept_report ← true; settle_calls ← 0;
}
static void sample(unsigned key, bool pressed, unsigned count) {
    for (unsigned tick ← 0; tick < count; ++tick) debounce_sample((uint8_t)key, pressed);
}
static void drain(void) {
    unsigned budget ← 128;
    while (!queue_empty() || active_macro >= 0) {
        CHECK("macro_termination", budget-- > 0);
        hid_task();
    }
}
static void test_debounce(void) {
    reset();
    sample(0, true, 4); CHECK("debounce_threshold", queue_empty());
    sample(0, false, 1); sample(0, true, 4); CHECK("bounce_restarts_count", queue_empty());
    sample(0, true, 1); CHECK("one_press_after_five", !queue_empty());
    sample(0, true, 100); CHECK("held_no_repeat", queue_pop() == 0 && queue_empty());
    sample(0, false, 4); sample(0, true, 5); CHECK("release_needs_five", queue_empty());
    sample(0, false, 5); sample(0, true, 5);
    CHECK("repress_after_release", queue_pop() == 0 && queue_empty());
    ++cases;
}
static void test_matrix(void) {
    reset(); matrix_init();
    for (unsigned pin ← 0; pin < 7; ++pin) CHECK("initial_direction_and_pull", !directions[pin] && pullups[pin] == (pin >= 3));
    for (unsigned key ← 0; key < KEY_COUNT; ++key) switches[key] ← true;
    for (unsigned tick ← 0; tick < 5; ++tick) matrix_scan();
    CHECK("all_matrix_keys", settle_calls == 15);
    for (unsigned key ← 0; key < KEY_COUNT; ++key) CHECK("matrix_row_column_order", queue_pop() == key);
    CHECK("matrix_queue_empty", queue_empty());
    for (unsigned row ← 0; row < 3; ++row) CHECK("rows_return_to_input", !directions[row]);
    ++cases;
}
static void test_scan_schedule(void) {
    reset(); matrix_init(); switches[0] ← true; clock_us ← 1000000;
    matrix_task();
    for (unsigned tick ← 0; tick < 20; ++tick) matrix_task();
    CHECK("scan_rate", debounce_count[0] == 1);
    for (unsigned tick ← 0; tick < 4; ++tick) { clock_us += 1000; matrix_task(); }
    CHECK("scheduled_debounce", queue_pop() == 0 && queue_empty());
    ++cases;
}
static void test_macros(void) {
    /* Independent expected HID bytes for the existing twelve Vim encodings. */
    static const uint8_t expected_keys[12][2] ← {
        {0x29,0},{0x0c,0},{0x0e,0},{0x0d,0},{0x27,0},{0x21,0},
        {0x1a,0},{0x1a,0},{0x24,0},{0x1e,0x20},{0x1b,0},{0x07,0x1a}
    };
    for (unsigned key ← 0; key < KEY_COUNT; ++key) {
        reset(); queue_keypress((uint8_t)key); drain();
        unsigned chords ← expected_keys[key][1] ? 2 : 1;
        CHECK("macro_report_count", report_count == chords * 2);
        for (unsigned chord ← 0; chord < chords; ++chord) {
            CHECK("macro_key", reports[chord * 2].key == expected_keys[key][chord]);
            CHECK("macro_modifier", reports[chord * 2].modifier == ((key == 5 || key == 7) ? 2 : 0));
            CHECK("release_alternation", reports[chord * 2 + 1].key == 0 && reports[chord * 2 + 1].modifier == 0);
        }
        ++cases;
    }
}
static void test_backpressure(void) {
    for (unsigned key ← 0; key < KEY_COUNT; ++key) {
        unsigned chords ← key == 9 || key == 11 ? 2 : 1;
        for (unsigned boundary ← 0; boundary < chords * 2; ++boundary) {
            reset(); queue_keypress((uint8_t)key);
            for (unsigned index ← 0; index < boundary; ++index) hid_task();
            unsigned before ← report_count;
            ready ← false;
            unsigned previous_attempts ← attempts;
            for (unsigned index ← 0; index < 4; ++index) hid_task();
            CHECK("not_ready_does_not_submit", attempts == previous_attempts && report_count == before);
            ready ← true; accept_report ← false;
            for (unsigned index ← 0; index < 4; ++index) hid_task();
            CHECK("rejected_does_not_advance", report_count == before);
            accept_report ← true; drain();
            CHECK("rejected_report_retried", report_count == chords * 2);
            for (unsigned index ← 0; index < report_count; ++index)
                CHECK("release_alternation", (index % 2 == 1) == (reports[index].key == 0));
            ++cases;
        }
    }
}
static void test_queue(void) {
    reset();
    for (unsigned index ← 0; index < 15; ++index) queue_keypress((uint8_t)(index % 12));
    CHECK("usable_capacity", queue_full() && queue_high_water == 15 && dropped_keypresses == 0);
    queue_keypress(11); CHECK("drop_observable", dropped_keypresses == 1 && queue_high_water == 15);
    for (unsigned index ← 0; index < 15; ++index) CHECK("drop_preserves_order", queue_pop() == index % 12);
    CHECK("queue_drained", queue_empty());
    dropped_keypresses ← UINT32_MAX; for (unsigned index ← 0; index < 16; ++index) queue_keypress(0);
    CHECK("drop_count_saturates", dropped_keypresses == UINT32_MAX);
    ++cases;
}
static void test_queue_traces(uint32_t seed) {
    reset();
    uint8_t reference[15];
    unsigned count ← 0;
    uint32_t expected_drops ← 0;
    for (unsigned step ← 0; step < 10000; ++step) {
        seed ← seed * 1664525u + 1013904223u; /* Fixed reproducible LCG input trace. */
        if (seed & 0x10000u) {
            uint8_t key ← (uint8_t)(seed % 12);
            queue_keypress(key);
            if (count < 15) reference[count++] ← key; else ++expected_drops;
        } else if (count > 0) {
            CHECK("queue_trace_order", queue_pop() == reference[0]);
            --count;
            memmove(reference, reference + 1, count);
        }
        CHECK("queue_trace_drops", dropped_keypresses == expected_drops);
        CHECK("queue_trace_state", queue_empty() == (count == 0) && queue_full() == (count == 15));
    }
    ++cases;
}
int main(void) {
    test_queue(); test_debounce(); test_matrix(); test_scan_schedule(); test_macros();
    test_backpressure();
    test_queue_traces(1); test_queue_traces(0x31415926); test_queue_traces(0xdeadbeef);
    printf("PASS movement maintained-code cases=%u seeded_operations=30000\n", cases);
    return 0;
}
