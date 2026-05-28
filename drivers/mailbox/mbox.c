#include <drivers/mailbox.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

void mbox_init() {
    printk(CYAN, "[MBOX] Mailbox driver initialized.\n");
}

static inline void cpu_relax() {
#if defined(__x86__) || defined(__x86_64__)
    asm volatile("rep; nop" ::: "memory"); 
#elif defined(__riscv)
    asm volatile(".word 0x0100000F" ::: "memory"); 
#else
    asm volatile("" ::: "memory");
#endif
}


int mbox_send(struct mbox_msg *msg) {
    volatile uint32_t *status = (uint32_t *)MBOX_REG_STATUS;
    volatile uint32_t *data_reg = (uint32_t *)MBOX_REG_DATA;

    while (*status & MBOX_FULL) {
        cpu_relax(); 
    }

    *data_reg = msg->data;
    return 0;
}

int mbox_receive(struct mbox_msg *msg) {
    volatile uint32_t *status = (uint32_t *)MBOX_REG_STATUS;
    volatile uint32_t *data_reg = (uint32_t *)MBOX_REG_DATA;

    if (*status & MBOX_EMPTY) {
        return -1;
    }

    msg->data = *data_reg;
    return 0;
}