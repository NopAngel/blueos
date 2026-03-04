#include <stdint.h>
#include <hpet.h>
#include <blueos/printk.h>
#include <blueos/colors.h>
uint64_t hpet_base;
static uint32_t period_fs = 0;

void hpet_init(hpet_table_t* table) {
    hpet_base = table->base_address.address;

    uint64_t caps = *(volatile uint64_t*)(hpet_base + 0x00);
    uint32_t period = caps >> 32; 
    *(volatile uint64_t*)(hpet_base + 0x10) |= 0x03; 

    *(volatile uint64_t*)(hpet_base + 0xf0) = 0;
}

uint64_t hpet_get_nanos() {
    uint64_t counter = *(volatile uint64_t*)(hpet_base + 0xf0);

    return counter * (period_fs / 1000000);
}