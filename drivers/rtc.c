#include <include/drivers/rtc.h>
#include <include/ports.h>
#include <include/gui/vga.h> 
int get_update_in_progress_flag() {
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

unsigned char get_rtc_register(int reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

void read_rtc(rtc_time_t* time) {

    while (get_update_in_progress_flag());

    time->second = get_rtc_register(0x00);
    time->minute = get_rtc_register(0x02);
    time->hour   = get_rtc_register(0x04);
    time->day    = get_rtc_register(0x07);
    time->month  = get_rtc_register(0x08);
    time->year   = get_rtc_register(0x09);

    time->second = (time->second & 0x0F) + ((time->second / 16) * 10);
    time->minute = (time->minute & 0x0F) + ((time->minute / 16) * 10);
    time->hour   = ((time->hour & 0x0F) + (((time->hour & 0x70) / 16) * 10)) | (time->hour & 0x80);
    time->day    = (time->day & 0x0F) + ((time->day / 16) * 10);
    time->month  = (time->month & 0x0F) + ((time->month / 16) * 10);
    time->year   = (time->year & 0x0F) + ((time->year / 16) * 10);
    
    time->year += 2000;
}