#ifndef PS3_DS3_H
#define PS3_DS3_H

#include <stdint.h>

#define DS3_BUTTON_SELECT 0x01
#define DS3_BUTTON_START 0x08
#define DS3_BUTTON_PS 0x10

struct ds3_report {
  uint8_t report_id;
  uint8_t reserved1;
  uint32_t buttons; // Square, Cross, Circle, Triangle, L1, R1...
  uint8_t ps_button;
  uint8_t reserved2;
  uint8_t left_stick_x;
  uint8_t left_stick_y;
  uint8_t right_stick_x;
  uint8_t right_stick_y;
} __attribute__((packed));

void ds3_init();
void ds3_handle_packet(uint8_t *buf, int len);

#endif