#include <kernel/colors.h>
#include <kernel/io.h>
#include <kernel/printk.h>

#define EC_DATA 0x62
#define EC_SC 0x66
#define EC_RD_CMD 0x80

#define BAT_STATE 0x01
#define BAT_RATE 0x02
#define BAT_CAPACITY 0x03
#define BAT_VOLTAGE 0x04

void ec_wait(uint8_t status_bit) {
  while ((inb(EC_SC) & status_bit))
    ;
}

uint8_t ec_read(uint8_t addr) {
  ec_wait(0x02);
  outb(EC_SC, EC_RD_CMD);
  ec_wait(0x02);
  outb(EC_DATA, addr);
  ec_wait(0x01);
  return inb(EC_DATA);
}

void battery_update() {
  uint8_t capacity = ec_read(BAT_CAPACITY);
  uint8_t state = ec_read(BAT_STATE);
  uint16_t voltage = ec_read(BAT_VOLTAGE);

  /* state: bit 0 = discharging, bit 1 = charging */
  char *status = "Idle";
  if (state & 0x01)
    status = "Discharging";
  if (state & 0x02)
    status = "Charging";

  printk("\r[Battery] %d%% | Status: %s | Volts: %dmV    ", capacity, status,
         voltage);
}

void battery_init() {
  uint8_t check = inb(EC_SC);
  if (check == 0xFF) {
    printk("[Battery] No EC found (Virtual Machine?)\n");
    return;
  }
  printk("[  OK  ] Battery driver initialized.\n");
}
