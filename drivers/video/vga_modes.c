#include <kernel/printk.h>
#include <kernel/io.h>
#include <stdint.h>


#define VGA_CRT_ICW  0x3D4
#define VGA_CRT_DATA 0x3D5
#define VGA_MISC_OUT 0x3C2

/* * Función inspirada en vga_set_8font de Linux,
 * pero adaptada para modo protegido 32-bit sin BIOS.
 * Cambia la terminal a 80x50 (fuente pequeña).
 */
void vga_set_80x50(void) {

    outb(0x11, VGA_CRT_ICW);
    uint8_t val = inb(VGA_CRT_DATA);
    outb(val & 0x7F, VGA_CRT_DATA);

    outb(0x09, VGA_CRT_ICW);
    outb(0x07, VGA_CRT_DATA); // 8 pixels

    outb(0x0A, VGA_CRT_ICW);
    outb(0x00, VGA_CRT_DATA);
    outb(0x0B, VGA_CRT_ICW);
    outb(0x07, VGA_CRT_DATA);

    outb(0x11, VGA_CRT_ICW);
    val = inb(VGA_CRT_DATA);
    outb(val | 0x80, VGA_CRT_DATA);

    pr_info("VGA: Modo 80x50 activado (8pt font)\n");
}

/* * Restablecer a 80x25 (estándar)
 */
void vga_set_80x25(void) {
    outb(0x09, VGA_CRT_ICW);
    outb(0x0F, VGA_CRT_DATA);

    pr_info("VGA: Modo 80x25 restaurado\n");
}
