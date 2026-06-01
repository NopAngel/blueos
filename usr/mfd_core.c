#include <kernel/printk.h>
#include <kernel/colors.h>

/**
 * mfd_add_devices - Registra sub-dispositivos de un chip multifunción
 */
void mfd_add_devices(const char* parent_name, int id) {
    printk("[  MFD  ] Registering sub-devices for %s (ID: %d)\n", parent_name, id);
}

int mfd_init() {
    printk("MFD: Multi-Function Device Core initialized.\n");
    return 0;
}
