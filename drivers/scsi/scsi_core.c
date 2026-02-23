#include <drivers/scsi.h>
#include <blueos/printk.h>
#include <lib/string.h>
#include <blueos/ports.h>
#include <blueos/colors.h>

unsigned int LSI_BASE_PORT = 0;

extern unsigned int pci_find_lsi_scsi();

int scsi_send_command(scsi_cdb10_t *cdb, void *data_buffer) {
    if (LSI_BASE_PORT == 0) return -1;

    int timeout = 1000000;
    while ((inb(LSI_BASE_PORT + 0x14) & 0x01) && timeout > 0) {
        timeout--;
    }

    if (timeout == 0) {
        printk(RED, "[ SCSI ] Timeout esperando al controlador.\n");
        return -1;
    }

    outb(LSI_BASE_PORT, cdb->opcode);
    
    return 0;
}


void scsi_init() {
    unsigned int port = pci_find_lsi_scsi();
    
    if (port == 0) {
        printk(RED, "[ SCSI ] Error: No LSI controller found on PCI bus.\n");
        return;
    }

    LSI_BASE_PORT = port; 

    scsi_cdb10_t inq;
    memset(&inq, 0, sizeof(scsi_cdb10_t));
    inq.opcode = SCSI_CMD_INQUIRY; 

    printk(YELLOW, "[ SCSI ] Probing device at port 0x%x...\n", LSI_BASE_PORT);

    if (scsi_send_command(&inq, NULL) == 0) {
        printk(GREEN, "[ SCSI ] Device detected and ready.\n");
    }
}