/* Test-only transport/GPIO boundary. The maintained scan, debounce, macro
 * and queue functions are included unchanged from main.c. */
#ifndef MOVEMENT_TEST_PLATFORM_H
#define MOVEMENT_TEST_PLATFORM_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
typedef unsigned int uint;
#define GPIO_IN 0
#define GPIO_OUT 1
#define HID_KEY_D 0x07
#define HID_KEY_I 0x0c
#define HID_KEY_J 0x0d
#define HID_KEY_K 0x0e
#define HID_KEY_W 0x1a
#define HID_KEY_X 0x1b
#define HID_KEY_1 0x1e
#define HID_KEY_3 0x20
#define HID_KEY_4 0x21
#define HID_KEY_7 0x24
#define HID_KEY_0 0x27
#define HID_KEY_ESCAPE 0x29
#define KEYBOARD_MODIFIER_LEFTSHIFT 0x02
#define TUSB_ROLE_DEVICE 1
#define TUSB_SPEED_AUTO 0
#define BOARD_TUD_RHPORT 0
typedef struct { int role; int speed; } tusb_rhport_init_t;
typedef unsigned int hid_report_type_t;
void gpio_init(uint pin);
void gpio_set_dir(uint pin, int direction);
void gpio_disable_pulls(uint pin);
void gpio_pull_up(uint pin);
void gpio_put(uint pin, int value);
bool gpio_get(uint pin);
void busy_wait_us_32(uint32_t duration);
uint64_t time_us_64(void);
bool tud_hid_ready(void);
bool tud_hid_keyboard_report(uint8_t report, uint8_t modifier, const uint8_t *keys);
void board_init(void);
void board_init_after_tusb(void);
void tusb_init(uint port, const tusb_rhport_init_t *initialization);
void tud_task(void);
#endif
