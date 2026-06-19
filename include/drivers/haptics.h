#ifndef HAPTICS_H
#define HAPTICS_H

#include <stdint.h>

#define HX_REG_STATUS 0x00
#define HX_REG_MODE 0x01
#define HX_REG_WAVEFORM 0x03
#define HX_REG_GO 0x0C

#define HX_EFFECT_CLICK 0x01
#define HX_EFFECT_BUMP 0x04
#define HX_EFFECT_LONG 0x0F

struct hx_device {
  uint8_t i2c_addr;
  uint8_t current_effect;
};

void hx_init(uint8_t addr);
void hx_play_effect(uint8_t effect_id);
void hx_stop();

#endif