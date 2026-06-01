#include <drivers/scsi.h>
#include <kernel/printk.h>
#include <lib/string.h>
#include <kernel/ports.h>
#include <kernel/colors.h>
#include <kernel/hal.h>

/*
 * On x86, this is usually an I/O Port.
 * On RISC-V, this is a Memory Mapped I/O address.
 */
unsigned int LSI_BASE_ADDR = 0;

extern unsigned int pci_find_lsi_scsi();

/**
 * Low-level register write abstraction
 */
static inline void lsi_write8(unsigned short offset, unsigned char val) {
#ifdef x86
    outb(LSI_BASE_ADDR + offset, val);
#else
    volatile unsigned char *reg = (volatile unsigned char *)(LSI_BASE_ADDR + offset);
    *reg = val;
#endif
}

/**
 * Low-level register read abstraction
 */
static inline unsigned char lsi_read8(unsigned short offset) {
#ifdef x86
    return inb(LSI_BASE_ADDR + offset);
#else
    volatile unsigned char *reg = (volatile unsigned char *)(LSI_BASE_ADDR + offset);
    return *reg;
#endif
}

/**
 * Sends a SCSI Command Descriptor Block (CDB) to the controller
 * @param cdb Pointer to the 10-byte command structure
 * @param data_buffer Buffer for data transfer (unused in this basic stub)
 * @return 0 on success, -1 on failure or timeout
 */
int scsi_send_command(scsi_cdb10_t *cdb, void *data_buffer) {
    if (LSI_BASE_ADDR == 0) return -1;

    /* Wait for the controller to be ready (checking status register) */
    int timeout = 1000000;
    while ((lsi_read8(0x14) & 0x01) && timeout > 0) {
        timeout--;
    }

    if (timeout == 0) {
        printk("[ SCSI ] Error: Controller timeout.\n");
        return -1;
    }

    /* Send the opcode to the data register */
    lsi_write8(0x00, cdb->opcode);

    return 0;
}

/**
 * Initializes the LSI SCSI controller by scanning the PCI bus
 */
void scsi_init() {
    /* PCI scan should return the BAR (Base Address Register) */
    unsigned int addr = pci_find_lsi_scsi();

    if (addr == 0) {
        printk("[ SCSI ] Error: No LSI controller found on PCI bus.\n");
        return;
    }

    LSI_BASE_ADDR = addr;

    /* Prepare a SCSI Inquiry command to probe the device */
    scsi_cdb10_t inq;
    memset(&inq, 0, sizeof(scsi_cdb10_t));
    inq.opcode = 0x12; // SCSI_CMD_INQUIRY

    printk("[ SCSI ] Probing device at %s 0x%x...\n",
#ifdef x86
        "port",
#else
        "address",
#endif
        LSI_BASE_ADDR);

    /* Send the initial probe command */
    if (scsi_send_command(&inq, NULL) == 0) {
        printk("[ SCSI ] Success: Device detected and ready.\n");
    }
}
