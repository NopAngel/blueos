#include <stdint.h>


void machine_power_off() {

    volatile uint32_t *syscon = (uint32_t *)0x100000; 
    *syscon = 0x5555;
    
    while(1);
}