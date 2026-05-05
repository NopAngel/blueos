#include <config.h>
#include <kernel/ports.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

/* --- Direcciones Base del i8259A (Master & Slave) --- */
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

/* Comandos ICW (Initialization Control Words) */
#define ICW1_INIT       0x10      /* Iniciar inicialización */
#define ICW1_ICW4       0x01      /* Indica que se enviará ICW4 */
#define ICW4_8086       0x01      /* Modo 8086/88 */

/**
 * pic_init: Configura el controlador de interrupciones
 */
void pic_init(uint8_t offset1, uint8_t offset2) {
#if defined(I386) || defined(__x86_64__)
    /* 1. Guardar las máscaras actuales (opcional) */
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);


    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* 3. Remapear los vectores de interrupción (EL PASO CRUCIAL) */
    /* Por defecto, el teclado es IRQ1. Lo movemos para que no choque
       con las excepciones del CPU (0x00-0x1F) */
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();

    /* 4. Configurar la cascada (PIC1 le avisa al PIC2 en el IRQ2) */
    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    /* 5. Establecer el modo 8086 */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* 6. Restaurar máscaras (o dejar todo habilitado) */
    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);

    printk(LIGHT_BLUE, "PIC: i8259A initialized. Offsets: 0x%x, 0x%x\n", offset1, offset2);

#elif defined(RISCV)

    unsigned long mie;
    asm volatile("csrr %0, mie" : "=r"(mie));
    mie |= (1 << 11); // External Interrupt Enable
    asm volatile("csrw mie, %0" :: "r"(mie));

    printk(LIGHT_BLUE, "PLIC: RISC-V Interrupts enabled (MIE)\n");
#endif
}

/**
 * pic_send_eoi: Avisa al controlador que terminamos de procesar la interrupción
 */
void pic_send_eoi(uint8_t irq) {
#if defined(I386) || defined(__x86_64__)
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20); // End of Interrupt para Slave
    }
    outb(PIC1_COMMAND, 0x20);     // End of Interrupt para Master
#endif
}
