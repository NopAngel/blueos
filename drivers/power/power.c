#include <stdint.h>
#include <blueos/io.h>

void sys_reboot___debug_() {
    outb(0x64, 0xFE);

    __asm__ volatile("lidt %0; int3" : : "m"((uint32_t[]){0, 0}));
}

void sys_shutdown___debug_() {

    outw(0x604, 0x2000); 
    outw(0xB004, 0x2000);

    __asm__ volatile("cli");
    while(1) {
        __asm__ volatile("hlt");
    }
}