#ifndef RTC_H
#define RTC_H

#include <stdint.h>


typedef struct {
    uint8_t  second;
    uint8_t  minute;
    uint8_t  hour;
    uint8_t  day;
    uint8_t  month;
    uint32_t year;
    uint16_t msec;
} rtc_time_t;

void rtc_init(void);
uint8_t get_rtc_register(int reg);
void get_rtc_time(rtc_time_t *time);
void get_local_time(rtc_time_t *time);

#endif
