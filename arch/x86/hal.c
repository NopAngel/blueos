#include <kernel/hal.h>
#include <kernel/io.h> // for outb

void hal_halt(void) {
    asm volatile("cli");
    while (1) {
        asm volatile("hlt");
    }
}

void hal_reboot(void) {
    outb(0x64, 0xFE);
}
void hal_get_cpu_info(char* out) {
    strcpy(out, "x86_64 GenuineIntel");
}
