#include <drivers/ps3_ds3.h>
#include <kernel/printk.h>
#include <kernel/arch.h>
#include <kernel/task.h>

void ds3_init() {
    printk(CYAN, "[PS3] DualShock 3 Driver Loaded. Waiting for controller...\n");
}

void ds3_handle_packet(uint8_t *buf, int len) {
    if (len < 49) return; 

    struct ds3_report *ds3 = (struct ds3_report *)buf;

    if (ds3->ps_button & DS3_BUTTON_PS) {
        printk(YELLOW, "\r[PS3] HOME Button: Returning to BlueOS...      ");
    }

    if (ds3->left_stick_x < 50 || ds3->left_stick_x > 200 || 
        ds3->left_stick_y < 50 || ds3->left_stick_y > 200) {
        printk(WHITE, "\r[PS3] L-Stick: X=%d Y=%d                  ", 
               ds3->left_stick_x, ds3->left_stick_y);
    }
}

