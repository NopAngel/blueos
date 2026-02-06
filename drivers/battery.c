#include <include/ports.h>


int battery_percentage = 100;
int is_charging = 0;

int read_cmos(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

void update_battery_status() {
   
    int status = read_cmos(0x0B);
    
    if (status & 0x10) {
        is_charging = 1;
    } else {
        is_charging = 0;
    }


    static int ticks = 0;
    ticks++;
    if (ticks > 1000) {
        if (is_charging && battery_percentage < 100) battery_percentage++;
        else if (!is_charging && battery_percentage > 0) battery_percentage--;
        ticks = 0;
    }
}

int get_bat_level() {
    return battery_percentage;
}

int get_bat_charging() {
    return is_charging;
}