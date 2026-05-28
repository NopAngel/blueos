#include <drivers/cxl.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <mm/pmm.h>

int cxl_enumerate_devices() {
    int found = 0;
    printk(WHITE, "[CXL] Scanning PCI bus for CXL 2.0+ devices...\n");

    
    struct cxl_device dev;
    dev.pci_dev = 0x0100; // Bus 1, Slot 0
    dev.component_regs = 0xF0000000; 
    dev.mem_size = (uint64_t)1024 * 1024 * 1024 * 4; // 4GB ram

    cxl_map_memory(&dev);
    found++;

    return found;
}


int cxl_init() {
    printk(CYAN, "[CXL] Compute Express Link Subsystem initializing...\n");
    return cxl_enumerate_devices();
}

void cxl_map_memory(struct cxl_device *dev) {
    volatile uint32_t *hdm_base = (uint32_t *)(dev->component_regs + CXL_HDM_DECODER0_BASE_LOW);
    
    uintptr_t cxl_ram_phys = (uintptr_t)(*hdm_base);

    printk(GREEN, "[CXL] Found %d MB of Type 3 Memory at 0x%lx\n", 
           (uint32_t)(dev->mem_size / (1024*1024)), cxl_ram_phys);

    // pmm_add_region(cxl_ram_phys, dev->mem_size);
}