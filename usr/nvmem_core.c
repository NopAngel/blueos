#include <kernel/printk.h>

void nvmem_register_provider() {
    printk("[ NVMEM ] New non-volatile memory provider registered.\n");
}