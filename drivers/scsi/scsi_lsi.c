#include <blueos/ports.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

#define LSI_BASE_PORT 0xC000

#define LSI_REG_ISTAT  (LSI_BASE_PORT + 0x14) 
#define LSI_REG_DSTAT  (LSI_BASE_PORT + 0x0C) 
#define LSI_REG_SIST0  (LSI_BASE_PORT + 0x42) 

void scsi_lsi_check() {
    // Leemos el status del controlador
    unsigned char istat = inb(LSI_REG_ISTAT);
    
    printk(WHITE, "[ SCSI ] LSI Controller ISTAT: 0x%x\n", istat);
    
    if (istat == 0xFF) {
        printk(RED, "Error: Controlador no responde en 0xC000\n");
    } else {
        printk(GREEN, "Controlador LSI detectado y respondiendo en BAR0!\n");
    }
}