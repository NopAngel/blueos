#include <blueos/ports.h>
#include <blueos/types.h>

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

int get_update_in_progress_flag() {
    outb(CMOS_ADDR, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

void read_rtc(int *second, int *minute, int *hour, int *day, int *month, int *year) {
    while (get_update_in_progress_flag());

    *second = get_rtc_register(0x00);
    *minute = get_rtc_register(0x02);
    *hour   = get_rtc_register(0x04);
    *day    = get_rtc_register(0x07);
    *month  = get_rtc_register(0x08);
    *year   = get_rtc_register(0x09);

    uint8_t registerB = get_rtc_register(0x0B);

    if (!(registerB & 0x04)) {
        *second = (*second & 0x0F) + ((*second / 16) * 10);
        *minute = (*minute & 0x0F) + ((*minute / 16) * 10);
        *hour = ((*hour & 0x0F) + (((*hour & 0x70) / 16) * 10)) | (*hour & 0x80);
        *day = (*day & 0x0F) + ((*day / 16) * 10);
        *month = (*month & 0x0F) + ((*month / 16) * 10);
        *year = (*year & 0x0F) + ((*year / 16) * 10);
    }
}