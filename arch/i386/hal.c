#include <kernel/hal.h>
#include <kernel/io.h> // for outb

void hal_halt(void) {
    /* Estilo Torvalds: Desactivamos interrupciones y detenemos el CPU.
       Si el usuario quiere salir, tendrá que apretar el botón físico. */
    asm volatile("cli");
    while (1) {
        asm volatile("hlt");
    }
}

void hal_reboot(void) {
    outb(0x64, 0xFE);
}
void hal_get_cpu_info(char* out) {
    /* TODO: Implementar cpuid de forma cruda */
    strcpy(out, "x86_64 GenuineIntel");
}
