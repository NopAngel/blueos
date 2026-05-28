#include <kernel/hibernate.h>
#include <kernel/printk.h>

struct hibernate_image_struct hibernate_image;

extern void machine_power_off();
extern void disable_interrupts();

void do_hibernate() {
    printk(YELLOW, "[PWR] Starting hibernation snapshot...\n");

    disable_interrupts();

    if (save_system_context(&hibernate_image.cpu_ctx) == 0) {
        
        hibernate_image.magic = 0x424C5545; // "BLUE"
        
        // disk_write(PARTITION_ID, RAM_START, RAM_SIZE);

        printk(RED, "System frozen. Shutting down hardware...\n");
        machine_power_off();
    } else {
        printk(GREEN, "BlueOS has come back to life from the disk!\n");
    }
}