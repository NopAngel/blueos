#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/* --- GPIO Modes --- */
#define GPIO_INPUT         0x00
#define GPIO_OUTPUT        0x01
#define GPIO_INPUT_PULLUP  0x02

/* --- GPIO Levels --- */
#define GPIO_LOW           0x00
#define GPIO_HIGH          0x01

/* --- Mock Ports (Architecture Dependent) --- */
#define GPIO_PORT_A        0x00
#define GPIO_PORT_B        0x01

void gpio_set_mode(uint8_t port, uint8_t pin, uint8_t mode);

void gpio_write_pin(uint8_t port, uint8_t pin, uint8_t level);

uint8_t gpio_read_pin(uint8_t port, uint8_t pin);

#endif