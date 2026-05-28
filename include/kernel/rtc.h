#ifndef RTC_H
#define RTC_H

#include <stdint.h>

/* Functions to interact with the Real Time Clock */
void rtc_init(void);
uint8_t get_rtc_register(int reg);
uint8_t get_rtc_second(void);

#endif