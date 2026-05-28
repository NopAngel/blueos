#include <stdint.h>
#include <kernel/io.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

#define ISAPNP_ADDR  0x279
#define ISAPNP_WRITE 0xA79
#define ISAPNP_READ  0x203

void isapnp_send_init_key() {
    uint8_t key = 0x6A;
    
    outb(ISAPNP_ADDR, 0x00);
    outb(ISAPNP_ADDR, 0x00);

    for (int i = 0; i < 32; i++) {
        outb(ISAPNP_ADDR, key);
        uint8_t bit = ((key & 1) ^ ((key >> 1) & 1)) << 7;
        key = (key >> 1) | bit;
    }
}
void isapnp_isolate() {
    isapnp_send_init_key();
    
    outb(ISAPNP_ADDR, 0x02); 
    outb(ISAPNP_WRITE, 0x01); 

    outb(ISAPNP_ADDR, 0x03); // reg WAKE[0]
    outb(ISAPNP_WRITE, ISAPNP_READ >> 2); 

    printk(GREEN, "ISA PnP: Initiating card isolation...\n");

    outb(ISAPNP_ADDR, 0x06); 
    outb(ISAPNP_WRITE, 0x01); 
}

void isapnp_init() {
    printk(GREEN, "BlueOS: Loading ISA PnP Enumerator...\n");
    
    isapnp_isolate();

    outb(ISAPNP_ADDR, 0x00);
    uint8_t vendor1 = inb(ISAPNP_READ);
    
    if (vendor1 != 0xFF) {
        printk(GREEN, "ISA PnP: Card detected on the bus.\n");
    } else {
        printk(RED,"ISA PnP: No legacy devices were found.\n");
    }
}