/*
 * BlueOS - Hello World ELF Test
 * Este programa corre en Ring 3 y usa syscalls por interrupción 0x80.
 */

#include <stdint.h>

// Definiciones manuales para no depender de headers externos en el test
#define SYS_PRINTK 4
#define SYS_EXIT   6

void _start() {
    const char* msg = "\n[USERSPACE] ¡Hola bro! Saludos desde mi primer ELF en BlueOS!\n";

    // Llamada a SYS_PRINTK (4)
    // Según tu syscall.c: eax = syscall, ebx = string pointer
    asm volatile (
        "int $0x80"
        :
        : "a"(SYS_PRINTK), "b"(msg)
    );

    // Llamada a SYS_EXIT (6)
    asm volatile (
        "int $0x80"
        :
        : "a"(SYS_EXIT), "b"(0)
    );
}
