#include <config.h>
#include <kernel/ports.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

/* --- x86 Legacy PIC Definitions --- */
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

#define ICW1_INIT       0x10
#define ICW1_ICW4       0x01
#define ICW4_8086       0x01
#define PIC_EOI         0x20

/**
 * pic_init - Initializes the interrupt controller for the current architecture.
 */
void pic_init(uint8_t offset1, uint8_t offset2) {
#if defined(x86) || defined(__x86_64__)
    /* 1. Start initialization sequence (ICW1) */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* 2. Remap interrupt vectors (ICW2) */
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();

    /* 3. Setup cascading (ICW3) */
    outb(PIC1_DATA, 4); // Slave PIC at IRQ2
    io_wait();
    outb(PIC2_DATA, 2); // Identity of Slave
    io_wait();

    /* 4. Set 8086 mode (ICW4) */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* 5. Unmask all interrupts (Set mask to 0) */
    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);

    printk(LIGHT_BLUE, "PIC: i8259A initialized. Offsets: 0x%x, 0x%x\n", offset1, offset2);

#elif defined(__riscv)
    /* RISC-V: Enable External Interrupts in Machine/Supervisor mode */
    unsigned long mie_val;
    
    // MEIE (Machine External Interrupt Enable) is bit 11
    // SEIE (Supervisor External Interrupt Enable) is bit 9
    asm volatile("csrr %0, mie" : "=r"(mie_val));
    
#if defined(RISCV_SUPERVISOR)
    mie_val |= (1 << 9); 
#else
    mie_val |= (1 << 11); 
#endif

    asm volatile("csrw mie, %0" :: "r"(mie_val));

    printk(LIGHT_BLUE, "PLIC: RISC-V External interrupts enabled in MIE\n");
#endif
}

/**
 * pic_send_eoi - Informs the controller that the interrupt handling is complete.
 */
void pic_send_eoi(uint8_t irq) {
#if defined(x86) || defined(__x86_64__)
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
#elif defined(__riscv)
    /* On RISC-V, EOI is usually handled by writing to the PLIC Claim register.
       For now, we acknowledge it via the trap handler logic. */
#endif
}

/**
 * pic_mask_irq - Disables a specific IRQ line.
 */
void pic_mask_irq(uint8_t irq) {
#if defined(x86) || defined(__x86_64__)
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);
#endif
}