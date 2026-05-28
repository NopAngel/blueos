#include <drivers/amba.h>
#include <kernel/printk.h>

#define UART_DR 0x00
#define UART_FR 0x18

static int pl011_probe(struct amba_device *dev) {
    pr_info("AMBA: Found PL011 UART at 0x%p\n", dev->res_start);


    return 0;
}

static struct amba_driver pl011_driver = {
    .name = "uart-pl011",
    .id_mask = 0x00041011,
    .probe = pl011_probe,
};
