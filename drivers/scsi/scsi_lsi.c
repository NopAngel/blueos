#include <kernel/colors.h>
#include <kernel/ports.h>
#include <kernel/printk.h>

#define LSI_BASE_PORT 0xC000

#define LSI_REG_ISTAT (LSI_BASE_PORT + 0x14)
#define LSI_REG_DSTAT (LSI_BASE_PORT + 0x0C)
#define LSI_REG_SIST0 (LSI_BASE_PORT + 0x42)

void scsi_lsi_check() {
  unsigned char istat = inb(LSI_REG_ISTAT);

  printk("[ SCSI ] LSI Controller ISTAT: 0x%x\n", istat);

  if (istat == 0xFF) {
    printk("Error: Controller not responding at 0xC000\n");
  } else {
    printk("LSI controller detected and responding on BAR0!\n");
  }
}
