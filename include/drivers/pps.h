#ifndef PPS_H
#define PPS_H

#include <stdint.h>
#include <drivers/ptp.h>

struct pps_event {
    struct blueos_timespec ts; 
    uint32_t sequence;        
};

void pps_init(int gpio_pin);
void pps_handler();
struct pps_event* pps_get_last_event();

#endif