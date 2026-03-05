#include <blueos/ports.h>

static int battery_percentage = 100;
static int is_charging = 0;



void update_battery_status() {
    static int ticks = 0;
    ticks++;

    if (ticks > 5000) { 
        if (is_charging) {
            if (battery_percentage < 100) battery_percentage++;
        } else {
            if (battery_percentage > 0) battery_percentage--;
        }
        ticks = 0;
    }
}


void set_charging_status(int status) {
    is_charging = status;
}

int get_bat_level() {
    return battery_percentage;
}

int get_bat_charging() {
    return is_charging;
}