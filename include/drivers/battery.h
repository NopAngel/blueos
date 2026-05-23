#ifndef BATTERY_H
#define BATTERY_H

void ec_wait(uint8_t status_bit);
uint8_t ec_read(uint8_t addr);
void battery_update();
void battery_init();

#endif