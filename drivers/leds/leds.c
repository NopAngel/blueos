#include <drivers/leds.h>
#include <kernel/io.h>
#include <kernel/colors.h>
#include <kernel/printk.h>

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64
#define KBD_CMD_WRITE_LEDS 0xED

static void kbd_wait_write() {
    while (inb(KBD_STATUS_PORT) & 0x02);
}

void led_set_state(uint8_t led, uint8_t state) {
    static uint8_t current_leds = 0;

    if (state == LED_ON) {
        current_leds |= led;
    } else {
        current_leds &= ~led;
    }

    kbd_wait_write();
    outb(KBD_DATA_PORT, KBD_CMD_WRITE_LEDS);
    kbd_wait_write();
    outb(KBD_DATA_PORT, current_leds);
}

void leds_init(void) {
    boot_msg("LED", "Initializing LED Driver (Keyboard i8042)...\n", 0);
    led_set_state(LED_CAPS_LOCK, LED_OFF);
    led_set_state(LED_NUM_LOCK, LED_OFF);
    led_set_state(LED_SCROLL_LOCK, LED_OFF);
}