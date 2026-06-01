#include <drivers/amba.h>
#include <kernel/printk.h>

#define UART_DR 0x00
#define UART_FR 0x18

static int pl011_probe(struct amba_device *dev) {
  boot_msg("AMBA", "Found PL011 UART\n", 0);

  return 0;
}

static struct amba_driver pl011_driver = {
    .name = "uart-pl011",
    .id_mask = 0x00041011,
    .probe = pl011_probe,
};
