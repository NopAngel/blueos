#include <kernel/colors.h>
#include <kernel/ports.h>
#include <kernel/printk.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_NAME "FLOPPY"

/* Floppy Disk Controller (FDC) I/O Ports */
#define FDC_SRA 0x3F0  /* Status Register A */
#define FDC_SRB 0x3F1  /* Status Register B */
#define FDC_DOR 0x3F2  /* Digital Output Register */
#define FDC_TDR 0x3F3  /* Tape Drive Register */
#define FDC_MSR 0x3F4  /* Main Status Register (Read Only) */
#define FDC_DSR 0x3F4  /* Data Rate Select Register (Write Only) */
#define FDC_FIFO 0x3F5 /* Data FIFO Register */
#define FDC_DIR 0x3F7  /* Digital Input Register (Read Only) */
#define FDC_CCR 0x3F7  /* Configuration Control Register (Write Only) */

#define FLOPPY_SECTOR_SIZE 512

/* FDC Commands */
#define CMD_SPECIFY 3
#define CMD_WRITE 5
#define CMD_READ 6
#define CMD_RECAL 7
#define CMD_SENSEI 8

static int g_floppy_motor_on = 0;

/**
 * floppy_write_cmd: Sends a single command byte into the FDC hardware FIFO.
 */
static void floppy_write_cmd(uint8_t cmd) {
  /* Wait for the FDC MSR RQM (Request for Master) bit to be set */
  while ((inb(FDC_MSR) & 0x80) == 0)
    ;
  outb(FDC_FIFO, cmd);
}

/**
 * floppy_recalibrate: Drives the floppy physical read head back to Track 0.
 */
int floppy_recalibrate(void) {
  printk("<6>[  %s  ] Recalibrating drive reader arm to track 0...\n",
         MODULE_NAME);

  floppy_write_cmd(CMD_RECAL);
  floppy_write_cmd(0); /* Drive 0 select */

  /* Hardware mechanical latency delay simulation loop */
  for (volatile int i = 0; i < 100000; i++)
    ;

  return 0;
}

/**
 * floppy_read_sector: Requests a physical 512-byte block transfer using CHS
 * parameters.
 */
int floppy_read_sector(int cylinder, int head, int sector, uint8_t *buffer) {
  if (!buffer)
    return -1;

  /* Turn on drive motor if currently idle */
  if (!g_floppy_motor_on) {
    outb(FDC_DOR, 0x1C); /* Turn on Motor A, select Drive A, enable IRQ/DMA */
    g_floppy_motor_on = 1;
    for (volatile int i = 0; i < 50000; i++)
      ; /* Motor spin-up delay timeout */
  }

  printk("<7>[  %s  ] CHS Read Request -> C:%d H:%d S:%d\n", MODULE_NAME,
         cylinder, head, sector);

  /* Setup native FDC multi-byte command stream packets */
  floppy_write_cmd(CMD_READ |
                   0xE0); /* Enable Multitrack, MFM framing configuration */
  floppy_write_cmd((head << 2) | 0); /* Head number and Drive 0 token mapping */
  floppy_write_cmd(cylinder);
  floppy_write_cmd(head);
  floppy_write_cmd(sector);
  floppy_write_cmd(2);    /* 2 = 512 bytes sector code definition standard */
  floppy_write_cmd(18);   /* End of track sector count threshold */
  floppy_write_cmd(27);   /* Gap 3 length descriptor legacy parameter */
  floppy_write_cmd(0xFF); /* Data length fallback flag */

  /* In a real scenario, the DMA channel 2 interrupt handler would copy raw
   * memory bytes to 'buffer' */
  return 0;
}

/**
 * floppy_init: Probes hardware nodes and provisions drive context structures.
 */
void floppy_init(void) {
  boot_msg(MODULE_NAME,
           "Initializing Legacy Floppy Disk Controller Subsystem...", 0);

  /* Query CMOS register 0x10 to detect fitted floppy drives natively */
  outb(0x70, 0x10);
  uint8_t drives = inb(0x71);

  if (drives >> 4 == 4) {
    boot_msg(MODULE_NAME, "Found Drive A: 1.44MB 3.5\" High-Density Floppy.",
             0);
    floppy_recalibrate();
  } else {
    printk("<4>[  %s  ] Warning: No legacy floppy drive signature found inside "
           "CMOS layout.\n",
           MODULE_NAME);
  }
}