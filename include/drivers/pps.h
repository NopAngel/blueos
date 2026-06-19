#ifndef PPS_H
#define PPS_H

#include <drivers/ptp.h>
#include <stdint.h>

struct pps_event {
  struct blueos_timespec ts;
  uint32_t sequence;
};

void pps_init(int gpio_pin);
void pps_handler();
struct pps_event *pps_get_last_event();

#endif