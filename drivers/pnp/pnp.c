#include <stdint.h>
#include <kernel/colors.h>

#define MAX_PNP_DEVICES 32

typedef struct {
    uint32_t device_id;
    uint16_t io_base;
    uint8_t  irq;
    const char* name;
    int is_active;
} pnp_device_t;

static pnp_device_t pnp_registry[MAX_PNP_DEVICES];
static uint32_t device_count = 0;

void pnp_init() {
    printk(CYAN, "[PnP] Starting Plug and Play subsystem...\n");
    
    pnp_register_device("PS/2 Keyboard", 0x60, 1);
    pnp_register_device("PS/2 Mouse", 0x60, 12);
    pnp_register_device("COM1 Serial Port", 0x3F8, 4);
    pnp_register_device("RTC Clock", 0x70, 8);
}

void pnp_register_device(const char* name, uint16_t io, uint8_t irq) {
    if (device_count < MAX_PNP_DEVICES) {
        pnp_registry[device_count].name = name;
        pnp_registry[device_count].io_base = io;
        pnp_registry[device_count].irq = irq;
        pnp_registry[device_count].is_active = 1;
        device_count++;
        printk(GREEN, "[PnP] Device detected: %s in I/O 0x%x, IRQ %d\n", name, io, irq);
    }
}