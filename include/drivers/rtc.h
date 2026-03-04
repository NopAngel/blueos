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

void read_rtc(int *second, int *minute, int *hour, int *day, int *month, int *year);
void get_time_string(char* buffer);

#endif