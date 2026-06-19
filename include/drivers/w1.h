#ifndef W1_H
#define W1_H

#include <stdint.h>

#define W1_RESET_US 480
#define W1_WRITE0_US 60
#define W1_WRITE1_US 10

#ifndef W1_DEFAULT_PIN
#define W1_DEFAULT_PIN 7
#endif

struct w1_bus {
  int pin;
  uint64_t last_rom;
};

void w1_init(int pin);
uint8_t w1_reset(int pin);
void w1_write_bit(int pin, uint8_t bit);
uint8_t w1_read_byte(int pin);

#endif