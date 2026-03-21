#include <drivers/ps3_ds3.h>
#include <kernel/printk.h>


void ds3_init() {
    printk(CYAN, "[PS3] DualShock 3 Driver Loaded. Waiting for controller...\n");
}


void ds3_handle_packet(uint8_t *buf, int len) {
    if (len < sizeof(struct ds3_report)) return;

    struct ds3_report *ds3 = (struct ds3_report *)buf;

    if (ds3->ps_button & DS3_BUTTON_PS) {
        printk(YELLOW, "[PS3] PS Button pressed! Returning to BlueOS Home...\n");
    }

    if (ds3->left_stick_x < 50 || ds3->left_stick_x > 200) {
        // printk(WHITE, "Stick L moved: X=%d\n", ds3->left_stick_x);
    }
}


void ds3_set_leds(uint8_t led_mask) {
    uint8_t report[48] = {0};
    report[0] = 0x01; // Report ID
    report[9] = led_mask; 
    
    // usb_send_control(report, 48);
}