#ifndef _BLUEOS_LEDS_H
#define _BLUEOS_LEDS_H

#include <stddef.h>
#include <stdint.h>

#define LED_OFF 0
#define LED_ON  1

#define LED_SCROLL_LOCK 0x01
#define LED_NUM_LOCK    0x02
#define LED_CAPS_LOCK   0x04

void led_set_state(uint8_t led, uint8_t state);
void leds_init(void);

#endif