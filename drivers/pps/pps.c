#include <drivers/pps.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

static struct pps_event last_pps;
static int pps_gpio = 0;


void pps_init(int gpio_pin) {
    pps_gpio = gpio_pin;
    last_pps.sequence = 0;
    
    // gpio_set_direction(gpio_pin, INPUT);
    // gpio_enable_irq(gpio_pin, pps_handler);

    printk(CYAN, "[PPS] Pulse Per Second driver armed on GPIO %d\n", gpio_pin);
}

void pps_handler() {

    struct blueos_timespec now;
    ptp_get_time(&now);

    last_pps.ts = now;
    last_pps.sequence++;

    // if (now.tv_nsec > 500000000) now.tv_sec++; 
    // system_time_sync(now.tv_sec, 0);

}

struct pps_event* pps_get_last_event() {
    return &last_pps;
}