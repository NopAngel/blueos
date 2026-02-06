#ifndef RTC_H
#define RTC_H


#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

typedef struct {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned int year;
} rtc_time_t;

void read_rtc(rtc_time_t* time);
void get_time_string(char* buffer);

#endif