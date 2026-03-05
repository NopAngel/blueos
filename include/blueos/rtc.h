#ifndef RTC_H
#define RTC_H
int get_update_in_progress_flag();
uint8_t get_rtc_register(int reg);
void read_rtc(int *second, int *minute, int *hour, int *day, int *month, int *year);
#endif